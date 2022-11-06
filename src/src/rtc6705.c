#include "rtc6705.h"
#include "targets.h"
#include "common.h"
#include "openVTxEEPROM.h"
#include "gpio.h"

static gpio_out_t ss_pin;
static gpio_out_t sck_pin;
static gpio_out_t mosi_pin;

static uint32_t powerUpAfterSettleTime = 0;

void rtc6705spiPinSetup(void)
{
  ss_pin = gpio_out_setup(SPI_SS, 1);
  sck_pin = gpio_out_setup(SPI_CLOCK, 0);
  mosi_pin = gpio_out_setup(SPI_MOSI, 0);
}

uint32_t rtc6705readRegister(uint8_t reg)
{
  uint32_t writeData = reg | (READ_BIT << 4);
  uint32_t readData = 0;
  uint8_t i;

  gpio_out_write(ss_pin, 0);
  delayMicroseconds(1);

  // Write register address and read bit
  for (i = 0; i < 5; i++)
  {
    gpio_out_write(sck_pin, 0);
    delayMicroseconds(1);
    gpio_out_write(mosi_pin, writeData & 0x1);
    delayMicroseconds(1);
    gpio_out_write(sck_pin, 1);
    delayMicroseconds(1);

    writeData >>= 1;
  }

  // Change pin from output to input
  gpio_in_t miso_pin = gpio_in_setup(SPI_MOSI, 0);

  // Read data 20 bits
  for (i = 0; i < 20; i++)
  {
    gpio_out_write(sck_pin, 0);
    delayMicroseconds(1);

    if (gpio_in_read(miso_pin))
    {
      readData = readData | (1 << (5 + i));
    }

    gpio_out_write(sck_pin, 1);
    delayMicroseconds(1);
  }

  // Change pin back to output
  mosi_pin = gpio_out_setup(SPI_MOSI, 0);

  gpio_out_write(sck_pin, 0);
  gpio_out_write(mosi_pin, 0);
  gpio_out_write(ss_pin, 1);
  delayMicroseconds(1);

  return readData;
}

void rtc6705writeRegister(uint32_t data)
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
  uint32_t newRegData = StateRegister | (WRITE_BIT << 4);

  rtc6705writeRegister(newRegData);
}

void rtc6705ResetSynthRegA(void)
{
  uint32_t newRegData = SynthesizerRegisterA | (WRITE_BIT << 4) | (SYNTH_REG_A_DEFAULT << 5);

  rtc6705writeRegister(newRegData);
}

void rtc6705PowerAmpOn(void)
{
  uint32_t newRegData = PredriverandPAControlRegister | (WRITE_BIT << 4) | POWER_AMP_ON;

  uint32_t currentRegData = PredriverandPAControlRegister | (WRITE_BIT << 4) | rtc6705readRegister(PredriverandPAControlRegister);

  if (newRegData == currentRegData)
    return;

  rtc6705writeRegister(newRegData);
}

void rtc6705PowerAmpOff(void)
{
  uint32_t newRegData = PredriverandPAControlRegister | (WRITE_BIT << 4);

  uint32_t currentRegData = PredriverandPAControlRegister | (WRITE_BIT << 4) | rtc6705readRegister(PredriverandPAControlRegister);

  if (newRegData == currentRegData)
    return;

  rtc6705writeRegister(newRegData);
}

uint8_t rtc6705CheckFrequency()
{
  uint32_t freq = myEEPROM.currFreq * 1000U;
  freq /= 40;
  uint32_t SYN_RF_N_REG = freq / 64;
  uint32_t SYN_RF_A_REG = freq % 64;

  uint32_t newRegData = SynthesizerRegisterB | (WRITE_BIT << 4) | (SYN_RF_A_REG << 5) | (SYN_RF_N_REG << 12);

  uint32_t currentRegData = SynthesizerRegisterB | (WRITE_BIT << 4) | rtc6705readRegister(SynthesizerRegisterB);

  if (newRegData == currentRegData)
    return 1;
  else
    return 0;
}

void rtc6705WriteFrequency(uint32_t newFreq)
{
  if (myEEPROM.currFreq != newFreq)
  {
    myEEPROM.currFreq = newFreq;
    updateEEPROM();
  }

  uint32_t freq = newFreq * 1000U;
  freq /= 40;
  uint32_t SYN_RF_N_REG = freq / 64;
  uint32_t SYN_RF_A_REG = freq % 64;

  uint32_t newRegData = SynthesizerRegisterB | (WRITE_BIT << 4) | (SYN_RF_A_REG << 5) | (SYN_RF_N_REG << 12);

  uint32_t currentRegData = SynthesizerRegisterB | (WRITE_BIT << 4) | rtc6705readRegister(SynthesizerRegisterB);

  // Always set powerUpAfterSettleTime so that setPowerdB() is latter called and any changes to power level are set.
  powerUpAfterSettleTime = millis() + PLL_SETTLE_TIME;

  if (newRegData == currentRegData)
    return;

  /* Switch off */
  rtc6705PowerAmpOff();
  target_set_power_dB(0);

  rtc6705ResetSynthRegA();

  /* Set frequency */
  rtc6705writeRegister(newRegData);
}

void rtc6705PowerUpAfterPLLSettleTime()
{
  if (!powerUpAfterSettleTime)
    return;

  if (powerUpAfterSettleTime < millis())
  {
    setPowerdB(myEEPROM.currPowerdB);
    powerUpAfterSettleTime = 0;
  }
}
