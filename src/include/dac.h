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

void DACWriteByte(uint32 offset, uint8 data, uint32 who = UNKNOWN);
void DACWriteWord(uint32 offset, uint16 data, uint32 who = UNKNOWN);
uint8 DACReadByte(uint32 offset, uint32 who = UNKNOWN);
uint16 DACReadWord(uint32 offset, uint32 who = UNKNOWN);

// Global variables

extern uint16 lrxd, rrxd;							// I2S ports (into Jaguar)

#endif	// __DAC_H__
