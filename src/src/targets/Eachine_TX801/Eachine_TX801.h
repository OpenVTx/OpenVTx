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
#ifndef __TARGET_DEF_H_
#define __TARGET_DEF_H_

#include <Arduino.h>

#define MAX_POWER 200 // mW

#define UART_TX 0
#define UART_RX 0
#define SERIAL_PIN PD5

#define SPI_SS PC6
#define SPI_CLOCK PC5
#define SPI_MOSI PC7

#define POWER_AMP_1 PA2
#define POWER_AMP_2 PD4
#define POWER_AMP_3 PD3
#define POWER_AMP_4 PD2
#define POWER_AMP_5 PC3
#define POWER_AMP_6 PC4

#endif /* __TARGET_DEF_H_ */
