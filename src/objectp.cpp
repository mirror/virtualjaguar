//
// Object Processor
//
// by cal2
// GCC/SDL port by Niels Wagenaar (Linux/WIN32) and Caz (BeOS)
// Cleanups/fixes/rewrites by James L. Hammons
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "jaguar.h"

//#define OP_DEBUG
//#define OP_DEBUG_BMP

#define BLEND_Y(dst, src)	op_blend_y[(((uint16)dst<<8)) | ((uint16)(src))]
#define BLEND_CR(dst, src)	op_blend_cr[(((uint16)dst)<<8) | ((uint16)(src))]
//Delete this once we're rid of zbmpop*.h...
#define BLEND_CC(dst, src)	op_blend_cr[(((uint16)dst)<<8) | ((uint16)(src))]

#define OBJECT_TYPE_BITMAP	0			// 000
#define OBJECT_TYPE_SCALE	1			// 001
#define OBJECT_TYPE_GPU		2			// 010
#define OBJECT_TYPE_BRANCH	3			// 011
#define OBJECT_TYPE_STOP	4			// 100

#define CONDITION_EQUAL				0
#define CONDITION_LESS_THAN			1
#define CONDITION_GREATER_THAN		2
#define CONDITION_OP_FLAG_SET		3
#define CONDITION_SECOND_HALF_LINE	4

//Delete this once we're rid of zbmpop*.h...
#define FLAGS_RELEASE		8
#define FLAGS_TRANSPARENT	4
#define FLAGS_READMODIFY	2
#define FLAGS_HFLIP			1

#define OPFLAG_RELEASE		8			// Bus release bit
#define OPFLAG_TRANS		4			// Transparency bit
#define OPFLAG_RMW			2			// Read-Modify-Write bit
#define OPFLAG_REFLECT		1			// Horizontal mirror bit

// Private function prototypes

void OPProcessFixedBitmap(int scanline, uint64 p0, uint64 p1, bool render);
void OPProcessScaledBitmap(int scanline, uint64 p0, uint64 p1, uint64 p2, bool render);

// External global variables

extern uint32 jaguar_mainRom_crc32;

// Local global variables

static uint8 * op_blend_y;
static uint8 * op_blend_cr;
// There may be a problem with this "RAM" overlapping some of the
// regular TOM RAM...
static uint8 objectp_ram[0x40];			// This is based at $F00000
uint8 objectp_running;
bool objectp_stop_reading_list;

static uint8 op_bitmap_bit_depth[8] = { 1, 2, 4, 8, 16, 24, 32, 0 };
static uint32 op_bitmap_bit_size[8] =
	{ (uint32)(0.125*65536), (uint32)(0.25*65536), (uint32)(0.5*65536), (uint32)(1*65536),
	  (uint32)(2*65536),     (uint32)(1*65536),    (uint32)(1*65536),   (uint32)(1*65536) };
static uint32 op_pointer;


//
// Object Processor initialization
//
void op_init(void)
{
	// Blend tables (64K each)
	memory_malloc_secure((void **)&op_blend_y, 0x10000, "Jaguar Object processor Y blend lookup table");
	memory_malloc_secure((void **)&op_blend_cr, 0x10000, "Jaguar Object processor CR blend lookup table");

	// Here we calculate the saturating blend of a signed 4-bit value and an
	// existing Cyan/Red value as well as a signed 8-bit value and an existing intensity...
	// Note: CRY is 4 bits Cyan, 4 bits Red, 16 bits intensitY
	for(int i=0; i<256*256; i++)
	{
		int y = (i >> 8) & 0xFF;
		int dy = (INT8)i;					// Sign extend the Y index
		int c1 = (i >> 8) & 0x0F;
		int dc1 = (INT8)(i << 4) >> 4;		// Sign extend the R index
		int c2 = (i >> 12) & 0x0F;
		int dc2 = (INT8)(i & 0xF0) >> 4;	// Sign extend the C index

		y += dy;
		if (y < 0)
			y = 0;
		else if (y > 0xFF)
			y = 0xFF;
		op_blend_y[i] = y;

		c1 += dc1;
		if (c1 < 0)
			c1 = 0;
		else if (c1 > 0x0F)
			c1 = 0x0F;
		c2 += dc2;

		if (c2 < 0)
			c2 = 0;
		else if (c2 > 0x0F)
			c2 = 0x0F;
		op_blend_cr[i] = (c2 << 4) | c1;
	}

	op_reset();
}

//
// Object Processor reset
//
void op_reset(void)
{
	memset(objectp_ram, 0x00, 0x40);
	objectp_running = 0;
}

void op_done(void)
{
}

//
// Object Processor memory access
// Memory range: F00010 (F00008?) - F00027
//
void op_byte_write(uint32 offset, uint8 data)
{
	offset &= 0x3F;
	objectp_ram[offset] = data;
}

void op_word_write(uint32 offset, uint16 data)
{
	offset &= 0x3F;
//	objectp_ram[offset] = (data >> 8) & 0xFF;
//	objectp_ram[offset+1] = data & 0xFF;
	SET16(objectp_ram, offset, data);

/*if (offset == 0x20)
WriteLog("OP: Setting lo list pointer: %04X\n", data);
if (offset == 0x22)
WriteLog("OP: Setting hi list pointer: %04X\n", data);//*/
}

uint8 op_byte_read(uint32 offset)
{
	offset &= 0x3F;
	return objectp_ram[offset];
}

uint16 op_word_read(uint32 offset)
{
//	return (objectp_ram[offset & 0x3F] << 8) | objectp_ram[(offset+1) & 0x3F];
	offset &= 0x3F;
	return GET16(objectp_ram, offset);
}

//	F00010-F00017   R     xxxxxxxx xxxxxxxx   OB - current object code from the graphics processor
//	F00020-F00023     W   xxxxxxxx xxxxxxxx   OLP - start of the object list
//	F00026            W   -------- -------x   OBF - object processor flag

uint32 op_get_list_pointer(void)
{
	// Note: This register is LO / HI WORD, hence the funky look of this...
//	return (objectp_ram[0x22] << 24) | (objectp_ram[0x23] << 16) | (objectp_ram[0x20] << 8) | objectp_ram[0x21];
	return GET16(objectp_ram, 0x20) | (GET16(objectp_ram, 0x22) << 16);
}

