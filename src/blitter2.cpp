//#include "stdafx.h"
#include "jaguar.h"
#include "blitter.h"
//#include "mem.h"
//#include "regs.h"

//extern BYTE* MEM;

// Stuff from the header file...

typedef struct {
	int srcen;
	int srcenz;
	int srcenx;
	int dsten;
	int dstenz;
	int dstwrz;
	int clip_a1;
	int upda1f;
	int upda1;
	int upda2;
	int dsta2;
	int gourd;
	int zbuff;
	int topben;
	int topnen;
	int patdsel;
	int adddsel;
	int zmode;
	int logic;
	int cmpdst;
	int bcompen;
	int dcompen;
	int bkgwren;
	int srcshade;
} BLITCMD;

typedef struct {
	DWORD base;
	DWORD pitch;
	DWORD width;
	DWORD depth;
	DWORD z_offset;
	signed int xadd;
	signed int yadd;
	WORD clip_x;
	WORD clip_y;
	unsigned int pixel_x;
	unsigned int pixel_y;
	signed int step_x;
	signed int step_y;
	unsigned int fstep_x;
	unsigned int fstep_y;
	signed int inc_x;
	signed int inc_y;
	DWORD pixelmode;
	DWORD xsign;
	DWORD ysign;
} BLIT_A1;

typedef struct {
	DWORD base;
	DWORD pitch;
	DWORD width;
	DWORD depth;
	DWORD z_offset;
	DWORD mask_enable;
	signed int xadd;
	signed int yadd;
	DWORD mask;
	unsigned int pixel_x;
	unsigned int pixel_y;
	signed int step_x;
	signed int step_y;
	DWORD pixelmode;
	DWORD xsign;
	DWORD ysign;
} BLIT_A2;

const DWORD bitdepth[8] = { 1,2,4,8,16,32,0,0 };
const DWORD bitsize[8] = { 1,1,1,1,2,4,0,0 };
const DWORD pitch[4] = { 8, 16, 32, 24 };
const DWORD pixels_phrase[8] = {64,32,16,8,4,2,1,0};

const DWORD width_table[64] = {   1,   1,   1,   1,   2,   2,   3,   3,
                                  4,   4,   6,   6,   8,  10,  12,  14,
							     16,  20,  24,  28,  32,  40,  48,  56,
								 64,  80,  96, 112, 128, 160, 192, 224,
							    256, 320, 384, 448, 512, 640, 768, 896,
							   1024,1280,1536,1792,2048,2560,3072,3584,
							   4096,5120,6144,7168,8192,10240,12288,14336,
							   16384,20480,24576,28672,32768,40960,49152,57344 };

//

// More stuff from REG.H

// Blitter Registers

