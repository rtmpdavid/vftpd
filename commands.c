#include "session.h"
#include "commands.h"
#include "ftp.h"
#include "fs_util.h"
#include "net_util.h"

#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <dirent.h>

#include <sys/ioctl.h>
#include <linux/if.h>
#include <linux/sockios.h>

typedef struct {
  char* str;
  command_handler handler;
}command_spec;

command_spec commands[] = {{"ABOR", &command_abor},
			   {"ACCT", &command_acct},
			   {"ALLO", &command_allo},
			   {"APPE", &command_appe},
			   {"CWD",  &command_cwd},
			   {"PWD",  &command_pwd},
			   {"DELE", &command_dele},
			   {"HELP", &command_help},
			   {"LIST", &command_list},
			   {"MODE", &command_mode},
			   {"NLST", &command_nlst},
			   {"NOOP", &command_noop},
			   {"PASS", &command_pass},
			   {"PASV", &command_pasv},
			   {"PORT", &command_port},
			   {"QUIT", &command_quit},
			   {"REIN", &command_rein},
			   {"REST", &command_rest},
			   {"RETR", &command_retr},
			   {"RNFR", &command_rnfr},
			   {"RNTO", &command_rnto},
			   {"SITE", &command_site},
			   {"STAT", &command_stat},
			   {"STOR", &command_stor},
			   {"STRU", &command_stru},
			   {"SYST", &command_syst},
			   {"TYPE", &command_type},
			   {"USER", &command_user},
			   {"FEAT", &command_feat},
			   {"OPTS", &command_opts},
			   {"SIZE", &command_size},
			   {"CDUP", &command_cdup}};

command_handler command_parse(char* cmd, char** arg)
{
  strtok(cmd, " \r\n");

  *arg = strtok(NULL, "\r\n");

  for(size_t i = 1; i < sizeof(commands)/sizeof(command_spec); i++)
  {
    if(!strcmp(cmd, commands[i].str))
      return commands[i].handler;
  }


  return &command_not_implemented;
}

int command_not_implemented(psession session, const char* arg __attribute__ ((unused)))
{
  ftp_send_status(session, NOT_IMPLEMENTED, "Command not implemented");
  return CMD_NOT_IMPLEMENTED;
}

int command_abor(psession session, const char* arg __attribute__ ((unused)))
{
  int rv = 0;
  switch(session->xfer)
  {
  case XFER_RETR:
    xfer_done(session, XFER_ABORTED);
    rv = CMD_DONE;
    break;
  default:
    ftp_send_status(session, NOT_IMPLEMENTED, "Command not implemented");
    rv = CMD_ERR;
    break;
  }
  return rv;
}

int command_acct(psession session, const char* arg __attribute__ ((unused)))
{
  ftp_send_status(session, NOT_IMPL_SUPERFLUOUS, NULL);
  return CMD_NOT_IMPLEMENTED;
}

int command_allo(psession session, const char* arg __attribute__ ((unused)))
{
  ftp_send_status(session, NOT_IMPLEMENTED, "Command not implemented");
  return CMD_NOT_IMPLEMENTED;
}

int command_appe(psession session, const char* arg __attribute__ ((unused)))
{
  ftp_send_status(session, NOT_IMPLEMENTED, "Command not implemented");
  return CMD_NOT_IMPLEMENTED;
}

int command_cwd(psession session, const char* dir)
{
  int rv = CMD_DONE;
  int status = ACTION_OK;
  char *status_str;

  if(!strcmp(dir, ".."))
  {
    if(strcmp(session->client_dir, "/"))
      for(int i = strlen(session->client_dir)-1; i >= 0; i--)
	if((session->client_dir)[i] == '/')
	{
	  if(i > 1)
	    (session->client_dir)[i] = '\0';
	  else
	    (session->client_dir)[i+1] = '\0';
	  break;
	}
  }
  else
  {
    char abs_filename[255];
    file_abs_name(abs_filename, dir, session->srv_dir, session->client_dir);

    if((!file_exists(abs_filename)) || !file_is_dir(abs_filename))
      status = FILE_ANAVAILABLE;
    else
    {
      char sess_abs_filename[255];
      file_abs_name(sess_abs_filename, dir, NULL, session->client_dir);
      strcpy(session->client_dir, sess_abs_filename);
    }
  }

  if(status == ACTION_OK)
    status_str = "Dir changed";
  else
    status_str = "Failed to change dir";

  ftp_send_status(session, status, status_str);
  return rv;
}

int command_pwd(psession session, const char* arg __attribute__ ((unused)))
{
  char dir[255];
  sprintf(dir, "\"%s\"", session->client_dir);
  ftp_send_status(session, PATHNAME_CREATED, dir);

  return CMD_DONE;
}

