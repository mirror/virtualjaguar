//
// filedb.h: File database definition
//

#ifndef __FILEDB_H__
#define __FILEDB_H__

#include "types.h"

// Useful enumerations

enum FileFlags { FF_ROM=0x01, FF_ALPINE=0x02, FF_BIOS=0x04, FF_REQ_DSP=0x08, FF_REQ_BIOS=0x10, FF_NON_WORKING=0x20, FF_BAD_DUMP=0x40, FF_VERIFIED=0x80 };

// Useful structs

struct RomIdentifier
{
	const uint32 crc32;
	const char name[128];
//	const uint8 compatibility;
	const uint32 flags;
};

// So other stuff can pull this in...

extern RomIdentifier romList[];

#endif	// __FILEDB_H__
