//
// DSP core
//
// by cal2
// GCC/SDL port by Niels Wagenaar (Linux/WIN32) and Caz (BeOS)
// Extensive cleanups/rewrites by James L. Hammons
//

#include <SDL.h>	// Used only for SDL_GetTicks...
#include "dsp.h"

#define DSP_DEBUG
//#define DSP_DEBUG_IRQ

// Disassembly definitions

#define DSP_DIS_ADD
#define DSP_DIS_ADDC
#define DSP_DIS_ADDQ
#define DSP_DIS_ADDQMOD
#define DSP_DIS_ADDQT
#define DSP_DIS_AND
#define DSP_DIS_BCLR
#define DSP_DIS_BSET
#define DSP_DIS_BTST
#define DSP_DIS_CMP
#define DSP_DIS_CMPQ
#define DSP_DIS_IMACN
#define DSP_DIS_IMULT
#define DSP_DIS_IMULTN
#define DSP_DIS_JR
#define DSP_DIS_JUMP
#define DSP_DIS_LOAD
#define DSP_DIS_LOAD14I
#define DSP_DIS_LOAD14R
#define DSP_DIS_LOAD15I
#define DSP_DIS_LOAD15R
#define DSP_DIS_LOADB
#define DSP_DIS_LOADW
#define DSP_DIS_MOVE
#define DSP_DIS_MOVEI
#define DSP_DIS_MOVEQ
#define DSP_DIS_MOVEFA
#define DSP_DIS_MOVETA
#define DSP_DIS_MULT
#define DSP_DIS_NEG
#define DSP_DIS_NOP
#define DSP_DIS_NOT
#define DSP_DIS_OR
#define DSP_DIS_RESMAC
#define DSP_DIS_ROR
#define DSP_DIS_RORQ
#define DSP_DIS_SHARQ
#define DSP_DIS_SHLQ
#define DSP_DIS_SHRQ
#define DSP_DIS_STORE
#define DSP_DIS_STORE14I
#define DSP_DIS_STORE15I
#define DSP_DIS_STOREB
#define DSP_DIS_STOREW
#define DSP_DIS_SUB
#define DSP_DIS_SUBC
#define DSP_DIS_SUBQ
#define DSP_DIS_SUBQT
#define DSP_DIS_XOR

bool doDSPDis = false;
//*/

/*
No dis yet:
+	subqt 4560
+	mult 1472
+	imultn 395024
+	resmac 395024
+	imacn 395024
+	addqmod 93328

dsp opcodes use:
+	add 1672497
+	addq 4366576
+	addqt 44405640
+	sub 94833
+	subq 111769
+	and 47416
+	btst 94521
+	bset 2277826
+	bclr 3223372
+	mult 47104
+	imult 237080
+	shlq 365464
+	shrq 141624
+	sharq 318368
+	cmp 45175078
+	move 2238994
+	moveq 335305
+	moveta 19
+	movefa 47406440
+	movei 1920664
+	loadb 94832
+	load 4031281
+	load_r15_indexed 284500
+	store 2161732
+	store_r15_indexed 47416
+	jump 3872424
+	jr 46386967
+	nop 3300029
+	load_r14_ri 1229448
*/

// DSP flags (old--have to get rid of this crap)

#define CINT0FLAG			0x00200
#define CINT1FLAG			0x00400
#define CINT2FLAG			0x00800
#define CINT3FLAG			0x01000
#define CINT4FLAG			0x02000
#define CINT04FLAGS			(CINT0FLAG | CINT1FLAG | CINT2FLAG | CINT3FLAG | CINT4FLAG)
#define CINT5FLAG			0x20000		/* DSP only */

// DSP_FLAGS bits

#define ZERO_FLAG		0x00001
#define CARRY_FLAG		0x00002
#define NEGA_FLAG		0x00004
#define IMASK			0x00008
#define INT_ENA0		0x00010
#define INT_ENA1		0x00020
#define INT_ENA2		0x00040
#define INT_ENA3		0x00080
#define INT_ENA4		0x00100
#define INT_CLR0		0x00200
#define INT_CLR1		0x00400
#define INT_CLR2		0x00800
#define INT_CLR3		0x01000
#define INT_CLR4		0x02000
#define REGPAGE			0x04000
#define DMAEN			0x08000
#define INT_ENA5		0x10000
#define INT_CLR5		0x20000

// DSP_CTRL bits

#define DSPGO			0x00001
#define CPUINT			0x00002
#define DSPINT0			0x00004
#define SINGLE_STEP		0x00008
#define SINGLE_GO		0x00010
// Bit 5 is unused!
#define INT_LAT0		0x00040
#define INT_LAT1		0x00080
#define INT_LAT2		0x00100
#define INT_LAT3		0x00200
#define INT_LAT4		0x00400
#define BUS_HOG			0x00800
#define VERSION			0x0F000
#define INT_LAT5		0x10000

extern uint32 jaguar_mainRom_crc32;

// Is opcode 62 *really* a NOP? Seems like it...
static void dsp_opcode_abs(void);
static void dsp_opcode_add(void);
static void dsp_opcode_addc(void);
static void dsp_opcode_addq(void);
static void dsp_opcode_addqmod(void);	
static void dsp_opcode_addqt(void);
static void dsp_opcode_and(void);
static void dsp_opcode_bclr(void);
static void dsp_opcode_bset(void);
static void dsp_opcode_btst(void);
static void dsp_opcode_cmp(void);
static void dsp_opcode_cmpq(void);
static void dsp_opcode_div(void);
static void dsp_opcode_imacn(void);
static void dsp_opcode_imult(void);
static void dsp_opcode_imultn(void);
static void dsp_opcode_jr(void);
static void dsp_opcode_jump(void);
static void dsp_opcode_load(void);
static void dsp_opcode_loadb(void);
static void dsp_opcode_loadw(void);
static void dsp_opcode_load_r14_indexed(void);
static void dsp_opcode_load_r14_ri(void);
static void dsp_opcode_load_r15_indexed(void);
static void dsp_opcode_load_r15_ri(void);
static void dsp_opcode_mirror(void);	
static void dsp_opcode_mmult(void);
static void dsp_opcode_move(void);
static void dsp_opcode_movei(void);
static void dsp_opcode_movefa(void);
static void dsp_opcode_move_pc(void);
static void dsp_opcode_moveq(void);
static void dsp_opcode_moveta(void);
static void dsp_opcode_mtoi(void);
static void dsp_opcode_mult(void);
static void dsp_opcode_neg(void);
static void dsp_opcode_nop(void);
static void dsp_opcode_normi(void);
static void dsp_opcode_not(void);
static void dsp_opcode_or(void);
static void dsp_opcode_resmac(void);
static void dsp_opcode_ror(void);
static void dsp_opcode_rorq(void);
static void dsp_opcode_xor(void);
static void dsp_opcode_sat16s(void);	
static void dsp_opcode_sat32s(void);	
static void dsp_opcode_sh(void);
static void dsp_opcode_sha(void);
static void dsp_opcode_sharq(void);
static void dsp_opcode_shlq(void);
static void dsp_opcode_shrq(void);
static void dsp_opcode_store(void);
static void dsp_opcode_storeb(void);
static void dsp_opcode_storew(void);
static void dsp_opcode_store_r14_indexed(void);
static void dsp_opcode_store_r14_ri(void);
static void dsp_opcode_store_r15_indexed(void);
static void dsp_opcode_store_r15_ri(void);
static void dsp_opcode_sub(void);
static void dsp_opcode_subc(void);
static void dsp_opcode_subq(void);
static void dsp_opcode_subqmod(void);	
static void dsp_opcode_subqt(void);

uint8 dsp_opcode_cycles[64] =
{
	3,  3,  3,  3,  
	3,  3,  3,  3,  
	3,  3,  3,  3,  
	3,  3,  3,  3,
	3,  3,  1,  3,  
	1, 18,  3,  3,  
	3,  3,  3,  3,  
	3,  3,  3,  3,
	3,  3,  2,  2,  
	2,  2,  3,  4,  
	5,  4,  5,  6,  
	6,  1,  1,  1,
	1,  2,  2,  2,  
	1,  1,  9,  3,  
	3,  1,  6,  6,  
	2,  2,  3,  3
};

void (* dsp_opcode[64])() =
{	
	dsp_opcode_add,					dsp_opcode_addc,				dsp_opcode_addq,				dsp_opcode_addqt,
	dsp_opcode_sub,					dsp_opcode_subc,				dsp_opcode_subq,				dsp_opcode_subqt,
	dsp_opcode_neg,					dsp_opcode_and,					dsp_opcode_or,					dsp_opcode_xor,
	dsp_opcode_not,					dsp_opcode_btst,				dsp_opcode_bset,				dsp_opcode_bclr,
	dsp_opcode_mult,				dsp_opcode_imult,				dsp_opcode_imultn,				dsp_opcode_resmac,
	dsp_opcode_imacn,				dsp_opcode_div,					dsp_opcode_abs,					dsp_opcode_sh,
	dsp_opcode_shlq,				dsp_opcode_shrq,				dsp_opcode_sha,					dsp_opcode_sharq,
	dsp_opcode_ror,					dsp_opcode_rorq,				dsp_opcode_cmp,					dsp_opcode_cmpq,
	dsp_opcode_subqmod,				dsp_opcode_sat16s,				dsp_opcode_move,				dsp_opcode_moveq,
	dsp_opcode_moveta,				dsp_opcode_movefa,				dsp_opcode_movei,				dsp_opcode_loadb,
	dsp_opcode_loadw,				dsp_opcode_load,				dsp_opcode_sat32s,				dsp_opcode_load_r14_indexed,
	dsp_opcode_load_r15_indexed,	dsp_opcode_storeb,				dsp_opcode_storew,				dsp_opcode_store,
	dsp_opcode_mirror,				dsp_opcode_store_r14_indexed,	dsp_opcode_store_r15_indexed,	dsp_opcode_move_pc,
	dsp_opcode_jump,				dsp_opcode_jr,					dsp_opcode_mmult,				dsp_opcode_mtoi,
	dsp_opcode_normi,				dsp_opcode_nop,					dsp_opcode_load_r14_ri,			dsp_opcode_load_r15_ri,
	dsp_opcode_store_r14_ri,		dsp_opcode_store_r15_ri,		dsp_opcode_nop,					dsp_opcode_addqmod,
};

uint32 dsp_opcode_use[64];

char * dsp_opcode_str[64]=
{	
	"add",				"addc",				"addq",				"addqt",
	"sub",				"subc",				"subq",				"subqt",
	"neg",				"and",				"or",				"xor",
	"not",				"btst",				"bset",				"bclr",
	"mult",				"imult",			"imultn",			"resmac",
	"imacn",			"div",				"abs",				"sh",
	"shlq",				"shrq",				"sha",				"sharq",
	"ror",				"rorq",				"cmp",				"cmpq",
	"subqmod",			"sat16s",			"move",				"moveq",
	"moveta",			"movefa",			"movei",			"loadb",
	"loadw",			"load",				"sat32s",			"load_r14_indexed",
	"load_r15_indexed",	"storeb",			"storew",			"store",
	"mirror",			"store_r14_indexed","store_r15_indexed","move_pc",
	"jump",				"jr",				"mmult",			"mtoi",
	"normi",			"nop",				"load_r14_ri",		"load_r15_ri",
	"store_r14_ri",		"store_r15_ri",		"nop",				"addqmod",
};

