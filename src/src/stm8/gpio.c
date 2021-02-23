#include "gpio.h"
#include "targets.h"

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
    pwm_out_write(g, 0); 
    return g;
}

void pwm_out_write(gpio_pwm_t pwm, uint16_t val)
{
    analogWrite(pwm, val);
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
