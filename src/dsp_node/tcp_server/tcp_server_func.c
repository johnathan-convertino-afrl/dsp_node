//******************************************************************************
/// @file     tcp_server_func.c
/// @author   Jay Convertino(johnathan.convertino.1@us.af.mil)
/// @date     2023.08.07
/// @brief    TCP server node for single connection only (PTP).
//******************************************************************************

// standard includes
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <poll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "tcp_server_func.h"
#include "kill_throbber.h"
#include "logger.h"

static struct sockaddr_in *gp_socket_info = NULL;
static struct pollfd g_poll_connection;

static pthread_t connection_thread;

// PRIVATE FUNCTIONS //
void* connection_keep_alive(void *p_data);

//Setup tcp arg struct for tcp server/client init callbacks
struct s_tcp_func_args *create_tcp_args(char *p_address, unsigned short port, enum e_binary_type input_type, enum e_binary_type output_type)
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

  p_temp->input_type = input_type;

  p_temp->output_type = output_type;

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
  struct s_dsp_node *p_dsp_node = NULL;

  struct s_tcp_func_args *p_tcp_args = NULL;

  p_dsp_node = (struct s_dsp_node *)p_object;

  p_tcp_args = (struct s_tcp_func_args *)p_init_args;

  p_dsp_node->input_type = p_tcp_args->input_type;

  p_dsp_node->output_type = p_tcp_args->output_type;

  //if and only if null, allocate and start connection thread.
  if(!gp_socket_info)
  {
    // open socket discriptor for client/server
    gp_socket_info = malloc(sizeof(struct sockaddr_in));

    if(!gp_socket_info)
    {
      logger_error_msg(p_dsp_node->p_logger, "TCP SERVER, TCP Allocation Issue");

      return ~0;
    }

    gp_socket_info->sin_family = AF_INET;

    gp_socket_info->sin_port = htons(p_tcp_args->port);

    gp_socket_info->sin_addr.s_addr = inet_addr(p_tcp_args->p_address);

    p_dsp_node->p_data = gp_socket_info;

    //launch keep_alive_connection thread to connect and keep connection alive.
    return pthread_create(&connection_thread, NULL, connection_keep_alive, p_dsp_node);
  }
  else
  {
    p_dsp_node->p_data = gp_socket_info;
  }

  return 0;
}

// Clean up all allocations from init_callback
int free_callback_tcp(void *p_object)
{
  (void)p_object;

  if(gp_socket_info) pthread_join(connection_thread, NULL);

  free(gp_socket_info);

  gp_socket_info = NULL;

  return 0;
}

// THREAD SERVER FUNCTIONS //

// Pthread function for server threading send
void* pthread_function_tcp_server_send(void *p_data)
{
  int error = 0;

  uint8_t *p_buffer = NULL;

  struct s_dsp_node *p_dsp_node = NULL;

  p_dsp_node = (struct s_dsp_node *)p_data;

  if(!p_dsp_node)
  {
    fprintf(stderr, "ERROR: Data Struct is NULL.\n");

    kill_thread = 1;

    goto error_cleanup;
  }

  p_dsp_node->active = 1;

  p_buffer = malloc(p_dsp_node->chunk_size * p_dsp_node->input_type_size);

  if(!p_buffer)
  {
    logger_error_msg(p_dsp_node->p_logger, "TCP SERVER, Could not allocate buffer");

    kill_thread = 1;

    goto error_cleanup;
  }

  p_dsp_node->total_bytes_processed = 0;

  logger_info_msg(p_dsp_node->p_logger, "TCP SERVER SEND thread started.");

  do
  {
    long numElemRead   = 0;
    long numElemWrote  = 0;

    error = poll(&g_poll_connection, 1, 0);

    if(error <= 0) continue;

    if(g_poll_connection.revents & POLLHUP) continue;

    if(g_poll_connection.revents & POLLERR) continue;

    if((g_poll_connection.revents & POLLOUT) && (p_dsp_node->input_type != DATA_INVALID))
    {
      //read from input buffer, write to TCP
      numElemRead = (long)ringBufferBlockingRead(p_dsp_node->p_input_ring_buffer, p_buffer, p_dsp_node->chunk_size, NULL);

      do
      {
        numElemWrote += send(g_poll_connection.fd, p_buffer, (size_t)(numElemRead * p_dsp_node->input_type_size), MSG_DONTWAIT);
      } while(numElemWrote < (numElemRead * p_dsp_node->input_type_size));

      if(numElemWrote <= 0) continue;
    }

  } while (!kill_thread);

  free(p_buffer);

error_cleanup:
  ringBufferEndBlocking(p_dsp_node->p_input_ring_buffer);

  logger_info_msg(p_dsp_node->p_logger, "TCP SERVER SEND thread finished.");

  p_dsp_node->active = 0;

  return NULL;
}

