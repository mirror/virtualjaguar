//
// Alternate GPU core... Testing purposes only!
//

//#include "gpu.h"

// Random stuff from GPU.CPP

/*static uint8 * gpu_ram_8;
extern uint32 gpu_pc;
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

static uint32 gpu_opcode_first_parameter;
static uint32 gpu_opcode_second_parameter;*/

//

const INT32 qtable[32] = 
{ 32, 1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15,
  16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31 };

const INT32 sqtable[32] = 
{ 0,   1,   2,   3,   4,   5,   6,   7,  8,  9,  10, 11, 12, 13, 14, 15,
  -16, -15, -14, -13, -12, -11, -10, -9, -8, -7, -6, -5, -4, -3, -2, -1 };

const UINT8 gpu_opcode_times[64] =
{  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
   3, 3, 1, 3, 1,18, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
   3, 3, 2, 2, 2, 2, 3, 6, 6, 4, 6, 6, 6, 1, 1, 1,
   1, 2, 2, 2, 1, 1,20, 3, 3, 1, 6, 6, 2, 2, 3, 3 };

UINT8 jump_condition[32][8];

void gpu2_init(void)
{
	memset(jump_condition, 0, 32 * 8 * sizeof(UINT8));

	for(int j=0; j<32; j++)
	{
		for(int i=0; i<8; i++)
		{
			UINT8 r = 1;
			if(j & 0x1) {
				if(i & 0x1)
					r = 0;
			}
			if(j & 0x2) {
				if(!(i & 0x1))
					r = 0;
			}
			if(j & 0x4) {
				if(i & (0x2 << (j >> 4)))
					r = 0;
			}
			if(j & 0x8) {
				if(!(i & (0x2 << (j >> 4))))
					r = 0;
			}
			jump_condition[j][i] = r;
		}
	}
}

//		case 22:  // ABS
void opcode_abs(void)
{
				int d = RN;
				if(d & 0x80000000) {
					d = abs(d);
					gpu_flag_c = 1;
				} else {
					gpu_flag_c = 0;
				}
				RN = d;
				gpu_flag_z = d == 0 ? 1 : 0;
				gpu_flag_n = 0;
}

