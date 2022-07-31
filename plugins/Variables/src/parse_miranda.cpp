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

#include "stdafx.h"

static wchar_t* parseCodeToStatus(ARGUMENTSINFO *ai)
{
	if (ai->argc != 2)
		return nullptr;

	unsigned int status = _wtoi(ai->argv.w[1]);
	wchar_t *szStatus = Clist_GetStatusModeDescription(status, 0);
	if (szStatus != nullptr)
		return mir_wstrdup(szStatus);

	return nullptr;
}

static int getContactInfoFlags(wchar_t *tszDesc)
{
	int flags = 0;
	for (wchar_t *cur = tszDesc; (cur < (tszDesc + mir_wstrlen(tszDesc))); cur++) {
		if (!wcsnicmp(cur, STR_PROTOID, mir_wstrlen(STR_PROTOID))) {
			flags |= CI_PROTOID;
			cur += mir_wstrlen(STR_PROTOID) - 1;
		}
		else if (!wcsnicmp(cur, STR_NICK, mir_wstrlen(STR_NICK))) {
			flags |= CI_NICK;
			cur += mir_wstrlen(STR_NICK) - 1;
		}
		else if (!wcsnicmp(cur, STR_FIRSTNAME, mir_wstrlen(STR_FIRSTNAME))) {
			flags |= CI_FIRSTNAME;
			cur += mir_wstrlen(STR_FIRSTNAME) - 1;
		}
		else if (!wcsnicmp(cur, STR_LASTNAME, mir_wstrlen(STR_LASTNAME))) {
			flags |= CI_LASTNAME;
			cur += mir_wstrlen(STR_LASTNAME) - 1;
		}
		else if (!wcsnicmp(cur, STR_DISPLAY, mir_wstrlen(STR_DISPLAY))) {
			flags |= CI_LISTNAME;
			cur += mir_wstrlen(STR_DISPLAY) - 1;
		}
		else if (!wcsnicmp(cur, STR_EMAIL, mir_wstrlen(STR_EMAIL))) {
			flags |= CI_EMAIL;
			cur += mir_wstrlen(STR_EMAIL) - 1;
		}
		else if (!wcsnicmp(cur, STR_UNIQUEID, mir_wstrlen(STR_UNIQUEID))) {
			flags |= CI_UNIQUEID;
			cur += mir_wstrlen(STR_UNIQUEID) - 1;
		}
	}
	if (flags == 0) {
		flags = getContactInfoType(tszDesc);
		if (flags != 0)
			flags |= CI_CNFINFO;
	}

	return flags;
}

static wchar_t* parseContact(ARGUMENTSINFO *ai)
{
	if (ai->argc < 3 || ai->argc > 4)
		return nullptr;

	int n = 0;
	if (ai->argc == 4) {
		if (*ai->argv.w[3] == 'r') // random contact
			n = -1;
		else
			n = _wtoi(ai->argv.w[3]) - 1;
	}

	MCONTACT hContact = getContactFromString(ai->argv.w[1], getContactInfoFlags(ai->argv.w[2]), n);
	if (hContact == INVALID_CONTACT_ID)
		return nullptr;

	log_debug(0, "contact: %x", hContact);
	return encodeContactToString(hContact);
}

static wchar_t* parseContactCount(ARGUMENTSINFO *ai)
{
	if (ai->argc != 3)
		return nullptr;

	int count = getContactFromString(ai->argv.w[1], CI_NEEDCOUNT | getContactInfoFlags(ai->argv.w[2]));
	return _itow(count);
}

static wchar_t* parseContactInfo(ARGUMENTSINFO *ai)
{
	if (ai->argc != 3)
		return nullptr;

	MCONTACT hContact = getContactFromString(ai->argv.w[1], CI_ALLFLAGS);
	if (hContact == INVALID_CONTACT_ID)
		return nullptr;

	uint8_t type = getContactInfoType(ai->argv.w[2]);
	if (type == 0)
		return nullptr;

	return getContactInfoT(type, hContact);
}

static wchar_t* parseDBProfileName(ARGUMENTSINFO *ai)
{
	if (ai->argc != 1)
		return nullptr;

	wchar_t name[MAX_PATH];
	if (Profile_GetNameW(_countof(name), name))
		return nullptr;

	return mir_wstrdup(name);
}

