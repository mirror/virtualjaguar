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

void JERRYExecPIT(void);
void JERRYExecI2S(void);

// 68000 Interrupt bit positions (enabled at $F10020)

enum { JERRY_IRQ_EXTERNAL0 = 0, JERRY_IRQ_DSP, JERRY_IRQ_TIMER1, JERRY_IRQ_TIMER2, JERRY_IRQ_ASI, JERRY_IRQ_I2S };

void JERRYSetIRQLine(int irq);

#endif
