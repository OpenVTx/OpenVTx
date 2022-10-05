#include "button.h"
#include "targets.h"
#include "common.h"
#include "rtc6705.h"
#include "helpers.h"
#include "openVTxEEPROM.h"

#ifdef LED_INDICATION_OF_VTX_MODE
#include "modeIndicator.h"
#endif

#define DEBOUNCE_TIME   50
#define LONG_PRESS_TIME 1000
#define DEFAULT_TIME    10000

enum
{
  BUTTON_PRESSED,
  BUTTON_RELEASED
};

static gpio_in_t buttonPin;

uint32_t pressed_time_ms = 0;
uint32_t released_time_ms = 0;
uint8_t current_button_state = BUTTON_RELEASED;
uint8_t previous_button_state = BUTTON_RELEASED;

void button_init(void)
{
    buttonPin = gpio_in_setup(BUTTON, 0);
}

void checkButton(void)
{
    uint32_t now = millis();
    previous_button_state = current_button_state;
    current_button_state = gpio_in_read(buttonPin);

    // Button pressed
    if (current_button_state == BUTTON_PRESSED && previous_button_state == BUTTON_RELEASED)
    {
        pressed_time_ms = now;
    } else
    // Button released
    if (current_button_state == BUTTON_RELEASED && previous_button_state == BUTTON_PRESSED)
    {
        vtxModeLocked = 1;

        uint32_t elapsedTime = now - pressed_time_ms;

        // debounce
        if (elapsedTime < DEBOUNCE_TIME)
        {
            return;
        } else
        // short press
        if (elapsedTime < LONG_PRESS_TIME)
        {
            myEEPROM.freqMode = 0;
            myEEPROM.channel = (myEEPROM.channel + 1) % FREQ_TABLE_SIZE;
            rtc6705WriteFrequency(channelFreqTable[myEEPROM.channel]);
        } else
        // long press
        if (elapsedTime < DEFAULT_TIME)
        {
            uint8_t idx = 0;
            for (idx = 0; idx < SA_NUM_POWER_LEVELS; idx++)
            {
                if (myEEPROM.currPowerdB == saPowerLevelsLut[idx]) break;
            }
            myEEPROM.currPowerdB = saPowerLevelsLut[(idx + 1) % SA_NUM_POWER_LEVELS];
            setPowerdB(myEEPROM.currPowerdB);
        } else
        // default EEPROM
        {
            defaultEEPROM();
            rtc6705WriteFrequency(channelFreqTable[myEEPROM.channel]);
        }

        resetModeIndication();
    }
}