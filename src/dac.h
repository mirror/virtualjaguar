//
// DAC.H: Header file
//

#ifndef __DAC_H__
#define __DAC_H__

#include "memory.h"

void DACInit(void);
void DACReset(void);
void DACPauseAudioThread(bool state = true);
void DACDone(void);
//int GetCalculatedFrequency(void);

// DAC memory access

void DACWriteByte(uint32_t offset, uint8_t data, uint32_t who = UNKNOWN);
void DACWriteWord(uint32_t offset, uint16_t data, uint32_t who = UNKNOWN);
uint8_t DACReadByte(uint32_t offset, uint32_t who = UNKNOWN);
uint16_t DACReadWord(uint32_t offset, uint32_t who = UNKNOWN);

#endif	// __DAC_H__
