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

#ifndef __GPU_H__
#define __GPU_H__

#include "jaguar.h"

#define gpu_control_ram_base    0x00f02100
#define gpu_work_ram_base		0x00f03000

void gpu_init(void);
void gpu_reset(void);
void gpu_exec(int32);
void gpu_done(void);
void gpu_update_register_banks(void);
void gpu_check_irqs(void);
void gpu_set_irq_line(int irqline, int state);
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

#endif
