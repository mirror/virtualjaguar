#define ZFLAG				0x00001
#define CFLAG				0x00002
#define NFLAG				0x00004
#define IFLAG				0x00008
static UINT8 *		condition_table=0;

#define CONDITION(x)	condition_table[(x) + ((jaguar_FLAGS & 7) << 5)]
static UINT16 *		mirror_table;




/*###################################################################################################
**	MEMORY ACCESSORS
**#################################################################################################*/

#define ROPCODE(pc)		(gpu_word_read(pc))
uint16 jaguar_ppc;
uint16 jaguar_op;
int32 jaguar_icount;
static const UINT32 convert_zero[32] =
{ 32,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31 };

static uint32 cnt_opcode=0;
int jaguargpu_execute(int cycles)
{
	int i,j;
	/* allocate the mirror table */
	if (!mirror_table)
	{
		mirror_table = (UINT16*)malloc(65536 * sizeof(mirror_table[0]));

		/* fill in the mirror table */
		if (mirror_table)
			for (i = 0; i < 65536; i++)
				mirror_table[i] = ((i >> 15) & 0x0001) | ((i >> 13) & 0x0002) |
								  ((i >> 11) & 0x0004) | ((i >> 9)  & 0x0008) |
								  ((i >> 7)  & 0x0010) | ((i >> 5)  & 0x0020) |
								  ((i >> 3)  & 0x0040) | ((i >> 1)  & 0x0080) |
								  ((i << 1)  & 0x0100) | ((i << 3)  & 0x0200) |
								  ((i << 5)  & 0x0400) | ((i << 7)  & 0x0800) |
								  ((i << 9)  & 0x1000) | ((i << 11) & 0x2000) |
								  ((i << 13) & 0x4000) | ((i << 15) & 0x8000);
	}
	
	if (!condition_table)
	{
		condition_table = (uint8*)malloc(32 * 8 * sizeof(condition_table[0]));

		/* fill in the condition table */
		if (condition_table)
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
					condition_table[i * 32 + j] = result;
				}
	}
	/* if we're halted, we shouldn't be here */
	if (!gpu_running)
	{
		return cycles;
	}
	jaguar_icount=cycles;

	do
	{
		gpu_flag_c=(gpu_flag_c?1:0);
		gpu_flag_z=(gpu_flag_z?1:0);
		gpu_flag_n=(gpu_flag_n?1:0);

//		fprintf(log_get(),"%i 0x%.8x [%i %i %i]\n",cnt_opcode++,gpu_pc,gpu_flag_c,gpu_flag_z,gpu_flag_n);
		jaguar_ppc = gpu_pc;

		/* instruction fetch */
		jaguar_op = ROPCODE(gpu_pc);
		gpu_pc += 2;

		/* parse the instruction */
		(*gpu_op_table[jaguar_op >> 10])();
		jaguar_icount--;

	} while ((jaguar_icount > 0)&&(gpu_running));

	return cycles - jaguar_icount;
}
/*###################################################################################################
**	OPCODES
**#################################################################################################*/

void abs_rn(void)
{
	int dreg = jaguar_op & 31;
	UINT32 res = gpu_reg[dreg];
	CLR_ZNC;
	if (res & 0x80000000)
	{
		gpu_reg[dreg] = res = -res;
		SET_C
	}
	SET_Z(res);
}

void add_rn_rn(void)
{
	int dreg = jaguar_op & 31;
	UINT32 r1 = gpu_reg[(jaguar_op >> 5) & 31];
	UINT32 r2 = gpu_reg[dreg];
	UINT32 res = r2 + r1;
	gpu_reg[dreg] = res;
	CLR_ZNC; SET_ZNC_ADD(r2,r1,res);
}

void addc_rn_rn(void)
{
	int dreg = jaguar_op & 31;
	UINT32 r1 = gpu_reg[(jaguar_op >> 5) & 31];
	UINT32 r2 = gpu_reg[dreg];
	UINT32 res = r2 + r1 + gpu_flag_c;
	gpu_reg[dreg] = res;
	CLR_ZNC; SET_ZNC_ADD(r2,r1,res);
}