uint32 dsp_pc;
static uint64 dsp_acc;								// 40 bit register, NOT 32!
static uint32 dsp_remain;
static uint32 dsp_modulo;
static uint32 dsp_flags;
static uint32 dsp_matrix_control;
static uint32 dsp_pointer_to_matrix;
static uint32 dsp_data_organization;
uint32 dsp_control;
static uint32 dsp_div_control;
static uint8 dsp_flag_z, dsp_flag_n, dsp_flag_c;    
static uint32 * dsp_reg, * dsp_alternate_reg;
static uint32 * dsp_reg_bank_0, * dsp_reg_bank_1;

static uint32 dsp_opcode_first_parameter;
static uint32 dsp_opcode_second_parameter;

#define DSP_RUNNING			(dsp_control & 0x01)

#define RM					dsp_reg[dsp_opcode_first_parameter]
#define RN					dsp_reg[dsp_opcode_second_parameter]
#define ALTERNATE_RM		dsp_alternate_reg[dsp_opcode_first_parameter]
#define ALTERNATE_RN		dsp_alternate_reg[dsp_opcode_second_parameter]
#define IMM_1				dsp_opcode_first_parameter
#define IMM_2				dsp_opcode_second_parameter

#define CLR_Z				(dsp_flag_z = 0)
#define CLR_ZN				(dsp_flag_z = dsp_flag_n = 0)
#define CLR_ZNC				(dsp_flag_z = dsp_flag_n = dsp_flag_c = 0)
#define SET_Z(r)			(dsp_flag_z = ((r) == 0))
#define SET_N(r)			(dsp_flag_n = (((UINT32)(r) >> 31) & 0x01))
#define SET_C_ADD(a,b)		(dsp_flag_c = ((UINT32)(b) > (UINT32)(~(a))))
#define SET_C_SUB(a,b)		(dsp_flag_c = ((UINT32)(b) > (UINT32)(a)))
#define SET_ZN(r)			SET_N(r); SET_Z(r)
#define SET_ZNC_ADD(a,b,r)	SET_N(r); SET_Z(r); SET_C_ADD(a,b)
#define SET_ZNC_SUB(a,b,r)	SET_N(r); SET_Z(r); SET_C_SUB(a,b)

uint32 dsp_convert_zero[32] = { 32,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31 };
uint8 * dsp_branch_condition_table = NULL;
static uint16 * mirror_table = NULL;
static uint8 * dsp_ram_8 = NULL;

#define BRANCH_CONDITION(x)		dsp_branch_condition_table[(x) + ((jaguar_flags & 7) << 5)]

static uint32 dsp_in_exec = 0;
static uint32 dsp_releaseTimeSlice_flag = 0;

FILE * dsp_fp;

// Private function prototypes

void DSPDumpRegisters(void);
void DSPDumpDisassembly(void);


void dsp_reset_stats(void)
{
	for(int i=0; i<64; i++)
		dsp_opcode_use[i] = 0;
}

void dsp_releaseTimeslice(void)
{
//This does absolutely nothing!!! !!! FIX !!!
	dsp_releaseTimeSlice_flag = 1;
}

void dsp_build_branch_condition_table(void)
{
	// Allocate the mirror table
	if (!mirror_table)
		mirror_table = (uint16 *)malloc(65536 * sizeof(mirror_table[0]));

	// Fill in the mirror table
	if (mirror_table)
		for(int i=0; i<65536; i++)
			mirror_table[i] = ((i >> 15) & 0x0001) | ((i >> 13) & 0x0002) |
			                  ((i >> 11) & 0x0004) | ((i >> 9)  & 0x0008) |
			                  ((i >> 7)  & 0x0010) | ((i >> 5)  & 0x0020) |
			                  ((i >> 3)  & 0x0040) | ((i >> 1)  & 0x0080) |
			                  ((i << 1)  & 0x0100) | ((i << 3)  & 0x0200) |
			                  ((i << 5)  & 0x0400) | ((i << 7)  & 0x0800) |
			                  ((i << 9)  & 0x1000) | ((i << 11) & 0x2000) |
			                  ((i << 13) & 0x4000) | ((i << 15) & 0x8000);

	if (!dsp_branch_condition_table)
	{
		dsp_branch_condition_table = (uint8 *)malloc(32 * 8 * sizeof(dsp_branch_condition_table[0]));

		// Fill in the condition table
		if (dsp_branch_condition_table)
		{
			for(int i=0; i<8; i++)
			{
				for(int j=0; j<32; j++)
				{
					int result = 1;
					if (j & 1)
						if (i & ZERO_FLAG)
							result = 0;
					if (j & 2)
						if (!(i & ZERO_FLAG))
							result = 0;
					if (j & 4)
						if (i & (CARRY_FLAG << (j >> 4)))
							result = 0;
					if (j & 8)
						if (!(i & (CARRY_FLAG << (j >> 4))))
							result = 0;
					dsp_branch_condition_table[i * 32 + j] = result;
				}
			}
		}
	}
}

uint8 DSPReadByte(uint32 offset, uint32 who/*=UNKNOWN*/)
{
	if (offset >= 0xF1A000 && offset <= 0xF1A0FF)
		WriteLog("DSP: ReadByte--Attempt to read from DSP register file by %s!\n", whoName[who]);
// battlemorph
//	if ((offset==0xF1CFE0)||(offset==0xF1CFE2))
//		return(0xffff);
	// mutant penguin
/*	if ((jaguar_mainRom_crc32==0xbfd751a4)||(jaguar_mainRom_crc32==0x053efaf9))
	{
		if (offset==0xF1CFE0)
			return(0xff);
	}*/
	if (offset >= DSP_WORK_RAM_BASE && offset <= (DSP_WORK_RAM_BASE + 0x1FFF))
		return dsp_ram_8[offset - DSP_WORK_RAM_BASE];

	if (offset >= DSP_CONTROL_RAM_BASE && offset <= (DSP_CONTROL_RAM_BASE + 0x1F))
	{
		uint32 data = DSPReadLong(offset & 0xFFFFFFFC, who);

		if ((offset&0x03)==0)
			return(data>>24);
		else
		if ((offset&0x03)==1)
			return((data>>16)&0xff);
		else
		if ((offset&0x03)==2)
			return((data>>8)&0xff);
		else
		if ((offset&0x03)==3)
			return(data&0xff);
	}

	return JaguarReadByte(offset, who);
} 

uint16 DSPReadWord(uint32 offset, uint32 who/*=UNKNOWN*/)
{
	if (offset >= 0xF1A000 && offset <= 0xF1A0FF)
		WriteLog("DSP: ReadWord--Attempt to read from DSP register file by %s!\n", whoName[who]);
	//???
	offset &= 0xFFFFFFFE;
	// jaguar cd bios
/*	if (jaguar_mainRom_crc32==0xa74a97cd)
	{
		if (offset==0xF1A114) return(0x0000);
		if (offset==0xF1A116) return(0x0000);
		if (offset==0xF1B000) return(0x1234);
		if (offset==0xF1B002) return(0x5678);
	}*/
/*
	if (jaguar_mainRom_crc32==0x7ae20823)
	{
		if (offset==0xF1B9D8) return(0x0000);
		if (offset==0xF1B9Da) return(0x0000);
		if (offset==0xF1B2C0) return(0x0000);
		if (offset==0xF1B2C2) return(0x0000);
	}
*/
	// pour permettre à wolfenstein 3d de tourner sans le dsp
/*	if ((offset==0xF1B0D0)||(offset==0xF1B0D2))
		return(0);
*/

		// pour permettre à nba jam de tourner sans le dsp
/*	if (jaguar_mainRom_crc32==0x4faddb18)
	{
		if (offset==0xf1b2c0) return(0);
		if (offset==0xf1b2c2) return(0);
		if (offset==0xf1b240) return(0);
		if (offset==0xf1b242) return(0);
		if (offset==0xF1B340) return(0);
		if (offset==0xF1B342) return(0);
		if (offset==0xF1BAD8) return(0);
		if (offset==0xF1BADA) return(0);
		if (offset==0xF1B040) return(0);
		if (offset==0xF1B042) return(0);
		if (offset==0xF1B0C0) return(0);
		if (offset==0xF1B0C2) return(0);
		if (offset==0xF1B140) return(0);
		if (offset==0xF1B142) return(0);
		if (offset==0xF1B1C0) return(0);
		if (offset==0xF1B1C2) return(0);
	}*/

	if (offset >= DSP_WORK_RAM_BASE && offset <= DSP_WORK_RAM_BASE+0x1FFF)
	{
		offset -= DSP_WORK_RAM_BASE;
/*		uint16 data = (((uint16)dsp_ram_8[offset])<<8)|((uint16)dsp_ram_8[offset+1]);
		return data;*/
		return GET16(dsp_ram_8, offset);
	}
	else if ((offset>=DSP_CONTROL_RAM_BASE)&&(offset<DSP_CONTROL_RAM_BASE+0x20))
	{
		uint32 data = DSPReadLong(offset & 0xFFFFFFFC, who);

		if (offset & 0x03)
			return data & 0xFFFF;
		else
			return data >> 16;
	}

	return JaguarReadWord(offset, who);
}

uint32 DSPReadLong(uint32 offset, uint32 who/*=UNKNOWN*/)
{
	if (offset >= 0xF1A000 && offset <= 0xF1A0FF)
		WriteLog("DSP: ReadLong--Attempt to read from DSP register file by %s!\n", whoName[who]);

	// ??? WHY ???
	offset &= 0xFFFFFFFC;
/*if (offset == 0xF1BCF4)
{
	WriteLog("DSPReadLong: Reading from 0xF1BCF4... -> %08X [%02X %02X %02X %02X][%04X %04X]\n", GET32(dsp_ram_8, 0x0CF4), dsp_ram_8[0x0CF4], dsp_ram_8[0x0CF5], dsp_ram_8[0x0CF6], dsp_ram_8[0x0CF7], JaguarReadWord(0xF1BCF4, DSP), JaguarReadWord(0xF1BCF6, DSP));
	DSPDumpDisassembly();
}*/
	if (offset >= DSP_WORK_RAM_BASE && offset <= DSP_WORK_RAM_BASE + 0x1FFF)
	{
		offset -= DSP_WORK_RAM_BASE;
		return GET32(dsp_ram_8, offset);
	}
//NOTE: Didn't return DSP_ACCUM!!!
//Mebbe it's not 'spose to! Yes, it is!
	if (offset >= DSP_CONTROL_RAM_BASE && offset <= DSP_CONTROL_RAM_BASE + 0x23)
	{
		offset &= 0x3F;
		switch (offset)
		{
		case 0x00:	/*dsp_flag_c?(dsp_flag_c=1):(dsp_flag_c=0);
					dsp_flag_z?(dsp_flag_z=1):(dsp_flag_z=0);
					dsp_flag_n?(dsp_flag_n=1):(dsp_flag_n=0);*/

					dsp_flags = (dsp_flags & 0xFFFFFFF8) | (dsp_flag_n << 2) | (dsp_flag_c << 1) | dsp_flag_z;
					return dsp_flags & 0xFFFFC1FF;
		case 0x04: return dsp_matrix_control;
		case 0x08: return dsp_pointer_to_matrix;
		case 0x0C: return dsp_data_organization;
		case 0x10: return dsp_pc;
		case 0x14: return dsp_control;
		case 0x18: return dsp_modulo;
		case 0x1C: return dsp_remain;
		case 0x20:
			return (int32)((int8)(dsp_acc >> 32));	// Top 8 bits of 40-bit accumulator, sign extended
		}
		// unaligned long read-- !!! FIX !!!
		return 0xFFFFFFFF;
	}

	return JaguarReadLong(offset, who);
}

