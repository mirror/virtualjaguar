//
// filedb.h: File database definition
//

#ifndef __FILEDB_H__
#define __FILEDB_H__

#include "types.h"

// Useful enumerations

enum FileFlags { FF_ROM=1, FF_ALPINE=2, FF_BIOS=4, FF_REQ_DSP=8, FF_REQ_BIOS=16, FF_NON_WORKING=32, FF_BAD_DUMP=64 };

// Useful structs

struct RomIdentifier
{
	const uint32 crc32;
	const char name[128];
//	const char file[128];
	const uint32 flags;
};

// So other stuff can pull this in...

extern RomIdentifier romList[];

#endif	// __FILEDB_H__
