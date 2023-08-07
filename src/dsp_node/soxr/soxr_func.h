//******************************************************************************
/// @file     soxr_func.h
/// @author   Jay Convertino(johnathan.convertino.1@us.af.mil)
/// @date     2023.06.29
/// @brief    Resampler for upsampling or downsampling data, complex or real.
//******************************************************************************

#ifndef __soxr_func
#define __soxr_func

#include "dsp_node_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @struct s_soxr_func_args
 * @brief Contains argument data for soxr node creation (pass to p_init_args for init_callback).
 */
struct s_soxr_func_args
{
  /**
   * @var s_soxr_func_args::input_rate
   * input rate to the node
   */
  double input_rate;
  /**
   * @var s_soxr_func_args::output_rate
   * output rate of the node
   */
  double output_rate;
  /**
   * @var s_soxr_func_args::input_type
   * input data format
   */
  enum e_binary_type input_type;
  /**
   * @var s_soxr_func_args::output_type
   * output data format
   */
  enum e_binary_type output_type;
  /**
   * @var s_soxr_func_args::channels
   * number of interleaved channels
   */
  unsigned channels;
};

// COMMON FUNCTIONS //

/**************************************************************************//**
  * @brief Setup soxr arg struct for input/output rates
  *
  * @param input_rate input sample rate
  * @param output_rate output sample rate
  * @param input_type input format
  * @param output_type output format
  * @param channels number of channels, 2 for complex interleaved, or 1 for real.
  *
  * @return Arg struct
  ****************************************************************************/
struct s_soxr_func_args *create_soxr_args(double input_rate, double output_rate, enum e_binary_type input_type, enum e_binary_type output_type, unsigned channels);

/**************************************************************************//**
  * @brief Free args struct created from create soxr args
  *
  * @param p_init_args soxr args struct to free
  ****************************************************************************/
void free_soxr_args(struct s_soxr_func_args *p_init_args);

// THREAD RESAMPLE FUNCTIONS //

/**************************************************************************//**
  * @brief Setup soxr resampler
  *
  * @param p_init_args pass struct s_soxr_func_args from create_soxr_args
  * @param p_object A dsp_node struct used to change various settings.
  *
  * @return 0 no error, non-zero indicates error.
  ****************************************************************************/
int init_callback_soxr(void *p_init_args, void *p_object);

/**************************************************************************//**
  * @brief Pthread function for threading soxr to resample data
  *
  * @param p_data This will contain dsp_node struct so all data is available.
  *
  * @return NULL
  ****************************************************************************/
void* pthread_function_soxr(void *p_data);

/**************************************************************************//**
  * @brief Clean up all allocations from init_callback
  *
  * @param p_object soxr dsp node object to free
  *
  * @return 0 no error, non-zero indicates error.
  ****************************************************************************/
int free_callback_soxr(void *p_object);

#ifdef __cplusplus
}
#endif

#endif
