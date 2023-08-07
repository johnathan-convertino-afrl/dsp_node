//******************************************************************************
/// @file     file_to_alsa.c
/// @author   Jay Convertino(johnathan.convertino.1@us.af.mil)
/// @date     2023.07.12
/// @brief    file to alsa
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
#include "codec2/codec2_func.h"
#include "alsa/alsa_func.h"

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
  unsigned int channels = 1;
  unsigned int rate = 8000;
  
  // arrays
  char *p_device_name = NULL;
  
  char *p_read_file = NULL;
  
  // structs
  struct s_dsp_node *p_file_read_node;
  struct s_dsp_node *p_alsa_write_node;
  struct s_dsp_node *p_codec2_mod_node;

  struct s_alsa_func_args *p_alsa_func_write_args;
  struct s_file_func_args *p_file_func_read_args;
  struct s_codec2_func_args *p_codec2_mod_func_args;
  
  // get args
  while((opt = getopt(argc, argv, "d:i:h")) != -1)
  {
    switch(opt)
    {
      case 'd':
        p_device_name = strdup(optarg);
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

  if(!p_device_name || !p_read_file)
  {
    fprintf(stderr, "ERROR: input and output file name needed. %s %s.\n", p_device_name, p_read_file);

    help();

    free(p_device_name);

    free(p_read_file);

    return EXIT_FAILURE;
  }

  kill_throbber_create();

  p_alsa_func_write_args = create_alsa_args(p_device_name, SND_PCM_FORMAT_S16_LE, channels, rate);

  if(!p_alsa_func_write_args) goto cleanup_file_names;

  p_file_func_read_args = create_file_args(p_read_file, DATA_U8, DATA_U8, OVERWRITE_FILE);

  if(!p_file_func_read_args) goto cleanup_write_args;

  p_codec2_mod_func_args = create_codec2_args(DATA_S16);

  if(!p_codec2_mod_func_args) goto cleanup_read_args;

  p_file_read_node = dsp_create(BUFFSIZE, DATACHUNK);

  if(!p_file_read_node) goto cleanup_codec2_args;

  p_alsa_write_node = dsp_create(BUFFSIZE, DATACHUNK);

  if(!p_alsa_write_node) goto cleanup_read;

  p_codec2_mod_node = dsp_create(BUFFSIZE, DATACHUNK);

  if(!p_codec2_mod_node) goto cleanup_write;

  error = dsp_setup(p_file_read_node, init_callback_file_read, pthread_function_file_read, free_callback_file_read, p_file_func_read_args);

  if(error) goto cleanup_codec2;

  error = dsp_setup(p_codec2_mod_node, init_callback_codec2_mod, pthread_function_codec2_mod, free_callback_codec2_mod, p_codec2_mod_func_args);

  if(error) goto cleanup_codec2;
  
  error = dsp_setup(p_alsa_write_node, init_callback_alsa_write, pthread_function_alsa_write, free_callback_alsa_write, p_alsa_func_write_args);

  if(error) goto cleanup_codec2;

  error = dsp_setInput(p_codec2_mod_node, p_file_read_node);

  if(error) goto cleanup_codec2;

  error = dsp_setInput(p_alsa_write_node, p_codec2_mod_node);

  if(error) goto cleanup_codec2;

  error = dsp_start(p_file_read_node);

  if(error) goto cleanup_codec2;

  error = dsp_start(p_codec2_mod_node);

  if(error) goto cleanup_codec2;

  error = dsp_start(p_alsa_write_node);

  if(error) goto cleanup_codec2;

  kill_throbber_start();

  kill_throbber_wait();

  error = dsp_wait(p_file_read_node);

  if(error) goto cleanup_codec2;

  error = dsp_wait(p_codec2_mod_node);

  if(error) goto cleanup_codec2;

  error = dsp_wait(p_alsa_write_node);

cleanup_codec2:
  dsp_cleanup(p_codec2_mod_node);

cleanup_write:
  dsp_cleanup(p_alsa_write_node);

cleanup_read:
  dsp_cleanup(p_file_read_node);

cleanup_codec2_args:
  free_codec2_args(p_codec2_mod_func_args);

cleanup_read_args:
  free_file_args(p_file_func_read_args);

cleanup_write_args:
  free_alsa_args(p_alsa_func_write_args);

cleanup_file_names:
  free(p_read_file);

  free(p_device_name);

  return error;
}

// help
void help()
{
  printf("\n");
  
  printf("Example of file to alsa.\n");
  
  printf("-d:\tOutput device for alsa.\n");
  printf("-i:\tInput file for copy.\n");
  printf("-h:\tThis help information.\n");
  
  printf("\n");
}
