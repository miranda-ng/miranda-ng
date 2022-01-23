#include "stdafx.h"

void Log(const wchar_t *pwszText)
{
	if (!options.logging)
		return;

	wchar_t buf[1024], tbuf[512], dbuf[512];
	GetLogFilename(buf, _countof(buf));

	//char TBcapt[255];
	SYSTEMTIME systime;

	GetLocalTime(&systime);

	GetTimeFormat(LOCALE_USER_DEFAULT, 0, &systime, nullptr, tbuf, 512);
	GetDateFormat(LOCALE_USER_DEFAULT, 0, &systime, nullptr, dbuf, 512);

	FILE *f = _wfopen(buf, L"a+");
	if (f) {
		if (options.log_csv) {
			fwprintf(f, L"%s, %s, %s\n", dbuf, tbuf, pwszText);
		}
		else {
			fwprintf(f, L"%s, %s: %s\n", dbuf, tbuf, pwszText);
		}
		fclose(f);
	}
}

void GetLogFilename(wchar_t *pBuf, size_t cbLen)
{
	ptrW wszLogName(g_plugin.getWStringA("LogFilename"));
	if (wszLogName == nullptr) {
		Profile_GetPathW(cbLen, pBuf);
		mir_wstrncat(pBuf, L"\\ping_log.txt", cbLen - mir_wstrlen(pBuf));
	}
	else mir_wstrncpy(pBuf, wszLogName, cbLen);

	pBuf[cbLen - 1] = 0;
}

INT_PTR ViewLogData(WPARAM wParam, LPARAM)
{
	wchar_t buf[MAX_PATH];
	GetLogFilename(buf, _countof(buf));
	return (INT_PTR)ShellExecute((HWND)wParam, L"edit", buf, L"", L"", SW_SHOW);
}
