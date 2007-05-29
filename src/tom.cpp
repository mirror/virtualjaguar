//
// TOM Processing
//
// by cal16
// GCC/SDL port by Niels Wagenaar (Linux/WIN32) and Caz (BeOS)
// Cleanups and endian wrongness amelioration by James L. Hammons
// Note: Endian wrongness probably stems from the MAME origins of this emu and
//       the braindead way in which MAME handles memory. :-)
//
// Note: TOM has only a 16K memory space
//
//	------------------------------------------------------------
//	TOM REGISTERS (Mapped by Aaron Giles)
//	------------------------------------------------------------
//	F00000-F0FFFF   R/W   xxxxxxxx xxxxxxxx   Internal Registers
//	F00000          R/W   -x-xx--- xxxxxxxx   MEMCON1 - memory config reg 1
//	                      -x------ --------      (CPU32 - is the CPU 32bits?)
//	                      ---xx--- --------      (IOSPEED - external I/O clock cycles)
//	                      -------- x-------      (FASTROM - reduces ROM clock cycles)
//	                      -------- -xx-----      (DRAMSPEED - sets RAM clock cycles)
//	                      -------- ---xx---      (ROMSPEED - sets ROM clock cycles)
//	                      -------- -----xx-      (ROMWIDTH - sets width of ROM: 8,16,32,64 bits)
//	                      -------- -------x      (ROMHI - controls ROM mapping)
//	F00002          R/W   --xxxxxx xxxxxxxx   MEMCON2 - memory config reg 2
//	                      --x----- --------      (HILO - image display bit order)
//	                      ---x---- --------      (BIGEND - big endian addressing?)
//	                      ----xxxx --------      (REFRATE - DRAM refresh rate)
//	                      -------- xx------      (DWIDTH1 - DRAM1 width: 8,16,32,64 bits)
//	                      -------- --xx----      (COLS1 - DRAM1 columns: 256,512,1024,2048)
//	                      -------- ----xx--      (DWIDTH0 - DRAM0 width: 8,16,32,64 bits)
//	                      -------- ------xx      (COLS0 - DRAM0 columns: 256,512,1024,2048)
//	F00004          R/W   -----xxx xxxxxxxx   HC - horizontal count
//	                      -----x-- --------      (which half of the display)
//	                      ------xx xxxxxxxx      (10-bit counter)
//	F00006          R/W   ----xxxx xxxxxxxx   VC - vertical count
//	                      ----x--- --------      (which field is being generated)
//	                      -----xxx xxxxxxxx      (11-bit counter)
//	F00008          R     -----xxx xxxxxxxx   LPH - light pen horizontal position
//	F0000A          R     -----xxx xxxxxxxx   LPV - light pen vertical position
//	F00010-F00017   R     xxxxxxxx xxxxxxxx   OB - current object code from the graphics processor
//	F00020-F00023     W   xxxxxxxx xxxxxxxx   OLP - start of the object list
//	F00026            W   -------- -------x   OBF - object processor flag
//	F00028            W   ----xxxx xxxxxxxx   VMODE - video mode
//	                  W   ----xxx- --------      (PWIDTH1-8 - width of pixel in video clock cycles)
//	                  W   -------x --------      (VARMOD - enable variable color resolution)
//	                  W   -------- x-------      (BGEN - clear line buffere to BG color)
//	                  W   -------- -x------      (CSYNC - enable composite sync on VSYNC)
//	                  W   -------- --x-----      (BINC - local border color if INCEN)
//	                  W   -------- ---x----      (INCEN - encrustation enable)
//	                  W   -------- ----x---      (GENLOCK - enable genlock)
//	                  W   -------- -----xx-      (MODE - CRY16,RGB24,DIRECT16,RGB16)
//	                  W   -------- -------x      (VIDEN - enables video)
//	F0002A            W   xxxxxxxx xxxxxxxx   BORD1 - border color (red/green)
//	F0002C            W   -------- xxxxxxxx   BORD2 - border color (blue)
//	F0002E            W   ------xx xxxxxxxx   HP - horizontal period
//	F00030            W   -----xxx xxxxxxxx   HBB - horizontal blanking begin
//	F00032            W   -----xxx xxxxxxxx   HBE - horizontal blanking end
//	F00034            W   -----xxx xxxxxxxx   HSYNC - horizontal sync
//	F00036            W   ------xx xxxxxxxx   HVS - horizontal vertical sync
//	F00038            W   -----xxx xxxxxxxx   HDB1 - horizontal display begin 1
//	F0003A            W   -----xxx xxxxxxxx   HDB2 - horizontal display begin 2
//	F0003C            W   -----xxx xxxxxxxx   HDE - horizontal display end
//	F0003E            W   -----xxx xxxxxxxx   VP - vertical period
//	F00040            W   -----xxx xxxxxxxx   VBB - vertical blanking begin
//	F00042            W   -----xxx xxxxxxxx   VBE - vertical blanking end
//	F00044            W   -----xxx xxxxxxxx   VS - vertical sync
//	F00046            W   -----xxx xxxxxxxx   VDB - vertical display begin
//	F00048            W   -----xxx xxxxxxxx   VDE - vertical display end
//	F0004A            W   -----xxx xxxxxxxx   VEB - vertical equalization begin
//	F0004C            W   -----xxx xxxxxxxx   VEE - vertical equalization end
//	F0004E            W   -----xxx xxxxxxxx   VI - vertical interrupt
//	F00050            W   xxxxxxxx xxxxxxxx   PIT0 - programmable interrupt timer 0
//	F00052            W   xxxxxxxx xxxxxxxx   PIT1 - programmable interrupt timer 1
//	F00054            W   ------xx xxxxxxxx   HEQ - horizontal equalization end
//	F00058            W   xxxxxxxx xxxxxxxx   BG - background color
//	F000E0          R/W   ---xxxxx ---xxxxx   INT1 - CPU interrupt control register
//	                      ---x---- --------      (C_JERCLR - clear pending Jerry ints)
//	                      ----x--- --------      (C_PITCLR - clear pending PIT ints)
//	                      -----x-- --------      (C_OPCLR - clear pending object processor ints)
//	                      ------x- --------      (C_GPUCLR - clear pending graphics processor ints)
//	                      -------x --------      (C_VIDCLR - clear pending video timebase ints)
//	                      -------- ---x----      (C_JERENA - enable Jerry ints)
//	                      -------- ----x---      (C_PITENA - enable PIT ints)
//	                      -------- -----x--      (C_OPENA - enable object processor ints)
//	                      -------- ------x-      (C_GPUENA - enable graphics processor ints)
//	                      -------- -------x      (C_VIDENA - enable video timebase ints)
//	F000E2            W   -------- --------   INT2 - CPU interrupt resume register
//	F00400-F005FF   R/W   xxxxxxxx xxxxxxxx   CLUT - color lookup table A
//	F00600-F007FF   R/W   xxxxxxxx xxxxxxxx   CLUT - color lookup table B
//	F00800-F00D9F   R/W   xxxxxxxx xxxxxxxx   LBUF - line buffer A
//	F01000-F0159F   R/W   xxxxxxxx xxxxxxxx   LBUF - line buffer B
//	F01800-F01D9F   R/W   xxxxxxxx xxxxxxxx   LBUF - line buffer currently selected
//	------------------------------------------------------------
//	F02000-F021FF   R/W   xxxxxxxx xxxxxxxx   GPU control registers
//	F02100          R/W   xxxxxxxx xxxxxxxx   G_FLAGS - GPU flags register
//	                R/W   x------- --------      (DMAEN - DMA enable)
//	                R/W   -x------ --------      (REGPAGE - register page)
//	                  W   --x----- --------      (G_BLITCLR - clear blitter interrupt)
//	                  W   ---x---- --------      (G_OPCLR - clear object processor int)
//	                  W   ----x--- --------      (G_PITCLR - clear PIT interrupt)
//	                  W   -----x-- --------      (G_JERCLR - clear Jerry interrupt)
//	                  W   ------x- --------      (G_CPUCLR - clear CPU interrupt)
//	                R/W   -------x --------      (G_BLITENA - enable blitter interrupt)
//	                R/W   -------- x-------      (G_OPENA - enable object processor int)
//	                R/W   -------- -x------      (G_PITENA - enable PIT interrupt)
//	                R/W   -------- --x-----      (G_JERENA - enable Jerry interrupt)
//	                R/W   -------- ---x----      (G_CPUENA - enable CPU interrupt)
//	                R/W   -------- ----x---      (IMASK - interrupt mask)
//	                R/W   -------- -----x--      (NEGA_FLAG - ALU negative)
//	                R/W   -------- ------x-      (CARRY_FLAG - ALU carry)
//	                R/W   -------- -------x      (ZERO_FLAG - ALU zero)
//	F02104            W   -------- ----xxxx   G_MTXC - matrix control register
//	                  W   -------- ----x---      (MATCOL - column/row major)
//	                  W   -------- -----xxx      (MATRIX3-15 - matrix width)
//	F02108            W   ----xxxx xxxxxx--   G_MTXA - matrix address register
//	F0210C            W   -------- -----xxx   G_END - data organization register
//	                  W   -------- -----x--      (BIG_INST - big endian instruction fetch)
//	                  W   -------- ------x-      (BIG_PIX - big endian pixels)
//	                  W   -------- -------x      (BIG_IO - big endian I/O)
//	F02110          R/W   xxxxxxxx xxxxxxxx   G_PC - GPU program counter
//	F02114          R/W   xxxxxxxx xx-xxxxx   G_CTRL - GPU control/status register
//	                R     xxxx---- --------      (VERSION - GPU version code)
//	                R/W   ----x--- --------      (BUS_HOG - hog the bus!)
//	                R/W   -----x-- --------      (G_BLITLAT - blitter interrupt latch)
//	                R/W   ------x- --------      (G_OPLAT - object processor int latch)
//	                R/W   -------x --------      (G_PITLAT - PIT interrupt latch)
//	                R/W   -------- x-------      (G_JERLAT - Jerry interrupt latch)
//	                R/W   -------- -x------      (G_CPULAT - CPU interrupt latch)
//	                R/W   -------- ---x----      (SINGLE_GO - single step one instruction)
//	                R/W   -------- ----x---      (SINGLE_STEP - single step mode)
//	                R/W   -------- -----x--      (FORCEINT0 - cause interrupt 0 on GPU)
//	                R/W   -------- ------x-      (CPUINT - send GPU interrupt to CPU)
//	                R/W   -------- -------x      (GPUGO - enable GPU execution)
//	F02118-F0211B   R/W   xxxxxxxx xxxxxxxx   G_HIDATA - high data register
//	F0211C-F0211F   R     xxxxxxxx xxxxxxxx   G_REMAIN - divide unit remainder
//	F0211C            W   -------- -------x   G_DIVCTRL - divide unit control
//	                  W   -------- -------x      (DIV_OFFSET - 1=16.16 divide, 0=32-bit divide)
//	------------------------------------------------------------

