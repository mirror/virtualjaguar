//
// JERRY.H: Header file
//

#ifndef __JERRY_H__
#define __JERRY_H__

//#include "types.h"
#include "memory.h"

void JERRYInit(void);
void JERRYReset(void);
void JERRYDone(void);

uint8 JERRYReadByte(uint32 offset, uint32 who = UNKNOWN);
uint16 JERRYReadWord(uint32 offset, uint32 who = UNKNOWN);
void JERRYWriteByte(uint32 offset, uint8 data, uint32 who = UNKNOWN);
void JERRYWriteWord(uint32 offset, uint16 data, uint32 who = UNKNOWN);

void JERRYExecPIT(uint32 cycles);
void JERRYI2SExec(uint32 cycles);

// 68000 Interrupt bit positions (enabled at $F10020)

enum { IRQ2_EXTERNAL = 0, IRQ2_DSP, IRQ2_TIMER1, IRQ2_TIMER2, IRQ2_ASI, IRQ2_SSI };

bool JERRYIRQEnabled(int irq);
void JERRYSetPendingIRQ(int irq);

// This should stay inside this file, but it's here for now...
// Need to set up an interface function so that this can go back
void JERRYI2SCallback(void);

// External variables

extern uint32 JERRYI2SInterruptDivide;
extern int32 JERRYI2SInterruptTimer;

#endif
