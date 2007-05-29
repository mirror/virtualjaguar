//
// JAGUAR.CPP
//
// by cal16
// GCC/SDL port by Niels Wagenaar (Linux/WIN32) and Caz (BeOS)
// Cleanups and endian wrongness amelioration by James L. Hammons
// Note: Endian wrongness probably stems from the MAME origins of this emu and
//       the braindead way in which MAME handles memory. :-)
// 

#include "jaguar.h"
#include "m68kdasmAG.h"
#include "crc32.h"

//#define LOG_UNMAPPED_MEMORY_ACCESSES
//#define SOUND_OUTPUT

// Some handy macros to help converting native endian to big endian (jaguar native)

#define SET32(r, a, v)	r[a] = ((v) & 0xFF000000) >> 24, r[a+1] = ((v) & 0x00FF0000) >> 16, \
						r[a+2] = ((v) & 0x0000FF00) >> 8, r[a+3] = (v) & 0x000000FF

//
// Function Prototypes
//

unsigned jaguar_unknown_readbyte(unsigned address);
unsigned jaguar_unknown_readword(unsigned address);
void jaguar_unknown_writebyte(unsigned address, unsigned data);
void jaguar_unknown_writeword(unsigned address, unsigned data);


#ifdef SOUND_OUTPUT
int dsp_enabled = 1;
#else
int dsp_enabled = 0;
#endif
uint32 jaguar_active_memory_dumps = 0;
uint32 jaguar_use_bios = 0;
#define JAGUAR_WIP_RELEASE
#define JAGUAR_REAL_SPEED


//////////////////////////////////////////////////////////////////////////////
// Bios path
//////////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////////
//static char  *jaguar_bootRom_path="c:/jaguarEmu/newload.img";
static char * jaguar_bootRom_path = "./bios/jagboot.rom";
//static char  *jaguar_bootRom_path="./bios/JagOS.bin";
char * jaguar_eeproms_path = "./eeproms/";

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////////
uint32 jaguar_mainRom_crc32;

static uint32 m68k_cycles_per_scanline;
static uint32 gpu_cycles_per_scanline;
static uint32 dsp_cycles_per_scanline;
static uint32 jaguar_screen_scanlines;

static uint8 * jaguar_mainRam = NULL;
static uint8 * jaguar_bootRom = NULL;
static uint8 * jaguar_mainRom = NULL;
//////////////////////////////////////////////////////////////////////////////


//
// Musashi 68000 read/write/IRQ functions
//

int irq_ack_handler(int level)
{
	int vector = M68K_INT_ACK_AUTOVECTOR;

	if (level = 7)
	{
		m68k_set_irq(0);						// Clear the IRQ...
		vector = 64;							// Set user interrupt #0
	}

	return vector;
}

//Do this in makefile???
//#define LOG_UNMAPPED_MEMORY_ACCESSES 1

