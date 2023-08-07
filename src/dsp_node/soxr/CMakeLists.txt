################################################################################
### date      2023.06.29
### author    Jay Convertino
################################################################################

cmake_minimum_required(VERSION 3.14)

set(SOXR_FUNC_SRCS
  soxr_func.c
  soxr_func.h
)

set(CMAKE_POLICY_DEFAULT_CMP0075 NEW)
set(CMAKE_POLICY_DEFAULT_CMP0115 OLD)
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
set(CMAKE_POLICY_DEFAULT_CMP0127 OLD)

set(LIB_NAME_SOXR soxr)

set(FETCHCONTENT_BASE_DIR ${CMAKE_BINARY_DIR}/${REPO_PATH}/${LIB_NAME_SOXR})

FetchContent_Declare(
        ${LIB_NAME_SOXR}
        GIT_REPOSITORY  https://git.code.sf.net/p/soxr/code
        GIT_TAG         master
        GIT_PROGRESS    FALSE
        CMAKE_ARGS      -DBUILD_SHARED_LIBS=OFF -Wno-dev
        PATCH_COMMAND  git apply --ignore-space-change --ignore-whitespace ${CMAKE_CURRENT_SOURCE_DIR}/patch/soxr_cmake.patch || true
        GIT_REMOTE_UPDATE_STRATEGY REBASE_CHECKOUT
)

FetchContent_MakeAvailable(${LIB_NAME_SOXR})

include_directories( ${FETCHCONTENT_BASE_DIR}/${LIB_NAME_SOXR}-src/src/ ../ ../../kill_throbber/)

find_package(OpenMP REQUIRED)

add_library(soxr_func ${SOXR_FUNC_SRCS})
target_link_libraries(soxr_func PUBLIC ${LIB_NAME_RINGBUFFER} ${LIB_NAME_SOXR} Threads::Threads OpenMP::OpenMP_C)
target_compile_options(soxr_func PRIVATE -Werror -Wall -Wextra -Wconversion -Wsign-conversion)
