#ifndef __SERIAL_H_
#define __SERIAL_H_

#include "printf.h"
#include <stdint.h>

void serial_begin(uint32_t baud, uint32_t tx_pin, uint32_t rx_pin, uint8_t stopbits);
uint8_t serial_available(void);
uint8_t serial_read(void);
void serial_write(uint8_t data);
void Serial_write_len(uint8_t *data, uint32_t size);
void serial_flush(void);

#endif /* __SERIAL_H_ */
