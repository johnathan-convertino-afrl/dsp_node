//******************************************************************************
/// @file     uhd_func.c
/// @author   Jay Convertino(johnathan.convertino.1@us.af.mil)
/// @date     2023.06.30
/// @brief    Create a signal UHD connection for tx and/or rx.
//******************************************************************************

// standard includes
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

// local includes
#include "uhd.h"
#include "uhd_func.h"
#include "kill_throbber.h"
#include "logger.h"

//uhd struct type for global data
struct s_uhd_data
{
  uhd_usrp_handle           usrp;
  uhd_stream_args_t         stream_args;
  uhd_string_vector_handle  device_vector;
};

//global UHD data variable, this will only allow one radio connection for this library....
//yes this is kinda dumb, but it was quick, dirty and did what I needed. Byte me.
static struct s_uhd_data *gp_uhd_data = NULL;

// PRIVATE FUNCTIONS //

//connect to the device if no connection has been made.
struct s_uhd_data *connect_to_uhd_device(struct s_dsp_node *p_dsp_node, struct s_uhd_func_args *p_func_args);

//disconnect if the device is still connected.
void disconnect_from_uhd_device();

//convert cpu string type to dsp_node enum type
enum e_binary_type convert_uhd_cpu_data_type(char *p_cpu_data);

// COMMON FUNCTIONS //

//Setup uhd arg struct for file rx/tx init callbacks
struct s_uhd_func_args *create_uhd_args(char *p_device_args, double freq, double rate, double gain, double bandwidth, char *p_cpu_data)
{
  struct s_uhd_func_args *p_temp = NULL;

  if(!p_device_args)
  {
    fprintf(stderr, "ERROR: Must specify device arguments.\n");

    return NULL;
  }

  if(!p_cpu_data)
  {
    fprintf(stderr, "ERROR: Must specify cpu data type.\n");

    return NULL;
  }

  p_temp = malloc(sizeof(struct s_uhd_func_args));

  if(!p_temp) return NULL;

  p_temp->p_device_args = strdup(p_device_args);

  p_temp->freq = freq;

  p_temp->rate = rate;

  p_temp->gain = gain;

  p_temp->bandwidth = bandwidth;

  p_temp->channel = 0;

  p_temp->p_cpu_data = strdup(p_cpu_data);

  return p_temp;
}

//Free args struct created from create uhd args
void free_uhd_args(struct s_uhd_func_args *p_init_args)
{
  if(!p_init_args)
  {
    fprintf(stderr, "ERROR: Null passed.\n");

    return;
  }

  free(p_init_args->p_device_args);

  free(p_init_args->p_cpu_data);

  free(p_init_args);
}

// THREAD RX FUNCTIONS //

