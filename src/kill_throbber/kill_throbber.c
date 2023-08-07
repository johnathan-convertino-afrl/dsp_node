//******************************************************************************
/// @file     kill_throbber.c
/// @author   Jay Convertino(johnathan.convertino.1@us.af.mil)
/// @date     2023.06.26
/// @brief    DSP node has access to kill_thread to all ctrl+c to terminate app.
//******************************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

#include "kill_throbber.h"

//define kill_thread
volatile sig_atomic_t kill_thread = 0;

//global pthread type for throbber
static pthread_t throbber_thread;

//signal handler function
static void sig_handler(int data);

//throbber function
void *throbber(void *data);

//create ctrl+c handler
void kill_throbber_create()
{
  // signal handler
  printf("\nINFO: Press CTRL+C to quit.\n");
  signal(SIGINT, sig_handler);
}

//start the throbber thread
int kill_throbber_start()
{
  return pthread_create(&throbber_thread, NULL, throbber, NULL);
}

//wait for throbber to end
int kill_throbber_wait()
{
  return pthread_join(throbber_thread, NULL);
}

//send kill signal via kill_thread
int kill_throbber_end()
{
  return kill_thread = 1;
}

//tell thread to quit(kill)
int kill_throbber_kill()
{
  return pthread_kill(throbber_thread, SIGUSR1);
}

//signal handler function, prints info when caught.
static void sig_handler(int data)
{
    (void) data;

    printf("\nINFO: CTRL+C Caught\n");
    kill_thread = 1;

    signal(SIGINT, SIG_IGN);
}

//Print a spinning throbber to the screen.
void *throbber(void *data)
{
  (void)data;

  long unsigned int index = 0;

  char throbber[] = "\\|/-";

  const struct timespec sleep_time =
  {
    .tv_sec = 0,
    .tv_nsec = 100000000
  };

  sleep(1);

  //stop flashing cursor
  printf("\033[?25l");

  do
  {
    printf("\r%c", throbber[index]);

    fflush(stdout);

    index = (index+1)%(sizeof(throbber)-1);

    nanosleep(&sleep_time, NULL);

  } while(!kill_thread);

  //set flashing cursor
  printf("\033[?25h");

  printf("\nINFO: Application Shutting down.\n");

  return NULL;
}
