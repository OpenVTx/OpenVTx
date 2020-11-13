#include "rtc6705.h"
#include "targets.h"
#include "common.h"
#include "openVTxEEPROM.h"
#include <Arduino.h>

void spiPinSetup(void)
{
  pinMode(SPI_CLOCK, OUTPUT);
  digitalWrite(SPI_CLOCK, LOW);
  pinMode(SPI_DATA, OUTPUT);
  digitalWrite(SPI_DATA, LOW);
  pinMode(SPI_SS, OUTPUT);
  digitalWrite(SPI_SS, HIGH);
}

void sendBits(uint32_t data)
{
  digitalWrite(SPI_SS, LOW);
  delayMicroseconds(1);

  for (uint8_t i = 0; i < 25; i++)
  {
    digitalWrite(SPI_CLOCK, LOW);
    delayMicroseconds(1);
    digitalWrite(SPI_DATA, data & 0x1);
    delayMicroseconds(1);
    digitalWrite(SPI_CLOCK, HIGH);
    delayMicroseconds(1);

    data >>= 1;
  }

  digitalWrite(SPI_CLOCK, LOW);
  digitalWrite(SPI_DATA, LOW);
  digitalWrite(SPI_SS, HIGH);
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