//Setup uhd rx threads
int init_callback_uhd_rx(void *p_init_args, void *p_object)
{
  int error     = 0;

  char err_str[512]   = {"\0"};

  struct s_dsp_node *p_dsp_node = NULL;
  struct s_uhd_func_args *p_uhd_args = NULL;

  uhd_tune_result_t   tune_result;
  uhd_tune_request_t  tune_request;

  p_dsp_node = (struct s_dsp_node *)p_object;

  p_uhd_args = (struct s_uhd_func_args *)p_init_args;

  p_dsp_node->p_data = connect_to_uhd_device(p_dsp_node, p_uhd_args);

  if(!p_dsp_node->p_data)
  {
    fprintf(stderr, "ERROR: Global UHD creation failed.\n");

    return ~0;
  }

  p_dsp_node->input_type = DATA_INVALID;

  p_dsp_node->output_type = convert_uhd_cpu_data_type(p_uhd_args->p_cpu_data);

  // create other structs, could add options to make this customizable.
  tune_request.target_freq      = p_uhd_args->freq;
  tune_request.rf_freq_policy   = UHD_TUNE_REQUEST_POLICY_AUTO;
  tune_request.dsp_freq_policy  = UHD_TUNE_REQUEST_POLICY_AUTO;
  tune_request.args             = "";

  // setup rx rate
  error = uhd_usrp_set_rx_rate(((struct s_uhd_data *)p_dsp_node->p_data)->usrp, p_uhd_args->rate, p_uhd_args->channel);

  if(error)
  {
    logger_error_msg(p_dsp_node->p_logger, "UHD RX, Could not set rx rate.\n");

    goto ERR_CLOSE_USRP;
  }

  // print what the rate is set to
  uhd_usrp_get_rx_rate(((struct s_uhd_data *)p_dsp_node->p_data)->usrp, p_uhd_args->channel, &p_uhd_args->rate);

  logger_info_msg(p_dsp_node->p_logger, "UHD RX, rate set to %f\n", p_uhd_args->rate);

  // setup rx gain
  error = uhd_usrp_set_rx_gain(((struct s_uhd_data *)p_dsp_node->p_data)->usrp, p_uhd_args->gain, p_uhd_args->channel, "");

  if(error)
  {
    logger_error_msg(p_dsp_node->p_logger, "UHD RX, Could not set rx gain.\n");

    goto ERR_CLOSE_USRP;
  }

  // print what the gain is set to
  uhd_usrp_get_rx_gain(((struct s_uhd_data *)p_dsp_node->p_data)->usrp, p_uhd_args->channel, "", &p_uhd_args->gain);

  logger_info_msg(p_dsp_node->p_logger, "UHD RX, gain set to %f\n", p_uhd_args->gain);

  // setup rx frequency
  error = uhd_usrp_set_rx_freq(((struct s_uhd_data *)p_dsp_node->p_data)->usrp, &tune_request, p_uhd_args->channel, &tune_result);

  if(error)
  {
    logger_error_msg(p_dsp_node->p_logger, "UHD RX, Could not set rx frequency.\n");

    goto ERR_CLOSE_USRP;
  }

  // print what the rx frequency is set to
  uhd_usrp_get_rx_freq(((struct s_uhd_data *)p_dsp_node->p_data)->usrp, p_uhd_args->channel, &p_uhd_args->freq);

  logger_info_msg(p_dsp_node->p_logger, "UHD RX, frequnecy set to %f\n", p_uhd_args->freq);

  // set bandwidth

  //! Set the bandwidth for the given channel's RX frontend
  error = uhd_usrp_set_rx_bandwidth(((struct s_uhd_data *)p_dsp_node->p_data)->usrp, p_uhd_args->bandwidth, p_uhd_args->channel);

  if(error)
  {
    logger_error_msg(p_dsp_node->p_logger, "UHD RX, Could not set RX bandwidth.\n");

    goto ERR_CLOSE_USRP;
  }

  // print what the bandwidth is
  uhd_usrp_get_rx_bandwidth(((struct s_uhd_data *)p_dsp_node->p_data)->usrp, p_uhd_args->channel, &p_uhd_args->bandwidth);

  logger_info_msg(p_dsp_node->p_logger, "UHD RX, bandwidth set to %f\n", p_uhd_args->bandwidth);

ERR_CLOSE_USRP:
  if(error)
  {
    uhd_usrp_free(&((struct s_uhd_data *)p_dsp_node->p_data)->usrp);

    uhd_get_last_error(err_str, 512);

    logger_error_msg(p_dsp_node->p_logger, "UHD RX, %s\n", err_str);

    uhd_string_vector_free(&((struct s_uhd_data *)p_dsp_node->p_data)->device_vector);
  }

  return error;
}

