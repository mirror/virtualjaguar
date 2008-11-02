//
// Miscellaneous GUI utility functions
//
// by James L. Hammons
//

#include "guimisc.h"

#include <stdarg.h>
#include "font1.h"
#include "font14pt.h"
#include "guielements.h"	// Make this go away...
#include "sdlemu_opengl.h"
#include "vj_title_small.c"
//#include "video.h"

//
// Case insensitive string compare function
// Taken straight out of Thinking In C++ by Bruce Eckel. Thanks Bruce!
//

int stringCmpi(const std::string &s1, const std::string &s2)
{
	// Select the first element of each string:
	std::string::const_iterator p1 = s1.begin(), p2 = s2.begin();

	while (p1 != s1.end() && p2 != s2.end())		// Donï¿½t run past the end
	{
		if (toupper(*p1) != toupper(*p2))			// Compare upper-cased chars
			return (toupper(*p1) < toupper(*p2) ? -1 : 1);// Report which was lexically greater

		p1++;
		p2++;
	}

	// If they match up to the detected eos, say which was longer. Return 0 if the same.
	return s2.size() - s1.size();
}

//
// Draw text at the given x/y coordinates. Can invert text as well.
//
void DrawString(uint32 * screen, uint32 x, uint32 y, bool invert, const char * text, ...)
{
	char string[4096];
	va_list arg;

	va_start(arg, text);
	vsprintf(string, text, arg);
	va_end(arg);

	uint32 pitch = sdlemuGetOverlayWidthInPixels();//GetSDLScreenWidthInPixels();
	uint32 length = strlen(string), address = x + (y * pitch);

	uint32 color1 = 0x0080FF;
	uint8 nBlue = (color1 >> 16) & 0xFF, nGreen = (color1 >> 8) & 0xFF, nRed = color1 & 0xFF;
	uint8 xorMask = (invert ? 0xFF : 0x00);

	for(uint32 i=0; i<length; i++)
	{
		uint8 c = string[i];
		uint32 fontAddr = (uint32)(c < 32 ? 0 : c - 32) * FONT_WIDTH * FONT_HEIGHT;

		for(uint32 yy=0; yy<FONT_HEIGHT; yy++)
		{
			for(uint32 xx=0; xx<FONT_WIDTH; xx++)
			{
				uint32 existingColor = *(screen + address + xx + (yy * pitch));

				uint8 eBlue = (existingColor >> 16) & 0xFF,
					eGreen = (existingColor >> 8) & 0xFF,
					eRed = existingColor & 0xFF;

				uint8 trans = font2[fontAddr] ^ xorMask;
				uint8 invTrans = trans ^ 0xFF;

				uint32 bRed = (eRed * invTrans + nRed * trans) / 255,
					bGreen = (eGreen * invTrans + nGreen * trans) / 255,
					bBlue = (eBlue * invTrans + nBlue * trans) / 255;

				*(screen + address + xx + (yy * pitch)) = 0xFF000000 | (bBlue << 16) | (bGreen << 8) | bRed;
				fontAddr++;
			}
		}

		address += FONT_WIDTH;
	}
}

//
// Draw text at the given x/y coordinates, using FG/BG colors.
//
void DrawStringOpaque(uint32 * screen, uint32 x, uint32 y, uint32 color1, uint32 color2, const char * text, ...)
{
	char string[4096];
	va_list arg;

	va_start(arg, text);
	vsprintf(string, text, arg);
	va_end(arg);

	uint32 pitch = sdlemuGetOverlayWidthInPixels();
	uint32 length = strlen(string), address = x + (y * pitch);

	uint8 eBlue = (color2 >> 16) & 0xFF, eGreen = (color2 >> 8) & 0xFF, eRed = color2 & 0xFF,
		nBlue = (color1 >> 16) & 0xFF, nGreen = (color1 >> 8) & 0xFF, nRed = color1 & 0xFF;

	for(uint32 i=0; i<length; i++)
	{
		uint8 c = string[i];
		c = (c < 32 ? 0 : c - 32);
		uint32 fontAddr = (uint32)c * FONT_WIDTH * FONT_HEIGHT;

		for(uint32 yy=0; yy<FONT_HEIGHT; yy++)
		{
			for(uint32 xx=0; xx<FONT_WIDTH; xx++)
			{
				uint8 trans = font2[fontAddr++];
				uint8 invTrans = trans ^ 0xFF;

				uint32 bRed   = (eRed   * invTrans + nRed   * trans) / 255;
				uint32 bGreen = (eGreen * invTrans + nGreen * trans) / 255;
				uint32 bBlue  = (eBlue  * invTrans + nBlue  * trans) / 255;

				*(screen + address + xx + (yy * pitch)) = 0xFF000000 | (bBlue << 16) | (bGreen << 8) | bRed;
			}
		}

		address += FONT_WIDTH;
	}
}

