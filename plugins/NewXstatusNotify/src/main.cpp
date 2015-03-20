/*
	NewXstatusNotify YM - Plugin for Miranda IM
	Copyright (c) 2001-2004 Luca Santarelli
	Copyright (c) 2005-2007 Vasilich
	Copyright (c) 2007-2011 yaho

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

#include "common.h"

HINSTANCE hInst;

LIST<DBEVENT> eventListXStatus(10);
LIST<DBEVENT> eventListStatus(10);
LIST<DBEVENT> eventListSMsg(10);

HANDLE hStatusModeChange, hServiceMenu, hHookContactStatusChanged, hToolbarButton;
HGENMENU hEnableDisableMenu;

STATUS StatusList[STATUS_COUNT];
STATUS StatusListEx[STATUSEX_COUNT];
HWND SecretWnd;
int hLangpack;

int ContactStatusChanged(MCONTACT hContact, WORD oldStatus, WORD newStatus);

PLUGININFOEX pluginInfoEx = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// EBF19652-E434-4D79-9897-91A0FF226F51
	{ 0xebf19652, 0xe434, 0x4d79, { 0x98, 0x97, 0x91, 0xa0, 0xff, 0x22, 0x6f, 0x51 } }
};

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD, LPVOID)
{
	hInst = hinstDLL;
	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD)
{
	return &pluginInfoEx;
}

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = { MIID_USERONLINE, MIID_LAST };

BYTE GetGender(MCONTACT hContact)
{
	char *szProto = GetContactProto(hContact);
	if (szProto) {
		switch (db_get_b(hContact, szProto, "Gender", 0)) {
		case 'M':
		case 'm':
			return GENDER_MALE;
		case 'F':
		case 'f':
			return GENDER_FEMALE;
		default:
			return GENDER_UNSPECIFIED;
		}
	}

	return GENDER_UNSPECIFIED;
}

HANDLE GetIconHandle(char *szIcon)
{
	char szSettingName[64];
	mir_snprintf(szSettingName, SIZEOF(szSettingName), "%s_%s", MODULE, szIcon);
	return Skin_GetIconHandle(szSettingName);
}

static int __inline CheckStr(char *str, int not_empty, int empty)
{
	if (str == NULL || str[0] == '\0')
		return empty;
	else
		return not_empty;
}

static int __inline CheckStrW(WCHAR *str, int not_empty, int empty)
{
	if (str == NULL || str[0] == L'\0')
		return empty;
	else
		return not_empty;
}

static int CompareStatusMsg(STATUSMSGINFO *smi, DBCONTACTWRITESETTING *cws_new, char *szSetting) {
	DBVARIANT dbv_old;
	int ret = -1;

	switch (cws_new->value.type) {
	case DBVT_ASCIIZ:
		smi->newstatusmsg = (CheckStr(cws_new->value.pszVal, 0, 1) ? NULL : mir_dupToUnicodeEx(cws_new->value.pszVal, CP_ACP));
		break;
	case DBVT_UTF8:
		smi->newstatusmsg = (CheckStr(cws_new->value.pszVal, 0, 1) ? NULL : mir_dupToUnicodeEx(cws_new->value.pszVal, CP_UTF8));
		break;
	case DBVT_WCHAR:
		smi->newstatusmsg = (CheckStrW(cws_new->value.pwszVal, 0, 1) ? NULL : mir_wstrdup(cws_new->value.pwszVal));
		break;
	case DBVT_DELETED:
	default:
		smi->newstatusmsg = NULL;
		break;
	}

	if (!db_get_s(smi->hContact, "UserOnline", szSetting, &dbv_old, 0)) {
		switch (dbv_old.type) {
		case DBVT_ASCIIZ:
			smi->oldstatusmsg = (CheckStr(dbv_old.pszVal, 0, 1) ? NULL : mir_dupToUnicodeEx(dbv_old.pszVal, CP_ACP));
			break;
		case DBVT_UTF8:
			smi->oldstatusmsg = (CheckStr(dbv_old.pszVal, 0, 1) ? NULL : mir_dupToUnicodeEx(dbv_old.pszVal, CP_UTF8));
			break;
		case DBVT_WCHAR:
			smi->oldstatusmsg = (CheckStrW(dbv_old.pwszVal, 0, 1) ? NULL : mir_wstrdup(dbv_old.pwszVal));
			break;
		default:
			smi->oldstatusmsg = NULL;
			break;
		}

		if (cws_new->value.type == DBVT_DELETED) {
			if (dbv_old.type == DBVT_WCHAR)
				ret = CheckStrW(dbv_old.pwszVal, COMPARE_DEL, COMPARE_SAME);
			else if (dbv_old.type == DBVT_UTF8 || dbv_old.type == DBVT_ASCIIZ)
				ret = CheckStr(dbv_old.pszVal, COMPARE_DEL, COMPARE_SAME);
			else
				ret = COMPARE_DEL;
		}
		else if (dbv_old.type != cws_new->value.type)
			ret = (mir_wstrcmp(smi->newstatusmsg, smi->oldstatusmsg) ? CheckStrW(smi->newstatusmsg, COMPARE_DIFF, COMPARE_DEL) : COMPARE_SAME);

		else if (dbv_old.type == DBVT_ASCIIZ)
			ret = (mir_strcmp(cws_new->value.pszVal, dbv_old.pszVal) ? CheckStr(cws_new->value.pszVal, COMPARE_DIFF, COMPARE_DEL) : COMPARE_SAME);

		else if (dbv_old.type == DBVT_UTF8)
			ret = (mir_strcmp(cws_new->value.pszVal, dbv_old.pszVal) ? CheckStr(cws_new->value.pszVal, COMPARE_DIFF, COMPARE_DEL) : COMPARE_SAME);

		else if (dbv_old.type == DBVT_WCHAR)
			ret = (mir_wstrcmp(cws_new->value.pwszVal, dbv_old.pwszVal) ? CheckStrW(cws_new->value.pwszVal, COMPARE_DIFF, COMPARE_DEL) : COMPARE_SAME);

		db_free(&dbv_old);
	}
	else {
		if (cws_new->value.type == DBVT_DELETED)
			ret = COMPARE_SAME;
		else if (cws_new->value.type == DBVT_WCHAR)
			ret = CheckStrW(cws_new->value.pwszVal, COMPARE_DIFF, COMPARE_SAME);
		else if (cws_new->value.type == DBVT_UTF8 || cws_new->value.type == DBVT_ASCIIZ)
			ret = CheckStr(cws_new->value.pszVal, COMPARE_DIFF, COMPARE_SAME);
		else
			ret = COMPARE_DIFF;

		smi->oldstatusmsg = NULL;
	}

	return ret;
}

TCHAR* GetStr(STATUSMSGINFO *n, const TCHAR *tmplt)
{
	if (n == NULL || tmplt == NULL || tmplt[0] == _T('\0'))
		return NULL;

	CMString res;
	size_t len = mir_tstrlen(tmplt);

	for (size_t i = 0; i < len; i++) {
		if (tmplt[i] == _T('%')) {
			i++;
			switch (tmplt[i]) {
			case 'n':
				if (n->compare == COMPARE_DEL || mir_tstrcmp(n->newstatusmsg, TranslateT("<no status message>")) == 0)
					res.Append(TranslateT("<no status message>"));
				else
					res.Append(ptrT(AddCR(n->newstatusmsg)));
				break;

			case 'o':
				if (n->oldstatusmsg == NULL || n->oldstatusmsg[0] == _T('\0') || _tcscmp(n->oldstatusmsg, TranslateT("<no status message>")) == 0)
					res.Append(TranslateT("<no status message>"));
				else
					res.Append(ptrT(AddCR(n->oldstatusmsg)));
				break;

			case 'c':
				if (n->hContact == NULL)
					res.Append(TranslateT("Contact"));
				else
					res.Append((TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)n->hContact, GCDNF_TCHAR));
				break;

			case 's':
				if (n->hContact == NULL)
					res.Append(TranslateT("<unknown>"));
				else
					res.Append(StatusList[Index(db_get_w(n->hContact, n->proto, "Status", ID_STATUS_ONLINE))].lpzStandardText);
				break;

			default:
				i--;
				res.AppendChar(tmplt[i]);
				break;
			}
		}
		else if (tmplt[i] == _T('\\')) {
			i++;
			switch (tmplt[i]) {
			case 'n':
				res.AppendChar('\r');
				res.AppendChar('\n');
				break;
			case 't':
				res.AppendChar('\t');
				break;
			default:
				i--;
				res.AppendChar(tmplt[i]);
				break;
			}
		}
		else res.AppendChar(tmplt[i]);
	}

	if (res.GetLength() > 2044) {
		res.Truncate(2044);
		res.Append(_T("..."));
	}

	return mir_tstrndup(res, res.GetLength());
}

bool SkipHiddenContact(MCONTACT hContact)
{
	return (!opt.HiddenContactsToo && (db_get_b(hContact, "CList", "Hidden", 0) == 1));
}

void LogSMsgToDB(STATUSMSGINFO *smi, const TCHAR *tmplt)
{
	TCHAR *str = GetStr(smi, tmplt);

	char *blob = mir_utf8encodeT(str);

	DBEVENTINFO dbei = { 0 };
	dbei.cbSize = sizeof(dbei);
	dbei.cbBlob = (DWORD)strlen(blob) + 1;
	dbei.pBlob = (PBYTE)blob;
	dbei.eventType = EVENTTYPE_STATUSCHANGE;
	dbei.flags = DBEF_READ | DBEF_UTF;

	dbei.timestamp = (DWORD)time(NULL);
	dbei.szModule = MODULE;
	MEVENT hDBEvent = db_event_add(smi->hContact, &dbei);
	mir_free(blob);
	mir_free(str);

	if (opt.SMsgLogToDB_WinOpen && opt.SMsgLogToDB_Remove) {
		DBEVENT *dbevent = (DBEVENT *)mir_alloc(sizeof(DBEVENT));
		dbevent->hContact = smi->hContact;
		dbevent->hDBEvent = hDBEvent;
		eventListSMsg.insert(dbevent);
	}
}

void GetStatusText(MCONTACT hContact, WORD newStatus, WORD oldStatus, TCHAR *stzStatusText)
{
	if (opt.UseAlternativeText) {
		switch (GetGender(hContact)) {
		case GENDER_MALE:
			_tcsncpy(stzStatusText, StatusList[Index(newStatus)].lpzMStatusText, MAX_STATUSTEXT);
			break;
		case GENDER_FEMALE:
			_tcsncpy(stzStatusText, StatusList[Index(newStatus)].lpzFStatusText, MAX_STATUSTEXT);
			break;
		case GENDER_UNSPECIFIED:
			_tcsncpy(stzStatusText, StatusList[Index(newStatus)].lpzUStatusText, MAX_STATUSTEXT);
			break;
		}
	}
	else
		_tcsncpy(stzStatusText, StatusList[Index(newStatus)].lpzStandardText, MAX_STATUSTEXT);

	if (opt.ShowPreviousStatus) {
		TCHAR buff[MAX_STATUSTEXT];
		mir_sntprintf(buff, SIZEOF(buff), TranslateTS(STRING_SHOWPREVIOUSSTATUS), StatusList[Index(oldStatus)].lpzStandardText);
		_tcscat(_tcscat(stzStatusText, _T(" ")), buff);
	}
}

void BlinkIcon(MCONTACT hContact, HICON hIcon, TCHAR *stzText)
{
	CLISTEVENT cle = { 0 };
	cle.cbSize = sizeof(cle);
	cle.flags = CLEF_ONLYAFEW | CLEF_TCHAR;
	cle.hContact = hContact;
	cle.hDbEvent = hContact;
	cle.hIcon = hIcon;
	cle.pszService = "UserOnline/Description";
	cle.ptszTooltip = stzText;
	CallService(MS_CLIST_ADDEVENT, 0, (LPARAM)&cle);
}

void PlayChangeSound(MCONTACT hContact, const char *name)
{
	if (opt.UseIndSnd) {
		DBVARIANT dbv;
		TCHAR stzSoundFile[MAX_PATH] = { 0 };
		if (!db_get_ts(hContact, MODULE, name, &dbv)) {
			_tcsncpy(stzSoundFile, dbv.ptszVal, SIZEOF(stzSoundFile)-1);
			db_free(&dbv);
		}

		if (stzSoundFile[0]) {
			//Now make path to IndSound absolute, as it isn't registered
			TCHAR stzSoundPath[MAX_PATH];
			PathToAbsoluteT(stzSoundFile, stzSoundPath);
			SkinPlaySoundFile(stzSoundPath);
			return;
		}
	}

	if (db_get_b(0, "SkinSoundsOff", name, 0) == 0)
		SkinPlaySound(name);
}

int ContactStatusChanged(MCONTACT hContact, WORD oldStatus, WORD newStatus)
{
	if (opt.LogToDB && (!opt.LogToDB_WinOpen || CheckMsgWnd(hContact))) {
		TCHAR stzStatusText[MAX_SECONDLINE] = { 0 };
		GetStatusText(hContact, newStatus, oldStatus, stzStatusText);
		char *blob = mir_utf8encodeT(stzStatusText);

		DBEVENTINFO dbei = { 0 };
		dbei.cbSize = sizeof(dbei);
		dbei.cbBlob = (DWORD)strlen(blob) + 1;
		dbei.pBlob = (PBYTE)blob;
		dbei.eventType = EVENTTYPE_STATUSCHANGE;
		dbei.flags = DBEF_READ | DBEF_UTF;

		dbei.timestamp = (DWORD)time(NULL);
		dbei.szModule = MODULE;
		MEVENT hDBEvent = db_event_add(hContact, &dbei);
		mir_free(blob);

		if (opt.LogToDB_WinOpen && opt.LogToDB_Remove) {
			DBEVENT *dbevent = (DBEVENT *)mir_alloc(sizeof(DBEVENT));
			dbevent->hContact = hContact;
			dbevent->hDBEvent = hDBEvent;
			eventListStatus.insert(dbevent);
		}
	}

	bool bEnablePopup = true, bEnableSound = true;
	char *szProto = GetContactProto(hContact);
	WORD myStatus = (WORD)CallProtoService(szProto, PS_GETSTATUS, 0, 0);

	if (!strcmp(szProto, META_PROTO)) { //this contact is Meta
		MCONTACT hSubContact = db_mc_getMostOnline(hContact);
		char *szSubProto = GetContactProto(hSubContact);
		if (szSubProto == NULL)
			return 0;

		if (newStatus == ID_STATUS_OFFLINE) {
			// read last online proto for metacontact if exists,
			// to avoid notifying when meta went offline but default contact's proto still online
			DBVARIANT dbv;
			if (!db_get_s(hContact, szProto, "LastOnline", &dbv)) {
				szSubProto = NEWSTR_ALLOCA(dbv.pszVal);
				db_free(&dbv);
			}
		}
		else
			db_set_s(hContact, szProto, "LastOnline", szSubProto);

		if (db_get_b(0, MODULE, szSubProto, 1) == 0)
			return 0;

		szProto = szSubProto;
	}
	else {
		if (myStatus == ID_STATUS_OFFLINE || db_get_b(0, MODULE, szProto, 1) == 0)
			return 0;
	}

	if (!opt.FromOffline || oldStatus != ID_STATUS_OFFLINE) { // Either it wasn't a change from Offline or we didn't enable that.
		char buff[8];
		mir_snprintf(buff, SIZEOF(buff), "%d", newStatus);
		if (db_get_b(0, MODULE, buff, 1) == 0)
			return 0; // "Notify when a contact changes to one of..." is unchecked
	}

	if (SkipHiddenContact(hContact))
		return 0;

	// check if that proto from which we received statuschange notification, isn't in autodisable list
	if (opt.AutoDisable) {
		char statusIDs[12], statusIDp[12];
		mir_snprintf(statusIDs, SIZEOF(statusIDs), "s%d", myStatus);
		mir_snprintf(statusIDp, SIZEOF(statusIDp), "p%d", myStatus);
		bEnableSound = db_get_b(0, MODULE, statusIDs, 1) ? FALSE : TRUE;
		bEnablePopup = db_get_b(0, MODULE, statusIDp, 1) ? FALSE : TRUE;
	}

	if (bEnablePopup && db_get_b(hContact, MODULE, "EnablePopups", 1) && !opt.TempDisabled) {
		WORD myStatus = (WORD)CallProtoService(szProto, PS_GETSTATUS, 0, 0);
		TCHAR str[MAX_SECONDLINE] = { 0 };
		if (opt.ShowStatus) {
			GetStatusText(hContact, newStatus, oldStatus, str);
		}

		if (opt.ReadAwayMsg && myStatus != ID_STATUS_INVISIBLE && StatusHasAwayMessage(szProto, newStatus))
			db_set_ws(hContact, MODULE, "LastPopupText", str);

		PLUGINDATA *pdp = (PLUGINDATA *)mir_calloc(sizeof(PLUGINDATA));
		pdp->oldStatus = oldStatus;
		pdp->newStatus = newStatus;
		pdp->hAwayMsgHook = NULL;
		pdp->hAwayMsgProcess = NULL;
		ShowChangePopup(hContact, LoadSkinnedProtoIcon(szProto, newStatus), newStatus, str, pdp);
	}

	if (opt.BlinkIcon && !opt.TempDisabled) {
		HICON hIcon = opt.BlinkIcon_Status ? LoadSkinnedProtoIcon(szProto, newStatus) : LoadSkinnedIcon(SKINICON_OTHER_USERONLINE);
		TCHAR str[256];
		mir_sntprintf(str, SIZEOF(str), TranslateT("%s is now %s"),
			CallService(MS_CLIST_GETCONTACTDISPLAYNAME, hContact, GCDNF_TCHAR), StatusList[Index(newStatus)].lpzStandardText);
		BlinkIcon(hContact, hIcon, str);
	}

	if (bEnableSound && db_get_b(0, "Skin", "UseSound", TRUE) && db_get_b(hContact, MODULE, "EnableSounds", 1) && !opt.TempDisabled) {
		if (oldStatus == ID_STATUS_OFFLINE)
			PlayChangeSound(hContact, StatusListEx[ID_STATUS_FROMOFFLINE].lpzSkinSoundName);
		else
			PlayChangeSound(hContact, StatusList[Index(newStatus)].lpzSkinSoundName);
	}

	if (opt.LogToFile) {
		TCHAR stzDate[MAX_STATUSTEXT], stzTime[MAX_STATUSTEXT], stzText[MAX_TEXT_LEN];

		GetTimeFormat(LOCALE_USER_DEFAULT, 0, NULL, _T("HH':'mm"), stzTime, SIZEOF(stzTime));
		GetDateFormat(LOCALE_USER_DEFAULT, 0, NULL, _T("dd/MM/yyyy"), stzDate, SIZEOF(stzDate));
		mir_sntprintf(stzText, SIZEOF(stzText), TranslateT("%s, %s. %s changed status to %s (was %s)\r\n"),
			stzDate, stzTime, CallService(MS_CLIST_GETCONTACTDISPLAYNAME, hContact, GCDNF_TCHAR), StatusList[Index(newStatus)].lpzStandardText,
			StatusList[Index(oldStatus)].lpzStandardText);
		LogToFile(stzText);
	}

	return 0;
}

int ProcessStatus(DBCONTACTWRITESETTING *cws, MCONTACT hContact)
{
	WORD newStatus = cws->value.wVal;
	if (newStatus < ID_STATUS_MIN || newStatus > ID_STATUS_MAX)
		return 0;

	char *szProto = GetContactProto(hContact);
	if (strcmp(cws->szModule, szProto))
		return 0;

	// we don't want to be notified if new chatroom comes online
	if (db_get_b(hContact, szProto, "ChatRoom", 0) == 1)
		return 0;

	WORD oldStatus = DBGetContactSettingRangedWord(hContact, "UserOnline", "LastStatus", ID_STATUS_OFFLINE, ID_STATUS_MIN, ID_STATUS_MAX);
	if (oldStatus == newStatus)
		return 0;

	//If we get here, the two statuses differ, so we can proceed.
	db_set_w(hContact, "UserOnline", "LastStatus", newStatus);

	// A simple implementation of Last Seen module, please don't touch this.
	if (opt.EnableLastSeen && newStatus == ID_STATUS_OFFLINE && oldStatus > ID_STATUS_OFFLINE) {
		SYSTEMTIME systime;
		GetLocalTime(&systime);

		db_set_w(hContact, "SeenModule", "Year", systime.wYear);
		db_set_w(hContact, "SeenModule", "Month", systime.wMonth);
		db_set_w(hContact, "SeenModule", "Day", systime.wDay);
		db_set_w(hContact, "SeenModule", "Hours", systime.wHour);
		db_set_w(hContact, "SeenModule", "Minutes", systime.wMinute);
		db_set_w(hContact, "SeenModule", "Seconds", systime.wSecond);
		db_set_w(hContact, "SeenModule", "Status", oldStatus);
	}

	//If *Miranda* ignores the UserOnline event, exit!
	if (CallService(MS_IGNORE_ISIGNORED, hContact, IGNOREEVENT_USERONLINE))
		return 0;

	//If we get here, we have to notify the Hooks.
	ContactStatusChanged(hContact, oldStatus, newStatus);
	NotifyEventHooks(hHookContactStatusChanged, hContact, (LPARAM)MAKELPARAM(oldStatus, newStatus));
	return 1;
}

int ProcessExtraStatus(DBCONTACTWRITESETTING *cws, MCONTACT hContact)
{
	XSTATUSCHANGE *xsc;
	STATUSMSGINFO smi = { 0 };
	char *szProto = GetContactProto(hContact);
	smi.hContact = hContact;

	if (strstr(cws->szSetting, "/mood/") || strstr(cws->szSetting, "/activity/")) { // Jabber mood or activity changed
		if (!ProtoServiceExists(szProto, JS_PARSE_XMPP_URI))
			return 0;

		char *szSetting;
		int type;

		if (strstr(cws->szSetting, "/mood/")) {
			type = TYPE_JABBER_MOOD;
			szSetting = "LastJabberMood";
		}
		else {
			type = TYPE_JABBER_ACTIVITY;
			szSetting = "LastJabberActivity";
		}

		if (strstr(cws->szSetting, "title")) {
			smi.compare = CompareStatusMsg(&smi, cws, szSetting);
			if (smi.compare == COMPARE_SAME) {
				replaceStrT(smi.newstatusmsg, 0);
				replaceStrT(smi.oldstatusmsg, 0);
			}

			if (cws->value.type == DBVT_DELETED)
				db_unset(hContact, "UserOnline", szSetting);
			else
				db_set(hContact, "UserOnline", szSetting, &cws->value);

			xsc = NewXSC(hContact, szProto, type, smi.compare, smi.newstatusmsg, NULL);
			ExtraStatusChanged(xsc);
		}
		else if (strstr(cws->szSetting, "text")) {
			char dbSetting[128];
			mir_snprintf(dbSetting, SIZEOF(dbSetting), "%s%s", szSetting, "Msg");
			smi.compare = CompareStatusMsg(&smi, cws, dbSetting);
			if (smi.compare == COMPARE_SAME) {
				replaceStrT(smi.newstatusmsg, 0);
				replaceStrT(smi.oldstatusmsg, 0);
			}

			if (cws->value.type == DBVT_DELETED)
				db_unset(hContact, "UserOnline", dbSetting);
			else
				db_set(hContact, "UserOnline", dbSetting, &cws->value);

			xsc = NewXSC(hContact, szProto, type, smi.compare * 4, NULL, smi.newstatusmsg);
			ExtraStatusChanged(xsc);
		}
		return 1;
	}

	if (strstr(cws->szSetting, "XStatus")) {
		if (strcmp(cws->szModule, szProto))
			return 0;

		if (strcmp(cws->szSetting, "XStatusName") == 0) {
			smi.compare = CompareStatusMsg(&smi, cws, "LastXStatusName");
			if (smi.compare == COMPARE_SAME) {
				replaceStrT(smi.newstatusmsg, 0);
				replaceStrT(smi.oldstatusmsg, 0);
			}

			if (cws->value.type == DBVT_DELETED)
				db_unset(hContact, "UserOnline", "LastXStatusName");
			else
				db_set(hContact, "UserOnline", "LastXStatusName", &cws->value);

			xsc = NewXSC(hContact, szProto, TYPE_ICQ_XSTATUS, smi.compare, smi.newstatusmsg, NULL);
			ExtraStatusChanged(xsc);
		}
		else if (!strcmp(cws->szSetting, "XStatusMsg")) {
			smi.compare = CompareStatusMsg(&smi, cws, "LastXStatusMsg");
			if (smi.compare == COMPARE_SAME) {
				replaceStrT(smi.newstatusmsg, 0);
				replaceStrT(smi.oldstatusmsg, 0);
			}

			if (cws->value.type == DBVT_DELETED)
				db_unset(hContact, "UserOnline", "LastXStatusMsg");
			else
				db_set(hContact, "UserOnline", "LastXStatusMsg", &cws->value);

			xsc = NewXSC(hContact, szProto, TYPE_ICQ_XSTATUS, smi.compare * 4, NULL, smi.newstatusmsg);
			ExtraStatusChanged(xsc);
		}
		return 1;
	}

	return 0;
}

int ProcessStatusMessage(DBCONTACTWRITESETTING *cws, MCONTACT hContact)
{
	STATUSMSGINFO smi;
	bool bEnablePopup = true, bEnableSound = true;
	char *szProto = GetContactProto(hContact);

	smi.proto = szProto;
	smi.hContact = hContact;
	smi.compare = CompareStatusMsg(&smi, cws, "LastStatusMsg");
	if (smi.compare == COMPARE_SAME)
		goto skip_notify;

	if (cws->value.type == DBVT_DELETED)
		db_unset(hContact, "UserOnline", "LastStatusMsg");
	else
		db_set(hContact, "UserOnline", "LastStatusMsg", &cws->value);

	//don't show popup when mradio connecting and disconnecting
	if (_stricmp(szProto, "mRadio") == 0 && !cws->value.type == DBVT_DELETED) {
		TCHAR buf[MAX_PATH];
		mir_sntprintf(buf, SIZEOF(buf), _T(" (%s)"), TranslateT("connecting"));
		ptrA pszUtf(mir_utf8encodeT(buf));
		mir_sntprintf(buf, SIZEOF(buf), _T(" (%s)"), TranslateT("aborting"));
		ptrA pszUtf2(mir_utf8encodeT(buf));
		mir_sntprintf(buf, SIZEOF(buf), _T(" (%s)"), TranslateT("playing"));
		ptrA pszUtf3(mir_utf8encodeT(buf));
		if (_stricmp(cws->value.pszVal, pszUtf) == 0 || _stricmp(cws->value.pszVal, pszUtf2) == 0 || _stricmp(cws->value.pszVal, pszUtf3) == 0)
			goto skip_notify;
	}

	// check per-contact ignored events
	if (db_get_b(hContact, MODULE, "EnableSMsgNotify", 1) == 0)
		bEnableSound = bEnablePopup = false;

	// we're offline or just connecting
	WORD myStatus = (WORD)CallProtoService(szProto, PS_GETSTATUS, 0, 0);
	if (myStatus == ID_STATUS_OFFLINE)
		goto skip_notify;

	char dbSetting[64];
	mir_snprintf(dbSetting, SIZEOF(dbSetting), "%s_enabled", szProto);
	// this proto is not set for status message notifications
	if (db_get_b(NULL, MODULE, dbSetting, 1) == 0)
		goto skip_notify;
	mir_snprintf(dbSetting, SIZEOF(dbSetting), "%d", IDC_CHK_STATUS_MESSAGE);
	// status message change notifications are disabled
	if (db_get_b(NULL, MODULE, dbSetting, 1) == 0)
		goto skip_notify;

	if (SkipHiddenContact(hContact))
		goto skip_notify;

	// check if our status isn't on autodisable list
	if (opt.AutoDisable) {
		char statusIDs[12], statusIDp[12];
		mir_snprintf(statusIDs, SIZEOF(statusIDs), "s%d", myStatus);
		mir_snprintf(statusIDp, SIZEOF(statusIDp), "p%d", myStatus);
		bEnableSound = db_get_b(0, MODULE, statusIDs, 1) ? FALSE : bEnableSound;
		bEnablePopup = db_get_b(0, MODULE, statusIDp, 1) ? FALSE : bEnablePopup;
	}

	// check flags
	if ((!(templates.PopupSMsgFlags & NOTIFY_REMOVE_MESSAGE) && (smi.compare == COMPARE_DEL))
		|| (!(templates.PopupSMsgFlags & NOTIFY_NEW_MESSAGE) && (smi.compare == COMPARE_DIFF)))
		bEnablePopup = false;

	if (db_get_b(0, MODULE, szProto, 1) == 0 && !opt.PSMsgOnConnect)
		bEnablePopup = false;

	if (bEnablePopup && db_get_b(hContact, MODULE, "EnablePopups", 1) && !opt.TempDisabled) {
		// cut message if needed
		TCHAR *copyText = NULL;
		if (opt.PSMsgTruncate && (opt.PSMsgLen > 0) && smi.newstatusmsg && (_tcslen(smi.newstatusmsg) > opt.PSMsgLen)) {
			TCHAR buff[MAX_TEXT_LEN + 3];
			copyText = mir_tstrdup(smi.newstatusmsg);
			_tcsncpy(buff, smi.newstatusmsg, opt.PSMsgLen);
			buff[opt.PSMsgLen] = 0;
			_tcscat(buff, _T("..."));
			replaceStrT(smi.newstatusmsg, buff);
		}

		TCHAR *str;
		if (smi.compare == COMPARE_DEL) {
			char protoname[MAX_PATH];
			mir_snprintf(protoname, SIZEOF(protoname), "%s_TPopupSMsgRemoved", szProto);
			DBVARIANT dbVar = { 0 };
			if (db_get_ts(NULL, MODULE, protoname, &dbVar)) {
				str = GetStr(&smi, DEFAULT_POPUP_SMSGREMOVED);
			}
			else  {
				str = GetStr(&smi, dbVar.ptszVal);
				db_free(&dbVar);
			}
		}
		else {
			char protoname[MAX_PATH];
			mir_snprintf(protoname, SIZEOF(protoname), "%s_TPopupSMsgChanged", szProto);
			DBVARIANT dbVar = { 0 };
			if (db_get_ts(NULL, MODULE, protoname, &dbVar)) {
				str = GetStr(&smi, DEFAULT_POPUP_SMSGCHANGED);
			}
			else {
				str = GetStr(&smi, dbVar.ptszVal);
				db_free(&dbVar);
			}
		}

		ShowChangePopup(hContact,
			LoadSkinnedProtoIcon(szProto, db_get_w(hContact, szProto, "Status", ID_STATUS_ONLINE)),
			ID_STATUS_STATUSMSG, str);

		mir_free(str);

		if (copyText) {
			mir_free(smi.newstatusmsg);
			smi.newstatusmsg = copyText;
		}
	}

	if (opt.BlinkIcon && opt.BlinkIcon_ForMsgs && !opt.TempDisabled) {
		HICON hIcon = opt.BlinkIcon_Status ? LoadSkinnedProtoIcon(szProto, db_get_w(hContact, szProto, "Status", ID_STATUS_ONLINE)) : LoadSkinnedIcon(SKINICON_OTHER_USERONLINE);
		TCHAR str[256];
		mir_sntprintf(str, SIZEOF(str), TranslateT("%s changed status message to %s"),
			CallService(MS_CLIST_GETCONTACTDISPLAYNAME, hContact, GCDNF_TCHAR), smi.newstatusmsg);
		BlinkIcon(hContact, hIcon, str);
	}

	if (bEnableSound && db_get_b(0, "Skin", "UseSound", TRUE) && db_get_b(hContact, MODULE, "EnableSounds", 1) && !opt.TempDisabled) {
		if (smi.compare == COMPARE_DEL)
			PlayChangeSound(hContact, StatusListEx[ID_STATUS_SMSGREMOVED].lpzSkinSoundName);
		else
			PlayChangeSound(hContact, StatusListEx[ID_STATUS_SMSGCHANGED].lpzSkinSoundName);
	}

	BOOL bEnableLog = opt.SMsgLogToDB && db_get_b(hContact, MODULE, "EnableSMsgLogging", 1);
	if (bEnableLog && (!opt.SMsgLogToDB_WinOpen || CheckMsgWnd(hContact)))
		LogSMsgToDB(&smi, smi.compare == COMPARE_DEL ? templates.LogSMsgRemoved : templates.LogSMsgChanged);

	if (opt.SMsgLogToFile && db_get_b(hContact, MODULE, "EnableSMsgLogging", 1)) {
		TCHAR stzDate[MAX_STATUSTEXT], stzTime[MAX_STATUSTEXT], stzText[MAX_TEXT_LEN];

		GetTimeFormat(LOCALE_USER_DEFAULT, 0, NULL, _T("HH':'mm"), stzTime, SIZEOF(stzTime));
		GetDateFormat(LOCALE_USER_DEFAULT, 0, NULL, _T("dd/MM/yyyy"), stzDate, SIZEOF(stzDate));

		TCHAR *str;
		if (smi.compare == COMPARE_DEL)
			str = GetStr(&smi, templates.LogSMsgRemoved);
		else
			str = GetStr(&smi, templates.LogSMsgChanged);

		mir_sntprintf(stzText, SIZEOF(stzText), _T("%s, %s. %s %s\r\n"), stzDate, stzTime,
			CallService(MS_CLIST_GETCONTACTDISPLAYNAME, hContact, GCDNF_TCHAR), str);

		LogToFile(stzText);
		mir_free(str);
	}

skip_notify:
	replaceStrT(smi.newstatusmsg, 0);
	replaceStrT(smi.oldstatusmsg, 0);
	return 1;
}

int ContactSettingChanged(WPARAM hContact, LPARAM lParam)
{
	if (hContact == NULL)
		return 0;

	char *szProto = GetContactProto(hContact);
	if (szProto == NULL)
		return 0;

	DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING *)lParam;
	if (db_get_w(hContact, szProto, "Status", ID_STATUS_OFFLINE) != ID_STATUS_OFFLINE)
		if (ProcessExtraStatus(cws, hContact))
			return 0;

	if (!strcmp(cws->szSetting, "Status"))
		if (ProcessStatus(cws, hContact))
			return 0;

	if (!strcmp(cws->szModule, "CList") && !strcmp(cws->szSetting, "StatusMsg"))
		if (ProcessStatusMessage(cws, hContact))
			return 0;

	return 0;
}

int StatusModeChanged(WPARAM wParam, LPARAM lParam)
{
	char *szProto = (char *)lParam;
	if (opt.AutoDisable && (!opt.OnlyGlobalChanges || szProto == NULL)) {
		if (opt.DisablePopupGlobally && ServiceExists(MS_POPUP_QUERY)) {
			char szSetting[12];
			mir_snprintf(szSetting, SIZEOF(szSetting), "p%d", wParam);
			BYTE hlpDisablePopup = db_get_b(0, MODULE, szSetting, 0);

			if (hlpDisablePopup != opt.PopupAutoDisabled) {
				BYTE hlpPopupStatus = (BYTE)CallService(MS_POPUP_QUERY, PUQS_GETSTATUS, 0);
				opt.PopupAutoDisabled = hlpDisablePopup;

				if (hlpDisablePopup) {
					db_set_b(0, MODULE, "OldPopupStatus", hlpPopupStatus);
					CallService(MS_POPUP_QUERY, PUQS_DISABLEPOPUPS, 0);
				}
				else {
					if (hlpPopupStatus == FALSE) {
						if (db_get_b(0, MODULE, "OldPopupStatus", TRUE) == TRUE)
							CallService(MS_POPUP_QUERY, PUQS_ENABLEPOPUPS, 0);
						else
							CallService(MS_POPUP_QUERY, PUQS_DISABLEPOPUPS, 0);
					}
				}
			}
		}

		if (opt.DisableSoundGlobally) {
			char szSetting[12];
			mir_snprintf(szSetting, SIZEOF(szSetting), "s%d", wParam);
			BYTE hlpDisableSound = db_get_b(0, MODULE, szSetting, 0);

			if (hlpDisableSound != opt.SoundAutoDisabled) {
				BYTE hlpUseSound = db_get_b(NULL, "Skin", "UseSound", 1);
				opt.SoundAutoDisabled = hlpDisableSound;

				if (hlpDisableSound) {
					db_set_b(0, MODULE, "OldUseSound", hlpUseSound);
					db_set_b(0, "Skin", "UseSound", FALSE);
				}
				else if (hlpUseSound == FALSE)
					db_set_b(0, "Skin", "UseSound", db_get_b(0, MODULE, "OldUseSound", 1));
			}
		}
	}

	return 0;
}

void InitStatusList()
{
	int index = 0;
	//Online
	index = Index(ID_STATUS_ONLINE);
	mir_tstrncpy(StatusList[index].lpzMStatusText, TranslateT("(M) is back online!"), MAX_STATUSTEXT);
	mir_tstrncpy(StatusList[index].lpzFStatusText, TranslateT("(F) is back online!"), MAX_STATUSTEXT);
	mir_tstrncpy(StatusList[index].lpzUStatusText, TranslateT("(U) is back online!"), MAX_STATUSTEXT);
	mir_tstrncpy(StatusList[index].lpzStandardText, TranslateT("Online"), MAX_STANDARDTEXT);
	mir_strncpy(StatusList[index].lpzSkinSoundName, "UserOnline", MAX_SKINSOUNDNAME);
	mir_tstrncpy(StatusList[index].lpzSkinSoundDesc, LPGENT("User: Online"), MAX_SKINSOUNDDESC);
	mir_tstrncpy(StatusList[index].lpzSkinSoundFile, _T("global.wav"), MAX_PATH);
	StatusList[index].colorBack = db_get_dw(NULL, MODULE, "40072bg", COLOR_BG_AVAILDEFAULT);
	StatusList[index].colorText = db_get_dw(NULL, MODULE, "40072tx", COLOR_TX_DEFAULT);

	//Offline
	index = Index(ID_STATUS_OFFLINE);
	mir_tstrncpy(StatusList[index].lpzMStatusText, TranslateT("(M) went offline! :("), MAX_STATUSTEXT);
	mir_tstrncpy(StatusList[index].lpzFStatusText, TranslateT("(F) went offline! :("), MAX_STATUSTEXT);
	mir_tstrncpy(StatusList[index].lpzUStatusText, TranslateT("(U) went offline! :("), MAX_STATUSTEXT);
	mir_tstrncpy(StatusList[index].lpzStandardText, TranslateT("Offline"), MAX_STANDARDTEXT);
	mir_strncpy(StatusList[index].lpzSkinSoundName, "UserOffline", MAX_SKINSOUNDNAME);
	mir_tstrncpy(StatusList[index].lpzSkinSoundDesc, LPGENT("User: Offline"), MAX_SKINSOUNDDESC);
	mir_tstrncpy(StatusList[index].lpzSkinSoundFile, _T("offline.wav"), MAX_PATH);
	StatusList[index].colorBack = db_get_dw(NULL, MODULE, "40071bg", COLOR_BG_NAVAILDEFAULT);
	StatusList[index].colorText = db_get_dw(NULL, MODULE, "40071tx", COLOR_TX_DEFAULT);

	//Invisible
	index = Index(ID_STATUS_INVISIBLE);
	mir_tstrncpy(StatusList[index].lpzMStatusText, TranslateT("(M) hides in shadows..."), MAX_STATUSTEXT);
	mir_tstrncpy(StatusList[index].lpzFStatusText, TranslateT("(F) hides in shadows..."), MAX_STATUSTEXT);
	mir_tstrncpy(StatusList[index].lpzUStatusText, TranslateT("(U) hides in shadows..."), MAX_STATUSTEXT);
	mir_tstrncpy(StatusList[index].lpzStandardText, TranslateT("Invisible"), MAX_STANDARDTEXT);
	mir_strncpy(StatusList[index].lpzSkinSoundName, "UserInvisible", MAX_SKINSOUNDNAME);
	mir_tstrncpy(StatusList[index].lpzSkinSoundDesc, LPGENT("User: Invisible"), MAX_SKINSOUNDDESC);
	mir_tstrncpy(StatusList[index].lpzSkinSoundFile, _T("invisible.wav"), MAX_PATH);
	StatusList[index].colorBack = db_get_dw(NULL, MODULE, "40078bg", COLOR_BG_AVAILDEFAULT);
	StatusList[index].colorText = db_get_dw(NULL, MODULE, "40078tx", COLOR_TX_DEFAULT);

	//Free for chat
	index = Index(ID_STATUS_FREECHAT);
	mir_tstrncpy(StatusList[index].lpzMStatusText, TranslateT("(M) feels talkative!"), MAX_STATUSTEXT);
	mir_tstrncpy(StatusList[index].lpzFStatusText, TranslateT("(F) feels talkative!"), MAX_STATUSTEXT);
	mir_tstrncpy(StatusList[index].lpzUStatusText, TranslateT("(U) feels talkative!"), MAX_STATUSTEXT);
	mir_tstrncpy(StatusList[index].lpzStandardText, TranslateT("Free for chat"), MAX_STANDARDTEXT);
	mir_strncpy(StatusList[index].lpzSkinSoundName, "UserFreeForChat", MAX_SKINSOUNDNAME);
	mir_tstrncpy(StatusList[index].lpzSkinSoundDesc, LPGENT("User: Free for chat"), MAX_SKINSOUNDDESC);
	mir_tstrncpy(StatusList[index].lpzSkinSoundFile, _T("free4chat.wav"), MAX_PATH);
	StatusList[index].colorBack = db_get_dw(NULL, MODULE, "40077bg", COLOR_BG_AVAILDEFAULT);
	StatusList[index].colorText = db_get_dw(NULL, MODULE, "40077tx", COLOR_TX_DEFAULT);

	//Away
	index = Index(ID_STATUS_AWAY);
	mir_tstrncpy(StatusList[index].lpzMStatusText, TranslateT("(M) went away"), MAX_STATUSTEXT);
	mir_tstrncpy(StatusList[index].lpzFStatusText, TranslateT("(F) went away"), MAX_STATUSTEXT);
	mir_tstrncpy(StatusList[index].lpzUStatusText, TranslateT("(U) went away"), MAX_STATUSTEXT);
	mir_tstrncpy(StatusList[index].lpzStandardText, TranslateT("Away"), MAX_STANDARDTEXT);
	mir_strncpy(StatusList[index].lpzSkinSoundName, "UserAway", MAX_SKINSOUNDNAME);
	mir_tstrncpy(StatusList[index].lpzSkinSoundDesc, LPGENT("User: Away"), MAX_SKINSOUNDDESC);
	mir_tstrncpy(StatusList[index].lpzSkinSoundFile, _T("away.wav"), MAX_PATH);
	StatusList[index].colorBack = db_get_dw(NULL, MODULE, "40073bg", COLOR_BG_NAVAILDEFAULT);
	StatusList[index].colorText = db_get_dw(NULL, MODULE, "40073tx", COLOR_TX_DEFAULT);

	//NA
	index = Index(ID_STATUS_NA);
	mir_tstrncpy(StatusList[index].lpzMStatusText, TranslateT("(M) isn't there anymore!"), MAX_STATUSTEXT);
	mir_tstrncpy(StatusList[index].lpzFStatusText, TranslateT("(F) isn't there anymore!"), MAX_STATUSTEXT);
	mir_tstrncpy(StatusList[index].lpzUStatusText, TranslateT("(U) isn't there anymore!"), MAX_STATUSTEXT);
	mir_tstrncpy(StatusList[index].lpzStandardText, TranslateT("NA"), MAX_STANDARDTEXT);
	mir_strncpy(StatusList[index].lpzSkinSoundName, "UserNA", MAX_SKINSOUNDNAME);
	mir_tstrncpy(StatusList[index].lpzSkinSoundDesc, LPGENT("User: Not available"), MAX_SKINSOUNDDESC);
	mir_tstrncpy(StatusList[index].lpzSkinSoundFile, _T("na.wav"), MAX_PATH);
	StatusList[index].colorBack = db_get_dw(NULL, MODULE, "40075bg", COLOR_BG_NAVAILDEFAULT);
	StatusList[index].colorText = db_get_dw(NULL, MODULE, "40075tx", COLOR_TX_DEFAULT);

	//Occupied
	index = Index(ID_STATUS_OCCUPIED);
	mir_tstrncpy(StatusList[index].lpzMStatusText, TranslateT("(M) has something else to do"), MAX_STATUSTEXT);
	mir_tstrncpy(StatusList[index].lpzFStatusText, TranslateT("(F) has something else to do"), MAX_STATUSTEXT);
	mir_tstrncpy(StatusList[index].lpzUStatusText, TranslateT("(U) has something else to do"), MAX_STATUSTEXT);
	mir_tstrncpy(StatusList[index].lpzStandardText, TranslateT("Occupied"), MAX_STANDARDTEXT);
	mir_strncpy(StatusList[index].lpzSkinSoundName, "UserOccupied", MAX_SKINSOUNDNAME);
	mir_tstrncpy(StatusList[index].lpzSkinSoundDesc, LPGENT("User: Occupied"), MAX_SKINSOUNDDESC);
	mir_tstrncpy(StatusList[index].lpzSkinSoundFile, _T("occupied.wav"), MAX_PATH);
	StatusList[index].colorBack = db_get_dw(NULL, MODULE, "40076bg", COLOR_BG_NAVAILDEFAULT);
	StatusList[index].colorText = db_get_dw(NULL, MODULE, "40076tx", COLOR_TX_DEFAULT);

	//DND
	index = Index(ID_STATUS_DND);
	mir_tstrncpy(StatusList[index].lpzMStatusText, TranslateT("(M) doesn't want to be disturbed!"), MAX_STATUSTEXT);
	mir_tstrncpy(StatusList[index].lpzFStatusText, TranslateT("(F) doesn't want to be disturbed!"), MAX_STATUSTEXT);
	mir_tstrncpy(StatusList[index].lpzUStatusText, TranslateT("(U) doesn't want to be disturbed!"), MAX_STATUSTEXT);
	mir_tstrncpy(StatusList[index].lpzStandardText, TranslateT("DND"), MAX_STANDARDTEXT);
	mir_strncpy(StatusList[index].lpzSkinSoundName, "UserDND", MAX_SKINSOUNDNAME);
	mir_tstrncpy(StatusList[index].lpzSkinSoundDesc, LPGENT("User: Do not disturb"), MAX_SKINSOUNDDESC);
	mir_tstrncpy(StatusList[index].lpzSkinSoundFile, _T("dnd.wav"), MAX_PATH);
	StatusList[index].colorBack = db_get_dw(NULL, MODULE, "40074bg", COLOR_BG_NAVAILDEFAULT);
	StatusList[index].colorText = db_get_dw(NULL, MODULE, "40074tx", COLOR_TX_DEFAULT);

	//Out to lunch
	index = Index(ID_STATUS_OUTTOLUNCH);
	mir_tstrncpy(StatusList[index].lpzMStatusText, TranslateT("(M) is eating something"), MAX_STATUSTEXT);
	mir_tstrncpy(StatusList[index].lpzFStatusText, TranslateT("(F) is eating something"), MAX_STATUSTEXT);
	mir_tstrncpy(StatusList[index].lpzUStatusText, TranslateT("(U) is eating something"), MAX_STATUSTEXT);
	mir_tstrncpy(StatusList[index].lpzStandardText, TranslateT("Out to lunch"), MAX_STANDARDTEXT);
	mir_strncpy(StatusList[index].lpzSkinSoundName, "UserOutToLunch", MAX_SKINSOUNDNAME);
	mir_tstrncpy(StatusList[index].lpzSkinSoundDesc, LPGENT("User: Out to lunch"), MAX_SKINSOUNDDESC);
	mir_tstrncpy(StatusList[index].lpzSkinSoundFile, _T("lunch.wav"), MAX_PATH);
	StatusList[index].colorBack = db_get_dw(NULL, MODULE, "40080bg", COLOR_BG_NAVAILDEFAULT);
	StatusList[index].colorText = db_get_dw(NULL, MODULE, "40080tx", COLOR_TX_DEFAULT);

	//On the phone
	index = Index(ID_STATUS_ONTHEPHONE);
	mir_tstrncpy(StatusList[index].lpzMStatusText, TranslateT("(M) had to answer the phone"), MAX_STATUSTEXT);
	mir_tstrncpy(StatusList[index].lpzFStatusText, TranslateT("(F) had to answer the phone"), MAX_STATUSTEXT);
	mir_tstrncpy(StatusList[index].lpzUStatusText, TranslateT("(U) had to answer the phone"), MAX_STATUSTEXT);
	mir_tstrncpy(StatusList[index].lpzStandardText, TranslateT("On the phone"), MAX_STANDARDTEXT);
	mir_strncpy(StatusList[index].lpzSkinSoundName, "UserOnThePhone", MAX_SKINSOUNDNAME);
	mir_tstrncpy(StatusList[index].lpzSkinSoundDesc, LPGENT("User: On the phone"), MAX_SKINSOUNDDESC);
	mir_tstrncpy(StatusList[index].lpzSkinSoundFile, _T("phone.wav"), MAX_PATH);
	StatusList[index].colorBack = db_get_dw(NULL, MODULE, "40079bg", COLOR_BG_NAVAILDEFAULT);
	StatusList[index].colorText = db_get_dw(NULL, MODULE, "40079tx", COLOR_TX_DEFAULT);

	//Extra status
	index = Index(ID_STATUS_EXTRASTATUS);
	StatusList[index].colorBack = db_get_dw(NULL, MODULE, "40081bg", COLOR_BG_AVAILDEFAULT);
	StatusList[index].colorText = db_get_dw(NULL, MODULE, "40081tx", COLOR_TX_DEFAULT);

	//Status message
	index = Index(ID_STATUS_STATUSMSG);
	StatusList[index].colorBack = db_get_dw(NULL, MODULE, "40082bg", COLOR_BG_AVAILDEFAULT);
	StatusList[index].colorText = db_get_dw(NULL, MODULE, "40082tx", COLOR_TX_DEFAULT);

	//From offline
	index = ID_STATUS_FROMOFFLINE;
	mir_strncpy(StatusListEx[index].lpzSkinSoundName, "UserFromOffline", MAX_SKINSOUNDNAME);
	mir_tstrncpy(StatusListEx[index].lpzSkinSoundDesc, LPGENT("User: from offline (has priority!)"), MAX_SKINSOUNDDESC);

	//Status message removed
	index = ID_STATUS_SMSGREMOVED;
	mir_strncpy(StatusListEx[index].lpzSkinSoundName, "StatusMsgRemoved", MAX_SKINSOUNDNAME);
	mir_tstrncpy(StatusListEx[index].lpzSkinSoundDesc, LPGENT("Status message removed"), MAX_SKINSOUNDDESC);

	//Status message changed
	index = ID_STATUS_SMSGCHANGED;
	mir_strncpy(StatusListEx[index].lpzSkinSoundName, "StatusMsgChanged", MAX_SKINSOUNDNAME);
	mir_tstrncpy(StatusListEx[index].lpzSkinSoundDesc, LPGENT("Status message changed"), MAX_SKINSOUNDDESC);

	//Extra status removed
	index = ID_STATUS_XREMOVED;
	mir_strncpy(StatusListEx[index].lpzSkinSoundName, XSTATUS_SOUND_REMOVED, MAX_SKINSOUNDNAME);
	mir_tstrncpy(StatusListEx[index].lpzSkinSoundDesc, LPGENT("Extra status removed"), MAX_SKINSOUNDDESC);

	//Extra status message changed
	index = ID_STATUS_XMSGCHANGED;
	mir_strncpy(StatusListEx[index].lpzSkinSoundName, XSTATUS_SOUND_MSGCHANGED, MAX_SKINSOUNDNAME);
	mir_tstrncpy(StatusListEx[index].lpzSkinSoundDesc, LPGENT("Extra status message changed"), MAX_SKINSOUNDDESC);

	//Extra status changed
	index = ID_STATUS_XCHANGED;
	mir_strncpy(StatusListEx[index].lpzSkinSoundName, XSTATUS_SOUND_CHANGED, MAX_SKINSOUNDNAME);
	mir_tstrncpy(StatusListEx[index].lpzSkinSoundDesc, LPGENT("Extra status changed"), MAX_SKINSOUNDDESC);

	//Extra status message removed
	index = ID_STATUS_XMSGREMOVED;
	mir_strncpy(StatusListEx[index].lpzSkinSoundName, XSTATUS_SOUND_MSGREMOVED, MAX_SKINSOUNDNAME);
	mir_tstrncpy(StatusListEx[index].lpzSkinSoundDesc, LPGENT("Extra status message removed"), MAX_SKINSOUNDDESC);
}

void CALLBACK ConnectionTimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD)
{
	if (uMsg == WM_TIMER) {
		KillTimer(hwnd, idEvent);

		//We've received a timer message: enable the popups for a specified protocol.
		char szProto[256];
		if (GetAtomNameA((ATOM)idEvent, szProto, sizeof(szProto)) > 0) {
			db_set_b(0, MODULE, szProto, 1);
			DeleteAtom((ATOM)idEvent);
		}
	}
}

int ProtoAck(WPARAM, LPARAM lParam)
{
	ACKDATA *ack = (ACKDATA *)lParam;

	if (ack->type == ACKTYPE_STATUS) {
		WORD newStatus = (WORD)ack->lParam;
		WORD oldStatus = (WORD)ack->hProcess;
		char *szProto = (char *)ack->szModule;

		if (oldStatus == newStatus)
			return 0;

		if (newStatus == ID_STATUS_OFFLINE) {
			//The protocol switched to offline. Disable the popups for this protocol
			db_set_b(NULL, MODULE, szProto, 0);
		}
		else if (oldStatus < ID_STATUS_ONLINE && newStatus >= ID_STATUS_ONLINE) {
			//The protocol changed from a disconnected status to a connected status.
			//Enable the popups for this protocol.
			int idTimer = AddAtomA(szProto);
			if (idTimer)
				SetTimer(SecretWnd, idTimer, (UINT)opt.PopupConnectionTimeout * 1000, ConnectionTimerProc);
		}
	}

	return 0;
}

INT_PTR EnableDisableMenuCommand(WPARAM, LPARAM)
{
	db_set_b(0, MODULE, "TempDisable", opt.TempDisabled = !opt.TempDisabled);

	CLISTMENUITEM mi = { sizeof(mi) };
	mi.flags = CMIM_ICON | CMIM_NAME | CMIF_TCHAR;
	if (opt.TempDisabled) {
		mi.ptszName = LPGENT("Enable status notification");
		mi.icolibItem = GetIconHandle(ICO_NOTIFICATION_OFF);
	}
	else {
		mi.ptszName = LPGENT("Disable status notification");
		mi.icolibItem = GetIconHandle(ICO_NOTIFICATION_ON);
	}

	Menu_ModifyItem(hEnableDisableMenu, &mi);
	CallService(MS_TTB_SETBUTTONSTATE, (WPARAM)hToolbarButton, opt.TempDisabled ? 0 : TTBST_PUSHED);
	return 0;
}

void InitMainMenuItem()
{
	CLISTMENUITEM mi = { sizeof(mi) };
	mi.flags = CMIF_TCHAR;
	mi.ptszPopupName = ServiceExists(MS_POPUP_ADDPOPUPT) ? _T("Popups") : NULL;
	mi.pszService = MS_STATUSCHANGE_MENUCOMMAND;
	hEnableDisableMenu = Menu_AddMainMenuItem(&mi);

	opt.TempDisabled = !opt.TempDisabled;
	EnableDisableMenuCommand(0, 0);
}

static IconItem iconList[] =
{
	{ LPGEN("Notification enabled"), ICO_NOTIFICATION_OFF, IDI_NOTIFICATION_OFF },
	{ LPGEN("Notification disabled"), ICO_NOTIFICATION_ON, IDI_NOTIFICATION_ON }
};

void InitIcolib()
{
	Icon_Register(hInst, LPGEN("New Status Notify"), iconList, SIZEOF(iconList), MODULE);
}

void InitSound()
{
	for (int i = ID_STATUS_MIN; i <= ID_STATUS_MAX; i++)
		SkinAddNewSoundExT(StatusList[Index(i)].lpzSkinSoundName, LPGENT("Status Notify"), StatusList[Index(i)].lpzSkinSoundDesc);

	for (int i = 0; i <= ID_STATUSEX_MAX; i++)
		SkinAddNewSoundExT(StatusListEx[i].lpzSkinSoundName, LPGENT("Status Notify"), StatusListEx[i].lpzSkinSoundDesc);
}

int InitTopToolbar(WPARAM, LPARAM)
{
	TTBButton tbb = { sizeof(tbb) };
	tbb.pszService = MS_STATUSCHANGE_MENUCOMMAND;
	tbb.dwFlags = (opt.TempDisabled ? 0 : TTBBF_PUSHED) | TTBBF_ASPUSHBUTTON;
	tbb.name = LPGEN("Toggle status notification");
	tbb.hIconHandleUp = iconList[0].hIcolib;
	tbb.hIconHandleDn = iconList[1].hIcolib;
	tbb.pszTooltipUp = LPGEN("Enable status notification");
	tbb.pszTooltipDn = LPGEN("Disable status notification");
	hToolbarButton = TopToolbar_AddButton(&tbb);

	return 0;
}

int ModulesLoaded(WPARAM, LPARAM)
{
	InitMainMenuItem();

	HookEvent(ME_USERINFO_INITIALISE, UserInfoInitialise);
	HookEvent(ME_MSG_WINDOWEVENT, OnWindowEvent);
	HookEvent(ME_TTB_MODULELOADED, InitTopToolbar);

	SecretWnd = CreateWindowEx(WS_EX_TOOLWINDOW, _T("static"), _T("ConnectionTimerWindow"), 0,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, HWND_DESKTOP,
		NULL, hInst, NULL);

	int count = 0;
	PROTOACCOUNT **accounts = NULL;
	ProtoEnumAccounts(&count, &accounts);
	for (int i = 0; i < count; i++)
		if (IsAccountEnabled(accounts[i]))
			db_set_b(NULL, MODULE, accounts[i]->szModuleName, 0);

	return 0;
}

static int OnShutdown(WPARAM, LPARAM)
{
	DestroyWindow(SecretWnd);
	return 0;
}

extern "C" int __declspec(dllexport) Load(void)
{
	mir_getLP(&pluginInfoEx);

	//"Service" Hook, used when the DB settings change: we'll monitor the "status" setting.
	HookEvent(ME_DB_CONTACT_SETTINGCHANGED, ContactSettingChanged);

	//We create this Hook which will notify everyone when a contact changes his status.
	hHookContactStatusChanged = CreateHookableEvent(ME_STATUSCHANGE_CONTACTSTATUSCHANGED);

	HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, OnShutdown);

	//We add the option page and the user info page (it's needed because options are loaded after plugins)
	HookEvent(ME_OPT_INITIALISE, OptionsInitialize);

	//This is needed for "NoSound"-like routines.
	HookEvent(ME_CLIST_STATUSMODECHANGE, StatusModeChanged);
	HookEvent(ME_PROTO_ACK, ProtoAck);

	LoadOptions();
	InitStatusList();
	InitIcolib();
	InitSound();

	db_set_resident("MetaContacts", "LastOnline");
	db_set_resident("NewStatusNotify", "LastPopupText");

	// register special type of event
	// there's no need to declare the special service for getting text
	// because a blob contains only text
	DBEVENTTYPEDESCR evtype = { sizeof(evtype) };
	evtype.module = MODULE;
	evtype.eventType = EVENTTYPE_STATUSCHANGE;
	evtype.descr = LPGEN("Status change");
	evtype.eventIcon = iconList[0].hIcolib;
	evtype.flags = DETF_HISTORY | DETF_MSGWINDOW;
	CallService(MS_DB_EVENT_REGISTERTYPE, 0, (LPARAM)&evtype);

	hServiceMenu = CreateServiceFunction(MS_STATUSCHANGE_MENUCOMMAND, EnableDisableMenuCommand);

	return 0;
}

extern "C" int __declspec(dllexport) Unload(void)
{
	DestroyHookableEvent(hHookContactStatusChanged);
	DestroyServiceFunction(hServiceMenu);
	return 0;
}