void DSPWriteByte(uint32 offset, uint8 data, uint32 who/*=UNKNOWN*/)
{
	if (offset >= 0xF1A000 && offset <= 0xF1A0FF)
		WriteLog("DSP: WriteByte--Attempt to write to DSP register file by %s!\n", whoName[who]);

	if ((offset >= DSP_WORK_RAM_BASE) && (offset < DSP_WORK_RAM_BASE+0x2000))
	{
		offset -= DSP_WORK_RAM_BASE;
		dsp_ram_8[offset] = data;
//This is rather stupid! !!! FIX !!!
/*		if (dsp_in_exec == 0)
		{
			m68k_end_timeslice();
			gpu_releaseTimeslice();
		}*/
		return;
	}
	if ((offset >= DSP_CONTROL_RAM_BASE) && (offset < DSP_CONTROL_RAM_BASE+0x20))
	{
		uint32 reg = offset & 0x1C;
		int bytenum = offset & 0x03;
		
		if ((reg >= 0x1C) && (reg <= 0x1F))
			dsp_div_control = (dsp_div_control & (~(0xFF << (bytenum << 3)))) | (data << (bytenum << 3));
		else
		{
//This looks funky. !!! FIX !!!
			uint32 old_data = DSPReadLong(offset&0xFFFFFFC, who);
			bytenum = 3 - bytenum; // convention motorola !!!
			old_data = (old_data & (~(0xFF << (bytenum << 3)))) | (data << (bytenum << 3));	
			DSPWriteLong(offset & 0xFFFFFFC, old_data, who);
		}
		return;
	}
//	WriteLog("dsp: writing %.2x at 0x%.8x\n",data,offset);
//Should this *ever* happen??? Shouldn't we be saying "unknown" here???
	JaguarWriteByte(offset, data, who);
}

void DSPWriteWord(uint32 offset, uint16 data, uint32 who/*=UNKNOWN*/)
{
	if (offset >= 0xF1A000 && offset <= 0xF1A0FF)
		WriteLog("DSP: WriteWord--Attempt to write to DSP register file by %s!\n", whoName[who]);
	offset &= 0xFFFFFFFE;
/*if (offset == 0xF1BCF4)
{
	WriteLog("DSPWriteWord: Writing to 0xF1BCF4... %04X -> %04X\n", GET16(dsp_ram_8, 0x0CF4), data);
}*/
//	WriteLog("dsp: writing %.4x at 0x%.8x\n",data,offset);
	if ((offset >= DSP_WORK_RAM_BASE) && (offset < DSP_WORK_RAM_BASE+0x2000))
	{
		offset -= DSP_WORK_RAM_BASE;
		dsp_ram_8[offset] = data >> 8;
		dsp_ram_8[offset+1] = data & 0xFF;
//This is rather stupid! !!! FIX !!!
/*		if (dsp_in_exec == 0)
		{
//			WriteLog("dsp: writing %.4x at 0x%.8x\n",data,offset+DSP_WORK_RAM_BASE);
			m68k_end_timeslice();
			gpu_releaseTimeslice();
		}*/
		return;
	}
	else if ((offset >= DSP_CONTROL_RAM_BASE) && (offset < DSP_CONTROL_RAM_BASE+0x20))
	{
		if ((offset & 0x1C) == 0x1C)
		{
			if (offset & 0x03)
				dsp_div_control = (dsp_div_control&0xffff0000)|(data&0xffff);
			else
				dsp_div_control = (dsp_div_control&0xffff)|((data&0xffff)<<16);
		}
		else
		{
			uint32 old_data = DSPReadLong(offset & 0xffffffc, who);
			if (offset & 0x03)
				old_data = (old_data&0xffff0000)|(data&0xffff);
			else
				old_data = (old_data&0xffff)|((data&0xffff)<<16);
			DSPWriteLong(offset & 0xffffffc, old_data, who);
		}
		return;
	}

	JaguarWriteWord(offset, data, who);
}

//bool badWrite = false;
void DSPWriteLong(uint32 offset, uint32 data, uint32 who/*=UNKNOWN*/)
{
	if (offset >= 0xF1A000 && offset <= 0xF1A0FF)
		WriteLog("DSP: WriteLong--Attempt to write to DSP register file by %s!\n", whoName[who]);
	// ??? WHY ???
	offset &= 0xFFFFFFFC;
/*if (offset == 0xF1BCF4)
{
	WriteLog("DSPWriteLong: Writing to 0xF1BCF4... %08X -> %08X\n", GET32(dsp_ram_8, 0x0CF4), data);
}*/
//	WriteLog("dsp: writing %.8x at 0x%.8x\n",data,offset);
	if (offset >= DSP_WORK_RAM_BASE && offset <= DSP_WORK_RAM_BASE + 0x1FFF)
	{
/*if (offset == 0xF1BE2C)
{
	WriteLog("DSP: %s is writing %08X at location 0xF1BE2C (DSP_PC: %08X)...\n", whoName[who], data, dsp_pc - 2);
}//*/
		offset -= DSP_WORK_RAM_BASE;
		SET32(dsp_ram_8, offset, data);
		return;
	}
	else if (offset >= DSP_CONTROL_RAM_BASE && offset <= (DSP_CONTROL_RAM_BASE + 0x1F))
	{
		offset &= 0x1F;
		switch (offset)
		{
		case 0x00:
		{
			bool IMASKCleared = (dsp_flags & IMASK) && !(data & IMASK);
			dsp_flags = data;
			dsp_flag_z = dsp_flags & 0x01;
			dsp_flag_c = (dsp_flags >> 1) & 0x01;
			dsp_flag_n = (dsp_flags >> 2) & 0x01;
			DSPUpdateRegisterBanks();
			dsp_control &= ~((dsp_flags & CINT04FLAGS) >> 3);
			dsp_control &= ~((dsp_flags & CINT5FLAG) >> 1);
			if (IMASKCleared)						// If IMASK was cleared,
#ifdef DSP_DEBUG_IRQ
			{
				WriteLog("DSP: Finished interrupt.\n");
#endif
				DSPHandleIRQs();					// see if any other interrupts need servicing!
#ifdef DSP_DEBUG_IRQ
			}
#endif
			break;
		}
		case 0x04:
			dsp_matrix_control = data;
			break;
		case 0x08:
			// According to JTRM, only lines 2-11 are adressable, the rest being
			// hardwired to $F1Bxxx.
			dsp_pointer_to_matrix = 0xF1B000 | (data & 0x000FFC);
			break;
		case 0x0C:
			dsp_data_organization = data;
			break;
		case 0x10:
			dsp_pc = data;
#ifdef DSP_DEBUG
			WriteLog("DSP: Setting DSP PC to %08X by %s%s\n", dsp_pc, whoName[who], (DSP_RUNNING ? " (DSP is RUNNING!)" : ""));//*/
#endif
			break;
		case 0x14:
		{	
//			uint32 dsp_was_running = DSP_RUNNING;
			// Check for DSP -> CPU interrupt
			if (data & CPUINT)
			{
//				WriteLog("DSP: DSP -> CPU interrupt\n");
// This was WRONG
// Why do we check for a valid handler at 64? Isn't that the Jag programmer's responsibility?
				if (JERRYIRQEnabled(IRQ2_DSP) && jaguar_interrupt_handler_is_valid(64))
				{
					JERRYSetPendingIRQ(IRQ2_DSP);
					dsp_releaseTimeslice();
					m68k_set_irq(7);			// Set 68000 NMI...
				}
				data &= ~CPUINT;
			}
			// Check for CPU -> DSP interrupt
			if (data & DSPINT0)
			{
				WriteLog("DSP: CPU -> DSP interrupt\n");
				m68k_end_timeslice();
				gpu_releaseTimeslice();
				DSPSetIRQLine(DSPIRQ_CPU, ASSERT_LINE);
				data &= ~DSPINT0;
			}
			// single stepping
			if (data & SINGLE_STEP)
			{
//				WriteLog("DSP: Asked to perform a single step (single step is %senabled)\n", (data & 0x8 ? "" : "not "));
			}

			// Protect writes to VERSION and the interrupt latches...
			uint32 mask = VERSION | INT_LAT0 | INT_LAT1 | INT_LAT2 | INT_LAT3 | INT_LAT4 | INT_LAT5;
			dsp_control = (dsp_control & mask) | (data & ~mask);

			// if dsp wasn't running but is now running
			// execute a few cycles
//This is just plain wrong, wrong, WRONG!
#ifndef DSP_SINGLE_STEPPING
/*			if (!dsp_was_running && DSP_RUNNING)
			{
				DSPExec(200);
			}*/
#else
//This is WRONG! !!! FIX !!!
			if (dsp_control & 0x18)
				DSPExec(1);
#endif
#ifdef DSP_DEBUG
WriteLog("Write to DSP CTRL: %08X ", data);
if (DSP_RUNNING)
	WriteLog(" --> Starting to run at %08X by %s...", dsp_pc, whoName[who]);
else
	WriteLog(" --> Stopped by %s! (DSP PC: %08X)", whoName[who], dsp_pc);
WriteLog("\n");
#endif	// DSP_DEBUG
//This isn't exactly right either--we don't know if it was the M68K or the GPU writing here...
// !!! FIX !!!
			if (DSP_RUNNING)
				m68k_end_timeslice();
			break;
		}
		case 0x18:
			dsp_modulo = data;
			break;
		case 0x1C:
			dsp_div_control = data;
			break;
//		default:   // unaligned long read
				   //__asm int 3
		}
		return;
	}

//We don't have to break this up like this! We CAN do 32 bit writes!
//	JaguarWriteWord(offset, (data>>16) & 0xFFFF, DSP);
//	JaguarWriteWord(offset+2, data & 0xFFFF, DSP);
//if (offset > 0xF1FFFF)
//	badWrite = true;
	JaguarWriteLong(offset, data, who);
}

//
// Update the DSP register file pointers depending on REGPAGE bit
//
void DSPUpdateRegisterBanks(void)
{
	int bank = (dsp_flags & REGPAGE);

	if (dsp_flags & IMASK)
		bank = 0;							// IMASK forces main bank to be bank 0

	if (bank)
		dsp_reg = dsp_reg_bank_1, dsp_alternate_reg = dsp_reg_bank_0;
	else
		dsp_reg = dsp_reg_bank_0, dsp_alternate_reg = dsp_reg_bank_1;
}

//
// Check for an handle any asserted DSP IRQs
//
void DSPHandleIRQs(void)
{
	if (dsp_flags & IMASK) 							// Bail if we're already inside an interrupt
		return;

	// Get the active interrupt bits (latches) & interrupt mask (enables)
	uint32 bits = ((dsp_control >> 10) & 0x20) | ((dsp_control >> 6) & 0x1F),
		mask = ((dsp_flags >> 11) & 0x20) | ((dsp_flags >> 4) & 0x1F);

//	WriteLog("dsp: bits=%.2x mask=%.2x\n",bits,mask);
	bits &= mask;

	if (!bits)										// Bail if nothing is enabled
		return;

	int which = 0;									// Determine which interrupt 
	if (bits & 0x01)
		which = 0;
	if (bits & 0x02)
		which = 1;
	if (bits & 0x04)
		which = 2;
	if (bits & 0x08)
		which = 3;
	if (bits & 0x10)
		which = 4;
	if (bits & 0x20)
		which = 5;

#ifdef DSP_DEBUG_IRQ
	WriteLog("DSP: Generating interrupt #%i...\n", which);
#endif

	dsp_flags |= IMASK;
	DSPUpdateRegisterBanks();

	// subqt  #4,r31		; pre-decrement stack pointer 
	// move  pc,r30			; address of interrupted code 
	// store  r30,(r31)     ; store return address
	dsp_reg[31] -= 4;
	DSPWriteLong(dsp_reg[31], dsp_pc - 2, DSP);

	// movei  #service_address,r30  ; pointer to ISR entry 
	// jump  (r30)					; jump to ISR 
	// nop
	dsp_pc = dsp_reg[30] = DSP_WORK_RAM_BASE + (which * 0x10);
}