int command_dele(psession session, const char* arg __attribute__ ((unused)))
{
  ftp_send_status(session, NOT_IMPLEMENTED, "Command not implemented");
  return CMD_NOT_IMPLEMENTED;
}

int command_help(psession session, const char* arg __attribute__ ((unused)))
{
  ftp_send_status(session, NOT_IMPLEMENTED, "Command not implemented");
  return CMD_NOT_IMPLEMENTED;
}

int command_list(psession session, const char* arg)
{
  if(arg != NULL && ( (!strcmp(arg, "-a")) || (!strcmp(arg, "-l")) ) )
  {
    printf("Listing current dir\n");
    return command_list(session, NULL);
  }

  if(open_data_con(session))
  {
    printf("Failed to open data connection.\n");
    ftp_send_status(session, CANT_OPEN, "Failed to open data connection");
    return CMD_ERR;
  }

  char dir[255] = {0};
  file_abs_name(dir, arg, session->srv_dir, session->client_dir);

  DIR *dp = opendir(dir);
  struct dirent *entry = NULL;

  if(dp)
  {
    while((entry = readdir(dp)))
    {
      char *file_rel_name = strrchr(entry->d_name, '/');
      if(!file_rel_name)
	file_rel_name = entry->d_name;
      else
	file_rel_name = file_rel_name+1;

      if( !strcmp(file_rel_name, "..") || !(strcmp(file_rel_name, ".")) )
	continue;

      char stat[255];
      if(file_stat_string(stat, dir, file_rel_name) < 0)
	continue;

      send_record(session, stat, strlen(stat));
    }

    closedir(dp);
  }
  else
    ftp_send_status(session, FILE_ACTION_FILE_ANAVAILABLE, "failed to list directory");

  ftp_send_status(session, ACTION_OK_CLOSING, "directory list sent");
  close_data_con(session);
  return CMD_DONE;
}

int command_mode(psession session, const char* arg __attribute__ ((unused)))
{
  ftp_send_status(session, NOT_IMPLEMENTED, "Command not implemented");
  return CMD_NOT_IMPLEMENTED;
}

int command_nlst(psession session, const char* arg __attribute__ ((unused)))
{
  ftp_send_status(session, NOT_IMPLEMENTED, "Command not implemented");
  return CMD_NOT_IMPLEMENTED;
}

int command_noop(psession session, const char* arg __attribute__ ((unused)))
{
  ftp_send_status(session, COMMAND_OK, NULL);
  return CMD_DONE;
}

int command_pass(psession session, const char* arg __attribute__ ((unused)))
{
  ftp_send_status(session, NOT_IMPLEMENTED, "Command not implemented");
  return CMD_NOT_IMPLEMENTED;
}

int command_pasv(psession session, const char* arg __attribute__ ((unused)))
{
  int listener = prepare_listener(session->bind_ip, "0");
  struct sockaddr_in listener_addr;
  socklen_t addr_len = sizeof(listener_addr);;

  getsockname(listener, (struct sockaddr*)&listener_addr, &addr_len);
  int listener_port = listener_addr.sin_port;

  char host_ip[32];
  strcpy(host_ip, session->host_ip);
  for(char* c = host_ip; *c != '\0'; c++)
    if(*c == '.')
      *c = ',';

  char pasv_info[255];
  sprintf(pasv_info ,"Entering passive mode (%s,%u,%u)", host_ip, (listener_port&0xFF), (listener_port&0xFF00)>>8);

  session->transfer_kind = XFER_PASSVE;
  session->pasv_listener = listener;
  ftp_send_status(session, ENTERING_PASSIVE, pasv_info);
  return CMD_DONE;
}

int command_port(psession session, const char* arg)
{
  int status;
  struct sockaddr *addr = (struct sockaddr *)(session->data_addr);
  socklen_t *socklen = &(session->data_addrlen);

  if(parse_arg_port(arg, addr, socklen))
    status = COMMAND_OK;
  else
    status = SYNTAX_ERROR_ARG;

  char status_str[255];
  if(status == COMMAND_OK)
    sprintf(status_str, "%s  -Ready for transfer", (session->type=='I')?"BINARY":"ASCII");
  else
    sprintf(status_str, "Error");

  session->transfer_kind = XFER_ACTIVE;
  ftp_send_status(session, status, status_str);
  return CMD_DONE;
}

int command_quit(psession session, const char* arg __attribute__ ((unused)))
{
  ftp_send_status(session, CONTROL_CLOSE, "sayoonara");
  return CMD_DONE_QUIT;
}

int command_rein(psession session, const char* arg __attribute__ ((unused)))
{
  ftp_send_status(session, NOT_IMPLEMENTED, "Command not implemented");
  return CMD_NOT_IMPLEMENTED;
}