#ifndef __PORT__
#include <windows.h>
#endif
#include <SDL.h>
#include "SDLptc.h"
#include "tom.h"
#include "gpu.h"
#include "objectp.h"
#include "cry2rgb.h"


extern uint32 jaguar_mainRom_crc32;

//This can be defined in the makefile as well...
//#define TOM_DEBUG

extern Console console;
extern Surface * surface;

// This makes sense IFF it's being used in an endian friendly way. Currently, it's not.
//#define SWAP_32_ALL(A) ((SWAP_16(A>>16))|(SWAP_16(A<<16))) 
//#define SWAP_32(A) ((A>>16)|(A<<16))
//#define SWAP_16(A) ((A>>8)|(A<<8))
// These are more endian friendly...
#define SET16(addr, val)	tom_ram_8[addr] = ((val) & 0xFF00) >> 8, tom_ram_8[addr+1] = (val) & 0x00FF
#define GET16(addr)			(tom_ram_8[addr] << 8) | tom_ram_8[addr+1]

static uint8 * tom_ram_8;
// This is just braindead and wrong!
//static uint16 * tom_ram_16;
//static uint32 * tom_ram_32;

/*
#define MEMCON1 tom_ram_16[0]
#define MEMCON2 tom_ram_16[1]
#define VMODE   tom_ram_16[0x28>>1]
#define VBB		tom_ram_16[0x40>>1]
#define VBE		tom_ram_16[0x42>>1]
#define VDB		tom_ram_16[0x46>>1]
#define VDE		tom_ram_16[0x48>>1]

#define BG		tom_ram_16[0x58>>1]

#define HBB		tom_ram_16[0x30>>1]
#define HBE		tom_ram_16[0x32>>1]
#define HDB		tom_ram_16[0x38>>1]
#define HDE		tom_ram_16[0x3C>>1]

#define HP		tom_ram_16[0x2E>>1]
#define VP		tom_ram_16[0x3E>>1]
#define VS		tom_ram_16[0x44>>1]

#define BKGCOLOR tom_ram_16[0x58>>1]
*/
#define MEMCON1		0x00
#define MEMCON2		0x02
#define VMODE		0x28
#define HP			0x2E
#define HBB			0x30
#define HBE			0x32
#define HDB			0x38
#define HDE			0x3C
#define VP			0x3E
#define VBB			0x40
#define VBE			0x42
#define VS			0x44
#define VDB			0x46
#define VDE			0x48
#define BG			0x58


