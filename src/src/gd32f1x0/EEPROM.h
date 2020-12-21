#ifndef __EEPROM_H_
#define __EEPROM_H_

#include <stdint.h>

#define EEPROM_put(idx,T)	eeprom_update_block(idx,(uint8_t*)(&(T)),sizeof(T)) //;&(T)
#define EEPROM_get(idx,T)	eeprom_read_block(idx,(uint8_t*)(&(T)),sizeof(T)) //;&(T)

void eeprom_update_block(uint16_t idx, uint8_t *ptr, uint32_t len);
void eeprom_read_block(const uint16_t idx, uint8_t *ptr, uint32_t len);


#endif /* __EEPROM_H_ */
