#ifndef __ADC_H_
#define __ADC_H_

#include <stdint.h>

struct adc {
    uint32_t ch;
};
struct adc adc_config(uint32_t pin);
uint32_t adc_read(struct adc config);

#endif /* __ADC_H_ */
