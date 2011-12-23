//
// OS specific CDROM interface (Mac OS X)
//
// by James L. Hammons & ?
//

#include "log.h"

//
// OS X support functions
// OS specific implementation of OS agnostic functions
//

bool CDIntfInit(void)
{
	WriteLog("CDINTF: Init unimplemented!\n");
	return false;
}

void CDIntfDone(void)
{
}

bool CDIntfReadBlock(uint32 sector, uint8 * buffer)
{
	WriteLog("CDINTF: ReadBlock unimplemented!\n");
	return false;
}

uint32 CDIntfGetNumSessions(void)
{
	// Still need relevant code here... !!! FIX !!!
	return 2;
}

void CDIntfSelectDrive(uint32 driveNum)
{
	WriteLog("CDINTF: SelectDrive unimplemented!\n");
}

uint32 CDIntfGetCurrentDrive(void)
{
	WriteLog("CDINTF: GetCurrentDrive unimplemented!\n");
	return 0;
}

const uint8 * CDIntfGetDriveName(uint32)
{
	WriteLog("CDINTF: GetDriveName unimplemented!\n");
	return NULL;
}

uint8 CDIntfGetSessionInfo(uint32 session, uint32 offset)
{
	WriteLog("CDINTF: GetSessionInfo unimplemented!\n");
	return 0xFF;
}

uint8 CDIntfGetTrackInfo(uint32 track, uint32 offset)
{
	WriteLog("CDINTF: GetTrackInfo unimplemented!\n");
	return 0xFF;
}
