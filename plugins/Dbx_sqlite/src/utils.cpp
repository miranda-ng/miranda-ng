#include "stdafx.h"

void logError(int rc, const char *szFile, int line)
{
	switch (rc) {
	case SQLITE_OK:
	case SQLITE_ROW:
	case SQLITE_DONE:
		return;
	}

	_ASSERT(rc == 0);
	Netlib_Logf(0, "SQLITE error %d (%s, %d)", rc, szFile, line);
}