//
// Set the specified DSP IRQ line to a given state
//
void DSPSetIRQLine(int irqline, int state)
{
//NOTE: This doesn't take INT_LAT5 into account. !!! FIX !!!
	uint32 mask = INT_LAT0 << irqline;
	dsp_control &= ~mask;							// Clear the latch bit

	if (state)
	{
		dsp_control |= mask;						// Set the latch bit
		DSPHandleIRQs();
	}
}

void DSPInit(void)
{
	memory_malloc_secure((void **)&dsp_ram_8, 0x2000, "DSP work RAM");
	memory_malloc_secure((void **)&dsp_reg_bank_0, 32 * sizeof(int32), "DSP bank 0 regs");
	memory_malloc_secure((void **)&dsp_reg_bank_1, 32 * sizeof(int32), "DSP bank 1 regs");

	dsp_build_branch_condition_table();
	DSPReset();
}

void DSPReset(void)
{
	dsp_pc				  = 0x00F1B000;
	dsp_acc				  = 0x00000000;
	dsp_remain			  = 0x00000000;
	dsp_modulo			  = 0xFFFFFFFF;
	dsp_flags			  = 0x00040000;
	dsp_matrix_control    = 0x00000000;
	dsp_pointer_to_matrix = 0x00000000;
	dsp_data_organization = 0xFFFFFFFF;
	dsp_control			  = 0x00002000;				// Report DSP version 2
	dsp_div_control		  = 0x00000000;
	dsp_in_exec			  = 0;

	dsp_reg = dsp_reg_bank_0;
	dsp_alternate_reg = dsp_reg_bank_1;

	for(int i=0; i<32; i++)
		dsp_reg[i] = dsp_alternate_reg[i] = 0x00000000;

	CLR_ZNC;

	dsp_reset_stats();
	memset(dsp_ram_8, 0xFF, 0x2000);
}

void DSPDumpDisassembly(void)
{
	char buffer[512];

	WriteLog("\n---[DSP code at 00F1B000]---------------------------\n");
	uint32 j = 0xF1B000;
	while (j <= 0xF1CFFF)
	{
		uint32 oldj = j;
		j += dasmjag(JAGUAR_DSP, buffer, j);
		WriteLog("\t%08X: %s\n", oldj, buffer);
	}
}

void DSPDumpRegisters(void)
{
//Shoud add modulus, etc to dump here...
	WriteLog("\n---[DSP flags: NCZ %d%d%d, DSP PC: %08X]------------\n", dsp_flag_n, dsp_flag_c, dsp_flag_z, dsp_pc);
	WriteLog("\nRegisters bank 0\n");
	for(int j=0; j<8; j++)
	{
		WriteLog("\tR%02i = %08X R%02i = %08X R%02i = %08X R%02i = %08X\n",
						  (j << 2) + 0, dsp_reg_bank_0[(j << 2) + 0],
						  (j << 2) + 1, dsp_reg_bank_0[(j << 2) + 1],
						  (j << 2) + 2, dsp_reg_bank_0[(j << 2) + 2],
						  (j << 2) + 3, dsp_reg_bank_0[(j << 2) + 3]);
	}
	WriteLog("Registers bank 1\n");
	for(int j=0; j<8; j++)
	{
		WriteLog("\tR%02i = %08X R%02i = %08X R%02i = %08X R%02i = %08X\n",
						  (j << 2) + 0, dsp_reg_bank_1[(j << 2) + 0],
						  (j << 2) + 1, dsp_reg_bank_1[(j << 2) + 1],
						  (j << 2) + 2, dsp_reg_bank_1[(j << 2) + 2],
						  (j << 2) + 3, dsp_reg_bank_1[(j << 2) + 3]);
	}
}

void DSPDone(void)
{
	int i, j;
	WriteLog("DSP: Stopped at PC=%08X dsp_modulo=%08X (dsp %s running)\n", dsp_pc, dsp_modulo, (DSP_RUNNING ? "was" : "wasn't"));
	WriteLog("DSP: %sin interrupt handler\n", (dsp_flags & IMASK ? "" : "not "));

	// get the active interrupt bits 
	int bits = ((dsp_control >> 10) & 0x20) | ((dsp_control >> 6) & 0x1F);
	// get the interrupt mask 
	int mask = ((dsp_flags >> 11) & 0x20) | ((dsp_flags >> 4) & 0x1F);

	WriteLog("DSP: pending=%08X enabled=%08X\n", bits, mask);
	WriteLog("\nRegisters bank 0\n");
	for(int j=0; j<8; j++)
	{
		WriteLog("\tr%2i=0x%.8x r%2i=0x%.8x r%2i=0x%.8x r%2i=0x%.8x\n",
						  (j << 2) + 0, dsp_reg_bank_0[(j << 2) + 0],
						  (j << 2) + 1, dsp_reg_bank_0[(j << 2) + 1],
						  (j << 2) + 2, dsp_reg_bank_0[(j << 2) + 2],
						  (j << 2) + 3, dsp_reg_bank_0[(j << 2) + 3]);
	}
	WriteLog("\nRegisters bank 1\n");
	for (j=0; j<8; j++)
	{
		WriteLog("\tr%2i=0x%.8x r%2i=0x%.8x r%2i=0x%.8x r%2i=0x%.8x\n",
						  (j << 2) + 0, dsp_reg_bank_1[(j << 2) + 0],
						  (j << 2) + 1, dsp_reg_bank_1[(j << 2) + 1],
						  (j << 2) + 2, dsp_reg_bank_1[(j << 2) + 2],
						  (j << 2) + 3, dsp_reg_bank_1[(j << 2) + 3]);

	}

	static char buffer[512];
	j = DSP_WORK_RAM_BASE;
	while (j <= 0xF1BFFF)
	{
		uint32 oldj = j;
		j += dasmjag(JAGUAR_DSP, buffer, j);
		WriteLog("\t%08X: %s\n", oldj, buffer);
	}//*/

	WriteLog("DSP opcodes use:\n");
	for (i=0;i<64;i++)
	{
		if (dsp_opcode_use[i])
			WriteLog("\t%s %i\n", dsp_opcode_str[i], dsp_opcode_use[i]);
	}//*/

	memory_free(dsp_ram_8);
}

//
// DSP execution core
//
static bool R20Set = false, tripwire = false;
static uint32 pcQueue[32], ptrPCQ = 0;
void DSPExec(int32 cycles)
{
/*HACKS!!! ->	if (cycles != 1 && jaguar_mainRom_crc32 == 0xba74c3ed)
		dsp_check_if_i2s_interrupt_needed();*/

#ifdef DSP_SINGLE_STEPPING
	if (dsp_control & 0x18)
	{
		cycles = 1;
		dsp_control &= ~0x10;
	}
#endif
//There is *no* good reason to do this here!
//	DSPHandleIRQs();
	dsp_releaseTimeSlice_flag = 0;
	dsp_in_exec++;

	while (cycles > 0 && DSP_RUNNING)
	{
/*if (badWrite)
{
	WriteLog("\nDSP: Encountered bad write in Atari Synth module. PC=%08X, R15=%08X\n", dsp_pc, dsp_reg[15]);
	for(int i=0; i<80; i+=4)
		WriteLog("     %08X: %08X\n", dsp_reg[15]+i, JaguarReadLong(dsp_reg[15]+i));
	WriteLog("\n");
}//*/
/*if (dsp_pc == 0xF1B55E)
{
	WriteLog("DSP: At $F1B55E--R15 = %08X at %u ms%s...\n", dsp_reg[15], SDL_GetTicks(), (dsp_flags & IMASK ? " (inside interrupt)" : ""));
}//*/
/*if (dsp_pc == 0xF1B7D2)	// Start here???
	doDSPDis = true;
pcQueue[ptrPCQ++] = dsp_pc;
ptrPCQ %= 32;*/
		uint16 opcode = DSPReadWord(dsp_pc, DSP);
		uint32 index = opcode >> 10;
		dsp_opcode_first_parameter = (opcode >> 5) & 0x1F;
		dsp_opcode_second_parameter = opcode & 0x1F;
		dsp_pc += 2;
		dsp_opcode[index]();
		dsp_opcode_use[index]++;
		cycles -= dsp_opcode_cycles[index];
/*if (dsp_reg_bank_0[20] == 0xF1A100 & !R20Set)
{
	WriteLog("DSP: R20 set to $F1A100 at %u ms%s...\n", SDL_GetTicks(), (dsp_flags & IMASK ? " (inside interrupt)" : ""));
	R20Set = true;
}
if (dsp_reg_bank_0[20] != 0xF1A100 && R20Set)
{
	WriteLog("DSP: R20 corrupted at %u ms from starting%s!\nAborting!\n", SDL_GetTicks(), (dsp_flags & IMASK ? " (inside interrupt)" : ""));
	DSPDumpRegisters();
	DSPDumpDisassembly();
	exit(1);
}
if ((dsp_pc < 0xF1B000 || dsp_pc > 0xF1CFFE) && !tripwire)
{
	char buffer[512];
	WriteLog("DSP: Jumping outside of DSP RAM at %u ms. Register dump:\n", SDL_GetTicks());
	DSPDumpRegisters();
	tripwire = true;
	WriteLog("\nBacktrace:\n");
	for(int i=0; i<32; i++)
	{
		dasmjag(JAGUAR_DSP, buffer, pcQueue[(ptrPCQ + i) % 32]);
		WriteLog("\t%08X: %s\n", pcQueue[(ptrPCQ + i) % 32], buffer);
	}
	WriteLog("\n");
}*/
	}

	dsp_in_exec--;
}

//
// DSP opcode handlers
//

// There is a problem here with interrupt handlers the JUMP and JR instructions that
// can cause trouble because an interrupt can occur *before* the instruction following the
// jump can execute... !!! FIX !!!
static void dsp_opcode_jump(void)
{
#ifdef DSP_DIS_JUMP
char * condition[32] =
{	"T", "nz", "z", "???", "nc", "nc nz", "nc z", "???", "c", "c nz",
	"c z", "???", "???", "???", "???", "???", "???", "???", "???",
	"???", "nn", "nn nz", "nn z", "???", "n", "n nz", "n z", "???",
	"???", "???", "???", "F" };
	if (doDSPDis)
		WriteLog("%06X: JUMP   %s, (R%02u) [NCZ:%u%u%u, R%02u=%08X] ", dsp_pc-2, condition[IMM_2], IMM_1, dsp_flag_n, dsp_flag_c, dsp_flag_z, IMM_1, RM);
#endif
	// normalize flags
/*	dsp_flag_c=dsp_flag_c?1:0;
	dsp_flag_z=dsp_flag_z?1:0;
	dsp_flag_n=dsp_flag_n?1:0;*/
	// KLUDGE: Used by BRANCH_CONDITION
	uint32 jaguar_flags = (dsp_flag_n << 2) | (dsp_flag_c << 1) | dsp_flag_z;

	if (BRANCH_CONDITION(IMM_2))
	{
#ifdef DSP_DIS_JUMP
	if (doDSPDis)
		WriteLog("Branched!\n");
#endif
		uint32 delayed_pc = RM;
		DSPExec(1);
		dsp_pc = delayed_pc;
	}
#ifdef DSP_DIS_JUMP
	else
		if (doDSPDis)
			WriteLog("Branch NOT taken.\n");
#endif
}

