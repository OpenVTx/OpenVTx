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
    5333, 5373, 5413, 5453, 5493, 5533, 5573, 5613  // L
*/
extern openVTxEEPROM myEEPROM;

mode_indicator_state_t indicating_state;

void getCurrentVtxState(mode_indicator_state_t *);
uint8_t getBlinksToMake(mode_indicator_state_t *);
void handleIndicationLogic(mode_indicator_state_t *, uint8_t);
indication_mode_type_t getNextIndicationModeType(indication_mode_type_t);
indication_mode_t getNextIndicationMode(indication_mode_t);

void resetModeIndication()
{
    indicating_state.mode = MODE;
    indicating_state.mode_type = BAND;
    indicating_state.blinks_done = 0;
    indicating_state.last_led_action_time = 0;
    indicating_state.led_is_on = 0;
}

void modeIndicationLoop()
{
    // get current vtx mode
    mode_indicator_state_t current_vtx_state;
    getCurrentVtxState(&current_vtx_state);

    // if state has been changed reset current indication and start from beginning
    if (current_vtx_state.channel != indicating_state.channel || current_vtx_state.currPowerdB != indicating_state.currPowerdB)
    {
        resetModeIndication();
        indicating_state.channel = current_vtx_state.channel;
        indicating_state.currPowerdB = current_vtx_state.currPowerdB;
        return;
    }
   
    handleIndicationLogic(&indicating_state, getBlinksToMake(&indicating_state));
}

void getCurrentVtxState(mode_indicator_state_t *s)
{
    s->channel = myEEPROM.channel;
    s->currPowerdB = myEEPROM.currPowerdB;

    return;
}

uint8_t getBlinksToMake(mode_indicator_state_t *s){
    uint8_t blinks_to_make = 0;
    if (s->mode == VALUE)
    {
        switch (s->mode_type)
        {
        case BAND:
            // each band has 8 fixed frequencies
            // doing this we define which band current channel belongs to
            blinks_to_make = s->channel / 8 + 1;
            break;

        case CHANNEL:
            // each band has 8 fixed frequencies
            // doing this we define channel number inside its ban
            blinks_to_make = s->channel % 8 + 1;
            break;

        case POWER:
            // 1 2 14 20 26
            // it's ugly but i don't want to spend MCU resources
            // on creating dectionary/hash map
            if (s->currPowerdB == 1)
            {
                blinks_to_make = 1;
            }
            else if (s->currPowerdB == 2)
            {
                blinks_to_make = 2;
            }
            else if (s->currPowerdB == 14)
            {
                blinks_to_make = 3;
            }
            else if (s->currPowerdB == 20)
            {
                blinks_to_make = 4;
            }
            else if (s->currPowerdB == 26)
            {
                blinks_to_make = 5;
            }
            break;

        default:
            break;
        }
    }
    else
    {
        if (s->mode_type == BAND)
        {
            blinks_to_make = 1;
        }
        else if (s->mode_type == CHANNEL)
        {
            blinks_to_make = 2;
        }
        else if (s->mode_type == POWER)
        {
            blinks_to_make = 3;
        }
    }
    return blinks_to_make;
}

void handleIndicationLogic(mode_indicator_state_t *s, uint8_t blinks)
{
    uint32_t now = millis();

    // turn off led if time has come
    if (s->led_is_on > 0)
    {
        if (s->mode == MODE && now - s->last_led_action_time >= LED_ON_DURATION)
        {
            SET_MODE_LED(0);
            s->led_is_on = 0;
            s->last_led_action_time = now;
            s->blinks_done++;
        }
        else if (s->mode == VALUE && now - s->last_led_action_time >= LED_ON_DURATION)
        {
            SET_VALUE_LED(0);
            s->led_is_on = 0;
            s->last_led_action_time = now;
            s->blinks_done++;
        }

        if (s->blinks_done >= blinks)
        {
            if (s->mode == VALUE)
            {
                s->mode_type = getNextIndicationModeType(s->mode_type);
                s->delay = INTER_MODE_DELAY * 2;
            }
            s->mode = getNextIndicationMode(s->mode);
            s->blinks_done = 0;
            s->delay = INTER_MODE_DELAY;
        }
    }
    // led is off
    else
    {
        if (now - s->last_led_action_time >= LED_OFF_DURATION + s->delay)
        {
            if (s->mode == MODE)
            {
                SET_MODE_LED(1);
            }
            else
            {
                SET_VALUE_LED(1);
            }
            s->last_led_action_time = now;
            s->led_is_on = 1;
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
    if (m == BAND)
        return CHANNEL;
    if (m == CHANNEL)
        return POWER;
    // if(m == POWER) return BAND;

    return BAND;
}