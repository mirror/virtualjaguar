//
// LOG.H
//

#ifndef __LOG_H__
#define __LOG_H__

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif
#ifdef _MSC_VER									// for Microsoft Visual Studio
#pragma warning(disable:4996)					// Avoid deprecated warnings
#endif

int log_init(char *);
FILE * log_get(void);
void log_done(void);
void WriteLog(const char * text, ...);

#ifdef __cplusplus
}
#endif

#endif	// __LOG_H__
