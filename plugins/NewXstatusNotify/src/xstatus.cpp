/*
	NewXstatusNotify YM - Plugin for Miranda IM
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

XSTATUSCHANGE *NewXSC(MCONTACT hContact, char *szProto, int xstatusType, int action, TCHAR *stzTitle, TCHAR *stzText)
{
	XSTATUSCHANGE *xsc = (XSTATUSCHANGE *)mir_alloc(sizeof(XSTATUSCHANGE));
	xsc->hContact = hContact;
	xsc->szProto = szProto;
	xsc->type = xstatusType;
	xsc->action = action;
	xsc->stzTitle = stzTitle;
	xsc->stzText = stzText;
	return xsc;
}

void FreeXSC(XSTATUSCHANGE *xsc)
{
	if (xsc) {
		mir_free(xsc->stzTitle);
		mir_free(xsc->stzText);
		mir_free(xsc);
	}
}

void RemoveLoggedEventsXStatus(MCONTACT hContact)
{
	for (int i = eventListXStatus.getCount() - 1; i >= 0; i--) {
		DBEVENT *dbevent = eventListXStatus[i];
		if (dbevent->hContact == hContact) {
			db_event_delete(dbevent->hContact, dbevent->hDBEvent);
			eventListXStatus.remove(i);
			mir_free(dbevent);
		}
	}
}

void RemoveLoggedEventsStatus(MCONTACT hContact)
{
	for (int i = eventListStatus.getCount() - 1; i >= 0; i--) {
		DBEVENT *dbevent = eventListStatus[i];
		if (dbevent->hContact == hContact) {
			db_event_delete(dbevent->hContact, dbevent->hDBEvent);
			eventListStatus.remove(i);
			mir_free(dbevent);
		}
	}
}

void RemoveLoggedEventsSMsg(MCONTACT hContact)
{
	for (int i = eventListSMsg.getCount() - 1; i >= 0; i--) {
		DBEVENT *dbevent = eventListSMsg[i];
		if (dbevent->hContact == hContact) {
			db_event_delete(dbevent->hContact, dbevent->hDBEvent);
			eventListSMsg.remove(i);
			mir_free(dbevent);
		}
	}
}

TCHAR *GetStatusTypeAsString(int type, TCHAR *buff)
{
	switch (type) {
	case TYPE_JABBER_MOOD:
		_tcscpy(buff, TranslateT("Mood")); return buff;
	case TYPE_JABBER_ACTIVITY:
		_tcscpy(buff, TranslateT("Activity")); return buff;
	case TYPE_ICQ_XSTATUS:
		_tcscpy(buff, TranslateT("xStatus")); return buff;
	default:
		_tcscpy(buff, TranslateT("<unknown>")); return buff;
	}
}

TCHAR *ReplaceVars(XSTATUSCHANGE *xsc, const TCHAR *tmplt)
{
	if (xsc == NULL || tmplt == NULL || tmplt[0] == _T('\0'))
		return NULL;

	TCHAR *str = (TCHAR *)mir_alloc(2048 * sizeof(TCHAR));
	str[0] = _T('\0');
	size_t len = mir_tstrlen(tmplt);

	TCHAR tmp[1024];
	for (size_t i = 0; i < len; i++) {
		tmp[0] = _T('\0');

		if (tmplt[i] == _T('%')) {
			i++;
			switch (tmplt[i]) {
			case 'n':
			{
				TCHAR stzType[32];
				mir_tstrncpy(tmp, GetStatusTypeAsString(xsc->type, stzType), SIZEOF(tmp));
			}
			break;

			case 't':
				if (xsc->stzTitle == NULL || xsc->stzTitle[0] == _T('\0'))
					mir_tstrncpy(tmp, TranslateT("<no title>"), SIZEOF(tmp));
				else
					mir_tstrncpy(tmp, xsc->stzTitle, SIZEOF(tmp));
				break;

			case 'm':
				if (xsc->stzText == NULL || xsc->stzText[0] == _T('\0'))
					mir_tstrncpy(tmp, TranslateT("<no status message>"), SIZEOF(tmp));
				else {
					TCHAR *_tmp = AddCR(xsc->stzText);
					mir_tstrncpy(tmp, _tmp, SIZEOF(tmp));
					mir_free(_tmp);
				}
				break;

			case 'c':
				if (xsc->hContact == NULL)
					mir_tstrncpy(tmp, TranslateT("Contact"), SIZEOF(tmp));
				else
					mir_tstrncpy(tmp, (TCHAR *)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)xsc->hContact, GCDNF_TCHAR), SIZEOF(tmp));
				break;

			default:
				i--;
				tmp[0] = tmplt[i], tmp[1] = _T('\0');
				break;
			}
		}
		else if (tmplt[i] == _T('\\')) {
			i++;
			switch (tmplt[i]) {
			case 'n':
				tmp[0] = _T('\r'), tmp[1] = _T('\n'), tmp[2] = _T('\0');
				break;
			case 't':
				tmp[0] = _T('\t'), tmp[1] = _T('\0');
				break;
			default:
				i--;
				tmp[0] = tmplt[i], tmp[1] = _T('\0');
				break;
			}
		}
		else
			tmp[0] = tmplt[i], tmp[1] = _T('\0');

		if (tmp[0] != _T('\0')) {
			if (mir_tstrlen(tmp) + mir_tstrlen(str) < 2044)
				mir_tstrcat(str, tmp);
			else {
				mir_tstrcat(str, _T("..."));
				break;
			}
		}
	}

	return str;
}

void ShowXStatusPopup(XSTATUSCHANGE *xsc)
{
	if (xsc == NULL)
		return;

	HICON hIcon = NULL;

	switch (xsc->type) {
	case TYPE_JABBER_MOOD:
	case TYPE_JABBER_ACTIVITY:
	{
		DBVARIANT dbv;
		char szSetting[64];
		mir_snprintf(szSetting, SIZEOF(szSetting), "%s/%s/icon", xsc->szProto, (xsc->type == TYPE_JABBER_MOOD) ? "mood" : "activity");
		if (!db_get_s(xsc->hContact, "AdvStatus", szSetting, &dbv)) {
			hIcon = Skin_GetIcon(dbv.pszVal);
			db_free(&dbv);
		}
		break;
	}
	case TYPE_ICQ_XSTATUS:
		int statusId = db_get_b(xsc->hContact, xsc->szProto, "XStatusId", 0);
		hIcon = (HICON)CallProtoService(xsc->szProto, PS_GETCUSTOMSTATUSICON, statusId, LR_SHARED);
	}

	if (hIcon == NULL)
		hIcon = LoadSkinnedProtoIcon(xsc->szProto, db_get_w(xsc->hContact, xsc->szProto, "Status", ID_STATUS_ONLINE));

	// cut message if needed
	TCHAR *copyText = NULL;
	if (opt.PXMsgTruncate && (opt.PXMsgLen > 0) && xsc->stzText && (_tcslen(xsc->stzText) > opt.PXMsgLen)) {
		TCHAR buff[MAX_TEXT_LEN + 3];
		copyText = mir_tstrdup(xsc->stzText);
		_tcsncpy(buff, xsc->stzText, opt.PXMsgLen);
		buff[opt.PXMsgLen] = 0;
		_tcscat(buff, _T("..."));
		mir_free(xsc->stzText);
		xsc->stzText = mir_tstrdup(buff);
	}

	TCHAR *Template = _T("");
	switch (xsc->action) {
	case NOTIFY_NEW_XSTATUS:
		Template = templates.PopupXstatusChanged; break;
	case NOTIFY_NEW_MESSAGE:
		Template = templates.PopupXMsgChanged; break;
	case NOTIFY_REMOVE_XSTATUS:
		Template = templates.PopupXstatusRemoved; break;
	case NOTIFY_REMOVE_MESSAGE:
		Template = templates.PopupXMsgRemoved; break;
	}

	TCHAR *stzPopupText = ReplaceVars(xsc, Template);

	ShowChangePopup(xsc->hContact, hIcon, ID_STATUS_EXTRASTATUS, stzPopupText);
	mir_free(stzPopupText);

	if (copyText) {
		mir_free(xsc->stzText);
		xsc->stzText = copyText;
	}
}

void BlinkXStatusIcon(XSTATUSCHANGE *xsc)
{
	if (xsc == NULL)
		return;

	HICON hIcon = NULL;
	TCHAR str[256] = { 0 };
	TCHAR stzType[32];
	mir_sntprintf(str, SIZEOF(str), TranslateT("%s changed %s"), CallService(MS_CLIST_GETCONTACTDISPLAYNAME, xsc->hContact, GCDNF_TCHAR), GetStatusTypeAsString(xsc->type, stzType));

	if (opt.BlinkIcon_Status) {
		DBVARIANT dbv;
		char szSetting[64];

		switch (xsc->type) {
		case TYPE_JABBER_MOOD:
		case TYPE_JABBER_ACTIVITY:
			mir_snprintf(szSetting, SIZEOF(szSetting), "%s/%s/icon", xsc->szProto, (xsc->type == TYPE_JABBER_MOOD) ? "mood" : "activity");
			if (!db_get_s(xsc->hContact, "AdvStatus", szSetting, &dbv)) {
				hIcon = Skin_GetIcon(dbv.pszVal);
				db_free(&dbv);
			}
			break;
		case TYPE_ICQ_XSTATUS:
			int statusId = db_get_b(xsc->hContact, xsc->szProto, "XStatusId", 0);
			hIcon = (HICON)CallProtoService(xsc->szProto, PS_GETCUSTOMSTATUSICON, statusId, LR_SHARED);
		}
	}

	if (hIcon == NULL)
		hIcon = LoadSkinnedIcon(SKINICON_OTHER_USERONLINE);

	BlinkIcon(xsc->hContact, hIcon, str);
	mir_free(str);
}

void PlayXStatusSound(MCONTACT hContact, int action)
{
	switch (action) {
	case NOTIFY_NEW_XSTATUS:
		PlayChangeSound(hContact, XSTATUS_SOUND_CHANGED); return;
	case NOTIFY_REMOVE_XSTATUS:
		PlayChangeSound(hContact, XSTATUS_SOUND_REMOVED); return;
	case NOTIFY_NEW_MESSAGE:
		PlayChangeSound(hContact, XSTATUS_SOUND_MSGCHANGED); return;
	case NOTIFY_REMOVE_MESSAGE:
		PlayChangeSound(hContact, XSTATUS_SOUND_MSGREMOVED); return;
	}
}

void LogChangeToDB(XSTATUSCHANGE *xsc)
{
	if (xsc == NULL || (opt.XLogToDB_WinOpen && !CheckMsgWnd(xsc->hContact)))
		return;

	TCHAR *Template = _T("");
	switch (xsc->action) {
	case NOTIFY_NEW_XSTATUS:
		Template = templates.LogXstatusChanged; break;
	case NOTIFY_REMOVE_XSTATUS:
		Template = templates.LogXstatusRemoved; break;
	case NOTIFY_NEW_MESSAGE:
		Template = templates.LogXMsgChanged; break;
	case NOTIFY_REMOVE_MESSAGE:
		Template = templates.LogXMsgRemoved; break;
	case NOTIFY_OPENING_ML:
		Template = templates.LogXstatusOpening; break;
	}

	TCHAR *stzLogText, stzLastLog[2 * MAX_TEXT_LEN];
	stzLogText = ReplaceVars(xsc, Template);
	DBGetStringDefault(xsc->hContact, MODULE, DB_LASTLOG, stzLastLog, SIZEOF(stzLastLog), _T(""));

	//	if (!opt.KeepInHistory || !(opt.PreventIdentical && _tcscmp(stzLastLog, stzLogText) == 0)) {
	if (opt.XLogToDB/* || !(opt.PreventIdentical && _tcscmp(stzLastLog, stzLogText) == 0)*/) {
		db_set_ws(xsc->hContact, MODULE, DB_LASTLOG, stzLogText);

		char *blob = mir_utf8encodeT(stzLogText);

		DBEVENTINFO dbei = { 0 };
		dbei.cbSize = sizeof(dbei);
		dbei.cbBlob = (DWORD)strlen(blob) + 1;
		dbei.pBlob = (PBYTE)blob;
		dbei.eventType = EVENTTYPE_STATUSCHANGE;
		dbei.flags = DBEF_READ | DBEF_UTF;

		dbei.timestamp = (DWORD)time(NULL);
		dbei.szModule = MODULE;
		MEVENT hDBEvent = db_event_add(xsc->hContact, &dbei);
		mir_free(blob);

		if (opt.XLogToDB_WinOpen && opt.XLogToDB_Remove) {
			DBEVENT *dbevent = (DBEVENT *)mir_alloc(sizeof(DBEVENT));
			dbevent->hContact = xsc->hContact;
			dbevent->hDBEvent = hDBEvent;
			eventListXStatus.insert(dbevent);
		}
	}
	mir_free(stzLogText);
}

