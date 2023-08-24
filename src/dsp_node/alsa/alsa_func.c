//******************************************************************************
/// @file     alsa_func.c
/// @author   Jay Convertino(johnathan.convertino.1@us.af.mil)
/// @date     2023.06.27
/// @brief    Connect to linux alsa audio devices for data i/o.
//******************************************************************************

// standard includes
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "alsa_func.h"
#include "kill_throbber.h"
#include "logger.h"

// PRIVATE FUNCTIONS //

//convert alsa lib types to dsp node types
enum e_binary_type convert_type(snd_pcm_format_t format);

// COMMON FUNCTIONS //

//create alsa arg struct with params
struct s_alsa_func_args *create_alsa_args(char *p_device_name, snd_pcm_format_t format, unsigned int channels, unsigned int rate)
{
  struct s_alsa_func_args *p_temp = NULL;

  if(!p_device_name)
  {
    fprintf(stderr, "ERROR: Must specify device name.\n");

    return NULL;
  }

  p_temp = malloc(sizeof(struct s_alsa_func_args));

  if(!p_temp) return NULL;

  p_temp->p_device_name = strdup(p_device_name);

  p_temp->format = format;

  p_temp->channels = channels;

  p_temp->rate = rate;

  return p_temp;
}

//free alsa arg struct
void free_alsa_args(struct s_alsa_func_args *p_init_args)
{
  if(!p_init_args)
  {
    fprintf(stderr, "ERROR: Null passed.\n");

    return;
  }

  free(p_init_args->p_device_name);

  free(p_init_args);
}

// THREAD READ FUNCTIONS //

//Setup alsa reading thread
int init_callback_alsa_read(void *p_init_args, void *p_object)
{
  int error = 0;

  struct s_dsp_node *p_dsp_node = NULL;

  struct s_alsa_func_args *p_alsa_args = NULL;

  p_dsp_node = (struct s_dsp_node *)p_object;

  p_alsa_args = (struct s_alsa_func_args *)p_init_args;

  // open alsa for input reading
  error = snd_pcm_open((snd_pcm_t **)&p_dsp_node->p_data, p_alsa_args->p_device_name, SND_PCM_STREAM_CAPTURE, 0);

  if(error < 0)
  {
    logger_error_msg(p_dsp_node->p_logger, "ALSA READ SND OPEN: %s", snd_strerror(error));

    return ~0;
  }

  error = snd_pcm_set_params((snd_pcm_t *)p_dsp_node->p_data, p_alsa_args->format, SND_PCM_ACCESS_RW_INTERLEAVED, p_alsa_args->channels, p_alsa_args->rate, 1, 500000);

  if(error < 0)
  {
    logger_error_msg(p_dsp_node->p_logger, "ALSA READ SET PARAM: %s", snd_strerror(error));

    snd_pcm_close((snd_pcm_t *)p_dsp_node->p_data);

    return ~0;
  }

  p_dsp_node->input_type = DATA_INVALID;

  p_dsp_node->output_type = convert_type(p_alsa_args->format);

  logger_info_msg(p_dsp_node->p_logger, "ALSA, read node created for %p.", p_dsp_node);

  return 0;
}

//Pthread function for threading alsa read
void* pthread_function_alsa_read(void *p_data)
{
  uint8_t *p_buffer = NULL;

  struct s_dsp_node *p_dsp_node = NULL;

  p_dsp_node = (struct s_dsp_node *)p_data;

  if(!p_dsp_node)
  {
    fprintf(stderr, "ERROR: Data Struct is NULL.\n");

    kill_thread = 1;

    goto error_cleanup;
  }

  p_dsp_node->active = 1;

  p_buffer = malloc(p_dsp_node->chunk_size * p_dsp_node->output_type_size);

  if(!p_buffer)
  {
    logger_error_msg(p_dsp_node->p_logger, "ALSA, Could not allocate file read buffer");

    kill_thread = 1;

    goto error_cleanup;
  }

  p_dsp_node->total_bytes_processed = 0;

  logger_info_msg(p_dsp_node->p_logger, "ALSA, read thread started.");

  do
  {
    unsigned long numElemRead   = 0;
    unsigned long numElemWrote  = 0;

    numElemRead = (unsigned long)snd_pcm_readi((snd_pcm_t *)p_dsp_node->p_data, p_buffer, p_dsp_node->chunk_size);

    p_dsp_node->total_bytes_processed += numElemRead * p_dsp_node->output_type_size;

    do
    {
      numElemWrote += ringBufferBlockingWrite(p_dsp_node->p_output_ring_buffer, p_buffer + (numElemWrote * p_dsp_node->output_type_size), numElemRead - numElemWrote, NULL);
    } while(numElemWrote < numElemRead);

  } while(!kill_thread);

error_cleanup:
  free(p_buffer);

  ringBufferEndBlocking(p_dsp_node->p_output_ring_buffer);

  logger_info_msg(p_dsp_node->p_logger, "ALSA, read thread finished.");

  p_dsp_node->active = 0;

  return NULL;
}

