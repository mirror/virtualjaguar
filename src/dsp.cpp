//
// DSP core
//
// by cal2
// GCC/SDL port by Niels Wagenaar (Linux/WIN32) and Caz (BeOS)
// Cleanups by James L. Hammons
//

#include "dsp.h"

#define CINT0FLAG			0x00200
#define CINT1FLAG			0x00400
#define CINT2FLAG			0x00800
#define CINT3FLAG			0x01000
#define CINT4FLAG			0x02000
#define CINT04FLAGS			(CINT0FLAG | CINT1FLAG | CINT2FLAG | CINT3FLAG | CINT4FLAG)
#define CINT5FLAG			0x20000		/* DSP only */

extern uint32 jaguar_mainRom_crc32;

static void dsp_opcode_addqmod(void);	
static void dsp_opcode_subqmod(void);	
static void dsp_opcode_mirror(void);	
static void dsp_opcode_sat32s(void);	
static void dsp_opcode_sat16s(void);	
static void dsp_opcode_add(void);
static void dsp_opcode_addc(void);
static void dsp_opcode_addq(void);
static void dsp_opcode_addqt(void);
static void dsp_opcode_sub(void);
static void dsp_opcode_subc(void);
static void dsp_opcode_subq(void);
static void dsp_opcode_subqt(void);
static void dsp_opcode_neg(void);
static void dsp_opcode_and(void);
static void dsp_opcode_or(void);
static void dsp_opcode_xor(void);
static void dsp_opcode_not(void);
static void dsp_opcode_btst(void);
static void dsp_opcode_bset(void);
static void dsp_opcode_bclr(void);
static void dsp_opcode_mult(void);
static void dsp_opcode_imult(void);
static void dsp_opcode_imultn(void);
static void dsp_opcode_resmac(void);
static void dsp_opcode_imacn(void);
static void dsp_opcode_div(void);
static void dsp_opcode_abs(void);
static void dsp_opcode_sh(void);
static void dsp_opcode_shlq(void);
static void dsp_opcode_shrq(void);
static void dsp_opcode_sha(void);
static void dsp_opcode_sharq(void);
static void dsp_opcode_ror(void);
static void dsp_opcode_rorq(void);
static void dsp_opcode_cmp(void);
static void dsp_opcode_cmpq(void);
static void dsp_opcode_sat8(void);
static void dsp_opcode_sat16(void);
static void dsp_opcode_move(void);
static void dsp_opcode_moveq(void);
static void dsp_opcode_moveta(void);
static void dsp_opcode_movefa(void);
static void dsp_opcode_movei(void);
static void dsp_opcode_loadb(void);
static void dsp_opcode_loadw(void);
static void dsp_opcode_load(void);
static void dsp_opcode_loadp(void);
static void dsp_opcode_load_r14_indexed(void);
static void dsp_opcode_load_r15_indexed(void);
static void dsp_opcode_storeb(void);
static void dsp_opcode_storew(void);
static void dsp_opcode_store(void);
static void dsp_opcode_storep(void);
static void dsp_opcode_store_r14_indexed(void);
static void dsp_opcode_store_r15_indexed(void);
static void dsp_opcode_move_pc(void);
static void dsp_opcode_jump(void);
static void dsp_opcode_jr(void);
static void dsp_opcode_mmult(void);
static void dsp_opcode_mtoi(void);
static void dsp_opcode_normi(void);
static void dsp_opcode_nop(void);
static void dsp_opcode_load_r14_ri(void);
static void dsp_opcode_load_r15_ri(void);
static void dsp_opcode_store_r14_ri(void);
static void dsp_opcode_store_r15_ri(void);
static void dsp_opcode_sat24(void);

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

void (*dsp_opcode[64])()= 
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

char *dsp_opcode_str[64]= 
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

static uint16	*mirror_table;
static uint8	*dsp_ram_8;

static uint32	dsp_pc;
static uint32	dsp_acc;
static uint32	dsp_remain;
static uint32	dsp_modulo;
static uint32	dsp_flags;
static uint32	dsp_matrix_control;
static uint32	dsp_pointer_to_matrix;
static uint32	dsp_data_organization;
static uint32	dsp_control;
static uint32	dsp_div_control;
static uint8	dsp_flag_z;
static uint8	dsp_flag_n;
static uint8	dsp_flag_c;    
static uint8	dsp_alternate_flag_z;
static uint8	dsp_alternate_flag_n;
static uint8	dsp_alternate_flag_c;    
static uint32	*dsp_reg;
static uint32	*dsp_alternate_reg;
static uint32	*dsp_reg_bank_0;
static uint32	*dsp_reg_bank_1;

static uint32	dsp_opcode_first_parameter;
static uint32	dsp_opcode_second_parameter;

#define dsp_running (dsp_control&0x01)

#define Rm dsp_reg[dsp_opcode_first_parameter]
#define Rn dsp_reg[dsp_opcode_second_parameter]
#define alternate_Rm dsp_alternate_reg[dsp_opcode_first_parameter]
#define alternate_Rn dsp_alternate_reg[dsp_opcode_second_parameter]
#define imm_1 dsp_opcode_first_parameter
#define imm_2 dsp_opcode_second_parameter

#define set_flag_z(r) dsp_flag_z=(r==0); 
#define set_flag_n(r) dsp_flag_n=(r&0x80000000);

#define reset_flag_z()	dsp_flag_z=0;
#define reset_flag_n()	dsp_flag_n=0;
#define reset_flag_c()	dsp_flag_c=0;    

uint32 dsp_convert_zero[32] = { 32,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31 };
uint8 *dsp_branch_condition_table=0;
#define branch_condition(x)	dsp_branch_condition_table[(x) + ((jaguar_flags & 7) << 5)]

static uint32 dsp_in_exec = 0;
static uint32 dsp_releaseTimeSlice_flag = 0;

FILE * dsp_fp;


void dsp_reset_stats(void)
{
	for (uint32 i=0; i<64; i++)
		dsp_opcode_use[i] = 0;
}

void dsp_releaseTimeslice(void)
{
	dsp_releaseTimeSlice_flag = 1;
}

void dsp_build_branch_condition_table(void)
{
	int i,j;
	#define ZFLAG				0x00001
	#define CFLAG				0x00002
	#define NFLAG				0x00004
	
	/* allocate the mirror table */
	if (!mirror_table)
		mirror_table = (uint16*)malloc(65536 * sizeof(mirror_table[0]));

	/* fill in the mirror table */
	if (mirror_table)
		for (int i = 0; i < 65536; i++)
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
		dsp_branch_condition_table = (uint8*)malloc(32 * 8 * sizeof(dsp_branch_condition_table[0]));

		/* fill in the condition table */
		if (dsp_branch_condition_table)
			for (i = 0; i < 8; i++)
				for (j = 0; j < 32; j++)
				{
					int result = 1;
					if (j & 1)
						if (i & ZFLAG) result = 0;
					if (j & 2)
						if (!(i & ZFLAG)) result = 0;
					if (j & 4)
						if (i & (CFLAG << (j >> 4))) result = 0;
					if (j & 8)
						if (!(i & (CFLAG << (j >> 4)))) result = 0;
					dsp_branch_condition_table[i * 32 + j] = result;
				}
	}
}

