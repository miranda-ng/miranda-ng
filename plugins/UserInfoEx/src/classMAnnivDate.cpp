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

===============================================================================

File name      : $HeadURL: https://userinfoex.googlecode.com/svn/trunk/classMAnnivDate.cpp $
Revision       : $Revision: 187 $
Last change on : $Date: 2010-09-08 16:05:54 +0400 (Ð¡Ñ€, 08 ÑÐµÐ½ 2010) $
Last change by : $Author: ing.u.horn $

===============================================================================
*/

/**
 * System Includes:
 **/
#include "commonheaders.h"
#include "svc_Reminder.h"

/**
 * name:	MAnnivDate
 * class:	MAnnivDate
 * desc:	default constructor
 * param:	none
 * return:	nothing
 **/ 
MAnnivDate::MAnnivDate()
{
	Clear();
}

/**
 * name:	MAnnivDate
 * class:	MAnnivDate
 * desc:	constructor, which duplicates an existing anniversary date
 * param:	mda	- anniversary date to copy
 * return:	nothing
 **/ 
MAnnivDate::MAnnivDate(MAnnivDate &mda)
{
	SetDate(mda);
}

/**
 * name:	Clear
 * class:	MAnnivDate
 * desc:	set all attributes to default value
 * param:	none
 * return:	nothing
 **/ 
VOID MAnnivDate::Clear()
{
	ZeroDate();
	_wID = ANID_NONE;
	_strDesc.clear();
	_strModule.clear();
	_wFlags = MADF_NONE;
	_bRemind = BST_INDETERMINATE;
	_wDaysEarlier = (WORD)-1;
}

/**
 * name:	Set
 * class:	MAnnivDate
 * desc:	set new date
 * param:	none
 * return:	nothing
 **/ 
VOID MAnnivDate::SetDate(SYSTEMTIME &st)
{
	ZeroDate();
	Year(st.wYear);
	Month(st.wMonth);
	Day(st.wDay);
}

/**
 * name:	Set
 * class:	MAnnivDate
 * desc:	duplicates the given anniversary date class
 * param:	none
 * return:	nothing
 **/ 
VOID MAnnivDate::SetDate(MAnnivDate &mda)
{
	SetDate(mda.SystemTime());
	_wID = mda.Id();
	_strDesc = mda.Description();
	_strModule = mda.Module();
	_wFlags = mda.Flags();
	_bRemind = mda.RemindOption();
	_wDaysEarlier = mda.RemindOffset();
}

/**
 * name:	IsValid
 * class:	MAnnivDate
 * desc:	compare the current date with the given one in st
 * param:	st	- SYSTEMTIME to compare with
 * return:	number of days the st differs from the class value
 **/ 
__inline BOOLEAN MAnnivDate::IsValid() const
{
	return (
		Year() > 1600 &&
		Month() > 0 && Month() < 13 &&
		Day() > 0 && Day() <= DaysInMonth(Month())
 );
}

/**
 * name:	CompareDays
 * class:	MAnnivDate
 * desc:	compare the current date with the given one in st
 * param:	mt	- MTime to compare with
 * return:	number of days the mt differs from the class value
 **/ 
INT MAnnivDate::CompareDays(MTime mt) const
{
	mt.Year(Year());
	return DayOfYear() - mt.DayOfYear();
}

/**
 * name:	Compare
 * class:	MAnnivDate
 * desc:	compare the current date with the given one in st
 * param:	st	- SYSTEMTIME to compare with
 * return:	number of days the st differs from the class value
 **/
BOOLEAN MAnnivDate::IsEqual(const SYSTEMTIME &st) const
{
	return (
		Day() == st.wDay &&
		Month() == st.wMonth &&
		Year() == st.wYear
 );
}

/**
 * name:	DateStamp
 * class:	MAnnivDate
 * desc:	return the current date encoded as an DWORD
 * param:	nothing
 * return:	DWORD encoded date
 **/
DWORD MAnnivDate::DateStamp() const
{
	DWORD dwStamp;

	if (!IsValid()) return 0;

	dwStamp	= (Day() << 24) & 0xFF000000;
	dwStamp |= (Month() << 16) & 0x00FF0000;
	dwStamp |= Year() & 0x0000FFFF;
	return dwStamp;
}