void LogChangeToFile(XSTATUSCHANGE *xsc)
{
	if (xsc == NULL)
		return;

	TCHAR stzDate[32], stzTime[32], stzText[MAX_TEXT_LEN];

	GetTimeFormat(LOCALE_USER_DEFAULT, 0, NULL, _T("HH':'mm"), stzTime, SIZEOF(stzTime));
	GetDateFormat(LOCALE_USER_DEFAULT, 0, NULL, _T("dd/MM/yyyy"), stzDate, SIZEOF(stzDate));

	TCHAR *Template = _T("");
	switch (xsc->action) {
	case NOTIFY_NEW_XSTATUS:
		Template = templates.LogXstatusChanged; break;
	case NOTIFY_REMOVE_XSTATUS:
		Template = templates.LogXstatusRemoved; break;
	case NOTIFY_NEW_MESSAGE:
		Template = templates.LogXMsgChanged; break;
	case NOTIFY_REMOVE_MESSAGE:
		Template = templates.LogXMsgRemoved; break;
	}

	TCHAR *stzLogText = ReplaceVars(xsc, Template);

	mir_sntprintf(stzText, SIZEOF(stzText), _T("%s, %s. %s %s\r\n"), stzDate, stzTime, CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)xsc->hContact, GCDNF_TCHAR), stzLogText);

	LogToFile(stzText);
	mir_free(stzLogText);
}

