//******************************************************************************
/// @file     uhd_codec2_demod.c
/// @author   Jay Convertino(johnathan.convertino.1@us.af.mil)
/// @date     2023.06.29
/// @brief    Example of UHD codec2 demod to file
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
#include "uhd/uhd_func.h"
#include "ncurses_dsp_monitor/ncurses_dsp_monitor.h"

// ring buffer size defines
// 4MB
#define BUFFSIZE  (1 << 22)
// 1MB
#define DATACHUNK (1 << 20)
// 1KB
#define RESAMPCHUNK (1 << 10)
// 1KB
#define WRITECHUNK (1 << 10)

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
  char *p_write_file = NULL;
  
  // structs
  struct s_dsp_node *p_uhd_rx_node;
  struct s_dsp_node *p_codec2_demod_node;
  struct s_dsp_node *p_file_write_node;
  struct s_dsp_node *p_soxr_node;

  struct s_file_func_args *p_file_func_write_args = NULL;
  struct s_uhd_func_args  *p_uhd_func_rx_args = NULL;
  struct s_soxr_func_args *p_soxr_func_args = NULL;
  struct s_codec2_func_args *p_codec2_demod_func_args = NULL;
  
  struct s_ncurses_dsp_monitor *p_uhd_rx_monitor;
  struct s_ncurses_dsp_monitor *p_file_write_monitor;
  struct s_ncurses_dsp_monitor *p_soxr_monitor;
  struct s_ncurses_dsp_monitor *p_codec2_demod_monitor;

  // get args
  while((opt = getopt(argc, argv, "o:a:f:r:g:b:h")) != -1)
  {
    switch(opt)
    {
      case 'o':
        p_write_file = strdup(optarg);
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

  if(!p_write_file && !p_device_args)
  {
    fprintf(stderr, "ERROR: output file name, and device arguments needed.\n");

    free(p_device_args);

    free(p_write_file);

    help();

    return EXIT_FAILURE;
  }

  kill_throbber_create();

  p_file_func_write_args = create_file_args(p_write_file, DATA_U8, DATA_INVALID, OVERWRITE_FILE);

  if(!p_file_func_write_args) goto cleanup_file_names;

  p_uhd_func_rx_args = create_uhd_args(p_device_args, freq, rate, gain, bandwidth, "fc32");

  if(!p_uhd_func_rx_args) goto cleanup_file_args;

  p_codec2_demod_func_args = create_codec2_args(DATA_CFLOAT);

  if(!p_codec2_demod_func_args) goto cleanup_uhd_args;

  p_uhd_rx_node = dsp_create(BUFFSIZE, DATACHUNK);

  if(!p_uhd_rx_node) goto cleanup_codec2_args;

  p_codec2_demod_node = dsp_create(BUFFSIZE, DATACHUNK);

  if(!p_codec2_demod_node) goto cleanup_uhd;

  p_file_write_node = dsp_create(BUFFSIZE, WRITECHUNK);

  if(!p_file_write_node) goto cleanup_codec2;

  p_soxr_node = dsp_create(BUFFSIZE, RESAMPCHUNK);

  if(!p_soxr_node) goto cleanup_write;

  error = dsp_setup(p_uhd_rx_node, init_callback_uhd_rx, pthread_function_uhd_rx, free_callback_uhd, p_uhd_func_rx_args);

  if(error) goto cleanup_soxr;

  //called here so if a invalid rate for the UHD is set and it selects something different, it gets passed to soxr.
  p_soxr_func_args = create_soxr_args(p_uhd_func_rx_args->rate, 8000, DATA_CFLOAT, DATA_CFLOAT, 2);

  if(!p_soxr_func_args) goto cleanup_soxr_args;

  error = dsp_setup(p_soxr_node, init_callback_soxr, pthread_function_soxr, free_callback_soxr, p_soxr_func_args);

  if(error) goto cleanup_soxr_args;

  error = dsp_setup(p_codec2_demod_node, init_callback_codec2_demod, pthread_function_codec2_demod, free_callback_codec2_demod, p_codec2_demod_func_args);

  if(error) goto cleanup_soxr_args;
  
  error = dsp_setup(p_file_write_node, init_callback_file_write, pthread_function_file_write, free_callback_file_write, p_file_func_write_args);

  if(error) goto cleanup_soxr_args;

  error = dsp_setInput(p_soxr_node, p_uhd_rx_node);

  if(error) goto cleanup_soxr_args;

  error = dsp_setInput(p_codec2_demod_node, p_soxr_node);

  if(error) goto cleanup_soxr_args;

  error = dsp_setInput(p_file_write_node, p_codec2_demod_node);

  if(error) goto cleanup_soxr_args;

  p_uhd_rx_monitor = ncurses_dsp_monitor_create(p_uhd_rx_node, "UHD RECEIVE");

  if(!p_uhd_rx_monitor) goto cleanup_soxr_args;

  p_soxr_monitor = ncurses_dsp_monitor_create(p_soxr_node, "SOXR DOWNSAMPLE");

  if(!p_soxr_monitor) goto cleanup_uhd_mon;

  p_codec2_demod_monitor = ncurses_dsp_monitor_create(p_codec2_demod_node, "CODEC2 DATAC1 DEMODULATION");

  if(!p_codec2_demod_monitor) goto cleanup_soxr_mon;

  p_file_write_monitor = ncurses_dsp_monitor_create(p_file_write_node, "FILE WRITE");

  if(!p_file_write_monitor) goto cleanup_codec2_mon;

  error = dsp_start(p_uhd_rx_node);

  if(error) goto cleanup_write_mon;

  error = dsp_start(p_soxr_node);

  if(error) goto cleanup_write_mon;

  error = dsp_start(p_codec2_demod_node);

  if(error) goto cleanup_write_mon;

  error = dsp_start(p_file_write_node);

  if(error) goto cleanup_write_mon;

  error = ncurses_dsp_monitor_start();

  if(error) goto cleanup_write_mon;

  error = ncurses_dsp_monitor_throughput_start(p_uhd_rx_monitor);

  if(error) goto cleanup_write_mon;

  error = ncurses_dsp_monitor_throughput_start(p_soxr_monitor);

  if(error) goto cleanup_write_mon;

  error = ncurses_dsp_monitor_throughput_start(p_codec2_demod_monitor);

  if(error) goto cleanup_write_mon;

  error = ncurses_dsp_monitor_throughput_start(p_file_write_monitor);

  if(error) goto cleanup_write_mon;

  error = ncurses_dsp_monitor_wait(p_uhd_rx_monitor);

  error = ncurses_dsp_monitor_wait(p_soxr_monitor);

  error = ncurses_dsp_monitor_wait(p_codec2_demod_monitor);

  error = ncurses_dsp_monitor_wait(p_file_write_monitor);

  error = dsp_wait(p_uhd_rx_node);

  error = dsp_wait(p_soxr_node);

  error = dsp_wait(p_codec2_demod_node);

  error = dsp_wait(p_file_write_node);

  kill_throbber_end();

cleanup_write_mon:
  ncurses_dsp_monitor_cleanup(p_file_write_monitor);

cleanup_codec2_mon:
  ncurses_dsp_monitor_cleanup(p_codec2_demod_monitor);

cleanup_soxr_mon:
  ncurses_dsp_monitor_cleanup(p_soxr_monitor);

cleanup_uhd_mon:
  ncurses_dsp_monitor_cleanup(p_uhd_rx_monitor);

cleanup_soxr_args:
  free_soxr_args(p_soxr_func_args);

cleanup_soxr:
  dsp_cleanup(p_soxr_node);

cleanup_write:
  dsp_cleanup(p_file_write_node);

cleanup_codec2:
  dsp_cleanup(p_codec2_demod_node);

cleanup_uhd:
  dsp_cleanup(p_uhd_rx_node);

cleanup_codec2_args:
  free_codec2_args(p_codec2_demod_func_args);

cleanup_uhd_args:
  free_uhd_args(p_uhd_func_rx_args);

cleanup_file_args:
  free_file_args(p_file_func_write_args);

cleanup_file_names:
  free(p_write_file);

  free(p_device_args);

  return error;
}

// help
void help()
{
  printf("\n");
  
  printf("Example of codec2 DATAC1 demod.\n");

  printf("-o:\tOutput file demod data. REQUIRED.\n");
  printf("-a:\tUHD Args, Example: addr=192.168.10.2,device=usrp2,name=,serial=30C569E. REQUIRED.\n");
  printf("-f:\tFrequency in Hz.\n");
  printf("-r:\tRate in Hz.\n");
  printf("-g:\tGain in db.\n");
  printf("-b:\tBandwidth in Hz.\n");
  printf("-h:\tThis help information.\n");
  
  printf("\n");
}
