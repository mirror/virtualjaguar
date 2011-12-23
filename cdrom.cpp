//
// CD handler
//
// Originally by David Raingeard
// GCC/SDL port by Niels Wagenaar (Linux/WIN32) and Caz (BeOS)
// Extensive rewrites/cleanups/fixes by James L. Hammons
// Extensive rewrites by Stephan Kapfer
//

#include "jaguar.h"									// For GET32/SET32 macros
//#include "m68k.h"
//#include "cdintf.h"								// System agnostic CD interface functions
//#include "cdrom.h"

#include <windows.h>		// for MessageBox
#include <time.h>			// for initializing random

//#define CDROM_LOG									// For CDROM logging, obviously
//#define CDROM_LOG_ATRI							// Log ATRI identifiers of CD data
//#define FIFO_LOG
//#define BUTCH_IRQ_LOG

void GPUDumpDisassembly(void);
void GPUDumpRegisters(void);

/*
BUTCH     equ  $DFFF00		; base of Butch=interrupt control register, R/W
DSCNTRL   equ  BUTCH+4		; DSA control register, R/W
DS_DATA   equ  BUTCH+$A		; DSA TX/RX data, R/W
I2CNTRL   equ  BUTCH+$10	; i2s bus control register, R/W
SBCNTRL   equ  BUTCH+$14	; CD subcode control register, R/W
SUBDATA   equ  BUTCH+$18	; Subcode data register A
SUBDATB   equ  BUTCH+$1C	; Subcode data register B
SB_TIME   equ  BUTCH+$20	; Subcode time and compare enable (D24)
FIFO_DATA equ  BUTCH+$24	; i2s FIFO data
I2SDATA1  equ  BUTCH+$24	; i2s FIFO data
I2SDATA2  equ  BUTCH+$28	; i2s FIFO data
I2SBUS    equ  BUTCH+$2E	; i2s serial bus

;
; Butch's hardware registers
;
;BUTCH     equ  $DFFF00		;base of Butch=interrupt control register, R/W
;
;  When written (Long):
;
;  bit0 - set to enable interrupts
;  bit1 - enable CD data FIFO half full interrupt
;  bit2 - enable CD subcode frame-time interrupt (@ 2x spped = 7ms.)
;  bit3 - enable pre-set subcode time-match found interrupt
;  bit4 - CD module command transmit buffer empty interrupt
;  bit5 - CD module command receive buffer full
;  bit6 - CIRC failure interrupt
;
;  bit7-31  reserved, set to 0
;
;  bit19 - set to enable reset of jag when opening CD lid
;  bit20 - set to enable BUTCH
;
;  When read (Long):
;
;  bit0-8 reserved
;
;  bit9  - CD data FIFO half-full flag pending
;  bit10 - Frame pending
;  bit11 - Subcode data pending
;  bit12 - Command to CD drive pending (trans buffer empty if 1)
;  bit13 - Response from CD drive pending (rec buffer full if 1)
;  bit14 - CD uncorrectable data error pending
;
;   Offsets from BUTCH
;
O_DSCNTRL   equ  4		; DSA control register, R/W
O_DS_DATA   equ  $A		; DSA TX/RX data, R/W
;
O_I2CNTRL   equ  $10		; i2s bus control register, R/W
;
;  When read:
;
;  b0 - I2S data from drive is ON if 1
;  b1 - I2S path to Jerry is ON if 1
;  b2 - I2S data transfer is ON if 1
;  b3 - host bus width is 16 if 1, else 32
;  b4 - FIFO state is not empty if 1
;
O_SBCNTRL   equ  $14		; CD subcode control register, R/W
O_SUBDATA   equ  $18		; Subcode data register A
O_SUBDATB   equ  $1C		; Subcode data register B
O_SB_TIME   equ  $20		; Subcode time and compare enable (D24)
O_FIFODAT   equ  $24		; i2s FIFO data
O_I2SDAT2   equ  $28		; i2s FIFO data (old)
*/

/*
Commands sent through DS_DATA:

$01nn - ? Play track nn ? Seek to track nn ?
$0200 - Stop CD
$03nn - Read session nn TOC (short)
$0400 - Pause CD
$0500 - Unpause CD
$10nn - Goto (min?)
$11nn - Goto (sec?)
$12nn - Goto (frm?)
$14nn - Read session nn TOC (full)
$15nn - Set CD mode
$18nn - Spin up CD to session nn
$5000 - ?
$5100 - Mute CD (audio mode only)
$51FF - Unmute CD (audio mode only)
$5400 - Read # of sessions on CD
$70nn - Set oversampling mode

Eeprom in Jaguar CD: IC 93C46, 64 * 16 bits

Serial bus:
; b0 - Acknowledge line
; b1 - Strobe line
; b2 - Transmit data line
; b3 - Receive data line

Commands send through serial bus:
$100 - Lock the NVRAM against writing
$130 - Unlock NVRAM for writing
$14x - Write to cell x ($00-$3f)
$18x - Read from cell x ($00-$3f)
Command is a 9-bit value
Data per cell is a 16-bit value

8 serial reads of NVRAM on $DFFF2E show the following:
Cmd: $184 (read cell 4)
1111 0110 1100 1010
1111 0101 1111 0000
1111 0111 0010 0100
1111 0101 0010 0111
1111 0101 1101 0000
1111 0101 1110 0111
1111 0111 1111 1001
1111 0110 0100 1011
1111 01xx xxxx xxxx
Cmd: $185 (read cell 5)
1001 1110 1101 1000
0111 0010 0101 1000
1101 1101 0000 0110
1111 1011 0101 1010
1110 0111 0001 1010
1011 1101 1101 1110
1000 0100 0011 1010
1100 0000 1010 1100
xxxx xxxx xxxx xxx0
Seems that only bits marked with x do change
*/

// Lines used by CD EEPROM BUS
#define CDEEPROMBUS_ACK	0x01
#define CDEEPROMBUS_STB	0x02
#define CDEEPROMBUS_TXD	0x04
#define CDEEPROMBUS_RXD	0x08

enum CDEepromBusState { STATE_LOW, STATE_HIGH, STATE_RISING, STATE_FALLING };
uint8 Line_ACK, Line_STB, Line_TXD, Line_RXD;

