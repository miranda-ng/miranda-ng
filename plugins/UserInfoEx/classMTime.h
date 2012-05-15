/*
UserinfoEx plugin for Miranda IM

Copyright:
� 2006-2010 DeathAxe, Yasnovidyashii, Merlin, K. Romanov, Kreol

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

===============================================================================

File name      : $HeadURL: https://userinfoex.googlecode.com/svn/trunk/classMTime.h $
Revision       : $Revision: 194 $
Last change on : $Date: 2010-09-20 15:57:18 +0400 (Пн, 20 сен 2010) $
Last change by : $Author: ing.u.horn $

===============================================================================
*/
#pragma once

class MTime {
	SYSTEMTIME	_SysTime;
	BOOLEAN		_isLocal;

	LONG		_Offset(TIME_ZONE_INFORMATION *tzi);

public:
	// contruction
	MTime();
	MTime(SYSTEMTIME &st, const BOOLEAN bIsLocal);
	MTime(FILETIME &ft, const BOOLEAN bIsLocal);
	MTime(LARGE_INTEGER &li, const BOOLEAN bIsLocal);
	MTime(DWORD dwStamp);
	MTime(const MTime& mtime);

	// checks
	__inline BOOLEAN IsLocal() const		{ return _isLocal; };
	BOOLEAN	IsValid() const;
	BOOLEAN	IsLeapYear() const;

	// compare by seconds
	LONG	Compare(SYSTEMTIME st) const;
	LONG	Compare(const FILETIME &ft) const;
	LONG	Compare(const MTime &mt) const;
	LONG	Compare(const DWORD dwTimeStamp) const;
	
	// get value from class
	LARGE_INTEGER	LargeInt() const;
	FILETIME		FileTime() const;
	DWORD			TimeStamp() const;
	SYSTEMTIME		SystemTime() const	{ return _SysTime; };
	WORD			DaysInMonth(const WORD &wMonth) const;
	WORD			DaysInYear(BOOLEAN bIgnoreLeap = FALSE) const;
	WORD			DayOfYear() const;
	WORD			AdjustYear(const INT nDiffDays);

	WORD			TimeFormat(LPTSTR ptszTimeFormat, WORD cchTimeFormat);
	WORD			TimeFormat(tstring& str);
	WORD			DateFormat(LPTSTR ptszTimeFormat, WORD cchTimeFormat);
	WORD			DateFormatLong(LPTSTR ptszTimeFormat, WORD cchTimeFormat);
	
	// return single attributes
	__inline WORD	DayOfWeek()	const			{ return _SysTime.wDayOfWeek;	};
	__inline WORD	Day()		const			{ return _SysTime.wDay;			};
	__inline WORD	Month()		const			{ return _SysTime.wMonth;		};
	__inline WORD	Year()		const			{ return _SysTime.wYear;		};
	__inline WORD	Hour()		const			{ return _SysTime.wHour;		};
	__inline WORD	Minute()	const			{ return _SysTime.wMinute;		};
	__inline WORD	Second()	const			{ return _SysTime.wSecond;		};

	// set single values
	__inline VOID	Minute(const WORD wMinute)	{ if (wMinute <= 59) _SysTime.wMinute = wMinute; };
	__inline VOID	Hour(const WORD wHour)		{ if (wHour <= 24) _SysTime.wHour = wHour; };
	__inline VOID	Day(const WORD wDay)		{ if (wDay <= 31) _SysTime.wDay = wDay; };
	__inline VOID	Month(const WORD wMonth)	{ if (wMonth <= 12) _SysTime.wMonth = wMonth; };
	__inline VOID	Year(const WORD wYear)		{ _SysTime.wYear = wYear; };

	// set value to class
	VOID	ZeroDate();
	VOID	FromStampAsUTC(const DWORD dwTimeStamp);
	VOID	FromStampAsLocal(const DWORD dwTimeStamp);
	VOID	Set(FILETIME &ftFileTime, const BOOLEAN bIsLocal);
	VOID	Set(LARGE_INTEGER liFileTime, const BOOLEAN bIsLocal);
	VOID	Set(SYSTEMTIME &st, const BOOLEAN bIsLocal);
	VOID	Set(const MTime &mt);

	// get current time
	VOID	GetTimeUTC();
	VOID	GetLocalTime();
	VOID	GetLocalTime(HANDLE hContact);

	// conversions
	VOID	UTCToLocal();
	VOID	UTCToTzSpecificLocal(INT tzh);
	VOID	UTCToTzSpecificLocal(TIME_ZONE_INFORMATION *tzi);
	VOID	LocalToUTC();
	VOID	TzSpecificLocalToUTC(TIME_ZONE_INFORMATION *tzi);

	// read and write from and to db
	INT		DBGetStamp(HANDLE hContact, LPCSTR pszModule, LPCSTR pszSetting);
	INT		DBWriteStamp(HANDLE hContact, LPCSTR pszModule, LPCSTR pszSetting);

	// operatoren
	VOID operator = (DWORD& dwTimeStamp)			{ FromStampAsUTC(dwTimeStamp); };
	VOID operator = (FILETIME &ftFileTime)			{ Set(ftFileTime, FALSE); };
	VOID operator = (LARGE_INTEGER &liFileTime)		{ Set(liFileTime, FALSE); };
	VOID operator = (SYSTEMTIME &st)				{ Set(st, FALSE); };
	VOID operator = (const MTime &mt)				{ Set(mt); };
};

/**
 * prototypes
 **/
VOID		UserTime_LoadModule(VOID);
