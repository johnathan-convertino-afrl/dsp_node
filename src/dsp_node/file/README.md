# File Node

C File read write node

author: Jay Convertino  

date: 2023.07.15

license: MIT

## Release Versions
### Current
  - none

### Past
  - none
  
## Info
  Generic C file read or write for DSP nodes. This can have its type in/out specified so it matches the node that feeds it.
  File read will ignore the input type (since its input is a file). File write will ignore the output type (since it is
  invalid and there is no ring buffer output from file write).
