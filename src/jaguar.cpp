//
// JAGUAR.CPP
//
// Originally by David Raingeard (Cal2)
// GCC/SDL port by Niels Wagenaar (Linux/WIN32) and Carwin Jones (BeOS)
// Cleanups and endian wrongness amelioration by James L. Hammons
// Note: Endian wrongness probably stems from the MAME origins of this emu and
//       the braindead way in which MAME handles memory. :-)
// 

#include "jaguar.h"
#include "video.h"
#include "settings.h"
//#include "m68kdasmAG.h"

#define CPU_DEBUG
//Do this in makefile??? Yes! Could, but it's easier to define here...
//#define LOG_UNMAPPED_MEMORY_ACCESSES
//#define ABORT_ON_UNMAPPED_MEMORY_ACCESS
#define ABORT_ON_ILLEGAL_INSTRUCTIONS
//#define ABORT_ON_OFFICIAL_ILLEGAL_INSTRUCTION
#define CPU_DEBUG_MEMORY

// Private function prototypes

unsigned jaguar_unknown_readbyte(unsigned address, uint32 who = UNKNOWN);
unsigned jaguar_unknown_readword(unsigned address, uint32 who = UNKNOWN);
void jaguar_unknown_writebyte(unsigned address, unsigned data, uint32 who = UNKNOWN);
void jaguar_unknown_writeword(unsigned address, unsigned data, uint32 who = UNKNOWN);
void M68K_show_context(void);

// External variables

//extern bool hardwareTypeNTSC;						// Set to false for PAL
#ifdef CPU_DEBUG_MEMORY
extern bool startMemLog;							// Set by "e" key
extern int effect_start;
extern int effect_start2, effect_start3, effect_start4, effect_start5, effect_start6;
#endif

// Memory debugging identifiers

char * whoName[9] =
	{ "Unknown", "Jaguar", "DSP", "GPU", "TOM", "JERRY", "M68K", "Blitter", "OP" };

uint32 jaguar_active_memory_dumps = 0;

uint32 jaguar_mainRom_crc32, jaguarRomSize, jaguarRunAddress;

/*static*/ uint8 * jaguar_mainRam = NULL;
/*static*/ uint8 * jaguar_bootRom = NULL;
/*static*/ uint8 * jaguar_mainRom = NULL;

#ifdef CPU_DEBUG_MEMORY
uint8 writeMemMax[0x400000], writeMemMin[0x400000];
uint8 readMem[0x400000];
uint32 returnAddr[4000], raPtr = 0xFFFFFFFF;
#endif

uint32 pcQueue[0x400];
uint32 pcQPtr = 0;

