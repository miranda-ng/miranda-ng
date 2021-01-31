#include "stdafx.h"
#include <time.h>

static HANDLE hLogger;

void logError(int rc, const char *szFile, int line)
{
	switch (rc) {
	case SQLITE_OK:
	case SQLITE_ROW:
	case SQLITE_DONE:
		return;
	}

	if (hLogger == nullptr) {
		wchar_t wszFileName[MAX_PATH];
		wsprintf(wszFileName, L"C:\\Users\\user\\Desktop\\sqlite.%d.log", (int)time(0));
		hLogger = mir_createLog("mdbx", L"mdbx", wszFileName, 0);
	}

	mir_writeLogA(hLogger, "sqlite: assertion failed (%s, %d): %d\n", szFile, line, rc);
}
