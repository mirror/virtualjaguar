//
// GUI.H
//
// Graphical User Interface support
//

#ifndef __GUI_H__
#define __GUI_H__

void BlitBackbuffer(void);
void DrawText(int16 * screen, uint32 x, uint32 y, bool invert, const char * text, ...);
bool UserSelectFile(char * path, char * filename);

#endif	// __GUI_H__
