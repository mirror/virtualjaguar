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
#include "crc32.h"
#include "zlib.h"
#include "unzip.h"
#include "gui.h"

using namespace std;								// For STL stuff

// Private function prototypes

class Window;										// Forward declaration...

void DrawTransparentBitmap(int16 * screen, uint32 x, uint32 y, uint16 * bitmap);
void DrawStringTrans(int16 * screen, uint32 x, uint32 y, uint16 color, uint8 opacity, const char * text, ...);
void DrawStringOpaque(int16 * screen, uint32 x, uint32 y, uint16 color1, uint16 color2, const char * text, ...);
Window * LoadROM(void);
Window * ResetJaguar(void);
Window * RunEmu(void);
Window * Quit(void);
Window * About(void);

int gzfilelength(gzFile gd);

// External variables

extern uint8 * jaguar_mainRam;
extern uint8 * jaguar_bootRom;
extern uint8 * jaguar_mainRom;

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

uint16 upArrowBox[] = {
	8, 8,

	0x4B5E,0x4B5E,0x4B5E,0x4B5E,0x4B5E,0x4B5E,0x4B5E,0x4B5E,		// ++++++++
	0x4B5E,0x0000,0x0000,0xFFFF,0xFFFF,0x0000,0x0000,0x0217,		// +  @@  .
	0x4B5E,0x0000,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0x0000,0x0217,		// + @@@@ .
	0x4B5E,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0x0217,		// +@@@@@@.
	0x4B5E,0x0000,0x0000,0xFFFF,0xFFFF,0x0000,0x0000,0x0217,		// +  @@  .
	0x4B5E,0x0000,0x0000,0xFFFF,0xFFFF,0x0000,0x0000,0x0217,		// +  @@  .
	0x4B5E,0x0000,0x0000,0xFFFF,0xFFFF,0x0000,0x0000,0x0217,		// +  @@  .
	0x0217,0x0217,0x0217,0x0217,0x0217,0x0217,0x0217,0x0217			// ........
};

uint16 downArrowBox[] = {
	8, 8,

	0x4B5E,0x4B5E,0x4B5E,0x4B5E,0x4B5E,0x4B5E,0x4B5E,0x4B5E,		// ++++++++
	0x4B5E,0x0000,0x0000,0xFFFF,0xFFFF,0x0000,0x0000,0x0217,		// +  @@  .
	0x4B5E,0x0000,0x0000,0xFFFF,0xFFFF,0x0000,0x0000,0x0217,		// +  @@  .
	0x4B5E,0x0000,0x0000,0xFFFF,0xFFFF,0x0000,0x0000,0x0217,		// +  @@  .
	0x4B5E,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0x0217,		// +@@@@@@.
	0x4B5E,0x0000,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0x0000,0x0217,		// + @@@@ .
	0x4B5E,0x0000,0x0000,0xFFFF,0xFFFF,0x0000,0x0000,0x0217,		// +  @@  .
	0x0217,0x0217,0x0217,0x0217,0x0217,0x0217,0x0217,0x0217			// ........
};

char separator[] = "--------------------------------------------------------";

uint16 background[1280 * 240];

//
// Local GUI classes
//

enum { WINDOW_CLOSE, MENU_ITEM_CHOSEN };

