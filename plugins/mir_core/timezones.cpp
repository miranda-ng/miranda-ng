/*
Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2010 Miranda ICQ/IM project, 
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, 
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

implements services to handle location - based timezones, instead of
simple UTC offsets.
*/

#include "commonheaders.h"

#include <m_timezones.h>

typedef DWORD 	(WINAPI *pfnGetDynamicTimeZoneInformation_t)(DYNAMIC_TIME_ZONE_INFORMATION *pdtzi);
static pfnGetDynamicTimeZoneInformation_t pfnGetDynamicTimeZoneInformation;

typedef HRESULT	(WINAPI *pfnSHLoadIndirectString_t)(LPCWSTR pszSource, LPWSTR pszOutBuf, UINT cchOutBuf,  void **ppvReserved);
static pfnSHLoadIndirectString_t pfnSHLoadIndirectString;

typedef LANGID (WINAPI *pfnGetUserDefaultUILanguage_t)(void);
static pfnGetUserDefaultUILanguage_t pfnGetUserDefaultUILanguage;

typedef LANGID (WINAPI *pfnGetSystemDefaultUILanguage_t)(void);
static pfnGetSystemDefaultUILanguage_t pfnGetSystemDefaultUILanguage;

typedef LPARAM (WINAPI *pfnSendMessageW_t)(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
static pfnSendMessageW_t pfnSendMessageW;

typedef struct _REG_TZI_FORMAT
{
    LONG Bias;
    LONG StandardBias;
    LONG DaylightBias;
    SYSTEMTIME StandardDate;
    SYSTEMTIME DaylightDate;
} REG_TZI_FORMAT;

#define MIM_TZ_DISPLAYLEN 128

struct MIM_TIMEZONE
{
	unsigned hash;
	int offset;

	TCHAR	tszName[MIM_TZ_NAMELEN];			// windows name for the time zone
	wchar_t	szDisplay[MIM_TZ_DISPLAYLEN];		// more descriptive display name (that's what usually appears in dialogs)
												// every hour should be sufficient.
	TIME_ZONE_INFORMATION tzi;

	static int compareBias(const MIM_TIMEZONE* p1, const MIM_TIMEZONE* p2)
	{ return p2->tzi.Bias - p1->tzi.Bias; }
};

typedef struct
{
	DWORD		timestamp;					// last time updated
	MIM_TIMEZONE myTZ;						// set to my own timezone
} TZ_INT_INFO;

static TZ_INT_INFO myInfo;
bool muiInstalled;

static OBJLIST<MIM_TIMEZONE>  g_timezones(55, NumericKeySortT);
static LIST<MIM_TIMEZONE>     g_timezonesBias(55, MIM_TIMEZONE::compareBias);

void FormatTime (const SYSTEMTIME *st, const TCHAR *szFormat, TCHAR *szDest, int cbDest);
void UnixTimeToFileTime(time_t ts, LPFILETIME pft);
time_t FileTimeToUnixTime(LPFILETIME pft);

#define fnSystemTimeToTzSpecificLocalTime SystemTimeToTzSpecificLocalTime

void GetLocalizedString(HKEY hSubKey, const TCHAR *szName, wchar_t *szBuf, DWORD cbLen)
{
	szBuf[0] = 0;
	if (muiInstalled)
	{
		TCHAR tszTempBuf[MIM_TZ_NAMELEN], tszName[30];
		mir_sntprintf(tszName, SIZEOF(tszName), _T("MUI_%s"), szName);
		DWORD dwLength = cbLen * sizeof(TCHAR);
		if (ERROR_SUCCESS == RegQueryValueEx(hSubKey, tszName, NULL, NULL, (unsigned char *)tszTempBuf, &dwLength))
		{
			tszTempBuf[min(dwLength / sizeof(TCHAR), cbLen - 1)] = 0;
			if (pfnSHLoadIndirectString)
				pfnSHLoadIndirectString(StrConvU(tszTempBuf), szBuf, cbLen, NULL);
		}
	}
	if (szBuf[0] == 0)
	{
		DWORD dwLength = cbLen * sizeof(wchar_t);


		RegQueryValueEx(hSubKey, szName, NULL, NULL, (unsigned char *)szBuf, &dwLength);
		szBuf[min(dwLength / sizeof(TCHAR), cbLen - 1)] = 0;
	}
}

void RecalculateTime(void)
{
	GetTimeZoneInformation(&myInfo.myTZ.tzi);
	myInfo.timestamp = time(NULL);
	myInfo.myTZ.offset = INT_MIN;

	bool found = false;
	DYNAMIC_TIME_ZONE_INFORMATION dtzi;

	if (pfnGetDynamicTimeZoneInformation && pfnGetDynamicTimeZoneInformation(&dtzi) != TIME_ZONE_ID_INVALID)
	{
		TCHAR *myTzKey = mir_u2t(dtzi.TimeZoneKeyName);
		_tcscpy(myInfo.myTZ.tszName, myTzKey);
		mir_free(myTzKey);
		found = true;
	}

	for (int i = 0; i < g_timezones.getCount(); ++i)
	{
		MIM_TIMEZONE &tz = g_timezones[i];
		if (tz.offset != INT_MIN) tz.offset = INT_MIN;

		if ( !found)
		{
			if ( !wcscmp(tz.tzi.StandardName, myInfo.myTZ.tzi.StandardName)  || 
				!wcscmp(tz.tzi.DaylightName, myInfo.myTZ.tzi.DaylightName))
			{
				_tcscpy(myInfo.myTZ.tszName, tz.tszName);
				found = true;
			}
		}
	}
}

void InitTimeZones(void)
{
	REG_TZI_FORMAT	tzi;
	HKEY			hKey;

	const TCHAR *tszKey = IsWinVerNT() ?
		_T("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Time Zones") :
		_T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Time Zones");

	/*
	 * use GetDynamicTimeZoneInformation() on Vista+ - this will return a structure with
	 * the registry key name, so finding our own time zone later will be MUCH easier for
	 * localized systems or systems with a MUI pack installed
	 */
	if (IsWinVerVistaPlus())
		pfnGetDynamicTimeZoneInformation = (pfnGetDynamicTimeZoneInformation_t)GetProcAddress(GetModuleHandle(_T("kernel32")), "GetDynamicTimeZoneInformation");

	if (IsWinVer2000Plus())
	{
		pfnSHLoadIndirectString = (pfnSHLoadIndirectString_t)GetProcAddress(GetModuleHandle(_T("shlwapi")), "SHLoadIndirectString");
		pfnGetSystemDefaultUILanguage = (pfnGetSystemDefaultUILanguage_t)GetProcAddress(GetModuleHandle(_T("kernel32")), "GetSystemDefaultUILanguage");
		pfnGetUserDefaultUILanguage = (pfnGetUserDefaultUILanguage_t)GetProcAddress(GetModuleHandle(_T("kernel32")), "GetUserDefaultUILanguage");
		muiInstalled = pfnSHLoadIndirectString && pfnGetSystemDefaultUILanguage() != pfnGetUserDefaultUILanguage();
	}

	pfnSendMessageW = (pfnSendMessageW_t)GetProcAddress(GetModuleHandle(_T("user32")), "SendMessageW");

	if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, tszKey, 0, KEY_ENUMERATE_SUB_KEYS, &hKey))
	{
		DWORD	dwIndex = 0;
		HKEY	hSubKey;
		TCHAR	tszName[MIM_TZ_NAMELEN];

		DWORD dwSize = SIZEOF(tszName);
		while (ERROR_NO_MORE_ITEMS != RegEnumKeyEx(hKey, dwIndex++, tszName, &dwSize, NULL, NULL, 0, NULL))
		{
			if (ERROR_SUCCESS == RegOpenKeyEx(hKey, tszName, 0, KEY_QUERY_VALUE, &hSubKey))
			{
				dwSize = sizeof(tszName);

				DWORD dwLength = sizeof(tzi);
				if (ERROR_SUCCESS != RegQueryValueEx(hSubKey, _T("TZI"), NULL, NULL, (unsigned char *)&tzi, &dwLength))
					continue;

				MIM_TIMEZONE *tz = new MIM_TIMEZONE;

				tz->tzi.Bias = tzi.Bias;
				tz->tzi.StandardDate = tzi.StandardDate;
				tz->tzi.StandardBias = tzi.StandardBias;
				tz->tzi.DaylightDate = tzi.DaylightDate;
				tz->tzi.DaylightBias = tzi.DaylightBias;

				_tcscpy(tz->tszName, tszName);
				tz->hash = mir_hashstrT(tszName);
				tz->offset = INT_MIN;

				GetLocalizedString(hSubKey, _T("Display"), tz->szDisplay, SIZEOF(tz->szDisplay));
				GetLocalizedString(hSubKey, _T("Std"), tz->tzi.StandardName, SIZEOF(tz->tzi.StandardName));
				GetLocalizedString(hSubKey, _T("Dlt"), tz->tzi.DaylightName, SIZEOF(tz->tzi.DaylightName));

				g_timezones.insert(tz);
				g_timezonesBias.insert(tz);

				RegCloseKey(hSubKey);
			}
			dwSize = SIZEOF(tszName);
		}
		RegCloseKey(hKey);
	}

	RecalculateTime();
}

void UninitTimeZones(void)
{
	g_timezonesBias.destroy();
	g_timezones.destroy();
}