static void dsp_opcode_jr(void)
{
#ifdef DSP_DIS_JR
char * condition[32] =
{	"T", "nz", "z", "???", "nc", "nc nz", "nc z", "???", "c", "c nz",
	"c z", "???", "???", "???", "???", "???", "???", "???", "???",
	"???", "nn", "nn nz", "nn z", "???", "n", "n nz", "n z", "???",
	"???", "???", "???", "F" };
	if (doDSPDis)
		WriteLog("%06X: JR     %s, %06X [NCZ:%u%u%u] ", dsp_pc-2, condition[IMM_2], dsp_pc+((IMM_1 & 0x10 ? 0xFFFFFFF0 | IMM_1 : IMM_1) * 2), dsp_flag_n, dsp_flag_c, dsp_flag_z);
#endif
	// normalize flags
/*	dsp_flag_c=dsp_flag_c?1:0;
	dsp_flag_z=dsp_flag_z?1:0;
	dsp_flag_n=dsp_flag_n?1:0;*/
	// KLUDGE: Used by BRANCH_CONDITION
	uint32 jaguar_flags = (dsp_flag_n << 2) | (dsp_flag_c << 1) | dsp_flag_z;

	if (BRANCH_CONDITION(IMM_2))
	{
#ifdef DSP_DIS_JR
	if (doDSPDis)
		WriteLog("Branched!\n");
#endif
		int32 offset = (IMM_1 & 0x10 ? 0xFFFFFFF0 | IMM_1 : IMM_1);		// Sign extend IMM_1
		int32 delayed_pc = dsp_pc + (offset * 2);
		DSPExec(1);
		dsp_pc = delayed_pc;
	}
#ifdef DSP_DIS_JR
	else
		if (doDSPDis)
			WriteLog("Branch NOT taken.\n");
#endif
}

static void dsp_opcode_add(void)
{
#ifdef DSP_DIS_ADD
	if (doDSPDis)
		WriteLog("%06X: ADD    R%02u, R%02u [NCZ:%u%u%u, R%02u=%08X, R%02u=%08X] -> ", dsp_pc-2, IMM_1, IMM_2, dsp_flag_n, dsp_flag_c, dsp_flag_z, IMM_1, RM, IMM_2, RN);
#endif
	UINT32 res = RN + RM;
	SET_ZNC_ADD(RN, RM, res);
	RN = res;
#ifdef DSP_DIS_ADD
	if (doDSPDis)
		WriteLog("[NCZ:%u%u%u, R%02u=%08X, R%02u=%08X]\n", dsp_flag_n, dsp_flag_c, dsp_flag_z, IMM_1, RM, IMM_2, RN);
#endif
}

static void dsp_opcode_addc(void)
{
#ifdef DSP_DIS_ADDC
	if (doDSPDis)
		WriteLog("%06X: ADDC   R%02u, R%02u [NCZ:%u%u%u, R%02u=%08X, R%02u=%08X] -> ", dsp_pc-2, IMM_1, IMM_2, dsp_flag_n, dsp_flag_c, dsp_flag_z, IMM_1, RM, IMM_2, RN);
#endif
	UINT32 res = RN + RM + dsp_flag_c;
	UINT32 carry = dsp_flag_c;
//	SET_ZNC_ADD(RN, RM, res); //???BUG??? Yes!
	SET_ZNC_ADD(RN + carry, RM, res);
//	SET_ZNC_ADD(RN, RM + carry, res);
	RN = res;
#ifdef DSP_DIS_ADDC
	if (doDSPDis)
		WriteLog("[NCZ:%u%u%u, R%02u=%08X, R%02u=%08X]\n", dsp_flag_n, dsp_flag_c, dsp_flag_z, IMM_1, RM, IMM_2, RN);
#endif
}

static void dsp_opcode_addq(void)
{
#ifdef DSP_DIS_ADDQ
	if (doDSPDis)
		WriteLog("%06X: ADDQ   #%u, R%02u [NCZ:%u%u%u, R%02u=%08X] -> ", dsp_pc-2, dsp_convert_zero[IMM_1], IMM_2, dsp_flag_n, dsp_flag_c, dsp_flag_z, IMM_2, RN);
#endif
	UINT32 r1 = dsp_convert_zero[IMM_1];
	UINT32 res = RN + r1;
	CLR_ZNC; SET_ZNC_ADD(RN, r1, res);
	RN = res;
#ifdef DSP_DIS_ADDQ
	if (doDSPDis)
		WriteLog("[NCZ:%u%u%u, R%02u=%08X]\n", dsp_flag_n, dsp_flag_c, dsp_flag_z, IMM_2, RN);
#endif
}

static void dsp_opcode_sub(void)
{
#ifdef DSP_DIS_SUB
	if (doDSPDis)
		WriteLog("%06X: SUB    R%02u, R%02u [NCZ:%u%u%u, R%02u=%08X, R%02u=%08X] -> ", dsp_pc-2, IMM_1, IMM_2, dsp_flag_n, dsp_flag_c, dsp_flag_z, IMM_1, RM, IMM_2, RN);
#endif
	UINT32 res = RN - RM;
	SET_ZNC_SUB(RN, RM, res);
	RN = res;
#ifdef DSP_DIS_SUB
	if (doDSPDis)
		WriteLog("[NCZ:%u%u%u, R%02u=%08X, R%02u=%08X]\n", dsp_flag_n, dsp_flag_c, dsp_flag_z, IMM_1, RM, IMM_2, RN);
#endif
}

static void dsp_opcode_subc(void)
{
#ifdef DSP_DIS_SUBC
	if (doDSPDis)
		WriteLog("%06X: SUBC   R%02u, R%02u [NCZ:%u%u%u, R%02u=%08X, R%02u=%08X] -> ", dsp_pc-2, IMM_1, IMM_2, dsp_flag_n, dsp_flag_c, dsp_flag_z, IMM_1, RM, IMM_2, RN);
#endif
	UINT32 res = RN - RM - dsp_flag_c;
	UINT32 borrow = dsp_flag_c;
	SET_ZNC_SUB(RN - borrow, RM, res);
	RN = res;
#ifdef DSP_DIS_SUBC
	if (doDSPDis)
		WriteLog("[NCZ:%u%u%u, R%02u=%08X, R%02u=%08X]\n", dsp_flag_n, dsp_flag_c, dsp_flag_z, IMM_1, RM, IMM_2, RN);
#endif
}

static void dsp_opcode_subq(void)
{
#ifdef DSP_DIS_SUBQ
	if (doDSPDis)
		WriteLog("%06X: SUBQ   #%u, R%02u [NCZ:%u%u%u, R%02u=%08X] -> ", dsp_pc-2, dsp_convert_zero[IMM_1], IMM_2, dsp_flag_n, dsp_flag_c, dsp_flag_z, IMM_2, RN);
#endif
	UINT32 r1 = dsp_convert_zero[IMM_1];
	UINT32 res = RN - r1;
	SET_ZNC_SUB(RN, r1, res);
	RN = res;
#ifdef DSP_DIS_SUBQ
	if (doDSPDis)
		WriteLog("[NCZ:%u%u%u, R%02u=%08X]\n", dsp_flag_n, dsp_flag_c, dsp_flag_z, IMM_2, RN);
#endif
}

static void dsp_opcode_cmp(void)
{
#ifdef DSP_DIS_CMP
	if (doDSPDis)
		WriteLog("%06X: CMP    R%02u, R%02u [NCZ:%u%u%u, R%02u=%08X, R%02u=%08X] -> ", dsp_pc-2, IMM_1, IMM_2, dsp_flag_n, dsp_flag_c, dsp_flag_z, IMM_1, RM, IMM_2, RN);
#endif
	UINT32 res = RN - RM;
	SET_ZNC_SUB(RN, RM, res);
#ifdef DSP_DIS_CMP
	if (doDSPDis)
		WriteLog("[NCZ:%u%u%u]\n", dsp_flag_n, dsp_flag_c, dsp_flag_z);
#endif
}

static void dsp_opcode_cmpq(void)
{
	static int32 sqtable[32] =
		{ 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,-16,-15,-14,-13,-12,-11,-10,-9,-8,-7,-6,-5,-4,-3,-2,-1 };
#ifdef DSP_DIS_CMPQ
	if (doDSPDis)
		WriteLog("%06X: CMPQ   #%d, R%02u [NCZ:%u%u%u, R%02u=%08X] -> ", dsp_pc-2, sqtable[IMM_1], IMM_2, dsp_flag_n, dsp_flag_c, dsp_flag_z, IMM_2, RN);
#endif
	UINT32 r1 = sqtable[IMM_1 & 0x1F]; // I like this better -> (INT8)(jaguar.op >> 2) >> 3;
	UINT32 res = RN - r1;
	SET_ZNC_SUB(RN, r1, res);
#ifdef DSP_DIS_CMPQ
	if (doDSPDis)
		WriteLog("[NCZ:%u%u%u]\n", dsp_flag_n, dsp_flag_c, dsp_flag_z);
#endif
}

static void dsp_opcode_and(void)
{
#ifdef DSP_DIS_AND
	if (doDSPDis)
		WriteLog("%06X: AND    R%02u, R%02u [NCZ:%u%u%u, R%02u=%08X, R%02u=%08X] -> ", dsp_pc-2, IMM_1, IMM_2, dsp_flag_n, dsp_flag_c, dsp_flag_z, IMM_1, RM, IMM_2, RN);
#endif
	RN = RN & RM;
	SET_ZN(RN);
#ifdef DSP_DIS_AND
	if (doDSPDis)
		WriteLog("[NCZ:%u%u%u, R%02u=%08X, R%02u=%08X]\n", dsp_flag_n, dsp_flag_c, dsp_flag_z, IMM_1, RM, IMM_2, RN);
#endif
}

static void dsp_opcode_or(void)
{
#ifdef DSP_DIS_OR
	if (doDSPDis)
		WriteLog("%06X: OR     R%02u, R%02u [NCZ:%u%u%u, R%02u=%08X, R%02u=%08X] -> ", dsp_pc-2, IMM_1, IMM_2, dsp_flag_n, dsp_flag_c, dsp_flag_z, IMM_1, RM, IMM_2, RN);
#endif
	RN = RN | RM;
	SET_ZN(RN);
#ifdef DSP_DIS_OR
	if (doDSPDis)
		WriteLog("[NCZ:%u%u%u, R%02u=%08X, R%02u=%08X]\n", dsp_flag_n, dsp_flag_c, dsp_flag_z, IMM_1, RM, IMM_2, RN);
#endif
}

static void dsp_opcode_xor(void)
{
#ifdef DSP_DIS_XOR
	if (doDSPDis)
		WriteLog("%06X: XOR    R%02u, R%02u [NCZ:%u%u%u, R%02u=%08X, R%02u=%08X] -> ", dsp_pc-2, IMM_1, IMM_2, dsp_flag_n, dsp_flag_c, dsp_flag_z, IMM_1, RM, IMM_2, RN);
#endif
	RN = RN ^ RM;
	SET_ZN(RN);
#ifdef DSP_DIS_XOR
	if (doDSPDis)
		WriteLog("[NCZ:%u%u%u, R%02u=%08X, R%02u=%08X]\n", dsp_flag_n, dsp_flag_c, dsp_flag_z, IMM_1, RM, IMM_2, RN);
#endif
}

