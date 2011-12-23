//
// OS specific CDROM interface (Win32)
//
// by Stephan Kapfer
//
// ASPI coding based on code by James L. Hammons
//
// IOCTL used if Windows NT/2000/XP/...
// ASPI used if IOCTL not available, e.g Win95/98/ME
//

#include <Windows.h>
#include <Winioctl.h>
#include "ntddcdrm.h"
#include "wnaspi32.h"
#include "scsidefs.h"
#include "log.h"
#include "types.h"
#include "cdrom.h"
#include "settings.h"				// for vjs.CDDrive

//#define CDROM_LOG_RAW_READ
//#define CDROM_LOG_RAW_DATA

#define CD_BLOCKS_PER_SECOND 75

extern uint8 CDReadBuffer[RAW_SECTOR_SIZE * BLOCKS_TO_READ];
extern uint16 CDReadBufferIndex;
extern s_CD_POSITION_MSF CDReadPositionMSF;
extern bool bCDRead;
extern uint8 CDStatus;

extern uint16 RXData[];			// 512 words should be enough
extern uint16 RXDataIndex;
extern uint16 MaxRXDataCount;

typedef struct _MY_CDROM_TOC_FULL_TOC_DATA {
    UCHAR Length[2];
    UCHAR FirstCompleteSession;
    UCHAR LastCompleteSession;
    // one to N descriptors included
    CDROM_TOC_FULL_TOC_DATA_BLOCK Descriptors[150];		// Some session infos + 100 Tracks
} MY_CDROM_TOC_FULL_TOC_DATA, *PMYCDROM_TOC_FULL_TOC_DATA;

MY_CDROM_TOC_FULL_TOC_DATA	CDFullToc;

s_CD_SESSION_TOC CDToc;

bool bUseASPI;
//uint8 CDRawData[RAW_SECTOR_SIZE];

// ASPI
uint8 UseCDDriveNumber = 0;
uint8 haID[8], tID[8];						// 8 CD devices should be enough
uint8 driveName[8][32];
HINSTANCE hASPILib = NULL;					// Handle to ASPI for Win32 (WNASPI.DLL)
uint32 (* ASPI_GetASPI32SupportInfo)(void);	// WNASPI32.DLL function pointers
uint32 (* ASPI_SendASPI32Command)(LPSRB);

/*
Full TOC data of a sample CD:

TOC: [Sess] [adrCtl] [?] [point] [?] [?] [?] [?] [min] [sec] [frame]
TOC: 1 10 00 a0 00:00:00 00 01:00:00   <-- First track: 1
TOC: 1 10 00 a1 00:00:00 00 01:00:00   <-- Last track: 1
TOC: 1 10 00 a2 00:00:00 00 00:18:29   <-- Lead out
TOC: 1 10 00  1 00:00:00 00 00:02:00   <-- Track #1 start
TOC: 1 50 00 b0 02:48:29 02 79:59:74
TOC: 1 50 00 c0 160:00:00 00 97:15:12
TOC: 2 10 00 a0 00:00:00 00 02:00:00   <-- First track: 2
TOC: 2 10 00 a1 00:00:00 00 04:00:00   <-- Last track: 4
TOC: 2 10 00 a2 00:00:00 00 03:22:55   <-- Lead out of session
TOC: 2 10 00  2 00:00:00 00 02:50:29   <-- Track #2 start
TOC: 2 10 00  3 00:00:00 00 02:58:27   <-- Track #3 start
TOC: 2 10 00  4 00:00:00 00 03:16:57   <-- Track #4 start
*/

// IOCTL
char UseCDDriveLetter[8] = "\0";	// Gets "\\.\x:"

/*DWORD MSFToBlocks( UCHAR msf[4] )
{
    DWORD cBlock;

    cBlock =
        ( msf[1] * ( CD_BLOCKS_PER_SECOND * 60 ) ) +
        ( msf[2] * CD_BLOCKS_PER_SECOND ) +
        msf[3];

    return( cBlock - (2 * CD_BLOCKS_PER_SECOND));	// necessary "alignment"
}*/