//
// Callback function to detect illegal instructions
//
//void GPUDumpDisassembly(void);
//void GPUDumpRegisters(void);
void M68KInstructionHook(void)
{
	uint32 m68kPC = m68k_get_reg(NULL, M68K_REG_PC);

// For tracebacks...
// Ideally, we'd save all the registers as well...
	pcQueue[pcQPtr++] = m68kPC;
	pcQPtr &= 0x3FF;

	if (m68kPC & 0x01)		// Oops! We're fetching an odd address!
	{
		WriteLog("M68K: Attempted to execute from an odd adress!\n\nBacktrace:\n\n");

		static char buffer[2048];
		for(int i=0; i<0x400; i++)
		{
			m68k_disassemble(buffer, pcQueue[(pcQPtr + i) & 0x3FF], M68K_CPU_TYPE_68000);
			WriteLog("\t%08X: %s\n", pcQueue[(pcQPtr + i) & 0x3FF], buffer);
		}
		WriteLog("\n");

		uint32 topOfStack = m68k_get_reg(NULL, M68K_REG_A7);
		WriteLog("M68K: Top of stack: %08X. Stack trace:\n", JaguarReadLong(topOfStack));
		for(int i=0; i<10; i++)
			WriteLog("%06X: %08X\n", topOfStack - (i * 4), JaguarReadLong(topOfStack - (i * 4)));
		WriteLog("Jaguar: VBL interrupt is %s\n", ((tom_irq_enabled(IRQ_VBLANK)) && (jaguar_interrupt_handler_is_valid(64))) ? "enabled" : "disabled");
		M68K_show_context();
		log_done();
		exit(0);
	}

/*	if (m68kPC >= 0x807EC4 && m68kPC <= 0x807EDB)
	{
		static char buffer[2048];
		m68k_disassemble(buffer, m68kPC, M68K_CPU_TYPE_68000);
		WriteLog("%08X: %s", m68kPC, buffer);
		WriteLog("\t\tA0=%08X, A1=%08X, D0=%08X, D1=%08X\n",
			m68k_get_reg(NULL, M68K_REG_A0), m68k_get_reg(NULL, M68K_REG_A1),
			m68k_get_reg(NULL, M68K_REG_D0), m68k_get_reg(NULL, M68K_REG_D1));
	}//*/
/*	if (m68kPC == 0x8D0E48 && effect_start5)
	{
		WriteLog("\nM68K: At collision detection code. Exiting!\n\n");
		GPUDumpRegisters();
		GPUDumpDisassembly();
		log_done();
		exit(0);
	}//*/
/*	uint16 opcode = JaguarReadWord(m68kPC);
	if (opcode == 0x4E75)	// RTS
	{
		if (startMemLog)
//			WriteLog("Jaguar: Returning from subroutine to %08X\n", JaguarReadLong(m68k_get_reg(NULL, M68K_REG_A7)));
		{
			uint32 addr = JaguarReadLong(m68k_get_reg(NULL, M68K_REG_A7));
			bool found = false;
			if (raPtr != 0xFFFFFFFF)
			{
				for(uint32 i=0; i<=raPtr; i++)
				{
					if (returnAddr[i] == addr)
					{
						found = true;
						break;
					}
				}
			}

			if (!found)
				returnAddr[++raPtr] = addr;
		}
	}//*/

/*	static char buffer[2048];
	m68k_disassemble(buffer, m68kPC, M68K_CPU_TYPE_68000);
	WriteLog("%08X: %s \t\tD0=%08X, A0=%08X\n", m68kPC, buffer, m68k_get_reg(NULL, M68K_REG_D0), m68k_get_reg(NULL, M68K_REG_A0));//*/

/*	if (m68kPC == 0x803F16)
	{
		WriteLog("M68K: Registers found at $803F16:\n");
		WriteLog("\t68K PC=%06X\n", m68k_get_reg(NULL, M68K_REG_PC));
		for(int i=M68K_REG_D0; i<=M68K_REG_D7; i++)
			WriteLog("\tD%i = %08X\n", i-M68K_REG_D0, m68k_get_reg(NULL, (m68k_register_t)i));
		WriteLog("\n");
		for(int i=M68K_REG_A0; i<=M68K_REG_A7; i++)
			WriteLog("\tA%i = %08X\n", i-M68K_REG_A0, m68k_get_reg(NULL, (m68k_register_t)i));
	}*/

#ifdef ABORT_ON_ILLEGAL_INSTRUCTIONS
	if (!m68k_is_valid_instruction(m68k_read_memory_16(m68kPC), M68K_CPU_TYPE_68000))
	{
#ifndef ABORT_ON_OFFICIAL_ILLEGAL_INSTRUCTION
		if (m68k_read_memory_16(m68kPC) == 0x4AFC)
		{
			// This is a kludge to let homebrew programs work properly (i.e., let the other processors
			// keep going even when the 68K dumped back to the debugger or what have you).
//dis no wok right!
//			m68k_set_reg(M68K_REG_PC, m68kPC - 2);
// Try setting the vector to the illegal instruction...
//This doesn't work right either! Do something else! Quick!
//			SET32(jaguar_mainRam, 0x10, m68kPC);

			return;
		}
#endif

		WriteLog("\nM68K encountered an illegal instruction at %08X!!!\n\nAborting!\n", m68kPC);
		uint32 topOfStack = m68k_get_reg(NULL, M68K_REG_A7);
		WriteLog("M68K: Top of stack: %08X. Stack trace:\n", JaguarReadLong(topOfStack));
		for(int i=0; i<10; i++)
			WriteLog("%06X: %08X\n", topOfStack - (i * 4), JaguarReadLong(topOfStack - (i * 4)));
		WriteLog("Jaguar: VBL interrupt is %s\n", ((tom_irq_enabled(IRQ_VBLANK)) && (jaguar_interrupt_handler_is_valid(64))) ? "enabled" : "disabled");
		M68K_show_context();

//temp
//	WriteLog("\n\n68K disasm\n\n");
//	jaguar_dasm(0x802000, 0x50C);
//	WriteLog("\n\n");
//endoftemp

		log_done();
		exit(0);
	}//*/
#endif
}

//
// Musashi 68000 read/write/IRQ functions
//

int irq_ack_handler(int level)
{
	int vector = M68K_INT_ACK_AUTOVECTOR;

	// The GPU/DSP/etc are probably *not* issuing an NMI, but it seems to work OK...

	if (level == 7)
	{
		m68k_set_irq(0);						// Clear the IRQ...
		vector = 64;							// Set user interrupt #0
	}

	return vector;
}

unsigned int m68k_read_memory_8(unsigned int address)
{
#ifdef CPU_DEBUG_MEMORY
	if ((address >= 0x000000) && (address <= 0x3FFFFF))
	{
		if (startMemLog)
			readMem[address] = 1;
	}
#endif
//WriteLog("[RM8] Addr: %08X\n", address);
	unsigned int retVal = 0;

	if ((address >= 0x000000) && (address <= 0x3FFFFF))
		retVal = jaguar_mainRam[address];
	else if ((address >= 0x800000) && (address <= 0xDFFFFF))
		retVal = jaguar_mainRom[address - 0x800000];
	else if ((address >= 0xE00000) && (address <= 0xE3FFFF))
		retVal = jaguar_bootRom[address - 0xE00000];
	else if ((address >= 0xDFFF00) && (address <= 0xDFFFFF))
		retVal = CDROMReadByte(address);
	else if ((address >= 0xF00000) && (address <= 0xF0FFFF))
		retVal = TOMReadByte(address, M68K);
	else if ((address >= 0xF10000) && (address <= 0xF1FFFF))
		retVal = JERRYReadByte(address, M68K);
	else
		retVal = jaguar_unknown_readbyte(address, M68K);

    return retVal;
}

void gpu_dump_disassembly(void);
void gpu_dump_registers(void);

