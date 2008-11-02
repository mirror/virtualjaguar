//
// Static text class
//
// by James L. Hammons
//

#include "text.h"

#include "guimisc.h"

Text::Text(uint32 x/*= 0*/, uint32 y/*= 0*/, uint32 w/*= 0*/, uint32 h/*= 0*/):
	Element(x, y, w, h), fgColor(0xFF8484FF), bgColor(0xFF84FF4D)
{
}

Text::Text(uint32 x, uint32 y, std::string s, uint32 fg/*= 0xFF8484FF*/, uint32 bg/*= 0xFF84FF4D*/):
	Element(x, y, 0, 0), fgColor(fg), bgColor(bg), text(s)
{
}

void Text::Draw(uint32 offsetX/*= 0*/, uint32 offsetY/*= 0*/)
{
	if (text.length() > 0)
//		DrawString(screenBuffer, extents.x + offsetX, extents.y + offsetY, false, "%s", text.c_str());
		DrawStringOpaque(screenBuffer, extents.x + offsetX, extents.y + offsetY, fgColor, bgColor, "%s", text.c_str());
}
