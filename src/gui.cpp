//
// GUI.CPP
//
// Graphical User Interface support
// by James L. Hammons
//

#include <dirent.h>
#include <SDL.h>
#include <string>
#include <vector>
#include <algorithm>
#include "types.h"
#include "settings.h"
#include "tom.h"
#include "video.h"
#include "font1.h"
#include "gui.h"

using namespace std;								// For STL stuff

// Private function prototypes

void DrawTransparentBitmap(int16 * screen, uint32 x, uint32 y, uint16 * bitmap);
void DrawStringTrans(int16 * screen, uint32 x, uint32 y, uint16 color, uint8 opacity, const char * text, ...);
void DrawStringOpaque(int16 * screen, uint32 x, uint32 y, uint16 color1, uint16 color2, const char * text, ...);
void LoadROM(void);
void RunEmu(void);
void Quit(void);
void About(void);

// Local global variables

int mouseX, mouseY;

uint16 mousePic[] = {
	6, 8,

	0x03E0,0x0000,0x0000,0x0000,0x0000,0x0000,		// +
	0x0300,0x03E0,0x0000,0x0000,0x0000,0x0000,		// @+
	0x0300,0x03E0,0x03E0,0x0000,0x0000,0x0000,		// @++
	0x0300,0x0300,0x03E0,0x03E0,0x0000,0x0000,		// @@++
	0x0300,0x0300,0x03E0,0x03E0,0x03E0,0x0000,		// @@+++
	0x0300,0x0300,0x0300,0x03E0,0x03E0,0x03E0,		// @@@+++
	0x0300,0x0300,0x0300,0x0000,0x0000,0x0000,		// @@@
	0x0300,0x0000,0x0000,0x0000,0x0000,0x0000		// @
/*
	0xFFFF,0x0000,0x0000,0x0000,0x0000,0x0000,		// +
	0xE318,0xFFFF,0x0000,0x0000,0x0000,0x0000,		// @+
	0xE318,0xFFFF,0xFFFF,0x0000,0x0000,0x0000,		// @++
	0xE318,0xE318,0xFFFF,0xFFFF,0x0000,0x0000,		// @@++
	0xE318,0xE318,0xFFFF,0xFFFF,0xFFFF,0x0000,		// @@+++
	0xE318,0xE318,0xE318,0xFFFF,0xFFFF,0xFFFF,		// @@@+++
	0xE318,0xE318,0xE318,0x0000,0x0000,0x0000,		// @@@
	0xE318,0x0000,0x0000,0x0000,0x0000,0x0000		// @
*/
};
// 1 111 00 11 100 1 1100 -> F39C
// 1 100 00 10 000 1 0000 -> C210
// 1 110 00 11 000 1 1000 -> E318
// 0 000 00 11 111 0 0000 -> 03E0
// 0 000 00 11 000 0 0000 -> 0300

uint16 closeBox[] = {
	7, 7,

	0x0000,0x4B5E,0x4B5E,0x4B5E,0x4B5E,0x4B5E,0x0000,		//  +++++
	0x4B5E,0xFFFF,0x0000,0x0000,0x0000,0xFFFF,0x0217,		// +@   @.
	0x4B5E,0x0000,0xFFFF,0x0000,0xFFFF,0x0000,0x0217,		// + @ @ .
	0x4B5E,0x0000,0x0000,0xFFFF,0x0000,0x0000,0x0217,		// +  @  .
	0x4B5E,0x0000,0xFFFF,0x0000,0xFFFF,0x0000,0x0217,		// + @ @ .
	0x4B5E,0xFFFF,0x0000,0x0000,0x0000,0xFFFF,0x0217,		// +@   @.
	0x0000,0x0217,0x0217,0x0217,0x0217,0x0217,0x0000		//  .....
};

char separator[] = "--------------------------------------------------------";

//
// Local GUI classes
//