// Predefine cells used by audio menu
uint8 CDEepromCell_00_Data[16] = { 0,0,0,0, 0,0,0,0, 0,0,1,0, 0,1,0,0 };
uint8 CDEepromCell_01_Data[16] = { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,1,0,0 };

// Predefine  8 different 'Receive Data' for cell 4 and 5 in Eeprom of Jaguar CD
// 0 = STATE_LOW, 1 = STATE_HIGH
uint8 CDEepromCell_04_Data[8][16] =
{ 1,1,1,1, 0,1,1,0, 1,1,0,0, 1,0,1,0,
  1,1,1,1, 0,1,0,1, 1,1,1,1, 0,0,0,0, 
  1,1,1,1, 0,1,1,1, 0,0,1,0, 0,1,0,0, 
  1,1,1,1, 0,1,0,1, 0,0,1,0, 0,1,1,1,
  1,1,1,1, 0,1,0,1, 1,1,0,1, 0,0,0,0,
  1,1,1,1, 0,1,0,1, 1,1,1,0, 0,1,1,1,
  1,1,1,1, 0,1,1,1, 1,1,1,1, 1,0,0,1,
  1,1,1,1, 0,1,1,0, 0,1,0,0, 1,0,1,1 };
uint8 CDEepromCell_05_Data[8][16] =
{ 1,0,0,1, 1,1,1,0, 1,1,0,1, 1,0,0,0,
  0,1,1,1, 0,0,1,0, 0,1,0,1, 1,0,0,0,
  1,1,0,1, 1,1,0,1, 0,0,0,0, 0,1,1,0,
  1,1,1,1, 1,0,1,1, 0,1,0,1, 1,0,1,0,
  1,1,1,0, 0,1,1,1, 0,0,0,1, 1,0,1,0,
  1,0,1,1, 1,1,0,1, 1,1,0,1, 1,1,1,0,
  1,0,0,0, 0,1,0,0, 0,0,1,1, 1,0,1,0,
  1,1,0,0, 0,0,0,0, 1,0,1,0, 1,1,0,0 };

uint8 CDEepromIntro;
bool bCDEepromBusSending;
uint16 CDEepromDataBitCounter;
uint16 CDEepromBusData;
uint16 CDEepromBusCommand;

// Private function prototypes

static void CDEepromBusWrite(uint16);
static uint16 CDEepromBusRead(void);
static void BUTCHExecI2S(void);

#define BUTCH		0xDFFF00			// Butch=interrupt control register, R/W
#define DSCNTRL 	BUTCH + 0x04		// DSA control register, R/W
#define DS_DATA		BUTCH + 0x0A		// DSA TX/RX data, R/W
#define I2CNTRL		BUTCH + 0x10		// i2s bus control register, R/W
#define SBCNTRL		BUTCH + 0x14		// CD subcode control register, R/W
#define SUBDATA		BUTCH + 0x18		// Subcode data register A
#define SUBDATB		BUTCH + 0x1C		// Subcode data register B
#define SB_TIME		BUTCH + 0x20		// Subcode time and compare enable (D24)
#define FIFO_DATA	BUTCH + 0x24		// i2s FIFO data
#define I2SDATA1	BUTCH + 0x24		// i2s FIFO data
#define I2SDATA2	BUTCH + 0x28		// i2s FIFO data
#define EEPROMBUS	BUTCH + 0x2C		// Serial interface for eeprom

char * BReg[12] = { "BUTCH", "DSCNTRL", "DS_DATA", "???", "I2CNTRL", "SBCNTRL", "SUBDATA", "SUBDATB",
	"SB_TIME", "FIFO_DATA", "I2SDAT2", "UNKNOWN" };
extern char * whoName[9];

static uint8 cdRam[0x100];

#define BUTCH_INTS_ENABLE		0x0001
#define DSA_FIFO_INT_ENABLE		0x0002
#define DSA_FRAME_INT_ENABLE	0x0004
#define DSA_SUBCODE_INT_ENABLE	0x0008
#define DSA_TX_INT_ENABLE		0x0010
#define DSA_RX_INT_ENABLE		0x0020
#define DSA_ERROR_INT_ENABLE	0x0040
// bits 7,8 unused
#define DSA_FIFO_INT_PENDING	0x0200
#define DSA_FRAME_INT_PENDING	0x0400
#define DSA_SUBCODE_INT_PENDING	0x0800
#define DSA_TX_INT_PENDING		0x1000
#define DSA_RX_INT_PENDING 		0x2000
#define DSA_ERROR_INT_PENDING	0x4000

/* I2SCNTRL
; b0 - I2S data from drive is ON if 1
; b1 - I2S path to Jerry is ON if 1
; b2 - Enable data transfer (subcode ???)
; b3 - host bus width is 16 if 1, else 32
; b4 - FIFO state is not empty if 1
*/
#define I2S_DATA_FROM_CD	0x01
#define I2S_DATA_TO_JERRY	0x02
#define I2S_DATA_ENABLE		0x04
#define I2S_BUS_WIDTH16		0x08
#define I2S_DATA_IN_FIFO	0x10

bool butch_enabled = true;	// Enable per default for simple CD position gui
uint16 butch_control;

//uint16 WaitTX;
uint16 NewCmdTX;
uint16 RXData[512*2];			// 512 words receive data should be enough
uint16 RXDataIndex;
uint16 MaxRXDataCount;
uint8 CDReadBuffer[RAW_SECTOR_SIZE * BLOCKS_TO_READ];
uint16 CDReadBufferIndex;
uint8 FiFoDataSize;
// Docu talks about "FiFo half full interrupt", CDBIOS reads 32 bytes on each FiFo interrupt
// so FiFo seems to have 64 bytes
uint8 FiFo[64];

s_CD_POSITION_MSF CDGotoMSF;
s_CD_POSITION_MSF CDPlayStartMSF;
s_CD_POSITION_MSF CDPlayStopMSF;
s_CD_POSITION_MSF CDReadPositionMSF;
bool bCDRead;
uint8 CDStatus;
uint8 CDMode;
uint8 CDDACMode;
//uint8 CDRomMode;
uint8 CDLastError;

// Used for I2S interface of Jerry and Butch
void GenerateI2SClock(uint16 I2SFrequency);

extern uint8 I2S_SerialMode;

