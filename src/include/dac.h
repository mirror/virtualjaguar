//
// DAC.H: Header file
//

#ifndef __DAC_H__
#define __DAC_H__

void DACInit(void);
void DACReset(void);
void DACDone(void);

// DAC memory access

void DACWriteByte(uint32 offset, uint8 data);
void DACWriteWord(uint32 offset, uint16 data);
uint8 DACReadByte(uint32 offset);
uint16 DACReadWord(uint32 offset);

#endif	// __DAC_H__
