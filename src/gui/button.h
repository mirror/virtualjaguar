//
// Button class
//
// by James L. Hammons
//

#ifndef __BUTTON_H__
#define __BUTTON_H__

#include <string>
#include "element.h"

class Button: public Element
{
	public:
		Button(uint32 x = 0, uint32 y = 0, uint32 w = 0, uint32 h = 0);
		Button(uint32 x, uint32 y, uint32 w, uint32 h, uint32 * p);
		Button(uint32 x, uint32 y, uint32 * p, uint32 * pH = NULL, uint32 * pD = NULL);
		Button(uint32 x, uint32 y, uint32 w, uint32 h, std::string s);
		Button(uint32 x, uint32 y, std::string s);
		virtual void HandleKey(SDLKey key) {}
		virtual void HandleMouseMove(uint32 x, uint32 y);
		virtual void HandleMouseButton(uint32 x, uint32 y, bool mouseDown);
		virtual void Draw(uint32 offsetX = 0, uint32 offsetY = 0);
		virtual void Notify(Element *) {}
		bool ButtonClicked(void);
		void SetNotificationElement(Element * e);

	protected:
		bool activated, clicked, inside;
		uint32 fgColor, bgColor;
		uint32 * pic, * picHover, * picDown;
		std::string text;
		Element * elementToTell;
};

#endif	// __BUTTON_H__
