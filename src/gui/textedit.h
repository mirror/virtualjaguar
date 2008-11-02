//
// TextEdit class
//
// by James L. Hammons
//

#ifndef __TEXTEDIT_H__
#define __TEXTEDIT_H__

#include <string>
#include "element.h"

class TextEdit: public Element
{
	public:
		TextEdit(uint32 x = 0, uint32 y = 0, uint32 w = 0, uint32 h = 0);
		TextEdit(uint32 x, uint32 y, std::string s, uint32 mss = 10, uint32 fg = 0xFF8484FF,
			uint32 bg = 0xFF84FF4D);
		virtual void HandleKey(SDLKey key);
		virtual void HandleMouseMove(uint32 x, uint32 y) {}
		virtual void HandleMouseButton(uint32 x, uint32 y, bool mouseDown) {}
		virtual void Draw(uint32 offsetX = 0, uint32 offsetY = 0);
		virtual void Notify(Element *) {}

	protected:
		uint32 fgColor, bgColor;
		std::string text;
		uint32 caretPos;
		uint32 maxScreenSize;
};

#endif	// __TEXTEDIT_H__
