#include "rtc6705.h"
#include "targets.h"
#include "common.h"
#include "openVTxEEPROM.h"
#include "gpio.h"

static gpio_out_t ss_pin;
static gpio_out_t sck_pin;
static gpio_out_t mosi_pin;


void spiPinSetup(void)
{
  ss_pin = gpio_out_setup(SPI_SS, 1);
  sck_pin = gpio_out_setup(SPI_CLOCK, 0);
  mosi_pin = gpio_out_setup(SPI_MOSI, 0);
}

void sendBits(uint32_t data)
{
  gpio_out_write(ss_pin, 0);
  delayMicroseconds(1);

  for (uint8_t i = 0; i < 25; i++)
  {
    gpio_out_write(sck_pin, 0);
    delayMicroseconds(1);
    gpio_out_write(mosi_pin, data & 0x1);
    delayMicroseconds(1);
    gpio_out_write(sck_pin, 1);
    delayMicroseconds(1);

    data >>= 1;
  }

  gpio_out_write(sck_pin, 0);
  gpio_out_write(mosi_pin, 0);
  gpio_out_write(ss_pin, 1);
  delayMicroseconds(1);
}

void rtc6705ResetState(void)
{
  uint32_t data = StateRegister | (1 << 4) | (0b0 << 5);

  sendBits(data);
}

void rtc6705PowerAmpOn(void)
{
  uint32_t data = PredriverandPAControlRegister | (1 << 4) | (0b00000100111110111111 << 5);

  sendBits(data);
}

void rtc6705PowerAmpOff(void)
{
  uint32_t data = PredriverandPAControlRegister | (1 << 4) | (0b00000000000000000000 << 5);

  sendBits(data);
}

void rtc6705WriteFrequency(uint32_t newFreq)
{
  uint32_t freq = newFreq * 1000U;
  freq /= 40;
  uint32_t SYN_RF_N_REG = freq / 64;
  uint32_t SYN_RF_A_REG = freq % 64;

  uint32_t data = SynthesizerRegisterB | (1 << 4) | (SYN_RF_A_REG << 5) | (SYN_RF_N_REG << 12);

  rtc6705PowerAmpOff();
  setPowermW(0);

  sendBits(data);

  if (!pitMode)
  {
    rtc6705PowerAmpOn();
    setPowerdB(myEEPROM.currPowerdB);
  }
}
