//
// OBJECTP.H: Object Processor header file
//

#ifndef __OBJECTP_H__
#define __OBJECTP_H__

#include "types.h"

void op_init(void);
void op_reset(void);
void op_done(void);

void OPProcessList(int scanline, bool render);
uint32 op_get_list_pointer(void);
void op_set_status_register(uint32 data);
uint32 op_get_status_register(void);
void op_set_current_object(uint64 object);

uint8 OPReadByte(uint32, uint32 who = UNKNOWN);
uint16 OPReadWord(uint32, uint32 who = UNKNOWN);
void OPWriteByte(uint32, uint8, uint32 who = UNKNOWN);
void OPWriteWord(uint32, uint16, uint32 who = UNKNOWN);

#endif	// __OBJECTP_H__
