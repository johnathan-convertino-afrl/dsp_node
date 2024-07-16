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

set(LIB_NAME_KALDI kaldi)

set(LIB_NAME_VOSK vosk)

#set(LIB_NAME_OPENFST openfst)

include_directories(../ ../../kill_throbber/ ../../logger/ ${CMAKE_BINARY_DIR}/${REPO_PATH}/${LIB_NAME_VOSK}/openfst-src/src/include)

#set(FETCHCONTENT_BASE_DIR ${CMAKE_BINARY_DIR}/${REPO_PATH}/${LIB_NAME_OPENFST})
#openfst version 1.7.2
#external build?

#FetchContent_Declare(
        #${LIB_NAME_OPENFST}
        #GIT_REPOSITORY  https://github.com/alphacep/openfst
        #GIT_PROGRESS    FALSE
        #CMAKE_ARGS      -DBUILD_SHARED_LIBS=${BUILD_SHARED_LIBS}
#)

 #https://github.com/alphacep/kaldi is the site they use.. whitch is an old version... great.

set(FETCHCONTENT_BASE_DIR ${CMAKE_BINARY_DIR}/${REPO_PATH}/${LIB_NAME_KALDI})

FetchContent_Declare(
        ${LIB_NAME_KALDI}
        GIT_REPOSITORY  https://github.com/alphacep/kaldi.git
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

set(CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR}/${REPO_PATH}/${LIB_NAME_KALDI}/kaldi-build/cmake)

FetchContent_MakeAvailable(${LIB_NAME_KALDI} ${LIB_NAME_VOSK})

add_library(vosk_func ${VOSK_FUNC_SRCS})
target_link_libraries(vosk_func PUBLIC ${LIB_NAME_RINGBUFFER} ${LIB_NAME_KALDI} ${LIB_NAME_VOSK} Threads::Threads)
target_compile_options(vosk_func PRIVATE -Werror -Wall -Wextra -Wconversion -Wsign-conversion)
