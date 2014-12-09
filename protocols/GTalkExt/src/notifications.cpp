//***************************************************************************************
//
//   Google Extension plugin for the Miranda IM's Jabber protocol
//   Copyright (c) 2011 bems@jabber.org, George Hazan (ghazan@jabber.ru)
//
//   This program is free software; you can redistribute it and/or
//   modify it under the terms of the GNU General Public License
//   as published by the Free Software Foundation; either version 2
//   of the License, or (at your option) any later version.
//
//   This program is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
//   You should have received a copy of the GNU General Public License
//   along with this program; if not, write to the Free Software
//   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
//***************************************************************************************

#include "StdAfx.h"
#include "notifications.h"
#include "db.h"
#include "options.h"
#include "inbox.h"

#define PLUGIN_DATA_PROP_NAME       _T("{DB5CE833-C3AC-4851-831C-DDEBD9FA0508}")
#define EVT_DELETED_HOOK_PROP_NAME  _T("{87CBD2BC-8806-413C-8FD5-1D61ABCA4AF8}")

#define EVENT_DELETED_MSG  RegisterWindowMessage(_T("{B9B00536-86A0-4BCE-B2FE-4ABD409C22AE}"))
#define MESSAGE_CLOSEPOPUP RegisterWindowMessage(_T("{7A60EA87-3E77-41DF-8A69-59B147F0C9C6}"))

#define CLIST_MODULE_NAME "CList"
#define CONTACT_DISPLAY_NAME_SETTING "MyHandle"
#define STATUS_MSG_SETTING "StatusMsg"
#define UNREAD_THREADS_SETTING "UnreadThreads"

struct POPUP_DATA_HEADER
{
	BOOL   MarkRead;
	HANDLE hDbEvent;
	MCONTACT hContact;
	LPTSTR jid;
	LPTSTR url;
};

extern DWORD itlsSettings;

LPCSTR GetJidAcc(LPCTSTR jid)
{
	int count = 0;
	PROTOACCOUNT **protos;
	ProtoEnumAccounts(&count, &protos);
	for (int i=0; i < count; i++) {
		if ( getJabberApi(protos[i]->szModuleName)) {
			ptrT tszJid( db_get_tsa(0, protos[i]->szModuleName, "jid"));
			if ( !mir_tstrcmpi(jid, tszJid))
				return protos[i]->szModuleName;
		}
	}

	return NULL;
}

void MarkEventRead(MCONTACT hCnt, HANDLE hEvt)
{
	DWORD settings = (DWORD)TlsGetValue(itlsSettings);
	if ( ReadCheckbox(0, IDC_POPUPSENABLED, settings) &&
		 ReadCheckbox(0, IDC_PSEUDOCONTACTENABLED, settings) &&
		 ReadCheckbox(0, IDC_MARKEVENTREAD, settings) &&
		 db_event_markRead(hCnt, hEvt) != -1)
		CallService(MS_CLIST_REMOVEEVENT, (WPARAM)hCnt, (LPARAM)hEvt);
}

int OnEventDeleted(WPARAM hContact, LPARAM hDbEvent, LPARAM wnd)
{
	if (db_get_b((MCONTACT)hContact, SHORT_PLUGIN_NAME, PSEUDOCONTACT_FLAG, 0)) {
		CallService(MS_CLIST_REMOVEEVENT, hContact, hDbEvent);
		PostMessage((HWND)wnd, EVENT_DELETED_MSG, hContact, hDbEvent);
	}

	return 0;
}

