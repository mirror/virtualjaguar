//
// Joystick handler
//
// by cal2
// GCC/SDL port by Niels Wagenaar (Linux/WIN32) and Caz (BeOS)
// Cleanups/fixes by James Hammons
// (C) 2010 Underground Software
//
// JLH = James Hammons <jlhamm@acm.org>
//
// Who  When        What
// ---  ----------  -------------------------------------------------------------
// JLH  01/16/2010  Created this log ;-)
//

#include "joystick.h"

//#include <SDL.h>
//#include <time.h>
#include <string.h>			// For memset()
#include "gpu.h"
#include "jaguar.h"
#include "log.h"
#include "settings.h"

// Global vars

static uint8_t joystick_ram[4];
uint8_t joypad_0_buttons[21];
uint8_t joypad_1_buttons[21];

bool keyBuffer[21];

//SDL_Joystick * joystick1;

bool GUIKeyHeld = false;
extern int start_logging;
int gpu_start_log = 0;
int op_start_log = 0;
int blit_start_log = 0;
int effect_start = 0;
int effect_start2 = 0, effect_start3 = 0, effect_start4 = 0, effect_start5 = 0, effect_start6 = 0;
bool interactiveMode = false;
bool iLeft, iRight, iToggle = false;
bool keyHeld1 = false, keyHeld2 = false, keyHeld3 = false;
int objectPtr = 0;
bool startMemLog = false;
extern bool doDSPDis, doGPUDis;

bool blitterSingleStep = false;
bool bssGo = false;
bool bssHeld = false;


void JoystickInit(void)
{
	JoystickReset();
}


void JoystickExec(void)
{
	gpu_start_log = 0;							// Only log while key down!
	effect_start = 0;
	effect_start2 = effect_start3 = effect_start4 = effect_start5 = effect_start6 = 0;
	blit_start_log = 0;
	iLeft = iRight = false;
}


void JoystickReset(void)
{
	memset(joystick_ram, 0x00, 4);
	memset(joypad_0_buttons, 0, 21);
	memset(joypad_1_buttons, 0, 21);
}


void JoystickDone(void)
{
}


uint8_t JoystickReadByte(uint32_t offset)
{
// For now, until we can fix the 2nd controller... :-P
//memset(joypad_1_buttons, 0, 21);

#warning "No bounds checking done in JoystickReadByte!"
//	extern bool hardwareTypeNTSC;
	offset &= 0x03;

	if (offset == 0)
	{
		uint8_t data = 0x00;
		int pad0Index = joystick_ram[1] & 0x0F;
		int pad1Index = (joystick_ram[1] >> 4) & 0x0F;

// This is bad--we're assuming that a bit is set in the last case. Might not be so!
// NOTE: values $7, B, D, & E are only legal ones for pad 0, (rows 3 to 0, in both cases)
//              $E, D, B, & 7 are only legal ones for pad 1
//       So the following code is WRONG! (now fixed! ;-)
// Also: we should explicitly check for those bit patterns, as other patterns
// are legal and yield other controllers... !!! FIX !!!
#warning "!!! Need to explicitly check for the proper bit combinations! !!!"

		if (!(pad0Index & 0x01))
			pad0Index = 0;
		else if (!(pad0Index & 0x02))
			pad0Index = 1;
		else if (!(pad0Index & 0x04))
			pad0Index = 2;
		else if (!(pad0Index & 0x08))
			pad0Index = 3;

		if (!(pad1Index & 0x01))
			pad1Index = 3;
		else if (!(pad1Index & 0x02))
			pad1Index = 2;
		else if (!(pad1Index & 0x04))
			pad1Index = 1;
		else if (!(pad1Index & 0x08))
			pad1Index = 0;

		if (joypad_0_buttons[(pad0Index << 2) + 0])	data |= 0x01;
		if (joypad_0_buttons[(pad0Index << 2) + 1]) data |= 0x02;
		if (joypad_0_buttons[(pad0Index << 2) + 2]) data |= 0x04;
		if (joypad_0_buttons[(pad0Index << 2) + 3]) data |= 0x08;
		if (joypad_1_buttons[(pad1Index << 2) + 0]) data |= 0x10;
		if (joypad_1_buttons[(pad1Index << 2) + 1]) data |= 0x20;
		if (joypad_1_buttons[(pad1Index << 2) + 2]) data |= 0x40;
		if (joypad_1_buttons[(pad1Index << 2) + 3]) data |= 0x80;

		return ~data;
	}
	else if (offset == 3)
	{
		// Hardware ID returns NTSC/PAL identification bit here
		uint8_t data = 0x2F | (vjs.hardwareTypeNTSC ? 0x10 : 0x00);
		int pad0Index = joystick_ram[1] & 0x0F;
		int pad1Index = (joystick_ram[1] >> 4) & 0x0F;

//This is more stuff to add to the button reading, as the preceeding only
//yields 16 buttons...
#warning "!!! This reports TeamTap incorrectly when PAUSE pressed on controller #1 or #2 !!!"
		if (!(pad0Index & 0x01))
		{
			if (joypad_0_buttons[BUTTON_PAUSE])
				data ^= 0x01;
			if (joypad_0_buttons[BUTTON_A])
				data ^= 0x02;
		}
		else if (!(pad0Index & 0x02))
		{
			if (joypad_0_buttons[BUTTON_B])
				data ^= 0x02;
		}
		else if (!(pad0Index & 0x04))
		{
			if (joypad_0_buttons[BUTTON_C])
				data ^= 0x02;
		}
		else if (!(pad0Index & 0x08))
		{
			if (joypad_0_buttons[BUTTON_OPTION])
				data ^= 0x02;
		}

		if (!(pad1Index & 0x08))
		{
			if (joypad_1_buttons[BUTTON_PAUSE])
				data ^= 0x04;
			if (joypad_1_buttons[BUTTON_A])
				data ^= 0x08;
		}
		else if (!(pad1Index & 0x04))
		{
			if (joypad_1_buttons[BUTTON_B])
				data ^= 0x08;
		}
		else if (!(pad1Index & 0x02))
		{
			if (joypad_1_buttons[BUTTON_C])
				data ^= 0x08;
		}
		else if (!(pad1Index & 0x01))
		{
			if (joypad_1_buttons[BUTTON_OPTION])
				data ^= 0x08;
		}

		return data;
	}

	return joystick_ram[offset];
}


uint16_t JoystickReadWord(uint32_t offset)
{
	return ((uint16_t)JoystickReadByte((offset + 0) & 0x03) << 8) | JoystickReadByte((offset + 1) & 0x03);
}


void JoystickWriteByte(uint32_t offset, uint8_t data)
{
	joystick_ram[offset & 0x03] = data;
}


void JoystickWriteWord(uint32_t offset, uint16_t data)
{
#warning "No bounds checking done for JoystickWriteWord!"
	offset &= 0x03;
	joystick_ram[offset + 0] = (data >> 8) & 0xFF;
	joystick_ram[offset + 1] = data & 0xFF;
}

