//
// EEPROM.H: Header file
//

#ifndef __EEPROM_H__
#define __EEPROM_H__

#include "jaguar.h"

void eeprom_init(void);
void eeprom_reset(void);
void eeprom_done(void);
void eeprom_update(void);

uint8 eeprom_byte_read(uint32 offset);
uint16 eeprom_word_read(uint32 offset);
void eeprom_byte_write(uint32 offset, uint8 data);
void eeprom_word_write(uint32 offset, uint16 data);

#endif	// __EEPROM_H__
