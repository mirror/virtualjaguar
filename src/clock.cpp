//
// Clock handler
//
// by cal2
// GCC/SDL port by Niels Wagenaar (Linux/WIN32) and Caz (BeOS)
// Cleanups by James L. Hammons
//

#include "jaguar.h"


void clock_init(void)
{
	clock_reset();
}

void clock_reset(void)
{
}

void clock_done(void)
{
}

void clock_byte_write(uint32 offset, uint8 data)
{
}

void clock_word_write(uint32 offset, uint16 data)
{
}

uint8 clock_byte_read(uint32 offset)
{
	return 0xFF;
}

uint16 clock_word_read(uint32 offset)
{
	return 0xFFFF;
}
