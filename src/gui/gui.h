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
bool GUIMain(char *);

// Not sure why these are in *this* file, but here they are... :-/

uint32 JaguarLoadROM(uint8 * rom, char * path);
bool JaguarLoadFile(char * path);

#ifdef __cplusplus
}
#endif

#endif	// __GUI_H__
