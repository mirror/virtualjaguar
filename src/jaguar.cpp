//
// JAGUAR.CPP
//
// by cal2
// GCC/SDL port by Niels Wagenaar (Linux/WIN32) and Caz (BeOS)
// Cleanups and endian wrongness amelioration by James L. Hammons
// Note: Endian wrongness probably stems from the MAME origins of this emu and
//       the braindead way in which MAME handles memory. :-)
// 

#include "jaguar.h"
//#include "m68kdasmAG.h"
//#include "crc32.h"

//#define LOG_UNMAPPED_MEMORY_ACCESSES
//#define SOUND_OUTPUT
#define CPU_DEBUG
#define JAGUAR_WIP_RELEASE
#define JAGUAR_REAL_SPEED
//Do this in makefile??? Yes! Could, but it's easier to define here...
//#define LOG_UNMAPPED_MEMORY_ACCESSES

//
// Private function prototypes
//

unsigned jaguar_unknown_readbyte(unsigned address);
unsigned jaguar_unknown_readword(unsigned address);
void jaguar_unknown_writebyte(unsigned address, unsigned data);
void jaguar_unknown_writeword(unsigned address, unsigned data);
void M68K_show_context(void);

// These values are overridden by command line switches...

bool dsp_enabled = false;
bool jaguar_use_bios = true;				// Default is now to USE the BIOS
uint32 jaguar_active_memory_dumps = 0;

uint32 jaguar_mainRom_crc32;

static uint32 m68k_cycles_per_scanline;
static uint32 gpu_cycles_per_scanline;
static uint32 dsp_cycles_per_scanline;
static uint32 jaguar_screen_scanlines;

/*static*/ uint8 * jaguar_mainRam = NULL;
/*static*/ uint8 * jaguar_bootRom = NULL;
/*static*/ uint8 * jaguar_mainRom = NULL;


//
// Callback function to detect illegal instructions
//