void ExtraStatusChanged(XSTATUSCHANGE *xsc)
{
	if (xsc == NULL)
		return;

	BOOL bEnablePopup = true, bEnableSound = true, bEnableLog = opt.XLogToDB;

	char buff[12] = { 0 };
	mir_snprintf(buff, SIZEOF(buff), "%d", ID_STATUS_EXTRASTATUS);
	if ((db_get_b(0, MODULE, buff, 1) == 0)
		|| (db_get_w(xsc->hContact, xsc->szProto, "Status", ID_STATUS_OFFLINE) == ID_STATUS_OFFLINE)
		|| (!opt.HiddenContactsToo && (db_get_b(xsc->hContact, "CList", "Hidden", 0) == 1))
		|| (CallProtoService(xsc->szProto, PS_GETSTATUS, 0, 0) == ID_STATUS_OFFLINE))
	{
		FreeXSC(xsc);
		return;
	}

	// check per-contact ignored events
	if (db_get_b(xsc->hContact, MODULE, "EnableXStatusNotify", 1) == 0)
		bEnableSound = bEnablePopup = false;

	// check if our status isn't on autodisable list
	if (opt.AutoDisable) {
		char statusIDs[12], statusIDp[12];
		WORD myStatus = (WORD)CallProtoService(xsc->szProto, PS_GETSTATUS, 0, 0);
		mir_snprintf(statusIDs, SIZEOF(statusIDs), "s%d", myStatus);
		mir_snprintf(statusIDp, SIZEOF(statusIDp), "p%d", myStatus);
		bEnableSound = db_get_b(0, MODULE, statusIDs, 1) ? FALSE : bEnableSound;
		bEnablePopup = db_get_b(0, MODULE, statusIDp, 1) ? FALSE : bEnablePopup;
	}

	if (!(templates.PopupXFlags & xsc->action))
		bEnablePopup = false;

	if (db_get_b(0, MODULE, xsc->szProto, 1) == 0 && !opt.PXOnConnect)
		bEnablePopup = false;

	int xstatusID = db_get_b(xsc->hContact, xsc->szProto, "XStatusId", 0);
	if (opt.PXDisableForMusic && xsc->type == TYPE_ICQ_XSTATUS && xstatusID == XSTATUS_MUSIC)
		bEnableSound = bEnablePopup = false;

	if (bEnablePopup && db_get_b(xsc->hContact, MODULE, "EnablePopups", 1) && !opt.TempDisabled)
		ShowXStatusPopup(xsc);

	if (bEnableSound && db_get_b(0, "Skin", "UseSound", 1) && db_get_b(xsc->hContact, MODULE, "EnableSounds", 1) && !opt.TempDisabled)
		PlayXStatusSound(xsc->hContact, xsc->action);

	if (opt.BlinkIcon && opt.BlinkIcon_ForMsgs && !opt.TempDisabled)
		BlinkXStatusIcon(xsc);

	if (opt.XLogDisableForMusic && xsc->type == TYPE_ICQ_XSTATUS && xstatusID == XSTATUS_MUSIC)
		bEnableLog = false;

	if (!(templates.LogXFlags & xsc->action))
		bEnableLog = false;

	if (bEnableLog && db_get_b(xsc->hContact, MODULE, "EnableXLogging", 1))
		LogChangeToDB(xsc);

	if (opt.XLogToFile && db_get_b(xsc->hContact, MODULE, "EnableXLogging", 1))
		LogChangeToFile(xsc);

	FreeXSC(xsc);
}