unsigned dsp_byte_read(unsigned int offset)
{
// battlemorph
//	if ((offset==0xF1CFE0)||(offset==0xF1CFE2))
//		return(0xffff);
	// mutant penguin
	if ((jaguar_mainRom_crc32==0xbfd751a4)||(jaguar_mainRom_crc32==0x053efaf9))
	{
		if (offset==0xF1CFE0)
			return(0xff);
	}
	if ((offset>=dsp_work_ram_base)&&(offset<dsp_work_ram_base+0x2000))
		return(dsp_ram_8[offset-dsp_work_ram_base]);

	if ((offset>=dsp_control_ram_base)&&(offset<dsp_control_ram_base+0x20))
	{
		uint32 data=dsp_long_read(offset&0xfffffffc);

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
	return(jaguar_byte_read(offset));
} 

unsigned dsp_word_read(unsigned int offset)
{
	offset&=0xFFFFFFFE;
	// jaguar cd bios
	if (jaguar_mainRom_crc32==0xa74a97cd)
	{
		if (offset==0xF1A114) return(0x0000);
		if (offset==0xF1A116) return(0x0000);
		if (offset==0xF1B000) return(0x1234);
		if (offset==0xF1B002) return(0x5678);
	}
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
	if (jaguar_mainRom_crc32==0x4faddb18)
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
	}

	if ((offset>=dsp_work_ram_base)&&(offset<dsp_work_ram_base+0x2000))
	{
		offset-=dsp_work_ram_base;
		uint16 data=(((uint16)dsp_ram_8[offset])<<8)|((uint16)dsp_ram_8[offset+1]);
		return(data);
	}
	if ((offset>=dsp_control_ram_base)&&(offset<dsp_control_ram_base+0x20))
	{
		uint32 data;

		data=dsp_long_read(offset&0xfffffffc);

		if (offset&0x03)
			return(data&0xffff);
		else
			return(data>>16);
	}
	return(jaguar_word_read(offset));
}

unsigned dsp_long_read(unsigned int offset)
{
	offset&=0xFFFFFFFC;

	if ((offset>=dsp_work_ram_base)&&(offset<dsp_work_ram_base+0x2000))
	{
		offset-=dsp_work_ram_base;
		uint32 data= (((uint32)dsp_ram_8[offset]  )<<24)|
			         (((uint32)dsp_ram_8[offset+1])<<16)|
					 (((uint32)dsp_ram_8[offset+2])<<8 )|
					 ( (uint32)dsp_ram_8[offset+3]);
		return(data);
	}
	if ((offset>=dsp_control_ram_base)&&(offset<dsp_control_ram_base+0x20))
	{
		offset&=0x1f;
		switch (offset)
		{
		case 0x00:	dsp_flag_c?(dsp_flag_c=1):(dsp_flag_c=0);
					dsp_flag_z?(dsp_flag_z=1):(dsp_flag_z=0);
					dsp_flag_n?(dsp_flag_n=1):(dsp_flag_n=0);

					dsp_flags=(dsp_flags&0xFFFFFFF8)|(dsp_flag_n<<2)|(dsp_flag_c<<1)|dsp_flag_z;
					
					return(dsp_flags&0xFFFFC1FF);
		case 0x04: return(dsp_matrix_control);
		case 0x08: return(dsp_pointer_to_matrix);
		case 0x0C: return(dsp_data_organization);
		case 0x10: return(dsp_pc);
		case 0x14: return(dsp_control);
		case 0x18: return(dsp_modulo);
		case 0x1C: return(dsp_remain);
//		default:   // unaligned long read //__asm int 3
		}
		return(0xfffffff);
	}
	uint32 data=jaguar_word_read(offset);
	data<<=16;
	data|=jaguar_word_read(offset+2);
	return(data);
}

void dsp_byte_write(unsigned offset, unsigned data)
{
	if ((offset >= dsp_work_ram_base) && (offset < dsp_work_ram_base+0x2000))
	{
		offset -= dsp_work_ram_base;
		dsp_ram_8[offset] = data;
		if (dsp_in_exec == 0)
		{
//			s68000releaseTimeslice();
			m68k_end_timeslice();
			gpu_releaseTimeslice();
		}
		return;
	}
	if ((offset >= dsp_control_ram_base) && (offset < dsp_control_ram_base+0x20))
	{
		uint32 reg = offset & 0x1C;
		int bytenum = offset & 0x03;
		
		if ((reg >= 0x1C) && (reg <= 0x1F))
			dsp_div_control = (dsp_div_control & (~(0xFF << (bytenum << 3)))) | (data << (bytenum << 3));
		else
		{
			uint32 old_data = dsp_long_read(offset&0xFFFFFFC);
			bytenum = 3 - bytenum; // convention motorola !!!
			old_data = (old_data & (~(0xFF << (bytenum << 3)))) | (data << (bytenum << 3));	
			dsp_long_write(offset & 0xFFFFFFC, old_data);
		}
		return;
	}
//	fprintf(log_get(),"dsp: writing %.2x at 0x%.8x\n",data,offset);
	jaguar_byte_write(offset, data);
}

void dsp_word_write(unsigned offset, unsigned data)
{
	offset &= 0xFFFFFFFE;
//	fprintf(log_get(),"dsp: writing %.4x at 0x%.8x\n",data,offset);
	if ((offset >= dsp_work_ram_base) && (offset < dsp_work_ram_base+0x2000))
	{
		offset -= dsp_work_ram_base;
		dsp_ram_8[offset] = data >> 8;
		dsp_ram_8[offset+1] = data & 0xFF;
		if (dsp_in_exec == 0)
		{
//			fprintf(log_get(),"dsp: writing %.4x at 0x%.8x\n",data,offset+dsp_work_ram_base);
//			s68000releaseTimeslice();
			m68k_end_timeslice();
			gpu_releaseTimeslice();
		}
		return;
	}
	else if ((offset >= dsp_control_ram_base) && (offset < dsp_control_ram_base+0x20))
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
			uint32 old_data = dsp_long_read(offset & 0xffffffc);
			if (offset & 0x03)
				old_data = (old_data&0xffff0000)|(data&0xffff);
			else
				old_data = (old_data&0xffff)|((data&0xffff)<<16);
			dsp_long_write(offset & 0xffffffc, old_data);
		}
		return;
	}
	jaguar_word_write(offset, data);
}

void dsp_long_write(unsigned offset, unsigned data)
{
	offset &= 0xFFFFFFFC;
//	fprintf(log_get(),"dsp: writing %.8x at 0x%.8x\n",data,offset);
	if ((offset >= dsp_work_ram_base) && (offset < dsp_work_ram_base+0x2000))
	{
		offset -= dsp_work_ram_base;
		dsp_ram_8[offset]  = data >> 24;
		dsp_ram_8[offset+1] = (data>>16) & 0xFF;
		dsp_ram_8[offset+2] = (data>>8) & 0xFF;
		dsp_ram_8[offset+3] = data & 0xFF;
		return;
	}
	else if ((offset >= dsp_control_ram_base) && (offset < dsp_control_ram_base+0x20))
	{
		offset&=0x1f;
		switch (offset)
		{
		case 0x00:
			dsp_flags = data;
			dsp_flag_z = dsp_flags & 0x01;
			dsp_flag_c = (dsp_flags>>1) & 0x01;
			dsp_flag_n = (dsp_flags>>2) & 0x01;
			dsp_update_register_banks();
			dsp_control &= ~((dsp_flags & CINT04FLAGS) >> 3);
			dsp_control &= ~((dsp_flags & CINT5FLAG) >> 1);
			dsp_check_irqs();
			break;
		case 0x04:
			dsp_matrix_control = data;
			break;
		case 0x08:
			dsp_pointer_to_matrix = data;
			break;
		case 0x0C:
			dsp_data_organization = data;
			break;
		case 0x10:
			dsp_pc = data;
			break;
		case 0x14:
		{	
			uint32 dsp_was_running = dsp_running;
			// check for DSP->CPU interrupt
			if (data & 0x02)
			{
//							fprintf(log_get(),"DSP->CPU interrupt\n");
				if (tom_irq_enabled(IRQ_GPU) && jaguar_interrupt_handler_is_valid(64))
				{
					tom_set_pending_gpu_int();
//					s68000interrupt(7,64);
//					s68000flushInterrupts();
					m68k_set_irq(7);			// Set 68000 NMI...
					dsp_releaseTimeslice();
				}
				data &= ~(0x02);
			}
			// check for CPU->DSP interrupt
			if (data & 0x04)
			{
//							fprintf(log_get(),"CPU->DSP interrupt\n");
				dsp_set_irq_line(0, 1);
//				s68000releaseTimeslice();
				m68k_end_timeslice();
				gpu_releaseTimeslice();
				data &= ~(0x04);
			}
			// single stepping
			if (data & 0x10)
			{
							//fprintf(log_get(),"asked to perform a single step (single step is %senabled)\n",(data&0x8)?"":"not ");
			}
			dsp_control = data;

			// if dsp wasn't running but is now running
			// execute a few cycles
#ifndef DSP_SINGLE_STEPPING
			if (!dsp_was_running && dsp_running)
			{
				dsp_exec(200);
			}
#else
			if (dsp_control & 0x18)
				dsp_exec(1);
#endif
			break;
		}
		case 0x18:
			dsp_modulo = data;
			break;
		case 0x1C:
			dsp_div_control=data;
			break;
//		default:   // unaligned long read
				   //__asm int 3
		}
		return;
	}
	jaguar_word_write(offset, (data>>16) & 0xFFFF);
	jaguar_word_write(offset+2, data & 0xFFFF);
}

