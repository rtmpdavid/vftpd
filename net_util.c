#include "net_util.h"
#include "util.h"

#include <stdlib.h>
#include <string.h>

int parse_arg_port(const char* arg_in, struct sockaddr *data_address, socklen_t *data_addrlen)
{
  char arg[255];
  strcpy(arg, arg_in);

  size_t arg_len = strlen(arg);
  int comma_count = 0;
  for(unsigned int i = 0; i < arg_len; i++)
  {
    if(arg[i] == ',')
      comma_count++;
  }

  if(comma_count != 5)
    return 1;

  comma_count = 0;
  char* pch = strtok(arg, ",");
  char addr[16] = {0};
  char porth[4] = {0};
  char portl[4] = {0};
  while(pch != NULL)
  {
    if(comma_count < 4)
    {
      strcat(addr, pch);
      if(comma_count < 3)
	strcat(addr, ".");
    }
    else if(comma_count < 6)
    {
      if(comma_count == 4)
	strcpy(porth, pch);
      else
	strcpy(portl, pch);
    }

    pch = strtok(NULL, ",");
    comma_count++;
  }

  struct sockaddr_in* data_addr = (struct sockaddr_in*)data_address;
  data_addr->sin_family = AF_INET;
  data_addr->sin_port = htons((atoi(porth)<<8) | atoi(portl));
  memset(data_addr->sin_zero, 0, 8);
  *data_addrlen = sizeof(struct sockaddr_in);

  if(!inet_pton(AF_INET, addr, &(data_addr->sin_addr)))
  {
    print_err("Failed to convert host address");
    return -1;
  }

  printf("Using data address: %s, port: %i\n", addr, data_addr->sin_port);

  return -1;
}

int prepare_listener(const char* host, const char* port)
{
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(atoi(port));
  memset(addr.sin_zero, 0, 8);
  if(!inet_pton(AF_INET, host, &(addr.sin_addr)))
  {
    print_err("Failed to convert host address");
    return -1;
  }

  int listener_socket = socket(AF_INET, SOCK_STREAM, 0);
  if(listener_socket == -1)
  {
    print_err("Failed to make new socket");
    return -1;
  }
    
  if(bind(listener_socket, (struct sockaddr *)&addr, sizeof(addr)))
  {
    print_err("Failed to bind listener socket");
    return -1;
  }

  if (listen(listener_socket, 5) == -1)
  {
    print_err("Failed to listen to socket");
    return -1;
  }

  return listener_socket;
}
