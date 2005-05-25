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

#ifndef __SDLEMU_OPENGL_H__
#define __SDLEMU_OPENGL_H__

#include "SDL.h"
#include "SDL_opengl.h"

#ifdef __cplusplus
extern "C" {
#endif

void sdlemu_init_opengl(SDL_Surface * src, SDL_Surface * dst, int texturetype, int filter, int src_bpp);
void sdlemu_draw_texture(SDL_Surface * dst, SDL_Surface * src, int texturetype);
void sdlemu_close_opengl(void);
void sdlemu_create_texture(SDL_Surface * src, SDL_Surface * dst, int filter, int src_bpp);
void * sdlemuGetOverlayPixels(void);
Uint32 sdlemuGetOverlayWidthInPixels(void);
void sdlemuEnableOverlay(void);
void sdlemuDisableOverlay(void);

#ifdef __cplusplus
}
#endif

#endif
