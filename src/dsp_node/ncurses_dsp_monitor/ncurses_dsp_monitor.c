//******************************************************************************
/// @file     ncurses_dsp_monitor.c
/// @author   Jay Convertino(johnathan.convertino.1@us.af.mil)
/// @date     2023.07.19
/// @brief    create windows in ncurses to display DSP node information
//******************************************************************************

// standard includes
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <wchar.h>
#include <locale.h>
#include <unistd.h>

#include "ncurses_dsp_monitor.h"
#include "kill_throbber.h"

#define THROBBER_COLORS 1
#define RED_TEXT 2

#define SAMPLE_RATE_HZ 30
#define SAMPLE_RATE_NS ((long)1000000000/SAMPLE_RATE_HZ)
#define AVG_SAMPLE_AMT 100

#define KILOBYTES ((long)1 << 10)
#define MEGABYTES ((long)1 << 20)
#define GIGABYTES ((long)1 << 30)
#define TERABYTES ((long)1 << 40)

#define DISPLAY_COL_SIZE  75
#define DISPLAY_ROW_SIZE  5
#define DISPLAY_COL_ONE   1
#define DISPLAY_COL_TWO   25
#define DISPLAY_COL_THREE 45

#define THROBBER_ROW_SIZE 3

#define HEADER_COL_SIZE 80
#define HEADER_ROW_SIZE 5

//case statements are readable with the enum type. use to pass around info for rate display.
enum e_scale_type {SCALE_BYTES, SCALE_KILOBYTES, SCALE_MEGABYTES, SCALE_GIGABYTES, SCALE_TERABYTES};

//globals
//contains the standard screen pointer on init, once populated don't call initscr again.
static WINDOW *gp_stdscr = NULL;

//throbber thread
static pthread_t throbber_thread;
// //resize thread
static pthread_t resize_thread;
// //display update thread
static pthread_t update_thread;

//mutex for ncurses
static pthread_mutex_t g_mutex;

//condition signal for throbber to allow other threads to process;
static pthread_cond_t g_refresh_condition;

//number of nodes created
static unsigned int g_node_number = 0;

//do we need a refresh (initial is YES)
static volatile sig_atomic_t g_need_refresh = 1;

//screen initializer and resize helper
void *init_screen_and_resize(void *p_data);

//has the screen been resized?
void handle_winch(int sig);

//function for displaying data from nodes on its window.
void *display_thread(void *p_data);

//function for displaying throbber, we are still alive!
void *display_throbber(void *p_data);

//thread to update displays
void *display_update(void *p_data);

//time difference in nano seconds
long int nano_second_time_diff(struct timespec previous, struct timespec current);

//data rate difference from previous sample.
unsigned long data_rate(unsigned long previous, unsigned long current);

//moving average method for filtering out noise in data rate.
unsigned long avg_rate(unsigned long *p_buffer, unsigned long current_bytes, unsigned long buf_len);

//scale factor for the current average rate, is it KBps, MBps?
enum e_scale_type scale_factor(unsigned long avg_rate, unsigned sample_rate);

//scale display so that the output data is 0 to 1023 for its scale (1 KBps instead of 1024 Bps)
unsigned long scale_rate(unsigned long avg_rate, unsigned sample_rate, enum e_scale_type scale);

//same as scale rate but gives the remainder for display.
unsigned long scale_rate_remainder(unsigned long avg_rate, unsigned sample_rate, enum e_scale_type scale);

//return scale string, basically whatever the scale has been set to, give us human readable string so the end user will know.
char *scale_string_sec(enum e_scale_type scale);

//return scale string, basically whatever the scale has been set to, give us human readable string so the end user will know.
char *scale_string(enum e_scale_type scale);

// THREAD FUNCTIONS //

//create an ncruses monitor window
struct s_ncurses_dsp_monitor *ncurses_dsp_monitor_create(struct s_dsp_node * const p_node, char *p_name)
{
  struct s_ncurses_dsp_monitor *p_temp = NULL;

