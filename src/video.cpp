//
// VIDEO.CPP: SDL/local hardware specific video routines
//
// by James L. Hammons
// (C) 2010 Underground Software
//
// JLH = James L. Hammons <jlhamm@acm.org>
//
// Who  When        What
// ---  ----------  -------------------------------------------------------------
// JLH  01/16/2010  Created this log ;-)
//

#include "video.h"

//#include "gui.h"								// For "finished"
#include "log.h"
#include "tom.h"
#include "sdlemu_opengl.h"
#include "settings.h"

// External global variables

//shouldn't these exist here??? Prolly.
//And now, they do! :-)
SDL_Surface * surface, * mainSurface;
SDL_Joystick * joystick1;
Uint32 mainSurfaceFlags;
//int16 * backbuffer;
uint32 * backbuffer;

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	#define RMASK 0xFF000000
	#define GMASK 0x00FF0000
	#define BMASK 0x0000FF00
	#define AMASK 0x000000FF
#else
	#define RMASK 0x000000FF
	#define GMASK 0x0000FF00
	#define BMASK 0x00FF0000
	#define AMASK 0xFF000000
#endif


// One of the reasons why OpenGL is slower then normal SDL rendering, is because
// the data is being pumped into the buffer every frame with a overflow as result.
// So, we going tot render every 1 frame instead of every 0 frame.

// [Shamus] This isn't the case. OpenGL is slower because 60 frames a second is a
//          lot of data to pump through the system. In any case, frameskip is probably
//          a good idea for now, since most systems are probably too slow to run at
//          60 FPS. But doing so will have some nasty side effects in some games.
//          You have been warned!

int frame_ticker = 0;

