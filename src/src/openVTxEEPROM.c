#include "openVTxEEPROM.h"
#include "targets.h"
#include <EEPROM.h>


openVTxEEPROM myEEPROM;
uint32_t eeprom_last_write;
uint8_t updateEEPROM;


void defaultEEPROM(void)
{
    myEEPROM.version = versionEEPROM;
    myEEPROM.vtxMode = TRAMP;
    myEEPROM.currFreq = BOOT_FREQ;
    myEEPROM.channel = 255;
    myEEPROM.freqMode = 0;
    myEEPROM.pitmodeInRange = 0;
    myEEPROM.pitmodeOutRange = 0;
    myEEPROM.currPowerdB = 0;
    myEEPROM.currPowermW = 0; // Required due to rounding errors when converting between dBm and mW
    myEEPROM.unlocked = 1;

    updateEEPROM = 1;
}

void readEEPROM(void)
{
    EEPROM_get(0, myEEPROM);

    if (myEEPROM.version != versionEEPROM) {
        defaultEEPROM();
    }
    eeprom_last_write = millis();
}

void writeEEPROM(void)
{
    uint32_t const now = millis();
    if (updateEEPROM && 1000 <= (now - eeprom_last_write)) {
        EEPROM_put(0, myEEPROM);
        updateEEPROM = 0;
        eeprom_last_write = now;
    }
}
