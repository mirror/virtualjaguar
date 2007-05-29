#define GEN_CODE
//#define LOG_BLITS
//#define USE_GENERIC_BLITTER
 
#include "jaguar.h"

#define null 0
extern int jaguar_active_memory_dumps;

#define REG(A)		blitter_reg_read(A)
#define WREG(A,D)	blitter_reg_write(A,D)

int start_logging = 0;

static uint8 blitter_ram[0x100];

#define A1_BASE			((UINT32)0x00)
#define A1_FLAGS		((UINT32)0x04)
#define A1_CLIP			((UINT32)0x08)	// height and width values for clipping
#define A1_PIXEL		((UINT32)0x0c)	// integer part of the pixel (Y.i and X.i)
#define A1_STEP			((UINT32)0x10)	// integer part of the step
#define A1_FSTEP		((UINT32)0x14)	// fractionnal part of the step
#define A1_FPIXEL		((UINT32)0x18)	// fractionnal part of the pixel (Y.f and X.f)
#define A1_INC			((UINT32)0x1C)	// integer part of the increment
#define A1_FINC			((UINT32)0x20)	// fractionnal part of the increment
#define A2_BASE			((UINT32)0x24)
#define A2_FLAGS		((UINT32)0x28)
#define A2_MASK			((UINT32)0x2c)	// modulo values for x and y (M.y  and M.x)
#define A2_PIXEL		((UINT32)0x30)	// integer part of the pixel (no fractionnal part for A2)
#define A2_STEP			((UINT32)0x34)	// integer part of the step (no fractionnal part for A2)
#define COMMAND			((UINT32)0x38)
#define PIXLINECOUNTER	((UINT32)0x3C)
#define SRCDATA			((UINT32)0x40)
#define DSTDATA			((UINT32)0x48)
#define DSTZ			((UINT32)0x50)
#define SRCZINT			((UINT32)0x58)
#define SRCZFRAC		((UINT32)0x60)
#define PATTERNDATA		((UINT32)0x68)
#define INTENSITYINC	((UINT32)0x70)
#define ZINC			((UINT32)0x74)
#define COLLISIONCTRL	((UINT32)0x78)
#define PHRASEINT3		((UINT32)0x7C)
#define PHRASEINT2		((UINT32)0x80)
#define PHRASEINT1		((UINT32)0x84)
#define PHRASEINT0		((UINT32)0x88)
#define PHRASEZ3		((UINT32)0x8C)
#define PHRASEZ2		((UINT32)0x90)
#define PHRASEZ1		((UINT32)0x94)
#define PHRASEZ0		((UINT32)0x98)

#define SRCEN			(cmd&0x00000001)
#define SRCENZ			(cmd&0x00000002)
#define SRCENX			(cmd&0x00000004)
#define DSTEN			(cmd&0x00000008)
#define DSTENZ			(cmd&0x00000010)
#define DSTWRZ			(cmd&0x00000020)
#define CLIPA1			(cmd&0x00000040)
#define DSTA2			(cmd&0x00000800)

#define Z_OP_INF		(cmd&0x00040000)
#define Z_OP_EQU		(cmd&0x00080000)
#define Z_OP_SUP		(cmd&0x00100000)

#define CMPDST			(cmd&0x02000000)
#define BCOMPEN			(cmd&0x04000000)
#define DCOMPEN			(cmd&0x08000000)

#define LFU_NAN			(cmd&0x00200000)
#define LFU_NA			(cmd&0x00400000)
#define LFU_AN			(cmd&0x00800000)
#define LFU_A			(cmd&0x01000000)

#define PATDSEL			(cmd&0x00010000)
#define INTADD			(cmd&0x00020000)
#define TOPBEN			(cmd&0x00004000)
#define TOPNEN			(cmd&0x00008000)
#define BKGWREN			(cmd&0x10000000)
#define GOURD			(cmd&0x00001000)
#define GOURZ			(cmd&0x00002000)
#define SRCSHADE		(cmd&0x40000000)


#define XADDPHR	 0
#define XADDPIX	 1
#define XADD0	 2
#define XADDINC	 3

#define XSIGNSUB_A1		(REG(A1_FLAGS)&0x80000)
#define XSIGNSUB_A2		(REG(A2_FLAGS)&0x80000)

#define YSIGNSUB_A1		(REG(A1_FLAGS)&0x100000)
#define YSIGNSUB_A2		(REG(A2_FLAGS)&0x100000)

#define YADD1_A1		(REG(A1_FLAGS)&0x40000)
#define YADD1_A2		(REG(A2_FLAGS)&0x40000)

// 1 bpp pixel read
#define PIXEL_SHIFT_1(a)      (((~a##_x) >> 16) & 7)
#define PIXEL_OFFSET_1(a)     (((((UINT32)a##_y >> 16) * a##_width / 8) + (((UINT32)a##_x >> 19) & ~7)) * (1 + a##_pitch) + (((UINT32)a##_x >> 19) & 7))
#define READ_PIXEL_1(a)       ((jaguar_byte_read(a##_addr+PIXEL_OFFSET_1(a)) >> PIXEL_SHIFT_1(a)) & 0x01)

// 2 bpp pixel read
#define PIXEL_SHIFT_2(a)      (((~a##_x) >> 15) & 6)
#define PIXEL_OFFSET_2(a)     (((((UINT32)a##_y >> 16) * a##_width / 4) + (((UINT32)a##_x >> 18) & ~7)) * (1 + a##_pitch) + (((UINT32)a##_x >> 18) & 7))
#define READ_PIXEL_2(a)       ((jaguar_byte_read(a##_addr+PIXEL_OFFSET_2(a)) >> PIXEL_SHIFT_2(a)) & 0x03)

// 4 bpp pixel read
#define PIXEL_SHIFT_4(a)      (((~a##_x) >> 14) & 4)
#define PIXEL_OFFSET_4(a)     (((((UINT32)a##_y >> 16) * (a##_width/2)) + (((UINT32)a##_x >> 17) & ~7)) * (1 + a##_pitch) + (((UINT32)a##_x >> 17) & 7))
#define READ_PIXEL_4(a)       ((jaguar_byte_read(a##_addr+PIXEL_OFFSET_4(a)) >> PIXEL_SHIFT_4(a)) & 0x0f)

// 8 bpp pixel read
#define PIXEL_OFFSET_8(a)     (((((UINT32)a##_y >> 16) * a##_width) + (((UINT32)a##_x >> 16) & ~7)) * (1 + a##_pitch) + (((UINT32)a##_x >> 16) & 7))
#define READ_PIXEL_8(a)       (jaguar_byte_read(a##_addr+PIXEL_OFFSET_8(a)))

// 16 bpp pixel read
#define PIXEL_OFFSET_16(a)    (((((UINT32)a##_y >> 16) * a##_width) + (((UINT32)a##_x >> 16) & ~3)) * (1 + a##_pitch) + (((UINT32)a##_x >> 16) & 3))
#define READ_PIXEL_16(a)       (jaguar_word_read(a##_addr+(PIXEL_OFFSET_16(a)<<1)))

// 32 bpp pixel read
#define PIXEL_OFFSET_32(a)    (((((UINT32)a##_y >> 16) * a##_width) + (((UINT32)a##_x >> 16) & ~1)) * (1 + a##_pitch) + (((UINT32)a##_x >> 16) & 1))
#define READ_PIXEL_32(a)      (jaguar_long_read(a##_addr+(PIXEL_OFFSET_32(a)<<2)))

// pixel read
#define READ_PIXEL(a,f) (\
	 (((f>>3)&0x07) == 0) ? (READ_PIXEL_1(a)) : \
	 (((f>>3)&0x07) == 1) ? (READ_PIXEL_2(a)) : \
	 (((f>>3)&0x07) == 2) ? (READ_PIXEL_4(a)) : \
	 (((f>>3)&0x07) == 3) ? (READ_PIXEL_8(a)) : \
	 (((f>>3)&0x07) == 4) ? (READ_PIXEL_16(a)) : \
	 (((f>>3)&0x07) == 5) ? (READ_PIXEL_32(a)) : 0)

