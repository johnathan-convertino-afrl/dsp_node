//******************************************************************************
/// @file     resample_codec2_mod_file.c
/// @author   Jay Convertino(johnathan.convertino.1@us.af.mil)
/// @date     2023.06.29
/// @brief    Example of codec2 datac1 of file demod
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
#include "soxr/soxr_func.h"

// ring buffer size defines
// 4MB
#define BUFFSIZE  (1 << 22)
// 1MB
#define DATACHUNK (1 << 20)
// 1KB
#define RESAMPCHUNK (1 << 10)

void help();

// main :)
int main(int argc, char *argv[])
{
  // varibles
  int error = 0;
  int opt   = 0;
  double rate = 200e3;
  
  // arrays
  char *p_write_file = NULL;
  
  char *p_read_file = NULL;
  
  // structs
  struct s_dsp_node *p_file_read_node;
  struct s_dsp_node *p_codec2_mod_node;
  struct s_dsp_node *p_file_write_node;
  struct s_dsp_node *p_soxr_node;

  struct s_file_func_args *p_file_func_write_args = NULL;
  struct s_file_func_args *p_file_func_read_args = NULL;
  struct s_soxr_func_args *p_soxr_func_args = NULL;
  struct s_codec2_func_args *p_codec2_mod_func_args = NULL;
  
  // get args
  while((opt = getopt(argc, argv, "o:i:r:h")) != -1)
  {
    switch(opt)
    {
      case 'o':
        p_write_file = strdup(optarg);
        break;
      case 'i':
        p_read_file = strdup(optarg);
        break;
      case 'r':
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

  kill_throbber_create();

  p_soxr_func_args = create_soxr_args(8000, rate, DATA_CFLOAT, DATA_CFLOAT, 2);

  if(!p_soxr_func_args) goto cleanup_file_names;

  p_file_func_write_args = create_file_args(p_write_file, DATA_CFLOAT, DATA_INVALID, OVERWRITE_FILE);

  if(!p_file_func_write_args) goto cleanup_soxr_args;

  p_file_func_read_args = create_file_args(p_read_file, DATA_INVALID, DATA_U8, OVERWRITE_FILE);

  if(!p_file_func_read_args) goto cleanup_write_args;

  p_codec2_mod_func_args = create_codec2_args(DATA_CFLOAT);

  if(!p_codec2_mod_func_args) goto cleanup_read_args;

  p_file_read_node = dsp_create(BUFFSIZE, DATACHUNK);

  if(!p_file_read_node) goto cleanup_codec2_args;

  p_codec2_mod_node = dsp_create(BUFFSIZE, DATACHUNK);

  if(!p_codec2_mod_node) goto cleanup_read;

  p_file_write_node = dsp_create(BUFFSIZE, DATACHUNK);

  if(!p_file_write_node) goto cleanup_codec2;

  p_soxr_node = dsp_create(BUFFSIZE, RESAMPCHUNK);

  if(!p_soxr_node) goto cleanup_write;

  error = dsp_setup(p_file_read_node, init_callback_file_read, pthread_function_file_read, free_callback_file_read, p_file_func_read_args);

  if(error) goto cleanup_soxr;

  error = dsp_setup(p_soxr_node, init_callback_soxr, pthread_function_soxr, free_callback_soxr, p_soxr_func_args);

  if(error) goto cleanup_soxr;

  error = dsp_setup(p_codec2_mod_node, init_callback_codec2_mod, pthread_function_codec2_mod, free_callback_codec2_mod, p_codec2_mod_func_args);

  if(error) goto cleanup_soxr;
  
  error = dsp_setup(p_file_write_node, init_callback_file_write, pthread_function_file_write, free_callback_file_write, p_file_func_write_args);

  if(error) goto cleanup_soxr;

  error = dsp_setInput(p_codec2_mod_node, p_file_read_node);

  if(error) goto cleanup_soxr;

  error = dsp_setInput(p_soxr_node, p_codec2_mod_node);

  if(error) goto cleanup_soxr;

  error = dsp_setInput(p_file_write_node, p_soxr_node);

  if(error) goto cleanup_soxr;

  error = dsp_start(p_file_read_node);

  if(error) goto cleanup_soxr;

  error = dsp_start(p_soxr_node);

  if(error) goto cleanup_soxr;

  error = dsp_start(p_codec2_mod_node);

  if(error) goto cleanup_soxr;

  error = dsp_start(p_file_write_node);

  if(error) goto cleanup_soxr;

  kill_throbber_start();

  error = dsp_wait(p_file_read_node);

  error = dsp_wait(p_codec2_mod_node);

  error = dsp_wait(p_soxr_node);

  error = dsp_wait(p_file_write_node);

  kill_throbber_end();

  kill_throbber_wait();

cleanup_soxr:
  dsp_cleanup(p_soxr_node);

cleanup_write:
  dsp_cleanup(p_file_write_node);

cleanup_codec2:
  dsp_cleanup(p_codec2_mod_node);

cleanup_read:
  dsp_cleanup(p_file_read_node);

cleanup_codec2_args:
  free_codec2_args(p_codec2_mod_func_args);

cleanup_read_args:
  free_file_args(p_file_func_read_args);

cleanup_write_args:
  free_file_args(p_file_func_write_args);

cleanup_soxr_args:
  free_soxr_args(p_soxr_func_args);

cleanup_file_names:
  free(p_write_file);

  free(p_read_file);

  return error;
}

// help
void help()
{
  printf("\n");
  
  printf("Example of codec2 DATAC1 demod.\n");
  
  printf("-o:\tOutput file demod data.\n");
  printf("-i:\tInput file for mod data.\n");
  printf("-r:\toutput rate for the file in hz (INPUT RATE IS 8000)\n");
  printf("-h:\tThis help information.\n");
  
  printf("\n");
}
