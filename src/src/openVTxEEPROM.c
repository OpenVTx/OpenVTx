#include "common.h"
#include "openVTxEEPROM.h"
#include "targets.h"
#include <EEPROM.h>


openVTxEEPROM myEEPROM;
uint32_t updateEEPROMtime = 0;


void updateEEPROM(void)
{
    updateEEPROMtime = millis();
}

void defaultEEPROM(void)
{
    myEEPROM.version = versionEEPROM;
    myEEPROM.vtxMode = TRAMP;
    myEEPROM.currFreq = 5800;
    myEEPROM.channel = 27; // F4
    myEEPROM.freqMode = 0;
    myEEPROM.pitmodeInRange = 0;
    myEEPROM.pitmodeOutRange = 0;
    myEEPROM.currPowerdB = 14;
    myEEPROM.currPowermW = 25; // Required due to rounding errors when converting between dBm and mW
    myEEPROM.unlocked = 1;

    updateEEPROM();
}

void readEEPROM(void)
{
    EEPROM_get(0, myEEPROM);

    if (myEEPROM.version != versionEEPROM) {
        defaultEEPROM();
    }
}

void writeEEPROM(void)
{
    // Dont write if no protocol has been detected, as nothing has changed. 
    if (!vtxModeLocked)
        updateEEPROMtime = 0;

    if (updateEEPROMtime && (millis() - updateEEPROMtime) > 1000) {
        EEPROM_put(0, myEEPROM);
        updateEEPROMtime = 0;

        // Red LED blinks a couple of times as a visual indication.
        for (uint8_t i = 0; i < 4; i++){
            delay(50);
            status_led1(0);
            delay(50);
            status_led1(1);
        }
    }
}