/*
Full TOC data of session #1:

FirstCompleteSession: 1
LastCompleteSession: 2
Descriptor[0].SessionNumber: 1
Descriptor[0].Msf: 01:00.00		first track of session
Descriptor[1].SessionNumber: 1
Descriptor[1].Msf: 01:00.00		last track of session
Descriptor[2].SessionNumber: 1
Descriptor[2].Msf: 00:18.29		lead out of session
Descriptor[3].SessionNumber: 1
Descriptor[3].Msf: 00:02.00		start of track 1
...

Full toc data of session #2:

FirstCompleteSession: 1
LastCompleteSession: 2
Descriptor[0].SessionNumber: 2
Descriptor[0].Msf: 02:00.00		first track of session
Descriptor[1].SessionNumber: 2
Descriptor[1].Msf: 04:00.00		last track of session
Descriptor[2].SessionNumber: 2
Descriptor[3].Msf: 03:22.55		lead out of session
Descriptor[4].SessionNumber: 2
Descriptor[4].Msf: 02:50.29		start of track 2
Descriptor[5].SessionNumber: 2
Descriptor[5].Msf: 02:58.27		start of track 3
Descriptor[6].SessionNumber: 2
Descriptor[6].Msf: 03:16.57		start of track 4
...
*/

void CDFullTocToSessionToc(uint8 WantedSession)
{
	int Descriptors = (((CDFullToc.Length[0] << 8) | CDFullToc.Length[1]) - sizeof(CDFullToc.Length)) / sizeof(CDROM_TOC_FULL_TOC_DATA_BLOCK);
	WriteLog("TOC: Descriptors: %d\n", Descriptors);

	CDToc.FirstSession = CDFullToc.FirstCompleteSession;
	CDToc.LastSession = CDFullToc.LastCompleteSession;

	uint8 * p = (uint8 *)&CDFullToc.Descriptors;

	// Important entries are 0, 3, 8, 9, 10 (session #, track #, M, S, F)
	WriteLog("TOC: [Sess] [AdrCtrl] [Res1] [Point] [MinX] [SecX] [FrmX] [0] [Min] [Sec] [Frame]\n");
	for(int i=0; i<Descriptors; i++, p+=sizeof(CDROM_TOC_FULL_TOC_DATA_BLOCK))
	{
		uint8 Session = p[0];
		uint8 Point = p[3];

		WriteLog("TOC: %d %02x %02d %2x %02d:%02d:%02d %02d %02d:%02d:%02d",
			p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], p[8], p[9], p[10]);

		// min/max track and lead out
		if (Point == 0xA0)
			WriteLog("   <-- min. Track #%u", p[8]);
		if (Point == 0xA1)
			WriteLog("   <-- max. Track #%u", p[8]);
		if (Point == 0xA2)
			WriteLog("   <-- Lead Out %02d:%02d:%02d", p[8], p[9], p[10]);
		// 1-99 identify track data
		if ((Point > 0) && (Point < 100))
			WriteLog("   <-- Track #%u", Point);

		WriteLog("\n");

		// Session number is here 1 based, Jaguar uses 0 based
		if (WantedSession == (Session - 1))
		{
			if (Point < 100)		// Track 1-100
			{
				// The 150 block alignment is done in CD read function
				// so we keep the real track data as stated in TOC

				// Store track's MSF
				CDToc.Track[Point].Minutes = p[8];
				CDToc.Track[Point].Seconds = p[9];
				CDToc.Track[Point].Frames = p[10];

			}
			else if (Point == 0xA0)			// Session's first track
				CDToc.FirstTrack = p[8];
			else if (Point == 0xA1)			// Session's last track
				CDToc.LastTrack = p[8];
			else if (Point == 0xA2)			// Session's lead out MSF
			{
				CDToc.LeadOut.Minutes = p[8];
				CDToc.LeadOut.Seconds = p[9];
				CDToc.LeadOut.Frames = p[10];
			}
		}
	}
}

//
// Sends the passed in Command Description Block to the APSI layer for processing.
// Since this uses the asynchronous EXEC_SCSI_CMD, we also wait for completion by
// using a semaphore.
//
bool SendAsyncASPICmd(uint8 hostID, uint8 targID, uint8 * cdb, uint8 CDBLen,
	uint8 * buffer, uint32 bufferLen, uint8 SRBFlags)
{
	// Create event we will wait for
	HANDLE hASPICompletionEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (!hASPICompletionEvent)
	{
		WriteLog("CDROM: Couldn't create ASPI event!\n");
		return false;
	}

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
	SRB.SRB_PostProc = (void *)hASPICompletionEvent;

	ASPI_SendASPI32Command(&SRB);

	// Wait until event occurred
	if (SRB.SRB_Status == SS_PENDING)
		WaitForSingleObject(hASPICompletionEvent, INFINITE);

	CloseHandle(hASPICompletionEvent);

	if (SRB.SRB_Status != SS_COMP)
	{
		for(int i=0; i<(SENSE_LEN+2); i++)
		{
			WriteLog("SenseArea%02d: $%02X\n", i, SRB.SenseArea[i]);
		}
		WriteLog("SenseKey: $%02X\n", SRB.SenseArea[2]);
		WriteLog("ASC: $%02X\n", SRB.SenseArea[12]);
		WriteLog("ASCQ: $%02X\n", SRB.SenseArea[13]);

		WriteLog("CDROM: SCSI command $%02X failed [Status: $%02X, HaStat: $%02X, TargStat: $%02X].\n", SRB.CDBByte[0], SRB.SRB_Status, SRB.SRB_HaStat, SRB.SRB_TargStat);
		return false;
	}

	return true;
}