void addq_n_rn(void)
{
	int dreg = jaguar_op & 31;
	UINT32 r1 = convert_zero[(jaguar_op >> 5) & 31];
	UINT32 r2 = gpu_reg[dreg];
	UINT32 res = r2 + r1;
	gpu_reg[dreg] = res;
	CLR_ZNC; SET_ZNC_ADD(r2,r1,res);
}

void addqmod_n_rn(void)	/* DSP only */
{
	int dreg = jaguar_op & 31;
	UINT32 r1 = convert_zero[(jaguar_op >> 5) & 31];
	UINT32 r2 = gpu_reg[dreg];
	UINT32 res = r2 + r1;
	res = (res & ~gpu_hidata) | (r2 & ~gpu_hidata);
	gpu_reg[dreg] = res;
	CLR_ZNC; SET_ZNC_ADD(r2,r1,res);
}

void addqt_n_rn(void)
{
	int dreg = jaguar_op & 31;
	UINT32 r1 = convert_zero[(jaguar_op >> 5) & 31];
	UINT32 r2 = gpu_reg[dreg];
	UINT32 res = r2 + r1;
	gpu_reg[dreg] = res;
}

void and_rn_rn(void)
{
	int dreg = jaguar_op & 31;
	UINT32 r1 = gpu_reg[(jaguar_op >> 5) & 31];
	UINT32 r2 = gpu_reg[dreg];
	UINT32 res = r2 & r1;
	gpu_reg[dreg] = res;
	CLR_ZN; SET_ZN(res);
}

void bclr_n_rn(void)
{
	int dreg = jaguar_op & 31;
	UINT32 r1 = (jaguar_op >> 5) & 31;
	UINT32 r2 = gpu_reg[dreg];
	UINT32 res = r2 & ~(1 << r1);
	gpu_reg[dreg] = res;
	CLR_ZN; SET_ZN(res);
}

void bset_n_rn(void)
{
	int dreg = jaguar_op & 31;
	UINT32 r1 = (jaguar_op >> 5) & 31;
	UINT32 r2 = gpu_reg[dreg];
	UINT32 res = r2 | (1 << r1);
	gpu_reg[dreg] = res;
	CLR_ZN; SET_ZN(res);
}

void btst_n_rn(void)
{
	UINT32 r1 = (jaguar_op >> 5) & 31;
	UINT32 r2 = gpu_reg[jaguar_op & 31];
	CLR_Z; gpu_flag_z= (~r2 >> r1) & 1;
}

void cmp_rn_rn(void)
{
	UINT32 r1 = gpu_reg[(jaguar_op >> 5) & 31];
	UINT32 r2 = gpu_reg[jaguar_op & 31];
	UINT32 res = r2 - r1;
	CLR_ZNC; SET_ZNC_SUB(r2,r1,res);
}

void cmpq_n_rn(void)
{
	UINT32 r1 = (INT8)(jaguar_op >> 2) >> 3;
	UINT32 r2 = gpu_reg[jaguar_op & 31];
	UINT32 res = r2 - r1;
	CLR_ZNC; SET_ZNC_SUB(r2,r1,res);
}

void div_rn_rn(void)
{
	int dreg = jaguar_op & 31;
	UINT32 r1 = gpu_reg[(jaguar_op >> 5) & 31];
	UINT32 r2 = gpu_reg[dreg];
	if (r1)
	{
		if (gpu_div_control & 1)
		{
			gpu_reg[dreg] = ((UINT64)r2 << 16) / r1;
			gpu_remain = ((UINT64)r2 << 16) % r1;
		}
		else
		{
			gpu_reg[dreg] = r2 / r1;
			gpu_remain = r2 % r1;
		}
	}
	else
		gpu_reg[dreg] = 0xffffffff;
}

void illegal(void)
{
}

void imacn_rn_rn(void)
{
	UINT32 r1 = gpu_reg[(jaguar_op >> 5) & 31];
	UINT32 r2 = gpu_reg[jaguar_op & 31];
	gpu_acc += (INT64)((INT16)r1 * (INT16)r2);
}

