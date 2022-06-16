/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org),
Copyright (c) 2000-12 Miranda IM project,
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

#include "../stdafx.h"

typedef uint32_t 	(WINAPI *pfnGetDynamicTimeZoneInformation_t)(DYNAMIC_TIME_ZONE_INFORMATION *pdtzi);
static pfnGetDynamicTimeZoneInformation_t pfnGetDynamicTimeZoneInformation;

struct REG_TZI_FORMAT
{
	LONG Bias;
	LONG StandardBias;
	LONG DaylightBias;
	SYSTEMTIME StandardDate;
	SYSTEMTIME DaylightDate;
};

#define MIM_TZ_DISPLAYLEN 128

struct MIM_TIMEZONE
{
	unsigned hash;
	int offset;

	wchar_t	tszName[MIM_TZ_NAMELEN];            // windows name for the time zone
	wchar_t szDisplay[MIM_TZ_DISPLAYLEN];    // more descriptive display name (that's what usually appears in dialogs)
	                                          // every hour should be sufficient.
	TIME_ZONE_INFORMATION tzi;

	static int compareBias(const MIM_TIMEZONE* p1, const MIM_TIMEZONE* p2)
	{	return p2->tzi.Bias - p1->tzi.Bias;
	}
};

struct TZ_INT_INFO
{
	uint32_t timestamp;  // last time updated
	MIM_TIMEZONE myTZ;   // set to my own timezone
};

static TZ_INT_INFO myInfo;

static OBJLIST<MIM_TIMEZONE>  g_timezones(55, NumericKeySortT);
static LIST<MIM_TIMEZONE>     g_timezonesBias(55, MIM_TIMEZONE::compareBias);

// KB167296
void UnixTimeToFileTime(mir_time ts, LPFILETIME pft)
{
	unsigned __int64 ll = UInt32x32To64(ts, 10000000) + 116444736000000000i64;
	pft->dwLowDateTime = (uint32_t)ll;
	pft->dwHighDateTime = ll >> 32;
}

mir_time FileTimeToUnixTime(LPFILETIME pft)
{
	unsigned __int64 ll = (unsigned __int64)pft->dwHighDateTime << 32 | pft->dwLowDateTime;
	ll -= 116444736000000000i64;
	return (mir_time)(ll / 10000000);
}

void FormatTime(const SYSTEMTIME *st, const wchar_t *szFormat, wchar_t *szDest, size_t cbDest)
{
	if (szDest == nullptr || cbDest == 0) return;

	CMStringW tszTemp;

	for (const wchar_t* pFormat = szFormat; *pFormat; ++pFormat) {
		uint32_t fmt = 0;
		bool date = false, iso = false;
		switch (*pFormat) {
		case 't':
			fmt = TIME_NOSECONDS;
			date = false;
			break;

		case 's':
			fmt = 0;
			date = false;
			break;

		case 'm':
			fmt = TIME_NOMINUTESORSECONDS;
			date = false;
			break;

		case 'd':
			fmt = DATE_SHORTDATE;
			date = true;
			break;

		case 'D':
			fmt = DATE_LONGDATE;
			date = true;
			break;

		case 'I':
			iso = true;
			break;

		default:
			tszTemp.AppendChar(*pFormat);
			continue;
		}

		wchar_t dateTimeStr[64];
		if (iso)
			tszTemp.AppendFormat(L"%d-%02d-%02dT%02d:%02d:%02dZ", st->wYear, st->wMonth, st->wDay, st->wHour, st->wMinute, st->wSecond);
		else if (date) {
			GetDateFormat(LOCALE_USER_DEFAULT, fmt, st, nullptr, dateTimeStr, _countof(dateTimeStr));
			tszTemp.Append(dateTimeStr);
		}
		else {
			GetTimeFormat(LOCALE_USER_DEFAULT, fmt, st, nullptr, dateTimeStr, _countof(dateTimeStr));
			tszTemp.Append(dateTimeStr);
		}
	}

	wcsncpy_s(szDest, cbDest, tszTemp, _TRUNCATE);
}

MIR_CORE_DLL(int) TimeZone_GetTimeZoneTime(HANDLE hTZ, SYSTEMTIME *st)
{
	if (st == nullptr) return 1;

	MIM_TIMEZONE *tz = (MIM_TIMEZONE*)hTZ;
	if (tz == UTC_TIME_HANDLE)
		GetSystemTime(st);
	else if (tz && tz != &myInfo.myTZ) {
		SYSTEMTIME sto;
		GetSystemTime(&sto);
		return !SystemTimeToTzSpecificLocalTime(&tz->tzi, &sto, st);
	}
	else
		GetLocalTime(st);

	return 0;
}

