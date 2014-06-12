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

#ifndef _SVC_CONSTANTS_INCLUDED_
#define _SVC_CONSTANTS_INCLUDED_

#define MODULELONGNAME		"Extended user info"
#define USERINFO			"UserInfo"
#define MODNAME				"UserInfoEx"
#define MODNAMEFLAGS		"Flags"

#define MODULELONGNAMET		_T(MODULELONGNAME)
#define MODNAMET			_T(MODNAME)

#define MAXDATASIZE			1024		// maximum character count of most static, temporary, ...., strings
#define MAXCATLEN			64			// maximum character count for a category string (phone, email, interest, ...)
#define MAXSETTING			255			// maximum character count for a setting string
#define MAXNAME				260			// maximum character count for a username
#define MAXUID				260			// maximum character count for a uin

// most important modules
#define MOD_MBIRTHDAY					"mBirthday"
#define MOD_CLIST						"CList"

// database settings (propertysheet)
#define	SET_PROPSHEET_PCBIREADONLY		"PBCIReadOnly"
#define	SET_PROPSHEET_READONLYLABEL		"TILReadonly"
#define	SET_PROPSHEET_AEROADAPTION		"AeroAdaption"
#define	SET_PROPSHEET_SHOWCOLOURS		"ShowColours"
#define	SET_PROPSHEET_CLRNORMAL			"colourNormal"
#define	SET_PROPSHEET_CLRCUSTOM			"colourUser"
#define	SET_PROPSHEET_CLRBOTH			"colourBoth"
#define	SET_PROPSHEET_CLRCHANGED		"colourChanged"
#define	SET_PROPSHEET_CLRMETA			"colourMeta"
#define	SET_PROPSHEET_SAVEVCARD			"vCardOnExit"
#define	SET_PROPSHEET_GROUPS			"TreeGroups"
#define	SET_PROPSHEET_SORTITEMS			"TreeSortItems"
#define	SET_PROPSHEET_CHANGEMYDETAILS	"ChangeMyDetails"
#define	SET_ABOUT_ACTIVEWINDOW			"AboutActiveWin"
#define	SET_MI_MAIN						"miMenu"
#define	SET_MI_CONTACT					"miContact"
#define	SET_MI_GROUP					"miGroup"
#define	SET_MI_SUBGROUP					"miSubGroup"
#define	SET_MI_STATUS					"miStatus"
#define	SET_MI_ACCOUNT					"miAccount"

#define	SET_EXTENDED_EMAILSERVICE		"emailEx"
#define	SET_CLIST_EXTRAICON_GENDER		"GenderColumn"
#define	SET_CLIST_EXTRAICON_GENDER2		"cliGender"
#define	SET_CLIST_EXTRAICON_COUNTRY		"CountryColumn"
#define	SET_CLIST_EXTRAICON_HOMEPAGE	"cliHomepage"
#define	SET_CLIST_EXTRAICON_EMAIL		"cliEmail"
#define	SET_CLIST_EXTRAICON_PHONE		"cliPhone"
#define	SET_OPT_AUTOTIMEZONE			"AutoTimezone"
#define	SET_ZODIAC_AVATARS				"ZodicAvatars"
#define	SET_META_SCAN					"MetaScan"
// database	settings (general psp)
#define	SET_ME_PASSWORD					"Password"
#define	SET_CONTACT_TITLE				"Title"
#define	SET_CONTACT_FIRSTNAME			"FirstName"
#define	SET_CONTACT_SECONDNAME			"SecondName"
#define	SET_CONTACT_LASTNAME			"LastName"
#define	SET_CONTACT_FIRSTLASTNAME		"FullName"
#define	SET_CONTACT_PREFIX				"Prefix"
#define	SET_CONTACT_NICK				"Nick"
#define	SET_CONTACT_MYHANDLE			"MyHandle"
#define	SET_CONTACT_STREET				"Street"
#define	SET_CONTACT_ZIP					"Zip"
#define	SET_CONTACT_CITY				"City"
#define	SET_CONTACT_STATE				"State"
#define	SET_CONTACT_COUNTRY				"Country"
#define	SET_CONTACT_GENDER				"Gender"
// database settings (advanced psp)
#define	SET_CONTACT_ORIGIN_STREET		"OriginStreet"
#define	SET_CONTACT_ORIGIN_ZIP			"OriginZip"
#define	SET_CONTACT_ORIGIN_CITY			"OriginCity"
#define	SET_CONTACT_ORIGIN_STATE		"OriginState"
#define	SET_CONTACT_ORIGIN_COUNTRY		"OriginCountry"
#define	SET_CONTACT_LANG1				"Language1"
#define	SET_CONTACT_LANG2				"Language2"
#define	SET_CONTACT_LANG3				"Language3"
#define	SET_CONTACT_MARITAL				"MaritalStatus"
#define	SET_CONTACT_PARTNER				"Partner"
#define	SET_CONTACT_ANNIVERSARY			"Anniv"
#define	SET_CONTACT_AGE					"Age"
#define	SET_CONTACT_TIMEZONE			"Timezone"
#define	SET_CONTACT_TIMEZONENAME		"TzName"
#define	SET_CONTACT_TIMEZONEINDEX		"TzIndex"
#define	SET_CONTACT_BIRTH				"Birth"
#define	SET_CONTACT_BIRTHDAY			"BirthDay"
#define	SET_CONTACT_BIRTHMONTH			"BirthMonth"
#define	SET_CONTACT_BIRTHYEAR			"BirthYear"
#define	SET_CONTACT_DOBD				"DOBd"
#define	SET_CONTACT_DOBM				"DOBm"
#define	SET_CONTACT_DOBY				"DOBy"
// database settings (company psp)
#define	SET_CONTACT_COMPANY_POSITION	"CompanyPosition"
#define	SET_CONTACT_COMPANY_OCCUPATION	"CompanyOccupation"
#define	SET_CONTACT_COMPANY_SUPERIOR	"CompanySuperior"
#define	SET_CONTACT_COMPANY_ASSISTENT	"CompanyAssistent"
#define	SET_CONTACT_COMPANY				"Company"
#define	SET_CONTACT_COMPANY_DEPARTMENT	"CompanyDepartment"
#define	SET_CONTACT_COMPANY_OFFICE		"CompanyOffice"
#define	SET_CONTACT_COMPANY_STREET		"CompanyStreet"
#define	SET_CONTACT_COMPANY_ZIP			"CompanyZip"
#define	SET_CONTACT_COMPANY_CITY		"CompanyCity"
#define	SET_CONTACT_COMPANY_STATE		"CompanyState"
#define	SET_CONTACT_COMPANY_COUNTRY		"CompanyCountry"
#define	SET_CONTACT_COMPANY_HOMEPAGE	"CompanyHomepage"
// database settings (about psp)
#define	SET_CONTACT_ABOUT				"About"
#define	SET_CONTACT_MYNOTES				"MyNotes"
// database settings (... psp)
#define	SET_CONTACT_PHONE				"Phone"
#define	SET_CONTACT_FAX					"Fax"
#define	SET_CONTACT_CELLULAR			"Cellular"
#define	SET_CONTACT_EMAIL				"e-mail"
#define	SET_CONTACT_EMAIL0				"e-mail0"
#define	SET_CONTACT_EMAIL1				"e-mail1"
#define	SET_CONTACT_HOMEPAGE			"Homepage"
#define	SET_CONTACT_COMPANY_PHONE		"CompanyPhone"
#define	SET_CONTACT_COMPANY_FAX			"CompanyFax"
#define	SET_CONTACT_COMPANY_CELLULAR	"CompanyCellular"
#define	SET_CONTACT_COMPANY_EMAIL		"Companye-mail"
#define	SET_CONTACT_COMPANY_EMAIL0		"Companye-mail0"
#define	SET_CONTACT_COMPANY_EMAIL1		"Companye-mail1"

