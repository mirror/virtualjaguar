//void gpu2_exec(int);
void gpu2_init(void);
//void gpu2_interrupt(int);
//void gpu2_set_regbank(int);
//int gpu2_get_regbank(void);

/*typedef struct {
	signed int r0[32];
	signed int r1[32];
	UINT8 z;
	UINT8 n;
	UINT8 c;
	UINT8 flags;
	UINT32 pc;
	UINT32 remain;
	int acc;
	UINT32 hidata;
	UINT32 matrix_address;
	UINT32 matrix_size;
	UINT32 irq_enable[6];
	UINT32 irq_active[6];
	UINT32 active;
	UINT32 imask;
	UINT32 regbank;
	UINT32 bcmd;
	signed int* reg;
	signed int* alt;
	UINT32 r;
	UINT32 divide;
	UINT32 single_step;
	UINT32 interrupt_stack[8];
	int i_pointer;
} GPUSTATE;*/

//extern GPUSTATE gpustate;

//extern void (* gpu2_opcode[64])();

void opcode_add(void);
void opcode_addc(void);
void opcode_addq(void);
void opcode_addqt(void);
void opcode_sub(void);
void opcode_subc(void);
void opcode_subq(void);
void opcode_subqt(void);
void opcode_neg(void);
void opcode_and(void);
void opcode_or(void);
void opcode_xor(void);
void opcode_not(void);
void opcode_btst(void);
void opcode_bset(void);
void opcode_bclr(void);
void opcode_mult(void);
void opcode_imult(void);
void opcode_imultn(void);
void opcode_resmac(void);
void opcode_imacn(void);
void opcode_div(void);
void opcode_abs(void);
void opcode_sh(void);
void opcode_shlq(void);
void opcode_shrq(void);
void opcode_sha(void);
void opcode_sharq(void);
void opcode_ror(void);
void opcode_rorq(void);
void opcode_cmp(void);
void opcode_cmpq(void);
void opcode_sat8(void);
void opcode_sat16(void);
void opcode_move(void);
void opcode_moveq(void);
void opcode_moveta(void);
void opcode_movefa(void);
void opcode_movei(void);
void opcode_loadb(void);
void opcode_loadw(void);
void opcode_load(void);
void opcode_loadp(void);
void opcode_load_r14_indexed(void);
void opcode_load_r15_indexed(void);
void opcode_storeb(void);
void opcode_storew(void);
void opcode_store(void);
void opcode_storep(void);
void opcode_store_r14_indexed(void);
void opcode_store_r15_indexed(void);
void opcode_move_pc(void);
void opcode_jump(void);
void opcode_jr(void);
void opcode_mmult(void);
void opcode_mtoi(void);
void opcode_normi(void);
void opcode_nop(void);
void opcode_load_r14_ri(void);
void opcode_load_r15_ri(void);
void opcode_store_r14_ri(void);
void opcode_store_r15_ri(void);
void opcode_sat24(void);
void opcode_pack(void);

void (* gpu2_opcode[64])()= 
{	
	opcode_add,					opcode_addc,				opcode_addq,				opcode_addqt,
	opcode_sub,					opcode_subc,				opcode_subq,				opcode_subqt,
	opcode_neg,					opcode_and,					opcode_or,					opcode_xor,
	opcode_not,					opcode_btst,				opcode_bset,				opcode_bclr,
	opcode_mult,				opcode_imult,				opcode_imultn,				opcode_resmac,
	opcode_imacn,				opcode_div,					opcode_abs,					opcode_sh,
	opcode_shlq,				opcode_shrq,				opcode_sha,					opcode_sharq,
	opcode_ror,					opcode_rorq,				opcode_cmp,					opcode_cmpq,
	opcode_sat8,				opcode_sat16,				opcode_move,				opcode_moveq,
	opcode_moveta,				opcode_movefa,				opcode_movei,				opcode_loadb,
	opcode_loadw,				opcode_load,				opcode_loadp,				opcode_load_r14_indexed,
	opcode_load_r15_indexed,	opcode_storeb,				opcode_storew,				opcode_store,
	opcode_storep,				opcode_store_r14_indexed,	opcode_store_r15_indexed,	opcode_move_pc,
	opcode_jump,				opcode_jr,					opcode_mmult,				opcode_mtoi,
	opcode_normi,				opcode_nop,					opcode_load_r14_ri,			opcode_load_r15_ri,
	opcode_store_r14_ri,		opcode_store_r15_ri,		opcode_sat24,				opcode_pack,
};
