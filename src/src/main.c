#include "targets.h"
#include "common.h"
#include "openVTxEEPROM.h"
#include "rtc6705.h"
#include "smartAudio.h"
#include "tramp.h"
#include <Arduino.h>

uint16_t dCycle;

void setup()
{
  rfPowerAmpPinSetup();
//   setPowerdB(0);

//   readEEPROM();
//   pitMode = (myEEPROM.pitmodeInRange || myEEPROM.pitmodeOutRange) ? 1 : 0;

  spiPinSetup();
  rtc6705ResetState(); // During testing registers got messed up. So now it gets reset on boot!
  rtc6705WriteFrequency(5800);
//   rtc6705WriteFrequency(myEEPROM.currFreq);

//   Serial_begin(myEEPROM.vtxMode == TRAMP ? TRAMP_BAUD : SMARTAUDIO_BAUD);

// #ifdef ARDUINO
//   while (!Serial)
//     ;
//   UART1_HalfDuplexCmd(ENABLE);
// #ifdef SERIAL_PIN
//   pinMode(SERIAL_PIN, INPUT_PULLUP);
// #endif
// #endif

//   // clear any uart garbage
//   clearSerialBuffer();


  Serial_begin(SMARTAUDIO_BAUD);

  // Below flashing is just for testing. Delete later.
  pinMode(LED, OUTPUT);
  for (int i = 0; i < 5; i++)
  {
    digitalWrite(LED, LOW);
    delay(50);
    digitalWrite(LED, HIGH);
    delay(50);
  }
}

void loop()
{
  // pwm_out_write(outputPowerTimer, 2999); // Here just for testing. 2999 = low mW, 0 = Max mW

  // digitalWrite(LED, HIGH);
  // delay(50);

  // digitalWrite(LED, LOW);
  // delay(50);

  // if (myEEPROM.vtxMode == TRAMP)
  // {
  //   trampProcessSerial();
  // }
  // else
  // {
    smartaudioProcessSerial();
  // }

  // writeEEPROM();
}
