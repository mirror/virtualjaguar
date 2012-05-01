//
// DSP.H
//

#ifndef __DSP_H__
#define __DSP_H__

//#include "types.h"
#include "memory.h"

#define DSP_CONTROL_RAM_BASE    0x00F1A100
#define DSP_WORK_RAM_BASE		0x00F1B000

void DSPInit(void);
void DSPReset(void);
void DSPExec(int32);
void DSPDone(void);
void DSPUpdateRegisterBanks(void);
void DSPHandleIRQs(void);
void DSPSetIRQLine(int irqline, int state);
uint8 DSPReadByte(uint32 offset, uint32 who = UNKNOWN);
uint16 DSPReadWord(uint32 offset, uint32 who = UNKNOWN);
uint32 DSPReadLong(uint32 offset, uint32 who = UNKNOWN);
void DSPWriteByte(uint32 offset, uint8 data, uint32 who = UNKNOWN);
void DSPWriteWord(uint32 offset, uint16 data, uint32 who = UNKNOWN);
void DSPWriteLong(uint32 offset, uint32 data, uint32 who = UNKNOWN);
void DSPReleaseTimeslice(void);
bool DSPIsRunning(void);

void DSPExecP(int32 cycles);
void DSPExecP2(int32 cycles);
//void DSPExecP3(int32 cycles);
void DSPExecComp(int32 cycles);

// Exported vars

extern bool doDSPDis;

// DSP interrupt numbers (in $F1A100, bits 4-8 & 16)

enum { DSPIRQ_CPU = 0, DSPIRQ_SSI, DSPIRQ_TIMER0, DSPIRQ_TIMER1, DSPIRQ_EXT0, DSPIRQ_EXT1 };

#endif	// __DSP_H__