class Element
{
	public:
		Element(uint32 x = 0, uint32 y = 0, uint32 w = 0, uint32 h = 0)
			{ extents.x = x, extents.y = y, extents.w = w, extents.h = h; }
		virtual void HandleKey(SDLKey key) = 0;
		virtual void HandleMouseMove(uint32 x, uint32 y) = 0;
		virtual void HandleMouseButton(uint32 x, uint32 y, bool mouseDown) = 0;
		virtual void Draw(uint32, uint32) = 0;
		virtual void Notify(Element *) = 0;
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
			bgColor(0x03E0), pic(NULL), elementToTell(NULL) {}
		Button(uint32 x, uint32 y, uint32 w, uint32 h, uint16 * p): Element(x, y, w, h),
			activated(false), clicked(false), inside(false), fgColor(0xFFFF),
			bgColor(0x03E0), pic(p), elementToTell(NULL) {}
		Button(uint32 x, uint32 y, uint16 * p): Element(x, y, 0, 0),
			activated(false), clicked(false), inside(false), fgColor(0xFFFF),
			bgColor(0x03E0), pic(p), elementToTell(NULL)
			{ if (pic) extents.w = pic[0], extents.h = pic[1]; }
		Button(uint32 x, uint32 y, uint32 w, uint32 h, string s): Element(x, y, w, h),
			activated(false), clicked(false), inside(false), fgColor(0xFFFF),
			bgColor(0x03E0), pic(NULL), text(s), elementToTell(NULL) {}
		Button(uint32 x, uint32 y, string s): Element(x, y, 0, 8),
			activated(false), clicked(false), inside(false), fgColor(0xFFFF),
			bgColor(0x03E0), pic(NULL), text(s), elementToTell(NULL)
			{ extents.w = s.length() * 8; }
		virtual void HandleKey(SDLKey key) {}
		virtual void HandleMouseMove(uint32 x, uint32 y);
		virtual void HandleMouseButton(uint32 x, uint32 y, bool mouseDown);
		virtual void Draw(uint32 offsetX = 0, uint32 offsetY = 0);
		virtual void Notify(Element *) {}
		bool ButtonClicked(void) { return activated; }
		void SetNotificationElement(Element * e) { elementToTell = e; }

	protected:
		bool activated, clicked, inside;
		uint16 fgColor, bgColor;
		uint16 * pic;
		string text;
		Element * elementToTell;
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
		{
			clicked = false, activated = true;

			// Send a message that we're activated (if there's someone to tell, that is)
			if (elementToTell)
				elementToTell->Notify(this);
		}
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

	if (pic != NULL)
		DrawTransparentBitmap(screenBuffer, extents.x + offsetX, extents.y + offsetY, pic);

	if (text.length() > 0)
		DrawString(screenBuffer, extents.x + offsetX, extents.y + offsetY, false, "%s", text.c_str());
}

class Window: public Element
{
	public:
/*		Window(uint32 x = 0, uint32 y = 0, uint32 w = 0, uint32 h = 0):	Element(x, y, w, h),
			fgColor(0x4FF0), bgColor(0xFE10)
			{ close = new Button(w - 8, 1, closeBox); list.push_back(close); }*/
		Window(uint32 x = 0, uint32 y = 0, uint32 w = 0, uint32 h = 0,
			void (* f)(Element *) = NULL): Element(x, y, w, h),
			/*clicked(false), inside(false),*/ fgColor(0x4FF0), bgColor(0x1E10),
			handler(f)
			{ close = new Button(w - 8, 1, closeBox); list.push_back(close);
			  close->SetNotificationElement(this); }
		virtual ~Window();
		virtual void HandleKey(SDLKey key);
		virtual void HandleMouseMove(uint32 x, uint32 y);
		virtual void HandleMouseButton(uint32 x, uint32 y, bool mouseDown);
		virtual void Draw(uint32 offsetX = 0, uint32 offsetY = 0);
		virtual void Notify(Element * e);
		void AddElement(Element * e);
//		bool WindowActive(void) { return true; }//return !close->ButtonClicked(); }

	protected:
//		bool clicked, inside;
		uint16 fgColor, bgColor;
		void (* handler)(Element *);
		Button * close;
//We have to use a list of Element *pointers* because we can't make a list that will hold
//all the different object types in the same list...
		vector<Element *> list;
};

Window::~Window()
{
	for(uint32 i=0; i<list.size(); i++)
		if (list[i])
			delete list[i];
}

