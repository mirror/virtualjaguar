//
// CDROM.H
//

#ifndef __CDROM_H__
#define __CDROM_H__

#include "jaguar.h"

void cdrom_init(void);
void cdrom_reset(void);
void cdrom_done(void);
//void cdrom_update(void);

uint8 CDROMReadByte(uint32 offset, uint32 who = UNKNOWN);
uint16 CDROMReadWord(uint32 offset, uint32 who = UNKNOWN);
void CDROMWriteByte(uint32 offset, uint8 data, uint32 who = UNKNOWN);
void CDROMWriteWord(uint32 offset, uint16 data, uint32 who = UNKNOWN);

#endif	// __CDROM_H__
