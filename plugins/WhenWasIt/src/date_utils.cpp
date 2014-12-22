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

#include "commonheaders.h"

time_t Today()
{
	time_t now = time(NULL);
	struct tm* date = localtime(&now);
	date->tm_hour = date->tm_min = date->tm_sec = 0;
	return mktime(date);
}

bool IsDOBValid(int year, int month, int day)
{
	return (month != 0 && day != 0);
}

int GetContactDOB(MCONTACT hContact, int &year, int &month, int &day)
{
	year = db_get_w(hContact, "UserInfo", "DOBy", 0);
	month = db_get_b(hContact, "UserInfo", "DOBm", 0);
	day = db_get_b(hContact, "UserInfo", "DOBd", 0);
	if ( IsDOBValid(year, month, day))
		return DOB_USERINFO;

	char *szProto = GetContactProto(hContact);
	year = db_get_w(hContact, szProto, "BirthYear", 0);
	month = db_get_b(hContact, szProto, "BirthMonth", 0);
	day = db_get_b(hContact, szProto, "BirthDay", 0);
	if ( IsDOBValid(year, month, day))
		return DOB_PROTOCOL;

	year = db_get_w(hContact, "BirthDay", "BirthYear", 0);
	month = db_get_b(hContact, "BirthDay", "BirthMonth", 0);
	day = db_get_b(hContact, "BirthDay", "BirthDay", 0);
	if ( IsDOBValid(year, month, day))
		return DOB_BIRTHDAYREMINDER;

	year = db_get_w(hContact, "mBirthday", "BirthYear", 0);
	month = db_get_b(hContact, "mBirthday", "BirthMonth", 0);
	day = db_get_b(hContact, "mBirthday", "BirthDay", 0);
	if ( IsDOBValid(year, month, day))
		return DOB_MBIRTHDAY;

	year = db_get_dw(hContact, "micqBirthday", "BirthYear", 0);
	month = db_get_dw(hContact, "micqBirthday", "BirthMonth", 0);
	day = db_get_dw(hContact, "micqBirthday", "BirthDay", 0);
	if ( IsDOBValid(year, month, day))
		return DOB_MICQBIRTHDAY;

	return DOB_UNKNOWN;
}

int GetContactAge(MCONTACT hContact)
{
	int year, month, day;
	time_t tNow;
	time(&tNow);
	struct tm *now = localtime(&tNow);
	GetContactDOB(hContact, year, month, day);
	if (year == 0) 
		return 0; 
	else 
		return (now->tm_year + 1900) - year;
}

char GetContactGender(MCONTACT hContact)
{
	char gender = db_get_b(hContact, "UserInfo", "Gender", 'U');
	if (gender == 'U')
		gender = db_get_b(hContact, GetContactProto(hContact), "Gender", 'U');

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
		int days = (int) floor(diff);
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
		int days = (int) floor(diff);
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
	if ( IsDOBValid(ctYear, ctMonth, ctDay)) {
		struct tm ct = {0};
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
	if ( IsDOBValid(ctYear, ctMonth, ctDay)) {
		struct tm ct = {0};
		struct tm *tmp = gmtime(&now);
		ct.tm_year = tmp->tm_year;
		ct.tm_mon = ctMonth - 1;
		ct.tm_mday = ctDay;
		time_t ctBirthday = mktime(&ct);

		return GetDaysDifferenceAfter(ctBirthday, now);
	}

	return -1;
}

void FillProtocol(char *&sYear, char *&sMonth, char *&sDay)
{
	sYear = "BirthYear";
	sMonth = "BirthMonth";
	sDay = "BirthDay";
}

void FillmBirthday(char *&module, char *&sYear, char *&sMonth, char *&sDay)
{
	module = "mBirthday";
	sYear = "BirthYear";
	sMonth = "BirthMonth";
	sDay = "BirthDay";
}

void FillStandard(char *&module, char *&sYear, char *&sMonth, char *&sDay)
{
	module = "UserInfo";
	sYear = "DOBy";
	sMonth = "DOBm";
	sDay = "DOBd";
}

int SaveBirthday(MCONTACT hContact, int year, int month, int day, int mode)
{
	char *sModule, *sdModule, *sd2Module; //s* = keep, sd* = delete, sd2* = delete
	char *sYear, *sdYear, *sd2Year;
	char *sMonth, *sdMonth, *sd2Month;
	char *sDay, *sdDay, *sd2Day;
	char *protocol = GetContactProto(hContact);
	
	switch (mode) {
	case SAVE_MODE_MBIRTHDAY:
		FillmBirthday(sModule, sYear, sMonth, sDay);
		FillStandard(sdModule, sdYear, sdMonth, sdDay);
		sd2Module = protocol;
		FillProtocol(sd2Year, sd2Month, sd2Day);
		break;

	case SAVE_MODE_PROTOCOL:
		sModule = protocol;
		FillProtocol(sYear, sMonth, sDay);

		FillmBirthday(sd2Module, sd2Year, sd2Month, sd2Day);
		FillStandard(sdModule, sdYear, sdMonth, sdDay);
		break;

	case SAVE_MODE_STANDARD:
	default:
		FillStandard(sModule, sYear, sMonth, sDay);
		FillmBirthday(sdModule, sdYear, sdMonth, sdDay);
		sd2Module = protocol;
		FillProtocol(sd2Year, sd2Month, sd2Day);
		break;
	}

	if (mode == SAVE_MODE_DELETEALL) {
		db_unset(hContact, sModule, sYear);
		db_unset(hContact, sModule, sMonth);
		db_unset(hContact, sModule, sDay);
	}
	else {
		db_set_dw(hContact, sModule, sYear, year);
		db_set_b(hContact, sModule, sMonth, month);
		db_set_b(hContact, sModule, sDay, day);
	}

	db_unset(hContact, sdModule, sdYear);
	db_unset(hContact, sdModule, sdMonth);
	db_unset(hContact, sdModule, sdDay);
	
	db_unset(hContact, sd2Module, sd2Year);
	db_unset(hContact, sd2Module, sd2Month);
	db_unset(hContact, sd2Module, sd2Day);
	
	db_unset(hContact, "BirthDay", "BirthYear");
	db_unset(hContact, "BirthDay", "BirthMonth");
	db_unset(hContact, "BirthDay", "BirthDay");
	return 0;
}
