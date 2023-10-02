#include "common.h"
#include "serial.h"
#include "targets.h"
#include "openVTxEEPROM.h"
#include "rtc6705.h"
#include <string.h>
#include <math.h>
#include "helpers.h"

const uint8_t channelFreqLabel[48] = {
    'B', 'A', 'N', 'D', '_', 'A', ' ', ' ', // A
    'B', 'A', 'N', 'D', '_', 'B', ' ', ' ', // B
    'B', 'A', 'N', 'D', '_', 'E', ' ', ' ', // E
    'F', 'A', 'T', 'S', 'H', 'A', 'R', 'K', // F
    'R', 'A', 'C', 'E', ' ', ' ', ' ', ' ', // R
    'R', 'A', 'C', 'E', '_', 'L', 'O', 'W', // L
};

const uint8_t bandLetter[6] = {'A', 'B', 'E', 'F', 'R', 'L'};

uint8_t rxPacket[64];
uint8_t txPacket[64];
uint8_t vtxModeLocked;
uint8_t pitMode = 0;
uint8_t initFreqPacketRecived = 0;

uint16_t channelFreqTable[FREQ_TABLE_SIZE] = {
    5865, 5845, 5825, 5805, 5785, 5765, 5745, 5725, // A
    5733, 5752, 5771, 5790, 5809, 5828, 5847, 5866, // B
    5705, 5685, 5665, 5645, 5885, 5905, 5925, 5945, // E
    5740, 5760, 5780, 5800, 5820, 5840, 5860, 5880, // F
    5658, 5695, 5732, 5769, 5806, 5843, 5880, 5917, // R
    5362, 5399, 5436, 5473, 5510, 5547, 5584, 5621  // LowRace
};

uint8_t getFreqTableSize(void)
{
    return ARRAY_SIZE(channelFreqTable);
}

uint8_t getFreqTableBands(void)
{
    return getFreqTableSize() / getFreqTableChannels();
}

uint8_t getFreqTableChannels(void)
{
    return 8;
}

uint16_t getFreqByIdx(uint8_t idx)
{
    return channelFreqTable[idx];
}

uint8_t channelFreqLabelByIdx(uint8_t idx)
{
    return channelFreqLabel[idx];
}

uint8_t getBandLetterByIdx(uint8_t idx)
{
    return bandLetter[idx];
}

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

void setPowerdB(float dB)
{
    if (myEEPROM.currPowerdB != dB)
    {
      myEEPROM.currPowerdB = dB;
      updateEEPROM();
    }

    if (pitMode)
    {
      rtc6705PowerAmpOff();
      target_set_power_dB(0);
    } else
    {
      rtc6705PowerAmpOn();
      target_set_power_dB(dB);
    }
}

void setPowermW(uint16_t mW)
{
    myEEPROM.currPowermW = mW;

    float dB = 10.0 * log10f((float)mW);  // avoid double conversion!
    setPowerdB(dB);
}

#define BOOTLOADER_KEY  0x4f565458 // OVTX
#define BOOTLOADER_TYPE 0xACDC

struct bootloader {
    uint32_t key;
    uint32_t reset_type;
    uint32_t baud;
};

void reboot_into_bootloader(uint32_t baud)
{
    extern uint32_t _bootloader_data;
    struct bootloader * blinfo = (struct bootloader*)&_bootloader_data;
    blinfo->key = BOOTLOADER_KEY;
    blinfo->reset_type = BOOTLOADER_TYPE;
    blinfo->baud = baud;

    rtc6705PowerAmpOff();
    target_set_power_dB(0);
    delay(200);

    mcu_reboot();
}
