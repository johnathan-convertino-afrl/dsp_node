################################################################################
### date      2023.06.26
### author    Jay Convertino
################################################################################

cmake_minimum_required(VERSION 3.14)

set(ALSA_FUNC_SRCS
  alsa_func.c
  alsa_func.h
)

include_directories(../ ../../kill_throbber/ ../../logger/)

find_package(ALSA)

if(ALSA_FOUND)
  add_library(alsa_func ${ALSA_FUNC_SRCS})
  target_link_libraries(alsa_func PUBLIC ${LIB_NAME_RINGBUFFER} ALSA::ALSA Threads::Threads)
  target_compile_options(alsa_func PRIVATE -Werror -Wall -Wextra -Wconversion -Wsign-conversion)
else()
  message(STATUS "ALSA library not found, not building ALSA FUNC")
endif()
