#include "gpio.h"
#include "gd32f1x0_gpio.h"
#include "gd32f1x0_rcu.h"


struct gpio_out gpio_out_setup(uint32_t pin, uint32_t val)
{
    uint32_t gpio_periph = GPIO_BASE + 0x400 * GPIO2PORT(pin);
    uint32_t gpio_pin = GPIO2BIT(pin);
    /* enable the clock */
    rcu_periph_clock_enable(RCU_REGIDX_BIT(IDX_AHBEN, (17U + GPIO2PORT(pin))));

        gpio_mode_set(gpio_periph, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, gpio_pin);
        gpio_output_options_set(gpio_periph, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,
                                gpio_pin);
    struct gpio_out g = (struct gpio_out){.regs = gpio_periph, .bit = gpio_pin};
    gpio_out_write(g, val);
    return g;
}

void gpio_out_toggle(struct gpio_out g)
{
    GPIO_TG(g.regs) = g.bit;
}

void gpio_out_write(struct gpio_out g, uint32_t val)
{
    if (val)
        GPIO_BOP(g.regs) = g.bit;
    else
        GPIO_BC(g.regs) = g.bit;
}

uint8_t gpio_out_read(struct gpio_out g)
{
    return !!(GPIO_OCTL(g.regs) & (g.bit));
}


struct gpio_in gpio_in_setup(uint32_t pin, int32_t pull_up)
{
    uint32_t gpio_periph = GPIO_BASE + 0x400 * GPIO2PORT(pin);
    uint32_t gpio_pin = GPIO2BIT(pin);
    uint32_t pud_val = GPIO_PUPD_NONE;
    if (pull_up < 0)
        pud_val = GPIO_PUPD_PULLDOWN;
    else if (0 < pull_up)
        pud_val = GPIO_PUPD_PULLUP;
    /* enable the clock */
    rcu_periph_clock_enable(RCU_REGIDX_BIT(IDX_AHBEN, (17U + GPIO2PORT(pin))));
    gpio_mode_set(gpio_periph, GPIO_MODE_INPUT, pud_val, gpio_pin);
    return (struct gpio_in){.regs = gpio_periph, .bit = gpio_pin};
}

uint8_t gpio_in_read(struct gpio_in g)
{
    return !!(GPIO_ISTAT(g.regs) & (g.bit));
}


void pinAlternateConfig(uint32_t pin, uint8_t af, int8_t pud)
{
    uint32_t gpio_periph = GPIO_BASE + 0x400 * GPIO2PORT(pin);
    uint32_t gpio_pin = GPIO2BIT(pin);
    uint32_t pud_val = GPIO_PUPD_NONE;
    if (pud < 0)
        pud_val = GPIO_PUPD_PULLDOWN;
    else if (0 < pud)
        pud_val = GPIO_PUPD_PULLUP;

    if (!pin || 12 <= af)
        return;

    /* enable the clock */
    rcu_periph_clock_enable(RCU_REGIDX_BIT(IDX_AHBEN, (17U + GPIO2PORT(pin))));

    /* configure gpio pin */
    gpio_af_set(gpio_periph, AF(af), gpio_pin);
    gpio_mode_set(gpio_periph, GPIO_MODE_AF, pud_val, gpio_pin);
    gpio_output_options_set(gpio_periph, GPIO_OTYPE_PP,
                            GPIO_OSPEED_50MHZ, gpio_pin);
}