#define	SET_CONTACT_MYPHONE_VAL			"MyPhone%d"
#define	SET_CONTACT_MYPHONE_CAT			"MyPhone%dCat"
#define	SET_CONTACT_COMPANY_MYPHONE_VAL	"MyCompanyPhone%d"
#define	SET_CONTACT_COMPANY_MYPHONE_CAT	"MyCompanyPhone%dCat"
#define	SET_CONTACT_MYEMAIL_VAL			"Mye-mail%d"
#define	SET_CONTACT_MYEMAIL_CAT			"Mye-mail%dCat"
#define	SET_CONTACT_COMPANY_MYEMAIL_VAL	"MyCompanye-mail%d"
#define	SET_CONTACT_COMPANY_MYEMAIL_CAT	"MyCompanye-mail%dCat"

#define	SET_CONTACT_ADDEDTIME			"ContactAddTime"
// default values for some of the options
#define DEFVAL_GETCONTACTINFO_ENABLED	1

#define DEFVAL_CLIST_EXTRAICON_GENDER	2
#define DEFVAL_CLIST_EXTRAICON_COUNTRY	3
#define DEFVAL_CLIST_EXTRAICON_HOMEPAGE 1
#define DEFVAL_CLIST_EXTRAICON_EMAIL	1
#define DEFVAL_CLIST_EXTRAICON_PHONE	1

typedef struct CIDList
{
	int		nID;
	LPCSTR	pszText;
	LPTSTR	ptszTranslated;

} IDSTRLIST, *LPIDSTRLIST;

INT_PTR GetMaritalList(LPUINT pListSize, LPIDSTRLIST *pList);
INT_PTR GetLanguageList(LPUINT pListSize, LPIDSTRLIST *pList);
INT_PTR GetCountryList(LPUINT pListSize, LPIDSTRLIST *pList);
INT_PTR GetOccupationList(LPUINT pListSize, LPIDSTRLIST *pList);
INT_PTR GetInterestsList(LPUINT pListSize, LPIDSTRLIST *pList);
INT_PTR GetPastList(LPUINT pListSize, LPIDSTRLIST *pList);
INT_PTR GetAffiliationsList(LPUINT pListSize, LPIDSTRLIST *pList);
INT_PTR GetNamePrefixList(LPUINT pListSize, LPIDSTRLIST *pList);

void SvcConstantsLoadModule(void);
void SvcConstantsUnloadModule(void);

#endif /* _SVC_CONSTANTS_INCLUDED_ */