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

	INT DBWriteDate(HANDLE hContact, LPCSTR pszModule, LPCSTR szDay, LPCSTR szMonth, LPCSTR szYear);
	INT DBDeleteDate(HANDLE hContact, LPCSTR pszModule, LPCSTR szDay, LPCSTR szMonth, LPCSTR szYear) const;

public:
	MAnnivDate();
	MAnnivDate(MAnnivDate &mda);

	// basic access to attributes
	__inline LPCTSTR	Description() const				{ return _strDesc.c_str(); };
	__inline VOID		Description(LPCTSTR pszDesc)	{ if (pszDesc) _strDesc = pszDesc; };
	__inline LPCSTR		Module() const					{ return _strModule.c_str(); };
	__inline VOID		Module(LPCSTR pszModule)		{ if (pszModule) _strModule = pszModule; else _strModule.clear(); };
	__inline BYTE		RemindOption() const			{ return _bRemind; };
	__inline VOID		RemindOption(BYTE bRemind)		{ if (bRemind <= BST_INDETERMINATE) _bRemind = bRemind; };
	__inline WORD		RemindOffset() const			{ return _wDaysEarlier; };
	__inline VOID		RemindOffset(WORD wOffset)		{ _wDaysEarlier = wOffset; };
	__inline WORD		Id() const						{ return _wID; };
	__inline VOID		Id(WORD wId)					{ if (_wID == ANID_NONE) _wID = wId; };
	
	DWORD				DateStamp() const;
	VOID				DateStamp(const DWORD dwStamp);
	
	// basic checks
	__inline BOOLEAN IsValid() const;
	__inline BOOLEAN IsChanged() const						{ return (_wFlags & MADF_CHANGED); };
	__inline BOOLEAN IsReminderChanged() const				{ return (_wFlags & MADF_REMINDER_CHANGED); };
	__inline BOOLEAN IsEqual(const MAnnivDate &mda) const	{ return IsEqual(mda.SystemTime()); };
	BOOLEAN	IsEqual(const SYSTEMTIME &st) const;

	// handling flags
	__inline WORD	Flags() const			{ return _wFlags; };
	__inline VOID	Flags(WORD wFlags)		{ _wFlags = wFlags; };
	__inline VOID	SetFlags(WORD wFlag)	{ _wFlags |= wFlag; };
	__inline VOID	RemoveFlags(WORD wFlag)	{ _wFlags &= ~wFlag; };

	// return diffence of days, ignoring the date
	INT	CompareDays(MTime mt) const;
	
	MZodiac	Zodiac();
	INT		Age(MTime *pNow = NULL);
	VOID	Clear();

	// read date from database
	INT DBGetDate(HANDLE hContact, LPCSTR pszModule, LPCSTR szDay, LPCSTR szMonth, LPCSTR szYear);
	INT DBGetDateStamp(HANDLE hContact, LPCSTR pszModule, LPCSTR pszSetting);
	INT DBGetAnniversaryDate(HANDLE hContact, WORD iIndex);
	INT DBGetBirthDate(HANDLE hContact, LPSTR pszProto = NULL);
	INT DBGetReminderOpts(HANDLE hContact);

	// write date to database
	INT DBWriteDateStamp(HANDLE hContact, LPCSTR pszModule, LPCSTR pszSetting);
	INT DBWriteAnniversaryDate(HANDLE hContact, WORD wIndex);
	INT DBWriteBirthDate(HANDLE hContact);
	INT DBWriteReminderOpts(HANDLE hContact);

	// delete date from database
	INT DBDeleteBirthDate(HANDLE hContact);

	INT DBMoveBirthDate(HANDLE hContact, BYTE bOld, BYTE bNew);
	INT BackupBirthday (HANDLE hContact, LPSTR pszProto = NULL, const BOOLEAN bDontIgnoreAnything = FALSE, PWORD lastAnswer = NULL);

	// setting values
	VOID	SetDate(SYSTEMTIME &st);
	VOID	SetDate(MAnnivDate &mda);

	BOOLEAN operator == (const SYSTEMTIME &st) { return IsEqual(st); };
	BOOLEAN operator == (const MAnnivDate &mda) { return IsEqual(mda); };

	VOID operator = (SYSTEMTIME &st) { SetDate(st); };
	VOID operator = (MAnnivDate &mda) { SetDate(mda); };
};
