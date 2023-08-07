################################################################################
### date      2023.06.26
### author    Jay Convertino
################################################################################

cmake_minimum_required(VERSION 3.14)

if(NOT DEFINED BUILD_EXAMPLES_ALL)
  set(BUILD_EXAMPLES_ALL OFF)
else()
  set(BUILD_ALSA_EXAMPLES ON)
  set(BUILD_CODEC2_EXAMPLES ON)
  set(BUILD_UHD_EXAMPLES ON)
  set(BUILD_SOXR_EXAMPLES ON)
  set(BUILD_EXAMPLES_ALL ON)
  set(BUILD_NCURSES_VERSIONS ON)
endif()

if(NOT DEFINED BUILD_ALSA_EXAMPLES)
  set(BUILD_ALSA_EXAMPLES OFF)
endif()

if(NOT DEFINED BUILD_CODEC2_EXAMPLES)
  set(BUILD_CODEC2_EXAMPLES OFF)
endif()

if(NOT DEFINED BUILD_UHD_EXAMPLES)
  set(BUILD_UHD_EXAMPLES OFF)
endif()

if(NOT DEFINED BUILD_SOXR_EXAMPLES)
  set(BUILD_SOXR_EXAMPLES OFF)
endif()

if(NOT DEFINED BUILD_NCURSES_VERSIONS)
  set(BUILD_NCURSES_VERSIONS OFF)
endif()

if(BUILD_SOXR_EXAMPLES OR BUILD_ALSA_EXAMPLES OR BUILD_CODEC2_EXAMPLES OR BUILD_UHD_EXAMPLES)
  set(BUILD_EXAMPLES ON)
endif()

if(NOT DEFINED BUILD_EXAMPLES)
  set(BUILD_EXAMPLES OFF)
endif()

if(NOT DEFINED CREATE_DOXYGEN)
  set(CREATE_DOXYGEN OFF)
endif()


include(FetchContent)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "STATIC" FORCE)

set(REPO_PATH git_pulls)

set(LIB_NAME_RINGBUFFER ringBuffer)

set(FETCHCONTENT_BASE_DIR ${CMAKE_BINARY_DIR}/${REPO_PATH}/${LIB_NAME_RINGBUFFER})

FetchContent_Declare(
        ${LIB_NAME_RINGBUFFER}
        GIT_REPOSITORY  https://github.com/sparkletron/C89_pthread_ring_buffer.git
        GIT_TAG         release_v1.6.1
        GIT_PROGRESS    FALSE
        CMAKE_ARGS      -DBUILD_SHARED_LIBS=OFF
)

FetchContent_MakeAvailable(${LIB_NAME_RINGBUFFER})

#set(THREADS_PREFER_PTHREAD_FLAG ON)

find_package(Threads REQUIRED)

add_subdirectory(dsp_node)
add_subdirectory(kill_throbber)

if(CREATE_DOXYGEN)
  find_package(Doxygen REQUIRED dot OPTIONAL_COMPONENTS mscgen dia)

  set(DOXYGEN_GENERATE_HTML YES)
  set(DOXYGEN_GENERATE_MAN NO)
  set(DOXYGEN_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/doxygen)
  set(DOXYGEN_PROJECT_NAME DSP_NODE)
  set(DOXYGEN_OPTIMIZE_OUTPUT_FOR_C YES)
  set(DOXYGEN_QUIET YES)
  set(DOXYGEN_WARN_IF_UNDOCUMENTED NO)

  if(DOXYGEN_FOUND)
    doxygen_add_docs(doxygen ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_BINARY_DIR}/${REPO_PATH}/${LIB_NAME_RINGBUFFER} ALL)
  endif()

endif()

if(BUILD_EXAMPLES)
  add_subdirectory(apps)
endif()
