#include "gpio.h"
#include "helpers.h"
#include "gd32f1x0_adc.h"
#include "gd32f1x0_gpio.h"
#include "gd32f1x0_rcu.h"

#define ADC_USE_ISR 1

#if ADC_USE_ISR
#define ADC_REF_VOLT_mW 3500
#else
#define ADC_REF_VOLT_mW 3100
#endif

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

#if ADC_USE_ISR
void adc_isr_config(uint32_t ch)
{
    adc_regular_channel_config(0, ch, ADC_SAMPLETIME_239POINT5);

    /* Enable end of group conversion interrupt */
    adc_interrupt_enable(ADC_INT_EOC);
    /* Enable ISR */
    nvic_irq_enable(ADC_CMP_IRQn, 0, 0);
}

static volatile uint_fast16_t conv_results[4];
static uint_fast8_t conv_idx;

/* Analog watchdog triggered */
void ADC_CMP_IRQHandler(void)
{
    if (adc_flag_get(ADC_FLAG_EOC)) {
        /* clear the ADC interrupt or status flag */
        adc_interrupt_flag_clear(ADC_FLAG_EOC);

        conv_results[conv_idx++] = adc_regular_data_read();
        conv_idx %= ARRAY_SIZE(conv_results);
    }
}
#endif

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

#if ADC_USE_ISR
    adc_isr_config(adc_ch);
#else
    adc_regular_channel_config(0, adc_ch, ADC_SAMPLETIME_239POINT5);
#endif

    return (gpio_adc_t){.ch = adc_ch};
}

uint32_t adc_read(gpio_adc_t config)
{
#if !ADC_USE_ISR
    if (config.ch < 0xff) {
        /* Wait ready */
        while(SET != adc_flag_get(ADC_FLAG_EOC));
        /* Read value */
        uint32_t val = adc_regular_data_read();
        val *= ADC_REF_VOLT_mW;
        val /= 4096;
        /* Clear flag */
        adc_flag_clear(ADC_FLAG_EOC);
        return val;
    }
    return 0;
#else
    uint32_t temp = 0;
    for (uint8_t iter = 0; iter < ARRAY_SIZE(conv_results); iter++) {
        temp += conv_results[iter];
    }
    temp /= ARRAY_SIZE(conv_results);
    temp *= ADC_REF_VOLT_mW;
    temp /= 4096;
    return temp;
#endif
}


#if 0
void adc_watchdog_init(uint16_t min, uint16_t max, uint8_t channel)
{
    /* ADC analog watchdog threshold config */
    adc_watchdog_threshold_config(min, max);
    /* ADC analog watchdog single channel config */
    adc_watchdog_single_channel_enable(channel);

    /* Enable analog watchdog interrupt */
    adc_interrupt_enable(ADC_INT_WDE);

    nvic_irq_enable(ADC_CMP_IRQn, 0, 0);
}

/* Analog watchdog triggered */
void ADC_CMP_IRQHandler(void)
{
    /* clear the ADC interrupt or status flag */
    adc_interrupt_flag_clear(ADC_FLAG_WDE);
}
#endif
