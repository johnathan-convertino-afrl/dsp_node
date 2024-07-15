################################################################################
### date      2023.06.26
### author    Jay Convertino
################################################################################

cmake_minimum_required(VERSION 3.14)

#set (DSP_NODE_VERSION_MAJOR 0)
#set (DSP_NODE_VERSION_MINOR 5)
#set (DSP_NODE_VERSION_PATCH 0)

#configure_file("dsp_version.h.in" "dsp_version.h")

set(DSP_NODE_SRCS
  dsp_node.c
  dsp_node.h
  dsp_node_types.h
)

add_library(dsp_node ${DSP_NODE_SRCS})
target_link_libraries(dsp_node PUBLIC ${LIB_NAME_RINGBUFFER} Threads::Threads)
target_compile_options(dsp_node PRIVATE -Werror -Wall -Wextra -Wconversion -Wsign-conversion)

include_directories(../logger/)

if(BUILD_EXAMPLES OR BUILD_LIB_FILE)
  add_subdirectory(file)
endif()

if(BUILD_NCURSES_VERSIONS)
  add_subdirectory(ncurses_dsp_monitor)
endif()

if(BUILD_ALSA_EXAMPLES OR BUILD_LIB_ALSA)
  add_subdirectory(alsa)
endif()

if(BUILD_CODEC2_EXAMPLES OR BUILD_LIB_CODEC2)
  add_subdirectory(codec2)
endif()

if(BUILD_SOXR_EXAMPLES OR BUILD_LIB_SOXR)
  add_subdirectory(soxr)
endif()

if(BUILD_UHD_EXAMPLES OR BUILD_LIB_UHD)
  add_subdirectory(uhd)
endif()

if(BUILD_TCP_EXAMPLES OR BUILD_LIB_TCP_SERVER)
  add_subdirectory(tcp_server)
endif()

if(BUILD_VOSK_EXAMPLES OR BUILD_LIB_VOSK)
  add_subdirectory(vosk)
endif()
