################################################################################
### date      2023.06.26
### author    Jay Convertino
### brief     dsp_node examples
################################################################################

cmake_minimum_required(VERSION 3.14)

include_directories(../dsp_node/ ../kill_throbber)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/apps)

add_executable(ncurses_file_to_file file_to_file.c)
target_link_libraries(ncurses_file_to_file PRIVATE ${LIB_NAME_RINGBUFFER} dsp_node file_func kill_throbber ncurses_dsp_monitor)
target_compile_options(ncurses_file_to_file PRIVATE -Werror -Wall -Wextra -Wconversion -Wsign-conversion)
install(TARGETS ncurses_file_to_file DESTINATION bin)

if(BUILD_ALSA_EXAMPLES AND BUILD_CODEC2_EXAMPLES)
  if(ALSA_FOUND)
    add_executable(ncurses_codec2_demod_alsa_to_file codec2_demod_alsa_to_file.c)
    target_link_libraries(ncurses_codec2_demod_alsa_to_file PRIVATE ${LIB_NAME_RINGBUFFER} dsp_node file_func kill_throbber alsa_func codec2_func ncurses_dsp_monitor)
    target_compile_options(ncurses_codec2_demod_alsa_to_file PRIVATE -Werror -Wall -Wextra -Wconversion -Wsign-conversion)
    install(TARGETS ncurses_codec2_demod_alsa_to_file DESTINATION bin)

    add_executable(ncurses_codec2_mod_file_to_alsa codec2_mod_file_to_alsa.c)
    target_link_libraries(ncurses_codec2_mod_file_to_alsa PRIVATE ${LIB_NAME_RINGBUFFER} dsp_node file_func kill_throbber alsa_func codec2_func ncurses_dsp_monitor)
    target_compile_options(ncurses_codec2_mod_file_to_alsa PRIVATE -Werror -Wall -Wextra -Wconversion -Wsign-conversion)
    install(TARGETS ncurses_codec2_mod_file_to_alsa DESTINATION bin)
  endif()
endif()

if(BUILD_UHD_EXAMPLES)
  add_executable(ncurses_uhd_rx_to_file uhd_rx_to_file.c)
  target_link_libraries(ncurses_uhd_rx_to_file PRIVATE ${LIB_NAME_RINGBUFFER} dsp_node file_func kill_throbber uhd_func ncurses_dsp_monitor)
  target_compile_options(ncurses_uhd_rx_to_file PRIVATE -Werror -Wall -Wextra -Wconversion -Wsign-conversion)
  install(TARGETS ncurses_uhd_rx_to_file DESTINATION bin)

  add_executable(ncurses_uhd_tx_from_file uhd_tx_from_file.c)
  target_link_libraries(ncurses_uhd_tx_from_file PRIVATE ${LIB_NAME_RINGBUFFER} dsp_node file_func kill_throbber uhd_func ncurses_dsp_monitor)
  target_compile_options(ncurses_uhd_tx_from_file PRIVATE -Werror -Wall -Wextra -Wconversion -Wsign-conversion)
  install(TARGETS ncurses_uhd_tx_from_file DESTINATION bin)
endif()

if(BUILD_CODEC2_EXAMPLES AND BUILD_SOXR_EXAMPLES AND BUILD_UHD_EXAMPLES)
  add_executable(ncurses_uhd_codec2_mod_file uhd_codec2_mod.c)
  target_link_libraries(ncurses_uhd_codec2_mod_file PRIVATE ${LIB_NAME_RINGBUFFER} dsp_node file_func kill_throbber codec2_func soxr_func uhd_func ncurses_dsp_monitor)
  target_compile_options(ncurses_uhd_codec2_mod_file PRIVATE -Werror -Wall -Wextra -Wconversion -Wsign-conversion)
  install(TARGETS ncurses_uhd_codec2_mod_file DESTINATION bin)

  add_executable(ncurses_uhd_codec2_demod_file uhd_codec2_demod.c)
  target_link_libraries(ncurses_uhd_codec2_demod_file PRIVATE ${LIB_NAME_RINGBUFFER} dsp_node file_func kill_throbber codec2_func soxr_func uhd_func ncurses_dsp_monitor)
  target_compile_options(ncurses_uhd_codec2_demod_file PRIVATE -Werror -Wall -Wextra -Wconversion -Wsign-conversion)
  install(TARGETS ncurses_uhd_codec2_demod_file DESTINATION bin)
endif()

message(STATUS "Ncurses applications selected to build:")

get_property(target_names DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY BUILDSYSTEM_TARGETS)

foreach(app_name ${target_names})
  message(STATUS "* ${app_name}")
endforeach(app_name ${target_names})
