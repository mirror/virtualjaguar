//
// GUI.H
//
// Graphical User Interface support
//

#ifndef __GUI_H__
#define __GUI_H__

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

void InitGUI(void);
void GUIDone(void);
//void DrawString(int16 * screen, uint32 x, uint32 y, bool invert, const char * text, ...);
//bool GUIMain(void);
bool GUIMain(char *);

uint32 JaguarLoadROM(uint8 * rom, char * path);
bool JaguarLoadFile(char * path);

#ifdef __cplusplus
}
#endif

#endif	// __GUI_H__
