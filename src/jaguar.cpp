//
// JAGUAR.CPP
//
// by Cal2
// GCC/SDL port by Niels Wagenaar (Linux/WIN32) and Caz (BeOS)
// Cleanups and endian wrongness amelioration by James L. Hammons
// Note: Endian wrongness probably stems from the MAME origins of this emu and
//       the braindead way in which MAME handles memory. :-)
// 

#include "jaguar.h"
//#include "m68kdasmAG.h"

#define CPU_DEBUG
//Do this in makefile??? Yes! Could, but it's easier to define here...
//#define LOG_UNMAPPED_MEMORY_ACCESSES

// Private function prototypes

unsigned jaguar_unknown_readbyte(unsigned address, uint32 who = UNKNOWN);
unsigned jaguar_unknown_readword(unsigned address, uint32 who = UNKNOWN);
void jaguar_unknown_writebyte(unsigned address, unsigned data, uint32 who = UNKNOWN);
void jaguar_unknown_writeword(unsigned address, unsigned data, uint32 who = UNKNOWN);
void M68K_show_context(void);

// External variables

extern bool hardwareTypeNTSC;				// Set to false for PAL

// Memory debugging identifiers

char * whoName[9] =
	{ "Unknown", "Jaguar", "DSP", "GPU", "TOM", "JERRY", "M68K", "Blitter", "OP" };

// These values are overridden by command line switches...

bool dsp_enabled = false;
bool jaguar_use_bios = true;				// Default is now to USE the BIOS
uint32 jaguar_active_memory_dumps = 0;

uint32 jaguar_mainRom_crc32;

/*static*/ uint8 * jaguar_mainRam = NULL;
/*static*/ uint8 * jaguar_bootRom = NULL;
/*static*/ uint8 * jaguar_mainRom = NULL;


//
// Callback function to detect illegal instructions
//
void M68KInstructionHook(void)
{
	uint32 m68kPC = m68k_get_reg(NULL, M68K_REG_PC);

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

	if (!m68k_is_valid_instruction(JaguarReadWord(m68kPC), M68K_CPU_TYPE_68000))
	{
		WriteLog("\nM68K encountered an illegal instruction at %08X!!!\n\nAborting!\n", m68kPC);
		uint32 topOfStack = m68k_get_reg(NULL, M68K_REG_A7);
		WriteLog("M68K: Top of stack: %08X. Stack trace:\n", JaguarReadLong(topOfStack));
		for(int i=0; i<10; i++)
			WriteLog("%06X: %08X\n", topOfStack - (i * 4), JaguarReadLong(topOfStack - (i * 4)));
		WriteLog("Jaguar: VBL interrupt is %s\n", ((tom_irq_enabled(IRQ_VBLANK)) && (jaguar_interrupt_handler_is_valid(64))) ? "enabled" : "disabled");
		M68K_show_context();
		log_done();
		exit(0);
	}
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
//if ((address >= 0x1FF020 && address <= 0x1FF03F) || (address >= 0x1FF820 && address <= 0x1FF83F))
//	WriteLog("M68K: Writing %04X at %08X\n", value, address);
//WriteLog("[WM16 PC=%08X] Addr: %08X, val: %04X\n", m68k_get_reg(NULL, M68K_REG_PC), address, value);
//if (address >= 0xF02200 && address <= 0xF0229F)
//	WriteLog("M68K: Writing to blitter --> %04X at %08X\n", value, address);
//if (address >= 0x0E75D0 && address <= 0x0E75E7)
//	WriteLog("M68K: Writing %04X at %08X, M68K PC=%08X\n", value, address, m68k_get_reg(NULL, M68K_REG_PC));
/*extern uint32 totalFrames;
/*if (address == 0xF02114)
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
		jaguar_unknown_writeword(address, value, M68K);
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

void jaguar_unknown_writebyte(unsigned address, unsigned data, uint32 who/*=UNKNOWN*/)
{
#ifdef LOG_UNMAPPED_MEMORY_ACCESSES
	WriteLog("Jaguar: Unknown byte %02X written at %08X by %s (M68K PC=%06X)\n", data, address, whoName[who], m68k_get_reg(NULL, M68K_REG_PC));
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
//if (offset >= 0xF0B000 && offset <= 0xF0BFFF)
//WriteLog("[JWW16] --> Possible GPU RAM mirror access! [%08X]", offset);
//if ((offset >= 0x1FF020 && offset <= 0x1FF03F) || (offset >= 0x1FF820 && offset <= 0x1FF83F))
//	WriteLog("JagWW: Writing %04X at %08X\n", data, offset);
	offset &= 0xFFFFFF;
	
	if (offset <= 0x3FFFFE)
	{
		jaguar_mainRam[(offset+0) & 0x3FFFFF] = (data>>8) & 0xFF;
		jaguar_mainRam[(offset+1) & 0x3FFFFF] = data & 0xFF;
		return;
	}
	else if ((offset >= 0xDFFF00) && (offset <= 0xDFFFFE))
	{
		CDROMWriteWord(offset, data, who);
		return;
	}
	else if ((offset >= 0xF00000) && (offset <= 0xF0FFFE))
	{
		TOMWriteWord(offset, data, who);
		return;
	}
	else if ((offset >= 0xF10000) && (offset <= 0xF1FFFE))
	{
		JERRYWriteWord(offset, data, who);
		return;
	}
    
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
	JaguarWriteWord(offset, data >> 16, who);
	JaguarWriteWord(offset+2, data & 0xFFFF, who);
}

