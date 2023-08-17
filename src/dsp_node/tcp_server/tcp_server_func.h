//******************************************************************************
/// @file     tcp_server_func.h
/// @author   Jay Convertino(johnathan.convertino.1@us.af.mil)
/// @date     2023.08.07
/// @brief    TCP server node for single connection only (PTP).
//******************************************************************************

#ifndef __tcp_server_func
#define __tcp_server_func

#include "dsp_node_types.h"

#ifdef __cplusplus
extern "C" {

#endif

/**
 * @struct s_tcp_func_args
 * @brief Contains argument data for file node creation (pass to p_init_args for init_callback).
 */
struct s_tcp_func_args
{
  /**
   * @var s_tcp_func_args::p_address
   * String version of IPV4 address (127.0.0.1).
   */
  char *p_address;
  /**
   * @var s_tcp_func_args::port
   * Port for the TCP node to connect/listen on.
   */
  unsigned short port;
  /**
   * @var s_tcp_func_args::input_type
   * input data format
   */
  enum e_binary_type input_type;
  /**
   * @var s_tcp_func_args::output_type
   * output data format
   */
  enum e_binary_type output_type;
};

// COMMON FUNCTIONS //

/**************************************************************************//**
  * @brief Setup tcp arg struct for tcp server/client init callbacks
  *
  * @param p_address IPV4 address to connect to (client), or allow from (server).
  * @param port TCP port to user for connection
  * @param input_type Data comming in to TCP node. DATA_INVALID will ignore all
  * incoming data.
  * @param output_type Data going out of TCP node. DATA_INVALID will ignore all
  * outgoing data.
  *
  * @return Arg struct
  ****************************************************************************/
struct s_tcp_func_args *create_tcp_args(char *p_address, unsigned short port, enum e_binary_type input_type, enum e_binary_type output_type);

/**************************************************************************//**
  * @brief Free args struct created from create tcp args
  *
  * @param p_init_args Filr args struct to free
  ****************************************************************************/
void free_tcp_args(struct s_tcp_func_args *p_init_args);

// THREAD COMMON FUNCTIONS //

/**************************************************************************//**
  * @brief Setup TCP sockets, create connection keep-alive-thread
  *
  * @param p_init_args Takes s_tcp_func_args for setup
  * @param p_object A dsp_node struct used to change various settings.
  *
  * @return 0 no error, non-zero indicates error.
  ****************************************************************************/
int init_callback_tcp(void *p_init_args, void *p_object);

/**************************************************************************//**
  * @brief Clean up all allocations from init_callback
  *
  * @param p_object tcp dsp node object to free.
  *
  * @return 0 no error, non-zero indicates error.
  ****************************************************************************/
int free_callback_tcp(void *p_object);

// THREAD SERVER FUNCTIONS //

/**************************************************************************//**
  * @brief Pthread function for server threading send
  *
  * @param p_data This will contain dsp_node struct so all data is available.
  *
  * @return NULL
  ****************************************************************************/
void* pthread_function_tcp_server_send(void *p_data);

/**************************************************************************//**
  * @brief Pthread function for server threading recv
  *
  * @param p_data This will contain dsp_node struct so all data is available.
  *
  * @return NULL
  ****************************************************************************/
void* pthread_function_tcp_server_recv(void *p_data);

#ifdef __cplusplus
}
#endif

#endif
