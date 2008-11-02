//
// Static image class
//
// by James L. Hammons
//

#ifndef __IMAGE_H__
#define __IMAGE_H__

#include "element.h"

class Image: public Element
{
	public:
		Image(uint32 x, uint32 y, const void * img);
		virtual void HandleKey(SDLKey key) {}
		virtual void HandleMouseMove(uint32 x, uint32 y) {}
		virtual void HandleMouseButton(uint32 x, uint32 y, bool mouseDown) {}
		virtual void Draw(uint32 offsetX = 0, uint32 offsetY = 0);
		virtual void Notify(Element *) {}

	protected:
		uint32 fgColor, bgColor;
		const void * image;
};

#endif	// __IMAGE_H__
