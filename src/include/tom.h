//
// TOM Header file
//

#ifndef __TOM_H__
#define __TOM_H__

#include "jaguar.h"

#define VIDEO_MODE_16BPP_CRY	0
#define VIDEO_MODE_24BPP_RGB	1
#define VIDEO_MODE_16BPP_DIRECT 2
#define VIDEO_MODE_16BPP_RGB	3

// 68000 Interrupt bit positions (enabled at $F000E0)

enum { IRQ_VBLANK = 0, IRQ_GPU, IRQ_OPFLAG, IRQ_TIMER, IRQ_DSP };

void tom_init(void);
void tom_reset(void);
void tom_done(void);

uint8 TOMReadByte(uint32 offset, uint32 who = UNKNOWN);
uint16 TOMReadWord(uint32 offset, uint32 who = UNKNOWN);
void TOMWriteByte(uint32 offset, uint8 data, uint32 who = UNKNOWN);
void TOMWriteWord(uint32 offset, uint16 data, uint32 who = UNKNOWN);

//void TOMExecScanline(int16 * backbuffer, int32 scanline, bool render);
void TOMExecScanline(uint16 scanline, bool render);
uint32 tom_getVideoModeWidth(void);
uint32 tom_getVideoModeHeight(void);
uint8 tom_getVideoMode(void);
uint8 * tom_get_ram_pointer(void);
uint16 tom_get_hdb(void);
uint16 tom_get_vdb(void);
//uint16 tom_get_scanline(void);
//uint32 tom_getHBlankWidthInPixels(void);

int	tom_irq_enabled(int irq);
uint16 tom_irq_control_reg(void);
void tom_set_irq_latch(int irq, int enabled);
void TOMExecPIT(uint32 cycles);
void tom_set_pending_jerry_int(void);
void tom_set_pending_timer_int(void);
void tom_set_pending_object_int(void);
void tom_set_pending_gpu_int(void);
void tom_set_pending_video_int(void);
void TOMResetPIT(void);

//uint32 TOMGetSDLScreenPitch(void);
void TOMResetBackbuffer(uint32 * backbuffer);

// Exported variables

extern uint32 tom_width;
extern uint32 tom_height;

#endif	// __TOM_H__