uint32 tom_width, tom_height, tom_real_internal_width;

static uint32 tom_timer_prescaler;
static uint32 tom_timer_divider;
static int32 tom_timer_counter;

uint32 tom_scanline;
uint32 hblankWidthInPixels = 0;

static char * videoMode_to_str[8] =
	{"16 bpp CRY", "24 bpp RGB", "16 bpp DIRECT", "16 bpp RGB",
	"Mixed mode", "24 bpp RGB", "16 bpp DIRECT", "16 bpp RGB"};

extern uint8 objectp_running;

typedef void (render_xxx_scanline_fn)(int16 *);

void tom_render_16bpp_cry_scanline(int16 * backbuffer);
void tom_render_24bpp_scanline(int16 * backbuffer);
void tom_render_16bpp_direct_scanline(int16 * backbuffer);
void tom_render_16bpp_rgb_scanline(int16 * backbuffer);
void tom_render_16bpp_cry_rgb_mix_scanline(int16 * backbuffer);

void tom_render_16bpp_cry_stretch_scanline(int16 * backbuffer);
void tom_render_24bpp_stretch_scanline(int16 * backbuffer);
void tom_render_16bpp_direct_stretch_scanline(int16 * backbuffer);
void tom_render_16bpp_rgb_stretch_scanline(int16 * backbuffer);
void tom_render_16bpp_cry_rgb_mix_stretch_scanline(int16 * backbuffer);

