//******************************************************************************
/// @file     file_to_tcp_to_file.c
/// @author   Jay Convertino(johnathan.convertino.1@us.af.mil)
/// @date     2023.08.16
/// @brief    file to tcp to file data transfer
//******************************************************************************

// standard includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>

// local includes
#include "dsp_node.h"
#include "kill_throbber.h"
#include "file/file_func.h"
#include "tcp_server/tcp_server_func.h"

// ring buffer size defines
// 4MB
#define BUFFSIZE  (1 << 22)
// 1MB
#define DATACHUNK (1 << 20)

void help();

// main :)
int main(int argc, char *argv[])
{
  // varibles
  int error = 0;
  int opt   = 0;
  
  // arrays
  char *p_write_file = NULL;
  
  char *p_read_file = NULL;
  
  // structs
  struct s_dsp_node *p_file_read_node;
  struct s_dsp_node *p_file_write_node;
  struct s_dsp_node *p_tcp_server_send_node;
  struct s_dsp_node *p_tcp_server_recv_node;
  struct s_file_func_args *p_file_func_write_args;
  struct s_file_func_args *p_file_func_read_args;
  struct s_tcp_func_args *p_tcp_func_args;
  
  // get args
  while((opt = getopt(argc, argv, "o:i:h")) != -1)
  {
    switch(opt)
    {
      case 'o':
        p_write_file = strdup(optarg);
        break;
      case 'i':
        p_read_file = strdup(optarg);
        break;
      case 'h':
      default:
        help();
        return EXIT_SUCCESS;
    }
  }

  if(!p_write_file || !p_read_file)
  {
    fprintf(stderr, "ERROR: input and output file name needed. %s %s.\n", p_write_file, p_read_file);

    help();

    free(p_write_file);

    free(p_read_file);

    return EXIT_FAILURE;
  }

  kill_throbber_create();

  p_file_func_write_args = create_file_args(p_write_file, DATA_U8, DATA_U8, OVERWRITE_FILE);

  if(!p_file_func_write_args) goto cleanup_file_names;

  p_file_func_read_args = create_file_args(p_read_file, DATA_U8, DATA_U8, OVERWRITE_FILE);

  if(!p_file_func_read_args) goto cleanup_write_args;

  p_tcp_func_args = create_tcp_args("127.0.0.1", 2000, DATA_U8, DATA_U8);

  if(!p_tcp_func_args) goto cleanup_read_args;

  p_file_read_node = dsp_create(BUFFSIZE, DATACHUNK);

  if(!p_file_read_node) goto cleanup_tcp_args;

  p_file_write_node = dsp_create(BUFFSIZE, DATACHUNK);

  if(!p_file_write_node) goto cleanup_read;

  p_tcp_server_send_node = dsp_create(BUFFSIZE, DATACHUNK);

  if(!p_tcp_server_send_node) goto cleanup_write;

  p_tcp_server_recv_node = dsp_create(BUFFSIZE, DATACHUNK);

  if(!p_tcp_server_recv_node) goto cleanup_tcp_send;

  error = dsp_setup(p_file_read_node, init_callback_file_read, pthread_function_file_read, free_callback_file_read, p_file_func_read_args);

  if(error) goto cleanup_tcp_recv;
  
  error = dsp_setup(p_file_write_node, init_callback_file_write, pthread_function_file_write, free_callback_file_write, p_file_func_write_args);

  if(error) goto cleanup_tcp_recv;

  error = dsp_setup(p_tcp_server_send_node, init_callback_tcp, pthread_function_tcp_server_send, free_callback_tcp, p_tcp_func_args);

  if(error) goto cleanup_tcp_recv;

  error = dsp_setup(p_tcp_server_recv_node, init_callback_tcp, pthread_function_tcp_server_recv, free_callback_tcp, p_tcp_func_args);

  if(error) goto cleanup_tcp_recv;

  error = dsp_setInput(p_file_write_node, p_tcp_server_recv_node);

  if(error) goto cleanup_tcp_recv;

  error = dsp_setInput(p_tcp_server_send_node, p_file_read_node);

  if(error) goto cleanup_tcp_recv;

  error = dsp_start(p_file_read_node);

  if(error) goto cleanup_tcp_recv;

  error = dsp_start(p_file_write_node);

  if(error) goto cleanup_tcp_recv;

  error = dsp_start(p_tcp_server_send_node);

  if(error) goto cleanup_tcp_recv;

  error = dsp_start(p_tcp_server_recv_node);

  if(error) goto cleanup_tcp_recv;

  kill_throbber_start();

  kill_throbber_wait();

  error = dsp_wait(p_file_read_node);

  error = dsp_wait(p_file_write_node);

  error = dsp_wait(p_tcp_server_send_node);

  error = dsp_wait(p_tcp_server_recv_node);

cleanup_tcp_recv:
  dsp_cleanup(p_tcp_server_recv_node);

cleanup_tcp_send:
  dsp_cleanup(p_tcp_server_send_node);

cleanup_write:
  dsp_cleanup(p_file_write_node);

cleanup_read:
  dsp_cleanup(p_file_read_node);

cleanup_tcp_args:
  free_tcp_args(p_tcp_func_args);

cleanup_read_args:
  free_file_args(p_file_func_read_args);

cleanup_write_args:
  free_file_args(p_file_func_write_args);

cleanup_file_names:
  free(p_write_file);

  free(p_read_file);

  return EXIT_SUCCESS;
}

// help
void help()
{
  printf("\n");
  
  printf("Example of file to tcp to file data transfer, local server only.\n");
  
  printf("-o:\tOutput file for copy.\n");
  printf("-i:\tInput file for copy.\n");
  printf("-h:\tThis help information.\n");
  
  printf("\n");
}
