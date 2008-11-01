//
// FILE.H
//
// File support
//

#ifndef __FILE_H__
#define __FILE_H__

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

uint32 JaguarLoadROM(uint8 * rom, char * path);
bool JaguarLoadFile(char * path);

#ifdef __cplusplus
}
#endif

#endif	// __FILE_H__
