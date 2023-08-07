################################################################################
### date      2023.06.26
### author    Jay Convertino
### brief     dsp_node examples
################################################################################

cmake_minimum_required(VERSION 3.14)

include_directories(../dsp_node/ ../kill_throbber)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/apps)

add_executable(file_to_file file_to_file.c)
target_link_libraries(file_to_file PRIVATE ${LIB_NAME_RINGBUFFER} dsp_node file_func kill_throbber)
target_compile_options(file_to_file PRIVATE -Werror -Wall -Wextra -Wconversion -Wsign-conversion)
install(TARGETS file_to_file DESTINATION bin)

if(BUILD_ALSA_EXAMPLES)
  find_package(ALSA)

  if(ALSA_FOUND)
    add_executable(alsa_to_file alsa_to_file.c)
    target_link_libraries(alsa_to_file PRIVATE ${LIB_NAME_RINGBUFFER} dsp_node file_func kill_throbber alsa_func)
    target_compile_options(alsa_to_file PRIVATE -Werror -Wall -Wextra -Wconversion -Wsign-conversion)
    install(TARGETS alsa_to_file DESTINATION bin)

    add_executable(file_to_alsa file_to_alsa.c)
    target_link_libraries(file_to_alsa PRIVATE ${LIB_NAME_RINGBUFFER} dsp_node file_func kill_throbber alsa_func)
    target_compile_options(file_to_alsa PRIVATE -Werror -Wall -Wextra -Wconversion -Wsign-conversion)
    install(TARGETS file_to_alsa DESTINATION bin)
  endif()
endif()

if(BUILD_ALSA_EXAMPLES AND BUILD_CODEC2_EXAMPLES)
  if(ALSA_FOUND)
    add_executable(codec2_demod_alsa_to_file codec2_demod_alsa_to_file.c)
    target_link_libraries(codec2_demod_alsa_to_file PRIVATE ${LIB_NAME_RINGBUFFER} dsp_node file_func kill_throbber alsa_func codec2_func)
    target_compile_options(codec2_demod_alsa_to_file PRIVATE -Werror -Wall -Wextra -Wconversion -Wsign-conversion)
    install(TARGETS codec2_demod_alsa_to_file DESTINATION bin)

    add_executable(codec2_mod_file_to_alsa codec2_mod_file_to_alsa.c)
    target_link_libraries(codec2_mod_file_to_alsa PRIVATE ${LIB_NAME_RINGBUFFER} dsp_node file_func kill_throbber alsa_func codec2_func)
    target_compile_options(codec2_mod_file_to_alsa PRIVATE -Werror -Wall -Wextra -Wconversion -Wsign-conversion)
    install(TARGETS codec2_mod_file_to_alsa DESTINATION bin)
  endif()
endif()

if(BUILD_CODEC2_EXAMPLES)
  add_executable(codec2_mod_file_comp codec2_mod_file_comp.c)
  target_link_libraries(codec2_mod_file_comp PRIVATE ${LIB_NAME_RINGBUFFER} dsp_node file_func kill_throbber codec2_func)
  target_compile_options(codec2_mod_file_comp PRIVATE -Werror -Wall -Wextra -Wconversion -Wsign-conversion)
  install(TARGETS codec2_mod_file_comp DESTINATION bin)

  add_executable(codec2_demod_file_comp codec2_demod_file_comp.c)
  target_link_libraries(codec2_demod_file_comp PRIVATE ${LIB_NAME_RINGBUFFER} dsp_node file_func kill_throbber codec2_func)
  target_compile_options(codec2_demod_file_comp PRIVATE -Werror -Wall -Wextra -Wconversion -Wsign-conversion)
  install(TARGETS codec2_demod_file_comp DESTINATION bin)

  add_executable(codec2_mod_file_short codec2_mod_file_short.c)
  target_link_libraries(codec2_mod_file_short PRIVATE ${LIB_NAME_RINGBUFFER} dsp_node file_func kill_throbber codec2_func)
  target_compile_options(codec2_mod_file_short PRIVATE -Werror -Wall -Wextra -Wconversion -Wsign-conversion)
  install(TARGETS codec2_mod_file_short DESTINATION bin)

  add_executable(codec2_demod_file_short codec2_demod_file_short.c)
  target_link_libraries(codec2_demod_file_short PRIVATE ${LIB_NAME_RINGBUFFER} dsp_node file_func kill_throbber codec2_func)
  target_compile_options(codec2_demod_file_short PRIVATE -Werror -Wall -Wextra -Wconversion -Wsign-conversion)
  install(TARGETS codec2_demod_file_short DESTINATION bin)
