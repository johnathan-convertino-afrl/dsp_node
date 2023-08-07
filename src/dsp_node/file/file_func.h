//******************************************************************************
/// @file     file_func.h
/// @author   Jay Convertino(johnathan.convertino.1@us.af.mil)
/// @date     2023.06.22
/// @brief    ANSI C file I/O functions for read and/or write.
//******************************************************************************

#ifndef __file_func
#define __file_func

#include "dsp_node_types.h"

#ifdef __cplusplus
extern "C" {

#endif

/**
 * @enum e_io_method
 * A enumeration of file write types, append will only allow data to be added to a new or existing file. Overwrite will destroy existing data.
 */
enum e_io_method {APPEND_FILE, OVERWRITE_FILE};

/**
 * @struct s_file_func_args
 * @brief Contains argument data for file node creation (pass to p_init_args for init_callback).
 */
struct s_file_func_args
{
  /**
   * @var s_file_func_args::p_name
   * name of the file
   */
  char *p_name;
  /**
   * @var s_file_func_args::input_type
   * input data format
   */
  enum e_binary_type input_type;
  /**
   * @var s_file_func_args::output_type
   * output data format
   */
  enum e_binary_type output_type;
  /**
   * @var s_file_func_args::io_method
   * file open io method
   */
  enum e_io_method io_method;
};

// COMMON FUNCTIONS //

/**************************************************************************//**
  * @brief Setup file arg struct for file read/write init callbacks
  *
  * @param p_name file name, string
  * @param input_type input format (ignored for read)
  * @param output_type output format (ignored for write)
  * @param io_method how to open for write. Overwrite destroy existing,
  * Append will add data. (ignored for read).
  *
  * @return Arg struct
  ****************************************************************************/
struct s_file_func_args *create_file_args(char *p_name, enum e_binary_type input_type, enum e_binary_type output_type, enum e_io_method io_method);

/**************************************************************************//**
  * @brief Free args struct created from create file args
  *
  * @param p_init_args Filr args struct to free
  ****************************************************************************/
void free_file_args(struct s_file_func_args *p_init_args);

// THREAD READ FUNCTIONS //

/**************************************************************************//**
  * @brief Setup file reading thread
  *
  * @param p_init_args Takes a file name for opening.
  * @param p_object A dsp_node struct used to change various settings.
  *
  * @return 0 no error, non-zero indicates error.
  ****************************************************************************/
int init_callback_file_read(void *p_init_args, void *p_object);

/**************************************************************************//**
  * @brief Pthread function for threading
  *
  * @param p_data This will contain dsp_node struct so all data is available.
  *
  * @return NULL
  ****************************************************************************/
void* pthread_function_file_read(void *p_data);

/**************************************************************************//**
  * @brief Clean up all allocations from init_callback
  *
  * @param p_object file read dsp node object to free.
  *
  * @return 0 no error, non-zero indicates error.
  ****************************************************************************/
int free_callback_file_read(void *p_object);

// THREAD WRITE FUNCTIONS //

/**************************************************************************//**
  * @brief Setup file writing thread
  *
  * @param p_init_args Takes a file name for opening.
  * @param p_object A dsp_node struct used to change various settings.
  *
  * @return 0 no error, non-zero indicates error.
  ****************************************************************************/
int init_callback_file_write(void *p_init_args, void *p_object);

/**************************************************************************//**
  * @brief Pthread function for threading
  *
  * @param p_data This will contain dsp_node struct so all data is available.
  *
  * @return NULL
  ****************************************************************************/
void* pthread_function_file_write(void *p_data);

/**************************************************************************//**
  * @brief Clean up all allocations from init_callback
  *
  * @param p_object file write dsp node object to free.
  *
  * @return 0 no error, non-zero indicates error.
  ****************************************************************************/
int free_callback_file_write(void *p_object);

#ifdef __cplusplus
}
#endif

#endif