/**
 * name:	DateStamp
 * class:	MAnnivDate
 * desc:	set the date according to a datestamp
 * param:	dwStamp	- the dword encoded date
 * return:	nothing
 **/
VOID MAnnivDate::DateStamp(const DWORD dwStamp)
{
	Day((const WORD)((dwStamp & 0xFF000000) >> 24));
	Month((const WORD)((dwStamp & 0x00FF0000) >> 16));
	Year((const WORD)(dwStamp & 0x0000FFFF));
}

/**
 * name:	Age
 * class:	MAnnivDate
 * desc:	calculates the age according to the date of the class and current date
 * param:	pNow	- optional pointer to a MTime class to specify a certain time to use for age calculation
 * return:	number of years the anniversary differs from now
 **/ 
INT MAnnivDate::Age(MTime *pNow)
{
	INT age = 0;
	MTime now;

	if (!IsValid()) return -1;

	if (pNow) now = *pNow;
	else now.GetLocalTime();

	age = now.Year() - Year();
	if (age > 1 && CompareDays(now) > 0)
		age--;
	return age;
}

/**
 * name:	Zodiac
 * class:	MAnnivDate
 * desc:	returns the zodiac icon and text for the date
 * param:	none
 * return:	structure, holding zodiac information
 **/ 
MZodiac MAnnivDate::Zodiac()
{
	static const struct TZodiac {
		const WORD startDays;
		const WORD endDays;
		LPCTSTR szZodiac;
		LPCSTR szZodiacIcon;
	} zodiac[] = {
		{ 80,	110,	LPGENT("Aries")			, ICO_ZOD_ARIES			},	// Widder
		{ 111,	140,	LPGENT("Taurus")		, ICO_ZOD_TAURUS		},	// Stier
		{ 141,	172,	LPGENT("Gemini")		, ICO_ZOD_GEMINI		},	// Zwillinge
		{ 173,	203,	LPGENT("Cancer")		, ICO_ZOD_CANCER		},	// Krebs
		{ 204,	235,	LPGENT("Leo")			, ICO_ZOD_LEO			},	// Löwe
		{ 236,	266,	LPGENT("Virgo")			, ICO_ZOD_VIRGO			},	// Jungfrau
		{ 267,	296,	LPGENT("Libra")			, ICO_ZOD_LIBRA			},	// Waage
		{ 297,	326,	LPGENT("Scorpio")		, ICO_ZOD_SCORPIO		},	// Scorpion
		{ 327,	355,	LPGENT("Sagittarius")	, ICO_ZOD_SAGITTARIUS	},	// Schütze
		{ 356,	364,	LPGENT("Capricorn")		, ICO_ZOD_CAPRICORN		},	// Steinbock
		{ 1,	19,		LPGENT("Capricorn")		, ICO_ZOD_CAPRICORN		},	// Steinbock
		{ 20,	49,		LPGENT("Aquarius")		, ICO_ZOD_AQUARIUS		},	// Wassermann
		{ 50,	79,		LPGENT("Pisces")		, ICO_ZOD_PISCES		},	// Fische
		//{ 0,	0,		LPGENT("Unknown")		, ICO_ZOD_UNKNOWN		},	// not found
		{ 0,	0,		NULL						, ""					}	// end of array
	};
	const WORD wDays = DayOfYear();
	BYTE i;
	MZodiac mZodiac;

	for (i = 0; i < 13 && (wDays < zodiac[i].startDays || wDays > zodiac[i].endDays); i++);

	mZodiac.hIcon = IcoLib_GetIcon(zodiac[i].szZodiacIcon);
	mZodiac.pszName = zodiac[i].szZodiac;
	return mZodiac;
}

/***********************************************************************************************************
 * reading and writing options
 ***********************************************************************************************************/

/**
 * name:	DBGetReminderOpts
 * class:	MAnnivDate
 * desc:	read reminder options for previously read date from database
 * param:	hContact	- handle to a contact to read the date from
 * return:	0 on success, 1 otherwise
 **/
