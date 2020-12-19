#ifndef __GPIO_H_
#define __GPIO_H_

#include "pins.h"
#include "adc.h"
#include "pwm.h"
#include <stdint.h>

#define GPIO_NUM_PINS   16
#define GPIO(PORT, NUM) (((PORT) - 'A') * GPIO_NUM_PINS + (NUM))
#define GPIO2PORT(PIN)  ((PIN) / GPIO_NUM_PINS)
#define GPIO2BIT(PIN)   (1U << GPIO2IDX(PIN))
#define GPIO2IDX(PIN)   ((PIN) % GPIO_NUM_PINS)

void pinAlternateConfig(uint32_t pin, uint8_t af, int8_t pud);

typedef struct
{
    uint32_t regs;
    uint32_t bit;
} gpio_out_t;
gpio_out_t gpio_out_setup(uint32_t pin, uint32_t val);
void gpio_out_toggle(gpio_out_t g);
void gpio_out_write(gpio_out_t g, uint32_t val);
uint8_t gpio_out_read(gpio_out_t g);
static inline uint8_t gpio_out_valid(gpio_out_t g) {
    return (!!g.regs);
}

typedef struct
{
    uint32_t regs;
    uint32_t bit;
} gpio_in_t;
gpio_in_t gpio_in_setup(uint32_t pin, int32_t pull_up);
uint8_t gpio_in_read(gpio_in_t g);
static inline uint8_t gpio_in_valid(gpio_in_t g) {
    return (!!g.regs);
}

#endif /* __GPIO_H_ */
