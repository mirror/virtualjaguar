//
// CD handler
//
// by cal2
// GCC/SDL port by Niels Wagenaar (Linux/WIN32) and Caz (BeOS)
// Cleanups by James L. Hammons
//

#include "cdrom.h"

//#define CDROM_LOG

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
	WriteLog("CDROM: reading byte from 0x%.8x\n",offset);
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

#ifdef CDROM_LOG
	WriteLog("CDROM: reading word 0x%.4x from 0x%.8x [68k pc=0x%.8x]\n",data,offset,s68000readPC());
#endif
	return data;
}

void CDROMWriteByte(uint32 offset, uint8 data, uint32 who/*=UNKNOWN*/)
{
	offset &= 0xFF;
	cdrom_ram[offset] = data;

#ifdef CDROM_LOG
	WriteLog("CDROM: writing byte 0x%.2x at 0x%.8x\n",data,offset);
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
			
			WriteLog("CDROM: READ(0x%.8x, 0x%.4x) [68k pc=0x%.8x]\n",offset,size,s68000readPC());
			return;
		}
		else
			WriteLog("CDROM: unknown command 0x%.4x\n",data);
	}
*/
#ifdef CDROM_LOG
	WriteLog("CDROM: writing word 0x%.4x at 0x%.8x\n",data,offset);
#endif
}
