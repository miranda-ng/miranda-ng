/*
    Variables Plugin for Miranda-IM (www.miranda-im.org)
    Copyright 2003-2006 P. Boon

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#include "variables.h"
#include "parse_miranda.h"
#include "contact.h"

static TCHAR *parseCodeToStatus(ARGUMENTSINFO *ai)
{
	TCHAR *szStatus;
	unsigned int status;

	if (ai->argc != 2)
		return NULL;

	status = ttoi(ai->targv[1]);
	szStatus = (TCHAR *)CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, (WPARAM)status, GSMDF_TCHAR);
	if (szStatus != NULL)
		return _tcsdup(szStatus);

	return NULL;
}

static int getContactInfoFlags(TCHAR *tszDesc)
{
	int flags;
	TCHAR *cur;

	flags = 0;
	for (cur=tszDesc;(cur < (tszDesc+_tcslen(tszDesc)));cur++) {
		if (!_tcsnicmp(cur, _T(STR_PROTOID), _tcslen(_T(STR_PROTOID)))) {
			flags|=CI_PROTOID;
			cur += _tcslen(_T(STR_PROTOID)) - 1;
		}
		else if (!_tcsnicmp(cur, _T(STR_NICK), _tcslen(_T(STR_NICK)))) {
			flags|=CI_NICK;
			cur += _tcslen(_T(STR_NICK)) - 1;
		}
		else if (!_tcsnicmp(cur, _T(STR_FIRSTNAME), _tcslen(_T(STR_FIRSTNAME)))) {
			flags|=CI_FIRSTNAME;
			cur += _tcslen(_T(STR_FIRSTNAME)) - 1;
		}
		else if (!_tcsnicmp(cur, _T(STR_LASTNAME), _tcslen(_T(STR_LASTNAME)))) {
			flags|=CI_LASTNAME;
			cur += _tcslen(_T(STR_LASTNAME)) - 1;
		}
		else if (!_tcsnicmp(cur, _T(STR_DISPLAY), _tcslen(_T(STR_DISPLAY)))) {
			flags|=CI_LISTNAME;
			cur += _tcslen(_T(STR_DISPLAY)) - 1;
		}
		else if (!_tcsnicmp(cur, _T(STR_EMAIL), _tcslen(_T(STR_EMAIL)))) {
			flags|=CI_EMAIL;
			cur += _tcslen(_T(STR_EMAIL)) - 1;
		}
		else if (!_tcsnicmp(cur, _T(STR_UNIQUEID), _tcslen(_T(STR_UNIQUEID)))) {
			flags|=CI_UNIQUEID;
			cur += _tcslen(_T(STR_UNIQUEID)) - 1;
		}
	}
	if (flags == 0) {
		flags = getContactInfoType(tszDesc);
		if (flags != 0)
			flags |= CI_CNFINFO;
	}
	flags |= CI_TCHAR;

	return flags;
}

static TCHAR *parseContact(ARGUMENTSINFO *ai)
{
	CONTACTSINFO ci;
	int count, n;
	HANDLE hContact;

	if ( ai->argc < 3 || ai->argc > 4 )
		return NULL;

	n = 0;
	if ( ai->argc == 4 && *ai->targv[3] != _T('r'))
		n = ttoi(ai->targv[3]) - 1;

	ZeroMemory(&ci, sizeof(ci));
	ci.cbSize = sizeof(ci);
	ci.tszContact = ai->targv[1];
	ci.flags = getContactInfoFlags(ai->targv[2]);
	count = getContactFromString( &ci );
	if ( count == 0 || ci.hContacts == NULL )
		return NULL;

	if ( ai->argc == 4 && *ai->targv[3] == _T('r'))
		n = rand() % count;

	if ( count != 1 && ai->argc != 4 ) {
		if ( ci.hContacts != NULL )
			free(ci.hContacts);

		return NULL;
	}
	hContact = ci.hContacts[n];
	log_debugA("contact: %x", hContact);
	free(ci.hContacts);

	return encodeContactToString(hContact);
}

static TCHAR *parseContactCount(ARGUMENTSINFO *ai)
{
	CONTACTSINFO ci;
	int count;

	if (ai->argc != 3)
		return NULL;

	ZeroMemory(&ci, sizeof(ci));
	ci.cbSize = sizeof(ci);
	ci.tszContact = ai->targv[1];
	ci.flags = getContactInfoFlags(ai->targv[2]);
	count = getContactFromString( &ci );
	if ( count != 0 && ci.hContacts != NULL )
		free(ci.hContacts);

	return itot(count);
}

static TCHAR *parseContactInfo(ARGUMENTSINFO *ai)
{
	HANDLE hContact;
	CONTACTSINFO ci;
	int count;
	BYTE type;

	if (ai->argc != 3)
		return NULL;

	hContact = NULL;
	ZeroMemory(&ci, sizeof(ci));
	ci.cbSize = sizeof(ci);
	ci.tszContact = ai->targv[1];
	ci.flags = 0xFFFFFFFF ^ (CI_TCHAR == 0 ? CI_UNICODE : 0);
	count = getContactFromString( &ci );
	if ( count == 1 && ci.hContacts != NULL ) {
		hContact = ci.hContacts[0];
		free(ci.hContacts);
	}
	else {
		if (ci.hContacts != NULL)
			free(ci.hContacts);
		return NULL;
	}
	type = getContactInfoType(ai->targv[2]);
	if (type == 0)
		return NULL;

	return getContactInfoT(type, hContact, 1);
}

static TCHAR *parseDBProfileName(ARGUMENTSINFO *ai)
{
	char name[MAX_PATH];

	if (ai->argc != 1)
		return NULL;

	if (CallService(MS_DB_GETPROFILENAME, SIZEOF(name), (LPARAM)name))
		return NULL;

	#ifdef UNICODE
		return a2u(name);
	#else
		return _strdup(name);
	#endif
}

static TCHAR *parseDBProfilePath(ARGUMENTSINFO *ai) {

	char path[MAX_PATH];

	if (ai->argc != 1)
		return NULL;

	if (CallService(MS_DB_GETPROFILEPATH, SIZEOF(path), (LPARAM)path))
		return NULL;

	#ifdef UNICODE
		return a2u(path);
	#else
		return _strdup(path);
	#endif
}

static TCHAR* getDBSetting(HANDLE hContact, char* module, char* setting, TCHAR* defaultValue) {

	DBVARIANT dbv;
	TCHAR* var = NULL;

	if (DBGetContactSettingW(hContact, module, setting, &dbv))
		return defaultValue;

	switch (dbv.type) {
	case DBVT_BYTE:
		var = itot(dbv.bVal);
		break;
	case DBVT_WORD:
		var = itot(dbv.wVal);
		break;
	case DBVT_DWORD:
		var = itot(dbv.dVal);
		break;
	case DBVT_ASCIIZ:
		#ifdef UNICODE
			var = a2u(dbv.pszVal);
		#else
			var = _strdup(dbv.pszVal);
		#endif
		break;
	case DBVT_WCHAR:
		#ifdef UNICODE
			var = _wcsdup(dbv.pwszVal);
		#else
			var = u2a(dbv.pwszVal);
		#endif
		break;
	case DBVT_UTF8:
		#ifdef UNICODE
			Utf8Decode(dbv.pszVal, &var);
		#else
			var = _strdup(dbv.pszVal);
			Utf8Decode(var, NULL);
		#endif
		break;
	}

	DBFreeVariant(&dbv);
	return var;
}

static TCHAR *parseDBSetting(ARGUMENTSINFO *ai)
{
	CONTACTSINFO ci;
	HANDLE hContact;
	int count;
	char *szModule, *szSetting;
	TCHAR *res, *szDefaultValue;

	if (ai->argc < 4)
		return NULL;

	res = NULL;
	hContact = NULL;
	szDefaultValue = NULL;
	if (_tcslen(ai->targv[1]) > 0) {
		ZeroMemory(&ci, sizeof(ci));
		ci.cbSize = sizeof(ci);
		ci.tszContact = ai->targv[1];
		ci.flags = 0xFFFFFFFF^(CI_TCHAR==0?CI_UNICODE:0);
		count = getContactFromString( &ci );
		if ( count == 1 && ci.hContacts != NULL ) {
			hContact = ci.hContacts[0];
			free(ci.hContacts);
		}
		else {
			if (ci.hContacts != NULL)
				free(ci.hContacts);
			return NULL;
		}
	}

	#ifdef UNICODE
		szModule = u2a(ai->targv[2]);
		szSetting = u2a(ai->targv[3]);
	#else
		szModule = _strdup(ai->argv[2]);
		szSetting = _strdup(ai->argv[3]);
	#endif

	if ( ai->argc > 4 && _tcslen(ai->targv[4]) > 0 )
		szDefaultValue = _tcsdup(ai->targv[4]);

	if ( szModule != NULL && szSetting != NULL ) {
		res = getDBSetting(hContact, szModule, szSetting, szDefaultValue);
		free(szModule);
		free(szSetting);
	}
	return res;
}

static TCHAR *parseLastSeenDate(ARGUMENTSINFO *ai)
{
	HANDLE hContact;
	CONTACTSINFO ci;
	int count, len;
	SYSTEMTIME lsTime;
	TCHAR *szFormat, *res;
	char *szModule;

	if (ai->argc <= 1)
		return NULL;

	hContact = NULL;
	ZeroMemory(&ci, sizeof(ci));
	ci.cbSize = sizeof(ci);
	ci.tszContact = ai->targv[1];
	ci.flags = 0xFFFFFFFF^(CI_TCHAR==0?CI_UNICODE:0);
	count = getContactFromString( &ci );
	if ( count == 1 && ci.hContacts != NULL ) {
		hContact = ci.hContacts[0];
		free(ci.hContacts);
	}
	else {
		if (ci.hContacts != NULL)
			free(ci.hContacts);
		return NULL;
	}
	if ( ai->argc == 2 || (ai->argc > 2 && _tcslen(ai->targv[2]) == 0))
		szFormat = NULL;
	else
		szFormat = ai->targv[2];

	ZeroMemory(&lsTime, sizeof(lsTime));
	szModule = CEX_MODULE;
	lsTime.wYear = DBGetContactSettingWord(hContact, szModule, "Year", 0);
	if (lsTime.wYear == 0)
		szModule = SEEN_MODULE;

	lsTime.wYear = DBGetContactSettingWord(hContact, szModule, "Year", 0);
	if (lsTime.wYear == 0)
		return NULL;

	lsTime.wMilliseconds = 0;
	lsTime.wSecond = DBGetContactSettingWord(hContact, szModule, "Seconds", 0);
	lsTime.wMinute = DBGetContactSettingWord(hContact, szModule, "Minutes", 0);
	lsTime.wHour = DBGetContactSettingWord(hContact, szModule, "Hours", 0);
	lsTime.wDay = DBGetContactSettingWord(hContact, szModule, "Day", 0);
	lsTime.wDayOfWeek = DBGetContactSettingWord(hContact, szModule, "WeekDay", 0);
	lsTime.wMonth = DBGetContactSettingWord(hContact, szModule, "Month", 0);

	len = GetDateFormat(LOCALE_USER_DEFAULT, 0, &lsTime, szFormat, NULL, 0);
	res = ( TCHAR* )malloc((len+1)*sizeof(TCHAR));
	if (res == NULL)
		return NULL;

	if (GetDateFormat(LOCALE_USER_DEFAULT, 0, &lsTime, szFormat, res, len) == 0) {
		free(res);
		return NULL;
	}

	return res;
}

static TCHAR *parseLastSeenTime(ARGUMENTSINFO *ai)
{
	HANDLE hContact;
	CONTACTSINFO ci;
	int count, len;
	SYSTEMTIME lsTime;
	TCHAR *szFormat, *res;
	char *szModule;

	if (ai->argc <= 1)
		return NULL;

	hContact = NULL;
	ZeroMemory(&ci, sizeof(ci));
	ci.cbSize = sizeof(ci);
	ci.tszContact = ai->targv[1];
	ci.flags = 0xFFFFFFFF^(CI_TCHAR==0?CI_UNICODE:0);
	count = getContactFromString( &ci );
	if ( count == 1 && ci.hContacts != NULL ) {
		hContact = ci.hContacts[0];
		free(ci.hContacts);
	}
	else {
		if (ci.hContacts != NULL)
			free(ci.hContacts);
		return NULL;
	}
	if ( ai->argc == 2 || (ai->argc > 2 && _tcslen(ai->targv[2]) == 0))
		szFormat = NULL;
	else
		szFormat = ai->targv[2];

	ZeroMemory(&lsTime, sizeof(lsTime));
	szModule = CEX_MODULE;
	lsTime.wYear = DBGetContactSettingWord(hContact, szModule, "Year", 0);
	if (lsTime.wYear == 0)
		szModule = SEEN_MODULE;

	lsTime.wYear = DBGetContactSettingWord(hContact, szModule, "Year", 0);
	if (lsTime.wYear == 0)
		return NULL;

	lsTime.wMilliseconds = 0;
	lsTime.wSecond = DBGetContactSettingWord(hContact, szModule, "Seconds", 0);
	lsTime.wMinute = DBGetContactSettingWord(hContact, szModule, "Minutes", 0);
	lsTime.wHour = DBGetContactSettingWord(hContact, szModule, "Hours", 0);
	lsTime.wDay = DBGetContactSettingWord(hContact, szModule, "Day", 0);
	lsTime.wDayOfWeek = DBGetContactSettingWord(hContact, szModule, "WeekDay", 0);
	lsTime.wMonth = DBGetContactSettingWord(hContact, szModule, "Month", 0);
	lsTime.wYear = DBGetContactSettingWord(hContact, szModule, "Year", 0);

	len = GetTimeFormat(LOCALE_USER_DEFAULT, 0, &lsTime, szFormat, NULL, 0);
	res = ( TCHAR* )malloc((len+1)*sizeof(TCHAR));
	if (res == NULL)
		return NULL;

	if (GetTimeFormat(LOCALE_USER_DEFAULT, 0, &lsTime, szFormat, res, len) == 0) {
		free(res);
		return NULL;
	}

	return res;
}

static TCHAR *parseLastSeenStatus(ARGUMENTSINFO *ai) {

	HANDLE hContact;
	CONTACTSINFO ci;
	int count, status;
	char *szModule;
	TCHAR *szStatus;

	if (ai->argc != 2)
		return NULL;

	hContact = NULL;
	ZeroMemory(&ci, sizeof(ci));
	ci.cbSize = sizeof(ci);
	ci.tszContact = ai->targv[1];
	ci.flags = 0xFFFFFFFF^(CI_TCHAR==0?CI_UNICODE:0);
	count = getContactFromString( &ci );
	if ( (count == 1) && (ci.hContacts != NULL) ) {
		hContact = ci.hContacts[0];
		free(ci.hContacts);
	}
	else {
		if (ci.hContacts != NULL)
			free(ci.hContacts);
		return NULL;
	}
	szModule = CEX_MODULE;
	status = DBGetContactSettingWord(hContact, szModule, "Status", 0);
	if (status == 0)
		szModule = SEEN_MODULE;

	status = DBGetContactSettingWord(hContact, szModule, "Status", 0);
	if (status == 0)
		return NULL;

	szStatus = (TCHAR *)CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, (WPARAM)status, GSMDF_UNICODE);
	if (szStatus != NULL)
		return _tcsdup(szStatus);

	return NULL;
}

static TCHAR *parseMirandaPath(ARGUMENTSINFO *ai) {

	TCHAR path[MAX_PATH];

	if (ai->argc != 1)
		return NULL;

	ai->flags |= AIF_DONTPARSE;
	if (GetModuleFileName(NULL, path, SIZEOF(path)) == 0)
		return NULL;

	return _tcsdup(path);
}

static TCHAR *parseMyStatus(ARGUMENTSINFO *ai) {

	int status;
	char *szProto;
	TCHAR *szStatus;

	if (ai->argc > 2)
		return NULL;

	if ( ai->argc == 1 || _tcslen(ai->targv[1]) == 0 )
		status = CallService(MS_CLIST_GETSTATUSMODE, 0, 0);
	else {
#ifdef UNICODE
		szProto = u2a(ai->targv[1]);
#else
		szProto = _strdup(ai->targv[1]);
#endif
		status = CallProtoService(szProto, PS_GETSTATUS, 0, 0);
		free(szProto);
	}
	szStatus = (TCHAR *)CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, (WPARAM)status, GSMDF_UNICODE);
	if (szStatus != NULL)
		return _tcsdup(szStatus);

	return NULL;
}

static TCHAR *parseProtoInfo(ARGUMENTSINFO *ai)
{
	TCHAR *tszRes;
	char *szProto, *szRes;

	if (ai->argc != 3)
		return NULL;

	szRes = NULL;
	tszRes = NULL;
#ifdef UNICODE
	szProto = u2a(ai->targv[1]);
#else
	szProto = _strdup(ai->targv[1]);
#endif
	if (!_tcscmp(ai->targv[2], _T(STR_PINAME)))
		tszRes = Hlp_GetProtocolName(szProto);
	else if (!_tcscmp(ai->targv[2], _T(STR_PIUIDTEXT))) {
		char *szText;

		if (!ProtoServiceExists(szProto, PS_GETCAPS))
			return NULL;

		szText = (char *)CallProtoService(szProto, PS_GETCAPS, (WPARAM)PFLAG_UNIQUEIDTEXT, 0);
		if (szText != NULL)
			szRes = _strdup(szText);
	}
	else if (!_tcscmp(ai->targv[2], _T(STR_PIUIDSETTING))) {
		char *szText;
		if (!ProtoServiceExists(szProto, PS_GETCAPS))
			return NULL;

		szText = (char *)CallProtoService(szProto, PS_GETCAPS, (WPARAM)PFLAG_UNIQUEIDSETTING, 0);
		if (szText != NULL)
			szRes = _strdup(szText);
	}
	free(szProto);
	if ( szRes == NULL && tszRes == NULL )
		return NULL;

	if ( szRes != NULL && tszRes == NULL ) {
		#ifdef UNICODE
			tszRes = a2u(szRes);
			free(szRes);
		#else
			tszRes = szRes;
		#endif
	}
	else if ( szRes != NULL && tszRes != NULL )
		free(szRes);

	return tszRes;
}

static TCHAR *parseSpecialContact(ARGUMENTSINFO *ai)
{
	char *szProto;
	TCHAR *szUniqueID, *res, *tszProto;

	if ( ai->argc != 1 || ai->fi->hContact == NULL )
		return NULL;

	ai->flags |= AIF_DONTPARSE;
	res = NULL;
	szUniqueID = NULL;
	szProto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)ai->fi->hContact, 0);
	if (szProto != NULL)
		szUniqueID = getContactInfoT(CNF_UNIQUEID, ai->fi->hContact, 1);

	if (szUniqueID == NULL) {
		szProto = PROTOID_HANDLE;
		szUniqueID = itot((int)ai->fi->hContact);
		if ( szProto == NULL || szUniqueID == NULL )
			return NULL;
	}

	res = ( TCHAR* )malloc((strlen(szProto) + _tcslen(szUniqueID) + 4)*sizeof(TCHAR));
	if (res == NULL) {
		free(szUniqueID);
		return NULL;
	}

	#ifdef UNICODE
		tszProto = a2u(szProto);
	#else
		tszProto = _strdup(szProto);
	#endif

	if ( tszProto != NULL && szUniqueID != NULL ) {
		wsprintf(res, _T("<%s:%s>"), tszProto, szUniqueID);
		free(szUniqueID);
		free(tszProto);
	}

	return res;
}

static BOOL isValidDbEvent(DBEVENTINFO *dbe, int flags)
{
	BOOL bEventType, bEventFlags;

	bEventType = ((dbe->eventType == EVENTTYPE_MESSAGE) && (flags&DBE_MESSAGE)) ||
			((dbe->eventType == EVENTTYPE_URL) && (flags&DBE_URL)) ||
			((dbe->eventType == EVENTTYPE_CONTACTS) && (flags&DBE_CONTACTS)) ||
			((dbe->eventType == EVENTTYPE_ADDED) && (flags&DBE_ADDED)) ||
			((dbe->eventType == EVENTTYPE_AUTHREQUEST) && (flags&DBE_AUTHREQUEST)) ||
			((dbe->eventType == EVENTTYPE_FILE) && (flags&DBE_FILE)) ||
			((dbe->eventType == EVENTTYPE_STATUSCHANGE) && (flags&DBE_STATUSCHANGE)) ||
			((flags&DBE_OTHER));
	bEventFlags = (dbe->flags&DBEF_SENT)?(flags&DBE_SENT):(flags&DBE_RCVD);
	bEventFlags = (bEventFlags && ((dbe->flags&DBEF_READ)?(flags&DBE_READ):(flags&DBE_UNREAD)) );

	return (bEventType && bEventFlags);
}

static HANDLE findDbEvent(HANDLE hContact, HANDLE hDbEvent, int flags)
{
	DBEVENTINFO dbe;
	BOOL bEventOk;

	do {
		ZeroMemory(&dbe, sizeof(DBEVENTINFO));
		dbe.cbSize = sizeof(DBEVENTINFO);
		dbe.cbBlob = 0;
		dbe.pBlob = NULL;
		if (hContact != NULL) {
			if ( (flags & DBE_FIRST) && (flags & DBE_UNREAD) ) {
				hDbEvent = (HANDLE)CallService(MS_DB_EVENT_FINDFIRSTUNREAD, (WPARAM)hContact, 0);
				if ( hDbEvent == NULL && (flags & DBE_READ) )
					hDbEvent = (HANDLE)CallService(MS_DB_EVENT_FINDFIRST, (WPARAM)hContact, 0);
			}
			else if (flags & DBE_FIRST)
				hDbEvent = (HANDLE)CallService(MS_DB_EVENT_FINDFIRST, (WPARAM)hContact, 0);
			else if (flags & DBE_LAST)
				hDbEvent = (HANDLE)CallService(MS_DB_EVENT_FINDLAST, (WPARAM)hContact, 0);
			else if (flags & DBE_NEXT)
				hDbEvent = (HANDLE)CallService(MS_DB_EVENT_FINDNEXT, (WPARAM)hDbEvent, 0);
			else if (flags & DBE_PREV)
				hDbEvent = (HANDLE)CallService(MS_DB_EVENT_FINDPREV, (WPARAM)hDbEvent, 0);
		}
		else {
			HANDLE hMatchEvent, hSearchEvent, hSearchContact;
			DWORD matchTimestamp, priorTimestamp;

			hMatchEvent = hSearchEvent = hSearchContact = NULL;
			matchTimestamp = priorTimestamp = 0;
			if (flags & DBE_FIRST) {
				hSearchContact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
				do {
					hSearchEvent = findDbEvent(hSearchContact, NULL, flags);
					dbe.cbBlob = 0;
					if (!CallService(MS_DB_EVENT_GET, (WPARAM)hSearchEvent, (LPARAM)&dbe)) {
						if ((dbe.timestamp < matchTimestamp) || (matchTimestamp == 0)) {
							hMatchEvent = hSearchEvent;
							matchTimestamp = dbe.timestamp;
						}
					}
					hSearchContact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM) hSearchContact, 0);
				} while (hSearchContact);
				hDbEvent = hMatchEvent;
			}
			else if (flags&DBE_LAST) {
				hSearchContact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
				do {
					hSearchEvent = findDbEvent(hSearchContact, NULL, flags);
					dbe.cbBlob = 0;
					if (!CallService(MS_DB_EVENT_GET, (WPARAM)hSearchEvent, (LPARAM)&dbe)) {
						if ((dbe.timestamp > matchTimestamp) || (matchTimestamp == 0)) {
							hMatchEvent = hSearchEvent;
							matchTimestamp = dbe.timestamp;
						}
					}
					hSearchContact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM) hSearchContact, 0);
				} while (hSearchContact);
				hDbEvent = hMatchEvent;
			}
			else if (flags&DBE_NEXT) {
				dbe.cbBlob = 0;
				if (!CallService(MS_DB_EVENT_GET, (WPARAM)hDbEvent, (LPARAM)&dbe)) {
					priorTimestamp = dbe.timestamp;
					hSearchContact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
					do {
						hSearchEvent = findDbEvent(hSearchContact, hDbEvent, flags);
						dbe.cbBlob = 0;
						if (!CallService(MS_DB_EVENT_GET, (WPARAM)hSearchEvent, (LPARAM)&dbe)) {
							if (((dbe.timestamp < matchTimestamp) || (matchTimestamp == 0) ) && (dbe.timestamp > priorTimestamp)) {
								hMatchEvent = hSearchEvent;
								matchTimestamp = dbe.timestamp;
							}
						}
						hSearchContact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM)hSearchContact, 0);
					} while (hSearchContact);
					hDbEvent = hMatchEvent;
				}
			}
			else if (flags&DBE_PREV) {
				if (!CallService(MS_DB_EVENT_GET, (WPARAM)hDbEvent, (LPARAM)&dbe)) {
					priorTimestamp = dbe.timestamp;
					hSearchContact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
					do {
						hSearchEvent = findDbEvent(hSearchContact, hDbEvent, flags);
						dbe.cbBlob = 0;
						if (!CallService(MS_DB_EVENT_GET, (WPARAM)hSearchEvent, (LPARAM)&dbe)) {
							if ( ((dbe.timestamp > matchTimestamp) || (matchTimestamp == 0)) && (dbe.timestamp < priorTimestamp) ) {
								hMatchEvent = hSearchEvent;
								matchTimestamp = dbe.timestamp;
							}
						}
						hSearchContact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM)hSearchContact, 0);
					} while (hSearchContact);
					hDbEvent = hMatchEvent;
				}
			}
		}
		dbe.cbBlob = 0;
		if (CallService(MS_DB_EVENT_GET, (WPARAM)hDbEvent, (LPARAM)&dbe))
			bEventOk = FALSE;
		else
			bEventOk = isValidDbEvent(&dbe, flags);
		if (!bEventOk) {
			if (flags&DBE_FIRST) {
				flags |= DBE_NEXT;
				flags &= ~DBE_FIRST;
			}
			else if (flags&DBE_LAST) {
				flags |= DBE_PREV;
				flags &= ~DBE_LAST;
			}
		}
	} while ( (!bEventOk) && (hDbEvent != NULL) );

	return hDbEvent;
}

// modified from history.c
static TCHAR *GetMessageDescription(DBEVENTINFO *dbei)
{
	TCHAR *tszRes;

	if ( ServiceExists( MS_DB_EVENT_GETTEXT )) {
		// Miranda 0.7
		TCHAR *buf = DbGetEventTextT(dbei, CP_ACP);
		tszRes = _tcsdup(buf);
		mir_free(buf);
	}
	else {
		char *pszSrc = ( char* )dbei->pBlob;
		size_t len = strlen(( char* )dbei->pBlob )+1;
		#if defined( _UNICODE )
			if ( dbei->cbBlob > len ) {
				int len2 = dbei->cbBlob - len;

				tszRes = ( TCHAR* )calloc(len2, 1);
				memcpy( tszRes, &dbei->pBlob[ len ], len2 );
			}
			else {
				char *szRes = ( char* )calloc(len, sizeof(char));
				strncpy( szRes, ( const char* )pszSrc, len );
				tszRes = a2u(szRes);
				free(szRes);
			}
		#else
			tszRes = ( char* )calloc(len, sizeof(char));
			strncpy( tszRes, ( const char* )pszSrc, len );
		#endif
	}

	return tszRes;
}
// end: from history.c

// ?message(%subject%,last|first,sent|recv,read|unread)
static TCHAR *parseDbEvent(ARGUMENTSINFO *ai)
{
	int flags, count;
	HANDLE hContact;
	HANDLE hDbEvent;
	DBEVENTINFO dbe;
	CONTACTSINFO ci;
	TCHAR *res;

	if (ai->argc != 5)
		return NULL;

	flags = DBE_MESSAGE;
	switch (*ai->targv[2]) {
	case _T('f'):
		flags |= DBE_FIRST;
		break;
	default:
		flags |= DBE_LAST;
		break;
	}
	switch (*ai->targv[3]) {
	case _T('s'):
		flags |= DBE_SENT;
		break;
	case _T('r'):
		flags |= DBE_RCVD;
		break;
	default:
		flags |= DBE_RCVD|DBE_SENT;
		break;
	}
	switch (*ai->targv[4]) {
	case _T('r'):
		flags |= DBE_READ;
		break;
	case _T('u'):
		flags |= DBE_UNREAD;
		break;
	default:
		flags |= DBE_READ|DBE_UNREAD;
		break;
	}
	hContact = NULL;
	ZeroMemory(&ci, sizeof(ci));
	ci.cbSize = sizeof(ci);
	ci.tszContact = ai->targv[1];
	ci.flags = 0xFFFFFFFF^(CI_TCHAR==0?CI_UNICODE:0);
	count = getContactFromString( &ci );
	if ( (count == 1) && (ci.hContacts != NULL) ) {
		hContact = ci.hContacts[0];
		free(ci.hContacts);
	}
	else if (ci.hContacts != NULL)
		free(ci.hContacts);

	hDbEvent = findDbEvent(hContact, NULL, flags);
	if (hDbEvent == NULL)
		return NULL;

	ZeroMemory(&dbe, sizeof(DBEVENTINFO));
	dbe.cbSize = sizeof(DBEVENTINFO);
	dbe.cbBlob = CallService(MS_DB_EVENT_GETBLOBSIZE, (WPARAM)hDbEvent, 0);
	dbe.pBlob = ( PBYTE )calloc(dbe.cbBlob, 1);
	if (CallService(MS_DB_EVENT_GET, (WPARAM)hDbEvent, (LPARAM)&dbe)) {
		free(dbe.pBlob);
		return NULL;
	}
	res = GetMessageDescription(&dbe);
	free(dbe.pBlob);

	return res;
}

static TCHAR *parseTranslate(ARGUMENTSINFO *ai)
{
	if (ai->argc != 2)
		return NULL;

	TCHAR* res = TranslateTS(ai->targv[1]);
	return (res == NULL) ? NULL : _tcsdup(res);
}

static TCHAR *parseVersionString(ARGUMENTSINFO *ai) {

	char versionString[128];

	if (ai->argc != 1)
		return NULL;

	ai->flags |= AIF_DONTPARSE;
	if (CallService(MS_SYSTEM_GETVERSIONTEXT, (WPARAM)sizeof(versionString), (LPARAM)versionString))
		return NULL;

	#ifdef UNICODE
		return a2u(versionString);
	#else
		return _strdup(versionString);
	#endif
}

static TCHAR *parseContactNameString(ARGUMENTSINFO *ai)
{
 	TCHAR *ret;

 	if (ai->argc != 1 || ai->fi->hContact == NULL)
 		return NULL;
 
 	ai->flags |= AIF_DONTPARSE;
 	ret = (TCHAR *) CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM) ai->fi->hContact, GCDNF_TCHAR);
 	if (ret == NULL)
 		return NULL;

	return _tcsdup(ret);
}

static TCHAR *parseMirDateString(ARGUMENTSINFO *ai)
{
 	TCHAR ret[128];
 	DBTIMETOSTRINGT tst = {0};

 	if (ai->argc != 1)
 		return NULL;

 	ai->flags |= AIF_DONTPARSE;
 	tst.szFormat = _T("d s");
 	tst.szDest = ret;
 	tst.cbDest = 128;
 	if (CallService(MS_DB_TIME_TIMESTAMPTOSTRINGT, (WPARAM) time(NULL), (LPARAM) &tst))
 		return NULL;

	return _tcsdup(ret);
}

static TCHAR *parseMirandaCoreVar(ARGUMENTSINFO *ai) {

	TCHAR path[MAX_PATH], corevar[MAX_PATH];
	TCHAR* tmpPath;

	if (ai->argc != 1)
		return NULL;

	ai->flags |= AIF_DONTPARSE;

	mir_sntprintf(corevar, MAX_PATH,_T("%%%s%%"), ai->targv[0]);
	tmpPath = Utils_ReplaceVarsT(corevar);
	mir_sntprintf(path, sizeof(path)-1, _T("%s"), tmpPath);
	mir_free(tmpPath);

	return _tcsdup(path);
}

static TCHAR *parseMirSrvExists(ARGUMENTSINFO *ai)
{
	if ( ai->argc != 2 )
		return NULL;

	char* serviceName;
	#ifdef _UNICODE
		serviceName = u2a( ai->targv[1] );
	#else
		serviceName = ai->targv[1];
	#endif

	if ( !ServiceExists( serviceName ))
		ai->flags |= AIF_FALSE;

	#ifdef _UNICODE
		free( serviceName );
	#endif

	return _tcsdup(_T(""));
}

int registerMirandaTokens() {
	if (ServiceExists(MS_UTILS_REPLACEVARS)) {
		// global vars
		registerIntToken(_T("miranda_path"),		parseMirandaCoreVar	, TRF_FIELD, "Miranda Core Global\tpath to root miranda folder");
		registerIntToken(_T("miranda_profile"),		parseMirandaCoreVar	, TRF_FIELD, "Miranda Core Global\tpath to current miranda profile");
		registerIntToken(_T("miranda_profilename"), parseMirandaCoreVar	, TRF_FIELD, "Miranda Core Global\tname of current miranda profile (filename, without extension)");
		registerIntToken(_T("miranda_userdata"),	parseMirandaCoreVar	, TRF_FIELD, "Miranda Core Global\twill return parsed string %miranda_profile%\\Profiles\\%miranda_profilename%");
		registerIntToken(_T("miranda_avatarcache"), parseMirandaCoreVar	, TRF_FIELD, "Miranda Core Global\twill return parsed string %miranda_profile%\\Profiles\\%miranda_profilename%\\AvatarCache");
		registerIntToken(_T("miranda_logpath"),		parseMirandaCoreVar	, TRF_FIELD, "Miranda Core Global\twill return parsed string %miranda_profile%\\Profiles\\%miranda_profilename%\\Logs");

		// OS vars
		registerIntToken(_T("appdata"),				parseMirandaCoreVar	, TRF_FIELD, "Miranda Core OS\tsame as environment variable %APPDATA% for currently logged-on Windows user");
		registerIntToken(_T("username"),			parseMirandaCoreVar	, TRF_FIELD, "Miranda Core OS\tusername for currently logged-on Windows user");
		registerIntToken(_T("mydocuments"),			parseMirandaCoreVar	, TRF_FIELD, "Miranda Core OS\t\"My Documents\" folder for currently logged-on Windows user");
		registerIntToken(_T("desktop"),				parseMirandaCoreVar	, TRF_FIELD, "Miranda Core OS\t\"Desktop\" folder for currently logged-on Windows user");
	}
	registerIntToken(_T(CODETOSTATUS), parseCodeToStatus, TRF_FUNCTION, "Miranda Related\t(x)\ttranslates status code x into a status description");
	registerIntToken(_T(CONTACT), parseContact, TRF_FUNCTION, "Miranda Related\t(x,y,z)\tzth contact with property y described by x, example: (unregistered,nick) (z is optional)");
	registerIntToken(_T(CONTACTCOUNT), parseContactCount, TRF_FUNCTION, "Miranda Related\t(x,y)\tnumber of contacts with property y described by x, example: (unregistered,nick)");
	registerIntToken(_T(CONTACTINFO), parseContactInfo, TRF_FUNCTION, "Miranda Related\t(x,y)\tinfo property y of contact x");
	registerIntToken(_T(DBPROFILENAME), parseDBProfileName, TRF_FIELD, "Miranda Related\tdb profile name");
	registerIntToken(_T(DBPROFILEPATH), parseDBProfilePath, TRF_FIELD, "Miranda Related\tdb profile path");
	registerIntToken(_T(DBSETTING), parseDBSetting, TRF_FUNCTION, "Miranda Related\t(x,y,z,w)\tdb setting z of module y of contact x and return w if z isn't exist (w is optional)");
	registerIntToken(_T(DBEVENT), parseDbEvent, TRF_FUNCTION, "Miranda Related\t(x,y,z,w)\tget event for contact x (optional), according to y,z,w, see documentation");
	registerIntToken(_T(LSTIME), parseLastSeenTime, TRF_FUNCTION, "Miranda Related\t(x,y)\tget last seen time of contact x in format y (y is optional)");
	registerIntToken(_T(LSDATE), parseLastSeenDate, TRF_FUNCTION, "Miranda Related\t(x,y)\tget last seen date of contact x in format y (y is optional)");
	registerIntToken(_T(LSSTATUS), parseLastSeenStatus, TRF_FUNCTION, "Miranda Related\t(x)\tget last seen status of contact x");
	registerIntToken(_T(MIRANDAPATH), parseMirandaPath, TRF_FIELD, "Miranda Related\tpath to the Miranda-IM executable");
	registerIntToken(_T(MYSTATUS), parseMyStatus, TRF_FUNCTION, "Miranda Related\t(x)\tcurrent status description of protocol x (without x, the global status is retrieved)");
	registerIntToken(_T(PROTOINFO), parseProtoInfo, TRF_FUNCTION, "Miranda Related\t(x,y)\tinfo property y of protocol id x");
	registerIntToken(_T(SUBJECT), parseSpecialContact, TRF_FIELD, "Miranda Related\tretrieves the subject, depending on situation");
	registerIntToken(_T(TRANSLATE), parseTranslate, TRF_FUNCTION, "Miranda Related\t(x)\ttranslates x");
	registerIntToken(_T(VERSIONSTRING), parseVersionString, TRF_FIELD, "Miranda Related\tget the version of Miranda");
 	registerIntToken(_T(CONTACT_NAME), parseContactNameString, TRF_FIELD, "Miranda Related\tget the contact display name");
 	registerIntToken(_T(MIR_DATE), parseMirDateString, TRF_FIELD, "Miranda Related\tget the date and time (using Miranda format)");
	registerIntToken(_T(SRVEXISTS), parseMirSrvExists,	TRF_FUNCTION, "Miranda Related\t(x)\tTRUE if service function is exists");

	return 0;
}
