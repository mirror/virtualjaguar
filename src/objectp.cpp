
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "jaguar.h"


//#define OP_DEBUG
//#define OP_DEBUG_BMP
static int cnt = 0;
extern uint32 jaguar_mainRom_crc32;

static uint8 * op_blend_y;
static uint8 * op_blend_cc;

#define BLEND_Y(dst,src) op_blend_y[(((uint16)dst)<<8) | ((uint16)(src))]
#define BLEND_CC(dst,src) op_blend_cc[(((uint16)dst)<<8) | ((uint16)(src))]


static uint8 objectp_ram[0x40];
uint8 objectp_running;

uint8 objectp_stop_reading_list;

#define OBJECT_TYPE_BITMAP	0
#define OBJECT_TYPE_SCALE	1
#define OBJECT_TYPE_GPU		2
#define OBJECT_TYPE_BRANCH	3
#define OBJECT_TYPE_STOP	4

#define CONDITION_EQUAL			0
#define CONDITION_LESS_THAN		1
#define CONDITION_GREATER_THAN	2
#define CONDITION_OP_FLAG_SET	3
#define CONDITION_SECOND_HALF_LINE	4

#define FLAGS_RELEASE		8
#define FLAGS_TRANSPARENT	4
#define FLAGS_READMODIFY	2
#define FLAGS_HFLIP			1


static char * condition_to_str[8] = {"==","<",">","(opflag set)","(second half line)","?","?","?"};