#define A1_BASE			jaguar_long_read(0xF02200)
#define A1_FLAGS		jaguar_long_read(0xF02204)
#define A1_CLIP			jaguar_long_read(0xF02208)
#define A1_CLIP_X		jaguar_word_read(0xF0220A)
#define A1_CLIP_Y		jaguar_word_read(0xF02208)
#define A1_PIXEL		jaguar_long_read(0xF0220C)
#define A1_PIXEL_Y		jaguar_word_read(0xF0220C)
#define A1_PIXEL_X		jaguar_word_read(0xF0220E)
#define A1_STEP			jaguar_long_read(0xF02210)
#define A1_STEP_Y		jaguar_word_read(0xF02210)
#define A1_STEP_X		jaguar_word_read(0xF02212)
#define A1_FSTEP		jaguar_long_read(0xF02214)
#define A1_FSTEP_Y		jaguar_word_read(0xF02214)
#define A1_FSTEP_X		jaguar_word_read(0xF02216)
#define A1_FPIXEL		jaguar_long_read(0xF02218)
#define A1_FPIXEL_Y		jaguar_word_read(0xF02218)
#define A1_FPIXEL_X		jaguar_word_read(0xF0221A)
#define A1_INC			jaguar_long_read(0xF0221C)
#define A1_INC_Y		jaguar_word_read(0xF0221C)
#define A1_INC_X		jaguar_word_read(0xF0221E)
#define A1_FINC			jaguar_long_read(0xF02220)
#define A1_FINC_Y		jaguar_word_read(0xF02220)
#define A1_FINC_X		jaguar_word_read(0xF02222)
#define A2_BASE			jaguar_long_read(0xF02224)
#define A2_FLAGS		jaguar_long_read(0xF02228)
#define A2_MASK			jaguar_long_read(0xF0222C)
#define A2_MASK_Y		jaguar_word_read(0xF0222C)
#define A2_MASK_X		jaguar_word_read(0xF0222E)
#define A2_PIXEL		jaguar_long_read(0xF02230)
#define A2_PIXEL_Y		jaguar_word_read(0xF02230)
#define A2_PIXEL_X		jaguar_word_read(0xF02232)
#define A2_STEP			jaguar_long_read(0xF02234)
#define A2_STEP_Y		jaguar_word_read(0xF02234)
#define A2_STEP_X		jaguar_word_read(0xF02236)
#define B_CMD			jaguar_long_read(0xF02238)
#define B_COUNT			jaguar_long_read(0xF0223C)
#define B_COUNT_OUT		jaguar_word_read(0xF0223C)
#define B_COUNT_IN		jaguar_word_read(0xF0223E)
#define B_SRCD			*(LONGLONG*)(MEM+0xF02240)
#define B_SRCD0			jaguar_long_read(0xF02240)
#define B_SRCD1			jaguar_long_read(0xF02244)
#define B_DSTD			*(LONGLONG*)(MEM+0xF02248)
#define B_DSTD0			jaguar_long_read(0xF02248)
#define B_DSTD1			jaguar_long_read(0xF0224C)
#define B_DSTZ			*(LONGLONG*)(MEM+0xF02250)
#define B_DSTZ0			jaguar_long_read(0xF02250)
#define B_DSTZ1			jaguar_long_read(0xF02254)
#define B_SRCZ			*(LONGLONG*)(MEM+0xF02258)   // Source Z Integer
#define B_SRCZ0			jaguar_long_read(0xF02258)
#define B_SRCZ1			jaguar_long_read(0xF0225C)
#define B_SRCZF			*(LONGLONG*)(MEM+0xF02260)   // Source Z Fraction
#define B_SRCZF0		jaguar_long_read(0xF02260)
#define B_SRCZF1		jaguar_long_read(0xF02264)
#define B_PATD			*(LONGLONG*)(MEM+0xF02268)
#define B_PATD0			jaguar_long_read(0xF02268)
#define B_PATD1			jaguar_long_read(0xF0226C)
#define B_IINC			jaguar_long_read(0xF02270)
#define B_IINC_I		jaguar_word_read(0xF02270)
#define B_IINC_F		jaguar_word_read(0xF02272)
#define B_ZINC			jaguar_long_read(0xF02274)
#define B_ZINC_I		jaguar_word_read(0xF02274)
#define B_ZINC_F		jaguar_word_read(0xF02276)
#define B_STOP			jaguar_long_read(0xF02278)
#define B_I3			jaguar_long_read(0xF0227C)
#define B_I3_I			jaguar_word_read(0xF0227C)
#define B_I3_F			jaguar_word_read(0xF0227E)
#define B_I2			jaguar_long_read(0xF02280)
#define B_I2_I			jaguar_word_read(0xF02280)
#define B_I2_F			jaguar_word_read(0xF02282)
#define B_I1			jaguar_long_read(0xF02284)
#define B_I1_I			jaguar_word_read(0xF02284)
#define B_I1_F			jaguar_word_read(0xF02286)
#define B_I0			jaguar_long_read(0xF02288)
#define B_I0_I			jaguar_word_read(0xF02288)
#define B_I0_F			jaguar_word_read(0xF0228A)
#define B_Z3			jaguar_long_read(0xF0228C)
#define B_Z3_I			jaguar_word_read(0xF0228C)
#define B_Z3_F			jaguar_word_read(0xF0228E)
#define B_Z2			jaguar_long_read(0xF02290)
#define B_Z2_I			jaguar_word_read(0xF02290)
#define B_Z2_F			jaguar_word_read(0xF02292)
#define B_Z1			jaguar_long_read(0xF02294)
#define B_Z1_I			jaguar_word_read(0xF02294)
#define B_Z1_F			jaguar_word_read(0xF02296)
#define B_Z0			jaguar_long_read(0xF02298)
#define B_Z0_I			jaguar_word_read(0xF02298)
#define B_Z0_F			jaguar_word_read(0xF0229A)