class Element
{
	public:
		Element(uint32 x = 0, uint32 y = 0, uint32 w = 0, uint32 h = 0)
			{ extents.x = x, extents.y = y, extents.w = w, extents.h = h; }
		virtual void HandleKey(SDLKey key) = 0;
		virtual void HandleMouseMove(uint32 x, uint32 y) = 0;
		virtual void HandleMouseButton(uint32 x, uint32 y, bool mouseDown) = 0;
		virtual void Draw(uint32, uint32) = 0;
//Needed?		virtual ~Element() = 0;
//We're not allocating anything in the base class, so the answer would be NO.
		bool Inside(uint32 x, uint32 y);
		// Class method
		static void SetScreenAndPitch(int16 * s, uint32 p) { screenBuffer = s, pitch = p; }

	protected:
		SDL_Rect extents;
		uint32 state;
		// Class variables...
		static int16 * screenBuffer;
		static uint32 pitch;
};

int16 * Element::screenBuffer = NULL;
uint32 Element::pitch = 0;

bool Element::Inside(uint32 x, uint32 y)
{
	return (x >= (uint32)extents.x && x < (uint32)(extents.x + extents.w)
		&& y >= (uint32)extents.y && y < (uint32)(extents.y + extents.h) ? true : false);
}

class Button: public Element
{
	public:
		Button(uint32 x = 0, uint32 y = 0, uint32 w = 0, uint32 h = 0): Element(x, y, w, h),
			activated(false), clicked(false), inside(false), fgColor(0xFFFF),
			bgColor(0x03E0), pic(NULL) {}
		Button(uint32 x, uint32 y, uint32 w, uint32 h, uint16 * p): Element(x, y, w, h),
			activated(false), clicked(false), inside(false), fgColor(0xFFFF),
			bgColor(0x03E0), pic(p) {}
		Button(uint32 x, uint32 y, uint32 w, uint32 h, string s): Element(x, y, w, h),
			activated(false), clicked(false), inside(false), fgColor(0xFFFF),
			bgColor(0x03E0), pic(NULL), text(s) {}
		virtual void HandleKey(SDLKey key) {}
		virtual void HandleMouseMove(uint32 x, uint32 y);
		virtual void HandleMouseButton(uint32 x, uint32 y, bool mouseDown);
		virtual void Draw(uint32 offsetX = 0, uint32 offsetY = 0);
		bool ButtonClicked(void) { return activated; }

	protected:
		bool activated, clicked, inside;
		uint16 fgColor, bgColor;
		uint16 * pic;
		string text;
};

void Button::HandleMouseMove(uint32 x, uint32 y)
{
	inside = Inside(x, y);
}

void Button::HandleMouseButton(uint32 x, uint32 y, bool mouseDown)
{
	if (inside)
	{
		if (mouseDown)
			clicked = true;

		if (clicked && !mouseDown)
			clicked = false, activated = true;
	}
	else
		clicked = activated = false;
}

void Button::Draw(uint32 offsetX/*= 0*/, uint32 offsetY/*= 0*/)
{
	uint32 addr = (extents.x + offsetX) + ((extents.y + offsetY) * pitch);

	for(uint32 y=0; y<extents.h; y++)
	{
		for(uint32 x=0; x<extents.w; x++)
		{
			// Doesn't clip in y axis! !!! FIX !!!
			if (extents.x + x < pitch)
				screenBuffer[addr + x + (y * pitch)] 
					= (clicked && inside ? fgColor : (inside ? 0x43F0 : bgColor));
		}
	}

//WriteLog("Button::Draw [%08X]\n", this);
	if (pic != NULL)
//{
//WriteLog("--> Button: About to draw pic [%08X].\n", pic);
		DrawTransparentBitmap(screenBuffer, extents.x + offsetX, extents.y + offsetY, pic);
//}

	if (text.length() > 0)
//{
//WriteLog("--> Button: About to draw string [%s].\n", text.c_str());
		DrawString(screenBuffer, extents.x + offsetX, extents.y + offsetY, false, "%s", text.c_str());
//}
}

class Window: public Element
{
	public:
		Window(uint32 x = 0, uint32 y = 0, uint32 w = 0, uint32 h = 0):	Element(x, y, w, h),
			/*clicked(false), inside(false),*/ fgColor(0x4FF0), bgColor(0xFE10),
			close(w - 8, 1, 7, 7, closeBox) { list.push_back(&close); }
		virtual void HandleKey(SDLKey key) {}
		virtual void HandleMouseMove(uint32 x, uint32 y);
		virtual void HandleMouseButton(uint32 x, uint32 y, bool mouseDown);
		virtual void Draw(uint32 offsetX = 0, uint32 offsetY = 0);
		void AddElement(Element * e);
		bool WindowActive(void) { return !close.ButtonClicked(); }