// Pthread function for server threading recv
void* pthread_function_tcp_server_recv(void *p_data)
{
  int error = 0;

  uint8_t *p_buffer = NULL;

  struct s_dsp_node *p_dsp_node = NULL;

  p_dsp_node = (struct s_dsp_node *)p_data;

  if(!p_dsp_node)
  {
    fprintf(stderr, "ERROR: Data Struct is NULL.\n");

    kill_thread = 1;

    goto error_cleanup;
  }

  p_dsp_node->active = 1;

  p_buffer = malloc(p_dsp_node->chunk_size * p_dsp_node->output_type_size);

  if(!p_buffer)
  {
    logger_error_msg(p_dsp_node->p_logger, "TCP SERVER, Could not allocate buffer");

    kill_thread = 1;

    goto error_cleanup;
  }

  p_dsp_node->total_bytes_processed = 0;

  logger_info_msg(p_dsp_node->p_logger, "TCP SERVER RECV thread started.");

  do
  {
    long numElemRead   = 0;
    long numElemWrote  = 0;

    error = poll(&g_poll_connection, 1, 0);

    if(error <= 0) continue;

    if(g_poll_connection.revents & POLLHUP) continue;

    if(g_poll_connection.revents & POLLERR) continue;

    if((g_poll_connection.revents & POLLIN) && (p_dsp_node->output_type != DATA_INVALID))
    {
      // read from TCP, write to output buffer.
      numElemRead = recv(g_poll_connection.fd, p_buffer, p_dsp_node->chunk_size * p_dsp_node->output_type_size, MSG_DONTWAIT);

      p_dsp_node->total_bytes_processed += (unsigned long)(numElemRead * p_dsp_node->output_type_size);

      if(numElemRead <= 0) continue;

      numElemWrote = (long)ringBufferBlockingWrite(p_dsp_node->p_output_ring_buffer, p_buffer + (numElemWrote * p_dsp_node->output_type_size), (unsigned)(numElemRead), NULL);
    }

  } while (!kill_thread);

  free(p_buffer);

error_cleanup:
  ringBufferEndBlocking(p_dsp_node->p_output_ring_buffer);

  logger_info_msg(p_dsp_node->p_logger, "TCP SERVER RECV thread finished.");

  p_dsp_node->active = 0;

  return NULL;
}

void* connection_keep_alive(void *p_data)
{
  int error = 0;
  int prev_revents = 0;
  unsigned socket_len = 0;

  char p_buffer[16];

  struct pollfd poll_socket;

  struct sockaddr_in client_socket_info;

  struct s_dsp_node *p_dsp_node = NULL;

  p_dsp_node = (struct s_dsp_node *)p_data;

  if(!p_dsp_node)
  {
    fprintf(stderr, "ERROR: Data Struct is NULL.\n");

    return NULL;
  }

  poll_socket.fd = socket(gp_socket_info->sin_family, SOCK_STREAM || SOCK_NONBLOCK, 0);

  if(poll_socket.fd == -1)
  {
    logger_error_msg(p_dsp_node->p_logger, "TCP SERVER, Failed to create socket");

    kill_thread = 1;

    return NULL;
  }

  error = bind(poll_socket.fd, (struct sockaddr *)gp_socket_info, sizeof(struct sockaddr_in));

  if(error == -1)
  {
    logger_error_msg(p_dsp_node->p_logger, "TCP SERVER, Failed to bind");

    close(poll_socket.fd);

    kill_thread = 1;

    return NULL;
  }

  error = listen(poll_socket.fd, 1);

  if(error == -1)
  {
    logger_error_msg(p_dsp_node->p_logger, "TCP SERVER, Failed to listen");

    close(poll_socket.fd);

    kill_thread = 1;

    return NULL;
  }

  poll_socket.events = POLLIN;

  logger_info_msg(p_dsp_node->p_logger, "TCP SERVER STARTED");

  logger_info_msg(p_dsp_node->p_logger, "TCP SERVER WAITING FOR CLIENT");

  do
  {
    error = poll(&poll_socket, 1, 0);

    if(error < 0)
    {
      logger_error_msg(p_dsp_node->p_logger, "TCP SERVER, Poll failed");

      kill_thread = 1;

      continue;
    }

    if(error == 0) continue;

    if(poll_socket.revents == POLLIN)
    {
      g_poll_connection.fd = accept(poll_socket.fd, (struct sockaddr *)&client_socket_info, &socket_len);

      if(g_poll_connection.fd < 0)
      {
        logger_error_msg(p_dsp_node->p_logger, "TCP SERVER, Accept failed");

        kill_thread = 1;

        continue;
      }

      g_poll_connection.events = POLLIN | POLLOUT | POLLHUP;
    }

    logger_info_msg(p_dsp_node->p_logger, "TCP SERVER CONNECTED %s", inet_ntoa(client_socket_info.sin_addr));

    // while connected, just wait till dissconnect or kill_thread
    // while(!(g_poll_connection.revents & POLLHUP) && !(g_poll_connection.revents & POLLERR) && !kill_thread);
    prev_revents = g_send_tcp_server[*p_index].poll_connection.revents;

    // while connected, just wait till dissconnect or kill_thread
    for(;;)
    {
      int num_bytes_read = 0;

      error = poll(&g_send_tcp_server[*p_index].poll_connection, 1, 0);

      if(error <= 0) continue;

      if(g_poll_connection.revents & POLLHUP) break;
      if(g_poll_connection.revents & POLLERR) break;
      if(!kill_thread) break;

      if(prev_revents != g_poll_connection.revents)
      {
        prev_revents = g_poll_connection.poll_connection.revents;

        num_bytes_read = recv(g_poll_connection.fd, p_buffer, 16, MSG_PEEK);

        if(num_bytes_read <= 0) break;
      }
    };

    logger_info_msg(p_dsp_node->p_logger, "TCP SERVER DISCONNECTED");

    close(g_poll_connection.fd);

    if(!kill_thread) logger_info_msg(p_dsp_node->p_logger, "TCP SERVER WAITING FOR CLIENT");
  }
  while(!kill_thread);

  logger_info_msg(p_dsp_node->p_logger, "TCP SERVER SHUTTING DOWN");

  close(poll_socket.fd);

  return NULL;
}

