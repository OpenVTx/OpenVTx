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

static void start_serial(uint8_t type)
{
  serial_begin(((type == TRAMP) ? TRAMP_BAUD : SMARTAUDIO_BAUD), UART_TX, UART_RX);
  myEEPROM.vtxMode = type;
  updateEEPROM = 1;
}

void setup(void)
{
  target_rfPowerAmpPinSetup();
  spiPinSetup();

  readEEPROM();

  myEEPROM.vtxMode = SMARTAUDIO;
  start_serial(myEEPROM.vtxMode);

  rtc6705ResetState(); // During testing registers got messed up. So now it gets reset on boot!
  rtc6705WriteFrequency(myEEPROM.currFreq);

  status_leds_init();

  // TODO DEBUG! Below flashing is just for testing. Delete later.
#if DEBUG
  // target_set_power_mW(0);
  // target_set_power_mW(25);
  // target_set_power_mW(100);
  // target_set_power_mW(400);
#endif /* DEBUG */
}

void loop(void)
{

#if !DEBUG
  if (!vtxModeLocked)
  {
    uint32_t now = millis();
    if (PROTOCOL_CHECK_TIMEOUT <= (now - protocol_checked))
    {
      start_serial((myEEPROM.vtxMode == TRAMP ? SMARTAUDIO: TRAMP));  
      protocol_checked = now;
    }
  }
#endif /* DEBUG */

  /* Process uart data */
  if (myEEPROM.vtxMode == TRAMP)
    trampProcessSerial();
  else
    smartaudioProcessSerial();

  checkPowerOutput();

  writeEEPROM();

  taget_loop();
  status_led2(vtxModeLocked);
}
