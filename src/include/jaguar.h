#ifndef __JAGUAR_H__
#define __JAGUAR_H__

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
#include "pcm.h"
#include "jagdasm.h"
#include "dsnd.h"
#include "cdrom.h"
#include "eeprom.h"
#include "cdi.h"
#include "cdbios.h"

extern int32 jaguar_cpu_in_exec;
extern uint32 jaguar_mainRom_crc32;
extern char * jaguar_eeproms_path;

//#ifdef __PORT__
//void jaguar_init(const char * filename);
//#else
void jaguar_init(void);
//#endif	// #ifdef __PORT__
void jaguar_reset(void);
void jaguar_reset_handler(void);
void jaguar_done(void);
void jaguar_exec(int16 * backbuffer, bool render);
unsigned jaguar_byte_read(unsigned int offset);
unsigned jaguar_word_read(unsigned int offset);
unsigned jaguar_long_read(unsigned int offset);
void jaguar_byte_write(unsigned offset, unsigned data);
void jaguar_word_write(unsigned offset, unsigned data);
void jaguar_long_write(unsigned offset, unsigned data);
uint32 jaguar_interrupt_handler_is_valid(uint32 i);
void jaguar_dasm(uint32 offset, uint32 qt);

// Some handy macros to help converting native endian to big endian (jaguar native)
// & vice versa

#define SET32(r, a, v)	r[a] = ((v) & 0xFF000000) >> 24, r[a+1] = ((v) & 0x00FF0000) >> 16, \
						r[a+2] = ((v) & 0x0000FF00) >> 8, r[a+3] = (v) & 0x000000FF
#define GET32(r, a)		((r[a] << 24) | (r[a+1] << 16) | (r[a+2] << 8) | r[a+3])
#define SET16(r, a, v)	r[a] = ((v) & 0xFF00) >> 8, r[a+1] = (v) & 0xFF
#define GET16(r, a)		((r[a] << 8) | r[a+1])

//Temp debug stuff

void DumpMainMemory(void);
uint8 * GetRamPtr(void);

#endif	// #ifndef __JAGUAR_H__
