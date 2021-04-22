#ifndef __SERIAL_H_
#define __SERIAL_H_

#include <stdint.h>

void serial_begin(uint32_t baud, uint32_t tx_pin, uint32_t rx_pin);
#define serial_available    Serial.available
#define serial_read         Serial.read
#define serial_write        Serial.write
void Serial_write_len(uint8_t *data, uint32_t size);
#define serial_flush        Serial.flush

#endif /* __SERIAL_H_ */

