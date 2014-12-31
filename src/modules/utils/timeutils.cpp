/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org),
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

#include "..\..\core\commonheaders.h"

//KB167296
void UnixTimeToFileTime(mir_time ts, LPFILETIME pft)
{
	unsigned __int64 ll = UInt32x32To64(ts, 10000000) + 116444736000000000i64;
	pft->dwLowDateTime = (DWORD)ll;
	pft->dwHighDateTime = ll >> 32;
}

mir_time FileTimeToUnixTime(LPFILETIME pft)
{
	unsigned __int64 ll = (unsigned __int64)pft->dwHighDateTime << 32 | pft->dwLowDateTime;
	ll -= 116444736000000000i64;
	return (mir_time)(ll / 10000000);
}

void FormatTime(const SYSTEMTIME *st, const TCHAR *szFormat, TCHAR *szDest, int cbDest)
{
	if (szDest == NULL || cbDest == 0) return;

	TCHAR *pDest = szDest;
	int destCharsLeft = cbDest - 1;

	for (const TCHAR* pFormat = szFormat; *pFormat; ++pFormat)
	{
		DWORD fmt;
		bool date, iso = false;
		switch (*pFormat)
		{
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
			if (destCharsLeft--)
				*pDest++=*pFormat;
			continue;
		}

		TCHAR dateTimeStr[64];
		int dateTimeStrLen;

		if (iso)
		{
			dateTimeStrLen = mir_sntprintf(dateTimeStr, SIZEOF(dateTimeStr),
				_T("%d-%02d-%02dT%02d:%02d:%02dZ"),
				st->wYear, st->wMonth, st->wDay,
				st->wHour, st->wMinute, st->wSecond) + 1;
		}
		else if (date)
			dateTimeStrLen = GetDateFormat(LOCALE_USER_DEFAULT, fmt, st, NULL,
				dateTimeStr, SIZEOF(dateTimeStr));
		else
			dateTimeStrLen = GetTimeFormat(LOCALE_USER_DEFAULT, fmt, st, NULL,
				dateTimeStr, SIZEOF(dateTimeStr));

		if (dateTimeStrLen) --dateTimeStrLen;
		if (destCharsLeft < dateTimeStrLen) dateTimeStrLen = destCharsLeft;
		memcpy(pDest, dateTimeStr, dateTimeStrLen * sizeof(dateTimeStr[0]));
		destCharsLeft -= dateTimeStrLen;
		pDest += dateTimeStrLen;
	}
	*pDest = 0;
}
