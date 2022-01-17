#ifndef __SERIAL_H_
#define __SERIAL_H_

#include <Arduino.h>
#include <stdint.h>

void serial_begin(uint32_t baud, uint32_t tx_pin, uint32_t rx_pin, uint8_t stopbit);
#define serial_available    Serial_available
#define serial_read         Serial_read
#define serial_write        Serial_write
void Serial_write_len(uint8_t *data, uint32_t size);
#define serial_flush        Serial_flush

#endif /* __SERIAL_H_ */