uint8 * jaguar_rom_load(char * path, uint32 * romSize);
void dsp_load_bin_at(char * path, uint32 offset)
{
	uint32 romSize;
	uint8 * rom;

	rom = jaguar_rom_load(path, &romSize);
	for(uint32 i=0; i<romSize; i++)
		jaguar_byte_write(offset+i, rom[i]);
}

static int go = 0;
void dsp_update_register_banks(void)
{
	uint32 temp;
	int i, bank;

	bank = dsp_flags&0x4000;

	if (dsp_flags & 0x8) 
		bank = 0;
	
	if ((!bank && (dsp_reg_bank_0 != dsp_reg)) || (bank && (dsp_reg_bank_1 != dsp_reg)))
	{
		for (i = 0; i < 32; i++)
		{
			temp = dsp_reg[i];
			dsp_reg[i] = dsp_alternate_reg[i];
			dsp_alternate_reg[i]= temp;
		}
		// switch flags
		temp=dsp_flag_z;
		dsp_flag_z=dsp_alternate_flag_z;
		dsp_alternate_flag_z=temp;

		temp=dsp_flag_n;
		dsp_flag_n=dsp_alternate_flag_n;
		dsp_alternate_flag_n=temp;

		temp=dsp_flag_c;
		dsp_flag_c=dsp_alternate_flag_c;
		dsp_alternate_flag_c=temp;
	
		if (!bank)
		{
			dsp_reg_bank_0 = dsp_reg;
			dsp_reg_bank_1 = dsp_alternate_reg;
		}
		else
		{
			dsp_reg_bank_0 = dsp_alternate_reg;
			dsp_reg_bank_1 = dsp_reg;
		}
	}
}

void dsp_check_if_i2s_interrupt_needed(void)
{
	static uint32 count=8;

	int mask;
	count--;
	if (count>0)
		return;

	count=4;
	// already in an interrupt handler ?
	if (dsp_flags & 0x8) 
		return;

	// get the interrupt mask 
	mask = (dsp_flags >> 4) & 0x1f;
	mask |= (dsp_flags >> 11) & 0x20;

	if (mask & 0x02)
	{
		dsp_set_irq_line(1,1);
	}
}

void dsp_check_irqs(void)
{
	int bits, mask, which = 0;

	if (dsp_flags & 0x8) 
		return;

	// get the active interrupt bits 
	bits = (dsp_control >> 6) & 0x1f;
	bits |= (dsp_control >> 10) & 0x20;

	// get the interrupt mask 
	mask = (dsp_flags >> 4) & 0x1f;
	mask |= (dsp_flags >> 11) & 0x20;

//	fprintf(log_get(),"dsp: bits=%.2x mask=%.2x\n",bits,mask);
	// bail if nothing is available
	bits &= mask;

	if (!bits)
		return;

	// determine which interrupt 
	if (bits & 0x01) which = 0;
	if (bits & 0x02) which = 1;
	if (bits & 0x04) which = 2;
	if (bits & 0x08) which = 3;
	if (bits & 0x10) which = 4;
	if (bits & 0x20) which = 5;

//	fprintf(log_get(),"dsp: generating interrupt %i\n",which);

	dsp_flags |= 0x08;
	dsp_update_register_banks();

	// subqt  #4,r31		; pre-decrement stack pointer 
	// move  pc,r30			; address of interrupted code 
	// store  r30,(r31)     ; store return address
	dsp_reg[31] -= 4;
	dsp_reg[30]=dsp_pc;
	dsp_long_write(dsp_reg[31], dsp_pc - 2);

	// movei  #service_address,r30  ; pointer to ISR entry 
	// jump  (r30)					; jump to ISR 
	// nop
	dsp_pc = dsp_work_ram_base;
	dsp_pc += which * 0x10;
	dsp_reg[30]=dsp_pc;
}

void dsp_set_irq_line(int irqline, int state)
{
	int mask = 0x40 << irqline;
	dsp_control &= ~mask;
	if (state)
	{
		dsp_control |= mask;
		dsp_check_irqs();
	}
}

void dsp_init(void)
{
	memory_malloc_secure((void **)&dsp_ram_8, 0x2000, "dsp work ram");
	memory_malloc_secure((void **)&dsp_reg, 32*sizeof(int32), "dsp bank 0 regs");
	memory_malloc_secure((void **)&dsp_alternate_reg, 32*sizeof(int32), "dsp bank 1 regs");
	
	dsp_build_branch_condition_table();
	dsp_reset();
}

void dsp_reset(void)
{
	dsp_pc				  = 0x00f1b000;
	dsp_acc				  = 0x00000000;
	dsp_remain			  = 0x00000000;
	dsp_modulo			  = 0xFFFFFFFF;
	dsp_flags			  = 0x00040000;
	dsp_matrix_control    = 0x00000000;
	dsp_pointer_to_matrix = 0x00000000;
	dsp_data_organization = 0xffffffff;
	dsp_control			  = 0x00012800;
	dsp_div_control		  = 0x00000000;
	dsp_in_exec			  = 0;

	for(int i=0; i<32; i++)
	{
		dsp_reg[i]           = 0x00000000;
		dsp_alternate_reg[i] = 0x00000000;
	}
	
	dsp_reg_bank_0 = dsp_reg;
	dsp_reg_bank_1 = dsp_alternate_reg;
//	dsp_reg_bank_1 = dsp_reg;
//	dsp_reg_bank_0 = dsp_alternate_reg;

	reset_flag_z();
	reset_flag_n();
	reset_flag_c();

	dsp_alternate_flag_z = 0;
	dsp_alternate_flag_n = 0;
	dsp_alternate_flag_c = 0;

	dsp_reset_stats();
	memset(dsp_ram_8, 0xFF, 0x2000);
}

void dsp_done(void)
{
	int i, j;
	fprintf(log_get(),"dsp: stopped at pc=0x%.8x dsp_modulo=0x%.8x(dsp %s running)\n",dsp_pc,dsp_modulo,dsp_running?"was":"wasn't");
	fprintf(log_get(),"dsp: %sin interrupt handler\n",(dsp_flags & 0x8)?"":"not ");
	int bits, mask;

	// get the active interrupt bits 
	bits = (dsp_control >> 6) & 0x1f;
	bits |= (dsp_control >> 10) & 0x20;

	// get the interrupt mask 
	mask = (dsp_flags >> 4) & 0x1f;
	mask |= (dsp_flags >> 11) & 0x20;

	fprintf(log_get(),"dsp: bits=0x%.8x mask=0x%.8x\n",bits,mask);
	fprintf(log_get(),"\nregisters bank 0\n");
	for (int j=0;j<8;j++)
	{
		fprintf(log_get(),"\tr%2i=0x%.8x r%2i=0x%.8x r%2i=0x%.8x r%2i=0x%.8x\n",
						  (j<<2)+0,dsp_reg[(j<<2)+0],
						  (j<<2)+1,dsp_reg[(j<<2)+1],
						  (j<<2)+2,dsp_reg[(j<<2)+2],
						  (j<<2)+3,dsp_reg[(j<<2)+3]);

	}
//	fprintf(log_get(),"registers bank 1\n");
//	for (j=0;j<8;j++)
//	{
//		fprintf(log_get(),"\tr%2i=0x%.8x r%2i=0x%.8x r%2i=0x%.8x r%2i=0x%.8x\n",
//						  (j<<2)+0,dsp_alternate_reg[(j<<2)+0],
//						  (j<<2)+1,dsp_alternate_reg[(j<<2)+1],
//						  (j<<2)+2,dsp_alternate_reg[(j<<2)+2],
//						  (j<<2)+3,dsp_alternate_reg[(j<<2)+3]);
//
//	}
	static char buffer[512];
	j=dsp_work_ram_base;
	for (int i=0;i<4096;i++)
	{
		uint32 oldj=j;
		j+=dasmjag(JAGUAR_DSP,buffer,j);
//		fprintf(log_get(),"\t0x%.8x: %s\n",oldj,buffer);
	}

/*
	fprintf(log_get(),"dsp opcodes use:\n");
	for (i=0;i<64;i++)
	{
		if (dsp_opcode_use[i])
			fprintf(log_get(),"\t%s %i\n",dsp_opcode_str[i],dsp_opcode_use[i]);
	}
*/
	memory_free(dsp_ram_8);
}

