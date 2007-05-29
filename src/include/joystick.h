#ifndef __JOYSTICK_H__
#define __JOYSTICK_H__

#include "types.h"

void joystick_init(void);
void joystick_reset(void);
void joystick_done(void);
void joystick_byte_write(uint32, uint8);
void joystick_word_write(uint32, uint16);
uint8 joystick_byte_read(uint32);
uint16 joystick_word_read(uint32);
void joystick_exec(void);

#endif
