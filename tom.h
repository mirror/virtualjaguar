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

#define TOM_INT0_ENABLE		0x0001
#define TOM_INT1_ENABLE		0x0002
#define TOM_INT2_ENABLE		0x0004
#define TOM_INT3_ENABLE		0x0008
#define TOM_INT4_ENABLE		0x0010
#define TOM_INT0_CLEAR		0x0100
#define TOM_INT1_CLEAR		0x0200
#define TOM_INT2_CLEAR		0x0400
#define TOM_INT3_CLEAR		0x0800
#define TOM_INT4_CLEAR		0x1000
#define TOM_INT0_PENDING	0x0001
#define TOM_INT1_PENDING	0x0002
#define TOM_INT2_PENDING	0x0004
#define TOM_INT3_PENDING	0x0008
#define TOM_INT4_PENDING	0x0010

#define TOM_INT04_ENABLE_FLAGS	(TOM_INT0_ENABLE | TOM_INT1_ENABLE | TOM_INT2_ENABLE | TOM_INT3_ENABLE | TOM_INT4_ENABLE)
#define TOM_INT04_CLEAR_FLAGS	(TOM_INT0_CLEAR | TOM_INT1_CLEAR | TOM_INT2_CLEAR | TOM_INT3_CLEAR | TOM_INT4_CLEAR)

// 68000 Interrupt bit positions (enabled at $F000E0)

enum { TOM_IRQ_VBLANK = 0, TOM_IRQ_GPU, TOM_IRQ_OPFLAG, TOM_IRQ_TIMER, TOM_IRQ_JERRY };

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

void TOMSetIRQLine(int irqline);
void TOMExecPIT(void);//uint32 cycles);
//void TOMResetPIT(void);

//uint32 TOMGetSDLScreenPitch(void);
void TOMResetBackbuffer(uint32 * backbuffer);

// Exported variables

extern uint32 tom_width;
extern uint32 tom_height;

extern uint16 tomIntControl;

#endif	// __TOM_H__
