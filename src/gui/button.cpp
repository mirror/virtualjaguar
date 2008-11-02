//
// Button class
//
// by James L. Hammons
//

#include "button.h"

//#include "font14pt.h"
#include "guimisc.h"

// Various constructors

Button::Button(uint32 x/*= 0*/, uint32 y/*= 0*/, uint32 w/*= 0*/, uint32 h/*= 0*/):
	Element(x, y, w, h), activated(false), clicked(false), inside(false),
	fgColor(0xFFFFFFFF), bgColor(0xFF00FF00), pic(NULL), elementToTell(NULL)
{
}

Button::Button(uint32 x, uint32 y, uint32 w, uint32 h, uint32 * p):
	Element(x, y, w, h), activated(false), clicked(false), inside(false),
	fgColor(0xFFFFFFFF), bgColor(0xFF00FF00), pic(p), elementToTell(NULL)
{
}

// Button::Button(uint32 x, uint32 y, uint32 * p): Element(x, y, 0, 0),

Button::Button(uint32 x, uint32 y, uint32 * p, uint32 * pH/*= NULL*/, uint32 * pD/*= NULL*/):
	Element(x, y, 0, 0), activated(false), clicked(false), inside(false),
	fgColor(0xFFFFFFFF), bgColor(0xFF00FF00), pic(p), picHover(pH), picDown(pD),
	elementToTell(NULL)
{
	if (pic)
		extents.w = pic[0], extents.h = pic[1];
}

Button::Button(uint32 x, uint32 y, uint32 w, uint32 h, std::string s):
	Element(x, y, w, h), activated(false), clicked(false), inside(false),
	fgColor(0xFFFFFFFF), bgColor(0xFF00FF00), pic(NULL), text(s), elementToTell(NULL)
{
}

Button::Button(uint32 x, uint32 y, std::string s):
	Element(x, y, 0, FONT_HEIGHT), activated(false), clicked(false), inside(false),
	fgColor(0xFFFFFFFF), bgColor(0xFF00FF00), pic(NULL), text(s), elementToTell(NULL)
{
	extents.w = s.length() * FONT_WIDTH;
}

// Implementation

void Button::HandleMouseMove(uint32 x, uint32 y)
{
	inside = Inside(x, y);
}

void Button::HandleMouseButton(uint32 x, uint32 y, bool mouseDown)
{
	if (inside)
	{
		if (mouseDown)
			clicked = true;

		if (clicked && !mouseDown)
		{
			clicked = false, activated = true;

			// Send a message that we're activated (if there's someone to tell, that is)
			if (elementToTell)
				elementToTell->Notify(this);
		}
	}
	else
		clicked = activated = false;
}

void Button::Draw(uint32 offsetX/*= 0*/, uint32 offsetY/*= 0*/)
{
	uint32 addr = (extents.x + offsetX) + ((extents.y + offsetY) * pitch);

	if (text.length() > 0)							// Simple text button
//	if (pic == NULL)
	{
		for(uint32 y=0; y<extents.h; y++)
		{
			for(uint32 x=0; x<extents.w; x++)
			{
				// Doesn't clip in y axis! !!! FIX !!!
				if (extents.x + x < pitch)
					screenBuffer[addr + x + (y * pitch)]
//					= (clicked && inside ? fgColor : (inside ? 0x43F0 : bgColor));
//43F0 -> 010000 11111 10000 -> 0100 0001 1111 1111 1000 0100 -> 41 FF 84
						= (clicked && inside ? fgColor : (inside ? 0xFF84FF41 : bgColor));
			}
		}

		DrawString(screenBuffer, extents.x + offsetX, extents.y + offsetY, false, "%s", text.c_str());
	}
	else											// Graphical button
	{
		uint32 * picToShow = pic;

		if (picHover != NULL && inside && !clicked)
			picToShow = picHover;

		if (picDown != NULL && inside && clicked)
			picToShow = picDown;

		DrawTransparentBitmapDeprecated(screenBuffer, extents.x + offsetX, extents.y + offsetY, picToShow);
	}
}

bool Button::ButtonClicked(void)
{
	return activated;
}

void Button::SetNotificationElement(Element * e)
{
	elementToTell = e;
}