void Window::HandleKey(SDLKey key)
{
	if (key == SDLK_ESCAPE)
	{
		SDL_Event event;
		event.type = SDL_USEREVENT, event.user.code = WINDOW_CLOSE;
		SDL_PushEvent(&event);
	}

	// Handle the items this window contains...
	for(uint32 i=0; i<list.size(); i++)
		// Make coords relative to upper right corner of this window...
		list[i]->HandleKey(key);
}

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

void Window::Notify(Element * e)
{
	if (e == close)
	{
		SDL_Event event;
		event.type = SDL_USEREVENT, event.user.code = WINDOW_CLOSE;
		SDL_PushEvent(&event);
	}
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
		virtual void Notify(Element *) {}

	protected:
		uint16 fgColor, bgColor;
		string text;
};

void Text::Draw(uint32 offsetX/*= 0*/, uint32 offsetY/*= 0*/)
{
	if (text.length() > 0)
		DrawString(screenBuffer, extents.x + offsetX, extents.y + offsetY, false, "%s", text.c_str());
}

class ListBox: public Element
//class ListBox: public Window
{
	public:
//		ListBox(uint32 x = 0, uint32 y = 0, uint32 w = 0, uint32 h = 0): Element(x, y, w, h),
		ListBox(uint32 x = 0, uint32 y = 0, uint32 w = 0, uint32 h = 0);//: Window(x, y, w, h),
//		windowPtr(0), cursor(0), limit(0), charWidth((w / 8) - 1), charHeight(h / 8),
//		elementToTell(NULL), upArrow(w - 8, 0, upArrowBox),
//		downArrow(w - 8, h - 8, downArrowBox), upArrow2(w - 8, h - 16, upArrowBox) {}
		virtual void HandleKey(SDLKey key);
		virtual void HandleMouseMove(uint32 x, uint32 y);
		virtual void HandleMouseButton(uint32 x, uint32 y, bool mouseDown);
		virtual void Draw(uint32 offsetX = 0, uint32 offsetY = 0);
		virtual void Notify(Element * e);
		void SetNotificationElement(Element * e) { elementToTell = e; }
		void AddItem(string s);
		string GetSelectedItem(void);

	protected:
		bool thumbClicked;
		uint32 windowPtr, cursor, limit;
		uint32 charWidth, charHeight;				// Box width/height in characters
		Element * elementToTell;
		Button upArrow, downArrow, upArrow2;
		vector<string> item;

	private:
		uint32 yRelativePoint;
};

ListBox::ListBox(uint32 x, uint32 y, uint32 w, uint32 h): Element(x, y, w, h),
	thumbClicked(false), windowPtr(0), cursor(0), limit(0), charWidth((w / 8) - 1),
	charHeight(h / 8), elementToTell(NULL), upArrow(w - 8, 0, upArrowBox),
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
//How to handle these???
/*	if (key == SDLK_RETURN)
		done = true;
	if (key == SDLK_ESCAPE)
	{
		WriteLog("GUI: Aborting VJ by user request.\n");
		return false;						// Bail out!
	}*/
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
					windowPtr = i - limit + 1,
					cursor = limit - 1;
				if (i < windowPtr)
					windowPtr = i,
					cursor = 0;
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
			thumb = (uint32)(((float)limit / (float)item.size()) * (float)sbHeight),
			thumbStart = (uint32)(((float)windowPtr / (float)item.size()) * (float)sbHeight);

//yRelativePoint is the spot on the thumb where we clicked...
//		int32 thumbDelta = y - yRelativePoint;
		int32 newThumbStart = y - yRelativePoint;

		if (newThumbStart < 0)
			newThumbStart = 0;

		if (newThumbStart > sbHeight - thumb)
			newThumbStart = sbHeight - thumb;

		windowPtr = (uint32)(((float)newThumbStart / (float)sbHeight) * (float)item.size());
//Check for cursor bounds as well... Or do we need to???
	}
}

