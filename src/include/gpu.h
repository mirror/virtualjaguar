//
// GPU.H: Header file
//

#ifndef __GPU_H__
#define __GPU_H__

#include "jaguar.h"

#define GPU_CONTROL_RAM_BASE    0x00F02100
#define GPU_WORK_RAM_BASE		0x00F03000

void gpu_init(void);
void gpu_reset(void);
void gpu_exec(int32);
void gpu_done(void);
void gpu_update_register_banks(void);
void GPUHandleIRQs(void);
void GPUSetIRQLine(int irqline, int state);

uint8 GPUReadByte(uint32 offset, uint32 who = UNKNOWN);
uint16 GPUReadWord(uint32 offset, uint32 who = UNKNOWN);
uint32 GPUReadLong(uint32 offset, uint32 who = UNKNOWN);
void GPUWriteByte(uint32 offset, uint8 data, uint32 who = UNKNOWN);
void GPUWriteWord(uint32 offset, uint16 data, uint32 who = UNKNOWN);
void GPUWriteLong(uint32 offset, uint32 data, uint32 who = UNKNOWN);

uint32 gpu_get_pc(void);
void gpu_releaseTimeslice(void);
void gpu_reset_stats(void);
uint32 gpu_read_pc(void);

// GPU interrupt numbers (from $F00100, bits 4-8)

enum { GPUIRQ_CPU = 0, GPUIRQ_DSP, GPUIRQ_TIMER, GPUIRQ_OBJECT, GPUIRQ_BLITTER };

#endif	// __GPU_H__
