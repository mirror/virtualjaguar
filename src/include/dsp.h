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

#ifndef __DSP_H__
#define __DSP_H__

#include "jaguar.h"

#define dsp_control_ram_base    0x00f1a100
#define dsp_work_ram_base		0x00f1b000

void dsp_init(void);
void dsp_reset(void);
void dsp_exec(int32);
void dsp_done(void);
void dsp_update_register_banks(void);
void dsp_check_irqs(void);
void dsp_set_irq_line(int irqline, int state);
unsigned dsp_byte_read(unsigned int offset);
unsigned dsp_word_read(unsigned int offset);
unsigned dsp_long_read(unsigned int offset);
void dsp_byte_write(unsigned  offset, unsigned  data);
void dsp_word_write(unsigned  offset, unsigned  data);
void dsp_long_write(unsigned  offset, unsigned  data);
void dsp_check_if_i2s_interrupt_needed(void);
void dsp_releaseTimeslice(void);

#endif
