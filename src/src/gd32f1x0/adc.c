#include "adc.h"
#include "Arduino.h"
#include "gd32f1x0_adc.h"
#include "gd32f1x0_gpio.h"
#include "gd32f1x0_rcu.h"

uint16_t adc_pins[] = {PA0, PA1, PA2, PA3, PA4, PA5, PA6, PA7, PB0, PB1};

static uint8_t adc_initialized;

struct adc adc_config(uint32_t pin)
{
    uint8_t adc_ch;
    for (adc_ch = 0; adc_ch < ARRAY_SIZE(adc_pins); adc_ch++)
      if (adc_pins[adc_ch] == pin)
        break;
    if (ARRAY_SIZE(adc_pins) <= adc_ch)
        return (struct adc){.ch = 0xff};

    pinMode(pin, ANALOG);

    /* ADC contineous function enable */
    adc_special_function_config(ADC_CONTINUOUS_MODE, ENABLE);
    /* ADC software trigger config */
    adc_external_trigger_source_config(ADC_REGULAR_CHANNEL,
                                       ADC_EXTTRIG_REGULAR_SWRCST);
    /* ADC data alignment config */
    adc_data_alignment_config(ADC_DATAALIGN_RIGHT);
    /* ADC channel length config */
    adc_channel_length_config(ADC_REGULAR_CHANNEL, 1);
    /* ADC regular channel config */
    adc_regular_channel_config(adc_ch, adc_ch, ADC_SAMPLETIME_55POINT5);
    adc_external_trigger_config(ADC_REGULAR_CHANNEL, ENABLE);
    adc_software_trigger_enable(ADC_REGULAR_CHANNEL);

    if (!adc_initialized) {
        /* enable ADC clock */
        rcu_periph_clock_enable(RCU_ADC);
        /* config ADC clock */
        rcu_adc_clock_config(RCU_ADCCK_APB2_DIV6);

        adc_enable();
        delay(1);
        adc_calibration_enable();
        adc_initialized = 1;
    }
    return (struct adc){.ch = adc_ch};
}

uint32_t adc_read(struct adc config)
{
    if (config.ch < 0xff) {
        /* TODO: set channel */

        adc_flag_clear(ADC_FLAG_EOC);
        while(SET != adc_flag_get(ADC_FLAG_EOC));
        return ADC_RDATA;
    }
    return 0;
}