//
// Create SDL/OpenGL surfaces
//
bool VideoInit(void)
{
	// Get proper info about the platform we're running on...
	const SDL_VideoInfo * info = SDL_GetVideoInfo();

	if (!info)
	{
		WriteLog("VJ: SDL is unable to get the video info: %s\n", SDL_GetError());
		return false;
	}

	if (vjs.useOpenGL)
	{
		// Initializing SDL attributes with OpenGL
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		mainSurfaceFlags = SDL_OPENGL;
	}
	else
	{
		if (info->hw_available)
			mainSurfaceFlags = SDL_HWSURFACE | SDL_HWPALETTE | SDL_DOUBLEBUF;

		if (info->blit_hw)
			mainSurfaceFlags |= SDL_HWACCEL;
	}

	if (vjs.fullscreen)
		mainSurfaceFlags |= SDL_FULLSCREEN;

/*	if (!vjs.useOpenGL)
//		mainSurface = SDL_SetVideoMode(VIRTUAL_SCREEN_WIDTH, VIRTUAL_SCREEN_HEIGHT_NTSC, 16, mainSurfaceFlags);
		mainSurface = SDL_SetVideoMode(VIRTUAL_SCREEN_WIDTH,
			(vjs.hardwareTypeNTSC ? VIRTUAL_SCREEN_HEIGHT_NTSC : VIRTUAL_SCREEN_HEIGHT_PAL),
			16, mainSurfaceFlags);
	else
		// When OpenGL is used, we're going to use a standard resolution of 640x480.
		// This way we have good scaling functionality and when the screen is resized
		// because of the NTSC <-> PAL resize, we only have to re-create the texture
		// instead of initializing the entire OpenGL texture en screens.
		mainSurface = SDL_SetVideoMode(640, 480, 16, mainSurfaceFlags);//*/
//24BPP
	if (!vjs.useOpenGL)
//		mainSurface = SDL_SetVideoMode(VIRTUAL_SCREEN_WIDTH, VIRTUAL_SCREEN_HEIGHT_NTSC, 16, mainSurfaceFlags);
		mainSurface = SDL_SetVideoMode(VIRTUAL_SCREEN_WIDTH,
			(vjs.hardwareTypeNTSC ? VIRTUAL_SCREEN_HEIGHT_NTSC : VIRTUAL_SCREEN_HEIGHT_PAL),
			32, mainSurfaceFlags);
	else
		// When OpenGL is used, we're going to use a standard resolution of 640x480.
		// This way we have good scaling functionality and when the screen is resized
		// because of the NTSC <-> PAL resize, we only have to re-create the texture
		// instead of initializing the entire OpenGL texture en screens.
		mainSurface = SDL_SetVideoMode(640, 480, 32, mainSurfaceFlags);//*/

	if (mainSurface == NULL)
	{
		WriteLog("VJ: SDL is unable to set the video mode: %s\n", SDL_GetError());
		return false;
	}

	SDL_WM_SetCaption("Virtual Jaguar", "Virtual Jaguar");

	// Create the primary SDL display (16 BPP, 5/5/5 RGB format)
/*	surface = SDL_CreateRGBSurface(SDL_SWSURFACE, VIRTUAL_SCREEN_WIDTH,
		(vjs.hardwareTypeNTSC ? VIRTUAL_SCREEN_HEIGHT_NTSC : VIRTUAL_SCREEN_HEIGHT_PAL),
		16, 0x7C00, 0x03E0, 0x001F, 0);//*/

	uint32 vsWidth = (vjs.renderType == RT_TV ? 1280 : VIRTUAL_SCREEN_WIDTH),
		vsHeight = (vjs.hardwareTypeNTSC ? VIRTUAL_SCREEN_HEIGHT_NTSC : VIRTUAL_SCREEN_HEIGHT_PAL);

//	if (vjs.renderType == RT_TV)
//		vsWidth = 1280;
//24BPP
//	surface = SDL_CreateRGBSurface(SDL_SWSURFACE, VIRTUAL_SCREEN_WIDTH,
	surface = SDL_CreateRGBSurface(SDL_SWSURFACE, vsWidth, vsHeight, 32, RMASK, GMASK, BMASK, AMASK);

	if (surface == NULL)
	{
		WriteLog("VJ: Could not create primary SDL surface: %s\n", SDL_GetError());
		return false;
	}

	if (vjs.useOpenGL)
		// Let us setup OpenGL and our rendering texture. We give the src (surface) and the
		// dst (mainSurface) display as well as the automatic bpp selection as options so that
		// our texture is automatically created :)
		sdlemu_init_opengl(surface, mainSurface, 1 /*method*/,
			vjs.glFilter /*texture type (linear, nearest)*/,
			0 /* Automatic bpp selection based upon src */);

	// Initialize Joystick support under SDL

	if (vjs.useJoystick)
	{
		if (SDL_NumJoysticks() <= 0)
		{
			vjs.useJoystick = false;
			printf("VJ: No joystick(s) or joypad(s) detected on your system. Using keyboard...\n");
		}
		else
		{
			if ((joystick1 = SDL_JoystickOpen(vjs.joyport)) == 0)
			{
				vjs.useJoystick = false;
				printf("VJ: Unable to open a Joystick on port: %d\n", (int)vjs.joyport);
			}
			else
				printf("VJ: Using: %s\n", SDL_JoystickName(vjs.joyport));
		}
	}

	// Set up the backbuffer
	// To be safe, this should be 1280 * 625 * 2...
	backbuffer = (uint32 *)malloc(1280 * 625 * sizeof(uint32));
	memset(backbuffer, 0x44, VIRTUAL_SCREEN_WIDTH *
		(vjs.hardwareTypeNTSC ? VIRTUAL_SCREEN_HEIGHT_NTSC : VIRTUAL_SCREEN_HEIGHT_PAL)
		* sizeof(uint32));

	return true;
}

//
// Free various SDL components
//
void VideoDone(void)
{
	if (vjs.useOpenGL)
		sdlemu_close_opengl();

	SDL_JoystickClose(joystick1);
	SDL_FreeSurface(surface);
	free(backbuffer);
}

