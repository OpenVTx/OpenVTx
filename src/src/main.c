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

#if DEBUG
  serial_begin(SMARTAUDIO_BAUD, UART_TX, UART_RX);
#else
  serial_begin(((type == TRAMP) ? TRAMP_BAUD : SMARTAUDIO_BAUD), UART_TX, UART_RX);
  myEEPROM.vtxMode = type;
  updateEEPROM = 1;
#endif /* !DEBUG */

void setup(void)
{
  target_rfPowerAmpPinSetup();
  spiPinSetup();

  readEEPROM();

  start_serial(myEEPROM.vtxMode);

  rtc6705ResetState(); // During testing registers got messed up. So now it gets reset on boot!
  rtc6705WriteFrequency(myEEPROM.currFreq);

  status_leds_init();

  // TODO DEBUG! Below flashing is just for testing. Delete later.
#if DEBUG
  myEEPROM.currFreq = 5600;
  rtc6705WriteFrequency(myEEPROM.currFreq);

  // target_set_power_dB(0);
  target_set_power_dB(14);
  // target_set_power_dB(20);
  // target_set_power_dB(26);
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
