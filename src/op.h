//
// OBJECTP.H: Object Processor header file
//

#ifndef __OBJECTP_H__
#define __OBJECTP_H__

#include "types.h"

void OPInit(void);
void OPReset(void);
void OPDone(void);

void OPProcessList(int scanline, bool render);
uint32 OPGetListPointer(void);
void OPSetStatusRegister(uint32 data);
uint32 OPGetStatusRegister(void);
void OPSetCurrentObject(uint64 object);

//uint8 OPReadByte(uint32, uint32 who = UNKNOWN);
//uint16 OPReadWord(uint32, uint32 who = UNKNOWN);
//void OPWriteByte(uint32, uint8, uint32 who = UNKNOWN);
//void OPWriteWord(uint32, uint16, uint32 who = UNKNOWN);

// Exported variables

extern uint8 objectp_running;

#endif	// __OBJECTP_H__
