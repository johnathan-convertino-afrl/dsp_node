//******************************************************************************
/// @file     file_to_file.c
/// @author   Jay Convertino(johnathan.convertino.1@us.af.mil)
/// @date     2023.06.26
/// @brief    file to file copy
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
#include "ncurses_dsp_monitor/ncurses_dsp_monitor.h"

// ring buffer size defines
// 4MB
#define BUFFSIZE  (1 << 24)
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

  struct s_file_func_args *p_file_func_write_args;
  struct s_file_func_args *p_file_func_read_args;

  struct s_ncurses_dsp_monitor *p_file_write_monitor;
  struct s_ncurses_dsp_monitor *p_file_read_monitor;
  
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

  p_file_read_node = dsp_create(BUFFSIZE, DATACHUNK);

  if(!p_file_read_node) goto cleanup_read_args;

  p_file_write_node = dsp_create(BUFFSIZE, DATACHUNK);

  if(!p_file_write_node) goto cleanup_read;

  p_file_write_monitor = ncurses_dsp_monitor_create(p_file_write_node, "FILE WRITE");

  if(!p_file_write_monitor) goto cleanup_write;

  p_file_read_monitor = ncurses_dsp_monitor_create(p_file_read_node, "FILE READ");

  if(!p_file_read_monitor) goto cleanup_write_mon;

  error = dsp_setup(p_file_read_node, init_callback_file_read, pthread_function_file_read, free_callback_file_read, p_file_func_read_args);

  if(error) goto cleanup_read_mon;
  
  error = dsp_setup(p_file_write_node, init_callback_file_write, pthread_function_file_write, free_callback_file_write, p_file_func_write_args);

  if(error) goto cleanup_read_mon;

  error = dsp_setInput(p_file_write_node, p_file_read_node);

  if(error) goto cleanup_read_mon;

  error = dsp_start(p_file_read_node);

  if(error) goto cleanup_read_mon;

  error = dsp_start(p_file_write_node);

  if(error) goto cleanup_read_mon;

  error = ncurses_dsp_monitor_start();

  if(error) goto cleanup_read_mon;

  error = ncurses_dsp_monitor_throughput_start(p_file_write_monitor);

  if(error) goto cleanup_read_mon;

  error = ncurses_dsp_monitor_throughput_start(p_file_read_monitor);

  if(error) goto cleanup_read_mon;

  error = ncurses_dsp_monitor_wait(p_file_read_monitor);

  if(error) goto cleanup_read_mon;

  error = ncurses_dsp_monitor_wait(p_file_write_monitor);

  if(error) goto cleanup_read_mon;

  error = dsp_wait(p_file_read_node);

  if(error) goto cleanup_read_mon;

  error = dsp_wait(p_file_write_node);

cleanup_read_mon:
  ncurses_dsp_monitor_cleanup(p_file_read_monitor);

cleanup_write_mon:
  ncurses_dsp_monitor_cleanup(p_file_write_monitor);

cleanup_write:
  dsp_cleanup(p_file_write_node);

cleanup_read:
  dsp_cleanup(p_file_read_node);

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
  
  printf("Example of file to file copy.\n");
  
  printf("-o:\tOutput file for copy.\n");
  printf("-i:\tInput file for copy.\n");
  printf("-h:\tThis help information.\n");
  
  printf("\n");
}
