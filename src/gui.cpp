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
#include "tom.h"
#include "video.h"
#include "font1.h"
#include "gui.h"

using namespace std;								// For STL stuff

// Private function prototypes

// Local global variables

int mouseX, mouseY;


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
// Very very crude GUI file selector
//
bool UserSelectFile(char * path, char * filename)
{
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

	// Main GUI selection loop

	uint32 cursor = 0, startFile = 0;

	if (fileList.size() > 1)	// Only go GUI if more than one possibility!
	{
		sort(fileList.begin(), fileList.end());

		bool done = false;
		uint32 limit = (fileList.size() > 30 ? 30 : fileList.size());
		SDL_Event event;

		while (!done)
		{
			while (SDL_PollEvent(&event))
			{
				// Draw the GUI...
//				memset(backbuffer, 0x11, tom_getVideoModeWidth() * tom_getVideoModeHeight() * 2);
				memset(backbuffer, 0x11, tom_getVideoModeWidth() * 240 * 2);

				for(uint32 i=0; i<limit; i++)
				{
					bool invert = (cursor == i ? true : false);
					// Clip our strings to guarantee that they fit on the screen...
					string s = fileList[startFile + i];
					if (s.length() > 38)
						s[38] = 0;
					DrawString(backbuffer, 0, i*8, invert, " %s ", s.c_str());
				}

					uint32 pitch = GetSDLScreenPitch() / 2;	// Returns pitch in bytes but we need words...
//					uint32 address = x + (y * pitch);
					backbuffer[mouseX + (mouseY * pitch)] = 0xFFFF;


				RenderBackbuffer();

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
					}
					if (key == SDLK_PAGEUP)
					{
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
					//Kludge: divide by two in order to display properly on our blown up
					//        screen...
					mouseX = event.motion.x / 2, mouseY = event.motion.y / 2;
				}
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