// 16 bpp z data read
#define ZDATA_OFFSET_16(a)     (PIXEL_OFFSET_16(a) + a##_zoffs * 4)
#define READ_ZDATA_16(a)       (jaguar_word_read(a##_addr+(ZDATA_OFFSET_16(a)<<1)))

// z data read
#define READ_ZDATA(a,f) (READ_ZDATA_16(a))

// 16 bpp z data write
#define WRITE_ZDATA_16(a,d)     {  jaguar_word_write(a##_addr+(ZDATA_OFFSET_16(a)<<1),d); }

// z data write
#define WRITE_ZDATA(a,f,d) WRITE_ZDATA_16(a,d); 

// 1 bpp r data read
#define READ_RDATA_1(r,a,p)  ((p) ?  ((REG(r+(((UINT32)a##_x>>19)&4)))>>(((UINT32)a##_x>>16)&0x1f))&   0x1 : (REG(r) &    0x1))

// 2 bpp r data read
#define READ_RDATA_2(r,a,p)  ((p) ?  ((REG(r+(((UINT32)a##_x>>18)&4)))>>(((UINT32)a##_x>>15)&0x3e))&   0x3 : (REG(r) &    0x3))

// 4 bpp r data read
#define READ_RDATA_4(r,a,p)  ((p) ?  ((REG(r+(((UINT32)a##_x>>17)&4)))>>(((UINT32)a##_x>>14)&0x28))&   0xf : (REG(r) &    0xf))

// 8 bpp r data read
#define READ_RDATA_8(r,a,p)  ((p) ?  ((REG(r+(((UINT32)a##_x>>16)&4)))>>(((UINT32)a##_x>>13)&0x18))&  0xff : (REG(r) &   0xff))

// 16 bpp r data read
#define READ_RDATA_16(r,a,p)  ((p) ? ((REG(r+(((UINT32)a##_x>>15)&4)))>>(((UINT32)a##_x>>12)&0x10))&0xffff : (REG(r) & 0xffff))

// 32 bpp r data read
#define READ_RDATA_32(r,a,p)  ((p) ? REG(r+(((UINT32)a##_x>>14)&4)) : REG(r))


// register data read
#define READ_RDATA(r,a,f,p) (\
	 (((f>>3)&0x07) == 0) ? (READ_RDATA_1(r,a,p)) : \
	 (((f>>3)&0x07) == 1) ? (READ_RDATA_2(r,a,p)) : \
	 (((f>>3)&0x07) == 2) ? (READ_RDATA_4(r,a,p)) : \
	 (((f>>3)&0x07) == 3) ? (READ_RDATA_8(r,a,p)) : \
	 (((f>>3)&0x07) == 4) ? (READ_RDATA_16(r,a,p)) : \
	 (((f>>3)&0x07) == 5) ? (READ_RDATA_32(r,a,p)) : 0)

// 1 bpp pixel write
#define WRITE_PIXEL_1(a,d)       { jaguar_byte_write(a##_addr+PIXEL_OFFSET_1(a),(jaguar_byte_read(a##_addr+PIXEL_OFFSET_1(a))&(~(0x01 << PIXEL_SHIFT_1(a))))|(d<<PIXEL_SHIFT_1(a))); }

// 2 bpp pixel write
#define WRITE_PIXEL_2(a,d)       { jaguar_byte_write(a##_addr+PIXEL_OFFSET_2(a),(jaguar_byte_read(a##_addr+PIXEL_OFFSET_2(a))&(~(0x03 << PIXEL_SHIFT_2(a))))|(d<<PIXEL_SHIFT_2(a))); }

// 4 bpp pixel write
#define WRITE_PIXEL_4(a,d)       { jaguar_byte_write(a##_addr+PIXEL_OFFSET_4(a),(jaguar_byte_read(a##_addr+PIXEL_OFFSET_4(a))&(~(0x0f << PIXEL_SHIFT_4(a))))|(d<<PIXEL_SHIFT_4(a))); }

// 8 bpp pixel write
#define WRITE_PIXEL_8(a,d)       { jaguar_byte_write(a##_addr+PIXEL_OFFSET_8(a),d); }

// 16 bpp pixel write
#define WRITE_PIXEL_16(a,d)     {  jaguar_word_write(a##_addr+(PIXEL_OFFSET_16(a)<<1),d); }

// 32 bpp pixel write
#define WRITE_PIXEL_32(a,d)		{ jaguar_long_write(a##_addr+(PIXEL_OFFSET_32(a)<<2),d); } 

// pixel write
#define WRITE_PIXEL(a,f,d) {\
	switch ((f>>3)&0x07) { \
	case 0: WRITE_PIXEL_1(a,d);  break;  \
	case 1: WRITE_PIXEL_2(a,d);  break;  \
	case 2: WRITE_PIXEL_4(a,d); break;  \
	case 3: WRITE_PIXEL_8(a,d);  break;  \
	case 4: WRITE_PIXEL_16(a,d); break;  \
	case 5: WRITE_PIXEL_32(a,d); break;  \
	}} \



// Width in Pixels of a Scanline
static uint32 blitter_scanline_width[48] = 
{             
     0,     0,     0,      0,      2,      0,      0,      0,      4,
     0,     6,     0,      8,     10,     12,     14,     16,     20,
    24,    28,    32,     40,     48,     56,     64,     80,     96,
   112,   128,   160,    192,    224,    256,    320,    384,    448,
   512,   640,   768,    896,   1024,   1280,   1536,   1792,   2048,
  2560,  3072,  3584
};

static uint8 * tom_ram_8;
static uint8 * paletteRam;
static uint8 src;
static uint8 dst;
static uint8 misc;
static uint8 a1ctl;
static uint8 mode;
static uint8 ity;
static uint8 zop;
static uint8 op;
static uint8 ctrl;
static uint32 a1_addr;
static uint32 a2_addr;
static int32 a1_zoffs;
static int32 a2_zoffs;
static uint32 xadd_a1_control;
static uint32 xadd_a2_control;
static int32 a1_pitch;
static int32 a2_pitch;
static uint32 n_pixels;
static uint32 n_lines;
static int32 a1_x;
static int32 a1_y;
static int32 a1_width;
static int32 a2_x;
static int32 a2_y;
static int32 a2_width;
static int32 a2_mask_x;
static int32 a2_mask_y;
static int32 a1_xadd;
static int32 a1_yadd;
static int32 a2_xadd;
static int32 a2_yadd;
static uint8 a1_phrase_mode;
static uint8 a2_phrase_mode;
static int32 a1_step_x=0;
static int32 a1_step_y=0;
static int32 a2_step_x=0;
static int32 a2_step_y=0;
static uint32 outer_loop;
static uint32 inner_loop;
static uint32 a2_psize;
static uint32 a1_psize;
static uint32 gouraud_add;
static uint32 gouraud_data;
static uint16 gint[4];
static uint16 gfrac[4];
static uint8  gcolour[4];
static int	  gd_i[4];
static int    gd_c[4];
static int    gd_ia,gd_ca;
static int    colour_index = 0;
static int32  zadd;
static uint32  z_i[4];

static uint8 blitter_code_cache[4096];
static uint8 * blitter_ptr;
uint8 blitter_working = 0;

typedef void (blitter_fn)(void);

typedef struct s_blitter_cache
{
	uint32 hashcode;
	uint8  *code;
	uint32 ready;
	uint8	used;
	struct s_blitter_cache *next;
	struct s_blitter_cache *prev;
} s_blitter_code_cache;

s_blitter_cache *blitter_cache[256];

uint8 blitter_cache_init=0;
static uint8 BPP_LUT[8]={1,2,4,8,16,32,0,0};

FILE *blitters_code_fp;
FILE *blitters_code_init_fp;

