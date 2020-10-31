/*

https://www.eachine.com/Eachine-TX801-5_8G-72CH-0_01mW-5mW-25mW-50mW-100mW-200mW-400mW-600mW-Switched-AV-VTX-FPV-Transmitter-p-842.html


# vtxtable for Betaflight
vtxtable bands 5
vtxtable channels 8
vtxtable band 1 BOSCAM_A A CUSTOM  5865 5845 5825 5805 5785 5765 5745 5725
vtxtable band 2 BOSCAM_B B CUSTOM  5733 5752 5771 5790 5809 5828 5847 5866
vtxtable band 3 BOSCAM_E E CUSTOM  5705 5685 5665 5645 5885 5905 5925 5945
vtxtable band 4 FATSHARK F CUSTOM  5740 5760 5780 5800 5820 5840 5860 5880
vtxtable band 5 RACEBAND R CUSTOM  5658 5695 5732 5769 5806 5843 5880 5917
vtxtable powerlevels 4
vtxtable powervalues 1 25 100 200
vtxtable powerlabels 1 25 100 200


The below measurements are done hot on the bench.  However mW output does increase when the VTx is cooled.
If you have made it this far and have the equipment to check these outputs, please do and report back on their accuracy :)

pinOutput value and mW measured
0 = 0
5 = 0
10 = 0
14 = 10
15 = 13
16 = 16
17 = 22
18 = 27 <-- 25 mW
19 = 31
20 = 32
25 = 50
30 = 80
35 = 90
40 = 100 <-- 100 mW
45 = 
50 = 
55 = 
60 = 
63 = 200 <-- 200 mW

*/

#include <Arduino.h>

#define MAX_POWER 200 // mW

#define SPI_SS PC6
#define SPI_CLOCK PC5
#define SPI_DATA PC7

#define POWER_AMP_1 PA2
#define POWER_AMP_2 PD4
#define POWER_AMP_3 PD3
#define POWER_AMP_4 PD2
#define POWER_AMP_5 PC3
#define POWER_AMP_6 PC4

void rfPowerAmpPinSetup()
{
  // pinMode(POWER_AMP_1, INPUT);
  // pinMode(POWER_AMP_2, INPUT);
  // pinMode(POWER_AMP_3, INPUT);
  // pinMode(POWER_AMP_4, INPUT);
  // pinMode(POWER_AMP_5, INPUT);
  
  // pinMode(POWER_AMP_1, OUTPUT);
  // pinMode(POWER_AMP_2, OUTPUT);
  // pinMode(POWER_AMP_3, OUTPUT);
  // pinMode(POWER_AMP_4, OUTPUT);
  // pinMode(POWER_AMP_5, OUTPUT);
  // pinMode(POWER_AMP_6, OUTPUT);
}

void setPowermW(uint16_t power)
{
  uint8_t pinOutput = 0;

  switch (power)
  {
  case 0:
    pinOutput = 1; // Setting to 0 does not reduce power for some reason :|
    myEEPROM.currPowerIndex = 0;
    myEEPROM.currPowermW = 0;
    myEEPROM.currPowerdB = 0;
    break;
  case 25:
    pinOutput = 18;
    myEEPROM.currPowerIndex = 1;
    myEEPROM.currPowermW = 25;
    myEEPROM.currPowerdB = 14;
    break;
  case 100:
    pinOutput = 50;
    myEEPROM.currPowerIndex = 2;
    myEEPROM.currPowermW = 100;
    myEEPROM.currPowerdB = 20;
    break;
  case 200:
    pinOutput = 63;
    myEEPROM.currPowerIndex = 3;
    myEEPROM.currPowermW = 200;
    myEEPROM.currPowerdB = 23;
    break;
  default:
    return; // power value not recognised and no change
    break;
  }

  if (pitMode)
  {
    pinOutput = 1; // Setting to 0 does not reduce power for some reason :|
    rtc6705PowerAmpOff();
  }
  else
  {
    rtc6705PowerAmpOn();
  }

  // digitalWrite(POWER_AMP_1, pinOutput & 0b000001);
  // digitalWrite(POWER_AMP_2, pinOutput & 0b000010);
  // digitalWrite(POWER_AMP_3, pinOutput & 0b000100);
  // digitalWrite(POWER_AMP_4, pinOutput & 0b001000);
  // digitalWrite(POWER_AMP_5, pinOutput & 0b010000);
  // digitalWrite(POWER_AMP_6, pinOutput & 0b100000);

}

void setPowerdB(uint16_t currPowerdB)
{
  switch (currPowerdB)
  {
  case 0:
    setPowermW(0);
    break;
  case 14:
    setPowermW(25);
    break;
  case 20:
    setPowermW(100);
    break;
  case 23:
    setPowermW(200);
    break;
  default:
    return; // power value not recognised and no change
    break;
  }
}
