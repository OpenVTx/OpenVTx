#include "openVTxEEPROM.h"
#include <EEPROM.h>

uint8_t updateEEPROM;

openVTxEEPROM myEEPROM;

void defaultEEPROM(void)
{
    myEEPROM.version = versionEEPROM;
    myEEPROM.vtxMode = SMARTAUDIO;
    myEEPROM.currFreq = 5800;
    myEEPROM.channel = 27;
    myEEPROM.freqMode = 0;
    myEEPROM.pitmodeInRange = 0;
    myEEPROM.pitmodeOutRange = 0;
    myEEPROM.currPowermW = 25;
    myEEPROM.currPowerdB = 14;
    myEEPROM.currPowerIndex = 1;
    myEEPROM.unlocked = 1;

    EEPROM_put(0, myEEPROM);
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
    if (updateEEPROM) {
        EEPROM_put(0, myEEPROM);
        updateEEPROM = 0;
    }
}