void imult_rn_rn(void)
{
	int dreg = jaguar_op & 31;
	UINT32 r1 = gpu_reg[(jaguar_op >> 5) & 31];
	UINT32 r2 = gpu_reg[dreg];
	UINT32 res = (INT16)r1 * (INT16)r2;
	gpu_reg[dreg] = res;
	CLR_ZN; SET_ZN(res);
}

void imultn_rn_rn(void)
{
	int dreg = jaguar_op & 31;
	UINT32 r1 = gpu_reg[(jaguar_op >> 5) & 31];
	UINT32 r2 = gpu_reg[dreg];
	UINT32 res = (INT16)r1 * (INT16)r2;
	gpu_acc = (INT32)res;
	CLR_ZN; SET_ZN(res);

	jaguar_op = ROPCODE(gpu_pc);
	while ((jaguar_op >> 10) == 20)
	{
		r1 = gpu_reg[(jaguar_op >> 5) & 31];
		r2 = gpu_reg[jaguar_op & 31];
		gpu_acc += (INT64)((INT16)r1 * (INT16)r2);
		gpu_pc += 2;
		jaguar_op = ROPCODE(gpu_pc);
	}
	if ((jaguar_op >> 10) == 19)
	{
		gpu_pc += 2;
		gpu_reg[jaguar_op & 31] = (UINT32)gpu_acc;
	}
}

void jr_cc_n(void)
{
	UINT32 jaguar_FLAGS;

	gpu_flag_c?(gpu_flag_c=1):(gpu_flag_c=0);
	gpu_flag_z?(gpu_flag_z=1):(gpu_flag_z=0);
	gpu_flag_n?(gpu_flag_n=1):(gpu_flag_n=0);

	jaguar_FLAGS=(gpu_flag_n<<2)|(gpu_flag_c<<1)|gpu_flag_z;

	if (CONDITION(jaguar_op & 31))
	{
		INT32 r1 = (INT8)((jaguar_op >> 2) & 0xf8) >> 2;
		UINT32 newpc = gpu_pc + r1;
		jaguar_op = ROPCODE(gpu_pc);
	//	fprintf(log_get(),"%i 0x%.8x [%i %i %i]\n",cnt_opcode++,gpu_pc,gpu_flag_c,gpu_flag_z,gpu_flag_n);
		gpu_pc = newpc;
		(*gpu_op_table[jaguar_op >> 10])();

		jaguar_icount -= 3;	/* 3 wait states guaranteed */
	}
}

void jump_cc_rn(void)
{
	UINT32 jaguar_FLAGS;

	gpu_flag_c?(gpu_flag_c=1):(gpu_flag_c=0);
	gpu_flag_z?(gpu_flag_z=1):(gpu_flag_z=0);
	gpu_flag_n?(gpu_flag_n=1):(gpu_flag_n=0);

	jaguar_FLAGS=(gpu_flag_n<<2)|(gpu_flag_c<<1)|gpu_flag_z;
	if (CONDITION(jaguar_op & 31))
	{
		UINT8 reg = (jaguar_op >> 5) & 31;

		/* special kludge for risky code in the cojag DSP interrupt handlers */
		UINT32 newpc = /*(jaguar_icount == bankswitch_icount) ?*/ gpu_reg[reg];// : gpu_reg[reg];
		jaguar_op = ROPCODE(gpu_pc);
	//	fprintf(log_get(),"%i 0x%.8x [%i %i %i]\n",cnt_opcode++,gpu_pc,gpu_flag_c,gpu_flag_z,gpu_flag_n);
		gpu_pc = newpc;
		(*gpu_op_table[jaguar_op >> 10])();

		jaguar_icount -= 3;	/* 3 wait states guaranteed */
	}
}

void load_rn_rn(void)
{
	UINT32 r1 = gpu_reg[(jaguar_op >> 5) & 31];
	gpu_reg[jaguar_op & 31] = gpu_long_read(r1);
}

void load_r14n_rn(void)
{
	UINT32 r1 = convert_zero[(jaguar_op >> 5) & 31];
	gpu_reg[jaguar_op & 31] = gpu_long_read(gpu_reg[14] + 4 * r1);
}