render_xxx_scanline_fn * scanline_render_normal[]=
{
	tom_render_16bpp_cry_scanline,
	tom_render_24bpp_scanline,
	tom_render_16bpp_direct_scanline,
	tom_render_16bpp_rgb_scanline,
	tom_render_16bpp_cry_rgb_mix_scanline,
	tom_render_24bpp_scanline,
	tom_render_16bpp_direct_scanline,
	tom_render_16bpp_rgb_scanline,
};
render_xxx_scanline_fn * scanline_render_stretch[]=
{
	tom_render_16bpp_cry_stretch_scanline,
	tom_render_24bpp_stretch_scanline,
	tom_render_16bpp_direct_stretch_scanline,
	tom_render_16bpp_rgb_stretch_scanline,
	tom_render_16bpp_cry_rgb_mix_stretch_scanline,
	tom_render_24bpp_stretch_scanline,
	tom_render_16bpp_direct_stretch_scanline,
	tom_render_16bpp_rgb_stretch_scanline,
};
render_xxx_scanline_fn * scanline_render[8];

uint16 tom_puck_int_pending;
uint16 tom_timer_int_pending;
uint16 tom_object_int_pending;
uint16 tom_gpu_int_pending;
uint16 tom_video_int_pending;

uint16 * tom_cry_rgb_mix_lut;

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
void tom_calc_cry_rgb_mix_lut(void)
{
	uint32 chrm, chrl, y;

	memory_malloc_secure((void **)&tom_cry_rgb_mix_lut, 0x20000, "cry/rgb mixed mode lut");

	for (uint32 i=0; i<0x10000; i++)
	{
		uint16 color=i;

		if (color & 0x01)
		{
			color >>= 1;
			color = (color & 0x007C00) | ((color & 0x00003E0) >> 5) | ((color & 0x0000001F) << 5);
		}
		else
		{
			chrm = (color & 0xF000) >> 12;    
			chrl = (color & 0x0F00) >> 8;
			y    = (color & 0x00FF);
					
			uint16 red   = ((((uint32)redcv[chrm][chrl]) * y) >> 11);
			uint16 green = ((((uint32)greencv[chrm][chrl]) * y) >> 11);
			uint16 blue  = ((((uint32)bluecv[chrm][chrl]) * y) >> 11);
			color = (red << 10) | (green << 5) | blue;
		}
		tom_cry_rgb_mix_lut[i] = color;
	}
}

void tom_set_pending_puck_int(void)
{
	tom_puck_int_pending = 1;
}

void tom_set_pending_timer_int(void)
{
	tom_timer_int_pending = 1;
}

void tom_set_pending_object_int(void)
{
	tom_object_int_pending = 1;
}

void tom_set_pending_gpu_int(void)
{
	tom_gpu_int_pending = 1;
}

void tom_set_pending_video_int(void)
{
	tom_video_int_pending = 1;
}

