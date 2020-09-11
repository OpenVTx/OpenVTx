#define versionEEPROM 0

bool updateEEPROM = false;

typedef enum
{
  TRAMP,
  SMARTAUDIO
} vtxMode_e;

typedef struct
{
    uint16_t version;    
    vtxMode_e vtxMode;
    uint16_t currFreq;
    uint8_t channel;
    uint8_t freqMode;
    uint8_t pitmodeInRange;
    uint8_t pitmodeOutRange;
    uint16_t currPowermW;
    uint8_t currPowerdB;
    uint8_t currPowerIndex;
    uint8_t unlocked;
} openVTxEEPROM;

openVTxEEPROM myEEPROM;

void defaultEEPROM()
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
    
void readEEPROM()
{
    EEPROM_get(0, myEEPROM);

    if (myEEPROM.version != versionEEPROM)
    {
        defaultEEPROM();
    }    
}
    
void writeEEPROM()
{
    if (updateEEPROM)
    {
        EEPROM_put(0, myEEPROM);
        updateEEPROM = false;
    }    
}