MIR_CORE_DLL(LPCTSTR) TimeZone_GetName(HANDLE hTZ)
{
	MIM_TIMEZONE *tz = (MIM_TIMEZONE*)hTZ;
	if (tz == nullptr)
		return myInfo.myTZ.tszName;
	if (tz == UTC_TIME_HANDLE)
		return L"UTC";

	return tz->tszName;
}

MIR_CORE_DLL(LPCTSTR) TimeZone_GetDescription(LPCTSTR TZname)
{
	for (auto &tz : g_timezonesBias)
		if (!mir_wstrcmp(tz->tszName, TZname))
			return tz->szDisplay;

	return L"";
}

static void CalcTsOffset(MIM_TIMEZONE *tz)
{
	SYSTEMTIME st, stl;
	GetSystemTime(&st);

	FILETIME ft;
	SystemTimeToFileTime(&st, &ft);
	mir_time ts1 = FileTimeToUnixTime(&ft);

	if (!SystemTimeToTzSpecificLocalTime(&tz->tzi, &st, &stl))
		return;

	SystemTimeToFileTime(&stl, &ft);
	mir_time ts2 = FileTimeToUnixTime(&ft);

	tz->offset = ts2 - ts1;
}

static bool IsSameTime(MIM_TIMEZONE *tz)
{
	SYSTEMTIME st, stl;

	if (tz == &myInfo.myTZ)
		return true;

	TimeZone_GetTimeZoneTime(tz, &stl);
	TimeZone_GetTimeZoneTime(nullptr, &st);

	return st.wHour == stl.wHour && st.wMinute == stl.wMinute;
}

MIR_CORE_DLL(HANDLE) TimeZone_CreateByName(LPCTSTR tszName, uint32_t dwFlags)
{
	if (tszName == nullptr)
		return (dwFlags & (TZF_DIFONLY | TZF_KNOWNONLY)) ? nullptr : &myInfo.myTZ;

	if (!(dwFlags & TZF_PLF_CB)) 
		if (mir_wstrcmp(myInfo.myTZ.tszName, tszName) == 0)
			return (dwFlags & TZF_DIFONLY) ? nullptr : &myInfo.myTZ;

	MIM_TIMEZONE tzsearch;
	tzsearch.hash = mir_hashstrT(tszName);

	MIM_TIMEZONE *tz = g_timezones.find(&tzsearch);
	if (tz == nullptr)
		return (dwFlags & (TZF_DIFONLY | TZF_KNOWNONLY)) ? nullptr : &myInfo.myTZ;

	if (dwFlags & TZF_DIFONLY)
		return IsSameTime(tz) ? nullptr : tz;

	return tz;
}

MIR_CORE_DLL(HANDLE) TimeZone_CreateByContact(MCONTACT hContact, LPCSTR szModule, uint32_t dwFlags)
{
	if (hContact == NULL && szModule == nullptr)
		return (dwFlags & (TZF_DIFONLY | TZF_KNOWNONLY)) ? nullptr : &myInfo.myTZ;

	if (szModule == nullptr) szModule = "UserInfo";

	DBVARIANT dbv;
	if (!db_get_ws(hContact, szModule, "TzName", &dbv)) {
		HANDLE res = TimeZone_CreateByName(dbv.pwszVal, dwFlags);
		db_free(&dbv);
		if (res) return res;
	}

	signed char timezone = (signed char)db_get_b(hContact, szModule, "Timezone", -1);
	if (timezone == -1) {
		char *szProto = Proto_GetBaseAccountName(hContact);
		if (!db_get_ws(hContact, szProto, "TzName", &dbv)) {
			HANDLE res = TimeZone_CreateByName(dbv.pwszVal, dwFlags);
			db_free(&dbv);
			if (res) return res;
		}
		timezone = (signed char)db_get_b(hContact, szProto, "Timezone", -1);
	}

	if (timezone != -1) {
		MIM_TIMEZONE tzsearch;
		tzsearch.tzi.Bias = timezone * 30;
		if (myInfo.myTZ.tzi.Bias == tzsearch.tzi.Bias) {
			if (dwFlags & TZF_DIFONLY) return nullptr;
			return &myInfo.myTZ;
		}

		int i = g_timezonesBias.getIndex(&tzsearch);
		while (i >= 0 && g_timezonesBias[i]->tzi.Bias == tzsearch.tzi.Bias) --i;

		int delta = LONG_MAX;
		for (int j = ++i; j < g_timezonesBias.getCount() && g_timezonesBias[j]->tzi.Bias == tzsearch.tzi.Bias; ++j) {
			int delta1 = abs(g_timezonesBias[j]->tzi.DaylightDate.wMonth - myInfo.myTZ.tzi.DaylightDate.wMonth);
			if (delta1 <= delta) {
				delta = delta1;
				i = j;
			}
		}

		if (i >= 0) {
			MIM_TIMEZONE *tz = g_timezonesBias[i];
			return ((dwFlags & TZF_DIFONLY) && IsSameTime(tz)) ? nullptr : tz;
		}
	}
	return (dwFlags & (TZF_DIFONLY | TZF_KNOWNONLY)) ? nullptr : &myInfo.myTZ;
}

