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

static TCHAR* parseCodeToStatus(ARGUMENTSINFO *ai)
{
	if (ai->argc != 2)
		return NULL;

	unsigned int status = ttoi(ai->targv[1]);
	TCHAR *szStatus = (TCHAR*)CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, (WPARAM)status, GSMDF_TCHAR);
	if (szStatus != NULL)
		return mir_tstrdup(szStatus);

	return NULL;
}

static int getContactInfoFlags(TCHAR *tszDesc)
{
	int flags = 0;
	for (TCHAR *cur = tszDesc; (cur < (tszDesc + _tcslen(tszDesc))); cur++) {
		if (!_tcsnicmp(cur, STR_PROTOID, _tcslen(STR_PROTOID))) {
			flags |= CI_PROTOID;
			cur += _tcslen(STR_PROTOID) - 1;
		}
		else if (!_tcsnicmp(cur, STR_NICK, _tcslen(STR_NICK))) {
			flags |= CI_NICK;
			cur += _tcslen(STR_NICK) - 1;
		}
		else if (!_tcsnicmp(cur, STR_FIRSTNAME, _tcslen(STR_FIRSTNAME))) {
			flags |= CI_FIRSTNAME;
			cur += _tcslen(STR_FIRSTNAME) - 1;
		}
		else if (!_tcsnicmp(cur, STR_LASTNAME, _tcslen(STR_LASTNAME))) {
			flags |= CI_LASTNAME;
			cur += _tcslen(STR_LASTNAME) - 1;
		}
		else if (!_tcsnicmp(cur, STR_DISPLAY, _tcslen(STR_DISPLAY))) {
			flags |= CI_LISTNAME;
			cur += _tcslen(STR_DISPLAY) - 1;
		}
		else if (!_tcsnicmp(cur, STR_EMAIL, _tcslen(STR_EMAIL))) {
			flags |= CI_EMAIL;
			cur += _tcslen(STR_EMAIL) - 1;
		}
		else if (!_tcsnicmp(cur, STR_UNIQUEID, _tcslen(STR_UNIQUEID))) {
			flags |= CI_UNIQUEID;
			cur += _tcslen(STR_UNIQUEID) - 1;
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

static TCHAR* parseContact(ARGUMENTSINFO *ai)
{
	if (ai->argc < 3 || ai->argc > 4)
		return NULL;

	int n = 0;
	if (ai->argc == 4 && *ai->targv[3] != 'r')
		n = ttoi(ai->targv[3]) - 1;

	CONTACTSINFO ci = { 0 };
	ci.cbSize = sizeof(ci);
	ci.tszContact = ai->targv[1];
	ci.flags = getContactInfoFlags(ai->targv[2]);
	int count = getContactFromString(&ci);
	if (count == 0 || ci.hContacts == NULL)
		return NULL;

	if (ai->argc == 4 && *ai->targv[3] == 'r')
		n = rand() % count;

	if (count != 1 && ai->argc != 4) {
		mir_free(ci.hContacts);
		return NULL;
	}
	MCONTACT hContact = ci.hContacts[n];
	log_debugA("contact: %x", hContact);
	mir_free(ci.hContacts);

	return encodeContactToString(hContact);
}

static TCHAR* parseContactCount(ARGUMENTSINFO *ai)
{
	if (ai->argc != 3)
		return NULL;

	CONTACTSINFO ci = { 0 };
	ci.cbSize = sizeof(ci);
	ci.tszContact = ai->targv[1];
	ci.flags = getContactInfoFlags(ai->targv[2]);
	int count = getContactFromString(&ci);
	if (count != 0 && ci.hContacts != NULL)
		mir_free(ci.hContacts);

	return itot(count);
}

static TCHAR* parseContactInfo(ARGUMENTSINFO *ai)
{
	if (ai->argc != 3)
		return NULL;

	MCONTACT hContact = NULL;
	CONTACTSINFO ci = { 0 };
	ci.cbSize = sizeof(ci);
	ci.tszContact = ai->targv[1];
	ci.flags = 0xFFFFFFFF ^ (CI_TCHAR == 0 ? CI_UNICODE : 0);
	int count = getContactFromString(&ci);
	if (count == 1 && ci.hContacts != NULL) {
		hContact = ci.hContacts[0];
		mir_free(ci.hContacts);
	}
	else {
		mir_free(ci.hContacts);
		return NULL;
	}
	BYTE type = getContactInfoType(ai->targv[2]);
	if (type == 0)
		return NULL;

	return getContactInfoT(type, hContact);
}

static TCHAR* parseDBProfileName(ARGUMENTSINFO *ai)
{
	if (ai->argc != 1)
		return NULL;

	TCHAR name[MAX_PATH];
	if (CallService(MS_DB_GETPROFILENAMET, SIZEOF(name), (LPARAM)name))
		return NULL;

	return mir_tstrdup(name);
}

static TCHAR* parseDBProfilePath(ARGUMENTSINFO *ai)
{
	if (ai->argc != 1)
		return NULL;

	TCHAR path[MAX_PATH];
	if (CallService(MS_DB_GETPROFILEPATHT, SIZEOF(path), (LPARAM)path))
		return NULL;

	return mir_tstrdup(path);
}

static TCHAR* getDBSetting(MCONTACT hContact, char* module, char* setting, TCHAR* defaultValue)
{
	DBVARIANT dbv;
	if (db_get_s(hContact, module, setting, &dbv, 0))
		return defaultValue;

	TCHAR* var = NULL;
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
		var = mir_a2t(dbv.pszVal);
		break;
	case DBVT_WCHAR:
		var = mir_wstrdup(dbv.pwszVal);
		break;
	case DBVT_UTF8:
		Utf8Decode(dbv.pszVal, &var);
		break;
	}

	db_free(&dbv);
	return var;
}

static TCHAR* parseDBSetting(ARGUMENTSINFO *ai)
{
	if (ai->argc < 4)
		return NULL;

	TCHAR *res = NULL, *szDefaultValue = NULL;
	MCONTACT hContact = NULL;

	if (_tcslen(ai->targv[1]) > 0) {
		CONTACTSINFO ci = { 0 };
		ci.cbSize = sizeof(ci);
		ci.tszContact = ai->targv[1];
		ci.flags = 0xFFFFFFFF ^ (CI_TCHAR == 0 ? CI_UNICODE : 0);
		int count = getContactFromString(&ci);
		if (count == 1 && ci.hContacts != NULL) {
			hContact = ci.hContacts[0];
			mir_free(ci.hContacts);
		}
		else {
			mir_free(ci.hContacts);
			return NULL;
		}
	}

	char *szModule = mir_t2a(ai->targv[2]);
	char *szSetting = mir_t2a(ai->targv[3]);

	if (ai->argc > 4 && _tcslen(ai->targv[4]) > 0)
		szDefaultValue = mir_tstrdup(ai->targv[4]);

	if (szModule != NULL && szSetting != NULL) {
		res = getDBSetting(hContact, szModule, szSetting, szDefaultValue);
		mir_free(szModule);
		mir_free(szSetting);
	}
	return res;
}

static TCHAR* parseLastSeenDate(ARGUMENTSINFO *ai)
{
	if (ai->argc <= 1)
		return NULL;

	MCONTACT hContact = NULL;
	CONTACTSINFO ci = { 0 };
	ci.cbSize = sizeof(ci);
	ci.tszContact = ai->targv[1];
	ci.flags = 0xFFFFFFFF ^ (CI_TCHAR == 0 ? CI_UNICODE : 0);
	int count = getContactFromString(&ci);
	if (count == 1 && ci.hContacts != NULL) {
		hContact = ci.hContacts[0];
		mir_free(ci.hContacts);
	}
	else {
		mir_free(ci.hContacts);
		return NULL;
	}

	TCHAR *szFormat;
	if (ai->argc == 2 || (ai->argc > 2 && _tcslen(ai->targv[2]) == 0))
		szFormat = NULL;
	else
		szFormat = ai->targv[2];

	SYSTEMTIME lsTime = { 0 };
	char *szModule = SEEN_MODULE;
	lsTime.wYear = db_get_w(hContact, szModule, "Year", 0);
	if (lsTime.wYear == 0)
		return NULL;

	lsTime.wMilliseconds = 0;
	lsTime.wSecond = db_get_w(hContact, szModule, "Seconds", 0);
	lsTime.wMinute = db_get_w(hContact, szModule, "Minutes", 0);
	lsTime.wHour = db_get_w(hContact, szModule, "Hours", 0);
	lsTime.wDay = db_get_w(hContact, szModule, "Day", 0);
	lsTime.wDayOfWeek = db_get_w(hContact, szModule, "WeekDay", 0);
	lsTime.wMonth = db_get_w(hContact, szModule, "Month", 0);

	int len = GetDateFormat(LOCALE_USER_DEFAULT, 0, &lsTime, szFormat, NULL, 0);
	TCHAR *res = (TCHAR*)mir_alloc((len + 1)*sizeof(TCHAR));
	if (res == NULL)
		return NULL;

	if (GetDateFormat(LOCALE_USER_DEFAULT, 0, &lsTime, szFormat, res, len) == 0) {
		mir_free(res);
		return NULL;
	}

	return res;
}

static TCHAR* parseLastSeenTime(ARGUMENTSINFO *ai)
{
	if (ai->argc <= 1)
		return NULL;

	MCONTACT hContact = NULL;

	CONTACTSINFO ci = { 0 };
	ci.cbSize = sizeof(ci);
	ci.tszContact = ai->targv[1];
	ci.flags = 0xFFFFFFFF ^ (CI_TCHAR == 0 ? CI_UNICODE : 0);
	int count = getContactFromString(&ci);
	if (count == 1 && ci.hContacts != NULL) {
		hContact = ci.hContacts[0];
		mir_free(ci.hContacts);
	}
	else {
		mir_free(ci.hContacts);
		return NULL;
	}

	TCHAR *szFormat;
	if (ai->argc == 2 || (ai->argc > 2 && _tcslen(ai->targv[2]) == 0))
		szFormat = NULL;
	else
		szFormat = ai->targv[2];

	SYSTEMTIME lsTime = { 0 };
	char *szModule = SEEN_MODULE;
	lsTime.wYear = db_get_w(hContact, szModule, "Year", 0);
	if (lsTime.wYear == 0)
		return NULL;

	lsTime.wMilliseconds = 0;
	lsTime.wSecond = db_get_w(hContact, szModule, "Seconds", 0);
	lsTime.wMinute = db_get_w(hContact, szModule, "Minutes", 0);
	lsTime.wHour = db_get_w(hContact, szModule, "Hours", 0);
	lsTime.wDay = db_get_w(hContact, szModule, "Day", 0);
	lsTime.wDayOfWeek = db_get_w(hContact, szModule, "WeekDay", 0);
	lsTime.wMonth = db_get_w(hContact, szModule, "Month", 0);
	lsTime.wYear = db_get_w(hContact, szModule, "Year", 0);

	int len = GetTimeFormat(LOCALE_USER_DEFAULT, 0, &lsTime, szFormat, NULL, 0);
	TCHAR *res = (TCHAR*)mir_alloc((len + 1)*sizeof(TCHAR));
	if (res == NULL)
		return NULL;

	if (GetTimeFormat(LOCALE_USER_DEFAULT, 0, &lsTime, szFormat, res, len) == 0) {
		mir_free(res);
		return NULL;
	}

	return res;
}

static TCHAR* parseLastSeenStatus(ARGUMENTSINFO *ai)
{
	if (ai->argc != 2)
		return NULL;

	MCONTACT hContact = NULL;
	CONTACTSINFO ci = { 0 };
	ci.cbSize = sizeof(ci);
	ci.tszContact = ai->targv[1];
	ci.flags = 0xFFFFFFFF ^ (CI_TCHAR == 0 ? CI_UNICODE : 0);
	int count = getContactFromString(&ci);
	if ((count == 1) && (ci.hContacts != NULL)) {
		hContact = ci.hContacts[0];
		mir_free(ci.hContacts);
	}
	else {
		mir_free(ci.hContacts);
		return NULL;
	}
	char *szModule = SEEN_MODULE;
	int status = db_get_w(hContact, szModule, "Status", 0);
	if (status == 0)
		return NULL;

	TCHAR *szStatus = (TCHAR*)CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, (WPARAM)status, GSMDF_TCHAR);
	if (szStatus != NULL)
		return mir_tstrdup(szStatus);

	return NULL;
}

static TCHAR* parseMirandaPath(ARGUMENTSINFO *ai)
{
	if (ai->argc != 1)
		return NULL;

	ai->flags |= AIF_DONTPARSE;
	TCHAR path[MAX_PATH];
	if (GetModuleFileName(NULL, path, SIZEOF(path)) == 0)
		return NULL;

	return mir_tstrdup(path);
}

static TCHAR* parseMyStatus(ARGUMENTSINFO *ai)
{
	if (ai->argc > 2)
		return NULL;

	int status;
	if (ai->argc == 1 || _tcslen(ai->targv[1]) == 0)
		status = CallService(MS_CLIST_GETSTATUSMODE, 0, 0);
	else
		status = CallProtoService(_T2A(ai->targv[1]), PS_GETSTATUS, 0, 0);

	TCHAR *szStatus = (TCHAR*)CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, (WPARAM)status, GSMDF_TCHAR);
	return (szStatus != NULL) ? mir_tstrdup(szStatus) : NULL;
}

