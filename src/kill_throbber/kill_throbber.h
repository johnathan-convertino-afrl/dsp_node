//******************************************************************************
/// @file     kill_throbber.h
/// @author   Jay Convertino(johnathan.convertino.1@us.af.mil)
/// @date     2023.06.26
/// @brief    DSP node has access to kill_thread to all ctrl+c to terminate app.
//******************************************************************************

#ifndef __kill_throbber
#define __kill_throbber

#include <signal.h>

#ifdef __cplusplus
extern "C" {
#endif

// atomic variable for thread kill, declared.
extern volatile sig_atomic_t kill_thread;

/**************************************************************************//**
  * @brief Create the throbber and setup the sigterm function.
  ****************************************************************************/
void kill_throbber_create();

/**************************************************************************//**
  * @brief Start the thread that checks if ctrl+c is pressed and prints throbber.
  *
  * @return The result of pthread_create.
  ****************************************************************************/
int kill_throbber_start();

/**************************************************************************//**
  * @brief Wait for the throbber thread to finish.
  *
  * @return The result of pthread join
  ****************************************************************************/
int kill_throbber_wait();

/**************************************************************************//**
  * @brief Set kill_thread to 1
  *
  * @return The result of of setting kill_thread.
  ****************************************************************************/
int kill_throbber_end();

/**************************************************************************//**
  * @brief Kill the throbber thread
  *
  * @return The result of pthread kill
  ****************************************************************************/
int kill_throbber_kill();

#ifdef __cplusplus
}
#endif

#endif
