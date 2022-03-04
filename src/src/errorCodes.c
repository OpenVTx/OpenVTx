#include "errorCodes.h"
#include "targets.h"
#include "common.h"
#include "helpers.h"

// LED blink period is in ms / 10
// Final index 250 is a 2.5s LED off period before repeating the cycle
static const uint8_t rtc6705NotDetectedime[8] = {100, 50, 10, 50, 10, 50, 10, 250};  // 1s on to indicate start of error code, followed by  3 x 0.1s short flashes.
static const uint8_t power5vNotDetectedime[6] = {100, 50, 10, 50, 10, 250};          // 1s on to indicate start of error code, followed by  2 x 0.1s short flashes.
static const uint8_t power3v3NotDetectedime[4] = {100, 50, 10, 250};                 // 1s on to indicate start of error code, followed by  1 x 0.1s short flashe.

uint8_t currentErrorMode = NO_ERROR;
uint8_t errorIndex;
uint32_t errorTime;

void errorCheck(void)
{
    if (currentErrorMode == NO_ERROR)
    {
        status_led1(1); // Default LED on and no error
        return;
    }

    uint32_t now = millis();
    if (now > errorTime)
    {
        errorIndex++;

        switch (currentErrorMode) {
            case RTC6705_NOT_DETECTED:
                errorTime = rtc6705NotDetectedime[errorIndex % ARRAY_SIZE(rtc6705NotDetectedime)];
            break;
            case POWER_5V_NOT_DETECTED:
                errorTime = power5vNotDetectedime[errorIndex % ARRAY_SIZE(power5vNotDetectedime)];
            break;
            case POWER_3V3_NOT_DETECTED:
                errorTime = power3v3NotDetectedime[errorIndex % ARRAY_SIZE(power3v3NotDetectedime)];
            break;
            default:
                return;
            break;
        }

        errorTime *= 10;
        errorTime += now;

        status_led1(!(errorIndex % 2));
    }
}