void dsp_exec(int32 cycles)
{
	if ((cycles!=1)&&(jaguar_mainRom_crc32==0xba74c3ed))
		dsp_check_if_i2s_interrupt_needed();

#ifdef DSP_SINGLE_STEPPING
	if (dsp_control&0x18)
	{
		cycles=1;
		dsp_control&=~0x10;
	}
#endif
	dsp_check_irqs();
	dsp_releaseTimeSlice_flag=0;
	dsp_in_exec++;
	while ((cycles>0)&&(dsp_running))
	{
		dsp_flag_c=(dsp_flag_c?1:0);
		dsp_flag_z=(dsp_flag_z?1:0);
		dsp_flag_n=(dsp_flag_n?1:0);


/*
		if (dsp_pc==0x00f1b02c)
		{
			if (dsp_fp)
			{
				fclose(dsp_fp);
				exit(0);
			}
		}
		else 
		if (dsp_pc==0x00f1b032)
		{
			dsp_fp=fopen("c:\\bad.log","wrt");
			if (dsp_fp==NULL)
				MessageBox(NULL,"Cannot open dsp log","",MB_OK);
		}
		if (dsp_fp)
		{
			fprintf(dsp_fp,"0x%.8x\n",dsp_pc);
		}
		
//		if (dsp_pc==0x00f1b130)
//			dsp_load_bin_at("SCHRIFT.DAT",0x120000);
*/		
		uint16 opcode=dsp_word_read(dsp_pc);
		dsp_opcode_first_parameter=(opcode&0x3e0)>>5;
		dsp_opcode_second_parameter=(opcode&0x1f);
		dsp_pc+=2;
		dsp_opcode[opcode>>10]();
		dsp_opcode_use[opcode>>10]++;
		cycles-=dsp_opcode_cycles[opcode>>10];
	}
	dsp_in_exec--;
}

static void dsp_opcode_jump(void)
{
	uint32 delayed_pc = Rm;
	uint32 jaguar_flags;

	// normalize flags
	dsp_flag_c=dsp_flag_c?1:0;
	dsp_flag_z=dsp_flag_z?1:0;
	dsp_flag_n=dsp_flag_n?1:0;

	jaguar_flags=(dsp_flag_n<<2)|(dsp_flag_c<<1)|dsp_flag_z;

	if (branch_condition(imm_2))
	{
		dsp_exec(1);
		dsp_pc=delayed_pc;
	}
}

static void dsp_opcode_jr(void)
{
	int32 offset=(imm_1&0x10) ? (0xFFFFFFF0|imm_1) : imm_1;

	int32 delayed_pc = dsp_pc + (offset * 2);
	uint32 jaguar_flags;

	// normalize flags
	dsp_flag_c=dsp_flag_c?1:0;
	dsp_flag_z=dsp_flag_z?1:0;
	dsp_flag_n=dsp_flag_n?1:0;
	
	jaguar_flags=(dsp_flag_n<<2)|(dsp_flag_c<<1)|dsp_flag_z;

	if (branch_condition(imm_2))
	{
		dsp_exec(1);
		dsp_pc=delayed_pc;
	}
}

static void dsp_opcode_add(void)
{
	uint32 _Rm=Rm;
	uint32 _Rn=Rn;
	uint32 res;
#ifdef __PORT__

    /*
       GCC on WIN32 (more importantly mingw) doesn't know the declared
       variables in asm until we put a _ before it.
       
       So the declaration dsp_flag_c needs to be _dsp_flag_c on mingw.
    */
    #ifdef __GCCWIN32__
	asm(
	"addl %1, %2
    setc  _dsp_flag_c
	setz  _dsp_flag_z
	sets  _dsp_flag_n
	movl %%eax, %0
	"
	: "=m"(res)
	: "d"(_Rm), "a"(_Rn));
	
	#else
	asm(
	"addl %1, %2
	setc  dsp_flag_c
	setz  dsp_flag_z
	sets  dsp_flag_n
	movl %%eax, %0
	"
	: "=m"(res)
	: "d"(_Rm), "a"(_Rn));
	#endif
	
#else
	__asm 
	{
		mov   edx,_Rm
		mov   eax,_Rn
		add   eax,edx
		setc  [dsp_flag_c]
		setz  [dsp_flag_z]
		sets  [dsp_flag_n]
		mov	  res,eax
	};
#endif
	Rn=res;
}

static void dsp_opcode_addc(void)
{
	uint32 _Rm=Rm;
	uint32 _Rn=Rn;
	uint32 res;
#ifdef __PORT__

    /*
       GCC on WIN32 (more importantly mingw) doesn't know the declared
       variables in asm until we put a _ before it.
       
       So the declaration dsp_flag_c needs to be _dsp_flag_c on mingw.
       We use __GCCWIN32__ for this "bug".
    */

    #ifdef __GCCWIN32__	
	
    asm(
	"addl %1, %2
	cmp	  $0, _dsp_flag_c
	clc
	jz 1f
	stc
	1:
	adc %1, %2
	setc  _dsp_flag_c
	setz  _dsp_flag_z
	sets  _dsp_flag_n
	movl %%eax, %0
	"
	: "=m"(res)
	: "d"(_Rm), "a"(_Rn));
	#else

    asm(
	"addl %1, %2
	cmp	  $0, dsp_flag_c
	clc
	jz 1f
	stc
	1:
	adc %1, %2
	setc  dsp_flag_c
	setz  dsp_flag_z
	sets  dsp_flag_n
	movl %%eax, %0
	"
	: "=m"(res)
	: "d"(_Rm), "a"(_Rn));

    #endif	
	
#else
	__asm 
	{
		mov   edx,_Rm
		mov   eax,_Rn
		cmp	  [dsp_flag_c],0
		clc
		jz	  dsp_opcode_addc_no_carry
		stc
dsp_opcode_addc_no_carry:
		adc   eax,edx
		setc  [dsp_flag_c]
		setz  [dsp_flag_z]
		sets  [dsp_flag_n]
		mov	  res,eax
	};
#endif
	Rn=res;
}

static void dsp_opcode_addq(void)
{
	uint32 _Rn=Rn;
	uint32 _Rm=dsp_convert_zero[imm_1];
	uint32 res;
	
#ifdef __PORT__

    /*
       GCC on WIN32 (more importantly mingw) doesn't know the declared
       variables in asm until we put a _ before it.
       
       So the declaration dsp_flag_c needs to be _dsp_flag_c on mingw.
    */

    #ifdef __GCCWIN32__
    
	asm(
	"addl %1, %2
	setc  _dsp_flag_c
	setz  _dsp_flag_z
	sets  _dsp_flag_n
	movl %%eax, %0
	"
	: "=m"(res)
	: "d"(_Rm), "a"(_Rn));
	
	#else
	
	asm(
	"addl %1, %2
	setc  dsp_flag_c
	setz  dsp_flag_z
	sets  dsp_flag_n
	movl %%eax, %0
	"
	: "=m"(res)
	: "d"(_Rm), "a"(_Rn));

	#endif
	
#else
	__asm 
	{
		mov   edx,_Rm
		mov   eax,_Rn
		add   eax,edx
		setc  [dsp_flag_c]
		setz  [dsp_flag_z]
		sets  [dsp_flag_n]
		mov	  res,eax
	};
#endif
	Rn=res;
}

