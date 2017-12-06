/*
Miranda ICQ: the free icq client for MS Windows 
Copyright (C) 2000-2  Richard Hughes, Roland Rabien & Tristan Van de Vreede

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

#include "stdafx.h"


static int daysInMonth[]={31,28,31,30,31,30,31,31,30,31,30,31};
static int IsLeapYear(int year)
{
	if (year&3) return 0;
	if (year%100) return 1;
	if (year%400) return 0;
	return 1;
}

static DWORD YMDHMSToTime(int year,int month,int day,int hour,int minute,int second)
{
	DWORD ret=0;
	int i;

	for(i=1970;i<year;i++) ret+=365+IsLeapYear(i);
	for(i=0;i<month-1;i++) ret+=daysInMonth[i];
	if (month>2 && IsLeapYear(year)) ret++;
	ret+=day-1;
	ret*=24*3600;
	return ret+3600*hour+60*minute+second;
}


static DWORD TimestampLocalToGMT(DWORD from)
{
	FILETIME ft1,ft2;
	LARGE_INTEGER liFiletime;

	//this huge number is the difference between 1970 and 1601 in seconds
	//liFiletime.QuadPart=(11644473600i64+(__int64)from)*10000000;
	__int64 t = 11644473600;				
	liFiletime.QuadPart=(t+(__int64)from)*10000000;
	ft1.dwHighDateTime=liFiletime.HighPart;
	ft1.dwLowDateTime=liFiletime.LowPart;
	LocalFileTimeToFileTime(&ft1,&ft2);
	liFiletime.HighPart=ft2.dwHighDateTime;
	liFiletime.LowPart=ft2.dwLowDateTime;
	return (DWORD)(liFiletime.QuadPart/10000000-t);
}

DWORD get_time()
{
	SYSTEMTIME stime;
	GetSystemTime(&stime);
	return YMDHMSToTime(stime.wYear,stime.wMonth,stime.wDay,stime.wHour,stime.wMinute,stime.wSecond);
}