//
// Draw text at the given x/y coordinates with transparency (0 is fully opaque, 32 is fully transparent).
//
void DrawStringTrans(uint32 * screen, uint32 x, uint32 y, uint32 color, uint8 trans, const char * text, ...)
{
	char string[4096];
	va_list arg;

	va_start(arg, text);
	vsprintf(string, text, arg);
	va_end(arg);

	uint32 pitch = sdlemuGetOverlayWidthInPixels();//GetSDLScreenWidthInPixels();
	uint32 length = strlen(string), address = x + (y * pitch);

	for(uint32 i=0; i<length; i++)
	{
		uint32 fontAddr = (uint32)string[i] * 64;

		for(uint32 yy=0; yy<8; yy++)
		{
			for(uint32 xx=0; xx<8; xx++)
			{
				if (font1[fontAddr])
				{
					uint32 existingColor = *(screen + address + xx + (yy * pitch));

					uint8 eBlue = (existingColor >> 16) & 0xFF,
						eGreen = (existingColor >> 8) & 0xFF,
						eRed = existingColor & 0xFF,
//This could be done ahead of time, instead of on each pixel...
						nBlue = (color >> 16) & 0xFF,
						nGreen = (color >> 8) & 0xFF,
						nRed = color & 0xFF;

//This could be sped up by using a table of 5 + 5 + 5 bits (32 levels transparency -> 32768 entries)
//Here we've modified it to have 33 levels of transparency (could have any # we want!)
//because dividing by 32 is faster than dividing by 31...!
					uint8 invTrans = 32 - trans;

					uint32 bRed = (eRed * trans + nRed * invTrans) / 32;
					uint32 bGreen = (eGreen * trans + nGreen * invTrans) / 32;
					uint32 bBlue = (eBlue * trans + nBlue * invTrans) / 32;

					*(screen + address + xx + (yy * pitch)) = 0xFF000000 | (bBlue << 16) | (bGreen << 8) | bRed;
				}

				fontAddr++;
			}
		}

		address += 8;
	}
}

//
// Draw text at the given x/y coordinates, using FG color and overlay alpha blending.
//
void DrawString2(uint32 * screen, uint32 x, uint32 y, uint32 color, uint8 transparency, const char * text, ...)
{
	char string[4096];
	va_list arg;

	va_start(arg, text);
	vsprintf(string, text, arg);
	va_end(arg);

	uint32 pitch = sdlemuGetOverlayWidthInPixels();
	uint32 length = strlen(string), address = x + (y * pitch);

	color &= 0x00FFFFFF;						// Just in case alpha was passed in...

	for(uint32 i=0; i<length; i++)
	{
		uint8 c = string[i];
		c = (c < 32 ? 0 : c - 32);
		uint32 fontAddr = (uint32)c * FONT_WIDTH * FONT_HEIGHT;

		for(uint32 yy=0; yy<FONT_HEIGHT; yy++)
		{
			for(uint32 xx=0; xx<FONT_WIDTH; xx++)
			{
				uint8 fontTrans = font2[fontAddr++];
				uint32 newTrans = (fontTrans * transparency / 255) << 24;
				uint32 pixel = newTrans | color;

				*(screen + address + xx + (yy * pitch)) = pixel;
			}
		}

		address += FONT_WIDTH;
	}
}

