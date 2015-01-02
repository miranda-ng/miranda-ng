/*
UserinfoEx plugin for Miranda IM

Copyright:
© 2006-2010 DeathAxe, Yasnovidyashii, Merlin, K. Romanov, Kreol

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
*/

#pragma once

class MTime {
	SYSTEMTIME	_SysTime;
	BYTE		_isLocal;

	LONG		_Offset(TIME_ZONE_INFORMATION *tzi);

public:
	// contruction
	MTime();
	MTime(SYSTEMTIME &st, const BYTE bIsLocal);
	MTime(FILETIME &ft, const BYTE bIsLocal);
	MTime(LARGE_INTEGER &li, const BYTE bIsLocal);
	MTime(DWORD dwStamp);
	MTime(const MTime& mtime);

	// checks
	__inline BYTE IsLocal() const		{ return _isLocal; };
	BYTE	IsValid() const;
	BYTE	IsLeapYear() const;

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
	WORD			DaysInYear(BYTE bIgnoreLeap = FALSE) const;
	WORD			DayOfYear() const;
	WORD			AdjustYear(const int nDiffDays);

	WORD			TimeFormat(LPTSTR ptszTimeFormat, WORD cchTimeFormat);
	WORD			TimeFormat(tstring& str);
	WORD			DateFormat(LPTSTR ptszTimeFormat, WORD cchTimeFormat);
	WORD			DateFormatAlt(LPTSTR ptszTimeFormat, WORD cchTimeFormat);
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
	__inline void	Minute(const WORD wMinute)	{ if (wMinute <= 59) _SysTime.wMinute = wMinute; };
	__inline void	Hour(const WORD wHour)		{ if (wHour <= 24) _SysTime.wHour = wHour; };
	__inline void	Day(const WORD wDay)		{ if (wDay <= 31) _SysTime.wDay = wDay; };
	__inline void	Month(const WORD wMonth)	{ if (wMonth <= 12) _SysTime.wMonth = wMonth; };
	__inline void	Year(const WORD wYear)		{ _SysTime.wYear = wYear; };

	// set value to class
	void	ZeroDate();
	void	FromStampAsUTC(const DWORD dwTimeStamp);
	void	FromStampAsLocal(const DWORD dwTimeStamp);
	void	Set(FILETIME &ftFileTime, const BYTE bIsLocal);
	void	Set(LARGE_INTEGER liFileTime, const BYTE bIsLocal);
	void	Set(SYSTEMTIME &st, const BYTE bIsLocal);
	void	Set(const MTime &mt);

	// get current time
	void	GetTimeUTC();
	void	GetLocalTime();
	void	GetLocalTime(MCONTACT hContact);

	// conversions
	void	UTCToLocal();
	void	UTCToTzSpecificLocal(int tzh);
	void	UTCToTzSpecificLocal(TIME_ZONE_INFORMATION *tzi);
	void	LocalToUTC();
	void	TzSpecificLocalToUTC(TIME_ZONE_INFORMATION *tzi);

	// read and write from and to db
	int		DBGetStamp(MCONTACT hContact, LPCSTR pszModule, LPCSTR pszSetting);
	int		DBWriteStamp(MCONTACT hContact, LPCSTR pszModule, LPCSTR pszSetting);

	// operatoren
	void operator = (DWORD& dwTimeStamp)			{ FromStampAsUTC(dwTimeStamp); };
	void operator = (FILETIME &ftFileTime)			{ Set(ftFileTime, FALSE); };
	void operator = (LARGE_INTEGER &liFileTime)		{ Set(liFileTime, FALSE); };
	void operator = (SYSTEMTIME &st)				{ Set(st, FALSE); };
	void operator = (const MTime &mt)				{ Set(mt); };
};

/**
 * prototypes
 **/
void		UserTime_LoadModule(void);
