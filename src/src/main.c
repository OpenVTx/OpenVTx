#include "targets.h"
#include "common.h"
#include "openVTxEEPROM.h"
#include "rtc6705.h"
#include "smartAudio.h"
#include "tramp.h"
#include "serial.h"
#include "gpio.h"


#if !DEBUG
static uint32_t protocol_checked;
#endif /* !DEBUG */
static gpio_out_t led_pin;


static void start_serial(uint8_t type)
{
  serial_begin(((type == TRAMP) ? TRAMP_BAUD : SMARTAUDIO_BAUD), UART_TX, UART_RX);
  myEEPROM.vtxMode = type;
  updateEEPROM = 1;
}


void setup(void)
{
  rfPowerAmpPinSetup();

  readEEPROM();

  /* TODO DEBUG! */
  myEEPROM.vtxMode = SMARTAUDIO;

  //pitMode = (myEEPROM.pitmodeInRange || myEEPROM.pitmodeOutRange) ? 1 : 0;

  spiPinSetup();
  rtc6705ResetState(); // During testing registers got messed up. So now it gets reset on boot!
  rtc6705WriteFrequency(myEEPROM.currFreq);

  start_serial(myEEPROM.vtxMode);

// #ifdef LED
  led_pin = gpio_out_setup(LED1, 1);
  // for (int i = 0; i < 10; i++) {
  //   gpio_out_write(led_pin, (i & 1));
  //   //fwdgt_counter_reload();
  //   delay(50);
  // }
// #endif // LED

  // TODO DEBUG! Below flashing is just for testing. Delete later.
#if DEBUG
  setPowermW(0); // 0mV
  //setPowermW(25); // 1170mV
  //setPowermW(100); // 1225mV
  //setPowermW(400);
#endif /* DEBUG */
}


void loop(void)
{
  uint8_t const mode = myEEPROM.vtxMode;
#if !DEBUG
  if (!vtxModeLocked) {
    uint32_t now = millis();
    if (PROTOCOL_CHECK_TIMEOUT <= (now - protocol_checked)) {
      start_serial((mode == TRAMP ? SMARTAUDIO: TRAMP));
      protocol_checked = now;
      return;
    }
  }
#endif /* DEBUG */

  /* Process uart data */
  if (mode == TRAMP)
    trampProcessSerial();
  else
    smartaudioProcessSerial();

  writeEEPROM();

  taget_loop();
}
