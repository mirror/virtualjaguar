//
// Log handler
//
// Originally by David Raingeard (Cal2)
// GCC/SDL port by Niels Wagenaar (Linux/WIN32) and Caz (BeOS)
// Cleanups/new stuff by James L. Hammons
//

#include "types.h"
#include "log.h"

#define MAX_LOG_SIZE		10000000				// Maximum size of log file (10 MB)

static FILE * log_stream = NULL;
static uint32 logSize = 0;

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
void WriteLog(const char * text, ...)
{
	va_list arg;

	va_start(arg, text);
	logSize += vfprintf(log_stream, text, arg);

	if (logSize > MAX_LOG_SIZE)
	{
		fflush(log_stream);
		fclose(log_stream);
		exit(1);
	}//*/

	va_end(arg);
	fflush(log_stream);					// Make sure that text is written!
}
