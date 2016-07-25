#include "stdafx.h"

INT_PTR Log(WPARAM wParam, LPARAM) {

	TCHAR buf[1024], tbuf[512], dbuf[512];
	CallService(PLUG "/GetLogFilename", (WPARAM)1024, (LPARAM)buf);

	//char TBcapt[255];
	SYSTEMTIME systime;

	GetLocalTime(&systime);

	GetTimeFormat(LOCALE_USER_DEFAULT, 0, &systime, 0, tbuf, 512);
	GetDateFormat(LOCALE_USER_DEFAULT, 0, &systime, 0, dbuf, 512);

	TCHAR *line = (TCHAR *)wParam;

	FILE *f = _tfopen(buf, L"a+");
	if (f) {
		if (options.log_csv) {
			_ftprintf(f, L"%s, %s, %s\n", dbuf, tbuf, line);
		}
		else {
			_ftprintf(f, L"%s, %s: %s\n", dbuf, tbuf, line);
		}
		fclose(f);
	}

	return 0;
}

INT_PTR GetLogFilename(WPARAM wParam, LPARAM lParam) {
	DBVARIANT dbv;
	TCHAR *filename = (TCHAR *)lParam;
	if (db_get_ts(0, PLUG, "LogFilename", &dbv)) {
		CallService(MS_DB_GETPROFILEPATHT, wParam, (LPARAM)filename);
		mir_tstrncat(filename, L"\\ping_log.txt", wParam - mir_tstrlen(filename));
	}
	else {
		mir_tstrncpy(filename, dbv.ptszVal, wParam);
		db_free(&dbv);
	}

	((TCHAR *)lParam)[wParam - 1] = 0;

	return 0;
}

INT_PTR SetLogFilename(WPARAM, LPARAM lParam) {
	db_set_ts(0, PLUG, "LogFilename", (TCHAR *)lParam);
	return 0;
}

INT_PTR ViewLogData(WPARAM wParam, LPARAM) {
	TCHAR buf[MAX_PATH];
	CallService(PLUG "/GetLogFilename", (WPARAM)MAX_PATH, (LPARAM)buf);
	return (INT_PTR)ShellExecute((HWND)wParam, L"edit", buf, L"", L"", SW_SHOW);
}

