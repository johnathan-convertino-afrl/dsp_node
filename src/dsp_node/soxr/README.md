# SOXR Node

SOXR DSP Node

author: Jay Convertino  

date: 2023.07.15

license: MIT

## Release Versions
### Current
  - none

### Past
  - none
  
## Info
  Uses the SOXR library to resample data. This is for up or down conversion of data streams.
  Recommend using input/output or output/input rations that are evenly divisable. This library
  requires a patch to use the fetchContent method of cmake. Library path does not get set
  correctly with old variable.
