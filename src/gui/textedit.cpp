//
// TextEdit class
//
// by James L. Hammons
//

#include "textedit.h"

#include "guimisc.h"

TextEdit::TextEdit(uint32 x/*= 0*/, uint32 y/*= 0*/, uint32 w/*= 0*/, uint32 h/*= 0*/):
	Element(x, y, w, h), fgColor(0xFF8484FF), bgColor(0xFF84FF4D), text(""),
	caretPos(0), maxScreenSize(10)
{
}

TextEdit::TextEdit(uint32 x, uint32 y, std::string s, uint32 mss/*= 10*/, uint32 fg/*= 0xFF8484FF*/, uint32 bg/*= 0xFF84FF4D*/):
	Element(x, y, 0, 0), fgColor(fg), bgColor(bg), text(s), caretPos(0), maxScreenSize(mss)
{
}

//Set different filters depending on type passed in on construction, e.g., filename, amount, etc...?
void TextEdit::HandleKey(SDLKey key)
{
	if ((key >= SDLK_a && key <= SDLK_z) || (key >= SDLK_0 && key <= SDLK_9) || key == SDLK_PERIOD
		|| key == SDLK_SLASH)
	{
		//Need to handle shift key as well...
		text[caretPos++] = key;
		Draw();
	}
	else if (key == SDLK_BACKSPACE)
	{

	}
	else if (key == SDLK_DELETE)
	{
	}
//left, right arrow
}

void TextEdit::Draw(uint32 offsetX/*= 0*/, uint32 offsetY/*= 0*/)
{
	if (text.length() > 0)
	{
		FillScreenRectangle(screenBuffer, extents.x + offsetX, extents.y + offsetY, FONT_WIDTH * maxScreenSize, FONT_HEIGHT, bgColor);
//		DrawString(screenBuffer, extents.x + offsetX, extents.y + offsetY, false, "%s", text.c_str());
		DrawStringOpaque(screenBuffer, extents.x + offsetX, extents.y + offsetY, fgColor, bgColor, "%s", text.c_str());
	}

	// Draw the caret (underscore? or vertical line?)
}
