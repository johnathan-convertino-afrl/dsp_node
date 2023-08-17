# DSP Node with examples

Examples for codec2 datac1 porition of freedv

author: Jay Convertino  

date: 2023.07.15

license: BEER-WARE  

## Release Versions
### Current
  - none

### Past
  - none

## Info
  - samples : Contains example files to be used for samples
  - src : contains all source code for DSP nodes and examples.
  
## Requirements (Ubuntu 20.04)
  - C89_pthread_ring_buffer (https://github.com/sparkletron/C89_pthread_ring_buffer)
  - build-essential
  - cmake

## Recommended
  - Codec2 v1.0.5 or greater (https://github.com/drowe67/codec2)
  - UHD v4.4.0.0 (https://github.com/EttusResearch/uhd)
  - SOXR master
  - libasound2-dev
  - ncurses5-dev
  - doxygen
  
## Building
  1. mkdir build
  2. cd build
  3. cmake ../
  4. make

## Cmake options

The Following options are off by default. ALSA will not build if it is not found.
  * EXAMPLE APPLICATIONS:
    - BUILD_EXAMPLES_ALL : Build all examples.
    - BUILD_ALSA_EXAMPLES : Only build ALSA only examples.
    - BUILD_CODEC2_EXAMPLES : Only build CODEC2 only examples.
    - BUILD_UHD_EXAMPLES : Only build UHD only examples
    - BUILD_SOXR_EXAMPLES : Only build SOXR only examples.
    - BUILD_EXAMPLES : Only build file to file examples.
    - BUILD_NCURSES_VERSIONS : Build any of the above, but as a version with ncurses gui.
    - CREATE_DOXYGEN : Generate doxygen documents for DSP Node.
  * LIBRARIES (will automagically build for applications above)
    - BUILD_LIB_ALL : Build all dsp_node libraries
    - BUILD_LIB_SOXR : resample functions
    - BUILD_LIB_FILE : file functions
    - BUILD_LIB_UHD  : ettus radio
    - BUILD_LIB_ALSA : linux audio
    - BUILD_LIB_CODEC2 : data mod/demod
    - BUILD_LIB_TCP : TCP server or client
  
To turn on codec2 for example:
  - cmake ../ -DBUILD_CODEC2_EXAMPLES=ON
