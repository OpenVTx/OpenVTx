#include "adc.h"
#include "Arduino.h"
#include "gd32f1x0_adc.h"
#include "gd32f1x0_gpio.h"
#include "gd32f1x0_rcu.h"

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

static uint8_t adc_ch_cnt, init_done;

static void init_adc(void)
{
    if (init_done)
        return;

    /* enable ADC clock */
    rcu_periph_clock_enable(RCU_ADC);
    /* config ADC clock */
    rcu_adc_clock_config(RCU_ADCCK_APB2_DIV6);

    /* Stop ADC */
    adc_disable();
    /* Disable discontinue mode */
    adc_discontinuous_mode_config(ADC_REGULAR_CHANNEL, 1);
    adc_special_function_config(ADC_CONTINUOUS_MODE, DISABLE);
    /* ADC software trigger config */
    adc_external_trigger_source_config(
        ADC_REGULAR_CHANNEL, ADC_EXTTRIG_REGULAR_SWRCST);
    /* ADC data alignment config */
    adc_data_alignment_config(ADC_DATAALIGN_RIGHT);

    adc_external_trigger_config(ADC_REGULAR_CHANNEL, ENABLE);
    //adc_software_trigger_enable(ADC_REGULAR_CHANNEL);

    adc_enable();
    delay(1);
    adc_calibration_enable();
}

struct adc adc_config(uint32_t pin)
{
    uint32_t adc_ch;
    for (adc_ch = 0; adc_ch < ARRAY_SIZE(adc_pins); adc_ch++)
        if (adc_pins[adc_ch].pin == pin)
            break;

    if (ARRAY_SIZE(adc_pins) <= adc_ch)
        return (struct adc){.ch = 0xff};

    adc_ch = adc_pins[adc_ch].adc;

    pinMode(pin, ANALOG);

#if 0
    if (!adc_ch_cnt) {
        /* enable ADC clock */
        rcu_periph_clock_enable(RCU_ADC);
        /* config ADC clock */
        rcu_adc_clock_config(RCU_ADCCK_APB2_DIV6);

        /* Stop ADC */
        adc_disable();
        /* ADC contineous function enable */
        adc_special_function_config(ADC_CONTINUOUS_MODE, ENABLE);
        /* ADC software trigger config */
        adc_external_trigger_source_config(
            ADC_REGULAR_CHANNEL, ADC_EXTTRIG_REGULAR_SWRCST);
        /* ADC data alignment config */
        adc_data_alignment_config(ADC_DATAALIGN_RIGHT);
    }

    /* ADC channel length config */
    adc_channel_length_config(ADC_REGULAR_CHANNEL, adc_ch_cnt+1);
    /* ADC regular channel config */
    adc_regular_channel_config(
        adc_ch_cnt, adc_ch, ADC_SAMPLETIME_55POINT5);
    adc_external_trigger_config(ADC_REGULAR_CHANNEL, ENABLE);
    adc_software_trigger_enable(ADC_REGULAR_CHANNEL);

    if (!adc_ch_cnt) {
        adc_enable();
        delay(1);
        adc_calibration_enable();
    }
    adc_ch_cnt++;
#else
    (void)adc_ch_cnt;

    init_adc();
#endif
    return (struct adc){.ch = adc_ch};
}

uint32_t adc_read(struct adc config)
{
    if (config.ch < 0xff) {
        /* Set channel */
        //adc_channel_length_config(ADC_REGULAR_CHANNEL, config.ch);
        adc_regular_channel_config(
            15, config.ch, ADC_SAMPLETIME_55POINT5);
        /* Clear flag */
        adc_flag_clear(ADC_FLAG_EOC);
        /* Trigger conversion */
        adc_software_trigger_enable(ADC_REGULAR_CHANNEL);
        /* Wait ready */
        while(SET != adc_flag_get(ADC_FLAG_EOC));
        /* Read value */
        return adc_regular_data_read();
    }
    return 0;
}
