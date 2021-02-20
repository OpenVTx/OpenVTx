#include "gpio.h"
#include <Arduino.h>

gpio_out_t gpio_out_setup(uint32_t pin, uint32_t val)
{
    if (160 <= pin)
        return GPIO_PIN_IVALID;
    pinMode(pin, OUTPUT);
    digitalWrite(pin, val);
    return (gpio_in_t)pin;
}

void gpio_out_toggle(gpio_out_t pin)
{
    digitalWrite(pin, digitalRead(pin));
}

void gpio_out_write(gpio_out_t pin, uint32_t val)
{
    digitalWrite(pin, val);
}


gpio_in_t gpio_in_setup(uint32_t pin, int32_t pull_up)
{
    if (160 <= pin)
        return GPIO_PIN_IVALID;
    pinMode(pin, (pull_up > 0) ? INPUT_PULLUP : INPUT);
    return (gpio_in_t)pin;
}

uint8_t gpio_in_read(gpio_in_t pin)
{
    return digitalRead(pin);
}


gpio_pwm_t pwm_init(uint32_t pin)
{
    if (160 <= pin)
        return GPIO_PIN_IVALID;
    gpio_pwm_t g = pin;
    pinMode(pin, OUTPUT);

// #define VPD           PC4 // TIM2_CH3

    uint16_t AutoReloadRegister = 1591; // 10khz
    TIM2->PSCR = 0x00; // 62kHz
    TIM2->ARRH = (uint8_t)(AutoReloadRegister >> 8);
    TIM2->ARRL = (uint8_t)(AutoReloadRegister);
    unsigned char tmp = TIM2->CCER2 & (uint8_t)(~(TIM2_CCER2_CC3E | TIM2_CCER2_CC3P));
    TIM2->CCER2 = tmp | TIM2_CCER2_CC3E;
    TIM2->CCMR3 = TIM2_OCMODE_PWM1 | TIM2_CCMR_OCxPE;
    TIM2->CCR3H = 0x00; // init with 0 duty cycle e.g. 0 volts
    TIM2->CCR3L = 0x00; // init with 0 duty cycle e.g. 0 volts

    pwm_out_write(g, 0);
    return g;
}

void pwm_out_write(gpio_pwm_t pwm, uint16_t val)
{
    analogWrite(pwm, val);

TIM2->CCR3H = (uint8_t)(val >> 8);
TIM2->CCR3L = (uint8_t)(val);

}

gpio_adc_t adc_config(uint32_t pin)
{
    pinMode(pin, INPUT);
    return (gpio_adc_t)pin;
}

uint32_t adc_read(gpio_adc_t pin)
{
    return (uint32_t)analogRead(pin);
}
