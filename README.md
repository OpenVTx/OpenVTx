Join us on discord https://discord.gg/uGUKaD2u8Z

**Open source video transmitter firmware for the FPV community**

OpenVTx aims to provide firmware with both the SmartAudio and Tramp protocols.  Either protocol can be used on the flight controller and on VTx power up the protocol used is automatically detected by OpenVTx.

Currently SA is fully implemented and test against [Rev. 09](https://www.team-blacksheep.com/tbs_smartaudio_rev09.pdf).  To date IMRC has not released a protocol standard and implementation is based on information found within Betaflight.

# Features

**Race Mode** - When at a race event and using pitmode on a switch with Betaflight, select the RCE VTx power level.  It will force pitmode on boot and only use 25mW.


# Currently Supported VTx
- [EWRF E7082VM V1 & V2](https://www.aliexpress.com/wholesale?catId=0&initiative_id=SB_20210125211126&SearchText=EWRF+E7082VM) - Max output power of [+500mW](img/EWRF_E7082VM_500mW.jpg).

# Setup
OpenVTx uses VS Code and [PlatformIO](https://platformio.org/platformio-ide).

fubarphill has made a fairly thorough video explaining the project and flashing process on youtube 
[![open vtx flashing guide](https://i.ytimg.com/vi/JsJOMwu4hBM/hqdefault.jpg?sqp=-oaymwEcCPYBEIoBSFXyq4qpAw4IARUAAIhCGAFwAcABBg==&amp;rs=AOn4CLA1-gKpZdeOFu6gtONHsG5YoIeSzg)](https://www.youtube.com/watch?v=JsJOMwu4hBM)

# Flashing the VTx
Flashing requires an [ST-LINK V2](https://www.aliexpress.com/wholesale?catId=0&initiative_id=SB_20210125211035&SearchText=ST-LINK+V2) and connection is via the below image.  Shown below is of the E7082VM and other targets will differ in appearance but the process will be similar.  Details will be added to a wiki as more targets are added.  

<img src="img/st_link_connection.png" width="50%">

Before flashing the read protection must be disabled. In the STM32 ST_Link Utility connect to the target and go to Option Bytes

<img src="img/OptionBytes.png" width="50%">

 **When you click apply, it will probably result in an error, but that is OK.**  
<img src="img/OptionBytes2.png" width="50%">

In the Option Bytes, disable Read Protection and Check the 3 boxes.  Apply.  
 **To reiterate, this will probably result in an error, but that is ok.**  
The next steps are:
 - with the ST-LINK V2, build and upload in platformio
 - wire the vtx to an fc (+, - and smart audio pin) to flash via serial passthrough - [explanation why this is done this way](https://youtu.be/JsJOMwu4hBM?t=594)
 - apply smart audio vtx table first to the fc via betaflight
 - flash the vtx via serial passthrough, build and upload in platformio


# Betaflight VTx Tables

Copy and paste your choice of protocol [VTx table](https://github.com/betaflight/betaflight/wiki/VTX-tables) into the Betaflights CLI.

```
# SMARTAUDIO
# vtxtable
vtxtable bands 6
vtxtable channels 8
vtxtable band 1 BOSCAM_A A FACTORY 5865 5845 5825 5805 5785 5765 5745 5725
vtxtable band 2 BOSCAM_B B FACTORY 5733 5752 5771 5790 5809 5828 5847 5866
vtxtable band 3 BOSCAM_E E FACTORY 5705 5685 5665 5645 5885 5905 5925 5945
vtxtable band 4 FATSHARK F FACTORY 5740 5760 5780 5800 5820 5840 5860 5880
vtxtable band 5 RACEBAND R FACTORY 5658 5695 5732 5769 5806 5843 5880 5917
vtxtable band 6 LOWRACE  L FACTORY 5333 5373 5413 5453 5493 5533 5573 5613
vtxtable powerlevels 5
vtxtable powervalues 1 2 14 20 26
vtxtable powerlabels 0 RCE 25 100 400
save
```
```
# TRAMP
# vtxtable
vtxtable bands 7
vtxtable channels 8
vtxtable band 1 BOSCAM_A A CUSTOM  5865 5845 5825 5805 5785 5765 5745 5725
vtxtable band 2 BOSCAM_B B CUSTOM  5733 5752 5771 5790 5809 5828 5847 5866
vtxtable band 3 BOSCAM_E E CUSTOM  5705 5685 5665 5645 5885 5905 5925 5945
vtxtable band 4 FATSHARK F CUSTOM  5740 5760 5780 5800 5820 5840 5860 5880
vtxtable band 5 RACEBAND R CUSTOM  5658 5695 5732 5769 5806 5843 5880 5917
vtxtable band 6 LOWRACE  L CUSTOM  5333 5373 5413 5453 5493 5533 5573 5613
vtxtable band 7 IMD6     I CUSTOM  5732 5765 5828 5840 5866 5740    0    0
vtxtable powerlevels 5
vtxtable powervalues 1 2 25 100 400
vtxtable powerlabels 0 RCE 25 100 400
save
```

# Contributors

Big thanks to cruwaller for adding the GD32 MCU found on the EWRF E7082VM.