extern uint8 I2SClkState;
extern uint8 I2SWSBitCount;
extern bool I2SWSHigh;

extern uint16 ButchShiftTXD;


void CDROMInit(void)
{
	InitCDWin32();
}

void CDROMReset(void)
{
	// Reset Eeprom bus
	Line_ACK = STATE_LOW;
	Line_STB = STATE_LOW;
	Line_TXD = STATE_LOW;
	Line_RXD = STATE_LOW;
	bCDEepromBusSending = false;			// No sending
	CDEepromDataBitCounter = 0;
	CDEepromBusData = 0;
	CDEepromBusCommand = 0;

	memset(cdRam, 0x00, 0x100);				// Clear internal CD ram

	butch_control = 0;						// Clear butch
	NewCmdTX = 0;							// No command to transmit
	RXDataIndex = 0;						// Start index of data
	MaxRXDataCount = 0;						// No data to receive
	CDReadBufferIndex = RAW_SECTOR_SIZE;	// Index out of array, buffer empty
	FiFoDataSize = 0;						// No FiFo content

	bCDRead = false;						// No read in progress

	CDStatus = CAROUSEL_SLOT1;				// No disc/no TOC, no multi session disc, no PAUSE mode, Carousel slot 1
	CDMode = 0x21;							// Single speed, Audio mode, ATTI
	CDDACMode = 0x01;						// I2S - FS mode
	//CDRomMode = ;							// Not used
	CDLastError = 0;						// No error

	srand((unsigned)time(NULL));			// Initialize random

	/*I2SButchWSState = false;
	I2SButchWSRising = false;
	I2SButchWSBitCount = 16;
	I2SButchDataBitCount = 1;*/
}

void CDROMDone(void)
{
	CDWin32Done();
}

void ReduceFiFo(uint8 BytePos, uint8 SizeInBytes)
{
	// Enough data for move to left ?
	if (FiFoDataSize > (BytePos + SizeInBytes))
	{
		// Copy content to the left in FiFo
		uint8 RemainingBytes = BytePos + SizeInBytes;
		// Bytes at the end are not cleared cause not used
		memcpy(FiFo + BytePos, FiFo + RemainingBytes, FiFoDataSize - RemainingBytes);				
	}
	// Reduce FIFO size
	FiFoDataSize = FiFoDataSize - SizeInBytes;
	if (FiFoDataSize <= 0)
	{
		// FIFO is empty
		FiFoDataSize = 0;
		// Clear bit 4 of I2CNTRL
		uint8 offset = (I2CNTRL+2) & 0xFF;
		uint16 data = GET16(cdRam, offset);
		data = data & ~I2S_DATA_IN_FIFO;
		SET16(cdRam, offset, data);
		WriteLog("CDROM: I2CNTRL bit 4 cleared.\n");
	}
#ifdef FIFO_LOG
	WriteLog("CDROM: FIFO, new size: %d\n", FiFoDataSize);
#endif
}

void SetButchControl(uint16 data)
{
	SET16(cdRam, (BUTCH + 2) & 0xFF, data);

#ifdef CDROM_LOG
	if (data & BUTCH_INTS_ENABLE)
		WriteLog("BUTCH: Interrupts enabled.\n");
	if (data & DSA_FIFO_INT_ENABLE)
		WriteLog("BUTCH: FIFO interrupt enabled.\n");
	if (data & DSA_FRAME_INT_ENABLE)
		WriteLog("BUTCH: Frame interrupt enabled.\n");
	if (data & DSA_SUBCODE_INT_ENABLE)
		WriteLog("BUTCH: Subcode interrupt enabled.\n");
	if (data & DSA_TX_INT_ENABLE)
		WriteLog("BUTCH: Transmit buffer empty interrupt enabled.\n");
	if (data & DSA_RX_INT_ENABLE)
		WriteLog("BUTCH: Receive buffer full interrupt enabled.\n");
	if (data & DSA_ERROR_INT_ENABLE)
		WriteLog("BUTCH: Errory interrupt enabled.\n");

	WriteLog("BUTCH: Set butch control: $%04X\n", data);
#endif
}

void BUTCHSetIRQLine(int irqline)
{
	// Are Butch interrupts allowed ?
	if (butch_control && BUTCH_INTS_ENABLE)
	{
		// Fifo interrupt enabled and now wanted ?
		if ((butch_control & DSA_FIFO_INT_ENABLE) && (irqline & DSA_FIFO_INT_PENDING))
		{
#ifdef BUTCH_IRQ_LOG
			WriteLog("BUTCH: Fifo interrupt, generating Jerry external interrupt.\n");
#endif
			JERRYSetIRQLine(JERRY_IRQ_EXTERNAL0);
		}
		if ((butch_control & DSA_FRAME_INT_ENABLE) && (irqline & DSA_FRAME_INT_PENDING))
		{
#ifdef BUTCH_IRQ_LOG
			WriteLog("BUTCH: Frame interrupt, generating Jerry external interrupt.\n");
#endif
			JERRYSetIRQLine(JERRY_IRQ_EXTERNAL0);
		}
		if ((butch_control & DSA_SUBCODE_INT_ENABLE) && (irqline & DSA_SUBCODE_INT_PENDING))
		{
#ifdef BUTCH_IRQ_LOG
			WriteLog("BUTCH: Subcode interrupt, generating Jerry external interrupt.\n");
#endif
			JERRYSetIRQLine(JERRY_IRQ_EXTERNAL0);
		}
		if ((butch_control & DSA_TX_INT_ENABLE) && (irqline & DSA_TX_INT_PENDING))
		{
#ifdef BUTCH_IRQ_LOG
			WriteLog("BUTCH: Transmit buffer empty interrupt, generating Jerry external interrupt.\n");
#endif
			JERRYSetIRQLine(JERRY_IRQ_EXTERNAL0);
		}
		if ((butch_control & DSA_RX_INT_ENABLE) && (irqline & DSA_RX_INT_PENDING))
		{
#ifdef BUTCH_IRQ_LOG
			WriteLog("BUTCH: Receive buffer full interrupt, generating Jerry external interrupt.\n");
#endif
			JERRYSetIRQLine(JERRY_IRQ_EXTERNAL0);
		}
		if ((butch_control & DSA_ERROR_INT_ENABLE) && (irqline & DSA_ERROR_INT_PENDING))
		{
#ifdef BUTCH_IRQ_LOG
			WriteLog("BUTCH: Error interrupt, generating Jerry external interrupt.\n");
#endif
			JERRYSetIRQLine(JERRY_IRQ_EXTERNAL0);
		}	
	}
}

