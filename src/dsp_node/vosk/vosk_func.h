//******************************************************************************
/// @file     vosk_func.h
/// @author   Jay Convertino(johnathan.convertino.1@us.af.mil)
/// @date     2024.07.15
/// @brief    Process raw audio speech samples (mono) into ANSI text strings.
//******************************************************************************

#ifndef __vosk_func
#define __vosk_func

#include "dsp_node_types.h"

#ifdef __cplusplus
extern "C" {

#endif

/**
 * @struct s_vosk_func_args
 * @brief Contains argument data for vosk node creation (pass to p_init_args for init_callback).
 */
struct s_vosk_func_args
{
  /**
   * @var s_vosk_func_args::output_rate
   * sample rate of the node
   */
  float sample_rate;
  /**
   * @var s_vosk_func_args::input_type
   * data format
   */
  enum e_binary_type sample_type;
};

// COMMON FUNCTIONS //

/**************************************************************************//**
  * @brief Setup vosk arg struct for vosk init callbacks
  *
  * @param sample_rate set the sample rate based upon incomming audio data.
  * @param sample_type set the type, DATA_FLOAT, DATA_U8 (char), DATA_S16 (short)
  *
  * @return Arg struct
  ****************************************************************************/
struct s_vosk_func_args *create_vosk_args(float sample_rate, enum e_binary_type sample_type);

/**************************************************************************//**
  * @brief Free args struct created from create vosk args
  *
  * @param p_init_args Vosk args struct to free
  ****************************************************************************/
void free_vosk_args(struct s_vosk_func_args *p_init_args);

// THREAD FUNCTIONS //

/**************************************************************************//**
  * @brief Setup Vosk speech to txt thread
  *
  * @param p_init_args Takes a vosk setup struct.
  * @param p_object A dsp_node struct used to change various settings.
  *
  * @return 0 no error, non-zero indicates error.
  ****************************************************************************/
int init_callback_vosk(void *p_init_args, void *p_object);

/**************************************************************************//**
  * @brief Pthread function for threading
  *
  * @param p_data This will contain dsp_node struct so all data is available.
  *
  * @return NULL
  ****************************************************************************/
void* pthread_function_vosk(void *p_data);

/**************************************************************************//**
  * @brief Clean up all allocations from init_callback
  *
  * @param p_object vosk dsp node object to free.
  *
  * @return 0 no error, non-zero indicates error.
  ****************************************************************************/
int free_callback_vosk(void *p_object);

#ifdef __cplusplus
}
#endif

#endif
