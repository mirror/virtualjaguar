//
// Menu class & supporting structs/classes
//
// by James L. Hammons
//

#include "menu.h"

// Local variables
static const char separator[] = "--------------------------------------------------------";

// Implementation

Menu::Menu(uint32 x/*= 0*/, uint32 y/*= 0*/, uint32 w/*= 0*/, uint32 h/*= FONT_HEIGHT*/,
	uint32 fgc/*= 0xFF7F0000*/, uint32 bgc/*= 0xFFFF3F3F*/, uint32 fgch/*= 0xFFFF3F3F*/,
	uint32 bgch/*= 0xFFFF8787*/):
	Element(x, y, w, h), activated(false), clicked(false), inside(0), insidePopup(0),
	fgColor(fgc), bgColor(bgc), fgColorHL(fgch), bgColorHL(bgch), menuChosen(-1),
	menuItemChosen(-1)
{
}

void Menu::HandleKey(SDLKey key)
{
	for(uint32 i=0; i<itemList.size(); i++)
	{
		for(uint32 j=0; j<itemList[i].item.size(); j++)
		{
			if (itemList[i].item[j].hotKey == key)
			{
				SDL_Event event;
				event.type = SDL_USEREVENT;
				event.user.code = MENU_ITEM_CHOSEN;
				event.user.data1 = (void *)itemList[i].item[j].action;
	    		SDL_PushEvent(&event);

				clicked = false, menuChosen = menuItemChosen = -1;
				break;
			}
		}
	}
}

void Menu::HandleMouseMove(uint32 x, uint32 y)
{
	inside = insidePopup = 0;

	if (Inside(x, y))
	{
		// Find out *where* we are inside the menu bar
		uint32 xpos = extents.x;

		for(uint32 i=0; i<itemList.size(); i++)
		{
			uint32 width = (itemList[i].title.length() + 2) * FONT_WIDTH;

			if (x >= xpos && x < xpos + width)
			{
				inside = i + 1;
				menuChosen = i;
				break;
			}

			xpos += width;
		}
	}

	if (!Inside(x, y) && !clicked)
	{
		menuChosen = -1;
	}

	if (itemList[menuChosen].Inside(x, y) && clicked)
	{
		insidePopup = ((y - itemList[menuChosen].extents.y) / FONT_HEIGHT) + 1;
		menuItemChosen = insidePopup - 1;
	}
}

void Menu::HandleMouseButton(uint32 x, uint32 y, bool mouseDown)
{
	if (!clicked)
	{
		if (mouseDown)
		{
			if (inside)
				clicked = true;
			else
				menuChosen = -1;					// clicked is already false...!
		}
	}
	else											// clicked == true
	{
		if (insidePopup && !mouseDown)				// I.e., mouse-button-up
		{
			activated = true;
			if (itemList[menuChosen].item[menuItemChosen].action != NULL)
			{
//				itemList[menuChosen].item[menuItemChosen].action();
				SDL_Event event;
				event.type = SDL_USEREVENT;
				event.user.code = MENU_ITEM_CHOSEN;
				event.user.data1 = (void *)itemList[menuChosen].item[menuItemChosen].action;
			    SDL_PushEvent(&event);

				clicked = false, menuChosen = menuItemChosen = -1;

/*				SDL_Event event;
				while (SDL_PollEvent(&event));		// Flush the event queue...
				event.type = SDL_MOUSEMOTION;
				int mx, my;
				SDL_GetMouseState(&mx, &my);
				event.motion.x = mx, event.motion.y = my;
			    SDL_PushEvent(&event);				// & update mouse position...!
*/			}
		}

		if (!inside && !insidePopup && mouseDown)
			clicked = false, menuChosen = menuItemChosen = -1;
	}
}

void Menu::Draw(uint32 offsetX/*= 0*/, uint32 offsetY/*= 0*/)
{
	uint32 xpos = extents.x + offsetX;

	for(uint32 i=0; i<itemList.size(); i++)
	{
//		uint16 color1 = fgColor, color2 = bgColor;
		uint32 color1 = fgColor, color2 = bgColor;
		if (inside == (i + 1) || (menuChosen != -1 && (uint32)menuChosen == i))
			color1 = fgColorHL, color2 = bgColorHL;

		DrawStringOpaque(screenBuffer, xpos, extents.y + offsetY, color1, color2,
			" %s ", itemList[i].title.c_str());
		xpos += (itemList[i].title.length() + 2) * FONT_WIDTH;
	}

	// Draw sub menu (but only if active)
	if (clicked)
	{
		uint32 ypos = extents.y + FONT_HEIGHT + 1;

		for(uint32 i=0; i<itemList[menuChosen].item.size(); i++)
		{
//			uint16 color1 = fgColor, color2 = bgColor;
			uint32 color1 = fgColor, color2 = bgColor;

			if (insidePopup == i + 1)
				color1 = fgColorHL, color2 = bgColorHL, menuItemChosen = i;

			if (itemList[menuChosen].item[i].name.length() > 0)
				DrawStringOpaque(screenBuffer, itemList[menuChosen].extents.x, ypos,
					color1, color2, " %-*.*s ", itemList[menuChosen].charLength,
					itemList[menuChosen].charLength, itemList[menuChosen].item[i].name.c_str());
			else
				DrawStringOpaque(screenBuffer, itemList[menuChosen].extents.x, ypos,
					fgColor, bgColor, "%.*s", itemList[menuChosen].charLength + 2, separator);

			ypos += FONT_HEIGHT;
		}
	}
}

void Menu::Add(MenuItems mi)
{
	for(uint32 i=0; i<mi.item.size(); i++)
		if (mi.item[i].name.length() > mi.charLength)
			mi.charLength = mi.item[i].name.length();

	// Set extents here as well...
	mi.extents.x = extents.x + extents.w, mi.extents.y = extents.y + FONT_HEIGHT + 1;
	mi.extents.w = (mi.charLength + 2) * FONT_WIDTH, mi.extents.h = mi.item.size() * FONT_HEIGHT;

	itemList.push_back(mi);
	extents.w += (mi.title.length() + 2) * FONT_WIDTH;
}