INT MAnnivDate::DBGetReminderOpts(HANDLE hContact)
{
	if (!hContact || hContact == INVALID_HANDLE_VALUE)
		return 1;
	if (_wID == ANID_BIRTHDAY) {
		_bRemind = DB::Setting::GetByte(hContact, USERINFO, SET_REMIND_BIRTHDAY_ENABLED, BST_INDETERMINATE);
		_wDaysEarlier = DB::Setting::GetWord(hContact, USERINFO, SET_REMIND_BIRTHDAY_OFFSET, (WORD)-1);
	}
	else if (_wID <= ANID_LAST) {
		CHAR pszSetting[MAXSETTING];

		// read reminder option
		mir_snprintf(pszSetting, MAXSETTING, "Anniv%dReminder", _wID);
		_bRemind = DB::Setting::GetByte(hContact, Module(), pszSetting, BST_INDETERMINATE);
		// read offset
		mir_snprintf(pszSetting, MAXSETTING, "Anniv%dOffset", _wID);
		_wDaysEarlier = DB::Setting::GetWord(hContact, Module(), pszSetting, (WORD)-1);
	}
	else {
		_bRemind = BST_INDETERMINATE;
		_wDaysEarlier = (WORD)-1;
	}
	return 0;
}

/**
 * name:	DBWriteReminderOpts
 * class:	MAnnivDate
 * desc:	write reminder options for date to database
 * param:	hContact	- handle to a contact to read the date from
 * return:	0 on success, 1 otherwise
 **/
INT MAnnivDate::DBWriteReminderOpts(HANDLE hContact)
{
	if (!hContact || hContact == INVALID_HANDLE_VALUE)
		return 1;
	if (_wID == ANID_BIRTHDAY) {
		if (_bRemind == BST_INDETERMINATE) DB::Setting::Delete(hContact, USERINFO, SET_REMIND_BIRTHDAY_ENABLED);
		else DB::Setting::WriteByte(hContact, USERINFO, SET_REMIND_BIRTHDAY_ENABLED, _bRemind);

		if (_wDaysEarlier == (WORD)-1) DB::Setting::Delete(hContact, USERINFO, SET_REMIND_BIRTHDAY_OFFSET);
		else DB::Setting::WriteWord(hContact, USERINFO, SET_REMIND_BIRTHDAY_OFFSET, _wDaysEarlier);
	}
	else if (_wID <= ANID_LAST) {
		CHAR pszSetting[MAXSETTING];
		// read reminder option
		mir_snprintf(pszSetting, MAXSETTING, "Anniv%dReminder", _wID);
		if (_bRemind == BST_INDETERMINATE) DB::Setting::Delete(hContact, USERINFO, pszSetting);
		else DB::Setting::WriteByte(hContact, USERINFO, pszSetting, _bRemind);
		
		// read offset
		mir_snprintf(pszSetting, MAXSETTING, "Anniv%dOffset", _wID);
		if (_wDaysEarlier == (WORD)-1) DB::Setting::Delete(hContact, USERINFO, pszSetting);
		else DB::Setting::WriteWord(hContact, USERINFO, pszSetting, _wDaysEarlier);
	}
	return 0;
}

/***********************************************************************************************************
 * reading, writing and deleting general date
 ***********************************************************************************************************/

/**
 * name:	DBGetDate
 * class:	MAnnivDate
 * desc:	read a certain date from database
 * param:	hContact	- handle to a contact to read the date from
 *			pszModule	- module holding the date
 *			szDay		- setting of the day to read
 *			szMonth		- setting of the month to read 
 *			szYear		- setting of the year to read
 * return:	0 on success, 1 otherwise
 **/ 
INT MAnnivDate::DBGetDate(HANDLE hContact, LPCSTR pszModule, LPCSTR szDay, LPCSTR szMonth, LPCSTR szYear)
{
	WORD wtmp;

	ZeroDate();

	wtmp = DB::Setting::GetWord(hContact, pszModule, szYear, 0);
	if (wtmp < 1601) return 1;
	Year(wtmp);

	wtmp = DB::Setting::GetWord(hContact, pszModule, szMonth, 0);
	if (wtmp > 0 && wtmp < 13) {
		Month(wtmp);

		wtmp = DB::Setting::GetWord(hContact, pszModule, szDay, 0);
		if (wtmp > 0 && wtmp <= DaysInMonth(Month())) {
			Day(wtmp);
			// date was correctly read from db
			_strModule = pszModule;
			return 0;
		}
	}
	ZeroDate();
	return 1;
}

