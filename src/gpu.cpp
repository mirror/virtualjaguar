//
// GPU Core
//
// by cal2
// GCC/SDL port by Niels Wagenaar (Linux/WIN32) and Caz (BeOS)
// Cleanups, endian wrongness, and bad ASM amelioration by James L. Hammons
// Note: Endian wrongness probably stems from the MAME origins of this emu and
//       the braindead way in which MAME handles memory. :-)
//
// Problem with not booting the BIOS was the incorrect way that the
// SUBC instruction set the carry when the carry was set going in...
// Same problem with ADDC...
//

#include "gpu.h"

// For GPU dissasembly...

//#define GPU_DIS_ABS
//#define GPU_DIS_ADDC
//#define GPU_DIS_CMP
//#define GPU_DIS_CMPQ
//#define GPU_DIS_DIV
//#define GPU_DIS_JUMP
//#define GPU_DIS_JR
//#define GPU_DIS_ROR
//#define GPU_DIS_RORQ
//#define GPU_DIS_SH
//#define GPU_DIS_SHA
//#define GPU_DIS_SHARQ
//#define GPU_DIS_SHLQ
//#define GPU_DIS_SHRQ
//#define GPU_DIS_STORE14R
//#define GPU_DIS_STORE15R
//#define GPU_DIS_SUBC
/*
GPU opcodes use (BIOS flying ATARI logo):
	              add 357416
	             addq 538030
	            addqt 6999
	              sub 116663
	             subq 188059
	            subqt 15086
	              neg 36097
	              and 233993
	               or 109332
	              xor 1384
	             btst 111924
	             bset 25029
	             bclr 10551
	             mult 28147
	            imult 69148
+	              div 64102
+	              abs 159394
+	             shlq 194690
+	             shrq 292587
+	            sharq 192649
+	             rorq 58672
+	              cmp 244963
+	             cmpq 114834
	             move 833472
	            moveq 56427
	           moveta 220814
	           movefa 170678
	            movei 152025
	            loadw 108220
	             load 430936
	           storew 3036
	            store 372490
	          move_pc 2330
+	             jump 349134
+	               jr 529171
	            mmult 64904
	              nop 432179
*/

// Private function prototypes

void gpu_dump_disassembly(void);
void gpu_dump_registers(void);
void gpu_dump_memory(void);

#define CINT0FLAG			0x00200
#define CINT1FLAG			0x00400
#define CINT2FLAG			0x00800
#define CINT3FLAG			0x01000
#define CINT4FLAG			0x02000
#define CINT04FLAGS			(CINT0FLAG | CINT1FLAG | CINT2FLAG | CINT3FLAG | CINT4FLAG)

extern int start_logging;
extern int gpu_start_log;

static void gpu_opcode_add(void);
static void gpu_opcode_addc(void);
static void gpu_opcode_addq(void);
static void gpu_opcode_addqt(void);
static void gpu_opcode_sub(void);
static void gpu_opcode_subc(void);
static void gpu_opcode_subq(void);
static void gpu_opcode_subqt(void);
static void gpu_opcode_neg(void);
static void gpu_opcode_and(void);
static void gpu_opcode_or(void);
static void gpu_opcode_xor(void);
static void gpu_opcode_not(void);
static void gpu_opcode_btst(void);
static void gpu_opcode_bset(void);
static void gpu_opcode_bclr(void);
static void gpu_opcode_mult(void);
static void gpu_opcode_imult(void);
static void gpu_opcode_imultn(void);
static void gpu_opcode_resmac(void);
static void gpu_opcode_imacn(void);
static void gpu_opcode_div(void);
static void gpu_opcode_abs(void);
static void gpu_opcode_sh(void);
static void gpu_opcode_shlq(void);
static void gpu_opcode_shrq(void);
static void gpu_opcode_sha(void);
static void gpu_opcode_sharq(void);
static void gpu_opcode_ror(void);
static void gpu_opcode_rorq(void);
static void gpu_opcode_cmp(void);
static void gpu_opcode_cmpq(void);
static void gpu_opcode_sat8(void);
static void gpu_opcode_sat16(void);
static void gpu_opcode_move(void);
static void gpu_opcode_moveq(void);
static void gpu_opcode_moveta(void);
static void gpu_opcode_movefa(void);
static void gpu_opcode_movei(void);
static void gpu_opcode_loadb(void);
static void gpu_opcode_loadw(void);
static void gpu_opcode_load(void);
static void gpu_opcode_loadp(void);
static void gpu_opcode_load_r14_indexed(void);
static void gpu_opcode_load_r15_indexed(void);
static void gpu_opcode_storeb(void);
static void gpu_opcode_storew(void);
static void gpu_opcode_store(void);
static void gpu_opcode_storep(void);
static void gpu_opcode_store_r14_indexed(void);
static void gpu_opcode_store_r15_indexed(void);
static void gpu_opcode_move_pc(void);
static void gpu_opcode_jump(void);
static void gpu_opcode_jr(void);
static void gpu_opcode_mmult(void);
static void gpu_opcode_mtoi(void);
static void gpu_opcode_normi(void);
static void gpu_opcode_nop(void);
static void gpu_opcode_load_r14_ri(void);
static void gpu_opcode_load_r15_ri(void);
static void gpu_opcode_store_r14_ri(void);
static void gpu_opcode_store_r15_ri(void);
static void gpu_opcode_sat24(void);
static void gpu_opcode_pack(void);

uint8 gpu_opcode_cycles[64] = 
{
	3,  3,  3,  3,  3,  3,  3,  3,
	3,  3,  3,  3,  3,  3,  3,  3,
	3,  3,  1,  3,  1, 18,  3,  3,
	3,  3,  3,  3,  3,  3,  3,  3,
	3,  3,  2,  2,  2,  2,  3,  4,
	5,  4,  5,  6,  6,  1,  1,  1,
	1,  2,  2,  2,  1,  1,  9,  3,
	3,  1,  6,  6,  2,  2,  3,  3
};

void (*gpu_opcode[64])()= 
{	
	gpu_opcode_add,					gpu_opcode_addc,				gpu_opcode_addq,				gpu_opcode_addqt,
	gpu_opcode_sub,					gpu_opcode_subc,				gpu_opcode_subq,				gpu_opcode_subqt,
	gpu_opcode_neg,					gpu_opcode_and,					gpu_opcode_or,					gpu_opcode_xor,
	gpu_opcode_not,					gpu_opcode_btst,				gpu_opcode_bset,				gpu_opcode_bclr,
	gpu_opcode_mult,				gpu_opcode_imult,				gpu_opcode_imultn,				gpu_opcode_resmac,
	gpu_opcode_imacn,				gpu_opcode_div,					gpu_opcode_abs,					gpu_opcode_sh,
	gpu_opcode_shlq,				gpu_opcode_shrq,				gpu_opcode_sha,					gpu_opcode_sharq,
	gpu_opcode_ror,					gpu_opcode_rorq,				gpu_opcode_cmp,					gpu_opcode_cmpq,
	gpu_opcode_sat8,				gpu_opcode_sat16,				gpu_opcode_move,				gpu_opcode_moveq,
	gpu_opcode_moveta,				gpu_opcode_movefa,				gpu_opcode_movei,				gpu_opcode_loadb,
	gpu_opcode_loadw,				gpu_opcode_load,				gpu_opcode_loadp,				gpu_opcode_load_r14_indexed,
	gpu_opcode_load_r15_indexed,	gpu_opcode_storeb,				gpu_opcode_storew,				gpu_opcode_store,
	gpu_opcode_storep,				gpu_opcode_store_r14_indexed,	gpu_opcode_store_r15_indexed,	gpu_opcode_move_pc,
	gpu_opcode_jump,				gpu_opcode_jr,					gpu_opcode_mmult,				gpu_opcode_mtoi,
	gpu_opcode_normi,				gpu_opcode_nop,					gpu_opcode_load_r14_ri,			gpu_opcode_load_r15_ri,
	gpu_opcode_store_r14_ri,		gpu_opcode_store_r15_ri,		gpu_opcode_sat24,				gpu_opcode_pack,
};

static uint8 * gpu_ram_8;
static uint32 gpu_pc;
static uint32 gpu_acc;
static uint32 gpu_remain;
static uint32 gpu_hidata;
static uint32 gpu_flags;
static uint32 gpu_matrix_control;
static uint32 gpu_pointer_to_matrix;
static uint32 gpu_data_organization;
static uint32 gpu_control;
static uint32 gpu_div_control;
static uint8 gpu_flag_z, gpu_flag_n, gpu_flag_c;
static uint8 gpu_alternate_flag_z, gpu_alternate_flag_n, gpu_alternate_flag_c;
static uint32 * gpu_reg;
static uint32 * gpu_alternate_reg;
static uint32 * gpu_reg_bank_0;
static uint32 * gpu_reg_bank_1;

static uint32 gpu_instruction;
static uint32 gpu_opcode_first_parameter;
static uint32 gpu_opcode_second_parameter;

#define GPU_RUNNING		(gpu_control & 0x01)

#define RM				gpu_reg[gpu_opcode_first_parameter]
#define RN				gpu_reg[gpu_opcode_second_parameter]
#define ALTERNATE_RM	gpu_alternate_reg[gpu_opcode_first_parameter]
#define ALTERNATE_RN	gpu_alternate_reg[gpu_opcode_second_parameter]
#define IMM_1			gpu_opcode_first_parameter
#define IMM_2			gpu_opcode_second_parameter

#define SET_FLAG_Z(r)	(gpu_flag_z = ((r) == 0));
#define SET_FLAG_N(r)	(gpu_flag_n = (((UINT32)(r) >> 31) & 0x01));

#define RESET_FLAG_Z()	gpu_flag_z = 0;
#define RESET_FLAG_N()	gpu_flag_n = 0;
#define RESET_FLAG_C()	gpu_flag_c = 0;    

