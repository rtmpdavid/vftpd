#ifndef SESSION_H
#define SESSION_H

#include <netinet/in.h>
#include <stdint.h>

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

  int is_transferring;
  size_t blocks_transferred;

  int transfer_kind;
  const char *host_ip;
  const char *bind_ip;

  int pasv_listener;
}session, *psession;

#define MAX_SESSIONS 50

#define XFER_PASSVE 0
#define XFER_ACTIVE 1

int add_session(int listener, psession* sessions, int *n_sessions, const char* srv_dir, const char* host_ip, const char* bind_ip);
int rem_session(int session_n, psession* sessions, int *n_sessions);

#endif