static void dsp_opcode_not(void)
{
#ifdef DSP_DIS_NOT
	if (doDSPDis)
		WriteLog("%06X: NOT    R%02u, R%02u [NCZ:%u%u%u, R%02u=%08X, R%02u=%08X] -> ", dsp_pc-2, IMM_1, IMM_2, dsp_flag_n, dsp_flag_c, dsp_flag_z, IMM_1, RM, IMM_2, RN);
#endif
	RN = ~RN;
	SET_ZN(RN);
#ifdef DSP_DIS_NOT
	if (doDSPDis)
		WriteLog("[NCZ:%u%u%u, R%02u=%08X, R%02u=%08X]\n", dsp_flag_n, dsp_flag_c, dsp_flag_z, IMM_1, RM, IMM_2, RN);
#endif
}

static void dsp_opcode_move_pc(void)
{
	RN = dsp_pc - 2;
}

static void dsp_opcode_store_r14_indexed(void)
{
#ifdef DSP_DIS_STORE14I
	if (doDSPDis)
		WriteLog("%06X: STORE  R%02u, (R14+$%02X) [NCZ:%u%u%u, R%02u=%08X, R14+$%02X=%08X]\n", dsp_pc-2, IMM_2, dsp_convert_zero[IMM_1] << 2, dsp_flag_n, dsp_flag_c, dsp_flag_z, IMM_2, RN, dsp_convert_zero[IMM_1] << 2, dsp_reg[14]+(dsp_convert_zero[IMM_1] << 2));
#endif
	DSPWriteLong(dsp_reg[14] + (dsp_convert_zero[IMM_1] << 2), RN, DSP);
}

static void dsp_opcode_store_r15_indexed(void)
{
#ifdef DSP_DIS_STORE15I
	if (doDSPDis)
		WriteLog("%06X: STORE  R%02u, (R15+$%02X) [NCZ:%u%u%u, R%02u=%08X, R15+$%02X=%08X]\n", dsp_pc-2, IMM_2, dsp_convert_zero[IMM_1] << 2, dsp_flag_n, dsp_flag_c, dsp_flag_z, IMM_2, RN, dsp_convert_zero[IMM_1] << 2, dsp_reg[15]+(dsp_convert_zero[IMM_1] << 2));
#endif
	DSPWriteLong(dsp_reg[15] + (dsp_convert_zero[IMM_1] << 2), RN, DSP);
}

static void dsp_opcode_load_r14_ri(void)
{
#ifdef DSP_DIS_LOAD14R
	if (doDSPDis)
		WriteLog("%06X: LOAD   (R14+R%02u), R%02u [NCZ:%u%u%u, R14+R%02u=%08X, R%02u=%08X] -> ", dsp_pc-2, IMM_1, IMM_2, dsp_flag_n, dsp_flag_c, dsp_flag_z, IMM_1, RM+dsp_reg[14], IMM_2, RN);
#endif
	RN = DSPReadLong(dsp_reg[14] + RM, DSP);
#ifdef DSP_DIS_LOAD14R
	if (doDSPDis)
		WriteLog("[NCZ:%u%u%u, R%02u=%08X]\n", dsp_flag_n, dsp_flag_c, dsp_flag_z, IMM_2, RN);
#endif
}

static void dsp_opcode_load_r15_ri(void)
{
#ifdef DSP_DIS_LOAD15R
	if (doDSPDis)
		WriteLog("%06X: LOAD   (R15+R%02u), R%02u [NCZ:%u%u%u, R15+R%02u=%08X, R%02u=%08X] -> ", dsp_pc-2, IMM_1, IMM_2, dsp_flag_n, dsp_flag_c, dsp_flag_z, IMM_1, RM+dsp_reg[15], IMM_2, RN);
#endif
	RN = DSPReadLong(dsp_reg[15] + RM, DSP);
#ifdef DSP_DIS_LOAD15R
	if (doDSPDis)
		WriteLog("[NCZ:%u%u%u, R%02u=%08X]\n", dsp_flag_n, dsp_flag_c, dsp_flag_z, IMM_2, RN);
#endif
}

static void dsp_opcode_store_r14_ri(void)
{
	DSPWriteLong(dsp_reg[14] + RM, RN, DSP);
}

static void dsp_opcode_store_r15_ri(void)
{
	DSPWriteLong(dsp_reg[15] + RM, RN, DSP);
}

static void dsp_opcode_nop(void)
{
#ifdef DSP_DIS_NOP
	if (doDSPDis)
		WriteLog("%06X: NOP    [NCZ:%u%u%u]\n", dsp_pc-2, dsp_flag_n, dsp_flag_c, dsp_flag_z);
#endif
}

static void dsp_opcode_storeb(void)
{
#ifdef DSP_DIS_STOREB
	if (doDSPDis)
		WriteLog("%06X: STOREB R%02u, (R%02u) [NCZ:%u%u%u, R%02u=%08X, R%02u=%08X]\n", dsp_pc-2, IMM_2, IMM_1, dsp_flag_n, dsp_flag_c, dsp_flag_z, IMM_2, RN, IMM_1, RM);
#endif
	if (RM >= DSP_WORK_RAM_BASE && RM <= (DSP_WORK_RAM_BASE + 0x1FFF))
		DSPWriteLong(RM, RN & 0xFF, DSP);
	else
		JaguarWriteByte(RM, RN, DSP);
}

static void dsp_opcode_storew(void)
{
#ifdef DSP_DIS_STOREW
	if (doDSPDis)
		WriteLog("%06X: STOREW R%02u, (R%02u) [NCZ:%u%u%u, R%02u=%08X, R%02u=%08X]\n", dsp_pc-2, IMM_2, IMM_1, dsp_flag_n, dsp_flag_c, dsp_flag_z, IMM_2, RN, IMM_1, RM);
#endif
	if (RM >= DSP_WORK_RAM_BASE && RM <= (DSP_WORK_RAM_BASE + 0x1FFF))
		DSPWriteLong(RM, RN & 0xFFFF, DSP);
	else
		JaguarWriteWord(RM, RN, DSP);
}

static void dsp_opcode_store(void)
{
#ifdef DSP_DIS_STORE
	if (doDSPDis)
		WriteLog("%06X: STORE  R%02u, (R%02u) [NCZ:%u%u%u, R%02u=%08X, R%02u=%08X]\n", dsp_pc-2, IMM_2, IMM_1, dsp_flag_n, dsp_flag_c, dsp_flag_z, IMM_2, RN, IMM_1, RM);
#endif
	DSPWriteLong(RM, RN, DSP);
}

static void dsp_opcode_loadb(void)
{
#ifdef DSP_DIS_LOADB
	if (doDSPDis)
		WriteLog("%06X: LOADB  (R%02u), R%02u [NCZ:%u%u%u, R%02u=%08X, R%02u=%08X] -> ", dsp_pc-2, IMM_1, IMM_2, dsp_flag_n, dsp_flag_c, dsp_flag_z, IMM_1, RM, IMM_2, RN);
#endif
	if (RM >= DSP_WORK_RAM_BASE && RM <= (DSP_WORK_RAM_BASE + 0x1FFF))
		RN = DSPReadLong(RM, DSP) & 0xFF;
	else
		RN = JaguarReadByte(RM, DSP);
#ifdef DSP_DIS_LOADB
	if (doDSPDis)
		WriteLog("[NCZ:%u%u%u, R%02u=%08X]\n", dsp_flag_n, dsp_flag_c, dsp_flag_z, IMM_2, RN);
#endif
}

static void dsp_opcode_loadw(void)
{
#ifdef DSP_DIS_LOADW
	if (doDSPDis)
		WriteLog("%06X: LOADW  (R%02u), R%02u [NCZ:%u%u%u, R%02u=%08X, R%02u=%08X] -> ", dsp_pc-2, IMM_1, IMM_2, dsp_flag_n, dsp_flag_c, dsp_flag_z, IMM_1, RM, IMM_2, RN);
#endif
	if (RM >= DSP_WORK_RAM_BASE && RM <= (DSP_WORK_RAM_BASE + 0x1FFF))
		RN = DSPReadLong(RM, DSP) & 0xFFFF;
	else
		RN = JaguarReadWord(RM, DSP);
#ifdef DSP_DIS_LOADW
	if (doDSPDis)
		WriteLog("[NCZ:%u%u%u, R%02u=%08X]\n", dsp_flag_n, dsp_flag_c, dsp_flag_z, IMM_2, RN);
#endif
}

static void dsp_opcode_load(void)
{
#ifdef DSP_DIS_LOAD
	if (doDSPDis)
		WriteLog("%06X: LOAD   (R%02u), R%02u [NCZ:%u%u%u, R%02u=%08X, R%02u=%08X] -> ", dsp_pc-2, IMM_1, IMM_2, dsp_flag_n, dsp_flag_c, dsp_flag_z, IMM_1, RM, IMM_2, RN);
#endif
	RN = DSPReadLong(RM, DSP);
#ifdef DSP_DIS_LOAD
	if (doDSPDis)
		WriteLog("[NCZ:%u%u%u, R%02u=%08X]\n", dsp_flag_n, dsp_flag_c, dsp_flag_z, IMM_2, RN);
#endif
}

static void dsp_opcode_load_r14_indexed(void)
{
#ifdef DSP_DIS_LOAD14I
	if (doDSPDis)
		WriteLog("%06X: LOAD   (R14+$%02X), R%02u [NCZ:%u%u%u, R14+$%02X=%08X, R%02u=%08X] -> ", dsp_pc-2, dsp_convert_zero[IMM_1] << 2, IMM_2, dsp_flag_n, dsp_flag_c, dsp_flag_z, dsp_convert_zero[IMM_1] << 2, dsp_reg[14]+(dsp_convert_zero[IMM_1] << 2), IMM_2, RN);
#endif
	RN = DSPReadLong(dsp_reg[14] + (dsp_convert_zero[IMM_1] << 2), DSP);
#ifdef DSP_DIS_LOAD14I
	if (doDSPDis)
		WriteLog("[NCZ:%u%u%u, R%02u=%08X]\n", dsp_flag_n, dsp_flag_c, dsp_flag_z, IMM_2, RN);
#endif
}

static void dsp_opcode_load_r15_indexed(void)
{
#ifdef DSP_DIS_LOAD15I
	if (doDSPDis)
		WriteLog("%06X: LOAD   (R15+$%02X), R%02u [NCZ:%u%u%u, R15+$%02X=%08X, R%02u=%08X] -> ", dsp_pc-2, dsp_convert_zero[IMM_1] << 2, IMM_2, dsp_flag_n, dsp_flag_c, dsp_flag_z, dsp_convert_zero[IMM_1] << 2, dsp_reg[15]+(dsp_convert_zero[IMM_1] << 2), IMM_2, RN);
#endif
	RN = DSPReadLong(dsp_reg[15] + (dsp_convert_zero[IMM_1] << 2), DSP);
#ifdef DSP_DIS_LOAD15I
	if (doDSPDis)
		WriteLog("[NCZ:%u%u%u, R%02u=%08X]\n", dsp_flag_n, dsp_flag_c, dsp_flag_z, IMM_2, RN);
#endif
}

static void dsp_opcode_movei(void)
{
#ifdef DSP_DIS_MOVEI
	if (doDSPDis)
		WriteLog("%06X: MOVEI  #$%08X, R%02u [NCZ:%u%u%u, R%02u=%08X] -> ", dsp_pc-2, (uint32)DSPReadWord(dsp_pc) | ((uint32)DSPReadWord(dsp_pc + 2) << 16), IMM_2, dsp_flag_n, dsp_flag_c, dsp_flag_z, IMM_2, RN);
#endif
	// This instruction is followed by 32-bit value in LSW / MSW format...
	RN = (uint32)DSPReadWord(dsp_pc, DSP) | ((uint32)DSPReadWord(dsp_pc + 2, DSP) << 16);
	dsp_pc += 4;
#ifdef DSP_DIS_MOVEI
	if (doDSPDis)
		WriteLog("[NCZ:%u%u%u, R%02u=%08X]\n", dsp_flag_n, dsp_flag_c, dsp_flag_z, IMM_2, RN);
#endif
}