#define CLR_Z				(gpu_flag_z = 0)
#define CLR_ZN				(gpu_flag_z = gpu_flag_n = 0)
#define CLR_ZNC				(gpu_flag_z = gpu_flag_n = gpu_flag_c = 0)
#define SET_Z(r)			(gpu_flag_z = ((r) == 0))
#define SET_N(r)			(gpu_flag_n = (((UINT32)(r) >> 31) & 0x01))
#define SET_C_ADD(a,b)		(gpu_flag_c = ((UINT32)(b) > (UINT32)(~(a))))
#define SET_C_SUB(a,b)		(gpu_flag_c = ((UINT32)(b) > (UINT32)(a)))
#define SET_ZN(r)			SET_N(r); SET_Z(r)
#define SET_ZNC_ADD(a,b,r)	SET_N(r); SET_Z(r); SET_C_ADD(a,b)
#define SET_ZNC_SUB(a,b,r)	SET_N(r); SET_Z(r); SET_C_SUB(a,b)

uint32 gpu_convert_zero[32] =
	{ 32,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31 };

uint8 * branch_condition_table = 0;
#define BRANCH_CONDITION(x)	branch_condition_table[(x) + ((jaguar_flags & 7) << 5)]

uint32 gpu_opcode_use[64];

void gpu_update_register_banks(void);

char * gpu_opcode_str[64]= 
{	
	"add",				"addc",				"addq",				"addqt",
	"sub",				"subc",				"subq",				"subqt",
	"neg",				"and",				"or",				"xor",
	"not",				"btst",				"bset",				"bclr",
	"mult",				"imult",			"imultn",			"resmac",
	"imacn",			"div",				"abs",				"sh",
	"shlq",				"shrq",				"sha",				"sharq",
	"ror",				"rorq",				"cmp",				"cmpq",
	"sat8",				"sat16",			"move",				"moveq",
	"moveta",			"movefa",			"movei",			"loadb",
	"loadw",			"load",				"loadp",			"load_r14_indexed",
	"load_r15_indexed",	"storeb",			"storew",			"store",
	"storep",			"store_r14_indexed","store_r15_indexed","move_pc",
	"jump",				"jr",				"mmult",			"mtoi",
	"normi",			"nop",				"load_r14_ri",		"load_r15_ri",
	"store_r14_ri",		"store_r15_ri",		"sat24",			"pack",
};

static uint32 gpu_in_exec = 0;
static uint32 gpu_releaseTimeSlice_flag = 0;


void gpu_releaseTimeslice(void)
{
	gpu_releaseTimeSlice_flag = 1;
}

uint32 gpu_get_pc(void)
{
	return gpu_pc;
}

#define ZFLAG	0x00001
#define CFLAG	0x00002
#define NFLAG	0x00004

void build_branch_condition_table(void)
{
	if (!branch_condition_table)
	{
		branch_condition_table = (uint8 *)malloc(32 * 8 * sizeof(branch_condition_table[0]));

		if (branch_condition_table)
		{
			for(int i=0; i<8; i++)
			{
				for(int j=0; j<32; j++)
				{
					int result = 1;
					if (j & 1)
						if (i & ZFLAG)
							result = 0;
					if (j & 2)
						if (!(i & ZFLAG))
							result = 0;
					if (j & 4)
						if (i & (CFLAG << (j >> 4)))
							result = 0;
					if (j & 8)
						if (!(i & (CFLAG << (j >> 4))))
							result = 0;
					branch_condition_table[i * 32 + j] = result;
				}
			}
		}
	}
}

//
// GPU byte access (read)
//

unsigned gpu_byte_read(unsigned int offset)
{	
	if ((offset >= GPU_WORK_RAM_BASE) && (offset < GPU_WORK_RAM_BASE+0x1000))
		return gpu_ram_8[offset & 0xFFF];
	else if ((offset >= GPU_CONTROL_RAM_BASE) && (offset < GPU_CONTROL_RAM_BASE+0x20))
	{
		uint32 data = gpu_long_read(offset & 0xFFFFFFFC);

		if ((offset & 0x03) == 0)
			return data >> 24;
		else if ((offset & 0x03) == 1)
			return (data >> 16) & 0xFF;
		else if ((offset & 0x03) == 2)
			return (data >> 8) & 0xFF;
		else if ((offset & 0x03) == 3)
			return data & 0xFF;
	}

	return jaguar_byte_read(offset);
}

//
// GPU word access (read)
//

unsigned gpu_word_read(unsigned int offset)
{
	if ((offset >= GPU_WORK_RAM_BASE) && (offset < GPU_WORK_RAM_BASE+0x1000))
	{
		offset &= 0xFFF;
		uint16 data = ((uint16)gpu_ram_8[offset] << 8) | (uint16)gpu_ram_8[offset+1];
		return data;
	}
	else if ((offset >= GPU_CONTROL_RAM_BASE) && (offset < GPU_CONTROL_RAM_BASE+0x20))
	{
// This looks and smells wrong...
// But it *might* be OK...
		if (offset & 0x01)			// Catch cases 1 & 3... (unaligned read)
			return (gpu_byte_read(offset) << 8) | gpu_byte_read(offset+1);

		uint32 data = gpu_long_read(offset & 0xFFFFFFFC);

		if (offset & 0x02)			// Cases 0 & 2...
			return data & 0xFFFF;
		else
			return data >> 16;
	}

//TEMP--Mirror of F03000?
if (offset >= 0xF0B000 && offset <= 0xF0BFFF)
WriteLog("[GPUR16] --> Possible GPU RAM mirror access!");

	return jaguar_word_read(offset);
}

//
// GPU dword access (read)
//

unsigned gpu_long_read(unsigned int offset)
{

	if ((offset >= GPU_WORK_RAM_BASE) && (offset < GPU_WORK_RAM_BASE+0x1000))
	{
		offset &= 0xFFF;
		return ((uint32)gpu_ram_8[offset] << 24) | ((uint32)gpu_ram_8[offset+1] << 16)
			| ((uint32)gpu_ram_8[offset+2] << 8) | (uint32)gpu_ram_8[offset+3];
	}
	else if ((offset >= GPU_CONTROL_RAM_BASE) && (offset < GPU_CONTROL_RAM_BASE+0x20))
	{
		offset &= 0x1F;
		switch (offset)
		{
		case 0x00:
			gpu_flag_c = (gpu_flag_c ? 1 : 0);
			gpu_flag_z = (gpu_flag_z ? 1 : 0);
			gpu_flag_n = (gpu_flag_n ? 1 : 0);

			gpu_flags = (gpu_flags & 0xFFFFFFF8) | (gpu_flag_n << 2) | (gpu_flag_c << 1) | gpu_flag_z;
					
			return gpu_flags & 0xFFFFC1FF;
		case 0x04:
			return gpu_matrix_control;
		case 0x08:
			return gpu_pointer_to_matrix;
		case 0x0C:
			return gpu_data_organization;
		case 0x10:
			return gpu_pc;
		case 0x14:
			return gpu_control;
		case 0x18:
			return gpu_hidata;
		case 0x1C:
			return gpu_remain;
		default:								// unaligned long read
			return 0;
			//exit(0);
			//	   __asm int 3
			//		   }
		}
		// to prevent any lock-ups
	}
//TEMP--Mirror of F03000?
if (offset >= 0xF0B000 && offset <= 0xF0BFFF)
	WriteLog("[GPUR32] --> Possible GPU RAM mirror access!\n");
/*if (offset >= 0xF1D000 && offset <= 0xF1DFFF)
	WriteLog("[GPUR32] --> Reading from Wavetable ROM!\n");//*/

	return (jaguar_word_read(offset) << 16) | jaguar_word_read(offset+2);
}

//
// GPU byte access (write)
//

void gpu_byte_write(unsigned offset, unsigned data)
{
	if ((offset >= GPU_WORK_RAM_BASE) && (offset < GPU_WORK_RAM_BASE+0x1000))
	{
		gpu_ram_8[offset & 0xFFF] = data;
		if (gpu_in_exec == 0)
		{
//			s68000releaseTimeslice();
			m68k_end_timeslice();
			dsp_releaseTimeslice();
		}
		return;
	}
	else if ((offset >= GPU_CONTROL_RAM_BASE) && (offset < GPU_CONTROL_RAM_BASE+0x20))
	{
		uint32 reg = offset & 0x1C;
		int bytenum = offset & 0x03;

//This is definitely wrong!
		if ((reg >= 0x1C) && (reg <= 0x1F))
			gpu_div_control = (gpu_div_control & (~(0xFF << (bytenum << 3)))) | (data << (bytenum << 3));				
		else
		{
			uint32 old_data = gpu_long_read(offset & 0xFFFFFFC);
			bytenum = 3 - bytenum; // convention motorola !!!
			old_data = (old_data & (~(0xFF << (bytenum << 3)))) | (data << (bytenum << 3));	
			gpu_long_write(offset & 0xFFFFFFC, old_data);
		}
		return;
	}
//	WriteLog("gpu: writing %.2x at 0x%.8x\n",data,offset);
	jaguar_byte_write(offset, data);
}

//
// GPU word access (write)
//

