#include <Arduino.h>
#include <EEPROM.h>
#include "openVTxEEPROM.h"

#define SERIAL_PIN PD5
#define TRAMP_BAUD 9600
#define SMARTAUDIO_BAUD 4800

char rxPacket[16] = {0};
char txPacket[18] = {0}; // May need to be increase for SA2.1 with more than 4 power levels

uint8_t pitMode = 1;
bool vtxModeLocked = false;
uint16_t temperature = 0; // Dummy value.

#include "targets.h"
#include "rtc6705.h"
#include "common.h"
#include "tramp.h"
#include "smartAudio.h"

void setup()
{
  rfPowerAmpPinSetup();
  setPowerdB(0);

  readEEPROM();
  pitMode = (myEEPROM.pitmodeInRange || myEEPROM.pitmodeOutRange) ? 1 : 0;

  spiPinSetup();  
  rtc6705ResetState(); // During testing registers got messed up. So now it gets reset on boot!
  rtc6705WriteFrequency(myEEPROM.currFreq);

  Serial_begin(myEEPROM.vtxMode == TRAMP ? TRAMP_BAUD : SMARTAUDIO_BAUD);

  while (!Serial)
  {
    ;
  }
  UART1_HalfDuplexCmd(ENABLE);
  pinMode(SERIAL_PIN, INPUT_PULLUP);

  // clear any uart garbage
  clearSerialBuffer();
}

void loop()
{
  if (myEEPROM.vtxMode == TRAMP)
  {
    trampProcessSerial();
  }
  else
  {
    smartaudioProcessSerial();
  }

  writeEEPROM();
}
