/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org)
Copyright (ñ) 2000-10 Miranda ICQ/IM project,
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

#include <m_core.h>

#define MIM_TZ_NAMELEN 64

#define TZF_PLF_CB		1				// UI element is assumed to be a combo box
#define TZF_PLF_LB		2				// UI element is assumed to be a list box
#define TZF_DIFONLY     4
#define TZF_KNOWNONLY   8

#define LOCAL_TIME_HANDLE NULL
#define UTC_TIME_HANDLE   ((void*)-1)

typedef INT_PTR mir_time;

typedef struct
{
	size_t cbSize;

	HANDLE (*createByName)(LPCTSTR tszName, DWORD dwFlags);
	HANDLE (*createByContact)(MCONTACT hContact, LPCSTR szModule, DWORD dwFlags);
	void (*storeByContact)(MCONTACT hContact, LPCSTR szModule, HANDLE hTZ);

	int (*printDateTime)(HANDLE hTZ, LPCTSTR szFormat, LPTSTR szDest, int cbDest, DWORD dwFlags);
	int (*printTimeStamp)(HANDLE hTZ, mir_time ts, LPCTSTR szFormat, LPTSTR szDest, int cbDest, DWORD dwFlags);

	int (*prepareList)(MCONTACT hContact, LPCSTR szModule, HWND hWnd, DWORD dwFlags);
	int (*selectListItem)(MCONTACT hContact, LPCSTR szModule, HWND hWnd, DWORD dwFlags);
	void (*storeListResults)(MCONTACT hContact, LPCSTR szModule, HWND hWnd, DWORD dwFlags);

	int (*getTimeZoneTime)(HANDLE hTZ, SYSTEMTIME *st);
	mir_time (*timeStampToTimeZoneTimeStamp)(HANDLE hTZ, mir_time ts);

	LPTIME_ZONE_INFORMATION (*getTzi)(HANDLE hTZ);
	LPCTSTR (*getTzName)(HANDLE hTZ);
	LPCTSTR (*getTzDescription)(LPCTSTR TZname);

#ifdef __cplusplus
	int printDateTimeByContact (MCONTACT hContact, LPCTSTR szFormat, LPTSTR szDest, int cbDest, DWORD dwFlags)
	{ return printDateTime(createByContact(hContact, 0, dwFlags), szFormat, szDest, cbDest, dwFlags); }

	int printTimeStampByContact(MCONTACT hContact, mir_time ts, LPCTSTR szFormat, LPTSTR szDest, int cbDest, DWORD dwFlags)
	{ return printTimeStamp(createByContact(hContact, 0, dwFlags), ts, szFormat, szDest, cbDest, dwFlags);
	}

	LPTIME_ZONE_INFORMATION getTziByContact(MCONTACT hContact)
	{ return getTzi(createByContact(hContact, 0, 0)); }

	int getTimeZoneTimeByContact(MCONTACT hContact, SYSTEMTIME *st)
	{ return getTimeZoneTime(createByContact(hContact, 0, 0), st); }

	mir_time timeStampToTimeZoneTimeStampByContact(MCONTACT hContact, mir_time ts)
	{ return timeStampToTimeZoneTimeStamp(createByContact(hContact, 0, 0), ts); }
#endif

} TIME_API;

/* every protocol should declare this variable to use the Time API */
extern TIME_API tmi;

/*
a service to obtain the Time API

wParam = 0;
lParam = (LPARAM)(TIME_API*).

returns TRUE if all is Ok, and FALSE otherwise
*/

#define MS_SYSTEM_GET_TMI "Miranda/System/GetTimeApi"

__forceinline int mir_getTMI(TIME_API* dest)
{
	dest->cbSize = sizeof(*dest);
	return CallService(MS_SYSTEM_GET_TMI, 0, (LPARAM)dest);
}

#endif /* __M_TIMEZONES_H */