void gpu_word_write(unsigned offset, unsigned data)
{

	if ((offset >= GPU_WORK_RAM_BASE) && (offset < GPU_WORK_RAM_BASE+0x1000))
	{
//if (offset >= 0xF03000 && offset <= 0xF03003)
//	WriteLog("--> GPU(16): Writing %04X at %08X ***\n", data, offset);

		gpu_ram_8[offset & 0xFFF] = (data>>8) & 0xFF;
		gpu_ram_8[(offset+1) & 0xFFF] = data & 0xFF;
		if (gpu_in_exec == 0)
		{
//			s68000releaseTimeslice();
			m68k_end_timeslice();
			dsp_releaseTimeslice();
		}
		return;
	}
	if ((offset >= GPU_CONTROL_RAM_BASE) && (offset < GPU_CONTROL_RAM_BASE+0x20))
	{
		if (offset & 0x01)		// This is supposed to weed out unaligned writes, but does nothing...
		{
			//exit(0);
			//__asm int 3
		}
		if ((offset & 0x1C) == 0x1C)
		{
//This doesn't look right either--handles cases 1, 2, & 3 all the same!
			if (offset & 0x03)
				gpu_div_control = (gpu_div_control&0xFFFF0000) | (data&0xFFFF);
			else
				gpu_div_control = (gpu_div_control&0xFFFF) | ((data&0xFFFF)<<16);
		}
		else 
		{
			uint32 old_data = gpu_long_read(offset & 0xFFFFFFC);
			if (offset & 0x03)
				old_data = (old_data & 0xFFFF0000) | (data & 0xFFFF);
			else
				old_data = (old_data & 0xFFFF) | ((data & 0xFFFF) << 16);
			gpu_long_write(offset & 0xFFFFFFC, old_data);
		}
		return;
	}

//	WriteLog("gpu: writing %.4x at 0x%.8x\n",data,offset);
//This is done by the blitter...
//if ((offset >= 0x1FF020 && offset <= 0x1FF03F) || (offset >= 0x1FF820 && offset <= 0x1FF83F))
//	WriteLog("GPU(16): Writing %08X at %08X\n", data, offset);
//if ((offset >= 0x1FE020 && offset <= 0x1FE03F) || (offset >= 0x1FE820 && offset <= 0x1FE83F))
//	WriteLog("GPU(16): Writing %08X at %08X\n", data, offset);
//if (offset >= 0xF02200 && offset <= 0xF0229F)
//	WriteLog("GPU(16): Writing to blitter --> %08X at %08X\n", data, offset);

	jaguar_word_write(offset, data);
}

//
// GPU dword access (write)
//

void gpu_long_write(unsigned offset, unsigned data)
{

	if ((offset >= GPU_WORK_RAM_BASE) && (offset < GPU_WORK_RAM_BASE+0x1000))
	{
#ifdef GPU_DEBUG
		if (offset & 0x03)
		{
			WriteLog("GPU: Someone is trying an unaligned write @ %08X [%08X]\n", offset, data)
			gpu_dump_registers();
		}
#endif	// #ifdef GPU_DEBUG
/*if (offset == 0xF03000)
{
	WriteLog("GPU Write [F03000]: %08X\n", data);
//	data = 0x03D0DEAD;	// Why isn't this there???
//	data = 0xABCDEFFF;	// Why isn't this there???
}//*/
		gpu_ram_8[offset & 0xFFF] = (data >> 24) & 0xFF,
		gpu_ram_8[(offset+1) & 0xFFF] = (data >> 16) & 0xFF,
		gpu_ram_8[(offset+2) & 0xFFF] = (data >> 8) & 0xFF,
		gpu_ram_8[(offset+3) & 0xFFF] = data & 0xFF;
		return;
	}
	else if ((offset >= GPU_CONTROL_RAM_BASE) && (offset < GPU_CONTROL_RAM_BASE+0x20))
	{
		offset &= 0x1F;
		switch (offset)
		{
		case 0x00:
			/*if (data&0x8)
				gpu_flags=(data&(~0x08))|(gpu_flags&0x08); // update dsp_flags, but keep imask unchanged
			else*/
			gpu_flags = data;
			gpu_flag_z = gpu_flags & 0x01;
			gpu_flag_c = (gpu_flags>>1) & 0x01;
			gpu_flag_n = (gpu_flags>>2) & 0x01;
			gpu_update_register_banks();
			gpu_control &= ~((gpu_flags & CINT04FLAGS) >> 3);
			gpu_check_irqs();
			break;
		case 0x04:
			gpu_matrix_control = data;
			break;
		case 0x08:
			// Can only point to long aligned addresses
			gpu_pointer_to_matrix = data & 0xFFFFFFFC;
			break;
		case 0x0C:
			gpu_data_organization = data;
			break;
		case 0x10:
			gpu_pc = data;	/*WriteLog("setting gpu pc to 0x%.8x\n",gpu_pc);*/
			break;
		case 0x14:
		{	
			uint32 gpu_was_running = GPU_RUNNING;
						
//			data &= (~0x07C0); // disable writes to irq pending
			data &= (~0xF7C0); // Disable writes to INT_LAT0-4 & TOM version number
			/*if (GPU_RUNNING)
			{
				WriteLog("gpu pc is 0x%.8x\n",gpu_pc);
				fclose(log_get());
				exit(0);
			}*/
			// check for GPU->CPU interrupt
			if (data & 0x02)
			{
//				WriteLog("GPU->CPU interrupt\n");
				if (tom_irq_enabled(IRQ_GPU))
				{
					if ((tom_irq_enabled(IRQ_GPU)) && (jaguar_interrupt_handler_is_valid(64)))
					{
						tom_set_pending_gpu_int();
//						s68000interrupt(7,64);
//						s68000flushInterrupts();
						m68k_set_irq(7);			// Set 68000 NMI
						gpu_releaseTimeslice();
					}
/*
					uint32 addr=jaguar_word_read(((IRQ_GPU+64)<<2)+0);
					addr<<=16;
					addr|=jaguar_word_read(((IRQ_GPU+64)<<2)+2);
					if ((addr)&&(jaguar_interrupt_handler_is_valid(IRQ_GPU+64)))
					{
						s68000interrupt(7,IRQ_GPU+64);
						s68000flushInterrupts();
					}
*/
				}
				data &= ~(0x02);
			}
			// check for CPU->GPU interrupt
			if (data & 0x04)
			{
				//WriteLog("CPU->GPU interrupt\n");
				gpu_set_irq_line(0, 1);
//				s68000releaseTimeslice();
				m68k_end_timeslice();
				dsp_releaseTimeslice();
				data &= ~(0x04);
			}
			// single stepping
			if (data & 0x10)
			{
				//WriteLog("asked to perform a single step (single step is %senabled)\n",(data&0x8)?"":"not ");
			}
//			gpu_control = (gpu_control & 0x107C0) | (data & (~0x107C0));
			gpu_control = (gpu_control & 0x1F7C0) | (data & (~0x1F7C0));

			// if gpu wasn't running but is now running, execute a few cycles
#ifndef GPU_SINGLE_STEPPING
			if ((!gpu_was_running) && (GPU_RUNNING))
				gpu_exec(200);
#else
			if (gpu_control & 0x18)
				gpu_exec(1);
#endif	// #ifndef GPU_SINGLE_STEPPING
#ifdef GPU_DEBUG
WriteLog("Write to GPU CTRL: %08X ", data);
if (GPU_RUNNING)
	WriteLog("-- Starting to run at %08X...", gpu_pc);
WriteLog("\n");
#endif	// #ifdef GPU_DEBUG
//if (GPU_RUNNING)
//	gpu_dump_disassembly();
			break;
		}
		case 0x18:
			gpu_hidata = data;
			break;
		case 0x1C:
			gpu_div_control = data;
			break;
//		default:   // unaligned long write
			//exit(0);
			//__asm int 3
		}
		return;
	}

//This is done by the blitter...
//if ((offset >= 0x1FF020 && offset <= 0x1FF03F) || (offset >= 0x1FF820 && offset <= 0x1FF83F))
//	WriteLog("GPU(32): Writing %08X at %08X\n", data, offset);
//if ((offset >= 0x1FE020 && offset <= 0x1FE03F) || (offset >= 0x1FE820 && offset <= 0x1FE83F))
//	WriteLog("GPU(32): Writing %08X at %08X\n", data, offset);
//if (offset >= 0xF02200 && offset <= 0xF0229F)
//	WriteLog("GPU(32): Writing to blitter --> %08X at %08X\n", data, offset);

	jaguar_word_write(offset, (data >> 16) & 0xFFFF);
	jaguar_word_write(offset+2, data & 0xFFFF);
}

void gpu_update_register_banks(void)
{
	uint32 temp;
	int bank = (gpu_flags & 0x4000);

//	WriteLog("gpu_update_register_banks at gpu pc 0x%.8x bank=%i iflag=%i\n",gpu_pc,bank?1:0,(gpu_flags&0x8)?1:0);

	if (gpu_flags & 0x8) 
		bank = 0;

	if ((!bank && (gpu_reg_bank_0 != gpu_reg)) || (bank && (gpu_reg_bank_1 != gpu_reg)))
	{
//		WriteLog("\tswitching to bank %i\n",bank?1:0);
		for(int i=0; i<32; i++)
		{
			temp = gpu_reg[i];
			gpu_reg[i] = gpu_alternate_reg[i];
			gpu_alternate_reg[i] = temp;
		}

		// switch flags
		temp = gpu_flag_z;
		gpu_flag_z = gpu_alternate_flag_z;
		gpu_alternate_flag_z = temp;

		temp = gpu_flag_n;
		gpu_flag_n = gpu_alternate_flag_n;
		gpu_alternate_flag_n = temp;

		temp = gpu_flag_c;
		gpu_flag_c = gpu_alternate_flag_c;
		gpu_alternate_flag_c = temp;

		if (!bank)
		{
			gpu_reg_bank_0 = gpu_reg;
			gpu_reg_bank_1 = gpu_alternate_reg;
		}
		else
		{
			gpu_reg_bank_0 = gpu_alternate_reg;
			gpu_reg_bank_1 = gpu_reg;
		}
	}
//	else
//	{
//		WriteLog("\tnot switching banks\n");
//	}
}

