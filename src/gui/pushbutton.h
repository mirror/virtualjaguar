//
// PushButton class
//
// by James L. Hammons
//

#ifndef __PUSHBUTTON_H__
#define __PUSHBUTTON_H__

#include <string>
#include "element.h"

// How to handle?
// Save state externally?
//We pass in a state variable if we want to track it externally, otherwise we use our own
//internal state var. Still need to do some kind of callback for pushbuttons that do things
//like change from fullscreen to windowed... !!! FIX !!!

class PushButton: public Element
{
	public:
		PushButton(uint32 x, uint32 y, bool * st, std::string s);
		virtual void HandleKey(SDLKey key) {}
		virtual void HandleMouseMove(uint32 x, uint32 y);
		virtual void HandleMouseButton(uint32 x, uint32 y, bool mouseDown);
		virtual void Draw(uint32 offsetX = 0, uint32 offsetY = 0);
		virtual void Notify(Element *) {}

	protected:
		bool * state;
		bool inside;
		std::string text;
		bool internalState;
};

#endif	// __PUSHBUTTON_H__