//////////////////////////////////////////////////////////////////////////////
// build C code for the specified blitter
//////////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////////
void blitter_gen_c_code(FILE *fp, uint32 cmd,uint32 hashcode)
{
	static uint8 inhibit_modified=0;

	fprintf(fp,"#ifndef blitter_code_0x%.8x\n",hashcode);
	fprintf(fp,"#define blitter_code_0x%.8x\n",hashcode);

	fprintf(fp,"void blitter_0x%.8x(void)\n",hashcode);
	fprintf(fp,"{\n");
	fprintf(fp,"\twhile (outer_loop--)\n");
	fprintf(fp,"\t{\n");
	fprintf(fp,"\t\tinner_loop=n_pixels;\n");
	fprintf(fp,"\t\twhile (inner_loop--)\n");
	fprintf(fp,"\t\t{\n");
	fprintf(fp,"\t\t\tuint32 srcdata   = 0;\n");
	fprintf(fp,"\t\t\tuint32 srczdata  = 0;\n");
	fprintf(fp,"\t\t\tuint32 dstdata   = 0;\n");
	fprintf(fp,"\t\t\tuint32 dstzdata  = 0;\n");
	fprintf(fp,"\t\t\tuint32 writedata = 0;\n");
	fprintf(fp,"\t\t\tuint32 inhibit   = 0;\n");
	
	char *src;
	char *dst;
	uint32 src_flags;
	uint32 dst_flags;

	if (!DSTA2)
	{
		src="a2";
		dst="a1";
		src_flags=A2_FLAGS;
		dst_flags=A1_FLAGS;
	}
	else
	{
		src="a1";
		dst="a2";
		src_flags=A1_FLAGS;
		dst_flags=A2_FLAGS;
	}

	// load src data and Z
	if (SRCEN)
	{
		fprintf(fp,"\t\t\tsrcdata = READ_PIXEL_%i(%s);\n",BPP_LUT[(((REG(dst_flags))>>3)&0x07)],src);
		if (SRCENZ)
			fprintf(fp,"\t\t\tsrczdata = READ_ZDATA_%i(%s);\n",BPP_LUT[(((REG(dst_flags))>>3)&0x07)],src);
		else 
		if (cmd & 0x001c020)
			fprintf(fp,"\t\t\tsrczdata = READ_RDATA_%i(SRCZINT, %s, %s_phrase_mode);\n",BPP_LUT[(((REG(src_flags))>>3)&0x07)],src,src);
	}
	else
	{
		fprintf(fp,"\t\t\tsrcdata = READ_RDATA_%i(SRCDATA, %s, %s_phrase_mode);\n",BPP_LUT[(((REG(dst_flags))>>3)&0x07)],src,src);
		if (cmd & 0x001c020)
			fprintf(fp,"\t\t\tsrczdata = READ_RDATA_%i(SRCZINT, %s, %s_phrase_mode);\n",BPP_LUT[(((REG(dst_flags))>>3)&0x07)],src,src);
	}

	// load dst data and Z 
	if (DSTEN)
	{
		fprintf(fp,"\t\t\tdstdata = READ_PIXEL_%i(%s);\n",BPP_LUT[(((REG(dst_flags))>>3)&0x07)],dst);
		if (DSTENZ)
			fprintf(fp,"\t\t\tdstzdata = READ_ZDATA_%i(%s);\n",BPP_LUT[(((REG(dst_flags))>>3)&0x07)],dst);
		else
			fprintf(fp,"\t\t\tdstzdata = READ_RDATA_%i(DSTZ, %s, %s_phrase_mode);\n",BPP_LUT[(((REG(dst_flags))>>3)&0x07)],dst,dst);
	}
	else
	{
		fprintf(fp,"\t\t\tdstdata = READ_RDATA_%i(DSTDATA, %s, %s_phrase_mode);\n",BPP_LUT[(((REG(dst_flags))>>3)&0x07)],dst,dst);

		if (DSTENZ)
			fprintf(fp,"\t\t\tdstzdata = READ_RDATA_%i(DSTZ, %s, %s_phrase_mode);\n",BPP_LUT[(((REG(dst_flags))>>3)&0x07)],dst,dst);
	}

	// a1 clipping
	if ((cmd & 0x00000040)&&(!DSTA2))

	{
		fprintf(fp,"\t\t\tif (a1_x < 0 || a1_y < 0 || (a1_x >> 16) >= (REG(A1_CLIP) & 0x7fff) || (a1_y >> 16) >= ((REG(A1_CLIP) >> 16) & 0x7fff))	inhibit = 1;\n");
		 inhibit_modified=1;
	}
	if(GOURZ) 
	{
		fprintf(fp,"\t\t\tsrczdata=z_i[colour_index]>>16;\n");
	}
	// apply z comparator
	if (Z_OP_INF) { fprintf(fp,"\t\t\tif (srczdata <  dstzdata)	inhibit = 1;\n"); inhibit_modified=1;}
	if (Z_OP_EQU) { fprintf(fp,"\t\t\tif (srczdata == dstzdata)	inhibit = 1;\n"); inhibit_modified=1;}
	if (Z_OP_SUP) { fprintf(fp,"\t\t\tif (srczdata >  dstzdata)	inhibit = 1;\n"); inhibit_modified=1;}

	// apply data comparator
	if (DCOMPEN)
	{
		if (!CMPDST)
		{
			// compare source pixel with pattern pixel
			fprintf(fp,"\t\t\tif (srcdata == READ_RDATA_%i(PATTERNDATA, %s,%s_phrase_mode)) inhibit=1;\n",BPP_LUT[(((REG(dst_flags))>>3)&0x07)],src,src);
			inhibit_modified=1;
		}
		else
		{
			// compare destination pixel with pattern pixel
			fprintf(fp,"\t\t\tif (dstdata == READ_RDATA_%i(PATTERNDATA, %s,%s_phrase_mode)) inhibit=1;\n",BPP_LUT[(((REG(dst_flags))>>3)&0x07)],dst,dst);
			inhibit_modified=1;
		}
	}
	
	// compute the write data and store
		if (inhibit_modified) fprintf(fp,"\t\t\tif (!inhibit)\n\t\t\t{\n");
		if (PATDSEL)
		{
			// use pattern data for write data
			fprintf(fp,"\t\t\t\twritedata= READ_RDATA_%i(PATTERNDATA, %s, %s_phrase_mode);\n",BPP_LUT[(((REG(dst_flags))>>3)&0x07)],dst,dst);
		}
		else 
		if (INTADD)
		{
			// intensity addition
			fprintf(fp,"\t\t\t\twritedata = (srcdata & 0xff) + (dstdata & 0xff);\n");
			if (!(TOPBEN))
				fprintf(fp,"\t\t\t\tif (writedata > 0xff) writedata = 0xff;\n");

			fprintf(fp,"\t\t\t\twritedata |= (srcdata & 0xf00) + (dstdata & 0xf00);\n");
			if (!(TOPNEN)) fprintf(fp,"\t\t\t\tif (writedata > 0xfff) writedata = 0xfff;\n");
			fprintf(fp,"\t\t\t\twritedata |= (srcdata & 0xf000) + (dstdata & 0xf000);\n");
		}
		else
		{
			if (LFU_NAN) fprintf(fp,"\t\t\t\twritedata |= ~srcdata & ~dstdata;\n");
			if (LFU_NA)  fprintf(fp,"\t\t\t\twritedata |= ~srcdata & dstdata;\n");
			if (LFU_AN)  fprintf(fp,"\t\t\t\twritedata |= srcdata  & ~dstdata;\n");
			if (LFU_A) 	 fprintf(fp,"\t\t\t\twritedata |= srcdata  & dstdata;\n");
		}
		if(GOURD) 
		{
			fprintf(fp,"\t\t\t\twritedata = ((gd_c[colour_index])<<8)|(gd_i[colour_index]>>16);\n");
		}
		if(SRCSHADE) 
		{
			fprintf(fp,"\t\t\t\t{\n");
			fprintf(fp,"\t\t\t\tint intensity = srcdata & 0xFF;\n");
			fprintf(fp,"\t\t\t\tint ia = gd_ia >> 16;\n");
			fprintf(fp,"\t\t\t\tif(ia & 0x80)\n");
			fprintf(fp,"\t\t\t\t	ia = 0xFFFFFF00 | ia;\n");
			fprintf(fp,"\t\t\t\tintensity += ia;\n");
			fprintf(fp,"\t\t\t\tif(intensity < 0)\n");
			fprintf(fp,"\t\t\t\t	intensity = 0;\n");
			fprintf(fp,"\t\t\t\tif(intensity > 0xFF)\n");
			fprintf(fp,"\t\t\t\t	intensity = 0xFF;\n");
			fprintf(fp,"\t\t\t\twritedata = (srcdata & 0xFF00) | intensity;\n");
			fprintf(fp,"\t\t\t\t}\n");
		}
	if (inhibit_modified)  
	{
		fprintf(fp,"\t\t\t} else { srczdata=dstzdata; writedata=dstdata; }\n");
	}

	if ((DSTA2?a2_phrase_mode:a1_phrase_mode) || BKGWREN)
	{
		// write to the destination
		fprintf(fp,"\t\t\tWRITE_PIXEL_%i(%s, writedata);\n",BPP_LUT[(((REG(dst_flags))>>3)&0x07)],dst);
		if (DSTWRZ) fprintf(fp,"\t\t\tWRITE_ZDATA_%i(%s, srczdata);\n",BPP_LUT[(((REG(dst_flags))>>3)&0x07)],dst);
	}
	else
	{
		if (inhibit_modified) fprintf(fp,"\t\t\tif (!inhibit)\n\t\t\t{\n");
		// write to the destination
		fprintf(fp,"\t\t\t\tWRITE_PIXEL_%i(%s, writedata);\n",BPP_LUT[(((REG(dst_flags))>>3)&0x07)],dst);
		if (DSTWRZ) fprintf(fp,"\t\t\t\tWRITE_ZDATA_%i(%s, srczdata);\n",BPP_LUT[(((REG(dst_flags))>>3)&0x07)],dst);
		if (inhibit_modified) fprintf(fp,"\t\t\t}\n");
	}
	// update x and y
	fprintf(fp,"\t\t\ta1_x += a1_xadd;\n");
	fprintf(fp,"\t\t\ta1_y += a1_yadd;\n");
	fprintf(fp,"\t\t\ta2_x = (a2_x + a2_xadd) & a2_mask_x;\n");
	fprintf(fp,"\t\t\ta2_y = (a2_y + a2_yadd) & a2_mask_y;\n");
	if (GOURZ)
	{
		fprintf(fp,"\t\t\tz_i[colour_index]+=zadd;\n");
	}
	if ((GOURD)||(SRCSHADE))
	{
		fprintf(fp,"\t\t\tgd_i[colour_index] += gd_ia;\n");
		fprintf(fp,"\t\t\tgd_c[colour_index] += gd_ca;\n");
	}
	if ((GOURD)||(SRCSHADE)||(GOURZ))
	{
		if (a1_phrase_mode)
		fprintf(fp,"\t\t\t colour_index=(colour_index+1)&0x3;\n");
	}
	fprintf(fp,"\t\t}\n");

	fprintf(fp,"\t\ta1_x+=a1_step_x;\n");
	fprintf(fp,"\t\ta1_y+=a1_step_y;\n");
	fprintf(fp,"\t\ta2_x+=a2_step_x;\n");
	fprintf(fp,"\t\ta2_y+=a2_step_y;\n");
	fprintf(fp,"\t}\n");
	
	// write values back to registers 
	fprintf(fp,"\tWREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));\n");
	fprintf(fp,"\tWREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));\n");
	fprintf(fp,"\tWREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));\n");
	fprintf(fp,"}\n");
	fprintf(fp,"#endif\n");
}