void gpu_check_irqs(void)
{
	int bits, mask, which = 0;

	// get the active interrupt bits 
	bits = (gpu_control >> 6) & 0x1F;
	bits |= (gpu_control >> 10) & 0x20;

	// get the interrupt mask 
	mask = (gpu_flags >> 4) & 0x1F;
	mask |= (gpu_flags >> 11) & 0x20;
	
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

	if (gpu_flags & 0x08)
		return;

	if (start_logging)
		WriteLog("GPU: generating irq %i\n", which);

	// set the interrupt flag 
	gpu_flags |= 0x08;
	gpu_update_register_banks();

	// subqt  #4,r31		; pre-decrement stack pointer 
	// move  pc,r30			; address of interrupted code 
	// store  r30,(r31)     ; store return address
	gpu_reg[31] -= 4;
	gpu_reg[30] = gpu_pc - 2;
	gpu_long_write(gpu_reg[31], gpu_pc - 2);
	
	// movei  #service_address,r30  ; pointer to ISR entry 
	// jump  (r30)					; jump to ISR 
	// nop
	gpu_pc = GPU_WORK_RAM_BASE + (which * 0x10);
	gpu_reg[30] = gpu_pc;
}

void gpu_set_irq_line(int irqline, int state)
{
	if (start_logging)
		WriteLog("GPU: setting irg line %i\n", irqline);
	int mask = 0x40 << irqline;
	gpu_control &= ~mask;

	if (state)
	{
		gpu_control |= mask;
		gpu_check_irqs();
	}
}

//TEMPORARY: Testing only!
#include "gpu2.h"
#include "gpu3.h"

void gpu_init(void)
{
	memory_malloc_secure((void **)&gpu_ram_8, 0x1000, "GPU work ram");
//	gpu_ram_16=(uint16*)gpu_ram_8;
//	gpu_ram_32=(uint32*)gpu_ram_8;

	memory_malloc_secure((void **)&gpu_reg, 32*sizeof(int32), "GPU bank 0 regs");
	memory_malloc_secure((void **)&gpu_alternate_reg, 32*sizeof(int32), "GPU bank 1 regs");
	
	build_branch_condition_table();

	gpu_reset();

//TEMPORARY: Testing only!
	gpu2_init();
	gpu3_init();
}

void gpu_reset(void)
{
	gpu_pc				  = 0x00F03000;
	gpu_acc				  = 0x00000000;
	gpu_remain			  = 0x00000000;
	gpu_hidata			  = 0x00000000;
	gpu_flags			  = 0x00040000;
	gpu_matrix_control    = 0x00000000;
	gpu_pointer_to_matrix = 0x00000000;
	gpu_data_organization = 0xFFFFFFFF;
	gpu_control			  = 0x00012800;			// Correctly sets this a TOM Rev. 2
	gpu_div_control		  = 0x00000000;
	gpu_in_exec			  = 0;

	for(int i=0; i<32; i++)
	{
		gpu_reg[i]           = 0x00000000;
		gpu_alternate_reg[i] = 0x00000000;
	}
	
	gpu_reg_bank_0 = gpu_reg;
	gpu_reg_bank_1 = gpu_alternate_reg;
//	gpu_reg_bank_1 = gpu_reg;
//	gpu_reg_bank_0 = gpu_alternate_reg;

	CLR_ZNC;

	gpu_alternate_flag_z = 0;
	gpu_alternate_flag_n = 0;
	gpu_alternate_flag_c = 0;

	memset(gpu_ram_8, 0xFF, 0x1000);

	gpu_reset_stats();
}

uint32 gpu_read_pc(void)
{
	return gpu_pc;
}

void gpu_reset_stats(void)
{
	for(uint32 i=0; i<64; i++)
		gpu_opcode_use[i] = 0;
	WriteLog("--> GPU stats were reset!\n");
}

void gpu_dump_disassembly(void)
{
	char buffer[512];

	WriteLog("\n---[GPU code at 00F03000]---------------------------\n");
	uint32 j = 0xF03000;
	while (j <= 0xF03FFF)
	{
		uint32 oldj = j;
		j += dasmjag(JAGUAR_GPU, buffer, j);
		WriteLog("\t%08X: %s\n", oldj, buffer);
	}
}

void gpu_dump_registers(void)
{
	WriteLog("\n---[GPU flags: NCZ %d%d%d]-----------------------\n", gpu_flag_n, gpu_flag_c, gpu_flag_z);
	WriteLog("\nRegisters bank 0\n");
	for(int j=0; j<8; j++)
	{
		WriteLog("\tr%2i = %08X r%2i = %08X r%2i = %08X r%2i = %08X\n",
						  (j << 2) + 0, gpu_reg[(j << 2) + 0],
						  (j << 2) + 1, gpu_reg[(j << 2) + 1],
						  (j << 2) + 2, gpu_reg[(j << 2) + 2],
						  (j << 2) + 3, gpu_reg[(j << 2) + 3]);
	}
	WriteLog("Registers bank 1\n");
	for(int j=0; j<8; j++)
	{
		WriteLog("\tr%2i = %08X r%2i = %08X r%2i = %08X r%2i = %08X\n",
						  (j << 2) + 0, gpu_alternate_reg[(j << 2) + 0],
						  (j << 2) + 1, gpu_alternate_reg[(j << 2) + 1],
						  (j << 2) + 2, gpu_alternate_reg[(j << 2) + 2],
						  (j << 2) + 3, gpu_alternate_reg[(j << 2) + 3]);
	}
}

void gpu_dump_memory(void)
{
	WriteLog("\n---[GPU data at 00F03000]---------------------------\n");
	for(int i=0; i<0xFFF; i+=4)
		WriteLog("\t%08X: %02X %02X %02X %02X\n", 0xF03000+i, gpu_ram_8[i],
			gpu_ram_8[i+1], gpu_ram_8[i+2], gpu_ram_8[i+3]);
}

void gpu_done(void)
{ 
	WriteLog("GPU: stopped at PC=%08X (GPU %s running)\n", (unsigned int)gpu_pc, GPU_RUNNING ? "was" : "wasn't");

	// get the active interrupt bits 
	int bits = (gpu_control >> 6) & 0x1F;
	bits |= (gpu_control >> 10) & 0x20;

	// get the interrupt mask 
	int mask = (gpu_flags >> 4) & 0x1F;
	mask |= (gpu_flags >> 11) & 0x20;
	

	WriteLog("GPU: ibits=0x%.8x imask=0x%.8x\n", bits, mask);
//	WriteLog("\nregisters bank 0\n");
//	for (int j=0;j<8;j++)
//	{
//		WriteLog("\tr%2i=0x%.8x r%2i=0x%.8x r%2i=0x%.8x r%2i=0x%.8x\n",
//						  (j<<2)+0,gpu_reg[(j<<2)+0],
//						  (j<<2)+1,gpu_reg[(j<<2)+1],
//						  (j<<2)+2,gpu_reg[(j<<2)+2],
//						  (j<<2)+3,gpu_reg[(j<<2)+3]);
//
//	}
//	WriteLog("registers bank 1\n");
//	for (j=0;j<8;j++)
//	{
//		WriteLog("\tr%2i=0x%.8x r%2i=0x%.8x r%2i=0x%.8x r%2i=0x%.8x\n",
//						  (j<<2)+0,gpu_alternate_reg[(j<<2)+0],
//						  (j<<2)+1,gpu_alternate_reg[(j<<2)+1],
//						  (j<<2)+2,gpu_alternate_reg[(j<<2)+2],
//						  (j<<2)+3,gpu_alternate_reg[(j<<2)+3]);
//
//	}
	WriteLog("\n---[GPU code at 00F03000]---------------------------\n");
	static char buffer[512];
	int j = 0xF03000;
//	for(int i=0; i<4096; i++)
	while (j <= 0xF03FFF)
	{
		uint32 oldj = j;
		j += dasmjag(JAGUAR_GPU, buffer, j);
		WriteLog("\t%08X: %s\n", oldj, buffer);
	}//*/

/*	WriteLog("---[GPU code at %08X]---------------------------\n", gpu_pc);
	j = gpu_pc - 64;
	for(int i=0; i<4096; i++)
	{
		uint32 oldj = j;
		j += dasmjag(JAGUAR_GPU, buffer, j);
		WriteLog("\t%08X: %s\n", oldj, buffer);
	}*/

	WriteLog("\nGPU opcodes use:\n");
	for(int i=0; i<64; i++)
	{
		if (gpu_opcode_use[i])
			WriteLog("\t%17s %lu\n", gpu_opcode_str[i], gpu_opcode_use[i]);
	}
	WriteLog("\n");

	memory_free(gpu_ram_8);
}

//
// Main GPU execution core
//

