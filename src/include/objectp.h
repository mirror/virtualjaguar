#ifndef __OBJECTP_H__
#define __OBJECTP_H__

#include "types.h"

void op_init(void);
void op_reset(void);
void op_done(void);
void op_byte_write(uint32, uint8);
void op_word_write(uint32, uint16);
uint8 op_byte_read(uint32);
uint16 op_word_read(uint32);
uint32 op_get_list_pointer(void);
void op_process_list(int16 *backbuffer, int scanline, int render);
void op_set_status_register(uint32 data);
uint32 op_get_status_register(void);
void op_set_current_object(uint64 object);


#endif
