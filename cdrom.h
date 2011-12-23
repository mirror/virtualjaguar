//
// CDROM.H
//

#ifndef __CDROM_H__
#define __CDROM_H__

#define RAW_SECTOR_SIZE		2352	// Real raw data stored on CD per sector/block
#define BLOCKS_TO_READ		1
#define MAX_TRACKS			100		// Maximum number of tracks on CD

// CD status
#define CD_PRESENT_TOC_READ	0x01
#define MULTI_SESSION_CD	0x02
#define CD_IN_PAUSE_MODE	0x04
// bit 3 not used
#define CAROUSEL_SLOT1		0x10

struct s_CD_POSITION_MSF
{
	uint8	Minutes;
	uint8	Seconds;
	uint8	Frames;
};

struct s_CD_SESSION_TOC
{
	uint8 FirstSession;
    uint8 LastSession;
	uint8 FirstTrack;
	uint8 LastTrack;
	s_CD_POSITION_MSF LeadOut;
	s_CD_POSITION_MSF Track[MAX_TRACKS];
};

enum TOC_Info_Type { TOC, LONG_TOC, SESSIONS, SPINUP_SESSION };

void CDROMInit(void);
void CDROMReset(void);
void CDROMDone(void);
void InitCDWin32(void);
void CDWin32Done(void);

void BUTCHExec(void);

uint8 CDROMReadByte(uint32 address, uint32 who = UNKNOWN);
uint16 CDROMReadWord(uint32 address, uint32 who = UNKNOWN);
void CDROMWriteByte(uint32 address, uint8 data, uint32 who = UNKNOWN);
void CDROMWriteWord(uint32 address, uint16 data, uint32 who = UNKNOWN);

bool ButchIsReadyToSend(void);
uint16 GetWordFromButchSSI(uint32 offset, uint32 who = UNKNOWN);
void SetSSIWordsXmittedFromButch(void);

void SetSingleRXData(uint16 data);

void CDGetTocInfo(uint8 WantedSession, uint8 TOCInfoType);
void CDReadData(void);

#endif	// __CDROM_H__