static int testCount = 1;
static int len = 0;
void gpu_exec(int32 cycles)
{
	if (!GPU_RUNNING)
		return;

#ifdef GPU_SINGLE_STEPPING
	if (gpu_control & 0x18)
	{
		cycles = 1;
		gpu_control &= ~0x10;
	}
#endif
	gpu_check_irqs();
	gpu_releaseTimeSlice_flag = 0;
	gpu_in_exec++;

	while ((cycles > 0) && GPU_RUNNING)
	{
		gpu_flag_c = (gpu_flag_c ? 1 : 0);
		gpu_flag_z = (gpu_flag_z ? 1 : 0);
		gpu_flag_n = (gpu_flag_n ? 1 : 0);
	
		uint16 opcode = gpu_word_read(gpu_pc);

		uint32 index = opcode >> 10;
		gpu_instruction = opcode;				// Added for GPU #3...
		gpu_opcode_first_parameter = (opcode >> 5) & 0x1F;
		gpu_opcode_second_parameter = opcode & 0x1F;
/*if (gpu_pc == 0xF03BE8)
WriteLog("Start of OP frame write...\n");
if (gpu_pc == 0xF03EEE)
WriteLog("--> Writing BRANCH object ---\n");
if (gpu_pc == 0xF03F62)
WriteLog("--> Writing BITMAP object ***\n");//*/
/*if (gpu_pc == 0xF03546)
{
	WriteLog("\n--> GPU PC: F03546\n");
	gpu_dump_registers();
	gpu_dump_disassembly();
}//*/
/*if (gpu_pc == 0xF033F6)
{
	WriteLog("\n--> GPU PC: F033F6\n");
	gpu_dump_registers();
	gpu_dump_disassembly();
}//*/
/*if (gpu_pc == 0xF033CC)
{
	WriteLog("\n--> GPU PC: F033CC\n");
	gpu_dump_registers();
	gpu_dump_disassembly();
}//*/
/*if (gpu_pc == 0xF033D6)
{
	WriteLog("\n--> GPU PC: F033D6 (#%d)\n", testCount++);
	gpu_dump_registers();
	gpu_dump_memory();
}//*/
/*if (gpu_pc == 0xF033D8)
{
	WriteLog("\n--> GPU PC: F033D8 (#%d)\n", testCount++);
	gpu_dump_registers();
	gpu_dump_memory();
}//*/
/*if (gpu_pc == 0xF0358E)
{
	WriteLog("\n--> GPU PC: F0358E (#%d)\n", testCount++);
	gpu_dump_registers();
	gpu_dump_memory();
}//*/
/*if (gpu_pc == 0xF034CA)
{
	WriteLog("\n--> GPU PC: F034CA (#%d)\n", testCount++);
	gpu_dump_registers();
}//*/
/*if (gpu_pc == 0xF034CA)
{
	len = gpu_reg[1] + 4;//, r9save = gpu_reg[9];
	WriteLog("\nAbout to subtract [#%d] (R14=%08X, R15=%08X, R9=%08X):\n   ", testCount++, gpu_reg[14], gpu_reg[15], gpu_reg[9]);
	for(int i=0; i<len; i+=4)
		WriteLog(" %08X", gpu_long_read(gpu_reg[15]+i));
	WriteLog("\n   ");
	for(int i=0; i<len; i+=4)
		WriteLog(" %08X", gpu_long_read(gpu_reg[14]+i));
	WriteLog("\n\n");
}
if (gpu_pc == 0xF034DE)
{
	WriteLog("\nSubtracted! (R14=%08X, R15=%08X):\n   ", gpu_reg[14], gpu_reg[15]);
	for(int i=0; i<len; i+=4)
		WriteLog(" %08X", gpu_long_read(gpu_reg[15]+i));
	WriteLog("\n   ");
	for(int i=0; i<len; i+=4)
		WriteLog(" %08X", gpu_long_read(gpu_reg[14]+i));
	WriteLog("\n   ");
	for(int i=0; i<len; i+=4)
		WriteLog(" --------");
	WriteLog("\n   ");
	for(int i=0; i<len; i+=4)
		WriteLog(" %08X", gpu_long_read(gpu_reg[9]+4+i));
	WriteLog("\n\n");
}//*/
/*if (gpu_pc == 0xF035C8)
{
	WriteLog("\n--> GPU PC: F035C8 (#%d)\n", testCount++);
	gpu_dump_registers();
	gpu_dump_disassembly();
}//*/

if (gpu_start_log)
{
	gpu_reset_stats();
static char buffer[512];
dasmjag(JAGUAR_GPU, buffer, gpu_pc);
WriteLog("GPU: [%08X] %s (RM=%08X, RN=%08X) -> ", gpu_pc, buffer, RM, RN);
}//*/
//$E400 -> 1110 01 -> $39 -> 57
//GPU #1
		gpu_pc += 2;
		gpu_opcode[index]();
//GPU #2
//		gpu2_opcode[index]();
//		gpu_pc += 2;
//GPU #3				(Doesn't show ATARI logo! #1 & #2 do...)
//		gpu_pc += 2;
//		gpu3_opcode[index]();

// BIOS hacking
//GPU: [00F03548] jr      nz,00F03560 (0xd561) (RM=00F03114, RN=00000004) ->     --> JR: Branch taken. 
/*static bool firstTime = true;
if (gpu_pc == 0xF03548 && firstTime)
{
	gpu_flag_z = 1;
//	firstTime = false;

//static char buffer[512];
//int k=0xF03548;
//while (k<0xF0356C)
//{
//int oldk = k;
//k += dasmjag(JAGUAR_GPU, buffer, k);
//WriteLog("GPU: [%08X] %s\n", oldk, buffer);
//}
//	gpu_start_log = 1;
}//*/
//GPU: [00F0354C] jump    nz,(r29) (0xd3a1) (RM=00F03314, RN=00000004) -> (RM=00F03314, RN=00000004)
/*if (gpu_pc == 0xF0354C)
	gpu_flag_z = 0;//, gpu_start_log = 1;//*/

		cycles -= gpu_opcode_cycles[index];
		gpu_opcode_use[index]++;
if (gpu_start_log)
	WriteLog("(RM=%08X, RN=%08X)\n", RM, RN);//*/
	}

	gpu_in_exec--;
}

//
// GPU opcodes
//

/*
GPU opcodes use (offset punch--vertically below bad guy):
	              add 18686
	             addq 32621
	              sub 7483
	             subq 10252
	              and 21229
	               or 15003
	             btst 1822
	             bset 2072
	             mult 141
	              div 2392
	             shlq 13449
	             shrq 10297
	            sharq 11104
	              cmp 6775
	             cmpq 5944
	             move 31259
	            moveq 4473
	            movei 23277
	            loadb 46
	            loadw 4201
	             load 28580
	 load_r14_indexed 1183
	 load_r15_indexed 1125
	           storew 178
	            store 10144
	store_r14_indexed 320
	store_r15_indexed 1
	          move_pc 1742
	             jump 24467
	               jr 18090
	              nop 41362
*/

static void gpu_opcode_jump(void)
{
#ifdef GPU_DIS_JUMP
char * condition[32] =
{	"T", "nz", "z", "???", "nc", "nc nz", "nc z", "???", "c", "c nz",
	"c z", "???", "???", "???", "???", "???", "???", "???", "???",
	"???", "nn", "nn nz", "nn z", "???", "n", "n nz", "n z", "???",
	"???", "???", "???", "F" };
	WriteLog("%06X: JUMP   %s, (R%02u) [NCZ:%u%u%u, R%02u=%08X] ", gpu_pc-2, condition[IMM_2], IMM_1, gpu_flag_n, gpu_flag_c, gpu_flag_z, IMM_1, RM);
#endif
	// normalize flags
/*	gpu_flag_c = (gpu_flag_c ? 1 : 0);
	gpu_flag_z = (gpu_flag_z ? 1 : 0);
	gpu_flag_n = (gpu_flag_n ? 1 : 0);*/
	// KLUDGE: Used by BRANCH_CONDITION
	uint32 jaguar_flags = (gpu_flag_n << 2) | (gpu_flag_c << 1) | gpu_flag_z;

	if (BRANCH_CONDITION(IMM_2))
	{
#ifdef GPU_DIS_JUMP
	WriteLog("Branched!\n");
#endif
if (gpu_start_log)
	WriteLog("    --> JUMP: Branch taken.\n");
		uint32 delayed_pc = RM;

		gpu_exec(1);
		gpu_pc = delayed_pc;
/*		uint16 opcode = gpu_word_read(gpu_pc);
		gpu_opcode_first_parameter = (opcode >> 5) & 0x1F;
		gpu_opcode_second_parameter = opcode & 0x1F;

		gpu_pc = delayed_pc;
		gpu_opcode[opcode>>10]();//*/
	}
#ifdef GPU_DIS_JUMP
	else
		WriteLog("Branch NOT taken.\n");
#endif
}

static void gpu_opcode_jr(void)
{
#ifdef GPU_DIS_JR
char * condition[32] =
{	"T", "nz", "z", "???", "nc", "nc nz", "nc z", "???", "c", "c nz",
	"c z", "???", "???", "???", "???", "???", "???", "???", "???",
	"???", "nn", "nn nz", "nn z", "???", "n", "n nz", "n z", "???",
	"???", "???", "???", "F" };
	WriteLog("%06X: JR     %s, %06X [NCZ:%u%u%u] ", gpu_pc-2, condition[IMM_2], gpu_pc+((IMM_1 & 0x10 ? 0xFFFFFFF0 | IMM_1 : IMM_1) * 2), gpu_flag_n, gpu_flag_c, gpu_flag_z);
#endif
/*	if (CONDITION(jaguar.op & 31))
	{
		INT32 r1 = (INT8)((jaguar.op >> 2) & 0xF8) >> 2;
		UINT32 newpc = jaguar.PC + r1;
		CALL_MAME_DEBUG;
		jaguar.op = ROPCODE(jaguar.PC);
		jaguar.PC = newpc;
		(*jaguar.table[jaguar.op >> 10])();

		jaguar_icount -= 3;	// 3 wait states guaranteed
	}*/
	// normalize flags
/*	gpu_flag_n = (gpu_flag_n ? 1 : 0);
	gpu_flag_c = (gpu_flag_c ? 1 : 0);
	gpu_flag_z = (gpu_flag_z ? 1 : 0);*/
	// KLUDGE: Used by BRANCH_CONDITION
	uint32 jaguar_flags = (gpu_flag_n << 2) | (gpu_flag_c << 1) | gpu_flag_z;

	if (BRANCH_CONDITION(IMM_2))
	{
#ifdef GPU_DIS_JR
	WriteLog("Branched!\n");
#endif
if (gpu_start_log)
	WriteLog("    --> JR: Branch taken.\n");
		int32 offset = (IMM_1 & 0x10 ? 0xFFFFFFF0 | IMM_1 : IMM_1);		// Sign extend IMM_1
		int32 delayed_pc = gpu_pc + (offset * 2);

		gpu_exec(1);
		gpu_pc = delayed_pc;
/*		uint16 opcode = gpu_word_read(gpu_pc);
		gpu_opcode_first_parameter = (opcode >> 5) & 0x1F;
		gpu_opcode_second_parameter = opcode & 0x1F;

		gpu_pc = delayed_pc;
		gpu_opcode[opcode>>10]();//*/
	}
#ifdef GPU_DIS_JR
	else
		WriteLog("Branch NOT taken.\n");
#endif
}

