//
// GUI.CPP
//
// Graphical User Interface support
// by James L. Hammons
//

#include <string.h>
#include <dirent.h>
#include <SDL.h>
#include "types.h"
#include "tom.h"
#include "font1.h"
#include "gui.h"

// Private function prototypes

uint32 CountROMFiles(char * path);


void InitGUI(void)
{
}

void GUIDone(void)
{
}

//
// Render the backbuffer to the primary screen surface
//
void BlitBackbuffer(void)
{
	extern SDL_Surface * surface, * mainSurface;
	extern int16 * backbuffer;

	if (SDL_MUSTLOCK(surface))
		while (SDL_LockSurface(surface) < 0)
			SDL_Delay(10);

	memcpy(surface->pixels, backbuffer, tom_getVideoModeWidth() * tom_getVideoModeHeight() * 2);

	if (SDL_MUSTLOCK(surface))
		SDL_UnlockSurface(surface);

	SDL_Rect srcrect, dstrect;
	srcrect.x = srcrect.y = 0, srcrect.w = surface->w, srcrect.h = surface->h;
	dstrect.x = dstrect.y = 0, dstrect.w = surface->w, dstrect.h = surface->h;
	SDL_BlitSurface(surface, &srcrect, mainSurface, &dstrect);
    SDL_Flip(mainSurface);      
}

//
// Draw text at the given x/y coordinates. Can invert text as well.
//
void DrawText(int16 * screen, uint32 x, uint32 y, bool invert, const char * text, ...)
{
	char string[4096];
	va_list arg;

	va_start(arg, text);
	vsprintf(string, text, arg);
	va_end(arg);

	uint32 pitch = TOMGetSDLScreenPitch() / 2;		// Returns pitch in bytes but we need words...
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
	uint32 numFiles = CountROMFiles(path);

	if (numFiles == 0)
		return false;

	char * names = (char *)malloc(numFiles * 2048);

	if (names == NULL)
	{
		WriteLog("Could not allocate memory for %u files!\nAborting!\n", numFiles);
		return false;
	}

	int i = 0;
	DIR * dp = opendir(path);
	dirent * de;

	while ((de = readdir(dp)) != NULL)
	{
		char * ext = strrchr(de->d_name, '.');
		if (ext != NULL)
			if (strcmpi(ext, ".zip") == 0 || strcmpi(ext, ".jag") == 0)
				strcpy(&names[i++ * 2048], de->d_name);
	}

	closedir(dp);

	// Main GUI selection loop

	uint32 cursor = 0, startFile = 0;

	if (numFiles > 1)	// Only go GUI if more than one possibility!
	{
		bool done = false;
		uint32 limit = (numFiles > 24 ? 24 : numFiles);
		SDL_Event event;

		while (!done)
		{
			while (SDL_PollEvent(&event))
			{
				// Draw the GUI...
				memset(backbuffer, 0x11, tom_getVideoModeWidth() * tom_getVideoModeHeight() * 2);

				for(uint32 i=0; i<limit; i++)
				{
					bool invert = (cursor == i ? true : false);
					char buf[41];
					// Guarantee that we clip our strings to fit in the screen...
					memcpy(buf, &names[(startFile + i) * 2048], 38);
					buf[38] = 0;
					DrawText(backbuffer, 0, i*8, invert, " %s ", buf);
				}

				BlitBackbuffer();

				if (event.type == SDL_KEYDOWN)
				{
					SDLKey key = event.key.keysym.sym;

					if (key == SDLK_DOWN)
					{
						if (cursor != limit - 1)	// Cursor is within its window
							cursor++;
						else						// Otherwise, scroll the window...
						{
							if (cursor + startFile != numFiles - 1)
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
					if (key == SDLK_RETURN)
						done = true;
					if (key == SDLK_ESCAPE)
					{
						WriteLog("GUI: Aborting VJ by user request.\n");
						return false;						// Bail out!
					}
				}
			}
		}
	}

	strcpy(filename, path);
	// Potential GPF here: If length of dir is zero, then this will cause a page fault!
	if (path[strlen(path) - 1] != '/')
		strcat(filename, "/");
	strcat(filename, &names[(cursor + startFile) * 2048]);
	free(names);

	return true;
}

//
// Count # of possible ROM files in the current directory
//
uint32 CountROMFiles(char * path)
{
	uint32 numFiles = 0;
	DIR * dp = opendir(path);

	if (dp == NULL)
	{
		WriteLog("VJ: Could not open directory \"%s\"!\nAborting!\n", path);
		return 0;
	}
	else
	{
		dirent * de;

		while ((de = readdir(dp)) != NULL)
		{
			char * ext = strrchr(de->d_name, '.');
			if (ext != NULL)
				if (strcmpi(ext, ".zip") == 0 || strcmpi(ext, ".jag") == 0)
					numFiles++;
		}

		closedir(dp);
	}

	return numFiles;
}
