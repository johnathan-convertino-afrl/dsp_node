################################################################################
### date      2023.06.26
### author    Jay Convertino
### brief     cmake for dsp_node
################################################################################

cmake_minimum_required(VERSION 3.14)

project(dsp_node)

set(REPO_PATH git_pulls)

if(!CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX "/usr/local/" CACHE PATH "..." FORCE)
endif()

add_subdirectory(src)