uint8 * tom_get_ram_pointer(void)
{
	return tom_ram_8;
}
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
uint8 tom_getVideoMode(void)
{
//	uint16 vmode = SWAP_16(VMODE);
	uint16 vmode = GET16(VMODE);
	return ((vmode >> 1) & 0x03) | ((vmode & 0x100) >> 6);
}

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
uint16 tom_get_scanline(void)
{
	return tom_scanline;
}
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
uint16 tom_get_hdb(void)
{
//	return SWAP_16(HDB);
	return GET16(HDB);
}
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
uint16 tom_get_vdb(void)
{
//	return SWAP_16(VBE);
	return GET16(VBE);
}
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
void tom_render_16bpp_cry_rgb_mix_scanline(int16 * backbuffer)
{
	uint32 chrm, chrl, y;

	uint16 width = tom_width;
	uint8 * current_line_buffer=(uint8 *)&tom_ram_8[0x1800];
	
	while (width)
	{
		uint16 color;
		color = *current_line_buffer++;
		color <<= 8;
		color |= *current_line_buffer++;
		*backbuffer++ = tom_cry_rgb_mix_lut[color];
		width--;
	}
}
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
void tom_render_16bpp_cry_scanline(int16 *backbuffer)
{
	uint32 chrm, chrl, y;

	uint16 width=tom_width;
	uint8 *current_line_buffer=(uint8*)&tom_ram_8[0x1800];
	
	while (width)
	{
		uint16 color;
		color=*current_line_buffer++;
		color<<=8;
		color|=*current_line_buffer++;
		
		chrm = (color & 0xF000) >> 12;    
		chrl = (color & 0x0F00) >> 8;
		y    = (color & 0x00FF);
				
		uint16 red   =	((((uint32)redcv[chrm][chrl])*y)>>11);
		uint16 green =	((((uint32)greencv[chrm][chrl])*y)>>11);
		uint16 blue  =	((((uint32)bluecv[chrm][chrl])*y)>>11);
		
		
		*backbuffer++=(red<<10)|(green<<5)|blue;
		width--;
	}
}
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
void tom_render_24bpp_scanline(int16 *backbuffer)
{
	uint16 width=tom_width;
	uint8 *current_line_buffer=(uint8*)&tom_ram_8[0x1800];
	
	while (width)
	{
		uint16 green=*current_line_buffer++;
		uint16 red=*current_line_buffer++;
		uint16 nc=*current_line_buffer++;
		uint16 blue=*current_line_buffer++;
		red>>=3;
		green>>=3;
		blue>>=3;
		*backbuffer++=(red<<10)|(green<<5)|blue;
		width--;
	}
}
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
void tom_render_16bpp_direct_scanline(int16 *backbuffer)
{
	uint16 width=tom_width;
	uint8 *current_line_buffer=(uint8*)&tom_ram_8[0x1800];
	
	while (width)
	{
		uint16 color=*current_line_buffer++;
		color<<=8;
		color|=*current_line_buffer++;
		color>>=1;
		*backbuffer++=color;
		width--;
	}
}
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
void tom_render_16bpp_rgb_scanline(int16 *backbuffer)
{
	uint16 width=tom_width;
	uint8 *current_line_buffer=(uint8*)&tom_ram_8[0x1800];
	
	while (width)
	{
		uint16 color=*current_line_buffer++;
		color<<=8;
		color|=*current_line_buffer++;
		color>>=1;
		color=(color&0x007c00)|((color&0x00003e0)>>5)|((color&0x0000001f)<<5);
		*backbuffer++=color;
		width--;
	}
}
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
void tom_render_16bpp_cry_rgb_mix_stretch_scanline(int16 *backbuffer)
{
	uint32 chrm, chrl, y;

	uint16 width=tom_width;
	uint8 *current_line_buffer=(uint8*)&tom_ram_8[0x1800];
	
	while (width)
	{
		uint16 color;
		color=*current_line_buffer++;
		color<<=8;
		color|=*current_line_buffer++;
		*backbuffer++=tom_cry_rgb_mix_lut[color];
		current_line_buffer+=2;
		width--;
	}
}
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
void tom_render_16bpp_cry_stretch_scanline(int16 *backbuffer)
{
	uint32 chrm, chrl, y;

	uint16 width=tom_width;
	uint8 *current_line_buffer=(uint8*)&tom_ram_8[0x1800];
	
	while (width)
	{
		uint16 color;
		color=*current_line_buffer++;
		color<<=8;
		color|=*current_line_buffer++;
		
		chrm = (color & 0xF000) >> 12;    
		chrl = (color & 0x0F00) >> 8;
		y    = (color & 0x00FF);
				
		uint16 red   =	((((uint32)redcv[chrm][chrl])*y)>>11);
		uint16 green =	((((uint32)greencv[chrm][chrl])*y)>>11);
		uint16 blue  =	((((uint32)bluecv[chrm][chrl])*y)>>11);
		
		uint16 color2;
		color2=*current_line_buffer++;
		color2<<=8;
		color2|=*current_line_buffer++;
		
		chrm = (color2 & 0xF000) >> 12;    
		chrl = (color2 & 0x0F00) >> 8;
		y    = (color2 & 0x00FF);
				
		uint16 red2   =	((((uint32)redcv[chrm][chrl])*y)>>11);
		uint16 green2 =	((((uint32)greencv[chrm][chrl])*y)>>11);
		uint16 blue2  =	((((uint32)bluecv[chrm][chrl])*y)>>11);
		
		red=(red+red2)>>1;
		green=(green+green2)>>1;
		blue=(blue+blue2)>>1;

		*backbuffer++=(red<<10)|(green<<5)|blue;
		width--;
	}
}
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
void tom_render_24bpp_stretch_scanline(int16 *backbuffer)
{
	uint16 width=tom_width;
	uint8 *current_line_buffer=(uint8*)&tom_ram_8[0x1800];
	
	while (width)
	{
		uint16 green=*current_line_buffer++;
		uint16 red=*current_line_buffer++;
		uint16 nc=*current_line_buffer++;
		uint16 blue=*current_line_buffer++;
		red>>=3;
		green>>=3;
		blue>>=3;
		*backbuffer++=(red<<10)|(green<<5)|blue;
		current_line_buffer+=4;
		width--;
	}
}
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
void tom_render_16bpp_direct_stretch_scanline(int16 *backbuffer)
{
	uint16 width=tom_width;
	uint8 *current_line_buffer=(uint8*)&tom_ram_8[0x1800];
	
	while (width)
	{
		uint16 color=*current_line_buffer++;
		color<<=8;
		color|=*current_line_buffer++;
		color>>=1;
		*backbuffer++=color;
		current_line_buffer+=2;
		width--;
	}
}
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
void tom_render_16bpp_rgb_stretch_scanline(int16 *backbuffer)
{
	uint16 width=tom_width;
	uint8 *current_line_buffer=(uint8*)&tom_ram_8[0x1800];
	
	while (width)
	{
		uint16 color1=*current_line_buffer++;
		color1<<=8;
		color1|=*current_line_buffer++;
		color1>>=1;
		uint16 color2=*current_line_buffer++;
		color2<<=8;
		color2|=*current_line_buffer++;
		color2>>=1;
		uint16 red=(((color1&0x7c00)>>10)+((color2&0x7c00)>>10))>>1;
		uint16 green=(((color1&0x00003e0)>>5)+((color2&0x00003e0)>>5))>>1;
		uint16 blue=(((color1&0x0000001f))+((color2&0x0000001f)))>>1;

		color1=(red<<10)|(blue<<5)|green;
		*backbuffer++=color1;
		width--;
	}
}
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
void tom_exec_scanline(int16 * backbuffer, int32 scanline, int8 render)
{
	UINT16 bg = GET16(BG);
	tom_scanline = scanline;

	jaguar_word_write(0xF00004, jaguar_word_read(0xF00004) + 1);

	if (render)
	{
		uint8 * current_line_buffer = (uint8 *)&tom_ram_8[0x1800];
		uint16 * current_line_buffer_16 = (uint16 *)current_line_buffer;

		for(int i=0; i<tom_real_internal_width; i++)
			*current_line_buffer_16++ = bg;

		op_process_list(backbuffer, scanline, render);
		
		(scanline_render[tom_getVideoMode()])(backbuffer);
	}
}
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
void tom_init(void)
{
	op_init();
	blitter_init();
	pcm_init();
//	fprintf(log_get(),"tom_init()\n");
	memory_malloc_secure((void **)&tom_ram_8, 0x4000, "tom ram");
//	tom_ram_16 = (uint16 *)tom_ram_8;
//	tom_ram_32 = (uint32 *)tom_ram_8;
	tom_reset();
	memcpy(scanline_render, scanline_render_normal, sizeof(scanline_render));
	tom_calc_cry_rgb_mix_lut();
}
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
uint32 tom_getHBlankWidthInPixels(void)
{
	return hblankWidthInPixels;
}
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
uint32 tom_getVideoModeWidth(void)
{
	static uint16 onetime = 1;

/*	uint16 vmode = SWAP_16(VMODE);
	uint16 hdb = SWAP_16(HDB);
	uint16 hde = SWAP_16(HDE);
	uint16 hbb = SWAP_16(HBB);
	uint16 hbe = SWAP_16(HBE);*/
	uint16 vmode = GET16(VMODE);
	uint16 hdb = GET16(HDB);
	uint16 hde = GET16(HDE);
	uint16 hbb = GET16(HBB);
	uint16 hbe = GET16(HBE);

	int clock_cycles_per_pixel = ((vmode >> 9) & 0x07);

	uint32 width = 640;
	switch (clock_cycles_per_pixel)
	{
	case 0: width = 640; break;
	case 1: width = 640; break;
	case 2: width = 448; break;
	case 3: width = 320; break;
	case 4: width = 256; break;
	case 5: width = 256; break;
	case 6: width = 256; break;
	case 7: width = 320; break;
//	default: fprintf(log_get(),"%i \n",clock_cycles_per_pixel);
	}
	
	if (jaguar_mainRom_crc32 == 0x3c7bfda8)
	{
		if (width == 320)
			width += 80;
		if (width == 448)
			width -= 16;
	}
	if (hdb == 123)
		hblankWidthInPixels = 16;
	else
		hblankWidthInPixels = 0;

//	fprintf(log_get(),"hdb=%i hbe=%i\n",hdb,hbe);
	return width;
}
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
uint32 tom_getVideoModeHeight(void)
{
/*	uint16 vmode = SWAP_16(VMODE);
	uint16 vdb = SWAP_16(VDB);
	uint16 vde = SWAP_16(VDE);
	uint16 vbb = SWAP_16(VBB);
	uint16 vbe = SWAP_16(VBE);*/
	uint16 vmode = GET16(VMODE);
	uint16 vdb = GET16(VDB);
	uint16 vde = GET16(VDE);
	uint16 vbb = GET16(VBB);
	uint16 vbe = GET16(VBE);
	
	if (vde == 65535)
		vde = vbb;
	
	uint32 screen_height = (vde/*-vdb*/) >> 1;
	return 227;//WAS:screen_height);
}
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
void tom_reset(void)
{
//	fprintf(log_get(),"tom_reset()\n");
	op_reset();
	blitter_reset();
	pcm_reset();

	memset(tom_ram_8, 0x00, 0x4000);
//	tom_ram_8[MEMCON1] = 0x18, tom_ram_8[MEMCON1+1] = 0x61;
	SET16(MEMCON1, 0x1861);
//	tom_ram_8[MEMCON2] = 0x00, tom_ram_8[MEMCON2+1] = 0x00;
	SET16(MEMCON2, 0x0000);
//	tom_ram_8[VMODE] = 0x06, tom_ram_8[VMODE+1] = 0xC1;
	SET16(VMODE, 0x06C1);
//	tom_ram_8[VP] = (523 & 0xFF00) >> 8, tom_ram_8[VP+1] = 523 & 0x00FF; // 525-2
	SET16(VP, 523);
//	tom_ram_8[HP] = SWAP_16(844);
	SET16(HP, 844);
//	tom_ram_8[VS] = SWAP_16(523-6);
	SET16(VS, 523 - 6);
//	tom_ram_8[VBB] = SWAP_16(434);
	SET16(VBB, 434);
//	tom_ram_8[VBE] = SWAP_16(24);
	SET16(VBE, 24);
//	tom_ram_8[HBB] = SWAP_16(689+0x400);
	SET16(HBB, 689 + 0x400);
//	tom_ram_8[HBE] = SWAP_16(125);
	SET16(HBE, 125);

//	tom_ram_8[VDE] = SWAP_16(65535);
	SET16(VDE, 65535);
//	tom_ram_8[VDB] = SWAP_16(28);
	SET16(VDB, 28);
//	tom_ram_8[HDB] = SWAP_16(166);
	SET16(HDB, 166);
//	tom_ram_8[HDE] = SWAP_16(65535);
	SET16(HDE, 65535);

	tom_width = tom_real_internal_width = 0;
	tom_height = 0;
	tom_scanline = 0;
	
//	hblankWidthInPixels = (tom_ram_8[HDB] << 8) | tom_ram_8[HDB+1];
//	hblankWidthInPixels >>= 1;
	hblankWidthInPixels = (GET16(HDB)) >> 1;

	tom_puck_int_pending = 0;
	tom_timer_int_pending = 0;
	tom_object_int_pending = 0;
	tom_gpu_int_pending = 0;
	tom_video_int_pending = 0;

	tom_timer_prescaler = 0;
	tom_timer_divider = 0;
	tom_timer_counter = 0;
	memcpy(scanline_render, scanline_render_normal, sizeof(scanline_render));
} 
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
void tom_done(void)
{
//	fprintf(log_get(),"tom: done()\n");
	op_done();
	pcm_done();
	blitter_done();
	fprintf(log_get(), "tom: resolution %ix%i %s\n", tom_getVideoModeWidth(), tom_getVideoModeHeight(),
		videoMode_to_str[tom_getVideoMode()]);
//	fprintf(log_get(),"\ntom: object processor:\n");
//	fprintf(log_get(),"tom: pointer to object list: 0x%.8x\n",op_get_list_pointer());
//	fprintf(log_get(),"tom: INT1=0x%.2x%.2x\n",tom_byte_read(0xf000e0),tom_byte_read(0xf000e1));
	gpu_done();
	dsp_done();
	memory_free(tom_ram_8);
}

