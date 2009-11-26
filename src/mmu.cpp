//
// mmu.cpp
//
// Jaguar Memory Manager Unit
//
// by James L. Hammons
//
// JLH = James L. Hammons
//
// WHO  WHEN        WHAT
// ---  ----------  -----------------------------------------------------------
// JLH  11/25/2009  Created this file. :-)
//

#include "mmu.h"

/*
Addresses to be handled:

SYSTEM SETUP REGISTERS

*MEMCON1	Memory Control Register 1			F00000		RW
*MEMCON2	Memory Control Register 2			F00002		RW
HC			Horizontal Count					F00004		RW
VC			Vertical Count						F00006		RW
LPH			Light Pen Horizontal				F00008		RO
LPV			Light Pen Vertical					F0000A		RO
OB[0-3]		Object Data Field					F00010-16	RO
OLP			Object List Pointer					F00020-23	WO
OBF			Object Flag							F00026		WO
VMODE		Video Mode							F00028		WO
BORD1		Border Colour (Red & Green)			F0002A		WO
BORD2		Border Colour (Blue)				F0002C		WO
*HP			Horizontal Period					F0002E		WO
*HBB		Horizontal Blank Begin				F00030		WO
*HBE		Horizontal Blank End				F00032		WO
*HS			Horizontal Sync						F00034		WO
*HVS		Horizontal Vertical Sync			F00036		WO
HDB1		Horizontal Display Begin 1			F00038		WO
HDB2		Horizontal Display Begin 2			F0003A		WO
HDE			Horizontal Display End				F0003C		WO
*VP			Vertical Period						F0003E		WO
*VBB		Vertical Blank Begin				F00040		WO
*VBE		Vertical Blank End					F00042		WO
*VS			Vertical Sync						F00044		WO
VDB			Vertical Display Begin				F00046		WO
VDE			Vertical Display End				F00048		WO
*VEB		Vertical Equalization Begin			F0004A		WO
*VEE		Vertical Equalization End			F0004C		WO
VI			Vertical Interrupt					F0004E		WO
PIT[0-1]	Programmable Interrupt Timer		F00050-52	WO
*HEQ		Horizontal Equalization End			F00054		WO
BG			Background Colour					F00058		WO
INT1		CPU Interrupt Control Register		F000E0		RW
INT2		CPU Interrupt Resume Register		F000E2		WO
CLUT		Colour Look-Up Table				F00400-7FE	RW
LBUF		Line Buffer							F00800-1D9E	RW

GPU REGISTERS

G_FLAGS		GPU Flags Register					F02100		RW
G_MTXC		Matrix Control Register				F02104		WO
G_MTXA		Matrix Address Register				F02108		WO
G_END		Data Organization Register			F0210C		WO
G_PC		GPU Program Counter					F02110		RW
G_CTRL		GPU Control/Status Register			F02114		RW
G_HIDATA	High Data Register					F02118		RW
G_REMAIN	Divide Unit Remainder				F0211C		RO
G_DIVCTRL	Divide Unit Control					F0211C		WO

BLITTER REGISTERS

A1_BASE		A1 Base Register					F02200		WO
A1_FLAGS	Flags Register						F02204		WO
A1_CLIP		A1 Clipping Size					F02208		WO
A1_PIXEL	A1 Pixel Pointer					F0220C		WO
												F02204		RO
A1_STEP		A1 Step Value						F02210		WO
A1_FSTEP	A1 Step Fraction Value				F02214		WO
A1_FPIXEL	A1 Pixel Pointer Fraction			F02218		RW
A1_INC		A1 Increment						F0221C		WO
A1_FINC		A1 Increment Fraction				F02220		WO
A2_BASE		A2 Base Register					F02224		WO
A2_FLAGS	A2 Flags Register					F02228		WO
A2_MASK		A2 Window Mask						F0222C		WO
A2_PIXEL	A2 Pixel Pointer					F02230		WO
												F0222C		RO
A2_STEP		A2 Step Value						F02234		WO
B_CMD		Command/Status Register				F02238		RW
B_COUNT		Counters Register					F0223C		WO
B_SRCD		Source Data Register				F02240		WO
B_DSTD		Destination Data Register			F02248		WO
B_DSTZ		Destination Z Register				F02250		WO
B_SRCZ1		Source Z Register 1					F02258		WO
B_SRCZ2		Source Z Register 2					F02260		WO
B_PATD		Pattern Data Register				F02268		WO
B_IINC		Intensity Increment					F02270		WO
B_ZINC		Z Increment							F02274		WO
B_STOP		Collision Control					F02278		WO
B_I3		Intensity 3							F0227C		WO
B_I2		Intensity 2							F02280		WO
B_I1		Intensity 1							F02284		WO
B_I0		Intensity 0							F02288		WO
B_Z3		Z 3									F0228C		WO
B_Z2		Z 2									F02290		WO
B_Z1		Z 1									F02294		WO
B_Z0		Z 0									F02298		WO

JERRY REGISTERS

*CLK1		Processor Clock Divider				F10010		WO
*CLK2		Video Clock Divider					F10012		WO
*CLK3		Chroma Clock Divider				F10014		WO
JPIT1		Timer 1 Pre-scaler					F10000		WO
JPIT3		Timer 2 Pre-scaler					F10004		WO
JPIT2		Timer 1 Divider						F10002		WO
JPIT4		Timer 2 Divider						F10006		WO
J_INT		Interrup Control Register			F10020		RW
SCLK		Serial Clock Frequency				F1A150		WO
SMODE		Serial Mode							F1A154		WO
LTXD		Left Transmit Data					F1A148		WO
RTXD		Right Transmit Data					F1A14C		WO
LRXD		Left Receive Data					F1A148		RO
RRXD		Right Receive Data					F1A14C		RO
L_I2S		Left I2S Serial Interface			F1A148		RW
R_I2S		Right I2S Serial Interface			F1A14C		RW
SSTAT		Serial Status						F1A150		RO
ASICLK		Asynchronous Serial Interface Clock	F10034		RW
ASICTRL		Asynchronous Serial Control			F10032		WO
ASISTAT		Asynchronous Serial Status			F10032		RO
ASIDATA		Asynchronous Serial Data			F10030		RW

JOYSTICK REGISTERS

JOYSTICK	Joystick Register					F14000		RW
JOYBUTS		Button Register						F14002		RW

DSP REGISTERS

D_FLAGS		DSP Flags Register					F1A100		RW
D_MTXC		DSP Matrix Control Register			F1A104		WO
D_MTXA		DSP Matrix Address Register			F1A108		WO
D_END		DSP Data Organization Register		F1A10C		WO
D_PC		DSP Program Counter					F1A110		RW
D_CTRL		DSP Control/Status Register			F1A114		RW
D_MOD		Modulo Instruction Mask				F1A118		WO
D_REMAIN	Divide Unit Remainder				F1A11C		RO
D_DIVCTRL	Divide Unit Control					F1A11C		WO
D_MACHI		MAC High Result Bits				F1A120		RO
*/