void ListBox::HandleMouseButton(uint32 x, uint32 y, bool mouseDown)
{
	if (Inside(x, y) && mouseDown)
	{
		// Why do we have to do this??? (- extents.y?)
		// I guess it's because only the Window class has offsetting implemented...
		cursor = (y - extents.y) / 8;
	}

	// Check for a hit on the scrollbar...
	if (x > (extents.x + extents.w) && x <= (extents.x + extents.w + 8)
		&& y > (extents.y + 8) && y <= (extents.y + extents.h - 16))
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
		else
			thumbClicked = false;
	}

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
		string s(item[windowPtr + i], 0, item[windowPtr + i].length() - 4);
		DrawString(screenBuffer, extents.x + offsetX, extents.y + offsetY + i*8,
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
				screenBuffer[x + (y * pitch)] = (thumbClicked ? 0x458E : 0xFFFF);
			else
				screenBuffer[x + (y * pitch)] = 0x0200;
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

void ListBox::AddItem(string s)
{
	item.push_back(s);
	limit = (item.size() > charHeight ? charHeight : item.size());
//WriteLog("ListBox: Adding item [%s], limit = %u...\n", s.c_str(), limit);

	//Do this *every* time?
	//What other choice is there? :-p
	sort(item.begin(), item.end());
}

string ListBox::GetSelectedItem(void)
{
	return item[windowPtr + cursor];
}

class FileList: public Window
{
	public:
		FileList(uint32 x = 0, uint32 y = 0, uint32 w = 0, uint32 h = 0);
		virtual ~FileList() {}
		virtual void HandleKey(SDLKey key);
		virtual void HandleMouseMove(uint32 x, uint32 y) { Window::HandleMouseMove(x, y); }
		virtual void HandleMouseButton(uint32 x, uint32 y, bool mouseDown) { Window::HandleMouseButton(x, y, mouseDown); }
		virtual void Draw(uint32 offsetX = 0, uint32 offsetY = 0) { Window::Draw(offsetX, offsetY); }
		virtual void Notify(Element * e);

	protected:
		ListBox * files;
		Button * load;
};

//Need 4 buttons, one scrollbar...
FileList::FileList(uint32 x, uint32 y, uint32 w, uint32 h): Window(x, y, w, h)
{
	files = new ListBox(8, 8, w - 16, h - 32);
	AddElement(files);
	load = new Button(8, h - 16, " Load ");
	AddElement(load);
	load->SetNotificationElement(this);

//	DIR * dp = opendir(path);
	DIR * dp = opendir(vjs.ROMPath);
	dirent * de;

	while ((de = readdir(dp)) != NULL)
	{
		char * ext = strrchr(de->d_name, '.');

		if (ext != NULL)
			if (stricmp(ext, ".zip") == 0 || stricmp(ext, ".jag") == 0)
				files->AddItem(string(de->d_name));
	}

	closedir(dp);
}

void FileList::HandleKey(SDLKey key)
{
	if (key == SDLK_RETURN)
		Notify(load);
	else
		Window::HandleKey(key);
}

void FileList::Notify(Element * e)
{
	if (e == load)
	{
		char filename[MAX_PATH];
		strcpy(filename, vjs.ROMPath);

		if (strlen(filename) > 0)
			if (filename[strlen(filename) - 1] != '/')
				strcat(filename, "/");

		strcat(filename, files->GetSelectedItem().c_str());

		uint32 romSize = JaguarLoadROM(jaguar_mainRom, filename);

		if (romSize == 0)
//We need better error checking here... !!! FIX !!!
			WriteLog("VJ: Could not load ROM from file \"%s\"...", files->GetSelectedItem().c_str());
		else
		{
			jaguar_mainRom_crc32 = crc32_calcCheckSum(jaguar_mainRom, romSize);
			WriteLog("CRC: %08X\n", (unsigned int)jaguar_mainRom_crc32);
			eeprom_init();

			SDL_Event event;
			event.type = SDL_USEREVENT, event.user.code = WINDOW_CLOSE;
			SDL_PushEvent(&event);

			event.type = SDL_USEREVENT, event.user.code = MENU_ITEM_CHOSEN;
			event.user.data1 = (void *)ResetJaguar;
		    SDL_PushEvent(&event);
		}
	}
	else
		Window::Notify(e);
}


struct NameAction
{
	string name;
	Window * (* action)(void);
	SDLKey hotKey;

	NameAction(string n, Window * (* a)(void) = NULL, SDLKey k = SDLK_UNKNOWN): name(n),
		action(a), hotKey(k) {}
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
			uint16 bgch = 0x1CFF): Element(x, y, w, h), activated(false), clicked(false),
			inside(0), insidePopup(0), fgColor(fgc), bgColor(bgc), fgColorHL(fgch),
			bgColorHL(bgch), menuChosen(-1), menuItemChosen(-1) {}
		virtual void HandleKey(SDLKey key);
		virtual void HandleMouseMove(uint32 x, uint32 y);
		virtual void HandleMouseButton(uint32 x, uint32 y, bool mouseDown);
		virtual void Draw(uint32 offsetX = 0, uint32 offsetY = 0);
		virtual void Notify(Element *) {}
		void Add(MenuItems mi);

	protected:
		bool activated, clicked;
		uint32 inside, insidePopup;
		uint16 fgColor, bgColor, fgColorHL, bgColorHL;
		int menuChosen, menuItemChosen;

	private:
		vector<MenuItems> itemList;
};

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
		virtual void Notify(Element *) {}

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
// Draw text at the given x/y coordinates with transparency (0 is fully opaque, 32 is fully transparent).
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
//because dividing by 32 is faster than dividing by 31...!
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
	Window * mainWindow = NULL;

	// Set up the GUI classes...
	Element::SetScreenAndPitch(backbuffer, GetSDLScreenPitch() / 2);

	Menu mainMenu;
	MenuItems mi;
	mi.title = "File";
	mi.item.push_back(NameAction("Load...", LoadROM, SDLK_l));
	mi.item.push_back(NameAction("Reset", ResetJaguar));
	mi.item.push_back(NameAction("Run", RunEmu, SDLK_ESCAPE));
	mi.item.push_back(NameAction(""));
	mi.item.push_back(NameAction("Quit", Quit, SDLK_q));
	mainMenu.Add(mi);
	mi.title = "Settings";
	mi.item.clear();
	mi.item.push_back(NameAction("Video..."));
	mi.item.push_back(NameAction("Audio..."));
	mi.item.push_back(NameAction("Misc..."));
	mainMenu.Add(mi);
	mi.title = "Info";
	mi.item.clear();
	mi.item.push_back(NameAction("About...", About));
	mainMenu.Add(mi);

	bool showMouse = true;

