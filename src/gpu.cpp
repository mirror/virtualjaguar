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

#include "include/gpu.h"

#define CINT0FLAG			0x00200
#define CINT1FLAG			0x00400
#define CINT2FLAG			0x00800
#define CINT3FLAG			0x01000
#define CINT4FLAG			0x02000
#define CINT04FLAGS			(CINT0FLAG | CINT1FLAG | CINT2FLAG | CINT3FLAG | CINT4FLAG)

extern int start_logging;


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
//static uint16	*gpu_ram_16;
//static uint32	*gpu_ram_32;


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
static uint8 gpu_flag_z;
static uint8 gpu_flag_n;
static uint8 gpu_flag_c;    
static uint8 gpu_alternate_flag_z;
static uint8 gpu_alternate_flag_n;
static uint8 gpu_alternate_flag_c;    
static uint32 * gpu_reg;
static uint32 * gpu_alternate_reg;
static uint32 * gpu_reg_bank_0;
static uint32 * gpu_reg_bank_1;

static uint32 gpu_opcode_first_parameter;
static uint32 gpu_opcode_second_parameter;

#define gpu_running (gpu_control&0x01)

#define Rm gpu_reg[gpu_opcode_first_parameter]
#define Rn gpu_reg[gpu_opcode_second_parameter]
#define alternate_Rm gpu_alternate_reg[gpu_opcode_first_parameter]
#define alternate_Rn gpu_alternate_reg[gpu_opcode_second_parameter]
#define imm_1 gpu_opcode_first_parameter
#define imm_2 gpu_opcode_second_parameter

#define set_flag_z(r) gpu_flag_z=(r==0); 
#define set_flag_n(r) gpu_flag_n=((r&0x80000000)>>31);

#define reset_flag_z()	gpu_flag_z=0;
#define reset_flag_n()	gpu_flag_n=0;
#define reset_flag_c()	gpu_flag_c=0;    

uint32 gpu_convert_zero[32] = { 32,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31 };

uint8 * branch_condition_table = 0;
#define branch_condition(x)	branch_condition_table[(x) + ((jaguar_flags & 7) << 5)]

uint32 gpu_opcode_use[64];

void gpu_update_register_banks(void);

