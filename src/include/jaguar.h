#ifndef __JAGUAR_H__
#define __JAGUAR_H__

#include <string.h>	// Why??? (for memset, etc... Lazy!) Dunno why, but this just strikes me as wrong...
#include "types.h"
#include "log.h"
#include "version.h"
#include "memory.h"
#include "m68k.h"
#include "tom.h"
#include "jerry.h"
#include "gpu.h"
#include "dsp.h"
#include "objectp.h"
#include "blitter.h"
#include "clock.h"
#include "anajoy.h"
#include "joystick.h"
#include "dac.h"
#include "jagdasm.h"
#include "cdrom.h"
#include "eeprom.h"
#include "cdi.h"
#include "cdbios.h"

// Exports from JAGUAR.CPP

extern int32 jaguar_cpu_in_exec;
extern uint32 jaguar_mainRom_crc32, jaguarRomSize, jaguarRunAddress;
extern char * jaguar_eeproms_path;
extern char * whoName[9];

void jaguar_init(void);
void jaguar_reset(void);
void jaguar_done(void);

uint8 JaguarReadByte(uint32 offset, uint32 who = UNKNOWN);
uint16 JaguarReadWord(uint32 offset, uint32 who = UNKNOWN);
uint32 JaguarReadLong(uint32 offset, uint32 who = UNKNOWN);
void JaguarWriteByte(uint32 offset, uint8 data, uint32 who = UNKNOWN);
void JaguarWriteWord(uint32 offset, uint16 data, uint32 who = UNKNOWN);
void JaguarWriteLong(uint32 offset, uint32 data, uint32 who = UNKNOWN);

uint32 jaguar_interrupt_handler_is_valid(uint32 i);
void jaguar_dasm(uint32 offset, uint32 qt);

void JaguarExecute(uint32 * backbuffer, bool render);

// Some handy macros to help converting native endian to big endian (jaguar native)
// & vice versa

#define SET32(r, a, v)	r[(a)] = ((v) & 0xFF000000) >> 24, r[(a)+1] = ((v) & 0x00FF0000) >> 16, \
						r[(a)+2] = ((v) & 0x0000FF00) >> 8, r[(a)+3] = (v) & 0x000000FF
#define GET32(r, a)		((r[(a)] << 24) | (r[(a)+1] << 16) | (r[(a)+2] << 8) | r[(a)+3])
#define SET16(r, a, v)	r[(a)] = ((v) & 0xFF00) >> 8, r[(a)+1] = (v) & 0xFF
#define GET16(r, a)		((r[(a)] << 8) | r[(a)+1])

// Various clock rates

#define M68K_CLOCK_RATE_PAL		13296950
#define M68K_CLOCK_RATE_NTSC	13295453
#define RISC_CLOCK_RATE_PAL		26593900
#define RISC_CLOCK_RATE_NTSC	26590906

// Stuff for IRQ handling

#define ASSERT_LINE		1
#define CLEAR_LINE		0

//Temp debug stuff (will go away soon, so don't depend on these)

void DumpMainMemory(void);
uint8 * GetRamPtr(void);

#endif	// __JAGUAR_H__