//This is crappy!!! !!! FIX !!!
	jaguar_reset();

	// Set up our background save...
	memset(background, 0x11, tom_getVideoModeWidth() * 240 * 2);

	while (!done)
	{
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_USEREVENT)
			{
				if (event.user.code == WINDOW_CLOSE)
				{
					delete mainWindow;
					mainWindow = NULL;
				}
				else if (event.user.code == MENU_ITEM_CHOSEN)
				{
					// Confused? Let me enlighten... What we're doing here is casting
					// data1 as a pointer to a function which returns a Window pointer and
					// which takes no parameters (the "(Window *(*)(void))" part), then
					// derefencing it (the "*" in front of that) in order to call the
					// function that it points to. Clear as mud? Yeah, I hate function
					// pointers too, but what else are you gonna do?
					mainWindow = (*(Window *(*)(void))event.user.data1)();

					while (SDL_PollEvent(&event));	// Flush the event queue...
					event.type = SDL_MOUSEMOTION;
					int mx, my;
					SDL_GetMouseState(&mx, &my);
					event.motion.x = mx, event.motion.y = my;
				    SDL_PushEvent(&event);			// & update mouse position...!

					mouseX = mx, mouseY = my;		// This prevents "mouse flash"...
					if (vjs.useOpenGL)
						mouseX /= 2, mouseY /= 2;
				}
			}
			else if (event.type == SDL_ACTIVEEVENT)
			{
				if (event.active.state == SDL_APPMOUSEFOCUS)
					showMouse = (event.active.gain ? true : false);
			}
			else if (event.type == SDL_KEYDOWN)
			{
				if (mainWindow)
					mainWindow->HandleKey(event.key.keysym.sym);
				else
					mainMenu.HandleKey(event.key.keysym.sym);
			}
			else if (event.type == SDL_MOUSEMOTION)
			{
				mouseX = event.motion.x, mouseY = event.motion.y;

				if (vjs.useOpenGL)
					mouseX /= 2, mouseY /= 2;

				if (mainWindow)
					mainWindow->HandleMouseMove(mouseX, mouseY);
				else
					mainMenu.HandleMouseMove(mouseX, mouseY);
			}
			else if (event.type == SDL_MOUSEBUTTONDOWN)
			{
				uint32 mx = event.button.x, my = event.button.y;

				if (vjs.useOpenGL)
					mx /= 2, my /= 2;

				if (mainWindow)
					mainWindow->HandleMouseButton(mx, my, true);
				else
					mainMenu.HandleMouseButton(mx, my, true);
			}
			else if (event.type == SDL_MOUSEBUTTONUP)
			{
				uint32 mx = event.button.x, my = event.button.y;

				if (vjs.useOpenGL)
					mx /= 2, my /= 2;

				if (mainWindow)
					mainWindow->HandleMouseButton(mx, my, false);
				else
					mainMenu.HandleMouseButton(mx, my, false);
			}

			// Draw the GUI...