// This is WRONG, since the OBF is only 16 bits wide!!! [FIXED]

uint32 op_get_status_register(void)
{
//	return (objectp_ram[0x26] << 24) | (objectp_ram[0x27] << 16) | (objectp_ram[0x28] << 8) | objectp_ram[0x29];
//	return GET32(objectp_ram, 0x26);
	return GET16(objectp_ram, 0x26);
}

// This is WRONG, since the OBF is only 16 bits wide!!! [FIXED]

void op_set_status_register(uint32 data)
{
/*	objectp_ram[0x26] = (data & 0xFF000000) >> 24;
	objectp_ram[0x27] = (data & 0x00FF0000) >> 16;
	objectp_ram[0x28] = (data & 0x0000FF00) >> 8;
	objectp_ram[0x29] |= (data & 0xFE);*/
	objectp_ram[0x26] = (data & 0x0000FF00) >> 8;
	objectp_ram[0x27] |= (data & 0xFE);
}

void op_set_current_object(uint64 object)
{
//Not sure this is right... Wouldn't it just be stored 64 bit BE?
	// Stored as least significant 32 bits first, ms32 last in big endian
	objectp_ram[0x13] = object & 0xFF; object >>= 8;
	objectp_ram[0x12] = object & 0xFF; object >>= 8;
	objectp_ram[0x11] = object & 0xFF; object >>= 8;
	objectp_ram[0x10] = object & 0xFF; object >>= 8;

	objectp_ram[0x17] = object & 0xFF; object >>= 8;
	objectp_ram[0x16] = object & 0xFF; object >>= 8;
	objectp_ram[0x15] = object & 0xFF; object >>= 8;
	objectp_ram[0x14] = object & 0xFF; 
}

uint64 op_load_phrase(uint32 offset)
{
	offset &= ~0x07;						// 8 byte alignment
	return ((uint64)jaguar_long_read(offset) << 32) | (uint64)jaguar_long_read(offset+4);
}

//
// OP replacement functions
//

void OPStorePhrase(uint32 offset, uint64 p)
{
	offset &= ~0x07;						// 8 byte alignment
	jaguar_long_write(offset, p >> 32);
	jaguar_long_write(offset + 4, p & 0xFFFFFFFF);
}

