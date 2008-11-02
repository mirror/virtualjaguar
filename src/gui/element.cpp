//
// GUI element base class
//
// by James L. Hammons
//

#include "element.h"

// Initialize class variables (Element)

uint32 * Element::screenBuffer = NULL;
uint32 Element::pitch = 0;

bool Element::Inside(uint32 x, uint32 y)
{
	return (x >= (uint32)extents.x && x < (uint32)(extents.x + extents.w)
		&& y >= (uint32)extents.y && y < (uint32)(extents.y + extents.h) ? true : false);
}
