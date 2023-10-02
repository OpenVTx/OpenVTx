#include "modeIndicator.h"
#include "openVTxEEPROM.h"
#include "targets.h"

/*
    Channel frequency table for reference
    5865, 5845, 5825, 5805, 5785, 5765, 5745, 5725, // A
    5733, 5752, 5771, 5790, 5809, 5828, 5847, 5866, // B
    5705, 5685, 5665, 5645, 5885, 5905, 5925, 5945, // E
    5740, 5760, 5780, 5800, 5820, 5840, 5860, 5880, // F
    5658, 5695, 5732, 5769, 5806, 5843, 5880, 5917, // R
    5362, 5399, 5436, 5473, 5510, 5547, 5584, 5621  // LowRace
*/
extern openVTxEEPROM myEEPROM;

mode_indicator_state_t indicatingState;

void getCurrentVtxState(mode_indicator_state_t *);
uint8_t getBlinksToMake(mode_indicator_state_t *);
void handleIndicationLogic(mode_indicator_state_t *, uint8_t);
indication_mode_type_t getNextIndicationModeType(indication_mode_type_t);
indication_mode_t getNextIndicationMode(indication_mode_t);

void resetModeIndication()
{
    indicatingState.mode = MODE;
    indicatingState.modeType = IND_BAND;
    indicatingState.blinksDone = 0;
    indicatingState.lastLedActionTime = 0;
    indicatingState.ledIsOn = 0;
    indicatingState.channel = 0;
    indicatingState.currPowerdB = -1;
}

void modeIndicationLoop()
{
    // get current vtx mode
    mode_indicator_state_t currentVtxState;
    getCurrentVtxState(&currentVtxState);

    // if state has been changed reset current indication and start from beginning
    if (currentVtxState.channel != indicatingState.channel || currentVtxState.currPowerdB != indicatingState.currPowerdB)
    {  
        indicatingState.channel = currentVtxState.channel;
        indicatingState.currPowerdB = currentVtxState.currPowerdB;
    }

    handleIndicationLogic(&indicatingState, getBlinksToMake(&indicatingState));
}

void getCurrentVtxState(mode_indicator_state_t *s)
{
    if (s == NULL)
    {
        return;
    }
    s->channel = myEEPROM.channel;
    s->currPowerdB = myEEPROM.currPowerdB;
}

uint8_t getBlinksToMake(mode_indicator_state_t *s)
{
    if (s == NULL)
    {
        return 0;
    }
    uint8_t blinksToDo = 0;
    if (s->mode == VALUE)
    {
        switch (s->modeType)
        {
        case IND_BAND:
            // each band has 8 fixed frequencies
            // doing this we define which band current channel belongs to
            blinksToDo = s->channel / 8 + 1;
            break;

        case IND_CHANNEL:
            // each band has 8 fixed frequencies
            // doing this we define channel number inside its ban
            blinksToDo = s->channel % 8 + 1;
            break;

        case IND_POWER:
            // 1 2 14 20 26
            // it's ugly but i don't want to spend MCU resources
            // on creating dectionary/hash map
            if (s->currPowerdB == 1)
            {
                blinksToDo = 1;
            }
            else if (s->currPowerdB == 2)
            {
                blinksToDo = 2;
            }
            else if (s->currPowerdB == 14)
            {
                blinksToDo = 3;
            }
            else if (s->currPowerdB == 20)
            {
                blinksToDo = 4;
            }
            else if (s->currPowerdB == 26)
            {
                blinksToDo = 5;
            }
            break;

        default:
            break;
        }
    }
    else
    {
        if (s->modeType == IND_BAND)
        {
            blinksToDo = 1;
        }
        else if (s->modeType == IND_CHANNEL)
        {
            blinksToDo = 2;
        }
        else if (s->modeType == IND_POWER)
        {
            blinksToDo = 3;
        }
    }
    return blinksToDo;
}

void handleIndicationLogic(mode_indicator_state_t *s, uint8_t blinks)
{
    if (s == NULL)
    {
        return;
    }

    uint32_t now = millis();

    // turn off led if time has come
    if (s->ledIsOn > 0)
    {
        if (s->mode == MODE && now - s->lastLedActionTime >= LED_ON_DURATION)
        {
            SET_MODE_LED(0);
            s->ledIsOn = 0;
            s->lastLedActionTime = now;
            s->blinksDone++;
        }
        else if (s->mode == VALUE && now - s->lastLedActionTime >= LED_ON_DURATION)
        {
            SET_VALUE_LED(0);
            s->ledIsOn = 0;
            s->lastLedActionTime = now;
            s->blinksDone++;
        }

        if (s->blinksDone >= blinks)
        {
            if (s->mode == VALUE)
            {
                s->modeType = getNextIndicationModeType(s->modeType);
                s->delay = INTER_MODE_DELAY * 2;
            }
            s->mode = getNextIndicationMode(s->mode);
            s->blinksDone = 0;
            s->delay = INTER_MODE_DELAY;
        }
    }
    // led is off
    else
    {
        if (now - s->lastLedActionTime >= LED_OFF_DURATION + s->delay)
        {
            if (s->mode == MODE)
            {
                SET_MODE_LED(1);
            }
            else
            {
                SET_VALUE_LED(1);
            }
            s->lastLedActionTime = now;
            s->ledIsOn = 1;
            // delay is a one shot thing. used to add additional delay after switching modes
            s->delay = 0;
        }
    }
}

indication_mode_t getNextIndicationMode(indication_mode_t m)
{
    if (m == MODE)
        return VALUE;
    return MODE;
}

indication_mode_type_t getNextIndicationModeType(indication_mode_type_t m)
{
    if (m == IND_BAND)
        return IND_CHANNEL;
    if (m == IND_CHANNEL)
        return IND_POWER;
    // if(m == IND_POWER) return IND_BAND;

    return IND_BAND;
}