# ALSA Node

ALSA DSP Node functions

author: Jay Convertino  

date: 2023.01.23  

license: MIT

## Release Versions
### Current
  - none

### Past
  - none
  
## Info
  This set of callbacks will morph DSP Node into a ALSA read or write, depending on specified callbacks.
  Selectable items are format, channels and rate.

  See doxygen for function specific information.

  * FORMAT :  Valid types: SND_PCM_FORMAT_S8, SND_PCM_FORMAT_U8, SND_PCM_FORMAT_S16_LE, SND_PCM_FORMAT_U16_LE, SND_PCM_FORMAT_FLOAT, SND_PCM_FORMAT_FLOAT64.
  * CHANNELS : 1 for real/mono. 2 for complex/stereo.
  * RATE : Any valid sampling rate for the ALSA device.