TCHAR *GetDefaultXstatusName(int statusID, char *szProto, TCHAR *buff, int bufflen)
{
	TCHAR nameBuff[64];
	buff[0] = 0;

	CUSTOM_STATUS xstatus = { 0 };
	xstatus.cbSize = sizeof(CUSTOM_STATUS);
	xstatus.flags = CSSF_MASK_NAME | CSSF_DEFAULT_NAME | CSSF_TCHAR;
	xstatus.ptszName = nameBuff;
	xstatus.wParam = (WPARAM *)&statusID;
	if (!CallProtoService(szProto, PS_GETCUSTOMSTATUSEX, 0, (LPARAM)&xstatus)) {
		_tcsncpy(buff, TranslateTS(nameBuff), bufflen);
		buff[bufflen - 1] = 0;
	}

	return buff;
}

TCHAR *GetIcqXStatus(MCONTACT hContact, char *szProto, char *szValue, TCHAR *buff, int bufflen)
{
	DBVARIANT dbv;
	buff[0] = 0;

	int statusID = db_get_b(hContact, szProto, "XStatusId", -1);
	if (statusID != -1) {
		if (!db_get_ts(hContact, szProto, szValue, &dbv)) {
			if ((strcmp(szValue, "XStatusName") == 0) && dbv.ptszVal[0] == 0)
				GetDefaultXstatusName(statusID, szProto, buff, bufflen);
			else
				_tcsncpy(buff, dbv.ptszVal, bufflen);

			buff[bufflen - 1] = 0;
			db_free(&dbv);
		}
	}

	return buff;
}

