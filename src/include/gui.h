//
// GUI.H
//
// Graphical User Interface support
//

#ifndef __GUI_H__
#define __GUI_H__

#ifdef __cplusplus
extern "C" {
#endif

void DrawString(int16 * screen, uint32 x, uint32 y, bool invert, const char * text, ...);
bool UserSelectFile(char * path, char * filename);

#ifdef __cplusplus
}
#endif

#endif	// __GUI_H__