static TCHAR* parseProtoInfo(ARGUMENTSINFO *ai)
{
	if (ai->argc != 3)
		return NULL;

	char *szRes = NULL;
	TCHAR *tszRes = NULL;
	char *szProto = mir_t2a(ai->targv[1]);

	if (!_tcscmp(ai->targv[2], _T(STR_PINAME)))
		tszRes = Hlp_GetProtocolName(szProto);
	else if (!_tcscmp(ai->targv[2], _T(STR_PIUIDTEXT))) {
		if (!ProtoServiceExists(szProto, PS_GETCAPS))
			return NULL;

		char *szText = (char *)CallProtoService(szProto, PS_GETCAPS, (WPARAM)PFLAG_UNIQUEIDTEXT, 0);
		if (szText != NULL)
			szRes = _strdup(szText);
	}
	else if (!_tcscmp(ai->targv[2], _T(STR_PIUIDSETTING))) {
		if (!ProtoServiceExists(szProto, PS_GETCAPS))
			return NULL;

		char *szText = (char *)CallProtoService(szProto, PS_GETCAPS, (WPARAM)PFLAG_UNIQUEIDSETTING, 0);
		if (szText != NULL)
			szRes = _strdup(szText);
	}
	else if (!_tcscmp(ai->targv[2], _T(STR_PINICK)))
	{
		CONTACTINFO ci;

		ci.cbSize = sizeof(CONTACTINFO);
		ci.dwFlag = CNF_DISPLAY | CNF_UNICODE;
		ci.hContact = NULL;
		ci.szProto = szProto;
		CallService(MS_CONTACT_GETCONTACTINFO, 0, (LPARAM)&ci);

		tszRes = mir_tstrdup(ci.pszVal);

		mir_free(ci.pszVal);
	}
	mir_free(szProto);
	if (szRes == NULL && tszRes == NULL)
		return NULL;

	if (szRes != NULL && tszRes == NULL) {
		tszRes = mir_a2t(szRes);
		mir_free(szRes);
	}
	else if (szRes != NULL && tszRes != NULL)
		mir_free(szRes);

	return tszRes;
}

