//
// SlideSwitch class
//
// by James L. Hammons
//

#ifndef __SLIDESWITCH_H__
#define __SLIDESWITCH_H__

#include <string>
#include "element.h"

// How to handle?
// Save state externally?
#warning Seems to be handled the same as PushButton, but without sanity checks. !!! FIX !!!

class SlideSwitch: public Element
{
	public:
		SlideSwitch(uint32 x, uint32 y, bool * st, std::string s1, std::string s2);
		virtual void HandleKey(SDLKey key) {}
		virtual void HandleMouseMove(uint32 x, uint32 y);
		virtual void HandleMouseButton(uint32 x, uint32 y, bool mouseDown);
		virtual void Draw(uint32 offsetX = 0, uint32 offsetY = 0);
		virtual void Notify(Element *) {}

	protected:
		bool * state;
		bool inside;
		std::string text1, text2;
};

#endif	// __SLIDESWITCH_H__
