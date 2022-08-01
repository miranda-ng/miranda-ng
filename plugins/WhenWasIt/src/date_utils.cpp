/*
WhenWasIt (birthday reminder) plugin for Miranda IM

Copyright © 2006 Cristian Libotean

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

time_t Today()
{
	time_t now = time(0);
	struct tm* date = localtime(&now);
	date->tm_hour = date->tm_min = date->tm_sec = 0;
	return mktime(date);
}

bool IsDOBValid(int, int month, int day)
{
	return (month != 0 && day != 0);
}

int GetContactDOB(MCONTACT hContact, int &year, int &month, int &day, int iModule)
{
	if (iModule != DOB_PROTOCOL) {
		year = db_get_w(hContact, "UserInfo", "BirthYear", 0);
		month = db_get_b(hContact, "UserInfo", "BirthMonth", 0);
		day = db_get_b(hContact, "UserInfo", "BirthDay", 0);
		if (IsDOBValid(year, month, day))
			return DOB_USERINFO;
		if (iModule == DOB_USERINFO)
			return DOB_UNKNOWN;
	}

	char *szProto = Proto_GetBaseAccountName(hContact);
	year = db_get_w(hContact, szProto, "BirthYear", 0);
	month = db_get_b(hContact, szProto, "BirthMonth", 0);
	day = db_get_b(hContact, szProto, "BirthDay", 0);
	if (IsDOBValid(year, month, day))
		return DOB_PROTOCOL;

	return DOB_UNKNOWN;
}

int GetContactAge(int year, int month, int day)
{
	if (year == 0)
		return 0;

	time_t now = Today();
	struct tm *today = gmtime(&now);
	int currentDay = today->tm_mday + 1;
	int currentMonth = today->tm_mon + 1;

	int age = (today->tm_year + 1900) - year;
	
	if (g_plugin.cShowAgeMode)
		if (month > currentMonth|| (month == currentMonth) && (day > currentDay)) // birthday still to come
			age--;

	return age;
}

int GetContactAge(MCONTACT hContact)
{
	int year, month, day;
	GetContactDOB(hContact, year, month, day);
	return GetContactAge(year, month, day);
}

char GetContactGender(MCONTACT hContact)
{
	char gender = db_get_b(hContact, "UserInfo", "Gender", 'U');
	if (gender == 'U')
		gender = db_get_b(hContact, Proto_GetBaseAccountName(hContact), "Gender", 'U');

	return gender;
}

bool IsLeapYear(int year)
{
	return ((year % 400 == 0) || ((year % 4 == 0) && (year % 100 != 0)));
}

unsigned int GetDaysDifference(time_t time1, time_t time2)
{
	errno = 0;
	double diff = difftime(time1, time2);
	if (errno == 0) {
		diff = diff / (60 * 60 * 24);
		int days = (int)floor(diff);
		if (days < 0) {
			struct tm *date = gmtime(&time1);
			int leap = 0;
			if ((date->tm_mon > 2) && (IsLeapYear(date->tm_year))) //if month > feb and it's a leap year
				leap = 1;

			days = 365 + days + leap;
		}
		return days;
	}
	return 0x7fffffff;
}

int GetDaysDifferenceAfter(time_t time1, time_t time2)
{
	errno = 0;
	double diff = difftime(time1, time2);
	if (errno == 0)
	{
		diff = diff / (60 * 60 * 24);
		int days = (int)floor(diff);
		if (days > 0)
		{
			return -1;
		}

		return -days;
	}

	return -1;
}

unsigned int DaysToBirthday(time_t now, int ctYear, int ctMonth, int ctDay)
{
	if (IsDOBValid(ctYear, ctMonth, ctDay)) {
		struct tm ct = { 0 };
		struct tm *tmp = gmtime(&now);
		ct.tm_year = tmp->tm_year;
		ct.tm_mon = ctMonth - 1;
		ct.tm_mday = ctDay;
		time_t ctBirthday = mktime(&ct);
		return GetDaysDifference(ctBirthday, now);
	}
	return -1;
}

int DaysAfterBirthday(time_t now, int ctYear, int ctMonth, int ctDay)
{
	if (IsDOBValid(ctYear, ctMonth, ctDay)) {
		struct tm ct = { 0 };
		struct tm *tmp = gmtime(&now);
		ct.tm_year = tmp->tm_year;
		ct.tm_mon = ctMonth - 1;
		ct.tm_mday = ctDay;
		time_t ctBirthday = mktime(&ct);

		return GetDaysDifferenceAfter(ctBirthday, now);
	}

	return -1;
}

void DeleteBirthday(MCONTACT hContact)
{
	db_unset(hContact, "UserInfo", "BirthYear");
	db_unset(hContact, "UserInfo", "BirthMonth");
	db_unset(hContact, "UserInfo", "BirthDay");
}

void SaveBirthday(MCONTACT hContact, int year, int month, int day, int mode)
{
	const char *szModule = (mode == DOB_PROTOCOL) ? Proto_GetBaseAccountName(hContact) : "UserInfo";

	db_set_dw(hContact, szModule, "BirthYear", year);
	db_set_b(hContact, szModule, "BirthMonth", month);
	db_set_b(hContact, szModule, "BirthDay", day);
}
