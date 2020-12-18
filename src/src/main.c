#include "targets.h"
#include "common.h"
#include "openVTxEEPROM.h"
#include "rtc6705.h"
#include "smartAudio.h"
#include "tramp.h"
#include <Arduino.h>

uint16_t dCycle;
struct adc adc_pin;

void setup(void)
{
  /* TODO: Configure WDG, fwdgt_config() */

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

  adc_pin = adc_config(VPD);

  Serial_begin(SMARTAUDIO_BAUD);

  // Below flashing is just for testing. Delete later.
  pinMode(LED, OUTPUT);
  for (int i = 0; i < 10; i++)
  {
    digitalWrite(LED, (i & 1));
    fwdgt_counter_reload();
    delay(50);
  }
}

static uint32_t temp;
void loop(void)
{
  uint32_t now = millis();
  if (1000 <= (now - temp)) {
    char buff[32];
    int len = snprintf(buff, sizeof(buff), "a:%u\n", adc_read(adc_pin));
    Serial_write_len((uint8_t*)buff, len);
    temp = now;
  }

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

  /* Reset WD */
  fwdgt_counter_reload();
}
