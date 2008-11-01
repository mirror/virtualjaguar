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

void GUIInit(void);
void GUIDone(void);
bool GUIMain(char *);
void GUICrashGracefully(const char *);

// Exported vars

extern bool showGUI;
extern bool exitGUI;							// Hmm.
extern bool finished;							// Current emulator loop is finished

#ifdef __cplusplus
}
#endif

#endif	// __GUI_H__