void M68KInstructionHook(void)
{
	uint32 m68kPC = m68k_get_reg(NULL, M68K_REG_PC);
	
	if (!m68k_is_valid_instruction(jaguar_word_read(m68kPC), M68K_CPU_TYPE_68000))
	{
		WriteLog("\nEncountered illegal instruction at %08X!!!\n\nAborting!\n", m68kPC);
		uint32 topOfStack = m68k_get_reg(NULL, M68K_REG_A7);
		WriteLog("M68K: Top of stack: %08X. Stack trace:\n", jaguar_long_read(topOfStack));
		for(int i=0; i<10; i++)
			WriteLog("%06X: %08X\n", topOfStack - (i * 4), jaguar_long_read(topOfStack - (i * 4)));
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
//WriteLog( "[RM8] Addr: %08X\n", address);
	unsigned int retVal = 0;

	if ((address >= 0x000000) && (address <= 0x3FFFFF))
		retVal = jaguar_mainRam[address];
	else if ((address >= 0x800000) && (address <= 0xDFFFFF))
		retVal = jaguar_mainRom[address - 0x800000];
	else if ((address >= 0xE00000) && (address <= 0xE3FFFF))
		retVal = jaguar_bootRom[address - 0xE00000];
	else if ((address >= 0xDFFF00) && (address <= 0xDFFFFF))
		retVal = cdrom_byte_read(address);
	else if ((address >= 0xF00000) && (address <= 0xF0FFFF))
		retVal = tom_byte_read(address);
	else if ((address >= 0xF10000) && (address <= 0xF1FFFF))
		retVal = jerry_byte_read(address);
	else
		retVal = jaguar_unknown_readbyte(address);

    return retVal;
}

void gpu_dump_disassembly(void);
void gpu_dump_registers(void);

unsigned int m68k_read_memory_16(unsigned int address)
{
//WriteLog( "[RM16] Addr: %08X\n", address);
/*if (m68k_get_reg(NULL, M68K_REG_PC) == 0x00005FBA)
//	for(int i=0; i<10000; i++)
	WriteLog("[M68K] In routine #6!\n");//*/
//if (m68k_get_reg(NULL, M68K_REG_PC) == 0x00006696) // GPU Program #4
//if (m68k_get_reg(NULL, M68K_REG_PC) == 0x00005B3C)	// GPU Program #2
/*if (m68k_get_reg(NULL, M68K_REG_PC) == 0x00005BA8)	// GPU Program #3
{
	WriteLog("[M68K] About to run GPU! (Addr:%08X, data:%04X)\n", address, tom_word_read(address));
	gpu_dump_registers();
	gpu_dump_disassembly();
//	for(int i=0; i<10000; i++)
//		WriteLog( "[M68K] About to run GPU!\n");
}//*/
//WriteLog( "[WM8  PC=%08X] Addr: %08X, val: %02X\n", m68k_get_reg(NULL, M68K_REG_PC), address, value);
/*if (m68k_get_reg(NULL, M68K_REG_PC) >= 0x00006696 && m68k_get_reg(NULL, M68K_REG_PC) <= 0x000066A8)
{
	if (address == 0x000066A0)
	{
		gpu_dump_registers();
		gpu_dump_disassembly();
	}
	for(int i=0; i<10000; i++)
		WriteLog( "[M68K] About to run GPU! (Addr:%08X, data:%04X)\n", address, tom_word_read(address));
}//*/
    unsigned int retVal = 0;

	if ((address >= 0x000000) && (address <= 0x3FFFFE))
		retVal = (jaguar_mainRam[address] << 8) | jaguar_mainRam[address+1];
	else if ((address >= 0x800000) && (address <= 0xDFFFFE))
		retVal = (jaguar_mainRom[address - 0x800000] << 8) | jaguar_mainRom[address - 0x800000 + 1];
	else if ((address >= 0xE00000) && (address <= 0xE3FFFE))
		retVal = (jaguar_bootRom[address - 0xE00000] << 8) | jaguar_bootRom[address - 0xE00000 + 1];
	else if ((address >= 0xDFFF00) && (address <= 0xDFFFFE))
		retVal = cdrom_word_read(address);
	else if ((address >= 0xF00000) && (address <= 0xF0FFFE))
		retVal = tom_word_read(address);
	else if ((address >= 0xF10000) && (address <= 0xF1FFFE))
		retVal = jerry_word_read(address);
	else
//{
//WriteLog( "[RM16] Unknown address: %08X\n", address);
		retVal = jaguar_unknown_readword(address);
//}

    return retVal;
}

unsigned int m68k_read_memory_32(unsigned int address)
{
//WriteLog( "--> [RM32]\n");
    return (m68k_read_memory_16(address) << 16) | m68k_read_memory_16(address + 2);
}

void m68k_write_memory_8(unsigned int address, unsigned int value)
{
//if ((address >= 0x1FF020 && address <= 0x1FF03F) || (address >= 0x1FF820 && address <= 0x1FF83F))
//	WriteLog("M68K: Writing %02X at %08X\n", value, address);
//WriteLog( "[WM8  PC=%08X] Addr: %08X, val: %02X\n", m68k_get_reg(NULL, M68K_REG_PC), address, value);
	if ((address >= 0x000000) && (address <= 0x3FFFFF))
		jaguar_mainRam[address] = value;
	else if ((address >= 0xDFFF00) && (address <= 0xDFFFFF))
		cdrom_byte_write(address, value);
	else if ((address >= 0xF00000) && (address <= 0xF0FFFF))
		tom_byte_write(address, value);
	else if ((address >= 0xF10000) && (address <= 0xF1FFFF))
		jerry_byte_write(address, value);
	else
		jaguar_unknown_writebyte(address, value);
}

void m68k_write_memory_16(unsigned int address, unsigned int value)
{
extern int dsp_pc;//, dsp_control;
if (address == 0xF1A116 && (value & 0x01))
{
	WriteLog("  M68K(16): DSP is GO! (DSP_PC: %08X)\n\n", dsp_pc);

/*	static char buffer[512];
	uint32 j = 0xF1B000;
	while (j <= 0xF1BFFF)
	{
		uint32 oldj = j;
		j += dasmjag(JAGUAR_DSP, buffer, j);
		WriteLog( "\t%08X: %s\n", oldj, buffer);
	}
	WriteLog( "\n");//*/
}
//else
//	WriteLog("M68K(16): DSP halted... (Old value: %08X)\n", dsp_control);

//if ((address >= 0x1FF020 && address <= 0x1FF03F) || (address >= 0x1FF820 && address <= 0x1FF83F))
//	WriteLog("M68K: Writing %04X at %08X\n", value, address);
//WriteLog( "[WM16 PC=%08X] Addr: %08X, val: %04X\n", m68k_get_reg(NULL, M68K_REG_PC), address, value);
//if (address >= 0xF02200 && address <= 0xF0229F)
//	WriteLog("M68K: Writing to blitter --> %04X at %08X\n", value, address);
	if ((address >= 0x000000) && (address <= 0x3FFFFE))
	{
		jaguar_mainRam[address] = value >> 8;
		jaguar_mainRam[address + 1] = value & 0xFF;
	}
	else if ((address >= 0xDFFF00) && (address <= 0xDFFFFE))
		cdrom_word_write(address, value);
	else if ((address >= 0xF00000) && (address <= 0xF0FFFE))
		tom_word_write(address, value);
	else if ((address >= 0xF10000) && (address <= 0xF1FFFE))
		jerry_word_write(address, value);
	else
		jaguar_unknown_writeword(address, value);
}

void m68k_write_memory_32(unsigned int address, unsigned int value)
{
extern int dsp_pc;//, dsp_control;
if (address == 0xF1A114 && (value & 0x01))
	WriteLog("M68K(32): DSP is GO! (DSP_PC: %08X)\n", dsp_pc);
//else
//	WriteLog("M68K(32): DSP halted... (Old value: %08X)\n", dsp_control);

//WriteLog( "--> [WM32]\n");
	m68k_write_memory_16(address, value >> 16);
	m68k_write_memory_16(address + 2, value & 0xFFFF);
}



uint32 jaguar_get_handler(uint32 i)
{
//	return (jaguar_word_read(i<<2) << 16) | jaguar_word_read((i<<2) + 2);
//	return (jaguar_word_read(i*4) << 16) | jaguar_word_read((i*4) + 2);
	return jaguar_long_read(i * 4);
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
	WriteLog( "\t68K PC=%06X\n", m68k_get_reg(NULL, M68K_REG_PC));
	for(int i=M68K_REG_D0; i<=M68K_REG_D7; i++)
		WriteLog( "\tD%i = %08X\n", i-M68K_REG_D0, m68k_get_reg(NULL, (m68k_register_t)i));
	WriteLog( "\n");
	for(int i=M68K_REG_A0; i<=M68K_REG_A7; i++)
		WriteLog( "\tA%i = %08X\n", i-M68K_REG_A0, m68k_get_reg(NULL, (m68k_register_t)i));

	WriteLog( "68K disasm\n");
//	jaguar_dasm(s68000readPC()-0x1000,0x20000);
	jaguar_dasm(m68k_get_reg(NULL, M68K_REG_PC) - 0x80, 0x200);
//	jaguar_dasm(0x5000, 0x14414);
	WriteLog( "..................\n");


	if (tom_irq_enabled(IRQ_VBLANK))
	{
		WriteLog( "vblank int: enabled\n");
		jaguar_dasm(jaguar_get_handler(64), 0x200);
	}
	else
		WriteLog( "vblank int: disabled\n");
	WriteLog( "..................\n");

	for(int i=0; i<256; i++)
		WriteLog( "handler %03i at $%08X\n", i, (unsigned int)jaguar_get_handler(i));
}

//
// Unknown read/write byte/word routines
//

void jaguar_unknown_writebyte(unsigned address, unsigned data)
{
#ifdef LOG_UNMAPPED_MEMORY_ACCESSES
	WriteLog( "jaguar: unknown byte %02X write at %08X (PC=%06X)\n", data, address, m68k_get_reg(NULL, M68K_REG_PC));
#endif
}

void jaguar_unknown_writeword(unsigned address, unsigned data)
{
#ifdef LOG_UNMAPPED_MEMORY_ACCESSES
	WriteLog( "jaguar: unknown word %04X write at %08X (PC=%06X)\n", data, address, m68k_get_reg(NULL, M68K_REG_PC));
#endif
}

unsigned jaguar_unknown_readbyte(unsigned address)
{
#ifdef LOG_UNMAPPED_MEMORY_ACCESSES
	WriteLog( "jaguar: unknown byte read at %08X (PC=%06X)\n", address, m68k_get_reg(NULL, M68K_REG_PC));
#endif
    return 0xFF;
}

unsigned jaguar_unknown_readword(unsigned address)
{
#ifdef LOG_UNMAPPED_MEMORY_ACCESSES
	WriteLog( "Jaguar: Unknown word read at %08X (PC=%06X)\n", address, m68k_get_reg(NULL, M68K_REG_PC));
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
		oldpc = pc;
//		for(int j=0; j<64; j++)
//			mem[j^0x01] = jaguar_byte_read(pc + j);

//		pc += Dasm68000((char *)mem, buffer, 0);
		pc += m68k_disassemble(buffer, pc, M68K_CPU_TYPE_68000);
		WriteLog("%08X: %s\n", oldpc, buffer);
	}
#endif
}