static TCHAR* parseSpecialContact(ARGUMENTSINFO *ai)
{
	if (ai->argc != 1 || ai->fi->hContact == NULL)
		return NULL;

	ai->flags |= AIF_DONTPARSE;
	ptrT szUniqueID;
	char *szProto = GetContactProto(ai->fi->hContact);
	if (szProto != NULL)
		szUniqueID = getContactInfoT(CNF_UNIQUEID, ai->fi->hContact);

	if (szUniqueID == NULL) {
		szProto = PROTOID_HANDLE;
		szUniqueID = (TCHAR*)mir_alloc(40);
		if (szUniqueID == NULL)
			return NULL;
		mir_sntprintf(szUniqueID, 20, _T("%p"), ai->fi->hContact);
	}

	if (szUniqueID == NULL)
		return NULL;

	size_t size = strlen(szProto) + _tcslen(szUniqueID) + 4;
	TCHAR *res = (TCHAR*)mir_alloc(size * sizeof(TCHAR));
	if (res == NULL)
		return NULL;

	mir_sntprintf(res, size, _T("<%S:%s>"), szProto, szUniqueID);
	return res;
}

static BOOL isValidDbEvent(DBEVENTINFO *dbe, int flags)
{
	BOOL bEventType, bEventFlags;

	bEventType = ((dbe->eventType == EVENTTYPE_MESSAGE) && (flags & DBE_MESSAGE)) ||
		((dbe->eventType == EVENTTYPE_URL) && (flags & DBE_URL)) ||
		((dbe->eventType == EVENTTYPE_CONTACTS) && (flags & DBE_CONTACTS)) ||
		((dbe->eventType == EVENTTYPE_ADDED) && (flags & DBE_ADDED)) ||
		((dbe->eventType == EVENTTYPE_AUTHREQUEST) && (flags & DBE_AUTHREQUEST)) ||
		((dbe->eventType == EVENTTYPE_FILE) && (flags & DBE_FILE)) ||
		((flags & DBE_OTHER));
	bEventFlags = (dbe->flags&DBEF_SENT) ? (flags&DBE_SENT) : (flags&DBE_RCVD);
	bEventFlags = (bEventFlags && ((dbe->flags&DBEF_READ) ? (flags&DBE_READ) : (flags&DBE_UNREAD)));

	return (bEventType && bEventFlags);
}

