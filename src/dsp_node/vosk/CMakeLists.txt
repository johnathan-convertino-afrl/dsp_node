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

set(LIB_NAME_KALDI kaldi-base)

set(LIB_NAME_VOSK vosk)

set(LIB_NAME_OPENFST openfst)

include_directories(../ ../../kill_throbber/ ../../logger/ ${CMAKE_BINARY_DIR}/${REPO_PATH}/${LIB_NAME_KALDI}/kaldi-base-src/tools/openfst/include ${CMAKE_BINARY_DIR}/${REPO_PATH}/${LIB_NAME_VOSK}/vosk-src/src)

link_directories(${CMAKE_BINARY_DIR}/${REPO_PATH}/${LIB_NAME_KALDI}/kaldi-base-src/tools/openfst/lib)

set(FETCHCONTENT_BASE_DIR ${CMAKE_BINARY_DIR}/${REPO_PATH}/${LIB_NAME_KALDI})

FetchContent_Declare(
        ${LIB_NAME_KALDI}
        GIT_REPOSITORY  https://github.com/johnathan-convertino-afrl/kaldi
        GIT_TAG         vosk
        GIT_PROGRESS    FALSE
        CMAKE_ARGS      -DBUILD_SHARED_LIBS=${BUILD_SHARED_LIBS}
)

set(FETCHCONTENT_BASE_DIR ${CMAKE_BINARY_DIR}/${REPO_PATH}/${LIB_NAME_VOSK})

FetchContent_Declare(
        ${LIB_NAME_VOSK}
        GIT_REPOSITORY  https://github.com/alphacep/vosk-api
        GIT_TAG         v0.3.50
        GIT_PROGRESS    FALSE
        CMAKE_ARGS      -DBUILD_SHARED_LIBS=${BUILD_SHARED_LIBS}
)

set(CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR}/${REPO_PATH}/${LIB_NAME_KALDI}/kaldi-base-build/cmake)

FetchContent_MakeAvailable(${LIB_NAME_KALDI} ${LIB_NAME_VOSK})

add_library(vosk_func ${VOSK_FUNC_SRCS})
target_link_libraries(vosk_func PUBLIC ${LIB_NAME_RINGBUFFER} ${LIB_NAME_VOSK} Threads::Threads)
target_link_directories(vosk_func PUBLIC ${CMAKE_BINARY_DIR}/${REPO_PATH}/${LIB_NAME_KALDI}/kaldi-base-src/tools/openfst/lib)
target_compile_options(vosk_func PRIVATE)