//Clean up all allocations from init_callback alsa read
int free_callback_alsa_read(void *p_object)
{
  struct s_dsp_node *p_dsp_node = NULL;

  p_dsp_node = (struct s_dsp_node *)p_object;

  return snd_pcm_close((snd_pcm_t *)p_dsp_node->p_data);
}

// THREAD WRITE FUNCTIONS //

//Setup file writing thread
int init_callback_alsa_write(void *p_init_args, void *p_object)
{
  int error = 0;

  struct s_dsp_node *p_dsp_node = NULL;

  struct s_alsa_func_args *p_alsa_args = NULL;

  p_dsp_node = (struct s_dsp_node *)p_object;

  p_alsa_args = (struct s_alsa_func_args *)p_init_args;

  // open alsa for input reading
  error = snd_pcm_open((snd_pcm_t **)&p_dsp_node->p_data, p_alsa_args->p_device_name, SND_PCM_STREAM_PLAYBACK, 0);

  if(error < 0)
  {
    logger_error_msg(p_dsp_node->p_logger, "ALSA WRITE SND OPEN: %s", snd_strerror(error));

    return ~0;
  }

  error = snd_pcm_set_params((snd_pcm_t *)p_dsp_node->p_data, p_alsa_args->format, SND_PCM_ACCESS_RW_INTERLEAVED, p_alsa_args->channels, p_alsa_args->rate, 1, 500000);

  if(error < 0)
  {
    logger_error_msg(p_dsp_node->p_logger, "ALSA WRITE SET PARAM: %s", snd_strerror(error));

    snd_pcm_close((snd_pcm_t *)p_dsp_node->p_data);

    return ~0;
  }

  p_dsp_node->output_type = DATA_INVALID;

  p_dsp_node->input_type = convert_type(p_alsa_args->format);

  logger_info_msg(p_dsp_node->p_logger, "ALSA, write node created for %p.", p_dsp_node);

  return 0;
}

//Pthread function for threading write
void* pthread_function_alsa_write(void *p_data)
{
  unsigned long numElemRead = 0;

  uint8_t *p_buffer = NULL;

  struct s_dsp_node *p_dsp_node = NULL;

  p_dsp_node = (struct s_dsp_node *)p_data;

  if(!p_dsp_node)
  {
    fprintf(stderr, "ERROR: Data Struct is NULL.\n");

    kill_thread = 1;

    goto error_cleanup;
  }

  p_dsp_node->active = 1;

  if(!p_dsp_node->p_input_ring_buffer)
  {
    logger_error_msg(p_dsp_node->p_logger, "ALSA, No input buffer set for file write!\n");

    kill_thread = 1;

    goto error_cleanup;
  }

  p_buffer = malloc(p_dsp_node->chunk_size * p_dsp_node->input_type_size);

  if(!p_buffer)
  {
    logger_error_msg(p_dsp_node->p_logger, "ALSA, Could not allocate file read buffer");

    kill_thread = 1;

    goto error_cleanup;
  }

  p_dsp_node->total_bytes_processed = 0;

  logger_info_msg(p_dsp_node->p_logger, "ALSA, write thread started.");

  do
  {
    snd_pcm_sframes_t numFrameWrote = 0;

    numElemRead = ringBufferBlockingRead(p_dsp_node->p_input_ring_buffer, p_buffer, p_dsp_node->chunk_size, NULL);

    p_dsp_node->total_bytes_processed += numElemRead * p_dsp_node->input_type_size;

    do
    {
      numFrameWrote += snd_pcm_writei((snd_pcm_t *)p_dsp_node->p_data, p_buffer + (numFrameWrote * p_dsp_node->input_type_size), numElemRead - (unsigned long)numFrameWrote);
    } while(numElemRead < (unsigned long)numFrameWrote);

  } while((numElemRead > 0) && !kill_thread);

error_cleanup:
  free(p_buffer);

  ringBufferEndBlocking(p_dsp_node->p_input_ring_buffer);

  logger_info_msg(p_dsp_node->p_logger, "ALSA, write thread finished.");

  p_dsp_node->active = 0;

  return NULL;
}

//Clean up all allocations from init_callback for write
int free_callback_alsa_write(void *p_object)
{
  struct s_dsp_node *p_dsp_node = NULL;

  p_dsp_node = (struct s_dsp_node *)p_object;

  return snd_pcm_close((snd_pcm_t *)p_dsp_node->p_data);
}

//convert alsa lib types to dsp node types
enum e_binary_type convert_type(snd_pcm_format_t format)
{
  switch(format)
  {
    case(SND_PCM_FORMAT_S8):
      return DATA_S8;
    case(SND_PCM_FORMAT_U8):
      return DATA_U8;
    case(SND_PCM_FORMAT_S16_LE):
      return DATA_S16;
    case(SND_PCM_FORMAT_U16_LE):
      return DATA_U16;
    case(SND_PCM_FORMAT_FLOAT):
      return DATA_FLOAT;
    case(SND_PCM_FORMAT_FLOAT64):
      return DATA_DOUBLE;
    default:
      return DATA_UNKNOWN;
  }

  return DATA_UNKNOWN;
}