//Pthread function for threading rx
void* pthread_function_uhd_rx(void *p_data)
{
  int     error = 0;
  size_t  samps_per_buff = 0;

  char    err_str[512] = {"\0"};

  uhd_stream_cmd_t        stream_cmd;
  uhd_rx_metadata_handle  md;
  uhd_rx_streamer_handle  rx_streamer;

  struct s_uhd_data *p_uhd_data = NULL;
  struct s_dsp_node *p_dsp_node = NULL;

  uint8_t *p_buffer = NULL;

  p_dsp_node = (struct s_dsp_node *)p_data;

  if(!p_dsp_node)
  {
    fprintf(stderr, "ERROR: Data Struct is NULL.\n");

    goto ERR_EXIT_THREAD;
  }

  p_dsp_node->active = 1;

  p_uhd_data = (struct s_uhd_data *)p_dsp_node->p_data;

  error = uhd_rx_streamer_make(&rx_streamer);

  if(error)
  {
    logger_error_msg(p_dsp_node->p_logger, "UHD RX, make rx streamer failed.");

    goto ERR_EXIT_THREAD;
  }

  error = uhd_usrp_get_rx_stream(p_uhd_data->usrp, &p_uhd_data->stream_args, rx_streamer);

  if(error)
  {
    logger_error_msg(p_dsp_node->p_logger, "UHD RX, could not setup stream.");

    goto ERR_EXIT_THREAD;
  }

  error = uhd_set_thread_priority(uhd_default_thread_priority, true);

  if(error)
  {
    uhd_get_last_error(err_str, 512);
    logger_warning_msg(p_dsp_node->p_logger, "UHD RX, Set priority failed, %s. Must be run as root. Running in non-root state.", err_str);
  }

  // get the max number of samples for buffer setup
  error = uhd_rx_streamer_max_num_samps(rx_streamer, &samps_per_buff);

  if(error)
  {
    logger_error_msg(p_dsp_node->p_logger, "UHD RX, could not get max number of samples.");

    goto ERR_KILL_STREAMER;
  }

  // populate sample info
//   printf("INFO: Samples per buffer is %ld\n", samps_per_buff);

  stream_cmd.stream_mode = UHD_STREAM_MODE_START_CONTINUOUS;
  // this is a bit confusing, we get the number of samples, but then have to tell it how many? Maybe I have this in the wrong order?
  stream_cmd.num_samps = samps_per_buff;
  stream_cmd.stream_now = true;

  // start streaming
  error = uhd_rx_streamer_issue_stream_cmd(rx_streamer, &stream_cmd);

  if(error)
  {
    logger_error_msg(p_dsp_node->p_logger, "UHD RX, could not issue stream command.");

    goto ERR_KILL_STREAMER;
  }

  // metadata, not using this at the moment
  error = uhd_rx_metadata_make(&md);

  if(error)
  {
    logger_error_msg(p_dsp_node->p_logger, "UHD RX, could not create metadata.");

    goto ERR_KILL_STREAMER;
  }

  // allocate a buffer for data
  p_buffer = malloc(samps_per_buff * p_dsp_node->output_type_size);

  if(!p_buffer)
  {
    logger_error_msg(p_dsp_node->p_logger, "UHD RX, allocation failed");

    goto ERR_EXIT_THREAD_MD;
  }

  p_dsp_node->total_bytes_processed = 0;

  logger_info_msg(p_dsp_node->p_logger, "UHD RX, thread started.");
  // read from USRP and write to ring buffer
  do
  {
    size_t numElemRead              = 0;
    unsigned long int numElemWrote  = 0;

    error = uhd_rx_streamer_recv(rx_streamer, (void**)&p_buffer, samps_per_buff, &md, 3.0, false, &numElemRead);

    if(error)
    {
      logger_error_msg(p_dsp_node->p_logger, "UHD RX, streamer issues.");

      break;
    }

//     if(samps_per_buff != numElemRead) printf("INFO: READ %ld of %ld\n", numElemRead, samps_per_buff);

    p_dsp_node->total_bytes_processed += numElemRead * p_dsp_node->output_type_size;

    do
    {
      numElemWrote += ringBufferBlockingWrite(p_dsp_node->p_output_ring_buffer, p_buffer + (numElemWrote * p_dsp_node->output_type_size), (unsigned long)numElemRead - numElemWrote, NULL);
    } while(numElemWrote < numElemRead);

  } while(!kill_thread);

  free(p_buffer);

ERR_EXIT_THREAD_MD:
  uhd_rx_metadata_free(&md);

ERR_KILL_STREAMER:
  uhd_rx_streamer_free(&rx_streamer);

ERR_EXIT_THREAD:
  ringBufferEndBlocking(p_dsp_node->p_output_ring_buffer);

  kill_thread = 1;

  logger_info_msg(p_dsp_node->p_logger, "UHD RX thread finished.");

  p_dsp_node->active = 0;

  return NULL;
}

// THREAD TX FUNCTIONS //