MIR_CORE_DLL(void) TimeZone_StoreByContact(MCONTACT hContact, LPCSTR szModule, HANDLE hTZ)
{
	if (szModule == nullptr) szModule = "UserInfo";

	MIM_TIMEZONE *tz = (MIM_TIMEZONE*)hTZ;
	if (tz) {
		db_set_ws(hContact, szModule, "TzName", tz->tszName);
		db_set_b(hContact, szModule, "Timezone", (char)((tz->tzi.Bias + tz->tzi.StandardBias) / 30));
	}
	else {
		db_unset(hContact, szModule, "TzName");
		db_unset(hContact, szModule, "Timezone");
	}
}

MIR_CORE_DLL(int) TimeZone_PrintDateTime(HANDLE hTZ, LPCTSTR szFormat, LPTSTR szDest, size_t cbDest, uint32_t dwFlags)
{
	MIM_TIMEZONE *tz = (MIM_TIMEZONE*)hTZ;
	if (tz == nullptr && (dwFlags & (TZF_DIFONLY | TZF_KNOWNONLY)))
		return 1;

	if (tz == nullptr)
		tz = &myInfo.myTZ;

	SYSTEMTIME st;
	if (TimeZone_GetTimeZoneTime(tz, &st))
		return 1;

	FormatTime(&st, szFormat, szDest, cbDest);
	return 0;
}

MIR_CORE_DLL(int) TimeZone_GetSystemTime(HANDLE hTZ, mir_time ts, SYSTEMTIME *dest, uint32_t dwFlags)
{
	if (dest == nullptr)
		return 2;

	MIM_TIMEZONE *tz = (MIM_TIMEZONE *)hTZ;
	if (tz == nullptr && (dwFlags & (TZF_DIFONLY | TZF_KNOWNONLY))) {
		memset(dest, 0, sizeof(SYSTEMTIME));
		return 1;
	}

	if (tz == nullptr)
		tz = &myInfo.myTZ;

	FILETIME ft;
	if (tz == UTC_TIME_HANDLE)
		UnixTimeToFileTime(ts, &ft);
	else {
		if (tz->offset == INT_MIN)
			CalcTsOffset(tz);

		UnixTimeToFileTime(ts + tz->offset, &ft);
	}

	FileTimeToSystemTime(&ft, dest);
	return 0;
}

MIR_CORE_DLL(int) TimeZone_PrintTimeStamp(HANDLE hTZ, mir_time ts, LPCTSTR szFormat, LPTSTR szDest, size_t cbDest, uint32_t dwFlags)
{
	SYSTEMTIME st;
	if (!TimeZone_GetSystemTime(hTZ, ts, &st, dwFlags))
		FormatTime(&st, szFormat, szDest, cbDest);
	return 0;
}

MIR_CORE_DLL(LPTIME_ZONE_INFORMATION) TimeZone_GetInfo(HANDLE hTZ)
{
	MIM_TIMEZONE *tz = (MIM_TIMEZONE*)hTZ;
	return tz ? &tz->tzi : &myInfo.myTZ.tzi;
}

MIR_CORE_DLL(mir_time) TimeZone_UtcToLocal(HANDLE hTZ, mir_time ts)
{
	MIM_TIMEZONE *tz = (MIM_TIMEZONE*)hTZ;
	if (tz == nullptr)
		tz = &myInfo.myTZ;

	if (tz == UTC_TIME_HANDLE)
		return ts;

	if (tz->offset == INT_MIN)
		CalcTsOffset(tz);

	return ts + tz->offset;
}

///////////////////////////////////////////////////////////////////////////////

struct ListMessages
{
	UINT addStr, getSel, setSel, getData, setData;
};

static const ListMessages lbMessages = { LB_ADDSTRING, LB_GETCURSEL, LB_SETCURSEL, LB_GETITEMDATA, LB_SETITEMDATA };
static const ListMessages cbMessages = { CB_ADDSTRING, CB_GETCURSEL, CB_SETCURSEL, CB_GETITEMDATA, CB_SETITEMDATA };