/**
 * name:	DBWriteDate
 * class:	MAnnivDate
 * desc:	write a certain date from database
 * param:	hContact	- handle to a contact to read the date from
 *			pszModule	- module holding the date
 *			szDay		- setting of the day to read
 *			szMonth		- setting of the month to read 
 *			szYear		- setting of the year to read
 * return:	0 on success, 1 otherwise
 **/ 
INT MAnnivDate::DBWriteDate(HANDLE hContact, LPCSTR pszModule, LPCSTR szDay, LPCSTR szMonth, LPCSTR szYear)
{
	return (
		DB::Setting::WriteByte(hContact, pszModule, szDay, (BYTE)Day()) ||
		DB::Setting::WriteByte(hContact, pszModule, szMonth, (BYTE)Month()) ||
		DB::Setting::WriteWord(hContact, pszModule, szYear, Year())
);
}

/**
 * name:	DBDeleteDate
 * class:	MAnnivDate
 * desc:	delete a certain date from database
 * param:	hContact	- handle to a contact to read the date from
 *			pszModule	- module holding the date
 *			szDay		- setting of the day to read
 *			szMonth		- setting of the month to read 
 *			szYear		- setting of the year to read
 * return:	0 on success, 1 otherwise
 **/ 
INT MAnnivDate::DBDeleteDate(HANDLE hContact, LPCSTR pszModule, LPCSTR szDay, LPCSTR szMonth, LPCSTR szYear) const
{
	INT ret;

	ret = DB::Setting::Delete(hContact, pszModule, szDay);
	ret &= DB::Setting::Delete(hContact, pszModule, szMonth);
	ret &= DB::Setting::Delete(hContact, pszModule, szYear);
	return ret;
}

/***********************************************************************************************************
 * reading, writing and deleting general datestamp
 ***********************************************************************************************************/

/**
 * name:	DBGetDateStamp
 * class:	MAnnivDate
 * desc:	Read a datestamp from database. A datestamp is an DWORD of the form <ddmmyyyy>.
 * param:	hContact		- handle to a contact to read the datestamp from
 *			pszModule		- module to read the datestamp from
 *			pszSetting		- key used to identify the datestamp
 * return:	0 on success, 1 otherwise
 **/ 
INT MAnnivDate::DBGetDateStamp(HANDLE hContact, LPCSTR pszModule, LPCSTR pszSetting) 
{
	DBVARIANT dbv;

	if (DB::Setting::GetAsIs(hContact, pszModule, pszSetting, &dbv))
		return 1;
	if (dbv.type != DBVT_DWORD) {
		DB::Variant::Free(&dbv);
		return 1;
	}
	DateStamp(dbv.dVal);
	return IsValid() == 0;
}

/**
 * name:	DBWriteDateStamp
 * class:	MAnnivDate
 * desc:	Write a datestamp to database. A datestamp is an DWORD of the form <ddmmyyyy>.
 * param:	hContact		- handle to a contact to write the datestamp to
 *			pszModule	 - module to write the datestamp to
 *			pszSetting	- key used to save the datestamp
 * return:	0 on success, 1 otherwise
 **/ 
INT MAnnivDate::DBWriteDateStamp(HANDLE hContact, LPCSTR pszModule, LPCSTR pszSetting) 
{
	DWORD dwStamp = DateStamp();

	if (hContact	 == INVALID_HANDLE_VALUE	||
			pszModule	== 0 || *pszModule	== 0 ||
			pszSetting == 0 || *pszSetting == 0 ||
			dwStamp == 0)
	{
		return 1;
	}
	return DB::Setting::WriteDWord(hContact, pszModule, pszSetting, dwStamp);
}

/***********************************************************************************************************
 * reading, writing and deleting birthday
 ***********************************************************************************************************/

/**
 * name:	DBGetBirthDate
 * class:	MAnnivDate
 * desc:	try to read birthday date from all known modules
 * param:	hContact		- handle to a contact to read the date from
 *			pszProto		- basic protocol module
 * return:	0 on success, 1 otherwise
 **/ 
