//
// JERRY.H: Header file
//

#ifndef __JERRY_H__
#define __JERRY_H__

#include "jaguar.h"

void jerry_init(void);
void jerry_reset(void);
void jerry_done(void);

uint8 JERRYReadByte(uint32 offset, uint32 who = UNKNOWN);
uint16 JERRYReadWord(uint32 offset, uint32 who = UNKNOWN);
void JERRYWriteByte(uint32 offset, uint8 data, uint32 who = UNKNOWN);
void JERRYWriteWord(uint32 offset, uint16 data, uint32 who = UNKNOWN);

void jerry_pit_exec(uint32 cycles);
void jerry_i2s_exec(uint32 cycles);

// 68000 Interrupt bit positions (enabled at $F10020)

enum { IRQ2_EXTERNAL = 0, IRQ2_DSP, IRQ2_TIMER1, IRQ2_TIMER2, IRQ2_ASI, IRQ2_SSI };

bool JERRYIRQEnabled(int irq);
void JERRYSetPendingIRQ(int irq);

#endif
