//
// Jaguar memory and I/O physical (hosted!) memory
//
// by James L. Hammons
//
// JLH = James L. Hammons
//
// WHO  WHEN        WHAT
// ---  ----------  -----------------------------------------------------------
// JLH  12/10/2009  Repurposed this file. :-)
//

/*
$FFFFFF => 16,777,215
$A00000 => 10,485,760

Really, just six megabytes short of using the entire address space...
Why not? We could just allocate the entire space and then use the MMU code to do
things like call functions and whatnot...
In other words, read/write would just tuck the value into the host RAM space and
the I/O function would take care of any weird stuff...

Actually: writes would tuck in the value, but reads would have to be handled
correctly since some registers do not fall on the same address as far as reading
goes... Still completely doable though. :-)
*/

#include "memory.h"

uint8 jagMemSpace[0xF20000];					// The entire memory space of the Jaguar...!

uint8 * jaguarMainRAM = &jagMemSpace[0x000000];
uint8 * jaguarMainROM = &jagMemSpace[0x800000];
uint8 * cdRAM         = &jagMemSpace[0xDFFF00];
uint8 * gpuRAM        = &jagMemSpace[0xF03000];
uint8 * dspRAM        = &jagMemSpace[0xF1B000];

uint8 jaguarBootROM[0x040000];					// 68K CPU BIOS ROM--uses only half of this!
uint8 jaguarCDBootROM[0x040000];				// 68K CPU CD BIOS ROM (256K)


#if 0
union Word
{
	uint16 word;
	struct {
		// This changes depending on endianness...
#ifdef __BIG_ENDIAN__
		uint8 hi, lo;							// Big endian
#else
		uint8 lo, hi;							// Little endian
#endif
	};
};
#endif

#if 0
union DWord
{
	uint32 dword;
	struct
	{
#ifdef __BIG_ENDIAN__
		uint16 hiw, low;
#else
		uint16 low, hiw;
#endif
	};
};
#endif

#if 0
static void test(void)
{
	Word reg;
	reg.word = 0x1234;
	reg.lo = 0xFF;
	reg.hi = 0xEE;

	DWord reg2;
	reg2.hiw = 0xFFFE;
	reg2.low = 0x3322;
	reg2.low.lo = 0x11;
}
#endif

// OR, we could do like so:
#if 0
#ifdef __BIG_ENDIAN__
#define DWORD_BYTE_HWORD_H 1
#define DWORD_BYTE_HWORD_L 2
#define DWORD_BYTE_LWORD_H 3
#define DWORD_BYTE_LWORD_L 4
#else
#define DWORD_BYTE_HWORD_H 4
#define DWORD_BYTE_HWORD_L 3
#define DWORD_BYTE_LWORD_H 2
#define DWORD_BYTE_LWORD_L 1
#endif
// But this starts to get cumbersome after a while... Is union really better?

//More union stuff...
unsigned long ByteSwap1 (unsigned long nLongNumber)
{
   union u {unsigned long vi; unsigned char c[sizeof(unsigned long)];};
   union v {unsigned long ni; unsigned char d[sizeof(unsigned long)];};
   union u un;
   union v vn;
   un.vi = nLongNumber;
   vn.d[0]=un.c[3];
   vn.d[1]=un.c[2];
   vn.d[2]=un.c[1];
   vn.d[3]=un.c[0];
   return (vn.ni);
}
#endif

