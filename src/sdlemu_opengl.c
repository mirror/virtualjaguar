/*
 * SDLEMU library - Free sdl related functions library
 * Copyrigh(c) 1999-2004 sdlemu development crew
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/*  SDLEMU_OPENGL.C
    SDLEMU related sources for using OpenGL with SDL.
    By Niels Wagenaar | http://sdlemu.ngemu.com | shalafi@xs4all.nl

    Version 1.0.001 - 4-10-2004

    - Added support for 16, 24 and 32 bit textures;
    - Added support for 16, 24 and 32 bit texture rendering;

    Version 1.0.002 - 6-10-2004

    - Cleaned up a lot of code and removed non functional and obsolete code;
    - Removed sdlemu_resize_texture function because of double code;
    - Removed the texture creation from the sdlemu_init_opengl;
    - Added sdlemu_create_texture function to replace the sdlemu_resize_texture function
      and the texture creation in sdlemu_init_opengl;
    - Added the usage of OPENGL_16BPP_CORRECT_RGBA for activating the correct 16bpp RGBA masks;
    - Added the usage of WANT_OPENGL_ALPHA for using ALPHA blending with 32bpp textures;
    - Added automatic and override texture bpp depth setting (based upon the src surface);

*/

#include "sdlemu_opengl.h"

#include "log.h"

// We want alpha on our OpenGL contexts...!
// Or do we? Seems to kill performance on X...
// Or does it? Could it be bad blitter performance?
#define WANT_OPENGL_ALPHA

static SDL_Surface *texture      = 0;
static GLuint       texid        = 0;
static GLfloat      texcoord[4];
static unsigned int glFilter;
static unsigned int texturebpp  = 0; // 16, 24 or 32 bpp

static SDL_Surface * overlay = 0;
static GLuint overlayID = 0;
static GLfloat overlayCoord[4];
void sdlemu_create_overlay(SDL_Surface * dst, int src_bpp);

static int showOverlay = 0;

static inline int power_of_two(int input)
{
	int value = 1;

	while (value < input)
		value <<= 1;

	return value;
}

void sdlemu_init_opengl(SDL_Surface * src, SDL_Surface * dst, int texturetype, int filter, int src_bpp)
{
	WriteLog("\nOpenGL driver information :\n");
	WriteLog("\n");
	WriteLog("Vendor:             %s\n", glGetString(GL_VENDOR));
	WriteLog("Renderer:           %s\n", glGetString(GL_RENDERER));
	WriteLog("Version:            %s\n", glGetString(GL_VERSION));
	WriteLog("OpenGL drawmethod: ");

	switch (texturetype)
	{
	case 1:
		WriteLog("GL_QUAD rendering\n\n");
		break;
	default:
		WriteLog("GL_TRIANGLE rendering\n\n");
		break;
	}

	glFilter = filter;

	// Let us create the texture information :
	sdlemu_create_texture(src, dst, filter, src_bpp);
	sdlemu_create_overlay(dst, src_bpp);
}

