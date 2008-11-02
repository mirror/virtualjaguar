//
// Menu class & supporting structs/classes
//
// by James L. Hammons
//

#ifndef __MENU_H__
#define __MENU_H__

#include <string>
#include <vector>
#include "element.h"
#include "guimisc.h"							// Ick.

class Window;

struct NameAction
{
	std::string name;
	Window * (* action)(void);
	SDLKey hotKey;

	NameAction(std::string n, Window * (* a)(void) = NULL, SDLKey k = SDLK_UNKNOWN):
		name(n), action(a), hotKey(k) {}
};

class MenuItems
{
	public:
		MenuItems(): charLength(0) {}
		// Normally, we avoid implementation in a header file but in this case
		// we can make an exception. ;-)
		bool Inside(uint32 x, uint32 y)
		{ return (x >= (uint32)extents.x && x < (uint32)(extents.x + extents.w)
		&& y >= (uint32)extents.y && y < (uint32)(extents.y + extents.h) ? true : false); }

		std::string title;
		std::vector<NameAction> item;
		uint32 charLength;
		SDL_Rect extents;
};

class Menu: public Element
{
	public:
// 1CFF -> 0 001 11 00  111 1 1111
// 421F -> 0 100 00 10  000 1 1111
		Menu(uint32 x = 0, uint32 y = 0, uint32 w = 0, uint32 h = FONT_HEIGHT,
			uint32 fgc = 0xFF7F0000, uint32 bgc = 0xFFFF3F3F, uint32 fgch = 0xFFFF3F3F,
			uint32 bgch = 0xFFFF8787);
		virtual void HandleKey(SDLKey key);
		virtual void HandleMouseMove(uint32 x, uint32 y);
		virtual void HandleMouseButton(uint32 x, uint32 y, bool mouseDown);
		virtual void Draw(uint32 offsetX = 0, uint32 offsetY = 0);
		virtual void Notify(Element *) {}
		void Add(MenuItems mi);

	protected:
		bool activated, clicked;
		uint32 inside, insidePopup;
		uint32 fgColor, bgColor, fgColorHL, bgColorHL;
		int menuChosen, menuItemChosen;

	private:
		std::vector<MenuItems> itemList;
};

#endif	// __MENU_H__