unsigned int m68k_read_memory_16(unsigned int address)
{
#ifdef CPU_DEBUG_MEMORY
/*	if ((address >= 0x000000) && (address <= 0x3FFFFE))
	{
		if (startMemLog)
			readMem[address] = 1, readMem[address + 1] = 1;
	}//*/
/*	if (effect_start && (address >= 0x8064FC && address <= 0x806501))
	{
		return 0x4E71;	// NOP
	}
	if (effect_start2 && (address >= 0x806502 && address <= 0x806507))
	{
		return 0x4E71;	// NOP
	}
	if (effect_start3 && (address >= 0x806512 && address <= 0x806517))
	{
		return 0x4E71;	// NOP
	}
	if (effect_start4 && (address >= 0x806524 && address <= 0x806527))
	{
		return 0x4E71;	// NOP
	}
	if (effect_start5 && (address >= 0x80653E && address <= 0x806543)) //Collision detection!
	{
		return 0x4E71;	// NOP
	}
	if (effect_start6 && (address >= 0x806544 && address <= 0x806547))
	{
		return 0x4E71;	// NOP
	}//*/
#endif
//WriteLog("[RM16] Addr: %08X\n", address);
/*if (m68k_get_reg(NULL, M68K_REG_PC) == 0x00005FBA)
//	for(int i=0; i<10000; i++)
	WriteLog("[M68K] In routine #6!\n");//*/
//if (m68k_get_reg(NULL, M68K_REG_PC) == 0x00006696) // GPU Program #4
//if (m68k_get_reg(NULL, M68K_REG_PC) == 0x00005B3C)	// GPU Program #2
/*if (m68k_get_reg(NULL, M68K_REG_PC) == 0x00005BA8)	// GPU Program #3
{
	WriteLog("[M68K] About to run GPU! (Addr:%08X, data:%04X)\n", address, TOMReadWord(address));
	gpu_dump_registers();
	gpu_dump_disassembly();
//	for(int i=0; i<10000; i++)
//		WriteLog("[M68K] About to run GPU!\n");
}//*/
//WriteLog("[WM8  PC=%08X] Addr: %08X, val: %02X\n", m68k_get_reg(NULL, M68K_REG_PC), address, value);
/*if (m68k_get_reg(NULL, M68K_REG_PC) >= 0x00006696 && m68k_get_reg(NULL, M68K_REG_PC) <= 0x000066A8)
{
	if (address == 0x000066A0)
	{
		gpu_dump_registers();
		gpu_dump_disassembly();
	}
	for(int i=0; i<10000; i++)
		WriteLog("[M68K] About to run GPU! (Addr:%08X, data:%04X)\n", address, TOMReadWord(address));
}//*/
    unsigned int retVal = 0;

	if ((address >= 0x000000) && (address <= 0x3FFFFE))
		retVal = (jaguar_mainRam[address] << 8) | jaguar_mainRam[address+1];
	else if ((address >= 0x800000) && (address <= 0xDFFFFE))
		retVal = (jaguar_mainRom[address - 0x800000] << 8) | jaguar_mainRom[address - 0x800000 + 1];
	else if ((address >= 0xE00000) && (address <= 0xE3FFFE))
		retVal = (jaguar_bootRom[address - 0xE00000] << 8) | jaguar_bootRom[address - 0xE00000 + 1];
	else if ((address >= 0xDFFF00) && (address <= 0xDFFFFE))
		retVal = CDROMReadWord(address, M68K);
	else if ((address >= 0xF00000) && (address <= 0xF0FFFE))
		retVal = TOMReadWord(address, M68K);
	else if ((address >= 0xF10000) && (address <= 0xF1FFFE))
		retVal = JERRYReadWord(address, M68K);
	else
		retVal = jaguar_unknown_readword(address, M68K);

    return retVal;
}

unsigned int m68k_read_memory_32(unsigned int address)
{
//WriteLog("--> [RM32]\n");
    return (m68k_read_memory_16(address) << 16) | m68k_read_memory_16(address + 2);
}

void m68k_write_memory_8(unsigned int address, unsigned int value)
{
#ifdef CPU_DEBUG_MEMORY
	if ((address >= 0x000000) && (address <= 0x3FFFFF))
	{
		if (startMemLog)
		{
			if (value > writeMemMax[address])
				writeMemMax[address] = value;
			if (value < writeMemMin[address])
				writeMemMin[address] = value;
		}
	}
#endif
//if ((address >= 0x1FF020 && address <= 0x1FF03F) || (address >= 0x1FF820 && address <= 0x1FF83F))
//	WriteLog("M68K: Writing %02X at %08X\n", value, address);
//WriteLog("[WM8  PC=%08X] Addr: %08X, val: %02X\n", m68k_get_reg(NULL, M68K_REG_PC), address, value);
	if ((address >= 0x000000) && (address <= 0x3FFFFF))
		jaguar_mainRam[address] = value;
	else if ((address >= 0xDFFF00) && (address <= 0xDFFFFF))
		CDROMWriteByte(address, value, M68K);
	else if ((address >= 0xF00000) && (address <= 0xF0FFFF))
		TOMWriteByte(address, value, M68K);
	else if ((address >= 0xF10000) && (address <= 0xF1FFFF))
		JERRYWriteByte(address, value, M68K);
	else
		jaguar_unknown_writebyte(address, value, M68K);
}