unsigned jaguar_byte_read(unsigned int offset)
{
	uint8 data = 0x00;

	offset &= 0xFFFFFF;
	if (offset < 0x400000)
//		data = (jaguar_mainRam[(offset^0x01) & 0x3FFFFF]);
		data = jaguar_mainRam[offset & 0x3FFFFF];
	else if ((offset >= 0x800000) && (offset < 0xC00000))
//		data = (jaguar_mainRom[(offset^0x01)-0x800000]);
		data = jaguar_mainRom[offset - 0x800000];
//	else if ((offset >= 0xDFFF00) && (offset < 0xDFFF00))
	else if ((offset >= 0xDFFF00) && (offset <= 0xDFFFFF))
		data = cdrom_byte_read(offset);
	else if ((offset >= 0xE00000) && (offset < 0xE40000))
//		data = (jaguar_bootRom[(offset^0x01) & 0x3FFFF]);
		data = jaguar_bootRom[offset & 0x3FFFF];
	else if ((offset >= 0xF00000) && (offset < 0xF10000))
		data = tom_byte_read(offset);
	else if ((offset >= 0xF10000) && (offset < 0xF20000))
		data = jerry_byte_read(offset);
	else
		data = jaguar_unknown_readbyte(offset);

	return data;
}

unsigned jaguar_word_read(unsigned int offset)
{
//TEMP--Mirror of F03000?
/*if (offset >= 0xF0B000 && offset <= 0xF0BFFF)
WriteLog( "[JWR16] --> Possible GPU RAM mirror access! [%08X]\n", offset);//*/

	offset &= 0xFFFFFF;
	if (offset <= 0x3FFFFE)
	{
//		return (jaguar_mainRam[(offset+1) & 0x3FFFFF] << 8) | jaguar_mainRam[(offset+0) & 0x3FFFFF];
		return (jaguar_mainRam[(offset+0) & 0x3FFFFF] << 8) | jaguar_mainRam[(offset+1) & 0x3FFFFF];
	}
	else if ((offset >= 0x800000) && (offset <= 0xBFFFFE))
	{
		offset -= 0x800000;
//		return (jaguar_mainRom[offset+1] << 8) | jaguar_mainRom[offset+0];
		return (jaguar_mainRom[offset+0] << 8) | jaguar_mainRom[offset+1];
	}
//	else if ((offset >= 0xDFFF00) && (offset < 0xDFFF00))
	else if ((offset >= 0xDFFF00) && (offset <= 0xDFFFFE))
		return cdrom_word_read(offset);
	else if ((offset >= 0xE00000) && (offset <= 0xE3FFFE))
//		return *((uint16 *)&jaguar_bootRom[offset & 0x3FFFF]);
		return (jaguar_bootRom[(offset+0) & 0x3FFFF] << 8) | jaguar_bootRom[(offset+1) & 0x3FFFF];
	else if ((offset >= 0xF00000) && (offset <= 0xF0FFFE))
		return tom_word_read(offset);
	else if ((offset >= 0xF10000) && (offset <= 0xF1FFFE))
//{
//WriteLog("Reading from JERRY offset %08X...\n", offset);
		return jerry_word_read(offset);
//}

	return jaguar_unknown_readword(offset);
}