//
// *** NEW ***
// Object Processor main routine
//
void OPProcessList(int scanline, bool render)
{
extern int op_start_log;
//	char * condition_to_str[8] =
//		{ "==", "<", ">", "(opflag set)", "(second half line)", "?", "?", "?" };

// If jaguar_exec() is working right, we should *never* have to check for this
// condition...
	if (scanline < tom_get_vdb())
		return;

	if (scanline >= 525)//tom_getVideoModeHeight()+tom_get_vdb())
		return;

	op_pointer = op_get_list_pointer();

	objectp_stop_reading_list = false;

// *** BEGIN OP PROCESSOR TESTING ONLY ***
extern bool interactiveMode;
extern bool iToggle;
extern int objectPtr;
bool inhibit;
int bitmapCounter = 0;
// *** END OP PROCESSOR TESTING ONLY ***

//	if (op_pointer) WriteLog(" new op list at 0x%.8x scanline %i\n",op_pointer,scanline);
	while (op_pointer)
	{
// *** BEGIN OP PROCESSOR TESTING ONLY ***
if (interactiveMode && bitmapCounter == objectPtr)
	inhibit = iToggle;
else
	inhibit = false;
// *** END OP PROCESSOR TESTING ONLY ***
		if (objectp_stop_reading_list)
			return;
			
		uint64 p0 = op_load_phrase(op_pointer);
		op_pointer += 8;
if (scanline == tom_get_vdb() && op_start_log)
{
WriteLog("%08X --> phrase %08X %08X", op_pointer - 8, (int)(p0>>32), (int)(p0&0xFFFFFFFF));
if ((p0 & 0x07) == OBJECT_TYPE_BITMAP)
{
WriteLog(" (BITMAP) ");
uint64 p1 = op_load_phrase(op_pointer);
WriteLog("\n%08X --> phrase %08X %08X ", op_pointer, (int)(p1>>32), (int)(p1&0xFFFFFFFF));
	uint8 bitdepth = (p1 >> 12) & 0x07;
	int16 ypos = ((p0 >> 3) & 0x3FF);			// ??? What if not interlaced (/2)?
int32 xpos = p1 & 0xFFF;
xpos = (xpos & 0x800 ? xpos | 0xFFFFF000 : xpos);
	uint32 iwidth = ((p1 >> 28) & 0x3FF);
	uint32 dwidth = ((p1 >> 18) & 0x3FF);		// Unsigned!
	uint16 height = ((p0 >> 14) & 0x3FF) - 1;
	uint32 link = ((p0 >> 24) & 0x7FFFF) << 3;
	uint32 ptr = ((p0 >> 43) & 0x1FFFFF) << 3;
	uint32 firstPix = (p1 >> 49) & 0x3F;
	uint8 flags = (p1 >> 45) & 0x0F;
	uint8 idx = (p1 >> 38) & 0x7F;
	uint32 pitch = (p1 >> 15) & 0x07;
WriteLog("\n    [%u (%u) x %u @ %i, %u (%u bpp), l: %08X, p: %08X fp: %02X, fl:%s%s%s%s, idx:%02X, pt:%02X]\n",
	iwidth, dwidth, height, xpos, ypos, op_bitmap_bit_depth[bitdepth], link, ptr, firstPix, (flags&FLAGS_HFLIP ? "REFLECT " : ""), (flags&FLAGS_READMODIFY ? "RMW " : ""), (flags&FLAGS_TRANSPARENT ? "TRANS " : ""), (flags&FLAGS_RELEASE ? "RELEASE" : ""), idx, pitch);
}
if ((p0 & 0x07) == OBJECT_TYPE_SCALE)
{
WriteLog(" (SCALED BITMAP)");
uint64 p1 = op_load_phrase(op_pointer), p2 = op_load_phrase(op_pointer+8);
WriteLog("\n%08X --> phrase %08X %08X ", op_pointer, (int)(p1>>32), (int)(p1&0xFFFFFFFF));
	uint8 bitdepth = (p1 >> 12) & 0x07;
	int16 ypos = ((p0 >> 3) & 0x3FF);			// ??? What if not interlaced (/2)?
int32 xpos = p1 & 0xFFF;
xpos = (xpos & 0x800 ? xpos | 0xFFFFF000 : xpos);
	uint32 iwidth = ((p1 >> 28) & 0x3FF);
	uint32 dwidth = ((p1 >> 18) & 0x3FF);		// Unsigned!
	uint16 height = ((p0 >> 14) & 0x3FF) - 1;
	uint32 link = ((p0 >> 24) & 0x7FFFF) << 3;
	uint32 ptr = ((p0 >> 43) & 0x1FFFFF) << 3;
	uint32 firstPix = (p1 >> 49) & 0x3F;
	uint8 flags = (p1 >> 45) & 0x0F;
	uint8 idx = (p1 >> 38) & 0x7F;
	uint32 pitch = (p1 >> 15) & 0x07;
WriteLog("\n    [%u (%u) x %u @ %i, %u (%u bpp), l: %08X, p: %08X fp: %02X, fl:%s%s%s%s, idx:%02X, pt:%02X]\n",
	iwidth, dwidth, height, xpos, ypos, op_bitmap_bit_depth[bitdepth], link, ptr, firstPix, (flags&FLAGS_HFLIP ? "REFLECT " : ""), (flags&FLAGS_READMODIFY ? "RMW " : ""), (flags&FLAGS_TRANSPARENT ? "TRANS " : ""), (flags&FLAGS_RELEASE ? "RELEASE" : ""), idx, pitch);
	uint32 hscale = p2 & 0xFF;
	uint32 vscale = (p2 >> 8) & 0xFF;
	uint32 remainder = (p2 >> 16) & 0xFF;
WriteLog("    [hsc: %02X, vsc: %02X, rem: %02X]\n", hscale, vscale, remainder);
}
if ((p0 & 0x07) == OBJECT_TYPE_GPU)
WriteLog(" (GPU)\n");
if ((p0 & 0x07) == OBJECT_TYPE_BRANCH)
{
WriteLog(" (BRANCH)\n");
uint8 * jaguar_mainRam = GetRamPtr();
WriteLog("[RAM] --> ");
for(int k=0; k<8; k++)
	WriteLog("%02X ", jaguar_mainRam[op_pointer-8 + k]);
WriteLog("\n");
}
if ((p0 & 0x07) == OBJECT_TYPE_STOP)
WriteLog("    --> List end\n");
}//*/
		
//		WriteLog("%08X type %i\n", op_pointer, (uint8)p0 & 0x07);		
		switch ((uint8)p0 & 0x07)
		{
		case OBJECT_TYPE_BITMAP:
		{
			// Would *not* be /2 if interlaced...!
			uint16 ypos = ((p0 >> 3) & 0x3FF) / 2;
// This is only theory implied by Rayman...!
// It seems that if the YPOS is zero, then bump the YPOS value so that it coincides with
// the VDB value. With interlacing, this would be slightly more tricky.
// There's probably another bit somewhere that enables this mode--but so far, doesn't seem
// to affect any other game in a negative way (that I've seen).
// Either that, or it's an undocumented bug...
			if (ypos == 0)
				ypos = tom_word_read(0xF00046) / 2;			// Get the VDB value
			uint32 height = (p0 & 0xFFC000) >> 14;
			uint32 oldOPP = op_pointer - 8;
// *** BEGIN OP PROCESSOR TESTING ONLY ***
if (inhibit && op_start_log)
	WriteLog("!!! ^^^ This object is INHIBITED! ^^^ !!!\n");
bitmapCounter++;
if (!inhibit)	// For OP testing only!
// *** END OP PROCESSOR TESTING ONLY ***
			if (scanline >= ypos && height > 0)
			{
				uint64 p1 = op_load_phrase(op_pointer);
				op_pointer += 8;
//WriteLog("OP: Writing scanline %d with ypos == %d...\n", scanline, ypos);
//WriteLog("--> Writing %u BPP bitmap...\n", op_bitmap_bit_depth[(p1 >> 12) & 0x07]);
				OPProcessFixedBitmap(scanline, p0, p1, render);

				// OP write-backs

//???Does this really happen??? Doesn't seem to work if you do this...!
//				uint32 link = (p0 & 0x7FFFF000000) >> 21;
//				SET16(objectp_ram, 0x20, link & 0xFFFF);	// OLP
//				SET16(objectp_ram, 0x22, link >> 16);
/*				uint32 height = (p0 & 0xFFC000) >> 14;
				if (height - 1 > 0)
					height--;*/
				// NOTE: Would subtract 2 if in interlaced mode...!
//				uint64 height = ((p0 & 0xFFC000) - 0x4000) & 0xFFC000;
//				if (height)
					height--;

				uint64 data = (p0 & 0xFFFFF80000000000) >> 40;
				uint64 dwidth = (p1 & 0xFFC0000) >> 15;
				data += dwidth;

				p0 &= ~0xFFFFF80000FFC000;			// Mask out old data...
				p0 |= (uint64)height << 14;
				p0 |= data << 40;
				OPStorePhrase(oldOPP, p0);
			}
			op_pointer = (p0 & 0x000007FFFF000000) >> 21;
			break;
		}
		case OBJECT_TYPE_SCALE:
		{
			// Would *not* be /2 if interlaced...!
			uint16 ypos = ((p0 >> 3) & 0x3FF) / 2;
// This is only theory implied by Rayman...!
// It seems that if the YPOS is zero, then bump the YPOS value so that it coincides with
// the VDB value. With interlacing, this would be slightly more tricky.
// There's probably another bit somewhere that enables this mode--but so far, doesn't seem
// to affect any other game in a negative way (that I've seen).
// Either that, or it's an undocumented bug...
			if (ypos == 0)
				ypos = tom_word_read(0xF00046) / 2;			// Get the VDB value
			uint32 height = (p0 & 0xFFC000) >> 14;
			uint32 oldOPP = op_pointer - 8;
// *** BEGIN OP PROCESSOR TESTING ONLY ***
if (inhibit && op_start_log)
	WriteLog("!!! ^^^ This object is INHIBITED! ^^^ !!!\n");
bitmapCounter++;
if (!inhibit)	// For OP testing only!
// *** END OP PROCESSOR TESTING ONLY ***
			if (scanline >= ypos && height > 0)
			{
				uint64 p1 = op_load_phrase(op_pointer);
				op_pointer += 8;
				uint64 p2 = op_load_phrase(op_pointer);
				op_pointer += 8;
//WriteLog("OP: %08X (%d) %08X%08X %08X%08X %08X%08X\n", oldOPP, scanline, (uint32)(p0>>32), (uint32)(p0&0xFFFFFFFF), (uint32)(p1>>32), (uint32)(p1&0xFFFFFFFF), (uint32)(p2>>32), (uint32)(p2&0xFFFFFFFF));
				OPProcessScaledBitmap(scanline, p0, p1, p2, render);

				// OP write-backs

//???Does this really happen??? Doesn't seem to work if you do this...!
//				uint32 link = (p0 & 0x7FFFF000000) >> 21;
//				SET16(objectp_ram, 0x20, link & 0xFFFF);	// OLP
//				SET16(objectp_ram, 0x22, link >> 16);
/*				uint32 height = (p0 & 0xFFC000) >> 14;
				if (height - 1 > 0)
					height--;*/
				// NOTE: Would subtract 2 if in interlaced mode...!
//				uint64 height = ((p0 & 0xFFC000) - 0x4000) & 0xFFC000;

				uint8 remainder = p2 >> 16, vscale = p2 >> 8;
				if (vscale == 0)
					vscale = 0x20;					// OP bug???

				remainder -= 0x20;					// 1.0f in [3.5] fixed point format
				if (remainder & 0x80)				// I.e., it's negative
				{
					uint64 data = (p0 & 0xFFFFF80000000000) >> 40;
					uint64 dwidth = (p1 & 0xFFC0000) >> 15;

					while (remainder & 0x80)
					{
						remainder += vscale;
						if (height)
							height--;

						data += dwidth;
					}
					p0 &= ~0xFFFFF80000FFC000;		// Mask out old data...
					p0 |= (uint64)height << 14;
					p0 |= data << 40;
					OPStorePhrase(oldOPP, p0);
				}

//WriteLog(" [%08X%08X -> ", (uint32)(p2>>32), (uint32)(p2&0xFFFFFFFF));
				p2 &= ~0x0000000000FF0000;
				p2 |= (uint64)remainder << 16;
//WriteLog("%08X%08X]\n", (uint32)(p2>>32), (uint32)(p2&0xFFFFFFFF));
				OPStorePhrase(oldOPP+16, p2);
//remainder = (uint8)(p2 >> 16), vscale = (uint8)(p2 >> 8);
//WriteLog(" [after]: rem=%02X, vscale=%02X\n", remainder, vscale);
			}
			op_pointer = (p0 & 0x000007FFFF000000) >> 21;
			break;
		}
		case OBJECT_TYPE_GPU:
		{
			op_set_current_object(p0);
			gpu_set_irq_line(3, 1);
			break;
		}
		case OBJECT_TYPE_BRANCH:
		{
			uint16 ypos = (p0 >> 3) & 0x7FF;
			uint8  cc   = (p0 >> 14) & 0x03;
			uint32 link = (p0 >> 21) & 0x3FFFF8;
			
//			if ((ypos!=507)&&(ypos!=25))
//				WriteLog("\t%i%s%i link=0x%.8x\n",scanline,condition_to_str[cc],ypos>>1,link);
			switch (cc)
			{
			case CONDITION_EQUAL:
				if (ypos != 0x7FF && (ypos & 0x01))
				 	ypos ^= 0x01;
				if ((2 * tom_get_scanline()) == ypos || ypos == 0x7FF)
					op_pointer = link;
				break;
			case CONDITION_LESS_THAN:
				if ((2 * tom_get_scanline()) < ypos)
					op_pointer = link;
				break;
			case CONDITION_GREATER_THAN:
				if ((2 * tom_get_scanline()) > ypos)
					op_pointer = link;
				break;
			case CONDITION_OP_FLAG_SET:
				if (op_get_status_register() & 0x01)
					op_pointer = link;
				break;
			case CONDITION_SECOND_HALF_LINE:
				// This basically means branch if bit 10 of HC is set
				WriteLog("OP: Unexpected CONDITION_SECOND_HALF_LINE in BRANCH object\nop: shuting down\n");
				fclose(log_get());
				exit(0);
				break;
			default:
				WriteLog("OP: Unimplemented branch condition %i\n", cc);
			}
			break;
		}
		case OBJECT_TYPE_STOP:
		{
//op_start_log = 0;
			// unsure
//WriteLog("OP: --> STOP\n");
			op_set_status_register(((p0>>3) & 0xFFFFFFFF));
			
			if (p0 & 0x8)
			{
				tom_set_pending_object_int();
				if (tom_irq_enabled(2) && jaguar_interrupt_handler_is_valid(64))
					m68k_set_irq(7);				// Cause an NMI to occur...
			}

			return;
			break;
		}
		default:
			WriteLog("op: unknown object type %i\n", ((uint8)p0 & 0x07)); 
			return;
		}
	}
}