void m68k_write_memory_16(unsigned int address, unsigned int value)
{
#ifdef CPU_DEBUG_MEMORY
	if ((address >= 0x000000) && (address <= 0x3FFFFE))
	{
		if (startMemLog)
		{
			uint8 hi = value >> 8, lo = value & 0xFF;

			if (hi > writeMemMax[address])
				writeMemMax[address] = hi;
			if (hi < writeMemMin[address])
				writeMemMin[address] = hi;

			if (lo > writeMemMax[address+1])
				writeMemMax[address+1] = lo;
			if (lo < writeMemMin[address+1])
				writeMemMin[address+1] = lo;
		}
	}
#endif
//if ((address >= 0x1FF020 && address <= 0x1FF03F) || (address >= 0x1FF820 && address <= 0x1FF83F))
//	WriteLog("M68K: Writing %04X at %08X\n", value, address);
//WriteLog("[WM16 PC=%08X] Addr: %08X, val: %04X\n", m68k_get_reg(NULL, M68K_REG_PC), address, value);
//if (address >= 0xF02200 && address <= 0xF0229F)
//	WriteLog("M68K: Writing to blitter --> %04X at %08X\n", value, address);
//if (address >= 0x0E75D0 && address <= 0x0E75E7)
//	WriteLog("M68K: Writing %04X at %08X, M68K PC=%08X\n", value, address, m68k_get_reg(NULL, M68K_REG_PC));
/*extern uint32 totalFrames;
if (address == 0xF02114)
	WriteLog("M68K: Writing to GPU_CTRL (frame:%u)... [M68K PC:%08X]\n", totalFrames, m68k_get_reg(NULL, M68K_REG_PC));
if (address == 0xF02110)
	WriteLog("M68K: Writing to GPU_PC (frame:%u)... [M68K PC:%08X]\n", totalFrames, m68k_get_reg(NULL, M68K_REG_PC));//*/
//if (address >= 0xF03B00 && address <= 0xF03DFF)
//	WriteLog("M68K: Writing %04X to %08X...\n", value, address);

	if ((address >= 0x000000) && (address <= 0x3FFFFE))
	{
/*		jaguar_mainRam[address] = value >> 8;
		jaguar_mainRam[address + 1] = value & 0xFF;*/
		SET16(jaguar_mainRam, address, value);
	}
	else if ((address >= 0xDFFF00) && (address <= 0xDFFFFE))
		CDROMWriteWord(address, value, M68K);
	else if ((address >= 0xF00000) && (address <= 0xF0FFFE))
		TOMWriteWord(address, value, M68K);
	else if ((address >= 0xF10000) && (address <= 0xF1FFFE))
		JERRYWriteWord(address, value, M68K);
	else
	{
		jaguar_unknown_writeword(address, value, M68K);
#ifdef LOG_UNMAPPED_MEMORY_ACCESSES
		WriteLog("\tA0=%08X, A1=%08X, D0=%08X, D1=%08X\n",
			m68k_get_reg(NULL, M68K_REG_A0), m68k_get_reg(NULL, M68K_REG_A1),
			m68k_get_reg(NULL, M68K_REG_D0), m68k_get_reg(NULL, M68K_REG_D1));
#endif
	}
}

void m68k_write_memory_32(unsigned int address, unsigned int value)
{
//WriteLog("--> [WM32]\n");
	m68k_write_memory_16(address, value >> 16);
	m68k_write_memory_16(address + 2, value & 0xFFFF);
}


uint32 jaguar_get_handler(uint32 i)
{
	return JaguarReadLong(i * 4);
}

uint32 jaguar_interrupt_handler_is_valid(uint32 i)
{
	uint32 handler = jaguar_get_handler(i);
	if (handler && (handler != 0xFFFFFFFF))
		return 1;
	else
		return 0;
}

void M68K_show_context(void)
{
	WriteLog("\t68K PC=%06X\n", m68k_get_reg(NULL, M68K_REG_PC));
	for(int i=M68K_REG_D0; i<=M68K_REG_D7; i++)
		WriteLog("\tD%i = %08X\n", i-M68K_REG_D0, m68k_get_reg(NULL, (m68k_register_t)i));
	WriteLog("\n");
	for(int i=M68K_REG_A0; i<=M68K_REG_A7; i++)
		WriteLog("\tA%i = %08X\n", i-M68K_REG_A0, m68k_get_reg(NULL, (m68k_register_t)i));

	WriteLog("68K disasm\n");
//	jaguar_dasm(s68000readPC()-0x1000,0x20000);
	jaguar_dasm(m68k_get_reg(NULL, M68K_REG_PC) - 0x80, 0x200);
//	jaguar_dasm(0x5000, 0x14414);

	WriteLog("..................\n");

	if (tom_irq_enabled(IRQ_VBLANK))
	{
		WriteLog("vblank int: enabled\n");
		jaguar_dasm(jaguar_get_handler(64), 0x200);
	}
	else
		WriteLog("vblank int: disabled\n");

	WriteLog("..................\n");

	for(int i=0; i<256; i++)
		WriteLog("handler %03i at $%08X\n", i, (unsigned int)jaguar_get_handler(i));
}

//
// Unknown read/write byte/word routines
//

// It's hard to believe that developers would be sloppy with their memory writes, yet in
// some cases the developers screwed up royal. E.g., Club Drive has the following code:
//
// 807EC4: movea.l #$f1b000, A1
// 807ECA: movea.l #$8129e0, A0
// 807ED0: move.l  A0, D0
// 807ED2: move.l  #$f1bb94, D1
// 807ED8: sub.l   D0, D1
// 807EDA: lsr.l   #2, D1
// 807EDC: move.l  (A0)+, (A1)+
// 807EDE: dbra    D1, 807edc
//
// The problem is at $807ED0--instead of putting A0 into D0, they really meant to put A1
// in. This mistake causes it to try and overwrite approximately $700000 worth of address
// space! (That is, unless the 68K causes a bus error...)