  if(!p_node)
  {
    fprintf(stderr, "ERROR: node passed is null.\n");

    return NULL;
  }

  p_temp = malloc(sizeof(struct s_ncurses_dsp_monitor));

  if(!p_temp)
  {
    fprintf(stderr, "ERROR: malloc failed for ncurses dsp monitor.\n");

    return NULL;
  }

  g_node_number++;

  p_temp->node_number = g_node_number;

  p_temp->p_dsp_node = p_node;

  p_temp->p_name = strdup(p_name);

  return p_temp;
}

//start the main display
int ncurses_dsp_monitor_start()
{
  if(!g_node_number)
  {
    fprintf(stderr, "WARNING: No nodes created, only title and throbber created at this point!\n");
  }

  if(!gp_stdscr)
  {
    pthread_create(&update_thread, NULL, display_update, NULL);

    pthread_create(&resize_thread, NULL, init_screen_and_resize, NULL);

    while(g_need_refresh);

    if(!gp_stdscr) return ~0;

    pthread_create(&throbber_thread, NULL, display_throbber, NULL);
  }
  else
  {
    fprintf(stderr, "WARNING: Only call ncurses start once!\n");
  }

  return 0;
}

//Start the thread using pthread function passed to create.
int ncurses_dsp_monitor_throughput_start(struct s_ncurses_dsp_monitor * const p_object)
{
  if(!p_object)
  {
    fprintf(stderr, "ERROR: thread start object is null.\n");

    kill_thread = 1;

    return ~0;
  }

  return pthread_create(&p_object->win_thread, NULL, display_thread, p_object);
}

//Wait for the pthread to finish
int ncurses_dsp_monitor_wait(struct s_ncurses_dsp_monitor const * const p_object)
{
  if(!p_object)
  {
    fprintf(stderr, "ERROR: object is NULL for wait.\n");

    return ~0;
  }

  return pthread_join(p_object->win_thread, NULL);
}

//Force the pthread to end
int ncurses_dsp_monitor_end(struct s_ncurses_dsp_monitor const * const p_object)
{
  if(!p_object)
  {
    fprintf(stderr, "ERROR: object is NULL for end.\n");

    return ~0;
  }

  return pthread_kill(p_object->win_thread, SIGUSR1);
}

//remove all allocations from create
void ncurses_dsp_monitor_cleanup(struct s_ncurses_dsp_monitor * p_object)
{
  if(!p_object)
  {
    fprintf(stderr, "ERROR: cleanup object is null.\n");

    return;
  }

  g_node_number--;

//   free(p_object->p_name);

  free(p_object);
}

