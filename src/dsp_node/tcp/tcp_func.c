//******************************************************************************
/// @file     tcp_func.c
/// @author   Jay Convertino(johnathan.convertino.1@us.af.mil)
/// @date     2023.08.07
/// @brief    TCP server/client nodes for single connection only (PTP).
//******************************************************************************

// standard includes
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "tcp_func.h"
#include "kill_throbber.h"

//Setup tcp arg struct for tcp server/client init callbacks
struct s_tcp_func_args *create_tcp_args(char *p_address, unsigned int port, enum e_binary_type data_type)
{
  struct s_tcp_func_args *p_temp = NULL;

  if(!p_address)
  {
    fprintf(stderr, "ERROR: Must specify address.\n");

    return NULL;
  }

  p_temp = malloc(sizeof(struct s_tcp_func_args));

  if(!p_temp) return NULL;

  p_temp->p_address = strdup(p_address);

  p_temp->input_type = data_type;

  p_temp->output_type = data_type;

  p_temp->port = port;

  return p_temp;
}

//Free args struct created from create tcp args
void free_tcp_args(struct s_tcp_func_args *p_init_args)
{
  if(!p_init_args)
  {
    fprintf(stderr, "ERROR: Null passed.\n");

    return;
  }

  free(p_init_args->p_address);

  free(p_init_args);
}

// THREAD COMMON FUNCTIONS //

// Setup TCP sockets
int init_callback_tcp(void *p_init_args, void *p_object)
{
  return 0;
}

// Clean up all allocations from init_callback
int free_callback_tcp(void *p_object)
{
  return 0;
}

// THREAD SERVER FUNCTIONS //

// @brief Pthread function for server threading
void* pthread_function_tcp_server(void *p_data)
{
  return NULL;
}

// THREAD CLIENT FUNCTIONS //

// @brief Pthread function for client threading
void* pthread_function_tcp_client(void *p_data)
{
  return NULL;
}
