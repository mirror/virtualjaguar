#ifndef __IDE_H__
#define __IDE_H__

#include "jaguar.h"

void ide_init(void);
void ide_reset(void);
void ide_done(void);

void ide_update(void);
void ide_byte_write(uint32 offset, uint8 data);
void ide_word_write(uint32 offset, uint16 data);
uint8 ide_byte_read(uint32 offset);
uint16 ide_word_read(uint32 offset);

#endif
