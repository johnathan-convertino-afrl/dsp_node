//******************************************************************************
/// @file     codec2_func.h
/// @author   Jay Convertino(johnathan.convertino.1@us.af.mil)
/// @date     2023.06.29
/// @brief    CODEC2 datac1 modulation/demodulation routines.
//******************************************************************************

#ifndef __codec2_func
#define __codec2_func

#include "freedv_api.h"
#include "dsp_node_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @struct s_codec2_func_args
 * @brief Contains argument data for codefc2 node creation (pass to p_init_args for init_callback).
 */
struct s_codec2_func_args
{
  /**
   * @var s_codec2_func_args::sample_type
   * sample type of codec2 datac1
   */
  enum e_binary_type sample_type;
};

// COMMON FUNCTIONS //

/**************************************************************************//**
  * @brief Setup codec2 arg struct for mod/demod init callbacks
  *
  * @param sample_type Modulation output format, demodulation input format.
  * DATA_S16 for real data signed 16bit, DATA_CFLOAT for complex floats.
  *
  * @return Arg struct
  ****************************************************************************/
struct s_codec2_func_args *create_codec2_args(enum e_binary_type sample_type);

/**************************************************************************//**
  * @brief Free args struct created from create codec2 args
  *
  * @param p_init_args codec2 args struct to free
  ****************************************************************************/
void free_codec2_args(struct s_codec2_func_args *p_init_args);

// THREAD MODULATE FUNCTIONS //

/**************************************************************************//**
  * @brief Setup codec2 DATAC1 modulation thread
  *
  * @param p_init_args struct s_codec2_func_args
  * @param p_object A dsp_node struct used to change various settings.
  *
  * @return 0 no error, non-zero indicates error.
  ****************************************************************************/
int init_callback_codec2_mod(void *p_init_args, void *p_object);

/**************************************************************************//**
  * @brief Pthread function for threading
  *
  * @param p_data This will contain dsp_node struct so all data is available.
  *
  * @return NULL
  ****************************************************************************/
void* pthread_function_codec2_mod(void *p_data);

/**************************************************************************//**
  * @brief Clean up all allocations from init_callback
  *
  * @param p_object codec2 mod dsp node object to free
  *
  * @return 0 no error, non-zero indicates error.
  ****************************************************************************/
int free_callback_codec2_mod(void *p_object);

// THREAD DEMODULATE FUNCTIONS //

/**************************************************************************//**
  * @brief Setup codec2 DATAC1 demodulation thread
  *
  * @param p_init_args struct s_codec2_func_args
  * @param p_object A dsp_node struct used to change various settings.
  *
  * @return 0 no error, non-zero indicates error.
  ****************************************************************************/
int init_callback_codec2_demod(void *p_init_args, void *p_object);

/**************************************************************************//**
  * @brief Pthread function for threading codec2 complex modulation
  *
  * @param p_data This will contain dsp_node struct so all data is available.
  *
  * @return NULL
  ****************************************************************************/
void* pthread_function_codec2_demod(void *p_data);

/**************************************************************************//**
  * @brief Clean up all allocations from init_callback demodulation
  *
  * @param p_object codec2 demod dsp node object to free
  *
  * @return 0 no error, non-zero indicates error.
  ****************************************************************************/
int free_callback_codec2_demod(void *p_object);

#ifdef __cplusplus
}
#endif

#endif
