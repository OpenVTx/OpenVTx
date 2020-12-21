#include "common.h"
#include "serial.h"
#include "targets.h"
#include "openVTxEEPROM.h"
#include "rtc6705.h"
#include <string.h>

extern struct PowerMapping power_mapping[];
extern uint8_t power_mapping_size;

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
#else
  (void)state;
#endif
}

void status_led2(uint8_t state)
{
#ifdef LED2
  gpio_out_write(led2_pin, state);
#else
  (void)state;
#endif
}

void status_led3(uint8_t state)
{
#ifdef LED3
  gpio_out_write(led3_pin, state);
#else
  (void)state;
#endif
}


uint8_t get_power_dB_by_index(uint8_t idx)
{
  if (idx < power_mapping_size)
    return power_mapping[idx].dB;
  return 0;
}

uint8_t get_power_dB_by_mW(uint16_t mW)
{
  uint_fast8_t iter;
  for (iter = 0; iter < power_mapping_size; iter++) {
    if (power_mapping[iter].mW == mW)
      return power_mapping[iter].dB;
  }
  return 0;
}

uint16_t get_power_mW_by_index(uint8_t idx)
{
  if (idx < power_mapping_size)
    return power_mapping[idx].mW;
  return 0;
}

uint16_t get_power_mW_by_dB(uint8_t dB)
{
  uint_fast8_t iter;
  for (iter = 0; iter < power_mapping_size; iter++) {
    if (power_mapping[iter].dB == dB)
      return power_mapping[iter].mW;
  }
  return 0;
}

uint8_t get_power_db_values(uint8_t * const list)
{
  uint8_t cnt = 0;
  for (cnt = 0; cnt < power_mapping_size; cnt++) {
    list[cnt] = power_mapping[cnt].dB;
  }
  return cnt;
}

static void setPowerIndex(uint8_t index)
{
    /* Update database values */
    if (index < power_mapping_size) {
        uint16_t mW = power_mapping[index].mW;
        myEEPROM.currPowerIndex = index;
        myEEPROM.currPowermW = mW;
        myEEPROM.currPowerdB = power_mapping[index].dB;
        updateEEPROM = 1;

        if (pitMode)
            // Pit mode set => force output power to zero
            mW = 0;

        if (mW <= 1)
            rtc6705PowerAmpOff();
        else
            rtc6705PowerAmpOn();

        target_set_power_mW(mW);
    }
}

void setPowerdB(uint8_t dB)
{
    uint8_t index;
    for (index = 0; index < power_mapping_size; index++) {
        if (power_mapping[index].dB == dB)
            break;
    }
    setPowerIndex(index);
}

void setPowermW(uint16_t mW)
{
    uint8_t index;
    for (index = 0; index < power_mapping_size; index++) {
        if (power_mapping[index].mW == mW)
            break;
    }
    setPowerIndex(index);
}
