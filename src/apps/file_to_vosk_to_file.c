//******************************************************************************
/// @file     file_to_vosk_to_file.c
/// @author   Jay Convertino(johnathan.convertino.1@us.af.mil)
/// @date     2024.07.15
/// @brief    file to vosk to file process
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
#include "vosk/vosk_func.h"

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

  float rate  = 0;
  
  // arrays
  char *p_write_file  = NULL;
  char *p_read_file   = NULL;
  
  // structs
  struct s_dsp_node *p_file_read_node;
  struct s_dsp_node *p_vosk_node;
  struct s_dsp_node *p_file_write_node;
  struct s_file_func_args *p_file_func_write_args;
  struct s_vosk_func_args *p_vosk_func_args;
  struct s_file_func_args *p_file_func_read_args;
  
  // get args
  while((opt = getopt(argc, argv, "o:i:h:s")) != -1)
  {
    switch(opt)
    {
      case 'o':
        p_write_file = strdup(optarg);
        break;
      case 'i':
        p_read_file = strdup(optarg);
        break;
      case 's':
        rate = atof(optarg);
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

  if(rate <= 0)
  {
    fprintf(stderr, "ERROR: Invalid rate set %f.\n", rate);

    help();

    free(p_write_file);

    free(p_read_file);

    return EXIT_FAILURE;
  }

  kill_throbber_create();

  p_file_func_write_args = create_file_args(p_write_file, DATA_U8, DATA_U8, OVERWRITE_FILE);

  if(!p_file_func_write_args) goto cleanup_file_names;

  p_vosk_func_args = create_vosk_args(rate, DATA_U8);

  if(!p_vosk_func_args) goto cleanup_write_args;

  p_file_func_read_args = create_file_args(p_read_file, DATA_U8, DATA_U8, OVERWRITE_FILE);

  if(!p_file_func_read_args) goto cleanup_vosk_args;

  p_file_read_node = dsp_create(BUFFSIZE, DATACHUNK);

  if(!p_file_read_node) goto cleanup_read_args;

  p_file_write_node = dsp_create(BUFFSIZE, DATACHUNK);

  if(!p_file_write_node) goto cleanup_read;

  p_vosk_node = dsp_create(BUFFSIZE, DATACHUNK);

  if(!p_vosk_node) goto cleanup_write;

  error = dsp_setup(p_file_read_node, init_callback_file_read, pthread_function_file_read, free_callback_file_read, p_file_func_read_args);

  if(error) goto cleanup_vosk;

  error = dsp_setup(p_vosk_node, init_callback_vosk, pthread_function_vosk, free_callback_vosk, p_vosk_func_args);

  if(error) goto cleanup_vosk;
  
  error = dsp_setup(p_file_write_node, init_callback_file_write, pthread_function_file_write, free_callback_file_write, p_file_func_write_args);

  if(error) goto cleanup_vosk;

  error = dsp_setInput(p_vosk_node, p_file_read_node);

  if(error) goto cleanup_vosk;

  error = dsp_setInput(p_file_write_node, p_vosk_node);

  if(error) goto cleanup_vosk;

  error = dsp_start(p_file_read_node);

  if(error) goto cleanup_vosk;

  error = dsp_start(p_vosk_node);

  if(error) goto cleanup_vosk;

  error = dsp_start(p_file_write_node);

  if(error) goto cleanup_vosk;

  kill_throbber_start();

  kill_throbber_wait();

  error = dsp_wait(p_file_read_node);

  if(error) goto cleanup_vosk;

  error = dsp_wait(p_vosk_node);

  if(error) goto cleanup_vosk;

  error = dsp_wait(p_file_write_node);

cleanup_vosk:
  dsp_cleanup(p_vosk_node);

cleanup_write:
  dsp_cleanup(p_file_write_node);

cleanup_read:
  dsp_cleanup(p_file_read_node);

cleanup_read_args:
  free_file_args(p_file_func_read_args);

cleanup_vosk_args:
  free_vosk_args(p_vosk_func_args);

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
  
  printf("Example of file to vosk to file, input is unsigned character (byte mono).\n");
  
  printf("-o:\tOutput file for copy.\n");
  printf("-i:\tInput file for copy.\n");
  printf("-s:\tAudio data sample rate\n");
  printf("-h:\tThis help information.\n");
  
  printf("\n");
}