void jaguar_byte_write(unsigned offset, unsigned data)
{
	offset &= 0xFFFFFF;
	if (offset < 0x400000)
	{
//		jaguar_mainRam[(offset^0x01) & 0x3FFFFF] = data;
		jaguar_mainRam[offset & 0x3FFFFF] = data;
		return;
	}
//	else if ((offset >= 0xDFFF00) && (offset < 0xDFFF00))
	else if ((offset >= 0xDFFF00) && (offset <= 0xDFFFFF))
	{
		cdrom_byte_write(offset, data);
		return;
	}
	else if ((offset >= 0xF00000) && (offset <= 0xF0FFFF))
	{
		tom_byte_write(offset, data);
		return;
	}
	else if ((offset >= 0xF10000) && (offset <= 0xF1FFFF))
	{
		jerry_byte_write(offset, data);
		return;
	}
    
	jaguar_unknown_writebyte(offset, data);
}

void jaguar_word_write(unsigned offset, unsigned data)
{
extern int dsp_pc;//, dsp_control;
if (offset == 0xF1A116 && (data & 0x01))
	WriteLog("  JagWW: DSP is GO! (DSP_PC: %08X)\n", dsp_pc);
//else
//	WriteLog("JagWW: DSP halted... (Old value: %08X)\n", dsp_control);

//extern int blit_start_log;
//if (blit_start_log)
/*{
	if (offset == 0x0674DE)
		WriteLog( "[JWW16] Bad write starting @ 0674DE! [%04X]\n", data);
}//*/
//TEMP--Mirror of F03000?
//if (offset >= 0xF0B000 && offset <= 0xF0BFFF)
//WriteLog( "[JWW16] --> Possible GPU RAM mirror access! [%08X]", offset);
//if ((offset >= 0x1FF020 && offset <= 0x1FF03F) || (offset >= 0x1FF820 && offset <= 0x1FF83F))
//	WriteLog("JagWW: Writing %04X at %08X\n", data, offset);
	offset &= 0xFFFFFF;
	
	if (offset <= 0x3FFFFE)
	{
//		jaguar_mainRam[(offset+0) & 0x3FFFFF] = data & 0xFF;
//		jaguar_mainRam[(offset+1) & 0x3FFFFF] = (data>>8) & 0xFF;
		jaguar_mainRam[(offset+0) & 0x3FFFFF] = (data>>8) & 0xFF;
		jaguar_mainRam[(offset+1) & 0x3FFFFF] = data & 0xFF;
		return;
	}
	else if ((offset >= 0xDFFF00) && (offset <= 0xDFFFFE))
	{
		cdrom_word_write(offset, data);
		return;
	}
	else if ((offset >= 0xF00000) && (offset <= 0xF0FFFE))
	{
		tom_word_write(offset, data);
		return;
	}
	else if ((offset >= 0xF10000) && (offset <= 0xF1FFFE))
	{
		jerry_word_write(offset, data);
		return;
	}
    
	jaguar_unknown_writeword(offset, data);
}

unsigned jaguar_long_read(unsigned int offset)
{
/*	uint32 data = jaguar_word_read(offset);
	data = (data<<16) | jaguar_word_read(offset+2);
	return data;*/
	return (jaguar_word_read(offset) << 16) | jaguar_word_read(offset+2);
}

void jaguar_long_write(unsigned offset, unsigned data)
{
extern int dsp_pc;//, dsp_control;
if (offset == 0xF1A114 && (data & 0x01))
	WriteLog("JagLW: DSP is GO! (DSP_PC: %08X)\n", dsp_pc);
//else
//	WriteLog("JagLW: DSP halted... (Old value: %08X)\n", dsp_control);

//extern int effect_start;
// $10, $0C, $0A, $09 too much, $08 too little...
//if (effect_start && offset == 0xF03000) data = (data & 0xFFFF0000) | (((data & 0xFFFF) + 0x0008) & 0xFFFF);
//Doesn't work--offsets horizontally if (effect_start && offset == 0xF03004) data -= (0x300 * 8); // one line is $300
//if (effect_start && offset == 0xF03000) data = 0x00000000;	// Let's try making the top/bottom *always* 0!
//Interesting: it seems to pin half of the screen down (but too low)...
//Definitely the fine scroll offsets (for left side of screen)...

//if ((offset >= 0x1FF020 && offset <= 0x1FF03F) || (offset >= 0x1FF820 && offset <= 0x1FF83F))
//	WriteLog("JagLW: Writing %08X at %08X\n", data, offset);
	jaguar_word_write(offset, data >> 16);
	jaguar_word_write(offset+2, data & 0xFFFF);
}

//
// Jaguar initialization
//