//Setup uhd tx threads
int init_callback_uhd_tx(void *p_init_args, void *p_object)
{
  int error     = 0;

  char err_str[512]   = {"\0"};

  struct s_dsp_node *p_dsp_node = NULL;
  struct s_uhd_func_args *p_uhd_args = NULL;

  uhd_tune_result_t   tune_result;
  uhd_tune_request_t  tune_request;

  p_dsp_node = (struct s_dsp_node *)p_object;

  p_uhd_args = (struct s_uhd_func_args *)p_init_args;

  p_dsp_node->p_data = connect_to_uhd_device(p_dsp_node, p_uhd_args);

  if(!p_dsp_node->p_data)
  {
    fprintf(stderr, "ERROR: Global UHD creation failed.\n");

    return ~0;
  }

  p_dsp_node->input_type = convert_uhd_cpu_data_type(p_uhd_args->p_cpu_data);

  p_dsp_node->output_type = DATA_INVALID;

  // create other structs, could add options to make this customizable.
  tune_request.target_freq      = p_uhd_args->freq;
  tune_request.rf_freq_policy   = UHD_TUNE_REQUEST_POLICY_AUTO;
  tune_request.dsp_freq_policy  = UHD_TUNE_REQUEST_POLICY_AUTO;
  tune_request.args             = "";

  // setup tx rate
  error = uhd_usrp_set_tx_rate(((struct s_uhd_data *)p_dsp_node->p_data)->usrp, p_uhd_args->rate, p_uhd_args->channel);

  if(error)
  {
    logger_error_msg(p_dsp_node->p_logger, "UHD TX, could not set tx rate.");

    goto ERR_CLOSE_USRP;
  }

  // print what the rate is set to
  uhd_usrp_get_tx_rate(((struct s_uhd_data *)p_dsp_node->p_data)->usrp, p_uhd_args->channel, &p_uhd_args->rate);

  logger_info_msg(p_dsp_node->p_logger, "UHD TX, rate set to %f", p_uhd_args->rate);

  // setup rx gain
  error = uhd_usrp_set_rx_gain(((struct s_uhd_data *)p_dsp_node->p_data)->usrp, p_uhd_args->gain, p_uhd_args->channel, "");

  if(error)
  {
    logger_error_msg(p_dsp_node->p_logger, "UHD TX, could not set rx gain.");

    goto ERR_CLOSE_USRP;
  }

  // print what the gain is set to
  uhd_usrp_get_tx_gain(((struct s_uhd_data *)p_dsp_node->p_data)->usrp, p_uhd_args->channel, "", &p_uhd_args->gain);

  logger_info_msg(p_dsp_node->p_logger, "UHD TX, gain set to %f", p_uhd_args->gain);

  // setup tx frequency
  error = uhd_usrp_set_tx_freq(((struct s_uhd_data *)p_dsp_node->p_data)->usrp, &tune_request, p_uhd_args->channel, &tune_result);

  if(error)
  {
    logger_error_msg(p_dsp_node->p_logger, "UHD TX, could not set tx frequency.");

    goto ERR_CLOSE_USRP;
  }

  // print what the tx frequency is set to
  uhd_usrp_get_tx_freq(((struct s_uhd_data *)p_dsp_node->p_data)->usrp, p_uhd_args->channel, &p_uhd_args->freq);

  logger_info_msg(p_dsp_node->p_logger, "UHD TX, frequnecy set to %f", p_uhd_args->freq);

  // set bandwidth

  //! Set the bandwidth for the given channel's TX frontend
  error = uhd_usrp_set_tx_bandwidth(((struct s_uhd_data *)p_dsp_node->p_data)->usrp, p_uhd_args->bandwidth, p_uhd_args->channel);

  if(error)
  {
    logger_error_msg(p_dsp_node->p_logger, "UHD TX, could not set TX bandwidth.");

    goto ERR_CLOSE_USRP;
  }

  // print what the bandwidth is
  uhd_usrp_get_tx_bandwidth(((struct s_uhd_data *)p_dsp_node->p_data)->usrp, p_uhd_args->channel, &p_uhd_args->bandwidth);

  logger_info_msg(p_dsp_node->p_logger, "UHD TX, bandwidth set to %f", p_uhd_args->bandwidth);

ERR_CLOSE_USRP:
  if(error)
  {
    uhd_usrp_free(&((struct s_uhd_data *)p_dsp_node->p_data)->usrp);

    uhd_get_last_error(err_str, 512);

    logger_error_msg(p_dsp_node->p_logger, "UHD TX, ERROR: %s", err_str);

    uhd_string_vector_free(&((struct s_uhd_data *)p_dsp_node->p_data)->device_vector);
  }

  return error;
}