TCHAR *GetJabberAdvStatusText(MCONTACT hContact, char *szProto, char *szSlot, char *szValue, TCHAR *buff, int bufflen)
{
	DBVARIANT dbv;
	char szSetting[128];
	buff[0] = 0;

	mir_snprintf(szSetting, SIZEOF(szSetting), "%s/%s/%s", szProto, szSlot, szValue);
	if (!db_get_ts(hContact, "AdvStatus", szSetting, &dbv)) {
		_tcsncpy(buff, dbv.ptszVal, bufflen);
		buff[bufflen - 1] = 0;
		db_free(&dbv);
	}

	return buff;
}

void LogXstatusChange(MCONTACT hContact, char *szProto, int xstatusType, TCHAR *stzTitle, TCHAR *stzText)
{
	XSTATUSCHANGE *xsc = NewXSC(hContact, szProto, xstatusType, NOTIFY_OPENING_ML, stzTitle[0] ? mir_tstrdup(stzTitle) : NULL, stzText[0] ? mir_tstrdup(stzText) : NULL);

	LogChangeToDB(xsc);
	FreeXSC(xsc);
}

void AddXStatusEventThread(void *arg)
{
	MCONTACT hContact = (MCONTACT)arg;

	char *szProto = GetContactProto(hContact);
	if (szProto == NULL)
		return;

	TCHAR stzTitle[MAX_TITLE_LEN], stzText[MAX_TEXT_LEN];
	if (ProtoServiceExists(szProto, JS_PARSE_XMPP_URI)) {
		GetJabberAdvStatusText(hContact, szProto, "mood", "title", stzTitle, SIZEOF(stzTitle));
		if (stzTitle[0]) {
			GetJabberAdvStatusText(hContact, szProto, "mood", "text", stzText, SIZEOF(stzText));
			LogXstatusChange(hContact, szProto, TYPE_JABBER_MOOD, stzTitle, stzText);
		}

		GetJabberAdvStatusText(hContact, szProto, "activity", "title", stzTitle, SIZEOF(stzTitle));
		if (stzTitle[0]) {
			GetJabberAdvStatusText(hContact, szProto, "activity", "text", stzText, SIZEOF(stzText));
			LogXstatusChange(hContact, szProto, TYPE_JABBER_ACTIVITY, stzTitle, stzText);
		}
	}
	else {
		GetIcqXStatus(hContact, szProto, "XStatusName", stzTitle, SIZEOF(stzTitle));
		if (stzTitle[0]) {
			GetIcqXStatus(hContact, szProto, "XStatusMsg", stzText, SIZEOF(stzText));
			LogXstatusChange(hContact, szProto, TYPE_ICQ_XSTATUS, stzTitle, stzText);
		}
	}
}

