#ifndef __EEPROM_H__
#define __EEPROm_H__

#include "jaguar.h"

void eeprom_init(void);
void eeprom_reset(void);
void eeprom_done(void);

void eeprom_update(void);
void eeprom_byte_write(uint32 offset, uint8 data);
void eeprom_word_write(uint32 offset, uint16 data);
uint8 eeprom_byte_read(uint32 offset);
uint16 eeprom_word_read(uint32 offset);

#endif