//Pthread function for threading tx
void* pthread_function_uhd_tx(void *p_data)
{
  int     error = 0;
  size_t  samps_per_buff = 0;
  size_t  numElemRead = 0;

  char    err_str[512] = {"\0"};

  uhd_tx_metadata_handle  md;
  uhd_tx_streamer_handle  tx_streamer;

  struct s_uhd_data *p_uhd_data = NULL;
  struct s_dsp_node *p_dsp_node = NULL;

  uint8_t *p_buffer = NULL;

  p_dsp_node = (struct s_dsp_node *)p_data;

  if(!p_dsp_node)
  {
    fprintf(stderr, "ERROR: Data Struct is NULL.");

    goto ERR_EXIT_THREAD;
  }

  p_dsp_node->active = 1;

  p_uhd_data = (struct s_uhd_data *)p_dsp_node->p_data;

  error = uhd_tx_streamer_make(&tx_streamer);

  if(error)
  {
    logger_error_msg(p_dsp_node->p_logger, "UHD TX, USRP make tx streamer failed.");

    goto ERR_EXIT_THREAD;
  }

  error = uhd_usrp_get_tx_stream(p_uhd_data->usrp, &p_uhd_data->stream_args, tx_streamer);

  if(error)
  {
    logger_error_msg(p_dsp_node->p_logger, "UHD TX, TX could not setup stream.");

    goto ERR_EXIT_THREAD;
  }

  error = uhd_set_thread_priority(uhd_default_thread_priority, true);

  if(error)
  {
    uhd_get_last_error(err_str, 512);
    logger_warning_msg(p_dsp_node->p_logger, "UHD TX, Set priority failed, %s. Must be run as root. Running in non-root state.", err_str);
  }

  // get the max number of samples for buffer setup
  error = uhd_tx_streamer_max_num_samps(tx_streamer, &samps_per_buff);

  if(error)
  {
    logger_error_msg(p_dsp_node->p_logger, "UHD TX, Could not get max number of samples.");

    goto ERR_KILL_STREAMER;
  }

  // populate sample info
//   printf("INFO: Samples per buffer is %ld\n", samps_per_buff);


  // metadata, not using this at the moment
  error = uhd_tx_metadata_make(&md, false, 0, 0.1, true, false);

  if(error)
  {
    logger_error_msg(p_dsp_node->p_logger, "UHD TX, Could not create metadata.");

    goto ERR_KILL_STREAMER;
  }

  // allocate a buffer for data
  p_buffer = malloc(samps_per_buff * p_dsp_node->input_type_size);

  if(!p_buffer)
  {
    logger_error_msg(p_dsp_node->p_logger, "UHD TX, ERROR TX");

    goto ERR_EXIT_THREAD_MD;
  }

  p_dsp_node->total_bytes_processed = 0;

  // read from ringbuffer, write to usrp
  do
  {
    unsigned long int numElemWrote  = 0;

    // read data
    numElemRead = ringBufferBlockingRead(p_dsp_node->p_input_ring_buffer, p_buffer, samps_per_buff, NULL);

    p_dsp_node->total_bytes_processed += numElemRead * p_dsp_node->input_type_size;

    error = uhd_tx_streamer_send(tx_streamer, (const void **)&p_buffer, numElemRead, &md, 3.0, &numElemWrote);

    if(error) logger_error_msg(p_dsp_node->p_logger, "UHD TX, TX streamer issues.");

  } while((numElemRead > 0) && !kill_thread);

  // the buffers of the device need some time to empty, before closing the stream, or total loss occurs. Would be nice if there was a method to check! grrr
  sleep(5.0);

  free(p_buffer);

ERR_EXIT_THREAD_MD:
  uhd_tx_metadata_free(&md);

ERR_KILL_STREAMER:
  uhd_tx_streamer_free(&tx_streamer);

ERR_EXIT_THREAD:
  kill_thread = 1;

  ringBufferEndBlocking(p_dsp_node->p_input_ring_buffer);

  p_dsp_node->active = 0;

  return NULL;
}

