#include "common.h"

INT_PTR Log(WPARAM wParam, LPARAM lParam) {

	TCHAR buf[1024], tbuf[512], dbuf[512];
	CallService(PLUG "/GetLogFilename", (WPARAM)1024, (LPARAM)buf);

	//char TBcapt[255];
	SYSTEMTIME systime;
	
	GetLocalTime(&systime);

	GetTimeFormat(LOCALE_USER_DEFAULT, 0, &systime, 0, tbuf, 512);
	GetDateFormat(LOCALE_USER_DEFAULT, 0, &systime, 0, dbuf, 512);

	char *line = (char *)wParam;

	FILE *f = _tfopen(buf, _T("a+"));
	if(f) {
		if(options.log_csv) {
			fprintf(f, "%s, %s, %s\n", dbuf, tbuf, line);
		} else {
			fprintf(f, "%s, %s: %s\n", dbuf, tbuf, line);
		}
		fclose(f);
	}

	return 0;
}

INT_PTR GetLogFilename(WPARAM wParam, LPARAM lParam) {
	DBVARIANT dbv;
	char *filename = (char *)lParam;
	if(db_get(0, PLUG, "LogFilename", &dbv)) {
		CallService(MS_DB_GETPROFILEPATH, (WPARAM)wParam, (LPARAM)filename);
		strcat(filename, "\\");
		strcat(filename, "ping_log.txt");
	} else {
		strncpy(filename, dbv.pszVal, wParam);
		db_free(&dbv);
	}

	((char *)lParam)[wParam - 1] = 0;

	return 0;
}

INT_PTR SetLogFilename(WPARAM wParam, LPARAM lParam) {
	db_set_s(0, PLUG, "LogFilename", (char *)lParam);
	return 0;
}

INT_PTR ViewLogData(WPARAM wParam, LPARAM lParam) {
	char buf[1024];
	CallService(PLUG "/GetLogFilename", (WPARAM)MAX_PATH, (LPARAM)buf);
	return (INT_PTR)ShellExecute((HWND)wParam, _T("edit"), buf, _T(""), _T(""), SW_SHOW);	
}