//
// *** NEW ***
// Store fixed size bitmap in line buffer
//

// Interesting thing about Rayman: There seems to be a transparent bitmap (1/8/16 bpp--which?)
// being rendered under his feet--doesn't align when walking... Check it out!

void OPProcessFixedBitmap(int scanline, uint64 p0, uint64 p1, bool render)
{
// Need to make sure that when writing that it stays within the line buffer...
// LBUF ($F01800 - $F01D9E) 360 x 32-bit RAM
	uint8 depth = (p1 >> 12) & 0x07;				// Color depth of image
//Why is HBlankWidthInPixels subtracted from this???
//	int32 xpos = (((int32)((p1 << 20) & 0xFFFFFFFF)) >> 20) - tom_getHBlankWidthInPixels();
	int32 xpos = ((int16)((p1 << 4) & 0xFFFF)) >> 4;// Image xpos in LBUF
	uint32 iwidth = (p1 >> 28) & 0x3FF;				// Image width in *phrases*
	uint32 data = (p0 >> 40) & 0xFFFFF8;			// Pixel data address
#ifdef OP_DEBUG_BMP
// Prolly should use this... Though not sure exactly how.
	uint32	firstPix = (p1 >> 49) & 0x3F;
#endif
// We can ignore the RELEASE (high order) bit for now--probably forever...!
//	uint8 flags = (p1 >> 45) & 0x0F;	// REFLECT, RMW, TRANS, RELEASE
//Optimize: break these out to their own BOOL values
	uint8 flags = (p1 >> 45) & 0x07;				// REFLECT (0), RMW (1), TRANS (2)
// "For images with 1 to 4 bits/pixel the top 7 to 4 bits of the index
//  provide the most significant bits of the palette address."
	uint8 index = (p1 >> 37) & 0xFE;				// CLUT index offset (upper pix, 1-4 bpp)
	uint32 pitch = (p1 >> 15) & 0x07;				// Phrase pitch

//	int16 scanlineWidth = tom_getVideoModeWidth();
	uint8 * tom_ram_8 = tom_get_ram_pointer();
	uint8 * paletteRAM = &tom_ram_8[0x400];
	// This is OK as long as it's used correctly: For 16-bit RAM to RAM direct copies--NOT
	// for use when using endian-corrected data (i.e., any of the *_word_read functions!)
	uint16 * paletteRAM16 = (uint16 *)paletteRAM;

//	WriteLog("bitmap %ix? %ibpp at %i,? firstpix=? data=0x%.8x pitch %i hflipped=%s dwidth=? (linked to ?) RMW=%s Tranparent=%s\n",
//		iwidth, op_bitmap_bit_depth[bitdepth], xpos, ptr, pitch, (flags&FLAGS_HFLIP ? "yes" : "no"), (flags&FLAGS_READMODIFY ? "yes" : "no"), (flags&FLAGS_TRANSPARENT ? "yes" : "no"));

// Is it OK to have a 0 for the data width??? (i.e., undocumented?)
// Seems to be... Seems that dwidth *can* be zero (i.e., reuse same line) as well.
// Pitch == 0 is OK too...
//	if (!render || op_pointer == 0 || dwidth == 0 || ptr == 0 || pitch == 0)
//I'm not convinced that we need to concern ourselves with data & op_pointer here either!
	if (!render || iwidth == 0) // || data == 0 || op_pointer == 0)
		return;

//#define OP_DEBUG_BMP
//#ifdef OP_DEBUG_BMP
//	WriteLog("bitmap %ix%i %ibpp at %i,%i firstpix=%i data=0x%.8x pitch %i hflipped=%s dwidth=%i (linked to 0x%.8x) Transluency=%s\n",
//		iwidth, height, op_bitmap_bit_depth[bitdepth], xpos, ypos, firstPix, ptr, pitch, (flags&FLAGS_HFLIP ? "yes" : "no"), dwidth, op_pointer, (flags&FLAGS_READMODIFY ? "yes" : "no"));
//#endif

	int32 phraseWidthToPixels[8] = { 64, 32, 16, 8, 4, 2, 0, 0 };
//	int32 leftMargin = xpos, rightMargin = (xpos + (phraseWidthToPixels[depth] * iwidth)) - 1;
	int32 startPos = xpos, endPos = xpos +
		(!(flags & OPFLAG_REFLECT) ? (phraseWidthToPixels[depth] * iwidth) - 1
		: -((phraseWidthToPixels[depth] * iwidth) + 1));
	uint32 clippedWidth = 0, phraseClippedWidth = 0, dataClippedWidth = 0;//, phrasePixel = 0;
	bool in24BPPMode = (((GET16(tom_ram_8, 0x0028) >> 1) & 0x03) == 1 ? true : false);	// VMODE
	// Not sure if this is Jaguar Two only location or what...
	// From the docs, it is... If we want to limit here we should think of something else.
//	int32 limit = GET16(tom_ram_8, 0x0008);			// LIMIT
	int32 limit = 720;
	int32 lbufWidth = (!in24BPPMode ? limit - 1 : (limit / 2) - 1);	// Zero based limit...

	// If the image is completely to the left or right of the line buffer, then bail.
//If in REFLECT mode, then these values are swapped! !!! FIX !!! [DONE]
//There are four possibilities:
//  1. image sits on left edge and no REFLECT; starts out of bounds but ends in bounds.
//  2. image sits on left edge and REFLECT; starts in bounds but ends out of bounds.
//  3. image sits on right edge and REFLECT; starts out of bounds but ends in bounds.
//  4. image sits on right edge and no REFLECT; starts in bounds but ends out of bounds.
//Numbers 2 & 4 can be caught by checking the LBUF clip while in the inner loop,
// numbers 1 & 3 are of concern.
// This *indirectly* handles only cases 2 & 4! And is WRONG is REFLECT is set...!
//	if (rightMargin < 0 || leftMargin > lbufWidth)

// It might be easier to swap these (if REFLECTed) and just use XPOS down below...
// That way, you could simply set XPOS to leftMargin if !REFLECT and to rightMargin otherwise.
// Still have to be careful with the DATA and IWIDTH values though...

//	if ((!(flags & OPFLAG_REFLECT) && (rightMargin < 0 || leftMargin > lbufWidth))
//		|| ((flags & OPFLAG_REFLECT) && (leftMargin < 0 || rightMargin > lbufWidth)))
//		return;
	if ((!(flags & OPFLAG_REFLECT) && (endPos < 0 || startPos > lbufWidth))
		|| ((flags & OPFLAG_REFLECT) && (startPos < 0 || endPos > lbufWidth)))
		return;

	// Otherwise, find the clip limits and clip the phrase as well...
	// NOTE: I'm fudging here by letting the actual blit overstep the bounds of the
	//       line buffer, but it shouldn't matter since there are two unused line
	//       buffers below and nothing above and I'll at most write 8 bytes outside
	//       the line buffer... I could use a fractional clip begin/end value, but
	//       this makes the blit a *lot* more hairy. I might fix this in the future
	//       if it becomes necessary. (JLH)
	//       Probably wouldn't be *that* hairy. Just use a delta that tells the inner loop
	//       which pixel in the phrase is being written, and quit when either end of phrases
	//       is reached or line buffer extents are surpassed.

//This stuff is probably wrong as well... !!! FIX !!!
//The strange thing is that it seems to work, but that's no guarantee that it's bulletproof!
//Yup. Seems that JagMania doesn't work correctly with this...
//Dunno if this is the problem, but Atari Karts is showing *some* of the road now...
//	if (!(flags & OPFLAG_REFLECT))

/*
	if (leftMargin < 0)
		clippedWidth = 0 - leftMargin,
		phraseClippedWidth = clippedWidth / phraseWidthToPixels[depth],
		leftMargin = 0 - (clippedWidth % phraseWidthToPixels[depth]);
//		leftMargin = 0;

	if (rightMargin > lbufWidth)
		clippedWidth = rightMargin - lbufWidth,
		phraseClippedWidth = clippedWidth / phraseWidthToPixels[depth];//,
//		rightMargin = lbufWidth + (clippedWidth % phraseWidthToPixels[depth]);
//		rightMargin = lbufWidth;
*/
	// NOTE: We're just using endPos to figure out how much, if any, to clip by.
	// ALSO: There may be another case where we start out of bounds and end out of bounds...!
	if (startPos < 0)			// Case #1: Begin out, end in, L to R
		clippedWidth = 0 - startPos,
		dataClippedWidth = phraseClippedWidth = clippedWidth / phraseWidthToPixels[depth],
		startPos = 0 - (clippedWidth % phraseWidthToPixels[depth]);

	if (endPos < 0)				// Case #2: Begin in, end out, R to L
		clippedWidth = 0 - endPos,
		phraseClippedWidth = clippedWidth / phraseWidthToPixels[depth];

	if (endPos > lbufWidth)		// Case #3: Begin in, end out, L to R
		clippedWidth = endPos - lbufWidth,
		phraseClippedWidth = clippedWidth / phraseWidthToPixels[depth];

	if (startPos > lbufWidth)	// Case #4: Begin out, end in, R to L
		clippedWidth = startPos - lbufWidth,
		dataClippedWidth = phraseClippedWidth = clippedWidth / phraseWidthToPixels[depth],
		startPos = lbufWidth + (clippedWidth % phraseWidthToPixels[depth]);

	// If the image is sitting on the line buffer left or right edge, we need to compensate
	// by decreasing the image phrase width accordingly.
	iwidth -= phraseClippedWidth;

	// Also, if we're clipping the phrase we need to make sure we're in the correct part of
	// the pixel data.
//	data += phraseClippedWidth * (pitch << 3);
	data += dataClippedWidth * (pitch << 3);

	// NOTE: When the bitmap is in REFLECT mode, the XPOS marks the *right* side of the
	//       bitmap! This makes clipping & etc. MUCH, much easier...!
//	uint32 lbufAddress = 0x1800 + (!in24BPPMode ? leftMargin * 2 : leftMargin * 4);
	uint32 lbufAddress = 0x1800 + (!in24BPPMode ? startPos * 2 : startPos * 4);
	uint8 * currentLineBuffer = &tom_ram_8[lbufAddress];

	// Render.

// Hmm. We check above for 24 BPP mode, but don't do anything about it below...
// If we *were* in 24 BPP mode, how would you convert CRY to RGB24? Seems to me
// that if you're in CRY mode then you wouldn't be able to use 24 BPP bitmaps
// anyway.

	if (depth == 0)									// 1 BPP
	{
//		uint32 paletteIndex = index << 1;
		// The LSB is OPFLAG_REFLECT, so sign extend it and or 2 into it.
		int32 lbufDelta = ((int8)((flags << 7) & 0xFF) >> 5) | 0x02;

		while (iwidth--)
		{
			// Fetch phrase...
			uint64 pixels = ((uint64)jaguar_long_read(data) << 32) | jaguar_long_read(data + 4);
			data += pitch << 3;						// Multiply pitch * 8 (optimize: precompute this value)

			for(int i=0; i<64; i++)
			{
				uint8 bit = pixels >> 63;
// Seems to me that both of these are in the same endian, so we could cast it as
// uint16 * and do straight across copies (what about 24 bpp? Treat it differently...)
// This only works for the palettized modes (1 - 8 BPP), since we actually have to
// copy data from memory in 16 BPP mode (or does it? Isn't this the same as the CLUT case?)
// No, it isn't because we read the memory in an endian safe way--this *won't* work...
				if ((flags & OPFLAG_TRANS) && bit == 0)
					;	// Do nothing...
				else
				{
					if (!(flags & OPFLAG_RMW))
//Optimize: Set palleteRAM16 to beginning of palette RAM + index*2 and use only [bit] as index...
						*(uint16 *)currentLineBuffer = paletteRAM16[index | bit];
					else
						*currentLineBuffer = 
							BLEND_CR(*currentLineBuffer, paletteRAM[(index | bit) << 1]),
						*(currentLineBuffer + 1) = 
							BLEND_Y(*(currentLineBuffer + 1), paletteRAM[((index | bit) << 1) + 1]);
				}

				currentLineBuffer += lbufDelta;
				pixels <<= 1;
			}
		}
	}
	else if (depth == 1)							// 2 BPP
	{
		index &= 0xFC;								// Top six bits form CLUT index
		// The LSB is OPFLAG_REFLECT, so sign extend it and or 2 into it.
		int32 lbufDelta = ((int8)((flags << 7) & 0xFF) >> 5) | 0x02;

		while (iwidth--)
		{
			// Fetch phrase...
			uint64 pixels = ((uint64)jaguar_long_read(data) << 32) | jaguar_long_read(data + 4);
			data += pitch << 3;						// Multiply pitch * 8 (optimize: precompute this value)

			for(int i=0; i<32; i++)
			{
				uint8 bits = pixels >> 62;
// Seems to me that both of these are in the same endian, so we could cast it as
// uint16 * and do straight across copies (what about 24 bpp? Treat it differently...)
// This only works for the palettized modes (1 - 8 BPP), since we actually have to
// copy data from memory in 16 BPP mode (or does it? Isn't this the same as the CLUT case?)
// No, it isn't because we read the memory in an endian safe way--this *won't* work...
				if ((flags & OPFLAG_TRANS) && bits == 0)
					;	// Do nothing...
				else
				{
					if (!(flags & OPFLAG_RMW))
						*(uint16 *)currentLineBuffer = paletteRAM16[index | bits];
					else
						*currentLineBuffer = 
							BLEND_CR(*currentLineBuffer, paletteRAM[(index | bits) << 1]),
						*(currentLineBuffer + 1) = 
							BLEND_Y(*(currentLineBuffer + 1), paletteRAM[((index | bits) << 1) + 1]);
				}

				currentLineBuffer += lbufDelta;
				pixels <<= 2;
			}
		}
	}
	else if (depth == 2)							// 4 BPP
	{
		index &= 0xF0;								// Top four bits form CLUT index
		// The LSB is OPFLAG_REFLECT, so sign extend it and or 2 into it.
		int32 lbufDelta = ((int8)((flags << 7) & 0xFF) >> 5) | 0x02;

		while (iwidth--)
		{
			// Fetch phrase...
			uint64 pixels = ((uint64)jaguar_long_read(data) << 32) | jaguar_long_read(data + 4);
			data += pitch << 3;						// Multiply pitch * 8 (optimize: precompute this value)

			for(int i=0; i<16; i++)
			{
				uint8 bits = pixels >> 60;
// Seems to me that both of these are in the same endian, so we could cast it as
// uint16 * and do straight across copies (what about 24 bpp? Treat it differently...)
// This only works for the palettized modes (1 - 8 BPP), since we actually have to
// copy data from memory in 16 BPP mode (or does it? Isn't this the same as the CLUT case?)
// No, it isn't because we read the memory in an endian safe way--this *won't* work...
				if ((flags & OPFLAG_TRANS) && bits == 0)
					;	// Do nothing...
				else
				{
					if (!(flags & OPFLAG_RMW))
						*(uint16 *)currentLineBuffer = paletteRAM16[index | bits];
					else
						*currentLineBuffer = 
							BLEND_CR(*currentLineBuffer, paletteRAM[(index | bits) << 1]),
						*(currentLineBuffer + 1) = 
							BLEND_Y(*(currentLineBuffer + 1), paletteRAM[((index | bits) << 1) + 1]);
				}

				currentLineBuffer += lbufDelta;
				pixels <<= 4;
			}
		}
	}
	else if (depth == 3)							// 8 BPP
	{
		// The LSB is OPFLAG_REFLECT, so sign extend it and or 2 into it.
		int32 lbufDelta = ((int8)((flags << 7) & 0xFF) >> 5) | 0x02;

		while (iwidth--)
		{
			// Fetch phrase...
			uint64 pixels = ((uint64)jaguar_long_read(data) << 32) | jaguar_long_read(data + 4);
			data += pitch << 3;						// Multiply pitch * 8 (optimize: precompute this value)

			for(int i=0; i<8; i++)
			{
				uint8 bits = pixels >> 56;
// Seems to me that both of these are in the same endian, so we could cast it as
// uint16 * and do straight across copies (what about 24 bpp? Treat it differently...)
// This only works for the palettized modes (1 - 8 BPP), since we actually have to
// copy data from memory in 16 BPP mode (or does it? Isn't this the same as the CLUT case?)
// No, it isn't because we read the memory in an endian safe way--this *won't* work...
				if ((flags & OPFLAG_TRANS) && bits == 0)
					;	// Do nothing...
				else
				{
					if (!(flags & OPFLAG_RMW))
						*(uint16 *)currentLineBuffer = paletteRAM16[bits];
					else
						*currentLineBuffer = 
							BLEND_CR(*currentLineBuffer, paletteRAM[bits << 1]),
						*(currentLineBuffer + 1) = 
							BLEND_Y(*(currentLineBuffer + 1), paletteRAM[(bits << 1) + 1]);
				}

				currentLineBuffer += lbufDelta;
				pixels <<= 8;
			}
		}
	}
	else if (depth == 4)							// 16 BPP
	{
		// The LSB is OPFLAG_REFLECT, so sign extend it and or 2 into it.
		int32 lbufDelta = ((int8)((flags << 7) & 0xFF) >> 5) | 0x02;

		while (iwidth--)
		{
			// Fetch phrase...
			uint64 pixels = ((uint64)jaguar_long_read(data) << 32) | jaguar_long_read(data + 4);
			data += pitch << 3;						// Multiply pitch * 8 (optimize: precompute this value)

			for(int i=0; i<4; i++)
			{
				uint8 bitsHi = pixels >> 56, bitsLo = pixels >> 48;
// Seems to me that both of these are in the same endian, so we could cast it as
// uint16 * and do straight across copies (what about 24 bpp? Treat it differently...)
// This only works for the palettized modes (1 - 8 BPP), since we actually have to
// copy data from memory in 16 BPP mode (or does it? Isn't this the same as the CLUT case?)
// No, it isn't because we read the memory in an endian safe way--it *won't* work...
				if ((flags & OPFLAG_TRANS) && (bitsLo | bitsHi) == 0)
					;	// Do nothing...
				else
				{
					if (!(flags & OPFLAG_RMW))
						*currentLineBuffer = bitsHi,
						*(currentLineBuffer + 1) = bitsLo;
					else
						*currentLineBuffer = 
							BLEND_CR(*currentLineBuffer, bitsHi),
						*(currentLineBuffer + 1) = 
							BLEND_Y(*(currentLineBuffer + 1), bitsLo);
				}

				currentLineBuffer += lbufDelta;
				pixels <<= 16;
			}
		}
	}
	else if (depth == 5)							// 24 BPP
	{
WriteLog("OP: Writing 24 BPP bitmap!\n");
		// Not sure, but I think RMW only works with 16 BPP and below, and only in CRY mode...
		// The LSB is OPFLAG_REFLECT, so sign extend it and or 4 into it.
		int32 lbufDelta = ((int8)((flags << 7) & 0xFF) >> 4) | 0x04;

		while (iwidth--)
		{
			// Fetch phrase...
			uint64 pixels = ((uint64)jaguar_long_read(data) << 32) | jaguar_long_read(data + 4);
			data += pitch << 3;						// Multiply pitch * 8 (optimize: precompute this value)

			for(int i=0; i<2; i++)
			{
				uint8 bits3 = pixels >> 56, bits2 = pixels >> 48,
					bits1 = pixels >> 40, bits0 = pixels >> 32;
// Seems to me that both of these are in the same endian, so we could cast it as
// uint16 * and do straight across copies (what about 24 bpp? Treat it differently...)
// This only works for the palettized modes (1 - 8 BPP), since we actually have to
// copy data from memory in 16 BPP mode (or does it? Isn't this the same as the CLUT case?)
// No, it isn't because we read the memory in an endian safe way--it *won't* work...
				if ((flags & OPFLAG_TRANS) && (bits3 | bits2 | bits1 | bits0) == 0)
					;	// Do nothing...
				else
					*currentLineBuffer = bits3,
					*(currentLineBuffer + 1) = bits2,
					*(currentLineBuffer + 2) = bits1,
					*(currentLineBuffer + 3) = bits0;

				currentLineBuffer += lbufDelta;
				pixels <<= 32;
			}
		}
	}
}

