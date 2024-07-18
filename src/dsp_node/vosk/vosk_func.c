//******************************************************************************
/// @file     vosk_func.c
/// @author   Jay Convertino(johnathan.convertino.1@us.af.mil)
/// @date     2024.07.15
/// @brief    Process raw audio speech samples (mono) into ANSI text strings.
//******************************************************************************

// standard includes
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "vosk_api.h"
#include "vosk_func.h"
#include "kill_throbber.h"
#include "logger.h"

//private data struct for vosk
struct s_vosk_data
{
  VoskModel       *model;
  VoskSpkModel    *spk_model;
  VoskRecognizer  *recognizer;
};

//Setup file arg struct for file read/write init callbacks
struct s_vosk_func_args *create_vosk_args(float sample_rate, enum e_binary_type sample_type)
{
  struct s_vosk_func_args *p_temp = NULL;

  p_temp = malloc(sizeof(struct s_vosk_func_args));

  if(!p_temp) return NULL;

  switch(sample_type)
  {
    case DATA_S16:
    case DATA_U8:
    case DATA_FLOAT:
      p_temp->sample_type = sample_type;
      break;
    default:
      p_temp->sample_type = DATA_FLOAT;
      fprintf(stderr, "ERROR: Type specified for vosk is incorrect, must be DATA_S16, DATA_FLOAT, or DATA_U8. Defaulting to DATA_FLOAT.\n");
      break;
  }

  p_temp->sample_rate = sample_rate;

  return p_temp;
}

//Free args struct created from create file args
void free_vosk_args(struct s_vosk_func_args *p_init_args)
{
  if(!p_init_args)
  {
    fprintf(stderr, "ERROR: Null passed.\n");

    return;
  }

  free(p_init_args);
}

// THREAD FUNCTIONS //

//Setup vosk processing thread
int init_callback_vosk(void *p_init_args, void *p_object)
{
  struct s_dsp_node *p_dsp_node = NULL;

  struct s_vosk_func_args *p_vosk_args = NULL;

  struct s_vosk_data *p_vosk_data = NULL;

  p_vosk_data = malloc(sizeof(struct s_vosk_data));

  if(!p_vosk_data) return ~0;

  p_dsp_node = (struct s_dsp_node *)p_object;

  p_vosk_args = (struct s_vosk_func_args *)p_init_args;

  p_dsp_node->input_type = p_vosk_args->sample_type;

  p_dsp_node->output_type = DATA_U8;

  p_dsp_node->p_data = p_vosk_data;

  p_vosk_data->model = vosk_model_new("model");

  p_vosk_data->spk_model = vosk_spk_model_new("spk-model");

  p_vosk_data->recognizer = vosk_recognizer_new_spk(p_vosk_data->model, p_vosk_args->sample_rate, p_vosk_data->spk_model);

  logger_info_msg(p_dsp_node->p_logger, "VOSK node created for %p.", p_dsp_node);

  return 0;
}

//Pthread function for threading vosk
void* pthread_function_vosk(void *p_data)
{
  unsigned long numElemRead   = 0;

  uint8_t *p_buffer = NULL;

  struct s_dsp_node *p_dsp_node = NULL;

  struct s_vosk_data *p_vosk_data = NULL;

  p_dsp_node = (struct s_dsp_node *)p_data;

  if(!p_dsp_node)
  {
    fprintf(stderr, "ERROR: Data Struct is NULL.\n");

    kill_thread = 1;

    goto error_cleanup;
  }

  p_vosk_data = (struct s_vosk_data *)p_dsp_node->p_data;

  p_dsp_node->active = 1;

  p_buffer = malloc(p_dsp_node->chunk_size * p_dsp_node->output_type_size);

  if(!p_buffer)
  {
    logger_error_msg(p_dsp_node->p_logger, "VOSK, could not allocate input buffer.");

    kill_thread = 1;

    goto error_cleanup;
  }

  p_dsp_node->total_bytes_processed = 0;

  logger_info_msg(p_dsp_node->p_logger, "VOSK thread started.");

  do
  {
    int final                   = 0;
    unsigned long numChars      = 0;
    unsigned long numElemWrote  = 0;
    char *p_json_txt            = NULL;

    numElemRead = ringBufferBlockingRead(p_dsp_node->p_input_ring_buffer, p_buffer, p_dsp_node->chunk_size, NULL);

    if(numElemRead <= 0) continue;

    p_dsp_node->total_bytes_processed += numElemRead * p_dsp_node->input_type_size;

    switch(p_dsp_node->input_type)
    {
      case(DATA_U8):
        final = vosk_recognizer_accept_waveform(p_vosk_data->recognizer, p_buffer, numElemRead);
        break;
      case(DATA_S16):
        final = vosk_recognizer_accept_waveform_s(p_vosk_data->recognizer, (short *)p_buffer, numElemRead);
        break;
      case(DATA_FLOAT):
      default:
        final = vosk_recognizer_accept_waveform_f(p_vosk_data->recognizer, (float *)p_buffer, numElemRead);
        break;
    }

    if(final <= 0) continue;

    p_json_txt = vosk_recognizer_result(p_vosk_data->recognizer);

    numChars = strlen(p_json_txt);

    do
    {
      numElemWrote += ringBufferBlockingWrite(p_dsp_node->p_output_ring_buffer, p_json_txt + numElemWrote, numChars - numElemWrote, NULL);
    } while(numElemWrote < numChars);

  } while((numElemRead > 0) && !kill_thread);

error_cleanup:
  free(p_buffer);

  ringBufferEndBlocking(p_dsp_node->p_output_ring_buffer);

  logger_info_msg(p_dsp_node->p_logger, "VOSK thread finished.");

  p_dsp_node->active = 0;

  return NULL;
}

//Clean up all allocations from init_callback read
int free_callback_vosk(void *p_object)
{
  struct s_dsp_node *p_dsp_node = NULL;

  struct s_vosk_data *p_vosk_data = NULL;

  p_dsp_node = (struct s_dsp_node *)p_object;

  p_vosk_data = (struct s_vosk_data *)p_dsp_node->p_data;

  if(!p_vosk_data) return 0;

  vosk_recognizer_free(p_vosk_data->recognizer);

  vosk_spk_model_free(p_vosk_data->spk_model);

  vosk_model_free(p_vosk_data->model);

  return 0;
}