//////////////////////////////////////////////////////////////////////////////
// Generate a start of function in x86 assembly
//////////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////////
void blitter_gen_start_of_function(void)
{
	*blitter_ptr++=0x55;	// push ebp
	*blitter_ptr++=0x8b;	// mov  ebp,esp
	*blitter_ptr++=0xec;
}
//////////////////////////////////////////////////////////////////////////////
// Generate a end of function in x86 assembly
//////////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////////
void blitter_gen_end_of_function(void)
{
	*blitter_ptr++=0x8B;	// mov         esp,ebp
	*blitter_ptr++=0xE5;
	*blitter_ptr++=0x5D;	// pop         ebp
	*blitter_ptr++=0xC3;	// ret
}
//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////////
#define HASHCODE_BIT(C,B) if (C) hashcode|=(1<<B);
#define HASHCODE_BIT_TEST(B) (hashcode&(1<<B))

uint32 blitter_calc_hashcode(uint32 cmd)
{
	uint32 hashcode=0x00000000;

	// source and destination bit depth
	hashcode|=((REG(A1_FLAGS)>>3)&0x07)<<0;
	hashcode|=((REG(A2_FLAGS)>>3)&0x07)<<3;

	HASHCODE_BIT(DSTA2,					6);
	HASHCODE_BIT(SRCEN,					7);
	HASHCODE_BIT(SRCENZ,				8);
	HASHCODE_BIT(DSTEN,					9);
	HASHCODE_BIT(DSTENZ,				10);
	HASHCODE_BIT(Z_OP_INF,				11);
	HASHCODE_BIT(Z_OP_EQU,				12);
	HASHCODE_BIT(Z_OP_SUP,				13);
	HASHCODE_BIT(DCOMPEN,				14);
	HASHCODE_BIT(CMPDST,				15);
	HASHCODE_BIT(PATDSEL,				16);
	HASHCODE_BIT(INTADD,				17);
	HASHCODE_BIT(TOPBEN,				18);
	HASHCODE_BIT(TOPNEN,				19);
	HASHCODE_BIT(LFU_NAN,				20);
	HASHCODE_BIT(LFU_NA,				21);
	HASHCODE_BIT(LFU_AN,				22);
	HASHCODE_BIT(LFU_A,					23);
	HASHCODE_BIT(BKGWREN,				24);
	HASHCODE_BIT(DSTWRZ,				25);
	HASHCODE_BIT((cmd & 0x001c020),		26); // extra data read/write
	HASHCODE_BIT((cmd & 0x00000040),	27); // source clipping
	HASHCODE_BIT(a1_phrase_mode,		28); 
	HASHCODE_BIT(a2_phrase_mode,		29); 


	return(hashcode);
}
//////////////////////////////////////////////////////////////////////////////
// Build the blitter code for the current blitter operation in the cache
//////////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////////
void blitter_build_cached_code(uint32 cmd, uint32 cache_index)
{
}
//////////////////////////////////////////////////////////////////////////////
// Check if the blitter code for the current blitter operation is cached
//////////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////////
struct s_blitter_cache * blitter_in_cache(uint32 cmd)
{
	uint32 i;
	uint32 hashcode=blitter_calc_hashcode(cmd);
#ifdef LOG_BLITS
	fprintf(log_get(),"blitter: hashcode= 0x%.8x\n",hashcode);
#endif
	struct s_blitter_cache *blitter_list=blitter_cache[hashcode>>24];
	
	i=0;
	while (blitter_list->next)
	{
		blitter_list=blitter_list->next;

		if (blitter_list->hashcode==hashcode)
			return(blitter_list);
	}
#ifdef GEN_CODE
	blitter_list->next=(struct s_blitter_cache *)malloc(sizeof(struct s_blitter_cache));
	blitter_list->next->prev=blitter_list;
	blitter_list->next->next=null;
	blitter_list=blitter_list->next;