static HANDLE findDbEvent(MCONTACT hContact, HANDLE hDbEvent, int flags)
{
	DBEVENTINFO dbe;
	BOOL bEventOk;

	do {
		ZeroMemory(&dbe, sizeof(DBEVENTINFO));
		dbe.cbSize = sizeof(DBEVENTINFO);
		dbe.cbBlob = 0;
		dbe.pBlob = NULL;
		if (hContact != NULL) {
			if ((flags & DBE_FIRST) && (flags & DBE_UNREAD)) {
				hDbEvent = db_event_firstUnread(hContact);
				if (hDbEvent == NULL && (flags & DBE_READ))
					hDbEvent = db_event_first(hContact);
			}
			else if (flags & DBE_FIRST)
				hDbEvent = db_event_first(hContact);
			else if (flags & DBE_LAST)
				hDbEvent = db_event_last(hContact);
			else if (flags & DBE_NEXT)
				hDbEvent = db_event_next(hContact, hDbEvent);
			else if (flags & DBE_PREV)
				hDbEvent = db_event_prev(hContact, hDbEvent);
		}
		else {
			HANDLE hMatchEvent, hSearchEvent;
			DWORD matchTimestamp, priorTimestamp;

			hMatchEvent = hSearchEvent = NULL;
			matchTimestamp = priorTimestamp = 0;
			if (flags & DBE_FIRST) {
				for (MCONTACT hSearchContact = db_find_first(); hSearchContact; hSearchContact = db_find_next(hSearchContact)) {
					hSearchEvent = findDbEvent(hSearchContact, NULL, flags);
					dbe.cbBlob = 0;
					if (!db_event_get(hSearchEvent, &dbe)) {
						if ((dbe.timestamp < matchTimestamp) || (matchTimestamp == 0)) {
							hMatchEvent = hSearchEvent;
							matchTimestamp = dbe.timestamp;
						}
					}
				}
				hDbEvent = hMatchEvent;
			}
			else if (flags & DBE_LAST) {
				for (MCONTACT hSearchContact = db_find_first(); hSearchContact; hSearchContact = db_find_next(hSearchContact)) {
					hSearchEvent = findDbEvent(hSearchContact, NULL, flags);
					dbe.cbBlob = 0;
					if (!db_event_get(hSearchEvent, &dbe)) {
						if ((dbe.timestamp > matchTimestamp) || (matchTimestamp == 0)) {
							hMatchEvent = hSearchEvent;
							matchTimestamp = dbe.timestamp;
						}
					}
				}
				hDbEvent = hMatchEvent;
			}
			else if (flags & DBE_NEXT) {
				dbe.cbBlob = 0;
				if (!db_event_get(hDbEvent, &dbe)) {
					priorTimestamp = dbe.timestamp;
					for (MCONTACT hSearchContact = db_find_first(); hSearchContact; hSearchContact = db_find_next(hSearchContact)) {
						hSearchEvent = findDbEvent(hSearchContact, hDbEvent, flags);
						dbe.cbBlob = 0;
						if (!db_event_get(hSearchEvent, &dbe)) {
							if (((dbe.timestamp < matchTimestamp) || (matchTimestamp == 0)) && (dbe.timestamp > priorTimestamp)) {
								hMatchEvent = hSearchEvent;
								matchTimestamp = dbe.timestamp;
							}
						}
					}
					hDbEvent = hMatchEvent;
				}
			}
			else if (flags & DBE_PREV) {
				if (!db_event_get(hDbEvent, &dbe)) {
					priorTimestamp = dbe.timestamp;
					for (MCONTACT hSearchContact = db_find_first(); hSearchContact; hSearchContact = db_find_next(hSearchContact)) {
						hSearchEvent = findDbEvent(hSearchContact, hDbEvent, flags);
						dbe.cbBlob = 0;
						if (!db_event_get(hSearchEvent, &dbe)) {
							if (((dbe.timestamp > matchTimestamp) || (matchTimestamp == 0)) && (dbe.timestamp < priorTimestamp)) {
								hMatchEvent = hSearchEvent;
								matchTimestamp = dbe.timestamp;
							}
						}
					}
					hDbEvent = hMatchEvent;
				}
			}
		}
		dbe.cbBlob = 0;
		if (db_event_get(hDbEvent, &dbe))
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
	}
	while ((!bEventOk) && (hDbEvent != NULL));

	return hDbEvent;
}

