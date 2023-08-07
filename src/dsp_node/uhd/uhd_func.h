//******************************************************************************
/// @file     uhd_func.h
/// @author   Jay Convertino(johnathan.convertino.1@us.af.mil)
/// @date     2023.06.30
/// @brief    Create a signal UHD connection for tx and/or rx.
//******************************************************************************

#ifndef __uhd_func
#define __uhd_func

#include "dsp_node_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @struct s_uhd_func_args
 * @brief Contains argument data for file node creation (pass to p_init_args for init_callback).
 */
struct s_uhd_func_args
{
  /**
   * @var s_uhd_func_args::p_device_args
   * UHD device args
   */
  char *p_device_args;
  /**
   * @var s_uhd_func_args::freq
   * Center frequnecy of radio in hz
   */
  double freq;
  /**
   * @var s_uhd_func_args::rate
   * Sample rate of radio in hz
   */
  double rate;
  /**
   * @var s_uhd_func_args::gain
   * Input/Output power?
   */
  double gain;
  /**
   * @var s_uhd_func_args::bandwidth
   * Bandwidth of the radio around the center frequency in hz
   */
  double bandwidth;
  /**
   * @var s_uhd_func_args::channel
   * channel to tune to, always 0 in our case.
   */
  size_t channel;
  /**
   * @var s_uhd_func_args::p_cpu_data
   * CPU type for UHD data.
   */
  char *p_cpu_data;
};

// COMMON FUNCTIONS //

/**************************************************************************//**
  * @brief Setup uhd arg struct for init callbacks, this will apply to RX/TX
  *
  * @param p_device_args UHD string of arguments to setup radio connection.
  * @param freq Center frequeny to tune to.
  * @param rate Sample rate of radio output.
  * @param gain Output power of the radio
  * @param bandwidth How large of slice to take around the center frequeny.
  * @param p_cpu_data String name of the data type for the CPU data (radio is set for SC16).
  * Types are fc64 (complex double), fc32 (complex float), sc16 (complex signed int 16bit),
  * and sc8 (complex signed int 8bit).
  *
  * @return Arg struct
  ****************************************************************************/
struct s_uhd_func_args *create_uhd_args(char *p_device_args, double freq, double rate, double gain, double bandwidth, char *p_cpu_data);

/**************************************************************************//**
  * @brief Free args struct created from create uhd args
  *
  * @param p_init_args uhd args struct to free
  ****************************************************************************/
void free_uhd_args(struct s_uhd_func_args *p_init_args);

// THREAD RX FUNCTIONS //

/**************************************************************************//**
  * @brief Setup base UHD config for RX thread
  *
  * @param p_init_args struct s_uhd_func_args created by create_uhd_func_args
  * @param p_object A dsp_node struct used to change various settings.
  *
  * @return 0 no error, non-zero indicates error.
  ****************************************************************************/
int init_callback_uhd_rx(void *p_init_args, void *p_object);

/**************************************************************************//**
  * @brief Pthread function for RX threading
  *
  * @param p_data This will contain dsp_node struct so all data is available.
  *
  * @return NULL
  ****************************************************************************/
void* pthread_function_uhd_rx(void *p_data);

// THREAD TX FUNCTIONS //

/**************************************************************************//**
  * @brief Setup base UHD config for TX thread
  *
  * @param p_init_args struct s_uhd_func_args created by create_uhd_func_args.
  * @param p_object A dsp_node struct used to change various settings.
  *
  * @return 0 no error, non-zero indicates error.
  ****************************************************************************/
int init_callback_uhd_tx(void *p_init_args, void *p_object);

/**************************************************************************//**
  * @brief Pthread function for TX threading
  *
  * @param p_data This will contain dsp_node struct so all data is available.
  *
  * @return NULL
  ****************************************************************************/
void* pthread_function_uhd_tx(void *p_data);

/**************************************************************************//**
  * @brief Clean up all allocations from init_callback
  *
  * @param p_object uhd tx and/or rx dsp node object to free
  *
  * @return 0 no error, non-zero indicates error.
  ****************************************************************************/
int free_callback_uhd(void *p_object);

#ifdef __cplusplus
}
#endif

#endif