static wchar_t* parseDBProfilePath(ARGUMENTSINFO *ai)
{
	if (ai->argc != 1)
		return nullptr;

	wchar_t path[MAX_PATH];
	Profile_GetPathW(_countof(path), path);
	return mir_wstrdup(path);
}

static wchar_t* getDBSetting(MCONTACT hContact, char* module, char* setting, wchar_t* defaultValue)
{
	DBVARIANT dbv;
	if (db_get_s(hContact, module, setting, &dbv, 0))
		return defaultValue;

	wchar_t *var = nullptr;
	switch (dbv.type) {
	case DBVT_BYTE:
		var = _itow(dbv.bVal);
		break;
	case DBVT_WORD:
		var = _itow(dbv.wVal);
		break;
	case DBVT_DWORD:
		var = _itow(dbv.dVal);
		break;
	case DBVT_ASCIIZ:
		var = mir_a2u(dbv.pszVal);
		break;
	case DBVT_WCHAR:
		var = mir_wstrdup(dbv.pwszVal);
		break;
	case DBVT_UTF8:
		mir_utf8decode(dbv.pszVal, &var);
		break;
	}

	db_free(&dbv);
	return var;
}

static wchar_t* parseDBSetting(ARGUMENTSINFO *ai)
{
	if (ai->argc < 4)
		return nullptr;

	MCONTACT hContact = NULL;
	if (mir_wstrlen(ai->argv.w[1]) > 0) {
		hContact = getContactFromString(ai->argv.w[1], CI_ALLFLAGS);
		if (hContact == INVALID_CONTACT_ID)
			return nullptr;
	}

	char *szModule = mir_u2a(ai->argv.w[2]);
	if (szModule == nullptr)
		return nullptr;

	char *szSetting = mir_u2a(ai->argv.w[3]);
	if (szSetting == nullptr) {
		mir_free(szModule);
		return nullptr;
	}

	wchar_t *szDefaultValue = ((ai->argc > 4 && mir_wstrlen(ai->argv.w[4]) > 0) ? mir_wstrdup(ai->argv.w[4]) : nullptr);
	wchar_t *res = getDBSetting(hContact, szModule, szSetting, szDefaultValue);
	mir_free(szDefaultValue);
	mir_free(szSetting);
	mir_free(szModule);
	return res;
}

static wchar_t* parseLastSeenDate(ARGUMENTSINFO *ai)
{
	if (ai->argc <= 1)
		return nullptr;

	MCONTACT hContact = getContactFromString(ai->argv.w[1], CI_ALLFLAGS);
	if (hContact == INVALID_CONTACT_ID)
		return nullptr;

	wchar_t *szFormat;
	if (ai->argc == 2 || (ai->argc > 2 && mir_wstrlen(ai->argv.w[2]) == 0))
		szFormat = nullptr;
	else
		szFormat = ai->argv.w[2];

	SYSTEMTIME lsTime = { 0 };
	char *szModule = SEEN_MODULE;
	lsTime.wYear = db_get_w(hContact, szModule, "Year", 0);
	if (lsTime.wYear == 0)
		return nullptr;

	lsTime.wMilliseconds = 0;
	lsTime.wSecond = db_get_w(hContact, szModule, "Seconds", 0);
	lsTime.wMinute = db_get_w(hContact, szModule, "Minutes", 0);
	lsTime.wHour = db_get_w(hContact, szModule, "Hours", 0);
	lsTime.wDay = db_get_w(hContact, szModule, "Day", 0);
	lsTime.wDayOfWeek = db_get_w(hContact, szModule, "WeekDay", 0);
	lsTime.wMonth = db_get_w(hContact, szModule, "Month", 0);

	int len = GetDateFormat(LOCALE_USER_DEFAULT, 0, &lsTime, szFormat, nullptr, 0);
	wchar_t *res = (wchar_t*)mir_alloc((len + 1) * sizeof(wchar_t));
	if (res == nullptr)
		return nullptr;

	if (GetDateFormat(LOCALE_USER_DEFAULT, 0, &lsTime, szFormat, res, len) == 0) {
		mir_free(res);
		return nullptr;
	}

	return res;
}

