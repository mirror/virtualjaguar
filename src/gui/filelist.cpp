//
// FileList class
//
// by James L. Hammons
//

#include "filelist.h"

#include <sys/types.h>								// For MacOS <dirent.h> dependency
#include <dirent.h>
#include "settings.h"
#include "file.h"
#include "guimisc.h"

//Need 4 buttons, one scrollbar...
FileList::FileList(uint32 x, uint32 y, uint32 w, uint32 h): Window(x, y, w, h)
{
	files = new ListBox(8, 8, w - 16, h - 32);
	AddElement(files);
	load = new Button(8, h - 16, " Load ");
	AddElement(load);
	load->SetNotificationElement(this);

#warning !!! FIX !!! Directory might not exist--this shouldn't cause VJ to crash!
	DIR * dp = opendir(vjs.ROMPath);
	dirent * de;

	if (dp != NULL)
	{
		while ((de = readdir(dp)) != NULL)
		{
			char * ext = strrchr(de->d_name, '.');

			if (ext != NULL)
				if (strcasecmp(ext, ".zip") == 0 || strcasecmp(ext, ".j64") == 0
					|| strcasecmp(ext, ".abs") == 0 || strcasecmp(ext, ".jag") == 0
					|| strcasecmp(ext, ".rom") == 0)
					files->AddItem(std::string(de->d_name));
		}

		closedir(dp);
	}
	else
	{
//Give a diagnostic message here so that the (l)user can figure out what went wrong. !!! FIX !!!
	}
}

void FileList::HandleKey(SDLKey key)
{
	if (key == SDLK_RETURN)
		Notify(load);
	else
		Window::HandleKey(key);
}

void FileList::HandleMouseMove(uint32 x, uint32 y)
{
	Window::HandleMouseMove(x, y);
}

void FileList::HandleMouseButton(uint32 x, uint32 y, bool mouseDown)
{
	Window::HandleMouseButton(x, y, mouseDown);
}

void FileList::Draw(uint32 offsetX/*= 0*/, uint32 offsetY/*= 0*/)
{
	Window::Draw(offsetX, offsetY);
}

extern Window * ResetJaguar(void);
#warning ICKY KLUDGE--FIX THIS MESS!!!
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

//		uint32 romSize = JaguarLoadROM(jaguar_mainRom, filename);
//		JaguarLoadCart(jaguar_mainRom, filename);
		if (JaguarLoadFile(filename))
		{
			SDL_Event event;
			event.type = SDL_USEREVENT, event.user.code = WINDOW_CLOSE;
			SDL_PushEvent(&event);

			event.type = SDL_USEREVENT, event.user.code = MENU_ITEM_CHOSEN;
			event.user.data1 = (void *)ResetJaguar;
	    	SDL_PushEvent(&event);
		}
		else
		{
			SDL_Event event;
			event.type = SDL_USEREVENT, event.user.code = WINDOW_CLOSE;
			SDL_PushEvent(&event);

			// Handle the error, but don't run...
			// Tell the user that we couldn't run their file for some reason... !!! FIX !!!
//how to kludge: Make a function like ResetJaguar which creates the dialog window
		}
	}
	else
		Window::Notify(e);
}
