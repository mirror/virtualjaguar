//
// Log handler
//
// by cal2
// GCC/SDL port by Niels Wagenaar (Linux/WIN32) and Caz (BeOS)
// Cleanups/new stuff by James L. Hammons
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

//
// This logger is used mainly to ensure that text gets written to the log file
// even if the program crashes. The performance hit is acceptable in this case!
//
//bool suppressOutput = true;//temporary stuff
bool suppressOutput = false;//temporary stuff
void WriteLog(const char * text, ...)
{
	if (suppressOutput)
		return;

	va_list arg;

	va_start(arg, text);
	vfprintf(log_stream, text, arg);
	va_end(arg);
	fflush(log_stream);					// Make sure that text is written!
}