static void dsp_opcode_moveta(void)
{
#ifdef DSP_DIS_MOVETA
	if (doDSPDis)
		WriteLog("%06X: MOVETA R%02u, R%02u [NCZ:%u%u%u, R%02u=%08X, R%02u(alt)=%08X] -> ", dsp_pc-2, IMM_1, IMM_2, dsp_flag_n, dsp_flag_c, dsp_flag_z, IMM_1, RM, IMM_2, ALTERNATE_RN);
#endif
	ALTERNATE_RN = RM;
#ifdef DSP_DIS_MOVETA
	if (doDSPDis)
		WriteLog("[NCZ:%u%u%u, R%02u=%08X, R%02u(alt)=%08X]\n", dsp_flag_n, dsp_flag_c, dsp_flag_z, IMM_1, RM, IMM_2, ALTERNATE_RN);
#endif
}

static void dsp_opcode_movefa(void)
{
#ifdef DSP_DIS_MOVEFA
	if (doDSPDis)
		WriteLog("%06X: MOVEFA R%02u, R%02u [NCZ:%u%u%u, R%02u(alt)=%08X, R%02u=%08X] -> ", dsp_pc-2, IMM_1, IMM_2, dsp_flag_n, dsp_flag_c, dsp_flag_z, IMM_1, ALTERNATE_RM, IMM_2, RN);
#endif
	RN = ALTERNATE_RM;
#ifdef DSP_DIS_MOVEFA
	if (doDSPDis)
		WriteLog("[NCZ:%u%u%u, R%02u(alt)=%08X, R%02u=%08X]\n", dsp_flag_n, dsp_flag_c, dsp_flag_z, IMM_1, ALTERNATE_RM, IMM_2, RN);
#endif
}

static void dsp_opcode_move(void)
{
#ifdef DSP_DIS_MOVE
	if (doDSPDis)
		WriteLog("%06X: MOVE   R%02u, R%02u [NCZ:%u%u%u, R%02u=%08X, R%02u=%08X] -> ", dsp_pc-2, IMM_1, IMM_2, dsp_flag_n, dsp_flag_c, dsp_flag_z, IMM_1, RM, IMM_2, RN);
#endif
	RN = RM;
#ifdef DSP_DIS_MOVE
	if (doDSPDis)
		WriteLog("[NCZ:%u%u%u, R%02u=%08X, R%02u=%08X]\n", dsp_flag_n, dsp_flag_c, dsp_flag_z, IMM_1, RM, IMM_2, RN);
#endif
}

static void dsp_opcode_moveq(void)
{
#ifdef DSP_DIS_MOVEQ
	if (doDSPDis)
		WriteLog("%06X: MOVEQ  #%u, R%02u [NCZ:%u%u%u, R%02u=%08X] -> ", dsp_pc-2, IMM_1, IMM_2, dsp_flag_n, dsp_flag_c, dsp_flag_z, IMM_2, RN);
#endif
	RN = IMM_1;
#ifdef DSP_DIS_MOVEQ
	if (doDSPDis)
		WriteLog("[NCZ:%u%u%u, R%02u=%08X]\n", dsp_flag_n, dsp_flag_c, dsp_flag_z, IMM_2, RN);
#endif
}

static void dsp_opcode_resmac(void)
{
#ifdef DSP_DIS_RESMAC
	if (doDSPDis)
		WriteLog("%06X: RESMAC R%02u [NCZ:%u%u%u, R%02u=%08X, DSP_ACC=%02X%08X] -> ", dsp_pc-2, IMM_2, dsp_flag_n, dsp_flag_c, dsp_flag_z, IMM_2, RN, (uint8)(dsp_acc >> 32), (uint32)(dsp_acc & 0xFFFFFFFF));
#endif
	RN = (uint32)dsp_acc;
#ifdef DSP_DIS_RESMAC
	if (doDSPDis)
		WriteLog("[NCZ:%u%u%u, R%02u=%08X]\n", dsp_flag_n, dsp_flag_c, dsp_flag_z, IMM_2, RN);
#endif
}

static void dsp_opcode_imult(void)
{
#ifdef DSP_DIS_IMULT
	if (doDSPDis)
		WriteLog("%06X: IMULT  R%02u, R%02u [NCZ:%u%u%u, R%02u=%08X, R%02u=%08X] -> ", dsp_pc-2, IMM_1, IMM_2, dsp_flag_n, dsp_flag_c, dsp_flag_z, IMM_1, RM, IMM_2, RN);
#endif
	RN = (int16)RN * (int16)RM;
	SET_ZN(RN);
#ifdef DSP_DIS_IMULT
	if (doDSPDis)
		WriteLog("[NCZ:%u%u%u, R%02u=%08X, R%02u=%08X]\n", dsp_flag_n, dsp_flag_c, dsp_flag_z, IMM_1, RM, IMM_2, RN);
#endif
}

static void dsp_opcode_mult(void)
{
#ifdef DSP_DIS_MULT
	if (doDSPDis)
		WriteLog("%06X: MULT   R%02u, R%02u [NCZ:%u%u%u, R%02u=%08X, R%02u=%08X] -> ", dsp_pc-2, IMM_1, IMM_2, dsp_flag_n, dsp_flag_c, dsp_flag_z, IMM_1, RM, IMM_2, RN);
#endif
	RN = (uint16)RM * (uint16)RN;
	SET_ZN(RN);
#ifdef DSP_DIS_MULT
	if (doDSPDis)
		WriteLog("[NCZ:%u%u%u, R%02u=%08X, R%02u=%08X]\n", dsp_flag_n, dsp_flag_c, dsp_flag_z, IMM_1, RM, IMM_2, RN);
#endif
}

static void dsp_opcode_bclr(void)
{
#ifdef DSP_DIS_BCLR
	if (doDSPDis)
		WriteLog("%06X: BCLR   #%u, R%02u [NCZ:%u%u%u, R%02u=%08X] -> ", dsp_pc-2, IMM_1, IMM_2, dsp_flag_n, dsp_flag_c, dsp_flag_z, IMM_2, RN);
#endif
	UINT32 res = RN & ~(1 << IMM_1);
	RN = res;
	SET_ZN(res);
#ifdef DSP_DIS_BCLR
	if (doDSPDis)
		WriteLog("[NCZ:%u%u%u, R%02u=%08X]\n", dsp_flag_n, dsp_flag_c, dsp_flag_z, IMM_2, RN);
#endif
}

static void dsp_opcode_btst(void)
{
#ifdef DSP_DIS_BTST
	if (doDSPDis)
		WriteLog("%06X: BTST   #%u, R%02u [NCZ:%u%u%u, R%02u=%08X] -> ", dsp_pc-2, IMM_1, IMM_2, dsp_flag_n, dsp_flag_c, dsp_flag_z, IMM_2, RN);
#endif
	dsp_flag_z = (~RN >> IMM_1) & 1;
#ifdef DSP_DIS_BTST
	if (doDSPDis)
		WriteLog("[NCZ:%u%u%u, R%02u=%08X]\n", dsp_flag_n, dsp_flag_c, dsp_flag_z, IMM_2, RN);
#endif
}

static void dsp_opcode_bset(void)
{
#ifdef DSP_DIS_BSET
	if (doDSPDis)
		WriteLog("%06X: BSET   #%u, R%02u [NCZ:%u%u%u, R%02u=%08X] -> ", dsp_pc-2, IMM_1, IMM_2, dsp_flag_n, dsp_flag_c, dsp_flag_z, IMM_2, RN);
#endif
	UINT32 res = RN | (1 << IMM_1);
	RN = res;
	SET_ZN(res);
#ifdef DSP_DIS_BSET
	if (doDSPDis)
		WriteLog("[NCZ:%u%u%u, R%02u=%08X]\n", dsp_flag_n, dsp_flag_c, dsp_flag_z, IMM_2, RN);
#endif
}

static void dsp_opcode_subqt(void)
{
#ifdef DSP_DIS_SUBQT
	if (doDSPDis)
		WriteLog("%06X: SUBQT  #%u, R%02u [NCZ:%u%u%u, R%02u=%08X] -> ", dsp_pc-2, dsp_convert_zero[IMM_1], IMM_2, dsp_flag_n, dsp_flag_c, dsp_flag_z, IMM_2, RN);
#endif
	RN -= dsp_convert_zero[IMM_1];
#ifdef DSP_DIS_SUBQT
	if (doDSPDis)
		WriteLog("[NCZ:%u%u%u, R%02u=%08X]\n", dsp_flag_n, dsp_flag_c, dsp_flag_z, IMM_2, RN);
#endif
}

static void dsp_opcode_addqt(void)
{
#ifdef DSP_DIS_ADDQT
	if (doDSPDis)
		WriteLog("%06X: ADDQT  #%u, R%02u [NCZ:%u%u%u, R%02u=%08X] -> ", dsp_pc-2, dsp_convert_zero[IMM_1], IMM_2, dsp_flag_n, dsp_flag_c, dsp_flag_z, IMM_2, RN);
#endif
	RN += dsp_convert_zero[IMM_1];
#ifdef DSP_DIS_ADDQT
	if (doDSPDis)
		WriteLog("[NCZ:%u%u%u, R%02u=%08X]\n", dsp_flag_n, dsp_flag_c, dsp_flag_z, IMM_2, RN);
#endif
}

static void dsp_opcode_imacn(void)
{
#ifdef DSP_DIS_IMACN
	if (doDSPDis)
		WriteLog("%06X: IMACN  R%02u, R%02u [NCZ:%u%u%u, R%02u=%08X, R%02u=%08X] -> ", dsp_pc-2, IMM_1, IMM_2, dsp_flag_n, dsp_flag_c, dsp_flag_z, IMM_1, RM, IMM_2, RN);
#endif
	int32 res = (int16)RM * (int16)RN;
	dsp_acc += (int64)res;
//Should we AND the result to fit into 40 bits here???
#ifdef DSP_DIS_IMACN
	if (doDSPDis)
		WriteLog("[NCZ:%u%u%u, DSP_ACC=%02X%08X]\n", dsp_flag_n, dsp_flag_c, dsp_flag_z, (uint8)(dsp_acc >> 32), (uint32)(dsp_acc & 0xFFFFFFFF));
#endif
} 

static void dsp_opcode_mtoi(void)
{
	RN = (((INT32)RM >> 8) & 0xFF800000) | (RM & 0x007FFFFF);
	SET_ZN(RN);
}

static void dsp_opcode_normi(void)
{
	uint32 _Rm = RM;
	uint32 res = 0;

	if (_Rm)
	{
		while ((_Rm & 0xffc00000) == 0)
		{
			_Rm <<= 1;
			res--;
		}
		while ((_Rm & 0xff800000) != 0)
		{
			_Rm >>= 1;
			res++;
		}
	}
	RN = res;
	SET_ZN(RN);
}

