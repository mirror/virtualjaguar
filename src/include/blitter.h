#ifndef __BLITTER_H__
#define __BLITTER_H__

#include "types.h"

void blitter_init(void);
void blitter_reset(void);
void blitter_done(void);
void blitter_byte_write(uint32, uint8);
void blitter_word_write(uint32, uint16);
uint8 blitter_byte_read(uint32);
uint16 blitter_word_read(uint32);
uint32 blitter_long_read(uint32 offset);
void blitter_long_write(uint32 offset, uint32 data);
uint32 blitter_reg_read(uint32 offset);
void blitter_reg_write(uint32 offset, uint32 data);
extern uint8 blitter_working;

#endif
