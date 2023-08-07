//******************************************************************************
/// @file     dsp_node.h
/// @author   Jay Convertino(johnathan.convertino.1@us.af.mil)
/// @date     2023.06.20
/// @brief    Digital Signal Processing nodes for one to one data transfer.
/// @details  Callbacks are used to create the unique nodes.
//******************************************************************************

#ifndef __dsp_node
#define __dsp_node

// includes
#include "dsp_node_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************//**
  * @brief Allocate the dsp_node struct.
  *
  * @param buffer_size size of ringbuffer total
  * @param chunk_size size to read or write from ringbuffer
  *
  * @return allocated base dsp node in need of setup.
  ****************************************************************************/
struct s_dsp_node * dsp_create(unsigned long buffer_size, unsigned long chunk_size);

/**************************************************************************//**
  * @brief Setup dsp_node with specifics to it's processing functionality.
  *
  * @param p_object struct s_dsp_node object from dsp_create
  * @param init_call callback function for initialization of specific DSP functions
  * @param thread_func callback function for input/output processing specific to the DSP
  * @param free_call callback function for deallocating DSP specific items.
  * @param p_init_args node specific initialization arguments for init_call.
  *
  * @return 0 no error, non-zero indicates error.
  ****************************************************************************/
int dsp_setup(struct s_dsp_node * const p_object, init_callback init_call, pthread_function thread_func, free_callback free_call, void *p_init_args);

/**************************************************************************//**
  * @brief Set an input node to the current node specified by p_object.
  *
  * @param p_object struct s_dsp_node object
  * @param p_input_object struct s_dsp_node object to set as a input to p_object.
  *
  * @return 0 no error, non-zero indicates error.
  ****************************************************************************/
int dsp_setInput(struct s_dsp_node * const p_object, struct s_dsp_node const * const p_input_object);

/**************************************************************************//**
  * @brief Start the thread using pthread function passed to create.
  *
  * @param p_object struct s_dsp_node object
  *
  * @return 0 no error, non-zero indicates error.
  ****************************************************************************/
int dsp_start(struct s_dsp_node * const p_object);

/**************************************************************************//**
  * @brief Wait for the pthread to finish
  *
  * @param p_object struct s_dsp_node object
  *
  * @return 0 no error, non-zero indicates error.
  ****************************************************************************/
int dsp_wait(struct s_dsp_node const * const p_object);

/**************************************************************************//**
  * @brief Force the pthread to end
  *
  * @param p_object struct s_dsp_node object
  *
  * @return 0 no error, non-zero indicates error.
  ****************************************************************************/
int dsp_end(struct s_dsp_node const * const p_object);

/**************************************************************************//**
  * @brief remove all allocations from create
  *
  * @param p_object struct s_dsp_node object
  *
  * @return 0 no error, non-zero indicates error.
  ****************************************************************************/
void dsp_cleanup(struct s_dsp_node *p_object);

#ifdef __cplusplus
}
#endif

#endif
