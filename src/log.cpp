//
// Log handler
//
// by cal2
// GCC/SDL port by Niels Wagenaar (Linux/WIN32) and Caz (BeOS)
// Cleanups by James L. Hammons
//

#include "log.h"

FILE * log_stream = NULL;

int log_init(char * path)
{
	log_stream = fopen(path, "wrt");
	if (log_stream == NULL)
		return 0;
	return 1;
}

FILE * log_get(void)
{
	return log_stream;
}

void log_done(void)
{
	fclose(log_stream);
}