// Get CD TOC data with ASPI (WNASPI32.dll needed <- CD Burner SW)
bool ASPI_GetRawTOC(uint8 WantedSession)
{
	uint8 cdb[10];
//	uint8 reqData[4];

	ZeroMemory(&CDFullToc, sizeof(CDFullToc));

	// Read disk TOC length, retrieve MSFs
	memset(cdb, 0, sizeof(cdb));
	cdb[0] = SCSI_READ_TOC;
	cdb[1] = 2;							// MSF
	cdb[2] = 2;							// Get full TOC for session info
	cdb[6] = 1;							// Session # to start reading
	//cdb[8] = 4;							// Buffer length
	cdb[7] = sizeof(CDFullToc) >> 8;
	cdb[8] = sizeof(CDFullToc) & 0xFF;

/*	if (!SendAsyncASPICmd(haID[UseCDDriveNumber], tID[UseCDDriveNumber], cdb, sizeof(cdb), reqData, sizeof(reqData), SRB_DIR_IN))
	{
		WriteLog("CDB 0-8: %d, %d, %d, %d, %d, %d, %d, %d, %d\n",
			cdb[0],cdb[1],cdb[2],cdb[3],cdb[4],cdb[5],cdb[6],cdb[7],cdb[8]);

		// Older drives do not support MSF
		WriteLog("CDROM: MSF not supported, trying LBA.\n");
		// Read disk TOC length, retrieve LBAs
		memset(cdb, 0, sizeof(cdb));
		cdb[0] = SCSI_READ_TOC;
		cdb[2] = 2;							// Get full TOC for session info
		cdb[6] = 1;							// Session # to start reading
		cdb[8] = 4;							// Buffer length
		//cdb[7] = bufsize >> 8;
		//cdb[8] = bufsize & 0xFF;
		if (!SendAsyncASPICmd(haID[UseCDDriveNumber], tID[UseCDDriveNumber], cdb, sizeof(cdb), reqData, sizeof(reqData), SRB_DIR_IN))
		{
			WriteLog("CDB 0-8: %d, %d, %d, %d, %d, %d, %d, %d, %d\n",
				cdb[0],cdb[1],cdb[2],cdb[3],cdb[4],cdb[5],cdb[6],cdb[7],cdb[8]);

			WriteLog("CDROM: Cannot read disk TOC length.\n");
			return false;
		}
	}
	WriteLog("CDB 0-8: %d, %d, %d, %d, %d, %d, %d, %d, %d\n",
		cdb[0],cdb[1],cdb[2],cdb[3],cdb[4],cdb[5],cdb[6],cdb[7],cdb[8]);
*/
	// Header is total TOC space needed (multiple of 11)
	// + header (0-1) + min session (2) + max session (3)
	// Returned requested data length is total TOC space needed + 2 bytes
	// So we add 2 more bytes otherwise last track time is wrong
	// and some more bytes for Win9x
	//uint32 dataLen = ((reqData[0] << 8) | reqData[1]) + 3 + 8+100;
	//WriteLog("TOC: Raw TOC data len: %d\n", dataLen);

	// Read disk TOC
	//cdb[7] = sizeof(CDFullToc) >> 8;
	//cdb[8] = sizeof(CDFullToc) & 0xFF;

	//if (!SendAsyncASPICmd(haID[UseCDDriveNumber], tID[UseCDDriveNumber], cdb, sizeof(cdb), data, dataLen, SRB_DIR_IN))
	if (!SendAsyncASPICmd(haID[UseCDDriveNumber], tID[UseCDDriveNumber], cdb, sizeof(cdb), (uint8*)&CDFullToc, sizeof(CDFullToc), SRB_DIR_IN))
	{
		WriteLog("TOC: Cannot read disk TOC.\n");
		return false;
	}

	// Extract toc data of session
	CDFullTocToSessionToc(WantedSession);

	return true;
}