static void gpu_opcode_add(void)
{
/*	int dreg = jaguar.op & 31;
	UINT32 r1 = jaguar.r[(jaguar.op >> 5) & 31];
	UINT32 r2 = jaguar.r[dreg];
	UINT32 res = r2 + r1;
	jaguar.r[dreg] = res;
	CLR_ZNC; SET_ZNC_ADD(r2,r1,res);*/

	UINT32 res = RN + RM;
	CLR_ZNC; SET_ZNC_ADD(RN, RM, res);
	RN = res;
}

static void gpu_opcode_addc(void)
{
#ifdef GPU_DIS_ADDC
	WriteLog("%06X: ADDC   R%02u, R%02u [NCZ:%u%u%u, R%02u=%08X, R%02u=%08X] -> ", gpu_pc-2, IMM_1, IMM_2, gpu_flag_n, gpu_flag_c, gpu_flag_z, IMM_1, RM, IMM_2, RN);
#endif
/*	int dreg = jaguar.op & 31;
	UINT32 r1 = jaguar.r[(jaguar.op >> 5) & 31];
	UINT32 r2 = jaguar.r[dreg];
	UINT32 res = r2 + r1 + ((jaguar.FLAGS >> 1) & 1);
	jaguar.r[dreg] = res;
	CLR_ZNC; SET_ZNC_ADD(r2,r1,res);*/

	UINT32 res = RN + RM + gpu_flag_c;
	UINT32 carry = gpu_flag_c;
//	SET_ZNC_ADD(RN, RM, res); //???BUG???
	SET_ZNC_ADD(RN + carry, RM, res);
//	SET_ZNC_ADD(RN, RM + carry, res);
	RN = res;
#ifdef GPU_DIS_ADDC
	WriteLog("[NCZ:%u%u%u, R%02u=%08X, R%02u=%08X]\n", gpu_flag_n, gpu_flag_c, gpu_flag_z, IMM_1, RM, IMM_2, RN);
#endif
}

static void gpu_opcode_addq(void)
{
/*	int dreg = jaguar.op & 31;
	UINT32 r1 = convert_zero[(jaguar.op >> 5) & 31];
	UINT32 r2 = jaguar.r[dreg];
	UINT32 res = r2 + r1;
	jaguar.r[dreg] = res;
	CLR_ZNC; SET_ZNC_ADD(r2,r1,res);*/

	UINT32 r1 = gpu_convert_zero[IMM_1];
	UINT32 res = RN + r1;
	CLR_ZNC; SET_ZNC_ADD(RN, r1, res);
	RN = res;
}

static void gpu_opcode_addqt(void)
{
	RN += gpu_convert_zero[IMM_1];
}

static void gpu_opcode_sub(void)
{
/*	int dreg = jaguar.op & 31;
	UINT32 r1 = jaguar.r[(jaguar.op >> 5) & 31];
	UINT32 r2 = jaguar.r[dreg];
	UINT32 res = r2 - r1;
	jaguar.r[dreg] = res;
	CLR_ZNC; SET_ZNC_SUB(r2,r1,res);*/

	UINT32 res = RN - RM;
	SET_ZNC_SUB(RN, RM, res);
	RN = res;
}

static void gpu_opcode_subc(void)
{
#ifdef GPU_DIS_SUBC
	WriteLog("%06X: SUBC   R%02u, R%02u [NCZ:%u%u%u, R%02u=%08X, R%02u=%08X] -> ", gpu_pc-2, IMM_1, IMM_2, gpu_flag_n, gpu_flag_c, gpu_flag_z, IMM_1, RM, IMM_2, RN);
#endif
	UINT32 res = RN - RM - gpu_flag_c;
	UINT32 borrow = gpu_flag_c;
//	SET_ZNC_SUB(RN, RM, res); //???BUG??? YES!!!
	SET_ZNC_SUB(RN - borrow, RM, res);
	RN = res;
#ifdef GPU_DIS_SUBC
	WriteLog("[NCZ:%u%u%u, R%02u=%08X, R%02u=%08X]\n", gpu_flag_n, gpu_flag_c, gpu_flag_z, IMM_1, RM, IMM_2, RN);
#endif
}

static void gpu_opcode_subq(void)
{
	UINT32 r1 = gpu_convert_zero[IMM_1];
	UINT32 res = RN - r1;
	SET_ZNC_SUB(RN, r1, res);
	RN = res;
}

static void gpu_opcode_subqt(void)
{
	RN -= gpu_convert_zero[IMM_1];
}

static void gpu_opcode_cmp(void)
{
#ifdef GPU_DIS_CMP
	WriteLog("%06X: CMP    R%02u, R%02u [NCZ:%u%u%u, R%02u=%08X, R%02u=%08X] -> ", gpu_pc-2, IMM_1, IMM_2, gpu_flag_n, gpu_flag_c, gpu_flag_z, IMM_1, RM, IMM_2, RN);
#endif
	UINT32 res = RN - RM;
	SET_ZNC_SUB(RN, RM, res);
#ifdef GPU_DIS_CMP
	WriteLog("[NCZ:%u%u%u]\n", gpu_flag_n, gpu_flag_c, gpu_flag_z);
#endif
}

static void gpu_opcode_cmpq(void)
{
#ifdef GPU_DIS_CMPQ
	WriteLog("%06X: CMPQ   #%d, R%02u [NCZ:%u%u%u, R%02u=%08X] -> ", gpu_pc-2, sqtable[IMM_1], IMM_2, gpu_flag_n, gpu_flag_c, gpu_flag_z, IMM_2, RN);
#endif
	static int32 sqtable[32] =
		{ 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,-16,-15,-14,-13,-12,-11,-10,-9,-8,-7,-6,-5,-4,-3,-2,-1 };

	UINT32 r1 = sqtable[IMM_1 & 0x1F]; // I like this better -> (INT8)(jaguar.op >> 2) >> 3;
	UINT32 res = RN - r1;
	SET_ZNC_SUB(RN, r1, res);
#ifdef GPU_DIS_CMPQ
	WriteLog("[NCZ:%u%u%u]\n", gpu_flag_n, gpu_flag_c, gpu_flag_z);
#endif
}

static void gpu_opcode_and(void)
{
	RN = RN & RM;
	SET_ZN(RN);
}

static void gpu_opcode_or(void)
{
	RN = RN | RM;
	SET_ZN(RN);
}

static void gpu_opcode_xor(void)
{
	RN = RN ^ RM;
	SET_ZN(RN);
}

static void gpu_opcode_not(void)
{
	RN = ~RN;
	SET_ZN(RN);
}

static void gpu_opcode_move_pc(void)
{
	// Should be previous PC--this might not always be previous instruction!
	// Then again, this will point right at the *current* instruction, i.e., MOVE PC,R!
	RN = gpu_pc - 2;
}

static void gpu_opcode_sat8(void)
{
	RN = ((int32)RN < 0 ? 0 : (RN > 0xFF ? 0xFF : RN));
	SET_ZN(RN);
}

static void gpu_opcode_sat16(void)
{
	RN = ((int32)RN < 0 ? 0 : (RN > 0xFFFF ? 0xFFFF : RN));
	SET_ZN(RN);
}

static void gpu_opcode_sat24(void)
{
	RN = ((int32)RN < 0 ? 0 : (RN > 0xFFFFFF ? 0xFFFFFF : RN));
	SET_ZN(RN);
}

static void gpu_opcode_store_r14_indexed(void)
{
	gpu_long_write(gpu_reg[14] + (gpu_convert_zero[IMM_1] << 2), RN);
}

static void gpu_opcode_store_r15_indexed(void)
{
	gpu_long_write(gpu_reg[15] + (gpu_convert_zero[IMM_1] << 2), RN);
}

static void gpu_opcode_load_r14_ri(void)
{
	RN = gpu_long_read(gpu_reg[14] + RM);
}

static void gpu_opcode_load_r15_ri(void)
{
	RN = gpu_long_read(gpu_reg[15] + RM);
}

static void gpu_opcode_store_r14_ri(void)
{
#ifdef GPU_DIS_STORE14R
	WriteLog("%06X: STORE  R%02u, (R14+R%02u) [NCZ:%u%u%u, R%02u=%08X, R14+R%02u=%08X]\n", gpu_pc-2, IMM_2, IMM_1, gpu_flag_n, gpu_flag_c, gpu_flag_z, IMM_2, RN, IMM_1, RM+gpu_reg[14]);
#endif
	gpu_long_write(gpu_reg[14] + RM, RN);
}

static void gpu_opcode_store_r15_ri(void)
{
#ifdef GPU_DIS_STORE15R
	WriteLog("%06X: STORE  R%02u, (R15+R%02u) [NCZ:%u%u%u, R%02u=%08X, R15+R%02u=%08X]\n", gpu_pc-2, IMM_2, IMM_1, gpu_flag_n, gpu_flag_c, gpu_flag_z, IMM_2, RN, IMM_1, RM+gpu_reg[15]);
#endif
	gpu_long_write(gpu_reg[15] + RM, RN);
}

static void gpu_opcode_nop(void)
{
#ifdef GPU_DIS_NOP
	WriteLog("%06X: NOP    [NCZ:%u%u%u]\n", gpu_pc-2, gpu_flag_n, gpu_flag_c, gpu_flag_z);
#endif
}

