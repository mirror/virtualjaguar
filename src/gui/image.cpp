//
// Static image class
//
// by James L. Hammons
//

#include "image.h"

#include "guimisc.h"

Image::Image(uint32 x, uint32 y, const void * img):
	Element(x, y, 0, 0), image(img)
{
}

void Image::Draw(uint32 offsetX/*= 0*/, uint32 offsetY/*= 0*/)
{
	if (image != NULL)
		DrawTransparentBitmap(screenBuffer, extents.x + offsetX, extents.y + offsetY, image);
}
