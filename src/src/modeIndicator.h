#pragma once

#include <stdint.h>
#include "common.h"

#define SET_MODE_LED(x) status_led2(x)
#define SET_VALUE_LED(x) status_led3(x)

#define LED_ON_DURATION 100
#define LED_OFF_DURATION 600
#define INTER_MODE_DELAY 300

typedef enum {
    BAND,
    CHANNEL,
    POWER
} indication_mode_type_t;

typedef enum {
    MODE,
    VALUE
} indication_mode_t;

typedef struct {
    indication_mode_t mode;
    indication_mode_type_t mode_type;
    uint8_t blinks_done;
    uint32_t last_led_action_time;
    uint8_t led_is_on;
    uint32_t delay; 

    uint8_t channel;
    float currPowerdB;
} mode_indicator_state_t;

void modeIndicationLoop();