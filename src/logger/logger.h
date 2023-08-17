//******************************************************************************
/// @file     logger.h
/// @author   Jay Convertino(johnathan.convertino.1@us.af.mil)
/// @date     2023.08.17
/// @brief    log all messages to a single file
//******************************************************************************

#ifndef __logger
#define __logger

#include "ringBuffer.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @struct s_logger
 * @brief Contains data for loggers, will log messages to specified file.
 */
struct s_logger
{
  /**
   * @var s_logger::fd
   * file descriptor
   */
  FILE *p_fd;
  /**
   * @var s_logger::p_file
   * file name and path
   */
  char *p_file;
  /**
   * @var s_logger::p_ringbuffer
   * ringbuffer that stores strings formatted by methods to be written by logger.
   */
  struct s_ringBuffer *p_ringbuffer;
};

/**************************************************************************//**
  * @brief Create logger
  *
  * @param p_file File name and path, log extension will be added.
  *
  * @return logger object
  ****************************************************************************/
struct s_logger *logger_create(char *p_file);

/**************************************************************************//**
  * @brief Write error messages, appends "ERROR: " at begining and newline at end.
  *
  * @param p_logger Logger object
  * @param p_string Message to write
  * @param ... va arg list (printf style)
  *
  * @return 0 no error, non-zero indicates error.
  ****************************************************************************/
int logger_error_msg(struct s_logger *p_logger, const char *p_string, ...);

/**************************************************************************//**
  * @brief Write warning messages, appends "WARNING: " at begining and newline at end.
  *
  * @param p_logger Logger object
  * @param p_string Message to write
  * @param ... va arg list (printf style)
  *
  * @return 0 no error, non-zero indicates error.
  ****************************************************************************/
int logger_warning_msg(struct s_logger *p_logger, const char *p_string, ...);

/**************************************************************************//**
  * @brief Write info messages, appends "INFO: " at begining and newline at end.
  *
  * @param p_logger Logger object
  * @param p_string Message to write
  * @param ... va arg list (printf style)
  *
  * @return 0 no error, non-zero indicates error.
  ****************************************************************************/
int logger_info_msg(struct s_logger *p_logger, const char *p_string, ...);

/**************************************************************************//**
  * @brief Cleanup logger and close file.
  *
  * @param p_logger Logger object
  *
  * @return 0 no error, non-zero indicates error.
  ****************************************************************************/
int logger_cleanup(struct s_logger *p_logger);

#ifdef __cplusplus
}
#endif

#endif
