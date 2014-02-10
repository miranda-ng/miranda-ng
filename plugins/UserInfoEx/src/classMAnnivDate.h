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

#define ANID_LAST		0xFFFC
#define ANID_BIRTHDAY	0xFFFE
#define ANID_NONE		0xFFFF

struct MZodiac {
	HICON	hIcon;
	LPCTSTR	pszName;
};

class MAnnivDate : public MTime
{
public:
	typedef enum {
		MADF_NONE				= 0,
		MADF_CHANGED			= 1,	// date has been edited (used, if date is used in controls)
		MADF_HASPROTO			= 2,	// basic protocol module contains date information
		MADF_HASCUSTOM			= 4,	// date is customized or read from a custom module 
		MADF_HASMETA			= 8,	// date is read from a metacontact's subcontact
		MADF_REMINDER_CHANGED	= 16	// reminder options have changed
	} EFlags;

private:
	WORD	_wID;			// index to anniversary in database or ANID_BIRTHDAY
	tstring	_strDesc;		// descripes the anniversary (e.g. birthday)
	string	_strModule;		// the module the anniversary has been read from
	WORD	_wFlags;		// the flags
	BYTE	_bRemind;		// per user setting for reminder (0 - disabled, 1 - use local offset, 2 - use global offset)
	WORD	_wDaysEarlier;	// number of days to the anniversary the user wants to be reminded of this anniversary

	int DBWriteDate(MCONTACT hContact, LPCSTR pszModule, LPCSTR szDay, LPCSTR szMonth, LPCSTR szYear);
	int DBDeleteDate(MCONTACT hContact, LPCSTR pszModule, LPCSTR szDay, LPCSTR szMonth, LPCSTR szYear) const;

public:
	MAnnivDate();
	MAnnivDate(MAnnivDate &mda);

	// basic access to attributes
	__inline LPCTSTR	Description() const				{ return _strDesc.c_str(); };
	__inline void		Description(LPCTSTR pszDesc)	{ if (pszDesc) _strDesc = pszDesc; };
	__inline LPCSTR		Module() const					{ return _strModule.c_str(); };
	__inline void		Module(LPCSTR pszModule)		{ if (pszModule) _strModule = pszModule; else _strModule.clear(); };
	__inline BYTE		RemindOption() const			{ return _bRemind; };
	__inline void		RemindOption(BYTE bRemind)		{ if (bRemind <= BST_INDETERMINATE) _bRemind = bRemind; };
	__inline WORD		RemindOffset() const			{ return _wDaysEarlier; };
	__inline void		RemindOffset(WORD wOffset)		{ _wDaysEarlier = wOffset; };
	__inline WORD		Id() const						{ return _wID; };
	__inline void		Id(WORD wId)					{ if (_wID == ANID_NONE) _wID = wId; };
	
	DWORD				DateStamp() const;
	void				DateStamp(const DWORD dwStamp);
	
	// basic checks
	__inline BYTE IsValid() const;
	__inline BYTE IsChanged() const						{ return (_wFlags & MADF_CHANGED); };
	__inline BYTE IsReminderChanged() const				{ return (_wFlags & MADF_REMINDER_CHANGED); };
	__inline BYTE IsEqual(const MAnnivDate &mda) const	{ return IsEqual(mda.SystemTime()); };
	BYTE	IsEqual(const SYSTEMTIME &st) const;

	// handling flags
	__inline WORD	Flags() const			{ return _wFlags; };
	__inline void	Flags(WORD wFlags)		{ _wFlags = wFlags; };
	__inline void	SetFlags(WORD wFlag)	{ _wFlags |= wFlag; };
	__inline void	RemoveFlags(WORD wFlag)	{ _wFlags &= ~wFlag; };

	// return diffence of days, ignoring the date
	int	CompareDays(MTime mt) const;
	
	MZodiac	Zodiac();
	int		Age(MTime *pNow = NULL);
	void	Clear();

	// read date from database
	int DBGetDate(MCONTACT hContact, LPCSTR pszModule, LPCSTR szDay, LPCSTR szMonth, LPCSTR szYear);
	int DBGetDateStamp(MCONTACT hContact, LPCSTR pszModule, LPCSTR pszSetting);
	int DBGetAnniversaryDate(MCONTACT hContact, WORD iIndex);
	int DBGetBirthDate(MCONTACT hContact, LPSTR pszProto = NULL);
	int DBGetReminderOpts(MCONTACT hContact);

	// write date to database
	int DBWriteDateStamp(MCONTACT hContact, LPCSTR pszModule, LPCSTR pszSetting);
	int DBWriteAnniversaryDate(MCONTACT hContact, WORD wIndex);
	int DBWriteBirthDate(MCONTACT hContact);
	int DBWriteReminderOpts(MCONTACT hContact);

	// delete date from database
	int DBDeleteBirthDate(MCONTACT hContact);

	int DBMoveBirthDate(MCONTACT hContact, BYTE bOld, BYTE bNew);
	int BackupBirthday (MCONTACT hContact, LPSTR pszProto = NULL, const BYTE bDontIgnoreAnything = FALSE, PWORD lastAnswer = NULL);

	// setting values
	void	SetDate(SYSTEMTIME &st);
	void	SetDate(MAnnivDate &mda);

	BYTE operator == (const SYSTEMTIME &st) { return IsEqual(st); };
	BYTE operator == (const MAnnivDate &mda) { return IsEqual(mda); };

	void operator = (SYSTEMTIME &st) { SetDate(st); };
	void operator = (MAnnivDate &mda) { SetDate(mda); };
};