void jaguar_unknown_writebyte(unsigned address, unsigned data, uint32 who/*=UNKNOWN*/)
{
#ifdef LOG_UNMAPPED_MEMORY_ACCESSES
	WriteLog("Jaguar: Unknown byte %02X written at %08X by %s (M68K PC=%06X)\n", data, address, whoName[who], m68k_get_reg(NULL, M68K_REG_PC));
#endif
#ifdef ABORT_ON_UNMAPPED_MEMORY_ACCESS
	extern bool finished;
	finished = true;
	extern bool doDSPDis;
	if (who == DSP)
		doDSPDis = true;
#endif
}

void jaguar_unknown_writeword(unsigned address, unsigned data, uint32 who/*=UNKNOWN*/)
{
#ifdef LOG_UNMAPPED_MEMORY_ACCESSES
	WriteLog("Jaguar: Unknown word %04X written at %08X by %s (M68K PC=%06X)\n", data, address, whoName[who], m68k_get_reg(NULL, M68K_REG_PC));
#endif
#ifdef ABORT_ON_UNMAPPED_MEMORY_ACCESS
	extern bool finished;
	finished = true;
	extern bool doDSPDis;
	if (who == DSP)
		doDSPDis = true;
#endif
}

unsigned jaguar_unknown_readbyte(unsigned address, uint32 who/*=UNKNOWN*/)
{
#ifdef LOG_UNMAPPED_MEMORY_ACCESSES
	WriteLog("Jaguar: Unknown byte read at %08X by %s (M68K PC=%06X)\n", address, whoName[who], m68k_get_reg(NULL, M68K_REG_PC));
#endif
#ifdef ABORT_ON_UNMAPPED_MEMORY_ACCESS
	extern bool finished;
	finished = true;
	extern bool doDSPDis;
	if (who == DSP)
		doDSPDis = true;
#endif
    return 0xFF;
}

unsigned jaguar_unknown_readword(unsigned address, uint32 who/*=UNKNOWN*/)
{
#ifdef LOG_UNMAPPED_MEMORY_ACCESSES
	WriteLog("Jaguar: Unknown word read at %08X by %s (M68K PC=%06X)\n", address, whoName[who], m68k_get_reg(NULL, M68K_REG_PC));
#endif
#ifdef ABORT_ON_UNMAPPED_MEMORY_ACCESS
	extern bool finished;
	finished = true;
	extern bool doDSPDis;
	if (who == DSP)
		doDSPDis = true;
#endif
    return 0xFFFF;
}

//
// Disassemble M68K instructions at the given offset
//

unsigned int m68k_read_disassembler_8(unsigned int address)
{
	return m68k_read_memory_8(address);
}

unsigned int m68k_read_disassembler_16(unsigned int address)
{
	return m68k_read_memory_16(address);
}

unsigned int m68k_read_disassembler_32(unsigned int address)
{
	return m68k_read_memory_32(address);
}

void jaguar_dasm(uint32 offset, uint32 qt)
{
#ifdef CPU_DEBUG
	static char buffer[2048];//, mem[64];
	int pc = offset, oldpc;

	for(uint32 i=0; i<qt; i++)
	{
/*		oldpc = pc;
		for(int j=0; j<64; j++)
			mem[j^0x01] = jaguar_byte_read(pc + j);

		pc += Dasm68000((char *)mem, buffer, 0);
		WriteLog("%08X: %s\n", oldpc, buffer);//*/
		oldpc = pc;
		pc += m68k_disassemble(buffer, pc, M68K_CPU_TYPE_68000);
		WriteLog("%08X: %s\n", oldpc, buffer);//*/
	}
#endif
}

uint8 JaguarReadByte(uint32 offset, uint32 who/*=UNKNOWN*/)
{
	uint8 data = 0x00;

	offset &= 0xFFFFFF;
	if (offset < 0x400000)
		data = jaguar_mainRam[offset & 0x3FFFFF];
	else if ((offset >= 0x800000) && (offset < 0xC00000))
		data = jaguar_mainRom[offset - 0x800000];
	else if ((offset >= 0xDFFF00) && (offset <= 0xDFFFFF))
		data = CDROMReadByte(offset, who);
	else if ((offset >= 0xE00000) && (offset < 0xE40000))
		data = jaguar_bootRom[offset & 0x3FFFF];
	else if ((offset >= 0xF00000) && (offset < 0xF10000))
		data = TOMReadByte(offset, who);
	else if ((offset >= 0xF10000) && (offset < 0xF20000))
		data = JERRYReadByte(offset, who);
	else
		data = jaguar_unknown_readbyte(offset, who);

	return data;
}

uint16 JaguarReadWord(uint32 offset, uint32 who/*=UNKNOWN*/)
{
	offset &= 0xFFFFFF;
	if (offset <= 0x3FFFFE)
	{
		return (jaguar_mainRam[(offset+0) & 0x3FFFFF] << 8) | jaguar_mainRam[(offset+1) & 0x3FFFFF];
	}
	else if ((offset >= 0x800000) && (offset <= 0xBFFFFE))
	{
		offset -= 0x800000;
		return (jaguar_mainRom[offset+0] << 8) | jaguar_mainRom[offset+1];
	}
//	else if ((offset >= 0xDFFF00) && (offset < 0xDFFF00))
	else if ((offset >= 0xDFFF00) && (offset <= 0xDFFFFE))
		return CDROMReadWord(offset, who);
	else if ((offset >= 0xE00000) && (offset <= 0xE3FFFE))
		return (jaguar_bootRom[(offset+0) & 0x3FFFF] << 8) | jaguar_bootRom[(offset+1) & 0x3FFFF];
	else if ((offset >= 0xF00000) && (offset <= 0xF0FFFE))
		return TOMReadWord(offset, who);
	else if ((offset >= 0xF10000) && (offset <= 0xF1FFFE))
		return JERRYReadWord(offset, who);

	return jaguar_unknown_readword(offset, who);
}

