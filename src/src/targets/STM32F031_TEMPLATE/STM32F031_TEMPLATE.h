#ifndef __TARGET_DEF_H_
#define __TARGET_DEF_H_

#include <Arduino.h>

#define MAX_POWER     400 // mW

#define UART_RX       PA2
#define UART_TX       PA2

#define SPI_MOSI      PB7
#define SPI_SS        PB6
#define SPI_CLOCK     PB4

// #define LED1          PA3 // NA (power)
// #define LED2          PA3 // Blue (connected)
#define LED3          PB3 // Red (SA message)

// #define LED1_REVERSED
// #define LED2_REVERSED
// #define LED3_REVERSED

#define VREF          PA8
#define VPD           -1

#define RTC_BIAS      PA4

/******* Target specific declarations *******/
#include "gpio.h"
#include "serial.h"

#endif /* __TARGET_DEF_H_ */
