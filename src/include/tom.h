//
// TOM Header file
//

#ifndef __TOM_H__
#define __TOM_H__

#include "jaguar.h"

#define TOM_VBLANK_DURATION_IN_SCANLINES 25

#define VIDEO_MODE_16BPP_CRY	0
#define VIDEO_MODE_24BPP_RGB	1
#define VIDEO_MODE_16BPP_DIRECT 2
#define VIDEO_MODE_16BPP_RGB	3

extern uint32 tom_width;
extern uint32 tom_height;

void tom_init(void);
void tom_reset(void);
void tom_done(void);
unsigned tom_byte_read(unsigned int offset);
unsigned tom_word_read(unsigned int offset);
void tom_byte_write(unsigned  offset, unsigned  data);
void tom_word_write(unsigned  offset, unsigned  data);
void tom_exec_scanline(int16 * backbuffer, int32 scanline, bool render);
uint32 tom_getVideoModeWidth(void);
uint32 tom_getVideoModeHeight(void);
uint8 tom_getVideoMode(void);
uint8 * tom_get_ram_pointer(void);
uint16 tom_get_hdb(void);
uint16 tom_get_vdb(void);
uint16 tom_get_scanline(void);
uint32 tom_getHBlankWidthInPixels(void);

// Interrupts

#define IRQ_VBLANK	0
#define IRQ_GPU		1
#define IRQ_HBLANK  2
#define IRQ_OPFLAG  IRQ_HBLANK
#define IRQ_TIMER	3
#define IRQ_DSP		4

int	tom_irq_enabled(int irq);
uint16 tom_irq_control_reg(void);
void tom_set_irq_latch(int irq, int enabled);
void tom_pit_exec(uint32 cycles);
void tom_set_pending_puck_int(void);
void tom_set_pending_timer_int(void);
void tom_set_pending_object_int(void);
void tom_set_pending_gpu_int(void);
void tom_set_pending_video_int(void);
void tom_reset_timer(void);

#endif	// __TOM_H__