	protected:
//		bool clicked, inside;
		uint16 fgColor, bgColor;
		Button close;
//We have to use a list of Element *pointers* because we can't make a list that will hold
//all the different object types in the same list...
		vector<Element *> list;
};

void Window::HandleMouseMove(uint32 x, uint32 y)
{
	// Handle the items this window contains...
	for(uint32 i=0; i<list.size(); i++)
		// Make coords relative to upper right corner of this window...
		list[i]->HandleMouseMove(x - extents.x, y - extents.y);
}

void Window::HandleMouseButton(uint32 x, uint32 y, bool mouseDown)
{
	// Handle the items this window contains...
	for(uint32 i=0; i<list.size(); i++)
		// Make coords relative to upper right corner of this window...
		list[i]->HandleMouseButton(x - extents.x, y - extents.y, mouseDown);
}

void Window::Draw(uint32 offsetX/*= 0*/, uint32 offsetY/*= 0*/)
{
	uint32 addr = (extents.x + offsetX) + ((extents.y + offsetY) * pitch);

	for(uint32 y=0; y<extents.h; y++)
	{
		for(uint32 x=0; x<extents.w; x++)
		{
			// Doesn't clip in y axis! !!! FIX !!!
			if (extents.x + x < pitch)
				screenBuffer[addr + x + (y * pitch)] = bgColor;
		}
	}

	// Handle the items this window contains...
	for(uint32 i=0; i<list.size(); i++)
		list[i]->Draw(extents.x, extents.y);
}

void Window::AddElement(Element * e)
{
	list.push_back(e);
}

class Text: public Element
{
	public:
		Text(uint32 x = 0, uint32 y = 0, uint32 w = 0, uint32 h = 0): Element(x, y, w, h),
			fgColor(0x4FF0), bgColor(0xFE10) {}
		Text(uint32 x, uint32 y, string s): Element(x, y, 0, 0),
			fgColor(0x4FF0), bgColor(0xFE10), text(s) {}
		virtual void HandleKey(SDLKey key) {}
		virtual void HandleMouseMove(uint32 x, uint32 y) {}
		virtual void HandleMouseButton(uint32 x, uint32 y, bool mouseDown) {}
		virtual void Draw(uint32 offsetX = 0, uint32 offsetY = 0);

	protected:
		uint16 fgColor, bgColor;
		string text;
};

void Text::Draw(uint32 offsetX/*= 0*/, uint32 offsetY/*= 0*/)
{
	if (text.length() > 0)
		DrawString(screenBuffer, extents.x + offsetX, extents.y + offsetY, false, "%s", text.c_str());
}

struct NameAction
{
	string name;
	void (* action)(void);
	bool isWindow;

	NameAction(string n, void (* a)(void) = NULL, bool w = false): name(n), action(a),
		isWindow(w) {}
};

class MenuItems
{
	public:
		MenuItems(): charLength(0) {}
		bool Inside(uint32 x, uint32 y)
		{ return (x >= (uint32)extents.x && x < (uint32)(extents.x + extents.w)
		&& y >= (uint32)extents.y && y < (uint32)(extents.y + extents.h) ? true : false); }

		string title;
		vector<NameAction> item;
		uint32 charLength;
		SDL_Rect extents;
};

class Menu: public Element
{
	public:
		Menu(uint32 x = 0, uint32 y = 0, uint32 w = 0, uint32 h = 8,
			uint16 fgc = 0x1CFF, uint16 bgc = 0x000F, uint16 fgch = 0x421F,
			uint16 bgch = 0x1CFF): Element(x, y, w, h), clicked(false), inside(0),
			insidePopup(0), fgColor(fgc), bgColor(bgc), fgColorHL(fgch), bgColorHL(bgch),
			menuChosen(-1), menuItemChosen(-1) {}
//			{ extents.x = x, extents.y = y, extents.w = w, extents.h = h; }
		virtual void HandleKey(SDLKey key);
		virtual void HandleMouseMove(uint32 x, uint32 y);
		virtual void HandleMouseButton(uint32 x, uint32 y, bool mouseDown);
		virtual void Draw(uint32 offsetX = 0, uint32 offsetY = 0);
		void Add(MenuItems mi);
		//This is wrong. !!! FIX !!!
		bool ItemChosen(void) { return (clicked && insidePopup); }
		//This is bad... !!! FIX !!!
		NameAction & GetItem(void)
//		 { if (ItemChosen()) return itemList[menuChosen].item[menuItemChosen]; return NULL; }
			{ return itemList[menuChosen].item[menuItemChosen]; }