void SetSingleRXData(uint16 data)
{
	RXData[0] = data;
	RXDataIndex = 0;
	MaxRXDataCount = 1;

#ifdef CDROM_LOG
	WriteLog("BUTCH: Prepared data count: $%04X\n", MaxRXDataCount);
#endif

	// DSA error ?
	if (data & 0x0400)
	{
		CDLastError = data & 0XFF;

		// Set DSA_ERROR
		butch_control = butch_control | DSA_ERROR_INT_PENDING;
		SetButchControl(butch_control);
		// Generate interrupt if enabled
		BUTCHSetIRQLine(DSA_ERROR_INT_PENDING);
	}
}

void BUTCHCommand(void)
{
	// Execute command
	if (NewCmdTX != 0)
	{
		// New command to transmit to Butch

		// Set DSA_TX
		// Do NOT clear DSA_RX
		butch_control = (butch_control | DSA_TX_INT_PENDING);// & ~DSA_RX_INT_PENDING;
		SetButchControl(butch_control);

#ifdef CDROM_LOG
		WriteLog("BUTCH: Execute command: $%04X\n", NewCmdTX);
#endif
		// Immediate content of DS_DATA
//		SET16(cdRam, DS_DATA & 0xFF, 0x0407);

		uint8 cmd = NewCmdTX >> 8;
		uint8 cmdData = NewCmdTX & 0xFF;

		NewCmdTX = 0;		// The command will be executed so clear the transferred command

		switch(cmd)
		{
		// Stop: 0x02XX
		case 0x02:
			{
				bCDRead = false;
				// Clear PAUSE bit
				CDStatus = CDStatus & ~CD_IN_PAUSE_MODE;
				// Response: 0x02XX
				SetSingleRXData(0x0200 | cmdData);
				WriteLog("CDROM: Stopped. Pause mode released.\n");
				
				// Simple approach to deactivate butch
				butch_enabled = false;

				break;
			}
		// Read TOC: 0x03<session nr>, first session is 0
		case 0x03:
			{
				// Response:
				// 0x20<minimum track nr>
				// 0x21<maximum track nr>
				// 0x22<maximum time, minutes>
				// 0x23<maximum time, seconds>
				// 0x24<maximum time, frames>
				// or DSA error
				CDGetTocInfo(cmdData, TOC);		// Session number, info type: "simple" TOC

				break;
			}
		// Pause: 0x04XX
		case 0x04:
			{
				// Set PAUSE bit
				CDStatus = CDStatus | CD_IN_PAUSE_MODE;
				// Response: 0x01XX
				SetSingleRXData(0x0100 | cmdData);
				WriteLog("CDROM: Pause mode set.\n");
				break;
			}
		// Pause Release: 0x05XX
		case 0x05:
			{
				// Clear PAUSE bit
				CDStatus = CDStatus & ~CD_IN_PAUSE_MODE;
				// Response: 0x01XX
				SetSingleRXData(0x0100 | cmdData);
				WriteLog("CDROM: Pause mode released.\n");
				break;
			}
		// Get complete time: 0x0DXX
		case 0x0D:
			{
				// Response:
				// 0x14<minutes>
				// 0x15<seconds>
				// 0x16<frames>
				RXData[0] = 0x1400 | CDReadPositionMSF.Minutes;
				RXData[1] = 0x1500 | CDReadPositionMSF.Seconds;
				RXData[2] = 0x1600 | CDReadPositionMSF.Frames;
				RXDataIndex = 0;
				MaxRXDataCount = 3;
				WriteLog("CDROM: Get complete time, minutes: %d, seconds: %d, frames: %d\n",
								CDReadPositionMSF.Minutes,
								CDReadPositionMSF.Seconds,
								CDReadPositionMSF.Frames);
				break;
			}
		// Goto time minutes: 0x10<absolute minutes>
		case 0x10:
			{
				CDGotoMSF.Minutes = cmdData;
				WriteLog("CDROM: Goto minutes: %d\n", CDGotoMSF.Minutes);
				break;
			}
		// Goto time seconds: 0x11<absolute seconds>
		case 0x11:
			{
				CDGotoMSF.Seconds = cmdData;
				WriteLog("CDROM: Goto seconds: %d\n", CDGotoMSF.Seconds);
				break;
			}
		// Goto time frames: 0x12<absolute frames>
		case 0x12:
			{
				CDGotoMSF.Frames = cmdData;
				WriteLog("CDROM: Goto frames: %d\n", CDGotoMSF.Frames);

				// Start reading CD
				CDReadPositionMSF = CDGotoMSF;

				// Use read to check for valid position
				CDReadData();		// Sets DSA Found or DSA Error

				// Ensure that I2S transfer restarts
				I2SWSHigh = true;//true;		// Should be low and start with left data not with right data, 2 byte difference ???!!! 
				I2SWSBitCount = 0;

				break;
			}
		// Read long TOC: 0x14<session nr>, first session is 0
		case 0x14:
			{
				// Response:
				// if 0 <= track number <= 63
				// 0x60<track number>
				// 0x61<control & address field>
				// 0x62<start time minutes>
				// 0x63<start time seconds>
				// 0x64<start time frames>
				// or DSA error
				CDGetTocInfo(cmdData, LONG_TOC);	// Session number, info type: long TOC

				break;
			}
		// Set mode: 0x15<mode>
		case 0x15:
			{
				// Set mode
				// bit 0 -> speed 0=>single, 1=>double	*
				// bit 1 -> mode  0=>Audio, 1=>CDROM	*
				if (cmdData != 0)
					CDMode = cmdData;	// Store new mode

				// Response: 0x17<mode>
				SetSingleRXData(0x1700 | CDMode);
				break;
			}
		// Get last error: 0x16<XX>
		case 0x16:
			{
				// Response: 0x04<error value>
				SetSingleRXData(0x0400 | CDLastError);
				break;
			}
		// Get last error: 0x17<XX>
		case 0x17:
			{
				// Response: 0x0400>
				SetSingleRXData(0x0400);
				break;
			}
		// Spin up: 0x18<session number>
		case 0x18:
			{
				// Response: 0x01<XX>
				// or DSA error
				CDGetTocInfo(cmdData, SPINUP_SESSION);	// Session number, info type: spin up session

				break;
			}
		// Play start time minutes: 0x20<absolute minutes>
		case 0x20:
			{
				CDPlayStartMSF.Minutes = cmdData;
				WriteLog("CDROM: Play start minutes: %d\n", CDPlayStartMSF.Minutes);
				break;
			}
		// Play start time seconds: 0x21<absolute seconds>
		case 0x21:
			{
				CDPlayStartMSF.Seconds = cmdData;
				WriteLog("CDROM: Play start seconds: %d\n", CDPlayStartMSF.Seconds);
				break;
			}
		// Play start time frames: 0x22<absolute frames>
		case 0x22:
			{
				CDPlayStartMSF.Frames = cmdData;
				WriteLog("CDROM: Play start frames: %d\n", CDPlayStartMSF.Frames);
				break;
			}
		// Play stop time minutes: 0x23<absolute minutes>
		case 0x23:
			{
				CDPlayStopMSF.Minutes = cmdData;
				WriteLog("CDROM: Play stop minutes: %d\n", CDPlayStopMSF.Minutes);
				break;
			}
		// Play stop time seconds: 0x24<absolute seconds>
		case 0x24:
			{
				CDPlayStopMSF.Seconds = cmdData;
				WriteLog("CDROM: Play stop seconds: %d\n", CDPlayStopMSF.Seconds);
				break;
			}
		// Play stop time frames: 0x25<absolute frames>
		case 0x25:
			{
				CDPlayStopMSF.Frames = cmdData;
				WriteLog("CDROM: Play stop frames: %d\n", CDPlayStopMSF.Frames);
				break;
			}
		// Get Disc and Carousel Status: 0x50<XX>
		case 0x50:
			{
				// Response: 0x03<XX>
				// bit 0:
				//		0 = no disc/TOC not read, 
				//		1 = disc present/TOC read
				// bit 1:
				//		0 = no multi session disc
				//		1 = multi session disc
				// bit 2:
				//		0 = not in PAUSE mode
				//		1 = in PAUSE mode
				// bit 3:	reserved
				// bit 4-5:	Carousel slot number
				// bit 6-7:	reserved
				SetSingleRXData(0x0300 | CDStatus);
				break;
			}
		// Set volume: 0x51<volume value>
		case 0x51:
			{
				// 0 = mute
				// 1-254 = fade
				// 255 = fullscale
				// Response: 0x51<volume value>
				SetSingleRXData(0x5100 | cmdData);
				break;
			}
		// Get session info: 0x54<XX>
		case 0x54:
			{
				// Response: 0x54<number of sessions>
				// or DSA error
				CDGetTocInfo(0, SESSIONS);			// Wanted session number not relevant

				break;
			}
		// Clear TOC: 0x6A<XX>
		case 0x6A:
			{
				// Clear TOC read flag
				CDStatus &= ~CD_PRESENT_TOC_READ;
				// Response: 0x6A<XX>
				SetSingleRXData(0x6A00 | cmdData);
				WriteLog("CDROM: TOC cleared.\n");
				break;
			}
		// Set DAC mode: 0x70<dac mode>
		case 0x70:
			{
				// Store the DAC mode
				CDDACMode = cmdData;
				// Response: 0x70<dac mode>
				SetSingleRXData(0x7000 | CDDACMode);
				break;
			}
		// NO return to receive data immediately
		default:
			{
				WriteLog("CDROM: unhandled command: $%04X\n", cmd);
				break;
			}
		}
	}

	// Data available
	if (RXDataIndex < MaxRXDataCount)
	{
		// Set DSA_RX
		butch_control = butch_control | DSA_RX_INT_PENDING;
		SetButchControl(butch_control);
		// Generate interrupt if enabled
		BUTCHSetIRQLine(DSA_RX_INT_PENDING);
	}
}