// ?message(%subject%,last|first,sent|recv,read|unread)
static TCHAR* parseDbEvent(ARGUMENTSINFO *ai)
{
	if (ai->argc != 5)
		return NULL;

	int flags = DBE_MESSAGE;
	switch (*ai->targv[2]) {
	case 'f':
		flags |= DBE_FIRST;
		break;
	default:
		flags |= DBE_LAST;
		break;
	}
	switch (*ai->targv[3]) {
	case 's':
		flags |= DBE_SENT;
		break;
	case 'r':
		flags |= DBE_RCVD;
		break;
	default:
		flags |= DBE_RCVD | DBE_SENT;
		break;
	}
	switch (*ai->targv[4]) {
	case 'r':
		flags |= DBE_READ;
		break;
	case 'u':
		flags |= DBE_UNREAD;
		break;
	default:
		flags |= DBE_READ | DBE_UNREAD;
		break;
	}

	MCONTACT hContact = NULL;

	CONTACTSINFO ci = { 0 };
	ci.cbSize = sizeof(ci);
	ci.tszContact = ai->targv[1];
	ci.flags = 0xFFFFFFFF ^ (CI_TCHAR == 0 ? CI_UNICODE : 0);
	int count = getContactFromString(&ci);
	if ((count == 1) && (ci.hContacts != NULL)) {
		hContact = ci.hContacts[0];
		mir_free(ci.hContacts);
	}
	else if (ci.hContacts != NULL)
		mir_free(ci.hContacts);

	HANDLE hDbEvent = findDbEvent(hContact, NULL, flags);
	if (hDbEvent == NULL)
		return NULL;

	DBEVENTINFO dbe = { sizeof(dbe) };
	dbe.cbBlob = db_event_getBlobSize(hDbEvent);
	dbe.pBlob = (PBYTE)mir_calloc(dbe.cbBlob);
	if (db_event_get(hDbEvent, &dbe)) {
		mir_free(dbe.pBlob);
		return NULL;
	}

	TCHAR *res = DbGetEventTextT(&dbe, CP_ACP);
	mir_free(dbe.pBlob);
	return res;
}

