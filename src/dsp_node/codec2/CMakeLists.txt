################################################################################
### date      2023.06.29
### author    Jay Convertino
################################################################################

cmake_minimum_required(VERSION 3.14)

set(CODEC2_FUNC_SRCS
  codec2_func.c
  codec2_func.h
)

set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)

include_directories(../ ../../kill_throbber/)

set(LIB_NAME_CODEC2 codec2)

set(FETCHCONTENT_BASE_DIR ${CMAKE_BINARY_DIR}/${REPO_PATH}/${LIB_NAME_CODEC2})

set(CMAKE_REQUIRED_LIBRARIES "m")

FetchContent_Declare(
        ${LIB_NAME_CODEC2}
        GIT_REPOSITORY https://github.com/drowe67/codec2-dev
        GIT_TAG        v1.0.5
        GIT_PROGRESS   FALSE
        CMAKE_ARGS     -DBUILD_SHARED_LIBS=OFF
)

FetchContent_MakeAvailable(${LIB_NAME_CODEC2})

add_library(codec2_func ${CODEC2_FUNC_SRCS})
target_link_libraries(codec2_func PUBLIC ${LIB_NAME_RINGBUFFER} ${LIB_NAME_CODEC2} Threads::Threads)
target_compile_options(codec2_func PRIVATE -Werror -Wall -Wextra -Wconversion -Wsign-conversion)