static void dsp_opcode_sub(void)
{
	uint32 _Rm=Rm;
	uint32 _Rn=Rn;
	uint32 res;
#ifdef __PORT__

    /*
       GCC on WIN32 (more importantly mingw) doesn't know the declared
       variables in asm until we put a _ before it.
       
       So the declaration dsp_flag_c needs to be _dsp_flag_c on mingw.
    */

    #ifdef __GCCWIN32__
    
	asm(
	"subl %1, %2
	setc  _dsp_flag_c
	setz  _dsp_flag_z
	sets  _dsp_flag_n
	movl %%eax, %0
	"
	: "=m"(res)
	: "d"(_Rm), "a"(_Rn));
	
	#else

	asm(
	"subl %1, %2
	setc  dsp_flag_c
	setz  dsp_flag_z
	sets  dsp_flag_n
	movl %%eax, %0
	"
	: "=m"(res)
	: "d"(_Rm), "a"(_Rn));

	#endif	
	
#else
	__asm 
	{
		mov   eax,_Rn
		mov   edx,_Rm
		sub   eax,edx
		setc  [dsp_flag_c]
		setz  [dsp_flag_z]
		sets  [dsp_flag_n]
		mov	  res,eax
	};
#endif
	Rn=res;
}

static void dsp_opcode_subc(void)
{
	uint32 _Rm=Rm;
	uint32 _Rn=Rn;
	uint32 res;
#ifdef __PORT__

    /*
       GCC on WIN32 (more importantly mingw) doesn't know the declared
       variables in asm until we put a _ before it.
       
       So the declaration dsp_flag_c needs to be _dsp_flag_c on mingw.
    */

    #ifdef __GCCWIN32__

	asm(
	"addl %1, %2
	cmp	  $0, _dsp_flag_c
	clc
	jz 1f
	stc
	1:
	sbb %1, %2
	setc  _dsp_flag_c
	setz  _dsp_flag_z
	sets  _dsp_flag_n
	movl %%eax, %0
	"
	: "=m"(res)
	: "d"(_Rm), "a"(_Rn));
	
	#else

	asm(
	"addl %1, %2
	cmp	  $0, dsp_flag_c
	clc
	jz 1f
	stc
	1:
	sbb %1, %2
	setc  dsp_flag_c
	setz  dsp_flag_z
	sets  dsp_flag_n
	movl %%eax, %0
	"
	: "=m"(res)
	: "d"(_Rm), "a"(_Rn));

	#endif
	
#else
	__asm 
	{
		mov   edx,_Rm
		mov   eax,_Rn
		cmp	  [dsp_flag_c],0
		clc
		jz	  dsp_opcode_subc_no_carry
		stc
dsp_opcode_subc_no_carry:
		sbb   eax,edx
		setc  [dsp_flag_c]
		setz  [dsp_flag_z]
		sets  [dsp_flag_n]
		mov	  res,eax
	};
#endif
	Rn=res;
}

static void dsp_opcode_subq(void)
{
	uint32 _Rm=dsp_convert_zero[imm_1];
	uint32 _Rn=Rn;
	uint32 res;
#ifdef __PORT__

    /*
       GCC on WIN32 (more importantly mingw) doesn't know the declared
       variables in asm until we put a _ before it.
       
       So the declaration dsp_flag_c needs to be _dsp_flag_c on mingw.
    */

    #ifdef __GCCWIN32__

	asm(
	"subl %1, %2
	setc  _dsp_flag_c
	setz  _dsp_flag_z
	sets  _dsp_flag_n
	movl %%eax, %0
	"
	: "=m"(res)
	: "d"(_Rm), "a"(_Rn));
	
	#else

	asm(
	"subl %1, %2
	setc  dsp_flag_c
	setz  dsp_flag_z
	sets  dsp_flag_n
	movl %%eax, %0
	"
	: "=m"(res)
	: "d"(_Rm), "a"(_Rn));
	
	#endif
#else
	__asm 
	{
		mov   eax,_Rn
		mov   edx,_Rm
		sub   eax,edx
		setc  [dsp_flag_c]
		setz  [dsp_flag_z]
		sets  [dsp_flag_n]
		mov	  res,eax
	};
#endif
	Rn=res;
}

static void dsp_opcode_cmp(void)
{
	uint32 _Rm=Rm;
	uint32 _Rn=Rn;
#ifdef __PORT__

    /*
       GCC on WIN32 (more importantly mingw) doesn't know the declared
       variables in asm until we put a _ before it.
       
       So the declaration dsp_flag_c needs to be _dsp_flag_c on mingw.
    */

    #ifdef __GCCWIN32__

	asm(
	"cmpl %0, %1
	setc  _dsp_flag_c
	setz  _dsp_flag_z
	sets  _dsp_flag_n
	"
	:
	: "d"(_Rm), "a"(_Rn));
	
	#else
	
	asm(
	"cmpl %0, %1
	setc  dsp_flag_c
	setz  dsp_flag_z
	sets  dsp_flag_n
	"
	:
	: "d"(_Rm), "a"(_Rn));
	
	#endif
	
#else
	__asm 
	{
		mov   eax,_Rn
		mov   edx,_Rm
		cmp   eax,edx
		setc  [dsp_flag_c]
		setz  [dsp_flag_z]
		sets  [dsp_flag_n]
	};
#endif
}

static void dsp_opcode_cmpq(void)
{
	static int32 sqtable[32] = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,-16,-15,-14,-13,-12,-11,-10,-9,-8,-7,-6,-5,-4,-3,-2,-1};
	int32 _Rm=sqtable[imm_1&0x1f];
	uint32 _Rn=Rn;
#ifdef __PORT__

    /*
       GCC on WIN32 (more importantly mingw) doesn't know the declared
       variables in asm until we put a _ before it.
       
       So the declaration dsp_flag_c needs to be _dsp_flag_c on mingw.
    */

    #ifdef __GCCWIN32__

	asm(
	"cmpl %0, %1
	setc  _dsp_flag_c
	setz  _dsp_flag_z
	sets  _dsp_flag_n
	"
	:
	: "d"(_Rm), "a"(_Rn));
	
	#else

	asm(
	"cmpl %0, %1
	setc  dsp_flag_c
	setz  dsp_flag_z
	sets  dsp_flag_n
	"
	:
	: "d"(_Rm), "a"(_Rn));
	
	#endif
	
#else
	__asm 
	{
		mov   eax,_Rn
		mov   edx,_Rm
		cmp   eax,edx
		setc  [dsp_flag_c]
		setz  [dsp_flag_z]
		sets  [dsp_flag_n]
	};
#endif
}

static void dsp_opcode_and(void)
{
	uint32 _Rm=Rm;
	uint32 _Rn=Rn;
	uint32 res;
#ifdef __PORT__

    /*
       GCC on WIN32 (more importantly mingw) doesn't know the declared
       variables in asm until we put a _ before it.
       
       So the declaration dsp_flag_c needs to be _dsp_flag_c on mingw.
    */

    #ifdef __GCCWIN32__

	asm(
	"andl %1, %2
	setz  _dsp_flag_z
	sets  _dsp_flag_n
	movl %%eax, %0
	"
	: "=m"(res)
	: "d"(_Rm), "a"(_Rn));
	
	#else

	asm(
	"andl %1, %2
	setz  dsp_flag_z
	sets  dsp_flag_n
	movl %%eax, %0
	"
	: "=m"(res)
	: "d"(_Rm), "a"(_Rn));
	
	#endif
	
#else
	__asm 
	{
		mov   eax,_Rn
		mov   edx,_Rm
		and   eax,edx
		setz  [dsp_flag_z]
		sets  [dsp_flag_n]
		mov   res,eax
	};
#endif
	Rn=res;
}

