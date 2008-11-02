//
// PushButton class
//
// by James L. Hammons
//

#include "pushbutton.h"

#include "guimisc.h"

PushButton::PushButton(uint32 x, uint32 y, bool * st, std::string s):
	Element(x, y, 16, 16), state(st), inside(false), text(s)
{
	if (st == NULL)
		state = &internalState;
}

void PushButton::HandleMouseMove(uint32 x, uint32 y)
{
	inside = Inside(x, y);
}

void PushButton::HandleMouseButton(uint32 x, uint32 y, bool mouseDown)
{
	if (inside && mouseDown)
		*state = !(*state);
}

void PushButton::Draw(uint32 offsetX/*= 0*/, uint32 offsetY/*= 0*/)
{
	if (*state)
		DrawTransparentBitmap(screenBuffer, extents.x + offsetX, extents.y + offsetY, &pbDown);
	else
		DrawTransparentBitmap(screenBuffer, extents.x + offsetX, extents.y + offsetY, &pbUp);

	if (text.length() > 0)
		DrawString(screenBuffer, extents.x + offsetX + 24, extents.y + offsetY, false, "%s", text.c_str());
}
