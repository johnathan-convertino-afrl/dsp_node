# UHD Node

UHD DSP Node

author: Jay Convertino  

date: 2023.07.15

license: BEER-WARE  

## Release Versions
### Current
  - none

### Past
  - none
  
## Info
  Create a single UHD device connecition. This can then have RX and/or TX methods called to create needed nodes.
  This will only connect to a single device in the same program regardless of how many you create. This uses
  a global variable in the c source file. Future would be to create a better method for this.
