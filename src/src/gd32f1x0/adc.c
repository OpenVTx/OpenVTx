#include "gpio.h"
#include "helpers.h"
#include "gd32f1x0_adc.h"
#include "gd32f1x0_gpio.h"
#include "gd32f1x0_rcu.h"

#define ADC_REF_VOLT_mW 3100

uint16_t adc_pins[] = {
    PA0, PA1, PA2, PA3, PA4, PA5, PA6, PA7,
    PB0, PB1,
    PC0, PC1, PC2, PC3, PC4, PC5,
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

gpio_adc_t adc_config(uint32_t pin)
{
    uint32_t adc_ch;
    for (adc_ch = 0; adc_ch < ARRAY_SIZE(adc_pins); adc_ch++)
        if (adc_pins[adc_ch] == pin)
            break;

    if (ARRAY_SIZE(adc_pins) <= adc_ch)
        return (gpio_adc_t){.ch = 0xff};

    uint32_t gpio_periph = GPIO_BASE + 0x400 * GPIO2PORT(pin);
    uint32_t gpio_pin = GPIO2BIT(pin);
    /* Enable the clock */
    rcu_periph_clock_enable(RCU_REGIDX_BIT(IDX_AHBEN, (17U + GPIO2PORT(pin))));
    /* Config pin to analog input */
    gpio_mode_set(gpio_periph, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, gpio_pin);

    init_adc();

    return (gpio_adc_t){.ch = adc_ch};
}

uint32_t adc_read(gpio_adc_t config)
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