	protected:
		bool clicked;
		uint32 inside, insidePopup;
		uint16 fgColor, bgColor, fgColorHL, bgColorHL;
		int menuChosen, menuItemChosen;

	private:
		vector<MenuItems> itemList;
};

void Menu::HandleKey(SDLKey Key)
{
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
			uint32 width = (itemList[i].title.length() + 2) * 8;

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
		insidePopup = ((y - itemList[menuChosen].extents.y) / 8) + 1;
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
			if (itemList[menuChosen].item[menuItemChosen].action != NULL)
			{
				itemList[menuChosen].item[menuItemChosen].action();

				clicked = false, menuChosen = menuItemChosen = -1;

				SDL_Event event;
				while (SDL_PollEvent(&event));		// Flush the event queue...
				event.type = SDL_MOUSEMOTION;
				int mx, my;
				SDL_GetMouseState(&mx, &my);
				event.motion.x = mx, event.motion.y = my;
			    SDL_PushEvent(&event);				// & update mouse position...!
			}
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
		uint16 color1 = fgColor, color2 = bgColor;
		if (inside == (i + 1) || (menuChosen != -1 && (uint32)menuChosen == i))
			color1 = fgColorHL, color2 = bgColorHL;

		DrawStringOpaque(screenBuffer, xpos, extents.y + offsetY, color1, color2,
			" %s ", itemList[i].title.c_str());
		xpos += (itemList[i].title.length() + 2) * 8;
	}

	// Draw sub menu (but only if active)
	if (clicked)
	{
		uint32 ypos = extents.y + 9;

		for(uint32 i=0; i<itemList[menuChosen].item.size(); i++)
		{
			uint16 color1 = fgColor, color2 = bgColor;

			if (insidePopup == i + 1)
				color1 = fgColorHL, color2 = bgColorHL, menuItemChosen = i;

			if (itemList[menuChosen].item[i].name.length() > 0)
				DrawStringOpaque(screenBuffer, itemList[menuChosen].extents.x, ypos,
					color1, color2, " %-*.*s ", itemList[menuChosen].charLength,
					itemList[menuChosen].charLength, itemList[menuChosen].item[i].name.c_str());
			else
				DrawStringOpaque(screenBuffer, itemList[menuChosen].extents.x, ypos,
					fgColor, bgColor, "%.*s", itemList[menuChosen].charLength + 2, separator);

			ypos += 8;
		}
	}
}

void Menu::Add(MenuItems mi)
{
	for(uint32 i=0; i<mi.item.size(); i++)
		if (mi.item[i].name.length() > mi.charLength)
			mi.charLength = mi.item[i].name.length();

	// Set extents here as well...
	mi.extents.x = extents.x + extents.w, mi.extents.y = extents.y + 9;
	mi.extents.w = (mi.charLength + 2) * 8, mi.extents.h = mi.item.size() * 8;

	itemList.push_back(mi);
	extents.w += (mi.title.length() + 2) * 8;
}

//Do we even *need* this?
class RootWindow: public Window
{
	public:
		RootWindow(Menu * m, Window * w = NULL): menu(m), window(w) {}
//Do we even need to care about this crap?
//			{ extents.x = extents.y = 0, extents.w = 320, extents.h = 240; }
		virtual void HandleKey(SDLKey key) {}
		virtual void HandleMouseMove(uint32 x, uint32 y) {}
		virtual void HandleMouseButton(uint32 x, uint32 y, bool mouseDown) {}
		virtual void Draw(uint32 offsetX = 0, uint32 offsetY = 0) {}