static wchar_t* parseLastSeenTime(ARGUMENTSINFO *ai)
{
	if (ai->argc <= 1)
		return nullptr;

	MCONTACT hContact = getContactFromString(ai->argv.w[1], CI_ALLFLAGS);
	if (hContact == INVALID_CONTACT_ID)
		return nullptr;

	wchar_t *szFormat;
	if (ai->argc == 2 || (ai->argc > 2 && mir_wstrlen(ai->argv.w[2]) == 0))
		szFormat = nullptr;
	else
		szFormat = ai->argv.w[2];

	SYSTEMTIME lsTime = { 0 };
	char *szModule = SEEN_MODULE;
	lsTime.wYear = db_get_w(hContact, szModule, "Year", 0);
	if (lsTime.wYear == 0)
		return nullptr;

	lsTime.wMilliseconds = 0;
	lsTime.wSecond = db_get_w(hContact, szModule, "Seconds", 0);
	lsTime.wMinute = db_get_w(hContact, szModule, "Minutes", 0);
	lsTime.wHour = db_get_w(hContact, szModule, "Hours", 0);
	lsTime.wDay = db_get_w(hContact, szModule, "Day", 0);
	lsTime.wDayOfWeek = db_get_w(hContact, szModule, "WeekDay", 0);
	lsTime.wMonth = db_get_w(hContact, szModule, "Month", 0);
	lsTime.wYear = db_get_w(hContact, szModule, "Year", 0);

	int len = GetTimeFormat(LOCALE_USER_DEFAULT, 0, &lsTime, szFormat, nullptr, 0);
	wchar_t *res = (wchar_t*)mir_alloc((len + 1) * sizeof(wchar_t));
	if (res == nullptr)
		return nullptr;

	if (GetTimeFormat(LOCALE_USER_DEFAULT, 0, &lsTime, szFormat, res, len) == 0) {
		mir_free(res);
		return nullptr;
	}

	return res;
}

static wchar_t* parseLastSeenStatus(ARGUMENTSINFO *ai)
{
	if (ai->argc != 2)
		return nullptr;

	MCONTACT hContact = getContactFromString(ai->argv.w[1], CI_ALLFLAGS);
	if (hContact == INVALID_CONTACT_ID)
		return nullptr;

	char *szModule = SEEN_MODULE;
	int status = db_get_w(hContact, szModule, "Status", 0);
	if (status == 0)
		return nullptr;

	wchar_t *szStatus = Clist_GetStatusModeDescription(status, 0);
	if (szStatus != nullptr)
		return mir_wstrdup(szStatus);

	return nullptr;
}

static wchar_t* parseMirandaPath(ARGUMENTSINFO *ai)
{
	if (ai->argc != 1)
		return nullptr;

	ai->flags |= AIF_DONTPARSE;
	wchar_t path[MAX_PATH];
	if (GetModuleFileName(nullptr, path, _countof(path)) == 0)
		return nullptr;

	return mir_wstrdup(path);
}

static wchar_t* parseMyStatus(ARGUMENTSINFO *ai)
{
	if (ai->argc > 2)
		return nullptr;

	int status;
	if (ai->argc == 1 || mir_wstrlen(ai->argv.w[1]) == 0)
		status = CallService(MS_CLIST_GETSTATUSMODE, 0, 0);
	else
		status = Proto_GetStatus(_T2A(ai->argv.w[1]));

	wchar_t *szStatus = Clist_GetStatusModeDescription(status, 0);
	return (szStatus != nullptr) ? mir_wstrdup(szStatus) : nullptr;
}

static wchar_t* parseProtoInfo(ARGUMENTSINFO *ai)
{
	if (ai->argc != 3)
		return nullptr;

	const char *szRes = nullptr;
	wchar_t *wszRes = nullptr;
	ptrA szProto(mir_u2a(ai->argv.w[1]));

	if (!mir_wstrcmp(ai->argv.w[2], _A2W(STR_PINAME)))
		wszRes = Hlp_GetProtocolName(szProto);
	else if (!mir_wstrcmp(ai->argv.w[2], _A2W(STR_PIUIDTEXT))) {
		wszRes = (wchar_t *)CallProtoService(szProto, PS_GETCAPS, PFLAG_UNIQUEIDTEXT, 0);
		if (INT_PTR(wszRes) == CALLSERVICE_NOTFOUND || wszRes == nullptr)
			return nullptr;
	}
	else if (!mir_wstrcmp(ai->argv.w[2], _A2W(STR_PIUIDSETTING))) {
		szRes = Proto_GetUniqueId(szProto);
		if (szRes == nullptr)
			return nullptr;
	}
	else if (!mir_wstrcmp(ai->argv.w[2], _A2W(STR_PINICK)))
		wszRes = Contact::GetInfo(CNF_DISPLAY, NULL, szProto);

	if (szRes == nullptr && wszRes == nullptr)
		return nullptr;

	if (szRes != nullptr && wszRes == nullptr)
		wszRes = mir_a2u(szRes);

	return wszRes;
}