// Get CD TOC data with IOCTL (only Windows NT/2000/XP/...)
bool IOCTL_GetRawTOC(uint8 WantedSession)
{
	CDROM_READ_TOC_EX	CDTocEx;
	DWORD				dwBytesReturned;

	// Get CD data with IOCTL
	HANDLE hDevice = CreateFile(UseCDDriveLetter, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (!hDevice)
	{
		WriteLog("CDROM: ERROR: %d, no device handle.\n", GetLastError());
		CloseHandle(hDevice);
		return false;
	}

/*	// Get sector size of device
	DISK_GEOMETRY dg;

	if (!DeviceIoControl(hDevice, IOCTL_CDROM_GET_DRIVE_GEOMETRY, NULL, 0, &dg, sizeof(dg), &dwBytesReturned, NULL))
	{
		WriteLog("CDROM: IOCTL_CDROM_GET_DRIVE_GEOMETRY ERROR: %d\n", GetLastError());
		CloseHandle(hDevice);
		SetSingleRXData(0x0402);	// DSA ERROR: Focus error, no disc
		return false;
	}
*/
/*
	WriteLog("CDROM: Cylinder: %d\n", dg.Cylinders);
	WriteLog("CDROM: Type: %d\n", dg.MediaType);	// RemovableMedia = 11
	WriteLog("CDROM: TracksPerCylinder: %d\n", dg.TracksPerCylinder);
	WriteLog("CDROM: SectorsPerTrack: %d\n", dg.SectorsPerTrack);
	WriteLog("CDROM: BytesPerSector: %d\n", dg.BytesPerSector);
*/

	// Get full TOC info of CD

	ZeroMemory(&CDTocEx, sizeof(CDTocEx));
	ZeroMemory(&CDFullToc, sizeof(CDFullToc));

	CDTocEx.Format = CDROM_READ_TOC_EX_FORMAT_FULL_TOC;
	CDTocEx.Msf = 1;							// 0 = LBA, 1 = MSF
	CDTocEx.SessionTrack = WantedSession + 1;	// Jaguar session is zero based, Windows one based
	//CDTocEx.Reserved1 = 0;
	//CDTocEx.Reserved2 = 0;
	//CDTocEx.Reserved3 = 0;

	if (!DeviceIoControl(hDevice, IOCTL_CDROM_READ_TOC_EX, &CDTocEx, sizeof(CDTocEx), &CDFullToc, sizeof(CDFullToc), &dwBytesReturned, NULL))
	{
		WriteLog("CDROM: IOCTL_CDROM_READ_TOC_EX ERROR: %d\n", GetLastError());
		CloseHandle(hDevice);
		return false;
	}
	CloseHandle(hDevice);

	// Extract toc data of session
	CDFullTocToSessionToc(WantedSession);

	return true;
}

void CDGetTocInfo(uint8 WantedSession, uint8 TOCInfoType)
{
	// Clear TOC data
	ZeroMemory(&CDToc, sizeof(CDToc));

	// "Clear" receive data array
	RXDataIndex = 0;
	MaxRXDataCount = 0;

	// Clear "Disc present/TOC" bit
	CDStatus &= ~CD_PRESENT_TOC_READ;
	// Clear "Multi Session Disc" bit
	CDStatus &= ~MULTI_SESSION_CD;

	if (bUseASPI)
	{
		// Get CD TOC data with ASPI (WNASPI32.dll needed <- CD Burner SW)
		if (!ASPI_GetRawTOC(WantedSession))		// Read TOC
		{
			SetSingleRXData(0x0402);	// DSA ERROR: Focus error, no disc
			return;
		}
	}
	else
	{
		// Get CD TOC data with IOCTL (only Windows 2000, Windows XP, ...)
		if (!IOCTL_GetRawTOC(WantedSession))	// Read TOC
		{
			SetSingleRXData(0x0402);	// DSA ERROR: Focus error, no disc
			return;
		}
	}

	WriteLog("CDROM: First session: %d\n", CDToc.FirstSession);
	WriteLog("CDROM: Last session: %d\n", CDToc.LastSession);

	// Set "Disc present/TOC read" bit
	CDStatus = CDStatus | CD_PRESENT_TOC_READ;

	// More than session #1 ?
	if (CDToc.LastSession > 1)
	{
		// Set "Multi Session Disc" bit
		CDStatus = CDStatus | MULTI_SESSION_CD;
		WriteLog("CDROM: Multi session disc: %d sessions.\n", CDToc.LastSession);
	}

	// Asking for number of sessions ?
	if (TOCInfoType == SESSIONS)
	{
		SetSingleRXData(0x5400 | CDToc.LastSession);	// Number of sessions
		return;
	}

	// Check whether wanted session number (zero based) exists
	if (WantedSession > (CDToc.LastSession - 1))
	{
		// Session not found
		SetSingleRXData(0x0429);	// DSA ERROR: illegal value
		return;
	}

	// Asking for "simple" TOC ?
	if (TOCInfoType == TOC)
	{
		WriteLog("CDROM: Session: %02d, first track: %02d, last track: %02d, lead out: %02d:%02d.%02d\n",
									WantedSession,
									CDToc.FirstTrack,
									CDToc.LastTrack,
									CDToc.LeadOut.Minutes,
									CDToc.LeadOut.Seconds,
									CDToc.LeadOut.Frames);

		// Get the TOC data for the wanted session and lead out time
		RXData[0] = 0x2000 | CDToc.FirstTrack;
		RXData[1] = 0x2100 | CDToc.LastTrack;
		RXData[2] = 0x2200 | CDToc.LeadOut.Minutes;
		RXData[3] = 0x2300 | CDToc.LeadOut.Seconds;
		RXData[4] = 0x2400 | CDToc.LeadOut.Frames;
		RXDataIndex = 0;
		MaxRXDataCount = 5;
		//WriteLog("BUTCH: Prepared data count: $%04X\n", MaxRXDataCount);
		return;
	}

	// Asking for long TOC ?
	if (TOCInfoType == LONG_TOC)
	{
		for (int Trk=CDToc.FirstTrack; Trk<=CDToc.LastTrack; Trk++)
		{
			// Add each track info to "Receive Data" array
			WriteLog("CDROM: Long TOC, Current track: %d, Start %02d:%02d.%02d\n",
										Trk,
										CDToc.Track[Trk].Minutes,
										CDToc.Track[Trk].Seconds,
										CDToc.Track[Trk].Frames);

			RXData[MaxRXDataCount + 0] = 0x6000 | Trk;
			RXData[MaxRXDataCount + 1] = 0x6100 | 0x01;		// Jaguar CD sets this
			RXData[MaxRXDataCount + 2] = 0x6200 | CDToc.Track[Trk].Minutes;
			RXData[MaxRXDataCount + 3] = 0x6300 | CDToc.Track[Trk].Seconds;
			RXData[MaxRXDataCount + 4] = 0x6400 | CDToc.Track[Trk].Frames;
			RXDataIndex = 0;
			MaxRXDataCount += 5;
			//WriteLog("BUTCH: Prepared data count: $%04X\n", MaxRXDataCount);

		}
		// Add track end info to "Receive Data" array
		RXData[MaxRXDataCount + 0] = 0x6000;
		RXData[MaxRXDataCount + 1] = 0x6100;
		RXData[MaxRXDataCount + 2] = 0x6200;
		RXData[MaxRXDataCount + 3] = 0x6300;
		RXData[MaxRXDataCount + 4] = 0x6400;
		RXDataIndex = 0;
		MaxRXDataCount += 5;
		//WriteLog("BUTCH: Prepared data count: $%04X\n", MaxRXDataCount);
		return;
	}

	// Asking for session start ?
	if (TOCInfoType == SPINUP_SESSION)
	{
		// Get start of first track of session
		CDReadPositionMSF.Minutes = CDToc.Track[CDToc.FirstTrack].Minutes;
		CDReadPositionMSF.Seconds = CDToc.Track[CDToc.FirstTrack].Seconds;
		CDReadPositionMSF.Frames = CDToc.Track[CDToc.FirstTrack].Frames;

		WriteLog("CDROM: Start of session %d: %02d:%02d.%02d\n",
									CDReadPositionMSF.Minutes,
									CDReadPositionMSF.Seconds,
									CDReadPositionMSF.Frames);

		// Use read to check for valid position
		CDReadData();		// Sets DSA Found or DSA Error

		return;
	}
}

bool ASPI_CDRawReadSector(uint32 Sector)
{
	uint8 cdb[12];

	for (uint8 Try = 0; Try <= vjs.ErrorRetry ; Try++)
	{
		memset(cdb, 0, sizeof(cdb));
		// 0: command, 2-5: sector # (hi->lo) 6-8: number of sectors to read
		// 9: read type, 10: subchannel select
		cdb[0] = 0xBE;								// Code for ReadCD CDB12 command
		//cdb[0] = SCSI_READ10;
		//cdb[0] = 0xD8;
		cdb[2] = Sector >> 24;
		cdb[3] = Sector >> 16;
		cdb[4] = Sector >> 8;
		cdb[5] = Sector;
		//cdb[2] = HIBYTE(HIWORD(Sector));
		//cdb[3] = LOBYTE(HIWORD(Sector));
		//cdb[4] = HIBYTE(LOWORD(Sector));
		//cdb[5] = LOBYTE(LOWORD(Sector));
		// No. of sectors to read from CD (LSB)
		//cdb[6] = (Count >> 16) & 0xFF;
		//cdb[7] = (Count >> 8) & 0xFF;
		cdb[8] = 1;
		// Sync + all headers + user data + EDC/ECC
		cdb[9] = 0xF8;
		// Selects read RAW 96 bytes/sector sub-channel data (Raw P-W)
		//cdb[10] = 1;		Sub channel data is not of interest
		if (SendAsyncASPICmd(haID[UseCDDriveNumber], tID[UseCDDriveNumber], cdb, sizeof(cdb), CDReadBuffer, sizeof(CDReadBuffer), SRB_DIR_IN))
		{
			// Read ok, quit retry loop
			return true;
			break;
		}
	}
	WriteLog("CDROM: Read error at sector %d\n", Sector);
	return false;
}

bool IOCTL_CDRawReadSector(uint32 Sector)
{
	DWORD dwBytesReturned = 0;
	RAW_READ_INFO CDRawReadInfo;

	HANDLE hDevice = CreateFile(UseCDDriveLetter, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
                              FILE_FLAG_OVERLAPPED|FILE_ATTRIBUTE_NORMAL, NULL);
	if (!hDevice)
	{
		WriteLog("CDROM: ERROR: %d, no device handle at sector %d\n", GetLastError(), Sector);
		CloseHandle(hDevice);
		return false;
	}

	DWORD Result;
	OVERLAPPED overlapped;

	for (uint8 Try = 0; Try <= vjs.ErrorRetry ; Try++)
	{
		// Initialize CDROM_RAW_READ structure
		// Sector size for offset must be 2048 NOT 2352
		CDRawReadInfo.DiskOffset.QuadPart = 2048 * Sector;
		CDRawReadInfo.SectorCount = 1;						// 1 block/sector		
		CDRawReadInfo.TrackMode = CDDA;

		// Asynchronous read
		memset(&overlapped, 0, sizeof(overlapped));
		overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		if (overlapped.hEvent == NULL)
	    {
		    Result = GetLastError();
			WriteLog("CDROM: Error %d getting event handle at sector %d\n", Result, Sector);
		}
		else
		{
			if (!DeviceIoControl(hDevice, IOCTL_CDROM_RAW_READ, &CDRawReadInfo, sizeof(RAW_READ_INFO),
								//data,
								//RAW_SECTOR_SIZE * nblocks,
								CDReadBuffer,
								sizeof(CDReadBuffer),
								&dwBytesReturned, &overlapped))
			{
				Result = GetLastError();
				if (Result == ERROR_IO_PENDING)	// = 997
			    {
				    if (!GetOverlappedResult(hDevice, &overlapped, &dwBytesReturned, TRUE))
					{
						Result = GetLastError();
						WriteLog("CDROM: Error %d waiting for CDROM IOCTL at sector %d\n", Result, Sector);
						//break;
					}
					else
					{
						Result = NO_ERROR;
					}
				}
				else
				{
					WriteLog("CDROM: IOCTLING CDROM ERROR %d at sector %d\n", Result, Sector);
					//break;
				}
			}
			CloseHandle(overlapped.hEvent);
			if (Result == NO_ERROR)
			{
				// Quit read retry loop
				break;
			}
#ifdef CDROM_LOG
		WriteLog("CDROM: Raw read, bytes: %d, error: %d\n", dwBytesReturned, Result);
#endif
		}
	}
	CloseHandle(hDevice);
	if (Result == NO_ERROR)
		return true;
	else
		return false;
}

bool CDRawRead(uint32 Sector)
{
	if (bUseASPI)
	{
		// Get CD data with ASPI (WNASPI32.dll needed <- CD Burner SW)
		return ASPI_CDRawReadSector(Sector);
	}
	else
	{
		// Get CD data with IOCTL (only Windows 2000, Windows XP, ...)
		return IOCTL_CDRawReadSector(Sector);
	}
}

void LogRawData(char * Buffer, uint32 Size)
{
	// Log the content
	char szItem[4] = "\0";
	char * pszBuf = (char*)malloc(Size * 4);
	*pszBuf = 0;	// Set end of string
	for (uint32 i = 0; i < Size ; i++)
	{
		sprintf(szItem, "%02x ", (unsigned char)Buffer[i]);
		strcat(pszBuf, szItem);
	}
	WriteLog("CDROM: Sector data: %s\n", pszBuf);
	free(pszBuf);
}

void CDReadData()
{
	DWORD Result;

	// Predefine read fault, no data
	bCDRead = false;
	CDReadBufferIndex = RAW_SECTOR_SIZE;		// Index out of array, buffer empty

	// Calculate offset where read starts
	uint32 CDSectorOffset = (CDReadPositionMSF.Minutes * CD_BLOCKS_PER_SECOND * 60) +
						(CDReadPositionMSF.Seconds * CD_BLOCKS_PER_SECOND) +
						CDReadPositionMSF.Frames - (2 * CD_BLOCKS_PER_SECOND);	// necessary "alignment" (150)

#ifdef CDROM_LOG_RAW_READ
	WriteLog("CDROM: Raw read, minutes: %d, seconds: %d, frames: %d, sector: %d\n",
							CDReadPositionMSF.Minutes, CDReadPositionMSF.Seconds, CDReadPositionMSF.Frames, CDSectorOffset);
#endif

	// Do raw read from CD
	if (!CDRawRead(CDSectorOffset))
	{
		SetSingleRXData(0x0402);	// DSA ERROR: Focus error, no disc
		return;
	}
#ifdef CDROM_LOG_RAW_DATA
	WriteLog("READ BUFFER SECTOR %d:", CDSectorOffset);
	LogRawData((char*)CDReadBuffer, RAW_SECTOR_SIZE);
#endif

	// Increment position
	CDReadPositionMSF.Frames++;
	if (CDReadPositionMSF.Frames >= CD_BLOCKS_PER_SECOND)
	{
		CDReadPositionMSF.Frames = CDReadPositionMSF.Frames - CD_BLOCKS_PER_SECOND;
		CDReadPositionMSF.Seconds++;
		if (CDReadPositionMSF.Seconds >= 60)
		{
			CDReadPositionMSF.Seconds = CDReadPositionMSF.Seconds - 60;
			CDReadPositionMSF.Minutes++;
		}
	}

	bCDRead = true;				// Read ok, data available
	CDReadBufferIndex = 0;		// Index to start of buffer
	SetSingleRXData(0x0100);	// DSA Found

	return;
}

//
// Initialize the Win32 ASPI layer
//
bool InitASPI(void)
{
	hASPILib = LoadLibrary("WNASPI32");
	if (!hASPILib)
	{
		WriteLog("CDROM: Could not load WNASPI32.DLL!\n");
		return false;
	}

	ASPI_GetASPI32SupportInfo = (uint32 (*)(void))GetProcAddress(hASPILib, "GetASPI32SupportInfo");
	ASPI_SendASPI32Command = (uint32 (*)(LPSRB))GetProcAddress(hASPILib, "SendASPI32Command");

	if (!ASPI_GetASPI32SupportInfo || !ASPI_SendASPI32Command)
	{
		WriteLog("CDROM: Could not import functions from WNASPI32.DLL!\n");
		return false;
	}

	uint32 SupportInfo = ASPI_GetASPI32SupportInfo();	// Initialize ASPI layer
	uint8 Status = (SupportInfo >> 8) & 0xFF;
	uint8 maxHostAdapters = SupportInfo & 0xFF;

	//WriteLog("HostAdapters: %d\n", maxHostAdapters);

	if ((Status != SS_COMP) && (Status != SS_NO_ADAPTERS))
	{
		WriteLog("CDROM: Could not initialise using GetASPI32SupportInfo function!\n");
		return false;
	}

	if (Status == SS_NO_ADAPTERS)
	{
		WriteLog("CDROM: ASPI initialized, but no host adapters were found!\n");
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
	srbTimeouts.SRB_HaId = 1;//0xFF;
	srbTimeouts.SRB_Target = 0xFF;
	srbTimeouts.SRB_Lun = 0xFF;
	srbTimeouts.SRB_Timeout = 15 * 2;
	ASPI_SendASPI32Command(&srbTimeouts);
	WriteLog("CDINTF: Set Timeout command returned %02X...\n", srbTimeouts.SRB_Status);//*/

	WriteLog("CDROM: ASPI successfully initialized.\n");

	SRB_HAInquiry srbHAInquiry;
	SRB_GDEVBlock srbGDEVBlock;
	uint8 inquiryCDB[6], inquiryBuf[36];

	uint8 numCDDrives = 0;		// No CD drives yet

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

			//WriteLog("HaID: %d, Target: %d, Device type: %d\n", HAID, target, srbGDEVBlock.SRB_DeviceType);

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
			uint32 VendorIDSize = 0, ProductIDSize = 0, ProductRevSize = 0;
			// Byte 8–15: Vendor identification
			for(int i=7; i>=0; i--)
			{
				if (inquiryBuf[8+i] != ' ')
				{
					VendorIDSize = i + 1;
					break;
				}
			}
			// Byte 16–31: Product identification
			for(int i=15; i>=0; i--)
			{
				if (inquiryBuf[16+i] != ' ')
				{
					ProductIDSize = i + 1;
					break;
				}
			}
			// Byte 32–35: Product revision level
			for(int i=3; i>=0; i--)
			{
				if (inquiryBuf[32+i] != ' ')
				{
					ProductRevSize = i + 1;
					break;
				}
			}

			memcpy(driveName[numCDDrives], inquiryBuf + 8, VendorIDSize);
			driveName[numCDDrives][VendorIDSize] = ' ';
			memcpy(driveName[numCDDrives] + VendorIDSize + 1, inquiryBuf + 16, ProductIDSize);
			driveName[numCDDrives][VendorIDSize + ProductIDSize + 1] = ' ';
			memcpy(driveName[numCDDrives] + VendorIDSize + ProductIDSize + 2, inquiryBuf + 32, ProductRevSize);
			driveName[numCDDrives][VendorIDSize + ProductIDSize + ProductRevSize + 2] = 0;

			WriteLog("CDROM: Found CD-ROM device [%s]. HAID:%u, TID:%u, LUN:0\n", driveName[numCDDrives], haID[numCDDrives], tID[numCDDrives]);

			// Use first CD drive as default
			if (numCDDrives == 0)
				UseCDDriveNumber = 0;
			// Use specified CD drive if available
			if (numCDDrives == vjs.CDDrive)
				UseCDDriveNumber = vjs.CDDrive;

			// Next CD drive
			numCDDrives++;
		}
	}

	if (numCDDrives == 0)
	{
		WriteLog("CDROM: No CDROM type drives found.\n");
		return false;
	}
	WriteLog("CDROM: Using CD drive HAID:%d, TID:%d, LUN:0\n", haID[UseCDDriveNumber], tID[UseCDDriveNumber]);
	return true;
}