//
// Jaguar console initialization
//
void jaguar_init(void)
{
	memory_malloc_secure((void **)&jaguar_mainRam, 0x400000, "Jaguar 68K CPU RAM");
	memory_malloc_secure((void **)&jaguar_bootRom, 0x040000, "Jaguar 68K CPU BIOS ROM");
	memory_malloc_secure((void **)&jaguar_mainRom, 0x600000, "Jaguar 68K CPU ROM");
	memset(jaguar_mainRam, 0x00, 0x400000);
//	memset(jaguar_mainRom, 0xFF, 0x200000);	// & set it to all Fs...
	memset(jaguar_mainRom, 0x00, 0x200000);	// & set it to all 0s...

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
	if (jaguar_use_bios)
		memcpy(jaguar_mainRam, jaguar_bootRom, 8);
	else
	{
		SET32(jaguar_mainRam, 4, 0x00802000);
		// Handle PD stuff...
		// This should definitely go elsewhere (like in the cart load section)!
		if (jaguar_mainRom[0] == 0x60 && jaguar_mainRom[1] == 0x1A)
		{
			uint32 runAddress = GET32(jaguar_mainRom, 0x2A);
			uint32 progLength = GET32(jaguar_mainRom, 0x02);
			WriteLog("Setting up PD ROM... Run address: %08X, length: %08X\n", runAddress, progLength);
			memcpy(jaguar_mainRam + runAddress, jaguar_mainRom + 0x2E, progLength);
			SET32(jaguar_mainRam, 4, runAddress);
		}
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

/*unused
void jaguar_reset_handler(void)
{
}*/

//
// Main Jaguar execution loop (1 frame)
//
void JaguarExecute(int16 * backbuffer, bool render)
{
	uint16 vp = TOMReadWord(0xF0003E) + 1;//Hmm. This is a WO register. Will work? Looks like. But wrong behavior!
	uint16 vi = TOMReadWord(0xF0004E);//Another WO register...
	uint16 vdb = TOMReadWord(0xF00046);
//Note: This is the *definite* end of the display, though VDE *might* be less than this...
//	uint16 vbb = TOMReadWord(0xF00040);
//It seems that they mean it when they say that VDE is the end of object processing.
//However, we need to be able to tell the OP (or TOM) that we've reached the end of the
//buffer and not to write any more pixels... !!! FIX !!!
	uint16 vde = TOMReadWord(0xF00048);

	uint16 refreshRate = (hardwareTypeNTSC ? 60 : 50);
	// Should these be hardwired or read from VP? Yes, from VP!
	uint32 M68KCyclesPerScanline
		= (hardwareTypeNTSC ? M68K_CLOCK_RATE_NTSC : M68K_CLOCK_RATE_PAL) / (vp * refreshRate);
	uint32 RISCCyclesPerScanline
		= (hardwareTypeNTSC ? RISC_CLOCK_RATE_NTSC : RISC_CLOCK_RATE_PAL) / (vp * refreshRate);

/*extern int effect_start;
if (effect_start)
{
	WriteLog("JagExe: VP=%u, VI=%u, VDB=%u, VBB=%u CPU CPS=%u, GPU CPS=%u\n", vp, vi, vdb, vbb, M68KCyclesPerScanline, RISCCyclesPerScanline);
}//*/

	for(uint16 i=0; i<vp; i++)
	{
		// Increment the horizontal count (why? RNG?)
		TOMWriteWord(0xF00004, (TOMReadWord(0xF00004) + 1) & 0x7FF);

		TOMWriteWord(0xF00006, i);					// Write the VC

		if (i == vi)								// Time for Vertical Interrupt?
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
		// These are divided by 2 because we're executing *half* lines...!
		// Err, this is *already* accounted for in jaguar_init...!
		m68k_execute(M68KCyclesPerScanline);
		// No CD handling... !!! FIX !!!
		cd_bios_exec(i);	// NOTE: Ignores parameter...
		tom_pit_exec(RISCCyclesPerScanline);
		jerry_pit_exec(RISCCyclesPerScanline);
		jerry_i2s_exec(RISCCyclesPerScanline);
		gpu_exec(RISCCyclesPerScanline);
		if (dsp_enabled)
			DSPExec(RISCCyclesPerScanline);

//Interlacing is still not handled correctly here... !!! FIX !!!
		if (i >= vdb && i < vde)//vbb)
		{
			if (!(i & 0x01))						// Execute OP only on even lines (non-interlaced only!)
			{
				tom_exec_scanline(backbuffer, i/2, render);	// i/2 is a kludge...
				backbuffer += TOMGetSDLScreenPitch() / 2;	// Convert bytes to words...
			}
		}
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
