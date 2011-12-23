//
// DAC.H: Header file
//

#ifndef __DAC_H__
#define __DAC_H__

#include "types.h"

void DACInit(void);
void DACReset(void);
void DACDone(void);

// DAC memory access

void DACWriteByte(uint32 address, uint8 data, uint32 who = UNKNOWN);
void DACWriteWord(uint32 address, uint16 data, uint32 who = UNKNOWN);
uint8 DACReadByte(uint32 address, uint32 who = UNKNOWN);
uint16 DACReadWord(uint32 address, uint32 who = UNKNOWN);

#endif	// __DAC_H__
