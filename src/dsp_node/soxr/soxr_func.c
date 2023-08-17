//******************************************************************************
/// @file     soxr_func.c
/// @author   Jay Convertino(johnathan.convertino.1@us.af.mil)
/// @date     2023.06.29
/// @brief    Resampler for upsampling or downsampling data, complex or real.
//******************************************************************************

// standard includes
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "soxr.h"
#include "soxr_func.h"
#include "kill_throbber.h"
#include "logger.h"

//contains data for soxr input_data_callback
struct s_soxr_callback_data
{
  float *p_data_buffer;
  struct s_ringBuffer *p_ring_buffer;
};

//private data struct to hold all data needed in dsp_node struct member p_data.
struct s_soxr_data
{
  soxr_t soxr;
  struct s_soxr_func_args soxr_args;
};

//soxr callback to load data. This is used since we are processing on a streaming set of data.
size_t input_data_callback(void *p_callback_helper, soxr_cbuf_t *data, size_t len);

//convert the dsp_node type to soxr type.
soxr_datatype_t get_soxr_type(enum e_binary_type type);

//Setup soxr input/output args
struct s_soxr_func_args *create_soxr_args(double input_rate, double output_rate, enum e_binary_type input_type, enum e_binary_type output_type, unsigned channels)
{
  struct s_soxr_func_args *p_temp = NULL;

  p_temp = malloc(sizeof(struct s_soxr_func_args));

  if(!p_temp) return NULL;

  p_temp->input_rate = input_rate;

  p_temp->output_rate = output_rate;

  p_temp->channels = channels;

  p_temp->input_type = input_type;

  p_temp->output_type = output_type;

  return p_temp;
}

//Free args struct created from create file args
void free_soxr_args(struct s_soxr_func_args *p_init_args)
{
  free(p_init_args);
}

// THREAD RESAMPLER FUNCTIONS //

//init soxr
int init_callback_soxr(void *p_init_args, void *p_object)
{
  soxr_io_spec_t soxr_io;
  soxr_error_t soxr_error;

  struct s_dsp_node *p_dsp_node = NULL;

  struct s_soxr_func_args *p_soxr_func_args = NULL;

  p_dsp_node = (struct s_dsp_node *)p_object;

  p_soxr_func_args = (struct s_soxr_func_args *)p_init_args;

  if(!p_dsp_node || !p_soxr_func_args)
  {
    fprintf(stderr, "ERROR: Arguments null.\n");

    return ~0;
  }

  p_dsp_node->p_data = malloc(sizeof(struct s_soxr_data));

  if(!p_dsp_node->p_data)
  {
    logger_error_msg(p_dsp_node->p_logger, "SOXR Malloc failed for soxr.");

    return ~0;
  }

  soxr_io = soxr_io_spec(get_soxr_type(p_soxr_func_args->input_type), get_soxr_type(p_soxr_func_args->output_type));

  //change amplitude?
  soxr_io.scale = 1;

  ((struct s_soxr_data *)p_dsp_node->p_data)->soxr = soxr_create(p_soxr_func_args->input_rate, p_soxr_func_args->output_rate, p_soxr_func_args->channels, &soxr_error, &soxr_io, NULL, NULL);

  if(soxr_error)
  {
    logger_error_msg(p_dsp_node->p_logger, "SOXR %s\n", soxr_strerror(soxr_error));

    free(p_dsp_node->p_data);

    return ~0;
  }

  memcpy(&((struct s_soxr_data *)p_dsp_node->p_data)->soxr_args, p_soxr_func_args, sizeof(struct s_soxr_func_args));

  p_dsp_node->input_type = p_soxr_func_args->input_type;

  p_dsp_node->output_type = p_soxr_func_args->output_type;

  logger_info_msg(p_dsp_node->p_logger, "SOXR node created for %p.", p_dsp_node);

  return 0;
}

