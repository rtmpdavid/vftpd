#include "session.h"

#include "ftp.h"
#include "fs_util.h"
#include "net_util.h"
#include "util.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <malloc.h>

#include <arpa/inet.h>

int add_session(int listener, psession* sessions, int *n_sessions, const char* _srv_dir, const char* _host, const char* _bind)
{
  session *new_session = malloc(sizeof(session));

  new_session->addr = malloc(sizeof(struct sockaddr_in));
  new_session->addrlen = sizeof(struct sockaddr_in);
  struct sockaddr_in *addr = (struct sockaddr_in *)(new_session->addr);

  new_session->socket = accept(listener,
			       (struct sockaddr *)(new_session->addr),
			       &(new_session->addrlen));

  if(new_session->socket == -1)
  {
    free(addr);
    print_err("Failed to accept a new connection\n");
    return -1;
  } 

  if(*n_sessions < MAX_SESSIONS)
  {
    sessions[*n_sessions] = new_session;
    *n_sessions+=1;

    new_session->srv_dir = _srv_dir;
    new_session->host_ip = _host;
    new_session->bind_ip = _bind;

    new_session->blocks_transferred = 0;
    new_session->data_socket = -1;
    new_session->pasv_listener = -1;
    new_session->data_addr = malloc(sizeof(struct sockaddr_in));

    struct sockaddr_in *addr_data = (struct sockaddr_in *)(new_session->data_addr);

    new_session->transfer_kind = -1;
    addr_data->sin_family = addr->sin_family;
    addr_data->sin_port = addr->sin_port;
    addr_data->sin_addr.s_addr= addr->sin_addr.s_addr;
    new_session->data_addrlen = sizeof(struct sockaddr_in);
    memset(addr_data->sin_zero, 0, 8);
    
    strcpy(new_session->client_dir, "/");
    new_session->type = 'I';

    new_session->utf8_on = 0;

    printf("A new client has connected from %s.\n", 
	   inet_ntoa(addr_data->sin_addr));

    ftp_send_status(new_session, SERVICE_READY, "vftpd ready");
    return 0;
  }
  else
  {
    free(addr);
    ftp_send_status(new_session, SERVICE_NOT_READY, NULL);
    close(new_session->socket);
    printf("Too much connections, closing connection.\n");
    return 1;
  }
}

int rem_session(int session_n, psession* sessions, int *n_sessions)
{
  session *sess = sessions[session_n];
  close(sess->socket);
  close(sess->data_socket);
  free(sess);

  for(int i = session_n; i < (*n_sessions)-1; i++)
  {
    sessions[i] = sessions[i+1];
  }

  (*n_sessions)-=1;

  return 0;
}

int xfer_block(psession session)
{
  int rv = fseek(session->xfer_file, session->blocks_transferred, SEEK_SET);

  if(rv)
    return XFER_SEEK_FAILED;

  uint8_t buff[MTU];

  int read_num = fread(buff, 1, MTU, session->xfer_file);

  if(rv < 0)
    return XFER_READ_FAILED;

  int send_num = send(session->data_socket, buff, read_num, 0);

  if(send_num < 0)
  {
    printf("Failed to send data\n");
    return XFER_SEND_FAILED;
  }

  session->blocks_transferred += send_num;

  if(feof(session->xfer_file))
    return XFER_DONE;

  return 0;
}

int xfer_done(psession session, int status)
{
  printf("Transferred %lu bytes\n", session->blocks_transferred);

  session->xfer = XFER_NONE;
  session->blocks_transferred = 0;
  close_data_con(session);

  switch(status)
  {
  case XFER_SEEK_FAILED:
  case XFER_READ_FAILED:
    ftp_send_status(session, FILE_ACTION_ERROR, "Error reading file");
    break;
  case XFER_SEND_FAILED:
    ftp_send_status(session, CON_CLOSED_XFER_ABORTED, "Error sending data");
    break;
  case XFER_DONE:
    ftp_send_status(session, ACTION_OK_CLOSING, "File transfer complete");
    break;
  case XFER_ABORTED:
    {
      fseek(session->xfer_file, 0L, SEEK_END);
      size_t size = ftell(session->xfer_file);
      
      if(session->blocks_transferred <  size)
	ftp_send_status(session, CON_CLOSED_XFER_ABORTED, "File transfer aborted");
      ftp_send_status(session, ACTION_OK_CLOSING, "Transfer done");
    }
    break;
  }

  fclose(session->xfer_file);
  return 0;
}
