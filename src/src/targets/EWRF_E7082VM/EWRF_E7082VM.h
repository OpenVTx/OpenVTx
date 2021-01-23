/*

# vtxtable
vtxtable bands 5
vtxtable channels 8
vtxtable band 1 BOSCAM_A A FACTORY 5865 5845 5825 5805 5785 5765 5745 5725
vtxtable band 2 BOSCAM_B B FACTORY 5733 5752 5771 5790 5809 5828 5847 5866
vtxtable band 3 BOSCAM_E E FACTORY 5705 5685 5665 5645 5885 5905 5925 5945
vtxtable band 4 FATSHARK F FACTORY 5740 5760 5780 5800 5820 5840 5860 5880
vtxtable band 5 RACEBAND R FACTORY 5658 5695 5732 5769 5806 5843 5880 5917
vtxtable powerlevels 5
vtxtable powervalues 0 14 17 20 26
vtxtable powerlabels 0 25 50 100 400

*/

#ifndef __TARGET_DEF_H_
#define __TARGET_DEF_H_

#define MAX_POWER     400 // mW

//#define SERIAL_PIN    -1

#define UART_RX       PA9 //PA10
#define UART_TX       PA9

#define SPI_SS        PB3
#define SPI_CLOCK     PA15
#define SPI_MOSI      PB4

#define LED1          PA4 // Red (power)
#define LED2          PA3 // Green (connected)
#define LED3          PA2 // Blue (SA message)
#define VREF          PA0
#define VPD           PA1

#define RTC_BIAS      PB5

/******* Target specific declarations *******/
#include <gd32f1x0.h>
#include "gpio.h"
#include "serial.h"

uint32_t millis(void);
void delay(uint32_t ms);
void delayMicroseconds(uint32_t us);
void checkPowerOutput(void);

#endif /* __TARGET_DEF_H_ */