	private:
		Menu * menu;
		Window * window;
		int16 * rootImage[1280 * 240 * 2];
};



//
// GUI stuff--it's not crunchy, it's GUI! ;-)
//

void InitGUI(void)
{
	SDL_ShowCursor(SDL_DISABLE);
	SDL_GetMouseState(&mouseX, &mouseY);
}

void GUIDone(void)
{
}

//
// Draw text at the given x/y coordinates. Can invert text as well.
//
void DrawString(int16 * screen, uint32 x, uint32 y, bool invert, const char * text, ...)
{
	char string[4096];
	va_list arg;

	va_start(arg, text);
	vsprintf(string, text, arg);
	va_end(arg);

	uint32 pitch = GetSDLScreenPitch() / 2;			// Returns pitch in bytes but we need words...
	uint32 length = strlen(string), address = x + (y * pitch);

	for(uint32 i=0; i<length; i++)
	{
		uint32 fontAddr = (uint32)string[i] * 64;

		for(uint32 yy=0; yy<8; yy++)
		{
			for(uint32 xx=0; xx<8; xx++)
			{
				if ((font1[fontAddr] && !invert) || (!font1[fontAddr] && invert))
					*(screen + address + xx + (yy * pitch)) = 0xFE00;
				fontAddr++;
			}
		}

		address += 8;
	}
}

//
// Draw text at the given x/y coordinates, using FG/BG colors.
//
void DrawStringOpaque(int16 * screen, uint32 x, uint32 y, uint16 color1, uint16 color2, const char * text, ...)
{
	char string[4096];
	va_list arg;

	va_start(arg, text);
	vsprintf(string, text, arg);
	va_end(arg);

	uint32 pitch = GetSDLScreenPitch() / 2;			// Returns pitch in bytes but we need words...
	uint32 length = strlen(string), address = x + (y * pitch);

	for(uint32 i=0; i<length; i++)
	{
		uint32 fontAddr = (uint32)string[i] * 64;

		for(uint32 yy=0; yy<8; yy++)
		{
			for(uint32 xx=0; xx<8; xx++)
			{
				*(screen + address + xx + (yy * pitch)) = (font1[fontAddr] ? color1 : color2);
				fontAddr++;
			}
		}

		address += 8;
	}
}

//
// Draw text at the given x/y coordinates. Can invert text as well.
//
void DrawStringTrans(int16 * screen, uint32 x, uint32 y, uint16 color, uint8 trans, const char * text, ...)
{
	char string[4096];
	va_list arg;

	va_start(arg, text);
	vsprintf(string, text, arg);
	va_end(arg);

	uint32 pitch = GetSDLScreenPitch() / 2;			// Returns pitch in bytes but we need words...
	uint32 length = strlen(string), address = x + (y * pitch);

	for(uint32 i=0; i<length; i++)
	{
		uint32 fontAddr = (uint32)string[i] * 64;

		for(uint32 yy=0; yy<8; yy++)
		{
			for(uint32 xx=0; xx<8; xx++)
			{
				if (font1[fontAddr])
				{
					uint16 existingColor = *(screen + address + xx + (yy * pitch));

					uint8 eRed = (existingColor >> 10) & 0x1F,
						eGreen = (existingColor >> 5) & 0x1F,
						eBlue = existingColor & 0x1F,
//This could be done ahead of time, instead of on each pixel...
						nRed = (color >> 10) & 0x1F,
						nGreen = (color >> 5) & 0x1F,
						nBlue = color & 0x1F;

//This could be sped up by using a table of 5 + 5 + 5 bits (32 levels transparency -> 32768 entries)
//Here we've modified it to have 33 levels of transparency (could have any # we want!)
//because dividing by 32 is faster than dividing by 31!
					uint8 invTrans = 32 - trans;
					uint16 bRed = (eRed * trans + nRed * invTrans) / 32;
					uint16 bGreen = (eGreen * trans + nGreen * invTrans) / 32;
					uint16 bBlue = (eBlue * trans + nBlue * invTrans) / 32;

					uint16 blendedColor = (bRed << 10) | (bGreen << 5) | bBlue;

					*(screen + address + xx + (yy * pitch)) = blendedColor;
				}

				fontAddr++;
			}
		}

		address += 8;
	}
}

