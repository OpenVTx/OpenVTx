#include "targets.h"
#include "common.h"
#include "openVTxEEPROM.h"
#include "rtc6705.h"
#include "smartAudio.h"
#include "tramp.h"
#include "mspVtx.h"
#include "serial.h"
#include "errorCodes.h"
#include "gpio.h"


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
    case MSP:
      baud = MSP_BAUD;
      stopbits = 1;
      break;
    default:
      baud = 115200;
      stopbits = 1;
      break;
  }
  serial_begin(baud, UART_TX, UART_RX, stopbits);
  myEEPROM.vtxMode = type;
  updateEEPROM = 1;
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

  // readEEPROM();
  defaultEEPROM();

  pitMode = myEEPROM.pitmodeInRange;

  rtc6705ResetState(); // During testing registers got messed up. So now it gets reset on boot!
  rtc6705WriteFrequency(myEEPROM.currFreq);

  start_serial(myEEPROM.vtxMode);

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
  uint32_t now = millis();

#if !DEBUG
  if (!vtxModeLocked)
  {
    if (PROTOCOL_CHECK_TIMEOUT <= (now - protocol_checked))
    {
      uint8_t mode = (uint8_t)myEEPROM.vtxMode;
      mode = (mode + 1) % VTX_MODE_MAX;
      start_serial((vtxMode_e)mode);
      protocol_checked = now;

      if (FLIGHT_CONTROLLER_CHECK_TIMEOUT < now)
      {
          switch (myEEPROM.vtxMode)
          {
          case TRAMP:
            trampBuildrPacket();
            break;
          case SMARTAUDIO:
            smartaudioBuildSettingsPacket();
            break;
          default:
            break;
          }

        // if (myEEPROM.vtxMode == TRAMP)
        //   trampBuildrPacket();
        // else
        //   smartaudioBuildSettingsPacket();
      }
    }
  }
#endif /* DEBUG */

  /* Process uart data */
  switch (myEEPROM.vtxMode)
  {
  case TRAMP:
    trampProcessSerial();
    break;
  case SMARTAUDIO:
    smartaudioProcessSerial();
    break;
  case MSP:
    mspQueryFlightController(now);
    mspProcessSerial();
    break;
  default:
    break;
  }

  rtc6705PowerUpAfterPLLSettleTime();

  checkPowerOutput();

  checkRTC6705isAlive();

  errorCheck();

  // writeEEPROM();

  taget_loop();
  status_led2(vtxModeLocked);
}