void load_r15n_rn(void)
{
	UINT32 r1 = convert_zero[(jaguar_op >> 5) & 31];
	gpu_reg[jaguar_op & 31] = gpu_long_read(gpu_reg[15] + 4 * r1);
}

void load_r14rn_rn(void)
{
	UINT32 r1 = gpu_reg[(jaguar_op >> 5) & 31];
	gpu_reg[jaguar_op & 31] = gpu_long_read(gpu_reg[14] + r1);
}

void load_r15rn_rn(void)
{
	UINT32 r1 = gpu_reg[(jaguar_op >> 5) & 31];
	gpu_reg[jaguar_op & 31] = gpu_long_read(gpu_reg[15] + r1);
}

void loadb_rn_rn(void)
{
	UINT32 r1 = gpu_reg[(jaguar_op >> 5) & 31];
	gpu_reg[jaguar_op & 31] = gpu_byte_read(r1);
}

void loadw_rn_rn(void)
{
	UINT32 r1 = gpu_reg[(jaguar_op >> 5) & 31];
	gpu_reg[jaguar_op & 31] = gpu_word_read(r1);
}

void loadp_rn_rn(void)	/* GPU only */
{
	UINT32 r1 = gpu_reg[(jaguar_op >> 5) & 31];
	gpu_hidata = gpu_word_read(r1);
	gpu_reg[jaguar_op & 31] = gpu_word_read(r1+4);
}

void mirror_rn(void)	/* DSP only */
{
	int dreg = jaguar_op & 31;
	UINT32 r1 = gpu_reg[dreg];
	UINT32 res = (mirror_table[r1 & 0xffff] << 16) | mirror_table[r1 >> 16];
	gpu_reg[dreg] = res;
	CLR_ZN; SET_ZN(res);
}

void mmult_rn_rn(void)
{
	int count = gpu_matrix_control & 15, i;
	int sreg = (jaguar_op >> 5) & 31;
	int dreg = jaguar_op & 31;
	UINT32 addr = gpu_pointer_to_matrix;
	INT64 accum = 0;
	UINT32 res;

	if (!(gpu_matrix_control & 0x10))
	{
		for (i = 0; i < count; i++)
		{
			accum += (INT16)(gpu_reg_bank_1[sreg + i/2] >> (16 * ((i & 1) ^ 1))) * (INT16)gpu_word_read(addr);
			addr += 2;
		}
	}
	else
	{
		for (i = 0; i < count; i++)
		{
			accum += (INT16)(gpu_reg_bank_1[sreg + i/2] >> (16 * ((i & 1) ^ 1))) * (INT16)gpu_word_read(addr);
			addr += 2 * count;
		}
	}
	gpu_reg[dreg] = res = (UINT32)accum;
	CLR_ZN; SET_ZN(res);
}

void move_rn_rn(void)
{
	gpu_reg[jaguar_op & 31] = gpu_reg[(jaguar_op >> 5) & 31];
}

void move_pc_rn(void)
{
	gpu_reg[jaguar_op & 31] = jaguar_ppc;
}

void movefa_rn_rn(void)
{
	gpu_reg[jaguar_op & 31] = gpu_alternate_reg[(jaguar_op >> 5) & 31];
}

void movei_n_rn(void)
{
	UINT32 res = ROPCODE(gpu_pc) | (ROPCODE(gpu_pc + 2) << 16);
	gpu_pc += 4;
	gpu_reg[jaguar_op & 31] = res;
}

void moveq_n_rn(void)
{
	gpu_reg[jaguar_op & 31] = (jaguar_op >> 5) & 31;
}

void moveta_rn_rn(void)
{
	gpu_alternate_reg[jaguar_op & 31] = gpu_reg[(jaguar_op >> 5) & 31];
}

void mtoi_rn_rn(void)
{
	UINT32 r1 = gpu_reg[(jaguar_op >> 5) & 31];
	gpu_reg[jaguar_op & 31] = (((INT32)r1 >> 8) & 0xff800000) | (r1 & 0x007fffff);
}

