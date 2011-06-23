//
// Jaguar joystick handler
//

#ifndef __JOYSTICK_H__
#define __JOYSTICK_H__

#include "types.h"

void JoystickInit(void);
void JoystickReset(void);
void JoystickDone(void);
void JoystickWriteByte(uint32, uint8);
void JoystickWriteWord(uint32, uint16);
uint8 JoystickReadByte(uint32);
uint16 JoystickReadWord(uint32);
void JoystickExec(void);

extern bool keyBuffer[];

#endif