	blitter_list->code=(uint8*)malloc(4096);
	blitter_list->hashcode=hashcode;
	blitter_list->ready=0;
	blitter_gen_c_code(blitters_code_fp,cmd,hashcode);
	fprintf(blitters_code_init_fp,"\tblitter_add(0x%.8x,(uint8*)&blitter_0x%.8x);\n",hashcode,hashcode);
#else
	//fprintf(log_get(),"warning: using generic blitter for blitter 0x%.8x\n",hashcode);
#endif
	return(null);
}
#ifndef USE_GENERIC_BLITTER
#include "include/blit_c.h"
#endif
//////////////////////////////////////////////////////////////////////////////
// Execute the cached blitter code for the current blitter operation
//////////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////////
uint32 blitter_execute_cached_code(struct s_blitter_cache *blitter)
{
	if ((blitter==null)||(blitter->ready==0))
		return 0;

	blitter_fn *fn=(blitter_fn*)blitter->code;
	blitter->used=1;
	(*fn)();

	return(1);
}
//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////////
void blitter_add(uint32 hashcode, uint8 *code)
{
	struct s_blitter_cache *blitter_list=blitter_cache[(hashcode>>24)];

//	fprintf(log_get(),"adding blitter for hashcode 0x%.8x\n",hashcode);

	while (blitter_list->next)
	{
		blitter_list=blitter_list->next;

		if (blitter_list->hashcode==hashcode)
			return;
	}
	blitter_list->next=(struct s_blitter_cache *)malloc(sizeof(struct s_blitter_cache));
	blitter_list->next->prev=blitter_list;
	blitter_list->next->next=null;
	blitter_list=blitter_list->next;

	blitter_list->code=code;
	blitter_list->hashcode=hashcode;
	blitter_list->ready=1;
	blitter_list->used=0;
}
//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////////
void blitter_list(void)
{
/*
	fprintf(log_get(),"Used blitters list:\n");

	for (int i=0;i<256;i++)
	{
		struct s_blitter_cache *blitter_list=blitter_cache[i];

		while (blitter_list->next)
		{
			blitter_list=blitter_list->next;
			if (blitter_list->used)
				fprintf(log_get(),"\t0%.8x\n",blitter_list->hashcode);
		}
	}
*/
}
//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////////
void blitter_generic(uint32 cmd)
{
	uint32 srcdata   = 0;
	uint32  srczdata  = 0;
	uint32 dstdata   = 0;
	uint32  dstzdata  = 0;
	uint32 writedata = 0;
	uint32 inhibit   = 0;
	while (outer_loop--)
	{
		inner_loop=n_pixels;
		while (inner_loop--)
		{
			srcdata   = 0;
			srczdata  = 0;
			dstdata   = 0;
			dstzdata  = 0;
			writedata = 0;
			inhibit   = 0;

			if (!DSTA2)
			{
				// load src data and Z
				if (SRCEN)
				{
					srcdata = READ_PIXEL(a2, REG(A2_FLAGS));
					if (SRCENZ)
					{
						srczdata = READ_ZDATA(a2, REG(A2_FLAGS));
					}
					else 
					if (cmd & 0x001c020)
					{
						srczdata = READ_RDATA(SRCZINT, a2, REG(A2_FLAGS), a2_phrase_mode);
					}
				}
				else
				{
					srcdata = READ_RDATA(SRCDATA, a2, REG(A2_FLAGS), a2_phrase_mode);
					if (cmd & 0x001c020)
					{
						srczdata = READ_RDATA(SRCZINT, a2, REG(A2_FLAGS), a2_phrase_mode);
					}
				}

				// load dst data and Z 
				if (DSTEN)
				{
					dstdata = READ_PIXEL(a1, REG(A1_FLAGS));
					if (DSTENZ)
					{
						dstzdata = READ_ZDATA(a1, REG(A1_FLAGS));
					}
					else
					{
						dstzdata = READ_RDATA(DSTZ, a1, REG(A1_FLAGS), a1_phrase_mode);
					}
				}
				else
				{
					dstdata = READ_RDATA(DSTDATA, a1, REG(A1_FLAGS), a1_phrase_mode);
		
					if (DSTENZ)
					{
						dstzdata = READ_RDATA(DSTZ, a1, REG(A1_FLAGS), a1_phrase_mode);
					}
				}

				// a1 clipping
				if (cmd & 0x00000040)
				{
					if ( 
						 a1_x < 0 || 
						 a1_y < 0 ||
					     (a1_x >> 16) >= (REG(A1_CLIP) & 0x7fff) ||
						 (a1_y >> 16) >= ((REG(A1_CLIP) >> 16) & 0x7fff)
						)
						inhibit = 1;
				}

				if(GOURZ) 
					srczdata=z_i[colour_index]>>16;

				// apply z comparator
				if (Z_OP_INF) if (srczdata <  dstzdata)	inhibit = 1;
				if (Z_OP_EQU) if (srczdata == dstzdata)	inhibit = 1;
				if (Z_OP_SUP) if (srczdata >  dstzdata)	inhibit = 1;
				
				// apply data comparator
				if (DCOMPEN|BCOMPEN)
				{
					if (!CMPDST)
					{
						// compare source pixel with pattern pixel
						if (srcdata == READ_RDATA(PATTERNDATA, a2, REG(A2_FLAGS), a2_phrase_mode))
							inhibit=1;
					}
					else
					{
						// compare destination pixel with pattern pixel
						if (dstdata == READ_RDATA(PATTERNDATA, a1, REG(A1_FLAGS), a1_phrase_mode))
							inhibit=1;
					}
					if (a1_phrase_mode||a2_phrase_mode)
						inhibit=!inhibit;
				}
				
				// compute the write data and store
				if (!inhibit)
				{			
					if (PATDSEL)
					{
						// use pattern data for write data
						writedata= READ_RDATA(PATTERNDATA, a1, REG(A1_FLAGS), a1_phrase_mode);
					}
					else 
					if (INTADD)
					{
						// intensity addition
						writedata = (srcdata & 0xff) + (dstdata & 0xff);
						if (!(TOPBEN) && writedata > 0xff)
							writedata = 0xff;
						writedata |= (srcdata & 0xf00) + (dstdata & 0xf00);
						if (!(TOPNEN) && writedata > 0xfff)
							writedata = 0xfff;
						writedata |= (srcdata & 0xf000) + (dstdata & 0xf000);
					}
					else
					{
						if (LFU_NAN) writedata |= ~srcdata & ~dstdata;
						if (LFU_NA)  writedata |= ~srcdata & dstdata;
						if (LFU_AN)  writedata |= srcdata  & ~dstdata;
						if (LFU_A) 	 writedata |= srcdata  & dstdata;
					}
					if(GOURD) 
						writedata = ((gd_c[colour_index])<<8)|(gd_i[colour_index]>>16);

					if(SRCSHADE) 
					{
						int intensity = srcdata & 0xFF;
						int ia = gd_ia >> 16;
						if(ia & 0x80)
							ia = 0xFFFFFF00 | ia;
						intensity += ia;
						if(intensity < 0)
							intensity = 0;
						if(intensity > 0xFF)
							intensity = 0xFF;
						writedata = (srcdata & 0xFF00) | intensity;
					}
				}
				else
				{
					writedata=dstdata;
					srczdata=dstzdata;
				}
				if (/*a1_phrase_mode || */BKGWREN || !inhibit)
				{
					// write to the destination
					WRITE_PIXEL(a1, REG(A1_FLAGS), writedata);
					if (DSTWRZ) WRITE_ZDATA(a1, REG(A1_FLAGS), srczdata);
				}
			}
			else
			{
				// load src data and Z
				if (SRCEN)
				{
					srcdata = READ_PIXEL(a1, REG(A1_FLAGS));
					if (SRCENZ)
					{
						srczdata = READ_ZDATA(a1, REG(A1_FLAGS));
					}
					else 
					if (cmd & 0x001c020)
					{
						srczdata = READ_RDATA(SRCZINT, a1, REG(A1_FLAGS), a1_phrase_mode);
					}
				}
				else
				{
					srcdata = READ_RDATA(SRCDATA, a1, REG(A1_FLAGS), a1_phrase_mode);
					if (cmd & 0x001c020)
					{
						srczdata = READ_RDATA(SRCZINT, a1, REG(A1_FLAGS), a1_phrase_mode);
					}
				}

				// load dst data and Z 
				if (DSTEN)
				{
					dstdata = READ_PIXEL(a2, REG(A2_FLAGS));
					if (DSTENZ)
					{
						dstzdata = READ_ZDATA(a2, REG(A2_FLAGS));
					}
					else
					{
						dstzdata = READ_RDATA(DSTZ, a2, REG(A2_FLAGS), a2_phrase_mode);
					}
				}
				else
				{
					dstdata = READ_RDATA(DSTDATA, a2, REG(A2_FLAGS), a2_phrase_mode);
		
					if (DSTENZ)
					{
						dstzdata = READ_RDATA(DSTZ, a2, REG(A2_FLAGS), a2_phrase_mode);
					}
				}

				if(GOURZ) 
					srczdata=z_i[colour_index]>>16;

				// apply z comparator
				if (Z_OP_INF) if (srczdata < dstzdata)	inhibit = 1;
				if (Z_OP_EQU) if (srczdata == dstzdata)	inhibit = 1;
				if (Z_OP_SUP) if (srczdata > dstzdata)	inhibit = 1;
				
				// apply data comparator
				if (DCOMPEN|BCOMPEN)
				{
					if (!CMPDST)
					{
						// compare source pixel with pattern pixel
						if (srcdata == READ_RDATA(PATTERNDATA, a1, REG(A1_FLAGS), a1_phrase_mode))
							inhibit=1;
					}
					else
					{
						// compare destination pixel with pattern pixel
						if (dstdata == READ_RDATA(PATTERNDATA, a2, REG(A2_FLAGS), a2_phrase_mode))
							inhibit=1;
					}
					if (a1_phrase_mode||a2_phrase_mode)
						inhibit=!inhibit;
				}
				
				// compute the write data and store
				if (!inhibit)
				{			
					if (PATDSEL)
					{
						// use pattern data for write data
						writedata= READ_RDATA(PATTERNDATA, a2, REG(A2_FLAGS), a2_phrase_mode);
					}
					else 
					if (INTADD)
					{
						// intensity addition
						writedata = (srcdata & 0xff) + (dstdata & 0xff);
						if (!(TOPBEN) && writedata > 0xff)
							writedata = 0xff;
						writedata |= (srcdata & 0xf00) + (dstdata & 0xf00);
						if (!(TOPNEN) && writedata > 0xfff)
							writedata = 0xfff;
						writedata |= (srcdata & 0xf000) + (dstdata & 0xf000);
					}
					else
					{
						if (LFU_NAN)
							writedata |= ~srcdata & ~dstdata;
						if (LFU_NA)
							writedata |= ~srcdata & dstdata;
						if (LFU_AN)
							writedata |= srcdata & ~dstdata;
						if (LFU_A)
							writedata |= srcdata & dstdata;
					}
					if(GOURD) 
						writedata = ((gd_c[colour_index])<<8)|(gd_i[colour_index]>>16);

					if(SRCSHADE) 
					{
						int intensity = srcdata & 0xFF;
						int ia = gd_ia >> 16;
						if(ia & 0x80)
							ia = 0xFFFFFF00 | ia;
						intensity += ia;
						if(intensity < 0)
							intensity = 0;
						if(intensity > 0xFF)
							intensity = 0xFF;
						writedata = (srcdata & 0xFF00) | intensity;
					}
				}
				else
				{
					writedata=dstdata;
					srczdata=dstzdata;
				}

				if (/*a2_phrase_mode || */BKGWREN || !inhibit)
				{
					// write to the destination
					WRITE_PIXEL(a2, REG(A2_FLAGS), writedata);
					if (DSTWRZ)
						WRITE_ZDATA(a2, REG(A2_FLAGS), srczdata);
				}
			}
			// update x and y
			a1_x += a1_xadd;
			a1_y += a1_yadd;
			a2_x = (a2_x + a2_xadd) & a2_mask_x;
			a2_y = (a2_y + a2_yadd) & a2_mask_y;

			if (GOURZ)
				z_i[colour_index]+=zadd;

			if ((GOURD)||(SRCSHADE))
			{
				gd_i[colour_index] += gd_ia;
				gd_c[colour_index] += gd_ca;
			}
			if ((GOURD)||(SRCSHADE)||(GOURZ))
			{
				if(a1_phrase_mode)
					colour_index=(colour_index+1)&0x3;
			}
		}

		a1_x+=a1_step_x;
		a1_y+=a1_step_y;
		a2_x+=a2_step_x;
		a2_y+=a2_step_y;

/*		if (a2_phrase_mode)
		{
			a1_x+=(64/a1_psize)*a1_xadd;
		}	
		if (a2_phrase_mode)
		{
			for (int nb=0;nb<(64/a2_psize)+1;nb++)
				a2_x = (a2_x + a2_xadd) & a2_mask_x;
		}
*/	}
	
	// write values back to registers 
	WREG(A1_PIXEL,  (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff));
	WREG(A1_FPIXEL, (a1_y << 16) | (a1_x & 0xffff));
	WREG(A2_PIXEL,  (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff));
}
//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////////
void blitter_blit(uint32 cmd)
{
	colour_index = 0;
	src=(cmd&0x07);
	dst=(cmd>>3)&0x07;
	misc=(cmd>>6)&0x03;
	a1ctl=(cmd>>8)&0x7;
	mode=(cmd>>11)&0x07;
	ity=(cmd>>14)&0x0f;
	zop=(cmd>>18)&0x07;
	op=(cmd>>21)&0x0f;
	ctrl=(cmd>>25)&0x3f;

	a1_addr=REG(A1_BASE);
	a2_addr=REG(A2_BASE);

	a1_zoffs = (REG(A1_FLAGS) >> 6) & 7;
	a2_zoffs = (REG(A2_FLAGS) >> 6) & 7;
	
	xadd_a1_control=(REG(A1_FLAGS)>>16)&0x03;
	xadd_a2_control=(REG(A2_FLAGS)>>16)&0x03;
	a1_pitch = (REG(A1_FLAGS) & 3) ^ ((REG(A1_FLAGS) & 2) >> 1);
	a2_pitch = (REG(A2_FLAGS) & 3) ^ ((REG(A2_FLAGS) & 2) >> 1);

	n_pixels=(REG(PIXLINECOUNTER)&0xffff);
	n_lines=((REG(PIXLINECOUNTER)>>16)&0xffff);

	a1_x=(REG(A1_PIXEL)<<16)|(REG(A1_FPIXEL)&0xffff);
	a1_y=(REG(A1_PIXEL)&0xffff0000)|(REG(A1_FPIXEL)>>16);
	a1_width=blitter_scanline_width[((REG(A1_FLAGS)&0x00007E00)>>9)];

	a2_x=(REG(A2_PIXEL)&0x0000ffff)<<16;
	a2_y=(REG(A2_PIXEL)&0xffff0000);
	a2_width=blitter_scanline_width[((REG(A2_FLAGS)&0x00007E00)>>9)];
	a2_mask_x=0xffff|((REG(A2_MASK)&0x0000ffff)<<16);
	a2_mask_y=((REG(A2_MASK)&0xffff0000)|0xffff);
	
	// 
	if (!((REG(A2_FLAGS))&0x8000))
	{
		a2_mask_x=0xffffffff; // must be 16.16
		a2_mask_y=0xffffffff; // must be 16.16
	}
	
	a1_phrase_mode=0;

	// determine a1_yadd
	if (YADD1_A1)
		a1_yadd = 1 << 16;
	else
		a1_yadd = 0;

	if (YSIGNSUB_A1)
		a1_yadd=-a1_yadd;

	// determine a1_xadd
	switch (xadd_a1_control)
	{
	case XADDPHR:
				// add phrase offset to X and truncate
				a1_xadd = 1 << 16;

				a1_phrase_mode=1;
				break;
	case XADDPIX:
				// add pixelsize (1) to X
				a1_xadd = 1 << 16;
				break;
	case XADD0:	
				// add zero (for those nice vertical lines)
				a1_xadd = 0;
				break;
	case XADDINC:
				// add the contents of the increment register
				a1_xadd = (REG(A1_INC) << 16)		 | (REG(A1_FINC) & 0xffff);
				a1_yadd = (REG(A1_INC) & 0xffff0000) | (REG(A1_FINC) >> 16);
				break;
	}
	if (XSIGNSUB_A1)
		a1_xadd=-a1_xadd;

	// determine a2_yadd
	if ((YADD1_A2)||(YADD1_A1))
		a2_yadd = 1 << 16;
	else
		a2_yadd = 0;

	if (YSIGNSUB_A2)
		a2_yadd=-a2_yadd;

	a2_phrase_mode=0;

	// determine a2_xadd
	switch (xadd_a2_control)
	{
	case XADDPHR:
				// add phrase offset to X and truncate
				a2_xadd = 1 << 16;

				a2_phrase_mode=1;
				break;
	case XADDPIX:
				// add pixelsize (1) to X
				a2_xadd = 1 << 16;
				break;
	case XADD0:	
				// add zero (for those nice vertical lines)
				a2_xadd = 0;
				break;
	case XADDINC:
				// add the contents of the increment register
				// since there is no register for a2 we just add 1
				a2_xadd = 1 << 16;
				break;
	}
	if (XSIGNSUB_A2)
		a2_xadd=-a2_xadd;

	// modify outer loop steps based on command 
	a1_step_x=0;
	a1_step_y=0;
	a2_step_x=0;
	a2_step_y=0;

	if (cmd & 0x00000100)
	{
		a1_step_x = (REG(A1_FSTEP)&0xffff);
		a1_step_y = (REG(A1_FSTEP)>>16);
	}
	if (cmd & 0x00000200)
	{
		a1_step_x += ((REG(A1_STEP)&0x0000ffff)<<16);
		a1_step_y += ((REG(A1_STEP)&0xffff0000));
	}
	if (cmd & 0x00000400)
	{
		a2_step_x = (REG(A2_STEP)&0x0000ffff)<<16;
		a2_step_y = (REG(A2_STEP)&0xffff0000);
	}


	outer_loop=n_lines;


	a2_psize=1 << ((REG(A2_FLAGS) >> 3) & 7);
	a1_psize=1 << ((REG(A1_FLAGS) >> 3) & 7);

	// zbuffering
	if (GOURZ)
	{
		zadd=jaguar_long_read(0xF02274);

		for(int v=0;v<4;v++) 
			z_i[v]=(int32)jaguar_long_read(0xF0228C+(v<<2));
	}
	if ((GOURD)||(GOURZ)||(SRCSHADE))
	{
		// gouraud shading
		gouraud_add = jaguar_long_read(0xF02270);

		
		gd_c[0]	= jaguar_byte_read(0xF02268);
		gd_i[0]	= jaguar_byte_read(0xF02269);
		gd_i[0]<<=16;
		gd_i[0]|=jaguar_word_read(0xF02240);

		gd_c[1]	= jaguar_byte_read(0xF0226A);
		gd_i[1]	= jaguar_byte_read(0xF0226b);
		gd_i[1]<<=16;
		gd_i[1]|=jaguar_word_read(0xF02242);

		gd_c[2]	= jaguar_byte_read(0xF0226C);
		gd_i[2]	= jaguar_byte_read(0xF0226d);
		gd_i[2]<<=16;
		gd_i[2]|=jaguar_word_read(0xF02244);

		gd_c[3]	= jaguar_byte_read(0xF0226E);
		gd_i[3]	= jaguar_byte_read(0xF0226f);
		gd_i[3]<<=16; 
		gd_i[3]|=jaguar_word_read(0xF02246);

		gd_ia = gouraud_add & 0xFFFFFF;
		if(gd_ia & 0x800000)
			gd_ia = 0xFF000000 | gd_ia;

		gd_ca = (gouraud_add>>24) & 0xFF;
		if(gd_ca & 0x80)
			gd_ca = 0xFFFFFF00 | gd_ca;
	}

	// fix for zoop! and syndicate
	if ((jaguar_mainRom_crc32==0x501be17c)||
		(jaguar_mainRom_crc32==0x70895c51)||
		(jaguar_mainRom_crc32==0x0f1f1497)||
		(jaguar_mainRom_crc32==0xfc8f0dcd)
	   )
	{
		if (a1_step_x<0)
			a1_step_x=(-n_pixels)*65536;

		if (a2_step_x<0)
			a2_step_x=(-n_pixels)*65536;;
	}
	else
	// fix for wolfenstein 3d
	if (jaguar_mainRom_crc32==0x3966698f)
	{
		if (n_pixels==24)
		{
			if ((a1_step_x / 65536)==-28)
			{
				a1_step_x=-24*65536; // au lieu de -28
				a2_step_x=  0*65536; // au lieu de -8
			}
		}
	} 
	else
	// fix for Tempest 2000
	if (jaguar_mainRom_crc32==0x32816d44)
	{
/*
		if ((n_lines!=1)&&((n_pixels==288)||(n_pixels==384)))
		{
			fprintf(log_get(),"Blit!\n");
			fprintf(log_get(),"  cmd      = 0x%.8x\n",cmd);
			fprintf(log_get(),"  a1_base  = %08X\n", a1_addr);
			fprintf(log_get(),"  a1_pitch = %d\n", a1_pitch);
			fprintf(log_get(),"  a1_psize = %d\n", a1_psize);
			fprintf(log_get(),"  a1_width = %d\n", a1_width);
			fprintf(log_get(),"  a1_xadd  = %f (phrase=%d)\n", (float)a1_xadd / 65536.0, a1_phrase_mode);
			fprintf(log_get(),"  a1_yadd  = %f\n", (float)a1_yadd / 65536.0);
			fprintf(log_get(),"  a1_xstep = %f\n", (float)a1_step_x / 65536.0);
			fprintf(log_get(),"  a1_ystep = %f\n", (float)a1_step_y / 65536.0);
			fprintf(log_get(),"  a1_x     = %f\n", (float)a1_x / 65536.0);
			fprintf(log_get(),"  a1_y     = %f\n", (float)a1_y / 65536.0);
			fprintf(log_get(),"  a1_zoffs = %i\n",a1_zoffs);

			fprintf(log_get(),"  a2_base  = %08X\n", a2_addr);
			fprintf(log_get(),"  a2_pitch = %d\n", a2_pitch);
			fprintf(log_get(),"  a2_psize = %d\n", a2_psize);
			fprintf(log_get(),"  a2_width = %d\n", a2_width);
			fprintf(log_get(),"  a2_xadd  = %f (phrase=%d)\n", (float)a2_xadd / 65536.0, a2_phrase_mode);
			fprintf(log_get(),"  a2_yadd  = %f\n", (float)a2_yadd / 65536.0);
			fprintf(log_get(),"  a2_xstep = %f\n", (float)a2_step_x / 65536.0);
			fprintf(log_get(),"  a2_ystep = %f\n", (float)a2_step_y / 65536.0);
			fprintf(log_get(),"  a2_x     = %f\n", (float)a2_x / 65536.0);
			fprintf(log_get(),"  a2_y     = %f\n", (float)a2_y / 65536.0);
			fprintf(log_get(),"  a2_mask_x= 0x%.4x\n",a2_mask_x);
			fprintf(log_get(),"  a2_mask_y= 0x%.4x\n",a2_mask_y);
			fprintf(log_get(),"  a2_zoffs = %i\n",a2_zoffs);

			fprintf(log_get(),"  count    = %d x %d\n", n_pixels, n_lines);

			fprintf(log_get(),"  command  = %08X\n", cmd);
			fprintf(log_get(),"  dsten    = %i\n",DSTEN);
			fprintf(log_get(),"  srcen    = %i\n",SRCEN);
			fprintf(log_get(),"  patdsel  = %i\n",PATDSEL);
			fprintf(log_get(),"  color    = 0x%.8x\n",REG(PATTERNDATA));
			fprintf(log_get(),"  dcompen  = %i\n",DCOMPEN);
			fprintf(log_get(),"  bcompen  = %i\n",BCOMPEN);
			fprintf(log_get(),"  cmpdst   = %i\n",CMPDST);
			fprintf(log_get(),"  GOURZ    = %i\n",GOURZ);
			fprintf(log_get(),"  GOURD    = %i\n",GOURD);
			fprintf(log_get(),"  SRCSHADE = %i\n",SRCSHADE);
			fprintf(log_get(),"  DSTDATA  = 0x%.8x%.8x\n",REG(DSTDATA),REG(DSTDATA+4));
		}	
*/	}

#ifdef LOG_BLITS
//	if (start_logging)
	{
		fprintf(log_get(),"Blit!\n");
		fprintf(log_get(),"  cmd      = 0x%.8x\n",cmd);
		fprintf(log_get(),"  a1_base  = %08X\n", a1_addr);
		fprintf(log_get(),"  a1_pitch = %d\n", a1_pitch);
		fprintf(log_get(),"  a1_psize = %d\n", a1_psize);
		fprintf(log_get(),"  a1_width = %d\n", a1_width);
		fprintf(log_get(),"  a1_xadd  = %f (phrase=%d)\n", (float)a1_xadd / 65536.0, a1_phrase_mode);
		fprintf(log_get(),"  a1_yadd  = %f\n", (float)a1_yadd / 65536.0);
		fprintf(log_get(),"  a1_xstep = %f\n", (float)a1_step_x / 65536.0);
		fprintf(log_get(),"  a1_ystep = %f\n", (float)a1_step_y / 65536.0);
		fprintf(log_get(),"  a1_x     = %f\n", (float)a1_x / 65536.0);
		fprintf(log_get(),"  a1_y     = %f\n", (float)a1_y / 65536.0);
		fprintf(log_get(),"  a1_zoffs = %i\n",a1_zoffs);

		fprintf(log_get(),"  a2_base  = %08X\n", a2_addr);
		fprintf(log_get(),"  a2_pitch = %d\n", a2_pitch);
		fprintf(log_get(),"  a2_psize = %d\n", a2_psize);
		fprintf(log_get(),"  a2_width = %d\n", a2_width);
		fprintf(log_get(),"  a2_xadd  = %f (phrase=%d)\n", (float)a2_xadd / 65536.0, a2_phrase_mode);
		fprintf(log_get(),"  a2_yadd  = %f\n", (float)a2_yadd / 65536.0);
		fprintf(log_get(),"  a2_xstep = %f\n", (float)a2_step_x / 65536.0);
		fprintf(log_get(),"  a2_ystep = %f\n", (float)a2_step_y / 65536.0);
		fprintf(log_get(),"  a2_x     = %f\n", (float)a2_x / 65536.0);
		fprintf(log_get(),"  a2_y     = %f\n", (float)a2_y / 65536.0);
		fprintf(log_get(),"  a2_mask_x= 0x%.4x\n",a2_mask_x);
		fprintf(log_get(),"  a2_mask_y= 0x%.4x\n",a2_mask_y);
		fprintf(log_get(),"  a2_zoffs = %i\n",a2_zoffs);

		fprintf(log_get(),"  count    = %d x %d\n", n_pixels, n_lines);

		fprintf(log_get(),"  command  = %08X\n", cmd);
		fprintf(log_get(),"  dsten    = %i\n",DSTEN);
		fprintf(log_get(),"  srcen    = %i\n",SRCEN);
		fprintf(log_get(),"  patdsel  = %i\n",PATDSEL);
		fprintf(log_get(),"  color    = 0x%.8x\n",REG(PATTERNDATA));
		fprintf(log_get(),"  dcompen  = %i\n",DCOMPEN);
		fprintf(log_get(),"  bcompen  = %i\n",BCOMPEN);
		fprintf(log_get(),"  cmpdst   = %i\n",CMPDST);
		fprintf(log_get(),"  GOURZ   = %i\n",GOURZ);
		fprintf(log_get(),"  GOURD   = %i\n",GOURD);
		fprintf(log_get(),"  SRCSHADE= %i\n",SRCSHADE);
	}	
#endif


	blitter_working=1;
#ifndef USE_GENERIC_BLITTER
	if (!blitter_execute_cached_code(blitter_in_cache(cmd)))
#endif
		blitter_generic(cmd);
	blitter_working=0;
}

