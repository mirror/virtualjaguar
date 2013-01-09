//
// gamepad.cpp - Host joystick handling (using SDL)
//
// by James Hammons
// (C) 2013 Underground Software
//
// JLH = James Hammons <jlhamm@acm.org>
//
// Who  When        What
// ---  ----------  -------------------------------------------------------------
// JLH  01/05/2013  Created this file
//

#include "gamepad.h"


bool Gamepad::GetState(int joystickID, int buttonID)
{
	if (buttonID & JOY_BUTTON)
	{
		// Handle SDL button
	}
	else if (buttonID & JOY_HAT)
	{
		// Handle SDL hats
		int hatNumber = (buttonID & JOY_HATNUM_MASK) >> 3;
		int hatDirection = hatMask[buttonID & JOY_HATBUT_MASK];
	}

	// Default == failure
	return false;
}


int Gamepad::GetButtonID(void)
{
	// Return single button ID being pressed (if any)
}


int Gamepad::GetJoystickID(void)
{
	// Return joystick ID of button being pressed (if any)
}

