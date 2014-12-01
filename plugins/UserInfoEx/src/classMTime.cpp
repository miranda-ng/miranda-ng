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

/**
 * System Includes:
 **/
#include "commonheaders.h"

/******************************************************************************************
 * class MTime
 *
 *****************************************************************************************/

/*********************************************
 * construction
 *********************************************/

MTime::MTime() 
{
	ZeroDate();
}

MTime::MTime(SYSTEMTIME &st, const BYTE bIsLocal)
{
	_SysTime = st;
	_isLocal = bIsLocal != FALSE;
}

MTime::MTime(FILETIME &ft, const BYTE bIsLocal)
{
	ZeroDate();
	Set(ft, bIsLocal);
}

MTime::MTime(LARGE_INTEGER &li, const BYTE bIsLocal)
{
	ZeroDate();
	Set(li, bIsLocal);
}

MTime::MTime(DWORD dwStamp)
{
	ZeroDate();
	FromStampAsUTC(dwStamp);
}

MTime::MTime(const MTime& mtime)
{
	Set(mtime);
}

void	MTime::ZeroDate() 
{
	_isLocal = FALSE;
	memset(&_SysTime, 0, sizeof(_SysTime));
}

/*********************************************
 * validation / checks
 *********************************************/

BYTE	MTime::IsValid() const
{
	return (
		_SysTime.wYear > 1600 &&
		_SysTime.wMonth > 0 && _SysTime.wMonth < 13 &&
		_SysTime.wDay > 0 && _SysTime.wDay <= DaysInMonth(_SysTime.wMonth) &&
		_SysTime.wHour < 25 && 
		_SysTime.wMinute < 60 && 
		_SysTime.wSecond < 60 );
}

BYTE	MTime::IsLeapYear() const
{
	return (!(((_SysTime.wYear) % 4 != 0) || (((_SysTime.wYear) % 100 == 0) && ((_SysTime.wYear) % 400 != 0))));
}

LONG	MTime::Compare(const DWORD dwTimeStamp) const
{
	return (LONG)(TimeStamp() - dwTimeStamp);
}

/**
 * name:	Compare
 * desc:	compare a filetime with the value of current object and return difference as number of seconds
 * param:	ft	- FILETIME to compare with
 * return:	number of seconds the ft differs from the class value
 **/ 
LONG	MTime::Compare(const FILETIME &ft) const
{
	const FILETIME ft1 = FileTime();
	return (LONG)((*(__int64*)&ft1 - *(__int64*)&ft) / 10000000i64);
}

/**
 * name:	Compare
 * desc:	compare a systemtime with the value of current object and return difference as number of seconds it handles some strange, too.
 * param:	st	- SYSTEMTIME to compare with
 * return:	number of seconds the st differs from the class value
 **/ 
LONG	MTime::Compare(SYSTEMTIME st) const
{
	FILETIME ft2;

	//strange day-in-month thing
	if (st.wYear == 0) {
		if (Month() < st.wMonth) return -1;
		if (Month() > st.wMonth) return 1;

		MTime mtTmp(st, FALSE);

		mtTmp.Year(Year());
		mtTmp.Day(1);
		mtTmp.Set(mtTmp.FileTime(), FALSE);	//gets the day of week of the first of the month
		mtTmp.Day(1 + (7 + st.wDayOfWeek - mtTmp.DayOfWeek()) % 7);

		//last wDayOfWeek in month
		if (st.wDay == 5) {
			mtTmp.Day(mtTmp.Day() + 7 * 3);	//can't be less than 4 of that day in the month
			if (mtTmp.Day() + 7 <= mtTmp.DaysInMonth(st.wMonth - 1))
				mtTmp.Day(mtTmp.Day() + 7);
		}
		else
			mtTmp.Day(7 * (st.wDay - 1)); //nth of month

		ft2 = mtTmp.FileTime();
	}
	else {
		SystemTimeToFileTime(&st, &ft2);
	}
	return Compare(ft2);
}

/**
 * name:	Compare
 * desc:	compare a MTime with the value of current object and return difference as number of seconds it handles some strange, too.
 * param:	mt	- MTime to compare with
 * return:	number of seconds the st differs from the class value
 **/ 
LONG	MTime::Compare(const MTime &mt) const
{
	return Compare(mt.SystemTime());
}

/*********************************************
 * conversions
 *********************************************/