//Not sure if this is a good approach yet...
//should be if we use proper aliasing, and htonl and friends...
#if 1
uint32 & butch     = *((uint32 *)&jagMemSpace[0xDFFF00]);	// base of Butch == interrupt control register, R/W
uint32 & dscntrl   = *((uint32 *)&jagMemSpace[0xDFFF04]);	// DSA control register, R/W
uint16 & ds_data   = *((uint16 *)&jagMemSpace[0xDFFF0A]);	// DSA TX/RX data, R/W
uint32 & i2cntrl   = *((uint32 *)&jagMemSpace[0xDFFF10]);	// i2s bus control register, R/W
uint32 & sbcntrl   = *((uint32 *)&jagMemSpace[0xDFFF14]);	// CD subcode control register, R/W
uint32 & subdata   = *((uint32 *)&jagMemSpace[0xDFFF18]);	// Subcode data register A
uint32 & subdatb   = *((uint32 *)&jagMemSpace[0xDFFF1C]);	// Subcode data register B
uint32 & sb_time   = *((uint32 *)&jagMemSpace[0xDFFF20]);	// Subcode time and compare enable (D24)
uint32 & fifo_data = *((uint32 *)&jagMemSpace[0xDFFF24]);	// i2s FIFO data
uint32 & i2sdat2   = *((uint32 *)&jagMemSpace[0xDFFF28]);	// i2s FIFO data (old)
uint32 & unknown   = *((uint32 *)&jagMemSpace[0xDFFF2C]);	// Seems to be some sort of I2S interface
#else
uint32 butch, dscntrl, ds_data, i2cntrl, sbcntrl, subdata, subdatb, sb_time, fifo_data, i2sdat2, unknown;
#endif

#warning "Need to separate out this stuff (or do we???)"
//if we use a contiguous memory space, we don't need this shit...
//err, maybe we do, let's not be so hasty now... :-)

//#define ENDIANSAFE(x) htonl(x)

// The nice thing about doing it this way is that on big endian machines, htons/l
// compile to nothing and on Intel machines, it compiles down to a single bswap instruction.
// So endianness issues go away nicely without a lot of drama. :-D

#define BSWAP16(x) (htons(x))
#define BSWAP32(x) (htonl(x))
//this isn't endian safe...
#define BSWAP64(x) ((htonl(x & 0xFFFFFFFF) << 32) | htonl(x >> 32))
// Actually, we use ESAFExx() macros instead of this, and we use GCC to check the endianness...

#if 0
uint16 * memcon1_t = (uint16 *)&jagMemSpace[0xF00000];
uint16 & memcon1 = *memcon1_t;
#else
uint16 & memcon1 = *((uint16 *)&jagMemSpace[0xF00000]);
uint16 & memcon2 = *((uint16 *)&jagMemSpace[0xF00002]);
uint16 & hc      = *((uint16 *)&jagMemSpace[0xF00004]);
uint16 & vc      = *((uint16 *)&jagMemSpace[0xF00006]);
uint16 & lph     = *((uint16 *)&jagMemSpace[0xF00008]);
uint16 & lpv     = *((uint16 *)&jagMemSpace[0xF0000A]);
#endif

uint32 obData, olp, obf, vmode, bord1, bord2, hp, hbb, hbe, hs, hvs,
	hdb1, hdb2, hde, vp, vbb, vbe, vs, vdb, vde, veb, vee, vi, pit0, pit1, heq, bg, int1, int2, clut, lbuf,
	g_flags, g_mtxc, g_mtxa, g_end, g_pc, g_ctrl, g_hidata, g_remain, g_divctrl,
	a1_base, a1_pixel, a1_flags, a1_clip, a1_step, a1_fstep, a1_fpixel, a1_inc, a1_finc,
	a2_base, a2_flags, a2_pixel, a2_mask, a2_step, b_cmd, b_count, b_srcd, b_dstd, b_dstz, b_srcz1, b_srcz2,
	b_patd, b_iinc, b_zinc, b_stop, b_i3, b_i2, b_i1, b_i0, b_z3, b_z2, b_z1, b_z0,
	jpit1, jpit2, jpit3, jpit4, clk1, clk2, clk3, j_int, asidata, asistat, asictrl, asiclk, joystick,
	joybuts, d_flags, d_mtxc, d_mtxa, d_end, d_pc, d_ctrl, d_mod, d_remain, d_divctrl, d_machi,
	ltxd, rtxd, sstat, sclk, smode;

// Memory debugging identifiers

const char * whoName[9] =
	{ "Unknown", "Jaguar", "DSP", "GPU", "TOM", "JERRY", "M68K", "Blitter", "OP" };