endif()

if(BUILD_UHD_EXAMPLES)
  add_executable(uhd_rx_to_file uhd_rx_to_file.c)
  target_link_libraries(uhd_rx_to_file PRIVATE ${LIB_NAME_RINGBUFFER} dsp_node file_func kill_throbber uhd_func)
  target_compile_options(uhd_rx_to_file PRIVATE -Werror -Wall -Wextra -Wconversion -Wsign-conversion)
  install(TARGETS uhd_rx_to_file DESTINATION bin)

  add_executable(uhd_tx_from_file uhd_tx_from_file.c)
  target_link_libraries(uhd_tx_from_file PRIVATE ${LIB_NAME_RINGBUFFER} dsp_node file_func kill_throbber uhd_func)
  target_compile_options(uhd_tx_from_file PRIVATE -Werror -Wall -Wextra -Wconversion -Wsign-conversion)
  install(TARGETS uhd_tx_from_file DESTINATION bin)
endif()

if(BUILD_CODEC2_EXAMPLES AND BUILD_SOXR_EXAMPLES)
  add_executable(resample_codec2_mod_file resample_codec2_mod_file.c)
  target_link_libraries(resample_codec2_mod_file PRIVATE ${LIB_NAME_RINGBUFFER} dsp_node file_func kill_throbber codec2_func soxr_func)
  target_compile_options(resample_codec2_mod_file PRIVATE -Werror -Wall -Wextra -Wconversion -Wsign-conversion)
  install(TARGETS resample_codec2_mod_file DESTINATION bin)

  add_executable(resample_codec2_demod_file resample_codec2_demod_file.c)
  target_link_libraries(resample_codec2_demod_file PRIVATE ${LIB_NAME_RINGBUFFER} dsp_node file_func kill_throbber codec2_func soxr_func)
  target_compile_options(resample_codec2_demod_file PRIVATE -Werror -Wall -Wextra -Wconversion -Wsign-conversion)
  install(TARGETS resample_codec2_demod_file DESTINATION bin)
endif()

if(BUILD_CODEC2_EXAMPLES AND BUILD_SOXR_EXAMPLES AND BUILD_UHD_EXAMPLES)
  add_executable(uhd_codec2_mod_file uhd_codec2_mod.c)
  target_link_libraries(uhd_codec2_mod_file PRIVATE ${LIB_NAME_RINGBUFFER} dsp_node file_func kill_throbber codec2_func soxr_func uhd_func)
  target_compile_options(uhd_codec2_mod_file PRIVATE -Werror -Wall -Wextra -Wconversion -Wsign-conversion)
  install(TARGETS uhd_codec2_mod_file DESTINATION bin)

  add_executable(uhd_codec2_demod_file uhd_codec2_demod.c)
  target_link_libraries(uhd_codec2_demod_file PRIVATE ${LIB_NAME_RINGBUFFER} dsp_node file_func kill_throbber codec2_func soxr_func uhd_func)
  target_compile_options(uhd_codec2_demod_file PRIVATE -Werror -Wall -Wextra -Wconversion -Wsign-conversion)
  install(TARGETS uhd_codec2_demod_file DESTINATION bin)
endif()

if(BUILD_NCURSES_VERSIONS)
  add_subdirectory(ncurses)
endif()

message(STATUS "Applications selected to build:")

get_property(target_names DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY BUILDSYSTEM_TARGETS)

foreach(app_name ${target_names})
  message(STATUS "* ${app_name}")
endforeach(app_name ${target_names})
