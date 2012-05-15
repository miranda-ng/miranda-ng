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

#include "date_utils.h"

time_t Today()
{
	time_t now = time(NULL);
	struct tm* date = localtime(&now);
	date->tm_hour = date->tm_min = date->tm_sec = 0;
	return mktime(date);
}

int IsDOBValid(int year, int month, int day)
{
	return ((year != 0) && (month != 0) && (day != 0));
}

int GetContactDOB(HANDLE hContact, int &year, int &month, int &day)
{
	int res = DOB_USERINFO;
	
	//TCHAR *contact = GetContactName(hContact, NULL);
	year = DBGetContactSettingWord(hContact, "UserInfo", "DOBy", 0);
	month = DBGetContactSettingByte(hContact, "UserInfo", "DOBm", 0);
	day = DBGetContactSettingByte(hContact, "UserInfo", "DOBd", 0);
	
	if (!IsDOBValid(year, month, day))
		{
			res = DOB_MBIRTHDAY;
			year = DBGetContactSettingWord(hContact, "mBirthday", "BirthYear", 0);
			month = DBGetContactSettingByte(hContact, "mBirthday", "BirthMonth", 0);
			day = DBGetContactSettingByte(hContact, "mBirthday", "BirthDay", 0);
		}
		
	if (!IsDOBValid(year, month, day))
		{
			res = DOB_PROTOCOL;
			char protocol[512];
			GetContactProtocol(hContact, protocol, sizeof(protocol));
			year = DBGetContactSettingWord(hContact, protocol, "BirthYear", 0);
			month = DBGetContactSettingByte(hContact, protocol, "BirthMonth", 0);
			day = DBGetContactSettingByte(hContact, protocol, "BirthDay", 0);
		}
		
	if (!IsDOBValid(year, month, day))
		{
			res = DOB_BIRTHDAYREMINDER;
			year = DBGetContactSettingWord(hContact, "BirthDay", "BirthYear", 0);
			month = DBGetContactSettingByte(hContact, "BirthDay", "BirthMonth", 0);
			day = DBGetContactSettingByte(hContact, "BirthDay", "BirthDay", 0);
		}
		
	if (!IsDOBValid(year, month, day))
		{
			res = DOB_MICQBIRTHDAY;
			year = DBGetContactSettingDword(hContact, "micqBirthday", "BirthYear", 0);
			month = DBGetContactSettingDword(hContact, "micqBirthday", "BirthMonth", 0);
			day = DBGetContactSettingDword(hContact, "micqBirthday", "BirthDay", 0);
		}

	if (!IsDOBValid(year, month, day))
		{
			res = DOB_UNKNOWN;
		}
	//free(contact);
	return res;
}

int GetContactAge(HANDLE hContact)
{
	int year, month, day;
	time_t tNow;
	time(&tNow);
	struct tm *now = localtime(&tNow);
	GetContactDOB(hContact, year, month, day);
	return (now->tm_year + 1900) - year;
}

char GetContactGender(HANDLE hContact)
{
	char gender = DBGetContactSettingByte(hContact, "UserInfo", "Gender", 'U');
	if (gender == 'U')
		{
			char protocol[512];
			GetContactProtocol(hContact, protocol, sizeof(protocol));
			
			gender = DBGetContactSettingByte(hContact, protocol, "Gender", 'U');
		}
	return gender;
}

int IsLeapYear(int year)
{
	int yes = ((year % 400 == 0) || ((year % 4 == 0) && (year % 100 != 0)));
	return yes;
}

unsigned int GetDaysDifference(time_t time1, time_t time2)
{
	errno = 0;
	double diff = difftime(time1, time2);
	if (errno == 0)
		{
			diff = diff / (60 * 60 * 24);
			int days = (int) floor(diff);
			if (days < 0)
				{
					struct tm *date = gmtime(&time1);
					int leap = 0;
					if ((date->tm_mon > 2) && (IsLeapYear(date->tm_year))) //if month > feb and it's a leap year
						{
							leap = 1;
						}
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
	if (IsDOBValid(ctYear, ctMonth, ctDay))
		{
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
	if (IsDOBValid(ctYear, ctMonth, ctDay))
	{
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

int SaveBirthday(HANDLE hContact, int year, int month, int day, int mode)
{
	char *sModule, *sdModule, *sd2Module; //s* = keep, sd* = delete, sd2* = delete
	char *sYear, *sdYear, *sd2Year;
	char *sMonth, *sdMonth, *sd2Month;
	char *sDay, *sdDay, *sd2Day;
	char protocol[256];
	GetContactProtocol(hContact, protocol, sizeof(protocol));
	
	switch (mode)
		{
			case SAVE_MODE_MBIRTHDAY:
				{
					FillmBirthday(sModule, sYear, sMonth, sDay);
					FillStandard(sdModule, sdYear, sdMonth, sdDay);
					sd2Module = protocol;
					FillProtocol(sd2Year, sd2Month, sd2Day);
					
					break;
				}
				
			case SAVE_MODE_PROTOCOL:
				{
					sModule = protocol;
					FillProtocol(sYear, sMonth, sDay);
				
					FillmBirthday(sd2Module, sd2Year, sd2Month, sd2Day);
					FillStandard(sdModule, sdYear, sdMonth, sdDay);
				
					break;
				}
			
			case SAVE_MODE_STANDARD:
			default:
				{
					FillStandard(sModule, sYear, sMonth, sDay);
					FillmBirthday(sdModule, sdYear, sdMonth, sdDay);
					sd2Module = protocol;
					FillProtocol(sd2Year, sd2Month, sd2Day);
					break;
				}
		}
		
	if (mode == SAVE_MODE_DELETEALL)
		{
			DBDeleteContactSetting(hContact, sModule, sYear);
			DBDeleteContactSetting(hContact, sModule, sMonth);
			DBDeleteContactSetting(hContact, sModule, sDay);
		}
		else{
			DBWriteContactSettingWord(hContact, sModule, sYear, year);
			DBWriteContactSettingByte(hContact, sModule, sMonth, month);
			DBWriteContactSettingByte(hContact, sModule, sDay, day);
		}
	
	DBDeleteContactSetting(hContact, sdModule, sdYear);
	DBDeleteContactSetting(hContact, sdModule, sdMonth);
	DBDeleteContactSetting(hContact, sdModule, sdDay);
	
	DBDeleteContactSetting(hContact, sd2Module, sd2Year);
	DBDeleteContactSetting(hContact, sd2Module, sd2Month);
	DBDeleteContactSetting(hContact, sd2Module, sd2Day);
	
	DBDeleteContactSetting(hContact, "BirthDay", "BirthYear");
	DBDeleteContactSetting(hContact, "BirthDay", "BirthMonth");
	DBDeleteContactSetting(hContact, "BirthDay", "BirthDay");

	return 0;
}