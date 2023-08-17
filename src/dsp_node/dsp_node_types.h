//******************************************************************************
/// @file     dsp_node_types.h
/// @author   Jay Convertino(johnathan.convertino.1@us.af.mil)
/// @date     2023.06.26
/// @brief    Digital Signal Processing nodes for one to one data transfer.
/// @details  Callbacks are used to create the unique nodes.
//******************************************************************************

#ifndef __dsp_node_types
#define __dsp_node_types

// includes
#include "ringBuffer.h"
#include "logger.h"

typedef int (*init_callback)(void *p_init_args, void *p_object);
typedef void* (*pthread_function)(void *p_data);
typedef int (*free_callback)(void *p_object);

/**
 * @enum e_binary_type
 * A enumeration of binary formats so when set input is used, or start it will warn of any type to type errors. DATA_C... indicates complex type.
 */
enum e_binary_type {DATA_INVALID=-1, DATA_S8=0, DATA_U8, DATA_CS8, DATA_S16, DATA_U16, DATA_CS16, DATA_S32, DATA_U32, DATA_FLOAT, DATA_CFLOAT, DATA_DOUBLE, DATA_CDOUBLE, DATA_UNKNOWN};

/**
 * @struct s_dsp_node
 * @brief Contains data for DSP nodes, such as callbacks and private data.
 */
struct s_dsp_node
{
  /**
   * @var s_dsp_node::p_logger
   * global logger pointer
   */
  struct s_logger *p_logger;
  /**
   * @var s_dsp_node::total_bytes_processed
   * number of bytes output by the node.
   */
  unsigned long total_bytes_processed;
  /**
   * @var s_dsp_node::buffer_size
   * size to set ringbuffer
   */
  unsigned long buffer_size;
  /**
   * @var s_dsp_node::chunk_size
   * size to read/write from ringbuffer
   */
  unsigned long chunk_size;
  /**
   * @var s_dsp_node::input_type
   * enum set by init_callback that specifies the input data type.
   */
  enum e_binary_type input_type;
  /**
   * @var s_dsp_node::input_type_size
   * size in bytes of the input type
   */
  unsigned int input_type_size;
  /**
   * @var s_dsp_node::output_type
   * enum set by init_callback that specifies the output data type.
   */
  enum e_binary_type output_type;
  /**
   * @var s_dsp_node::output_type_size
   * size in bytes of the output type
   */
  unsigned int output_type_size;
  /**
   * @var s_dsp_node::p_input_ring_buffer
   * input data ring buffer set by set input.
   */
  struct s_ringBuffer *p_input_ring_buffer;
  /**
   * @var s_dsp_node::p_output_ring_buffer
   * output data ring buffer created by the node that creates this struct.
   */
  struct s_ringBuffer *p_output_ring_buffer;
  /**
   * @var s_dsp_node::dsp_thread
   * pthread thread
   */
  pthread_t dsp_thread;
  /**
   * @var s_dsp_node::init_call
   * Callback to initialize node specific functionality.
   */
  init_callback init_call;
  /**
   * @var s_dsp_node::thread_func
   * Function pointer for pthread to use for its thread.
   */
  pthread_function thread_func;
  /**
   * @var s_dsp_node::free_call
   * Callback to free initialization callback allocations.
   */
  free_callback free_call;
  /**
   * @var s_dsp_node::p_data
   * void pointer for init/free callbacks to use for data storage.
   */
  void *p_data;
};

#endif