INT MAnnivDate::DBGetBirthDate(HANDLE hContact, LPSTR pszProto)
{
	Clear();

	// try to get birthday from any custom module 
	if (	!DBGetDate(hContact, USERINFO, SET_CONTACT_BIRTHDAY, SET_CONTACT_BIRTHMONTH, SET_CONTACT_BIRTHYEAR) ||
			!DBGetDate(hContact, MOD_MBIRTHDAY, SET_CONTACT_BIRTHDAY, SET_CONTACT_BIRTHMONTH, SET_CONTACT_BIRTHYEAR) ||
			!DBGetDate(hContact, SET_CONTACT_BIRTHDAY, SET_CONTACT_BIRTHDAY, SET_CONTACT_BIRTHMONTH, SET_CONTACT_BIRTHYEAR) ||
			!DBGetDate(hContact, USERINFO, SET_CONTACT_DOBD, SET_CONTACT_DOBM, SET_CONTACT_DOBY))
	{
		SetFlags(MADF_HASCUSTOM);
	}
	// if pszProto is set to NULL, this will be scaned only incase the birthday date has not been found yet
	else if (pszProto || (pszProto = DB::Contact::Proto(hContact)) != NULL)
	{
		// try to get birthday from basic protocol
		if (!DBGetDate(hContact, pszProto, SET_CONTACT_BIRTHDAY, SET_CONTACT_BIRTHMONTH, SET_CONTACT_BIRTHYEAR)) 
		{
			SetFlags(MADF_HASPROTO);
		}
		// try to get birthday date from metacontact's subcontact
		else if (DB::Module::IsMetaAndScan(pszProto))
		{
			const INT def = DB::MetaContact::SubDefNum(hContact);
			HANDLE hSubContact;

			// try to get setting from the default subcontact first
			if (def > -1 && def < INT_MAX) 
			{
				hSubContact = DB::MetaContact::Sub(hContact, def);
				if (hSubContact != NULL && !DBGetBirthDate(hSubContact, NULL)) 
				{
					RemoveFlags(MADF_HASCUSTOM);
					SetFlags(MADF_HASMETA);
				}
			}

			// scan all subcontacts for the setting
			if (_wFlags == 0)
			{
				const INT cnt = DB::MetaContact::SubCount(hContact);

				if (cnt < INT_MAX) 
				{
					INT i;
					for (i = 0; i < cnt; i++) 
					{
						if (i != def)
						{
							hSubContact = DB::MetaContact::Sub(hContact, i);
							if (hSubContact != NULL && !DBGetBirthDate(hSubContact, NULL)) 
							{
								RemoveFlags(MADF_HASCUSTOM);
								SetFlags(MADF_HASMETA);
								break;
		}	}	}	}	}	}	
	}

	if (_wFlags != 0) 
	{
		_wID = ANID_BIRTHDAY;
		_strDesc = TranslateT("Birthday");
		return 0;
	}
	return 1;
}

/**
 * name:	DBMoveBirthDate
 * class:	MAnnivDate
 * desc:	keep the database clean 
 * param:	hContact		- handle to a contact to read the date from
 *			bOld			- byte RemindBirthModule src
 *			bNew			- byte RemindBirthModule dest
 * return:	0 on success, 1 otherwise
 **/ 
// 
INT MAnnivDate::DBMoveBirthDate(HANDLE hContact, BYTE bOld, BYTE bNew) {
	Clear();
	switch(bOld) {
		case 0:		//MOD_MBIRTHDAY
			if (!DBGetDate(hContact, MOD_MBIRTHDAY, SET_CONTACT_BIRTHDAY, SET_CONTACT_BIRTHMONTH, SET_CONTACT_BIRTHYEAR)) {
				if(DBWriteDate(hContact, USERINFO, SET_CONTACT_BIRTHDAY, SET_CONTACT_BIRTHMONTH, SET_CONTACT_BIRTHYEAR))
					return 1;
				DBDeleteDate(hContact, MOD_MBIRTHDAY, SET_CONTACT_BIRTHDAY, SET_CONTACT_BIRTHMONTH, SET_CONTACT_BIRTHYEAR);
				DB::Setting::Delete(hContact, MOD_MBIRTHDAY, "BirthMode");
				}
			break;
		case 1:		//USERINFO
			if (!DBGetDate(hContact, USERINFO, SET_CONTACT_BIRTHDAY, SET_CONTACT_BIRTHMONTH, SET_CONTACT_BIRTHYEAR)) {
				if(DBWriteDate(hContact, MOD_MBIRTHDAY, SET_CONTACT_BIRTHDAY, SET_CONTACT_BIRTHMONTH, SET_CONTACT_BIRTHYEAR))
					return 1;
				DB::Setting::WriteByte(hContact, MOD_MBIRTHDAY, "BirthMode", 2);
				DBDeleteDate(hContact, USERINFO, SET_CONTACT_BIRTHDAY, SET_CONTACT_BIRTHMONTH, SET_CONTACT_BIRTHYEAR);
			}
			break;
		default:
			return 1;
			break;
		}
	return 0;
}

