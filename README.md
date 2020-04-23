# OpenVTx
Open source video transmitter firmware for FPV.

Some older 'dumb' video transmitters have the capability to use serial communication (aka telemetry or Smart Audio) for setting frequency, power, etc.  For example, the Eachine TX801 shown below can be flashed by removing the VTx PCB from the power/control PCB to access the SWM and RST pads.  Once flashed the VTx PCB can be used on its own by connecting 5V, GND, video, and Tx for smart audio.

OpenVTx uses the Tramp protocol due to it simplicity setting up the VTx table within Betaflight.  Examples of the VTx table for each VTx supported can be found in the target file.

https://github.com/JyeSmith/OpenVTx/blob/master/src/src/targets

Working
- Implements the Tramp protocol
- Change frequency
- Change power. Power settings depends on the VTx and how the settings have been configured in target.h
- Remembers previous settings on reboot.

Attempts to do but needs checking and more work
- Power settings
- Clean startup and channel change.
- Pitmode (currently disabled)

# Supported VTx
- Eachine TX801
- Eachine TX526 (hardware and power levels need a closer look :/ )

# Setup
OpenVTx uses VS Code and PlatformIO.

# Flashing the VTx
Flashing requires an ST-LINK V2.  Connection is via the below images.  Select your VTx target from within PlatformIO and press Upload.

<img src="img/flashing1.jpg" width="50%"><img src="img/flashing2.jpg" width="50%">
