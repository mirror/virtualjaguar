//
// state.h: Machine state save/load support
//
// by James L. Hammons
//

#ifndef __STATE_H__
#define __STATE_H__

//#include <stdio.h>

//#ifdef __cplusplus
//extern "C" {
//#endif

//int LogInit(const char *);
//FILE * LogGet(void);
//void LogDone(void);
//void WriteLog(const char * text, ...);

//#ifdef __cplusplus
//}
//#endif

bool SaveState(void);
bool LoadState(void);

#endif	// __STATE_H__
