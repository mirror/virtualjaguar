#ifndef __JAGUAR_H__
#define __JAGUAR_H__

#include "types.h"

void JaguarInit(void);
void JaguarReset(void);
void JaguarDone(void);

uint8 JaguarReadByte(uint32 offset, uint32 who = UNKNOWN);
uint16 JaguarReadWord(uint32 offset, uint32 who = UNKNOWN);
uint32 JaguarReadLong(uint32 offset, uint32 who = UNKNOWN);
void JaguarWriteByte(uint32 offset, uint8 data, uint32 who = UNKNOWN);
void JaguarWriteWord(uint32 offset, uint16 data, uint32 who = UNKNOWN);
void JaguarWriteLong(uint32 offset, uint32 data, uint32 who = UNKNOWN);

bool JaguarInterruptHandlerIsValid(uint32 i);
void JaguarDasm(uint32 offset, uint32 qt);

void JaguarExecute(uint32 * backbuffer, bool render);
//For testing the new system...
void JaguarExecuteNew(void);

// Exports from JAGUAR.CPP

extern uint8 jaguarMainRam[];
extern uint8 jaguarMainRom[];
extern uint8 jaguarBootRom[];
extern uint8 jaguarCDBootROM[];
extern bool BIOSLoaded;
extern bool CDBIOSLoaded;
extern int32 jaguarCPUInExec;
extern uint32 jaguarMainRomCRC32, jaguarRomSize, jaguarRunAddress;
extern char * jaguarEepromsPath;
extern const char * whoName[9];

// Some handy macros to help converting native endian to big endian (jaguar native)
// & vice versa

#define SET64(r, a, v) 	r[(a)] = ((v) & 0xFF00000000000000) >> 56, r[(a)+1] = ((v) & 0x00FF000000000000) >> 48, \
						r[(a)+2] = ((v) & 0x0000FF0000000000) >> 40, r[(a)+3] = ((v) & 0x000000FF00000000) >> 32, \
						r[(a)+4] = ((v) & 0xFF000000) >> 24, r[(a)+5] = ((v) & 0x00FF0000) >> 16, \
						r[(a)+6] = ((v) & 0x0000FF00) >> 8, r[(a)+7] = (v) & 0x000000FF
#define GET64(r, a)		(((uint64)r[(a)] << 56) | ((uint64)r[(a)+1] << 48) | \
						((uint64)r[(a)+2] << 40) | ((uint64)r[(a)+3] << 32) | \
						((uint64)r[(a)+4] << 24) | ((uint64)r[(a)+5] << 16) | \
						((uint64)r[(a)+6] << 8) | (uint64)r[(a)+7])
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