//Pthread function for threading resampler
void* pthread_function_soxr(void *p_data)
{
  soxr_error_t soxr_error;

  unsigned long int num_wrote = 0;
  unsigned long int scaled_chunk_size = 0;

  uint8_t *p_output_buffer = NULL;

  struct s_soxr_callback_data soxr_callback_data;

  struct s_dsp_node *p_dsp_node = NULL;

  p_dsp_node = (struct s_dsp_node *)p_data;

  if(!p_dsp_node)
  {
    fprintf(stderr, "ERROR: Data Struct is NULL.\n");

    goto error_cleanup;
  }

  soxr_error = soxr_set_input_fn(((struct s_soxr_data *)p_dsp_node->p_data)->soxr, (soxr_input_fn_t)input_data_callback, &soxr_callback_data, p_dsp_node->chunk_size);

  if(soxr_error)
  {
    logger_error_msg(p_dsp_node->p_logger, "SOXR, %s\n", soxr_strerror(soxr_error));

    goto error_cleanup;
  }

  soxr_callback_data.p_ring_buffer = p_dsp_node->p_input_ring_buffer;

  soxr_callback_data.p_data_buffer = malloc(p_dsp_node->chunk_size * p_dsp_node->input_type_size * ((struct s_soxr_data *)p_dsp_node->p_data)->soxr_args.channels);

  if(!soxr_callback_data.p_data_buffer)
  {
    logger_error_msg(p_dsp_node->p_logger, "SOXR, Malloc failed for buffer.\n");

    goto error_cleanup;
  }

  //scale the data buffer based up it output to input rate. This deals with integer divisable values only at the moment, which should be ok with fractional (will be 1024 vs 1024.5 for example).
  if(((struct s_soxr_data *)p_dsp_node->p_data)->soxr_args.output_rate > ((struct s_soxr_data *)p_dsp_node->p_data)->soxr_args.input_rate)
  {
    scaled_chunk_size = p_dsp_node->chunk_size * (long unsigned int)(((struct s_soxr_data *)p_dsp_node->p_data)->soxr_args.output_rate/((struct s_soxr_data *)p_dsp_node->p_data)->soxr_args.input_rate);
  }
  else
  {
    scaled_chunk_size = p_dsp_node->chunk_size / (long unsigned int)((((struct s_soxr_data *)p_dsp_node->p_data)->soxr_args.input_rate/((struct s_soxr_data *)p_dsp_node->p_data)->soxr_args.output_rate));
  }

  p_output_buffer = malloc(scaled_chunk_size * p_dsp_node->output_type_size * ((struct s_soxr_data *)p_dsp_node->p_data)->soxr_args.channels);

  if(!p_output_buffer)
  {
    logger_error_msg(p_dsp_node->p_logger, "SOXR, Malloc failed for buffer.\n");

    goto error_cleanup;
  }

  p_dsp_node->total_bytes_processed = 0;

  logger_info_msg(p_dsp_node->p_logger, "SOXR thread started.");

  do
  {
    size_t num_resampled = 0;

    num_resampled = soxr_output(((struct s_soxr_data *)p_dsp_node->p_data)->soxr, p_output_buffer, scaled_chunk_size);

    p_dsp_node->total_bytes_processed += num_resampled * p_dsp_node->output_type_size;

    num_wrote = ringBufferBlockingWrite(p_dsp_node->p_output_ring_buffer, p_output_buffer, num_resampled, NULL);

  } while((num_wrote > 0) && !kill_thread);

error_cleanup:
  free(p_output_buffer);

  free(soxr_callback_data.p_data_buffer);

  ringBufferEndBlocking(p_dsp_node->p_output_ring_buffer);
  ringBufferEndBlocking(p_dsp_node->p_input_ring_buffer);

  kill_thread = 1;

  logger_info_msg(p_dsp_node->p_logger, "SOXR thread finished.");

  return NULL;
}

//clean up all allocations from init_callback
int free_callback_soxr(void *p_object)
{
  struct s_dsp_node *p_dsp_node = NULL;

  p_dsp_node = (struct s_dsp_node *)p_object;

  soxr_delete(((struct s_soxr_data *)p_dsp_node->p_data)->soxr);

  free(p_dsp_node->p_data);

  return 0;
}

//soxr callback to load data. This is used since we are processing on a streaming set of data.
size_t input_data_callback(void *p_callback_helper, soxr_cbuf_t *data, size_t len)
{
  unsigned long int number_read = 0;

  struct s_soxr_callback_data *p_soxr_callback_data = NULL;

  p_soxr_callback_data = (struct s_soxr_callback_data *)p_callback_helper;

  *data = NULL;

  //invalid? set data to null and read to 0. this will end soxr_output process.
  if(!p_soxr_callback_data) return number_read;

  number_read = ringBufferBlockingRead(p_soxr_callback_data->p_ring_buffer, p_soxr_callback_data->p_data_buffer, len, NULL);

  //data is a double pointer, only way to return null.
  *data = (void *)p_soxr_callback_data->p_data_buffer;

  return number_read;
}

//get the soxr type from the dsp_node type
soxr_datatype_t get_soxr_type(enum e_binary_type type)
{
  switch(type)
  {
    case(DATA_S16):
      return SOXR_INT16_I;
    case(DATA_S32):
      return SOXR_INT32_I;
    case(DATA_FLOAT):
    case(DATA_CFLOAT):
      return SOXR_FLOAT32_I;
    case(DATA_DOUBLE):
    case(DATA_CDOUBLE):
      return SOXR_FLOAT64_I;
    default:
      fprintf(stderr, "ERROR: Invalid type for soxr.\n");

      return 0;
  }
}