static void dsp_opcode_or(void)
{
	uint32 _Rm=Rm;
	uint32 _Rn=Rn;
	uint32 res;
#ifdef __PORT__

    /*
       GCC on WIN32 (more importantly mingw) doesn't know the declared
       variables in asm until we put a _ before it.
       
       So the declaration dsp_flag_c needs to be _dsp_flag_c on mingw.
    */

    #ifdef __GCCWIN32__

	asm(
	"orl %1, %2
	setz  _dsp_flag_z
	sets  _dsp_flag_n
	movl %%eax, %0
	"
	: "=m"(res)
	: "d"(_Rm), "a"(_Rn));
	
	#else
	
	asm(
	"orl %1, %2
	setz  dsp_flag_z
	sets  dsp_flag_n
	movl %%eax, %0
	"
	: "=m"(res)
	: "d"(_Rm), "a"(_Rn));
	
	#endif

#else
	__asm 
	{
		mov   eax,_Rn
		mov   edx,_Rm
		or    eax,edx
		setz  [dsp_flag_z]
		sets  [dsp_flag_n]
		mov   res,eax
	};
#endif
	Rn=res;
}

static void dsp_opcode_xor(void)
{
	uint32 _Rm=Rm;
	uint32 _Rn=Rn;
	uint32 res;
#ifdef __PORT__

    /*
       GCC on WIN32 (more importantly mingw) doesn't know the declared
       variables in asm until we put a _ before it.
       
       So the declaration dsp_flag_c needs to be _dsp_flag_c on mingw.
    */

    #ifdef __GCCWIN32__

	asm(
	"xorl %1, %2
	setz  _dsp_flag_z
	sets  _dsp_flag_n
	movl %%eax, %0
	"
	: "=m"(res)
	: "d"(_Rm), "a"(_Rn));
	
	#else
	
	asm(
	"xorl %1, %2
	setz  dsp_flag_z
	sets  dsp_flag_n
	movl %%eax, %0
	"
	: "=m"(res)
	: "d"(_Rm), "a"(_Rn));
	
	#endif
	
#else
	__asm 
	{
		mov   eax,_Rn
		mov   edx,_Rm
		xor   eax,edx
		setz  [dsp_flag_z]
		sets  [dsp_flag_n]
		mov   res,eax
	};
#endif
	Rn=res;
}

static void dsp_opcode_not(void)
{
	uint32 _Rn=Rn;
	uint32 res;
#ifdef __PORT__

    /*
       GCC on WIN32 (more importantly mingw) doesn't know the declared
       variables in asm until we put a _ before it.
       
       So the declaration dsp_flag_c needs to be _dsp_flag_c on mingw.
    */

    #ifdef __GCCWIN32__

	asm(
	"notl %1
	setz  _dsp_flag_z
	sets  _dsp_flag_n
	movl %%eax, %0
	"
	: "=m"(res)
	: "a"(_Rn));
	
	#else
	
	asm(
	"notl %1
	setz  dsp_flag_z
	sets  dsp_flag_n
	movl %%eax, %0
	"
	: "=m"(res)
	: "a"(_Rn));
	
	#endif
	
#else
	__asm 
	{
		mov   eax,_Rn
		not   eax
		setz  [dsp_flag_z]
		sets  [dsp_flag_n]
		mov   res,eax
	};
#endif
	Rn=res;
}

static void dsp_opcode_move_pc(void)
{
	Rn = dsp_pc-2; 
}

static void dsp_opcode_sat8(void)
{
	int32 _Rn=(int32)Rn;

	uint32 res= Rn = (_Rn<0) ? 0 : (_Rn > 0xff ? 0xff : _Rn);
	set_flag_z(res);
	reset_flag_n();
}

static void dsp_opcode_sat16(void)
{
	uint32 _Rn=Rn;
	uint32 res= Rn = (_Rn&0x80000000) ? 0 : (_Rn > 0xFFFF ? 0xFFFF : _Rn);
	set_flag_z(res);
	reset_flag_n();
}

static void dsp_opcode_sat24(void)
{
	uint32 _Rn=Rn;

	uint32 res= Rn = (_Rn&0x80000000) ? 0 : (_Rn > 0xFFFFFF ? 0xFFFFFF : _Rn);
	set_flag_z(res);
	reset_flag_n();
}

static void dsp_opcode_store_r14_indexed(void)
{
	dsp_long_write( dsp_reg[14] + (dsp_convert_zero[imm_1] << 2),Rn);
}

static void dsp_opcode_store_r15_indexed(void)
{
	dsp_long_write( dsp_reg[15] + (dsp_convert_zero[imm_1] << 2),Rn);
}

static void dsp_opcode_load_r14_ri(void)
{
	Rn=dsp_long_read(dsp_reg[14] + Rm);
}

static void dsp_opcode_load_r15_ri(void)
{
	Rn=dsp_long_read(dsp_reg[15] + Rm);
}

static void dsp_opcode_store_r14_ri(void)
{
	dsp_long_write(dsp_reg[14] + Rm,Rn);
}

static void dsp_opcode_store_r15_ri(void)
{
	dsp_long_write(dsp_reg[15] + Rm,Rn);
}

static void dsp_opcode_nop(void)
{
}

static void dsp_opcode_pack(void)
{
	uint32 _Rn=Rn;

	if (Rm==0)
	{
		Rn =((_Rn & 0x03C00000) >> 10) |
			((_Rn & 0x0001E000) >> 5)  |
			((_Rn & 0x000000FF));
	}
	else
	{
		Rn =((_Rn & 0x0000F000) << 10) |
			((_Rn & 0x00000F00) << 5)  |
			((_Rn & 0x000000FF));
	}
	reset_flag_z();
	reset_flag_n();
	set_flag_z(Rn);
	set_flag_n(Rn);
}

static void dsp_opcode_storeb(void)
{
	if ((Rm >= dsp_work_ram_base) && (Rm < (dsp_work_ram_base+0x2000)))
		dsp_long_write(Rm,Rn&0xff);
	else
		jaguar_byte_write(Rm,Rn);
}

static void dsp_opcode_storew(void)
{
	if ((Rm >= dsp_work_ram_base) && (Rm < (dsp_work_ram_base+0x2000)))
		dsp_long_write(Rm,Rn&0xffff);
	else
		jaguar_word_write(Rm,Rn);
}

static void dsp_opcode_store(void)
{
	dsp_long_write(Rm,Rn);
}

static void dsp_opcode_loadb(void)
{
	if ((Rm >= dsp_work_ram_base) && (Rm < (dsp_work_ram_base+0x2000)))
		Rn=dsp_long_read(Rm)&0xff;
	else
		Rn=jaguar_byte_read(Rm);
}

static void dsp_opcode_loadw(void)
{
	if ((Rm >= dsp_work_ram_base) && (Rm < (dsp_work_ram_base+0x2000)))
		Rn=dsp_long_read(Rm)&0xffff;
	else
		Rn=jaguar_word_read(Rm);
}

static void dsp_opcode_load(void)
{
	Rn = dsp_long_read(Rm);
}

static void dsp_opcode_load_r14_indexed(void)
{
	Rn = dsp_long_read( dsp_reg[14] + (dsp_convert_zero[imm_1] << 2));
}

static void dsp_opcode_load_r15_indexed(void)
{
	Rn = dsp_long_read( dsp_reg[15] + (dsp_convert_zero[imm_1] << 2));
}

static void dsp_opcode_movei(void)
{
	Rn = ((uint32)dsp_word_read(dsp_pc)) + (((uint32)dsp_word_read(dsp_pc+2))<<16);
	dsp_pc+=4;
}

static void dsp_opcode_moveta(void)
{
	alternate_Rn = Rm;
}

static void dsp_opcode_movefa(void)
{
	Rn = alternate_Rm;
}

static void dsp_opcode_move(void)
{
	Rn = Rm;
}

static void dsp_opcode_moveq(void)
{
	Rn = imm_1;    
}

static void dsp_opcode_resmac(void)
{
	Rn = dsp_acc;
}

static void dsp_opcode_imult(void)
{
	uint32 res=Rn=((int16)Rn)*((int16)Rm);
	set_flag_z(res);
	set_flag_n(res);
}

static void dsp_opcode_mult(void)
{
	uint32 res=Rn =  ((uint16)Rm) * ((uint16)Rn);
	set_flag_z(res);
	set_flag_n(res);
}