void mult_rn_rn(void)
{
	int dreg = jaguar_op & 31;
	UINT32 r1 = gpu_reg[(jaguar_op >> 5) & 31];
	UINT32 r2 = gpu_reg[dreg];
	UINT32 res = (UINT16)r1 * (UINT16)r2;
	gpu_reg[dreg] = res;
	CLR_ZN; SET_ZN(res);
}

void neg_rn(void)
{
	int dreg = jaguar_op & 31;
	UINT32 r2 = gpu_reg[dreg];
	UINT32 res = -r2;
	gpu_reg[dreg] = res;
	CLR_ZNC; SET_ZNC_SUB(0,r2,res);
}

void nop(void)
{
}

void normi_rn_rn(void)
{
	UINT32 r1 = gpu_reg[(jaguar_op >> 5) & 31];
	UINT32 res = 0;
	while ((r1 & 0xffc00000) == 0)
	{
		r1 <<= 1;
		res--;
	}
	while ((r1 & 0xff800000) != 0)
	{
		r1 >>= 1;
		res++;
	}
	gpu_reg[jaguar_op & 31] = res;
	CLR_ZN; SET_ZN(res);
}

void not_rn(void)
{
	int dreg = jaguar_op & 31;
	UINT32 res = ~gpu_reg[dreg];
	gpu_reg[dreg] = res;
	CLR_ZN; SET_ZN(res);
}

void or_rn_rn(void)
{
	int dreg = jaguar_op & 31;
	UINT32 r1 = gpu_reg[(jaguar_op >> 5) & 31];
	UINT32 r2 = gpu_reg[dreg];
	UINT32 res = r1 | r2;
	gpu_reg[dreg] = res;
	CLR_ZN; SET_ZN(res);
}

void pack_rn(void)		/* GPU only */
{
	int dreg = jaguar_op & 31;
	UINT32 r1 = gpu_reg[(jaguar_op >> 5) & 31];
	UINT32 r2 = gpu_reg[dreg];
	UINT32 res;
	if (r1 == 0)	/* PACK */
		res = ((r2 >> 10) & 0xf000) | ((r2 >> 5) & 0x0f00) | (r2 & 0xff);
	else			/* UNPACK */
		res = ((r2 & 0xf000) << 10) | ((r2 & 0x0f00) << 5) | (r2 & 0xff);
	gpu_reg[dreg] = res;
	CLR_ZN; SET_ZN(res);
}

void resmac_rn(void)
{
	gpu_reg[jaguar_op & 31] = (UINT32)gpu_acc;
}

void ror_rn_rn(void)
{
	int dreg = jaguar_op & 31;
	UINT32 r1 = gpu_reg[(jaguar_op >> 5) & 31] & 31;
	UINT32 r2 = gpu_reg[dreg];
	UINT32 res = (r2 >> r1) | (r2 << (32 - r1));
	gpu_reg[dreg] = res;
	CLR_ZNC; SET_ZN(res); gpu_flag_c = (r2 >> 30) & 2;
}

void rorq_n_rn(void)
{
	int dreg = jaguar_op & 31;
	UINT32 r1 = convert_zero[(jaguar_op >> 5) & 31];
	UINT32 r2 = gpu_reg[dreg];
	UINT32 res = (r2 >> r1) | (r2 << (32 - r1));
	gpu_reg[dreg] = res;
	CLR_ZNC; SET_ZN(res); gpu_flag_c= (r2 >> 30) & 2;
}

void sat8_rn(void)		/* GPU only */
{
	int dreg = jaguar_op & 31;
	INT32 r2 = gpu_reg[dreg];
	UINT32 res = (r2 < 0) ? 0 : (r2 > 255) ? 255 : r2;
	gpu_reg[dreg] = res;
	CLR_ZN; SET_ZN(res);
}

void sat16_rn(void)		/* GPU only */
{
	int dreg = jaguar_op & 31;
	INT32 r2 = gpu_reg[dreg];
	UINT32 res = (r2 < 0) ? 0 : (r2 > 65535) ? 65535 : r2;
	gpu_reg[dreg] = res;
	CLR_ZN; SET_ZN(res);
}

