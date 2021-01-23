#ifndef __PWM_H_
#define __PWM_H_

#include <stdint.h>

typedef struct {
    uint32_t tim;
    uint16_t ch;
} gpio_pwm_t;
gpio_pwm_t pwm_init(uint32_t pin);
void pwm_out_write(gpio_pwm_t pwm, uint16_t val);

#endif /* __PWM_H_ */