uint32 blitter_reg_read(uint32 offset)
{
	uint32 data = blitter_ram[offset];
	data <<= 8;
	data |= blitter_ram[offset+1];
	data <<= 8;
	data |= blitter_ram[offset+2];
	data <<= 8;
	data |= blitter_ram[offset+3];
	return data;
}

void blitter_reg_write(uint32 offset, uint32 data)
{
	blitter_ram[offset+0] = (data>>24) & 0xFF;
	blitter_ram[offset+1] = (data>>16) & 0xFF;
	blitter_ram[offset+2] = (data>>8) & 0xFF;
	blitter_ram[offset+3] = data & 0xFF;
}

uint32 blitter_long_read(uint32 offset)
{
	return (blitter_word_read(offset) << 16) | blitter_word_read(offset+2);
}

void blitter_long_write(uint32 offset, uint32 data)
{
	blitter_word_write(offset, data >> 16);
	blitter_word_write(offset+2, data & 0xFFFF);
}

void blitter_init(void)
{
	if (!blitter_cache_init)
	{
		for (int i=0;i<256;i++)
		{
			blitter_cache[i]=(struct s_blitter_cache *)malloc(sizeof(struct s_blitter_cache));
			blitter_cache[i]->next=null;
			blitter_cache[i]->prev=null;
		}
		blitter_cache_init=1;
	}
#ifndef USE_GENERIC_BLITTER
	#include "include/blit_i.h"
#endif

	blitter_reset();
#ifdef GEN_CODE
	blitters_code_fp=fopen("include/blit_c.h","awrt");
	blitters_code_init_fp=fopen("include/blit_i.h","awrt");
#endif
}