LRESULT CALLBACK PopupProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	POPUP_DATA_HEADER *ppdh = (POPUP_DATA_HEADER*)PUGetPluginData(wnd);
	LPCSTR acc;

	if (EVENT_DELETED_MSG == msg) {
		if ((HANDLE)lParam == ppdh->hDbEvent)
			ppdh->hDbEvent = NULL;
		return 0;
	}

	if (MESSAGE_CLOSEPOPUP == msg) {
		ppdh->MarkRead = TRUE;
		PUDeletePopup(wnd);
	}

	switch (msg) {
	case UM_INITPOPUP:
		SetProp(wnd, PLUGIN_DATA_PROP_NAME, (HANDLE)ppdh);
		SetProp(wnd, EVT_DELETED_HOOK_PROP_NAME,
			HookEventParam(ME_DB_EVENT_DELETED, OnEventDeleted, (LPARAM)wnd));
		return 0;

	case UM_FREEPLUGINDATA:
		{
			HANDLE hHook = GetProp(wnd, EVT_DELETED_HOOK_PROP_NAME);
			RemoveProp(wnd, EVT_DELETED_HOOK_PROP_NAME);
			UnhookEvent(hHook);
		}

		if (ppdh->MarkRead && ppdh->hDbEvent && (acc = GetJidAcc(ppdh->jid))) {
			ReadNotificationSettings(acc);
			MarkEventRead(ppdh->hContact, ppdh->hDbEvent);
			CallService(MS_CLIST_REMOVEEVENT, (WPARAM)ppdh->hContact, (LPARAM)ppdh->hDbEvent);
		}
		RemoveProp(wnd, PLUGIN_DATA_PROP_NAME);
		free(ppdh);
		return 0;

	case WM_LBUTTONUP:
		acc = NULL;
		__try {
			if (!(acc = GetJidAcc(ppdh->jid))) return 0;

			ReadNotificationSettings(acc);
			OpenUrl(acc, ppdh->jid, ppdh->url);
		}
		__finally {
			CloseNotifications(acc, ppdh->url, ppdh->jid, TRUE);
		}
		return 0;

	case WM_RBUTTONUP:
		SendMessage(wnd, MESSAGE_CLOSEPOPUP, 0, 0);
		return 0;
	}
	return DefWindowProc(wnd, msg, wParam, lParam);
}

static bool DoAddPopup(POPUPDATAT *data)
{
	if ( ReadCheckbox(0, IDC_POPUPSINFULLSCREEN, (DWORD)TlsGetValue(itlsSettings)) && IsFullScreen()) {
		HWND handle = CreateWindowEx(WS_EX_TOOLWINDOW, TEMP_WINDOW_CLASS_NAME, NULL, WS_OVERLAPPED | WS_VISIBLE, -100, -100, 10, 10, 0, 0, 0, 0);
		if (handle) {
			ShowWindow(handle, SW_MINIMIZE);
			ShowWindow(handle, SW_RESTORE);
		}
		DestroyWindow(handle);
	}

	return PUAddPopupT(data) == 0;
}

void FormatPseudocontactDisplayName(LPTSTR buff, LPCTSTR jid, LPCTSTR unreadCount)
{
	if (mir_tstrcmp(unreadCount,  _T("0")))
		wsprintf(buff, _T("%s [%s]"), jid, unreadCount); //!!!!!!!!!!!
	else
		wsprintf(buff, _T("%s"), jid); //!!!!!!!!!!!
}

MCONTACT SetupPseudocontact(LPCTSTR jid, LPCTSTR unreadCount, LPCSTR acc, LPCTSTR displayName)
{
	MCONTACT hContact = db_get_dw(NULL, acc, PSEUDOCONTACT_LINK, 0);
	if (!hContact || !db_get_b(hContact, SHORT_PLUGIN_NAME, PSEUDOCONTACT_FLAG, 0)) {
		hContact = (MCONTACT)CallService(MS_DB_CONTACT_ADD, 0, 0);
		db_set_dw(0, acc, PSEUDOCONTACT_LINK, hContact);
		db_set_b(hContact, SHORT_PLUGIN_NAME, PSEUDOCONTACT_FLAG, 1);
		CallService(MS_PROTO_ADDTOCONTACT, hContact, (LPARAM)acc);
	}

	// SetAvatar(hContact);

	if (displayName == NULL) {
		TCHAR *tszTemp = (TCHAR*)alloca((mir_tstrlen(jid) + mir_tstrlen(unreadCount) + 3 + 1) * sizeof(TCHAR));
		FormatPseudocontactDisplayName(tszTemp, jid, unreadCount);
		db_set_ts(hContact, CLIST_MODULE_NAME, CONTACT_DISPLAY_NAME_SETTING, tszTemp);
	}
	else db_set_ts(hContact, CLIST_MODULE_NAME, CONTACT_DISPLAY_NAME_SETTING, displayName);

	db_set_ts(hContact, CLIST_MODULE_NAME, STATUS_MSG_SETTING, TranslateTS(MAIL_NOTIFICATIONS));
	db_set_ts(hContact, SHORT_PLUGIN_NAME, UNREAD_THREADS_SETTING, unreadCount);
	return hContact;
}