//		case 0:   // ADD
void opcode_add(void)
{
				int s = RM;
				int d = RN;
				INT64 r = s + d;
				gpu_flag_c = r & 0x100000000 ? 1 : 0;
				RN = r;
				gpu_flag_z = RN == 0 ? 1 : 0;
				gpu_flag_n = RN & 0x80000000 ? 1 : 0;
}
//			break;
//		case 1:   // ADDC
void opcode_addc(void)
{
				int s = RM;
				int d = RN;
				int c = gpu_flag_c;
				INT64 r = s + d + c;
				gpu_flag_c = r & 0x100000000 ? 1 : 0;
				RN = r;
				gpu_flag_z = RN == 0 ? 1 : 0;
				gpu_flag_n = RN & 0x80000000 ? 1 : 0;
}
//		    break;
//		case 2:   // ADDQ
void opcode_addq(void)
{
				int s = qtable[IMM_1];
				int d = RN;
				INT64 r = s + d;
				gpu_flag_c = r & 0x100000000 ? 1 : 0;
				RN = r;
				gpu_flag_z = RN == 0 ? 1 : 0;
				gpu_flag_n = RN & 0x80000000 ? 1 : 0;
}
//		    break;
//		case 3:   // ADDQT
void opcode_addqt(void)
{
				RN += qtable[IMM_1];
}
//			break;
//		case 9:   // AND
void opcode_and(void)
{
				RN &= RM;
				gpu_flag_z = RN == 0 ? 1 : 0;
				gpu_flag_n = RN & 0x80000000 ? 1 : 0;
}
//			break;
//		case 15:  // BLCR
void opcode_bclr(void)
{
				RN &= ~(1 << IMM_1);
				gpu_flag_z = RN == 0 ? 1 : 0;
				gpu_flag_n = RN & 0x80000000 ? 1 : 0;
}
//		    break;
//		case 14:  // BSET
void opcode_bset(void)
{
				RN |= 1 << IMM_1;
				gpu_flag_z = RN == 0 ? 1 : 0;
				gpu_flag_n = RN & 0x80000000 ? 1 : 0;
}
//			break;
//		case 13:  // BTST
void opcode_btst(void)
{
				gpu_flag_z = RN & (1 << IMM_1) ? 0 : 1;
}
//		    break;
//		case 30:  // CMP
void opcode_cmp(void)
{
				int s = RM;
				int d = RN;
				gpu_flag_c = (unsigned int)d < (unsigned int)s;
				d -= s;
				gpu_flag_z = d == 0 ? 1 : 0;
				gpu_flag_n = d & 0x80000000 ? 1 : 0;
}
//			break;
//		case 31:  // CMPQ
void opcode_cmpq(void)
{
				int s = sqtable[IMM_1];
				int d = RN;
				gpu_flag_c = (unsigned int)d < (unsigned int)s;
				d -= s;
				gpu_flag_z = d == 0 ? 1 : 0;
				gpu_flag_n = d & 0x80000000 ? 1 : 0;
}
//			break;
//		case 21:  // DIV
void opcode_div(void)
{
				if(RM != 0) {
					if(gpu_div_control == 0) {
						UINT32 q = RN;
						UINT32 d = RM;
						UINT32 r = q / d;
						UINT32 r2 = q % d;
						RN = r;
						gpu_remain = r2;
					} else {
						UINT64 q = (UINT64)(RN)<<16;
						UINT64 d = (UINT64)(RM);
						UINT32 r = (UINT64)(q / d);
						UINT32 r2 = (UINT64)(q % d);
						RN = r;
						gpu_remain = r2;
					}
				}
}
//			break;
//		case 20:  // IMACN   
void opcode_imacn(void)
{
				short s = RM;
				short d = RN;
				int r = s * d;
				gpu_acc += r;
}
//			break;
//		case 17:  // IMULT
void opcode_imult(void)
{
				short s = RM;
				short d = RN;
				int r = s * d;
				RN = r;
				gpu_flag_z = r == 0 ? 1 : 0;
				gpu_flag_n = r & 0x80000000 ? 1 : 0;
}
//			break;
//		case 18:  // IMULTN
void opcode_imultn(void)
{
				short s = RM;
				short d = RN;
				int r = s * d;
				gpu_acc = r;
				gpu_flag_z = r == 0 ? 1 : 0;
				gpu_flag_n = r & 0x80000000 ? 1 : 0;
}
//			break;
//        case 53:  // JR;
void opcode_jr(void)
{
			UINT32 dw = (gpu_flag_z & 0x1) | ((gpu_flag_n & 0x1) << 2) | ((gpu_flag_c & 0x1) << 1);
			if (jump_condition[IMM_2][dw])
			{
if (gpu_start_log)
	fprintf(log_get(), "    --> JR: Branch taken. ");
				signed int offset = IMM_1 & 0x10 ? (0xFFFFFFF0 | (IMM_1 & 0xF)) : (IMM_1 & 0xF);
				UINT32 delayed_jump_address = gpu_pc + 2 + (offset * 2);
//				delayed_jump = 1;
				gpu_pc += 2;
				gpu_exec(1);
//				gpu_pc = delayed_jump_address;
				gpu_pc = delayed_jump_address - 2;
			}
}
//			break;
//		case 52:  // JUMP
void opcode_jump(void)
{
			UINT32 dw = (gpu_flag_z & 0x1) | ((gpu_flag_n & 0x1) << 2) | ((gpu_flag_c & 0x1) << 1);
			if (jump_condition[IMM_2][dw])
			{
if (gpu_start_log)
	fprintf(log_get(), "    --> JUMP: Branch taken. ");
				UINT32 delayed_jump_address = RM & 0xFFFFFE;
//				delayed_jump = 1;
				gpu_pc += 2;
				gpu_exec(1);
//				gpu_pc = delayed_jump_address;
				gpu_pc = delayed_jump_address - 2;
			}
}
//			break;
//		case 41:  // LOAD
void opcode_load(void)
{
				UINT32 address = RM;
				if(address >= 0xF03000 && address < 0xF04000) {
//					RN = _rotl(*(UINT32*)(&MEM[address]),16);
					RN = gpu_long_read(address);
				} else if(address >= 0xF0B000 && address < 0xF0C000) {
//					RN = _rotl(*(UINT32*)(&MEM[address-0x8000]),16);
					RN = gpu_long_read(address-0x8000);
				} else {
//					RN = ReadMem32(address);
					RN = jaguar_long_read(address);
				}
}
//			break;
//		case 43:  // LOAD (R14+m)
void opcode_load_r14_indexed(void)
{
				UINT32 address = gpu_reg[14] + (qtable[IMM_1] << 2);
				if(address >= 0xF03000 && address < 0xF04000) {
//					RN = _rotl(*(UINT32*)(&MEM[address]),16);
					RN = gpu_long_read(address);
				} else if(address >= 0xF0B000 && address < 0xF0C000) {
//					RN = _rotl(*(UINT32*)(&MEM[address-0x8000]),16);
					RN = gpu_long_read(address-0x8000);
				} else {
//					RN = ReadMem32(address);
					RN = jaguar_long_read(address);
				}
}
//			break;
//		case 44:  // LOAD (R15+m)
void opcode_load_r15_indexed(void)
{
				UINT32 address = gpu_reg[15] + (qtable[IMM_1] << 2);
				if(address >= 0xF03000 && address < 0xF04000) {
//					RN = _rotl(*(UINT32*)(&MEM[address]),16);
					RN = gpu_long_read(address);
				} else if(address >= 0xF0B000 && address < 0xF0C000) {
//					RN = _rotl(*(UINT32*)(&MEM[address-0x8000]),16);
					RN = gpu_long_read(address-0x8000);
				} else {
//					RN = ReadMem32(address);
					RN = jaguar_long_read(address);
				}
}
//			break; 
//		case 58:  // LOAD (R14+Rm)
void opcode_load_r14_ri(void)
{
				UINT32 address = gpu_reg[14] + RM;
				if(address >= 0xF03000 && address < 0xF04000) {
//					RN = _rotl(*(UINT32*)(&MEM[address]),16);
					RN = gpu_long_read(address);
				} else if(address >= 0xF0B000 && address < 0xF0C000) {
//					RN = _rotl(*(UINT32*)(&MEM[address-0x8000]),16);
					RN = gpu_long_read(address-0x8000);
				} else {
//					RN = ReadMem32(address);
					RN = jaguar_long_read(address);
				}
}
//			break;
//		case 59:  // LOAD (R15+Rm)
void opcode_load_r15_ri(void)
{
				UINT32 address = gpu_reg[15] + RM;
				if(address >= 0xF03000 && address < 0xF04000) {
//					RN = _rotl(*(UINT32*)(&MEM[address]),16);
					RN = gpu_long_read(address);
				} else if(address >= 0xF0B000 && address < 0xF0C000) {
//					RN = _rotl(*(UINT32*)(&MEM[address-0x8000]),16);
					RN = gpu_long_read(address-0x8000);
				} else {
//					RN = ReadMem32(address);
					RN = jaguar_long_read(address);
				}
}
//			break;
//		case 39:  // LOADB
void opcode_loadb(void)
{
			if(RM >= 0xF03000 && RM < 0xF04000) {
//				RN = ReadMem32(RM);
				RN = gpu_long_read(RM);
			} else {
//				RN = ReadMem8(RM);
				RN = jaguar_byte_read(RM);
			}
}
//			break;
//		case 40:  // LOADW
void opcode_loadw(void)
{
			if(RM >= 0xF03000 && RM < 0xF04000) {
//				RN = ReadMem32(RM);
				RN = gpu_long_read(RM);
			} else {
//				RN = ReadMem16(RM);
				RN = jaguar_word_read(RM);
			}
}
//			break;
//		case 42:  // LOADP
void opcode_loadp(void)
{
			if(RM >= 0xF03000 && RM < 0xF04000) {
//				RN = ReadMem32(RM);
				RN = gpu_long_read(RM);
			} else {
//				RN = ReadMem32(RM);
//				gpu_hidata = ReadMem32(RM+4);
				RN = gpu_long_read(RM);
				gpu_hidata = gpu_long_read(RM + 4);
			}
}
//			break;
//		case 34:  // MOVE
void opcode_move(void)
{
				RN = RM;
}
//			break;
//		case 51:  // MOVE PC,Rn
void opcode_move_pc(void)
{
				RN = gpu_pc;
}
//			break;
//		case 37:  // MOVEFA
void opcode_movefa(void)
{
				RN = gpu_alternate_reg[IMM_1];
}
//			break;
//		case 38:  // MOVEI
void opcode_movei(void)
{
	// This instruction is followed by 32-bit value in LSW / MSW format...
//	RN = (uint32)gpu_word_read(gpu_pc) | ((uint32)gpu_word_read(gpu_pc + 2) << 16);
//	gpu_pc += 4;
//					RN = _rotl(*(UINT32*)(&MEM[address]),16);
//				RN = *(UINT32*)(&MEM[gpu_pc+2]);
				RN = _rotl(gpu_long_read(gpu_pc + 2), 16);
				gpu_pc += 4;
}
//			break;
//        case 35:  // MOVEQ
void opcode_moveq(void)
{
				RN = IMM_1;
}
//			break;
//		case 36:  // MOVETA
void opcode_moveta(void)
{
				gpu_alternate_reg[IMM_2] = RM;
}
//			break;
//		case 55:  // MTOI
void opcode_mtoi(void)
{
				int d = RN & 0x7FFFFF;
				if(RN & 0x80000000) {
					d |= 0xFF800000;
				}
				RN = d;
				gpu_flag_z = d == 0 ? 1 : 0;
				gpu_flag_n = d & 0x80000000 ? 1 : 0;
}
//			break;
//		case 16:  // MULT
void opcode_mult(void)
{
				unsigned short s = RM;
				unsigned short d = RN;
				int r = s * d;
				RN = r;
				gpu_flag_z = r == 0 ? 1 : 0;
				gpu_flag_n = r & 0x80000000 ? 1 : 0;
}
//			break;
//		case 54:  // MMULT
void opcode_mmult(void)
{
				int size = gpu_matrix_control & 0xF;
				int address = gpu_pointer_to_matrix;
				int add;
				if (gpu_matrix_control & 0x10)
					add = size * 4;
				else
					add = 4;
				int result = 0;
				for(int i=0; i<size; i++)
				{
					short m, r;
//					m = ReadMem16(address+2);
					m = gpu_word_read(address + 2);
					if (i & 0x1)
						r = gpu_alternate_reg[IMM_1+(i>>1)] >> 16;
					else
						r = (gpu_alternate_reg[IMM_1+(i>>1)] & 0xFFFF);
					result += (int)(r * m);
/*					int mult = r*m;
					__asm {
						mov eax,[result]
						mov edx,[mult]
						add eax,edx
						setc [gpu_flag_c]
						mov [result],eax
					}*/

					address += add;
				}
				RN = result;
				gpu_flag_n = (result < 0) ? 1 : 0;
				gpu_flag_z = (result == 0) ? 1 : 0;
}
//			break;
//		case 8:   // NEG
void opcode_neg(void)
{
				int s = 0;
				int d = RN;
				gpu_flag_c = d - s < d;
				d = s - d;
				RN = d;
				gpu_flag_z = d == 0 ? 1 : 0;
				gpu_flag_n = d & 0x80000000 ? 1 : 0;
}
//			break;
//		case 57:  // NOP
void opcode_nop(void)
{
}
//			break;
//		case 56:  // NORMI
void opcode_normi(void)
{
				/*unsigned int d = RN;
				int r = 0;
				while ((d & 0xffc00000) == 0)
				{
					d <<= 1;
					r--;
				}
				while ((d & 0xff800000) != 0)
				{
					d >>= 1;
					r++;
				}
				RN = r;
				gpu_flag_z = r == 0 ? 1 : 0;
				gpu_flag_n = r & 0x80000000 ? 1 : 0;*/
				RN = 0;
}
//			break;
//		case 12:  // NOT
void opcode_not(void)
{
				int d = RN;
				d ^= 0xFFFFFFFF;
				RN = d;
				gpu_flag_z = d == 0 ? 1 : 0;
				gpu_flag_n = d & 0x80000000 ? 1 : 0;
}
//			break;
//		case 10:  // OR
void opcode_or(void)
{
				int s = RM;
				int d = RN;
				d |= s;
				RN = d;
				gpu_flag_z = d == 0 ? 1 : 0;
				gpu_flag_n = d & 0x80000000 ? 1 : 0;
}
//			break;
//		case 63:  // PACK / UNPACK
void opcode_pack(void)
{
				if (IMM_1 == 0)
				{
					int c1 = (RN & 0x3C00000) >> 10;
					int c2 = (RN & 0x1E000) >> 5;
					int y = (RN & 0xFF);
					RN = c1 | c2 | y;
				}
				else
				{
					int c1 = (RN & 0xF000) << 10;
					int c2 = (RN & 0xF00) << 5;
					int y = (RN & 0xFF);
					RN = c1 | c2 | y;
				}
}
//			break;
//		case 19:  // RESMAC
void opcode_resmac(void)
{
				RN = gpu_acc;
}
//			break;
//		case 28:  // ROR
void opcode_ror(void)
{
				unsigned int d = RN;
				int shift = RM;
				gpu_flag_c = d & 0x80000000 ? 1 : 0;
				d = _rotr(d, shift);
				RN = d;
				gpu_flag_z = d == 0 ? 1 : 0;
				gpu_flag_n = d & 0x80000000 ? 1 : 0;
}
//			break;
//		case 29:  // RORQ
void opcode_rorq(void)
{
				unsigned int d = RN;
				int shift = qtable[IMM_1];
				gpu_flag_c = d & 0x80000000 ? 1 : 0;
				d = _rotr(d, shift);
				RN = d;
				gpu_flag_z = d == 0 ? 1 : 0;
				gpu_flag_n = d & 0x80000000 ? 1 : 0;
}
//			break;
//		case 32:  // SAT8
void opcode_sat8(void)
{
				int d = RN;
				if(d < 0)
					d = 0;
				if(d > 255)
					d = 255;
				RN = d;
				gpu_flag_z = d == 0 ? 1 : 0;
				gpu_flag_n = 0;
}
//			break;
//		case 33:  // SAT16
void opcode_sat16(void)
{
				int d = RN;
				if(d < 0)
					d = 0;
				if(d > 65535)
					d = 65535;
				RN = d;
				gpu_flag_z = d == 0 ? 1 : 0;
				gpu_flag_n = 0;
}
//			break;
//		case 62:  // SAT24
void opcode_sat24(void)
{
				int d = RN;
				if (d < 0)
					d = 0;
				if (d > 16777215)
					d = 16777215;
				RN = d;
				gpu_flag_z = d == 0 ? 1 : 0;
				gpu_flag_n = 0;
}
//			break;
//		case 23:  // SH
void opcode_sh(void)
{
				int shift = RM;
				if (shift & 0x80000000)
				{
					gpu_flag_c = RN & 0x80000000 ? 1 : 0;
					UINT32 d = RN;
					d <<= 0-shift;
					RN = d;
					gpu_flag_z = RN == 0 ? 1 : 0;
					gpu_flag_n = RN & 0x80000000 ? 1 : 0;
				}
				else
				{
					gpu_flag_c = RN & 0x1 ? 1 : 0;
					UINT32 d = RN;
					d >>= shift;
					RN = d;
					gpu_flag_z = RN == 0 ? 1 : 0;
					gpu_flag_n = RN & 0x80000000 ? 1 : 0;
				}
}
//			break;
//		case 26:  // SHA
void opcode_sha(void)
{
				int shift = RM;
				if(shift & 0x80000000) {
					gpu_flag_c = RN & 0x80000000 ? 1 : 0;
					INT32 d = RN;
					d <<= 0-shift;
					RN = d;
					gpu_flag_z = RN == 0 ? 1 : 0;
					gpu_flag_n = RN & 0x80000000 ? 1 : 0;
				} else {
					gpu_flag_c = RN & 0x1 ? 1 : 0;
					INT32 d = RN;
					d >>= shift;
					RN = d;
					gpu_flag_z = RN == 0 ? 1 : 0;
					gpu_flag_n = RN & 0x80000000 ? 1 : 0;
				}
}
//			break;
//		case 27:  // SHARQ
void opcode_sharq(void)
{
				INT32 d = RN;
				int shift = qtable[IMM_1];
				gpu_flag_c = d & 0x1 ? 1 : 0;
				d >>= shift;
				RN = d;
				gpu_flag_z = d == 0 ? 1 : 0;
				gpu_flag_n = d & 0x80000000 ? 1 : 0;
}
//			break;
//		case 24:  // SHLQ
void opcode_shlq(void)
{
				UINT32 d = RN;
				int shift = 32 - IMM_1;
				gpu_flag_c = d & 0x80000000 ? 1 : 0;
				d <<= shift;
				RN = d;
				gpu_flag_z = d == 0 ? 1 : 0;
				gpu_flag_n = d & 0x80000000 ? 1 : 0;
}
//			break;
 //       case 25:  // SHRQ
