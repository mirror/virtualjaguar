//
// Some simple emulation classes to get PTC code running on SDL
//
// by cal16
// GCC/SDL port by Niels Wagenaar (Linux/WIN32) and Caz (BeOS)
// Cleanups in some classes and code in general by James L. Hammons
//

#ifndef __SDLPTC_H__
#define __SDLPTC_H__

#include "SDL.h"
#ifdef __PORT__
#include <string.h>
#include <stdlib.h>
#endif	// #ifdef __PORT__

#include "types.h"

#ifndef __PORT__
#define randomize()	srand(time(NULL))
#define random(max)	(rand() % (max))
#endif	// #ifndef __PORT__

class Error
{
	public:
		Error(const char * msg)	{ strcpy(message, msg); }
		void report(void)		{ printf("Error: %s\n", message); }

	private:
		char message[1024];
};

class Area
{
	public:
		Area(int ll, int tt, int rr, int bb): l(ll), t(tt), r(rr), b(bb) {}
		int left(void) const	{ return l; }
		int right(void) const	{ return r; }
		int top(void) const		{ return t; }
		int bottom(void) const	{ return b; }
		int width(void) const	{ return r - l; }
		int height(void) const	{ return b - t; }

	private:
		int l, t, r, b;
};

	
class Format
{
	public:
		Format(int pBpp, int r=0, int g=0, int b=0): bpp(pBpp), maskR(r), maskG(g), maskB(b) {}
		Uint8 BPP(void) const		{ return bpp; }
		Uint32 MaskR(void) const	{ return maskR; }
		Uint32 MaskG(void) const	{ return maskG; }
		Uint32 MaskB(void) const	{ return maskB; }

	private:
		Uint8 bpp;
		Uint32 maskR, maskG, maskB;
};

class Surface
{
	public:
	    Surface(int w, int h, const Format &format)
	    {
			surface = SDL_AllocSurface(SDL_SWSURFACE, w, h, format.BPP(),
					format.MaskR(), format.MaskG(), format.MaskB(), 0);
			if (surface == NULL)
			{
				throw Error(SDL_GetError());
			}
			nupdates = 0;
			is_console = 0;
		}
		Surface(void)
		{
			nupdates = 0;
			is_console = 1;
		}
		~Surface()
		{
			if (!is_console)
				SDL_FreeSurface(surface);
		}

		virtual int width(void)		{ return surface->w; }
		virtual int height(void)	{ return surface->h; }
		virtual int pitch(void)		{ return surface->pitch; }
		virtual void palette(int32 * pcolors)
		{
			SDL_Color colors[256];

			for(int i=0; i<256; ++i)
				colors[i].r = (pcolors[i] >> 16) & 0xFF,
				colors[i].g = (pcolors[i] >> 8) & 0xFF,
				colors[i].b = (pcolors[i] >> 0) & 0xFF;
			SDL_SetColors(surface, colors, 0, 256);
		}

		virtual void * lock(void)
		{
			if (SDL_MUSTLOCK(surface))
				while (SDL_LockSurface(surface) < 0)
					SDL_Delay(10);

			return (Uint8 *)surface->pixels;
		}

		virtual void unlock(void)
		{
			if (SDL_MUSTLOCK(surface))
				SDL_UnlockSurface(surface);
		}
	
		virtual void copy(Surface &dst,	const Area &srcarea, const Area &dstarea)
		{
			SDL_Rect srcrect, dstrect;
			srcrect.x = srcarea.left();
			srcrect.y = srcarea.top();
			srcrect.w = srcarea.width();
			srcrect.h = srcarea.height();
			dstrect.x = dstarea.left();
			dstrect.y = dstarea.top();
			dstrect.w = dstarea.width();
			dstrect.h = dstarea.height();
			SDL_BlitSurface(surface, &srcrect, dst.surface, &dstrect);
			dst.updates[dst.nupdates++] = dstrect;
		}
		virtual void copy(Surface &dst)
		{
			SDL_Rect srcrect, dstrect;
			srcrect.x = 0;
			srcrect.y = 0;
			srcrect.w = surface->w;
			srcrect.h = surface->h;
			dstrect.x = 0;
			dstrect.y = 0;
			dstrect.w = surface->w;
			dstrect.h = surface->h;
			SDL_LowerBlit(surface, &srcrect, dst.surface, &dstrect);
			dst.updates[dst.nupdates++] = dstrect;
		}

