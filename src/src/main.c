#include "targets.h"
#include "common.h"
#include "openVTxEEPROM.h"
#include "rtc6705.h"
#include "smartAudio.h"
#include "tramp.h"
#include "printf.h"


static uint32_t protocol_checked;
static struct gpio_out led_pin;


void start_tramp(void)
{
  Serial_begin(TRAMP_BAUD, UART_TX, UART_RX);
  myEEPROM.vtxMode = TRAMP;
  updateEEPROM = 1;
}

void start_smart_audio(void)
{
  Serial_begin(SMARTAUDIO_BAUD, UART_TX, UART_RX);
  myEEPROM.vtxMode = SMARTAUDIO;
  updateEEPROM = 1;
}


void setup(void)
{
  rfPowerAmpPinSetup();

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

  start_smart_audio();

  // Below flashing is just for testing. Delete later.
  led_pin = gpio_out_setup(LED, 0);
  for (int i = 0; i < 10; i++)
  {
    gpio_out_write(led_pin, (i & 1));
    //fwdgt_counter_reload();
    delay(50);
  }

#if DEBUG
  setPowermW(0); // 0mV
  //setPowermW(25); // 1170mV
  //setPowermW(100); // 1225mV
  //setPowermW(400);
#endif /* DEBUG */
}


void loop(void)
{
  if (!DEBUG && !vtxModeLocked) {
    uint32_t now = millis();
    if (PROTOCOL_CHECK_TIMEOUT <= (now - protocol_checked)) {
      if (myEEPROM.vtxMode == TRAMP)
        start_smart_audio();
      else
        start_tramp();
      protocol_checked = now;
    }
  }

  /* Process uart data */
  if (myEEPROM.vtxMode == TRAMP)
    trampProcessSerial();
  else
    smartaudioProcessSerial();

  // writeEEPROM();

  taget_loop();
}
