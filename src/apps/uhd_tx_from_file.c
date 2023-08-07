//******************************************************************************
/// @file     uhd_tx_from_file.c
/// @author   Jay Convertino(johnathan.convertino.1@us.af.mil)
/// @date     2023.07.12
/// @brief    Example uhd tx from file
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
#include "uhd/uhd_func.h"

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

  double freq         = 10e6;
  double rate         = 200e3;
  double gain         = 0.0;
  double bandwidth    = 10e3;
  char *p_device_args = NULL;
  
  // arrays
  char *p_read_file = NULL;
  
  // structs
  struct s_dsp_node *p_file_read_node;
  struct s_dsp_node *p_uhd_tx_node;

  struct s_uhd_func_args  *p_uhd_func_tx_args = NULL;
  struct s_file_func_args *p_file_func_read_args = NULL;
  
  // get args
  while((opt = getopt(argc, argv, "i:a:f:r:g:b:h")) != -1)
  {
    switch(opt)
    {
      case 'i':
        p_read_file = strdup(optarg);
        break;
      case 'a':
        p_device_args = strdup(optarg);
        break;
      case 'f':
        freq = atof(optarg);
        break;
      case 'r':
        rate = atof(optarg);
        break;
      case 'g':
        gain = atof(optarg);
        break;
      case 'b':
        bandwidth = atof(optarg);
        break;
      case 'h':
      default:
        help();
        return EXIT_SUCCESS;
    }
  }

  if(!p_read_file && !p_device_args)
  {
    fprintf(stderr, "ERROR: input file name, and device argument string needed.\n");

    help();

    return EXIT_FAILURE;
  }

  kill_throbber_create();

  p_uhd_func_tx_args = create_uhd_args(p_device_args, freq, rate, gain, bandwidth, "sc16");

  if(!p_uhd_func_tx_args) goto cleanup_file_names;

  p_file_func_read_args = create_file_args(p_read_file, DATA_INVALID, DATA_CS16, OVERWRITE_FILE);

  if(!p_file_func_read_args) goto cleanup_uhd_args;

  p_file_read_node = dsp_create(BUFFSIZE, DATACHUNK);

  if(!p_file_read_node) goto cleanup_read_args;

  p_uhd_tx_node = dsp_create(BUFFSIZE, DATACHUNK);

  if(!p_uhd_tx_node) goto cleanup_read;

  error = dsp_setup(p_file_read_node, init_callback_file_read, pthread_function_file_read, free_callback_file_read, p_file_func_read_args);

  if(error) goto cleanup_uhd;
  
  error = dsp_setup(p_uhd_tx_node, init_callback_uhd_tx, pthread_function_uhd_tx, free_callback_uhd, p_uhd_func_tx_args);

  if(error) goto cleanup_uhd;

  error = dsp_setInput(p_uhd_tx_node, p_file_read_node);

  if(error) goto cleanup_uhd;

  error = dsp_start(p_file_read_node);

  if(error) goto cleanup_uhd;

  error = dsp_start(p_uhd_tx_node);

  if(error) goto cleanup_uhd;

  kill_throbber_start();

  error = dsp_wait(p_file_read_node);

  error = dsp_wait(p_uhd_tx_node);

  kill_throbber_end();

  kill_throbber_wait();

cleanup_uhd:
  dsp_cleanup(p_uhd_tx_node);

cleanup_read:
  dsp_cleanup(p_file_read_node);

cleanup_read_args:
  free_file_args(p_file_func_read_args);

cleanup_uhd_args:
  free_uhd_args(p_uhd_func_tx_args);

cleanup_file_names:
  free(p_read_file);

  free(p_device_args);

  return error;
}

// help
void help()
{
  printf("\n");
  
  printf("Example UHD tx from file.\n");

  printf("-i:\tInput file for mod data.\n");
  printf("-a:\tUHD Args, Example: addr=192.168.10.2,device=usrp2,name=,serial=30C569E. REQUIRED.\n");
  printf("-f:\tFrequency in Hz.\n");
  printf("-r:\tRate in Hz.\n");
  printf("-g:\tGain in db.\n");
  printf("-b:\tBandwidth in Hz.\n");
  printf("-h:\tThis help information.\n");
  
  printf("\n");
}