static void dsp_opcode_bclr(void)
{
	uint32 _Rm=imm_1;
	uint32 _Rn=Rn;
	uint32 res;
#ifdef __PORT__

    /*
       GCC on WIN32 (more importantly mingw) doesn't know the declared
       variables in asm until we put a _ before it.
       
       So the declaration dsp_flag_c needs to be _dsp_flag_c on mingw.
    */

    #ifdef __GCCWIN32__

	asm(
	"btrl %1, %2
	cmpl $0, %2
	setz  _dsp_flag_z
	sets  _dsp_flag_n
	movl %%eax, %0
	"
	: "=m"(res)
	: "c"(_Rm), "a"(_Rn));
	
	#else

	asm(
	"btrl %1, %2
	cmpl $0, %2
	setz  dsp_flag_z
	sets  dsp_flag_n
	movl %%eax, %0
	"
	: "=m"(res)
	: "c"(_Rm), "a"(_Rn));
	
	#endif
	
	
#else
	__asm 
	{
		mov   eax,_Rn
		mov   ecx,_Rm
		btr	  eax,ecx
		cmp   eax,0
		setz  [dsp_flag_z]
		sets  [dsp_flag_n]
		mov   res,eax
	};
#endif
	Rn=res;
}

static void dsp_opcode_btst(void)
{
	uint32 _Rm=imm_1;
	uint32 _Rn=Rn;
#ifdef __PORT__

    /*
       GCC on WIN32 (more importantly mingw) doesn't know the declared
       variables in asm until we put a _ before it.
       
       So the declaration dsp_flag_c needs to be _dsp_flag_c on mingw.
    */

    #ifdef __GCCWIN32__

	asm(
	"bt %0, %1
	setnc _dsp_flag_z
	"
	:
	: "c"(_Rm), "a"(_Rn));
	
	#else
	
	asm(
	"bt %0, %1
	setnc dsp_flag_z
	"
	:
	: "c"(_Rm), "a"(_Rn));
	
	#endif
	
#else
	__asm 
	{
		mov   eax,_Rn
		mov   ecx,_Rm
		bt	  eax,ecx
		setnc [dsp_flag_z]
	};
#endif
}

static void dsp_opcode_bset(void)
{
	uint32 _Rm=imm_1;
	uint32 _Rn=Rn;
	uint32 res;
#ifdef __PORT__

    /*
       GCC on WIN32 (more importantly mingw) doesn't know the declared
       variables in asm until we put a _ before it.
       
       So the declaration dsp_flag_c needs to be _dsp_flag_c on mingw.
    */

    #ifdef __GCCWIN32__

	asm(
	"btsl %1, %2
	cmpl $0, %2
	setz  _dsp_flag_z
	sets  _dsp_flag_n
	movl %%eax, %0
	"
	: "=m"(res)
	: "c"(_Rm), "a"(_Rn));
	
	#else
	
	asm(
	"btsl %1, %2
	cmpl $0, %2
	setz  dsp_flag_z
	sets  dsp_flag_n
	movl %%eax, %0
	"
	: "=m"(res)
	: "c"(_Rm), "a"(_Rn));
	
	#endif
	
#else
	__asm 
	{
		mov   eax,_Rn
		mov   ecx,_Rm
		bts	  eax,ecx
		cmp   eax,0
		setz  [dsp_flag_z]
		sets  [dsp_flag_n]
		mov   res,eax
	};
#endif
	Rn=res;
}

static void dsp_opcode_subqt(void)
{
	Rn -= dsp_convert_zero[imm_1];
}

static void dsp_opcode_addqt(void)
{
	Rn += dsp_convert_zero[imm_1];
}

static void dsp_opcode_imacn(void)
{
	uint32 res=((int16)Rm) * ((int16)(Rn));
	dsp_acc += res;
} 

static void dsp_opcode_mtoi(void)
{
	uint32 _Rm=Rm;
	uint32 res=Rn=(((INT32)_Rm >> 8) & 0xff800000) | (_Rm & 0x007fffff);
	set_flag_z(res);
	set_flag_n(res);
}

static void dsp_opcode_normi(void)
{
	uint32 _Rm = Rm;
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
	Rn = res;
	set_flag_z(res);
	set_flag_n(res);
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
			int16 b=((int16)dsp_word_read(addr+2));
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
			int16 b=((int16)dsp_word_read(addr+2));
			accum += a*b;
			addr += 4 * count;
		}
	}
	Rn = res = (int32)accum;
	// carry flag to do
	set_flag_z(res);
	set_flag_n(res);
}

static void dsp_opcode_abs(void)
{
	uint32 _Rn=Rn;
	uint32 res;
	
	if (_Rn==0x80000000)
	{
		set_flag_n(1);
	}
	else
	{
		dsp_flag_c  = ((_Rn&0x80000000)>>31);
		res= Rn =  (_Rn & 0x80000000) ? -_Rn : _Rn;
		reset_flag_n();
		set_flag_z(res);
	}
}

static void dsp_opcode_div(void)
{
	uint32 _Rm=Rm;
	uint32 _Rn=Rn;

	if (_Rm)
	{
		if (dsp_div_control & 1)
		{
			dsp_remain = (((uint64)_Rn) << 16) % _Rm;
			if (dsp_remain&0x80000000)
				dsp_remain-=_Rm;
			Rn = (((uint64)_Rn) << 16) / _Rm;
		}
		else
		{
			dsp_remain = _Rn % _Rm;
			if (dsp_remain&0x80000000)
				dsp_remain-=_Rm;
			Rn/=_Rm;
		}
	}
	else
		Rn=0xffffffff;
}

static void dsp_opcode_imultn(void)
{
	uint32 res=(int32)(((int16)Rn)*((int16)Rm));
	dsp_acc=(int32)res;
	set_flag_z(res);
	set_flag_n(res);
}

static void dsp_opcode_neg(void)
{
	uint32 _Rn=Rn;
	uint32 res;
#ifdef __PORT__

    /*
       GCC on WIN32 (more importantly mingw) doesn't know the declared
       variables in asm until we put a _ before it.
       
       So the declaration dsp_flag_c needs to be _dsp_flag_c on mingw.
    */

    #ifdef __GCCWIN32__

	asm(
	"
	subl %1, %2
	setc  _dsp_flag_c
	setz  _dsp_flag_z
	sets  _dsp_flag_n
	movl %%eax, %0
	"
	: "=m"(res)
	: "d"(_Rn), "a"(0));
	
	#else

	asm(
	"
	subl %1, %2
	setc  dsp_flag_c
	setz  dsp_flag_z
	sets  dsp_flag_n
	movl %%eax, %0
	"
	: "=m"(res)
	: "d"(_Rn), "a"(0));
	
	#endif
	
#else
	__asm 
	{
		xor	  eax,eax
		mov   edx,_Rn
		sub   eax,edx
		setc  [dsp_flag_c]
		setz  [dsp_flag_z]
		sets  [dsp_flag_n]
		mov	  res,eax
	};
#endif
	Rn=res;
}

static void dsp_opcode_shlq(void)
{
	uint32 shift=(32-dsp_convert_zero[imm_1]);
	uint32 _Rn=Rn;
	uint32 res;
#ifdef __PORT__

    /*
       GCC on WIN32 (more importantly mingw) doesn't know the declared
       variables in asm until we put a _ before it.
       
       So the declaration dsp_flag_c needs to be _dsp_flag_c on mingw.
    */

    #ifdef __GCCWIN32__

	asm(
	"testl $0x80000000, %2
	setnz _dsp_flag_c
	shl %%cl, %2
	cmpl $0, %2
	setz  _dsp_flag_z
	sets  _dsp_flag_n
	movl %%eax, %0
	"
	: "=m"(res)
	: "c"(shift), "a"(_Rn));
	
	#else
	
	asm(
	"testl $0x80000000, %2
	setnz dsp_flag_c
	shl %%cl, %2
	cmpl $0, %2
	setz  dsp_flag_z
	sets  dsp_flag_n
	movl %%eax, %0
	"
	: "=m"(res)
	: "c"(shift), "a"(_Rn));

	#endif	
	
#else
	__asm 
	{
		mov ecx,shift
		mov eax,_Rn
		test eax,0x80000000
		setnz [dsp_flag_c]
		shl eax,cl
		cmp eax,0
		setz [dsp_flag_z]
		sets [dsp_flag_n]
		mov res,eax
	}
#endif
	Rn=res;
}