static void gpu_opcode_pack(void)
{
	uint32 val = RN;

	if (RM == 0)				// Pack
		RN = ((val >> 10) & 0x0000F000) | ((val >> 5) & 0x00000F00) | (val & 0x000000FF);
	else						// Unpack
		RN = ((val & 0x0000F000) << 10) | ((val & 0x00000F00) << 5) | (val & 0x000000FF);
}

static void gpu_opcode_storeb(void)
{
//Is this right???
// Would appear to be so...!
	if ((RM >= 0xF03000) && (RM <= 0xF03FFF))
		gpu_long_write(RM, RN & 0xFF);
	else
		jaguar_byte_write(RM, RN);
}

static void gpu_opcode_storew(void)
{
	if ((RM >= 0xF03000) && (RM <= 0xF03FFF))
		gpu_long_write(RM, RN & 0xFFFF);
	else
		jaguar_word_write(RM, RN);
}

static void gpu_opcode_store(void)
{
	gpu_long_write(RM, RN);
}

static void gpu_opcode_storep(void)
{
	gpu_long_write(RM + 0, gpu_hidata);
	gpu_long_write(RM + 4, RN);
}

static void gpu_opcode_loadb(void)
{
	if ((RM >= 0xF03000) && (RM <= 0xF03FFF))
		RN = gpu_long_read(RM) & 0xFF;
	else
		RN = jaguar_byte_read(RM);
}

static void gpu_opcode_loadw(void)
{
	if ((RM >= 0xF03000) && (RM <= 0xF03FFF))
		RN = gpu_long_read(RM) & 0xFFFF;
	else
		RN = jaguar_word_read(RM);
}

static void gpu_opcode_load(void)
{
	RN = gpu_long_read(RM);
}

static void gpu_opcode_loadp(void)
{
	gpu_hidata = gpu_long_read(RM + 0);
	RN		   = gpu_long_read(RM + 4);
}

static void gpu_opcode_load_r14_indexed(void)
{
	RN = gpu_long_read(gpu_reg[14] + (gpu_convert_zero[IMM_1] << 2));
}

static void gpu_opcode_load_r15_indexed(void)
{
	RN = gpu_long_read(gpu_reg[15] + (gpu_convert_zero[IMM_1] << 2));
}

static void gpu_opcode_movei(void)
{
	// This instruction is followed by 32-bit value in LSW / MSW format...
	RN = (uint32)gpu_word_read(gpu_pc) | ((uint32)gpu_word_read(gpu_pc + 2) << 16);
	gpu_pc += 4;
}

static void gpu_opcode_moveta(void)
{
	ALTERNATE_RN = RM;
}

static void gpu_opcode_movefa(void)
{
	RN = ALTERNATE_RM;
}

static void gpu_opcode_move(void)
{
	RN = RM;
}

static void gpu_opcode_moveq(void)
{
	RN = IMM_1;    
}

static void gpu_opcode_resmac(void)
{
	RN = gpu_acc;
}

static void gpu_opcode_imult(void)
{
	RN = (int16)RN * (int16)RM;
	SET_ZN(RN);
}

static void gpu_opcode_mult(void)
{
	RN = (uint16)RM * (uint16)RN;
	SET_ZN(RN);
}

static void gpu_opcode_bclr(void)
{
	UINT32 res = RN & ~(1 << IMM_1);
	RN = res;
	SET_ZN(res);
}

static void gpu_opcode_btst(void)
{
	gpu_flag_z = (~RN >> IMM_1) & 1;
}

static void gpu_opcode_bset(void)
{
	UINT32 res = RN | (1 << IMM_1);
	RN = res;
	SET_ZN(res);
}

static void gpu_opcode_imacn(void)
{
	uint32 res = (int16)RM * (int16)(RN);
	gpu_acc += res;
}

static void gpu_opcode_mtoi(void)
{
	uint32 _RM = RM;
	uint32 res = RN = (((INT32)_RM >> 8) & 0xFF800000) | (_RM & 0x007FFFFF);
	SET_ZN(res);
}

static void gpu_opcode_normi(void)
{
	uint32 _RM = RM;
	uint32 res = 0;

	if (_RM)
	{
		while ((_RM & 0xFFC00000) == 0)
		{
			_RM <<= 1;
			res--;
		}
		while ((_RM & 0xFF800000) != 0)
		{
			_RM >>= 1;
			res++;
		}
	}
	RN = res;
	SET_ZN(res);
}

static void gpu_opcode_mmult(void)
{
	int count	= gpu_matrix_control & 0x0F;	// Matrix width
	uint32 addr = gpu_pointer_to_matrix;		// In the GPU's RAM
	int64 accum = 0;
	uint32 res;

	if (gpu_matrix_control & 0x10)				// Column stepping
	{
		for(int i=0; i<count; i++)
		{ 
			int16 a;
			if (i & 0x01)
				a = (int16)((gpu_alternate_reg[IMM_1 + (i >> 1)] >> 16) & 0xFFFF);
			else
				a = (int16)(gpu_alternate_reg[IMM_1 + (i >> 1)] & 0xFFFF);

			int16 b = ((int16)gpu_word_read(addr + 2));
			accum += a * b;
			addr += 4 * count;
		}
	}
	else										// Row stepping
	{
		for(int i=0; i<count; i++)
		{
			int16 a;
			if (i & 0x01)
				a = (int16)((gpu_alternate_reg[IMM_1 + (i >> 1)] >> 16) & 0xFFFF);
			else
				a = (int16)(gpu_alternate_reg[IMM_1 + (i >> 1)] & 0xFFFF);

			int16 b = ((int16)gpu_word_read(addr + 2));
			accum += a * b;
			addr += 4;
		}
	}
	RN = res = (int32)accum;
	// carry flag to do (out of the last add)
	SET_ZN(res);
}

static void gpu_opcode_abs(void)
{
#ifdef GPU_DIS_ABS
	WriteLog("%06X: ABS    R%02u [NCZ:%u%u%u, R%02u=%08X] -> ", gpu_pc-2, IMM_2, gpu_flag_n, gpu_flag_c, gpu_flag_z, IMM_2, RN);
#endif
	gpu_flag_c = RN >> 31;
	if (RN == 0x80000000)
	//Is 0x80000000 a positive number? If so, then we need to set C to 0 as well!
		gpu_flag_n = 1, gpu_flag_z = 0;
	else
	{
		if (gpu_flag_c)
			RN = -RN;
		gpu_flag_n = 0; SET_FLAG_Z(RN);
	}
#ifdef GPU_DIS_ABS
	WriteLog("[NCZ:%u%u%u, R%02u=%08X]\n", gpu_flag_n, gpu_flag_c, gpu_flag_z, IMM_2, RN);
#endif
}

static void gpu_opcode_div(void)	// RN / RM
{
#ifdef GPU_DIS_DIV
	WriteLog("%06X: DIV    R%02u, R%02u (%s) [NCZ:%u%u%u, R%02u=%08X, R%02u=%08X] -> ", gpu_pc-2, IMM_1, IMM_2, (gpu_div_control & 0x01 ? "16.16" : "32"), gpu_flag_n, gpu_flag_c, gpu_flag_z, IMM_1, RM, IMM_2, RN);
#endif
// NOTE: remainder is NOT calculated correctly here!
//       The original tried to get it right by checking to see if the
//       remainder was negative, but that's too late...
// The code there should do it now, but I'm not 100% sure...

	if (RM)
	{
		if (gpu_div_control & 0x01)		// 16.16 division
		{
			RN = ((UINT64)RN << 16) / RM;
			gpu_remain = ((UINT64)RN << 16) % RM;
		}
		else
		{
			RN = RN / RM;
			gpu_remain = RN % RM;
		}

		if ((gpu_remain - RM) & 0x80000000)	// If the result would have been negative...
			gpu_remain -= RM;			// Then make it negative!
	}
	else
		RN = 0xFFFFFFFF;

/*	uint32 _RM=RM;
	uint32 _RN=RN;

	if (_RM)
	{
		if (gpu_div_control & 1)
		{
			gpu_remain = (((uint64)_RN) << 16) % _RM;
			if (gpu_remain&0x80000000)
				gpu_remain-=_RM;
			RN = (((uint64)_RN) << 16) / _RM;
		}
		else
		{
			gpu_remain = _RN % _RM;
			if (gpu_remain&0x80000000)
				gpu_remain-=_RM;
			RN/=_RM;
		}
	}
	else
		RN=0xffffffff;*/
#ifdef GPU_DIS_DIV
	WriteLog("[NCZ:%u%u%u, R%02u=%08X, R%02u=%08X] Remainder: %08X\n", gpu_flag_n, gpu_flag_c, gpu_flag_z, IMM_1, RM, IMM_2, RN, gpu_remain);
#endif
}

static void gpu_opcode_imultn(void)
{
	uint32 res = (int32)((int16)RN * (int16)RM);
	gpu_acc = (int32)res;
	SET_FLAG_Z(res);
	SET_FLAG_N(res);
}

static void gpu_opcode_neg(void)
{
	UINT32 res = -RN;
	SET_ZNC_SUB(0, RN, res);
	RN = res;
}

