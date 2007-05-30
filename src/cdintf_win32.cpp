//
// OS specific CDROM interface (Win32)
//
// by James L. Hammons
//
// Currently, we use the ASPI layer for Win32, but this may or may not
// work on NT based Windows. If necessary, we'll put in the required
// NT based code, but for now, it's ASPI or nothing.
//

// STILL TO DO:
//
// - Convert this shit to platform independent libcdio
//

// *** OS dependent CDROM stuffola ***
#include <windows.h>
#include "wnaspi32.h"
#include "scsidefs.h"
// *** End OS dependent ***
#include "log.h"

using namespace std;

// Local variables

static uint8 maxHostAdapters = 0, numCDDrives = 0, driveNum;
static uint8 haID[8], tID[8];
static uint8 driveName[8][26];
static uint8 tracks[100][3];						// One-based index
static uint32 numTracks = 0;
static uint8 sessions[40][5];						// Zero-based index
static uint32 numSessions = 0;
static bool readTOC = false;

static HINSTANCE hASPILib = NULL;					// Handle to ASPI for Win32 (WNASPI.DLL)
static uint32 (* ASPI_GetASPI32SupportInfo)(void);	// WNASPI.DLL function pointers
static uint32 (* ASPI_SendASPI32Command)(LPSRB);

// Private function prototypes

static bool InitASPI(void);
static bool SendAsyncASPICmd(uint8, uint8, uint8 *, uint8, uint8 *, uint32, uint8);
/*static*/ bool GetRawTOC(void);


//
// Initialize the Win32 ASPI layer
//
static bool InitASPI(void)
{
	hASPILib = LoadLibrary("WNASPI32");

	if (!hASPILib)
	{
		WriteLog("CDINTF: Could not load WNASPI32.DLL!\n");
		return false;
	}

	ASPI_GetASPI32SupportInfo = (uint32 (*)(void))GetProcAddress(hASPILib, "GetASPI32SupportInfo");
	ASPI_SendASPI32Command = (uint32 (*)(LPSRB))GetProcAddress(hASPILib, "SendASPI32Command");

	if (!ASPI_GetASPI32SupportInfo || !ASPI_SendASPI32Command)
	{
		WriteLog("CDINTF: Could not import functions from WNASPI32.DLL!\n");
		return false;
	}

	uint32 supportInfo = ASPI_GetASPI32SupportInfo();	// Initialize ASPI layer
	uint8 retCode = (supportInfo >> 8) & 0xFF;
	maxHostAdapters = supportInfo & 0xFF;

	if (retCode != SS_COMP && retCode != SS_NO_ADAPTERS)
	{
		WriteLog("CDINTF: Could not initialise using GetASPI32SupportInfo function!\n");
		return false;
	}

	if (retCode == SS_NO_ADAPTERS)
	{
		WriteLog("CDINTF: ASPI initialized, but no host adapters were found!\n");
		return false;
	}

/*	// Set timeouts for ALL devices to 15 seconds.  Nothing we deal with should
	// take that long to do ANYTHING.  We are just doing inquiries to most
	// devices, and then simple reads to CDs, disks, etc. so 10 seconds (even
	// if they have to spin up) should be plenty. 

	SRB_GetSetTimeouts srbTimeouts;
//This doesn't seem to do anything, and isn't even mentioned in Adaptec's ASPI paper...
//(It *is* mentioned elsewhere, in other Adaptec documentation, and it does nothing because it
// errors out!)
//It *does* return code $81 (SS_INVALID_HA) which means it doesn't like $FF for the HAID...
//OK, it works with Adaptec's driver, but not the default MS driver...
//Looks like we really don't need it anyway.
//If we really wanted to, we could do it in CDIntfInit()...!
	memset(&srbTimeouts, 0, sizeof(SRB_GetSetTimeouts));
	srbTimeouts.SRB_Cmd = SC_GETSET_TIMEOUTS;
	srbTimeouts.SRB_Flags = SRB_DIR_OUT;
	srbTimeouts.SRB_HaId = 0xFF;
	srbTimeouts.SRB_Target = 0xFF;
	srbTimeouts.SRB_Lun = 0xFF;
	srbTimeouts.SRB_Timeout = 15 * 2;
	ASPI_SendASPI32Command(&srbTimeouts);
	WriteLog("CDINTF: Set Timeout command returned %02X...\n", srbTimeouts.SRB_Status);//*/

	WriteLog("CDINTF: Successfully initialized.\n");
	return true;
}

