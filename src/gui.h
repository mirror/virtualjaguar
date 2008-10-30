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
bool GUIMain(char *);

// Exported vars

extern bool showGUI;
extern bool finished;

#ifdef __cplusplus
}
#endif

#endif	// __GUI_H__
