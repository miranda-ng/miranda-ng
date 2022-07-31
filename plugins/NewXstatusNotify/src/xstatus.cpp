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

#include "stdafx.h"

XSTATUSCHANGE *NewXSC(MCONTACT hContact, char *szProto, int xstatusType, int action, wchar_t *stzTitle, wchar_t *stzText)
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
	for (auto &it : eventListXStatus.rev_iter())
		if (it->hContact == hContact) {
			db_event_delete(it->hDBEvent);
			mir_free(eventListXStatus.removeItem(&it));
		}
}

void RemoveLoggedEventsStatus(MCONTACT hContact)
{
	for (auto &it : eventListStatus.rev_iter())
		if (it->hContact == hContact) {
			db_event_delete(it->hDBEvent);
			mir_free(eventListStatus.removeItem(&it));
		}
}

void RemoveLoggedEventsSMsg(MCONTACT hContact)
{
	for (auto &it : eventListSMsg.rev_iter())
		if (it->hContact == hContact) {
			db_event_delete(it->hDBEvent);
			mir_free(eventListSMsg.removeItem(&it));
		}
}

wchar_t* GetStatusTypeAsString(int type, wchar_t *buff)
{
	switch (type) {
	case TYPE_JABBER_MOOD:
		mir_wstrcpy(buff, TranslateT("Mood")); return buff;
	case TYPE_JABBER_ACTIVITY:
		mir_wstrcpy(buff, TranslateT("Activity")); return buff;
	case TYPE_ICQ_XSTATUS:
		mir_wstrcpy(buff, TranslateT("xStatus")); return buff;
	default:
		mir_wstrcpy(buff, TranslateT("<unknown>")); return buff;
	}
}

