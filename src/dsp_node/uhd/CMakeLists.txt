################################################################################
### date      2023.06.30
### author    Jay Convertino
################################################################################

cmake_minimum_required(VERSION 3.14)

set(UHD_FUNC_SRCS
  uhd_func.c
  uhd_func.h
)

set(LIB_NAME_UHD uhd)

set(FETCHCONTENT_BASE_DIR ${CMAKE_BINARY_DIR}/${REPO_PATH}/${LIB_NAME_UHD})

FetchContent_Declare(
        ${LIB_NAME_UHD}
        GIT_REPOSITORY  https://github.com/EttusResearch/uhd.git
        GIT_TAG         v4.4.0.0
        GIT_PROGRESS    FALSE
)

include_directories( ${FETCHCONTENT_BASE_DIR}/${LIB_NAME_UHD}-src/host/include/ ${FETCHCONTENT_BASE_DIR}/${LIB_NAME_UHD}-build/host/include/ )
link_directories( ${FETCHCONTENT_BASE_DIR}/${LIB_NAME_UHD}-build/host/lib )

FetchContent_GetProperties(${LIB_NAME_UHD})

if (NOT ${LIB_NAME_UHD}_POPULATED)
    FetchContent_Populate(${LIB_NAME_UHD})

#     set(ENABLE_STATIC_LIBS ON CACHE BOOL "STATIC" FORCE) needs to be patched, then it wouldn't find the device... sooo weird.

    set(ENABLE_MAN_PAGES OFF CACHE BOOL "MAN_PAGE" FORCE)

    set(ENABLE_MANUAL OFF CACHE BOOL "MANUAL" FORCE)

    set(ENABLE_TESTS OFF CACHE BOOL "TESTS" FORCE)

    set(ENABLE_PYTHON_API OFF CACHE BOOL "PYTHON" FORCE)

    set(ENABLE_EXAMPLES OFF CACHE BOOL "EXAMPLES" FORCE)

    set(ENABLE_UTILS OFF CACHE BOOL "UTIL" FORCE)

    set(ENABLE_C_API ON CACHE BOOL "CAPI" FORCE)

    set(ENABLE_DOXYGEN OFF CACHE BOOL "DOX" FORCE)

    set(ENABLE_OCTOCLOCK ON CACHE BOOL "CLOCK" FORCE)

    set(UHDHOST_PKG ON CACHE BOOL "HOST" FORCE)

    add_subdirectory(${${LIB_NAME_UHD}_SOURCE_DIR}/host ${${LIB_NAME_UHD}_BINARY_DIR}/host)
endif ()

include_directories(../ ../../kill_throbber/)

add_library(uhd_func ${UHD_FUNC_SRCS})
target_link_libraries(uhd_func PUBLIC ${LIB_NAME_RINGBUFFER} ${LIB_NAME_UHD} Threads::Threads)
target_compile_options(uhd_func PRIVATE -Werror -Wall -Wextra -Wconversion -Wsign-conversion)
