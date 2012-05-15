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

//KB167296
void UnixTimeToFileTime(time_t ts, LPFILETIME pft)
{
	unsigned __int64 ll = UInt32x32To64(ts, 10000000) + 116444736000000000i64;
	pft->dwLowDateTime = (DWORD)ll;
	pft->dwHighDateTime = ll >> 32;
}

time_t FileTimeToUnixTime(LPFILETIME pft)
{
	unsigned __int64 ll = (unsigned __int64)pft->dwHighDateTime << 32 | pft->dwLowDateTime;
	ll -= 116444736000000000i64;
	return (time_t)(ll / 10000000);
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
				*pDest++ = *pFormat;
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


#ifndef _UNICODE
void ConvertToAbsolute (const SYSTEMTIME * pstLoc, const SYSTEMTIME * pstDst, SYSTEMTIME * pstDstAbs)
{
     static int    iDays [12] = { 31, 28, 31, 30, 31, 30, 
                                  31, 31, 30, 31, 30, 31 } ;
     int           iDay ;

          // Set up the aboluste date structure except for wDay, which we must find

     pstDstAbs->wYear         = pstLoc->wYear ;      // Notice from local date/time
     pstDstAbs->wMonth        = pstDst->wMonth ;
     pstDstAbs->wDayOfWeek    = pstDst->wDayOfWeek ;

     pstDstAbs->wHour         = pstDst->wHour ;
     pstDstAbs->wMinute       = pstDst->wMinute ;
     pstDstAbs->wSecond       = pstDst->wSecond ;
     pstDstAbs->wMilliseconds = pstDst->wMilliseconds ;

          // Fix the iDays array for leap years

     if ((pstLoc->wYear % 4 == 0) && ((pstLoc->wYear % 100 != 0) || 
                                      (pstLoc->wYear % 400 == 0)))
     {
          iDays[1] = 29 ;
     }

          // Find a day of the month that falls on the same 
          //   day of the week as the transition.

          // Suppose today is the 20th of the month (pstLoc->wDay = 20)
          // Suppose today is a Wednesday (pstLoc->wDayOfWeek = 3)
          // Suppose the transition occurs on a Friday (pstDst->wDayOfWeek = 5)
          // Then iDay = 31, meaning that the 31st falls on a Friday
          // (The 7 is this formula avoids negatives.)

     iDay = pstLoc->wDay + pstDst->wDayOfWeek + 7 - pstLoc->wDayOfWeek ;

          // Now shrink iDay to a value between 1 and 7.

     iDay = (iDay - 1) % 7 + 1 ;

          // Now iDay is a day of the month ranging from 1 to 7.
          // Recall that the wDay field of the structure can range
          //   from 1 to 5, 1 meaning "first", 2 meaning "second",
          //   and 5 meaning "last".
          // So, increase iDay so it's the proper day of the month.

     iDay += 7 * (pstDst->wDay - 1) ;

          // Could be that iDay overshot the end of the month, so
          //   fix it up using the number of days in each month

     if (iDay > iDays[pstDst->wMonth - 1])
          iDay -= 7 ;

          // Assign that day to the structure. 

     pstDstAbs->wDay = iDay ;
}

BOOL LocalGreaterThanTransition (const SYSTEMTIME * pstLoc, const SYSTEMTIME * pstTran)
{
     FILETIME      ftLoc, ftTran ;
     LARGE_INTEGER liLoc, liTran ;
     SYSTEMTIME    stTranAbs ;

          // Easy case: Just compare the two months

     if (pstLoc->wMonth != pstTran->wMonth)
          return (pstLoc->wMonth > pstTran->wMonth) ;

          // Well, we're in a transition month. That requires a bit more work.

          // Check if pstDst is in absolute or day-in-month format.
          //   (See documentation of TIME_ZONE_INFORMATION, StandardDate field.)

     if (pstTran->wYear)       // absolute format (haven't seen one yet!)
     {
          stTranAbs = * pstTran ;
     }
     else                     // day-in-month format
     {
          ConvertToAbsolute (pstLoc, pstTran, &stTranAbs) ;
     }

          // Now convert both date/time structures to large integers & compare
     
     SystemTimeToFileTime (pstLoc, &ftLoc) ;
     liLoc = * (LARGE_INTEGER *) (void *) &ftLoc ;

     SystemTimeToFileTime (&stTranAbs, &ftTran) ;
     liTran = * (LARGE_INTEGER *) (void *) &ftTran ;

     return (liLoc.QuadPart > liTran.QuadPart) ;
}

BOOL MySystemTimeToTzSpecificLocalTime(LPTIME_ZONE_INFORMATION ptzi, LPSYSTEMTIME pstUtc, LPSYSTEMTIME pstLoc) 
{
	// st is UTC

	FILETIME      ft ;
	LARGE_INTEGER li ;
	SYSTEMTIME    stDst ;

	if (IsWinVerNT())
		return SystemTimeToTzSpecificLocalTime(ptzi, pstUtc, pstLoc);

	// Convert time to a LARGE_INTEGER and subtract the bias

	SystemTimeToFileTime (pstUtc, &ft) ;
	li = * (LARGE_INTEGER *) (void *) &ft;
	li.QuadPart -= (LONGLONG) 600000000 * ptzi->Bias ;

	// Convert to a local date/time before application of daylight saving time.
	// The local date/time must be used to determine when the conversion occurs.

	ft = * (FILETIME *) (void *) &li ;
	FileTimeToSystemTime (&ft, pstLoc) ;

	// Find the time assuming Daylight Saving Time

	li.QuadPart -= (LONGLONG) 600000000 * ptzi->DaylightBias ;
	ft = * (FILETIME *) (void *) &li ;
	FileTimeToSystemTime (&ft, &stDst) ;

	// Now put li back the way it was

	li.QuadPart += (LONGLONG) 600000000 * ptzi->DaylightBias ;

	if (ptzi->StandardDate.wMonth)          // ie, daylight savings time
	{
          // Northern hemisphere
          if ((ptzi->DaylightDate.wMonth < ptzi->StandardDate.wMonth) &&

               (stDst.wMonth >= pstLoc->wMonth) &&           // avoid the end of year problem
               
               LocalGreaterThanTransition (pstLoc, &ptzi->DaylightDate) &&
              !LocalGreaterThanTransition (&stDst, &ptzi->StandardDate))
          {
               li.QuadPart -= (LONGLONG) 600000000 * ptzi->DaylightBias ;
          }
               // Southern hemisphere

          else if ((ptzi->StandardDate.wMonth < ptzi->DaylightDate.wMonth) &&
                  (!LocalGreaterThanTransition (&stDst, &ptzi->StandardDate) ||
                    LocalGreaterThanTransition (pstLoc, &ptzi->DaylightDate)))
          {
               li.QuadPart -= (LONGLONG) 600000000 * ptzi->DaylightBias ;
          }
          else
          {
               li.QuadPart -= (LONGLONG) 600000000 * ptzi->StandardBias ;
          }
     }

     ft = * (FILETIME *) (void *) &li ;
     FileTimeToSystemTime (&ft, pstLoc) ;
     return TRUE ;
}
#endif
