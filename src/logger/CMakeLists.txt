################################################################################
### date      2023.06.26
### author    Jay Convertino
################################################################################

cmake_minimum_required(VERSION 3.14)

set(LOG_NODE_SRCS
  logger.c
  logger.h
)

add_library(logger ${LOG_NODE_SRCS})
target_link_libraries(logger PUBLIC ${LIB_NAME_RINGBUFFER} Threads::Threads)
target_compile_options(logger PRIVATE -Werror -Wall -Wextra -Wconversion -Wsign-conversion)
