#ifndef __PWM_H_
#define __PWM_H_

#include <stdint.h>

struct timeout {
    uint32_t tim;
    uint16_t ch;
};
struct timeout pwm_init(uint32_t pin);
// Value in percent [0...100]
void pwm_out_write(struct timeout pwm, int val);

#endif /* __PWM_H_ */