void	MTime::UTCToLocal()
{
	if (!IsLocal()) {
		TIME_ZONE_INFORMATION tzInfo;
		
		memset(&tzInfo, 0, sizeof(TIME_ZONE_INFORMATION));
		GetTimeZoneInformation(&tzInfo);
		UTCToTzSpecificLocal(&tzInfo);
	}
}

void	MTime::UTCToTzSpecificLocal(int tzh)
{

	TIME_ZONE_INFORMATION tzInfo;

	if (IsLocal()) LocalToUTC();
	memset(&tzInfo, 0, sizeof(TIME_ZONE_INFORMATION));

	if (tzh > 24) tzh = 24;
	if (tzh < -24)tzh = -24;

	GetTimeZoneInformation(&tzInfo);
	tzInfo.Bias = tzh * 30i64;
	UTCToTzSpecificLocal(&tzInfo);
}

LONG	MTime::_Offset(TIME_ZONE_INFORMATION *tzi)
{
	LONG offset = tzi->Bias;
	// daylight saving times
	if (tzi->StandardDate.wMonth != 0) {
		if (tzi->DaylightDate.wMonth < tzi->StandardDate.wMonth) {
			//northern hemisphere
			if (Compare(tzi->DaylightDate) < 0 || Compare(tzi->StandardDate) > 0) 
				offset += tzi->StandardBias;
			else
				offset += tzi->DaylightBias;
		 }
		 else {
			//southern hemisphere
			if (Compare(tzi->StandardDate) < 0 || Compare(tzi->DaylightDate) > 0)
				offset += tzi->DaylightBias;
			else
				offset += tzi->StandardBias;
		 }
	}
	return offset;
}

void	MTime::UTCToTzSpecificLocal(TIME_ZONE_INFORMATION *tzi)
{
	LARGE_INTEGER liFiletime;

	// do not transform to local twice
	if (tzi && !_isLocal) {
		liFiletime = LargeInt();
		liFiletime.QuadPart -= _Offset(tzi) * 60 * 10000000i64;
		Set(liFiletime, TRUE);
	}
}

void	MTime::TzSpecificLocalToUTC(TIME_ZONE_INFORMATION *tzi)
{
	LARGE_INTEGER liFiletime;

	// do not transform to utc twice
	if (tzi && _isLocal) {
		liFiletime = LargeInt();
		liFiletime.QuadPart += _Offset(tzi) * 60 * 10000000i64;
		Set(liFiletime, TRUE);
	}
}

void	MTime::LocalToUTC()
{
	TIME_ZONE_INFORMATION tzInfo;
	
	GetTimeZoneInformation(&tzInfo);
	TzSpecificLocalToUTC(&tzInfo);
}

/*********************************************
 * return values
 *********************************************/

LARGE_INTEGER	MTime::LargeInt() const
{
	LARGE_INTEGER liFileTime = { 0 };

	SystemTimeToFileTime(&_SysTime, (LPFILETIME)&liFileTime);
	return liFileTime;
}

FILETIME		MTime::FileTime() const
{
	FILETIME ftFileTime;

	SystemTimeToFileTime(&_SysTime, &ftFileTime);
	return ftFileTime;
}

DWORD	MTime::TimeStamp() const
{
	LARGE_INTEGER li;

	if (IsLocal()) {
		MTime mt(*this);
		mt.LocalToUTC();
		li = mt.LargeInt();
	}
	else
		li = LargeInt();

	li.QuadPart /= 10000000i64;
	li.QuadPart -= 11644473600i64;

	if (li.QuadPart < 0)
		return 0;

	return (DWORD)li.QuadPart;
}

WORD	MTime::DaysInMonth(const WORD &wMonth)	const
{
	static const WORD wDaysInMonth[] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

	if (wMonth > 12) return 0;
	return (IsLeapYear() && wMonth == 2) ? wDaysInMonth[wMonth] + 1 : wDaysInMonth[wMonth];
}

WORD	MTime::DaysInYear(BYTE bIgnoreLeap)	const
{
	return ((!bIgnoreLeap && IsLeapYear()) ? 366 : 365); 
};

WORD	MTime::DayOfYear()	const
{
	WORD daysResult = 0;
	WORD i;

	for (i = 0; i < _SysTime.wMonth; i++)
		daysResult += DaysInMonth(i);
	daysResult += _SysTime.wDay;
	return daysResult;
}

WORD	MTime::AdjustYear(const int nDiffDays)
{
	const int nDay = DayOfYear() + nDiffDays;

	if (nDay > DaysInYear())
		return _SysTime.wYear + 1;
	else if (nDay < 0)
		return _SysTime.wYear - 1;
	return _SysTime.wYear;
}