static void dsp_opcode_shrq(void)
{
	uint32 shift=dsp_convert_zero[imm_1];
	uint32 _Rn=Rn;
	
	uint32 res;
#ifdef __PORT__

    /*
       GCC on WIN32 (more importantly mingw) doesn't know the declared
       variables in asm until we put a _ before it.
       
       So the declaration dsp_flag_c needs to be _dsp_flag_c on mingw.
    */

    #ifdef __GCCWIN32__

	asm(
	"testl $0x00000001, %2
	setnz _dsp_flag_c
	shr %%cl, %2
	cmpl $0, %2
	setz  _dsp_flag_z
	sets  _dsp_flag_n
	movl %%eax, %0
	"
	: "=m"(res)
	: "c"(shift), "a"(_Rn));
	
	#else
	
	asm(
	"testl $0x00000001, %2
	setnz dsp_flag_c
	shr %%cl, %2
	cmpl $0, %2
	setz  dsp_flag_z
	sets  dsp_flag_n
	movl %%eax, %0
	"
	: "=m"(res)
	: "c"(shift), "a"(_Rn));

	#endif	

#else
	__asm 
	{
		mov ecx,shift
		mov eax,_Rn
		test eax,0x00000001
		setnz [dsp_flag_c]
		shr eax,cl
		cmp eax,0
		setz [dsp_flag_z]
		sets [dsp_flag_n]
		mov res,eax
	}
#endif
	Rn=res;
}

static void dsp_opcode_ror(void)
{
	uint32 shift=Rm;
	uint32 _Rn=Rn;
	uint32 res;
#ifdef __PORT__

    /*
       GCC on WIN32 (more importantly mingw) doesn't know the declared
       variables in asm until we put a _ before it.
       
       So the declaration dsp_flag_c needs to be _dsp_flag_c on mingw.
    */

    #ifdef __GCCWIN32__

	asm(
	"testl $0x80000000, %2
	setnz _dsp_flag_c
	ror %%cl, %2
	cmpl $0, %2
	setz  _dsp_flag_z
	sets  _dsp_flag_n
	movl %%eax, %0
	"
	: "=m"(res)
	: "c"(shift), "a"(_Rn));
	
	#else

	asm(
	"testl $0x80000000, %2
	setnz dsp_flag_c
	ror %%cl, %2
	cmpl $0, %2
	setz  dsp_flag_z
	sets  dsp_flag_n
	movl %%eax, %0
	"
	: "=m"(res)
	: "c"(shift), "a"(_Rn));
	
	#endif
	
#else
	__asm 
	{
		mov ecx,shift
		mov eax,_Rn
		test eax,0x80000000
		setnz [dsp_flag_c]
		ror eax,cl
		cmp eax,0
		setz [dsp_flag_z]
		sets [dsp_flag_n]
		mov res,eax
	}
#endif
	Rn=res;
}

static void dsp_opcode_rorq(void)
{
	uint32 shift=dsp_convert_zero[imm_1&0x1f];
	uint32 _Rn=Rn;
	uint32 res;
#ifdef __PORT__

    /*
       GCC on WIN32 (more importantly mingw) doesn't know the declared
       variables in asm until we put a _ before it.
       
       So the declaration dsp_flag_c needs to be _dsp_flag_c on mingw.
    */

    #ifdef __GCCWIN32__

	asm(
	"testl $0x80000000, %2
	setnz _dsp_flag_c
	ror %%cl, %2
	cmpl $0, %2
	setz  _dsp_flag_z
	sets  _dsp_flag_n
	movl %%eax, %0
	"
	: "=m"(res)
	: "c"(shift), "a"(_Rn));
	
	#else
	
	asm(
	"testl $0x80000000, %2
	setnz dsp_flag_c
	ror %%cl, %2
	cmpl $0, %2
	setz  dsp_flag_z
	sets  dsp_flag_n
	movl %%eax, %0
	"
	: "=m"(res)
	: "c"(shift), "a"(_Rn));

	#endif
	
#else
	__asm 
	{
		mov ecx,shift
		mov eax,_Rn
		test eax,0x80000000
		setnz [dsp_flag_c]
		ror eax,cl
		cmp eax,0
		setz [dsp_flag_z]
		sets [dsp_flag_n]
		mov res,eax
	}
#endif
	Rn=res;
}

static void dsp_opcode_sha(void)
{
	int32 sRm=(int32)Rm;
	uint32 _Rn=Rn;

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
	Rn=_Rn;
	set_flag_z(_Rn);
	set_flag_n(_Rn);
}

static void dsp_opcode_sharq(void)
{
	uint32 shift=dsp_convert_zero[imm_1];
	uint32 _Rn=Rn;

	dsp_flag_c  = (_Rn & 0x1);
	while (shift)
	{
		_Rn=((int32)_Rn)>>1;
		shift--;
	}
	Rn=_Rn;
	set_flag_z(_Rn);
	set_flag_n(_Rn);
}

static void dsp_opcode_sh(void)
{
	int32 sRm=(int32)Rm;
	uint32 _Rn=Rn;

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
	Rn=_Rn;
	set_flag_z(_Rn);
	set_flag_n(_Rn);
}

#define CLR_ZN  { dsp_flag_n=0; dsp_flag_z=0; };
#define CLR_ZNC { dsp_flag_c=0; dsp_flag_n=0; dsp_flag_z=0; };
#define SET_Z(r)			(dsp_flag_z= ((r) == 0))
#define SET_C_ADD(a,b)		(dsp_flag_c= ((UINT32)(b) > (UINT32)(~(a))) << 1)
#define SET_C_SUB(a,b)		(dsp_flag_c= ((UINT32)(b) > (UINT32)(a)) << 1)
#define SET_N(r)			(dsp_flag_n= (((UINT32)(r) >> 29) & 4))
#define SET_ZN(r)			SET_N(r); SET_Z(r)
#define SET_ZNC_ADD(a,b,r)	SET_N(r); SET_Z(r); SET_C_ADD(a,b)
#define SET_ZNC_SUB(a,b,r)	SET_N(r); SET_Z(r); SET_C_SUB(a,b)

void dsp_opcode_addqmod(void)	
{
	UINT32 r1 = dsp_convert_zero[imm_1];
	UINT32 r2 = Rn;
	UINT32 res = r2 + r1;
	res = (res & (~dsp_modulo)) | (r2 & dsp_modulo);
	Rn = res;
	CLR_ZNC; SET_ZNC_ADD(r2,r1,res);
}
void dsp_opcode_subqmod(void)	
{
	UINT32 r1 = dsp_convert_zero[imm_1];
	UINT32 r2 = Rn;
	UINT32 res = r2 - r1;
	res = (res & (~dsp_modulo)) | (r2 & dsp_modulo);
	Rn = res;
	
	SET_ZNC_SUB(r2,r1,res);
}
void dsp_opcode_mirror(void)	
{
	UINT32 r1 = Rn;
	UINT32 res = (mirror_table[r1 & 0xffff] << 16) | mirror_table[r1 >> 16];
	Rn = res;
	CLR_ZN; SET_ZN(res);
}
void dsp_opcode_sat32s(void)		
{
	INT32 r2 = (UINT32)Rn;
	INT32 temp = dsp_acc >> 32;
	UINT32 res = (temp < -1) ? (INT32)0x80000000 : (temp > 0) ? (INT32)0x7fffffff : r2;
	Rn = res;
	CLR_ZN; SET_ZN(res);
}
void dsp_opcode_sat16s(void)		
{
	INT32 r2 = Rn;
	UINT32 res = (r2 < -32768) ? -32768 : (r2 > 32767) ? 32767 : r2;
	Rn = res;
	CLR_ZN; SET_ZN(res);
}
