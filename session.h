#ifndef SESSION_H
#define SESSION_H

#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>

#define BLOCK_BYTE_COUNT_MAX UINT16_MAX

typedef struct session{
  int socket;
  struct sockaddr *addr;
  socklen_t addrlen;

  char type;

  int data_socket;
  struct sockaddr *data_addr;
  socklen_t data_addrlen;

  char client_dir[255];
  const char *srv_dir;

  int utf8_on;

  int xfer;
  size_t blocks_transferred;

  FILE *xfer_file;

  int transfer_kind;
  const char *host_ip;
  const char *bind_ip;

  int pasv_listener;
}session, *psession;

#define MAX_SESSIONS 50

#define XFER_PASSVE 0
#define XFER_ACTIVE 1

#define XFER_NONE 0
#define XFER_RETR 1
#define XFER_STOR 2

#define XFER_ABORTED 2
#define XFER_DONE 1
#define XFER_SEEK_FAILED -1
#define XFER_READ_FAILED -2
#define XFER_SEND_FAILED -3

int add_session(int listener, psession* sessions, int *n_sessions, const char* srv_dir, const char* host_ip, const char* bind_ip);
int rem_session(int session_n, psession* sessions, int *n_sessions);

int xfer_block(psession session);
int xfer_done(psession session, int status);

#endif