void BUTCHExec(void)
{
	if (butch_enabled)
	{
		// Handle commands sent to Butch
		BUTCHCommand();

		// Data available and not in PAUSE mode ?
		if ((bCDRead) && (~CDStatus & CD_IN_PAUSE_MODE))
		{
			// I2S data transfer enabled ?
			uint16 data = GET16(cdRam, (I2CNTRL + 2) & 0xFF);
			if (data & I2S_DATA_ENABLE)
			{
				// Buffer empty ?
				if (CDReadBufferIndex >= RAW_SECTOR_SIZE)		// Index out of array, buffer empty
				{
					CDReadData();				// Sets DSA Found or DSA Error
				}

				// Data available after CD read ?
				if (CDReadBufferIndex < RAW_SECTOR_SIZE)
				{
#ifdef CDROM_LOG
					WriteLog("BUTCH: FIFO data size: %d\n", FiFoDataSize);
#endif
					// Append data to maximum size of FIFO
					uint8 AppendBytes = sizeof(FiFo) - FiFoDataSize;
					if (AppendBytes > 0)
					{
						// Enough data available to put something in FIFO ?
						if ((RAW_SECTOR_SIZE - CDReadBufferIndex) < AppendBytes)
							AppendBytes = RAW_SECTOR_SIZE - CDReadBufferIndex;

						// Data to append ?
						if (AppendBytes > 0)
						{
							// Copy from CD buffer to FIFO
							memcpy(FiFo + FiFoDataSize, CDReadBuffer + CDReadBufferIndex, AppendBytes);

							// Update sizes
							FiFoDataSize = FiFoDataSize + AppendBytes;
#ifdef FIFO_LOG
							WriteLog("BUTCH: FIFO data append: %d byte(s), new size: %d\n", AppendBytes, FiFoDataSize);
#endif
							CDReadBufferIndex = CDReadBufferIndex + AppendBytes;
#ifdef CDROM_LOG
							WriteLog("BUTCH: CD read buffer, new index: %d\n", CDReadBufferIndex);
#endif
							// Set bit 4 in I2CNTRL
							uint8 offset = (I2CNTRL+2) & 0xFF;
							uint16 data = GET16(cdRam, offset);
							data = data | I2S_DATA_IN_FIFO;
							SET16(cdRam, offset, data);
						}
					}
					// FiFo half full ?
					if (FiFoDataSize >= (sizeof(FiFo) / 2))
					{
//#ifdef FIFO_LOG
//						WriteLog("BUTCH: FIFO is half full.\n");
//#endif
						// Set DSA_FIFO
						butch_control = butch_control | DSA_FIFO_INT_PENDING;
						SetButchControl(butch_control);
						// Generate interrupt if enabled
						BUTCHSetIRQLine(DSA_FIFO_INT_PENDING);
					}

					// I2S data transfer to Jerry enabled ?
					//uint16 data = GET16(cdRam, (I2CNTRL + 2) & 0xFF);
					if (data & I2S_DATA_TO_JERRY)
					{
						BUTCHExecI2S();
					}
				}
			}
		}
	}
}