static const ListMessages* GetListMessages(HWND hWnd, uint32_t dwFlags)
{
	if (hWnd == nullptr)
		return nullptr;

	if (!(dwFlags & (TZF_PLF_CB | TZF_PLF_LB))) {
		wchar_t	tszClassName[128];
		GetClassName(hWnd, tszClassName, _countof(tszClassName));
		if (!mir_wstrcmpi(tszClassName, L"COMBOBOX"))
			dwFlags |= TZF_PLF_CB;
		else if (!mir_wstrcmpi(tszClassName, L"LISTBOX"))
			dwFlags |= TZF_PLF_LB;
	}
	
	if (dwFlags & TZF_PLF_CB)
		return &cbMessages;
	if (dwFlags & TZF_PLF_LB)
		return &lbMessages;
	return nullptr;
}

///////////////////////////////////////////////////////////////////////////////

MIR_CORE_DLL(int) TimeZone_SelectListItem(MCONTACT hContact, LPCSTR szModule, HWND hWnd, uint32_t dwFlags)
{
	const ListMessages *lstMsg = GetListMessages(hWnd, dwFlags);
	if (lstMsg == nullptr)
		return -1;

	if (szModule == nullptr)
		szModule = "UserInfo";

	int iSelection = 0;
	ptrW tszName(db_get_wsa(hContact, szModule, "TzName"));
	if (tszName != NULL) {
		unsigned hash = mir_hashstrT(tszName);
		for (auto &it : g_timezonesBias) {
			if (hash == it->hash) {
				iSelection = g_timezonesBias.indexOf(&it) + 1;
				break;
			}
		}
	}
	else {
		signed char cBias = db_get_b(hContact, szModule, "Timezone", -100);
		if (cBias != -100) {
			int iBias = cBias * 30;
			for (auto &it : g_timezonesBias) {
				if (iBias == it->tzi.Bias) {
					iSelection = g_timezonesBias.indexOf(&it) + 1;
					break;
				}
			}
		}
	}

	SendMessage(hWnd, lstMsg->setSel, iSelection, 0);
	return iSelection;
}

MIR_CORE_DLL(int) TimeZone_PrepareList(MCONTACT hContact, LPCSTR szModule, HWND hWnd, uint32_t dwFlags)
{
	const ListMessages *lstMsg = GetListMessages(hWnd, dwFlags);
	if (lstMsg == nullptr)
		return 0;

	SendMessage(hWnd, lstMsg->addStr, 0, (LPARAM)TranslateW_LP(L"<unspecified>"));

	for (auto &it : g_timezonesBias) {
		SendMessage(hWnd, lstMsg->addStr, 0, (LPARAM)it->szDisplay);
		SendMessage(hWnd, lstMsg->setData, g_timezonesBias.indexOf(&it) + 1, (LPARAM)it);
	}

	return TimeZone_SelectListItem(hContact, szModule, hWnd, dwFlags);
}

MIR_CORE_DLL(void) TimeZone_StoreListResult(MCONTACT hContact, LPCSTR szModule, HWND hWnd, uint32_t dwFlags)
{
	if (szModule == nullptr) szModule = "UserInfo";

	const ListMessages *lstMsg = GetListMessages(hWnd, dwFlags);
	if (lstMsg) {
		LRESULT offset = SendMessage(hWnd, lstMsg->getSel, 0, 0);
		if (offset > 0) {
			MIM_TIMEZONE *tz = (MIM_TIMEZONE*)SendMessage(hWnd, lstMsg->getData, offset, 0);
			if ((INT_PTR)tz != CB_ERR && tz != nullptr)
				TimeZone_StoreByContact(hContact, szModule, tz);
		}
		else TimeZone_StoreByContact(hContact, szModule, nullptr);
	}
}

///////////////////////////////////////////////////////////////////////////////

MIR_CORE_DLL(uint32_t) TimeZone_ToLocal(uint32_t timeVal)
{
	return TimeZone_UtcToLocal(nullptr, (mir_time)timeVal);
}

MIR_CORE_DLL(char*) TimeZone_ToString(mir_time timeVal, const char *szFormat, char *szDest, size_t cchDest)
{
	wchar_t *szTemp = (wchar_t*)alloca(cchDest*sizeof(wchar_t));
	TimeZone_PrintTimeStamp(nullptr, timeVal, _A2T(szFormat), szTemp, cchDest, 0);
	WideCharToMultiByte(CP_ACP, 0, szTemp, -1, szDest, (int)cchDest, nullptr, nullptr);
	return szDest;
}

