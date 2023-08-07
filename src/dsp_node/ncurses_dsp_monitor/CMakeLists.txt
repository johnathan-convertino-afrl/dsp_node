################################################################################
### date      2023.07.19
### author    Jay Convertino
################################################################################

cmake_minimum_required(VERSION 3.14)

set(NCURSES_DSP_SRCS
  ncurses_dsp_monitor.c
  ncurses_dsp_monitor.h
)

include_directories(../ ../../kill_throbber/)

find_package(Curses REQUIRED)

add_library(ncurses_dsp_monitor ${NCURSES_DSP_SRCS})
target_link_libraries(ncurses_dsp_monitor PUBLIC ${CURSES_LIBRARY} Threads::Threads ${LIB_NAME_RINGBUFFER} )
target_compile_options(ncurses_dsp_monitor PRIVATE -Werror -Wall -Wextra -Wconversion -Wsign-conversion)
