//
// VERSION.CPP
//
// by cal2
// GCC/SDL port by Niels Wagenaar (Linux/WIN32) and Caz (BeOS)
// Cleanups by James L. Hammons
//

#include "version.h"

void version_init(void)
{
}

void version_display(FILE * fp)
{
	fprintf(fp, "VirtualJaguar v1.0.3 (Last full build on %s %s)\n", __DATE__, __TIME__);
}

void version_done(void)
{
}
