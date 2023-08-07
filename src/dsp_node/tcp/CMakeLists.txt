################################################################################
### date      2023.08.07
### author    Jay Convertino
################################################################################

cmake_minimum_required(VERSION 3.14)

set(TCP_FUNC_SRCS
  tcp_func.c
  tcp_func.h
)

include_directories(../ ../../kill_throbber/)

add_library(tcp_func ${TCP_FUNC_SRCS})
target_link_libraries(tcp_func PUBLIC ${LIB_NAME_RINGBUFFER} Threads::Threads)
target_compile_options(tcp_func PRIVATE -Werror -Wall -Wextra -Wconversion -Wsign-conversion)