void sat16s_rn(void)		/* DSP only */
{
	int dreg = jaguar_op & 31;
	INT32 r2 = gpu_reg[dreg];
	UINT32 res = (r2 < -32768) ? -32768 : (r2 > 32767) ? 32767 : r2;
	gpu_reg[dreg] = res;
	CLR_ZN; SET_ZN(res);
}

void sat24_rn(void)			/* GPU only */
{
	int dreg = jaguar_op & 31;
	INT32 r2 = gpu_reg[dreg];
	UINT32 res = (r2 < 0) ? 0 : (r2 > 16777215) ? 16777215 : r2;
	gpu_reg[dreg] = res;
	CLR_ZN; SET_ZN(res);
}

void sat32s_rn(void)		/* DSP only */
{
	int dreg = jaguar_op & 31;
	INT32 r2 = (UINT32)gpu_reg[dreg];
	INT32 temp = gpu_acc >> 32;
	UINT32 res = (temp < -1) ? (INT32)0x80000000 : (temp > 0) ? (INT32)0x7fffffff : r2;
	gpu_reg[dreg] = res;
	CLR_ZN; SET_ZN(res);
}

void sh_rn_rn(void)
{
	int dreg = jaguar_op & 31;
	INT32 r1 = (INT32)gpu_reg[(jaguar_op >> 5) & 31];
	UINT32 r2 = gpu_reg[dreg];
	UINT32 res;

	CLR_ZNC;
	if (r1 < 0)
	{
		res = (r1 <= -32) ? 0 : (r2 << -r1);
		gpu_flag_c= (r2 >> 30) & 2;
	}
	else
	{
		res = (r1 >= 32) ? 0 : (r2 >> r1);
		gpu_flag_c= (r2 << 1) & 2;
	}
	gpu_reg[dreg] = res;
	SET_ZN(res);
}

void sha_rn_rn(void)
{
	int dreg = jaguar_op & 31;
	INT32 r1 = (INT32)gpu_reg[(jaguar_op >> 5) & 31];
	UINT32 r2 = gpu_reg[dreg];
	UINT32 res;

	CLR_ZNC;
	if (r1 < 0)
	{
		res = (r1 <= -32) ? 0 : (r2 << -r1);
		gpu_flag_c= (r2 >> 30) & 2;
	}
	else
	{
		res = (r1 >= 32) ? ((INT32)r2 >> 31) : ((INT32)r2 >> r1);
		gpu_flag_c= (r2 << 1) & 2;
	}
	gpu_reg[dreg] = res;
	SET_ZN(res);
}

void sharq_n_rn(void)
{
	int dreg = jaguar_op & 31;
	INT32 r1 = convert_zero[(jaguar_op >> 5) & 31];
	UINT32 r2 = gpu_reg[dreg];
	UINT32 res = (INT32)r2 >> r1;
	gpu_reg[dreg] = res;
	CLR_ZNC; SET_ZN(res); gpu_flag_c= (r2 << 1) & 2;
}

void shlq_n_rn(void)
{
	int dreg = jaguar_op & 31;
	INT32 r1 = convert_zero[(jaguar_op >> 5) & 31];
	UINT32 r2 = gpu_reg[dreg];
	UINT32 res = r2 << (32 - r1);
	gpu_reg[dreg] = res;
	CLR_ZNC; SET_ZN(res); gpu_flag_c= (r2 >> 30) & 2;
}

void shrq_n_rn(void)
{
	int dreg = jaguar_op & 31;
	INT32 r1 = convert_zero[(jaguar_op >> 5) & 31];
	UINT32 r2 = gpu_reg[dreg];
	UINT32 res = r2 >> r1;
	gpu_reg[dreg] = res;
	CLR_ZNC; SET_ZN(res); gpu_flag_c= (r2 << 1) & 2;
}

void store_rn_rn(void)
{
	UINT32 r1 = gpu_reg[(jaguar_op >> 5) & 31];
	gpu_long_write(r1, gpu_reg[jaguar_op & 31]);
}