//
// Sends the passed in Command Description Block to the APSI layer for processing.
// Since this uses the asynchronous EXEC_SCSI_CMD, we also wait for completion by
// using a semaphore.
//
static bool SendAsyncASPICmd(uint8 hostID, uint8 targID, uint8 * cdb, uint8 CDBLen,
	uint8 * buffer, uint32 bufferLen, uint8 SRBFlags)
{
	SRB_ExecSCSICmd SRB;							// The SRB variable with CDB included

	memset(&SRB, 0, sizeof(SRB));
	memcpy(SRB.CDBByte, cdb, CDBLen);				// Copy CDB into SRB's CDB

	SRB.SRB_Cmd        = SC_EXEC_SCSI_CMD;
	SRB.SRB_Flags      = SRBFlags | SRB_EVENT_NOTIFY;
	SRB.SRB_HaId       = hostID;
	SRB.SRB_Target     = targID;
	SRB.SRB_BufPointer = buffer;
	SRB.SRB_BufLen     = bufferLen;
	SRB.SRB_CDBLen     = CDBLen;
	SRB.SRB_SenseLen   = SENSE_LEN;

	HANDLE hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	if (!hEvent)
	{
		WriteLog("CDINTF: Couldn't create event!\n");
		return false;
	}

	SRB.SRB_PostProc = (void *)hEvent;
	ASPI_SendASPI32Command(&SRB);

	if (SRB.SRB_Status == SS_PENDING)
		WaitForSingleObject(hEvent, INFINITE);

	CloseHandle(hEvent);

	if (SRB.SRB_Status != SS_COMP)
	{
		WriteLog("CDINTF: SCSI command %02X failed [Error: %02X].\n", SRB.CDBByte[0], SRB.SRB_Status);
		return false;
	}

	return true;
}

//
// OS specific implementation of OS agnostic functions
//

bool CDIntfInit(void)
{
	if (!InitASPI())
	{
		WriteLog("CDINTF: Failed to init Win32 ASPI layer!\n");
		return false;
	}

	SRB_HAInquiry srbHAInquiry;
	SRB_GDEVBlock srbGDEVBlock;
	uint8 inquiryCDB[6], inquiryBuf[36];

	for(uint8 HAID=0; HAID<maxHostAdapters; HAID++)
	{
		memset(&srbHAInquiry, 0, sizeof(srbHAInquiry));
		srbHAInquiry.SRB_Cmd = SC_HA_INQUIRY;
		srbHAInquiry.SRB_HaId = HAID;

		ASPI_SendASPI32Command(&srbHAInquiry);

		if (srbHAInquiry.SRB_Status != SS_COMP)
			continue;

		// Do a host adapter inquiry to get max target count.  If the 
		// target count isn't 8 or 16 then go with a default of 8.

		uint8 maxTargets = srbHAInquiry.HA_Unique[3];
		if (maxTargets != 8 && maxTargets != 16)
			maxTargets = 8;

		// Loop over all the targets on this host adapter.

		for(uint8 target=0; target<maxTargets; target++)
		{
			// Issue get device type call to see if there is a device we're
			// interested in at this address.  We're interested in CDROMs.

			memset(&srbGDEVBlock, 0, sizeof(srbGDEVBlock));
			srbGDEVBlock.SRB_Cmd = SC_GET_DEV_TYPE;
			srbGDEVBlock.SRB_HaId = HAID;
			srbGDEVBlock.SRB_Target = target;

			ASPI_SendASPI32Command(&srbGDEVBlock);

			if (srbGDEVBlock.SRB_Status != SS_COMP || srbGDEVBlock.SRB_DeviceType != DTYPE_CDROM)
				continue;

			memset(inquiryCDB, 0, 6);				// Issue an INQUIRY.
			inquiryCDB[0] = SCSI_INQUIRY;
			inquiryCDB[4] = 36;						// Size in bytes of inquiry buffer.

			bool successful = SendAsyncASPICmd(HAID, target, inquiryCDB, 6, inquiryBuf, 36, SRB_DIR_IN);

			// Make sure the inquiry worked. Check if it failed, or if the inquiry data
			// returns a different device type than we got before (guards against certain
			// device drivers and against vendor unique devices).

			if (!successful || inquiryBuf[0] != DTYPE_CDROM)
				continue;

			haID[numCDDrives] = HAID, tID[numCDDrives] = target;

			// Here we do a 'stringTrimRight' on the vendor and product strings...

			uint32 vendorSize = 0, productSize = 0;

			for(int i=7; i>=0; i--)
			{
				if (inquiryBuf[8+i] != ' ')
				{
					vendorSize = i + 1;
					break;
				}
			}

			for(int i=15; i>=0; i--)
			{
				if (inquiryBuf[16+i] != ' ')
				{
					productSize = i + 1;
					break;
				}
			}

			memcpy(driveName[numCDDrives], inquiryBuf + 8, vendorSize);
			driveName[numCDDrives][vendorSize] = ' ';
			memcpy(driveName[numCDDrives] + vendorSize + 1, inquiryBuf + 16, productSize);
			driveName[numCDDrives][vendorSize + productSize + 1] = 0;

			WriteLog("CDINTF: Found CD-ROM device [%s]. HAID:%u, TID:%u LUN:0\n", driveName[numCDDrives], haID[numCDDrives], tID[numCDDrives]);

			numCDDrives++;
		}
	}

	if (numCDDrives == 0)
	{
		WriteLog("CDINTF: No CDROM type drives found.\n");
		return false;
	}

//Most likely, will need to read a default out of the config file. But for now... !!! FIX !!!
	driveNum = 0;								// For now, default to first drive found

	return true;
}