//
// CD-ROM memory access functions
//

uint8 CDROMReadByte(uint32 address, uint32 who)
{
	MessageBox(0,"BUTCH: Read byte","",0);

	WriteLog("AddressReadByte: $%08X\n", address);

#ifdef CDROM_LOG
	uint32 offset = address & 0xFF;
	if (offset < sizeof(BReg))
		WriteLog("[%s] ", BReg[offset / 4]);
	WriteLog("CDROM: %s reading byte $%02X from $%08X [68K PC=$%08X]\n", whoName[who], address, cdRam[address & 0xFF], m68k_get_reg(NULL, M68K_REG_PC));
#endif
	return cdRam[address & 0xFF];
}

uint16 CDROMReadWord(uint32 address, uint32 who/*=UNKNOWN*/)
{
	uint16 data = GET16(cdRam, address & 0xFF);

	if (address == BUTCH)
		return (data);
	else if (address == (BUTCH + 2))
		return (butch_control);
	else if (address == DS_DATA)
	{
//		uint32 offset = address & 0xFF;
//		data = GET16(cdRam, offset);
//		WriteLog("BUTCH: Data received: $%04X\n", data);
		// Clear data register
//		SET16(cdRam, offset, 0x0000);

		if (RXDataIndex < MaxRXDataCount)
		{
			data = RXData[RXDataIndex];
			RXDataIndex++;
			WriteLog("BUTCH: Data received: $%04X\n", data);
		}
		else
			data = 0x0000;

		// Data received, so clear DSA_RX, DSA_TX
		butch_control &= ~DSA_TX_INT_PENDING;
		butch_control &= ~DSA_RX_INT_PENDING;
#ifdef CDROM_LOG
		WriteLog("BUTCH: ReadWord DS_DATA: $%04X\n", data);
#endif
		SetButchControl(butch_control);

		return (data);
	}
	else if (address == DSCNTRL)
	{
		// Clear DSA_TX, DSA_RX
		butch_control &= ~DSA_TX_INT_PENDING;
		butch_control &= ~DSA_RX_INT_PENDING;
#ifdef CDROM_LOG
		WriteLog("BUTCH: ReadWord DSCNTRL: $0\n");
#endif
		SetButchControl(butch_control);
		return (0x0000);	// ???
	}
	else if (address == (DSCNTRL + 2))
	{
		// Clear DSA_TX, DSA_RX
		butch_control &= ~DSA_TX_INT_PENDING;
		butch_control &= ~DSA_RX_INT_PENDING;
#ifdef CDROM_LOG
		WriteLog("BUTCH: ReadWord DSCNTRL + 2: $0\n");
#endif
		SetButchControl(butch_control);
		return (0x0000);	// ???
	}
	else if (address == I2CNTRL)
	{
		// Clear pending error
		butch_control &= ~DSA_ERROR_INT_PENDING;
#ifdef CDROM_LOG
		WriteLog("BUTCH: ReadWord I2CNTRL: $0\n");
#endif
		SetButchControl(butch_control);

	}
	else if (address == (I2CNTRL + 2))
	{
		data = 0x0000;
#ifdef CDROM_LOG
		WriteLog("BUTCH: ReadWord I2CNTRL + 2: $%d\n", data);
#endif
	}

	// FIFO content size = 64 bytes
	else if (address == FIFO_DATA)				// = I2SDATA1
	{
		// Get word and decrement FIFO
		if (FiFoDataSize > 0)
		{
			data = (FiFo[3] << 8) + FiFo[2];
			ReduceFiFo(2, 2);					// Position of byte 2, reduce 2 bytes

#ifdef FIFO_LOG
			//if (data != 0)
			{
				WriteLog("FIFO: ReadWord I2SDATA1: %04X\n", data);
			}
#endif
		}
	}
	else if (address == (FIFO_DATA + 2))		// = I2SDATA1 + 2
	{
		// Get word and decrement FIFO
		if (FiFoDataSize > 0)
		{
			data = (FiFo[5] << 8) + FiFo[4];
			ReduceFiFo(4, 2);					// Position of byte 4, reduce 2 bytes

#ifdef FIFO_LOG
			//if (data != 0)
			{
				WriteLog("FIFO: ReadWord I2SDATA1 + 2: %04X\n", data);
			}
#endif
		}
	}
	else if (address == I2SDATA2)
	{
		// Get word and decrement FIFO
		if (FiFoDataSize > 0)
		{
			data = (FiFo[3] << 8) + FiFo[2];
			ReduceFiFo(2, 2);					// Position of byte 2, reduce 2 bytes

#ifdef FIFO_LOG
			//if (data != 0)
			{
				WriteLog("FIFO: ReadWord I2SDATA2: %04X\n", data);
			}
#endif
		}
	}
	else if (address == (I2SDATA2 + 2))
	{
		// Get word and reduce FIFO
		if (FiFoDataSize > 0)
		{
			data = (FiFo[5] << 8) + FiFo[4];
			ReduceFiFo(4, 2);					// Position of byte 4, reduce 2 bytes

#ifdef FIFO_LOG
			//if (data != 0)
			{
				WriteLog("FIFO: ReadWord I2SDATA2 + 2: %04X\n", data);
			}
#endif
		}
	}
	else if (address == EEPROMBUS)
	{
		data = 0x0000;
	}
	else if (address == (EEPROMBUS + 2))
	{
		data = CDEepromBusRead();
	}
	return data;
}