void sdlemu_draw_texture(SDL_Surface * dst, SDL_Surface * src, int texturetype)
{
/*
	This is needed when we want to render OpenGL textures with the Alpha mask set.
	Be warned! This only works with the bpp of texture en *src set to 32.
*/
#ifdef WANT_OPENGL_ALPHA
	Uint32 saved_flags;
	Uint8  saved_alpha;

	/* Save the alpha blending attributes */
	saved_flags = src->flags&(SDL_SRCALPHA|SDL_RLEACCELOK);
	saved_alpha = src->format->alpha;
	if ( (saved_flags & SDL_SRCALPHA) == SDL_SRCALPHA ) {
		SDL_SetAlpha(src, 0, 0);
	}

	// Blit the src display to the texture.
	SDL_BlitSurface(src, NULL, texture, NULL);

	/* Restore the alpha blending attributes */
	if ( (saved_flags & SDL_SRCALPHA) == SDL_SRCALPHA ) {
		SDL_SetAlpha(src, saved_flags, saved_alpha);
	}
#else
	SDL_BlitSurface(src, NULL, texture, NULL);
#endif
//	SDL_BlitSurface(src, NULL, overlay, NULL);
/*Uint32 * pix = (Uint32 *)overlay->pixels;
Uint32 y,x;
for(y=10; y<200; y++)
for(x=30; x<250; x++)
pix[x+(y*1024)] = 0x800000FF;//*/

glBlendFunc(GL_ONE, GL_ZERO);
glBindTexture(GL_TEXTURE_2D, texid);
	// Texturemap complete texture to surface so we have free scaling
	// and antialiasing
	switch (texturebpp)
	{
	case 16:
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, texture->w, texture->h,
			GL_RGB, GL_UNSIGNED_SHORT_5_6_5, texture->pixels);
		break;
	case 24:
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, texture->w, texture->h,
			GL_RGB, GL_UNSIGNED_BYTE, texture->pixels);
		break;
	case 32:
	default:
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, texture->w, texture->h,
			GL_RGBA, GL_UNSIGNED_BYTE, texture->pixels);
		break;
	}

	// Render the texture to the screen using OpenGL!
	switch (texturetype)
	{
	case 1:
		glBegin(GL_QUADS);
			glTexCoord2f(texcoord[0], texcoord[1]);
			glVertex2f(0, 0);
			glTexCoord2f(texcoord[2], texcoord[1]);
			glVertex2f(dst->w, 0);
			glTexCoord2f(texcoord[2], texcoord[3]);
			glVertex2f(dst->w, dst->h);
			glTexCoord2f(texcoord[0], texcoord[3]);
			glVertex2f(0, dst->h);
		glEnd();

	default:
		glBegin(GL_TRIANGLE_STRIP);
			glTexCoord2f(texcoord[0], texcoord[1]); glVertex3i(0, 0, 0);
			glTexCoord2f(texcoord[2], texcoord[1]); glVertex3i(dst->w, 0, 0);
			glTexCoord2f(texcoord[0], texcoord[3]); glVertex3i(0, dst->h, 0);
			glTexCoord2f(texcoord[2], texcoord[3]); glVertex3i(dst->w, dst->h, 0);
		glEnd();
	}//*/

	if (showOverlay)
	{
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glBindTexture(GL_TEXTURE_2D, overlayID);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, overlay->w, overlay->h, GL_RGBA, GL_UNSIGNED_BYTE, overlay->pixels);
		glBegin(GL_QUADS);
			glTexCoord2f(overlayCoord[0], overlayCoord[1]);
			glVertex2f(0, 0);
			glTexCoord2f(overlayCoord[2], overlayCoord[1]);
			glVertex2f(dst->w, 0);
			glTexCoord2f(overlayCoord[2], overlayCoord[3]);
			glVertex2f(dst->w, dst->h);
			glTexCoord2f(overlayCoord[0], overlayCoord[3]);
			glVertex2f(0, dst->h);
		glEnd();
	}

//Do some OpenGL stuff here...
//Doesn't work...
/*unsigned long int map[25] = {
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF
};
glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
glRasterPos2i(10, 10);
glDrawPixels(5, 5, GL_RGBA, GL_UNSIGNED_INT, map);//*/

//  glFlush();
	SDL_GL_SwapBuffers();
//	glFinish();
}

void sdlemu_close_opengl(void)
{
	if (texture)
		SDL_FreeSurface(texture);

	if (overlay)
		SDL_FreeSurface(overlay);
}

void sdlemu_create_texture(SDL_Surface * src, SDL_Surface * dst, int filter, int src_bpp)
{
    // Local variables.
	int w , h;                         // w and h contain the width and height of the OpenGL texture.
	Uint32 rmask, gmask, bmask, amask; // Needed for creating RGBA masks.
	int bpp;

	// Delete old texture (if allocated). Useful when there is a screen resize.
	if (texture)
		SDL_FreeSurface(texture);

	// Texture width/height should be power of 2 of the SDL_Surface *src when using OpenGL.
	// So, find the largest power of two that will contain both the width and height
	w = power_of_two(src->w);
	h = power_of_two(src->h);

	WriteLog("OpenGL - Texture size : %d x %d\n", w, h);

	// Setting bpp based upon src_bpp.
	bpp = src_bpp;

	// We allow the developer to set its own texture bpp. But if the value is NULL or
	// not equal to 16, 24 or 32, we make the texturebpp the same as the BPP from src.
	if (bpp == 16 || bpp == 24 || bpp == 32)
		texturebpp = bpp;
	else
		texturebpp = src->format->BitsPerPixel;

	WriteLog("OpenGL - Texture depth : %d bpp\n", texturebpp);

	// Now were are going to create a SDL_Surface named texture. This will be our surface
	// which will function as a buffer between the SDL_Surface *src and SDL_Surface *dst.
	// This buffer is needed because we need to convert the SDL_Surface *src to an OpenGL
	// texture with a depth of 16, 24 or 32 bpp, before we can blit the pixels to *dst
	// using OpenGL.
	//
	// NOTE: Seems the byte order here *is* important!
	switch (texturebpp)
	{
	case 16: // *src has depth of 16 bpp
/*
	According to information on the SDL mailinglist and on internet, the following
	rgba masks should be the ones to use. But somehow the screen gets f*cked up and
	the RGB colours are incorrect (at least in Virtual Jaguar/SDL).

	Compile with -DOPENGL_16BPP_CORRECT_RGBA to use this RGBA values.
*/
#ifdef OPENGL_16BPP_CORRECT_RGBA
		rmask = 0x7C00;
		gmask = 0x03E0;
		bmask = 0x001F;
		amask = 0x0000;
#else
		rmask = 0x0000;
		gmask = 0x0000;
		bmask = 0x0000;
		amask = 0x0000;
#endif
		break;
	case 24: // *src has depth of 24 bpp
	#if SDL_BYTEORDER == SDL_BIG_ENDIAN
		rmask = 0x00FF0000;
		gmask = 0x0000FF00;
		bmask = 0x000000FF;
		amask = 0x00000000; // IMPORTANT! 24 bpp doesn't use Alpha (at least in our case).
	#else
		rmask = 0x000000FF;
		gmask = 0x0000FF00;
		bmask = 0x00FF0000;
		amask = 0x00000000; // IMPORTANT! 24 bpp doesn't use Alpha (at least in our case).
	#endif
		break;
	case 32: //*src has depth of 32 bpp
	default: //which is also the default.
	#if SDL_BYTEORDER == SDL_BIG_ENDIAN
		rmask = 0xFF000000;
		gmask = 0x00FF0000;
		bmask = 0x0000FF00;
		amask = 0x000000FF;
	#else
		rmask = 0x000000FF;
		gmask = 0x0000FF00;
		bmask = 0x00FF0000;
		amask = 0xFF000000;
	#endif
		break;
	}

	// Creating SDL_Surface texture based upon the above settings.
	texture = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, texturebpp, rmask, gmask, bmask, amask);

	if (texture == NULL)
	{
		WriteLog("sdlemu_opengl: Could not create texture surface! (SDL: %s)\n", SDL_GetError());
	}

	// Setting up OpenGL
	glDisable(GL_FOG);
	glDisable(GL_LIGHTING);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