//
// TOM byte access (read)
//

unsigned tom_byte_read(unsigned int offset)
{
	offset &= 0xFF3FFF;

#ifdef TOM_DEBUG
	fprintf(log_get(), "TOM: Reading byte at %06X\n", offset);
#endif

	if ((offset >= gpu_control_ram_base) && (offset < gpu_control_ram_base+0x20))
		return gpu_byte_read(offset);
	else if ((offset >= gpu_work_ram_base) && (offset < gpu_work_ram_base+0x1000))
		return gpu_byte_read(offset);
	else if ((offset >= 0xF00010) && (offset < 0xF00028))
		return op_byte_read(offset);
	else if ((offset >= 0xF02200) && (offset < 0xF022A0))
		return blitter_byte_read(offset);
	else if (offset == 0xF00050)
		return tom_timer_prescaler >> 8;
	else if (offset == 0xF00051)
		return tom_timer_prescaler & 0xFF;
	else if (offset == 0xF00052)
		return tom_timer_divider >> 8;
	else if (offset == 0xF00053)
		return tom_timer_divider & 0xFF;

	return tom_ram_8[offset & 0x3FFF];
}

//
// TOM word access (read)
//

unsigned tom_word_read(unsigned int offset)
{
	offset &= 0xFF3FFF;
#ifdef TOM_DEBUG
	fprintf(log_get(), "TOM: Reading word at %06X\n", offset);
#endif
	if (offset == 0xF000E0)
	{
		uint16 data = (tom_puck_int_pending << 4) | (tom_timer_int_pending << 3)
			| (tom_object_int_pending << 2) | (tom_gpu_int_pending << 1)
			| (tom_video_int_pending << 0);
		//fprintf(log_get(),"tom: interrupt status is 0x%.4x \n",data);
		return data;
	}
	else if (offset == 0xF00006)
		return (tom_scanline << 1) + 1;
	else if ((offset >= gpu_control_ram_base) && (offset < gpu_control_ram_base+0x20))
		return gpu_word_read(offset);
	else if ((offset >= gpu_work_ram_base) && (offset < gpu_work_ram_base+0x1000))
		return gpu_word_read(offset);
	else if ((offset >= 0xF00010) && (offset < 0xF00028))
		return op_word_read(offset);
	else if ((offset >= 0xF02200) && (offset < 0xF022A0))
		return blitter_word_read(offset);
	else if (offset == 0xF00050)
		return tom_timer_prescaler;
	else if (offset == 0xF00052)
		return tom_timer_divider;

	offset &= 0x3FFF;

//	uint16 data = tom_byte_read(offset);
//	data <<= 8;
//	data |= tom_byte_read(offset+1);

//	return data;
	return (tom_byte_read(offset) << 8) | tom_byte_read(offset+1);
}