void CDIntfDone(void)
{
	if (hASPILib)
		FreeLibrary(hASPILib);						// Unload ASPI library if it was loaded.
}

bool CDIntfReadBlock(uint32 sector, uint8 * buffer)
{
	uint8 cdb[12];

	memset(cdb, 0, sizeof(cdb));
	// 0: command, 2-5: block # (hi->lo) 6-8: number of blocks to read, 9: read type,
	// 10: subchannel select
	cdb[0] = 0xBE;									// Code for ReadCD CDB12 command
	cdb[2] = (sector >> 24) & 0xFF;
	cdb[3] = (sector >> 16) & 0xFF;
	cdb[4] = (sector >> 8) & 0xFF;
	cdb[5] = sector & 0xFF;
	cdb[8] = 1;										// No. of sectors to read from CD (LSB)
	cdb[9] = 0xF8;									// Raw read, 2352 bytes per sector
	cdb[10] = 1;									// Selects read RAW 96 bytes/sector sub-channel data (Raw P-W)

	return SendAsyncASPICmd(haID[driveNum], tID[driveNum], cdb, 12, buffer, 2352+96, SRB_DIR_IN);
}

uint32 CDIntfGetNumSessions(void)
{
//	WriteLog("CDINTF: GetNumSessions unimplemented!\n");
	// Still need relevant code here... !!! FIX !!! [DONE]
	if (!readTOC)
		GetRawTOC();

	return numSessions - 1;
}

void CDIntfSelectDrive(uint32 driveNum)
{
	if (driveNum < numCDDrives)
		driveNum = driveNum;
}

uint32 CDIntfGetCurrentDrive(void)
{
	return driveNum;
}

const uint8 * CDIntfGetDriveName(uint32 driveNum)
{
	if (driveNum > numCDDrives)
		return NULL;

	return driveName[driveNum];
}

//This stuff could probably be OK in the unified cdintf.cpp file...
uint8 CDIntfGetSessionInfo(uint32 session, uint32 offset)
{
// Need better error handling than this... !!! FIX !!!
	if (!readTOC)
		if (!GetRawTOC())
			return 0xFF;

	if (session >= numSessions || offset > 4)
		return 0xFF;								// Bad index passed in...

	return sessions[session][offset];
}

uint8 CDIntfGetTrackInfo(uint32 track, uint32 offset)
{
// Need better error handling than this... !!! FIX !!!
	if (!readTOC)
		if (!GetRawTOC())
			return 0xFF;

	if (track > numTracks || offset > 2)
		return 0xFF;								// Bad index passed in...

	return tracks[track][offset];
}