//	glDisable(GL_BLEND);
	glEnable(GL_BLEND);
	glDisable(GL_NORMALIZE);
	glDisable(GL_ALPHA_TEST);
	glEnable(GL_TEXTURE_2D);
//	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
//	glBlendFunc(GL_ONE, GL_SRC_ALPHA);
//This works, but in a wrong way...
//	glBlendFunc(GL_ONE, GL_ONE);

	// Definitely needed for screen resolution larger then the *src.
	// This way we can have automatic scaling functionality.
	glViewport(0, 0, dst->w, dst->h);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0.0, (GLdouble)dst->w, (GLdouble)dst->h, 0.0, 0.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	// Setting up the texture coordinates.
	texcoord[0] = 0.0f;
	texcoord[1] = 0.0f;
	texcoord[2] = (GLfloat)(src->w) / texture->w;
	texcoord[3] = (GLfloat)(src->h) / texture->h;

	// create a RGB(A) texture for the texture surface
	glGenTextures(1, &texid);
	glBindTexture(GL_TEXTURE_2D, texid);

	// Setting up the OpenGL Filters. These filters are important when we/you
	// want to scale the texture.
	if (filter)
	{
		// Textures are rendered in best quality.
		WriteLog("OpenGL filters: enabled\n");
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	}
	else
	{
		// Textures are rendered in normal quality.
		WriteLog("OpenGL filters: disabled\n");
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}

	// Setting texture mode.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	// Generate the texture using the above information.
	switch (texturebpp)
	{
	case 16:
		// Normal 16bpp depth based textures consist out of GL_RGB5 and doesn't have support for Alpha channels.
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB5, texture->w, texture->h, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		break;
	case 24:
		// The 24bpp depth based textures consist out of GL_RGB8 and doesn't have support for Alpha channels.
		//
		// IMPORTANT : If you don't use Alpha. Use textures with a depth of 16bpp.
		//             If you use Alpha. Use textures with a depth of 32bpp.
		//             24bpp textures are SLOW and avoid them at all costs!
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, texture->w, texture->h, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		break;
	case 32:
	default:
		// The 32bpp depth based textures consist out of GL_RGBA8 and has support for Alpha channels.
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, texture->w, texture->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		break;
	}
}