void CDROMWriteByte(uint32 address, uint8 data, uint32 who/*=UNKNOWN*/)
{
	cdRam[address & 0xFF] = data;

	MessageBox(0,"BUTCH: Write byte !!!","",0);

	WriteLog("AddressWriteByte: %08X\n", address);

#ifdef CDROM_LOG
	uint32 offset = address & 0xFF;
	if (offset < sizeof(BReg))
		WriteLog("[%s] ", BReg[offset / 4]);
	WriteLog("CDROM: %s writing byte $%02X at $%08X [68K PC=$%08X]\n", whoName[who], data, address, m68k_get_reg(NULL, M68K_REG_PC));
#endif
}

void CDROMWriteWord(uint32 address, uint16 data, uint32 who/*=UNKNOWN*/)
{
	if (address == BUTCH)
	{
		// Setting bit 20 enables Butch
		// Setting bit 19 enables the reboot of jag if cd lid is opened
		// If bit 19 is not set the jag does not reboot if cd lid is opened
		if ((data & 0x0010) == 0x0010)
			butch_enabled = true;
		else
			butch_enabled = false;
#ifdef CDROM_LOG
		WriteLog("BUTCH: WriteWord BUTCH: $%04X\n", data);
#endif
	}
	else if (address == (BUTCH + 2))
	{
		butch_control = data;
#ifdef CDROM_LOG
		WriteLog("BUTCH: WriteWord BUTCH+2: $%04X\n", data);
#endif
		SetButchControl(butch_control);
	}

	// Command register
	else if (address == DS_DATA)
	{
		NewCmdTX = data;
		// Immediate response: $0407

#ifdef CDROM_LOG
		WriteLog("BUTCH: WriteWord DS_DATA: $%04X\n", data);
#endif
		// Immediate content of DS_DATA
		data = 0x0407;
		//SET16(cdRam, DS_DATA & 0xFF, 0x0407);
	}
	//if (address == I2CNTRL)	//	not necessary
	//	WriteLog("BUTCH: WriteWord I2CNTRL: $%04X\n", data);
	else if (address == (I2CNTRL + 2))
	{
//#ifdef CDROM_LOG
		WriteLog("BUTCH: WriteWord I2CNTRL+2: $%04X\n", data);
//#endif
		// b0 - I2S data from drive is ON if 1
		// b1 - I2S path to Jerry is ON if 1
		// b2 - Enable data transfer (subcode ???)
		// b3 - host bus width is 16 if 1, else 32
		// b4 - FIFO state is not empty if 1
		if (data & I2S_DATA_FROM_CD)
			WriteLog("BUTCH: I2S data from CD is ON.\n");
		else
			WriteLog("BUTCH: I2S data from CD is OFF.\n");

		if (data & I2S_DATA_TO_JERRY)
			WriteLog("BUTCH: I2S data to Jerry is ON.\n");
		else
			WriteLog("BUTCH: I2S data to Jerry is OFF.\n");
		if (data & I2S_DATA_ENABLE)
		{
			WriteLog("BUTCH: I2S data transfer is ON.\n");
		}
		else
		{
			// Stop I2S data transfer
			CDReadBufferIndex = RAW_SECTOR_SIZE;		// Index out of array, buffer empty
			WriteLog("BUTCH: I2S data transfer stopped.\n");
		}
#ifdef CDROM_LOG
		if (data & I2S_BUS_WIDTH16)
			WriteLog("BUTCH: Host bus width is 16.\n");
		else
			WriteLog("BUTCH: Host bus width is 32.\n");

		if (data & I2S_DATA_IN_FIFO)
			WriteLog("BUTCH: FiFo is not empty.\n");
		else
			WriteLog("BUTCH: FiFo is empty.\n");
#endif
	}
	//else if (address == EEPROMBUS)
		//WriteLog("BUTCH: WriteWord $DFFF2C: $%04X\n", data);
	else if (address == (EEPROMBUS + 2))
	{
		//WriteLog("BUTCH: WriteWord $DFFF2e: $%04X\n", data);
		CDEepromBusWrite(data);
	}

	SET16(cdRam, address & 0xFF, data);

	//	WriteLog("AddressWriteWord: %08X, data: %08X\n", address, data);
}

//
// State machine for sending/receiving data along a serial bus
//
//static CDEepromBusState GetNewState(bool bNewSignal, CDEepromBusState Line)
uint8 GetNewState(bool bNewSignal, uint8 Line)
{
	if (bNewSignal)	// True = High
	{
		switch(Line)
		{
		case STATE_LOW:
				return(STATE_RISING);
		case STATE_HIGH:
				return(STATE_HIGH);
		case STATE_RISING:
				return(STATE_HIGH);
		case STATE_FALLING:
				return(STATE_RISING);
		}
	}
	else			// False = Low
	{
		switch(Line)
		{
		case STATE_LOW:
				return(STATE_LOW);
		case STATE_HIGH:
				return(STATE_FALLING);
		case STATE_RISING:
				return(STATE_FALLING);
		case STATE_FALLING:
				return(STATE_LOW);
		}
	}
	return(STATE_LOW);	// to avoid compiler warning
}

