#include "common.h"
#include "serial.h"
#include "targets.h"
#include "openVTxEEPROM.h"
#include "rtc6705.h"
#include <string.h>
#include <math.h>
#include "gpio.h"

uint8_t rxPacket[16];
uint8_t txPacket[18];
uint8_t vtxModeLocked;
uint8_t pitMode = 0;

void clearSerialBuffer(void)
{
    while (serial_available()) {
        serial_read();
    }
}

void zeroRxPacket(void)
{
    memset(rxPacket, 0, sizeof(rxPacket));
}

void zeroTxPacket(void)
{
    memset(txPacket, 0, sizeof(txPacket));
}


#if !defined(LED1) && defined(LED)
#define LED1 LED
#endif

#ifdef LED1
static gpio_out_t led1_pin;
#endif
#ifdef LED2
static gpio_out_t led2_pin;
#endif
#ifdef LED3
static gpio_out_t led3_pin;
#endif

void status_leds_init(void)
{
#ifdef LED1
  #ifdef LED1_REVERSED
    led1_pin = gpio_out_setup(LED1, 0);
  #else
    led1_pin = gpio_out_setup(LED1, 1);
  #endif
#endif
#ifdef LED2
  #ifdef LED2_REVERSED
    led2_pin = gpio_out_setup(LED2, 1);
  #else
  led2_pin = gpio_out_setup(LED2, 0);
  #endif
#endif
#ifdef LED3
  #ifdef LED3_REVERSED
    led3_pin = gpio_out_setup(LED3, 1);
  #else
  led3_pin = gpio_out_setup(LED3, 0);
  #endif
#endif
}

void status_led1(uint8_t state)
{
#ifdef LED1
  #ifdef LED1_REVERSED
    gpio_out_write(led1_pin, !state);
  #else
    gpio_out_write(led1_pin, state);
  #endif
#else
  (void)state;
#endif
}

void status_led2(uint8_t state)
{
#ifdef LED2
  #ifdef LED2_REVERSED
    gpio_out_write(led2_pin, !state);
  #else
    gpio_out_write(led2_pin, state);
  #endif
#else
  (void)state;
#endif
}

void status_led3(uint8_t state)
{
#ifdef LED3
  #ifdef LED3_REVERSED
    gpio_out_write(led3_pin, !state);
  #else
    gpio_out_write(led3_pin, state);
  #endif
#else
  (void)state;
#endif
}

void setPowerdB(float dB)
{
    myEEPROM.currPowerdB = dB;
    updateEEPROM = 1;

    if (pitMode)
        // Pit mode set => force output power to zero
        dB = 0;

    if (dB <= 1)
        rtc6705PowerAmpOff();
    else
        rtc6705PowerAmpOn();

    target_set_power_dB(dB);
}

void setPowermW(uint16_t mW)
{
    // float dB = 10.0 * log10((float)mW);
    // setPowerdB(dB);
}
