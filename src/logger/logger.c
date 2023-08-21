//******************************************************************************
/// @file     logger.c
/// @author   Jay Convertino(johnathan.convertino.1@us.af.mil)
/// @date     2023.08.17
/// @brief    log all messages to a single file
//******************************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>

#include "logger.h"

#define BUF_SIZE 1 << 10
#define RD_SIZE  1 << 8

// pthread writer var
static pthread_t writer_thread;
// pthread writer func
void *pthread_file_writer(void *p_data);
// string writer function
int string_write(struct s_logger *p_logger, const char *p_append, const char *p_message, va_list arg);

// Create logger
struct s_logger *logger_create(char *p_file)
{
  int error = 0;

  char temp[256] = {"/0"};

  struct s_logger *p_temp = NULL;

  if(!p_file)
  {
    fprintf(stderr, "ERROR: file name is null\n");

    return NULL;
  }

  p_temp = malloc(sizeof(struct s_logger));

  if(!p_temp)
  {
    fprintf(stderr, "ERROR: allocation failed.\n");

    return NULL;
  }

  sprintf(temp, "%s.log", p_file);

  p_temp->p_fd = fopen(temp, "w");

  if(!p_temp->p_fd)
  {
    perror("ERROR: file open failed");

    goto error_fopen;
  }

  p_temp->p_file = strdup(temp);

  if(!p_temp->p_file)
  {
    perror("ERROR: file name duplication failed");

    goto error_strdup;
  }

  p_temp->p_ringbuffer = initRingBuffer(BUF_SIZE, 1);

  if(!p_temp->p_ringbuffer)
  {
    fprintf(stderr, "ERROR: ring buffer init failed.\n");

    goto error_ringbuffer;
  }

  error = pthread_create(&writer_thread, NULL, pthread_file_writer, p_temp);

  if(error)
  {
    perror("ERROR: Pthread create failed");

    goto error_cleanup_all;
  }

  return p_temp;

error_cleanup_all:
  freeRingBuffer(&p_temp->p_ringbuffer);

error_ringbuffer:
  free(p_temp->p_file);

error_strdup:
  fclose(p_temp->p_fd);

error_fopen:
  free(p_temp);

  return NULL;
}

// Write error messages, appends "ERROR: " at begining and newline at end.
int logger_error_msg(struct s_logger *p_logger, const char *p_string, ...)
{
  int error = 0;

  va_list cp_vlist;

  if(!p_logger) return ~0;

  va_start (cp_vlist, p_string);

  error = string_write(p_logger, "ERROR  ", p_string, cp_vlist);

  va_end(cp_vlist);

  return error;
}

// Write warning messages, appends "WARNING: " at begining and newline at end.
int logger_warning_msg(struct s_logger *p_logger, const char *p_string, ...)
{
  int error = 0;

  va_list cp_vlist;

  if(!p_logger) return ~0;

  va_start (cp_vlist, p_string);

  error = string_write(p_logger, "WARNING", p_string, cp_vlist);

  va_end(cp_vlist);

  return error;
}

// Write info messages, appends "INFO: " at begining and newline at end.
int logger_info_msg(struct s_logger *p_logger, const char *p_string, ...)
{
  int error = 0;

  va_list cp_vlist;

  if(!p_logger) return ~0;

  va_start (cp_vlist, p_string);

  error = string_write(p_logger, "INFO   ", p_string, cp_vlist);

  va_end(cp_vlist);

  return error;
}

// Cleanup logger and close file.
int logger_cleanup(struct s_logger *p_logger)
{
  if(!p_logger) return ~0;

  ringBufferEndBlocking(p_logger->p_ringbuffer);

  pthread_join(writer_thread, NULL);

  freeRingBuffer(&p_logger->p_ringbuffer);

  free(p_logger->p_file);

  fclose(p_logger->p_fd);

  free(p_logger);

  return 0;
}

// thread to write strings to file from buffer
void *pthread_file_writer(void *p_data)
{
  uint8_t *p_buffer = NULL;

  struct s_logger *p_logger = NULL;

  p_logger = (struct s_logger *)p_data;

  if(!p_logger)
  {
    fprintf(stderr, "ERROR: Logger thread data is null!\n");

    goto error_cleanup;
  }

  p_buffer = malloc(RD_SIZE);

  if(!p_buffer)
  {
    perror("ERROR: Failed to allocate buffer");

    goto error_cleanup;
  }

  do
  {
    unsigned long numElemRead = 0;
    unsigned long numElemWrote = 0;

    numElemRead = ringBufferBlockingRead(p_logger->p_ringbuffer, p_buffer, RD_SIZE, NULL);

    do
    {
      numElemWrote += fwrite(p_buffer + numElemWrote, 1, numElemRead, p_logger->p_fd);
    } while(numElemWrote < numElemRead);

    fflush(p_logger->p_fd);

  } while(ringBufferIsAlive(p_logger->p_ringbuffer));

error_cleanup:
  ringBufferEndBlocking(p_logger->p_ringbuffer);

  free(p_buffer);

  return NULL;
}

int string_write(struct s_logger *p_logger, const char *p_append, const char *p_message, va_list arg)
{
  char temp[256] = {"/0"};
  char vtemp[256] = {"/0"};

  if(!p_message) return ~0;

  if(strlen(p_message) > 240) return ~0;

  sprintf(temp, "%s :: %s\n", p_append, p_message);

  vsprintf(vtemp, temp, arg);

  ringBufferBlockingWrite(p_logger->p_ringbuffer, vtemp, strlen(vtemp), NULL);

  return 0;
}