//
// TOM byte access (write)
//

void tom_byte_write(unsigned offset, unsigned data)
{
	offset &= 0xFF3FFF;

#ifdef TOM_DEBUG
	fprintf(log_get(), "TOM: Writing byte %02X at %06X\n", data, offset);
#endif

	if ((offset >= gpu_control_ram_base) && (offset < gpu_control_ram_base+0x20))
	{
		gpu_byte_write(offset, data);
		return;
	}
	else if ((offset >= gpu_work_ram_base) && (offset < gpu_work_ram_base+0x1000))
	{
		gpu_byte_write(offset, data);
		return;
	}
	else if ((offset >= 0xF00010) && (offset < 0xF00028))
	{
		op_byte_write(offset, data);
		return;
	}
	else if ((offset >= 0xF02200) && (offset < 0xF022A0))
	{
		blitter_byte_write(offset, data);
		return;
	}
	else if (offset == 0xF00050)
	{
		tom_timer_prescaler = (tom_timer_prescaler & 0x00FF) | (data << 8);
		tom_reset_timer();
		return;
	}
	else if (offset == 0xF00051)
	{
		tom_timer_prescaler = (tom_timer_prescaler & 0xFF00) | data;
		tom_reset_timer();
		return;
	}
	else if (offset == 0xF00052)
	{
		tom_timer_divider = (tom_timer_divider & 0x00FF) | (data << 8);
		tom_reset_timer();
		return;
	}
	else if (offset == 0xF00053)
	{
		tom_timer_divider = (tom_timer_divider & 0xFF00) | data;
		tom_reset_timer();
		return;
	}

	tom_ram_8[offset & 0x3FFF] = data;
}