		virtual void update(void)
		{
			/* Added/Changed by SDLEMU (http://sdlemu.ngemu.com) */
			/* SDL_Flip is infact the same as SDL_Blitsurface but
				if SDL_DOUBLEBUF | SDL_HWSURFACE is available, it 
				makes use of it. If not is uses SDL_Blitsurface */
		    SDL_Flip(surface);      
			nupdates = 0;
		}

	public:
		SDL_Surface * surface;

	protected:
		int nupdates;
		SDL_Rect updates[1];		/* Definitely increase this.. */
		int is_console;
};

class Console: public Surface
{
	public:
		Console(): Surface(), fullscreen(0), nJoystick(0) {}
		~Console()				{ SDL_Quit(); }

		/* Added/changed by SDLEMU (http://sdlemu.ngemu.com) */
		/* We need to close several items in SDL because of memory leaks
			and core dumps and stuff :) */
		void close(void)
		{
			SDL_JoystickClose(joystick);
			SDL_FreeSurface(surface);
			SDL_QuitSubSystem(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_AUDIO | SDL_INIT_TIMER);
			SDL_Quit();
		}
		void option(char * option)
		{
			if (!stricmp(option, "fullscreen output"))
				fullscreen = 1;
			else if (!stricmp(option, "windowed output"))
				fullscreen = 0;
			/* Added/changed by SDLEMU (http://sdlemu.ngemu.com) */
			else if (!stricmp(option, "joystick enabled"))
				nJoystick = 1;
			else if (!stricmp(option, "joystick disabled"))
				nJoystick = 0;
		}
	
		/* The following was changed/added by SDLEMU (http://sdlemu.ngemu.com) */
	
		void open(const char * title, int width, int height, const Format &format)
		{
			Uint32 flags = SDL_SWSURFACE;

//Testing			if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_AUDIO | SDL_INIT_TIMER) < 0)
			if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_AUDIO | SDL_INIT_TIMER | SDL_INIT_NOPARACHUTE) < 0)
				throw Error(SDL_GetError());

		    const SDL_VideoInfo * info = SDL_GetVideoInfo(); // Let us get proper info about the platform.

			if (!info)
			{
				fprintf(stderr, "SDL is unable to get the video query : %s\n", SDL_GetError());
				exit(1);
			}

			if (info->hw_available)
				flags = SDL_HWSURFACE | SDL_HWPALETTE | SDL_DOUBLEBUF;
		
			if (info->blit_hw)
				flags |= SDL_HWACCEL;

			if (fullscreen)
				flags |= SDL_FULLSCREEN;

			surface = SDL_SetVideoMode(width, height, 16, flags);

			if (surface == NULL)
				throw Error(SDL_GetError());
		
	        SDL_WM_SetCaption(title, title);
	    }

		int key(void)
		{
			SDL_Event event;
			int keyevent = 0;

			while (SDL_PollEvent(&event))
			{
				/* Real key events trigger this function */
				if (event.type == SDL_KEYDOWN)
					keyevent = 1;

				/* So do quit events -- let the app know about it */
				if (event.type == SDL_QUIT)
					keyevent = 1;
			}
			return keyevent;
		}

		int JoyEnabled(void)	{ return (nJoystick == 1 ? 1 : 0); }

	public:
		SDL_Joystick * joystick;

	private:
		int fullscreen;
		int nJoystick;
};

#endif	// #ifndef __SDLPTC_H__