static TCHAR* parseTranslate(ARGUMENTSINFO *ai)
{
	if (ai->argc != 2)
		return NULL;

	TCHAR *res = TranslateTS(ai->targv[1]);
	return (res == NULL) ? NULL : mir_tstrdup(res);
}

static TCHAR* parseVersionString(ARGUMENTSINFO *ai)
{
	if (ai->argc != 1)
		return NULL;

	ai->flags |= AIF_DONTPARSE;
	char versionString[128];
	if (CallService(MS_SYSTEM_GETVERSIONTEXT, (WPARAM)sizeof(versionString), (LPARAM)versionString))
		return NULL;

	return mir_a2t(versionString);
}

static TCHAR *parseContactNameString(ARGUMENTSINFO *ai)
{
	if (ai->argc != 1 || ai->fi->hContact == NULL)
		return NULL;

	ai->flags |= AIF_DONTPARSE;
	TCHAR *ret = (TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)ai->fi->hContact, GCDNF_TCHAR);
	return (ret == NULL) ? NULL : mir_tstrdup(ret);
}

static TCHAR *parseMirDateString(ARGUMENTSINFO *ai)
{
	if (ai->argc != 1)
		return NULL;

	ai->flags |= AIF_DONTPARSE;

	TCHAR ret[128];
	DBTIMETOSTRINGT tst = { 0 };
	tst.szFormat = _T("d s");
	tst.szDest = ret;
	tst.cbDest = 128;
	if (CallService(MS_DB_TIME_TIMESTAMPTOSTRINGT, (WPARAM)time(NULL), (LPARAM)&tst))
		return NULL;

	return mir_tstrdup(ret);
}

