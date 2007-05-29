#ifndef __ANAJOY_H__
#define __ANAJOY_H__

#include "types.h"

void anajoy_init(void);
void anajoy_reset(void);
void anajoy_done(void);
void anajoy_byte_write(uint32, uint8);
void anajoy_word_write(uint32, uint16);
uint8 anajoy_byte_read(uint32);
uint16 anajoy_word_read(uint32);

#endif
