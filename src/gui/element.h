//
// GUI element base class
//
// by James L. Hammons
//

#ifndef __ELEMENT_H__
#define __ELEMENT_H__

#include "SDL.h"
#include "types.h"

class Element
{
	public:
		Element(uint32 x = 0, uint32 y = 0, uint32 w = 0, uint32 h = 0)
			{ extents.x = x, extents.y = y, extents.w = w, extents.h = h; }
		// These are "pure" virtual functions...
		virtual void HandleKey(SDLKey key) = 0;
		virtual void HandleMouseMove(uint32 x, uint32 y) = 0;
		virtual void HandleMouseButton(uint32 x, uint32 y, bool mouseDown) = 0;
		virtual void Draw(uint32, uint32) = 0;
		virtual void Notify(Element *) = 0;
//Needed?		virtual ~Element() = 0;
//We're not allocating anything in the base class, so the answer would be NO.
		bool Inside(uint32 x, uint32 y);

		// Class method
		static void SetScreenAndPitch(uint32 * s, uint32 p) { screenBuffer = s, pitch = p; }

	protected:
		SDL_Rect extents;
		uint32 state;

		// Class variables...
		static uint32 * screenBuffer;
		static uint32 pitch;
};

#endif	// __ELEMENT_H__