//void jaguar_init(const char * filename)
void jaguar_init(void)
{
//	uint32 romsize;

	jaguar_screen_scanlines = 525;			// PAL screen size
	m68k_cycles_per_scanline = 13300000 / (jaguar_screen_scanlines * 60);
	gpu_cycles_per_scanline = (26591000 / 4) / (jaguar_screen_scanlines * 60);
	dsp_cycles_per_scanline = (26591000 / 4) / (jaguar_screen_scanlines * 60);

	memory_malloc_secure((void **)&jaguar_mainRam, 0x400000, "Jaguar 68K CPU RAM");
	memory_malloc_secure((void **)&jaguar_bootRom, 0x040000, "Jaguar 68K CPU BIOS ROM");
	memory_malloc_secure((void **)&jaguar_mainRom, 0x600000, "Jaguar 68K CPU ROM");
	memset(jaguar_mainRam, 0x00, 0x400000);
//	memset(jaguar_mainRom, 0xFF, 0x200000);	// & set it to all Fs...
	memset(jaguar_mainRom, 0x00, 0x200000);	// & set it to all 0s...

//	jaguar_rom_load_to(jaguar_bootRom, jaguar_bootRom_path, &romsize);
//	memcpy(jaguar_mainRam, jaguar_bootRom, 8);
//	SET32(jaguar_mainRam, 0, 0x00200000);

#ifdef JAGUAR_WIP_RELEASE
//	strcpy(romLoadDialog_filePath, filename);
// 	jaguar_load_cart(romLoadDialog_filePath, jaguar_mainRom, 0x0000, 0x00802000, 0);

	if ((jaguar_mainRom_crc32 == 0x3966698f) || (jaguar_mainRom_crc32 == 0x5e705756)
		|| (jaguar_mainRom_crc32 == 0x2630cbc4) || (jaguar_mainRom_crc32 == 0xd46437e8)
		|| (jaguar_mainRom_crc32 == 0x2630cbc4))
		dsp_enabled = true;

	if ((jaguar_mainRom_crc32 == 0x6e90989f) || (jaguar_mainRom_crc32 == 0xfc8f0dcd)
		|| (jaguar_mainRom_crc32 == 0x2a512a83) || (jaguar_mainRom_crc32 == 0x41307601)
		|| (jaguar_mainRom_crc32 == 0x3c7bfda8) || (jaguar_mainRom_crc32 == 0x5e705756))
		gpu_cycles_per_scanline = (26591000 / 1) / (jaguar_screen_scanlines * 60);

	if (jaguar_mainRom_crc32 == 0x7ae20823)
	{
		dsp_enabled = true;
		gpu_cycles_per_scanline = (26591000 / 1) / (jaguar_screen_scanlines * 60);
		dsp_cycles_per_scanline = (26591000 / 1) / (jaguar_screen_scanlines * 60);
	}
	if (jaguar_mainRom_crc32 == 0xe21d0e2f)
	{
		dsp_enabled = true;
		gpu_cycles_per_scanline = (26591000 / 1) / (jaguar_screen_scanlines * 60);
		dsp_cycles_per_scanline = (26591000 / 1) / (jaguar_screen_scanlines * 60);
	}
	if (jaguar_mainRom_crc32 == 0x66f8914c)
	{
		gpu_cycles_per_scanline = (26591000 / 1) /(jaguar_screen_scanlines * 60);
	}
	if (jaguar_mainRom_crc32 == 0x5a5b9c68)
	{
		gpu_cycles_per_scanline = (26591000 / 1) / (jaguar_screen_scanlines * 60);
	}
	if (jaguar_mainRom_crc32 == 0xdcb0197a)
	{
		dsp_enabled = false; // dsp not needed
		gpu_cycles_per_scanline = (26591000 / 1) / (jaguar_screen_scanlines * 60);
		//dsp_cycles_per_scanline=(26591000/1) /((jaguar_screen_scanlines)*60);
	}
	if ((jaguar_mainRom_crc32 == 0x3966698f) || (jaguar_mainRom_crc32 == 0xe21d0e2f))
		dsp_enabled = true;
	if (jaguar_mainRom_crc32 == 0x5e705756)
	{
		gpu_cycles_per_scanline = (26591000 / 1) / (jaguar_screen_scanlines * 60);
		dsp_enabled = true;
	}
	if (jaguar_mainRom_crc32 == 0x2630cbc4)
	{
		// ultra vortek
		gpu_cycles_per_scanline = (26591000 / 1) / (jaguar_screen_scanlines * 60);
		dsp_cycles_per_scanline = (26591000 / 1) / (jaguar_screen_scanlines * 60);
		dsp_enabled = true;
	}
	if ((jaguar_mainRom_crc32 == 0xd46437e8) || (jaguar_mainRom_crc32 == 0xba74c3ed))
	{
		gpu_cycles_per_scanline = (26591000 / 1) / (jaguar_screen_scanlines * 60);
		dsp_enabled = true;
	}
	if (jaguar_mainRom_crc32 == 0x6e90989f)
		gpu_cycles_per_scanline = (26591000 / 1) / (jaguar_screen_scanlines * 60);

	if (jaguar_mainRom_crc32 == 0x41307601)
	{
		gpu_cycles_per_scanline = (26591000 / 1) / (jaguar_screen_scanlines * 60);
	}

	if (jaguar_mainRom_crc32 == 0x8483392b)
	{
		dsp_enabled = true;
	}

#else	// #ifdef JAGUAR_WIP_RELEASE
//	jaguar_load_cart("C:/ftp/jaguar/roms/roms/flashback.jag",jaguar_mainRom,0x0000, 0x20000080,0);
//	jaguar_load_cart("C:/ftp/jaguar/roms/roms/Pinball Fantasies.JAG",jaguar_mainRom,0x0000, 0x20000080,0);
//	jaguar_load_cart("C:/ftp/jaguar/roms/roms/alien vs predator (1994).jag",jaguar_mainRom,0x0000, 0x20000080,0);
//	jaguar_load_cart("C:/ftp/jaguar/roms/roms/cannon fodder (1995) (computer west).jag",jaguar_mainRom,0x0000, 0x20000080,0);
//	jaguar_load_cart("C:/ftp/jaguar/roms/roms/double dragon v (1995) (williams).jag",jaguar_mainRom,0x0000, 0x20000080,0);
//	jaguar_load_cart("C:/ftp/jaguar/roms/roms/Dragon - The Bruce Lee Story.JAG",jaguar_mainRom,0x0000, 0x20000080,0);
//	jaguar_load_cart("C:/ftp/jaguar/roms/roms/Syndicate.JAG",jaguar_mainRom,0x0000, 0x20000080,0);
//	jaguar_load_cart("C:/ftp/jaguar/roms/roms/Theme Park.JAG",jaguar_mainRom,0x0000, 0x20000080,0);
//	jaguar_load_cart("C:/ftp/jaguar/roms/roms/Brutal Sports Football.JAG",jaguar_mainRom,0x0000, 0x20000080,0);
//	jaguar_load_cart("C:/ftp/jaguar/roms/roms/International Sensible Soccer.JAG",jaguar_mainRom,0x0000, 0x20000080,0);
//  jaguar_load_cart("C:/ftp/jaguar/roms/roms/Defender 2000.JAG",jaguar_mainRom,0x0000, 0x20000080,0);
//	jaguar_load_cart("C:/ftp/jaguar/roms/roms/Fever Pitch Soccer.JAG",jaguar_mainRom,0x0000, 0x20000080,0);
//	jaguar_load_cart("C:/ftp/jaguar/roms/roms/Rayman.JAG",jaguar_mainRom,0x0000, 0x20000080,0);
//	jaguar_load_cart("C:/ftp/jaguar/roms/roms/Tempest 2000.JAG",jaguar_mainRom,0x0000, 0x20000080,0);
//	jaguar_load_cart("C:/ftp/jaguar/roms/roms/zool 2 (1994).jag",jaguar_mainRom,0x0000, 0x20000080,0);
//	jaguar_load_cart("C:/ftp/jaguar/roms/roms/Bubsy - Fractured Furry Tails.JAG",jaguar_mainRom,0x0000, 0x20000080,0);
//	jaguar_load_cart("C:/ftp/jaguar/roms/roms/Raiden.JAG",jaguar_mainRom,0x0000, 0x20000080,0);
//	jaguar_load_cart("C:/ftp/jaguar/roms/roms/Dino Olympics.jag",jaguar_mainRom,0x0000, 0x20000080,0);
//	jaguar_load_cart("C:/ftp/jaguar/roms/roms/I-War.jag",jaguar_mainRom,0x0000, 0x20000080,0);
//	jaguar_load_cart("C:/ftp/jaguar/roms/roms/Attack of the Mutant Penguins.JAG",jaguar_mainRom,0x0000, 0x20000080,0);
//	jaguar_load_cart("C:/ftp/jaguar/roms/roms/Cybermorph.jag",jaguar_mainRom,0x0000, 0x20000080,0);
//	jaguar_load_cart("C:/ftp/jaguar/roms/roms/Troy Aikman NFL Football (1995) (Williams).jag",jaguar_mainRom,0x0000, 0x20000080,0);
//	jaguar_load_cart("C:/ftp/jaguar/roms/roms/Power Drive Rally (1995) (TWI).jag",jaguar_mainRom,0x0000, 0x20000080,0);
//	jaguar_load_cart("C:/ftp/jaguar/roms/roms/Zoop! (1996).jag",jaguar_mainRom,0x0000, 0x20000080,0);
//	jaguar_load_cart("C:/ftp/jaguar/roms/roms/Missile Command 3D.JAG",jaguar_mainRom,0x0000, 0x20000080,0);
//	jaguar_load_cart("C:/ftp/jaguar/roms/roms/Hover Strike.jag",jaguar_mainRom,0x0000, 0x20000080,0);
//	jaguar_load_cart("C:/ftp/jaguar/roms/roms/worms.bin",jaguar_mainRom,0x0000, 0x20000080,0);
//	jaguar_load_cart("C:/ftp/jaguar/roms/roms/Atari Kart.JAG",jaguar_mainRom,0x0000, 0x20000080,0);
//	jaguar_load_cart("C:/ftp/jaguar/roms/roms/native.bin",jaguar_mainRam,0x5000, 0x50000000,0x00);

	if (jaguar_mainRom_crc32==0xe21d0e2f)
	{
		dsp_enabled=true;
		gpu_cycles_per_scanline=(26591000/1) /((jaguar_screen_scanlines)*60);
		dsp_cycles_per_scanline=(26591000/1) /((jaguar_screen_scanlines)*60);
	}
	if (jaguar_mainRom_crc32==0x66f8914c)
	{
		gpu_cycles_per_scanline=(26591000/1) /((jaguar_screen_scanlines)*60);
	}
	if (jaguar_mainRom_crc32==0x5a5b9c68)
	{
		gpu_cycles_per_scanline=(26591000/1) /((jaguar_screen_scanlines)*60);
	}
//	jaguar_load_cart("C:/ftp/jaguar/roms/roms/Super Cross 3D.JAG",jaguar_mainRom,0x0000, 0x20000080,0);
	if (jaguar_mainRom_crc32==0xdcb0197a)
	{
		dsp_enabled=true; // dsp not needed
		gpu_cycles_per_scanline=(26591000/1) /((jaguar_screen_scanlines)*60);
		//dsp_cycles_per_scanline=(26591000/1) /((jaguar_screen_scanlines)*60);
	}
//  jaguar_load_cart("C:/ftp/jaguar/roms/roms/wolfenstein 3d (1994).jag",jaguar_mainRom,0x0000, 0x20000080,0);
	if ((jaguar_mainRom_crc32==0x3966698f)||(jaguar_mainRom_crc32==0xe21d0e2f))
		dsp_enabled=true;
//	jaguar_load_cart("C:/ftp/jaguar/roms/roms/NBA JAM.jag",jaguar_mainRom,0x0000, 0x20000080,0);
//	jaguar_load_cart("C:/ftp/jaguar/roms/roms/Doom - Evil Unleashed.JAG",jaguar_mainRom,0x0000, 0x20000080,0);
	if (jaguar_mainRom_crc32==0x5e705756)
	{
		gpu_cycles_per_scanline=(26591000/1) /((jaguar_screen_scanlines)*60);
		dsp_enabled=true;
	}
//	jaguar_load_cart("C:/ftp/jaguar/roms/roms/Ultra Vortek.JAG",jaguar_mainRom,0x0000, 0x20000080,0);
	if (jaguar_mainRom_crc32==0x2630cbc4)
	{
		// ultra vortek
		gpu_cycles_per_scanline=(26591000/1) /((jaguar_screen_scanlines)*60);
		dsp_cycles_per_scanline=(26591000/1) /((jaguar_screen_scanlines)*60);
		dsp_enabled=true; 
	}
//	jaguar_load_cart("C:/ftp/jaguar/roms/roms/fflbeta.rom",jaguar_mainRom,0x0000, 0x20000080,0);
// 	jaguar_load_cart("C:/ftp/jaguar/roms/roms/Fight for Your Life.jag",jaguar_mainRom,0x0000, 0x20000080,0);
	if ((jaguar_mainRom_crc32==0xd46437e8)||(jaguar_mainRom_crc32==0xba74c3ed))
	{
		gpu_cycles_per_scanline=(26591000/1) /((jaguar_screen_scanlines)*60);
//		dsp_cycles_per_scanline=(26591000/1) /((jaguar_screen_scanlines)*60);
		dsp_enabled=true;
	}
//	jaguar_load_cart("C:/ftp/jaguar/roms/roms/Pitfall - The Mayan Adventure.JAG",jaguar_mainRom,0x0000, 0x20000080,0);
	if (jaguar_mainRom_crc32==0x6e90989f)
		gpu_cycles_per_scanline=(26591000/1) /((jaguar_screen_scanlines)*60);

// missing some sprites
//	jaguar_load_cart("C:/ftp/jaguar/roms/roms/Crescent Galaxy.jag",jaguar_mainRom,0x0000, 0x20000080,0);
	if (jaguar_mainRom_crc32==0x41307601)
	{
		gpu_cycles_per_scanline=(26591000/1) /((jaguar_screen_scanlines)*60);
	}

// missing vertical bar shades	
//	jaguar_load_cart("C:/ftp/jaguar/roms/roms/Phase Zero (2000) (PD).rom",jaguar_mainRom,0x0000, 0x20000080,0);
	if (jaguar_mainRom_crc32==0x8483392b)
	{
		dsp_enabled=true;
	}
// cpu/dsp/gpu synchronization problems


// locks up during the game
//	jaguar_load_cart("C:/ftp/jaguar/roms/roms/Club Drive.JAG",jaguar_mainRom,0x0000, 0x20000080,0);

// no parallax floor, locks up at the start of the game	
// jaguar_load_cart("C:/ftp/jaguar/roms/roms/Kasumi Ninja.JAG",jaguar_mainRom,0x0000, 0x20000080,0);

// displaying the sound control dialog. no way to exit from it	
//	jaguar_load_cart("C:/ftp/jaguar/roms/roms/Checkered Flag.JAG",jaguar_mainRom,0x0000, 0x20000080,0);

// no 3d	
//  jaguar_load_cart("C:/ftp/jaguar/roms/roms/Iron Soldier.jag",jaguar_mainRom,0x0000, 0x20000080,0);

// locks up at the start of the game
//	jaguar_load_cart("C:/ftp/jaguar/roms/roms/Super Burnout.JAG",jaguar_mainRom,0x0000, 0x20000080,0);
	if (jaguar_mainRom_crc32==0x20ae75f4)
	{
		dsp_enabled=true;
		gpu_cycles_per_scanline=(26591000/1) /((jaguar_screen_scanlines)*60);
		dsp_cycles_per_scanline=(26591000/1) /((jaguar_screen_scanlines)*60);
	}
// locks up at the start of the game	
//	jaguar_load_cart("C:/ftp/jaguar/roms/roms/Val D'Isere Skiing & Snowboarding (1994).jag",jaguar_mainRom,0x0000, 0x20000080,0);
	if (jaguar_mainRom_crc32==0x4664ebd1)
	{
		dsp_enabled=true;
	}

// fonctionne avec le gpu et le dsp activés et gpu à frequence nominale, et dsp à 1/4 de la frequence nominale
//	jaguar_load_cart("C:/ftp/jaguar/roms/roms/white men can't jump (1995).jag",jaguar_mainRom,0x0000, 0x20000080,0);
	if (jaguar_mainRom_crc32==0x7ae20823)
	{
		dsp_enabled=true;
		gpu_cycles_per_scanline=(26591000/1) /((jaguar_screen_scanlines)*60);
	}
// not working at all
//	jaguar_load_cart("C:/ftp/jaguar/roms/roms/Flip Out.JAG",jaguar_mainRom,0x0000, 0x20000080,0);
	if (jaguar_mainRom_crc32==0x6f57dcd2)
	{
		gpu_cycles_per_scanline=(26591000/1) /((jaguar_screen_scanlines)*60);
		dsp_enabled=false;

	}

	jaguar_load_cart("C:/ftp/jaguar/roms/roms/Ruiner.JAG",jaguar_mainRom,0x0000, 0x20000080,0);
	if (jaguar_mainRom_crc32==0x6a7c7430)
	{
		dsp_enabled=true;
	}

	if (jaguar_mainRom_crc32==0x2f032271)
	{
		dsp_enabled=true;
		dsp_cycles_per_scanline=(26591000/1) /((jaguar_screen_scanlines)*60);
		gpu_cycles_per_scanline=(26591000/1) /((jaguar_screen_scanlines)*60);
	}
//	jaguar_load_cart("C:/ftp/jaguar/roms/roms/tetris.bin",jaguar_mainRam,0x4fe4, 0x50000000,0x00);
//	jaguar_load_cart("C:/ftp/jaguar/roms/roms/painter.bin",jaguar_mainRam,0xffe4, 0x00000001,0x00);
//	jaguar_load_cart("./roms/jagcd.rom",jaguar_mainRom,0x0000, 0x20000080,0);

//	jaguar_load_cart("cart.jag",jaguar_mainRom,0x0000, 0x20000080,0);

	
//	cd_bios_boot("C:\\ftp\\jaguar\\cd\\Brain Dead 13.cdi");
//	cd_bios_boot("C:\\ftp\\jaguar\\cd\\baldies.cdi");
//	cd_bios_boot("C:\\ftp\\jaguar\\cd\\mystdemo.cdi");
//	cd_bios_boot("C:\\ftp\\jaguar\\cd\\battlemorph.cdi");
//	cd_bios_boot("C:\\ftp\\jaguar\\cd\\primalrage.cdi");
//	cd_bios_boot("C:\\ftp\\jaguar\\cd\\Dragons Lair.cdi");

//	jaguar_load_cart("C:/ftp/jaguar/roms/roms/raw.jag",jaguar_mainRam,0x4000, 0x40000000,0x00);
#endif	// #ifdef JAGUAR_WIP_RELEASE

#ifdef JAGUAR_REAL_SPEED
	gpu_cycles_per_scanline = (26591000 / 1) / (jaguar_screen_scanlines * 60);
	dsp_cycles_per_scanline = (26591000 / 1) / (jaguar_screen_scanlines * 60);
#endif
#ifdef SOUND_OUTPUT
	ws_audio_init();
#endif

	m68k_set_cpu_type(M68K_CPU_TYPE_68000);
	gpu_init();
	dsp_init();
	tom_init();
	jerry_init();
	cdrom_init();
}

