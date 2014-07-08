#include "commonheaders.h"

HINSTANCE hRtfconv = NULL;
RTFCONVSTRING pRtfconvString = NULL;

BOOL load_rtfconv()
{
	hRtfconv = LoadLibrary(_T("rtfconv.dll"));
	if (hRtfconv == NULL) {
		hRtfconv = LoadLibrary(_T("plugins\\rtfconv.dll"));
		if (hRtfconv == NULL)
			return FALSE;
	}

	pRtfconvString = (RTFCONVSTRING)GetProcAddress(hRtfconv, "RtfconvString");
	if (pRtfconvString == NULL) {
		FreeLibrary(hRtfconv);
		return FALSE;
	}

	return TRUE;
}

void free_rtfconv()
{
	if (hRtfconv)
		FreeLibrary(hRtfconv);
	pRtfconvString = NULL;
	hRtfconv = NULL;
}

void rtfconvA(LPCSTR rtf, LPWSTR plain)
{
	pRtfconvString(rtf, plain, 0, 1200, CONVMODE_USE_SYSTEM_TABLE, (strlen(rtf) + 1)*sizeof(WCHAR));
}

void rtfconvW(LPCWSTR rtf, LPWSTR plain)
{
	pRtfconvString(rtf, plain, 0, 1200, CONVMODE_USE_SYSTEM_TABLE, (wcslen(rtf) + 1)*sizeof(WCHAR));
}