/*
The approach here is to have a list of addresses and who handles them. Could be
a one-to-one memory location up to a range for each function. Will look
something like this:

	{ 0xF14000, 0xF14001, MM_IO, JoystickReadHanlder, JoystickWriteHandler },

Would be nice to have a way of either calling a handler function or reading/writing
directly to/from a variable or array...
*/

enum MemType { MM_NOP = 0, MM_RAM, MM_ROM, MM_IO };

#if 0
// Jaguar Memory map/handlers
uint32 memoryMap[] = {
	{ 0x000000, 0x3FFFFF, MM_RAM, jaguarMainRAM },
	{ 0x800000, 0xDFFEFF, MM_ROM, jaguarMainROM },
// Note that this is really memory mapped I/O region...
//	{ 0xDFFF00, 0xDFFFFF, MM_RAM, cdRAM },
	{ 0xDFFF00, 0xDFFF03, MM_IO,  cdBUTCH }, // base of Butch == interrupt control register, R/W
	{ 0xDFFF04, 0xDFFF07, MM_IO,  cdDSCNTRL }, // DSA control register, R/W
	{ 0xDFFF0A, 0xDFFF0B, MM_IO,  cdDS_DATA }, // DSA TX/RX data, R/W
	{ 0xDFFF10, 0xDFFF13, MM_IO,  cdI2CNTRL }, // i2s bus control register, R/W
	{ 0xDFFF14, 0xDFFF17, MM_IO,  cdSBCNTRL }, // CD subcode control register, R/W
	{ 0xDFFF18, 0xDFFF1B, MM_IO,  cdSUBDATA }, // Subcode data register A
	{ 0xDFFF1C, 0xDFFF1F, MM_IO,  cdSUBDATB }, // Subcode data register B
	{ 0xDFFF20, 0xDFFF23, MM_IO,  cdSB_TIME }, // Subcode time and compare enable (D24)
	{ 0xDFFF24, 0xDFFF27, MM_IO,  cdFIFO_DATA }, // i2s FIFO data
	{ 0xDFFF28, 0xDFFF2B, MM_IO,  cdI2SDAT2 }, // i2s FIFO data (old)
	{ 0xDFFF2C, 0xDFFF2F, MM_IO,  cdUNKNOWN }, // Seems to be some sort of I2S interface

	{ 0xE00000, 0xE3FFFF, MM_ROM, jaguarBootROM },

//	{ 0xF00000, 0xF0FFFF, MM_IO,  TOM_REGS_RW },
	{ 0xF00050, 0xF00051, MM_IO,  tomTimerPrescaler },
	{ 0xF00052, 0xF00053, MM_IO,  tomTimerDivider },
	{ 0xF00400, 0xF005FF, MM_RAM, tomRAM }, // CLUT A&B: How to link these? Write to one writes to the other...
	{ 0xF00600, 0xF007FF, MM_RAM, tomRAM }, // Actually, this is a good approach--just make the reads the same as well
	//What about LBUF writes???
	{ 0xF02100, 0xF0211F, MM_IO,  GPUWriteByte }, // GPU CONTROL
	{ 0xF02200, 0xF0229F, MM_IO,  BlitterWriteByte }, // BLITTER
	{ 0xF03000, 0xF03FFF, MM_RAM, GPUWriteByte }, // GPU RAM

	{ 0xF10000, 0xF1FFFF, MM_IO,  JERRY_REGS_RW },

/*
	EEPROM:
	{ 0xF14001, 0xF14001, MM_IO_RO, eepromFOO }
	{ 0xF14801, 0xF14801, MM_IO_WO, eepromBAR }
	{ 0xF15001, 0xF15001, MM_IO_RW, eepromBAZ }

	JOYSTICK:
	{ 0xF14000, 0xF14003, MM_IO,  joystickFoo }
	0 = pad0/1 button values (4 bits each), RO(?)
	1 = pad0/1 index value (4 bits each), WO
	2 = unused, RO
	3 = NTSC/PAL, certain button states, RO

JOYSTICK    $F14000               Read/Write
            15.....8  7......0
Read        fedcba98  7654321q    f-1    Signals J15 to J1
                                  q      Cartridge EEPROM  output data
Write       exxxxxxm  76543210    e      1 = enable  J7-J0 outputs
                                         0 = disable J7-J0 outputs
                                  x      don't care
                                  m      Audio mute
                                         0 = Audio muted (reset state)
                                         1 = Audio enabled
                                  7-4    J7-J4 outputs (port 2)
                                  3-0    J3-J0 outputs (port 1)
JOYBUTS     $F14002               Read Only
            15.....8  7......0
Read        xxxxxxxx  rrdv3210    x      don't care
                                  r      Reserved
                                  d      Reserved
                                  v      1 = NTSC Video hardware
                                         0 = PAL  Video hardware
                                  3-2    Button inputs B3 & B2 (port 2)
                                  1-0    Button inputs B1 & B0 (port 1)

J4 J5 J6 J7  Port 2    B2     B3    J12  J13   J14  J15
J3 J2 J1 J0  Port 1    B0     B1    J8   J9    J10  J11
 0  0  0  0
 0  0  0  1
 0  0  1  0
 0  0  1  1
 0  1  0  0
 0  1  0  1
 0  1  1  0
 0  1  1  1  Row 3     C3   Option  #     9     6     3
 1  0  0  0
 1  0  0  1
 1  0  1  0
 1  0  1  1  Row 2     C2      C    0     8     5     2
 1  1  0  0
 1  1  0  1  Row 1     C1      B    *     7     4     1
 1  1  1  0  Row 0   Pause     A    Up  Down  Left  Right
 1  1  1  1

0 bit read in any position means that button is pressed.
C3 = C2 = 1 means std. Jag. cntrlr. or nothing attached.
*/
};
#endif

void MMUWrite8(uint32 address, uint8 data, uint32 who/*= UNKNOWN*/)
{
}

void MMUWrite16(uint32 address, uint16 data, uint32 who/*= UNKNOWN*/)
{
}

void MMUWrite32(uint32 address, uint32 data, uint32 who/*= UNKNOWN*/)
{
}

void MMUWrite64(uint32 address, uint64 data, uint32 who/*= UNKNOWN*/)
{
}

uint8 MMURead8(uint32 address, uint32 who/*= UNKNOWN*/)
{
	return 0;
}

uint16 MMURead16(uint32 address, uint32 who/*= UNKNOWN*/)
{
	return 0;
}

uint32 MMURead32(uint32 address, uint32 who/*= UNKNOWN*/)
{
	return 0;
}

uint64 MMURead64(uint32 address, uint32 who/*= UNKNOWN*/)
{
	return 0;
}

