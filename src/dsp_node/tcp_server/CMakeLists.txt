################################################################################
### date      2023.08.07
### author    Jay Convertino
################################################################################

cmake_minimum_required(VERSION 3.14)

set(TCP_SERVER_FUNC_SRCS
  tcp_server_func.c
  tcp_server_func.h
)

include_directories(../ ../../kill_throbber/ ../../logger/)

add_library(tcp_server_func ${TCP_SERVER_FUNC_SRCS})
target_link_libraries(tcp_server_func PUBLIC ${LIB_NAME_RINGBUFFER} Threads::Threads)
target_compile_options(tcp_server_func PRIVATE -Werror -Wall -Wextra -Wconversion -Wsign-conversion)
