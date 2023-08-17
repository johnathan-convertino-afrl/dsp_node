//******************************************************************************
/// @file     ncurses_dsp_monitor.h
/// @author   Jay Convertino(johnathan.convertino.1@us.af.mil)
/// @date     2023.07.19
/// @brief    create windows in ncurses to display DSP node information
//******************************************************************************

#ifndef __ncurses_dsp_monitor
#define __ncurses_dsp_monitor

#include <curses.h>

#include "dsp_node_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @struct s_ncurses_dsp_monitor
 * @brief Contains data for ncurses nodes, such as dsp nodes.
 */
struct s_ncurses_dsp_monitor
{
  /**
   * @var s_ncurses_dsp_monitor:p_name
   * name of given to the node
   */
  char *p_name;
  /**
   * @var s_ncurses_dsp_monitor:node_number
   * number for particular node (used for window offset calculations).
   */
  unsigned int node_number;
  /**
   * @var s_ncurses_dsp_monitor::p_dsp_node
   * dsp node to monitor
   */
  struct s_dsp_node * p_dsp_node;
  /**
   * @var s_ncurses_dsp_monitor::win_thread;
   * thread for data window
   */
  pthread_t win_thread;
};

// THREAD FUNCTIONS //

/**************************************************************************//**
  * @brief create an monitor with an attached window for a particular dsp node
  *
  * @param p_dsp_node struct s_dsp_node object to monitor
  * @param p_name name of to display for the monitor
  *
  * @return allocated ncurses_dsp_monitor struct that needs nodes added.
  ****************************************************************************/
struct s_ncurses_dsp_monitor *ncurses_dsp_monitor_create(struct s_dsp_node * const p_dsp_node, char *p_name);

/**************************************************************************//**
  * @brief Start main monitor display, call this once after create.
  *
  * @return 0 no error, non-zero indicates error.
  ****************************************************************************/
int ncurses_dsp_monitor_start();

/**************************************************************************//**
  * @brief Start the ncurses window thread that shows throughput
  *
  * @param p_object struct s_ncurses_dsp_monitor object
  *
  * @return 0 no error, non-zero indicates error.
  ****************************************************************************/
int ncurses_dsp_monitor_throughput_start(struct s_ncurses_dsp_monitor * const p_object);

/**************************************************************************//**
  * @brief Wait for the pthread to finish
  *
  * @param p_object struct s_ncurses_dsp_monitor object
  *
  * @return 0 no error, non-zero indicates error.
  ****************************************************************************/
int ncurses_dsp_monitor_wait(struct s_ncurses_dsp_monitor const * const p_object);

/**************************************************************************//**
  * @brief Force the pthread to end
  *
  * @param p_object struct s_ncurses_dsp_monitor object
  *
  * @return 0 no error, non-zero indicates error.
  ****************************************************************************/
int ncurses_dsp_monitor_end(struct s_ncurses_dsp_monitor const * const p_object);

/**************************************************************************//**
  * @brief remove all allocations from create
  *
  * @param p_object struct s_ncurses_dsp_monitor object
  *
  * @return 0 no error, non-zero indicates error.
  ****************************************************************************/
void ncurses_dsp_monitor_cleanup(struct s_ncurses_dsp_monitor * p_object);

#ifdef __cplusplus
}
#endif

#endif
