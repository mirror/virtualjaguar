#ifndef __CDROM_H__
#define __CDROM_H__

#include "jaguar.h"

void cdrom_init(void);
void cdrom_reset(void);
void cdrom_done(void);

void cdrom_update(void);
void cdrom_byte_write(uint32 offset, uint8 data);
void cdrom_word_write(uint32 offset, uint16 data);
uint8 cdrom_byte_read(uint32 offset);
uint16 cdrom_word_read(uint32 offset);

#endif
