//
// CD handler
//
// by David Raingeard
// GCC/SDL port by Niels Wagenaar (Linux/WIN32) and Caz (BeOS)
// Cleanups/fixes by James L. Hammons
//

#include "m68k.h"
#include "cdrom.h"


#define CDROM_LOG									// For CDROM logging, obviously

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
I2SDAT1   equ  BUTCH+$24	; i2s FIFO data
I2SDAT2   equ  BUTCH+$28	; i2s FIFO data
2C = ?

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
;  b2 - reserved
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
char * BReg[11] = { "BUTCH", "DSCNTRL", "DS_DATA", "I2CNTRL", "SBCNTRL", "SUBDATA", "SUBDATB",
	"SB_TIME", "FIFO_DATA", "I2SDAT1", "I2SDAT2" };
extern char * whoName[9];


static uint8 cdrom_ram[0x100];
static uint16 cdrom_cmd = 0;


void cdrom_init(void)
{
}

void cdrom_reset(void)
{
	memset(cdrom_ram, 0x00, 0x100);
	cdrom_cmd = 0;
}

void cdrom_done(void)
{
}

//
// CD-ROM memory access functions
//

uint8 CDROMReadByte(uint32 offset, uint32 who/*=UNKNOWN*/)
{
#ifdef CDROM_LOG
	if ((offset & 0xFF) < 11 * 4)
		WriteLog("[%s] ", BReg[(offset & 0xFF) / 4]);
	WriteLog("CDROM: %s reading byte $%02X from $%08X [68K PC=$%08X]\n", whoName[who], offset, cdrom_ram[offset & 0xFF], m68k_get_reg(NULL, M68K_REG_PC));
#endif
	return cdrom_ram[offset & 0xFF];
}

uint16 CDROMReadWord(uint32 offset, uint32 who/*=UNKNOWN*/)
{
	offset &= 0xFF;

	uint16 data = 0x0000;
	
	if (offset == 0x00) 
		data = 0x0000;
	else if (offset == 0x02) 
		data = 0x2000;
	else if (offset == 0x0A) 
	{
		if (cdrom_cmd == 0x7001)
			data = cdrom_cmd;
		else
			data = 0x0400;
	}
	else
		data = (cdrom_ram[offset+0] << 8) | cdrom_ram[offset+1];

//Returning $00000008 seems to cause it to use the starfield. Dunno why.
//Temp, for testing...
//Very interesting...! Seems to control sumthin' or other...
/*if (offset == 0x2C || offset == 0x2E)
	data = 0xFFFF;//*/
if (offset == 0x2C)
	data = 0x0000;
if (offset == 0x2E)
	data = 0x0008;//*/ // $0000 000F ($B) works, but not $0000 00001... or $7

#ifdef CDROM_LOG
	if ((offset & 0xFF) < 11 * 4)
		WriteLog("[%s] ", BReg[(offset & 0xFF) / 4]);
	WriteLog("CDROM: %s reading word $%04X from $%08X [68K PC=$%08X]\n", whoName[who], data, offset, m68k_get_reg(NULL, M68K_REG_PC));
#endif
	return data;
}

void CDROMWriteByte(uint32 offset, uint8 data, uint32 who/*=UNKNOWN*/)
{
	offset &= 0xFF;
	cdrom_ram[offset] = data;

#ifdef CDROM_LOG
	if ((offset & 0xFF) < 11 * 4)
		WriteLog("[%s] ", BReg[(offset & 0xFF) / 4]);
	WriteLog("CDROM: %s writing byte $%02X at $%08X [68K PC=$%08X]\n", whoName[who], data, offset, m68k_get_reg(NULL, M68K_REG_PC));
#endif
}

void CDROMWriteWord(uint32 offset, uint16 data, uint32 who/*=UNKNOWN*/)
{
	offset &= 0xFF;
	cdrom_ram[offset+0] = (data >> 8) & 0xFF;
	cdrom_ram[offset+1] = data & 0xFF;
		
	// command register
/*
	if (offset==0x0A)
	{
		cdrom_cmd=data;
		if ((data&0xff00)==0x1500)
		{
			WriteLog("CDROM: setting mode 0x%.2x\n",data&0xff);
			return;
		}
		if (data==0x7001)
		{
			uint32 offset=cdrom_ram[0x00];
			offset<<=8;
			offset|=cdrom_ram[0x01];
			offset<<=8;
			offset|=cdrom_ram[0x02];
			offset<<=8;
			offset|=cdrom_ram[0x03];

			uint32 size=cdrom_ram[0x04];
			offset<<=8;
			offset|=cdrom_ram[0x05];
			
			WriteLog("CDROM: READ(0x%.8x, 0x%.4x) [68k pc=0x%.8x]\n", offset, size, m68k_get_reg(NULL, M68K_REG_PC));
			return;
		}
		else
			WriteLog("CDROM: unknown command 0x%.4x\n",data);
	}
//*/
#ifdef CDROM_LOG
	if ((offset & 0xFF) < 11 * 4)
		WriteLog("[%s] ", BReg[(offset & 0xFF) / 4]);
	WriteLog("CDROM: %s writing word $%04X at $%08X [68K PC=$%08X]\n", whoName[who], data, offset, m68k_get_reg(NULL, M68K_REG_PC));
#endif
}