unsigned int m68k_read_memory_8(unsigned int address)
{
//fprintf(log_get(), "[RM8] Addr: %08X\n", address);
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

unsigned int m68k_read_memory_16(unsigned int address)
{
//fprintf(log_get(), "[RM16] Addr: %08X\n", address);
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
//fprintf(log_get(), "[RM16] Unknown address: %08X\n", address);
		retVal = jaguar_unknown_readword(address);
//}

    return retVal;
}

unsigned int m68k_read_memory_32(unsigned int address)
{
//fprintf(log_get(), "--> [RM32]\n");
    return (m68k_read_memory_16(address) << 16) | m68k_read_memory_16(address + 2);
}

void m68k_write_memory_8(unsigned int address, unsigned int value)
{
//fprintf(log_get(), "[WM8  PC=%08X] Addr: %08X, val: %02X\n", m68k_get_reg(NULL, M68K_REG_PC), address, value);
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
//fprintf(log_get(), "[WM16 PC=%08X] Addr: %08X, val: %04X\n", m68k_get_reg(NULL, M68K_REG_PC), address, value);
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
//fprintf(log_get(), "--> [WM32]\n");
	m68k_write_memory_16(address, value >> 16);
	m68k_write_memory_16(address + 2, value & 0xFFFF);
}

//////////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////////
/*struct STARSCREAM_PROGRAMREGION *jaguar_programfetch;
struct STARSCREAM_DATAREGION    *jaguar_readbyte;
struct STARSCREAM_DATAREGION    *jaguar_readword;
struct STARSCREAM_DATAREGION    *jaguar_writebyte;
struct STARSCREAM_DATAREGION    *jaguar_writeword;
UINT16 jaguar_regionsCount[6];*/

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////////
uint32 jaguar_get_handler(uint32 i)
{
//	return (jaguar_word_read(i<<2) << 16) | jaguar_word_read((i<<2) + 2);
	return (jaguar_word_read(i*4) << 16) | jaguar_word_read((i*4) + 2);
}

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
//
//
//
//
//
//////////////////////////////////////////////////////////////////////////////
static char romLoadDialog_filePath[1024];
#ifndef __PORT__
static char romLoadDialog_initialDirectory[1024];

int jaguar_open_rom(HWND hWnd,char *title,char *filterString)
{
	OPENFILENAME ofn;


	romLoadDialog_initialDirectory[0] = 0;


	romLoadDialog_filePath[0] = 0;

	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hWnd;
	ofn.lpstrFile = romLoadDialog_filePath;
	ofn.nMaxFile = sizeof(romLoadDialog_filePath);
	ofn.lpstrFilter =  filterString;
	ofn.nFilterIndex = 0;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = (const char *)romLoadDialog_initialDirectory;
	ofn.lpstrTitle = title; 
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	if(GetOpenFileName(&ofn) == FALSE) 
	{
		DWORD res = CommDlgExtendedError();
		SendMessage(hWnd, WM_MOVE, 0,0);
		return 0;
	}


	SendMessage(hWnd, WM_MOVE, 0,0);
	return 1;
}
#endif
//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////////
uint32 jaguar_interrupt_handler_is_valid(uint32 i)
{
	uint32 handler = jaguar_get_handler(i);
	if (handler && (handler != 0xFFFFFFFF))
		return 1;
	else
		return 0;
}
//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////////
void s68000show_context(void)
{
//	fprintf(log_get(),"\t68k PC=0x%.6x\n",s68000readPC());
	fprintf(log_get(),"\t68k PC=0x%.6x\n", m68k_get_reg(NULL, M68K_REG_PC));
//	for (int i=0;i<8;i++)
//		fprintf(log_get(),"\tD%i = 0x%.8x\n",i,s68000context.dreg[i]);
	for(int i=M68K_REG_D0; i<=M68K_REG_D7; i++)
		fprintf(log_get(), "\tD%i = 0x%.8x\n", i-M68K_REG_D0, m68k_get_reg(NULL, (m68k_register_t)i));
	fprintf(log_get(), "\n");
//	for (i=0;i<8;i++)
//		fprintf(log_get(),"\tA%i = 0x%.8x\n",i,s68000context.areg[i]);
	for(int i=M68K_REG_A0; i<=M68K_REG_A7; i++)
		fprintf(log_get(), "\tA%i = 0x%.8x\n", i-M68K_REG_A0, m68k_get_reg(NULL, (m68k_register_t)i));

	fprintf(log_get(), "68k dasm\n");
//	jaguar_dasm(s68000readPC()-0x1000,0x20000);
//	jaguar_dasm(m68k_get_reg(NULL, M68K_REG_PC) - 0x1000, 0x20000);
	jaguar_dasm(m68k_get_reg(NULL, M68K_REG_PC) - 0x80, 0x200);
	fprintf(log_get(), "..................\n");


	if (tom_irq_enabled(IRQ_VBLANK))
	{
		fprintf(log_get(), "vblank int: enabled\n");
		jaguar_dasm(jaguar_get_handler(64), 0x200);
	}
	else
		fprintf(log_get(), "vblank int: disabled\n");
	fprintf(log_get(), "..................\n");

	for(int i=0; i<256; i++)
		fprintf(log_get(), "handler %03i at $%08X\n", i, jaguar_get_handler(i));
}

// Starscream crap ripped out...

//
// Unknown read/write byte/word routines
//

void jaguar_unknown_writebyte(unsigned address, unsigned data)
{
#ifdef LOG_UNMAPPED_MEMORY_ACCESSES
	fprintf(log_get(), "jaguar: unknown byte %02X write at %08X (PC=%06X)\n", data, address, m68k_get_reg(NULL, M68K_REG_PC));
#endif
}

void jaguar_unknown_writeword(unsigned address, unsigned data)
{
#ifdef LOG_UNMAPPED_MEMORY_ACCESSES
	fprintf(log_get(), "jaguar: unknown word %04X write at %08X (PC=%06X)\n", data, address, m68k_get_reg(NULL, M68K_REG_PC));
#endif
}

unsigned jaguar_unknown_readbyte(unsigned address)
{
#ifdef LOG_UNMAPPED_MEMORY_ACCESSES
	fprintf(log_get(), "jaguar: unknown byte read at %08X (PC=%06X)\n", address, m68k_get_reg(NULL, M68K_REG_PC));
#endif
    return 0xFF;
}

unsigned jaguar_unknown_readword(unsigned address)
{
#ifdef LOG_UNMAPPED_MEMORY_ACCESSES
	fprintf(log_get(), "jaguar: unknown word read at %08X (PC=%06X)\n", address, m68k_get_reg(NULL, M68K_REG_PC));
#endif
    return 0xFFFF;
}

//
// Jaguar ROM loading
//

uint8 * jaguar_rom_load(char * path, uint32 * romSize)
{
	uint8 * rom = NULL;
	fpos_t pos;
	int	x;
	__int64	filepos;

	fprintf(log_get(), "jaguar: loading %s...", path);
	FILE * fp = fopen(path, "rb");
	if (fp == NULL)
	{
		fprintf(log_get(), "failed\n");
		log_done();
		exit(0);
		return NULL;
	}
	fseek(fp, 0, SEEK_END);

	/* Added by SDLEMU (http://sdlemu.ngemu.com) */
	/* Added for GCC UNIX compatibility          */
	#ifdef __GCCUNIX__
	fgetpos(fp, (fpos_t *)&filepos);
	#else
	fgetpos(fp, &filepos);
	#endif
	
	*romSize = (int)filepos;
	fseek(fp, 0, SEEK_SET);
	rom = (uint8 *)malloc(*romSize);
	fread(rom, 1, *romSize, fp);
	fclose(fp);
	fprintf(log_get(), "ok (%i bytes)\n", *romSize);
//	jaguar_mainRom_crc32=crc32_calcCheckSum(jaguar_mainRom,*romSize);
//	fprintf(log_get(),"crc: 0x%.8x\n",jaguar_mainRom_crc32);
	return rom;
}

//
// Load a ROM at a specific address
//

void jaguar_rom_load_to(uint8 * rom, char * path, uint32 * romSize)
{
	__int64 filepos;

	fprintf(log_get(), "jaguar: loading %s...", path);
	FILE * fp = fopen(path, "rb");
	if (fp == NULL)
	{
		fprintf(log_get(), "failed\n");
		log_done();
		exit(0);
		return;
	}
	fseek(fp, 0, SEEK_END);

	/* Added by SDLEMU (http://sdlemu.ngemu.com) */
	/* Added for GCC UNIX compatibility          */
	#ifdef __GCCUNIX__
	fgetpos(fp, (fpos_t *)&filepos);
	#else
	fgetpos(fp, &filepos);
	#endif

	*romSize = (int)filepos;
	fseek(fp, 0, SEEK_SET);
	fread(rom, 1, *romSize, fp);
	fclose(fp);
	fprintf(log_get(), "ok (%i bytes)\n", *romSize);
}

//
// Byte swap a region of memory
//

void jaguar_byte_swap(uint8 * rom, uint32 size)
{
	while (size > 0)
	{
		uint8 tmp = rom[0];
		rom[0] = rom[1];
		rom[1] = tmp;
		rom += 2;
		size -= 2;
	}
}

//
// Disassemble instructions at the given offset
//

void jaguar_dasm(uint32 offset, uint32 qt)
{
	static char buffer[2048], mem[64];
	int pc = offset, oldpc;

	for(int i=0; i<qt; i++)
	{
		oldpc = pc;
		for(int j=0; j<64; j++)
//			mem[j^0x01] = jaguar_byte_read(pc + j);
			mem[j^0x01] = jaguar_byte_read(pc + j);

		pc += Dasm68000((char *)mem, buffer, 0);
#ifdef CPU_DEBUG
		fprintf(log_get(), "%08X: %s\n", oldpc, buffer);
#endif
	}
}

//
// Jaguar cartridge ROM loading
//

void jaguar_load_cart(char * path, uint8 * mem, uint32 offs, uint32 boot, uint32 header)
{
	uint32 romsize;

	jaguar_rom_load_to(mem+offs-header, path, &romsize);
// Is there a need for this? The answer is !!! NO !!!
//	jaguar_byte_swap(mem+offs, romsize);
	jaguar_mainRom_crc32 = crc32_calcCheckSum(jaguar_mainRom, romsize);
	fprintf(log_get(), "crc: %08X\n", jaguar_mainRom_crc32);

// Brain dead endian dependent crap
//	*((uint32 *)&jaguar_mainRam[4]) = boot;
// This is how it *should* have been done...
	SET32(jaguar_mainRam, 4, boot);
// Same as above...
//	jaguar_dasm((boot>>16) | (boot<<16), 32*4);
	jaguar_dasm(boot, 32*4);
}

//
// Jaguar initialization
//

#ifdef __PORT__
void jaguar_init(const char * filename)
#else
void jaguar_init(void)
#endif
{
	uint32 romsize;

	jaguar_screen_scanlines = 525;			// PAL screen size
	m68k_cycles_per_scanline = 13300000 / (jaguar_screen_scanlines * 60);
	gpu_cycles_per_scanline = (26591000 / 4) / (jaguar_screen_scanlines * 60);
	dsp_cycles_per_scanline = (26591000 / 4) / (jaguar_screen_scanlines * 60);

	memory_malloc_secure((void**)&jaguar_mainRam, 0x400000, "Jaguar 68k cpu ram");
	memory_malloc_secure((void**)&jaguar_bootRom, 0x040000, "Jaguar 68k cpu boot rom");
	memory_malloc_secure((void**)&jaguar_mainRom, 0x600000, "Jaguar 68k cpu rom");
	memset(jaguar_mainRam, 0x00, 0x400000);

	jaguar_rom_load_to(jaguar_bootRom, jaguar_bootRom_path, &romsize);
// No need to do this anymore, since Starcrap is gone!
//	jaguar_byte_swap(jaguar_bootRom, romsize);
	memcpy(jaguar_mainRam, jaguar_bootRom, 8);
// More braindead endian dependent crap
//WAS:	*((uint32 *)&jaguar_mainRam[0]) = 0x00000020;
	SET32(jaguar_mainRam, 0, 0x00200000);

#ifdef JAGUAR_WIP_RELEASE
#ifdef __PORT__
	strcpy(romLoadDialog_filePath, filename);
#else
	jaguar_open_rom(GetForegroundWindow(), "Load", "Jaguar roms (*.JAG)\0*.JAG\0\0");
#endif
//WAS: 	jaguar_load_cart(romLoadDialog_filePath, jaguar_mainRom, 0x0000, 0x20000080, 0);
 	jaguar_load_cart(romLoadDialog_filePath, jaguar_mainRom, 0x0000, 0x00802000, 0);
	if ((jaguar_mainRom_crc32 == 0x3966698f) || (jaguar_mainRom_crc32 == 0x5e705756)
		|| (jaguar_mainRom_crc32 == 0x2630cbc4) || (jaguar_mainRom_crc32 == 0xd46437e8)
		|| (jaguar_mainRom_crc32 == 0x2630cbc4))
		dsp_enabled = 1;

	if ((jaguar_mainRom_crc32 == 0x6e90989f) || (jaguar_mainRom_crc32 == 0xfc8f0dcd)
		|| (jaguar_mainRom_crc32 == 0x2a512a83) || (jaguar_mainRom_crc32 == 0x41307601)
		|| (jaguar_mainRom_crc32 == 0x3c7bfda8) || (jaguar_mainRom_crc32 == 0x5e705756))
		gpu_cycles_per_scanline = (26591000 / 1) / (jaguar_screen_scanlines * 60);

	if (jaguar_mainRom_crc32 == 0x7ae20823)
	{
		dsp_enabled = 1;
		gpu_cycles_per_scanline = (26591000 / 1) / (jaguar_screen_scanlines * 60);
		dsp_cycles_per_scanline = (26591000 / 1) / (jaguar_screen_scanlines * 60);
	}
	if (jaguar_mainRom_crc32 == 0xe21d0e2f)
	{
		dsp_enabled = 1;
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
		dsp_enabled = 0; // dsp not needed
		gpu_cycles_per_scanline = (26591000 / 1) / (jaguar_screen_scanlines * 60);
		//dsp_cycles_per_scanline=(26591000/1) /((jaguar_screen_scanlines)*60);
	}
	if ((jaguar_mainRom_crc32 == 0x3966698f) || (jaguar_mainRom_crc32 == 0xe21d0e2f))
		dsp_enabled = 1;
	if (jaguar_mainRom_crc32 == 0x5e705756)
	{
		gpu_cycles_per_scanline = (26591000 / 1) / (jaguar_screen_scanlines * 60);
		dsp_enabled = 1;
	}
	if (jaguar_mainRom_crc32 == 0x2630cbc4)
	{
		// ultra vortek
		gpu_cycles_per_scanline = (26591000 / 1) / (jaguar_screen_scanlines * 60);
		dsp_cycles_per_scanline = (26591000 / 1) / (jaguar_screen_scanlines * 60);
		dsp_enabled = 1;
	}
	if ((jaguar_mainRom_crc32 == 0xd46437e8) || (jaguar_mainRom_crc32 == 0xba74c3ed))
	{
		gpu_cycles_per_scanline = (26591000 / 1) / (jaguar_screen_scanlines * 60);
		dsp_enabled = 1;
	}
	if (jaguar_mainRom_crc32 == 0x6e90989f)
		gpu_cycles_per_scanline = (26591000 / 1) / (jaguar_screen_scanlines * 60);

	if (jaguar_mainRom_crc32 == 0x41307601)
	{
		gpu_cycles_per_scanline = (26591000 / 1) / (jaguar_screen_scanlines * 60);
	}

	if (jaguar_mainRom_crc32 == 0x8483392b)
	{
		dsp_enabled = 1;
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
		dsp_enabled=1;
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
		dsp_enabled=0; // dsp not needed
		gpu_cycles_per_scanline=(26591000/1) /((jaguar_screen_scanlines)*60);
		//dsp_cycles_per_scanline=(26591000/1) /((jaguar_screen_scanlines)*60);
	}
//  jaguar_load_cart("C:/ftp/jaguar/roms/roms/wolfenstein 3d (1994).jag",jaguar_mainRom,0x0000, 0x20000080,0);
	if ((jaguar_mainRom_crc32==0x3966698f)||(jaguar_mainRom_crc32==0xe21d0e2f))
		dsp_enabled=1;
//	jaguar_load_cart("C:/ftp/jaguar/roms/roms/NBA JAM.jag",jaguar_mainRom,0x0000, 0x20000080,0);
//	jaguar_load_cart("C:/ftp/jaguar/roms/roms/Doom - Evil Unleashed.JAG",jaguar_mainRom,0x0000, 0x20000080,0);
	if (jaguar_mainRom_crc32==0x5e705756)
	{
		gpu_cycles_per_scanline=(26591000/1) /((jaguar_screen_scanlines)*60);
		dsp_enabled=1;
	}
//	jaguar_load_cart("C:/ftp/jaguar/roms/roms/Ultra Vortek.JAG",jaguar_mainRom,0x0000, 0x20000080,0);
	if (jaguar_mainRom_crc32==0x2630cbc4)
	{
		// ultra vortek
		gpu_cycles_per_scanline=(26591000/1) /((jaguar_screen_scanlines)*60);
		dsp_cycles_per_scanline=(26591000/1) /((jaguar_screen_scanlines)*60);
		dsp_enabled=1; 
	}
//	jaguar_load_cart("C:/ftp/jaguar/roms/roms/fflbeta.rom",jaguar_mainRom,0x0000, 0x20000080,0);
// 	jaguar_load_cart("C:/ftp/jaguar/roms/roms/Fight for Your Life.jag",jaguar_mainRom,0x0000, 0x20000080,0);
	if ((jaguar_mainRom_crc32==0xd46437e8)||(jaguar_mainRom_crc32==0xba74c3ed))
	{
		gpu_cycles_per_scanline=(26591000/1) /((jaguar_screen_scanlines)*60);
//		dsp_cycles_per_scanline=(26591000/1) /((jaguar_screen_scanlines)*60);
		dsp_enabled=1;
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
		dsp_enabled=1;
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
		dsp_enabled=1;
		gpu_cycles_per_scanline=(26591000/1) /((jaguar_screen_scanlines)*60);
		dsp_cycles_per_scanline=(26591000/1) /((jaguar_screen_scanlines)*60);
	}
// locks up at the start of the game	
//	jaguar_load_cart("C:/ftp/jaguar/roms/roms/Val D'Isere Skiing & Snowboarding (1994).jag",jaguar_mainRom,0x0000, 0x20000080,0);
	if (jaguar_mainRom_crc32==0x4664ebd1)
	{
		dsp_enabled=1;
	}

// fonctionne avec le gpu et le dsp activés et gpu à frequence nominale, et dsp à 1/4 de la frequence nominale
//	jaguar_load_cart("C:/ftp/jaguar/roms/roms/white men can't jump (1995).jag",jaguar_mainRom,0x0000, 0x20000080,0);
	if (jaguar_mainRom_crc32==0x7ae20823)
	{
		dsp_enabled=1;
		gpu_cycles_per_scanline=(26591000/1) /((jaguar_screen_scanlines)*60);
	}
// not working at all
//	jaguar_load_cart("C:/ftp/jaguar/roms/roms/Flip Out.JAG",jaguar_mainRom,0x0000, 0x20000080,0);
	if (jaguar_mainRom_crc32==0x6f57dcd2)
	{
		gpu_cycles_per_scanline=(26591000/1) /((jaguar_screen_scanlines)*60);
		dsp_enabled=0;

	}

	jaguar_load_cart("C:/ftp/jaguar/roms/roms/Ruiner.JAG",jaguar_mainRom,0x0000, 0x20000080,0);
	if (jaguar_mainRom_crc32==0x6a7c7430)
	{
		dsp_enabled=1;
	}

	if (jaguar_mainRom_crc32==0x2f032271)
	{
		dsp_enabled=1;
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
	if (jaguar_use_bios)
	{
		memcpy(jaguar_mainRam, jaguar_bootRom, 8);
		*((uint32 *)&jaguar_mainRam[0]) = 0x00000020;
	}

//	s68000init();
	m68k_set_cpu_type(M68K_CPU_TYPE_68000);
	gpu_init();
	dsp_init();
	tom_init();
	jerry_init();
	cdrom_init();
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
unsigned jaguar_byte_read(unsigned int offset)
{
	uint8 data = 0x00;

	offset &= 0xFFFFFF;
	if (offset < 0x400000)
//		data = (jaguar_mainRam[(offset^0x01) & 0x3FFFFF]);
		data = (jaguar_mainRam[offset & 0x3FFFFF]);
	else if ((offset >= 0x800000) && (offset < 0xC00000))
//		data = (jaguar_mainRom[(offset^0x01)-0x800000]);
		data = (jaguar_mainRom[offset - 0x800000]);
	else if ((offset >= 0xDFFF00) && (offset < 0xDFFF00))
		data = cdrom_byte_read(offset);
	else if ((offset >= 0xE00000) && (offset < 0xE40000))
//		data = (jaguar_bootRom[(offset^0x01) & 0x3FFFF]);
		data = (jaguar_bootRom[offset & 0x3FFFF]);
	else if ((offset >= 0xF00000) && (offset < 0xF10000))
		data = (tom_byte_read(offset));
	else if ((offset >= 0xF10000) && (offset < 0xF20000))
		data = (jerry_byte_read(offset));
	else
		data = jaguar_unknown_readbyte(offset);

	return data;
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
unsigned jaguar_word_read(unsigned int offset)
{
	offset &= 0xFFFFFF;
	if (offset < 0x400000)
	{
//		return (jaguar_mainRam[(offset+1) & 0x3FFFFF] << 8) | jaguar_mainRam[(offset+0) & 0x3FFFFF];
		return (jaguar_mainRam[(offset+0) & 0x3FFFFF] << 8) | jaguar_mainRam[(offset+1) & 0x3FFFFF];
	}
	else if ((offset >= 0x800000) && (offset < 0xC00000))
	{
		offset -= 0x800000;
//		return (jaguar_mainRom[offset+1] << 8) | jaguar_mainRom[offset+0];
		return (jaguar_mainRom[offset+0] << 8) | jaguar_mainRom[offset+1];
	}
	else if ((offset >= 0xDFFF00) && (offset < 0xDFFF00))
		return cdrom_word_read(offset);
	else if ((offset >= 0xE00000) && (offset < 0xE40000))
		return *((uint16 *)&jaguar_bootRom[offset & 0x3FFFF]);
	else if ((offset >= 0xF00000) && (offset < 0xF10000))
		return tom_word_read(offset);
	else if ((offset >= 0xF10000) && (offset < 0xF20000))
		return jerry_word_read(offset);

	return jaguar_unknown_readword(offset);
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
void jaguar_byte_write(unsigned  offset, unsigned  data)
{
	offset &= 0xFFFFFF;
	if (offset < 0x400000)
	{
//		jaguar_mainRam[(offset^0x01) & 0x3FFFFF] = data;
		jaguar_mainRam[offset & 0x3FFFFF] = data;
		return;
	}
	else if ((offset >= 0xDFFF00) && (offset < 0xDFFF00))
	{
		cdrom_byte_write(offset, data);
		return;
	}
	else if ((offset >= 0xF00000) && (offset < 0xF10000))
	{
		tom_byte_write(offset, data);
		return;
	}
	else if ((offset >= 0xF10000) && (offset < 0xF20000))
	{
		jerry_byte_write(offset, data);
		return;
	}
    
	jaguar_unknown_writebyte(offset, data);
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
void jaguar_word_write(unsigned offset, unsigned data)
{
	offset &= 0xFFFFFF;
	
	if (offset < 0x400000)
	{
//		jaguar_mainRam[(offset+0) & 0x3FFFFF] = data & 0xFF;
//		jaguar_mainRam[(offset+1) & 0x3FFFFF] = (data>>8) & 0xFF;
		jaguar_mainRam[(offset+0) & 0x3FFFFF] = (data>>8) & 0xFF;
		jaguar_mainRam[(offset+1) & 0x3FFFFF] = data & 0xFF;
		return;
	}
	else if ((offset >= 0xDFFF00) && (offset < 0xDFFFFF))
	{
		cdrom_word_write(offset, data);
		return;
	}
	else if ((offset >= 0xF00000) && (offset < 0xF10000))
	{
		tom_word_write(offset, data);
		return;
	}
	else if ((offset >= 0xF10000) && (offset < 0xF20000))
	{
		jerry_word_write(offset, data);
		return;
	}
    
	jaguar_unknown_writeword(offset, data);
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
unsigned jaguar_long_read(unsigned int offset)
{
/*	uint32 data = jaguar_word_read(offset);
	data = (data<<16) | jaguar_word_read(offset+2);
	return data;*/
	return (jaguar_word_read(offset) << 16) | jaguar_word_read(offset+2);
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
void jaguar_long_write(unsigned offset, unsigned data)
{
	jaguar_word_write(offset, data >> 16);
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
void jaguar_done(void)
{
	fprintf(log_get(), "jaguar: top of stack: %08X\n", jaguar_long_read(0x001FFFF8));
//	fprintf(log_get(),"jaguar: cd bios version 0x%.4x\n",jaguar_word_read(0x3004));
//	fprintf(log_get(),"jaguar: vbl interrupt is %s\n",((tom_irq_enabled(IRQ_VBLANK))&&(jaguar_interrupt_handler_is_valid(64)))?"enabled":"disabled");
	s68000show_context();
#ifdef SOUND_OUTPUT
	ws_audio_done();
#endif
	cd_bios_done();
	cdrom_done();
	tom_done();
	jerry_done();
//	jaguar_regionsDone();
	memory_free(jaguar_mainRom);
	memory_free(jaguar_bootRom);
	memory_free(jaguar_mainRam);
//	fprintf(log_get(),"jaguar_done()\n");
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
void jaguar_reset(void)
{
//	fprintf(log_get(),"jaguar_reset():\n");
#ifdef SOUND_OUTPUT
	ws_audio_reset();
#endif
	cd_bios_reset();
	tom_reset();
	jerry_reset();
	gpu_reset();
	dsp_reset();
	cdrom_reset();
//	s68000reset();
    m68k_pulse_reset();                         // Reset the 68000
	fprintf(log_get(), "\t68K PC=%06X SP=%08X\n", m68k_get_reg(NULL, M68K_REG_PC), m68k_get_reg(NULL, M68K_REG_A7));
}
//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////////
void jaguar_reset_handler(void)
{
}
//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////////
void jaguar_exec(int16 * backbuffer, uint8 render)
{ 
	int i;
	uint32 vblank_duration = tom_get_vdb();

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
		// No CD handling... Hmm...

		cd_bios_exec(i);
		tom_pit_exec(m68k_cycles_per_scanline);
		tom_exec_scanline(backbuffer, i, 0);
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
		// No CD handling... Hmm...
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