CMStringW ReplaceVars(XSTATUSCHANGE *xsc, const wchar_t *tmplt)
{
	if (xsc == nullptr || tmplt == nullptr || tmplt[0] == '\0')
		return CMStringW();

	size_t len = mir_wstrlen(tmplt);
	CMStringW res;

	for (size_t i = 0; i < len; i++) {
		if (tmplt[i] == '%') {
			i++;
			switch (tmplt[i]) {
			case 'n':
				wchar_t stzType[32];
				res.Append(GetStatusTypeAsString(xsc->type, stzType));
				break;

			case 't':
				if (xsc->stzTitle == nullptr || xsc->stzTitle[0] == '\0')
					res.Append(TranslateT("<no title>"));
				else
					res.Append(xsc->stzTitle);
				break;

			case 'm':
				if (xsc->stzText == nullptr || xsc->stzText[0] == '\0')
					res.Append(TranslateT("<no status message>"));
				else
					AddCR(res, xsc->stzText);
				break;

			case 'c':
				if (xsc->hContact == NULL)
					res.Append(TranslateT("Contact"));
				else
					res.Append(Clist_GetContactDisplayName(xsc->hContact));
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
				res.AppendChar('\r'); res.AppendChar('\n');
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

	return res;
}

void ShowXStatusPopup(XSTATUSCHANGE *xsc)
{
	if (xsc == nullptr)
		return;

	HICON hIcon = nullptr;

	switch (xsc->type) {
	case TYPE_JABBER_MOOD:
	case TYPE_JABBER_ACTIVITY:
		DBVARIANT dbv;
		char szSetting[64];
		mir_snprintf(szSetting, "%s/%s/icon", xsc->szProto, (xsc->type == TYPE_JABBER_MOOD) ? "mood" : "activity");
		if (!db_get_s(xsc->hContact, "AdvStatus", szSetting, &dbv)) {
			hIcon = IcoLib_GetIcon(dbv.pszVal);
			db_free(&dbv);
		}
		break;

	case TYPE_ICQ_XSTATUS:
		int statusId = db_get_b(xsc->hContact, xsc->szProto, "XStatusId", 0);
		hIcon = (HICON)CallProtoService(xsc->szProto, PS_GETCUSTOMSTATUSICON, statusId, LR_SHARED);
	}

	if (hIcon == nullptr)
		hIcon = Skin_LoadProtoIcon(xsc->szProto, db_get_w(xsc->hContact, xsc->szProto, "Status", ID_STATUS_ONLINE));

	// cut message if needed
	wchar_t *copyText = nullptr;
	if (opt.PXMsgTruncate && (opt.PXMsgLen > 0) && xsc->stzText && (mir_wstrlen(xsc->stzText) > opt.PXMsgLen)) {
		wchar_t buff[MAX_TEXT_LEN + 3];
		copyText = mir_wstrdup(xsc->stzText);
		wcsncpy(buff, xsc->stzText, opt.PXMsgLen);
		buff[opt.PXMsgLen] = 0;
		mir_wstrcat(buff, L"...");
		replaceStrW(xsc->stzText, buff);
	}

	wchar_t *Template = L"";
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

	ShowChangePopup(xsc->hContact, hIcon, ID_STATUS_EXTRASTATUS, ReplaceVars(xsc, Template));

	if (copyText) {
		mir_free(xsc->stzText);
		xsc->stzText = copyText;
	}
}

void BlinkXStatusIcon(XSTATUSCHANGE *xsc)
{
	if (xsc == nullptr)
		return;

	HICON hIcon = nullptr;
	wchar_t str[256] = { 0 };
	wchar_t stzType[32];
	mir_snwprintf(str, TranslateT("%s changed %s"), Clist_GetContactDisplayName(xsc->hContact), GetStatusTypeAsString(xsc->type, stzType));

	if (opt.BlinkIcon_Status) {
		DBVARIANT dbv;
		char szSetting[64];

		switch (xsc->type) {
		case TYPE_JABBER_MOOD:
		case TYPE_JABBER_ACTIVITY:
			mir_snprintf(szSetting, "%s/%s/icon", xsc->szProto, (xsc->type == TYPE_JABBER_MOOD) ? "mood" : "activity");
			if (!db_get_s(xsc->hContact, "AdvStatus", szSetting, &dbv)) {
				hIcon = IcoLib_GetIcon(dbv.pszVal);
				db_free(&dbv);
			}
			break;
		case TYPE_ICQ_XSTATUS:
			int statusId = db_get_b(xsc->hContact, xsc->szProto, "XStatusId", 0);
			hIcon = (HICON)CallProtoService(xsc->szProto, PS_GETCUSTOMSTATUSICON, statusId, LR_SHARED);
		}
	}

	if (hIcon == nullptr)
		hIcon = Skin_LoadIcon(SKINICON_OTHER_USERONLINE);

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
	if (xsc == nullptr || (opt.XLogToDB_WinOpen && !CheckMsgWnd(xsc->hContact)))
		return;

	wchar_t *Template = L"";
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

	wchar_t stzLastLog[2 * MAX_TEXT_LEN];
	CMStringW stzLogText(ReplaceVars(xsc, Template));
	DBGetStringDefault(xsc->hContact, DB_LASTLOG, stzLastLog, _countof(stzLastLog), L"");

	if (opt.XLogToDB) {
		g_plugin.setWString(xsc->hContact, DB_LASTLOG, stzLogText);

		T2Utf blob(stzLogText);

		DBEVENTINFO dbei = {};
		dbei.cbBlob = (uint32_t)mir_strlen(blob) + 1;
		dbei.pBlob = (uint8_t*)(char*)blob;
		dbei.eventType = EVENTTYPE_STATUSCHANGE;
		dbei.flags = DBEF_READ | DBEF_UTF;
		dbei.timestamp = (uint32_t)time(0);
		dbei.szModule = MODULENAME;

		MEVENT hDBEvent = db_event_add(xsc->hContact, &dbei);
		if (opt.XLogToDB_WinOpen && opt.XLogToDB_Remove) {
			DBEVENT *dbevent = (DBEVENT *)mir_alloc(sizeof(DBEVENT));
			dbevent->hContact = xsc->hContact;
			dbevent->hDBEvent = hDBEvent;
			eventListXStatus.insert(dbevent);
		}
	}
}

void LogChangeToFile(XSTATUSCHANGE *xsc)
{
	if (xsc == nullptr)
		return;

	wchar_t stzDate[32], stzTime[32], stzText[MAX_TEXT_LEN];

	GetTimeFormat(LOCALE_USER_DEFAULT, 0, nullptr, L"HH':'mm", stzTime, _countof(stzTime));
	GetDateFormat(LOCALE_USER_DEFAULT, 0, nullptr, L"dd/MM/yyyy", stzDate, _countof(stzDate));

	wchar_t *Template = L"";
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

	mir_snwprintf(stzText, L"%s, %s. %s %s\r\n", stzDate, stzTime, 
		Clist_GetContactDisplayName(xsc->hContact), ReplaceVars(xsc, Template).GetString());

	LogToFile(stzText);
}

void ExtraStatusChanged(XSTATUSCHANGE *xsc)
{
	if (xsc == nullptr)
		return;

	BOOL bEnablePopup = true, bEnableSound = true, bEnableLog = opt.XLogToDB;

	char buff[12] = { 0 };
	mir_snprintf(buff, "%d", ID_STATUS_EXTRASTATUS);
	if ((g_plugin.getByte(buff, 1) == 0)
		|| (db_get_w(xsc->hContact, xsc->szProto, "Status", ID_STATUS_OFFLINE) == ID_STATUS_OFFLINE)
		|| (!opt.HiddenContactsToo && Contact::IsHidden(xsc->hContact))
		|| (Proto_GetStatus(xsc->szProto) == ID_STATUS_OFFLINE))
	{
		FreeXSC(xsc);
		return;
	}

	// check per-contact ignored events
	if (g_plugin.getByte(xsc->hContact, "EnableXStatusNotify", 1) == 0)
		bEnableSound = bEnablePopup = false;

	// check if our status isn't on autodisable list
	if (opt.AutoDisable) {
		char statusIDs[12], statusIDp[12];
		int myStatus = Proto_GetStatus(xsc->szProto);
		mir_snprintf(statusIDs, "s%d", myStatus);
		mir_snprintf(statusIDp, "p%d", myStatus);
		bEnableSound = g_plugin.getByte(statusIDs, 1) ? FALSE : bEnableSound;
		bEnablePopup = g_plugin.getByte(statusIDp, 1) ? FALSE : bEnablePopup;
	}

	if (!(templates.PopupXFlags & xsc->action))
		bEnablePopup = false;

	if (g_plugin.getByte(xsc->szProto, 1) == 0 && !opt.PXOnConnect)
		bEnablePopup = false;

	int xstatusID = db_get_b(xsc->hContact, xsc->szProto, "XStatusId", 0);
	if (opt.PXDisableForMusic && xsc->type == TYPE_ICQ_XSTATUS && xstatusID == XSTATUS_MUSIC)
		bEnableSound = bEnablePopup = false;

	if (bEnablePopup && g_plugin.getByte(xsc->hContact, "EnablePopups", 1) && g_plugin.bPopups)
		ShowXStatusPopup(xsc);

	if (bEnableSound && db_get_b(0, "Skin", "UseSound", 1) && g_plugin.getByte(xsc->hContact, "EnableSounds", 1) && g_plugin.bPopups)
		PlayXStatusSound(xsc->hContact, xsc->action);

	if (opt.BlinkIcon && opt.BlinkIcon_ForMsgs && g_plugin.bPopups)
		BlinkXStatusIcon(xsc);

	if (opt.XLogDisableForMusic && xsc->type == TYPE_ICQ_XSTATUS && xstatusID == XSTATUS_MUSIC)
		bEnableLog = false;

	if (!(templates.LogXFlags & xsc->action))
		bEnableLog = false;

	if (bEnableLog && g_plugin.getByte(xsc->hContact, "EnableXLogging", 1))
		LogChangeToDB(xsc);

	if (opt.XLogToFile && g_plugin.getByte(xsc->hContact, "EnableXLogging", 1))
		LogChangeToFile(xsc);

	FreeXSC(xsc);
}

wchar_t* GetDefaultXstatusName(int statusID, char *szProto, wchar_t *buff, int bufflen)
{
	wchar_t nameBuff[64];
	buff[0] = 0;

	CUSTOM_STATUS xstatus = { 0 };
	xstatus.cbSize = sizeof(CUSTOM_STATUS);
	xstatus.flags = CSSF_MASK_NAME | CSSF_DEFAULT_NAME | CSSF_UNICODE;
	xstatus.ptszName = nameBuff;
	xstatus.wParam = (WPARAM*)&statusID;
	if (!CallProtoService(szProto, PS_GETCUSTOMSTATUSEX, 0, (LPARAM)&xstatus))
		wcsncpy_s(buff, bufflen, TranslateW(nameBuff), _TRUNCATE);

	return buff;
}

wchar_t* GetIcqXStatus(MCONTACT hContact, char *szProto, char *szValue, wchar_t *buff, int bufflen)
{
	DBVARIANT dbv;
	buff[0] = 0;

	int statusID = db_get_b(hContact, szProto, "XStatusId", -1);
	if (statusID != -1) {
		if (!db_get_ws(hContact, szProto, szValue, &dbv)) {
			if ((mir_strcmp(szValue, "XStatusName") == 0) && dbv.pwszVal[0] == 0)
				GetDefaultXstatusName(statusID, szProto, buff, bufflen);
			else
				wcsncpy(buff, dbv.pwszVal, bufflen);

			buff[bufflen - 1] = 0;
			db_free(&dbv);
		}
	}

	return buff;
}

wchar_t* GetJabberAdvStatusText(MCONTACT hContact, char *szProto, char *szSlot, char *szValue, wchar_t *buff, int bufflen)
{
	DBVARIANT dbv;
	char szSetting[128];
	buff[0] = 0;

	mir_snprintf(szSetting, "%s/%s/%s", szProto, szSlot, szValue);
	if (!db_get_ws(hContact, "AdvStatus", szSetting, &dbv)) {
		wcsncpy(buff, dbv.pwszVal, bufflen);
		buff[bufflen - 1] = 0;
		db_free(&dbv);
	}

	return buff;
}

void LogXstatusChange(MCONTACT hContact, char *szProto, int xstatusType, wchar_t *stzTitle, wchar_t *stzText)
{
	XSTATUSCHANGE *xsc = NewXSC(hContact, szProto, xstatusType, NOTIFY_OPENING_ML, stzTitle[0] ? mir_wstrdup(stzTitle) : nullptr, stzText[0] ? mir_wstrdup(stzText) : nullptr);

	LogChangeToDB(xsc);
	FreeXSC(xsc);
}

void AddXStatusEventThread(void *arg)
{
	MCONTACT hContact = (MCONTACT)(DWORD_PTR)arg;

	char *szProto = Proto_GetBaseAccountName(hContact);
	if (szProto == nullptr)
		return;

	wchar_t stzTitle[MAX_TITLE_LEN], stzText[MAX_TEXT_LEN];
	if (ProtoServiceExists(szProto, JS_PARSE_XMPP_URI)) {
		GetJabberAdvStatusText(hContact, szProto, "mood", "title", stzTitle, _countof(stzTitle));
		if (stzTitle[0]) {
			GetJabberAdvStatusText(hContact, szProto, "mood", "text", stzText, _countof(stzText));
			LogXstatusChange(hContact, szProto, TYPE_JABBER_MOOD, stzTitle, stzText);
		}

		GetJabberAdvStatusText(hContact, szProto, "activity", "title", stzTitle, _countof(stzTitle));
		if (stzTitle[0]) {
			GetJabberAdvStatusText(hContact, szProto, "activity", "text", stzText, _countof(stzText));
			LogXstatusChange(hContact, szProto, TYPE_JABBER_ACTIVITY, stzTitle, stzText);
		}
	}
	else {
		GetIcqXStatus(hContact, szProto, "XStatusName", stzTitle, _countof(stzTitle));
		if (stzTitle[0]) {
			GetIcqXStatus(hContact, szProto, "XStatusMsg", stzText, _countof(stzText));
			LogXstatusChange(hContact, szProto, TYPE_ICQ_XSTATUS, stzTitle, stzText);
		}
	}
}

void AddSMsgEventThread(void *arg)
{
	MCONTACT hContact = (MCONTACT)(DWORD_PTR)arg;

	STATUSMSGINFO smi;
	smi.hContact = hContact;
	smi.proto = Proto_GetBaseAccountName(hContact);
	if (smi.proto == nullptr)
		return;

	smi.newstatusmsg = db_get_wsa(smi.hContact, "CList", "StatusMsg");
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
		if (opt.XLogToDB && (templates.LogXFlags & NOTIFY_OPENING_ML) && g_plugin.getByte(mwed->hContact, "EnableXLogging", 1))
			mir_forkthread(AddXStatusEventThread, (void *)mwed->hContact);

		if (opt.SMsgLogToDB && (templates.LogSMsgFlags & NOTIFY_OPENING_ML) && g_plugin.getByte(mwed->hContact, "EnableSMsgLogging", 1))
			mir_forkthread(AddSMsgEventThread, (void *)mwed->hContact);
	}
	return 0;
}
