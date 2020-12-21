#ifndef _GPIO_H__
#define _GPIO_H__

#include <stdint.h> // uint32_t

#define GPIO_PIN_IVALID (uint32_t)-1

typedef uint32_t gpio_out_t;
gpio_out_t gpio_out_setup(uint32_t pin, uint32_t val);
void gpio_out_toggle(gpio_out_t g);
void gpio_out_write(gpio_out_t g, uint32_t val);
static inline uint8_t gpio_out_valid(gpio_out_t g) {
    return (g != GPIO_PIN_IVALID);
}

typedef uint32_t gpio_in_t;
gpio_in_t gpio_in_setup(uint32_t pin, int32_t pull_up);
uint8_t gpio_in_read(gpio_in_t g);
static inline uint8_t gpio_in_valid(gpio_in_t g) {
    return (g != GPIO_PIN_IVALID);
}

typedef uint32_t gpio_pwm_t;
gpio_pwm_t pwm_init(uint32_t pin);
void pwm_out_write(gpio_pwm_t, int val);

typedef uint32_t gpio_adc_t;
gpio_adc_t adc_config(uint32_t pin);
uint32_t adc_read(gpio_adc_t config);

#endif /*_GPIO_H__*/
