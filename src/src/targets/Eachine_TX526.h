/*

https://www.eachine.com/Eachine-TX526-5_8G-40CH-25MW-or-200MW-or-600MW-Switchable-AV-Wireless-FPV-Transmitter-RP-SMA-Female-p-543.html


# vtxtable for Betaflight
vtxtable bands 5
vtxtable channels 8
vtxtable band 1 BOSCAM_A A CUSTOM  5865 5845 5825 5805 5785 5765 5745 5725
vtxtable band 2 BOSCAM_B B CUSTOM  5733 5752 5771 5790 5809 5828 5847 5866
vtxtable band 3 BOSCAM_E E CUSTOM  5705 5685 5665 5645 5885 5905 5925 5945
vtxtable band 4 FATSHARK F CUSTOM  5740 5760 5780 5800 5820 5840 5860 5880
vtxtable band 5 RACEBAND R CUSTOM  5658 5695 5732 5769 5806 5843 5880 5917
vtxtable powerlevels 3
vtxtable powervalues 1 25 200
vtxtable powerlabels 1 25 200


The below measurements are done hot on the bench.  However mW output does increase when the VTx is cooled.
If you have made it this far and have the equipment to check these outputs, please do and report back on their accuracy :)

*/

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

#define POWER_AMP_7 PA2
#define POWER_AMP_8 PA3
#define POWER_AMP_9 PB5
#define POWER_AMP_10 PB4
#define POWER_AMP_11 PD1

void rfPowerAmpPinSetup()
{
  pinMode(POWER_AMP_2, OUTPUT);
  pinMode(POWER_AMP_3, OUTPUT);
  pinMode(POWER_AMP_5, OUTPUT);
}

void setPower(uint16_t power)
{
  if (pitMode)
  {
    power = 1;
  }

  switch (power)
  {
  case 1:
    digitalWrite(POWER_AMP_2, LOW);
    digitalWrite(POWER_AMP_3, LOW);
    digitalWrite(POWER_AMP_5, HIGH);
    break;
  case 25:
    digitalWrite(POWER_AMP_2, LOW);
    digitalWrite(POWER_AMP_3, HIGH);
    digitalWrite(POWER_AMP_5, HIGH);
    break;
  case 200:
    digitalWrite(POWER_AMP_2, HIGH);
    digitalWrite(POWER_AMP_5, HIGH);
    break;
  default:
    return; // power value not recognised and no change
    break;
  }
}