void *init_screen_and_resize(void *p_data)
{
  int X = 0;
  int Y = 0;

  (void)X;
  (void)p_data;

  do
  {
    if(g_need_refresh)
    {
      pthread_mutex_lock(&g_mutex);

      pthread_cond_wait(&g_refresh_condition, &g_mutex);

      endwin();

      gp_stdscr = initscr();

      wclear(gp_stdscr);

      curs_set(0);

      if(!has_colors())
      {
        fprintf(stderr, "ERROR: Colors not supported by terminal.\n");

        kill_thread = 1;

        pthread_mutex_unlock(&g_mutex);

        continue;
      }

      getmaxyx(gp_stdscr, X ,Y);

      if(Y < HEADER_COL_SIZE)
      {
        fprintf(stderr, "ERROR: Terminal size is too small COL: %d %d\n", HEADER_COL_SIZE, Y);

        kill_thread = 1;

        pthread_mutex_unlock(&g_mutex);

        continue;
      }

      if((unsigned)X < (g_node_number * DISPLAY_ROW_SIZE + HEADER_ROW_SIZE + THROBBER_ROW_SIZE))
      {
        fprintf(stderr, "ERROR: Terminal size is too small ROW: %d %d\n", g_node_number * DISPLAY_ROW_SIZE + HEADER_ROW_SIZE + THROBBER_ROW_SIZE, X);

        kill_thread = 1;

        pthread_mutex_unlock(&g_mutex);

        continue;
      }

      signal(SIGWINCH, handle_winch);

      start_color();

      init_pair(THROBBER_COLORS, COLOR_CYAN, COLOR_CYAN);

      init_pair(RED_TEXT, COLOR_RED, COLOR_BLACK);

      touchwin(gp_stdscr);

      box(gp_stdscr, '*', '*');

      wattron(gp_stdscr, COLOR_PAIR(RED_TEXT));

      mvwaddstr(gp_stdscr, 2, 2, "DSP Node Monitor");

      mvwaddstr(gp_stdscr, 3, 2, "INFO: Press CTRL+C to quit.");

      wattroff(gp_stdscr, COLOR_PAIR(RED_TEXT));

      wrefresh(gp_stdscr);

      g_need_refresh = 0;

      pthread_mutex_unlock(&g_mutex);
    }
  } while(!kill_thread);

  use_default_colors();

  endwin();

  gp_stdscr = NULL;

  g_need_refresh = 0;

  signal(SIGWINCH, SIG_IGN);

  pthread_join(update_thread, NULL);

  pthread_join(throbber_thread, NULL);

  return NULL;
}

void handle_winch(int sig)
{
  (void)sig;

  signal(SIGWINCH, SIG_IGN);

  g_need_refresh = 1;

  signal(SIGWINCH, handle_winch);
}

//function for displaying data from nodes on its window.
void *display_thread(void *p_data)
{
  unsigned long previous_total_bytes = 0;
  unsigned long diff_total_bytes = 0;
  unsigned long avg_bytes = 1;
  unsigned long max_bytes = 0;

  unsigned long mov_avg_array[AVG_SAMPLE_AMT] = {0};

  enum e_scale_type scale_result = SCALE_BYTES;
  enum e_scale_type max_scale_result = SCALE_BYTES;
  enum e_scale_type scale_total_result = SCALE_BYTES;

  struct s_ncurses_dsp_monitor *p_object;

  WINDOW *p_window = NULL;

  p_object = (struct s_ncurses_dsp_monitor *)p_data;

  if(!p_object)
  {
    fprintf(stderr, "ERROR: Thread data is null.\n");

    kill_thread = 1;

    return NULL;
  }

  p_window = newwin(DISPLAY_ROW_SIZE, DISPLAY_COL_SIZE, (int)((p_object->node_number-1) * DISPLAY_ROW_SIZE + HEADER_ROW_SIZE + THROBBER_ROW_SIZE), 1);

  if(!p_window)
  {
    fprintf(stderr, "ERROR: newwin failed to create window.\n");

    kill_thread = 1;

    return NULL;
  }

  do
  {
    diff_total_bytes = data_rate(previous_total_bytes, p_object->p_dsp_node->total_bytes_processed);

    previous_total_bytes = p_object->p_dsp_node->total_bytes_processed;

    avg_bytes = avg_rate(mov_avg_array, diff_total_bytes, AVG_SAMPLE_AMT);

    scale_result = scale_factor(avg_bytes, SAMPLE_RATE_HZ);

    scale_total_result = scale_factor(previous_total_bytes, 1);

    if(avg_bytes > max_bytes)
    {
      max_bytes = avg_bytes;
      max_scale_result = scale_result;
    }

    pthread_mutex_lock(&g_mutex);

    pthread_cond_wait(&g_refresh_condition, &g_mutex);

    box(p_window, '|', '-');

    wattron(p_window, COLOR_PAIR(RED_TEXT));

    mvwaddstr(p_window, 0, (int)(DISPLAY_COL_SIZE/2-strlen(p_object->p_name)/2), p_object->p_name);

    wattroff(p_window, COLOR_PAIR(RED_TEXT));

    wmove(p_window, 1, DISPLAY_COL_ONE);

    wprintw(p_window, "DRATE: %6ld.%02ld %s", scale_rate(avg_bytes, SAMPLE_RATE_HZ, scale_result), scale_rate_remainder(avg_bytes, SAMPLE_RATE_HZ, scale_result), scale_string_sec(scale_result));

    wmove(p_window, 2, DISPLAY_COL_ONE);

    wprintw(p_window, "DMAX : %6ld.%02ld %s", scale_rate(max_bytes, SAMPLE_RATE_HZ, max_scale_result), scale_rate_remainder(max_bytes, SAMPLE_RATE_HZ, max_scale_result), scale_string_sec(max_scale_result));

    wmove(p_window, 3, DISPLAY_COL_ONE);

    wprintw(p_window, "DPROC: %6ld.%02ld %s", scale_rate(previous_total_bytes, 1, scale_total_result), scale_rate_remainder(previous_total_bytes, 1, scale_total_result), scale_string(scale_total_result));

    wmove(p_window, 1, DISPLAY_COL_TWO);

    wprintw(p_window, "Type Size In : %3d Bytes", p_object->p_dsp_node->input_type_size);

    wmove(p_window, 2, DISPLAY_COL_TWO);

    wprintw(p_window, "Type Size Out: %3d Bytes",  p_object->p_dsp_node->output_type_size);

    wnoutrefresh(p_window);

    pthread_mutex_unlock(&g_mutex);

  } while(!kill_thread);

  delwin(p_window);

  return NULL;
}