//
// GUI Main loop
//
bool GUIMain(void)
{
	extern int16 * backbuffer;
	bool done = false;
	SDL_Event event;

	// Set up the GUI classes...
	Element::SetScreenAndPitch(backbuffer, GetSDLScreenPitch() / 2);

	Button closeButton(45, 90, 16, 16);
	Window someWindow(15, 16, 60, 60);
	Button button1(50, 15, 9, 9), button2(10, 10, 8, 8), button3(25, 48, 32, 8, " Ok ");
	someWindow.AddElement(&button1);
	someWindow.AddElement(&button2);
	someWindow.AddElement(&button3);

	Menu mainMenu;//(0, 160);
	MenuItems mi;
	mi.title = "File";
	mi.item.push_back(NameAction("Load...", LoadROM));
	mi.item.push_back(NameAction("Reset"));
	mi.item.push_back(NameAction("Run", RunEmu));
	mi.item.push_back(NameAction(""));
	mi.item.push_back(NameAction("Quit", Quit));
	mainMenu.Add(mi);
	mi.title = "Settings";
	mi.item.clear();
	mi.item.push_back(NameAction("Video..."));
	mi.item.push_back(NameAction("Audio..."));
	mi.item.push_back(NameAction("Misc..."));
	mainMenu.Add(mi);
	mi.title = "Options";
	mi.item.clear();
	mi.item.push_back(NameAction("About..."));
	mainMenu.Add(mi);

	bool showMouse = true;

//This is crappy!!! !!! FIX !!!
	jaguar_reset();

	while (!done)
	{
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_ACTIVEEVENT)
			{
				if (event.active.state == SDL_APPMOUSEFOCUS)
					showMouse = (event.active.gain ? true : false);
			}
			if (event.type == SDL_KEYDOWN)
			{
				closeButton.HandleKey(event.key.keysym.sym);
				if (someWindow.WindowActive())
					someWindow.HandleKey(event.key.keysym.sym);
				mainMenu.HandleKey(event.key.keysym.sym);
			}
			else if (event.type == SDL_MOUSEMOTION)
			{
				mouseX = event.motion.x, mouseY = event.motion.y;

				if (vjs.useOpenGL)
					mouseX /= 2, mouseY /= 2;

				closeButton.HandleMouseMove(mouseX, mouseY);
				if (someWindow.WindowActive())
					someWindow.HandleMouseMove(mouseX, mouseY);
				mainMenu.HandleMouseMove(mouseX, mouseY);
			}
			else if (event.type == SDL_MOUSEBUTTONDOWN)
			{
				uint32 mx = event.button.x, my = event.button.y;

				if (vjs.useOpenGL)
					mx /= 2, my /= 2;

				closeButton.HandleMouseButton(mx, my, true);
				if (someWindow.WindowActive())
					someWindow.HandleMouseButton(mx, my, true);
				mainMenu.HandleMouseButton(mx, my, true);
			}
			else if (event.type == SDL_MOUSEBUTTONUP)
			{
				uint32 mx = event.button.x, my = event.button.y;

				if (vjs.useOpenGL)
					mx /= 2, my /= 2;

				closeButton.HandleMouseButton(mx, my, false);
				if (someWindow.WindowActive())
					someWindow.HandleMouseButton(mx, my, false);
				mainMenu.HandleMouseButton(mx, my, false);
			}

			// Draw the GUI...
// The way we do things here is kinda stupid (redrawing the screen every frame), but
// it's simple. Perhaps there may be a reason down the road to be more selective with
// our clearing, but for now, this will suffice.
			memset(backbuffer, 0x11, tom_getVideoModeWidth() * 240 * 2);

			closeButton.Draw();
			if (someWindow.WindowActive())
				someWindow.Draw();
			mainMenu.Draw();

			if (showMouse)
				DrawTransparentBitmap(backbuffer, mouseX, mouseY, mousePic);

			RenderBackbuffer();
		}
	}

	return true;
}

//
// GUI "action" functions
//
void LoadROM(void)
{
}

