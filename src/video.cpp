//
// VIDEO.CPP: SDL/local hardware specific video routines
//
// by James L. Hammons
//

#include "types.h"
#include "tom.h"
#include "sdlemu_opengl.h"							// For testing only... ;-)
#include "video.h"

// External global variables

extern SDL_Surface * surface, * mainSurface;
extern Uint32 mainSurfaceFlags;
extern int16 * backbuffer;
extern bool useOpenGL;								// Testing, testing... :-)
extern bool fullscreen;

//
// Render the backbuffer to the primary screen surface
//
void RenderBackbuffer(void)
{
	if (SDL_MUSTLOCK(surface))
		while (SDL_LockSurface(surface) < 0)
			SDL_Delay(10);

	memcpy(surface->pixels, backbuffer, tom_getVideoModeWidth() * tom_getVideoModeHeight() * 2);

	if (SDL_MUSTLOCK(surface))
		SDL_UnlockSurface(surface);

	if (useOpenGL)
		sdlemu_draw_texture(mainSurface, surface, 1/*1=GL_QUADS*/);
	else
	{
		SDL_Rect rect = { 0, 0, surface->w, surface->h };
		SDL_BlitSurface(surface, &rect, mainSurface, &rect);
		SDL_Flip(mainSurface);
    }
}

//
// Resize the main SDL screen & backbuffer
//
void ResizeScreen(uint32 width, uint32 height)
{
	char window_title[256];

	SDL_FreeSurface(surface);
	surface = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height,
		16, 0x7C00, 0x03E0, 0x001F, 0);
	if (surface == NULL)
	{
		WriteLog("TOM: Could not create primary SDL surface: %s", SDL_GetError());
		exit(1);
	}

	sprintf(window_title, "Virtual Jaguar (%i x %i)", (int)width, (int)height);
//Hmm.
/*	mainSurface = SDL_SetVideoMode(tom_width, tom_height, 16, mainSurfaceFlags);

	if (mainSurface == NULL)
	{
		WriteLog("Joystick: SDL is unable to set the video mode: %s\n", SDL_GetError());
		exit(1);
	}//*/

	SDL_WM_SetCaption(window_title, window_title);

//This seems to work well for resizing...
	if (useOpenGL)
		sdlemu_resize_texture(surface, mainSurface);
}

//
// Return the screen's pitch
//
uint32 GetSDLScreenPitch(void)
{
//	extern SDL_Surface * surface;

	return surface->pitch;
}

//
// Fullscreen <-> window switching
//
//NOTE: This does *NOT* work with OpenGL rendering! !!! FIX !!!
void ToggleFullscreen(void)
{
	fullscreen = !fullscreen;
	mainSurfaceFlags &= ~SDL_FULLSCREEN;

	if (fullscreen)
		mainSurfaceFlags |= SDL_FULLSCREEN;

	mainSurface = SDL_SetVideoMode(tom_width, tom_height, 16, mainSurfaceFlags);

	if (mainSurface == NULL)
	{
		WriteLog("Joystick: SDL is unable to set the video mode: %s\n", SDL_GetError());
		exit(1);
	}

	SDL_WM_SetCaption("Virtual Jaguar", "Virtual Jaguar");
}