/**
 * name:	DBWriteBirthDate
 * class:	MAnnivDate
 * desc:	write birthday date to desired module
 * param:	hContact		- handle to a contact to read the date from
 *			pszProto		- basic protocol module
 * return:	0 on success, 1 otherwise
 **/ 
INT MAnnivDate::DBWriteBirthDate(HANDLE hContact)
{
	INT rc = 0;
	LPCSTR pszModule = SvcReminderGetMyBirthdayModule();

	rc = DBWriteDate(hContact, pszModule, SET_CONTACT_BIRTHDAY, SET_CONTACT_BIRTHMONTH, SET_CONTACT_BIRTHYEAR);
	if (!rc)
	{
		if (!mir_strcmp(pszModule, MOD_MBIRTHDAY))
		{
			DB::Setting::WriteByte(hContact, MOD_MBIRTHDAY, "BirthMode", 2);
		}

		if (
				// only delete values from current contact's custom modules
				!(_wFlags & (MADF_HASPROTO|MADF_HASMETA)) &&
				// check whether user wants this feature
				DB::Setting::GetByte(SET_REMIND_SECUREBIRTHDAY, TRUE) &&
				!myGlobals.UseDbxTree)
		{
			// keep the database clean
			
			if (mir_strcmp(pszModule, MOD_MBIRTHDAY)!= 0)
			{
				DBDeleteDate(hContact, MOD_MBIRTHDAY, SET_CONTACT_BIRTHDAY, SET_CONTACT_BIRTHMONTH, SET_CONTACT_BIRTHYEAR);
				DB::Setting::Delete(hContact, MOD_MBIRTHDAY, "BirthMode");
			}
			else if (mir_strcmp(pszModule, USERINFO) !=0)
			{
				DBDeleteDate(hContact, USERINFO, SET_CONTACT_BIRTHDAY, SET_CONTACT_BIRTHMONTH, SET_CONTACT_BIRTHYEAR);
			}
			DBDeleteDate(hContact, SET_CONTACT_BIRTHDAY, SET_CONTACT_BIRTHDAY, SET_CONTACT_BIRTHMONTH, SET_CONTACT_BIRTHYEAR);
			DBDeleteDate(hContact, USERINFO, SET_CONTACT_DOBD, SET_CONTACT_DOBM, SET_CONTACT_DOBY);
		}

		rc = DB::Setting::WriteWord(hContact, USERINFO, SET_CONTACT_AGE, Age());
	}
	return rc;
}

/**
 * name:	DBDeleteBirthDate
 * class:	MAnnivDate
 * desc:	delete birthday date from desired module
 * param:	hContact		- handle to a contact to read the date from
 *			pszProto		- basic protocol module
 * return:	0 on success, 1 otherwise
 **/ 
INT MAnnivDate::DBDeleteBirthDate(HANDLE hContact)
{
	return DBDeleteDate(hContact, Module(), SET_CONTACT_BIRTHDAY, SET_CONTACT_BIRTHMONTH, SET_CONTACT_BIRTHYEAR);
}

/***********************************************************************************************************
 * reading, writing and deleting anniversary
 ***********************************************************************************************************/

/**
 * name:	DBGetAnniversaryDate
 * class:	MAnnivDate
 * desc:	try to read anniversary date from userinfo module
 * param:	hContact		- handle to a contact to read the date from
 *			pszProto		- basic protocol module
 * return: 0 on success, 1 otherwise
 **/ 
