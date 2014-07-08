#include "commonheaders.h"


/* FILETIME unit is 100 nanoseconds */
const static long div_100_nsec = 10000000;

/* POSIX or Unix Epoch (1-Jan-1970 00:00) in FILETIME units */
const static ULONGLONG ix_epoch = 116444736000000000;

DWORD gettime(void)
{
	ULONGLONG diff_100_nsec;
	union
	{
		FILETIME	f;
		ULARGE_INTEGER	u;
	} now;

	GetSystemTimeAsFileTime(&now.f);

	diff_100_nsec = now.u.QuadPart - ix_epoch;

	return (DWORD)(diff_100_nsec / div_100_nsec);
}
