//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
//
//
//
//
//
//
//////////////////////////////////////////////////////////////////////////////

#ifndef __JERRY_H__
#define __JERRY_H__

#include "jaguar.h"

void jerry_init(void);
void jerry_reset(void);
void jerry_done(void);
unsigned jerry_byte_read(unsigned int offset);
unsigned jerry_word_read(unsigned int offset);
void jerry_byte_write(unsigned  offset, unsigned  data);
void jerry_word_write(unsigned  offset, unsigned  data);
void jerry_pit_exec(uint32 cycles);
void jerry_i2s_exec(uint32 cycles);

#endif
