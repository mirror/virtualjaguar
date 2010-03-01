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

enum FileType { FT_SOFTWARE=0, FT_EEPROM, FT_LABEL, FT_BOXART, FT_OVERLAY };

uint32 JaguarLoadROM(uint8 * rom, char * path);
bool JaguarLoadFile(char * path);
uint32 GetFileFromZIP(const char * zipFile, FileType type, uint8 * buffer);

#ifdef __cplusplus
}
#endif

#endif	// __FILE_H__
