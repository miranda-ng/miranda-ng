/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org)
Copyright (c) 2000-10 Miranda ICQ/IM project,
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
*/

#ifndef __M_TIMEZONES_H
#define __M_TIMEZONES_H

#ifndef M_CORE_H__
#include <m_core.h>
#endif

#define MIM_TZ_NAMELEN 64

#define TZF_PLF_CB		1				// UI element is assumed to be a combo box
#define TZF_PLF_LB		2				// UI element is assumed to be a list box
#define TZF_DIFONLY     4
#define TZF_KNOWNONLY   8

#define LOCAL_TIME_HANDLE NULL
#define UTC_TIME_HANDLE   ((void*)-1)

typedef INT_PTR mir_time;

EXTERN_C MIR_CORE_DLL(HANDLE) TimeZone_CreateByName(LPCTSTR tszName, uint32_t dwFlags);
EXTERN_C MIR_CORE_DLL(HANDLE) TimeZone_CreateByContact(MCONTACT hContact, LPCSTR szModule, uint32_t dwFlags);

EXTERN_C MIR_CORE_DLL(void) TimeZone_StoreByContact(MCONTACT hContact, LPCSTR szModule, HANDLE hTZ);
EXTERN_C MIR_CORE_DLL(void) TimeZone_StoreListResult(MCONTACT hContact, LPCSTR szModule, HWND hWnd, uint32_t dwFlags);

EXTERN_C MIR_CORE_DLL(int) TimeZone_PrintDateTime(HANDLE hTZ, LPCTSTR szFormat, LPTSTR szDest, size_t cbDest, uint32_t dwFlags);
EXTERN_C MIR_CORE_DLL(int) TimeZone_PrintTimeStamp(HANDLE hTZ, mir_time ts, LPCTSTR szFormat, LPTSTR szDest, size_t cbDest, uint32_t dwFlags);

EXTERN_C MIR_CORE_DLL(int) TimeZone_PrepareList(MCONTACT hContact, LPCSTR szModule, HWND hWnd, uint32_t dwFlags);
EXTERN_C MIR_CORE_DLL(int) TimeZone_SelectListItem(MCONTACT hContact, LPCSTR szModule, HWND hWnd, uint32_t dwFlags);

#ifdef _MSC_VER
EXTERN_C MIR_CORE_DLL(int) TimeZone_GetTimeZoneTime(HANDLE hTZ, SYSTEMTIME *st);
EXTERN_C MIR_CORE_DLL(int) TimeZone_GetSystemTime(HANDLE hTZ, mir_time src, SYSTEMTIME *dest, uint32_t dwFlags);

EXTERN_C MIR_CORE_DLL(LPTIME_ZONE_INFORMATION) TimeZone_GetInfo(HANDLE hTZ);
#endif
EXTERN_C MIR_CORE_DLL(mir_time) TimeZone_UtcToLocal(HANDLE hTZ, mir_time ts);

EXTERN_C MIR_CORE_DLL(LPCTSTR) TimeZone_GetName(HANDLE hTZ);
EXTERN_C MIR_CORE_DLL(LPCTSTR) TimeZone_GetDescription(LPCTSTR TZname);

#ifdef __cplusplus

__forceinline int printDateTimeByContact (MCONTACT hContact, LPCTSTR szFormat, LPTSTR szDest, int cbDest, uint32_t dwFlags)
{
	return TimeZone_PrintDateTime(TimeZone_CreateByContact(hContact, nullptr, dwFlags), szFormat, szDest, cbDest, dwFlags);
}

__forceinline	int printTimeStampByContact(MCONTACT hContact, mir_time ts, LPCTSTR szFormat, LPTSTR szDest, int cbDest, uint32_t dwFlags)
{
	return TimeZone_PrintTimeStamp(TimeZone_CreateByContact(hContact, nullptr, dwFlags), ts, szFormat, szDest, cbDest, dwFlags);
}

#ifdef _MSC_VER
__forceinline LPTIME_ZONE_INFORMATION getTziByContact(MCONTACT hContact, uint32_t dwFlags = 0)
{
	return TimeZone_GetInfo(TimeZone_CreateByContact(hContact, nullptr, dwFlags));
}

__forceinline int getTimeZoneTimeByContact(MCONTACT hContact, SYSTEMTIME *st, uint32_t dwFlags = 0)
{
	return TimeZone_GetTimeZoneTime(TimeZone_CreateByContact(hContact, nullptr, dwFlags), st);
}
#endif

__forceinline mir_time timeStampToTimeZoneTimeStampByContact(MCONTACT hContact, mir_time ts, uint32_t dwFlags = 0)
{
	return TimeZone_UtcToLocal(TimeZone_CreateByContact(hContact, nullptr, dwFlags), ts);
}

#endif

/////////////////////////////////////////////////////////////////////////////////////////
// Time services
//
// Converts a GMT timestamp into local time
// Returns the converted value
//
// Timestamps have zero at midnight 1/1/1970 GMT, this service converts such a
// value to be based at midnight 1/1/1970 local time.
// This service does not use a simple conversion based on the current offset
// between GMT and local. Rather, it figures out whether daylight savings time
// would have been in place at the time of the stamp and gives the local time as
// it would have been at the time and date the stamp contains.
// This service isn't nearly as useful as db/time/TimestampToString below and I
// recommend avoiding its use when possible so that you don't get your timezones
// mixed up (like I did. Living at GMT makes things easier for me, but has certain
// disadvantages :-)).

EXTERN_C MIR_CORE_DLL(uint32_t) TimeZone_ToLocal(uint32_t);

/////////////////////////////////////////////////////////////////////////////////////////
// Converts a GMT timestamp into a customisable local time string
// Returns the destination buffer
//
// Uses db/time/timestamptolocal for the conversion so read that description to
// see what's going on.
// The string is formatted according to the current user's locale, language and
// preferences.
// szFormat can have the following special characters:
// t  Time without seconds, eg hh:mm
// s  Time with seconds, eg hh:mm:ss
// m  Time without minutes, eg hh
// d  Short date, eg dd/mm/yyyy
// D  Long date, eg d mmmm yyyy
// I  ISO 8061 Time yyyy-mm-ddThh:mm:ssZ
// All other characters are copied across to szDest as-is

EXTERN_C MIR_CORE_DLL(char*) TimeZone_ToString(mir_time timeVal, const char *szFormat, char *szDest, size_t cchDest);
EXTERN_C MIR_CORE_DLL(wchar_t*) TimeZone_ToStringW(mir_time timeVal, const wchar_t *wszFormat, wchar_t *wszDest, size_t cchDest);

#define TimeZone_ToStringT TimeZone_ToStringW

#endif /* __M_TIMEZONES_H */