void store_rn_r14n(void)
{
	UINT32 r1 = convert_zero[(jaguar_op >> 5) & 31];
	gpu_long_write(gpu_reg[14] + r1 * 4, gpu_reg[jaguar_op & 31]);
}

void store_rn_r15n(void)
{
	UINT32 r1 = convert_zero[(jaguar_op >> 5) & 31];
	gpu_long_write(gpu_reg[15] + r1 * 4, gpu_reg[jaguar_op & 31]);
}

void store_rn_r14rn(void)
{
	UINT32 r1 = gpu_reg[(jaguar_op >> 5) & 31];
	gpu_long_write(gpu_reg[14] + r1, gpu_reg[jaguar_op & 31]);
}

void store_rn_r15rn(void)
{
	UINT32 r1 = gpu_reg[(jaguar_op >> 5) & 31];
	gpu_long_write(gpu_reg[15] + r1, gpu_reg[jaguar_op & 31]);
}

void storeb_rn_rn(void)
{
	UINT32 r1 = gpu_reg[(jaguar_op >> 5) & 31];
	gpu_byte_write(r1, gpu_reg[jaguar_op & 31]);
}

void storew_rn_rn(void)
{
	UINT32 r1 = gpu_reg[(jaguar_op >> 5) & 31];
	gpu_word_write(r1, gpu_reg[jaguar_op & 31]);
}

void storep_rn_rn(void)	/* GPU only */
{
	UINT32 r1 = gpu_reg[(jaguar_op >> 5) & 31];
	gpu_long_write(r1, gpu_hidata);
	gpu_long_write(r1+4, gpu_reg[jaguar_op & 31]);
}

void sub_rn_rn(void)
{
	int dreg = jaguar_op & 31;
	UINT32 r1 = gpu_reg[(jaguar_op >> 5) & 31];
	UINT32 r2 = gpu_reg[dreg];
//	fprintf(log_get(),"r%i=0x%.8x r%i=0x%.8x\n",(jaguar_op >> 5) & 31,r1,dreg,r2);
	UINT32 res = r2 - r1;
	gpu_reg[dreg] = res;
	CLR_ZNC; SET_ZNC_SUB(r2,r1,res);
}

void subc_rn_rn(void)
{
	int dreg = jaguar_op & 31;
	UINT32 r1 = gpu_reg[(jaguar_op >> 5) & 31];
	UINT32 r2 = gpu_reg[dreg];
	UINT32 res = r2 - r1 - gpu_flag_c;
	gpu_reg[dreg] = res;
	CLR_ZNC; SET_ZNC_SUB(r2,r1,res);
}

void subq_n_rn(void)
{
	int dreg = jaguar_op & 31;
	UINT32 r1 = convert_zero[(jaguar_op >> 5) & 31];
	UINT32 r2 = gpu_reg[dreg];
	UINT32 res = r2 - r1;
	gpu_reg[dreg] = res;
	CLR_ZNC; SET_ZNC_SUB(r2,r1,res);
}

void subqmod_n_rn(void)	/* DSP only */
{
	int dreg = jaguar_op & 31;
	UINT32 r1 = convert_zero[(jaguar_op >> 5) & 31];
	UINT32 r2 = gpu_reg[dreg];
	UINT32 res = r2 - r1;
//	res = (res & ~jaguar.ctrl[D_MOD]) | (r2 & ~jaguar.ctrl[D_MOD]);
	gpu_reg[dreg] = res;
	CLR_ZNC; SET_ZNC_SUB(r2,r1,res);
}

void subqt_n_rn(void)
{
	int dreg = jaguar_op & 31;
	UINT32 r1 = convert_zero[(jaguar_op >> 5) & 31];
	UINT32 r2 = gpu_reg[dreg];
	UINT32 res = r2 - r1;
	gpu_reg[dreg] = res;
}

void xor_rn_rn(void)
{
	int dreg = jaguar_op & 31;
	UINT32 r1 = gpu_reg[(jaguar_op >> 5) & 31];
	UINT32 r2 = gpu_reg[dreg];
	UINT32 res = r1 ^ r2;
	gpu_reg[dreg] = res;
	CLR_ZN; SET_ZN(res);
}