int command_rest(psession session, const char* arg __attribute__ ((unused)))
{
  ftp_send_status(session, NOT_IMPLEMENTED, "Command not implemented");
  return CMD_NOT_IMPLEMENTED;
}

int command_retr(psession session, const char* arg)
{
  char abs_file_loc[255];
  file_abs_name(abs_file_loc, arg, session->srv_dir, session->client_dir);

  FILE *send_file = fopen(abs_file_loc, "rb");

  if(!send_file)
  {
    ftp_send_status(session, FILE_ACTION_FILE_ANAVAILABLE, "failed to list directory");
    return -1;
  }

  if(open_data_con(session))
    return CMD_ERR;

  session->xfer = XFER_RETR;
  session->xfer_file = send_file;
  return CMD_DONE;
}

int command_rnfr(psession session, const char* arg __attribute__ ((unused)))
{
  ftp_send_status(session, NOT_IMPLEMENTED, "Command not implemented");
  return CMD_NOT_IMPLEMENTED;
}

int command_rnto(psession session, const char* arg __attribute__ ((unused)))
{
  ftp_send_status(session, NOT_IMPLEMENTED, "Command not implemented");
  return CMD_NOT_IMPLEMENTED;
}

int command_site(psession session, const char* arg __attribute__ ((unused)))
{
  ftp_send_status(session, NOT_IMPLEMENTED, "Command not implemented");
  return CMD_NOT_IMPLEMENTED;
}

int command_stat(psession session, const char* arg __attribute__ ((unused)))
{
  ftp_send_status(session, NOT_IMPLEMENTED, "Command not implemented");
  return CMD_NOT_IMPLEMENTED;
}

int command_stor(psession session, const char* arg __attribute__ ((unused)))
{
  ftp_send_status(session, NOT_IMPLEMENTED, "Command not implemented");
  return CMD_NOT_IMPLEMENTED;
}

int command_stru(psession session, const char* arg __attribute__ ((unused)))
{
  ftp_send_status(session, NOT_IMPLEMENTED, "Command not implemented");
  return CMD_NOT_IMPLEMENTED;
}

int command_syst(psession session, const char* arg __attribute__ ((unused)))
{
  ftp_send_status(session, SYSTEM_TYPE, "UNIX Type: L8");
  return CMD_DONE;
}

int command_type(psession session, const char* arg)
{
  switch(arg[0])
  {
  case 'A':
  case 'I':
    session->type = arg[0];
    ftp_send_status(session, COMMAND_OK, "Transfer type changed");
    break;
  case 'L':
  case 'E':
    ftp_send_status(session, NOT_IMPLEMENTED_ARG, "Type not implemented");
    break;
  default:
    ftp_send_status(session, SYNTAX_ERROR_ARG, "Unrecognized type");
    break;
  }

  return CMD_DONE;
}

int command_user(psession session, const char* arg __attribute__ ((unused)))
{
  ftp_send_status(session, FTP_GREETING, "No user required, you may proceed");
  return CMD_DONE;
}

int command_feat(psession session, const char* arg __attribute__ ((unused)))
{
  const char *feats[] = {"FEAT",
			 "UTF8",
			 NULL};

  ftp_send_feats(session, feats);

  return CMD_DONE;
}

int command_opts(psession session, const char* arg)
{
  char opt[255];
  strcpy(opt, arg);

  char* feat = strtok(opt, " ");
  char* feat_arg = strtok(opt, " ");

  if(!strcmp(feat, "UTF8"))
  {
    int ok = 1;
    if(!strcmp(feat_arg, "ON"))
      session->utf8_on = 1;
    else if(!strcmp(feat_arg, "OFF"))
      session->utf8_on = 0;
    else
      ok = 0;

    if(ok)
      ftp_send_status(session, COMMAND_OK, feat_arg);
    else
      ftp_send_status(session, SYNTAX_ERROR_ARG, "Syntax error");
  }
  else
    ftp_send_status(session, SYNTAX_ERROR_ARG, "Feat not supported");
  return CMD_ERR;
}

int command_size(psession session, const char* arg)
{
  char ret[255];
  char file[255] = {0};
  file_abs_name(file, arg, session->srv_dir, session->client_dir);
  

  if(!file_exists(arg))
  {
    sprintf(ret, "failed to stat file %s", arg);
    ftp_send_status(session, FILE_ACTION_FILE_ANAVAILABLE, ret);
    return CMD_ERR;
  }

  ftp_send_status(session, FILE_STAT, ret);
  return CMD_DONE;
}

int command_cdup(psession session, const char* arg __attribute__((unused)))
{
  return command_cwd(session, "..");
}
