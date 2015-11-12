#include "ftp.h"
#include "util.h"

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int ftp_send_feats(session* session, const char** feats)
{
  const char* feat_send_str = "211-Extensions supported";
  write(session->socket, feat_send_str, strlen(feat_send_str));

  for( unsigned int i = 0; feats[i] != NULL; i++)
  {
    char feat[32];
    int len = sprintf(feat, " %s\r\n", feats[i]);
    write(session->socket, feat, len);
  }
  
  ftp_send_status(session, EXT_SUPPORT, "End");

  return 0;
}

int ftp_send_status(session* session, int status, const char* arg)
{
  char cmd[MAX_CMD_LEN];
  int cmd_len = 0;  

  if(arg)
    cmd_len = sprintf(cmd, "%i %s\r\n", status, arg);
  else
    cmd_len = sprintf(cmd, "%i\r\n", status);

  return write(session->socket, cmd, cmd_len);
}

int ftp_read_cmd(session* session, char* cmd)
{
  size_t cmd_size = read(session->socket, cmd, MAX_CMD_LEN);

  return cmd_size;
}

int send_record(psession session, const char* record, int rec_len)
{
  int count = 0;
  int rv = 1;
  for(;;)
  {
    rv = send(session->data_socket, record+count, rec_len-count, 0);
    if(rv <= 0)
      break;
    count += rv;
  
  } 

  rv = send(session->data_socket, "\r\n", 2, 0);
  return rv;
}

int open_data_con(psession session)
{
  if(session->data_socket > 0)
  {
    ftp_send_status(session, ALREADY_OPEN_SENDING, "data connection already open");
    return 0;
  }

  if(session->transfer_kind == XFER_ACTIVE)
  {
    ftp_send_status(session, ABOUT_TO_OPEN, "opening data connection");

    session->data_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(session->data_socket == -1)
    {
      print_err("Failed to init socket");
      ftp_send_status(session, CANT_OPEN, "failed to open data connection");
      return -1;
    }
  
    if(connect(session->data_socket, session->data_addr, session->data_addrlen) == -1)
    {
      print_err("Failed to connect socket");
      ftp_send_status(session, CANT_OPEN, "failed to open data connection");
      return -1;
    }
    printf("Opened data connection on socket %i\n", session->data_socket);
  }
  else
  {
    ftp_send_status(session, ABOUT_TO_OPEN, "Waiting for a connection to passive listener ");
    session->data_socket = accept(session->pasv_listener, (struct sockaddr*)(session->data_addr), &(session->data_addrlen));
    close(session->pasv_listener);
  }

  return 0;
}

int close_data_con(psession session)
{
  printf("Closed data connection socket %i\n", session->data_socket);
  int rv = close(session->data_socket);

  session->data_socket = -1;
  
  return rv;
}
