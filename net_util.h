#ifndef NET_UTIL_H
#define NET_UTIL_H

#include <arpa/inet.h>
//#include <netinet/in.h>

int prepare_listener(const char* host, const char* port);

int parse_arg_port(const char* arg, struct sockaddr *data_addr, socklen_t *data_addrlen);

#endif