char *gpu_opcode_str[64]= 
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
void gpu_releaseTimeslice(void)
{
	gpu_releaseTimeSlice_flag = 1;
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
uint32 gpu_get_pc(void)
{
	return gpu_pc;
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
void build_branch_condition_table(void)
{
#define ZFLAG	0x00001
#define CFLAG	0x00002
#define NFLAG	0x00004
	
	if (!branch_condition_table)
	{
		branch_condition_table = (uint8*)malloc(32 * 8 * sizeof(branch_condition_table[0]));

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
	if ((offset >= gpu_work_ram_base) && (offset < gpu_work_ram_base+0x1000))
		return gpu_ram_8[offset & 0xFFF];
	else if ((offset >= gpu_control_ram_base) && (offset < gpu_control_ram_base+0x20))
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
	if ((offset >= gpu_work_ram_base) && (offset < gpu_work_ram_base+0x1000))
	{
		offset &= 0xFFF;
		uint16 data = (((uint16)gpu_ram_8[offset]) << 8) | (uint16)gpu_ram_8[offset+1];
		return data;
	}
	else if ((offset >= gpu_control_ram_base) && (offset < gpu_control_ram_base+0x20))
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

	return jaguar_word_read(offset);
}

//
// GPU dword access (read)
//

unsigned gpu_long_read(unsigned int offset)
{

	if ((offset >= gpu_work_ram_base) && (offset < gpu_work_ram_base+0x1000))
	{
		offset &= 0xFFF;
		return ((uint32)gpu_ram_8[offset] << 24) | ((uint32)gpu_ram_8[offset+1] << 16)
			| ((uint32)gpu_ram_8[offset+2] << 8) | (uint32)gpu_ram_8[offset+3];
	}
	else if ((offset >= gpu_control_ram_base) && (offset < gpu_control_ram_base+0x20))
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

	return (jaguar_word_read(offset) << 16) | jaguar_word_read(offset+2);
}

//
// GPU byte access (write)
//

void gpu_byte_write(unsigned offset, unsigned data)
{
	if ((offset >= gpu_work_ram_base) && (offset < gpu_work_ram_base+0x1000))
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
	else if ((offset >= gpu_control_ram_base) && (offset < gpu_control_ram_base+0x20))
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
//	fprintf(log_get(),"gpu: writing %.2x at 0x%.8x\n",data,offset);
	jaguar_byte_write(offset, data);
}

//
// GPU word access (write)
//

void gpu_word_write(unsigned offset, unsigned data)
{

	if ((offset >= gpu_work_ram_base) && (offset < gpu_work_ram_base+0x1000))
	{

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
	if ((offset >= gpu_control_ram_base) && (offset < gpu_control_ram_base+0x20))
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
//	fprintf(log_get(),"gpu: writing %.4x at 0x%.8x\n",data,offset);
	jaguar_word_write(offset, data);
}

//
// GPU dword access (write)
//

void gpu_long_write(unsigned offset, unsigned data)
{

	if ((offset >= gpu_work_ram_base) && (offset < gpu_work_ram_base+0x1000))
	{
		gpu_ram_8[offset & 0xFFF] = (data >> 24) & 0xFF;
		gpu_ram_8[(offset+1) & 0xFFF] = (data >> 16) & 0xFF;
		gpu_ram_8[(offset+2) & 0xFFF] = (data >> 8) & 0xFF;
		gpu_ram_8[(offset+3) & 0xFFF] = data & 0xFF;
		return;
	}
	else if ((offset >= gpu_control_ram_base) && (offset < gpu_control_ram_base+0x20))
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
			gpu_pointer_to_matrix=data;
			break;
		case 0x0C:
			gpu_data_organization=data;
			break;
		case 0x10:
			gpu_pc = data;	/*fprintf(log_get(),"setting gpu pc to 0x%.8x\n",gpu_pc);*/
			break;
		case 0x14:
		{	
			uint32 gpu_was_running = gpu_running;
						
			data &= (~0x7C0); // disable writes to irq pending
			/*if (gpu_running)
			{
				fprintf(log_get(),"gpu pc is 0x%.8x\n",gpu_pc);
				fclose(log_get());
				exit(0);
			}*/
			// check for GPU->CPU interrupt
			if (data & 0x02)
			{
//				fprintf(log_get(),"GPU->CPU interrupt\n");
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
				//fprintf(log_get(),"CPU->GPU interrupt\n");
				gpu_set_irq_line(0, 1);
//				s68000releaseTimeslice();
				m68k_end_timeslice();
				dsp_releaseTimeslice();
				data &= ~(0x04);
			}
			// single stepping
			if (data & 0x10)
			{
				//fprintf(log_get(),"asked to perform a single step (single step is %senabled)\n",(data&0x8)?"":"not ");
			}
			gpu_control = (gpu_control & 0x107C0) | (data & (~0x107C0));

			// if gpu wasn't running but is now running, execute a few cycles
#ifndef GPU_SINGLE_STEPPING
			if ((!gpu_was_running) && (gpu_running))
				gpu_exec(200);
#else
			if (gpu_control & 0x18)
				gpu_exec(1);
#endif
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
//	fprintf(log_get(),"gpu: writing %.8x at 0x%.8x\n",data,offset);
	jaguar_word_write(offset, (data >> 16) & 0xFFFF);
	jaguar_word_write(offset+2, data & 0xFFFF);
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
void gpu_update_register_banks(void)
{
	uint32 temp;
	int bank = (gpu_flags & 0x4000);

//	fprintf(log_get(),"gpu_update_register_banks at gpu pc 0x%.8x bank=%i iflag=%i\n",gpu_pc,bank?1:0,(gpu_flags&0x8)?1:0);

	if (gpu_flags & 0x8) 
		bank = 0;

	if ((!bank && (gpu_reg_bank_0 != gpu_reg)) || (bank && (gpu_reg_bank_1 != gpu_reg)))
	{
//		fprintf(log_get(),"\tswitching to bank %i\n",bank?1:0);
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
//		fprintf(log_get(),"\tnot switching banks\n");
//	}
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

	if (gpu_flags & 0x8) 
		return;

	if (start_logging)
		fprintf(log_get(),"gpu: generating irg  %i\n",which);

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
	gpu_pc = gpu_work_ram_base;
	gpu_pc += which * 0x10;
	gpu_reg[30] = gpu_pc;
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
void gpu_set_irq_line(int irqline, int state)
{
	if (start_logging)
		fprintf(log_get(),"gpu: setting irg line %i\n",irqline);
	int mask = 0x40 << irqline;
	gpu_control &= ~mask;

	if (state)
	{
		gpu_control |= mask;
		gpu_check_irqs();
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
void gpu_init(void)
{
	memory_malloc_secure((void **)&gpu_ram_8, 0x1000, "GPU work ram");
//	gpu_ram_16=(uint16*)gpu_ram_8;
//	gpu_ram_32=(uint32*)gpu_ram_8;

	memory_malloc_secure((void **)&gpu_reg, 32*sizeof(int32), "GPU bank 0 regs");
	memory_malloc_secure((void **)&gpu_alternate_reg, 32*sizeof(int32), "GPU bank 1 regs");
	
	build_branch_condition_table();

	gpu_reset();
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
	gpu_control			  = 0x00012800;
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

	reset_flag_z();
	reset_flag_n();
	reset_flag_c();

	gpu_alternate_flag_z = 0;
	gpu_alternate_flag_n = 0;
	gpu_alternate_flag_c = 0;

	memset(gpu_ram_8, 0xFF, 0x1000);

	gpu_reset_stats();
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
uint32 gpu_read_pc(void)
{
	return gpu_pc;
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
void gpu_reset_stats(void)
{
	for(uint32 i=0; i<64; i++)
		gpu_opcode_use[i] = 0;
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
void gpu_done(void)
{ 
	int i, j;
	fprintf(log_get(),"gpu: stopped at pc=0x%.8x (gpu %s running)\n",gpu_pc,gpu_running?"was":"wasn't");
	int bits, mask;

	// get the active interrupt bits 
	bits = (gpu_control >> 6) & 0x1F;
	bits |= (gpu_control >> 10) & 0x20;

	// get the interrupt mask 
	mask = (gpu_flags >> 4) & 0x1F;
	mask |= (gpu_flags >> 11) & 0x20;
	

	fprintf(log_get(),"gpu: bits=0x%.8x mask=0x%.8x\n",bits,mask);
//	fprintf(log_get(),"\nregisters bank 0\n");
//	for (int j=0;j<8;j++)
//	{
//		fprintf(log_get(),"\tr%2i=0x%.8x r%2i=0x%.8x r%2i=0x%.8x r%2i=0x%.8x\n",
//						  (j<<2)+0,gpu_reg[(j<<2)+0],
//						  (j<<2)+1,gpu_reg[(j<<2)+1],
//						  (j<<2)+2,gpu_reg[(j<<2)+2],
//						  (j<<2)+3,gpu_reg[(j<<2)+3]);
//
//	}
//	fprintf(log_get(),"registers bank 1\n");
//	for (j=0;j<8;j++)
//	{
//		fprintf(log_get(),"\tr%2i=0x%.8x r%2i=0x%.8x r%2i=0x%.8x r%2i=0x%.8x\n",
//						  (j<<2)+0,gpu_alternate_reg[(j<<2)+0],
//						  (j<<2)+1,gpu_alternate_reg[(j<<2)+1],
//						  (j<<2)+2,gpu_alternate_reg[(j<<2)+2],
//						  (j<<2)+3,gpu_alternate_reg[(j<<2)+3]);
//
//	}
//	fprintf(log_get(),"---[GPU code at 0x00f03000]---------------------------\n");
	static char buffer[512];
	j = 0xF03000;
	for(int i=0; i<4096; i++)
	{
		uint32 oldj = j;
		j += dasmjag(JAGUAR_GPU, buffer, j);
		fprintf(log_get(),"\t%08X: %s\n", oldj, buffer);
	}

	fprintf(log_get(), "---[GPU code at %08X]---------------------------\n", gpu_pc);
	j = gpu_pc - 64;
	for(i=0; i<4096; i++)
	{
		uint32 oldj = j;
		j += dasmjag(JAGUAR_GPU, buffer, j);
		fprintf(log_get(), "\t%08X: %s\n", oldj, buffer);
	}

	fprintf(log_get(), "gpu opcodes use:\n");
	for(i=0; i<64; i++)
	{
		if (gpu_opcode_use[i])
			fprintf(log_get(), "\t%s %lu\n", gpu_opcode_str[i], gpu_opcode_use[i]);
	}
	memory_free(gpu_ram_8);
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
void gpu_exec(int32 cycles)
{
	if (!gpu_running)
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

	while ((cycles > 0) && gpu_running)
	{
		gpu_flag_c = (gpu_flag_c ? 1 : 0);
		gpu_flag_z = (gpu_flag_z ? 1 : 0);
		gpu_flag_n = (gpu_flag_n ? 1 : 0);
	
		uint16 opcode = gpu_word_read(gpu_pc);

		uint32 index = opcode >> 10;		
		gpu_opcode_first_parameter = (opcode & 0x3E0) >> 5;
		gpu_opcode_second_parameter = (opcode & 0x1F);
		gpu_pc += 2;
		gpu_opcode[index]();
		cycles -= gpu_opcode_cycles[index];
		gpu_opcode_use[index]++;
	}

	gpu_in_exec--;
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
static void gpu_opcode_jump(void)
{
	uint32 delayed_pc = Rm;
	uint32 jaguar_flags;

	// normalize flags
	gpu_flag_c = (gpu_flag_c ? 1 : 0);
	gpu_flag_z = (gpu_flag_z ? 1 : 0);
	gpu_flag_n = (gpu_flag_n ? 1 : 0);

	jaguar_flags = (gpu_flag_n << 2) | (gpu_flag_c << 1) | gpu_flag_z;

	if (branch_condition(imm_2))
	{
		gpu_exec(1);
		gpu_pc = delayed_pc;
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
static void gpu_opcode_jr(void)
{
	int32 offset=(imm_1&0x10) ? (0xFFFFFFF0|imm_1) : imm_1;

	int32 delayed_pc = gpu_pc + (offset * 2);
	uint32 jaguar_flags;

	// normalize flags
	gpu_flag_c=gpu_flag_c?1:0;
	gpu_flag_z=gpu_flag_z?1:0;
	gpu_flag_n=gpu_flag_n?1:0;
	
	jaguar_flags=(gpu_flag_n<<2)|(gpu_flag_c<<1)|gpu_flag_z;

	if (branch_condition(imm_2))
	{
		gpu_exec(1);
		gpu_pc=delayed_pc;
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
static void gpu_opcode_add(void)
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
	"addl %1, %2					\n\
	setc  _gpu_flag_c				\n\
	setz  _gpu_flag_z				\n\
	sets  _gpu_flag_n				\n\
	movl %%eax, %0					\n\
	"
	: "=m"(res)
	: "d"(_Rm), "a"(_Rn));
	
	#else

	asm(
	"addl %1, %2					\n\
	setc  gpu_flag_c				\n\
	setz  gpu_flag_z				\n\
	sets  gpu_flag_n				\n\
	movl %%eax, %0					\n\
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
		setc  [gpu_flag_c]
		setz  [gpu_flag_z]
		sets  [gpu_flag_n]
		mov	  res,eax
	};
#endif
	Rn=res;
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
static void gpu_opcode_addc(void)
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
	"addl %1, %2					\n\
	cmp	  $0, _gpu_flag_c			\n\
	clc								\n\
	jz 1f							\n\
	stc								\n\
	1:								\n\
	adc %1, %2						\n\
	setc  _gpu_flag_c				\n\
	setz  _gpu_flag_z				\n\
	sets  _gpu_flag_n				\n\
	movl %%eax, %0					\n\
	"
	: "=m"(res)
	: "d"(_Rm), "a"(_Rn));

	#else
	
	asm(
	"addl %1, %2					\n\
	cmp	  $0, gpu_flag_c			\n\
	clc								\n\
	jz 1f							\n\
	stc								\n\
	1:								\n\
	adc %1, %2						\n\
	setc  gpu_flag_c				\n\
	setz  gpu_flag_z				\n\
	sets  gpu_flag_n				\n\
	movl %%eax, %0					\n\
	"
	: "=m"(res)
	: "d"(_Rm), "a"(_Rn));

	#endif
	
#else
	__asm 
	{
		mov   edx,_Rm
		mov   eax,_Rn
		cmp	  [gpu_flag_c],0
		clc
		jz	  gpu_opcode_addc_no_carry
		stc
gpu_opcode_addc_no_carry:
		adc   eax,edx
		setc  [gpu_flag_c]
		setz  [gpu_flag_z]
		sets  [gpu_flag_n]
		mov	  res,eax
	};
#endif
	Rn=res;
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
static void gpu_opcode_addq(void)
{
	uint32 _Rn=Rn;
	uint32 _Rm=gpu_convert_zero[imm_1];
	uint32 res;
#ifdef __PORT__

    /*
       GCC on WIN32 (more importantly mingw) doesn't know the declared
       variables in asm until we put a _ before it.
       
       So the declaration dsp_flag_c needs to be _dsp_flag_c on mingw.
    */

    #ifdef __GCCWIN32__

	asm(
	"addl %1, %2					\n\
	setc  _gpu_flag_c				\n\
	setz  _gpu_flag_z				\n\
	sets  _gpu_flag_n				\n\
	movl %%eax, %0					\n\
	"
	: "=m"(res)
	: "d"(_Rm), "a"(_Rn));
	
	#else

	asm(
	"addl %1, %2					\n\
	setc  gpu_flag_c				\n\
	setz  gpu_flag_z				\n\
	sets  gpu_flag_n				\n\
	movl %%eax, %0					\n\
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
		setc  [gpu_flag_c]
		setz  [gpu_flag_z]
		sets  [gpu_flag_n]
		mov	  res,eax
	};
#endif
	Rn=res;
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
static void gpu_opcode_addqt(void)
{
	Rn += gpu_convert_zero[imm_1];
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
static void gpu_opcode_sub(void)
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
	"subl %1, %2					\n\
	setc  _gpu_flag_c				\n\
	setz  _gpu_flag_z				\n\
	sets  _gpu_flag_n				\n\
	movl %%eax, %0					\n\
	"
	: "=m"(res)
	: "d"(_Rm), "a"(_Rn));
	
	#else
	
	asm(
	"subl %1, %2					\n\
	setc  gpu_flag_c				\n\
	setz  gpu_flag_z				\n\
	sets  gpu_flag_n				\n\
	movl %%eax, %0					\n\
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
		setc  [gpu_flag_c]
		setz  [gpu_flag_z]
		sets  [gpu_flag_n]
		mov	  res,eax
	};
#endif
	Rn=res;
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
static void gpu_opcode_subc(void)
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
	"addl %1, %2					\n\
	cmp	  $0, _gpu_flag_c			\n\
	clc								\n\
	jz 1f							\n\
	stc								\n\
	1:								\n\
	sbb %1, %2						\n\
	setc  _gpu_flag_c				\n\
	setz  _gpu_flag_z				\n\
	sets  _gpu_flag_n				\n\
	movl %%eax, %0					\n\
	"
	: "=m"(res)
	: "d"(_Rm), "a"(_Rn));
	
	#else
	
	asm(
	"addl %1, %2					\n\
	cmp	  $0, gpu_flag_c			\n\
	clc								\n\
	jz 1f							\n\
	stc								\n\
	1:								\n\
	sbb %1, %2						\n\
	setc  gpu_flag_c				\n\
	setz  gpu_flag_z				\n\
	sets  gpu_flag_n				\n\
	movl %%eax, %0					\n\
	"
	: "=m"(res)
	: "d"(_Rm), "a"(_Rn));

	#endif
	
#else
	__asm 
	{
		cmp	  [gpu_flag_c],0
		clc
		jz	  gpu_opcode_subc_no_carry
		stc
gpu_opcode_subc_no_carry:
		mov   edx,_Rm
		mov   eax,_Rn
		sbb   eax,edx
		setc  [gpu_flag_c]
		setz  [gpu_flag_z]
		sets  [gpu_flag_n]
		mov	  res,eax
	};
#endif
	Rn=res;
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
static void gpu_opcode_subq(void)
{
	uint32 _Rm=gpu_convert_zero[imm_1];
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
	"subl %1, %2					\n\
	setc  _gpu_flag_c				\n\
	setz  _gpu_flag_z				\n\
	sets  _gpu_flag_n				\n\
	movl %%eax, %0					\n\
	"
	: "=m"(res)
	: "d"(_Rm), "a"(_Rn));
	
	#else
	
	asm(
	"subl %1, %2					\n\
	setc  gpu_flag_c				\n\
	setz  gpu_flag_z				\n\
	sets  gpu_flag_n				\n\
	movl %%eax, %0					\n\
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
		setc  [gpu_flag_c]
		setz  [gpu_flag_z]
		sets  [gpu_flag_n]
		mov	  res,eax
	};
#endif
	Rn=res;
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
static void gpu_opcode_subqt(void)
{
	Rn -= gpu_convert_zero[imm_1];
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
static void gpu_opcode_cmp(void)
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
	"cmpl %0, %1					\n\
	setc  _gpu_flag_c				\n\
	setz  _gpu_flag_z				\n\
	sets  _gpu_flag_n				\n\
	"
	:
	: "d"(_Rm), "a"(_Rn));
	
	#else
	
	asm(
	"cmpl %0, %1					\n\
	setc  gpu_flag_c				\n\
	setz  gpu_flag_z				\n\
	sets  gpu_flag_n				\n\
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
		setc  [gpu_flag_c]
		setz  [gpu_flag_z]
		sets  [gpu_flag_n]
	};
#endif
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
static void gpu_opcode_cmpq(void)
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
	"cmpl %0, %1					\n\
	setc  _gpu_flag_c				\n\
	setz  _gpu_flag_z				\n\
	sets  _gpu_flag_n				\n\
	"
	:
	: "d"(_Rm), "a"(_Rn));
	
	#else

	asm(
	"cmpl %0, %1					\n\
	setc  gpu_flag_c				\n\
	setz  gpu_flag_z				\n\
	sets  gpu_flag_n				\n\
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
		setc  [gpu_flag_c]
		setz  [gpu_flag_z]
		sets  [gpu_flag_n]
	};
#endif
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
static void gpu_opcode_and(void)
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
	"andl %1, %2					\n\
	setz  _gpu_flag_z				\n\
	sets  _gpu_flag_n				\n\
	movl %%eax, %0					\n\
	"
	: "=m"(res)
	: "d"(_Rm), "a"(_Rn));
	
	#else

	asm(
	"andl %1, %2					\n\
	setz  gpu_flag_z				\n\
	sets  gpu_flag_n				\n\
	movl %%eax, %0					\n\
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
		setz  [gpu_flag_z]
		sets  [gpu_flag_n]
		mov   res,eax
	};
#endif
	Rn=res;
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
static void gpu_opcode_or(void)
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
	"orl %1, %2						\n\
	setz  _gpu_flag_z				\n\
	sets  _gpu_flag_n				\n\
	movl %%eax, %0					\n\
	"
	: "=m"(res)
	: "d"(_Rm), "a"(_Rn));
	
	#else

	asm(
	"orl %1, %2						\n\
	setz  gpu_flag_z				\n\
	sets  gpu_flag_n				\n\
	movl %%eax, %0					\n\
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
		setz  [gpu_flag_z]
		sets  [gpu_flag_n]
		mov   res,eax
	};
#endif
	Rn=res;
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
static void gpu_opcode_xor(void)
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
	"xorl %1, %2					\n\
	setz  _gpu_flag_z				\n\
	sets  _gpu_flag_n				\n\
	movl %%eax, %0					\n\
	"
	: "=m"(res)
	: "d"(_Rm), "a"(_Rn));
	
	#else

	asm(
	"xorl %1, %2					\n\
	setz  gpu_flag_z				\n\
	sets  gpu_flag_n				\n\
	movl %%eax, %0					\n\
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
		setz  [gpu_flag_z]
		sets  [gpu_flag_n]
		mov   res,eax
	};
#endif
	Rn=res;
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
static void gpu_opcode_not(void)
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
	"notl %1						\n\
	setz  _gpu_flag_z				\n\
	sets  _gpu_flag_n				\n\
	movl %%eax, %0					\n\
	"
	: "=m"(res)
	: "a"(_Rn));
	
	#else

	asm(
	"notl %1						\n\
	setz  gpu_flag_z				\n\
	sets  gpu_flag_n				\n\
	movl %%eax, %0					\n\
	"
	: "=m"(res)
	: "a"(_Rn));
	
	#endif
#else
	__asm 
	{
		mov   eax,_Rn
		not   eax
		setz  [gpu_flag_z]
		sets  [gpu_flag_n]
		mov   res,eax
	};
#endif
	Rn=res;
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
static void gpu_opcode_move_pc(void)
{
	Rn = gpu_pc-2; 
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
static void gpu_opcode_sat8(void)
{
	int32 _Rn=(int32)Rn;

	uint32 res= Rn = (_Rn<0) ? 0 : (_Rn > 0xff ? 0xff : _Rn);
	set_flag_z(res);
	reset_flag_n();
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
static void gpu_opcode_sat16(void)
{
	int32 _Rn=(int32)Rn;
	uint32 res= Rn = (_Rn<0) ? 0 : (_Rn > 0xFFFF ? 0xFFFF : _Rn);
	set_flag_z(res);
	reset_flag_n();
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
static void gpu_opcode_sat24(void)
{
	int32 _Rn=(int32)Rn;

	uint32 res= Rn = (_Rn<0) ? 0 : (_Rn > 0xFFFFFF ? 0xFFFFFF : _Rn);
	set_flag_z(res);
	reset_flag_n();
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
static void gpu_opcode_store_r14_indexed(void)
{
	gpu_long_write( gpu_reg[14] + (gpu_convert_zero[imm_1] << 2),Rn);
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
static void gpu_opcode_store_r15_indexed(void)
{
	gpu_long_write( gpu_reg[15] + (gpu_convert_zero[imm_1] << 2),Rn);
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
static void gpu_opcode_load_r14_ri(void)
{
	Rn=gpu_long_read(gpu_reg[14] + Rm);
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
static void gpu_opcode_load_r15_ri(void)
{
	Rn=gpu_long_read(gpu_reg[15] + Rm);
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
static void gpu_opcode_store_r14_ri(void)
{
	gpu_long_write(gpu_reg[14] + Rm,Rn);
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
static void gpu_opcode_store_r15_ri(void)
{
	gpu_long_write(gpu_reg[15] + Rm,Rn);
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
static void gpu_opcode_nop(void)
{
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
static void gpu_opcode_pack(void)
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
static void gpu_opcode_storeb(void)
{
	if ((Rm >= 0xF03000) && (Rm < 0xF04000))
		gpu_long_write(Rm,Rn&0xff);
	else
		jaguar_byte_write(Rm,Rn);
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
static void gpu_opcode_storew(void)
{
	if ((Rm >= 0xF03000) && (Rm < 0xF04000))
		gpu_long_write(Rm,Rn&0xffff);
	else
		jaguar_word_write(Rm,Rn);
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
static void gpu_opcode_store(void)
{
	gpu_long_write(Rm,Rn);
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
static void gpu_opcode_storep(void)
{
	uint32 _Rm=Rm;
	gpu_long_write(_Rm,	 gpu_hidata);
	gpu_long_write(_Rm+4, Rn);
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
static void gpu_opcode_loadb(void)
{
	if ((Rm >= 0xF03000) && (Rm < 0xF04000))
		Rn=gpu_long_read(Rm)&0xff;
	else
		Rn=jaguar_byte_read(Rm);
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
static void gpu_opcode_loadw(void)
{
	if ((Rm >= 0xF03000) && (Rm < 0xF04000))
		Rn=gpu_long_read(Rm)&0xffff;
	else
		Rn=jaguar_word_read(Rm);
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
static void gpu_opcode_load(void)
{
	Rn = gpu_long_read(Rm);
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
static void gpu_opcode_loadp(void)
{
	uint32 _Rm=Rm;

	gpu_hidata = gpu_long_read(_Rm);
	Rn		   = gpu_long_read(_Rm+4);
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
static void gpu_opcode_load_r14_indexed(void)
{
	Rn = gpu_long_read( gpu_reg[14] + (gpu_convert_zero[imm_1] << 2));
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
static void gpu_opcode_load_r15_indexed(void)
{
	Rn = gpu_long_read( gpu_reg[15] + (gpu_convert_zero[imm_1] << 2));
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
static void gpu_opcode_movei(void)
{
	Rn = ((uint32)gpu_word_read(gpu_pc)) + (((uint32)gpu_word_read(gpu_pc+2))<<16);
	gpu_pc+=4;
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
static void gpu_opcode_moveta(void)
{
	alternate_Rn = Rm;
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
static void gpu_opcode_movefa(void)
{
	Rn = alternate_Rm;
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
static void gpu_opcode_move(void)
{
	Rn = Rm;
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
static void gpu_opcode_moveq(void)
{
	Rn = imm_1;    
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
static void gpu_opcode_resmac(void)
{
	Rn = gpu_acc;
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
static void gpu_opcode_imult(void)
{
	uint32 res=Rn=((int16)Rn)*((int16)Rm);
	set_flag_z(res);
	set_flag_n(res);
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
static void gpu_opcode_mult(void)
{
	uint32 res=Rn =  ((uint16)Rm) * ((uint16)Rn);
	set_flag_z(res);
	set_flag_n(res);
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
static void gpu_opcode_bclr(void)
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
	"btrl %1, %2					\n\
	cmpl $0, %2						\n\
	setz  _gpu_flag_z				\n\
	sets  _gpu_flag_n				\n\
	movl %%eax, %0					\n\
	"
	: "=m"(res)
	: "c"(_Rm), "a"(_Rn));
	
	#else
	
	asm(
	"btrl %1, %2					\n\
	cmpl $0, %2						\n\
	setz  gpu_flag_z				\n\
	sets  gpu_flag_n				\n\
	movl %%eax, %0					\n\
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
		setz  [gpu_flag_z]
		sets  [gpu_flag_n]
		mov   res,eax
	};
#endif
	Rn=res;
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
static void gpu_opcode_btst(void)
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
	"bt %0, %1						\n\
	setnc _gpu_flag_z				\n\
	"
	:
	: "c"(_Rm), "a"(_Rn));
	
	#else

	asm(
	"bt %0, %1						\n\
	setnc gpu_flag_z				\n\
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
		setnc [gpu_flag_z]
	};
#endif
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
static void gpu_opcode_bset(void)
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
	"btsl %1, %2					\n\
	cmpl $0, %2						\n\
	setz  _gpu_flag_z				\n\
	sets  _gpu_flag_n				\n\
	movl %%eax, %0					\n\
	"
	: "=m"(res)
	: "c"(_Rm), "a"(_Rn));
	
	#else
	
	asm(
	"btsl %1, %2					\n\
	cmpl $0, %2						\n\
	setz  gpu_flag_z				\n\
	sets  gpu_flag_n				\n\
	movl %%eax, %0					\n\
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
		setz  [gpu_flag_z]
		sets  [gpu_flag_n]
		mov   res,eax
	};
#endif
	Rn=res;
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
static void gpu_opcode_imacn(void)
{
	uint32 res = ((int16)Rm) * ((int16)(Rn));
	gpu_acc += res;
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
static void gpu_opcode_mtoi(void)
{
	uint32 _Rm=Rm;
	uint32 res=Rn=(((INT32)_Rm >> 8) & 0xff800000) | (_Rm & 0x007fffff);
	set_flag_z(res);
	set_flag_n(res);
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
static void gpu_opcode_normi(void)
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
static void gpu_opcode_mmult(void)
{
	int count	= gpu_matrix_control&0x0f;
	uint32 addr = gpu_pointer_to_matrix; // in the gpu ram
	int64 accum = 0;
	uint32 res;

	if (!(gpu_matrix_control & 0x10))
	{
		for (int i = 0; i < count; i++)
		{ 
			int16 a;
			if (i&0x01)
				a=(int16)((gpu_alternate_reg[gpu_opcode_first_parameter + (i>>1)]>>16)&0xffff);
			else
				a=(int16)(gpu_alternate_reg[gpu_opcode_first_parameter + (i>>1)]&0xffff);

			int16 b=((int16)gpu_word_read(addr+2));
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
				a=(int16)((gpu_alternate_reg[gpu_opcode_first_parameter + (i>>1)]>>16)&0xffff);
			else
				a=(int16)(gpu_alternate_reg[gpu_opcode_first_parameter + (i>>1)]&0xffff);

			int16 b=((int16)gpu_word_read(addr+2));
			accum += a*b;
			addr += 4 * count;
		}
	}
	Rn = res = (int32)accum;
	// carry flag to do
	set_flag_z(res);
	set_flag_n(res);
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
static void gpu_opcode_abs(void)
{
	uint32 _Rn=Rn;
	uint32 res;
	
	if (_Rn==0x80000000)
	{
		set_flag_n(1);
	}
	else
	{
		gpu_flag_c  = ((_Rn&0x80000000)>>31);
		res= Rn =  (((int32)_Rn)<0) ? -_Rn : _Rn;
		reset_flag_n();
		set_flag_z(res);
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
static void gpu_opcode_div(void)
{
	uint32 _Rm=Rm;
	uint32 _Rn=Rn;

	if (_Rm)
	{
		if (gpu_div_control & 1)
		{
			gpu_remain = (((uint64)_Rn) << 16) % _Rm;
			if (gpu_remain&0x80000000)
				gpu_remain-=_Rm;
			Rn = (((uint64)_Rn) << 16) / _Rm;
		}
		else
		{
			gpu_remain = _Rn % _Rm;
			if (gpu_remain&0x80000000)
				gpu_remain-=_Rm;
			Rn/=_Rm;
		}
	}
	else
		Rn=0xffffffff;
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
static void gpu_opcode_imultn(void)
{
	uint32 res = (int32)((int16)Rn * (int16)Rm);
	gpu_acc = (int32)res;
	set_flag_z(res);
	set_flag_n(res);
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
static void gpu_opcode_neg(void)
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
	"subl %1, %2					\n\
	setc  _gpu_flag_c				\n\
	setz  _gpu_flag_z				\n\
	sets  _gpu_flag_n				\n\
	movl %%eax, %0					\n\
	"
	: "=m"(res)
	: "d"(_Rn), "a"(0));
	
	#else

	asm(
	"subl %1, %2					\n\
	setc  gpu_flag_c				\n\
	setz  gpu_flag_z				\n\
	sets  gpu_flag_n				\n\
	movl %%eax, %0					\n\
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
		setc  [gpu_flag_c]
		setz  [gpu_flag_z]
		sets  [gpu_flag_n]
		mov	  res,eax
	};
#endif
	Rn=res;
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
static void gpu_opcode_shlq(void)
{
	uint32 shift=(32-gpu_convert_zero[imm_1]);
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
	"testl $0x80000000, %2			\n\
	setnz _gpu_flag_c				\n\
	shl %%cl, %2					\n\
	cmpl $0, %2						\n\
	setz  _gpu_flag_z				\n\
	sets  _gpu_flag_n				\n\
	movl %%eax, %0					\n\
	"
	: "=m"(res)
	: "c"(shift), "a"(_Rn));
	
	#else
	
	asm(
	"testl $0x80000000, %2			\n\
	setnz gpu_flag_c				\n\
	shl %%cl, %2					\n\
	cmpl $0, %2						\n\
	setz  gpu_flag_z				\n\
	sets  gpu_flag_n				\n\
	movl %%eax, %0					\n\
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
		setnz [gpu_flag_c]
		shl eax,cl
		cmp eax,0
		setz [gpu_flag_z]
		sets [gpu_flag_n]
		mov res,eax
	}
#endif
	Rn=res;
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
static void gpu_opcode_shrq(void)
{
	uint32 shift=gpu_convert_zero[imm_1];
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
	"testl $0x00000001, %2			\n\
	setnz _gpu_flag_c				\n\
	shr %%cl, %2					\n\
	cmpl $0, %2						\n\
	setz  _gpu_flag_z				\n\
	sets  _gpu_flag_n				\n\
	movl %%eax, %0					\n\
	"
	: "=m"(res)
	: "c"(shift), "a"(_Rn));
	
	#else

	asm(
	"testl $0x00000001, %2			\n\
	setnz gpu_flag_c				\n\
	shr %%cl, %2					\n\
	cmpl $0, %2						\n\
	setz  gpu_flag_z				\n\
	sets  gpu_flag_n				\n\
	movl %%eax, %0					\n\
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
		setnz [gpu_flag_c]
		shr eax,cl
		cmp eax,0
		setz [gpu_flag_z]
		sets [gpu_flag_n]
		mov res,eax
	}
#endif
	Rn=res;
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
static void gpu_opcode_ror(void)
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
	"testl $0x80000000, %2			\n\
	setnz _gpu_flag_c				\n\
	ror %%cl, %2					\n\
	cmpl $0, %2						\n\
	setz  _gpu_flag_z				\n\
	sets  _gpu_flag_n				\n\
	movl %%eax, %0					\n\
	"
	: "=m"(res)
	: "c"(shift), "a"(_Rn));
	
	#else

	asm(
	"testl $0x80000000, %2			\n\
	setnz gpu_flag_c				\n\
	ror %%cl, %2					\n\
	cmpl $0, %2						\n\
	setz  gpu_flag_z				\n\
	sets  gpu_flag_n				\n\
	movl %%eax, %0					\n\
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
		setnz [gpu_flag_c]
		ror eax,cl
		cmp eax,0
		setz [gpu_flag_z]
		sets [gpu_flag_n]
		mov res,eax
	}
#endif
	Rn=res;
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
static void gpu_opcode_rorq(void)
{
	uint32 shift = gpu_convert_zero[imm_1 & 0x1F];
	uint32 _Rn = Rn;
	uint32 res;
#ifdef __PORT__

    /*
       GCC on WIN32 (more importantly mingw) doesn't know the declared
       variables in asm until we put a _ before it.
       
       So the declaration dsp_flag_c needs to be _dsp_flag_c on mingw.
    */

#ifdef __GCCWIN32__

	asm(
	"testl $0x80000000, %2			\n\
	setnz _gpu_flag_c				\n\
	ror %%cl, %2					\n\
	cmpl $0, %2						\n\
	setz  _gpu_flag_z				\n\
	sets  _gpu_flag_n				\n\
	movl %%eax, %0					\n\
	"
	: "=m"(res)
	: "c"(shift), "a"(_Rn));
	
#else
	
	asm(
	"testl $0x80000000, %2			\n\
	setnz gpu_flag_c				\n\
	ror %%cl, %2					\n\
	cmpl $0, %2						\n\
	setz  gpu_flag_z				\n\
	sets  gpu_flag_n				\n\
	movl %%eax, %0					\n\
	"
	: "=m"(res)
	: "c"(shift), "a"(_Rn));

#endif	// #ifdef __GCCWIN32__
	
#else
	__asm 
	{
		mov ecx,shift
		mov eax,_Rn
		test eax,0x80000000
		setnz [gpu_flag_c]
		ror eax,cl
		cmp eax,0
		setz [gpu_flag_z]
		sets [gpu_flag_n]
		mov res,eax
	}
#endif	// #ifdef __PORT__
	Rn = res;
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
static void gpu_opcode_sha(void)
{
	int32 sRm=(int32)Rm;
	uint32 _Rn=Rn;

	if (sRm<0)
	{
		uint32 shift=-sRm;
		if (shift>=32) shift=32;
		gpu_flag_c=(_Rn&0x80000000)>>31;
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
		gpu_flag_c=_Rn&0x1;
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
static void gpu_opcode_sharq(void)
{
	uint32 shift=gpu_convert_zero[imm_1];
	uint32 _Rn=Rn;

	gpu_flag_c  = (_Rn & 0x1);
	while (shift)
	{
		_Rn=((int32)_Rn)>>1;
		shift--;
	}
	Rn=_Rn;
	set_flag_z(_Rn);
	set_flag_n(_Rn);
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
static void gpu_opcode_sh(void)
{
	int32 sRm=(int32)Rm;
	uint32 _Rn=Rn;	

	if (sRm<0)
	{
		uint32 shift=(-sRm);
		if (shift>=32) shift=32;
		gpu_flag_c=(_Rn&0x80000000)>>31;
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
		gpu_flag_c=_Rn&0x1;
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
