#ifndef __ADC_H_
#define __ADC_H_

#include <stdint.h>

typedef struct {
    uint32_t ch;
} gpio_adc_t;
gpio_adc_t adc_config(uint32_t pin);
uint32_t adc_read(gpio_adc_t config);

#endif /* __ADC_H_ */
