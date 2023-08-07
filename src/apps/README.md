# DSP Node Example Apps

Example applications

author: Jay Convertino  

date: 2023.07.15

license: BEER-WARE  

## Release Versions
### Current
  - none

### Past
  - none
  
## Applications (Source)
  - alsa_to_file.c
    * INFO: Read ALSA and write data to file.

    <details>

    <summery>REQUIREMENTS</summery>

      * C89_pthread_ring_buffer (https://github.com/sparkletron/C89_pthread_ring_buffer)
      * libasound2-dev

    </details>

  - file_to_alsa.c
    * INFO: Write ALSA and read data from file.

    <details>

    <summery>REQUIREMENTS</summery>

      * C89_pthread_ring_buffer (https://github.com/sparkletron/C89_pthread_ring_buffer)
      * libasound2-dev

    </details>

  - codec2_demod_alsa_to_file.c
    * INFO: Read ALSA and write data to file.

    <details>

    <summery>REQUIREMENTS</summery>

      * C89_pthread_ring_buffer (https://github.com/sparkletron/C89_pthread_ring_buffer)
      * libasound2-dev
      * Codec2 v1.0.5 or greater (https://github.com/drowe67/codec2)

    </details>

  - codec2_mod_file_to_alsa.c
    * INFO: Write ALSA and read data from file.

    <details>

    <summery>REQUIREMENTS</summery>

      * C89_pthread_ring_buffer (https://github.com/sparkletron/C89_pthread_ring_buffer)
      * libasound2-dev
      * Codec2 v1.0.5 or greater (https://github.com/drowe67/codec2)

    </details>

  - file_to_file.c
    * INFO: Read file and write to file.

    <details>

    <summery>REQUIREMENTS</summery>

      * C89_pthread_ring_buffer (https://github.com/sparkletron/C89_pthread_ring_buffer)

    </details>

  - uhd_codec2_mod.c
    * INFO: Use the UHD TX to send codec2 datac1 modulated data from file.

    <details>

    <summery>REQUIREMENTS</summery>

      * C89_pthread_ring_buffer (https://github.com/sparkletron/C89_pthread_ring_buffer)
      * UHD v4.4.0.0 (https://github.com/EttusResearch/uhd)
      * SOXR master
      * Codec2 v1.0.5 or greater (https://github.com/drowe67/codec2)

    </details>

  - uhd_codec2_demod.c
    * INFO: Use the UHD RX to receive codec2 datac1 demodulated and write to file.

    <details>

    <summery>REQUIREMENTS</summery>

      * C89_pthread_ring_buffer (https://github.com/sparkletron/C89_pthread_ring_buffer)
      * UHD v4.4.0.0 (https://github.com/EttusResearch/uhd)
      * SOXR master
      * Codec2 v1.0.5 or greater (https://github.com/drowe67/codec2)

    </details>

  - codec2_demod_file_comp.c

    * INFO: Decode codec2 datac1 complex encoded file.

    <details>

    <summery>REQUIREMENTS</summery>

      * C89_pthread_ring_buffer (https://github.com/sparkletron/C89_pthread_ring_buffer)
      * Codec2 v1.0.5 or greater (https://github.com/drowe67/codec2)

    </details>

  - codec2_mod_file_comp.c
    * INFO: Encode data file with codec2 datac1 complex.

    <details>

    <summery>REQUIREMENTS</summery>

      * C89_pthread_ring_buffer (https://github.com/sparkletron/C89_pthread_ring_buffer)
      * Codec2 v1.0.5 or greater (https://github.com/drowe67/codec2)

    </details>

  - codec2_demod_file_short.c

    * INFO: Decode codec2 datac1 complex encoded file.

    <details>

    <summery>REQUIREMENTS</summery>

      * C89_pthread_ring_buffer (https://github.com/sparkletron/C89_pthread_ring_buffer)
      * Codec2 v1.0.5 or greater (https://github.com/drowe67/codec2)

    </details>

  - codec2_mod_file_short.c
    * INFO: Encode data file with codec2 datac1 complex.

    <details>

    <summery>REQUIREMENTS</summery>

      * C89_pthread_ring_buffer (https://github.com/sparkletron/C89_pthread_ring_buffer)
      * Codec2 v1.0.5 or greater (https://github.com/drowe67/codec2)

    </details>

  - uhd_rx_to_file.c
    * INFO: Use the UHD RX to receive data samples to file.

    <details>

    <summery>REQUIREMENTS</summery>

      * C89_pthread_ring_buffer (https://github.com/sparkletron/C89_pthread_ring_buffer)
      * UHD v4.4.0.0 (https://github.com/EttusResearch/uhd)

    </details>

  - uhd_tx_from_file.c
    * INFO: Use the UHD TX to transmit data samples from a file.

    <details>

    <summery>REQUIREMENTS</summery>

      * C89_pthread_ring_buffer (https://github.com/sparkletron/C89_pthread_ring_buffer)
      * UHD v4.4.0.0 (https://github.com/EttusResearch/uhd)

    </details>

  - resample_codec2_demod_file.c
    * INFO: Decode codec2 datac1 encoded file that has been upsampled by SOXR.

    <details>

    <summery>REQUIREMENTS</summery>

      * C89_pthread_ring_buffer (https://github.com/sparkletron/C89_pthread_ring_buffer)
      * SOXR master
      * Codec2 v1.0.5 or greater (https://github.com/drowe67/codec2)

    </details>

  - resample_codec2_mod_file.c
    * INFO: Encode data file with codec2 datac1 that has been upsampled by SOXR.

    <details>

    <summery>REQUIREMENTS</summery>

      * C89_pthread_ring_buffer (https://github.com/sparkletron/C89_pthread_ring_buffer)
      * SOXR master
      * Codec2 v1.0.5 or greater (https://github.com/drowe67/codec2)

    </details>

## Execution Notes
### Sample Information.
  - All samples for ALSA apps data are as follows:
    * 8000 samples per second
    * 16 bit little endian
    * mono (single channel)
  - Radio based apps are as follows:
    * User Adjustable sample rate.
    * Floating point I/Q
    
### ALSA example programs
  - aplay -L will list devices for -d parameter of this application (-D for aplay).
  
  ``` 
    plughw:CARD=CODEC,DEV=0
      USB Audio CODEC, USB Audio
      Hardware device with all software conversions
    usbstream:CARD=CODEC
      USB Audio CODEC
      USB Stream Output
  ```
      
  - for the -d parameter use "plughw:CARD=CODEC,DEV=0" for example.
  - All sound devices will need to be setup in ALSA. 
    * alsamixer to alter ALSA level devices.
  - aplay will play modulated data.
    * defaults to 8000 samples at 16bit little endian, which is format for the datac1 samples
    * if it doesn't, change it. aplay -h
  - if the application complains about the library codec2 missing. Add the local library path.
    * export LD_LIBRARY_PATH=/path/to/the/folder/lib/codec2/build/src/

### UHD example programs
  - UHD Args will need to be a string that needs to contain at least the IP address to connect to.
    * addr=192.168.10.2
  - Long string, each option is to help be more specific.
    * addr=192.168.10.2,device=usrp2,name=,serial=30C569E

## Analysis Notes
### GNUPLOT
The following line is useful for viewing data in time in gnuplot
  1. Start gnuplot
  2. At the promopt use the following. Record sets the limit to the number of samples used. Format is the data format.
    - plot '1_out.bin' binary record=10000 format="%float" using 1 with lines