HANDLE AddCListNotification(MCONTACT hContact, LPCSTR acc, POPUPDATAT *data, LPCTSTR jid, LPCTSTR url, LPCTSTR unreadCount)
{
	mir_ptr<char> szUrl( mir_utf8encodeT(url)), szText( mir_utf8encodeT(data->lptzText));

	DBEVENTINFO dbei = { sizeof(dbei) };
	dbei.szModule = (LPSTR)acc;
	dbei.timestamp = time(NULL);
	dbei.flags = DBEF_UTF;
	dbei.eventType = EVENTTYPE_MESSAGE;

	char szEventText[4096];
	dbei.cbBlob = mir_snprintf(szEventText, SIZEOF(szEventText), "%s\r\n%s", szUrl, szText);
	dbei.pBlob = (PBYTE)szEventText;
	return db_event_add(hContact, &dbei);
}

BOOL UsePopups()
{
	return ServiceExists(MS_POPUP_QUERY) &&
		CallService(MS_POPUP_QUERY, PUQS_GETSTATUS, 0) &&
		ReadCheckbox(0, IDC_POPUPSENABLED, (DWORD)TlsGetValue(itlsSettings));
}

void ShowNotification(LPCSTR acc, POPUPDATAT *data, LPCTSTR jid, LPCTSTR url, LPCTSTR unreadCount)
{
	MCONTACT hCnt = SetupPseudocontact(jid, unreadCount, acc, &data->lptzContactName[0]);
	HANDLE hEvt = ReadCheckbox(0, IDC_PSEUDOCONTACTENABLED, (DWORD)TlsGetValue(itlsSettings))
		? AddCListNotification(hCnt, acc, data, jid, url, unreadCount) : NULL;

	if (!UsePopups())
		return;

	extern HICON g_hPopupIcon;
	data->lchIcon = g_hPopupIcon;
	data->iSeconds = (int)db_get_dw(0, SHORT_PLUGIN_NAME, TIMEOUT_SETTING, 0);
	data->colorBack = (COLORREF)db_get_dw(0, SHORT_PLUGIN_NAME, BACK_COLOR_SETTING, 0);
	data->colorText = (COLORREF)db_get_dw(0, SHORT_PLUGIN_NAME, TEXT_COLOR_SETTING, 0);
	if (data->colorBack == data->colorText) {
		data->colorBack = 0;
		data->colorText = 0;
	}

	data->PluginWindowProc = PopupProc;
	size_t lurl = (mir_tstrlen(url) + 1) * sizeof(TCHAR);
	size_t ljid = (mir_tstrlen(jid) + 1) * sizeof(TCHAR);

	POPUP_DATA_HEADER *ppdh = (POPUP_DATA_HEADER*)malloc(sizeof(POPUP_DATA_HEADER) + lurl + ljid);
	ppdh->MarkRead = FALSE;
	ppdh->hContact = hCnt;
	ppdh->hDbEvent = hEvt;
	ppdh->jid = (LPTSTR)((PBYTE)ppdh + sizeof(*ppdh));
	memcpy(ppdh->jid, jid, ljid);

	ppdh->url = (LPTSTR)((PBYTE)ppdh->jid + ljid);
	memcpy(ppdh->url, url, lurl);
	data->PluginData = ppdh;

	if ( !DoAddPopup(data))
		free(data->PluginData);
}

void UnreadMailNotification(LPCSTR acc, LPCTSTR jid, LPCTSTR url, LPCTSTR unreadCount)
{
	POPUPDATAT data = {0};

	FormatPseudocontactDisplayName(&data.lptzContactName[0], jid, unreadCount);
	mir_sntprintf(data.lptzText, SIZEOF(data.lptzText), TranslateT("You've received an e-mail\n%s unread threads"), unreadCount);

	ShowNotification(acc, &data, jid, url, unreadCount);
}

