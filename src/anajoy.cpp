//
// Analog joystick handler
//
// by cal2
// GCC/SDL port by Niels Wagenaar (Linux/WIN32) and Caz (BeOS)
// Cleanups by James L. Hammons
//
// Is it even necessary anymore to have this? According to the JTRM, these ports
// aren't even wired up in later models and I can't think of one game that used
// them. Maybe it's time to retire this?

#include "jaguar.h"

static uint8 anajoy_ram[2];
static uint8 analog_x, analog_y;


void anajoy_init(void)
{
	anajoy_reset();
}

void anajoy_reset(void)
{
	memset(anajoy_ram,0x00,2);
	analog_x=128;
	analog_y=128;
}

void anajoy_done(void)
{
}

void anajoy_byte_write(uint32 offset, uint8 data)
{
	anajoy_ram[offset&0x01]=data;
}

void anajoy_word_write(uint32 offset, uint16 data)
{
	offset&=0x01;
	anajoy_ram[offset+0]=(data>>8)&0xff;
	anajoy_ram[offset+1]=data&0xff;
}

uint8 anajoy_byte_read(uint32 offset)
{
	if (anajoy_ram[1]&0x01)
		return(analog_y);
	else
		return(analog_x);
}

uint16 anajoy_word_read(uint32 offset)
{
	uint16 data=anajoy_byte_read((offset+0)&0x01);
	data<<=8;
	data|=anajoy_byte_read((offset+1)&0x01);
	return(data);
}