void AddSMsgEventThread(void *arg)
{
	MCONTACT hContact = (MCONTACT)arg;

	STATUSMSGINFO smi;
	smi.hContact = hContact;
	smi.proto = GetContactProto(hContact);
	if (smi.proto == NULL)
		return;

	smi.newstatusmsg = db_get_tsa(smi.hContact, "CList", "StatusMsg");
	LogSMsgToDB(&smi, templates.LogSMsgOpening);
	mir_free(smi.newstatusmsg);
}

int OnWindowEvent(WPARAM, LPARAM lParam)
{
	MessageWindowEventData *mwed = (MessageWindowEventData *)lParam;
	if (mwed->uType == MSG_WINDOW_EVT_CLOSE) {
		if (opt.XLogToDB && opt.XLogToDB_WinOpen && opt.XLogToDB_Remove)
			RemoveLoggedEventsXStatus(mwed->hContact);

		if (opt.LogToDB && opt.LogToDB_WinOpen && opt.LogToDB_Remove)
			RemoveLoggedEventsStatus(mwed->hContact);

		if (opt.SMsgLogToDB && opt.SMsgLogToDB_WinOpen && opt.SMsgLogToDB_Remove)
			RemoveLoggedEventsSMsg(mwed->hContact);
	}
	else if (mwed->uType == MSG_WINDOW_EVT_OPEN) {
		if (opt.XLogToDB && (templates.LogXFlags & NOTIFY_OPENING_ML) && db_get_b(mwed->hContact, MODULE, "EnableXLogging", 1))
			mir_forkthread(AddXStatusEventThread, (void *)mwed->hContact);

		if (opt.SMsgLogToDB && (templates.LogSMsgFlags & NOTIFY_OPENING_ML) && db_get_b(mwed->hContact, MODULE, "EnableSMsgLogging", 1))
			mir_forkthread(AddSMsgEventThread, (void *)mwed->hContact);
	}
	return 0;
}