void RunEmu(void)
{
//This is crappy... !!! FIX !!!
	extern int16 * backbuffer;
	extern bool finished;
	extern bool showGUI;
	uint32 nFrame = 0, nFrameskip = 0;
	uint32 totalFrames = 0;
	finished = false;
	bool showMessage = true;
	uint32 showMsgFrames = 60;
	uint8 transparency = 0;

	while (!finished)
	{
		// Set up new backbuffer with new pixels and data
		JaguarExecute(backbuffer, true);
		totalFrames++;
//WriteLog("Frame #%u...\n", totalFrames);
//extern bool doDSPDis;
//if (totalFrames == 373)
//	doDSPDis = true;

		// Some QnD GUI stuff here...
		if (showGUI)
		{
			extern uint32 gpu_pc, dsp_pc;
			DrawString(backbuffer, 8, 8, false, "GPU PC: %08X", gpu_pc);
			DrawString(backbuffer, 8, 16, false, "DSP PC: %08X", dsp_pc);
		}

		if (showMessage)
		{
			DrawStringTrans(backbuffer, 8, 24*8, 0xFF0F, transparency, "Running...");

			if (showMsgFrames == 0)
			{			
				transparency++;

				if (transparency == 33)
					showMessage = false;
			}
			else
				showMsgFrames--;
		}

		// Simple frameskip
		if (nFrame == nFrameskip)
		{
			RenderBackbuffer();
			nFrame = 0;
		}
		else
			nFrame++;

		joystick_exec();
	}
}

void Quit(void)
{
	WriteLog("GUI: Quitting due to user request.\n");
	log_done();
	exit(0);
}

void About(void)
{
	extern int16 * backbuffer;
	SDL_Event event;
	uint16 * bgSave = (uint16 *)malloc(tom_getVideoModeWidth() * 240 * 2);
	memcpy(bgSave, backbuffer, tom_getVideoModeWidth() * 240 * 2);

	bool done = false;
	while (!done)
	{
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_KEYDOWN)
			{
				if (event.key.keysym.sym == SDLK_ESCAPE || event.key.keysym.sym == SDLK_RETURN)
					done = true;
			}
			else if (event.type == SDL_MOUSEMOTION)
			{
				mouseX = event.motion.x, mouseY = event.motion.y;
				if (vjs.useOpenGL)
					mouseX /= 2, mouseY /= 2;
			}
			else if (event.type == SDL_MOUSEBUTTONDOWN)
			{
				uint32 mx = event.button.x, my = event.button.y;
				if (vjs.useOpenGL)
					mx /= 2, my /= 2;

				done = true;
			}

			// Draw "About" box
			memcpy(backbuffer, bgSave, tom_getVideoModeWidth() * 240 * 2);

			DrawStringOpaque(backbuffer, 64, 64, 0x1CFF, 0x000F, "                              ");
			DrawStringOpaque(backbuffer, 64, 72, 0x1CFF, 0x000F, " Virtual Jaguar by JLH & crew ");
			DrawStringOpaque(backbuffer, 64, 80, 0x1CFF, 0x000F, "                              ");

			DrawTransparentBitmap(backbuffer, mouseX, mouseY, mousePic);

			RenderBackbuffer();
		}
	}

	free(bgSave);
}

//
// Draw "picture"
// Uses zero as transparent color
//
void DrawTransparentBitmap(int16 * screen, uint32 x, uint32 y, uint16 * bitmap)
{
	uint16 width = bitmap[0], height = bitmap[1];
	bitmap += 2;

	uint32 pitch = GetSDLScreenPitch() / 2;			// Returns pitch in bytes but we need words...
	uint32 address = x + (y * pitch);

	for(int yy=0; yy<height; yy++)
	{
		for(int xx=0; xx<width; xx++)
		{
				if (*bitmap && x + xx < pitch)		// NOTE: Still doesn't clip the Y val...
					*(screen + address + xx + (yy * pitch)) = *bitmap;
				bitmap++;
		}
	}
}

