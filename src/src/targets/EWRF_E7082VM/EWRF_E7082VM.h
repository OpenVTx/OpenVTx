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

#ifndef __TARGET_DEF_H_
#define __TARGET_DEF_H_

#define MAX_POWER     25 // mW

//#define SERIAL_PIN    -1

#define UART_RX       PA9 //PA10
#define UART_TX       PA9

#define SPI_SS        PB3
#define SPI_CLOCK     PA15
#define SPI_MOSI      PB4

#define LED           PA4
#define VREF          PA0
#define VPD           PA1

#define PWM_BIAS_FREQ 10000 // Hz

#endif /* __TARGET_DEF_H_ */