void jaguar_done(void)
{
//#ifdef CPU_DEBUG
//	for(int i=M68K_REG_A0; i<=M68K_REG_A7; i++)
//		WriteLog("\tA%i = 0x%.8x\n", i-M68K_REG_A0, m68k_get_reg(NULL, (m68k_register_t)i));
	uint32 topOfStack = m68k_get_reg(NULL, M68K_REG_A7);
	WriteLog("M68K: Top of stack: %08X. Stack trace:\n", jaguar_long_read(topOfStack));
	for(int i=0; i<10; i++)
		WriteLog("%06X: %08X\n", topOfStack - (i * 4), jaguar_long_read(topOfStack - (i * 4)));
//	WriteLog("Jaguar: CD BIOS version %04X\n", jaguar_word_read(0x3004));
	WriteLog("Jaguar: VBL interrupt is %s\n", ((tom_irq_enabled(IRQ_VBLANK)) && (jaguar_interrupt_handler_is_valid(64))) ? "enabled" : "disabled");
	M68K_show_context();
//#endif
#ifdef SOUND_OUTPUT
	ws_audio_done();
#endif
	cd_bios_done();
	cdrom_done();
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
#ifdef SOUND_OUTPUT
	ws_audio_reset();
#endif
	cd_bios_reset();
	tom_reset();
	jerry_reset();
	gpu_reset();
	dsp_reset();
	cdrom_reset();
    m68k_pulse_reset();                         // Reset the 68000
	WriteLog( "\t68K PC=%06X SP=%08X\n", m68k_get_reg(NULL, M68K_REG_PC), m68k_get_reg(NULL, M68K_REG_A7));
}