//Clean up all allocations from init_callback
int free_callback_uhd(void *p_object)
{
  struct s_dsp_node *p_dsp_node = NULL;

  p_dsp_node = (struct s_dsp_node *)p_object;

  if(!p_dsp_node->p_data) return 0;

  disconnect_from_uhd_device();

  p_dsp_node->p_data = NULL;

  return 0;
}

//connect to the device if no connection has been made.
struct s_uhd_data *connect_to_uhd_device(struct s_dsp_node *p_dsp_node, struct s_uhd_func_args *p_func_args)
{
  int error = 0;
  size_t num    = 0;
  size_t index  = 0;

  char err_str[512]   = {"\0"};
  char temp_str[512]  = {"\0"};

  if(gp_uhd_data)
  {
    logger_info_msg(p_dsp_node->p_logger, "UHD, USRP Device descriptor previously created, reusing.");

    return gp_uhd_data;
  }

  gp_uhd_data = malloc(sizeof(struct s_uhd_data));

  if(!gp_uhd_data)
  {
    logger_error_msg(p_dsp_node->p_logger, "UHD, Could not allocate UHD global struct.");

    return NULL;
  }

  logger_info_msg(p_dsp_node->p_logger, "UHD, searching for USRP device with args %s", p_func_args->p_device_args);

  error = uhd_string_vector_make(&gp_uhd_data->device_vector);

  if(error)
  {
    logger_error_msg(p_dsp_node->p_logger, "UHD, Device vector make failed.");

    goto ERR_FREE_VECTOR;
  }

  error = uhd_usrp_find(p_func_args->p_device_args, &gp_uhd_data->device_vector);

  if(error)
  {
    logger_error_msg(p_dsp_node->p_logger, "UHD, Device find failed.");

    goto ERR_FREE_VECTOR;
  }

  uhd_string_vector_size(gp_uhd_data->device_vector, &num);

  logger_info_msg(p_dsp_node->p_logger, "UHD, Found %ld devices.", num);

  if(num <= 0)
  {
    error = ~0;

    logger_error_msg(p_dsp_node->p_logger, "UHD, No devices found, bad args %s.", p_func_args->p_device_args);

    goto ERR_FREE_VECTOR;
  }

  for(index = 0; index < num; index++)
  {
    uhd_string_vector_at(gp_uhd_data->device_vector, index, temp_str, 512);
    logger_info_msg(p_dsp_node->p_logger, "UHD, Device found at %ld is %s", index, temp_str);
  }

  // create usrp device handle
  error = uhd_usrp_make(&gp_uhd_data->usrp, p_func_args->p_device_args);

  if(error)
  {
    uhd_get_last_error(err_str, 512);

    logger_error_msg(p_dsp_node->p_logger, "UHD, ERROR: %s", err_str);
  }

ERR_FREE_VECTOR:
  if(error)
  {
    uhd_string_vector_free(&gp_uhd_data->device_vector);

    gp_uhd_data = NULL;

    return NULL;
  }

  gp_uhd_data->stream_args.cpu_format = p_func_args->p_cpu_data;
  gp_uhd_data->stream_args.otw_format = "sc16";
  gp_uhd_data->stream_args.args = "";
  gp_uhd_data->stream_args.channel_list = &p_func_args->channel;
  gp_uhd_data->stream_args.n_channels = 1;

  return gp_uhd_data;
}

//disconnect if the device is still connected.
void disconnect_from_uhd_device()
{
  if(!gp_uhd_data) return;

  sleep(2.0);

  uhd_usrp_free(&gp_uhd_data->usrp);

  uhd_string_vector_free(&gp_uhd_data->device_vector);

  free(gp_uhd_data);

  gp_uhd_data = NULL;
}

//convert cpu string type to dsp_node enum type
enum e_binary_type convert_uhd_cpu_data_type(char *p_cpu_data)
{
  if(!p_cpu_data) return DATA_INVALID;

  if(!strcmp("fc64", p_cpu_data)) return DATA_CDOUBLE;

  if(!strcmp("fc32", p_cpu_data)) return DATA_CFLOAT;

  if(!strcmp("sc16", p_cpu_data)) return DATA_CS16;

  if(!strcmp("sc8", p_cpu_data)) return DATA_CS8;

  return DATA_INVALID;
}
