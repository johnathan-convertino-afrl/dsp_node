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

//Allocate the dsp_node struct with defined buffer size.
struct s_dsp_node * dsp_create(unsigned long buffer_size, unsigned long chunk_size)
{
  struct s_dsp_node *p_temp = NULL;

  p_temp = malloc(sizeof(struct s_dsp_node));

  if(!p_temp)
  {
    perror("DSP Node struct failed:");

    return NULL;
  }

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

  return p_temp;
}

//Setup dsp_node with specifics to it's processing functionality.
int dsp_setup(struct s_dsp_node * const p_object, init_callback init_call, pthread_function thread_func, free_callback free_call, void *p_init_args)
{
  int error = 0;

  if(!p_object)
  {
    fprintf(stderr, "ERROR: object is NULL for setup.\n");

    return ~0;
  }

  if(!init_call || !thread_func || !free_call)
  {
    fprintf(stderr, "ERROR: Callback functions can not be null.\n");

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
    fprintf(stderr, "ERROR: Output ringbuffer init failed\n");

    return ~0;
  }

  return error;
}

//Set an input node to the current node specified by p_object.
int dsp_setInput(struct s_dsp_node * const p_object, struct s_dsp_node const * const p_input_object)
{
  if(!p_object)
  {
    fprintf(stderr, "ERROR: object is NULL for setInput.\n");

    return ~0;
  }

  if(!p_input_object)
  {
    fprintf(stderr, "ERROR: input object is NULL for setInput.\n");

    return ~0;
  }

  //check data types to make sure they are the same. Also check if the input is set to something valid.
  if(p_input_object->output_type == DATA_INVALID)
  {
    fprintf(stderr, "WARNING: Data type is invalid for input node output. This node does not output data from its output ringbuffer.\n");
  }

  if(p_object->input_type == DATA_INVALID)
  {
      fprintf(stderr, "WARNING: Data type is invalid, no input needed or error has occured in init callback.\n");
  }

  if(p_object->input_type != p_input_object->output_type)
  {
    fprintf(stderr, "WARNING: Formats between nodes do not match. Input needed is %d to node. Output is %d from input node.\n", p_object->input_type, p_input_object->output_type);
  }

  p_object->p_input_ring_buffer = p_input_object->p_output_ring_buffer;

  return 0;
}

//Start the thread using pthread function passed to create.
int dsp_start(struct s_dsp_node * const p_object)
{
  if(!p_object)
  {
    fprintf(stderr, "ERROR: object is NULL for wait.\n");

    return ~0;
  }

  return pthread_create(&p_object->dsp_thread, NULL, p_object->thread_func, p_object);
}

//Wait for the pthread to finish
int dsp_wait(struct s_dsp_node const * const p_object)
{
  if(!p_object)
  {
    fprintf(stderr, "ERROR: object is NULL for wait.\n");

    return ~0;
  }

  return pthread_join(p_object->dsp_thread, NULL);
}

//Force the pthread to end
int dsp_end(struct s_dsp_node const * const p_object)
{
  if(!p_object)
  {
    fprintf(stderr, "ERROR: object is NULL for end.\n");

    return ~0;
  }

  return pthread_kill(p_object->dsp_thread, SIGUSR1);
}

//remove all allocations from create
void dsp_cleanup(struct s_dsp_node *p_object)
{
  if(!p_object)
  {
    fprintf(stderr, "ERROR: object is NULL for cleanup.\n");

    return;
  }

  if(p_object->free_call)
  {
    p_object->free_call(p_object);
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
