//******************************************************************************
/// @file     alsa_func.h
/// @author   Jay Convertino(johnathan.convertino.1@us.af.mil)
/// @date     2023.06.27
/// @brief    Connect to linux alsa audio devices for data i/o.
//******************************************************************************

#ifndef __alsa_func
#define __alsa_func

#include <alsa/asoundlib.h>

#include "dsp_node_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @struct s_alsa_func_args
 * @brief Contains argument data for alsa node creation (pass to p_init_args for init_callback).
 */
struct s_alsa_func_args
{
  /**
   * @var s_alsa_func_args::p_device_name
   * name of the device
   */
  char *p_device_name;
  /**
   * @var s_alsa_func_args::format
   * format of the data for alsa
   */
  snd_pcm_format_t format;
  /**
   * @var s_alsa_func_args::channels
   * number of channels for alsa to use
   */
  unsigned int channels;
  /**
   * @var s_alsa_func_args::rate
   * sample rate for device io
   */
  unsigned int rate;
};

// COMMON FUNCTIONS //

/**************************************************************************//**
  * @brief Setup alsa arg struct for alsa read/write init callbacks
  *
  * @param p_device_name Name of the device to open
  * @param format Format per alsa snd_pcm_format_t
  * @param channels Number of channels, 1 = mono, 2 = Stereo
  * @param rate Sample rate of the device
  *
  * @return Arg struct
  ****************************************************************************/
struct s_alsa_func_args *create_alsa_args(char *p_device_name, snd_pcm_format_t format, unsigned int channels, unsigned int rate);

/**************************************************************************//**
  * @brief Free args struct created from create alsa args
  *
  * @param p_init_args ALSA args struct to free
  ****************************************************************************/
void free_alsa_args(struct s_alsa_func_args *p_init_args);

// THREAD READ FUNCTIONS //

/**************************************************************************//**
  * @brief Setup alsa reading thread
  *
  * @param p_init_args struct s_alsa_func_args created by create_alsa_args
  * @param p_object A dsp_node struct used to change various settings.
  *
  * @return 0 no error, non-zero indicates error.
  ****************************************************************************/
int init_callback_alsa_read(void *p_init_args, void *p_object);

/**************************************************************************//**
  * @brief Pthread function for threading alsa device reads (input audio device).
  *
  * @param p_data This will contain dsp_node struct, all data members are available.
  *
  * @return NULL
  ****************************************************************************/
void* pthread_function_alsa_read(void *p_data);

/**************************************************************************//**
  * @brief Clean up all allocations from init_callback alsa read
  *
  * @param p_object alsa read dsp node object to free
  *
  * @return 0 no error, non-zero indicates error.
  ****************************************************************************/
int free_callback_alsa_read(void *p_object);

// THREAD WRITE FUNCTIONS //

/**************************************************************************//**
  * @brief Setup alsa writing thread
  *
  * @param p_init_args struct s_alsa_func_args created by create_alsa_args
  * @param p_object A dsp_node struct used to change various settings.
  *
  * @return 0 no error, non-zero indicates error.
  ****************************************************************************/
int init_callback_alsa_write(void *p_init_args, void *p_object);

/**************************************************************************//**
  * @brief Pthread function for threading alsa device writes (ouput audio device).
  *
  * @param p_data This will contain dsp_node struct so all data is available.
  *
  * @return NULL
  ****************************************************************************/
void* pthread_function_alsa_write(void *p_data);

/**************************************************************************//**
  * @brief Clean up all allocations from init_callback alsa write
  *
  * @param p_object alsa write dsp node object to free
  *
  * @return 0 no error, non-zero indicates error.
  ****************************************************************************/
int free_callback_alsa_write(void *p_object);

#ifdef __cplusplus
}
#endif

#endif
