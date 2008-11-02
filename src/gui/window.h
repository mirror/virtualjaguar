//
// Window class
//
// by James L. Hammons

#ifndef __WINDOW_H__
#define __WINDOW_H__

#include <vector>
#include "button.h"
#include "element.h"

class Window: public Element
{
	public:
		Window(uint32 x = 0, uint32 y = 0, uint32 w = 0, uint32 h = 0,
			void (* f)(Element *) = NULL);
		virtual ~Window();
		virtual void HandleKey(SDLKey key);
		virtual void HandleMouseMove(uint32 x, uint32 y);
		virtual void HandleMouseButton(uint32 x, uint32 y, bool mouseDown);
		virtual void Draw(uint32 offsetX = 0, uint32 offsetY = 0);
		virtual void Notify(Element * e);
		void AddElement(Element * e);

	protected:
		uint32 fgColor, bgColor;
		void (* handler)(Element *);
		Button * close;
		// We have to use a list of Element *pointers* because we can't make a
		// list that will hold all the different object types in the same list...
		std::vector<Element *> list;
};

#endif	// __WINDOW_H__
