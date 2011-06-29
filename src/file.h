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
// JST = Jaguar Software Type
enum { JST_NONE = 0, JST_ROM, JST_ALPINE, JST_ABS_TYPE1, JST_ABS_TYPE2, JST_JAGSERVER };

uint32 JaguarLoadROM(uint8 * rom, char * path);
bool JaguarLoadFile(char * path);
uint32 GetFileFromZIP(const char * zipFile, FileType type, uint8 * &buffer);
uint32 ParseFileType(uint8 header1, uint8 header2, uint32 size);
bool HasUniversalHeader(uint8 * rom, uint32 romSize);

#ifdef __cplusplus
}
#endif

#endif	// __FILE_H__