WORD	MTime::TimeFormat(LPTSTR ptszTimeFormat, WORD cchTimeFormat)
{
	if (!ptszTimeFormat || !cchTimeFormat)
		return 0;
	if ((cchTimeFormat = GetTimeFormat(LOCALE_USER_DEFAULT, TIME_NOSECONDS, &_SysTime, NULL, ptszTimeFormat, cchTimeFormat)) == 0) {
		*ptszTimeFormat = 0;
		return 0;
	}
	return cchTimeFormat;
}

WORD	MTime::DateFormat(LPTSTR ptszTimeFormat, WORD cchTimeFormat)
{
	if (!ptszTimeFormat || !cchTimeFormat)
		return 0;
	if ((cchTimeFormat = GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &_SysTime, NULL, ptszTimeFormat, cchTimeFormat)) == 0) {
		*ptszTimeFormat = 0;
		return 0;
	}
	return cchTimeFormat;
}

WORD	MTime::DateFormatLong(LPTSTR ptszTimeFormat, WORD cchTimeFormat)
{
	if (!ptszTimeFormat || !cchTimeFormat)
		return 0;
	if ((cchTimeFormat = GetDateFormat(LOCALE_USER_DEFAULT, DATE_LONGDATE, &_SysTime, NULL, ptszTimeFormat, cchTimeFormat)) == 0) {
		*ptszTimeFormat = 0;
		return 0;
	}
	return cchTimeFormat;
}

/*********************************************
 * set class value
 *********************************************/

void	MTime::FromStampAsUTC(const DWORD dwTimeStamp)
{
	LARGE_INTEGER li;
	li.QuadPart = (dwTimeStamp + 11644473600i64) * 10000000i64;
	Set(li, FALSE);
}

void	MTime::FromStampAsLocal(const DWORD dwTimeStamp)
{
	FromStampAsUTC(dwTimeStamp);
	UTCToLocal();
}

void	MTime::Set(LARGE_INTEGER liFileTime, const BYTE bIsLocal)
{
	if (liFileTime.QuadPart < 0i64) liFileTime.QuadPart = 0;
	FileTimeToSystemTime((LPFILETIME)&liFileTime, &_SysTime);
	_isLocal = bIsLocal != FALSE;
}

void	MTime::Set(FILETIME &ftFileTime, const BYTE bIsLocal)
{
	FileTimeToSystemTime(&ftFileTime, &_SysTime);
	_isLocal = bIsLocal != FALSE;
}

void	MTime::Set(const MTime &mt)
{
	Set(mt.SystemTime(), mt.IsLocal());
}

void	MTime::Set(SYSTEMTIME &st, const BYTE bIsLocal)
{
	memcpy(&_SysTime, &st, sizeof(SYSTEMTIME));
	_isLocal = bIsLocal != FALSE;
}

void	MTime::GetTimeUTC()
{
	_isLocal = FALSE;
	::GetSystemTime(&_SysTime);
}

void	MTime::GetLocalTime()
{
	_isLocal = TRUE;
	::GetLocalTime(&_SysTime);
}

void	MTime::GetLocalTime(MCONTACT hContact)
{
	TIME_ZONE_INFORMATION tzi;

	GetTimeUTC();

	if (!GetContactTimeZoneInformation(hContact, (LPARAM)&tzi)) {
		UTCToTzSpecificLocal(&tzi);
	}
}

/*********************************************
 * read and write time to miranda's database
 *********************************************/

int MTime::DBGetStamp  (MCONTACT hContact, LPCSTR pszModule, LPCSTR pszSetting)
{
	if (hContact == INVALID_CONTACT_ID || pszModule == NULL || pszModule[0] == 0 || pszSetting == NULL || pszSetting[0] == 0) {
		ZeroDate();
		return 1;
	}

	DWORD dwTimeStamp = db_get_dw(hContact, pszModule, pszSetting, 0);
	if (dwTimeStamp == 0) {
		ZeroDate();
		return 1;
	}
	FromStampAsUTC(dwTimeStamp);
	_isLocal = FALSE;
	return 0;
}

int MTime::DBWriteStamp(MCONTACT hContact, LPCSTR pszModule, LPCSTR pszSetting)
{
	if (hContact == INVALID_CONTACT_ID || pszModule == NULL || pszModule[0] == 0 || pszSetting == NULL || pszSetting[0] == 0)
		return 1;

	return db_set_dw(hContact, pszModule, pszSetting, TimeStamp());
}
