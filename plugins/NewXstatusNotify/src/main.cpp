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

#include "stdafx.h"

CMPlugin g_plugin;

LIST<DBEVENT> eventListXStatus(10, PtrKeySortT);
LIST<DBEVENT> eventListStatus(10, PtrKeySortT);
LIST<DBEVENT> eventListSMsg(10, PtrKeySortT);

HANDLE hStatusModeChange, hHookContactStatusChanged, hToolbarButton;
HGENMENU hEnableDisableMenu;

STATUS StatusList[STATUS_COUNT];
STATUS StatusListEx[STATUSEX_COUNT];
HWND SecretWnd;

int ContactStatusChanged(MCONTACT hContact, uint16_t oldStatus, uint16_t newStatus);

IconItem iconList[ICO_MAXID] =
{
	{ LPGEN("Reset"),                  "reset",                  IDI_RESET },
	{ LPGEN("Sounds"),                 "sound",                  IDI_SOUND },
	{ LPGEN("Extra status notify"),    "xstatus",                IDI_XSTATUS },
	{ LPGEN("Disable all"),            "disable_all",            IDI_DISABLEALL },
	{ LPGEN("Enable all"),             "enable_all",             IDI_ENABLEALL },
	{ LPGEN("Variables"),              "variables",              IDI_VARIABLES },
	{ LPGEN("Status message notify"),  "status_message",         IDI_STATUS_MESSAGE },
	{ LPGEN("Extra status logging"),   "logging_xstatus",        IDI_LOGGING_XSTATUS },
	{ LPGEN("Status message logging"), "logging_status_message", IDI_LOGGING_SMSG }
};

/////////////////////////////////////////////////////////////////////////////////////////

