//
// VERSION.CPP
//
// by David Raingeard (Cal2)
// GCC/SDL port by Niels Wagenaar (Linux/WIN32) and Caz (BeOS)
// Cleanups by James L. Hammons
//

#include "version.h"

void InitVersion(void)
{
}

void version_display(FILE * fp)
{
	fprintf(fp, "Virtual Jaguar v1.0.8 (Last full build was on %s %s)\n", __DATE__, __TIME__);
}

void VersionDone(void)
{
}
