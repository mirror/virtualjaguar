//
// SlideSwitch class
//
// by James L. Hammons
//

#include "slideswitch.h"

#include "guimisc.h"

SlideSwitch::SlideSwitch(uint32 x, uint32 y, bool * st, std::string s1, std::string s2):
	Element(x, y, 16, 32), state(st), inside(false), text1(s1), text2(s2)
{
}

void SlideSwitch::HandleMouseMove(uint32 x, uint32 y)
{
	inside = Inside(x, y);
}

void SlideSwitch::HandleMouseButton(uint32 x, uint32 y, bool mouseDown)
{
	if (inside && mouseDown)
	{
		*state = !(*state);
	}
}

void SlideSwitch::Draw(uint32 offsetX/*= 0*/, uint32 offsetY/*= 0*/)
{
	DrawTransparentBitmapDeprecated(screenBuffer, extents.x + offsetX, extents.y + offsetY, (*state ? slideSwitchDown : slideSwitchUp));

	if (text1.length() > 0)
		DrawString(screenBuffer, extents.x + offsetX + 24, extents.y + offsetY, false, "%s", text1.c_str());

	if (text2.length() > 0)
		DrawString(screenBuffer, extents.x + offsetX + 24, extents.y + offsetY + 16, false, "%s", text2.c_str());
}