void sdlemu_create_overlay(SDL_Surface * dst, int src_bpp)
{
    // Local variables.
	Uint32 rmask, gmask, bmask, amask; // Needed for creating RGBA masks.

	// Delete old texture (if allocated). Useful when there is a screen resize.
	if (overlay)
		SDL_FreeSurface(overlay);

	// Texture width/height should be power of 2 of the SDL_Surface *src when using OpenGL.
	// So, find the largest power of two that will contain both the width and height
	int w = power_of_two(dst->w);
	int h = power_of_two(dst->h);

	WriteLog("OpenGL - Overlay size : %d x %d\n", w, h);

	// Setting bpp based upon src_bpp.
	int bpp = src_bpp;

	// We allow the developer to set its own texture bpp. But if the value is NULL or
	// not equal to 16, 24 or 32, we make the texturebpp the same as the BPP from src.
	if (bpp == 16 || bpp == 24 || bpp == 32)
		texturebpp = bpp;
	else
		texturebpp = dst->format->BitsPerPixel;

	WriteLog("OpenGL - Overlay depth : %d bpp\n", texturebpp);

	// Now were are going to create a SDL_Surface named texture. This will be our surface
	// which will function as a buffer between the SDL_Surface *src and SDL_Surface *dst.
	// This buffer is needed because we need to convert the SDL_Surface *src to an OpenGL
	// texture with a depth of 16, 24 or 32 bpp, before we can blit the pixels to *dst
	// using OpenGL.
	//
	// NOTE: Seems the byte order here *is* important!
	switch (texturebpp)
	{
	case 16: // *src has depth of 16 bpp
/*
	According to information on the SDL mailinglist and on internet, the following
	rgba masks should be the ones to use. But somehow the screen gets f*cked up and
	the RGB colours are incorrect (at least in Virtual Jaguar/SDL).

	Compile with -DOPENGL_16BPP_CORRECT_RGBA to use this RGBA values.
*/
#ifdef OPENGL_16BPP_CORRECT_RGBA
		rmask = 0x7C00;
		gmask = 0x03E0;
		bmask = 0x001F;
		amask = 0x0000;
#else
		rmask = 0x0000;
		gmask = 0x0000;
		bmask = 0x0000;
		amask = 0x0000;
#endif
		break;
	case 24: // *src has depth of 24 bpp
	#if SDL_BYTEORDER == SDL_BIG_ENDIAN
		rmask = 0x00FF0000;
		gmask = 0x0000FF00;
		bmask = 0x000000FF;
		amask = 0x00000000; // IMPORTANT! 24 bpp doesn't use Alpha (at least in our case).
	#else
		rmask = 0x000000FF;
		gmask = 0x0000FF00;
		bmask = 0x00FF0000;
		amask = 0x00000000; // IMPORTANT! 24 bpp doesn't use Alpha (at least in our case).
	#endif
		break;
	case 32: //*src has depth of 32 bpp
	default: //which is also the default.
	#if SDL_BYTEORDER == SDL_BIG_ENDIAN
		rmask = 0xFF000000;
		gmask = 0x00FF0000;
		bmask = 0x0000FF00;
		amask = 0x000000FF;
	#else
		rmask = 0x000000FF;
		gmask = 0x0000FF00;
		bmask = 0x00FF0000;
		amask = 0xFF000000;
	#endif
		break;
	}

	// Creating SDL_Surface texture based upon the above settings.
	overlay = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, texturebpp, rmask, gmask, bmask, amask);

	if (overlay == NULL)
	{
		WriteLog("sdlemu_opengl: Could not create overlay surface! (SDL: %s)\n", SDL_GetError());
	}

	// Setting up the texture coordinates.
	overlayCoord[0] = 0.0f;
	overlayCoord[1] = 0.0f;
	overlayCoord[2] = (GLfloat)(dst->w) / overlay->w;
	overlayCoord[3] = (GLfloat)(dst->h) / overlay->h;

	// create a RGB(A) texture for the texture surface
	glGenTextures(1, &overlayID);
	glBindTexture(GL_TEXTURE_2D, overlayID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	// Setting texture mode.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	// Generate the texture using the above information.
	switch (texturebpp)
	{
	case 16:
		// Normal 16bpp depth based textures consist out of GL_RGB5 and doesn't have support for Alpha channels.
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB5, overlay->w, overlay->h, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		break;
	case 24:
		// The 24bpp depth based textures consist out of GL_RGB8 and doesn't have support for Alpha channels.
		//
		// IMPORTANT : If you don't use Alpha. Use textures with a depth of 16bpp.
		//             If you use Alpha. Use textures with a depth of 32bpp.
		//             24bpp textures are SLOW and avoid them at all costs!
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, overlay->w, overlay->h, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		break;
	case 32:
	default:
		// The 32bpp depth based textures consist out of GL_RGBA8 and has support for Alpha channels.
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, overlay->w, overlay->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		break;
	}
}

void * sdlemuGetOverlayPixels(void)
{
	return overlay->pixels;
}

Uint32 sdlemuGetOverlayWidthInPixels(void)
{
	return overlay->pitch / 4;
}

void sdlemuEnableOverlay(void)
{
	showOverlay = 1;
}

void sdlemuDisableOverlay(void)
{
	showOverlay = 0;
}