//
// Very very crude GUI file selector
//
bool UserSelectFile(char * path, char * filename)
{
//Testing...
//GUIMain();
	
	extern int16 * backbuffer;
	vector<string> fileList;

	// Read in the candidate files from the directory pointed to by "path"

	DIR * dp = opendir(path);
	dirent * de;

	while ((de = readdir(dp)) != NULL)
	{
		char * ext = strrchr(de->d_name, '.');

		if (ext != NULL)
			if (stricmp(ext, ".zip") == 0 || stricmp(ext, ".jag") == 0)
				fileList.push_back(string(de->d_name));
	}

	closedir(dp);

	if (fileList.size() == 0)						// Any files found?
		return false;								// Nope. Bail!

	// Main GUI selection loop

	uint32 cursor = 0, startFile = 0;

	if (fileList.size() > 1)	// Only go GUI if more than one possibility!
	{
		sort(fileList.begin(), fileList.end());

		bool done = false;
		uint32 limit = (fileList.size() > 30 ? 30 : fileList.size());
		SDL_Event event;

		// Ensure that the GUI is drawn before any user input...
		event.type = SDL_USEREVENT;
		SDL_PushEvent(&event);

		while (!done)
		{
			while (SDL_PollEvent(&event))
			{
				if (event.type == SDL_KEYDOWN)
				{
					SDLKey key = event.key.keysym.sym;

					if (key == SDLK_DOWN)
					{
						if (cursor != limit - 1)	// Cursor is within its window
							cursor++;
						else						// Otherwise, scroll the window...
						{
							if (cursor + startFile != fileList.size() - 1)
								startFile++;
						}
					}
					if (key == SDLK_UP)
					{
						if (cursor != 0)
							cursor--;
						else
						{
							if (startFile != 0)
								startFile--;
						}
					}
					if (key == SDLK_PAGEDOWN)
					{
						if (cursor != limit - 1)
							cursor = limit - 1;
						else
						{
							startFile += limit;
							if (startFile > fileList.size() - limit)
								startFile = fileList.size() - limit;
						}
					}
					if (key == SDLK_PAGEUP)
					{
						if (cursor != 0)
							cursor = 0;
						else
						{
							if (startFile < limit)
								startFile = 0;
							else
								startFile -= limit;
						}
					}
					if (key == SDLK_RETURN)
						done = true;
					if (key == SDLK_ESCAPE)
					{
						WriteLog("GUI: Aborting VJ by user request.\n");
						return false;						// Bail out!
					}
					if (key >= SDLK_a && key <= SDLK_z)
					{
						// Advance cursor to filename with first letter pressed...
						uint8 which = (key - SDLK_a) + 65;	// Convert key to A-Z char

						for(uint32 i=0; i<fileList.size(); i++)
						{
							if ((fileList[i][0] & 0xDF) == which)
							{
								cursor = i - startFile;
								if (i > startFile + limit - 1)
									startFile = i - limit + 1,
									cursor = limit - 1;
								if (i < startFile)
									startFile = i,
									cursor = 0;
								break;
							}
						}
					}
				}
				else if (event.type == SDL_MOUSEMOTION)
				{
					mouseX = event.motion.x, mouseY = event.motion.y;
					if (vjs.useOpenGL)
						mouseX /= 2, mouseY /= 2;
				}
				else if (event.type == SDL_MOUSEBUTTONDOWN)
				{
					uint32 mx = event.button.x, my = event.button.y;
					if (vjs.useOpenGL)
						mx /= 2, my /= 2;
					cursor = my / 8;
				}

				// Draw the GUI...
//				memset(backbuffer, 0x11, tom_getVideoModeWidth() * tom_getVideoModeHeight() * 2);
				memset(backbuffer, 0x11, tom_getVideoModeWidth() * 240 * 2);

				for(uint32 i=0; i<limit; i++)
				{
					// Clip our strings to guarantee that they fit on the screen...
					// (and strip off the extension too)
					string s(fileList[startFile + i], 0, fileList[startFile + i].length() - 4);
					if (s.length() > 38)
						s[38] = 0;
					DrawString(backbuffer, 0, i*8, (cursor == i ? true : false), " %s ", s.c_str());
				}

				DrawTransparentBitmap(backbuffer, mouseX, mouseY, mousePic);

				RenderBackbuffer();
			}
		}
	}

	strcpy(filename, path);

	if (strlen(path) > 0)
		if (path[strlen(path) - 1] != '/')
			strcat(filename, "/");

	strcat(filename, fileList[startFile + cursor].c_str());

	return true;
}
