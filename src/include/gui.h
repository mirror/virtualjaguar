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

void InitGUI(void);
void GUIDone(void);
void DrawString(int16 * screen, uint32 x, uint32 y, bool invert, const char * text, ...);
//bool UserSelectFile(char * path, char * filename);
bool GUIMain(void);

uint32 JaguarLoadROM(uint8 * rom, char * path);
//void JaguarLoadCart(uint8 * mem, char * path);
bool JaguarLoadFile(char * path);

#ifdef __cplusplus
}
#endif

#endif	// __GUI_H__