void jaguar_reset_handler(void)
{
}

void jaguar_exec(int16 * backbuffer, bool render)
{ 
	uint32 i, vblank_duration = tom_get_vdb();

	// vblank
	if ((tom_irq_enabled(IRQ_VBLANK)) && (jaguar_interrupt_handler_is_valid(64)))
	{
		if (jaguar_word_read(0xF0004E) != 0xFFFF)
		{
			tom_set_pending_video_int();
//			s68000interrupt(7, IRQ_VBLANK+64);
//			s68000flushInterrupts();
			m68k_set_irq(7);					// IRQ_VBLANK+64??? Not autovectored??? No.
// Could set a global variable here, to signal that this is a VBLANK interrupt...
// Then again, since IRQ_VBLANK is set to zero, this would not be necessary in this case.
		}
	}

	for(i=0; i<vblank_duration; i++)
	{
/*		uint32 invalid_instruction_address = s68000exec(m68k_cycles_per_scanline);
		if (invalid_instruction_address != 0x80000000)
			cd_bios_process(invalid_instruction_address);*/
		m68k_execute(m68k_cycles_per_scanline);
		// No CD handling... !!! FIX !!!

		cd_bios_exec(i);
		tom_pit_exec(m68k_cycles_per_scanline);
		tom_exec_scanline(backbuffer, i, false);
		jerry_pit_exec(m68k_cycles_per_scanline);
		jerry_i2s_exec(m68k_cycles_per_scanline);
		gpu_exec(gpu_cycles_per_scanline);
		if (dsp_enabled)
			dsp_exec(dsp_cycles_per_scanline);
	}
	
	for (; i<jaguar_screen_scanlines; i++)
	{
/*		uint32 invalid_instruction_address = s68000exec(m68k_cycles_per_scanline);
		if (invalid_instruction_address != 0x80000000)
			cd_bios_process(invalid_instruction_address);*/
		m68k_execute(m68k_cycles_per_scanline);
		// No CD handling... !!! FIX !!!
		cd_bios_exec(i);
		tom_pit_exec(m68k_cycles_per_scanline);
		jerry_pit_exec(m68k_cycles_per_scanline);
		jerry_i2s_exec(m68k_cycles_per_scanline);
		tom_exec_scanline(backbuffer, i, render);
		gpu_exec(gpu_cycles_per_scanline);
		if (dsp_enabled)
			dsp_exec(dsp_cycles_per_scanline);
		backbuffer += tom_width;
	}
#ifdef SOUND_OUTPUT
	system_sound_update();
#endif
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