void JaguarWriteByte(uint32 offset, uint8 data, uint32 who/*=UNKNOWN*/)
{
	offset &= 0xFFFFFF;
	if (offset < 0x400000)
	{
		jaguar_mainRam[offset & 0x3FFFFF] = data;
		return;
	}
	else if ((offset >= 0xDFFF00) && (offset <= 0xDFFFFF))
	{
		CDROMWriteByte(offset, data, who);
		return;
	}
	else if ((offset >= 0xF00000) && (offset <= 0xF0FFFF))
	{
		TOMWriteByte(offset, data, who);
		return;
	}
	else if ((offset >= 0xF10000) && (offset <= 0xF1FFFF))
	{
		JERRYWriteByte(offset, data, who);
		return;
	}
    
	jaguar_unknown_writebyte(offset, data, who);
}

void JaguarWriteWord(uint32 offset, uint16 data, uint32 who/*=UNKNOWN*/)
{
//TEMP--Mirror of F03000? Yes, but only 32-bit CPUs can do it (i.e., NOT the 68K!)
// PLUS, you would handle this in the GPU/DSP WriteLong code! Not here!
	offset &= 0xFFFFFF;

	if (offset <= 0x3FFFFE)
	{
//This MUST be done by the 68K!
/*if (offset == 0x670C)
	WriteLog("Jaguar: %s writing to location $670C...\n", whoName[who]);*/

		jaguar_mainRam[(offset+0) & 0x3FFFFF] = (data>>8) & 0xFF;
		jaguar_mainRam[(offset+1) & 0x3FFFFF] = data & 0xFF;
		return;
	}
	else if (offset >= 0xDFFF00 && offset <= 0xDFFFFE)
	{
		CDROMWriteWord(offset, data, who);
		return;
	}
	else if (offset >= 0xF00000 && offset <= 0xF0FFFE)
	{
		TOMWriteWord(offset, data, who);
		return;
	}
	else if (offset >= 0xF10000 && offset <= 0xF1FFFE)
	{
		JERRYWriteWord(offset, data, who);
		return;
	}
	// Don't bomb on attempts to write to ROM
	else if (offset >= 0x800000 && offset <= 0xEFFFFF)
		return;

	jaguar_unknown_writeword(offset, data, who);
}

// We really should re-do this so that it does *real* 32-bit access... !!! FIX !!!
uint32 JaguarReadLong(uint32 offset, uint32 who/*=UNKNOWN*/)
{
	return (JaguarReadWord(offset, who) << 16) | JaguarReadWord(offset+2, who);
}

// We really should re-do this so that it does *real* 32-bit access... !!! FIX !!!
void JaguarWriteLong(uint32 offset, uint32 data, uint32 who/*=UNKNOWN*/)
{
/*	extern bool doDSPDis;
	if (offset < 0x400 && !doDSPDis)
	{
		WriteLog("JLW: Write to %08X by %s... Starting DSP log!\n\n", offset, whoName[who]);
		doDSPDis = true;
	}//*/

	JaguarWriteWord(offset, data >> 16, who);
	JaguarWriteWord(offset+2, data & 0xFFFF, who);
}

//
// Jaguar console initialization
//
void jaguar_init(void)
{
#ifdef CPU_DEBUG_MEMORY
	memset(readMem, 0x00, 0x400000);
	memset(writeMemMin, 0xFF, 0x400000);
	memset(writeMemMax, 0x00, 0x400000);
#endif
	memory_malloc_secure((void **)&jaguar_mainRam, 0x400000, "Jaguar 68K CPU RAM");
	memory_malloc_secure((void **)&jaguar_bootRom, 0x040000, "Jaguar 68K CPU BIOS ROM");
	memory_malloc_secure((void **)&jaguar_mainRom, 0x600000, "Jaguar 68K CPU ROM");
	memset(jaguar_mainRam, 0x00, 0x400000);
//	memset(jaguar_mainRom, 0xFF, 0x200000);	// & set it to all Fs...
//	memset(jaguar_mainRom, 0x00, 0x200000);	// & set it to all 0s...
//NOTE: This *doesn't* fix FlipOut...
	memset(jaguar_mainRom, 0x01, 0x600000);	// & set it to all 01s...

//	cd_bios_boot("C:\\ftp\\jaguar\\cd\\Brain Dead 13.cdi");
//	cd_bios_boot("C:\\ftp\\jaguar\\cd\\baldies.cdi");
//	cd_bios_boot("C:\\ftp\\jaguar\\cd\\mystdemo.cdi");
//	cd_bios_boot("C:\\ftp\\jaguar\\cd\\battlemorph.cdi");
//	cd_bios_boot("C:\\ftp\\jaguar\\cd\\primalrage.cdi");
//	cd_bios_boot("C:\\ftp\\jaguar\\cd\\Dragons Lair.cdi");

	m68k_set_cpu_type(M68K_CPU_TYPE_68000);
	gpu_init();
	DSPInit();
	tom_init();
	jerry_init();
	cdrom_init();
}