//
// *** NEW ***
// Store scaled bitmap in line buffer
//
void OPProcessScaledBitmap(int scanline, uint64 p0, uint64 p1, uint64 p2, bool render)
{
	int32 xpos = (((int32)((p1 << 20) & 0xFFFFFFFF)) >> 20) - tom_getHBlankWidthInPixels();
//	uint16 ypos = ((p0 & 0x3FF8) >> 3) / 2;
	uint16 iwidth = ((p1 >> 28) & 0x3FF) * 4;
	uint16 dwidth = ((p1 >> 18) & 0x3FF) * 4;		// Unsigned!
//	uint16 height = (p0 >> 14) & 0x3FF;				// Unsigned!
//	uint32 link = ((p0 >> 24) & 0x7FFFF) << 3;
	uint32 ptr = (p0 >> 40) & 0xFFFFF8;
//unused	uint32 firstPix = (p1 >> 49) & 0x3F;
	uint8 flags = (p1 >> 45) & 0x0F;
	uint8 idx = (p1 >> 38) & 0x7F;
	uint8 pitch = (p1 >> 15) & 0x07;
	uint8 bitdepth = (p1 >> 12) & 0x07;

	int16 scanline_width = tom_getVideoModeWidth();
	uint8 * tom_ram_8 = tom_get_ram_pointer();
	uint8 * current_line_buffer = &tom_ram_8[0x1800];

	uint32 vscale_fixed3p5 = (p2 >> 8) & 0xFF;
	uint32 hscale_fixed3p5 = p2 & 0xFF;
	float vscale = (float)vscale_fixed3p5 / 32.0f, hscale = (float)hscale_fixed3p5 / 32.0f;

//No hacks!
/*	if (jaguar_mainRom_crc32==0x5a5b9c68) // atari karts
	{
		if (vscale == 0.0f) 
			vscale = 1.0f;

		if (ypos == 0)
			ypos = scanline;
	}*/
#ifdef OP_DEBUG_BMP
	if (xpos == -3)
		WriteLog("[scanline %i] %ix%i scaled to %ix%i scale (%f, %f)%i bpp pitch %i at (%i,%i) @ 0x%.8x Transluency=%s\n",
			scanline, iwidth,height, (int)(iwidth*hscale), (int)(height*vscale), hscale, vscale,
			op_bitmap_bit_depth[bitdepth], pitch, xpos, ypos, ptr, (flags&FLAGS_READMODIFY) ? "yes" : "no");
#endif
//No hacks!
/*	if (jaguar_mainRom_crc32==0x2f032271)
		ypos += 8;*/

	if (!render || dwidth == 0 || ptr == 0 || pitch == 0)
		return;

	if (bitdepth <= 3)					// 1, 2, 4, 8 BPP
		iwidth *= 2, dwidth *= 2;

	uint16 scaled_width = (uint16)((float)iwidth * hscale);

	if (op_bitmap_bit_depth[bitdepth] == 4)	// why ?
		scaled_width *= 2;

	if (op_bitmap_bit_depth[bitdepth] == 2)	// why ?
		scaled_width *= 4;

	if (op_bitmap_bit_depth[bitdepth] == 1)	// why ?
		scaled_width *= 8;

	// visible ?
/*	if ((scanline < ypos) || (scanline > (ypos + scaled_height)) || ((xpos + scaled_width) < 0)
		|| (xpos >= scanline_width))*/
	if ((xpos + scaled_width) < 0 || xpos >= scanline_width)
		return;

	if (xpos < 0)
	{
		scaled_width += xpos;
		ptr += (pitch * op_bitmap_bit_size[bitdepth] * ((uint32)((-xpos) / hscale))) >> 16;
		xpos = 0;
	}

	if (iwidth <= 0)
		return;

	if (flags & FLAGS_HFLIP)
	{
		if (xpos < 0 || (xpos-scaled_width) >= scanline_width)
			return;

		if ((xpos - scaled_width) < 0)
			scaled_width = xpos;
	}
	else
	{
		if ((xpos + scaled_width) < 0 || xpos >= scanline_width)
			return;

		if ((xpos + scaled_width) > scanline_width)
			scaled_width = scanline_width-xpos;
	}
	
	current_line_buffer += xpos * 2;

	int32 hscale_fixed = (int32)(65536.0f / hscale);
	int32 cnt = 0;

	if (op_bitmap_bit_depth[bitdepth] == 1)
	{
		if (pitch == 1)
		{
#include "zbmpop1.h"
		}
		else
		{
#include "zbmpop1p.h"
		}
	}
	else if (op_bitmap_bit_depth[bitdepth] == 2)
	{
		if (pitch == 1)
		{
#include "zbmpop2.h"
		}
		else
		{
#include "zbmpop2p.h"
		}
	}
	else if (op_bitmap_bit_depth[bitdepth] == 4)
	{
		if (pitch == 1)
		{
#include "zbmpop4.h"
		}
		else
		{
#include "zbmpop4p.h"
		}
	}
	else if (op_bitmap_bit_depth[bitdepth] == 8)
	{
		if (pitch == 1)
		{
#include "zbmpop8.h"
		}
		else
		{
#include "zbmpop8p.h"
		}
	}
	else if (op_bitmap_bit_depth[bitdepth] == 16)
	{
		if (pitch == 1)
		{
#include "zbmpop16.h"
		}
		else
		{
#include "zbmpop16p.h"
		}
	}
	else
		WriteLog("(unimplemented) %i bpp scaled bitmap\n",op_bitmap_bit_depth[bitdepth]);
}
