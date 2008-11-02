//
// ListBox class
//
// by James L. Hammons
//

#include "listbox.h"

#include "guimisc.h"

ListBox::ListBox(uint32 x, uint32 y, uint32 w, uint32 h): Element(x, y, w, h),
	thumbClicked(false), windowPtr(0), cursor(0), limit(0), charWidth((w / FONT_WIDTH) - 1),
	charHeight(h / FONT_HEIGHT), elementToTell(NULL), upArrow(w - 8, 0, upArrowBox),
	downArrow(w - 8, h - 8, downArrowBox), upArrow2(w - 8, h - 16, upArrowBox)
{
	upArrow.SetNotificationElement(this);
	downArrow.SetNotificationElement(this);
	upArrow2.SetNotificationElement(this);
	extents.w -= 8;									// Make room for scrollbar...
}

void ListBox::HandleKey(SDLKey key)
{
	if (key == SDLK_DOWN)
	{
		if (cursor != limit - 1)	// Cursor is within its window
			cursor++;
		else						// Otherwise, scroll the window...
		{
			if (cursor + windowPtr != item.size() - 1)
				windowPtr++;
		}
	}
	else if (key == SDLK_UP)
	{
		if (cursor != 0)
			cursor--;
		else
		{
			if (windowPtr != 0)
				windowPtr--;
		}
	}
	else if (key == SDLK_PAGEDOWN)
	{
		if (cursor != limit - 1)
			cursor = limit - 1;
		else
		{
			windowPtr += limit;
			if (windowPtr > item.size() - limit)
				windowPtr = item.size() - limit;
		}
	}
	else if (key == SDLK_PAGEUP)
	{
		if (cursor != 0)
			cursor = 0;
		else
		{
			if (windowPtr < limit)
				windowPtr = 0;
			else
				windowPtr -= limit;
		}
	}
	else if (key >= SDLK_a && key <= SDLK_z)
	{
		// Advance cursor to filename with first letter pressed...
		uint8 which = (key - SDLK_a) + 65;	// Convert key to A-Z char

		for(uint32 i=0; i<item.size(); i++)
		{
			if ((item[i][0] & 0xDF) == which)
			{
				cursor = i - windowPtr;
				if (i > windowPtr + limit - 1)
					windowPtr = i - limit + 1, cursor = limit - 1;
				if (i < windowPtr)
					windowPtr = i, cursor = 0;
				break;
			}
		}
	}
}

void ListBox::HandleMouseMove(uint32 x, uint32 y)
{
	upArrow.HandleMouseMove(x - extents.x, y - extents.y);
	downArrow.HandleMouseMove(x - extents.x, y - extents.y);
	upArrow2.HandleMouseMove(x - extents.x, y - extents.y);

	if (thumbClicked)
	{
		uint32 sbHeight = extents.h - 24,
			thumb = (uint32)(((float)limit / (float)item.size()) * (float)sbHeight);

//yRelativePoint is the spot on the thumb where we clicked...
		int32 newThumbStart = y - yRelativePoint;

		if (newThumbStart < 0)
			newThumbStart = 0;

		if ((uint32)newThumbStart > sbHeight - thumb)
			newThumbStart = sbHeight - thumb;

		windowPtr = (uint32)(((float)newThumbStart / (float)sbHeight) * (float)item.size());
//Check for cursor bounds as well... Or do we need to???
//Actually, we don't...!
	}
}