void InitCDWin32()
{
	char tmpCDDriveLetter[4] = "\0";
	char szDrivesBuffer[26*4];		// 26 times "X:\0"
	//char VolumeName[MAX_PATH];

	// Get all logical drive letters, "A:\" - "Z:\"
	int32 BufferLen = GetLogicalDriveStrings(sizeof(szDrivesBuffer), szDrivesBuffer); 

	int8 Pos = 0;
	int8 numCDDrives = 0;

	while (Pos < BufferLen) 
	{
		// Get current drive letter
		char * pDrive = szDrivesBuffer + Pos;

		// Get drive type
		if (GetDriveType(pDrive) == DRIVE_CDROM)
		{
			WriteLog("CDROM: %s found.\n", pDrive);

			// Use first CD drive as default
			if (numCDDrives == 0)
				strncpy(tmpCDDriveLetter, pDrive, 2);
			// Use specified CD drive
			if (numCDDrives == vjs.CDDrive)
				strncpy(tmpCDDriveLetter, pDrive, 2);

			// Next CD drive
			numCDDrives++;
		}
		// Set position to next drive string
	    Pos = Pos + 4;
	}

	// Only VolumeName requested
	//GetVolumeInformation(tmpCDDriveLetter, VolumeName, MAX_PATH, NULL, NULL, NULL, NULL, 0);

	WriteLog("CDROM: Using drive %s\n", tmpCDDriveLetter);
	// Add prefix for CreateFile
	strcpy(UseCDDriveLetter, "\\\\.\\");
	strcat(UseCDDriveLetter, tmpCDDriveLetter);

	// Ensure that read loop performs at least one read
	if (vjs.ErrorRetry < 0)
		 vjs.ErrorRetry = 0; 

	// Predefine using ASPI
	bUseASPI = true;
	// Check whether drive is accessible with IOCTL (only Windows NT/2000/XP/...)
	HANDLE hDevice = CreateFile(UseCDDriveLetter, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hDevice)
	{
		// Get sector size of device
		DISK_GEOMETRY	dg;
		DWORD			dwBytesReturned;

		if (!DeviceIoControl(hDevice, IOCTL_CDROM_GET_DRIVE_GEOMETRY, NULL, 0, &dg, sizeof(dg), &dwBytesReturned, NULL))
		{
			// DeviceIOControl failed
			// Either no CD inserted or no Windows NT/2000/XP/... system
			// we try ASPI
		}
		else
		{
			// DeviceIOControl OK -> Windows NT/2000/XP/... system
			// Use IOCTL, not ASPI
			bUseASPI = false;
		}
		CloseHandle(hDevice);	// Release handle -> release drive
	}
	//bUseASPI = true;	// For testing ASPI
	if (bUseASPI)
	{
		// Trying ASPI
		InitASPI();
	}
}

void CDWin32Done()
{
	if (hASPILib)
		FreeLibrary(hASPILib);		// Unload ASPI library if it was loaded.
}
