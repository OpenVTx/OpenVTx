#include "common.h"
#include "serial.h"
#include "targets.h"
#include <string.h>

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
  case 26:
    setPowermW(400);
    break;
  default:
    return; // power value not recognised and no change
    break;
  }
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
  led1_pin = gpio_out_setup(LED1, 1);
#endif
#ifdef LED2
  led2_pin = gpio_out_setup(LED2, 0);
#endif
#ifdef LED3
  led3_pin = gpio_out_setup(LED3, 0);
#endif
}

void status_led1(uint8_t state)
{
#ifdef LED1
  gpio_out_write(led1_pin, state);
#endif
}

void status_led2(uint8_t state)
{
#ifdef LED2
  gpio_out_write(led2_pin, state);
#endif
}

void status_led3(uint8_t state)
{
#ifdef LED3
  gpio_out_write(led3_pin, state);
#endif
}