INT MAnnivDate::DBGetAnniversaryDate(HANDLE hContact, WORD iIndex)
{
	CHAR szStamp[MAXSETTING];
	DBVARIANT dbv;
	INT rc;

	Clear();

	// read date and convert older versions
	mir_snprintf(szStamp, SIZEOF(szStamp), "Anniv%dDate", iIndex);
	rc = DBGetDateStamp(hContact, USERINFO, szStamp);
	if (!rc)
	{
		_strModule = USERINFO;
		_wFlags |= MADF_HASCUSTOM;
		_wID = iIndex;
		
		// read description
		mir_snprintf(szStamp, SIZEOF(szStamp), "Anniv%dDesc", iIndex);
		if (!DB::Setting::GetTString(hContact, USERINFO, szStamp, &dbv)) 
		{
			_strDesc = dbv.ptszVal;
			DB::Variant::Free(&dbv);
		}
	}
	return rc;
}

/**
 * name:	DBWriteAnniversaryDate
 * class:	MAnnivDate
 * desc:	write birthday date to desired module
 * param:	hContact		- handle to a contact to read the date from
 *			pszProto		- basic protocol module
 * return:	0 on success, 1 otherwise
 **/ 
INT MAnnivDate::DBWriteAnniversaryDate(HANDLE hContact, WORD wIndex)
{
	INT ret = 0;

	// date can only be written to db as anniversary if it is not marked as birthday
	if (wIndex <= ANID_LAST && _wID != ANID_BIRTHDAY) 
	{
		CHAR	pszSetting[MAXSETTING];

		_wID = wIndex;
		
		mir_snprintf(pszSetting, SIZEOF(pszSetting), "Anniv%dDate", wIndex);
		if (!DBWriteDateStamp(hContact, USERINFO, pszSetting)) 
		{
			// write description
			mir_snprintf(pszSetting, SIZEOF(pszSetting), "Anniv%dDesc", wIndex);
			DB::Setting::WriteTString(hContact, USERINFO, pszSetting, (LPTSTR)Description());
			return 0;
		}
		// delete date if written incompletely
		DB::Setting::Delete(hContact, USERINFO, pszSetting);
	}
	return 1;
}

/***********************************************************************************************************
 * automatic backup service
 ***********************************************************************************************************/

static WORD AskUser(HANDLE hContact, MAnnivDate *pOldCustomDate, MAnnivDate *pNewProtoDate)
{
	MSGBOX	MB;
	TCHAR	 szMsg[MAXDATASIZE];
	TCHAR	 szDate[MAX_PATH];
	TCHAR	 szoldDate[MAX_PATH];

	pOldCustomDate->DateFormat(szoldDate, SIZEOF(szoldDate));
	pNewProtoDate->DateFormat(szDate, SIZEOF(szDate));
	
	mir_sntprintf(szMsg, SIZEOF(szMsg),
		TranslateT("%s provides a new birthday via protocol.\nIt is %s. The old one was %s.\n\nDo you want to use this as the new birthday for this contact?"), 
		DB::Contact::DisplayName(hContact), szDate, szoldDate
 );

	MB.cbSize = sizeof(MSGBOX);
	MB.hParent = NULL;
	MB.hiLogo = IcoLib_GetIcon(ICO_DLG_ANNIVERSARY);
	MB.hiMsg = NULL;
	MB.uType = MB_YESALLNO|MB_ICON_QUESTION|MB_INFOBAR|MB_NOPOPUP;
	MB.ptszTitle = LPGENT("Update custom birthday");
	MB.ptszInfoText = LPGENT("Keeps your custom birthday up to date.");
	MB.ptszMsg = szMsg;
	return MsgBoxService(NULL, (LPARAM)&MB);
}

/**
 * name:	BackupBirthday
 * class:	MAnnivDate
 * desc:	tries to read birthday date from protocol and compares it with the classes date
 * param:	hContact		- handle to a contact to read the date from
 *			pszProto		- basic protocol module
 * return:	0 if backup was done, 1 otherwise
 **/ 