//function for displaying throbber
void *display_throbber(void *p_data)
{
  int index = 1;
  //throbber window
  WINDOW *th_window = NULL;

  (void)p_data;

  th_window = newwin(THROBBER_ROW_SIZE, DISPLAY_COL_SIZE, 5, 1);

  if(!th_window)
  {
    fprintf(stderr, "ERROR: newwin failed to create window.\n");

    kill_thread = 1;

    return NULL;
  }

  do
  {
    pthread_mutex_lock(&g_mutex);

    pthread_cond_wait(&g_refresh_condition, &g_mutex);

    box(th_window, '|', '-');

    wattron(th_window, COLOR_PAIR(THROBBER_COLORS));

    mvwaddch(th_window, 1, index, '*');

    wattroff(th_window, COLOR_PAIR(THROBBER_COLORS));

    if(index > 1)
    {
      mvwaddch(th_window, 1, index-1, ' ');
    }
    else
    {
      mvwaddch(th_window, 1, DISPLAY_COL_SIZE-2, ' ');
    }

    wnoutrefresh(th_window);

    pthread_mutex_unlock(&g_mutex);

    index %= DISPLAY_COL_SIZE-2;

    index++;

  } while(!kill_thread);

  delwin(th_window);

  return NULL;
}

//thread to update displays
void *display_update(void *p_data)
{
  long int diff = 0;

  struct timespec current;
  struct timespec previous;

  (void)p_data;

  clock_gettime(CLOCK_MONOTONIC, &previous);

  do
  {
    clock_gettime(CLOCK_MONOTONIC, &current);

    diff = nano_second_time_diff(previous, current);

    if(diff < SAMPLE_RATE_NS) continue;

    pthread_mutex_lock(&g_mutex);

    previous = current;

    doupdate();

    pthread_cond_broadcast(&g_refresh_condition);

    pthread_mutex_unlock(&g_mutex);

  } while(!kill_thread);

  pthread_cond_broadcast(&g_refresh_condition);

  return NULL;

}

//time difference
long int nano_second_time_diff(struct timespec previous, struct timespec current)
{
  if((current.tv_nsec - previous.tv_nsec) < 0)
  {
    return (~((long int)0) >> 1) - current.tv_nsec + previous.tv_nsec;
  }

  return current.tv_nsec - previous.tv_nsec;
}

