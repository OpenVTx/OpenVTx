#include "adc.h"
#include "Arduino.h"
#include "gd32f1x0_adc.h"
#include "gd32f1x0_gpio.h"
#include "gd32f1x0_rcu.h"

#define ADC_REF_VOLT_mW 3100

struct adc_pins {
    uint16_t pin;
    uint8_t adc;
};

struct adc_pins adc_pins[] = {
    {PA0, ADC_CHANNEL_0},
    {PA1, ADC_CHANNEL_1},
    {PA2, ADC_CHANNEL_2},
    {PA3, ADC_CHANNEL_3},
    {PA4, ADC_CHANNEL_4},
    {PA5, ADC_CHANNEL_5},
    {PA6, ADC_CHANNEL_6},
    {PA7, ADC_CHANNEL_7},
    {PB0, ADC_CHANNEL_8},
    {PB1, ADC_CHANNEL_9},
    {PC0, ADC_CHANNEL_10},
    {PC1, ADC_CHANNEL_11},
    {PC2, ADC_CHANNEL_12},
    {PC3, ADC_CHANNEL_13},
    {PC4, ADC_CHANNEL_14},
    {PC5, ADC_CHANNEL_15},
};

static uint8_t init_done;

static void init_adc(void)
{
    if (init_done)
        return;
    init_done = 1;

    /* enable ADC clock */
    rcu_periph_clock_enable(RCU_ADC);
    /* config ADC clock */
    rcu_adc_clock_config(RCU_ADCCK_APB2_DIV6);

    /* ADC regular channel length config */
    adc_channel_length_config(ADC_REGULAR_CHANNEL, 1);
    /* ADC external trigger enable */
    adc_external_trigger_config(ADC_REGULAR_CHANNEL, ENABLE);
    /* ADC external trigger source config */
    adc_external_trigger_source_config(
        ADC_REGULAR_CHANNEL, ADC_EXTTRIG_REGULAR_SWRCST);
    /* ADC data alignment config */
    adc_data_alignment_config(ADC_DATAALIGN_RIGHT);
    /* enable ADC interface */
    adc_enable();
    /* ADC calibration and reset calibration */
    adc_calibration_enable();
    /* ADC contineous function enable */
    adc_special_function_config(ADC_CONTINUOUS_MODE, ENABLE);

    /* ADC software trigger enable */
    adc_software_trigger_enable(ADC_REGULAR_CHANNEL);
}

struct adc adc_config(uint32_t pin)
{
    uint32_t adc_ch;
    for (adc_ch = 0; adc_ch < ARRAY_SIZE(adc_pins); adc_ch++)
        if (adc_pins[adc_ch].pin == pin)
            break;

    if (ARRAY_SIZE(adc_pins) <= adc_ch)
        return (struct adc){.ch = 0xff};

    //adc_ch = adc_pins[adc_ch].adc;

    pinMode(pin, ANALOG);

    init_adc();

    return (struct adc){.ch = adc_ch};
}

uint32_t adc_read(struct adc config)
{
    if (config.ch < 0xff) {
        adc_regular_channel_config(0, config.ch, ADC_SAMPLETIME_239POINT5);
        /* Clear flag */
        adc_flag_clear(ADC_FLAG_EOC);
        /* Wait ready */
        while(SET != adc_flag_get(ADC_FLAG_EOC));
        /* Read value */
        uint32_t val = adc_regular_data_read();
        val *= ADC_REF_VOLT_mW;
        val /= 4096;
        return val;
    }
    return 0;
}