//#define DWORDBIG(x) ((x>>16)&0xFFFF)|(x<<16)
#define DWORDBIG(x)		x

//

//const pixels_per_phrase[8] = { 64, 32, 16, 8, 4, 2, 1, 0 };

void blitter2_exec(DWORD cmd)
{
	DWORD* src,* dst,* read;
	DWORD a1_address, a2_address;
	DWORD a1_bm,a2_bm;
	DWORD* src_bm,* dst_bm;
	BLITCMD bcmd;
	BLIT_A1 a1;
	BLIT_A2 a2;
	memset(&a1,0,sizeof(BLIT_A1));
	memset(&a2,0,sizeof(BLIT_A2));
	memset(&bcmd,0,sizeof(BLITCMD));
	DWORD srcd = 0xF02240;
	DWORD dstd = 0xF02248;
	DWORD patd = 0xF02268;

	bcmd.srcen = (cmd & 0x1) ? 1 : 0;
	bcmd.srcenz = (cmd & 0x2) ? 1 : 0;
	bcmd.srcenx = (cmd & 0x4) ? 1 : 0;
	bcmd.dsten = (cmd & 0x8) ? 1 : 0;
	bcmd.dstenz = (cmd & 0x10) ? 1 : 0;
	bcmd.dstwrz = (cmd & 0x20) ? 1 : 0;
	bcmd.clip_a1 = (cmd & 0x40) ? 1 : 0;
	bcmd.upda1f = (cmd & 0x100) ? 1 : 0;
	bcmd.upda1 = (cmd & 0x200) ? 1 : 0;
	bcmd.upda2 = (cmd & 0x400) ? 1 : 0;
	bcmd.dsta2 = (cmd & 0x800) ? 1 : 0;
	bcmd.gourd = (cmd & 0x1000) ? 1 : 0;
	bcmd.patdsel = (cmd & 0x10000) ? 1 : 0;
	bcmd.cmpdst = (cmd & 0x2000000) ? 1 : 0;
	bcmd.bcompen = (cmd & 0x4000000) ? 1 : 0;
	bcmd.dcompen = (cmd & 0x8000000) ? 1 : 0;
	bcmd.bkgwren = (cmd & 0x10000000) ? 1 : 0;
	bcmd.srcshade = (cmd & 0x40000000) ? 1 : 0;
	bcmd.logic = (cmd >> 21) & 0xF;
	bcmd.zmode = (cmd >> 18) & 0x7;

	DWORD outer_loop = B_COUNT_OUT;
	DWORD inner_loop = B_COUNT_IN;

	a1.base = DWORDBIG(A1_BASE);
	DWORD flags = DWORDBIG(A1_FLAGS);
	a1.pitch = flags & 0x3;
	a1.depth = (flags >> 3) & 0x7;
	a1.z_offset = ((flags >> 6) & 0x7)*8;
	a1.xadd = (short)((flags >> 16) & 0x3);
	a1.pixel_x = A1_PIXEL_X << 16 | A1_FPIXEL_X;
	a1.pixel_y = A1_PIXEL_Y << 16 | A1_FPIXEL_Y;
	a1.step_x = (signed short)A1_STEP_X << 16;
	a1.step_y = (signed short)A1_STEP_Y << 16;
	a1.fstep_x = A1_FSTEP_X;
	a1.fstep_y = A1_FSTEP_Y;
	a1.inc_x = A1_INC_X << 16 | A1_FINC_X;
	a1.inc_y = A1_INC_Y << 16 | A1_FINC_Y;
	a1.clip_x = A1_CLIP_X & 0x7FFF;
	a1.clip_y = A1_CLIP_Y & 0x7FFF;
	a1.width = width_table[(flags >> 9) & 0x3F];

	a1.yadd = (short)((flags >> 18) & 0x1);
	a1.xsign = (flags >> 19) & 0x1;
	a1.ysign = (flags >> 20) & 0x1;
	if(a1.ysign)
		a1.yadd = -a1.yadd;
	a1.pixelmode = (flags >> 16) & 0x3;

	a2.base = DWORDBIG(A2_BASE);
	flags = DWORDBIG(A2_FLAGS);
	a2.pitch = flags & 0x3;
	a2.z_offset = ((flags >> 6) & 0x7)*8;
	a2.pixel_x = A2_PIXEL_X;
	a2.pixel_y = A2_PIXEL_Y;
	a2.step_x = (signed short)A2_STEP_X;
	a2.step_y = (signed short)A2_STEP_Y;
	a2.mask = DWORDBIG(A2_MASK);
	a2.width = width_table[(flags >> 9) & 0x3F];
//	a2.yadd = (flags >> 18) & 0x1;
	a2.yadd = a1.yadd; // Buggy blitter !!? YES! It is!!!
	a2.pixelmode = (flags >> 16) & 0x3;
	a2.xsign = (flags >> 19) & 0x1;
	a2.ysign = (flags >> 20) & 0x1;
	if(a2.ysign)
		a2.yadd = -a2.yadd;

	if(bcmd.dsta2 == 0) {
		dst = &a1_address;
		src = &a2_address;
		read = &a1_address;
		dst_bm = &a1_bm;
		src_bm = &a2_bm;
	} else {
		src = &a1_address;
		dst = &a2_address;
		read = &a2_address;
		dst_bm = &a2_bm;
		src_bm = &a1_bm;
	}
	if(!bcmd.srcen) {
		src = &srcd;
	}
	if(bcmd.patdsel) {
		src = &patd;
	}
	if(!bcmd.dsten) {
		read = &dstd;
	}

	int src_zoffset,dst_zoffset;
	if(bcmd.dsta2) {
		src_zoffset = a1.z_offset;
		dst_zoffset = a2.z_offset;
	} else {
		src_zoffset = a2.z_offset;
		dst_zoffset = a1.z_offset;
	}

	unsigned int a1_x = a1.pixel_x;
	unsigned int a1_y = a1.pixel_y;
	unsigned int a2_x = a2.pixel_x;
	unsigned int a2_y = a2.pixel_y;
	int a1_size = pitch[a1.pitch];
	int a2_size = pitch[a2.pitch];
	//inner_loop = (inner_loop * bitdepth[a1.depth]) / 8;

	int a1_inc_x,a1_inc_y;
	short a2_inc_x;
	switch(a1.pixelmode) {
		case 0:a1_inc_x = 1<<16; a1_inc_y = a1.yadd<<16; break;
		case 1:a1_inc_x = 1<<16; a1_inc_y = a1.yadd<<16; break;
		case 2:a1_inc_x = 0; a1_inc_y = a1.yadd<<16; break;
		case 3:a1_inc_x = a1.inc_x; a1_inc_y = a1.inc_y; break;
	}
	if(a1.xsign)
		a1_inc_x = -a1_inc_x;
	switch(a2.pixelmode) {
		case 0:a2_inc_x = 1;break;
		case 1:a2_inc_x = 1;break;
		case 2:a2_inc_x = 0;break;
		case 3:a2_inc_x = 1;break;
	}
	if(a2.xsign)
		a2_inc_x = -a2_inc_x;

	int gd_i[4];
	int gd_c[4];
	int gd_ia,gd_ca;
	DWORD gouraud_add,gouraud_data;
	WORD gint[4],gfrac[4];
	BYTE gcolour[4];
	gouraud_add = jaguar_long_read(0xF02270);
	gcolour[3] = jaguar_byte_read(0xF02268);
	gcolour[2] = jaguar_byte_read(0xF0226A);
	gcolour[1] = jaguar_byte_read(0xF0226C);
	gcolour[0] = jaguar_byte_read(0xF0226E);
	gint[3] = jaguar_word_read(0xF0227C);
	gint[2] = jaguar_word_read(0xF02280);
	gint[1] = jaguar_word_read(0xF02284);
	gint[0] = jaguar_word_read(0xF02288);
	gfrac[3] = jaguar_word_read(0xF0227E);
	gfrac[2] = jaguar_word_read(0xF02282);
	gfrac[1] = jaguar_word_read(0xF02286);
	gfrac[0] = jaguar_word_read(0xF0228A);
	gd_ia = gouraud_add & 0xFFFFFF;
	if(gd_ia & 0x800000)
		gd_ia = 0xFF000000 | gd_ia;
	gd_ca = (gouraud_add>>24) & 0xFF;
	if(gd_ca & 0x80)
		gd_ca = 0xFFFFFF00 | gd_ca;
	for(int v=0;v<4;v++) {
		gd_i[v] = gint[v] & 0xFF;
		//if(gd_i[v] & 0x80)
		//	gd_i[v] = 0xFF00 | gd_i[v];
		gd_i[v] = (gd_i[v] << 16) | gfrac[v];
		gd_c[v] = gcolour[v]<<4;
	}
	int colour_index = 0;
	int pi=0;
	switch(a1.depth) {
		case 0:case 1:case 2:case 3:
			for(DWORD j=0; j<outer_loop; j++) {
				int old_a1x = a1_x;
				int old_a2x = a2_x;
				for(DWORD i=0; i<(inner_loop*bitdepth[a1.depth]); i+=bitdepth[a1.depth]) {
					BYTE src1,src2=0;
					BYTE dst1,dst2;
					BYTE dst_old;
					a1_address = a1.base + ((((a1_y>>16) * a1.width + (a1_x>>16))*bitdepth[a1.depth])/8);
					a2_address = a2.base + (((a2_y * a2.width + a2_x)*bitdepth[a1.depth])/8);
					src1 = jaguar_byte_read(*src);
					dst1 = jaguar_byte_read(*dst);
					dst_old = dst1;
					//jaguar_long_read(*src+4,&src2);
					if(!bcmd.patdsel) {
						//jaguar_long_read(*dst+4,&dst2);
						switch(bcmd.logic)
						{
						case 0:dst1 = 0;dst2 = 0;break;				
						case 1:dst1 = !src1 & !dst1; dst2 = !src2 & !dst2;break;
						case 2:dst1 = !src1 &  dst1; dst2 = !src2 & dst2;break;				
						case 3:dst1 = !src1; dst2 = !src2;break;
						case 4:dst1 = src1 & !dst1; dst2 = src2 & !dst2;break;
						case 5:dst1 = !dst1; dst2 = !dst2;break;
						case 6:dst1 = !(src1 ^ dst1); dst2 = !(src2 ^ dst2);break;
						case 7:dst1 = !src1 | !dst1; dst2 = !src2 | !dst2;break;
						case 8:dst1 = src1 & dst1; dst2 = src2 & dst2;break;
						case 9:dst1 = src1 ^ dst1; dst2 = src2 ^ dst2;break;
						case 10:dst1 = dst1; dst2 = dst2;break;
						case 11:dst1 = !src1 | dst1; dst2 = !src2 | dst2;break;
						case 12:dst1 = src1; dst2 = src2;break;
						case 13:dst1 = src1 | !dst1; dst2 = src2 | !dst2;break;
						case 14:dst1 = src1 | dst1; dst2 = src2 | dst2;break;
						case 15:dst1 = 0xFF; dst2 = 0xFF;break;
						}
					} else {
						dst1 = src1;
						//dst2 = src2;
					}
					if(bcmd.dcompen) {
						BYTE pattern;
						pattern = jaguar_byte_read(0xF02268);
						if(!bcmd.cmpdst) {
							if(src1 == !dst_old)
							dst1 = dst_old;
						} else {					
							if(dst1 == !dst_old)
							dst1 = dst_old;
						}
					}
					if(bcmd.clip_a1) {
						if((a1_x>>16) < a1.clip_x && (a1_x>>16) >= 0 && (a1_y>>16) < a1.clip_y && (a1_y>>16) >= 0) {
							jaguar_byte_write(*dst,dst1);
							//jaguar_long_write(*dst+4,dst2);
						}
					} else {
						jaguar_byte_write(*dst,dst1);
						//jaguar_long_write(*dst+4,dst2);
					}
					a1_x += a1_inc_x;
					a2_x += a2_inc_x;
					a1_y += a1_inc_y;
					a2_y += a2.yadd;
					for(int v=0;v<4;v++) {
						gd_i[v] += gd_ia;
						gd_c[v] += gd_ca;
					}
				}
				if(bcmd.upda1) {
					a1_x += a1.step_x;
					a1_y += a1.step_y;
				}
				if(bcmd.upda1f) {
					a1_x += a1.fstep_x;
					a1_y += a1.fstep_y;
				}
				if(bcmd.upda2) {
					a2_x += a2.step_x;
					a2_y += a2.step_y;
				}
				//a1_x = old_a1x;
				//a2_x = old_a2x;
			}
			break;
		case 4:
			{
			int zinc = jaguar_long_read(0xF02274);
			INT32 compz[4];
			compz[0] = jaguar_long_read(0xF0228C);
			compz[1] = jaguar_long_read(0xF02290);
			compz[2] = jaguar_long_read(0xF02294);
			compz[3] = jaguar_long_read(0xF02298);
			for(DWORD j=0; j<outer_loop; j++) {
				for(DWORD i=0; i<inner_loop*2; i+=2) {
					WORD src1,src2;
					WORD dst1,dst2;
					WORD dst_old;
					WORD srcz,dstz;
					a1_address = a1.base + (((a1_y>>16) * a1.width + ((a1_x>>16)&0xFFFFFFFC))*2*(a1_size/8)) + (((a1_x>>16)&0x3)*2);
					a2_address = a2.base + ((a2_y * a2.width + (a2_x&0xFFFFFFFC))*2*(a2_size/8)) + ((a2_x&0x3)*2);
					src1 = jaguar_word_read(*src);
					dst1 = jaguar_word_read(*dst);
					dst_old = dst1;
					if(bcmd.srcenz)
						srcz = jaguar_word_read(*src+src_zoffset);
					else
						srcz = compz[pi & 0x3] >> 16;
					if(bcmd.dstenz)
						dstz = jaguar_word_read(*dst+dst_zoffset);

					//jaguar_long_read(*src+4,&src2);
					if(bcmd.patdsel) {
//						dst1 = *(WORD*)(&MEM[0xF02268]);
						dst1 = jaguar_word_read(0xF02268);
					} else {
						//jaguar_long_read(*dst+4,&dst2);
						switch(bcmd.logic)
						{
						case 0:dst1 = 0;dst2 = 0;break;				
						case 1:dst1 = !src1 & !dst1; dst2 = !src2 & !dst2;break;
						case 2:dst1 = !src1 &  dst1; dst2 = !src2 & dst2;break;				
						case 3:dst1 = !src1; dst2 = !src2;break;
						case 4:dst1 = src1 & !dst1; dst2 = src2 & !dst2;break;
						case 5:dst1 = !dst1; dst2 = !dst2;break;
						case 6:dst1 = (src1 ^ dst1); dst2 = !(src2 ^ dst2);break;
						case 7:dst1 = !src1 | !dst1; dst2 = !src2 | !dst2;break;
						case 8:dst1 = src1 & dst1; dst2 = src2 & dst2;break;
						case 9:dst1 = src1 ^ dst1; dst2 = src2 ^ dst2;break;
						case 10:dst1 = dst1; dst2 = dst2;break;
						case 11:dst1 = !src1 | dst1; dst2 = !src2 | dst2;break;
						case 12:dst1 = src1; dst2 = src2;break;
						case 13:dst1 = src1 | !dst1; dst2 = src2 | !dst2;break;
						case 14:dst1 = src1 | dst1; dst2 = src2 | dst2;break;
						case 15:dst1 = 0xFFFFFFFF; dst2 = 0xFFFFFFFF;break;
						}
					} 
					if(bcmd.gourd) {
						DWORD gdt = (gd_i[pi & 0x3] & 0xFFFFFF) | ((gd_c[pi & 0x3]>>4) << 24);
						dst1 = gdt >> 16;
					}
					if(bcmd.srcshade) {
						int intensity = src1 & 0xFF;
						int ia = gd_ia >> 16;
						if(ia & 0x80)
							ia = 0xFFFFFF00 | ia;
						intensity += ia;
						if(intensity < 0)
							intensity = 0;
						if(intensity > 0xFF)
							intensity = 0xFF;
						dst1 = (src1 & 0xFF00) | intensity;
					}
					if(bcmd.dcompen) {
						WORD pattern;
						pattern = jaguar_word_read(0xF02268);
						if(!bcmd.cmpdst) {
							if(src1 == pattern)//dst_old)
								dst1 = dst_old;
						} else {
							if(dst1 == pattern)
								dst1 = dst_old;
						}
					}
					if(bcmd.zmode > 0) {
						bool zc = false;
						switch(bcmd.zmode) {
							case 1: zc = (srcz < dstz);break;
							case 2: zc = (srcz == dstz);break;
							case 3: zc = (srcz <= dstz);break;
							case 4: zc = (srcz > dstz);break;
							case 5: zc = (srcz != dstz);break;
							case 6: zc = (srcz >= dstz);break;
						}
						if(zc == false) {
							if(bcmd.clip_a1) {
								if((a1_x>>16) < a1.clip_x && (a1_x>>16) >= 0 && (a1_y>>16) < a1.clip_y && (a1_y>>16) >= 0) {
									jaguar_word_write(*dst,dst1);
									//jaguar_long_write(*dst+4,dst2);
								}
							} else {
								jaguar_word_write(*dst,dst1);
								//jaguar_long_write(*dst+4,dst2);
							}
						} else {
							srcz = dstz;
						}
					} else {
						if(bcmd.clip_a1) {
							if((a1_x>>16) < a1.clip_x && (a1_x>>16) >= 0 && (a1_y>>16) < a1.clip_y && (a1_y>>16) >= 0) {
								jaguar_word_write(*dst,dst1);
								//jaguar_long_write(*dst+4,dst2);
							}
						} else {
							jaguar_word_write(*dst,dst1);
							//jaguar_long_write(*dst+4,dst2);
						}
					}
					if(bcmd.dstwrz) {
						jaguar_word_write(*dst+dst_zoffset,srcz & 0xFFFF);
					}
					a1_x += a1_inc_x;
					a2_x += a2_inc_x;
					a1_y += a1_inc_y;
					a2_y += a2.yadd;
					gd_i[pi & 0x3] += gd_ia;
					gd_c[pi & 0x3] += gd_ca;
					if(gd_i[pi & 0x3] > 0xFFFFFF)
						gd_i[pi & 0x3] = 0xFFFFFF;
					if(gd_i[pi & 0x3] < 0)
						gd_i[pi & 0x3] = 0;
					compz[pi & 0x3] += zinc;
					if(compz[pi & 0x3] > 0xFFFFFFFF)
						compz[pi & 0x3] = 0xFFFFFFFF;
					if(compz[pi & 0x3] < 0)
						compz[pi & 0x3] = 0;
					if(a1.pixelmode == 0) {
						pi++;
					}
				}
				if(bcmd.upda1) {
					a1_x += a1.step_x;
					a1_y += a1.step_y;
				}
				if(bcmd.upda1f) {
					a1_x += a1.fstep_x;
					a1_y += a1.fstep_y;
				}
				if(bcmd.upda2) {
					a2_x += a2.step_x;
					a2_y += a2.step_y;
				}
			}
			break;
			}
		case 5:
			for(DWORD j=0; j<outer_loop; j++) {
				for(DWORD i=0; i<(inner_loop*bitdepth[a1.depth])/8; i+=bitsize[a1.depth]) {
					DWORD src1,src2=0;
					DWORD dst1,dst2;
					a1_address = a1.base + ((((a1_y>>16) * a1.width + (a1_x>>16))*bitdepth[a1.depth])/8);
					a2_address = a2.base + (((a2_y * a2.width + a2_x)*bitdepth[a1.depth])/8);
					src1 = jaguar_long_read(*src);
					//jaguar_long_read(*src+4,&src2);
					if(!bcmd.patdsel) {
						dst1 = jaguar_long_read(*read);
						//jaguar_long_read(*dst+4,&dst2);
						switch(bcmd.logic)
						{
						case 0:dst1 = 0;dst2 = 0;break;				
						case 1:dst1 = !src1 & !dst1; dst2 = !src2 & !dst2;break;
						case 2:dst1 = !src1 &  dst1; dst2 = !src2 & dst2;break;				
						case 3:dst1 = !src1; dst2 = !src2;break;
						case 4:dst1 = src1 & !dst1; dst2 = src2 & !dst2;break;
						case 5:dst1 = !dst1; dst2 = !dst2;break;
						case 6:dst1 = !(src1 ^ dst1); dst2 = !(src2 ^ dst2);break;
						case 7:dst1 = !src1 | !dst1; dst2 = !src2 | !dst2;break;
						case 8:dst1 = src1 & dst1; dst2 = src2 & dst2;break;
						case 9:dst1 = src1 ^ dst1; dst2 = src2 ^ dst2;break;
						case 10:dst1 = dst1; dst2 = dst2;break;
						case 11:dst1 = !src1 | dst1; dst2 = !src2 | dst2;break;
						case 12:dst1 = src1; dst2 = src2;break;
						case 13:dst1 = src1 | !dst1; dst2 = src2 | !dst2;break;
						case 14:dst1 = src1 | dst1; dst2 = src2 | dst2;break;
						case 15:dst1 = 0xFFFFFFFF; dst2 = 0xFFFFFFFF;break;
						}
					} else {
						dst1 = src1;
						dst2 = src2;
					}
					if(bcmd.clip_a1) {
						if((a1_x>>16) < a1.clip_x && (a1_x>>16) >= 0 && (a1_y>>16) < a1.clip_y && (a1_y>>16) >= 0) {
							jaguar_long_write(*dst,dst1);
							//jaguar_long_write(*dst+4,dst2);
						}
					} else {
						jaguar_long_write(*dst,dst1);
						//jaguar_long_write(*dst+4,dst2);
					}
					a1_x += a1_inc_x;
					a2_x += a2_inc_x;
					a1_y += a1_inc_y;
					a2_y += a2.yadd;
					for(int v=0;v<4;v++) {
						gd_i[v] += gd_ia;
						gd_c[v] += gd_ca;
					}
				}
				if(bcmd.upda1) {
					a1_x += a1.step_x;
					a1_y += a1.step_y;
				}
				if(bcmd.upda1f) {
					a1_x += a1.fstep_x;
					a1_y += a1.fstep_y;
				}
				if(bcmd.upda2) {
					a2_x += a2.step_x;
					a2_y += a2.step_y;
				}
			}
			break;
		default:
			break;
	}
/*	A1_PIXEL_X = a1_x >> 16;
	A1_PIXEL_Y = a1_y >> 16;
	A1_FPIXEL_X = a1_x & 0xFFFF;
	A1_FPIXEL_Y = a1_y & 0xFFFF;
	A2_PIXEL_X = a2_x;
	A2_PIXEL_Y = a2_y;*/
	jaguar_word_write(0xF0220E, a1_x >> 16);	// A1_PIXEL_X = 
	jaguar_word_write(0xF0220C, a1_y >> 16);	// A1_PIXEL_Y = 
	jaguar_word_write(0xF0221A, a1_x & 0xFFFF);	// A1_FPIXEL_X = 
	jaguar_word_write(0xF02218, a1_y & 0xFFFF);	// A1_FPIXEL_Y = 
	jaguar_word_write(0xF02232, a2_x);			// A2_PIXEL_X = 
	jaguar_word_write(0xF02230, a2_y);			// A2_PIXEL_Y = 
}
/*
#define A1_PIXEL_Y		jaguar_word_read(0xF0220C)
#define A1_PIXEL_X		jaguar_word_read(0xF0220E)
#define A1_FPIXEL_Y		jaguar_word_read(0xF02218)
#define A1_FPIXEL_X		jaguar_word_read(0xF0221A)
#define A2_PIXEL_Y		jaguar_word_read(0xF02230)
#define A2_PIXEL_X		jaguar_word_read(0xF02232)
*/
