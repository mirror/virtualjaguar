#ifndef __JAGUAR_H__
#define __JAGUAR_H__

#include "types.h"
#include "log.h"
#include "version.h"
#include "memory.h"
//#include "../star026c/starcpu.h"
#include "m68k.h"								// Musashi! Not StarCrap! (Why are you afraid to show us the source, Neill? :-)
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

#ifdef __PORT__
void jaguar_init(const char * filename);
#else
void jaguar_init(void);
#endif	// #ifdef __PORT__
void jaguar_reset(void);
void jaguar_reset_handler(void);
void jaguar_done(void);
void jaguar_exec(int16 * backbuffer, uint8 render);
unsigned jaguar_byte_read(unsigned int offset);
unsigned jaguar_word_read(unsigned int offset);
unsigned jaguar_long_read(unsigned int offset);
void jaguar_byte_write(unsigned offset, unsigned data);
void jaguar_word_write(unsigned offset, unsigned data);
void jaguar_long_write(unsigned offset, unsigned data);
uint32 jaguar_interrupt_handler_is_valid(uint32 i);
void jaguar_dasm(uint32 offset, uint32 qt);

//Temp debug stuff

void DumpMainMemory(void);
uint8 * GetRamPtr(void);

#endif	// #ifndef __JAGUAR_H__