static uint8 op_bitmap_bit_depth[8]={1,           2,          4,         8,       16,      24,      32,      0      };
static uint32 op_bitmap_bit_size[8]={0.125*65536, 0.25*65536, 0.5*65536, 1*65536, 2*65536, 1*65536, 1*65536, 1*65536};
static uint32 op_pointer;
static int cnt_bitmap = 0;

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////////
void op_process_bitmap(int16 *backbuffer, int scanline, uint64 p0, uint64 p1, int render)
{
	uint8	bitdepth=(p1>>12)&0x07;
	
	int16	ypos=(((p0>>3)&0x3ff)/2);
	int32	xpos=(((int32)((p1<<20)&0xffffffff))>>20)-tom_getHBlankWidthInPixels();
	int32	iwidth=((p1>>28)&0x3ff)*4;
	uint32	dwidth=((p1>>18)&0x3ff)*4;
	int16	height=((p0>>14)&0x3ff)-1;
	uint32	link=((p0>>24)&0x7ffff)<<3;
	uint32	ptr=(((p0>>43)&0x1fffff)<<3);
	uint32	firstPix=(p1>>49)&0x3f;
	uint8	flags=(p1>>45)&0x0f;
	uint8	idx=(p1>>38)&0x7f;
	uint32	pitch=(p1>>15)&0x07;
	int16	scanline_width=tom_getVideoModeWidth();
	uint8	*tom_ram_8=tom_get_ram_pointer();
	uint8	*current_line_buffer=&tom_ram_8[0x1800];
	uint8	*paletteRam=&tom_ram_8[0x400];
	uint8	 doom_hack=0;

	op_pointer=link;

	if ((!render)||(op_pointer==0)||(height<0)||(dwidth<0)||(ptr==0)||(pitch==0))
	{
		return;
	}
	if (iwidth==0)
	{
		dwidth=iwidth=height;
	}
	if (jaguar_mainRom_crc32==0x5e705756) // doom
	{
		if ((iwidth==160)&&(height==179)&&(xpos==-9)&&(ypos==24))
		{
			doom_hack=1;
			xpos=0;
		}
	}
	// il y a un offset vertical de 20 pour un des bitmaps
	// dans dinon olympics pour une raison encore inconnue
	if (jaguar_mainRom_crc32==0x3c7bfda8)
	{
		if ((iwidth==220)&&(height==184)&&(ypos==0))
		{
			ypos=20;
		}
	}
	else
	if (jaguar_mainRom_crc32==0x2f032271)
	{
		ypos+=8;
	}
	if (op_bitmap_bit_depth[bitdepth]==24)
	{
		iwidth*=2;
		dwidth*=2;
	}
	if (op_bitmap_bit_depth[bitdepth]==8)
	{
		iwidth*=2;
		dwidth*=2;
	}
	if (op_bitmap_bit_depth[bitdepth]==4)
	{
		iwidth*=2;
		dwidth*=2;
	}
	if (op_bitmap_bit_depth[bitdepth]==2)
	{
		iwidth*=2;
		dwidth*=2;
	}
	if (op_bitmap_bit_depth[bitdepth]==1)
	{
		iwidth*=2;
		dwidth*=2;
	}
	// Power Drive Rally (fixed size bitmaps have a 240 lines vertical offset)
	// but could well be a modulo 240
	if (jaguar_mainRom_crc32==0x0ab88d90)
		ypos%=240;


#ifdef OP_DEBUG_BMP
	fprintf(log_get(),"bitmap %ix%i %ibpp at %i,%i firstpix=%i ptr=0x%.8x pitch %i hflipped=%s dwidth=%i (linked to 0x%.8x) Transluency=%s\n",iwidth,height,op_bitmap_bit_depth[bitdepth],xpos,ypos,firstPix,ptr,pitch,flags&FLAGS_HFLIP?"yes":"no",dwidth,op_pointer,(flags&FLAGS_READMODIFY)?"yes":"no");
#endif

	if ((scanline<ypos)||(scanline>(ypos+height)))
	{
		return;
	}
	// seek to the good bitmap scanline
	// not sure for palettized modes
	if (op_bitmap_bit_depth[bitdepth] > 8)
	{
		ptr += ((dwidth * op_bitmap_bit_size[bitdepth]) >> 16) * (scanline - ypos);
	}
	else
		ptr += dwidth * (scanline - ypos);

	if (xpos < 0)
	{
		iwidth += xpos;
		ptr += (pitch * op_bitmap_bit_size[bitdepth] * (-xpos)) >> 16;
		xpos = 0;
	}

	if (iwidth <= 0)
	{
#ifdef OP_DEBUG	
	fprintf(log_get(),"not rendering because iwidth<0\n");
#endif
		return;
	}
	
	if (flags&FLAGS_HFLIP)
	{
		if ((xpos<0)||((xpos-iwidth)>=scanline_width))
			return;

		if ((xpos-iwidth)<0)
			iwidth=xpos;
	}
	else
	{
		if (((xpos+iwidth)<0)||(xpos>=scanline_width))
			return;

		if ((xpos+iwidth)>scanline_width)
			iwidth=scanline_width-xpos;
	}


	current_line_buffer+=(xpos*2); // 2 in 16 bpp modes (need to take the mode into account)

	// doom switches the resolution from 320 to 160 to double the display width
	// this is not yet supported
	if (doom_hack)
	{
		while (iwidth)
		{
			uint8 d0=jaguar_byte_read(ptr+0);
			uint8 d1=jaguar_byte_read(ptr+1);
			*current_line_buffer++=d0;
			*current_line_buffer++=d1;
			*current_line_buffer++=d0;
			*current_line_buffer++=d1;
			ptr+=2;
			iwidth--;
		}
		return;
	}
	if (op_bitmap_bit_depth[bitdepth]==1)
	{
		if (pitch==1)
		{
			#include "include/fbmpop1.h"
		}
		else
		{
			#include "include/fbmpop1p.h"
		}
	}
	else
	if (op_bitmap_bit_depth[bitdepth]==2)
	{
		if (pitch==1)
		{
			#include "include/fbmpop2.h"
		}
		else
		{
			#include "include/fbmpop2p.h"
		}
	}
	else
	if (op_bitmap_bit_depth[bitdepth]==4)
	{
		if (pitch==1)
		{
			#include "include/fbmpop4.h"
		}
		else
		{
			#include "include/fbmpop4p.h"
		}
	}
	else
	if (op_bitmap_bit_depth[bitdepth]==8)
	{
		if (pitch==1)
		{
			#include "include/fbmpop8.h"
		}
		else
		{
			#include "include/fbmpop8p.h"
		}
	}
	else
	if (op_bitmap_bit_depth[bitdepth]==16)
	{
		if (pitch==1)
		{
			#include "include/fbmpop16.h"
		}
		else
		{
			#include "include/fbmpop16p.h"
		}
	}
	else
	if (op_bitmap_bit_depth[bitdepth]==24)
	{
		if (pitch==1)
		{
			#include "include/fbmpop24.h"
		}
		else
		{
			#include "include/fbmpop24p.h"
		}
	}
}
//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////////
void op_process_scaled_bitmap(int16 *backbuffer, int scanline, uint64 p0, uint64 p1, uint64 p2, int render)
{
	int32	xpos=(((int32)((p1<<20)&0xffffffff))>>20)-tom_getHBlankWidthInPixels();
	int16	ypos=(((p0&0x3ff8)>>3)/2);
	uint16	iwidth=((p1>>28)&0x3ff)*4;
	uint16	dwidth=((p1>>18)&0x3ff)*4;
	uint16	height=(p0>>14)&0x3ff;
	uint32	link=((p0>>24)&0x7ffff)<<3;
	uint32	ptr=((p0>>43)&0x1fffff)<<3;
	uint32	firstPix=(p1>>49)&0x3f;
	uint8	flags=(p1>>45)&0x0f;
	uint8	idx=(p1>>38)&0x7f; 
	uint8	pitch=(p1>>15)&0x07;
	uint8	bitdepth=(p1>>12)&0x07;
	int16	scanline_width=tom_getVideoModeWidth();
	uint8	*tom_ram_8=tom_get_ram_pointer();
	uint8	*current_line_buffer=&tom_ram_8[0x1800];
	float	vscale;
	float	hscale;
	
	uint32	vscale_fixed3p5=((p2>>8)&0xff);
	uint32	hscale_fixed3p5=(p2&0xff);
	vscale=((float)vscale_fixed3p5)/32;
	hscale=((float)hscale_fixed3p5)/32;

	op_pointer=link;

	if (jaguar_mainRom_crc32==0x5a5b9c68) // atari karts
	{
		if (vscale == 0.0f) 
			vscale = 1.0f;

		if (ypos == 0)
			ypos = scanline;
	}
#ifdef OP_DEBUG_BMP
	if (xpos == -3)
		fprintf(log_get(),"[scanline %i] %ix%i scaled to %ix%i scale (%f, %f)%i bpp pitch %i at (%i,%i) @ 0x%.8x Transluency=%s\n",
																			  scanline,
																			  iwidth,height,
																			  (int)(iwidth*hscale),
																			  (int)(height*vscale),
																			  hscale, vscale,
																			  op_bitmap_bit_depth[bitdepth],pitch,xpos,ypos,ptr,(flags&FLAGS_READMODIFY)?"yes":"no");
#endif
	if (jaguar_mainRom_crc32==0x2f032271)
	{
		ypos+=8;
	}

	if ((render==0)||(op_pointer==0)||(height<0)||(dwidth<0)||(ptr==0)||(pitch==0))
	{
		return;
	}

	if (op_bitmap_bit_depth[bitdepth]==8)
	{
		iwidth*=2;
		dwidth*=2;
	}
	if (op_bitmap_bit_depth[bitdepth]==4)
	{
		iwidth*=2;
		dwidth*=2;
	}
	if (op_bitmap_bit_depth[bitdepth]==2)
	{
		iwidth*=2;
		dwidth*=2;
	}
	if (op_bitmap_bit_depth[bitdepth]==1)
	{
		iwidth*=2;
		dwidth*=2;
	}

	uint16	scaled_width=iwidth*hscale;
	uint16	scaled_height=height*vscale;

	if (op_bitmap_bit_depth[bitdepth]==4)	// why ?
		scaled_width*=2;

	if (op_bitmap_bit_depth[bitdepth]==2)	// why ?
		scaled_width*=4;

	if (op_bitmap_bit_depth[bitdepth]==1)	// why ?
		scaled_width*=8;

	// seek to the good bitmap scanline
	// not sure for palettized modes
	if (op_bitmap_bit_depth[bitdepth]>8)
		ptr+=((dwidth*op_bitmap_bit_size[bitdepth])>>16)*((uint32)((scanline-ypos)/vscale));
	else
		ptr+=dwidth*((uint32)((scanline-ypos)/vscale));

	// visible ?
	if ((scanline<ypos)||(scanline>(ypos+scaled_height))||((xpos+scaled_width)<0)||(xpos>=scanline_width))
		return;

	
	if (xpos<0)
	{
		scaled_width+=xpos;
		ptr+=(pitch*op_bitmap_bit_size[bitdepth]*((uint32)((-xpos)/hscale)))>>16;
		xpos=0;
	}

	if (iwidth<=0)
		return;

	if (flags&FLAGS_HFLIP)
	{
		if ((xpos<0)||((xpos-scaled_width)>=scanline_width))
			return;

		if ((xpos-scaled_width)<0)
			scaled_width=xpos;
	}
	else
	{
		if (((xpos+scaled_width)<0)||(xpos>=scanline_width))
			return;

		if ((xpos+scaled_width)>scanline_width)
			scaled_width=scanline_width-xpos;
	}
	
	current_line_buffer+=(xpos*2);

	int32 hscale_fixed=(int32)(65536.0f/hscale);
	int32 cnt=0;

	if (op_bitmap_bit_depth[bitdepth]==1)
	{
		if (pitch==1)
		{
			#include "include/zbmpop1.h"
		}
		else
		{
			#include "include/zbmpop1p.h"
		}
	}
	else
	if (op_bitmap_bit_depth[bitdepth]==2)
	{
		if (pitch==1)
		{
			#include "include/zbmpop2.h"
		}
		else
		{
			#include "include/zbmpop2p.h"
		}
	}
	else
	if (op_bitmap_bit_depth[bitdepth]==4)
	{
		if (pitch==1)
		{
			#include "include/zbmpop4.h"
		}
		else
		{
			#include "include/zbmpop4p.h"
		}
	}
	else
	if (op_bitmap_bit_depth[bitdepth]==8)
	{
		if (pitch==1)
		{
			#include "include/zbmpop8.h"
		}
		else
		{
			#include "include/zbmpop8p.h"
		}
	}
	else
	if (op_bitmap_bit_depth[bitdepth]==16)
	{
		if (pitch==1)
		{
			#include "include/zbmpop16.h"
		}
		else
		{
			#include "include/zbmpop16p.h"
		}
	}
	else
		fprintf(log_get(),"(unimplemented) %i bpp scaled bitmap\n",op_bitmap_bit_depth[bitdepth]);
}
//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////////
uint64 op_load_phrase(uint32 offset)
{
	offset &= (~0x07);						// 8 byte alignment

	uint64 data = jaguar_byte_read(offset++);
	data <<= 8;
	data |= jaguar_byte_read(offset++);
	data <<= 8;
	data |= jaguar_byte_read(offset++);
	data <<= 8;
	data |= jaguar_byte_read(offset++);
	data <<= 8;
	data |= jaguar_byte_read(offset++);
	data <<= 8;
	data |= jaguar_byte_read(offset++);
	data <<= 8;
	data |= jaguar_byte_read(offset++);
	data <<= 8;
	data |= jaguar_byte_read(offset++);

	return data;
}
//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////////
void op_process_list(int16 * backbuffer, int scanline, int render)
{
	uint64 p0;	

	cnt_bitmap = 0;

	if (scanline < tom_get_vdb())
		return;

	if (scanline >= 525)//tom_getVideoModeHeight()+tom_get_vdb())
		return;

	op_pointer = op_get_list_pointer();

	objectp_stop_reading_list = 0;
	cnt = 0;

//	if (op_pointer) fprintf(log_get()," new op list at 0x%.8x scanline %i\n",op_pointer,scanline);
	while (op_pointer)
	{
		if (objectp_stop_reading_list)
			return;
			
		p0 = op_load_phrase(op_pointer);
		op_pointer += 8;
		

//		fprintf(log_get(),"0x%.8x type %i\n",op_pointer,((uint8)p0&0x07));		
		switch ((uint8)p0&0x07)
		{
		case OBJECT_TYPE_BITMAP:
		{
			uint64 p1 = op_load_phrase(op_pointer);
			op_pointer += 8;
			op_process_bitmap(backbuffer, scanline, p0, p1, render);
			break;
		}
		case OBJECT_TYPE_SCALE:
		{
			uint64 p1 = op_load_phrase(op_pointer);
			op_pointer += 8;
			uint64 p2 = op_load_phrase(op_pointer);
			op_pointer += 8;
			op_process_scaled_bitmap(backbuffer, scanline, p0, p1, p2, render);
			break;
		}
		case OBJECT_TYPE_GPU:
		{
			uint64 data = p0 >> 3;
			op_set_current_object(p0);
			gpu_set_irq_line(3, 1); 
			break;
		}
		case OBJECT_TYPE_BRANCH:
		{
			uint16 vcnt = (p0>>3) & 0x7FF;
			uint8  cc   = (p0>>14) & 0x03;
			uint32 link = ((p0>>24) & 0x1FFFFF) << 3;
			
//			if ((vcnt!=507)&&(vcnt!=25))
//				fprintf(log_get(),"\t%i%s%i link=0x%.8x\n",scanline,condition_to_str[cc],vcnt>>1,link);
			switch (cc)
			{
			case CONDITION_EQUAL:
				if ((vcnt != 0x7FF) && (vcnt & 0x01))
				 	vcnt ^= 0x01;
				if (((2*tom_get_scanline()) == vcnt) || (vcnt == 0x7FF))
					op_pointer=link;
				break;
			case CONDITION_LESS_THAN:
				if ((2*tom_get_scanline()) < vcnt)
					op_pointer=link;
				break;
			case CONDITION_GREATER_THAN:
				if ((2*tom_get_scanline()) > vcnt)
					op_pointer=link;
				break;
			case CONDITION_OP_FLAG_SET:
				if (op_get_status_register() & 0x01)
					op_pointer=link;
				break;
			case CONDITION_SECOND_HALF_LINE:
				fprintf(log_get(), "op: unexpected CONDITION_SECOND_HALF_LINE in BRANCH object\nop: shuting down\n");
				fclose(log_get());
				exit(0);
				break;
			default:
				fprintf(log_get(),"op: unimplemented branch condition %i\n", cc);
			}
			break;
		}
		case OBJECT_TYPE_STOP:
		{
			// unsure
			op_set_status_register(((p0>>3) & 0xFFFFFFFF));
			
			if (p0 & 0x8)
			{
				tom_set_pending_object_int();
				if ((tom_irq_enabled(2)) && (jaguar_interrupt_handler_is_valid(64)))
				{
//					s68000interrupt(7,64);
//					s68000flushInterrupts();
					m68k_set_irq(7);				// Cause an NMI to occur...
				}
			}

			return;
			break;
		}
		default:
		{
			fprintf(log_get(),"op: unknown object type %i\n", ((uint8)p0 & 0x07)); 
			return;
		}
		}
	}
}
//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////////
void op_init(void)
{
	memory_malloc_secure((void **)&op_blend_y, 0x10000, "Jaguar Object processor Y blend lookup table");
	memory_malloc_secure((void **)&op_blend_cc, 0x10000, "Jaguar Object processor C blend lookup table");

	for(int i=0; i<256*256; i++)
	{
		int y = (i >> 8) & 0xFF;
		int dy = (int8)(i & 0xFF);
		y += dy;
		if (y < 0)
			y = 0;
		else if (y > 0xFF)
			y = 0xFF;
		op_blend_y[i] = y;
	}

	for(int i=0; i<256*256; i++)
	{
		int cl = (i >> 8) & 0xFF;
		int dcl = (int8)(i & 0xFF);
		cl += dcl;
		if (cl < 0)
			cl = 0;
		else if (cl > 0xFF)
			cl = 0xFF;
		op_blend_cc[i] = cl;
	}
	op_reset();
}
//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////////
void op_reset(void)
{
	memset(objectp_ram, 0x00, 0x18);
	objectp_running = 0;
}
//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////////
void op_done(void)
{
}
//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////////
void op_byte_write(uint32 offset, uint8 data)
{
	offset &= 0x3F;
	objectp_ram[offset] = data;
}
//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////////
void op_word_write(uint32 offset, uint16 data)
{
	offset &= 0x3F;
	objectp_ram[offset] = (data >> 8) & 0xFF;
	objectp_ram[offset+1] = data & 0xFF;

}
//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////////
uint8 op_byte_read(uint32 offset)
{
	offset &= 0x3F;
	return objectp_ram[offset];
}
//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////////
uint16 op_word_read(uint32 offset)
{
	return (objectp_ram[offset & 0x3F] << 8) | objectp_ram[(offset+1) & 0x3F];
}
//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////////
uint32 op_get_list_pointer(void)
{
	uint32 ptr = objectp_ram[0x22];
	ptr <<= 8;
	ptr |= objectp_ram[0x23];
	ptr <<= 8;
	ptr |= objectp_ram[0x20];
	ptr <<= 8;
	ptr |= objectp_ram[0x21];
	ptr &= ~0x07;

	return ptr;//*/
//	return (objectp_ram[0x20] << 24) | (objectp_ram[0x21] << 16) | (objectp_ram[0x22] << 8) | objectp_ram[0x23];
}
//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////////
uint32 op_get_status_register(void)
{
	uint32 ptr = objectp_ram[0x26];
	ptr <<= 8;
	ptr |= objectp_ram[0x27];
	ptr <<= 8;
	ptr |= objectp_ram[0x28];
	ptr <<= 8;
	ptr |= objectp_ram[0x29];

	return ptr;
}
//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////////
void op_set_status_register(uint32 data)
{
	objectp_ram[0x26] = (data & 0xFF000000) >> 24;
	objectp_ram[0x27] = (data & 0x00FF0000) >> 16;
	objectp_ram[0x28] = (data & 0x0000FF00) >> 8;
	objectp_ram[0x29] |= (data & 0xFE);
}
//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////////
void op_set_current_object(uint64 object)
{
	// less significant 32 bits first
	objectp_ram[0x13] = object&0xff; object>>=8;
	objectp_ram[0x12] = object&0xff; object>>=8;
	objectp_ram[0x11] = object&0xff; object>>=8;
	objectp_ram[0x10] = object&0xff; object>>=8;

	objectp_ram[0x17] = object&0xff; object>>=8;
	objectp_ram[0x16] = object&0xff; object>>=8;
	objectp_ram[0x15] = object&0xff; object>>=8;
	objectp_ram[0x14] = object&0xff; 
}

