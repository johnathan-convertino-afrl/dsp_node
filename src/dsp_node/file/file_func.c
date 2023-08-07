//******************************************************************************
/// @file     file_func.c
/// @author   Jay Convertino(johnathan.convertino.1@us.af.mil)
/// @date     2023.06.22
/// @brief    ANSI C file I/O functions for read and/or write.
//******************************************************************************

// standard includes
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "file_func.h"
#include "kill_throbber.h"

//Setup file arg struct for file read/write init callbacks
struct s_file_func_args *create_file_args(char *p_name, enum e_binary_type input_type, enum e_binary_type output_type, enum e_io_method io_method)
{
  struct s_file_func_args *p_temp = NULL;

  if(!p_name)
  {
    fprintf(stderr, "ERROR: Must specify file name.\n");

    return NULL;
  }

  p_temp = malloc(sizeof(struct s_file_func_args));

  if(!p_temp) return NULL;

  p_temp->p_name = strdup(p_name);

  p_temp->input_type = input_type;

  p_temp->output_type = output_type;

  p_temp->io_method = io_method;

  return p_temp;
}

//Free args struct created from create file args
void free_file_args(struct s_file_func_args *p_init_args)
{
  if(!p_init_args)
  {
    fprintf(stderr, "ERROR: Null passed.\n");

    return;
  }

  free(p_init_args->p_name);

  free(p_init_args);
}

// THREAD READ FUNCTIONS //

//Setup file reading thread
int init_callback_file_read(void *p_init_args, void *p_object)
{
  struct s_dsp_node *p_dsp_node = NULL;

  struct s_file_func_args *p_file_args = NULL;

  p_dsp_node = (struct s_dsp_node *)p_object;

  p_file_args = (struct s_file_func_args *)p_init_args;

  p_dsp_node->input_type = DATA_INVALID;

  p_dsp_node->output_type = p_file_args->output_type;

  // open file for input reading
  p_dsp_node->p_data = fopen(p_file_args->p_name, "rb");

  if(!p_dsp_node->p_data)
  {
    perror("File IO Issue.");

    return ~0;
  }

  return 0;
}

//Pthread function for threading read
void* pthread_function_file_read(void *p_data)
{
  uint8_t *p_buffer = NULL;

  struct s_dsp_node *p_dsp_node = NULL;

  p_dsp_node = (struct s_dsp_node *)p_data;

  if(!p_dsp_node)
  {
    fprintf(stderr, "Data Struct is NULL.\n");
    return NULL;
  }

  p_buffer = malloc(p_dsp_node->chunk_size * p_dsp_node->output_type_size);

  if(!p_buffer)
  {
    perror("Could not allocate file read buffer.");
    return NULL;
  }

  p_dsp_node->total_bytes_processed = 0;

  do
  {
    unsigned long numElemRead   = 0;
    unsigned long numElemWrote  = 0;

    numElemRead = fread(p_buffer, p_dsp_node->output_type_size, p_dsp_node->chunk_size, (FILE *)p_dsp_node->p_data);

    p_dsp_node->total_bytes_processed += numElemRead * p_dsp_node->output_type_size;

    do
    {
      numElemWrote += ringBufferBlockingWrite(p_dsp_node->p_output_ring_buffer, p_buffer + (numElemWrote * p_dsp_node->output_type_size), numElemRead - numElemWrote, NULL);
    } while(numElemWrote < numElemRead);

  } while(!feof((FILE *)p_dsp_node->p_data) && !kill_thread);

  ringBufferEndBlocking(p_dsp_node->p_output_ring_buffer);

  free(p_buffer);

  return NULL;
}

//Clean up all allocations from init_callback read
int free_callback_file_read(void *p_object)
{
  struct s_dsp_node *p_dsp_node = NULL;

  p_dsp_node = (struct s_dsp_node *)p_object;

  if(!p_dsp_node->p_data) return 0;

  return fclose((FILE *)p_dsp_node->p_data);
}

// THREAD WRITE FUNCTIONS //

//Setup file writing thread
int init_callback_file_write(void *p_init_args, void *p_object)
{
  struct s_dsp_node *p_dsp_node = NULL;

  struct s_file_func_args *p_file_args = NULL;

  p_dsp_node = (struct s_dsp_node *)p_object;

  p_file_args = (struct s_file_func_args *)p_init_args;

  p_dsp_node->input_type = p_file_args->input_type;

  p_dsp_node->output_type = DATA_INVALID;

  // open file for writing
  switch(p_file_args->io_method)
  {
    case(OVERWRITE_FILE):
      p_dsp_node->p_data = fopen(p_file_args->p_name, "wb");
      break;
    case(APPEND_FILE):
      p_dsp_node->p_data = fopen(p_file_args->p_name, "ab");
      break;
  }

  if(!p_dsp_node->p_data)
  {
    perror("File IO Issue.");

    return ~0;
  }

  return 0;
}

//Pthread function for threading write
void* pthread_function_file_write(void *p_data)
{
  unsigned long numElemRead = 0;

  uint8_t *p_buffer = NULL;

  struct s_dsp_node *p_dsp_node = NULL;

  p_dsp_node = (struct s_dsp_node *)p_data;

  if(!p_dsp_node)
  {
    fprintf(stderr, "Data Struct is NULL.\n");
    return NULL;
  }

  if(!p_dsp_node->p_input_ring_buffer)
  {
    fprintf(stderr, "ERROR: No input buffer set for file write!\n");
    return NULL;
  }

  p_buffer = malloc(p_dsp_node->chunk_size * p_dsp_node->input_type_size);

  if(!p_buffer)
  {
    perror("Could not allocate file read buffer.");
    return NULL;
  }

  //this line sets the buffer size to the chunk size and allows data to be flushed out quicker for writes.
  //keeps linux from buffering up so much data before writing it.
  setvbuf((FILE *)p_dsp_node->p_data, NULL, _IOFBF, p_dsp_node->chunk_size * p_dsp_node->input_type_size);

  p_dsp_node->total_bytes_processed = 0;

  do
  {
    unsigned long numElemWrote  = 0;

    numElemRead = ringBufferBlockingRead(p_dsp_node->p_input_ring_buffer, p_buffer, p_dsp_node->chunk_size, NULL);

    p_dsp_node->total_bytes_processed += numElemRead * p_dsp_node->input_type_size;

    do
    {
      numElemWrote += fwrite(p_buffer + (numElemWrote * p_dsp_node->input_type_size), p_dsp_node->input_type_size, numElemRead - numElemWrote, (FILE *)p_dsp_node->p_data);
    } while(numElemRead < numElemWrote);

    fflush((FILE *)p_dsp_node->p_data);

  } while((numElemRead > 0) && !kill_thread);

  ringBufferEndBlocking(p_dsp_node->p_input_ring_buffer);

  free(p_buffer);

  return NULL;
}

//Clean up all allocations from init_callback write
int free_callback_file_write(void *p_object)
{
  struct s_dsp_node *p_dsp_node = NULL;

  p_dsp_node = (struct s_dsp_node *)p_object;

  if(!p_dsp_node->p_data) return 0;

  return fclose((FILE *)p_dsp_node->p_data);
}
