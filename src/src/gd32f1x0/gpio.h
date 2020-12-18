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

struct gpio_out
{
    uint32_t regs;
    uint32_t bit;
};
struct gpio_out gpio_out_setup(uint32_t pin, uint32_t val);
void gpio_out_toggle(struct gpio_out g);
void gpio_out_write(struct gpio_out g, uint32_t val);
uint8_t gpio_out_read(struct gpio_out g);
static inline uint8_t gpio_out_valid(struct gpio_out g) {
    return (!!g.regs);
}

struct gpio_in
{
    uint32_t regs;
    uint32_t bit;
};
struct gpio_in gpio_in_setup(uint32_t pin, int32_t pull_up);
uint8_t gpio_in_read(struct gpio_in g);
static inline uint8_t gpio_in_valid(struct gpio_in g) {
    return (!!g.regs);
}

#endif /* __GPIO_H_ */