void jaguar_done(void)
{
#ifdef CPU_DEBUG_MEMORY
/*	WriteLog("\n\nM68000 disassembly at $8D0D44 (collision routine!)...\n");
	jaguar_dasm(0x8D0D44, 5000);
	WriteLog("\n");//*/
/*	WriteLog("\n\nM68000 disassembly at $806300 (look @ $806410)...\n");
	jaguar_dasm(0x806300, 5000);
	WriteLog("\n");//*/

/*	WriteLog("\nJaguar: Memory Usage Stats (return addresses)\n\n");

	for(uint32 i=0; i<=raPtr; i++)
	{
		WriteLog("\t%08X\n", returnAddr[i]);
		WriteLog("M68000 disassembly at $%08X...\n", returnAddr[i] - 16);
		jaguar_dasm(returnAddr[i] - 16, 16);
		WriteLog("\n");
	}
	WriteLog("\n");//*/

/*	int start = 0, end = 0;
	bool endTriggered = false, startTriggered = false;
	for(int i=0; i<0x400000; i++)
	{
		if (readMem[i] && writeMemMin[i] != 0xFF && writeMemMax != 0x00)
		{
			if (!startTriggered)
				startTriggered = true, endTriggered = false, start = i;

			WriteLog("\t\tMin/Max @ %06X: %u/%u\n", i, writeMemMin[i], writeMemMax[i]);
		}
		else
		{
			if (!endTriggered)
			{
				end = i - 1, endTriggered = true, startTriggered = false;
				WriteLog("\tMemory range accessed: %06X - %06X\n", start, end);
			}
		}
	}
	WriteLog("\n");//*/
#endif
//#ifdef CPU_DEBUG
//	for(int i=M68K_REG_A0; i<=M68K_REG_A7; i++)
//		WriteLog("\tA%i = 0x%.8x\n", i-M68K_REG_A0, m68k_get_reg(NULL, (m68k_register_t)i));
	int32 topOfStack = m68k_get_reg(NULL, M68K_REG_A7);
	WriteLog("M68K: Top of stack: %08X. Stack trace:\n", JaguarReadLong(topOfStack));
	for(int i=-2; i<9; i++)
		WriteLog("%06X: %08X\n", topOfStack + (i * 4), JaguarReadLong(topOfStack + (i * 4)));

/*	WriteLog("\nM68000 disassembly at $802288...\n");
	jaguar_dasm(0x802288, 3);
	WriteLog("\nM68000 disassembly at $802200...\n");
	jaguar_dasm(0x802200, 500);
	WriteLog("\nM68000 disassembly at $802518...\n");
	jaguar_dasm(0x802518, 100);//*/

/*	WriteLog("\n\nM68000 disassembly at $803F00 (look @ $803F2A)...\n");
	jaguar_dasm(0x803F00, 500);
	WriteLog("\n");//*/

/*	WriteLog("\n\nM68000 disassembly at $802B00 (look @ $802B5E)...\n");
	jaguar_dasm(0x802B00, 500);
	WriteLog("\n");//*/

/*	WriteLog("\n\nM68000 disassembly at $809900 (look @ $8099F8)...\n");
	jaguar_dasm(0x809900, 500);
	WriteLog("\n");//*/
//8099F8
/*	WriteLog("\n\nDump of $8093C8:\n\n");
	for(int i=0x8093C8; i<0x809900; i+=4)
		WriteLog("%06X: %08X\n", i, JaguarReadLong(i));//*/
/*	WriteLog("\n\nM68000 disassembly at $90006C...\n");
	jaguar_dasm(0x90006C, 500);
	WriteLog("\n");//*/

//	WriteLog("Jaguar: CD BIOS version %04X\n", JaguarReadWord(0x3004));
	WriteLog("Jaguar: Interrupt enable = %02X\n", TOMReadByte(0xF000E1) & 0x1F);
	WriteLog("Jaguar: VBL interrupt is %s\n", ((tom_irq_enabled(IRQ_VBLANK)) && (jaguar_interrupt_handler_is_valid(64))) ? "enabled" : "disabled");
	M68K_show_context();
//#endif

	cd_bios_done();
	cdrom_done();
	gpu_done();
	DSPDone();
	tom_done();
	jerry_done();

	memory_free(jaguar_mainRom);
	memory_free(jaguar_bootRom);
	memory_free(jaguar_mainRam);
}

void jaguar_reset(void)
{
	if (vjs.useJaguarBIOS)
		memcpy(jaguar_mainRam, jaguar_bootRom, 8);
	else
	{
// Should also make a run address global as well, for when we reset the jag (PD mainly)
/*		SET32(jaguar_mainRam, 4, 0x00802000);
		// Handle PD stuff...
		// This should definitely go elsewhere (like in the cart load section)!
//NOTE: The bytes 'JAGR' should also be at position $1C...
		if (jaguar_mainRom[0] == 0x60 && jaguar_mainRom[1] == 0x1A)
		{
			uint32 loadAddress = GET32(jaguar_mainRom, 0x22), runAddress = GET32(jaguar_mainRom, 0x2A);
//This is not always right! Especially when converted via bin2jag1!!!
//We should have access to the length of the furshlumiger file that was loaded anyway!
//Now, we do! ;-)
//			uint32 progLength = GET32(jaguar_mainRom, 0x02);
//jaguarRomSize
//jaguarRunAddress
//			WriteLog("Jaguar: Setting up PD ROM... Run address: %08X, length: %08X\n", runAddress, progLength);
//			memcpy(jaguar_mainRam + loadAddress, jaguar_mainRom + 0x2E, progLength);
			WriteLog("Jaguar: Setting up PD ROM... Run address: %08X, length: %08X\n", runAddress, jaguarRomSize - 0x2E);
			memcpy(jaguar_mainRam + loadAddress, jaguar_mainRom + 0x2E, jaguarRomSize - 0x2E);
			SET32(jaguar_mainRam, 4, runAddress);
		}//*/
		SET32(jaguar_mainRam, 4, jaguarRunAddress);
	}

//	WriteLog("jaguar_reset():\n");
//#ifdef SOUND_OUTPUT
//	ws_audio_reset();
//#endif
	cd_bios_reset();
	tom_reset();
	jerry_reset();
	gpu_reset();
	DSPReset();
	cdrom_reset();
    m68k_pulse_reset();                         // Reset the 68000
	WriteLog("\t68K PC=%06X SP=%08X\n", m68k_get_reg(NULL, M68K_REG_PC), m68k_get_reg(NULL, M68K_REG_A7));
}

