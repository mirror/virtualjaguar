//
// GPU.H: Header file
//

#ifndef __GPU_H__
#define __GPU_H__

#include "jaguar.h"

#define GPU_CONTROL_RAM_BASE    0x00F02100
#define GPU_WORK_RAM_BASE		0x00F03000

#define ASSERT_LINE		1
#define CLEAR_LINE		0

void gpu_init(void);
void gpu_reset(void);
void gpu_exec(int32);
void gpu_done(void);
void gpu_update_register_banks(void);
void GPUHandleIRQs(void);
void GPUSetIRQLine(int irqline, int state);
unsigned gpu_byte_read(unsigned int offset);
unsigned gpu_word_read(unsigned int offset);
unsigned gpu_long_read(unsigned int offset);
void gpu_byte_write(unsigned  offset, unsigned  data);
void gpu_word_write(unsigned  offset, unsigned  data);
void gpu_long_write(unsigned  offset, unsigned  data);
uint32 gpu_get_pc(void);
void gpu_releaseTimeslice(void);
void gpu_reset_stats(void);
uint32 gpu_read_pc(void);

#endif	// __GPU_H__
