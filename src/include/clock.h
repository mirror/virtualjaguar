#ifndef __CLOCK_H__
#define __CLOCK_H__

#include "types.h"

void clock_init(void);
void clock_reset(void);
void clock_done(void);
void clock_byte_write(uint32, uint8);
void clock_word_write(uint32, uint16);
uint8 clock_byte_read(uint32);
uint16 clock_word_read(uint32);

#endif
