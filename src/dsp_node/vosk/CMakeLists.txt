################################################################################
### date      2023.06.26
### author    Jay Convertino
################################################################################

cmake_minimum_required(VERSION 3.14)

set(VOSK_FUNC_SRCS
  vosk_func.c
  vosk_func.h
)

set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)

include_directories(../ ../../kill_throbber/ ../../logger/)

set(LIB_NAME_KALDI kaldi)

set(FETCHCONTENT_BASE_DIR ${CMAKE_BINARY_DIR}/${REPO_PATH}/${LIB_NAME_KALDI})

FetchContent_Declare(
        ${LIB_NAME_KALDI}
        GIT_REPOSITORY https://github.com/kaldi-asr/kaldi
        GIT_PROGRESS   FALSE
        CMAKE_ARGS
          -DBUILD_SHARED_LIBS=OFF
)

FetchContent_MakeAvailable(${LIB_NAME_KALDI})

set(LIB_NAME_VOSK vosk)

set(FETCHCONTENT_BASE_DIR ${CMAKE_BINARY_DIR}/${REPO_PATH}/${LIB_NAME_VOSK})

FetchContent_Declare(
        ${LIB_NAME_VOSK}
        GIT_REPOSITORY https://github.com/alphacep/vosk-api
        GIT_TAG        v0.3.50
        GIT_PROGRESS   FALSE
        CMAKE_ARGS
          -DBUILD_SHARED_LIBS=OFF
          -DCMAKE_PREFIX_PATH=${CMAKE_BINARY_DIR}/${REPO_PATH}/${LIB_NAME_KALDI}
)

FetchContent_MakeAvailable(${LIB_NAME_VOSK})

add_library(vosk_func ${VOSK_FUNC_SRCS})
target_link_libraries(vosk_func PUBLIC ${LIB_NAME_RINGBUFFER} ${LIB_NAME_VOSK} Threads::Threads)
target_compile_options(vosk_func PRIVATE -Werror -Wall -Wextra -Wconversion -Wsign-conversion)
