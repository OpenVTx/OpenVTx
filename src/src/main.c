#include "targets.h"
#include "common.h"
#include "openVTxEEPROM.h"
#include "rtc6705.h"
#include "smartAudio.h"
#include "tramp.h"
#include "serial.h"
#include "errorCodes.h"
#include "gpio.h"

#ifdef LED_INDICATION_OF_VTX_MODE
#include "modeIndicator.h"
#endif

#if !DEBUG
static uint32_t protocol_checked;
#endif /* !DEBUG */

static void start_serial(uint8_t type)
{
  uint32_t baud, stopbits;
  switch (type) {
    case TRAMP:
      baud = TRAMP_BAUD;
      stopbits = 1;
      break;
    case SMARTAUDIO:
      baud = SMARTAUDIO_BAUD;
      stopbits = 2;
      break;
    default:
      baud = 115200;
      stopbits = 1;
      break;
  }
  serial_begin(baud, UART_TX, UART_RX, stopbits);
  myEEPROM.vtxMode = type;
}

void checkRTC6705isAlive()
{
  if (!rtc6705CheckFrequency())
  {
    rtc6705WriteFrequency(myEEPROM.currFreq); // Tries and set the correct freq to the RTC6705

    if (currentErrorMode == NO_ERROR)
    {
      currentErrorMode = RTC6705_NOT_DETECTED;
    }
  }
}

void setup(void)
{
  target_rfPowerAmpPinSetup();
  rtc6705spiPinSetup();

  readEEPROM();

  pitMode = myEEPROM.pitmodeInRange;

  rtc6705ResetState(); // During testing registers got messed up. So now it gets reset on boot!
  rtc6705WriteFrequency(myEEPROM.currFreq);

  start_serial(myEEPROM.vtxMode);

  status_leds_init();

#ifdef LED_INDICATION_OF_VTX_MODE
  resetModeIndication();
#endif /* LED_INDICATION_OF_VTX_MODE */
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

      if (FLIGHT_CONTROLLER_CHECK_TIMEOUT < now)
      {
        if (myEEPROM.vtxMode == TRAMP)
          trampBuildrPacket();
        else
          smartaudioBuildSettingsPacket();
      }
    }
  }
#endif /* DEBUG */

  /* Process uart data */
  if (myEEPROM.vtxMode == TRAMP)
    trampProcessSerial();
  else
    smartaudioProcessSerial();

  rtc6705PowerUpAfterPLLSettleTime();

  checkPowerOutput();

  checkRTC6705isAlive();

  errorCheck();

  writeEEPROM();

  target_loop();

#ifndef LED_INDICATION_OF_VTX_MODE
  status_led2(vtxModeLocked);
#else
    modeIndicationLoop();
#endif
}
