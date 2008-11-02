//
// ListBox class
//
// by James L. Hammons
//

#ifndef __LISTBOX_H__
#define __LISTBOX_H__

#include <string>
#include <vector>
#include "button.h"
//#include "element.h"

class ListBox: public Element
//class ListBox: public Window
{
	public:
//		ListBox(uint32 x = 0, uint32 y = 0, uint32 w = 0, uint32 h = 0): Element(x, y, w, h),
		ListBox(uint32 x = 0, uint32 y = 0, uint32 w = 0, uint32 h = 0);//: Window(x, y, w, h),
//		windowPtr(0), cursor(0), limit(0), charWidth((w / 8) - 1), charHeight(h / 8),
//		elementToTell(NULL), upArrow(w - 8, 0, upArrowBox),
//		downArrow(w - 8, h - 8, downArrowBox), upArrow2(w - 8, h - 16, upArrowBox) {}
		virtual void HandleKey(SDLKey key);
		virtual void HandleMouseMove(uint32 x, uint32 y);
		virtual void HandleMouseButton(uint32 x, uint32 y, bool mouseDown);
		virtual void Draw(uint32 offsetX = 0, uint32 offsetY = 0);
		virtual void Notify(Element * e);
		void SetNotificationElement(Element * e);
		void AddItem(std::string s);
		std::string GetSelectedItem(void);

	protected:
		bool thumbClicked;
		uint32 windowPtr, cursor, limit;
		uint32 charWidth, charHeight;				// Box width/height in characters
		Element * elementToTell;
		Button upArrow, downArrow, upArrow2;
		std::vector<std::string> item;

	private:
		uint32 yRelativePoint;
};

#endif	// __LISTBOX_H__