//OK, now the rest is OK, but this is still locking up like a MF!
// Testing, testing...
//Still don't know why this is locking up! Especially as the following function works! Aarrrgggghhhhh!
//It was the dataLen. For some reason, it needs at *least* 11 more bytes (could be less!)
/*static*/ bool GetRawTOC(void)
{
	uint8 cmd[10];
	uint8 reqData[4];

	// Read disk TOC length
	memset(cmd, 0, 10);
	cmd[0] = SCSI_READ_TOC;
	cmd[2] = 2;										// Get session info also
	cmd[6] = 1;										// Session # to start reading
	cmd[8] = 4;										// Buffer length

	if (!SendAsyncASPICmd(haID[driveNum], tID[driveNum], cmd, 10, reqData, 4, SRB_DIR_IN))
	{
		WriteLog("TOC: Cannot read disk TOC length.\n");
		return false;
	}

	// Header is total TOC space needed + header (0-1), min session (2), max session (3)
//	uint32 dataLen = ((reqData[0] << 8) | reqData[1]);
	uint32 dataLen = ((reqData[0] << 8) | reqData[1]) + 11;	// Why the extra???
  
	WriteLog("TOC: Raw TOC data len: %d\n", dataLen);

	uint8 * data = new uint8[dataLen];
  
	// Read disk TOC
	cmd[7] = dataLen >> 8;
	cmd[8] = dataLen;

	if (!SendAsyncASPICmd(haID[driveNum], tID[driveNum], cmd, 10, data, dataLen, SRB_DIR_IN))
	{
		delete[] data;
		WriteLog("TOC: Cannot read disk TOC.\n");
		return false;
	}

	int numEntries = (((data[0] << 8) | data[1]) - 2) / 11;
	uint8 * p = data + 4;

	numSessions = data[3], numTracks = 0;
	// Important entries are 0, 3, 8, 9, 10 (session #, track #, M, S, F)
//	WriteLog("TOC: [Sess] [adrCtl] [?] [point] [?] [?] [?] [?] [pmin] [psec] [pframe]\n");
	uint32 firstTrackOffset = 0;
	for(int i=0; i<numEntries; i++, p+=11)
	{
/*		WriteLog("TOC: %d %02x %02d %2x %02d:%02d:%02d %02d %02d:%02d:%02d",
			p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], p[8], p[9], p[10]);

		if (p[3] > 0 && p[3] < 99)
			WriteLog("   <-- Track #%u", p[3]);

		WriteLog("\n");//*/

		// We do session # - 1 to make it zero-based, since this is what the Jaguar
		// CD BIOS expects. We leave the tracks one-based.

		uint32 s = p[0] - 1, t = p[3];

		if (t < 100)
		{
			if (t == 1)
				firstTrackOffset = (((p[8] * 60) + p[9]) * 75) + p[10];

			tracks[t][0] = p[8], tracks[t][1] = p[9], tracks[t][2] = p[10];

			// For some reason, the TOC returned from the "session TOC" command
			// causes all tracks to have a 150 block (from what I've seen) offset
			// from what's reported. Apparently it's not possible to read those
			// first 150 blocks using the current incarnation of CDIntf_ReadBlock.
			// So we subtract the offset out here...

			uint32 curTrack = (((tracks[t][0] * 60) + tracks[t][1]) * 75) + tracks[t][2];
			curTrack -= firstTrackOffset;
			tracks[t][2] = curTrack % 75;
			curTrack /= 75;
			tracks[t][1] = curTrack % 60;
			tracks[t][0] = curTrack / 60;

			if (t > numTracks)
				numTracks = t;
		}
		else if (t == 0xA0)
			sessions[s][0] = p[8];
		else if (t == 0xA1)
			sessions[s][1] = p[8];
		else if (t == 0xA2)
			sessions[s][2] = p[8], sessions[s][3] = p[9], sessions[s][4] = p[10];
	}

WriteLog("CDINTF: Disc summary\n        # of sessions: %u, # of tracks: %u\n", numSessions, numTracks);
WriteLog("        Session info:\n");
for(uint32 i=0; i<numSessions; i++)
	WriteLog("        %u: min track=%2u, max track=%2u, lead out=%2d:%02d:%02d\n", i+1, sessions[i][0], sessions[i][1], sessions[i][2], sessions[i][3], sessions[i][4]);
WriteLog("        Track info:\n");
for(uint32 i=1; i<=numTracks; i++)
	WriteLog("        %2u: start=%2d:%02d:%02d\n", i, tracks[i][0], tracks[i][1], tracks[i][2]);

	delete[] data;
	readTOC = true;

	return true;
}