void blitter_reset(void)
{
	memset(blitter_ram, 0x00, 0xA0);
}

void blitter_done(void)
{
	blitter_list();
#ifdef GEN_CODE
	fclose(blitters_code_fp);
	fclose(blitters_code_init_fp);
#endif
}

void blitter_byte_write(uint32 offset, uint8 data)
{
	offset &= 0xFF;

	if ((offset >= 0x7C) && (offset <= 0x9B))
	{
		switch (offset)
		{
		case 0x7C: break;
		case 0x7D: blitter_ram[0x69] = data; break;
		case 0x7E: blitter_ram[0x40] = data; break;
		case 0x7F: blitter_ram[0x41] = data; break;

		case 0x80: break;
		case 0x81: blitter_ram[0x6B] = data; break;
		case 0x82: blitter_ram[0x42] = data; break;
		case 0x83: blitter_ram[0x43] = data; break;
		
		case 0x84: break;
		case 0x85: blitter_ram[0x6D] = data; break;
		case 0x86: blitter_ram[0x44] = data; break;
		case 0x87: blitter_ram[0x45] = data; break;
		
		case 0x88: break;
		case 0x89: blitter_ram[0x6F] = data; break;
		case 0x9A: blitter_ram[0x46] = data; break;
		case 0x9B: blitter_ram[0x47] = data; break;

		}
	}

	blitter_ram[offset] = data;
}

void blitter_word_write(uint32 offset, uint16 data)
{
	blitter_byte_write(offset+0, (data>>8) & 0xFF);
	blitter_byte_write(offset+1, data & 0xFF);

	if ((offset & 0xFF) == 0x3A)
	{
		uint32 cmd = blitter_ram[0x38];
		cmd <<= 8;
		cmd |= blitter_ram[0x39];
		cmd <<= 8;
		cmd |= blitter_ram[0x3A];
		cmd <<= 8;
		cmd |= blitter_ram[0x3B];

		blitter_blit(cmd);
	}
}

uint8 blitter_byte_read(uint32 offset)
{
	offset &= 0xFF;

	// status register
	if (offset == (0x38+3))
		return 0x01;	// always idle

	return blitter_ram[offset];
}

uint16 blitter_word_read(uint32 offset)
{
	return (blitter_byte_read(offset) << 8) | blitter_byte_read(offset+1);
}
