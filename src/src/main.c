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
  uint32_t baud, stopbits = 2;
  switch (type) {
    case TRAMP:
      baud = TRAMP_BAUD;
      stopbits = 1;
      break;
    case SMARTAUDIO:
      baud = SMARTAUDIO_BAUD;
      break;
    default:
      baud = 115200;
      break;
  }
  serial_begin(baud, UART_TX, UART_RX, stopbits);
  myEEPROM.vtxMode = type;
  updateEEPROM = 1;
}

void setup(void)
{
  target_rfPowerAmpPinSetup();
  spiPinSetup();

  // readEEPROM();
  defaultEEPROM();

  pitMode = myEEPROM.pitmodeInRange;

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
      uint8_t mode = (uint8_t)myEEPROM.vtxMode;
      mode = (mode + 1) % VTX_MODE_MAX;
      start_serial((vtxMode_e)mode);
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

  // writeEEPROM();

  taget_loop();
  status_led2(vtxModeLocked);
}
