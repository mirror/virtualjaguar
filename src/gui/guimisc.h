//
// Miscellaneous GUI utility functions
//
// by James L. Hammons
//

#ifndef __GUIMISC_H__
#define __GUIMISC_H__

#include <string>
#include "types.h"

int stringCmpi(const std::string &, const std::string &);
//void DrawTransparentBitmap(uint32 * screen, uint32 x, uint32 y, uint32 * bitmap, uint8 * alpha = NULL);
void DrawTransparentBitmapDeprecated(uint32 * screen, uint32 x, uint32 y, uint32 * bitmap);
void DrawTransparentBitmap(uint32 * screen, uint32 x, uint32 y, const void * bitmap);
void DrawBitmap(uint32 * screen, uint32 x, uint32 y, const void * bitmap);
//Should call this FillScreenRectangle with a number representing the RGBA value to fill. !!! FIX !!!
//void ClearScreenRectangle(uint32 * screen, uint32 x, uint32 y, uint32 w, uint32 h);
void FillScreenRectangle(uint32 * screen, uint32 x, uint32 y, uint32 w, uint32 h, uint32 color);
void DrawStringTrans(uint32 * screen, uint32 x, uint32 y, uint32 color, uint8 opacity, const char * text, ...);
void DrawStringOpaque(uint32 * screen, uint32 x, uint32 y, uint32 color1, uint32 color2, const char * text, ...);
void DrawString(uint32 * screen, uint32 x, uint32 y, bool invert, const char * text, ...);
void DrawString2(uint32 * screen, uint32 x, uint32 y, uint32 color, uint8 transparency, const char * text, ...);

// Useful enumerations

enum { WINDOW_CLOSE, MENU_ITEM_CHOSEN };

// This is for font2 (crappy--fix!!!)

#define FONT_WIDTH		8
#define FONT_HEIGHT		16

// And this is for the various closeBox stuff--crappy--fix!!!

#define CLOSEBOX_WIDTH		15
#define CLOSEBOX_HEIGHT		15

// Okay, this is ugly but works and I can't think of any better way to handle this. So what
// we do when we pass the GIMP bitmaps into a function is pass them as a (void *) and then
// cast them as type (Bitmap *) in order to use them. Yes, it's ugly. Come up with something
// better!

struct Bitmap {
	unsigned int width;
	unsigned int height;
	unsigned int bytesPerPixel;					// 3:RGB, 4:RGBA
	unsigned char pixelData[];
};

// GUI bitmaps/fonts (exported)

extern char font1[];
extern char font2[];

extern uint32 mousePic[];
extern uint32 upArrowBox[];
extern uint32 downArrowBox[];
extern uint32 pushButtonUp[];
extern uint8 pbuAlpha[];
extern uint32 pushButtonDown[];
extern uint8 pbdAlpha[];
extern uint32 slideSwitchUp[];
extern uint32 slideSwitchDown[];
extern uint32 closeBox[];
extern uint32 closeBoxHover[];
extern uint32 closeBoxDown[];

extern uint8 pbUp[];
extern uint8 pbDown[];
extern uint8 vj_title_small[];

#endif	// __GUIMISC_H__