MIR_CORE_DLL(wchar_t*) TimeZone_ToStringW(mir_time timeVal, const wchar_t *wszFormat, wchar_t *wszDest, size_t cchDest)
{
	TimeZone_PrintTimeStamp(nullptr, timeVal, wszFormat, wszDest, cchDest, 0);
	return wszDest;
}

///////////////////////////////////////////////////////////////////////////////

void GetLocalizedString(HKEY hSubKey, const wchar_t *szName, wchar_t *szBuf, uint32_t cbLen)
{
	DWORD dwLength = cbLen * sizeof(wchar_t);
	RegQueryValueEx(hSubKey, szName, nullptr, nullptr, (unsigned char *)szBuf, &dwLength);
	szBuf[min(dwLength / sizeof(wchar_t), cbLen - 1)] = 0;
}

void RecalculateTime(void)
{
	GetTimeZoneInformation(&myInfo.myTZ.tzi);
	myInfo.timestamp = time(0);
	myInfo.myTZ.offset = INT_MIN;

	bool found = false;
	DYNAMIC_TIME_ZONE_INFORMATION dtzi;

	if (pfnGetDynamicTimeZoneInformation && pfnGetDynamicTimeZoneInformation(&dtzi) != TIME_ZONE_ID_INVALID) {
		wcsncpy_s(myInfo.myTZ.tszName, dtzi.TimeZoneKeyName, _TRUNCATE);
		found = true;
	}

	for (auto &tz : g_timezones) {
		if (tz->offset != INT_MIN)
			tz->offset = INT_MIN;

		if (!found) {
			if (!mir_wstrcmp(tz->tzi.StandardName, myInfo.myTZ.tzi.StandardName) || !mir_wstrcmp(tz->tzi.DaylightName, myInfo.myTZ.tzi.DaylightName)) {
				wcsncpy_s(myInfo.myTZ.tszName, tz->tszName, _TRUNCATE);
				found = true;
			}
		}
	}
}

void InitTimeZones(void)
{
	REG_TZI_FORMAT	tzi;
	HKEY			hKey;

	const wchar_t *tszKey = L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Time Zones";

	/*
	 * use GetDynamicTimeZoneInformation() on Vista+ - this will return a structure with
	 * the registry key name, so finding our own time zone later will be MUCH easier for
	 * localized systems or systems with a MUI pack installed
	 */
	if (IsWinVerVistaPlus())
		pfnGetDynamicTimeZoneInformation = (pfnGetDynamicTimeZoneInformation_t)GetProcAddress(GetModuleHandle(L"kernel32"), "GetDynamicTimeZoneInformation");

	if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, tszKey, 0, KEY_ENUMERATE_SUB_KEYS, &hKey)) {
		uint32_t	dwIndex = 0;
		HKEY	hSubKey;
		wchar_t	tszName[MIM_TZ_NAMELEN];

		DWORD dwSize = _countof(tszName);
		while (ERROR_NO_MORE_ITEMS != RegEnumKeyEx(hKey, dwIndex++, tszName, &dwSize, nullptr, nullptr, nullptr, nullptr)) {
			if (ERROR_SUCCESS == RegOpenKeyEx(hKey, tszName, 0, KEY_QUERY_VALUE, &hSubKey)) {
				dwSize = sizeof(tszName);

				DWORD dwLength = sizeof(tzi);
				if (ERROR_SUCCESS != RegQueryValueEx(hSubKey, L"TZI", nullptr, nullptr, (unsigned char *)&tzi, &dwLength))
					continue;

				MIM_TIMEZONE *tz = new MIM_TIMEZONE;

				tz->tzi.Bias = tzi.Bias;
				tz->tzi.StandardDate = tzi.StandardDate;
				tz->tzi.StandardBias = tzi.StandardBias;
				tz->tzi.DaylightDate = tzi.DaylightDate;
				tz->tzi.DaylightBias = tzi.DaylightBias;

				mir_wstrcpy(tz->tszName, tszName);
				tz->hash = mir_hashstrT(tszName);
				tz->offset = INT_MIN;

				GetLocalizedString(hSubKey, L"Display", tz->szDisplay, _countof(tz->szDisplay));
				GetLocalizedString(hSubKey, L"Std", tz->tzi.StandardName, _countof(tz->tzi.StandardName));
				GetLocalizedString(hSubKey, L"Dlt", tz->tzi.DaylightName, _countof(tz->tzi.DaylightName));

				g_timezones.insert(tz);
				g_timezonesBias.insert(tz);

				RegCloseKey(hSubKey);
			}
			dwSize = _countof(tszName);
		}
		RegCloseKey(hKey);
	}

	RecalculateTime();
}
