//
// VERSION.CPP
//
// by Cal2
// GCC/SDL port by Niels Wagenaar (Linux/WIN32) and Caz (BeOS)
// Cleanups by James L. Hammons
//

#include "version.h"

void version_init(void)
{
}

void version_display(FILE * fp)
{
	fprintf(fp, "VirtualJaguar v1.0.7 (Last full build was on %s %s)\n", __DATE__, __TIME__);
}

void version_done(void)
{
}
