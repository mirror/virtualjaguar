//
// OS specific CDROM interface (linux)
//
// by James L. Hammons and Niels Wagenaar
//
// NOTE : This CD-ROM code *could* work with other UN*X related OS. However,
//        we/I are not sure on this matter.
//
// This is very experimental and I have the feeling that this won't even compile.
// Hell, I don't even have a Linux dev system (broken) or Jaguar CD releases to
// test this code :(
//
// Big thanks to the DOSBOX team which provided us with great knowlegde about
// CD-ROM access and Linux/UN*X.

#ifdef LINUX_JAGUAR_CDROM

// *** OS dependent CDROM stuffola ***
#include <fcntl.h>
#include <unistd.h>
#include <linux/cdrom.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>

// *** End OS dependent ***
#include "log.h"
#include "string.h"

// *** Virtual Jaguar dependent ***
//#include "SDL.h"                // Yes, we use SDL for initializing the CD-ROM and
//                                // give us access to certain CD-ROM states. But not just yet.

// *** SDL CD-ROM dependent ***        // Not yet needed!
// SDL_CD      *cdrom;                 // Our variable for SDL CD-ROM access.
// CDstatus     status;                // Let us get our status.
// char        *status_str;

// *** Local variables ***
char	               device_name[512] ; // Devicename, for example /dev/cdrom

//
// Linux support functions
// OS specific implementation of OS agnostic functions
//

bool CDIntfInit(void)
{
    // Setting device_name to /deb/cdrom. /dev/cdrom is the default CD-ROM
    // drive on most UN*X systems. Well, I think it is.
    //
    // In the future we can probably use SDL for getting CDROM states and
    // CD-ROM specific information.
    strcpy(device_name, "/dev/cdrom");

    // Let us open the device_name and check if we can open the CD-ROM.
    int cdrom_fd = open(device_name, O_RDONLY | O_NONBLOCK);
    
	if (cdrom_fd <= 0) 
    {
        // CD-ROM isn't accessable.
        // Write the error in the log file and return false.
        WriteLog("CDINTF: CDIntfInit - Unable to open CDROM!\n");
        return false;
    }    
    else
    {
        // CD-ROM is accessable.
        // Write the success in the log file and return true.
        WriteLog("CDINTF: CDIntfInit - Succesfully opened CDROM!\n"); 
        close(device_name);
        return true;
    }
        
}

void CDIntfDone(void)
{
    // Just in case : closing device_name.
    WriteLog("CDINTF: CDIntfDone - Closing CDROM!\n");
    close(device_name);
}

bool CDIntfReadBlock(uint32 sector, uint8 * buffer)
{
	unsigned int   buflen = CD_FRAMESIZE_RAW;          // Raw read, 2352 bytes per sector
	//unsigned char *buf    = new unsigned int[buflen];	// DOSBOX, do we need this?
	int ret;
	struct cdrom_read cdrom_read;

	// Let us open the device_name and check if we can open the CD-ROM.
    int cdrom_fd = open(device_name, O_RDONLY | O_NONBLOCK);
	if (cdrom_fd <= 0)
    {
        // CD-ROM isn't accessable.
        // Write the error in the log file and return false.
        WriteLog("CDINTF: CDIntfReadBlock - Unable to open CDROM!\n");
        return false;
    }    
	
	// Setting up the cdrom_read struct :
	cdrom_read.cdread_lba = sector;             // Which sector to read.
	cdrom_read.cdread_bufaddr = (char*)buffer;  // Where to put the data (?)
	cdrom_read.cdread_buflen = buflen;          // 2352 bytes/sector -> RAW read
                                          
	// Let us read the content we want.	-1 (false) when it didn't work.
	ret = ioctl(cdrom_fd, CDROMREADRAW, &cdrom_read);		

	// Close the CD-ROM.
	close(cdrom_fd);

	// The following was taken from DOSBOX. After reading the content, they write
	// back the information from buf (based upon the size of buflen) to buffer.
	// I think that this is not needed. *fingers crossed*
	//
	// MEM_BlockWrite(buffer, buf, buflen);
	// delete[] buf;
	
	// Uncomment the following for debug reasons.
	//
    // WriteLog("CDINTF: CDIntfReadBlock - Reading sector %d!\n", sector);
    
	return (ret > 0);

}

uint32 CDIntfGetNumSessions(void)
{
	// Still need relevant code here...
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

#else

#include "log.h"

//
// Linux support functions
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
#endif