// The way we do things here is kinda stupid (redrawing the screen every frame), but
// it's simple. Perhaps there may be a reason down the road to be more selective with
// our clearing, but for now, this will suffice.
//			memset(backbuffer, 0x11, tom_getVideoModeWidth() * 240 * 2);
			memcpy(backbuffer, background, tom_getVideoModeWidth() * 240 * 2);

			mainMenu.Draw();
			if (mainWindow)
				mainWindow->Draw();

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
Window * LoadROM(void)
{
	FileList * fileList = new FileList(8, 16, 304, 216);

	return (Window *)fileList;
}

Window * ResetJaguar(void)
{
	jaguar_reset();
	return RunEmu();
}

bool debounceRunKey = true;
Window * RunEmu(void)
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
	// Pass a message to the "joystick" code to debounce the ESC key...
	debounceRunKey = true;

	while (!finished)
	{
		// Set up new backbuffer with new pixels and data
		JaguarExecute(backbuffer, true);
		totalFrames++;
//WriteLog("Frame #%u...\n", totalFrames);
//extern bool doDSPDis;
//if (totalFrames == 373)
//	doDSPDis = true;

//This sucks... !!! FIX !!!
		joystick_exec();
		if (finished)
			break;

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
	}

	// Reset the pitch, since it may have been changed in-game...
	Element::SetScreenAndPitch(backbuffer, GetSDLScreenPitch() / 2);

	// Save the background for the GUI...
//	memcpy(background, backbuffer, tom_getVideoModeWidth() * 240 * 2);
	// In this case, we squash the color to monochrome, then force it to blue + green...
	for(uint32 i=0; i<tom_getVideoModeWidth() * 240; i++)
	{
		uint16 word = backbuffer[i];
		uint8 r = (word >> 10) & 0x1F, g = (word >> 5) & 0x1F, b = word & 0x1F;
		word = ((r + g + b) / 3) & 0x001F;
		word = (word << 5) | word;
		background[i] = word;
	}

	return NULL;
}