static TCHAR *parseMirandaCoreVar(ARGUMENTSINFO *ai)
{
	if (ai->argc != 1)
		return NULL;

	ai->flags |= AIF_DONTPARSE;

	TCHAR corevar[MAX_PATH];
	mir_sntprintf(corevar, MAX_PATH, _T("%%%s%%"), ai->targv[0]);
	return Utils_ReplaceVarsT(corevar);
}

static TCHAR *parseMirSrvExists(ARGUMENTSINFO *ai)
{
	if (ai->argc != 2)
		return NULL;

	if (!ServiceExists(_T2A(ai->targv[1])))
		ai->flags |= AIF_FALSE;

	return mir_tstrdup(_T(""));
}

void registerMirandaTokens()
{
	// global vars
	registerIntToken(_T("miranda_path"), parseMirandaCoreVar, TRF_FIELD, LPGEN("Miranda Core Global")"\t"LPGEN("path to Miranda root folder"));
	registerIntToken(_T("miranda_profilesdir"), parseMirandaCoreVar, TRF_FIELD, LPGEN("Miranda Core Global")"\t"LPGEN("path to folder containing Miranda profiles"));
	registerIntToken(_T("miranda_profilename"), parseMirandaCoreVar, TRF_FIELD, LPGEN("Miranda Core Global")"\t"LPGEN("name of current Miranda profile (filename, without extension)"));
	registerIntToken(_T("miranda_userdata"), parseMirandaCoreVar, TRF_FIELD, LPGEN("Miranda Core Global")"\t"LPGEN("will return parsed string %miranda_profilesdir%\\%miranda_profilename%"));
	registerIntToken(_T("miranda_avatarcache"), parseMirandaCoreVar, TRF_FIELD, LPGEN("Miranda Core Global")"\t"LPGEN("will return parsed string %miranda_profilesdir%\\%miranda_profilename%\\AvatarCache"));
	registerIntToken(_T("miranda_logpath"), parseMirandaCoreVar, TRF_FIELD, LPGEN("Miranda Core Global")"\t"LPGEN("will return parsed string %miranda_profilesdir%\\%miranda_profilename%\\Logs"));

	// OS vars
	registerIntToken(_T("appdata"), parseMirandaCoreVar, TRF_FIELD, LPGEN("Miranda Core OS")"\t"LPGEN("same as environment variable %APPDATA% for currently logged-on Windows user"));
	registerIntToken(_T("username"), parseMirandaCoreVar, TRF_FIELD, LPGEN("Miranda Core OS")"\t"LPGEN("username for currently logged-on Windows user"));
	registerIntToken(_T("mydocuments"), parseMirandaCoreVar, TRF_FIELD, LPGEN("Miranda Core OS")"\t"LPGEN("\"My Documents\" folder for currently logged-on Windows user"));
	registerIntToken(_T("desktop"), parseMirandaCoreVar, TRF_FIELD, LPGEN("Miranda Core OS")"\t"LPGEN("\"Desktop\" folder for currently logged-on Windows user"));

	registerIntToken(CODETOSTATUS, parseCodeToStatus, TRF_FUNCTION, LPGEN("Miranda Related")"\t(x)\t"LPGEN("translates status code x into a status description"));
	registerIntToken(CONTACT, parseContact, TRF_FUNCTION, LPGEN("Miranda Related")"\t(x,y,z)\t"LPGEN("zth contact with property y described by x, example: (unregistered,nick) (z is optional)"));
	registerIntToken(CONTACTCOUNT, parseContactCount, TRF_FUNCTION, LPGEN("Miranda Related")"\t(x,y)\t"LPGEN("number of contacts with property y described by x, example: (unregistered,nick)"));
	registerIntToken(MIR_CONTACTINFO, parseContactInfo, TRF_FUNCTION, LPGEN("Miranda Related")"\t(x,y)\t"LPGEN("info property y of contact x"));
	registerIntToken(DBPROFILENAME, parseDBProfileName, TRF_FIELD, LPGEN("Miranda Related")"\t"LPGEN("database profile name"));
	registerIntToken(DBPROFILEPATH, parseDBProfilePath, TRF_FIELD, LPGEN("Miranda Related")"\t"LPGEN("database profile path"));
	registerIntToken(DBSETTING, parseDBSetting, TRF_FUNCTION, LPGEN("Miranda Related")"\t(x,y,z,w)\t"LPGEN("database setting z of module y of contact x and return w if z doesn't exist (w is optional)"));
	registerIntToken(DBEVENT, parseDbEvent, TRF_FUNCTION, LPGEN("Miranda Related")"\t(x,y,z,w)\t"LPGEN("get event for contact x (optional), according to y,z,w, see documentation"));
	registerIntToken(LSTIME, parseLastSeenTime, TRF_FUNCTION, LPGEN("Miranda Related")"\t(x,y)\t"LPGEN("get last seen time of contact x in format y (y is optional)"));
	registerIntToken(LSDATE, parseLastSeenDate, TRF_FUNCTION, LPGEN("Miranda Related")"\t(x,y)\t"LPGEN("get last seen date of contact x in format y (y is optional)"));
	registerIntToken(LSSTATUS, parseLastSeenStatus, TRF_FUNCTION, LPGEN("Miranda Related")"\t(x)\t"LPGEN("get last seen status of contact x"));
	registerIntToken(MIRANDAPATH, parseMirandaPath, TRF_FIELD, LPGEN("Miranda Related")"\t"LPGEN("path to the Miranda NG executable"));
	registerIntToken(MYSTATUS, parseMyStatus, TRF_FUNCTION, LPGEN("Miranda Related")"\t(x)\t"LPGEN("current status description of protocol x (without x, the global status is retrieved)"));
	registerIntToken(PROTOINFO, parseProtoInfo, TRF_FUNCTION, LPGEN("Miranda Related")"\t(x,y)\t"LPGEN("info property y of protocol ID x"));
	registerIntToken(SUBJECT, parseSpecialContact, TRF_FIELD, LPGEN("Miranda Related")"\t"LPGEN("retrieves the subject, depending on situation"));
	registerIntToken(TRANSLATE, parseTranslate, TRF_FUNCTION, LPGEN("Miranda Related")"\t(x)\t"LPGEN("translates x"));
	registerIntToken(VERSIONSTRING, parseVersionString, TRF_FIELD, LPGEN("Miranda Related")"\t"LPGEN("get the version of Miranda"));
	registerIntToken(CONTACT_NAME, parseContactNameString, TRF_FIELD, LPGEN("Miranda Related")"\t"LPGEN("get the contact display name"));
	registerIntToken(MIR_DATE, parseMirDateString, TRF_FIELD, LPGEN("Miranda Related")"\t"LPGEN("get the date and time (using Miranda format)"));
	registerIntToken(SRVEXISTS, parseMirSrvExists, TRF_FUNCTION, LPGEN("Miranda Related")"\t(x)\t"LPGEN("TRUE if service function exists"));
}
