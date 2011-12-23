//
// DSP.H
//

#ifndef __DSP_H__
#define __DSP_H__

#include "jaguar.h"

#define DSP_CONTROL_RAM_BASE    0x00F1A100
#define DSP_WORK_RAM_BASE		0x00F1B000

#define INT_ENA1		0x00020

void DSPInit(void);
void DSPReset(void);
void DSPExec(int32);
void DSPDone(void);
void DSPUpdateRegisterBanks(void);
void DSPHandleIRQs(void);
void DSPSetIRQLine(int irqline);
uint8 DSPReadByte(uint32 offset, uint32 who = UNKNOWN);
uint16 DSPReadWord(uint32 offset, uint32 who = UNKNOWN);
uint32 DSPReadLong(uint32 offset, uint32 who = UNKNOWN);
void DSPWriteByte(uint32 offset, uint8 data, uint32 who = UNKNOWN);
void DSPWriteWord(uint32 offset, uint16 data, uint32 who = UNKNOWN);
void DSPWriteLong(uint32 offset, uint32 data, uint32 who = UNKNOWN);
void dsp_releaseTimeslice(void);

void DSPExecP(int32 cycles);
void DSPExecP2(int32 cycles);
//void DSPExecP3(int32 cycles);
void DSPExecComp(int32 cycles);

void DSPDumpRegisters(void);

// DSP interrupt numbers (in $F1A100, bits 4-8 & 16)

enum { DSP_IRQ_CPU = 0, DSP_IRQ_I2S, DSP_IRQ_TIMER1, DSP_IRQ_TIMER2, DSP_IRQ_EXT0, DSP_IRQ_EXT1 };

#endif	// __DSP_H__