static wchar_t* parseSpecialContact(ARGUMENTSINFO *ai)
{
	if (ai->argc != 1 || ai->fi->hContact == NULL)
		return nullptr;

	ai->flags |= AIF_DONTPARSE;
	ptrW szUniqueID;
	char *szProto = Proto_GetBaseAccountName(ai->fi->hContact);
	if (szProto != nullptr)
		szUniqueID = getContactInfoT(CNF_UNIQUEID, ai->fi->hContact);

	if (szUniqueID == NULL) {
		szProto = PROTOID_HANDLE;
		szUniqueID = (wchar_t*)mir_alloc(40);
		if (szUniqueID == NULL)
			return nullptr;
		_itow_s(ai->fi->hContact, szUniqueID, 20, 10);
	}

	if (szUniqueID == NULL)
		return nullptr;

	return CMStringW(FORMAT, L"<%S:%s>", szProto, szUniqueID.get()).Detach();
}

static BOOL isValidDbEvent(DBEVENTINFO *dbe, int flags)
{
	BOOL bEventType, bEventFlags;

	bEventType = ((dbe->eventType == EVENTTYPE_MESSAGE) && (flags & DBE_MESSAGE)) ||
		((dbe->eventType == EVENTTYPE_CONTACTS) && (flags & DBE_CONTACTS)) ||
		((dbe->eventType == EVENTTYPE_ADDED) && (flags & DBE_ADDED)) ||
		((dbe->eventType == EVENTTYPE_AUTHREQUEST) && (flags & DBE_AUTHREQUEST)) ||
		((dbe->eventType == EVENTTYPE_FILE) && (flags & DBE_FILE)) ||
		((flags & DBE_OTHER));
	bEventFlags = (dbe->flags&DBEF_SENT) ? (flags&DBE_SENT) : (flags&DBE_RCVD);
	bEventFlags = (bEventFlags && ((dbe->flags&DBEF_READ) ? (flags&DBE_READ) : (flags&DBE_UNREAD)));

	return (bEventType && bEventFlags);
}