void opcode_shrq(void)
{
				UINT32 d = RN;
				int shift = qtable[IMM_1];
				gpu_flag_c = d & 0x1 ? 1 : 0;
				d >>= shift;
				RN = d;
				gpu_flag_z = d == 0 ? 1 : 0;
				gpu_flag_n = d & 0x80000000 ? 1 : 0;
}
//			break;
//		case 47:  // STORE
void opcode_store(void)
{
				UINT32 address = RM;
				if(address >= 0xF03000 && address < 0xF04000) {
//					*(UINT32*)(&MEM[address]) = _rotl(RN,16);
					gpu_long_write(address, RN);
				} else if(address >= 0xF0B000 && address < 0xF0C000) {
//					*(UINT32*)(&MEM[address-0x8000]) = _rotl(RN,16);
					gpu_long_write(address-0x8000, RN);
				} else {
//					WriteMem32(address,RN);
					jaguar_long_write(address, RN);
				}
}
//			break;
//		case 49:  // STORE (R14+m)
void opcode_store_r14_indexed(void)
{
				UINT32 address = gpu_reg[14] + (qtable[IMM_1] << 2);
				if(address >= 0xF03000 && address < 0xF04000) {
//					*(UINT32*)(&MEM[address]) = _rotl(RN,16);
					gpu_long_write(address, RN);
				} else if(address >= 0xF0B000 && address < 0xF0C000) {
//					*(UINT32*)(&MEM[address-0x8000]) = _rotl(RN,16);
					gpu_long_write(address-0x8000, RN);
				} else {
//					WriteMem32(address,RN);
					jaguar_long_write(address, RN);
				}
}
//			break;
//		case 50:  // STORE (R15+m)
void opcode_store_r15_indexed(void)
{
				UINT32 address = gpu_reg[15] + (qtable[IMM_1] << 2);
				if(address >= 0xF03000 && address < 0xF04000) {
//					*(UINT32*)(&MEM[address]) = _rotl(RN,16);
					gpu_long_write(address, RN);
				} else if(address >= 0xF0B000 && address < 0xF0C000) {
//					*(UINT32*)(&MEM[address-0x8000]) = _rotl(RN,16);
					gpu_long_write(address-0x8000, RN);
				} else {
//					WriteMem32(address,RN);
					jaguar_long_write(address, RN);
				}
}
//			break;
//		case 60:  // STORE (R14+Rm)
void opcode_store_r14_ri(void)
{
				UINT32 address = gpu_reg[14] + RM;
				if(address >= 0xF03000 && address < 0xF04000) {
//					*(UINT32*)(&MEM[address]) = _rotl(RN,16);
					gpu_long_write(address, RN);
				} else if(address >= 0xF0B000 && address < 0xF0C000) {
//					*(UINT32*)(&MEM[address-0x8000]) = _rotl(RN,16);
					gpu_long_write(address-0x8000, RN);
				} else {
//					WriteMem32(address,RN);
					jaguar_long_write(address, RN);
				}
}
//			break;
//		case 61:  // STORE (R15+Rm)
void opcode_store_r15_ri(void)
{
				UINT32 address = gpu_reg[15] + RM;
				if(address >= 0xF03000 && address < 0xF04000) {
//					*(UINT32*)(&MEM[address]) = _rotl(RN,16);
					gpu_long_write(address, RN);
				} else if(address >= 0xF0B000 && address < 0xF0C000) {
//					*(UINT32*)(&MEM[address-0x8000]) = _rotl(RN,16);
					gpu_long_write(address-0x8000, RN);
				} else {
//					WriteMem32(address,RN);
					jaguar_long_write(address, RN);
				}
}
//			break;
//		case 45:  // STOREB
void opcode_storeb(void)
{
			if(RM>0xF03000 && RM<0xF04000) {
//				WriteMem32(RM,RN);
				gpu_long_write(RM, RN);
			} else {
//				WriteMem8(RM,(UINT8)RN);
				jaguar_byte_write(RM, (UINT8)RN);
			}
}
//			break;
//		case 46:  // STOREW
void opcode_storew(void)
{
			if(RM>0xF03000 && RM<0xF04000) {
//				WriteMem32(RM,RN);
				gpu_long_write(RM, RN);
			} else {
//				WriteMem16(RM,(WORD)RN);
				jaguar_word_write(RM, (UINT16)RN);
			}
}
//			break;
//		case 48:  // STOREP
void opcode_storep(void)
{
			if (RM>0xF03000 && RM<0xF04000)
			{
//				WriteMem32(RM,RN);
				gpu_long_write(RM, RN);
			}
			else
			{
//				WriteMem32(RM,RN);
//				WriteMem32(RM+4,gpu_hidata);
				jaguar_long_write(RM, RN);
				jaguar_long_write(RM + 4, gpu_hidata);
			}
}
//			break;
//		case 4:   // SUB
void opcode_sub(void)
{
				int s = RM;
				int d = RN;
				INT64 r = d - s;
				gpu_flag_c = r & 0x100000000 ? 1 : 0;
				RN = r;
				gpu_flag_z = RN == 0 ? 1 : 0;
				gpu_flag_n = RN & 0x80000000 ? 1 : 0;
}
//			break;
//		case 5:   // SUBC
void opcode_subc(void)
{
				int s = RM;
				int d = RN;
				int c = gpu_flag_c;
				INT64 r = d - s - c;
				gpu_flag_c = r & 0x100000000 ? 1 : 0;
				RN = r;
				gpu_flag_z = RN == 0 ? 1 : 0;
				gpu_flag_n = RN & 0x80000000 ? 1 : 0;
}
//			break;
//		case 6:   // SUBQ
void opcode_subq(void)
{
				int s = qtable[IMM_1];
				int d = RN;
				INT64 r = d - s;
				gpu_flag_c = r & 0x100000000 ? 1 : 0;
				RN = r;
				gpu_flag_z = RN == 0 ? 1 : 0;
				gpu_flag_n = RN & 0x80000000 ? 1 : 0;
}
//			break;
//		case 7:   // SUBQT
void opcode_subqt(void)
{
				RN -= qtable[IMM_1];
}
//			break;
//		case 11:  // XOR
void opcode_xor(void)
{
				int s = RM;
				int d = RN;
				d ^= s;
				RN = d;
				gpu_flag_z = d == 0 ? 1 : 0;
				gpu_flag_n = d & 0x80000000 ? 1 : 0;
}