static void gpu_opcode_shlq(void)
{
/*	int dreg = jaguar.op & 31;
	INT32 r1 = convert_zero[(jaguar.op >> 5) & 31];
	UINT32 r2 = jaguar.r[dreg];
	UINT32 res = r2 << (32 - r1);
	jaguar.r[dreg] = res;
	CLR_ZNC; SET_ZN(res); jaguar.FLAGS |= (r2 >> 30) & 2;*/

#ifdef GPU_DIS_SHLQ
	WriteLog("%06X: SHLQ   #%u, R%02u [NCZ:%u%u%u, R%02u=%08X] -> ", gpu_pc-2, 32 - IMM_1, IMM_2, gpu_flag_n, gpu_flag_c, gpu_flag_z, IMM_2, RN);
#endif
// Was a bug here...
// (Look at Aaron's code: If r1 = 32, then 32 - 32 = 0 which is wrong!)
	INT32 r1 = 32 - IMM_1;
	UINT32 res = RN << r1;
	SET_ZN(res); gpu_flag_c = (RN >> 31) & 1;
	RN = res;
#ifdef GPU_DIS_SHLQ
	WriteLog("[NCZ:%u%u%u, R%02u=%08X]\n", gpu_flag_n, gpu_flag_c, gpu_flag_z, IMM_2, RN);
#endif
}

static void gpu_opcode_shrq(void)
{
/*	int dreg = jaguar.op & 31;
	INT32 r1 = convert_zero[(jaguar.op >> 5) & 31];
	UINT32 r2 = jaguar.r[dreg];
	UINT32 res = r2 >> r1;
	jaguar.r[dreg] = res;
	CLR_ZNC; SET_ZN(res); jaguar.FLAGS |= (r2 << 1) & 2;*/

#ifdef GPU_DIS_SHRQ
	WriteLog("%06X: SHRQ   #%u, R%02u [NCZ:%u%u%u, R%02u=%08X] -> ", gpu_pc-2, gpu_convert_zero[IMM_1], IMM_2, gpu_flag_n, gpu_flag_c, gpu_flag_z, IMM_2, RN);
#endif
	INT32 r1 = gpu_convert_zero[IMM_1];
	UINT32 res = RN >> r1;
	SET_ZN(res); gpu_flag_c = RN & 1;
	RN = res;
#ifdef GPU_DIS_SHRQ
	WriteLog("[NCZ:%u%u%u, R%02u=%08X]\n", gpu_flag_n, gpu_flag_c, gpu_flag_z, IMM_2, RN);
#endif
}

static void gpu_opcode_ror(void)
{
/*	int dreg = jaguar.op & 31;
	UINT32 r1 = jaguar.r[(jaguar.op >> 5) & 31] & 31;
	UINT32 r2 = jaguar.r[dreg];
	UINT32 res = (r2 >> r1) | (r2 << (32 - r1));
	jaguar.r[dreg] = res;
	CLR_ZNC; SET_ZN(res); jaguar.FLAGS |= (r2 >> 30) & 2;*/

#ifdef GPU_DIS_ROR
	WriteLog("%06X: ROR    R%02u, R%02u [NCZ:%u%u%u, R%02u=%08X, R%02u=%08X] -> ", gpu_pc-2, IMM_1, IMM_2, gpu_flag_n, gpu_flag_c, gpu_flag_z, IMM_1, RM, IMM_2, RN);
#endif
	UINT32 r1 = RM & 0x1F;
	UINT32 res = (RN >> r1) | (RN << (32 - r1));
	SET_ZN(res); gpu_flag_c = (RN >> 31) & 1;
	RN = res;
#ifdef GPU_DIS_ROR
	WriteLog("[NCZ:%u%u%u, R%02u=%08X, R%02u=%08X]\n", gpu_flag_n, gpu_flag_c, gpu_flag_z, IMM_1, RM, IMM_2, RN);
#endif
}

static void gpu_opcode_rorq(void)
{
/*	int dreg = jaguar.op & 31;
	UINT32 r1 = convert_zero[(jaguar.op >> 5) & 31];
	UINT32 r2 = jaguar.r[dreg];
	UINT32 res = (r2 >> r1) | (r2 << (32 - r1));
	jaguar.r[dreg] = res;
	CLR_ZNC; SET_ZN(res); jaguar.FLAGS |= (r2 >> 30) & 2;*/

#ifdef GPU_DIS_RORQ
	WriteLog("%06X: RORQ   #%u, R%02u [NCZ:%u%u%u, R%02u=%08X] -> ", gpu_pc-2, gpu_convert_zero[IMM_1], IMM_2, gpu_flag_n, gpu_flag_c, gpu_flag_z, IMM_2, RN);
#endif
	UINT32 r1 = gpu_convert_zero[IMM_1 & 0x1F];
	UINT32 r2 = RN;
	UINT32 res = (r2 >> r1) | (r2 << (32 - r1));
	RN = res;
	SET_ZN(res); gpu_flag_c = (r2 >> 31) & 0x01;
#ifdef GPU_DIS_RORQ
	WriteLog("[NCZ:%u%u%u, R%02u=%08X]\n", gpu_flag_n, gpu_flag_c, gpu_flag_z, IMM_2, RN);
#endif
}

static void gpu_opcode_sha(void)
{
/*	int dreg = jaguar.op & 31;
	INT32 r1 = (INT32)jaguar.r[(jaguar.op >> 5) & 31];
	UINT32 r2 = jaguar.r[dreg];
	UINT32 res;

	CLR_ZNC;
	if (r1 < 0)
	{
		res = (r1 <= -32) ? 0 : (r2 << -r1);
		jaguar.FLAGS |= (r2 >> 30) & 2;
	}
	else
	{
		res = (r1 >= 32) ? ((INT32)r2 >> 31) : ((INT32)r2 >> r1);
		jaguar.FLAGS |= (r2 << 1) & 2;
	}
	jaguar.r[dreg] = res;
	SET_ZN(res);*/

#ifdef GPU_DIS_SHA
	WriteLog("%06X: SHA    R%02u, R%02u [NCZ:%u%u%u, R%02u=%08X, R%02u=%08X] -> ", gpu_pc-2, IMM_1, IMM_2, gpu_flag_n, gpu_flag_c, gpu_flag_z, IMM_1, RM, IMM_2, RN);
#endif
	UINT32 res;

	if ((INT32)RM < 0)
	{
		res = ((INT32)RM <= -32) ? 0 : (RN << -(INT32)RM);
		gpu_flag_c = RN >> 31;
	}
	else
	{
		res = ((INT32)RM >= 32) ? ((INT32)RN >> 31) : ((INT32)RN >> (INT32)RM);
		gpu_flag_c = RN & 0x01;
	}
	RN = res;
	SET_ZN(res);
#ifdef GPU_DIS_SHA
	WriteLog("[NCZ:%u%u%u, R%02u=%08X, R%02u=%08X]\n", gpu_flag_n, gpu_flag_c, gpu_flag_z, IMM_1, RM, IMM_2, RN);
#endif

/*	int32 sRM=(int32)RM;
	uint32 _RN=RN;

	if (sRM<0)
	{
		uint32 shift=-sRM;
		if (shift>=32) shift=32;
		gpu_flag_c=(_RN&0x80000000)>>31;
		while (shift)
		{
			_RN<<=1;
			shift--;
		}
	}
	else
	{
		uint32 shift=sRM;
		if (shift>=32) shift=32;
		gpu_flag_c=_RN&0x1;
		while (shift)
		{
			_RN=((int32)_RN)>>1;
			shift--;
		}
	}
	RN=_RN;
	SET_FLAG_Z(_RN);
	SET_FLAG_N(_RN);*/
}

static void gpu_opcode_sharq(void)
{
/*	int dreg = jaguar.op & 31;
	INT32 r1 = convert_zero[(jaguar.op >> 5) & 31];
	UINT32 r2 = jaguar.r[dreg];
	UINT32 res = (INT32)r2 >> r1;
	jaguar.r[dreg] = res;
	CLR_ZNC; SET_ZN(res); jaguar.FLAGS |= (r2 << 1) & 2;*/

#ifdef GPU_DIS_SHARQ
	WriteLog("%06X: SHARQ  #%u, R%02u [NCZ:%u%u%u, R%02u=%08X] -> ", gpu_pc-2, gpu_convert_zero[IMM_1], IMM_2, gpu_flag_n, gpu_flag_c, gpu_flag_z, IMM_2, RN);
#endif
	UINT32 res = (INT32)RN >> gpu_convert_zero[IMM_1];
	SET_ZN(res); gpu_flag_c = RN & 0x01;
	RN = res;
#ifdef GPU_DIS_SHARQ
	WriteLog("[NCZ:%u%u%u, R%02u=%08X]\n", gpu_flag_n, gpu_flag_c, gpu_flag_z, IMM_2, RN);
#endif

//OLD:
/*	uint32 shift = gpu_convert_zero[IMM_1];
	uint32 _RN = RN;

	gpu_flag_c = (_RN & 0x01);
	while (shift)
	{
		_RN = ((int32)_RN) >> 1;
		shift--;
	}
	RN = _RN;
	SET_FLAG_Z(_RN);
	SET_FLAG_N(_RN);*/
}

static void gpu_opcode_sh(void)
{
#ifdef GPU_DIS_SH
	WriteLog("%06X: SH     R%02u, R%02u [NCZ:%u%u%u, R%02u=%08X, R%02u=%08X] -> ", gpu_pc-2, IMM_1, IMM_2, gpu_flag_n, gpu_flag_c, gpu_flag_z, IMM_1, RM, IMM_2, RN);
#endif
	if (RM & 0x80000000)		// Shift left
	{
		gpu_flag_c = RN >> 31;
		RN = ((int32)RM <= -32 ? 0 : RN << -(int32)RM);
	}
	else						// Shift right
	{
		gpu_flag_c = RN & 0x01;
		RN = (RM >= 32 ? 0 : RN >> RM);
	}
	SET_ZN(RN);
#ifdef GPU_DIS_SH
	WriteLog("[NCZ:%u%u%u, R%02u=%08X, R%02u=%08X]\n", gpu_flag_n, gpu_flag_c, gpu_flag_z, IMM_1, RM, IMM_2, RN);
#endif
}

//Temporary: Testing only!
#include "gpu2.cpp"
#include "gpu3.cpp"
