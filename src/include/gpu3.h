void gpu3_init(void);

/*###################################################################################################
**	FUNCTION TABLES
**#################################################################################################*/

void abs_rn(void);
void add_rn_rn(void);
void addc_rn_rn(void);
void addq_n_rn(void);
void addqmod_n_rn(void);	/* DSP only */
void addqt_n_rn(void);
void and_rn_rn(void);
void bclr_n_rn(void);
void bset_n_rn(void);
void btst_n_rn(void);
void cmp_rn_rn(void);
void cmpq_n_rn(void);
void div_rn_rn(void);
void illegal(void);
void imacn_rn_rn(void);
void imult_rn_rn(void);
void imultn_rn_rn(void);
void jr_cc_n(void);
void jump_cc_rn(void);
void load_rn_rn(void);
void load_r14n_rn(void);
void load_r15n_rn(void);
void load_r14rn_rn(void);
void load_r15rn_rn(void);
void loadb_rn_rn(void);
void loadw_rn_rn(void);
void loadp_rn_rn(void);	/* GPU only */
void mirror_rn(void);	/* DSP only */
void mmult_rn_rn(void);
void move_rn_rn(void);
void move_pc_rn(void);
void movefa_rn_rn(void);
void movei_n_rn(void);
void moveq_n_rn(void);
void moveta_rn_rn(void);
void mtoi_rn_rn(void);
void mult_rn_rn(void);
void neg_rn(void);
void nop(void);
void normi_rn_rn(void);
void not_rn(void);
void or_rn_rn(void);
void pack_rn(void);		/* GPU only */
void resmac_rn(void);
void ror_rn_rn(void);
void rorq_n_rn(void);
void sat8_rn(void);		/* GPU only */
void sat16_rn(void);		/* GPU only */
void sat16s_rn(void);		/* DSP only */
void sat24_rn(void);			/* GPU only */
void sat32s_rn(void);		/* DSP only */
void sh_rn_rn(void);
void sha_rn_rn(void);
void sharq_n_rn(void);
void shlq_n_rn(void);
void shrq_n_rn(void);
void store_rn_rn(void);
void store_rn_r14n(void);
void store_rn_r15n(void);
void store_rn_r14rn(void);
void store_rn_r15rn(void);
void storeb_rn_rn(void);
void storew_rn_rn(void);
void storep_rn_rn(void);	/* GPU only */
void sub_rn_rn(void);
void subc_rn_rn(void);
void subq_n_rn(void);
void subqmod_n_rn(void);	/* DSP only */
void subqt_n_rn(void);
void xor_rn_rn(void);

void (* gpu3_opcode[64])(void) =
{
	/* 00-03 */	add_rn_rn,		addc_rn_rn,		addq_n_rn,		addqt_n_rn,
	/* 04-07 */	sub_rn_rn,		subc_rn_rn,		subq_n_rn,		subqt_n_rn,
	/* 08-11 */	neg_rn,			and_rn_rn,		or_rn_rn,		xor_rn_rn,
	/* 12-15 */	not_rn,			btst_n_rn,		bset_n_rn,		bclr_n_rn,
	/* 16-19 */	mult_rn_rn,		imult_rn_rn,	imultn_rn_rn,	resmac_rn,
	/* 20-23 */	imacn_rn_rn,	div_rn_rn,		abs_rn,			sh_rn_rn,
	/* 24-27 */	shlq_n_rn,		shrq_n_rn,		sha_rn_rn,		sharq_n_rn,
	/* 28-31 */	ror_rn_rn,		rorq_n_rn,		cmp_rn_rn,		cmpq_n_rn,
	/* 32-35 */	sat8_rn,		sat16_rn,		move_rn_rn,		moveq_n_rn,
	/* 36-39 */	moveta_rn_rn,	movefa_rn_rn,	movei_n_rn,		loadb_rn_rn,
	/* 40-43 */	loadw_rn_rn,	load_rn_rn,		loadp_rn_rn,	load_r14n_rn,
	/* 44-47 */	load_r15n_rn,	storeb_rn_rn,	storew_rn_rn,	store_rn_rn,
	/* 48-51 */	storep_rn_rn,	store_rn_r14n,	store_rn_r15n,	move_pc_rn,
	/* 52-55 */	jump_cc_rn,		jr_cc_n,		mmult_rn_rn,	mtoi_rn_rn,
	/* 56-59 */	normi_rn_rn,	nop,			load_r14rn_rn,	load_r15rn_rn,
	/* 60-63 */	store_rn_r14rn,	store_rn_r15rn,	sat24_rn,		pack_rn
};

void (* dsp3_opcode[64])(void) =
{
	/* 00-03 */	add_rn_rn,		addc_rn_rn,		addq_n_rn,		addqt_n_rn,
	/* 04-07 */	sub_rn_rn,		subc_rn_rn,		subq_n_rn,		subqt_n_rn,
	/* 08-11 */	neg_rn,			and_rn_rn,		or_rn_rn,		xor_rn_rn,
	/* 12-15 */	not_rn,			btst_n_rn,		bset_n_rn,		bclr_n_rn,
	/* 16-19 */	mult_rn_rn,		imult_rn_rn,	imultn_rn_rn,	resmac_rn,
	/* 20-23 */	imacn_rn_rn,	div_rn_rn,		abs_rn,			sh_rn_rn,
	/* 24-27 */	shlq_n_rn,		shrq_n_rn,		sha_rn_rn,		sharq_n_rn,
	/* 28-31 */	ror_rn_rn,		rorq_n_rn,		cmp_rn_rn,		cmpq_n_rn,
	/* 32-35 */	subqmod_n_rn,	sat16s_rn,		move_rn_rn,		moveq_n_rn,
	/* 36-39 */	moveta_rn_rn,	movefa_rn_rn,	movei_n_rn,		loadb_rn_rn,
	/* 40-43 */	loadw_rn_rn,	load_rn_rn,		sat32s_rn,		load_r14n_rn,
	/* 44-47 */	load_r15n_rn,	storeb_rn_rn,	storew_rn_rn,	store_rn_rn,
	/* 48-51 */	mirror_rn,		store_rn_r14n,	store_rn_r15n,	move_pc_rn,
	/* 52-55 */	jump_cc_rn,		jr_cc_n,		mmult_rn_rn,	mtoi_rn_rn,
	/* 56-59 */	normi_rn_rn,	nop,			load_r14rn_rn,	load_r15rn_rn,
	/* 60-63 */	store_rn_r14rn,	store_rn_r15rn,	illegal,		addqmod_n_rn
};