//
// Draw "picture"
// Uses zero as transparent color
// Can also use an optional alpha channel
// Alpha channel is now mandatory! ;-)
//
//void DrawTransparentBitmap(int16 * screen, uint32 x, uint32 y, uint16 * bitmap, uint8 * alpha/*=NULL*/)
/*void DrawTransparentBitmap(uint32 * screen, uint32 x, uint32 y, uint32 * bitmap, uint8 * alpha)
{
	uint32 width = bitmap[0], height = bitmap[1];
	bitmap += 2;

//	uint32 pitch = GetSDLScreenPitch() / 2;			// Returns pitch in bytes but we need words...
	uint32 pitch = sdlemuGetOverlayWidthInPixels();//GetSDLScreenWidthInPixels();
	uint32 address = x + (y * pitch);

	for(uint32 yy=0; yy<height; yy++)
	{
		for(uint32 xx=0; xx<width; xx++)
		{
			if (alpha == NULL)
			{
				if (*bitmap && x + xx < pitch)			// NOTE: Still doesn't clip the Y val...
					*(screen + address + xx + (yy * pitch)) = *bitmap;
			}
			else
			{
				uint8 trans = *alpha;
				uint32 color = *bitmap;
				uint32 existingColor = *(screen + address + xx + (yy * pitch));

				uint8 eRed = existingColor & 0xFF,
					eGreen = (existingColor >> 8) & 0xFF,
					eBlue = (existingColor >> 16) & 0xFF,

					nRed = color & 0xFF,
					nGreen = (color >> 8) & 0xFF,
					nBlue = (color >> 16) & 0xFF;

				uint8 invTrans = 255 - trans;
				uint32 bRed = (eRed * trans + nRed * invTrans) / 255;
				uint32 bGreen = (eGreen * trans + nGreen * invTrans) / 255;
				uint32 bBlue = (eBlue * trans + nBlue * invTrans) / 255;

				uint32 blendedColor = 0xFF000000 | bRed | (bGreen << 8) | (bBlue << 16);

				*(screen + address + xx + (yy * pitch)) = blendedColor;

				alpha++;
			}

			bitmap++;
		}
	}
}*/
void DrawTransparentBitmapDeprecated(uint32 * screen, uint32 x, uint32 y, uint32 * bitmap)
{
	uint32 width = bitmap[0], height = bitmap[1];
	bitmap += 2;

	uint32 pitch = sdlemuGetOverlayWidthInPixels();//GetSDLScreenWidthInPixels();
	uint32 address = x + (y * pitch);

	for(uint32 yy=0; yy<height; yy++)
	{
		for(uint32 xx=0; xx<width; xx++)
		{
			uint32 color = *bitmap;
			uint32 blendedColor = color;
			uint32 existingColor = *(screen + address + xx + (yy * pitch));

			if (existingColor >> 24 != 0x00)		// Pixel needs blending
			{
				uint8 trans = color >> 24;
				uint8 invTrans = trans ^ 0xFF;//255 - trans;

				uint8 eRed = existingColor & 0xFF,
					eGreen = (existingColor >> 8) & 0xFF,
					eBlue = (existingColor >> 16) & 0xFF,

					nRed = color & 0xFF,
					nGreen = (color >> 8) & 0xFF,
					nBlue = (color >> 16) & 0xFF;

				uint32 bRed = (eRed * invTrans + nRed * trans) / 255;
				uint32 bGreen = (eGreen * invTrans + nGreen * trans) / 255;
				uint32 bBlue = (eBlue * invTrans + nBlue * trans) / 255;

				blendedColor = 0xFF000000 | bRed | (bGreen << 8) | (bBlue << 16);
			}

			*(screen + address + xx + (yy * pitch)) = blendedColor;
			bitmap++;
		}
	}
}

void DrawTransparentBitmap(uint32 * screen, uint32 x, uint32 y, const void * bitmap)
{
	uint32 pitch = sdlemuGetOverlayWidthInPixels();
	uint32 address = x + (y * pitch);
	uint32 count = 0;

	for(uint32 yy=0; yy<((Bitmap *)bitmap)->height; yy++)
	{
		for(uint32 xx=0; xx<((Bitmap *)bitmap)->width; xx++)
		{
			uint32 color = ((uint32 *)((Bitmap *)bitmap)->pixelData)[count];
			uint32 blendedColor = color;
			uint32 existingColor = *(screen + address + xx + (yy * pitch));

			if (existingColor >> 24 != 0x00)	// Pixel needs blending
			{
				uint8 trans = color >> 24;
				uint8 invTrans = trans ^ 0xFF;

				uint8 eRed = existingColor & 0xFF,
					eGreen = (existingColor >> 8) & 0xFF,
					eBlue = (existingColor >> 16) & 0xFF,

					nRed = color & 0xFF,
					nGreen = (color >> 8) & 0xFF,
					nBlue = (color >> 16) & 0xFF;

				uint32 bRed = (eRed * invTrans + nRed * trans) / 255;
				uint32 bGreen = (eGreen * invTrans + nGreen * trans) / 255;
				uint32 bBlue = (eBlue * invTrans + nBlue * trans) / 255;

// Instead of $FF, should use the alpha from the destination pixel as the final alpha value...
				blendedColor = 0xFF000000 | bRed | (bGreen << 8) | (bBlue << 16);
			}

			*(screen + address + xx + (yy * pitch)) = blendedColor;
			count++;
		}
	}
}

//
// Draw a bitmap without using blending
//
void DrawBitmap(uint32 * screen, uint32 x, uint32 y, const void * bitmap)
{
	uint32 pitch = sdlemuGetOverlayWidthInPixels();
	uint32 address = x + (y * pitch);
	uint32 count = 0;

	for(uint32 yy=0; yy<((Bitmap *)bitmap)->height; yy++)
	{
		for(uint32 xx=0; xx<((Bitmap *)bitmap)->width; xx++)
		{
			*(screen + address + xx + (yy * pitch)) = ((uint32 *)((Bitmap *)bitmap)->pixelData)[count];
			count++;
		}
	}
}

//
// Fill a portion of the screen with the passed in color
//
void FillScreenRectangle(uint32 * screen, uint32 x, uint32 y, uint32 w, uint32 h, uint32 color)
//void ClearScreenRectangle(uint32 * screen, uint32 x, uint32 y, uint32 w, uint32 h)
{
	uint32 pitch = sdlemuGetOverlayWidthInPixels();
	uint32 address = x + (y * pitch);

	for(uint32 yy=0; yy<h; yy++)
		for(uint32 xx=0; xx<w; xx++)
			*(screen + address + xx + (yy * pitch)) = color;
}