//
// Main Jaguar execution loop (1 frame)
//
void JaguarExecute(int16 * backbuffer, bool render)
{
	uint16 vp = TOMReadWord(0xF0003E) + 1;//Hmm. This is a WO register. Will work? Looks like. But wrong behavior!
	uint16 vi = TOMReadWord(0xF0004E);//Another WO register...
//	uint16 vdb = TOMReadWord(0xF00046);
//Note: This is the *definite* end of the display, though VDE *might* be less than this...
//	uint16 vbb = TOMReadWord(0xF00040);
//It seems that they mean it when they say that VDE is the end of object processing.
//However, we need to be able to tell the OP (or TOM) that we've reached the end of the
//buffer and not to write any more pixels... !!! FIX !!!
//	uint16 vde = TOMReadWord(0xF00048);

	uint16 refreshRate = (vjs.hardwareTypeNTSC ? 60 : 50);
	// Should these be hardwired or read from VP? Yes, from VP!
	uint32 M68KCyclesPerScanline
		= (vjs.hardwareTypeNTSC ? M68K_CLOCK_RATE_NTSC : M68K_CLOCK_RATE_PAL) / (vp * refreshRate);
	uint32 RISCCyclesPerScanline
		= (vjs.hardwareTypeNTSC ? RISC_CLOCK_RATE_NTSC : RISC_CLOCK_RATE_PAL) / (vp * refreshRate);

	TOMResetBackbuffer(backbuffer);
/*extern int effect_start;
if (effect_start)
{
	WriteLog("JagExe: VP=%u, VI=%u, VDB=%u, VBB=%u CPU CPS=%u, GPU CPS=%u\n", vp, vi, vdb, vbb, M68KCyclesPerScanline, RISCCyclesPerScanline);
}//*/

//extern int start_logging;
	for(uint16 i=0; i<vp; i++)
	{
		// Increment the horizontal count (why? RNG?)
		TOMWriteWord(0xF00004, (TOMReadWord(0xF00004) + 1) & 0x7FF);

		TOMWriteWord(0xF00006, i);					// Write the VC

//		if (i == vi)								// Time for Vertical Interrupt?
//Not sure if this is correct...
//Seems to be, kinda. According to the JTRM, this should only fire on odd lines in non-interlace mode...
//Which means that it normally wouldn't go when it's zero.
		if (i == vi && i > 0)						// Time for Vertical Interrupt?
		{
			if (tom_irq_enabled(IRQ_VBLANK))// && jaguar_interrupt_handler_is_valid(64))
			{
				// We don't have to worry about autovectors & whatnot because the Jaguar
				// tells you through its HW registers who sent the interrupt...
				tom_set_pending_video_int();
				m68k_set_irq(7);
			}
		}

//		uint32 invalid_instruction_address = s68000exec(M68KCyclesPerScanline);
//		if (invalid_instruction_address != 0x80000000)
//			cd_bios_process(invalid_instruction_address);
//if (start_logging)
//	WriteLog("About to execute M68K (%u)...\n", i);
		m68k_execute(M68KCyclesPerScanline);
		// No CD handling... !!! FIX !!!
//if (start_logging)
//	WriteLog("About to execute CD BIOS (%u)...\n", i);
		cd_bios_exec(i);	// NOTE: Ignores parameter...
//if (start_logging)
//	WriteLog("About to execute TOM's PIT (%u)...\n", i);
		TOMExecPIT(RISCCyclesPerScanline);
//if (start_logging)
//	WriteLog("About to execute JERRY's PIT (%u)...\n", i);
		jerry_pit_exec(RISCCyclesPerScanline);
//if (start_logging)
//	WriteLog("About to execute JERRY's SSI (%u)...\n", i);
		jerry_i2s_exec(RISCCyclesPerScanline);
//if (start_logging)
//	WriteLog("About to execute GPU (%u)...\n", i);
		gpu_exec(RISCCyclesPerScanline);

		if (vjs.DSPEnabled)
			if (vjs.usePipelinedDSP)
				DSPExecP2(RISCCyclesPerScanline);	// Pipelined DSP execution (3 stage)...
			else
				DSPExec(RISCCyclesPerScanline);		// Ordinary non-pipelined DSP
//			DSPExecComp(RISCCyclesPerScanline);		// Comparison core

//if (start_logging)
//	WriteLog("About to execute OP (%u)...\n", i);
		TOMExecScanline(i, render);
	}
}

// Temp debugging stuff

void DumpMainMemory(void)
{
	FILE * fp = fopen("./memdump.bin", "wb");

	if (fp == NULL)
		return;

	fwrite(jaguar_mainRam, 1, 0x400000, fp);
	fclose(fp);
}

uint8 * GetRamPtr(void)
{
	return jaguar_mainRam;
}