INT MAnnivDate::BackupBirthday(HANDLE hContact, LPSTR pszProto, const BOOLEAN bDontIgnoreAnything, PWORD lastAnswer)
{
	if (hContact)
	{
		// This birthday is a protocol based or metasubcontact's anniversary and no custom information exist,
		// so directly back it up under all circumstances!
		if ((_wFlags & MADF_HASPROTO) || (_wFlags & MADF_HASMETA))
		{
			DBWriteDateStamp(hContact, USERINFO, SET_REMIND_BIRTHDAY_IGNORED);
			DBWriteBirthDate(hContact);
		}
		// A custom birthday was set by user before and is not to be ignored
		else if ((_wFlags & MADF_HASCUSTOM) && (bDontIgnoreAnything || !lastAnswer || (*lastAnswer != IDNONE)))
		{
			if (!pszProto)
			{
				pszProto = DB::Contact::Proto(hContact);
			}
			if (pszProto)
			{
				BOOLEAN bIsMeta = DB::Module::IsMeta(pszProto);
				BOOLEAN bIsMetaSub = !bIsMeta && DB::MetaContact::IsSub(hContact);
				BOOLEAN bWantBackup = FALSE;
				MAnnivDate mdbNewProto;
				MAnnivDate mdbIgnore;
				HANDLE hSubContact;

				const INT nSubContactCount = (bIsMeta) ? DB::MetaContact::SubCount(hContact) : 0;
				
				bWantBackup = !mdbNewProto.DBGetDate(hContact, pszProto, SET_CONTACT_BIRTHDAY, SET_CONTACT_BIRTHMONTH, SET_CONTACT_BIRTHYEAR) 
									 && !IsEqual(mdbNewProto.SystemTime())
									 && (bDontIgnoreAnything || (DB::Setting::GetDWord(hContact, USERINFO, SET_REMIND_BIRTHDAY_IGNORED, 0) != mdbNewProto.DateStamp()))
									 && !bIsMetaSub;

				// allow backup only, if the custom setting differs from all meta subcontacts' protocol based settings, too.
				for (INT i = 0; (i < nSubContactCount) && bWantBackup && bIsMeta; i++)
				{
					hSubContact = DB::MetaContact::Sub(hContact, i);
					if (hSubContact && !mdbIgnore.DBGetDate(hSubContact, pszProto, SET_CONTACT_BIRTHDAY, SET_CONTACT_BIRTHMONTH, SET_CONTACT_BIRTHYEAR))
					{
						bWantBackup = bWantBackup 
											 && !IsEqual(mdbIgnore.SystemTime()) 
											 && (bDontIgnoreAnything || (DB::Setting::GetDWord(hSubContact, USERINFO, SET_REMIND_BIRTHDAY_IGNORED, 0) != mdbIgnore.DateStamp()));
					}
				}
				if (bWantBackup)
				{
					if (!lastAnswer || *lastAnswer != IDALL)
					{
						WORD rc = AskUser(hContact, this, &mdbNewProto);
						if (lastAnswer)
						{
							*lastAnswer = rc;
						}
						if (IDYES != rc && IDALL != rc) 
						{
							// special handling for metasubcontacts required?!
							mdbNewProto.DBWriteDateStamp(hContact, USERINFO, SET_REMIND_BIRTHDAY_IGNORED);
							bWantBackup = FALSE;
						}
					}
					if (bWantBackup)
					{
						Set(mdbNewProto);
						DBWriteDateStamp(hContact, USERINFO, SET_REMIND_BIRTHDAY_IGNORED);
						DBWriteBirthDate(hContact);
						
						// update metasubcontacts
						for (INT i = 0; i < nSubContactCount; i++)
						{
							hSubContact = DB::MetaContact::Sub(hContact, i);
							if (hSubContact != NULL) 
							{
								if (!mdbIgnore.DBGetDate(hSubContact, DB::Contact::Proto(hSubContact), SET_CONTACT_BIRTHDAY, SET_CONTACT_BIRTHMONTH, SET_CONTACT_BIRTHYEAR))
								{
									mdbIgnore.DBWriteDateStamp(hSubContact, USERINFO, SET_REMIND_BIRTHDAY_IGNORED);
								}
								DBWriteBirthDate(hSubContact);
							}
						}
						return 0;
					}
				}
			}
		}				
		/*
		else if (mir_stricmp(Module(), SvcReminderGetMyBirthdayModule()))
		{
			DBWriteBirthDate(hContact);
		}
		*/
	}
	return 1;
}