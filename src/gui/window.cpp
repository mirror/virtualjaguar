//
// Window class
//
// by James L. Hammons
//

#include "window.h"

#include "guimisc.h"

Window::Window(uint32 x/*= 0*/, uint32 y/*= 0*/, uint32 w/*= 0*/, uint32 h/*= 0*/,
	void (* f)(Element *)/*= NULL*/): Element(x, y, w, h),
//			/*clicked(false), inside(false),*/ fgColor(0x4FF0), bgColor(0x1E10),
//4FF0 -> 010011 11111 10000 -> 0100 1101 1111 1111 1000 0100 -> 4D FF 84
//1E10 -> 000111 10000 10000 -> 0001 1111 1000 0100 1000 0100 -> 1F 84 84
	/*clicked(false), inside(false),*/ fgColor(0xFF84FF4D), bgColor(0xFF84841F),
	handler(f)
{
	close = new Button(w - (CLOSEBOX_WIDTH + 1), 1, closeBox, closeBoxHover, closeBoxDown);
	list.push_back(close);
	close->SetNotificationElement(this);
}

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