Window * Quit(void)
{
	WriteLog("GUI: Quitting due to user request.\n");
	log_done();
	exit(0);

	return NULL;									// We never get here...
}

Window * About(void)
{
	Window * window = new Window(8, 16, 304, 160);
	window->AddElement(new Text(8, 8, "Virtual Jaguar 1.0.7"));
	window->AddElement(new Text(8, 24, "Coders:"));
	window->AddElement(new Text(16, 32, "Niels Wagenaar (nwagenaar)"));
	window->AddElement(new Text(16, 40, "Carwin Jones (Caz)"));
	window->AddElement(new Text(16, 48, "James L. Hammons (shamus)"));
	window->AddElement(new Text(16, 56, "Adam Green"));

	return window;
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
/*bool UserSelectFile(char * path, char * filename)
{
//Testing...
GUIMain();
	
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
}*/

//
// Generic ROM loading
//
uint32 JaguarLoadROM(uint8 * rom, char * path)
{
	uint32 romSize = 0;

	char * ext = strrchr(path, '.');
	if (ext != NULL)
	{
		WriteLog("VJ: Loading \"%s\"...", path);

		if (stricmp(ext, ".zip") == 0)
		{
			// Handle ZIP file loading here...
			WriteLog("(ZIPped)...");

			if (load_zipped_file(0, 0, path, NULL, &rom, &romSize) == -1)
			{
				WriteLog("Failed!\n");
				return 0;
			}
		}
		else
		{
/*			FILE * fp = fopen(path, "rb");

			if (fp == NULL)
			{
				WriteLog("Failed!\n");
				return 0;
			}

			fseek(fp, 0, SEEK_END);
			romSize = ftell(fp);
			fseek(fp, 0, SEEK_SET);
			fread(rom, 1, romSize, fp);
			fclose(fp);*/

			gzFile fp = gzopen(path, "rb");

			if (fp == NULL)
			{
				WriteLog("Failed!\n");
				return 0;
			}

			romSize = gzfilelength(fp);
			gzseek(fp, 0, SEEK_SET);
			gzread(fp, rom, romSize);
			gzclose(fp);
		}

		WriteLog("OK (%i bytes)\n", romSize);
	}

	return romSize;
}

//
// Jaguar cartridge ROM loading
//
void JaguarLoadCart(uint8 * mem, char * path)
{
	uint32 romSize = JaguarLoadROM(mem, path);

	if (romSize == 0)
	{
/*		char newPath[2048];
		WriteLog("VJ: Trying GUI...\n");

//This is not *nix friendly for some reason...
//		if (!UserSelectFile(path, newPath))
		if (!UserSelectFile((strlen(path) == 0 ? (char *)"." : path), newPath))
		{
			WriteLog("VJ: Could not find valid ROM in directory \"%s\"...\nAborting!\n", path);
			log_done();
			exit(0);
		}

		romSize = JaguarLoadROM(mem, newPath);
*/
		if (romSize == 0)
		{
//			WriteLog("VJ: Could not load ROM from file \"%s\"...\nAborting!\n", newPath);
			WriteLog("VJ: Could not load ROM from file \"%s\"...\nAborting!\n", path);
			log_done();
			exit(0);
		}
	}

	jaguar_mainRom_crc32 = crc32_calcCheckSum(jaguar_mainRom, romSize);
	WriteLog("CRC: %08X\n", (unsigned int)jaguar_mainRom_crc32);
	eeprom_init();
}

//
// Get the length of a (possibly) gzipped file
//
int gzfilelength(gzFile gd)
{
   int size = 0, length = 0;
   unsigned char buffer[0x10000];

   gzrewind(gd);

   do
   {
      // Read in chunks until EOF
      size = gzread(gd, buffer, 0x10000);

      if (size <= 0)
      	break;

      length += size;
   }
   while (!gzeof(gd));

   gzrewind(gd);
   return length;
}
