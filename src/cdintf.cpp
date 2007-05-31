//
// OS agnostic CDROM interface functions
//
// by James L. Hammons
//
// This file is basically a shell to keep the front-end clean and also pull in the
// appropriate back-end code depending on which target is being compiled for.
//

#include "cdintf.h"								// Every OS has to implement these

#include <cdio/cdio.h>							// Now using OS agnostic CD access routines!
#include "log.h"

// Not any more!
#if 0

// OS dependent implementations

#if defined(__GCCWIN32__)

#include "cdintf_win32.cpp"

#elif defined(__GCCUNIX__)
	#if defined(_OSX_)

#include "cdintf_osx.cpp"

	#else

#include "cdintf_linux.cpp"

	#endif
#endif

#endif

/*
static void TestCDIO(void)
{
	// See what (if anything) is installed.
	CdIo_t * p_cdio = cdio_open(0, DRIVER_DEVICE);
	driver_id_t driver_id;

	if (p_cdio != NULL)
	{
		WriteLog("CDIO: The driver selected is %s.\n", cdio_get_driver_name(p_cdio));
		WriteLog("CDIO: The default device for this driver is %s.\n\n", cdio_get_default_device(p_cdio));
		cdio_destroy(p_cdio);
	}
	else
	{
		WriteLog("CDIO: A suitable CD-ROM driver was not found.\n\n");
	}
}
*/

//
// *** OK, here's where we're going to attempt to put the platform agnostic CD interface ***
//

static CdIo_t * cdioPtr = NULL;

bool CDIntfInit(void)
{
	cdioPtr = cdio_open(NULL, DRIVER_DEVICE);

	if (cdioPtr == NULL)
	{
		WriteLog("CDINTF: No suitable CD-ROM driver found.\n");
		return false;
	}

	return true;
}

void CDIntfDone(void)
{
	WriteLog("CDINTF: Shutting down CD-ROM subsystem.\n");

	if (cdioPtr)
		cdio_destroy(cdioPtr);
}

bool CDIntfReadBlock(uint32 sector, uint8 * buffer)
{
	// !!! FIX !!!
	WriteLog("CDINTF: ReadBlock unimplemented!\n");
	return false;
}

uint32 CDIntfGetNumSessions(void)
{
	// !!! FIX !!!
	// Still need relevant code here... !!! FIX !!!
	return 2;
}

void CDIntfSelectDrive(uint32 driveNum)
{
	// !!! FIX !!!
	WriteLog("CDINTF: SelectDrive unimplemented!\n");
}

uint32 CDIntfGetCurrentDrive(void)
{
	// !!! FIX !!!
	WriteLog("CDINTF: GetCurrentDrive unimplemented!\n");
	return 0;
}

const uint8 * CDIntfGetDriveName(uint32 driveNum)
{
	// driveNum is currently ignored... !!! FIX !!!

	uint8 * driveName = (uint8 *)cdio_get_default_device(cdioPtr);
	WriteLog("CDINTF: The drive name for the current driver is %s.\n", driveName);

	return driveName;
}

uint8 CDIntfGetSessionInfo(uint32 session, uint32 offset)
{
	// !!! FIX !!!
	WriteLog("CDINTF: GetSessionInfo unimplemented!\n");
	return 0xFF;
}

uint8 CDIntfGetTrackInfo(uint32 track, uint32 offset)
{
	// !!! FIX !!!
	WriteLog("CDINTF: GetTrackInfo unimplemented!\n");
	return 0xFF;
}