//
// TOM word access (write)
//

void tom_word_write(unsigned offset, unsigned data)
{
	offset &= 0xFF3FFF;

#ifdef TOM_DEBUG
	fprintf(log_get(), "TOM: Writing word %04X at %06X\n", data, offset);
#endif

	if ((offset >= gpu_control_ram_base) && (offset < gpu_control_ram_base+0x20))
	{
		gpu_word_write(offset, data);
		return;
	}
	else if ((offset >= gpu_work_ram_base) && (offset < gpu_work_ram_base+0x1000))
	{
		gpu_word_write(offset, data);
		return;
	}
	else if ((offset >= 0xF00000) && (offset < 0xF00002))
	{
		tom_byte_write(offset, (data>>8));
		tom_byte_write(offset+1, (data&0xFF));
	}
	else if ((offset >= 0xF00010) && (offset < 0xF00028))
	{
		op_word_write(offset, data);
		return;
	}
	else if (offset == 0xF00050)
	{
		tom_timer_prescaler = data;
		tom_reset_timer();
		return;
	}
	else if (offset == 0xF00052)
	{
		tom_timer_divider = data;
		tom_reset_timer();
		return;
	}
	else if (offset == 0xF000E0)
	{
		if (data & 0x0100)
			tom_video_int_pending = 0;
		if (data & 0x0200)
			tom_gpu_int_pending = 0;
		if (data & 0x0400)
			tom_object_int_pending = 0;
		if (data & 0x0800)
			tom_timer_int_pending = 0;
		if (data & 0x1000)
			tom_puck_int_pending = 0;
	}
	else if ((offset >= 0xF02200) && (offset < 0xF022A0))
	{
		blitter_word_write(offset, data);
		return;
	}

	offset &= 0x3FFF;
	if (offset == 0x28)
		objectp_running = 1;

	tom_byte_write(offset, data >> 8);
	tom_byte_write(offset+1,  data & 0xFF);

	// detect screen resolution changes
	if ((offset >= 0x28) && (offset <= 0x4F))
	{
		int width, height;
		tom_real_internal_width = width = tom_getVideoModeWidth();
		height = tom_getVideoModeHeight();
		if (width == 640)
		{
			memcpy(scanline_render, scanline_render_stretch, sizeof(scanline_render));
			width = 320;
		}
		else
		{
			memcpy(scanline_render, scanline_render_normal, sizeof(scanline_render));
		}

		
		if ((width != tom_width) || (height != tom_height))
		{
			ws_audio_done();
		
			static char window_title[256];
			delete surface;
			
			tom_width = width;
			tom_height = height;
			Format format(16, 0x007C00, 0x00003E0, 0x0000001F);
			surface = new Surface(tom_width, tom_height, format);
			console.close();
			sprintf(window_title, "Virtual Jaguar (%ix%i)", tom_width, tom_height);
			console.open(window_title, width, tom_height, format);

			ws_audio_init();
			ws_audio_reset();
		}
	}
}
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
int tom_irq_enabled(int irq)
{
	return jaguar_byte_read(0xF000E1) & (1 << irq);
}
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
void tom_set_irq_latch(int irq, int enabled)
{
	tom_ram_8[0xE0] = (tom_ram_8[0xE0] & (~(1<<irq))) | (enabled ? (1<<irq) : 0);
}
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
uint16 tom_irq_control_reg(void)
{
	return (tom_ram_8[0xE0] << 8) | tom_ram_8[0xE1];
}
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
void tom_reset_timer(void)
{
	if ((!tom_timer_prescaler) || (!tom_timer_divider))
		tom_timer_counter = 0;
	else
		tom_timer_counter = (1 + tom_timer_prescaler) * (1 + tom_timer_divider);
//	fprintf(log_get(),"tom: reseting timer to 0x%.8x (%i)\n",tom_timer_counter,tom_timer_counter);
}
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
void tom_pit_exec(uint32 cycles)
{
	if (tom_timer_counter > 0)
	{
		tom_timer_counter -= cycles;

		if (tom_timer_counter <= 0)
		{
			tom_set_pending_timer_int();
			gpu_set_irq_line(2, 1);
			if ((tom_irq_enabled(IRQ_TIMER)) && (jaguar_interrupt_handler_is_valid(64)))
			{
//				s68000interrupt(7, 64);
//				s68000flushInterrupts();
				m68k_set_irq(7);				// Cause a 68000 NMI...
			}
			tom_reset_timer();
		}
	}
}
