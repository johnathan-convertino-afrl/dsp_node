//******************************************************************************
/// @file     dsp_node.c
/// @author   Jay Convertino(johnathan.convertino.1@us.af.mil)
/// @date     2023.06.20
/// @brief    Digital Signal Processing nodes for one to one data transfer.
/// @details  Callbacks are used to create the unique nodes.
//******************************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <signal.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

#include "dsp_node.h"

//return the size of the type in bytes.
unsigned int get_type_size(enum e_binary_type type);
//global logger
static struct s_logger *gp_logger = NULL;
//global node count
static unsigned long int g_node_count = 0;

//Allocate the dsp_node struct with defined buffer size.
struct s_dsp_node * dsp_create(unsigned long buffer_size, unsigned long chunk_size)
{
  struct s_dsp_node *p_temp = NULL;

  p_temp = malloc(sizeof(struct s_dsp_node));

  if(!p_temp)
  {
    perror("DSP Node struct failed");

    return NULL;
  }

  if(!gp_logger)
  {
    gp_logger = logger_create("dsp_node");

    if(!gp_logger)
    {
      perror("Logger Creation failed");

      return NULL;
    }
  }

  p_temp->p_logger = gp_logger;

  p_temp->input_type = DATA_U8;

  p_temp->input_type_size = 1;

  p_temp->output_type = DATA_U8;

  p_temp->output_type_size = 1;

  p_temp->p_input_ring_buffer = NULL;

  p_temp->p_output_ring_buffer = NULL;

  p_temp->chunk_size = chunk_size;

  p_temp->buffer_size = buffer_size;

  p_temp->init_call = NULL;

  p_temp->thread_func = NULL;

  p_temp->free_call = NULL;

  p_temp->p_data = NULL;

  p_temp->total_bytes_processed = 0;

  p_temp->active = 0;

  p_temp->id_number = ++g_node_count;

  logger_info_msg(gp_logger, "DSP NODE %p created.", p_temp);

  return p_temp;
}

//Setup dsp_node with specifics to it's processing functionality.
int dsp_setup(struct s_dsp_node * const p_object, init_callback init_call, pthread_function thread_func, free_callback free_call, void *p_init_args)
{
  int error = 0;

  if(!p_object)
  {
    logger_error_msg(gp_logger, "Object is NULL for setup.");

    return ~0;
  }

  if(!init_call || !thread_func || !free_call)
  {
    logger_error_msg(gp_logger, "Callback functions can not be null.");

    return ~0;
  }

  p_object->init_call = init_call;

  p_object->thread_func = thread_func;

  p_object->free_call = free_call;

  error = p_object->init_call(p_init_args, p_object);

  p_object->input_type_size = get_type_size(p_object->input_type);

  p_object->output_type_size = get_type_size(p_object->output_type);

  //data invalid means no output ring buffer is used.
  if(p_object->output_type == DATA_INVALID) return error;

  p_object->p_output_ring_buffer = initRingBuffer(p_object->buffer_size, p_object->output_type_size);

  if(!p_object->p_output_ring_buffer)
  {
    logger_error_msg(gp_logger, "Output ringbuffer init failed.");

    return ~0;
  }

  return error;
}

//Set an input node to the current node specified by p_object.
int dsp_setInput(struct s_dsp_node * const p_object, struct s_dsp_node const * const p_input_object)
{
  if(!p_object)
  {
    logger_error_msg(gp_logger, "Object is NULL for setInput.");

    return ~0;
  }

  if(!p_input_object)
  {
    logger_error_msg(gp_logger, "Input object is NULL for setInput.");

    return ~0;
  }

  //check data types to make sure they are the same. Also check if the input is set to something valid.
  if(p_input_object->output_type == DATA_INVALID)
  {
    logger_warning_msg(gp_logger, "Data type is invalid for input node output. This node does not output data from its output ringbuffer.");
  }

  if(p_object->input_type == DATA_INVALID)
  {
      logger_warning_msg(gp_logger, "Data type is invalid, no input needed or error has occured in init callback.");
  }

  if(p_object->input_type != p_input_object->output_type)
  {
    logger_warning_msg(gp_logger, "Formats between nodes do not match. Input needed is %d to node. Output is %d from input node.", p_object->input_type, p_input_object->output_type);
  }

  p_object->p_input_ring_buffer = p_input_object->p_output_ring_buffer;

  logger_info_msg(gp_logger, "DSP NODE %p has input from %p.", p_object, p_input_object);

  return 0;
}

//Start the thread using pthread function passed to create.
int dsp_start(struct s_dsp_node * const p_object)
{
  if(!p_object)
  {
    logger_error_msg(gp_logger, "Object is NULL for wait.");

    return ~0;
  }

  logger_info_msg(gp_logger, "DSP NODE %p started.", p_object);

  return pthread_create(&p_object->dsp_thread, NULL, p_object->thread_func, p_object);
}

//Wait for the pthread to finish
int dsp_wait(struct s_dsp_node const * const p_object)
{
  int error = 0;

  if(!p_object)
  {
    logger_error_msg(gp_logger, "Object is NULL for wait.");

    return ~0;
  }

  error = pthread_join(p_object->dsp_thread, NULL);

  logger_info_msg(gp_logger, "DSP NODE %p joined.", p_object);

  return error;
}

//Force the pthread to end
int dsp_end(struct s_dsp_node const * const p_object)
{
  if(!p_object)
  {
    logger_error_msg(gp_logger, "Object is NULL for end.");

    return ~0;
  }

  return pthread_kill(p_object->dsp_thread, SIGUSR1);
}

//remove all allocations from create
void dsp_cleanup(struct s_dsp_node *p_object)
{
  if(!p_object)
  {
    logger_error_msg(gp_logger, "Object is NULL for cleanup.");

    return;
  }

  while(p_object->active);

  if(p_object->free_call)
  {
    p_object->free_call(p_object);
  }

  g_node_count--;

  if(gp_logger && !g_node_count)
  {
    logger_info_msg(gp_logger, "LOGGER FINISHED, DSP NODE CLEANUP STARTED.");

    logger_cleanup(gp_logger);

    gp_logger = NULL;
  }

  if(p_object->output_type != DATA_INVALID) freeRingBuffer(&p_object->p_output_ring_buffer);

  free(p_object);
}

//return the size of the type in bytes.
unsigned int get_type_size(enum e_binary_type type)
{
  switch(type)
  {
    case(DATA_S8):
    case(DATA_U8):
      return 1;
    case(DATA_CS8):
    case(DATA_S16):
    case(DATA_U16):
      return 2;
    case(DATA_CS16):
    case(DATA_FLOAT):
      return 4;
    case(DATA_CFLOAT):
      return 8;
    case(DATA_DOUBLE):
      return 8;
    case(DATA_CDOUBLE):
      return 16;
    default:
      return 0;
  }

  return 0;
}
