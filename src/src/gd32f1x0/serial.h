#ifndef __SERIAL_H_
#define __SERIAL_H_

#include <stdint.h>

void Serial_begin(uint32_t baud, uint32_t tx_pin, uint32_t rx_pin);
uint8_t Serial_available(void);
uint8_t Serial_read(void);
void Serial_write(uint8_t data);
void Serial_write_len(uint8_t *data, uint32_t size);
void Serial_flush(void);

#endif /* __SERIAL_H_ */
