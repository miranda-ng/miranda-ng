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
	SYSTEMTIME _SysTime;
	bool _isLocal;

	LONG _Offset(TIME_ZONE_INFORMATION *tzi);

public:
	// contruction
	MTime();
	MTime(SYSTEMTIME &st, bool bIsLocal);
	MTime(FILETIME &ft, bool bIsLocal);
	MTime(LARGE_INTEGER &li, bool bIsLocal);
	MTime(uint32_t dwStamp);
	MTime(const MTime& mtime);

	// checks
	__inline bool IsLocal() const { return _isLocal; }
	uint8_t	IsValid() const;
	uint8_t	IsLeapYear() const;

	// compare by seconds
	LONG	Compare(SYSTEMTIME st) const;
	LONG	Compare(const FILETIME &ft) const;
	LONG	Compare(const MTime &mt) const;
	LONG	Compare(const uint32_t dwTimeStamp) const;
	
	// get value from class
	LARGE_INTEGER LargeInt() const;
	FILETIME   FileTime() const;
	uint32_t   TimeStamp() const;
	SYSTEMTIME SystemTime() const { return _SysTime; }
	
	uint16_t DaysInMonth(const uint16_t &wMonth) const;
	uint16_t DaysInYear(uint8_t bIgnoreLeap = FALSE) const;
	uint16_t DayOfYear() const;
	uint16_t AdjustYear(const int nDiffDays);

	uint16_t TimeFormat(LPTSTR ptszTimeFormat, uint16_t cchTimeFormat);
	uint16_t DateFormat(LPTSTR ptszTimeFormat, uint16_t cchTimeFormat);
	uint16_t DateFormatAlt(LPTSTR ptszTimeFormat, uint16_t cchTimeFormat);
	uint16_t DateFormatLong(LPTSTR ptszTimeFormat, uint16_t cchTimeFormat);
	
	// return single attributes
	__inline uint16_t DayOfWeek()	const { return _SysTime.wDayOfWeek; }
	__inline uint16_t Day() const { return _SysTime.wDay; }
	__inline uint16_t Month() const { return _SysTime.wMonth; }
	__inline uint16_t Year() const { return _SysTime.wYear; }
	__inline uint16_t Hour() const { return _SysTime.wHour; }
	__inline uint16_t Minute() const { return _SysTime.wMinute; }
	__inline uint16_t Second() const { return _SysTime.wSecond; }

	// set single values
	__inline void Minute(const uint16_t wMinute) { if (wMinute <= 59) _SysTime.wMinute = wMinute; }
	__inline void Hour(const uint16_t wHour) { if (wHour <= 24) _SysTime.wHour = wHour; }
	__inline void Day(const uint16_t wDay) { if (wDay <= 31) _SysTime.wDay = wDay; }
	__inline void Month(const uint16_t wMonth) { if (wMonth <= 12) _SysTime.wMonth = wMonth; }
	__inline void Year(const uint16_t wYear) { _SysTime.wYear = wYear; }

	// set value to class
	void ZeroDate();
	void FromStampAsUTC(const uint32_t dwTimeStamp);
	void FromStampAsLocal(const uint32_t dwTimeStamp);
	void Set(const FILETIME &ftFileTime, bool bIsLocal);
	void Set(LARGE_INTEGER liFileTime, bool bIsLocal);
	void Set(const SYSTEMTIME &st, bool bIsLocal);
	void Set(const MTime &mt);

	// get current time
	void GetTimeUTC();
	void GetLocalTime();
	void GetLocalTime(MCONTACT hContact);

	// conversions
	void UTCToLocal();
	void UTCToTzSpecificLocal(int tzh);
	void UTCToTzSpecificLocal(TIME_ZONE_INFORMATION *tzi);
	void LocalToUTC();
	void TzSpecificLocalToUTC(TIME_ZONE_INFORMATION *tzi);

	// read and write from and to db
	int DBGetStamp(MCONTACT hContact, LPCSTR pszModule, LPCSTR pszSetting);
	int DBWriteStamp(MCONTACT hContact, LPCSTR pszModule, LPCSTR pszSetting);

	// operatoren
	void operator=(uint32_t& dwTimeStamp)			{ FromStampAsUTC(dwTimeStamp); };
	void operator=(FILETIME &ftFileTime)			{ Set(ftFileTime, FALSE); };
	void operator=(LARGE_INTEGER &liFileTime)		{ Set(liFileTime, FALSE); };
	void operator=(SYSTEMTIME &st)				{ Set(st, FALSE); };
	void operator=(const MTime &mt)				{ Set(mt); };
};

/**
 * prototypes
 **/
void		UserTime_LoadModule(void);
