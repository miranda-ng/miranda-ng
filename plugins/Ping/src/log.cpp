#include "stdafx.h"

INT_PTR Log(WPARAM wParam, LPARAM) {

	wchar_t buf[1024], tbuf[512], dbuf[512];
	CallService(PLUG "/GetLogFilename", (WPARAM)1024, (LPARAM)buf);

	//char TBcapt[255];
	SYSTEMTIME systime;

	GetLocalTime(&systime);

	GetTimeFormat(LOCALE_USER_DEFAULT, 0, &systime, 0, tbuf, 512);
	GetDateFormat(LOCALE_USER_DEFAULT, 0, &systime, 0, dbuf, 512);

	wchar_t *line = (wchar_t *)wParam;

	FILE *f = _wfopen(buf, L"a+");
	if (f) {
		if (options.log_csv) {
			fwprintf(f, L"%s, %s, %s\n", dbuf, tbuf, line);
		}
		else {
			fwprintf(f, L"%s, %s: %s\n", dbuf, tbuf, line);
		}
		fclose(f);
	}

	return 0;
}

INT_PTR GetLogFilename(WPARAM wParam, LPARAM lParam) {
	DBVARIANT dbv;
	wchar_t *filename = (wchar_t *)lParam;
	if (db_get_ws(0, PLUG, "LogFilename", &dbv)) {
		Profile_GetPathW(wParam, filename);
		mir_wstrncat(filename, L"\\ping_log.txt", wParam - mir_wstrlen(filename));
	}
	else {
		mir_wstrncpy(filename, dbv.ptszVal, wParam);
		db_free(&dbv);
	}

	((wchar_t *)lParam)[wParam - 1] = 0;

	return 0;
}

INT_PTR SetLogFilename(WPARAM, LPARAM lParam) {
	db_set_ws(0, PLUG, "LogFilename", (wchar_t *)lParam);
	return 0;
}

INT_PTR ViewLogData(WPARAM wParam, LPARAM) {
	wchar_t buf[MAX_PATH];
	CallService(PLUG "/GetLogFilename", (WPARAM)MAX_PATH, (LPARAM)buf);
	return (INT_PTR)ShellExecute((HWND)wParam, L"edit", buf, L"", L"", SW_SHOW);
}