PLUGININFOEX pluginInfoEx =
{
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// EBF19652-E434-4D79-9897-91A0FF226F51
	{ 0xebf19652, 0xe434, 0x4d79, { 0x98, 0x97, 0x91, 0xa0, 0xff, 0x22, 0x6f, 0x51 } }
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>(MODULENAME, pluginInfoEx),
	bPopups(MODULENAME, "Popups", true)
{
}

/////////////////////////////////////////////////////////////////////////////////////////

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = { MIID_USERONLINE, MIID_LAST };

/////////////////////////////////////////////////////////////////////////////////////////

uint8_t GetGender(MCONTACT hContact)
{
	char *szProto = Proto_GetBaseAccountName(hContact);
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

static int __inline CheckStr(char *str, int not_empty, int empty)
{
	if (str == nullptr || str[0] == '\0')
		return empty;
	else
		return not_empty;
}

static int __inline CheckStrW(wchar_t *str, int not_empty, int empty)
{
	if (str == nullptr || str[0] == L'\0')
		return empty;
	else
		return not_empty;
}

static int CompareStatusMsg(STATUSMSGINFO *smi, DBCONTACTWRITESETTING *cws_new, char *szSetting)
{
	DBVARIANT dbv_old;
	int ret = -1;

	switch (cws_new->value.type) {
	case DBVT_ASCIIZ:
		smi->newstatusmsg = (CheckStr(cws_new->value.pszVal, 0, 1) ? nullptr : mir_a2u_cp(cws_new->value.pszVal, CP_ACP));
		break;
	case DBVT_UTF8:
		smi->newstatusmsg = (CheckStr(cws_new->value.pszVal, 0, 1) ? nullptr : mir_a2u_cp(cws_new->value.pszVal, CP_UTF8));
		break;
	case DBVT_WCHAR:
		smi->newstatusmsg = (CheckStrW(cws_new->value.pwszVal, 0, 1) ? nullptr : mir_wstrdup(cws_new->value.pwszVal));
		break;
	case DBVT_DELETED:
	default:
		smi->newstatusmsg = nullptr;
		break;
	}

	if (!db_get_s(smi->hContact, "UserOnline", szSetting, &dbv_old, 0)) {
		switch (dbv_old.type) {
		case DBVT_ASCIIZ:
			smi->oldstatusmsg = (CheckStr(dbv_old.pszVal, 0, 1) ? nullptr : mir_a2u_cp(dbv_old.pszVal, CP_ACP));
			break;
		case DBVT_UTF8:
			smi->oldstatusmsg = (CheckStr(dbv_old.pszVal, 0, 1) ? nullptr : mir_a2u_cp(dbv_old.pszVal, CP_UTF8));
			break;
		case DBVT_WCHAR:
			smi->oldstatusmsg = (CheckStrW(dbv_old.pwszVal, 0, 1) ? nullptr : mir_wstrdup(dbv_old.pwszVal));
			break;
		default:
			smi->oldstatusmsg = nullptr;
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

		smi->oldstatusmsg = nullptr;
	}

	return ret;
}

wchar_t* GetStr(STATUSMSGINFO *n, const wchar_t *tmplt)
{
	if (n == nullptr || tmplt == nullptr || tmplt[0] == '\0')
		return nullptr;

	CMStringW res;
	size_t len = mir_wstrlen(tmplt);

	for (size_t i = 0; i < len; i++) {
		if (tmplt[i] == '%') {
			i++;
			switch (tmplt[i]) {
			case 'n':
				if (n->compare == COMPARE_DEL || mir_wstrcmp(n->newstatusmsg, TranslateT("<no status message>")) == 0)
					res.Append(TranslateT("<no status message>"));
				else
					AddCR(res, n->newstatusmsg);
				break;

			case 'o':
				if (n->oldstatusmsg == nullptr || n->oldstatusmsg[0] == '\0' || mir_wstrcmp(n->oldstatusmsg, TranslateT("<no status message>")) == 0)
					res.Append(TranslateT("<no status message>"));
				else
					AddCR(res, n->oldstatusmsg);
				break;

			case 'c':
				if (n->hContact == NULL)
					res.Append(TranslateT("Contact"));
				else
					res.Append(Clist_GetContactDisplayName(n->hContact));
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
		else if (tmplt[i] == '\\') {
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
		res.Append(L"...");
	}

	return mir_wstrndup(res, res.GetLength());
}

bool SkipHiddenContact(MCONTACT hContact)
{
	return (!opt.HiddenContactsToo && Contact::IsHidden(hContact));
}

void LogSMsgToDB(STATUSMSGINFO *smi, const wchar_t *tmplt)
{
	ptrW str(GetStr(smi, tmplt));
	T2Utf blob(str);

	DBEVENTINFO dbei = {};
	dbei.cbBlob = (uint32_t)mir_strlen(blob) + 1;
	dbei.pBlob = (uint8_t*)blob;
	dbei.eventType = EVENTTYPE_STATUSCHANGE;
	dbei.flags = DBEF_READ | DBEF_UTF;
	dbei.timestamp = (uint32_t)time(0);
	dbei.szModule = MODULENAME;

	MEVENT hDBEvent = db_event_add(smi->hContact, &dbei);
	if (opt.SMsgLogToDB_WinOpen && opt.SMsgLogToDB_Remove) {
		DBEVENT *dbevent = (DBEVENT *)mir_alloc(sizeof(DBEVENT));
		dbevent->hContact = smi->hContact;
		dbevent->hDBEvent = hDBEvent;
		eventListSMsg.insert(dbevent);
	}
}

void GetStatusText(MCONTACT hContact, uint16_t newStatus, uint16_t oldStatus, wchar_t *stzStatusText)
{
	if (opt.UseAlternativeText) {
		switch (GetGender(hContact)) {
		case GENDER_MALE:
			wcsncpy(stzStatusText, StatusList[Index(newStatus)].lpzMStatusText, MAX_STATUSTEXT);
			break;
		case GENDER_FEMALE:
			wcsncpy(stzStatusText, StatusList[Index(newStatus)].lpzFStatusText, MAX_STATUSTEXT);
			break;
		case GENDER_UNSPECIFIED:
			wcsncpy(stzStatusText, StatusList[Index(newStatus)].lpzUStatusText, MAX_STATUSTEXT);
			break;
		}
	}
	else
		wcsncpy(stzStatusText, StatusList[Index(newStatus)].lpzStandardText, MAX_STATUSTEXT);

	if (opt.ShowPreviousStatus) {
		wchar_t buff[MAX_STATUSTEXT];
		mir_snwprintf(buff, TranslateW(STRING_SHOWPREVIOUSSTATUS), StatusList[Index(oldStatus)].lpzStandardText);
		mir_wstrcat(mir_wstrcat(stzStatusText, L" "), buff);
	}
}

void BlinkIcon(MCONTACT hContact, HICON hIcon, wchar_t *stzText)
{
	CLISTEVENT cle = {};
	cle.flags = CLEF_ONLYAFEW | CLEF_UNICODE;
	cle.hContact = hContact;
	cle.hDbEvent = hContact;
	cle.hIcon = hIcon;
	cle.pszService = "UserOnline/Description";
	cle.szTooltip.w = stzText;
	g_clistApi.pfnAddEvent(&cle);
}

void PlayChangeSound(MCONTACT hContact, const char *name)
{
	if (opt.UseIndSnd) {
		DBVARIANT dbv;
		wchar_t stzSoundFile[MAX_PATH] = { 0 };
		if (!g_plugin.getWString(hContact, name, &dbv)) {
			wcsncpy(stzSoundFile, dbv.pwszVal, _countof(stzSoundFile) - 1);
			db_free(&dbv);
		}

		if (stzSoundFile[0]) {
			//Now make path to IndSound absolute, as it isn't registered
			wchar_t stzSoundPath[MAX_PATH];
			PathToAbsoluteW(stzSoundFile, stzSoundPath);
			Skin_PlaySoundFile(stzSoundPath);
			return;
		}
	}

	if (db_get_b(0, "SkinSoundsOff", name, 0) == 0)
		Skin_PlaySound(name);
}

int ContactStatusChanged(MCONTACT hContact, uint16_t oldStatus, uint16_t newStatus)
{
	if (opt.LogToDB && (!opt.LogToDB_WinOpen || CheckMsgWnd(hContact))) {
		wchar_t stzStatusText[MAX_SECONDLINE] = { 0 };
		GetStatusText(hContact, newStatus, oldStatus, stzStatusText);
		T2Utf blob(stzStatusText);

		DBEVENTINFO dbei = {};
		dbei.cbBlob = (uint32_t)mir_strlen(blob) + 1;
		dbei.pBlob = (uint8_t*)blob;
		dbei.eventType = EVENTTYPE_STATUSCHANGE;
		dbei.flags = DBEF_READ | DBEF_UTF;
		dbei.timestamp = (uint32_t)time(0);
		dbei.szModule = MODULENAME;

		MEVENT hDBEvent = db_event_add(hContact, &dbei);
		if (opt.LogToDB_WinOpen && opt.LogToDB_Remove) {
			DBEVENT *dbevent = (DBEVENT *)mir_alloc(sizeof(DBEVENT));
			dbevent->hContact = hContact;
			dbevent->hDBEvent = hDBEvent;
			eventListStatus.insert(dbevent);
		}
	}

	bool bEnablePopup = true, bEnableSound = true;
	char *szProto = Proto_GetBaseAccountName(hContact);
	int myStatus = Proto_GetStatus(szProto);

	if (!mir_strcmp(szProto, META_PROTO)) { //this contact is Meta
		MCONTACT hSubContact = db_mc_getMostOnline(hContact);
		char *szSubProto = Proto_GetBaseAccountName(hSubContact);
		if (szSubProto == nullptr)
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

		if (g_plugin.getByte(szSubProto, 1) == 0)
			return 0;

		szProto = szSubProto;
	}
	else {
		if (myStatus == ID_STATUS_OFFLINE || g_plugin.getByte(szProto, 1) == 0)
			return 0;
	}

	if (!opt.FromOffline || oldStatus != ID_STATUS_OFFLINE) { // Either it wasn't a change from Offline or we didn't enable that.
		char buff[8];
		mir_snprintf(buff, "%d", newStatus);
		if (g_plugin.getByte(buff, 1) == 0)
			return 0; // "Notify when a contact changes to one of..." is unchecked
	}

	if (SkipHiddenContact(hContact))
		return 0;

	// check if that proto from which we received statuschange notification, isn't in autodisable list
	if (opt.AutoDisable) {
		char statusIDs[12], statusIDp[12];
		mir_snprintf(statusIDs, "s%d", myStatus);
		mir_snprintf(statusIDp, "p%d", myStatus);
		bEnableSound = g_plugin.getByte(statusIDs, 1) ? FALSE : TRUE;
		bEnablePopup = g_plugin.getByte(statusIDp, 1) ? FALSE : TRUE;
	}

	if (bEnablePopup && g_plugin.getByte(hContact, "EnablePopups", 1) && g_plugin.bPopups) {
		int wStatus = Proto_GetStatus(szProto);
		wchar_t str[MAX_SECONDLINE] = { 0 };
		if (opt.ShowStatus)
			GetStatusText(hContact, newStatus, oldStatus, str);

		if (opt.ReadAwayMsg && wStatus != ID_STATUS_INVISIBLE && StatusHasAwayMessage(szProto, newStatus))
			g_plugin.setWString(hContact, "LastPopupText", str);

		PLUGINDATA *pdp = (PLUGINDATA *)mir_calloc(sizeof(PLUGINDATA));
		pdp->oldStatus = oldStatus;
		pdp->newStatus = newStatus;
		pdp->hAwayMsgHook = nullptr;
		pdp->hAwayMsgProcess = nullptr;
		ShowChangePopup(hContact, Skin_LoadProtoIcon(szProto, newStatus), newStatus, str, pdp);
	}

	if (opt.BlinkIcon && g_plugin.bPopups) {
		HICON hIcon = opt.BlinkIcon_Status ? Skin_LoadProtoIcon(szProto, newStatus) : Skin_LoadIcon(SKINICON_OTHER_USERONLINE);
		wchar_t str[256];
		mir_snwprintf(str, TranslateT("%s is now %s"), Clist_GetContactDisplayName(hContact), StatusList[Index(newStatus)].lpzStandardText);
		BlinkIcon(hContact, hIcon, str);
	}

	if (bEnableSound && db_get_b(0, "Skin", "UseSound", TRUE) && g_plugin.getByte(hContact, "EnableSounds", 1) && g_plugin.bPopups) {
		if (oldStatus == ID_STATUS_OFFLINE)
			PlayChangeSound(hContact, StatusListEx[ID_STATUS_FROMOFFLINE].lpzSkinSoundName);
		else
			PlayChangeSound(hContact, StatusList[Index(newStatus)].lpzSkinSoundName);
	}

	if (opt.LogToFile) {
		wchar_t stzDate[MAX_STATUSTEXT], stzTime[MAX_STATUSTEXT], stzText[MAX_TEXT_LEN];

		GetTimeFormat(LOCALE_USER_DEFAULT, 0, nullptr, L"HH':'mm", stzTime, _countof(stzTime));
		GetDateFormat(LOCALE_USER_DEFAULT, 0, nullptr, L"dd/MM/yyyy", stzDate, _countof(stzDate));
		mir_snwprintf(stzText, TranslateT("%s, %s. %s changed status to %s (was %s)\r\n"),
			stzDate, stzTime, Clist_GetContactDisplayName(hContact), StatusList[Index(newStatus)].lpzStandardText,
			StatusList[Index(oldStatus)].lpzStandardText);
		LogToFile(stzText);
	}

	return 0;
}

int ProcessStatus(DBCONTACTWRITESETTING *cws, MCONTACT hContact)
{
	uint16_t newStatus = cws->value.wVal;
	if (newStatus < ID_STATUS_MIN || newStatus > ID_STATUS_MAX)
		return 0;

	char *szProto = Proto_GetBaseAccountName(hContact);
	if (mir_strcmp(cws->szModule, szProto))
		return 0;

	// we don't want to be notified if new chatroom comes online
	if (Contact::IsGroupChat(hContact, szProto))
		return 0;

	uint16_t oldStatus = DBGetContactSettingRangedWord(hContact, "UserOnline", "LastStatus", ID_STATUS_OFFLINE, ID_STATUS_MIN, ID_STATUS_MAX);
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
	if (Ignore_IsIgnored(hContact, IGNOREEVENT_USERONLINE))
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
	char *szProto = Proto_GetBaseAccountName(hContact);
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
				replaceStrW(smi.newstatusmsg, nullptr);
				replaceStrW(smi.oldstatusmsg, nullptr);
			}

			if (cws->value.type == DBVT_DELETED)
				db_unset(hContact, "UserOnline", szSetting);
			else
				db_set(hContact, "UserOnline", szSetting, &cws->value);

			xsc = NewXSC(hContact, szProto, type, smi.compare, smi.newstatusmsg, nullptr);
			ExtraStatusChanged(xsc);
		}
		else if (strstr(cws->szSetting, "text")) {
			char dbSetting[128];
			mir_snprintf(dbSetting, "%s%s", szSetting, "Msg");
			smi.compare = CompareStatusMsg(&smi, cws, dbSetting);
			if (smi.compare == COMPARE_SAME) {
				replaceStrW(smi.newstatusmsg, nullptr);
				replaceStrW(smi.oldstatusmsg, nullptr);
			}

			if (cws->value.type == DBVT_DELETED)
				db_unset(hContact, "UserOnline", dbSetting);
			else
				db_set(hContact, "UserOnline", dbSetting, &cws->value);

			xsc = NewXSC(hContact, szProto, type, smi.compare * 4, nullptr, smi.newstatusmsg);
			ExtraStatusChanged(xsc);
		}
		return 1;
	}

	if (strstr(cws->szSetting, "XStatus")) {
		if (mir_strcmp(cws->szModule, szProto))
			return 0;

		if (mir_strcmp(cws->szSetting, "XStatusName") == 0) {
			smi.compare = CompareStatusMsg(&smi, cws, "LastXStatusName");
			if (smi.compare == COMPARE_SAME) {
				replaceStrW(smi.newstatusmsg, nullptr);
				replaceStrW(smi.oldstatusmsg, nullptr);
			}

			if (cws->value.type == DBVT_DELETED)
				db_unset(hContact, "UserOnline", "LastXStatusName");
			else
				db_set(hContact, "UserOnline", "LastXStatusName", &cws->value);

			xsc = NewXSC(hContact, szProto, TYPE_ICQ_XSTATUS, smi.compare, smi.newstatusmsg, nullptr);
			ExtraStatusChanged(xsc);
		}
		else if (!mir_strcmp(cws->szSetting, "XStatusMsg")) {
			smi.compare = CompareStatusMsg(&smi, cws, "LastXStatusMsg");
			if (smi.compare == COMPARE_SAME) {
				replaceStrW(smi.newstatusmsg, nullptr);
				replaceStrW(smi.oldstatusmsg, nullptr);
			}

			if (cws->value.type == DBVT_DELETED)
				db_unset(hContact, "UserOnline", "LastXStatusMsg");
			else
				db_set(hContact, "UserOnline", "LastXStatusMsg", &cws->value);

			xsc = NewXSC(hContact, szProto, TYPE_ICQ_XSTATUS, smi.compare * 4, nullptr, smi.newstatusmsg);
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
	char *szProto = Proto_GetBaseAccountName(hContact);

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
		wchar_t buf[MAX_PATH];
		mir_snwprintf(buf, L" (%s)", TranslateT("connecting"));
		T2Utf pszUtf(buf);
		mir_snwprintf(buf, L" (%s)", TranslateT("aborting"));
		T2Utf pszUtf2(buf);
		mir_snwprintf(buf, L" (%s)", TranslateT("playing"));
		T2Utf pszUtf3(buf);
		if (_stricmp(cws->value.pszVal, pszUtf) == 0 || _stricmp(cws->value.pszVal, pszUtf2) == 0 || _stricmp(cws->value.pszVal, pszUtf3) == 0)
			goto skip_notify;
	}

	// check per-contact ignored events
	if (g_plugin.getByte(hContact, "EnableSMsgNotify", 1) == 0)
		bEnableSound = bEnablePopup = false;

	// we're offline or just connecting
	int myStatus = Proto_GetStatus(szProto);
	if (myStatus == ID_STATUS_OFFLINE)
		goto skip_notify;

	char dbSetting[64];
	mir_snprintf(dbSetting, "%s_enabled", szProto);
	// this proto is not set for status message notifications
	if (g_plugin.getByte(dbSetting, 1) == 0)
		goto skip_notify;
	mir_snprintf(dbSetting, "%d", IDC_CHK_STATUS_MESSAGE);
	// status message change notifications are disabled
	if (g_plugin.getByte(dbSetting, 1) == 0)
		goto skip_notify;

	if (SkipHiddenContact(hContact))
		goto skip_notify;

	// check if our status isn't on autodisable list
	if (opt.AutoDisable) {
		char statusIDs[12], statusIDp[12];
		mir_snprintf(statusIDs, "s%d", myStatus);
		mir_snprintf(statusIDp, "p%d", myStatus);
		bEnableSound = g_plugin.getByte(statusIDs, 1) ? FALSE : bEnableSound;
		bEnablePopup = g_plugin.getByte(statusIDp, 1) ? FALSE : bEnablePopup;
	}

	// check flags
	if ((!(templates.PopupSMsgFlags & NOTIFY_REMOVE_MESSAGE) && (smi.compare == COMPARE_DEL))
		|| (!(templates.PopupSMsgFlags & NOTIFY_NEW_MESSAGE) && (smi.compare == COMPARE_DIFF)))
		bEnablePopup = false;

	if (g_plugin.getByte(szProto, 1) == 0 && !opt.PSMsgOnConnect)
		bEnablePopup = false;

	if (bEnablePopup && g_plugin.getByte(hContact, "EnablePopups", 1) && g_plugin.bPopups) {
		// cut message if needed
		wchar_t *copyText = nullptr;
		if (opt.PSMsgTruncate && (opt.PSMsgLen > 0) && smi.newstatusmsg && (mir_wstrlen(smi.newstatusmsg) > opt.PSMsgLen)) {
			wchar_t buff[MAX_TEXT_LEN + 3];
			copyText = mir_wstrdup(smi.newstatusmsg);
			wcsncpy(buff, smi.newstatusmsg, opt.PSMsgLen);
			buff[opt.PSMsgLen] = 0;
			mir_wstrcat(buff, L"...");
			replaceStrW(smi.newstatusmsg, buff);
		}

		wchar_t *str;
		if (smi.compare == COMPARE_DEL) {
			char protoname[MAX_PATH];
			mir_snprintf(protoname, "%s_TPopupSMsgRemoved", szProto);
			DBVARIANT dbVar = { 0 };
			if (g_plugin.getWString(protoname, &dbVar)) {
				str = GetStr(&smi, DEFAULT_POPUP_SMSGREMOVED);
			}
			else {
				str = GetStr(&smi, dbVar.pwszVal);
				db_free(&dbVar);
			}
		}
		else {
			char protoname[MAX_PATH];
			mir_snprintf(protoname, "%s_TPopupSMsgChanged", szProto);
			DBVARIANT dbVar = { 0 };
			if (g_plugin.getWString(protoname, &dbVar)) {
				str = GetStr(&smi, DEFAULT_POPUP_SMSGCHANGED);
			}
			else {
				str = GetStr(&smi, dbVar.pwszVal);
				db_free(&dbVar);
			}
		}

		ShowChangePopup(hContact,
			Skin_LoadProtoIcon(szProto, db_get_w(hContact, szProto, "Status", ID_STATUS_ONLINE)),
			ID_STATUS_STATUSMSG, str);

		mir_free(str);

		if (copyText) {
			mir_free(smi.newstatusmsg);
			smi.newstatusmsg = copyText;
		}
	}

	if (opt.BlinkIcon && opt.BlinkIcon_ForMsgs && g_plugin.bPopups) {
		HICON hIcon = opt.BlinkIcon_Status ? Skin_LoadProtoIcon(szProto, db_get_w(hContact, szProto, "Status", ID_STATUS_ONLINE)) : Skin_LoadIcon(SKINICON_OTHER_USERONLINE);
		wchar_t str[256];
		mir_snwprintf(str, TranslateT("%s changed status message to %s"), Clist_GetContactDisplayName(hContact), smi.newstatusmsg);
		BlinkIcon(hContact, hIcon, str);
	}

	if (bEnableSound && db_get_b(0, "Skin", "UseSound", TRUE) && g_plugin.getByte(hContact, "EnableSounds", 1) && g_plugin.bPopups) {
		if (smi.compare == COMPARE_DEL)
			PlayChangeSound(hContact, StatusListEx[ID_STATUS_SMSGREMOVED].lpzSkinSoundName);
		else
			PlayChangeSound(hContact, StatusListEx[ID_STATUS_SMSGCHANGED].lpzSkinSoundName);
	}

	BOOL bEnableLog = opt.SMsgLogToDB && g_plugin.getByte(hContact, "EnableSMsgLogging", 1);
	if (bEnableLog && (!opt.SMsgLogToDB_WinOpen || CheckMsgWnd(hContact)))
		LogSMsgToDB(&smi, smi.compare == COMPARE_DEL ? templates.LogSMsgRemoved : templates.LogSMsgChanged);

	if (opt.SMsgLogToFile && g_plugin.getByte(hContact, "EnableSMsgLogging", 1)) {
		wchar_t stzDate[MAX_STATUSTEXT], stzTime[MAX_STATUSTEXT], stzText[MAX_TEXT_LEN];

		GetTimeFormat(LOCALE_USER_DEFAULT, 0, nullptr, L"HH':'mm", stzTime, _countof(stzTime));
		GetDateFormat(LOCALE_USER_DEFAULT, 0, nullptr, L"dd/MM/yyyy", stzDate, _countof(stzDate));

		wchar_t *str;
		if (smi.compare == COMPARE_DEL)
			str = GetStr(&smi, templates.LogSMsgRemoved);
		else
			str = GetStr(&smi, templates.LogSMsgChanged);

		mir_snwprintf(stzText, L"%s, %s. %s %s\r\n", stzDate, stzTime, Clist_GetContactDisplayName(hContact), str);

		LogToFile(stzText);
		mir_free(str);
	}

skip_notify:
	replaceStrW(smi.newstatusmsg, nullptr);
	replaceStrW(smi.oldstatusmsg, nullptr);
	return 1;
}

int ContactSettingChanged(WPARAM hContact, LPARAM lParam)
{
	if (hContact == NULL)
		return 0;

	char *szProto = Proto_GetBaseAccountName(hContact);
	if (szProto == nullptr)
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
	if (opt.AutoDisable && (!opt.OnlyGlobalChanges || szProto == nullptr)) {
		if (opt.DisablePopupGlobally) {
			char szSetting[12];
			mir_snprintf(szSetting, "p%d", wParam);
			uint8_t hlpDisablePopup = g_plugin.getByte(szSetting, 0);

			if (hlpDisablePopup != opt.PopupAutoDisabled) {
				bool hlpPopupStatus = Popup_Enabled();
				opt.PopupAutoDisabled = hlpDisablePopup;

				if (hlpDisablePopup) {
					g_plugin.setByte("OldPopupStatus", hlpPopupStatus);
					Popup_Enable(false);
				}
				else if (!hlpPopupStatus)
					Popup_Enable(g_plugin.getByte("OldPopupStatus", true));
			}
		}

		if (opt.DisableSoundGlobally) {
			char szSetting[12];
			mir_snprintf(szSetting, "s%d", wParam);
			uint8_t hlpDisableSound = g_plugin.getByte(szSetting, 0);

			if (hlpDisableSound != opt.SoundAutoDisabled) {
				uint8_t hlpUseSound = db_get_b(0, "Skin", "UseSound", 1);
				opt.SoundAutoDisabled = hlpDisableSound;

				if (hlpDisableSound) {
					g_plugin.setByte("OldUseSound", hlpUseSound);
					db_set_b(0, "Skin", "UseSound", FALSE);
				}
				else if (hlpUseSound == FALSE)
					db_set_b(0, "Skin", "UseSound", g_plugin.getByte("OldUseSound", 1));
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
	mir_wstrncpy(StatusList[index].lpzMStatusText, TranslateT("(M) is back online!"), MAX_STATUSTEXT);
	mir_wstrncpy(StatusList[index].lpzFStatusText, TranslateT("(F) is back online!"), MAX_STATUSTEXT);
	mir_wstrncpy(StatusList[index].lpzUStatusText, TranslateT("(U) is back online!"), MAX_STATUSTEXT);
	mir_wstrncpy(StatusList[index].lpzStandardText, TranslateT("Online"), MAX_STANDARDTEXT);
	mir_strncpy(StatusList[index].lpzSkinSoundName, "UserOnline", MAX_SKINSOUNDNAME);
	mir_wstrncpy(StatusList[index].lpzSkinSoundDesc, LPGENW("User: Online"), MAX_SKINSOUNDDESC);
	mir_wstrncpy(StatusList[index].lpzSkinSoundFile, L"global.wav", MAX_PATH);
	StatusList[index].colorBack = g_plugin.getDword("40072bg", COLOR_BG_AVAILDEFAULT);
	StatusList[index].colorText = g_plugin.getDword("40072tx", COLOR_TX_DEFAULT);

	//Offline
	index = Index(ID_STATUS_OFFLINE);
	mir_wstrncpy(StatusList[index].lpzMStatusText, TranslateT("(M) went offline! :("), MAX_STATUSTEXT);
	mir_wstrncpy(StatusList[index].lpzFStatusText, TranslateT("(F) went offline! :("), MAX_STATUSTEXT);
	mir_wstrncpy(StatusList[index].lpzUStatusText, TranslateT("(U) went offline! :("), MAX_STATUSTEXT);
	mir_wstrncpy(StatusList[index].lpzStandardText, TranslateT("Offline"), MAX_STANDARDTEXT);
	mir_strncpy(StatusList[index].lpzSkinSoundName, "UserOffline", MAX_SKINSOUNDNAME);
	mir_wstrncpy(StatusList[index].lpzSkinSoundDesc, LPGENW("User: Offline"), MAX_SKINSOUNDDESC);
	mir_wstrncpy(StatusList[index].lpzSkinSoundFile, L"offline.wav", MAX_PATH);
	StatusList[index].colorBack = g_plugin.getDword("40071bg", COLOR_BG_NAVAILDEFAULT);
	StatusList[index].colorText = g_plugin.getDword("40071tx", COLOR_TX_DEFAULT);

	//Invisible
	index = Index(ID_STATUS_INVISIBLE);
	mir_wstrncpy(StatusList[index].lpzMStatusText, TranslateT("(M) hides in shadows..."), MAX_STATUSTEXT);
	mir_wstrncpy(StatusList[index].lpzFStatusText, TranslateT("(F) hides in shadows..."), MAX_STATUSTEXT);
	mir_wstrncpy(StatusList[index].lpzUStatusText, TranslateT("(U) hides in shadows..."), MAX_STATUSTEXT);
	mir_wstrncpy(StatusList[index].lpzStandardText, TranslateT("Invisible"), MAX_STANDARDTEXT);
	mir_strncpy(StatusList[index].lpzSkinSoundName, "UserInvisible", MAX_SKINSOUNDNAME);
	mir_wstrncpy(StatusList[index].lpzSkinSoundDesc, LPGENW("User: Invisible"), MAX_SKINSOUNDDESC);
	mir_wstrncpy(StatusList[index].lpzSkinSoundFile, L"invisible.wav", MAX_PATH);
	StatusList[index].colorBack = g_plugin.getDword("40078bg", COLOR_BG_AVAILDEFAULT);
	StatusList[index].colorText = g_plugin.getDword("40078tx", COLOR_TX_DEFAULT);
	
	//Free for chat
	index = Index(ID_STATUS_FREECHAT);
	mir_wstrncpy(StatusList[index].lpzMStatusText, TranslateT("(M) feels talkative!"), MAX_STATUSTEXT);
	mir_wstrncpy(StatusList[index].lpzFStatusText, TranslateT("(F) feels talkative!"), MAX_STATUSTEXT);
	mir_wstrncpy(StatusList[index].lpzUStatusText, TranslateT("(U) feels talkative!"), MAX_STATUSTEXT);
	mir_wstrncpy(StatusList[index].lpzStandardText, TranslateT("Free for chat"), MAX_STANDARDTEXT);
	mir_strncpy(StatusList[index].lpzSkinSoundName, "UserFreeForChat", MAX_SKINSOUNDNAME);
	mir_wstrncpy(StatusList[index].lpzSkinSoundDesc, LPGENW("User: Free for chat"), MAX_SKINSOUNDDESC);
	mir_wstrncpy(StatusList[index].lpzSkinSoundFile, L"free4chat.wav", MAX_PATH);
	StatusList[index].colorBack = g_plugin.getDword("40077bg", COLOR_BG_AVAILDEFAULT);
	StatusList[index].colorText = g_plugin.getDword("40077tx", COLOR_TX_DEFAULT);

	//Away
	index = Index(ID_STATUS_AWAY);
	mir_wstrncpy(StatusList[index].lpzMStatusText, TranslateT("(M) went away"), MAX_STATUSTEXT);
	mir_wstrncpy(StatusList[index].lpzFStatusText, TranslateT("(F) went away"), MAX_STATUSTEXT);
	mir_wstrncpy(StatusList[index].lpzUStatusText, TranslateT("(U) went away"), MAX_STATUSTEXT);
	mir_wstrncpy(StatusList[index].lpzStandardText, TranslateT("Away"), MAX_STANDARDTEXT);
	mir_strncpy(StatusList[index].lpzSkinSoundName, "UserAway", MAX_SKINSOUNDNAME);
	mir_wstrncpy(StatusList[index].lpzSkinSoundDesc, LPGENW("User: Away"), MAX_SKINSOUNDDESC);
	mir_wstrncpy(StatusList[index].lpzSkinSoundFile, L"away.wav", MAX_PATH);
	StatusList[index].colorBack = g_plugin.getDword("40073bg", COLOR_BG_NAVAILDEFAULT);
	StatusList[index].colorText = g_plugin.getDword("40073tx", COLOR_TX_DEFAULT);

	//NA
	index = Index(ID_STATUS_NA);
	mir_wstrncpy(StatusList[index].lpzMStatusText, TranslateT("(M) isn't there anymore!"), MAX_STATUSTEXT);
	mir_wstrncpy(StatusList[index].lpzFStatusText, TranslateT("(F) isn't there anymore!"), MAX_STATUSTEXT);
	mir_wstrncpy(StatusList[index].lpzUStatusText, TranslateT("(U) isn't there anymore!"), MAX_STATUSTEXT);
	mir_wstrncpy(StatusList[index].lpzStandardText, TranslateT("Not available"), MAX_STANDARDTEXT);
	mir_strncpy(StatusList[index].lpzSkinSoundName, "UserNA", MAX_SKINSOUNDNAME);
	mir_wstrncpy(StatusList[index].lpzSkinSoundDesc, LPGENW("User: Not available"), MAX_SKINSOUNDDESC);
	mir_wstrncpy(StatusList[index].lpzSkinSoundFile, L"na.wav", MAX_PATH);
	StatusList[index].colorBack = g_plugin.getDword("40075bg", COLOR_BG_NAVAILDEFAULT);
	StatusList[index].colorText = g_plugin.getDword("40075tx", COLOR_TX_DEFAULT);

	//Occupied
	index = Index(ID_STATUS_OCCUPIED);
	mir_wstrncpy(StatusList[index].lpzMStatusText, TranslateT("(M) has something else to do"), MAX_STATUSTEXT);
	mir_wstrncpy(StatusList[index].lpzFStatusText, TranslateT("(F) has something else to do"), MAX_STATUSTEXT);
	mir_wstrncpy(StatusList[index].lpzUStatusText, TranslateT("(U) has something else to do"), MAX_STATUSTEXT);
	mir_wstrncpy(StatusList[index].lpzStandardText, TranslateT("Occupied"), MAX_STANDARDTEXT);
	mir_strncpy(StatusList[index].lpzSkinSoundName, "UserOccupied", MAX_SKINSOUNDNAME);
	mir_wstrncpy(StatusList[index].lpzSkinSoundDesc, LPGENW("User: Occupied"), MAX_SKINSOUNDDESC);
	mir_wstrncpy(StatusList[index].lpzSkinSoundFile, L"occupied.wav", MAX_PATH);
	StatusList[index].colorBack = g_plugin.getDword("40076bg", COLOR_BG_NAVAILDEFAULT);
	StatusList[index].colorText = g_plugin.getDword("40076tx", COLOR_TX_DEFAULT);

	//Do not disturb
	index = Index(ID_STATUS_DND);
	mir_wstrncpy(StatusList[index].lpzMStatusText, TranslateT("(M) doesn't want to be disturbed!"), MAX_STATUSTEXT);
	mir_wstrncpy(StatusList[index].lpzFStatusText, TranslateT("(F) doesn't want to be disturbed!"), MAX_STATUSTEXT);
	mir_wstrncpy(StatusList[index].lpzUStatusText, TranslateT("(U) doesn't want to be disturbed!"), MAX_STATUSTEXT);
	mir_wstrncpy(StatusList[index].lpzStandardText, TranslateT("Do not disturb"), MAX_STANDARDTEXT);
	mir_strncpy(StatusList[index].lpzSkinSoundName, "UserDND", MAX_SKINSOUNDNAME);
	mir_wstrncpy(StatusList[index].lpzSkinSoundDesc, LPGENW("User: Do not disturb"), MAX_SKINSOUNDDESC);
	mir_wstrncpy(StatusList[index].lpzSkinSoundFile, L"dnd.wav", MAX_PATH);
	StatusList[index].colorBack = g_plugin.getDword("40074bg", COLOR_BG_NAVAILDEFAULT);
	StatusList[index].colorText = g_plugin.getDword("40074tx", COLOR_TX_DEFAULT);

	//Extra status
	index = Index(ID_STATUS_EXTRASTATUS);
	StatusList[index].colorBack = g_plugin.getDword("40081bg", COLOR_BG_AVAILDEFAULT);
	StatusList[index].colorText = g_plugin.getDword("40081tx", COLOR_TX_DEFAULT);

	//Status message
	index = Index(ID_STATUS_STATUSMSG);
	StatusList[index].colorBack = g_plugin.getDword("40082bg", COLOR_BG_AVAILDEFAULT);
	StatusList[index].colorText = g_plugin.getDword("40082tx", COLOR_TX_DEFAULT);

	//From offline
	index = ID_STATUS_FROMOFFLINE;
	mir_strncpy(StatusListEx[index].lpzSkinSoundName, "UserFromOffline", MAX_SKINSOUNDNAME);
	mir_wstrncpy(StatusListEx[index].lpzSkinSoundDesc, LPGENW("User: from offline (has priority!)"), MAX_SKINSOUNDDESC);

	//Status message removed
	index = ID_STATUS_SMSGREMOVED;
	mir_strncpy(StatusListEx[index].lpzSkinSoundName, "StatusMsgRemoved", MAX_SKINSOUNDNAME);
	mir_wstrncpy(StatusListEx[index].lpzSkinSoundDesc, LPGENW("Status message removed"), MAX_SKINSOUNDDESC);

	//Status message changed
	index = ID_STATUS_SMSGCHANGED;
	mir_strncpy(StatusListEx[index].lpzSkinSoundName, "StatusMsgChanged", MAX_SKINSOUNDNAME);
	mir_wstrncpy(StatusListEx[index].lpzSkinSoundDesc, LPGENW("Status message changed"), MAX_SKINSOUNDDESC);

	//Extra status removed
	index = ID_STATUS_XREMOVED;
	mir_strncpy(StatusListEx[index].lpzSkinSoundName, XSTATUS_SOUND_REMOVED, MAX_SKINSOUNDNAME);
	mir_wstrncpy(StatusListEx[index].lpzSkinSoundDesc, LPGENW("Extra status removed"), MAX_SKINSOUNDDESC);

	//Extra status message changed
	index = ID_STATUS_XMSGCHANGED;
	mir_strncpy(StatusListEx[index].lpzSkinSoundName, XSTATUS_SOUND_MSGCHANGED, MAX_SKINSOUNDNAME);
	mir_wstrncpy(StatusListEx[index].lpzSkinSoundDesc, LPGENW("Extra status message changed"), MAX_SKINSOUNDDESC);

	//Extra status changed
	index = ID_STATUS_XCHANGED;
	mir_strncpy(StatusListEx[index].lpzSkinSoundName, XSTATUS_SOUND_CHANGED, MAX_SKINSOUNDNAME);
	mir_wstrncpy(StatusListEx[index].lpzSkinSoundDesc, LPGENW("Extra status changed"), MAX_SKINSOUNDDESC);

	//Extra status message removed
	index = ID_STATUS_XMSGREMOVED;
	mir_strncpy(StatusListEx[index].lpzSkinSoundName, XSTATUS_SOUND_MSGREMOVED, MAX_SKINSOUNDNAME);
	mir_wstrncpy(StatusListEx[index].lpzSkinSoundDesc, LPGENW("Extra status message removed"), MAX_SKINSOUNDDESC);
}

void CALLBACK ConnectionTimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD)
{
	if (uMsg == WM_TIMER) {
		KillTimer(hwnd, idEvent);

		//We've received a timer message: enable the popups for a specified protocol.
		char szProto[256];
		if (GetAtomNameA((ATOM)idEvent, szProto, sizeof(szProto)) > 0) {
			g_plugin.setByte(szProto, 1);
			DeleteAtom((ATOM)idEvent);
		}
	}
}

int ProtoAck(WPARAM, LPARAM lParam)
{
	ACKDATA *ack = (ACKDATA *)lParam;

	if (ack->type == ACKTYPE_STATUS) {
		uint16_t newStatus = (uint16_t)ack->lParam;
		uint16_t oldStatus = (DWORD_PTR)ack->hProcess;
		if (oldStatus == newStatus)
			return 0;

		char *szProto = (char *)ack->szModule;
		if (newStatus == ID_STATUS_OFFLINE) {
			//The protocol switched to offline. Disable the popups for this protocol
			g_plugin.setByte(szProto, 0);
		}
		else if (oldStatus < ID_STATUS_ONLINE && newStatus >= ID_STATUS_ONLINE) {
			//The protocol changed from a disconnected status to a connected status.
			//Enable the popups for this protocol.
			ATOM idTimer = AddAtomA(szProto);
			if (idTimer)
				SetTimer(SecretWnd, idTimer, (UINT)opt.PopupConnectionTimeout * 1000, ConnectionTimerProc);
		}
	}

	return 0;
}

static void InitSound()
{
	for (int i = ID_STATUS_MIN; i <= ID_STATUS_MAX; i++)
		g_plugin.addSound(StatusList[Index(i)].lpzSkinSoundName, LPGENW("Status Notify"), StatusList[Index(i)].lpzSkinSoundDesc);

	for (int i = 0; i <= ID_STATUSEX_MAX; i++)
		g_plugin.addSound(StatusListEx[i].lpzSkinSoundName, LPGENW("Status Notify"), StatusListEx[i].lpzSkinSoundDesc);
}

static int ModulesLoaded(WPARAM, LPARAM)
{
	HookEvent(ME_USERINFO_INITIALISE, UserInfoInitialise);
	HookEvent(ME_MSG_WINDOWEVENT, OnWindowEvent);

	SecretWnd = CreateWindowEx(WS_EX_TOOLWINDOW, L"static", L"ConnectionTimerWindow", 0,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, HWND_DESKTOP,
		nullptr, g_plugin.getInst(), nullptr);

	for (auto &pa : Accounts())
		if (pa->IsEnabled())
			g_plugin.setByte(pa->szModuleName, 0);

	return 0;
}

static int OnShutdown(WPARAM, LPARAM)
{
	DestroyWindow(SecretWnd);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int CMPlugin::Load()
{
	if (getByte("TempDisable")) {
		bPopups = false;
		delSetting("TempDisable");
	}

	addPopupOption(LPGEN("Status notifications"), bPopups);

	registerIcon(LPGEN("New Status Notify"), iconList, MODULENAME);

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
	InitSound();

	db_set_resident("MetaContacts", "LastOnline");
	db_set_resident("NewStatusNotify", "LastPopupText");

	// register special type of event
	// there's no need to declare the special service for getting text
	// because a blob contains only text
	DBEVENTTYPEDESCR evtype = {};
	evtype.module = MODULENAME;
	evtype.eventType = EVENTTYPE_STATUSCHANGE;
	evtype.descr = LPGEN("Status change");
	evtype.eventIcon = iconList[ICO_XSTATUS].hIcolib;
	evtype.flags = DETF_HISTORY | DETF_MSGWINDOW;
	DbEvent_RegisterType(&evtype);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int CMPlugin::Unload()
{
	DestroyHookableEvent(hHookContactStatusChanged);
	return 0;
}