static void dsp_opcode_mmult(void)
{
	int count	= dsp_matrix_control&0x0f;
	uint32 addr = dsp_pointer_to_matrix; // in the gpu ram
	int64 accum = 0;
	uint32 res;

	if (!(dsp_matrix_control & 0x10))
	{
		for (int i = 0; i < count; i++)
		{ 
			int16 a;
			if (i&0x01)
				a=(int16)((dsp_alternate_reg[dsp_opcode_first_parameter + (i>>1)]>>16)&0xffff);
			else
				a=(int16)(dsp_alternate_reg[dsp_opcode_first_parameter + (i>>1)]&0xffff);
			int16 b=((int16)DSPReadWord(addr + 2, DSP));
			accum += a*b;
			addr += 4;
		}
	}
	else
	{
		for (int i = 0; i < count; i++)
		{
			int16 a;
			if (i&0x01)
				a=(int16)((dsp_alternate_reg[dsp_opcode_first_parameter + (i>>1)]>>16)&0xffff);
			else
				a=(int16)(dsp_alternate_reg[dsp_opcode_first_parameter + (i>>1)]&0xffff);
			int16 b=((int16)DSPReadWord(addr + 2, DSP));
			accum += a*b;
			addr += 4 * count;
		}
	}
	RN = res = (int32)accum;
	// carry flag to do
//NOTE: The flags are set based upon the last add/multiply done...
	SET_ZN(RN);
}

static void dsp_opcode_abs(void)
{
	uint32 _Rn = RN;
	uint32 res;
	
	if (_Rn == 0x80000000)
		dsp_flag_n = 1;
	else
	{
		dsp_flag_c = ((_Rn & 0x80000000) >> 31);
		res = RN = (_Rn & 0x80000000 ? -_Rn : _Rn);
		CLR_ZN; SET_Z(res);
	}
}

static void dsp_opcode_div(void)
{
	uint32 _Rm=RM;
	uint32 _Rn=RN;

	if (_Rm)
	{
		if (dsp_div_control & 1)
		{
			dsp_remain = (((uint64)_Rn) << 16) % _Rm;
			if (dsp_remain&0x80000000)
				dsp_remain-=_Rm;
			RN = (((uint64)_Rn) << 16) / _Rm;
		}
		else
		{
			dsp_remain = _Rn % _Rm;
			if (dsp_remain&0x80000000)
				dsp_remain-=_Rm;
			RN/=_Rm;
		}
	}
	else
		RN=0xffffffff;
}

static void dsp_opcode_imultn(void)
{
#ifdef DSP_DIS_IMULTN
	if (doDSPDis)
		WriteLog("%06X: IMULTN R%02u, R%02u [NCZ:%u%u%u, R%02u=%08X, R%02u=%08X] -> ", dsp_pc-2, IMM_1, IMM_2, dsp_flag_n, dsp_flag_c, dsp_flag_z, IMM_1, RM, IMM_2, RN);
#endif
	// This is OK, since this multiply won't overflow 32 bits...
	int32 res = (int32)((int16)RN * (int16)RM);
	dsp_acc = (int64)res;
	SET_ZN(res);
#ifdef DSP_DIS_IMULTN
	if (doDSPDis)
		WriteLog("[NCZ:%u%u%u, DSP_ACC=%02X%08X]\n", dsp_flag_n, dsp_flag_c, dsp_flag_z, (uint8)(dsp_acc >> 32), (uint32)(dsp_acc & 0xFFFFFFFF));
#endif
}

static void dsp_opcode_neg(void)
{
#ifdef DSP_DIS_NEG
	if (doDSPDis)
		WriteLog("%06X: NEG    R%02u [NCZ:%u%u%u, R%02u=%08X] -> ", dsp_pc-2, IMM_2, dsp_flag_n, dsp_flag_c, dsp_flag_z, IMM_2, RN);
#endif
	UINT32 res = -RN;
	SET_ZNC_SUB(0, RN, res);
	RN = res;
#ifdef DSP_DIS_NEG
	if (doDSPDis)
		WriteLog("[NCZ:%u%u%u, R%02u=%08X]\n", dsp_flag_n, dsp_flag_c, dsp_flag_z, IMM_2, RN);
#endif
}

static void dsp_opcode_shlq(void)
{
#ifdef DSP_DIS_SHLQ
	if (doDSPDis)
		WriteLog("%06X: SHLQ   #%u, R%02u [NCZ:%u%u%u, R%02u=%08X] -> ", dsp_pc-2, 32 - IMM_1, IMM_2, dsp_flag_n, dsp_flag_c, dsp_flag_z, IMM_2, RN);
#endif
	INT32 r1 = 32 - IMM_1;
	UINT32 res = RN << r1;
	SET_ZN(res); dsp_flag_c = (RN >> 31) & 1;
	RN = res;
#ifdef DSP_DIS_SHLQ
	if (doDSPDis)
		WriteLog("[NCZ:%u%u%u, R%02u=%08X]\n", dsp_flag_n, dsp_flag_c, dsp_flag_z, IMM_2, RN);
#endif
}

static void dsp_opcode_shrq(void)
{
#ifdef DSP_DIS_SHRQ
	if (doDSPDis)
		WriteLog("%06X: SHRQ   #%u, R%02u [NCZ:%u%u%u, R%02u=%08X] -> ", dsp_pc-2, dsp_convert_zero[IMM_1], IMM_2, dsp_flag_n, dsp_flag_c, dsp_flag_z, IMM_2, RN);
#endif
	INT32 r1 = dsp_convert_zero[IMM_1];
	UINT32 res = RN >> r1;
	SET_ZN(res); dsp_flag_c = RN & 1;
	RN = res;
#ifdef DSP_DIS_SHRQ
	if (doDSPDis)
		WriteLog("[NCZ:%u%u%u, R%02u=%08X]\n", dsp_flag_n, dsp_flag_c, dsp_flag_z, IMM_2, RN);
#endif
}

static void dsp_opcode_ror(void)
{
#ifdef DSP_DIS_ROR
	if (doDSPDis)
		WriteLog("%06X: ROR    R%02u, R%02u [NCZ:%u%u%u, R%02u=%08X, R%02u=%08X] -> ", dsp_pc-2, IMM_1, IMM_2, dsp_flag_n, dsp_flag_c, dsp_flag_z, IMM_1, RM, IMM_2, RN);
#endif
	UINT32 r1 = RM & 0x1F;
	UINT32 res = (RN >> r1) | (RN << (32 - r1));
	SET_ZN(res); dsp_flag_c = (RN >> 31) & 1;
	RN = res;
#ifdef DSP_DIS_ROR
	if (doDSPDis)
		WriteLog("[NCZ:%u%u%u, R%02u=%08X, R%02u=%08X]\n", dsp_flag_n, dsp_flag_c, dsp_flag_z, IMM_1, RM, IMM_2, RN);
#endif
}

static void dsp_opcode_rorq(void)
{
#ifdef DSP_DIS_RORQ
	if (doDSPDis)
		WriteLog("%06X: RORQ   #%u, R%02u [NCZ:%u%u%u, R%02u=%08X] -> ", dsp_pc-2, dsp_convert_zero[IMM_1], IMM_2, dsp_flag_n, dsp_flag_c, dsp_flag_z, IMM_2, RN);
#endif
	UINT32 r1 = dsp_convert_zero[IMM_1 & 0x1F];
	UINT32 r2 = RN;
	UINT32 res = (r2 >> r1) | (r2 << (32 - r1));
	RN = res;
	SET_ZN(res); dsp_flag_c = (r2 >> 31) & 0x01;
#ifdef DSP_DIS_RORQ
	if (doDSPDis)
		WriteLog("[NCZ:%u%u%u, R%02u=%08X]\n", dsp_flag_n, dsp_flag_c, dsp_flag_z, IMM_2, RN);
#endif
}

static void dsp_opcode_sha(void)
{
	int32 sRm=(int32)RM;
	uint32 _Rn=RN;

	if (sRm<0)
	{
		uint32 shift=-sRm;
		if (shift>=32) shift=32;
		dsp_flag_c=(_Rn&0x80000000)>>31;
		while (shift)
		{
			_Rn<<=1;
			shift--;
		}
	}
	else
	{
		uint32 shift=sRm;
		if (shift>=32) shift=32;
		dsp_flag_c=_Rn&0x1;
		while (shift)
		{
			_Rn=((int32)_Rn)>>1;
			shift--;
		}
	}
	RN = _Rn;
	SET_ZN(RN);
}

static void dsp_opcode_sharq(void)
{
#ifdef DSP_DIS_SHARQ
	if (doDSPDis)
		WriteLog("%06X: SHARQ  #%u, R%02u [NCZ:%u%u%u, R%02u=%08X] -> ", dsp_pc-2, dsp_convert_zero[IMM_1], IMM_2, dsp_flag_n, dsp_flag_c, dsp_flag_z, IMM_2, RN);
#endif
	UINT32 res = (INT32)RN >> dsp_convert_zero[IMM_1];
	SET_ZN(res); dsp_flag_c = RN & 0x01;
	RN = res;
#ifdef DSP_DIS_SHARQ
	if (doDSPDis)
		WriteLog("[NCZ:%u%u%u, R%02u=%08X]\n", dsp_flag_n, dsp_flag_c, dsp_flag_z, IMM_2, RN);
#endif
}

static void dsp_opcode_sh(void)
{
	int32 sRm=(int32)RM;
	uint32 _Rn=RN;

	if (sRm<0)
	{
		uint32 shift=(-sRm);
		if (shift>=32) shift=32;
		dsp_flag_c=(_Rn&0x80000000)>>31;
		while (shift)
		{
			_Rn<<=1;
			shift--;
		}
	}
	else
	{
		uint32 shift=sRm;
		if (shift>=32) shift=32;
		dsp_flag_c=_Rn&0x1;
		while (shift)
		{
			_Rn>>=1;
			shift--;
		}
	}
	RN = _Rn;
	SET_ZN(RN);
}

void dsp_opcode_addqmod(void)
{
#ifdef DSP_DIS_ADDQMOD
	if (doDSPDis)
		WriteLog("%06X: ADDQMOD #%u, R%02u [NCZ:%u%u%u, R%02u=%08X, DSP_MOD=%08X] -> ", dsp_pc-2, dsp_convert_zero[IMM_1], IMM_2, dsp_flag_n, dsp_flag_c, dsp_flag_z, IMM_2, RN, dsp_modulo);
#endif
	UINT32 r1 = dsp_convert_zero[IMM_1];
	UINT32 r2 = RN;
	UINT32 res = r2 + r1;
	res = (res & (~dsp_modulo)) | (r2 & dsp_modulo);
	RN = res;
	SET_ZNC_ADD(r2, r1, res);
#ifdef DSP_DIS_ADDQMOD
	if (doDSPDis)
		WriteLog("[NCZ:%u%u%u, R%02u=%08X]\n", dsp_flag_n, dsp_flag_c, dsp_flag_z, IMM_2, RN);
#endif
}

void dsp_opcode_subqmod(void)	
{
	UINT32 r1 = dsp_convert_zero[IMM_1];
	UINT32 r2 = RN;
	UINT32 res = r2 - r1;
	res = (res & (~dsp_modulo)) | (r2 & dsp_modulo);
	RN = res;
	
	SET_ZNC_SUB(r2, r1, res);
}

void dsp_opcode_mirror(void)	
{
	UINT32 r1 = RN;
	RN = (mirror_table[r1 & 0xFFFF] << 16) | mirror_table[r1 >> 16];
	SET_ZN(RN);
}

void dsp_opcode_sat32s(void)		
{
	INT32 r2 = (UINT32)RN;
	INT32 temp = dsp_acc >> 32;
	UINT32 res = (temp < -1) ? (INT32)0x80000000 : (temp > 0) ? (INT32)0x7FFFFFFF : r2;
	RN = res;
	SET_ZN(res);
}

void dsp_opcode_sat16s(void)		
{
	INT32 r2 = RN;
	UINT32 res = (r2 < -32768) ? -32768 : (r2 > 32767) ? 32767 : r2;
	RN = res;
	SET_ZN(res);
}
