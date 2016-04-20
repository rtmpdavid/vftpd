#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>

#include "util.h"
#include "net_util.h"
#include "session.h"
#include "commands.h"
#include "ftp.h"

int main(int argc, char**argv)
{
  char bind[255]       = "127.0.0.1";
  char host[255]       = "127.0.0.1";
  char port[255]       = "2131";

  char server_dir[255];
  getcwd(server_dir, 255);
  
  int opt;

  while ((opt = getopt(argc, argv, "b:h:p:d:")) != -1)
    switch (opt)
    {
    case 'h':
      strcpy(host, optarg);
      break;
    case 'b':
      strcpy(bind, optarg);
      break;
    case 'p':
      strcpy(port, optarg);
      break;
    case 'd':
      strcpy(server_dir, optarg);
      break;
    default:
      printf("Unknown arg: %c\n", opt);
      return -1;
      break;
    }

  int listener_socket = prepare_listener(bind, port);

  if(listener_socket >= 0)
    printf("Opened listening socket succesfullt, listening for clients on port %s.\n", port);
  else
    return -1;

  psession sessions[MAX_SESSIONS];
  int session_count = 0;

  fd_set rfds, xwfds;
  struct timeval tv;
  int retval;

  for(;;)
  {
    tv.tv_sec = 5;
    tv.tv_usec = 100;

    FD_ZERO(&rfds);
    FD_ZERO(&xwfds);

    FD_SET(listener_socket, &rfds);
    int max_fd = listener_socket;
    for(int i = 0; i < session_count; i++)
    {
      max_fd = MAX(max_fd, sessions[i]->socket);
      FD_SET(sessions[i]->socket, &rfds);

      if(sessions[i]->xfer == XFER_RETR)
      {
	FD_SET(sessions[i]->data_socket, &xwfds);
	max_fd = MAX(max_fd, sessions[i]->data_socket);
      }
    }

    retval = select(max_fd+1, &rfds, &xwfds, NULL, &tv);

    if(!retval)
      continue;

    if(FD_ISSET(listener_socket ,&rfds))
      add_session(listener_socket, sessions, &session_count, server_dir, host, bind);

    char cmd[MAX_CMD_LEN];
    for(int i = 0; i < session_count; i++)
    {
      session *sess = sessions[i];
      if(FD_ISSET(sess->socket ,&rfds))
      {
	memset(cmd, 0, MAX_CMD_LEN);
	int cmd_len = ftp_read_cmd(sess, cmd);

	if(cmd_len == 0)
	{
	  printf("Connection terminated (%p).\n", (sessions+i));
	  rem_session(i, sessions, &session_count);
	  i--;
	}
	else
	{
	  char* cmd_arg = NULL;
	  command_handler handler = command_parse(cmd, &cmd_arg);
	  printf("cmd: %s\targ: %s\t%s\n", cmd, cmd_arg, (handler == &command_not_implemented)?"Not implemented":"OK");

	  int status = handler(sess, cmd_arg);
	  
	  if(status == CMD_DONE_QUIT || status == CMD_ERR_QUIT)
	    rem_session(i, sessions, &session_count);
	}
      }

      if(sess->xfer == XFER_RETR)
      {
	if(FD_ISSET(sess->data_socket, &xwfds))
	{
	  int xfer_status = xfer_block(sess);

	  if(xfer_status)
	    xfer_done(sess, xfer_status);
	}
      }
    }
  }

  close(listener_socket);

  return 0;
}

