//
// gamepad.h: Header file
//
// by James Hammons
// (C) 2013 Underground Software
//

#ifndef __GAMEPAD_H__
#define __GAMEPAD_H__

#define JOY_BUTTON		0x0100
#define JOY_HAT			0x0200

#define	JOY_TYPE_MASK	0xFF00
#define JOY_HATNUM_MASK	0x00F8
#define JOY_HATBUT_MASK	0x0007

#include <stdint.h>

uint8_t hatMask[8] = { 1, 2, 4, 8, 16, 32, 64, 128 };

// buttonID is the combination of the type (BUTTON, HAT) and the button #
// (0-255 for buttons, 0-31 for hats). Hats also have 0-7 for a button #
// that corresponds to a direction.

class Gamepad
{
// really should make all methods and members be static so that we can
// call this stuff without instantiating one. :-)
	public:
		Gamepad();
		~Gamepad();

		bool GetState(int joystickID, int buttonID);
		int GetButtonID(void);
		int GetJoystickID(void);
};

#endif	// __GAMEPAD_H__