//
// Render the backbuffer to the primary screen surface
//
void RenderBackbuffer(void)
{
	// Handle frameskip *before* we do any heavy lifting here...

	if (frame_ticker > 0)
	{
		frame_ticker--;
		return;
	}

	frame_ticker = vjs.frameSkip;				// Reset frame_ticker

	if (SDL_MUSTLOCK(surface))
		while (SDL_LockSurface(surface) < 0)
			SDL_Delay(10);

//	memcpy(surface->pixels, backbuffer, tom_getVideoModeWidth() * tom_getVideoModeHeight() * 2);
// This memcpy is expensive--do some profiling to see what the impact is!
	if (vjs.renderType == RT_NORMAL)
		memcpy(surface->pixels, backbuffer, TOMGetVideoModeWidth() * TOMGetVideoModeHeight() * 4);
	else if (vjs.renderType == RT_TV)
		memcpy(surface->pixels, backbuffer, 1280 * TOMGetVideoModeHeight() * 4);

	if (SDL_MUSTLOCK(surface))
		SDL_UnlockSurface(surface);

	if (vjs.useOpenGL)
		// One of the reasons why OpenGL is slower then normal SDL rendering, is because
		// the data is being pumped into the buffer every frame with a overflow as result.
		// So, we going to render every 1 fps instead of every 0 fps.
		// [Shamus] This is isn't why it's slower--see top of file for explanation... ;-)
//The problem lies in this function...
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
	SDL_FreeSurface(surface);
	surface = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, 32, RMASK, GMASK, BMASK, AMASK);

	if (surface == NULL)
	{
		WriteLog("Video: Could not create primary SDL surface: %s", SDL_GetError());
//This is just crappy. We shouldn't exit this way--it leaves all kinds of memory leaks
//as well as screwing up SDL... !!! FIX !!!
//		exit(1);
		// OK, this is cleaner. We can't continue if there is no surface created!
#warning "!!! FIX !!! (finished = true)"
//		finished = true;
	}

	if (vjs.useOpenGL)
	{
		// Recreate the texture because of the NTSC <-> PAL screen resize.
//Not sure why this is here...
//Is it because of the resized surface up above?
		sdlemu_create_texture(surface, mainSurface, vjs.glFilter, 0);
	}
	else
	{
		mainSurface = SDL_SetVideoMode(width, height, 32, mainSurfaceFlags);

		if (mainSurface == NULL)
		{
			WriteLog("Video: SDL is unable to set the video mode: %s\n", SDL_GetError());
// Don't exit because we can't resize!
//			exit(1);
		}
	}

	char window_title[64];

	sprintf(window_title, "Virtual Jaguar (%i x %i)", (int)width, (int)height);
	SDL_WM_SetCaption((vjs.useOpenGL ? "Virtual Jaguar (OpenGL)" : window_title), "Virtual Jaguar");
}

//
// Return the screen's width in pixels
//
uint32 GetSDLScreenWidthInPixels(void)
{
	return surface->pitch / 4;						// Pitch / 4 since we're in 32BPP mode
}

//
// Fullscreen <-> window switching
//
void ToggleFullscreen(void)
{
	// Set our internal variable, then toggle the SDL flag
	vjs.fullscreen = !vjs.fullscreen;
	mainSurfaceFlags ^= SDL_FULLSCREEN;
//	mainSurfaceFlags = (vjs.fullscreen ? mainSurfaceFlags | SDL_FULLSCREEN :
//		mainSurfaceFlags & ~SDL_FULLSCREEN);

//	mainSurfaceFlags &= ~SDL_FULLSCREEN;

//	if (vjs.fullscreen)
//		mainSurfaceFlags |= SDL_FULLSCREEN;

	if (vjs.useOpenGL)
	{
		// When OpenGL is used, we're going to use a standard resolution of 640x480.
		// This way we have good scaling functionality and when the screen is resized
		// because of the NTSC <-> PAL resize, we only have to re-create the texture
		// instead of initializing the entire OpenGL texture en screens.
		mainSurface = SDL_SetVideoMode(640, 480, 32, mainSurfaceFlags);

		// Reset viewport, etc.
		glViewport(0, 0, mainSurface->w, mainSurface->h);
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		glOrtho(0.0, (GLdouble)mainSurface->w, (GLdouble)mainSurface->h, 0.0, 0.0, 1.0);
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();
	}
	else
		mainSurface = SDL_SetVideoMode(VIRTUAL_SCREEN_WIDTH,
			(vjs.hardwareTypeNTSC ? VIRTUAL_SCREEN_HEIGHT_NTSC : VIRTUAL_SCREEN_HEIGHT_PAL),
			32, mainSurfaceFlags);

	if (mainSurface == NULL)
	{
		WriteLog("Video: SDL was unable to switch the video to %s: %s\n", (vjs.fullscreen ? "fullscreen" : "windowed"), SDL_GetError());
// Shouldn't exit because we can't switch! BAD!!!
//		exit(1);
		return;
	}

	SDL_WM_SetCaption((vjs.useOpenGL ? "Virtual Jaguar (OpenGL)" : "Virtual Jaguar"), "Virtual Jaguar");

	return;
}