//calculate number of bytes since previous
unsigned long data_rate(unsigned long previous, unsigned long current)
{
  if(current < previous)
  {
    return ~(unsigned long)0 - previous + current;
  }

  return current - previous;
}

//moving average method for filtering out noise in data rate.
unsigned long avg_rate(unsigned long *p_buffer, unsigned long current_bytes, unsigned long buf_len)
{
  unsigned long int index = 0;

  unsigned long temp = 0;

  if(!p_buffer) return 0;

  for(index = 0; index < buf_len; index++)
  {
    if(index < buf_len-1)
    {
      p_buffer[index] = p_buffer[index+1];
    }
    else
    {
      p_buffer[index] = current_bytes;
    }

    temp += p_buffer[index];
  }

  return temp/buf_len;
}

//scale factor
enum e_scale_type scale_factor(unsigned long avg_rate, unsigned sample_rate)
{
  unsigned long temp = avg_rate * sample_rate;

  if(temp > TERABYTES) return SCALE_TERABYTES;

  if(temp > GIGABYTES) return SCALE_GIGABYTES;

  if(temp > MEGABYTES) return SCALE_MEGABYTES;

  if(temp > KILOBYTES) return SCALE_KILOBYTES;

  return SCALE_BYTES;
}

//scale display
unsigned long scale_rate(unsigned long avg_rate, unsigned sample_rate, enum e_scale_type scale)
{
  switch(scale)
  {
    case(SCALE_BYTES):
      return avg_rate * sample_rate;
    case(SCALE_KILOBYTES):
      return (avg_rate * sample_rate)/KILOBYTES;
    case(SCALE_MEGABYTES):
      return (avg_rate * sample_rate)/MEGABYTES;
    case(SCALE_GIGABYTES):
      return (avg_rate * sample_rate)/GIGABYTES;
    case(SCALE_TERABYTES):
      return (avg_rate * sample_rate)/TERABYTES;
    default:
      break;
  }

  return 0;
}

//scale display remainder
unsigned long scale_rate_remainder(unsigned long avg_rate, unsigned sample_rate, enum e_scale_type scale)
{
  //idea is its a fraction of the scale factor the result will be .34999 something, it then gets scaled
  //by 100 to get 34 for integer printing.
  switch(scale)
  {
    case(SCALE_BYTES):
      return 0;
    case(SCALE_KILOBYTES):
      return ((avg_rate * sample_rate)%KILOBYTES*100)/KILOBYTES;
    case(SCALE_MEGABYTES):
      return ((avg_rate * sample_rate)%MEGABYTES*100)/MEGABYTES;
    case(SCALE_GIGABYTES):
      return ((avg_rate * sample_rate)%GIGABYTES*100)/GIGABYTES;
    case(SCALE_TERABYTES):
      return ((avg_rate * sample_rate)%TERABYTES*100)/TERABYTES;
    default:
      break;
  }

  return 0;
}

//string for us people to read
char *scale_string_sec(enum e_scale_type scale)
{
  switch(scale)
  {
    case(SCALE_BYTES):
      return "Bps ";
    case(SCALE_KILOBYTES):
      return "KBps";
    case(SCALE_MEGABYTES):
      return "MBps";
    case(SCALE_GIGABYTES):
      return "GBps";
    case(SCALE_TERABYTES):
      return "TBps";
    default:
      break;
  }
  return "????";
}

//string for us people to read
char *scale_string(enum e_scale_type scale)
{
  switch(scale)
  {
    case(SCALE_BYTES):
      return "B ";
    case(SCALE_KILOBYTES):
      return "KB";
    case(SCALE_MEGABYTES):
      return "MB";
    case(SCALE_GIGABYTES):
      return "GB";
    case(SCALE_TERABYTES):
      return "TB";
    default:
      break;
  }
  return "??";
}