// Send command to CD internal eeprom
// Set receive data line
static void CDEepromBusWrite(uint16 data)
{
	// Get new values for ACK, STB and DATA
	bool bNew_ACK = data & CDEEPROMBUS_ACK;
	bool bNew_STB = data & CDEEPROMBUS_STB;
	bool bNew_TXD = data & CDEEPROMBUS_TXD;

	Line_ACK = GetNewState(bNew_ACK, Line_ACK);
	Line_STB = GetNewState(bNew_STB, Line_STB);
	Line_TXD = GetNewState(bNew_TXD, Line_TXD);

	//WriteLog("ACK: %d\n", Line_ACK);
	//WriteLog("STB: %d\n", Line_STB);
	//WriteLog("TXD: %d\n", Line_TXD);

	// Synchronize transfer
	if (Line_ACK == STATE_FALLING)
	{
		CDEepromBusData = 0;
		CDEepromDataBitCounter = 0;			// Start bit counting
		CDEepromBusCommand = 0;
		bCDEepromBusSending = true;			// Send data
	}

	if (Line_STB == STATE_FALLING)
	{
		if (bCDEepromBusSending)				// Sending data
		{
			CDEepromBusData <<= 1;				// Make room for this bit
			CDEepromBusData |= Line_TXD;		// Data should now be STATE_LOW=0 or STATE_HIGH=1

			CDEepromDataBitCounter++;

			if (CDEepromBusCommand == 0)		// No command yet
			{
				if (CDEepromDataBitCounter == 9)	// Commands have 9 bits 
				{
					// Command complete
					CDEepromBusCommand = CDEepromBusData;	// Store cmd for receive data handling
					WriteLog("CDROM: EepromBusWrite, command: $%04X\n", CDEepromBusCommand);
					// Clear bus
					CDEepromBusData = 0;
					CDEepromDataBitCounter = 0;

					if ((CDEepromBusCommand & 0x1F0) == 0x100)		// Write Disable
					{
						// No data to transmit
						bCDEepromBusSending = false;
					}
					else if ((CDEepromBusCommand & 0x1F0) == 0x130)		// Write Enable
					{
						// No data transmit
						bCDEepromBusSending = false;
					}
					else if ((CDEepromBusCommand & 0x1C0) == 0x140)		// Write data to cell
					{
						// Data to transmit
						bCDEepromBusSending = true;
					}
					else if ((CDEepromBusCommand & 0x1C0) == 0x180)		// Read data from cell
					{
						// No data to transmit, receive data
						bCDEepromBusSending = false;

						// CD bios selects graphics intro on reading cell 4/5
						// For now 8 (0-7) values for cell 4/5 are filled
						// So let's take a random graphics intro
						CDEepromIntro = rand() & 0x7;
					}
					else if ((CDEepromBusCommand & 0x1C0) == 0x1C0)		// Erase cell
					{
						// No Data to transmit
						bCDEepromBusSending = false;
					}
				}
			}
			else
			{
				if (CDEepromDataBitCounter == 16)	// Data has 16 bits 
				{
					// Data complete

					// Get cell of command
					uint8 CDEepromCell = CDEepromBusCommand & 0x3F;
					WriteLog("CDROM: EepromBusWrite, cell: $%04X, data: $%04X\n", CDEepromCell, CDEepromBusData);
					// Clear bus
					CDEepromBusData = 0;
					CDEepromDataBitCounter = 0;
					bCDEepromBusSending = false;
				}
			}
		}
		else							// Receiving data
		{
			Line_RXD = STATE_LOW;		// For unknown/ignored commands
			// Set receive data line depending on command

			// CDBios audio function reads cell 0 and 1
			if (CDEepromBusCommand == 0x0180)
			{
				Line_RXD = CDEepromCell_00_Data[CDEepromDataBitCounter];
			}
			if (CDEepromBusCommand == 0x0181)
			{
				Line_RXD = CDEepromCell_01_Data[CDEepromDataBitCounter];
			}
			// CDBios reads cell 4 and 5
			if (CDEepromBusCommand == 0x0184)
			{
				Line_RXD = CDEepromCell_04_Data[CDEepromIntro][CDEepromDataBitCounter];
			}
			if (CDEepromBusCommand == 0x0185)
			{
				Line_RXD = CDEepromCell_05_Data[CDEepromIntro][CDEepromDataBitCounter];
			}

			// Rest of sub is only needed for logging
			CDEepromBusData <<= 1;				// Make room for this bit
			CDEepromBusData |= Line_RXD;

			CDEepromDataBitCounter++;
			if (CDEepromDataBitCounter == 16)	// Receive data has 16 bits
			{
				// Data received
				// Get cell of command
				uint8 CDEepromCell = CDEepromBusCommand & 0x3F;
				WriteLog("CDROM: EepromBusRead, cell: $%04X, data: $%04X\n", CDEepromCell, CDEepromBusData);
			}
		}
	}
}

static uint16 CDEepromBusRead(void)
{
	// Set bit 3 if RXD line is 1 (high), CD bios shifts this back to the right
	uint16 data = Line_RXD * CDEEPROMBUS_RXD;
	// Clear line
	Line_RXD = STATE_LOW;
	return (data);
}

void BUTCHExecI2S(void)
{
	if (!(I2S_SerialMode & 0x01))			// INTERNAL flag NOT set -> Jerry is slave, Butch can be master
	{
		GenerateI2SClock(0x0);				// Min. frequency -> max. performance

		// Only if falling clock edge
		if (I2SClkState == STATE_FALLING)
		{
			I2SClkState = STATE_LOW;		// Set static

			// Handle WS
			if (I2SWSBitCount >= 16)		// Toggle WS after 16 data bits
			{
				I2SWSBitCount = 0;			

				I2SWSHigh = !I2SWSHigh;

				/*if (I2SWSHigh == true)
					WriteLog("I2S_BUTCH: Master WS is rising.\n");
				else
					WriteLog("I2S_BUTCH: Master WS is falling.\n");
*/			}

			// Handle transmitting data
			if (I2SWSBitCount == 1)		// Skip first data bit 0
			{
				if (I2SWSHigh == true)		// Transfer left channel data
				{
					// Serial interface data
					ButchShiftTXD = (CDReadBuffer[CDReadBufferIndex + 1] << 8) | CDReadBuffer[CDReadBufferIndex + 0];
					//WriteLog("I2S_BUTCH: CDReadBufferIndex: %d\n", CDReadBufferIndex);
					CDReadBufferIndex = CDReadBufferIndex + 2;
					//WriteLog("I2S_BUTCH: LTXD to shift register: %04X\n", ButchShiftTXD);
				}
				else	// WS is low, transfer right channel data
				{
					// Serial interface data
					ButchShiftTXD = (CDReadBuffer[CDReadBufferIndex + 1] << 8) | CDReadBuffer[CDReadBufferIndex + 0];
					//WriteLog("I2S_BUTCH: CDReadBufferIndex: %d\n", CDReadBufferIndex);
					CDReadBufferIndex = CDReadBufferIndex + 2;
					//WriteLog("I2S_BUTCH: RTXD to shift register: %04X\n", ButchShiftTXD);
				}
			}
			I2SWSBitCount++;	// Next data bit
			//WriteLog("I2S: WS bit count: %02X\n", I2SWSBitCount);
		}
	}
	// Butch does not receive I2S data therefore no I2S slave procedure is necessary 
}
