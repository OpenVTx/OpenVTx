#ifndef __TARGET_DEF_H_
#define __TARGET_DEF_H_

#define MAX_POWER     400 // mW

#define SA_NUM_POWER_LEVELS 5 // Max 5 for INAV.
extern uint8_t saPowerLevelsLut[SA_NUM_POWER_LEVELS];

#define UART_RX       PA9
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

#endif /* __TARGET_DEF_H_ */