void UnreadThreadNotification(LPCSTR acc, LPCTSTR jid, LPCTSTR url, LPCTSTR unreadCount, const MAIL_THREAD_NOTIFICATION *mtn)
{
	POPUPDATAT data = {0};

	FormatPseudocontactDisplayName(&data.lptzContactName[0], jid, unreadCount);
	LPTSTR senders = (LPTSTR)malloc(SENDER_COUNT * 100 * sizeof(TCHAR));
	LPTSTR currSender = senders;

	for (int i = 0; i < SENDER_COUNT && mtn->senders[i].addr; i++) {
		if (mtn->senders[i].name)
			mir_sntprintf(currSender, SENDER_COUNT * 100, _T("    %s <%s>\n"), mtn->senders[i].name, mtn->senders[i].addr);
		else
			mir_sntprintf(currSender, SENDER_COUNT * 100, _T("    %s\n"), mtn->senders[i].addr);
		currSender += mir_tstrlen(currSender);
	}

	if ( ReadCheckbox(0, IDC_ADDSNIP, (DWORD)TlsGetValue(itlsSettings)))
		mir_sntprintf(data.lptzText, SIZEOF(data.lptzText), TranslateTS(FULL_NOTIFICATION_FORMAT), mtn->subj, senders, mtn->snip);
	else
		mir_sntprintf(data.lptzText, SIZEOF(data.lptzText), TranslateTS(SHORT_NOTIFICATION_FORMAT), mtn->subj, senders);

	free(senders);

	ShowNotification(acc, &data, jid, url, unreadCount);
}

void ClearNotificationContactHistory(LPCSTR acc)
{
	MCONTACT hContact = db_get_dw(NULL, acc, PSEUDOCONTACT_LINK, 0);
	if (!hContact || !db_get_b(hContact, SHORT_PLUGIN_NAME, PSEUDOCONTACT_FLAG, 0))
		return;

	for (HANDLE hEvent = db_event_first(hContact); hEvent;) {
		HANDLE hEvent1 = db_event_next(hContact, hEvent);
		db_event_delete(hContact, hEvent);
		hEvent = hEvent1;
	}
}

DWORD ReadNotificationSettings(LPCSTR acc)
{
	DWORD result = ReadCheckboxes(0, acc);
	TlsSetValue(itlsSettings, (PVOID)result);
	return result;
}

struct POPUP_IDENT_STRINGS {
	LPCTSTR url;
	LPCTSTR jid;
};

BOOL CALLBACK ClosePopupFunc(__in  HWND hwnd, __in LPARAM lParam)
{
	DWORD pid = 0;
	GetWindowThreadProcessId(hwnd, &pid);
	if (pid != GetCurrentProcessId())
		return TRUE;

	POPUP_IDENT_STRINGS *ppis = (POPUP_IDENT_STRINGS*)lParam;
	POPUP_DATA_HEADER *ppdh = (POPUP_DATA_HEADER*)GetProp(hwnd, PLUGIN_DATA_PROP_NAME);
	if (!ppdh)
		return TRUE;

	if (!mir_tstrcmpi(ppis->url, ppdh->url) && !mir_tstrcmpi(ppis->jid, ppdh->jid))
		SendMessage(hwnd, MESSAGE_CLOSEPOPUP, 0, 0);

	return TRUE;
}

void CloseNotifications(LPCSTR acc, LPCTSTR url, LPCTSTR jid, BOOL PopupsOnly)
{
	DWORD settings = (DWORD)TlsGetValue(itlsSettings);
	if (acc && !PopupsOnly &&
		 ReadCheckbox(0, IDC_PSEUDOCONTACTENABLED, settings) &&
		 ReadCheckbox(0, IDC_CLEARPSEUDOCONTACTLOG, settings))
		ClearNotificationContactHistory(acc);

	POPUP_IDENT_STRINGS pis = {url, jid};
	EnumWindows(ClosePopupFunc, (LPARAM)&pis);
}
