/*
 * SDLEMU library - Free sdl related functions library
 * Copyrigh(c) 1999-2002 sdlemu development crew
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

#include "sdlemu_opengl.h"

static SDL_Surface *texture      = 0;
static GLuint       texid        = 0;
static GLfloat      texcoord[4];

static inline int power_of_two(int input)
{
	int value = 1;

	while (value < input)
		value <<= 1;

	return value;
}

void sdlemu_init_opengl(SDL_Surface * src, int texturetype, int size, int glfilter)
{
	int w, h;

	printf("\nOpenGL driver information :\n");
	printf("\n");
	printf("Vendor            : %s\n", glGetString(GL_VENDOR));
	printf("Renderer          : %s\n", glGetString(GL_RENDERER));
	printf("Version           : %s\n", glGetString(GL_VERSION));
	printf("OpenGL drawmethod : ");

	switch (texturetype)
	{
	case 1:
		printf("GL_QUAD rendering\n\n");
		break;
	default:
		printf("GL_TRIANGLE rendering\n\n");
		break;
	}

	// Texture width/height should be power of 2
	// So, find the largest power of two that will contain both the width and height
	w = power_of_two(src->w);
	h = power_of_two(src->h);

	// create texture surface
	texture = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, 32,
	#if SDL_BYTEORDER == SDL_LIL_ENDIAN 
		0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
	#else
		0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
	#endif

	// setup 2D gl environment
	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glEnable(GL_TEXTURE_2D);

	glViewport(0, 0, src->w * size, src->h * size);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

	glOrtho(0.0, (GLdouble)(src->w * size), (GLdouble)(src->h * size), 0.0, 0.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	// texture coordinates
	texcoord[0] = 0.0f;
	texcoord[1] = 0.0f;
	texcoord[2] = (GLfloat)(src->w) / texture->w;
	texcoord[3] = (GLfloat)(src->h) / texture->h;

	// create an RGBA texture for the texture surface
	glGenTextures(1, &texid);
	glBindTexture(GL_TEXTURE_2D, texid);
	
	if (glfilter)
	{
       printf("OpenGL filters : enabled\n");
	   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	}
	else
	{
       printf("OpenGL filters : disabled\n");
	   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    }

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture->w, texture->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture->pixels);
}

void sdlemu_draw_texture(SDL_Surface * dst, SDL_Surface * src, int texturetype)
{

	// convert color-indexed surface to RGB texture
	SDL_BlitSurface(src, NULL, texture, NULL);

	// Texturemap complete texture to surface so we have free scaling 
	// and antialiasing 
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, texture->w, texture->h,
		GL_RGBA, GL_UNSIGNED_BYTE, texture->pixels);
	
	switch (texturetype)
	{
	case 1:
        glBegin(GL_QUADS);
                glTexCoord2f(texcoord[0], texcoord[1]);
                glVertex2f(0, 0);
                glTexCoord2f(texcoord[2], texcoord[1]);
                glVertex2f(dst->w , 0);
                glTexCoord2f(texcoord[2], texcoord[3]);
                glVertex2f(dst->w , dst->h );
                glTexCoord2f(texcoord[0], texcoord[3]);
                glVertex2f(0, dst->h );
        glEnd();      

	default:
		glBegin(GL_TRIANGLE_STRIP);
				glTexCoord2f(texcoord[0], texcoord[1]); glVertex3i(0, 0, 0);
				glTexCoord2f(texcoord[2], texcoord[1]); glVertex3i(dst->w, 0, 0);
				glTexCoord2f(texcoord[0], texcoord[3]); glVertex3i(0, dst->h, 0);
				glTexCoord2f(texcoord[2], texcoord[3]); glVertex3i(dst->w, dst->h, 0);
		glEnd();
	}

	SDL_GL_SwapBuffers();    
}

void sdlemu_close_opengl(void)
{
	if (texture)
		SDL_FreeSurface(texture);
}


//
// Resize the texture
// This should honor the glFilter flag that is passed in to the initialization code,
// but, at the moment, it doesn't...
//
void sdlemu_resize_texture(SDL_Surface * src, SDL_Surface * dst)
{
	// Texture width/height should be power of 2
	// So, find the largest power of two that will contain both the width and height
	int w = power_of_two(src->w), h = power_of_two(src->h);

	// Delete old texture (if allocated)
	if (texture)
		SDL_FreeSurface(texture);

	// create texture surface
//NOTE: Seems the byte order here *is* important! (Perhaps only for 32 BPP?)
	texture = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, 32,
	#if SDL_BYTEORDER == SDL_LIL_ENDIAN 
		0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
	#else
		0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
	#endif

	// setup 2D gl environment
	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glEnable(GL_TEXTURE_2D);

	glViewport(0, 0, dst->w, dst->h);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

	glOrtho(0.0, (GLdouble)dst->w, (GLdouble)dst->h, 0.0, 0.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	// texture coordinates
	texcoord[0] = 0.0f, texcoord[1] = 0.0f,
	texcoord[2] = (GLfloat)(src->w) / texture->w,
	texcoord[3] = (GLfloat)(src->h) / texture->h;

	// create an RGBA texture for the texture surface
	glGenTextures(1, &texid);
	glBindTexture(GL_TEXTURE_2D, texid);
	
/*	if (glfilter)
	{
       printf("OpenGL filters : enabled\n");
	   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	}
	else*/
	{
       printf("OpenGL filters : disabled\n");
	   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    }

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture->w, texture->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture->pixels);
}