static MEVENT findDbEvent(MCONTACT hContact, MEVENT hDbEvent, int flags)
{
	BOOL bEventOk;

	do {
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
			MEVENT hMatchEvent = NULL, hSearchEvent = NULL;
			uint32_t matchTimestamp = 0, priorTimestamp = 0;

			DBEVENTINFO dbe = {};
			if (flags & DBE_FIRST) {
				for (auto &hSearchContact : Contacts()) {
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
				for (auto &hSearchContact : Contacts()) {
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
					for (auto &hSearchContact : Contacts()) {
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
					for (auto &hSearchContact : Contacts()) {
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

		DBEVENTINFO dbe = {};
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
	} while ((!bEventOk) && (hDbEvent != NULL));

	return hDbEvent;
}

// ?message(%subject%,last|first,sent|recv,read|unread)
static wchar_t* parseDbEvent(ARGUMENTSINFO *ai)
{
	if (ai->argc != 5)
		return nullptr;

	int flags = DBE_MESSAGE;
	switch (*ai->argv.w[2]) {
	case 'f':
		flags |= DBE_FIRST;
		break;
	default:
		flags |= DBE_LAST;
		break;
	}
	switch (*ai->argv.w[3]) {
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
	switch (*ai->argv.w[4]) {
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

	MCONTACT hContact = getContactFromString(ai->argv.w[1], CI_ALLFLAGS);
	if (hContact == INVALID_CONTACT_ID)
		return nullptr;

	MEVENT hDbEvent = findDbEvent(hContact, NULL, flags);
	if (hDbEvent == NULL)
		return nullptr;

	DB::EventInfo dbe;
	dbe.cbBlob = -1;
	if (db_event_get(hDbEvent, &dbe))
		return nullptr;

	return DbEvent_GetTextW(&dbe, CP_ACP);
}

static wchar_t* parseTranslate(ARGUMENTSINFO *ai)
{
	if (ai->argc != 2)
		return nullptr;

	wchar_t *res = TranslateW(ai->argv.w[1]);
	return (res == nullptr) ? nullptr : mir_wstrdup(res);
}

static wchar_t* parseVersionString(ARGUMENTSINFO *ai)
{
	if (ai->argc != 1)
		return nullptr;

	ai->flags |= AIF_DONTPARSE;
	char versionString[128];
	Miranda_GetVersionText(versionString, sizeof(versionString));
	return mir_a2u(versionString);
}

static wchar_t* parseContactNameString(ARGUMENTSINFO *ai)
{
	if (ai->argc != 1 || ai->fi->hContact == NULL)
		return nullptr;

	ai->flags |= AIF_DONTPARSE;
	wchar_t *ret = Clist_GetContactDisplayName(ai->fi->hContact);
	return (ret == nullptr) ? nullptr : mir_wstrdup(ret);
}

static wchar_t* parseMirDateString(ARGUMENTSINFO *ai)
{
	if (ai->argc != 1)
		return nullptr;

	ai->flags |= AIF_DONTPARSE;

	wchar_t ret[128];
	return mir_wstrdup(TimeZone_ToStringT(time(0), L"d s", ret, _countof(ret)));
}

static wchar_t* parseMirandaCoreVar(ARGUMENTSINFO *ai)
{
	if (ai->argc != 1)
		return nullptr;

	ai->flags |= AIF_DONTPARSE;

	wchar_t corevar[MAX_PATH];
	mir_snwprintf(corevar, L"%%%s%%", ai->argv.w[0]);
	return Utils_ReplaceVarsW(corevar);
}

static wchar_t* parseMirSrvExists(ARGUMENTSINFO *ai)
{
	if (ai->argc != 2)
		return nullptr;

	if (!ServiceExists(_T2A(ai->argv.w[1])))
		ai->flags |= AIF_FALSE;

	return mir_wstrdup(L"");
}

void registerMirandaTokens()
{
	// global vars
	registerIntToken(L"miranda_path", parseMirandaCoreVar, TRF_FIELD, LPGEN("Miranda Core Global") "\t" LPGEN("path to Miranda root folder"));
	registerIntToken(L"miranda_profilesdir", parseMirandaCoreVar, TRF_FIELD, LPGEN("Miranda Core Global") "\t" LPGEN("path to folder containing Miranda profiles"));
	registerIntToken(L"miranda_profilename", parseMirandaCoreVar, TRF_FIELD, LPGEN("Miranda Core Global") "\t" LPGEN("name of current Miranda profile (filename, without extension)"));
	registerIntToken(L"miranda_userdata", parseMirandaCoreVar, TRF_FIELD, LPGEN("Miranda Core Global") "\t" LPGEN("will return parsed string %miranda_profilesdir%\\%miranda_profilename%"));
	registerIntToken(L"miranda_avatarcache", parseMirandaCoreVar, TRF_FIELD, LPGEN("Miranda Core Global") "\t" LPGEN("will return parsed string %miranda_profilesdir%\\%miranda_profilename%\\AvatarCache"));
	registerIntToken(L"miranda_logpath", parseMirandaCoreVar, TRF_FIELD, LPGEN("Miranda Core Global") "\t" LPGEN("will return parsed string %miranda_profilesdir%\\%miranda_profilename%\\Logs"));

	// OS vars
	registerIntToken(L"appdata", parseMirandaCoreVar, TRF_FIELD, LPGEN("Miranda Core OS") "\t" LPGEN("same as environment variable %APPDATA% for currently logged-on Windows user"));
	registerIntToken(L"username", parseMirandaCoreVar, TRF_FIELD, LPGEN("Miranda Core OS") "\t" LPGEN("username for currently logged-on Windows user"));
	registerIntToken(L"mydocuments", parseMirandaCoreVar, TRF_FIELD, LPGEN("Miranda Core OS") "\t" LPGEN("\"My Documents\" folder for currently logged-on Windows user"));
	registerIntToken(L"desktop", parseMirandaCoreVar, TRF_FIELD, LPGEN("Miranda Core OS") "\t" LPGEN("\"Desktop\" folder for currently logged-on Windows user"));

	registerIntToken(CODETOSTATUS, parseCodeToStatus, TRF_FUNCTION, LPGEN("Miranda Related") "\t(x)\t" LPGEN("translates status code x into a status description"));
	registerIntToken(CONTACT, parseContact, TRF_FUNCTION, LPGEN("Miranda Related") "\t(x,y,z)\t" LPGEN("zth contact with property y described by x, example: (unregistered,nick) (z is optional)"));
	registerIntToken(CONTACTCOUNT, parseContactCount, TRF_FUNCTION, LPGEN("Miranda Related") "\t(x,y)\t" LPGEN("number of contacts with property y described by x, example: (unregistered,nick)"));
	registerIntToken(MIR_CONTACTINFO, parseContactInfo, TRF_FUNCTION, LPGEN("Miranda Related") "\t(x,y)\t" LPGEN("info property y of contact x"));
	registerIntToken(DBPROFILENAME, parseDBProfileName, TRF_FIELD, LPGEN("Miranda Related") "\t" LPGEN("database profile name"));
	registerIntToken(DBPROFILEPATH, parseDBProfilePath, TRF_FIELD, LPGEN("Miranda Related") "\t" LPGEN("database profile path"));
	registerIntToken(DBSETTING, parseDBSetting, TRF_FUNCTION, LPGEN("Miranda Related") "\t(x,y,z,w)\t" LPGEN("database setting z of module y of contact x and return w if z doesn't exist (w is optional)"));
	registerIntToken(DBEVENT, parseDbEvent, TRF_FUNCTION, LPGEN("Miranda Related") "\t(x,y,z,w)\t" LPGEN("get event for contact x (optional), according to y,z,w, see documentation"));
	registerIntToken(LSTIME, parseLastSeenTime, TRF_FUNCTION, LPGEN("Miranda Related") "\t(x,y)\t" LPGEN("get last seen time of contact x in format y (y is optional)"));
	registerIntToken(LSDATE, parseLastSeenDate, TRF_FUNCTION, LPGEN("Miranda Related") "\t(x,y)\t" LPGEN("get last seen date of contact x in format y (y is optional)"));
	registerIntToken(LSSTATUS, parseLastSeenStatus, TRF_FUNCTION, LPGEN("Miranda Related") "\t(x)\t" LPGEN("get last seen status of contact x"));
	registerIntToken(MIRANDAPATH, parseMirandaPath, TRF_FIELD, LPGEN("Miranda Related") "\t" LPGEN("path to the Miranda NG executable"));
	registerIntToken(MYSTATUS, parseMyStatus, TRF_FUNCTION, LPGEN("Miranda Related") "\t(x)\t" LPGEN("current status description of protocol x (without x, the global status is retrieved)"));
	registerIntToken(PROTOINFO, parseProtoInfo, TRF_FUNCTION, LPGEN("Miranda Related") "\t(x,y)\t" LPGEN("info property y of protocol ID x"));
	registerIntToken(SUBJECT, parseSpecialContact, TRF_FIELD, LPGEN("Miranda Related") "\t" LPGEN("retrieves the subject, depending on situation"));
	registerIntToken(TRANSLATE, parseTranslate, TRF_FUNCTION, LPGEN("Miranda Related") "\t(x)\t" LPGEN("translates x"));
	registerIntToken(VERSIONSTRING, parseVersionString, TRF_FIELD, LPGEN("Miranda Related") "\t" LPGEN("get the version of Miranda"));
	registerIntToken(CONTACT_NAME, parseContactNameString, TRF_FIELD, LPGEN("Miranda Related") "\t" LPGEN("get the contact display name"));
	registerIntToken(MIR_DATE, parseMirDateString, TRF_FIELD, LPGEN("Miranda Related") "\t" LPGEN("get the date and time (using Miranda format)"));
	registerIntToken(SRVEXISTS, parseMirSrvExists, TRF_FUNCTION, LPGEN("Miranda Related") "\t(x)\t" LPGEN("TRUE if service function exists"));
}
