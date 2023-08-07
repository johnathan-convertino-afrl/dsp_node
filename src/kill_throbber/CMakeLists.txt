################################################################################
### date      2023.06.26
### author    Jay Convertino
################################################################################

cmake_minimum_required(VERSION 3.14)

set(KILL_NODE_SRCS
  kill_throbber.c
  kill_throbber.h
)

add_library(kill_throbber ${KILL_NODE_SRCS})
target_link_libraries(kill_throbber PUBLIC Threads::Threads)
target_compile_options(kill_throbber PRIVATE -Werror -Wall -Wextra -Wconversion -Wsign-conversion)