void ListBox::HandleMouseButton(uint32 x, uint32 y, bool mouseDown)
{
	if (Inside(x, y) && mouseDown)
	{
		// Why do we have to do this??? (- extents.y?)
		// I guess it's because only the Window class has offsetting implemented... !!! FIX !!!
//		cursor = (y - extents.y) / 8;
		cursor = (y - extents.y) / FONT_HEIGHT;
	}

	// Check for a hit on the scrollbar...
	if (x > (uint32)(extents.x + extents.w) && x <= (uint32)(extents.x + extents.w + 8)
		&& y > (uint32)(extents.y + 8) && y <= (uint32)(extents.y + extents.h - 16))
	{
		if (mouseDown)
		{
// This shiaut should be calculated in AddItem(), not here... (or in Draw() for that matter)
			uint32 sbHeight = extents.h - 24,
				thumb = (uint32)(((float)limit / (float)item.size()) * (float)sbHeight),
				thumbStart = (uint32)(((float)windowPtr / (float)item.size()) * (float)sbHeight);

			// Did we hit the thumb?
			if (y >= (extents.y + 8 + thumbStart) && y < (extents.y + 8 + thumbStart + thumb))
				thumbClicked = true, yRelativePoint = y - thumbStart;
		}
//Seems that this is useless--never reached except in rare cases and that the code outside is
//more effective...
//		else
//			thumbClicked = false;
	}

	if (!mouseDown)
		thumbClicked = false;

	upArrow.HandleMouseButton(x - extents.x, y - extents.y, mouseDown);
	downArrow.HandleMouseButton(x - extents.x, y - extents.y, mouseDown);
	upArrow2.HandleMouseButton(x - extents.x, y - extents.y, mouseDown);
}

void ListBox::Draw(uint32 offsetX/*= 0*/, uint32 offsetY/*= 0*/)
{
	for(uint32 i=0; i<limit; i++)
	{
		// Strip off the extension
		// (extension stripping should be an option, not default!)
		std::string s(item[windowPtr + i], 0, item[windowPtr + i].length() - 4);
//		DrawString(screenBuffer, extents.x + offsetX, extents.y + offsetY + i*8,
		DrawString(screenBuffer, extents.x + offsetX, extents.y + offsetY + i*FONT_HEIGHT,
			(cursor == i ? true : false), "%-*.*s", charWidth, charWidth, s.c_str());
	}

	upArrow.Draw(extents.x + offsetX, extents.y + offsetY);
	downArrow.Draw(extents.x + offsetX, extents.y + offsetY);
	upArrow2.Draw(extents.x + offsetX, extents.y + offsetY);

	uint32 sbHeight = extents.h - 24,
		thumb = (uint32)(((float)limit / (float)item.size()) * (float)sbHeight),
		thumbStart = (uint32)(((float)windowPtr / (float)item.size()) * (float)sbHeight);

	for(uint32 y=extents.y+offsetY+8; y<extents.y+offsetY+extents.h-16; y++)
	{
//		for(uint32 x=extents.x+offsetX+extents.w-8; x<extents.x+offsetX+extents.w; x++)
		for(uint32 x=extents.x+offsetX+extents.w; x<extents.x+offsetX+extents.w+8; x++)
		{
			if (y >= thumbStart + (extents.y+offsetY+8) && y < thumbStart + thumb + (extents.y+offsetY+8))
//				screenBuffer[x + (y * pitch)] = (thumbClicked ? 0x458E : 0xFFFF);
//458E -> 01 0001  0 1100  0 1110 -> 0100 0101  0110 0011  0111 0011 -> 45 63 73
				screenBuffer[x + (y * pitch)] = (thumbClicked ? 0xFF736345 : 0xFFFFFFFF);
			else
//				screenBuffer[x + (y * pitch)] = 0x0200;
//0200 -> 000000 10000 00000 -> 00 1000 0100 00
				screenBuffer[x + (y * pitch)] = 0xFF008400;
		}
	}
}

void ListBox::Notify(Element * e)
{
	if (e == &upArrow || e == &upArrow2)
	{
		if (windowPtr != 0)
		{
			windowPtr--;

			if (cursor < limit - 1)
				cursor++;
		}
	}
	else if (e == &downArrow)
	{
		if (windowPtr < item.size() - limit)
		{
			windowPtr++;

			if (cursor != 0)
				cursor--;
		}
	}
}

void ListBox::SetNotificationElement(Element * e)
{
	elementToTell = e;
}

void ListBox::AddItem(std::string s)
{
	// Do a simple insertion sort
	bool inserted = false;

	for(std::vector<std::string>::iterator i=item.begin(); i<item.end(); i++)
	{
		if (stringCmpi(s, *i) == -1)
		{
			item.insert(i, s);
			inserted = true;
			break;
		}
	}

	if (!inserted)
		item.push_back(s);

	limit = (item.size() > charHeight ? charHeight : item.size());
}

std::string ListBox::GetSelectedItem(void)
{
	return item[windowPtr + cursor];
}
