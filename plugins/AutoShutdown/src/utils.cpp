/*

'AutoShutdown'-Plugin for Miranda IM

Copyright 2004-2007 H. Herkenrath

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program (Shutdown-License.txt); if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "stdafx.h"

/************************* Error Output ***************************/

static void MessageBoxIndirectFree(MSGBOXPARAMSA *mbp)
{
	MessageBoxIndirectA(mbp);
	mir_free((char*)mbp->lpszCaption); /* does NULL check */
	mir_free((char*)mbp->lpszText);    /* does NULL check */
	mir_free(mbp);
}

void ShowInfoMessage(uint8_t flags, const char *pszTitle, const char *pszTextFmt, ...)
{
	char szText[256]; /* max for systray */

	va_list va;
	va_start(va, pszTextFmt);
	mir_vsnprintf(szText, _countof(szText), pszTextFmt, va);
	va_end(va);

	if (!Clist_TrayNotifyA(nullptr, pszTitle, szText, flags, 30000)) // success
		return;

	MSGBOXPARAMSA *mbp = (MSGBOXPARAMSA*)mir_calloc(sizeof(*mbp));
	if (mbp == nullptr) return;
	mbp->cbSize = sizeof(*mbp);
	mbp->lpszCaption = mir_strdup(pszTitle);
	mbp->lpszText = mir_strdup(szText);
	mbp->dwStyle = MB_OK | MB_SETFOREGROUND | MB_TASKMODAL;
	mbp->dwLanguageId = LANGIDFROMLCID(Langpack_GetDefaultLocale());
	switch (flags&NIIF_ICON_MASK) {
	case NIIF_INFO:    mbp->dwStyle |= MB_ICONINFORMATION; break;
	case NIIF_WARNING: mbp->dwStyle |= MB_ICONWARNING; break;
	case NIIF_ERROR:   mbp->dwStyle |= MB_ICONERROR;
	}
	mir_forkThread<MSGBOXPARAMSA>(MessageBoxIndirectFree, mbp);
}

// LocalFree() the return value
char* GetWinErrorDescription(uint32_t dwLastError)
{
	char *buf = nullptr;
	uint32_t flags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM;
	if (!FormatMessageA(flags, nullptr, dwLastError, LANGIDFROMLCID(Langpack_GetDefaultLocale()), (char*)&buf, 0, nullptr))
		if (GetLastError() == ERROR_RESOURCE_LANG_NOT_FOUND)
			FormatMessageA(flags, nullptr, dwLastError, 0, (char*)&buf, 0, nullptr);
	return buf;
}

/************************* Time ***********************************/

BOOL SystemTimeToTimeStamp(SYSTEMTIME *st, time_t *timestamp)
{
	struct tm ts;
	ts.tm_isdst = -1;             /* daylight saving time (-1=compute) */
	ts.tm_sec = st->wSecond;      /* 0-59 */
	ts.tm_min = st->wMinute;      /* 0-59 */
	ts.tm_hour = st->wHour;       /* 0-23 */
	ts.tm_mday = st->wDay;        /* 1-31 */
	ts.tm_wday = st->wDayOfWeek;  /* 0-6 (Sun-Sat) */
	ts.tm_mon = st->wMonth - 1;     /* 0-11 (Jan-Dec) */
	ts.tm_year = st->wYear - 1900;  /* current year minus 1900 */
	ts.tm_yday = 0;               /* 0-365 (Jan1=0) */
	*timestamp = mktime(&ts);
	return (*timestamp != -1);
}

BOOL TimeStampToSystemTime(time_t timestamp, SYSTEMTIME *st)
{
	struct tm ts = { 0 };
	errno_t err = localtime_s(&ts, &timestamp);  /* statically alloced, local time correction */
	if (err != 0)
		return FALSE;

	st->wMilliseconds = 0;                 /* 0-999 (not given in tm) */
	st->wSecond = (uint16_t)ts.tm_sec;       /* 0-59 */
	st->wMinute = (uint16_t)ts.tm_min;       /* 0-59 */
	st->wHour = (uint16_t)ts.tm_hour;        /* 0-23 */
	st->wDay = (uint16_t)ts.tm_mday;         /* 1-31 */
	st->wDayOfWeek = (uint16_t)ts.tm_wday;   /* 0-6 (Sun-Sat) */
	st->wMonth = (uint16_t)(ts.tm_mon + 1);    /* 1-12 (Jan-Dec) */
	st->wYear = (uint16_t)(ts.tm_year + 1900); /* 1601-30827 */
	return TRUE;
}

BOOL GetFormatedCountdown(wchar_t *pszOut, int nSize, time_t countdown)
{
	static BOOL fInited = FALSE;
	static int (WINAPI *pfnGetDurationFormat)(LCID, uint32_t, const SYSTEMTIME*, double, wchar_t*, wchar_t*, int);
	/* Init */
	if (!fInited && IsWinVerVistaPlus()) {
		*(PROC*)&pfnGetDurationFormat = GetProcAddress(GetModuleHandleA("KERNEL32"), "GetDurationFormat");
		fInited = TRUE;
	}
	
	/* WinVista */
	if (pfnGetDurationFormat != nullptr) {
		SYSTEMTIME st;
		LCID locale;
		locale = Langpack_GetDefaultLocale();
		if (TimeStampToSystemTime(countdown, &st))
			if (pfnGetDurationFormat(locale, 0, &st, 0, nullptr, pszOut, nSize))
				return TRUE;
		return FALSE;
	}

	/* Win9x/NT/XP */
	return StrFromTimeInterval(pszOut, nSize, (countdown > (MAXDWORD / 1000)) ? MAXDWORD : (countdown * 1000), 10) != 0;
}

BOOL GetFormatedDateTime(wchar_t *pszOut, int nSize, time_t timestamp, BOOL fShowDateEvenToday)
{
	SYSTEMTIME st, stNow;
	LCID locale = Langpack_GetDefaultLocale();
	GetLocalTime(&stNow);
	TimeStampToSystemTime(timestamp, &st);
	/* today: no need to show the date */
	if (!fShowDateEvenToday && st.wDay == stNow.wDay && st.wMonth == stNow.wMonth && st.wYear == stNow.wYear)
		return GetTimeFormat(locale, ((st.wSecond == 0) ? TIME_NOSECONDS : 0) | TIME_FORCE24HOURFORMAT, &st, nullptr, pszOut, nSize) != 0;
	/* show both date and time */
	{
		wchar_t szDate[128], szTime[128];
		if (!GetTimeFormat(locale, ((st.wSecond == 0) ? TIME_NOSECONDS : 0) | TIME_FORCE24HOURFORMAT, &st, nullptr, szTime, _countof(szTime)))
			return FALSE;
		if (!GetDateFormat(locale, DATE_SHORTDATE, &st, nullptr, szDate, _countof(szDate)))
			return FALSE;
		mir_snwprintf(pszOut, nSize, L"%s %s", szTime, szDate);
		return TRUE;
	}
}

/************************* Skin ***********************************/

void AddHotkey()
{
	HOTKEYDESC hkd = {};
	hkd.dwFlags = HKD_UNICODE;
	hkd.pszName = "AutoShutdown_Toggle";
	hkd.szDescription.w = LPGENW("Toggle automatic shutdown");
	hkd.szSection.w = LPGENW("Main");
	hkd.pszService = "AutoShutdown/MenuCommand";
	hkd.DefHotKey = HOTKEYCODE(HOTKEYF_CONTROL | HOTKEYF_SHIFT, 'T') | HKF_MIRANDA_LOCAL;
	g_plugin.addHotkey(&hkd);
}
