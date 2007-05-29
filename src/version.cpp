//
// VERSION.CPP
//

#include "version.h"

void version_init(void)
{
}

void version_display(FILE * fp)
{
	fprintf(fp, "VirtualJaguar v1.0.1 (Last full build on %s %s)\n", __DATE__, __TIME__);
}

void version_done(void)
{
}
