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

const LPTSTR _T(NUMBER_EMAILS_MESSAGE) = LPGENT("You've received an e-mail\n%s unread threads");

const LPTSTR PLUGIN_DATA_PROP_NAME = _T("{DB5CE833-C3AC-4851-831C-DDEBD9FA0508}");

#define MESSAGE_CLOSEPOPUP RegisterWindowMessage(_T("{7A60EA87-3E77-41DF-8A69-59B147F0C9C6}"))

const LPSTR CLIST_MODULE_NAME = "CList";
const LPSTR CONTACT_DISPLAY_NAME_SETTING = "MyHandle";
const LPSTR STATUS_MSG_SETTING = "StatusMsg";
const LPSTR UNREAD_THREADS_SETTING = "UnreadThreads";

struct POPUP_DATA_HEADER
{
	LPTSTR jid;
	LPTSTR url;
};

extern DWORD itlsSettings;

LPCSTR GetJidAcc(LPCTSTR jid)
{
	int count = 0;
	PROTOACCOUNT **protos;
	ProtoEnumAccounts(&count, &protos);

	DBVARIANT dbv;
	for (int i=0; i < count; i++)
		if (getJabberApi(protos[i]->szModuleName))
			if (!db_get_ts(0, protos[i]->szModuleName, "jid", &dbv))
				__try {
					if (!lstrcmpi(jid, dbv.ptszVal)) return protos[i]->szModuleName;
				}
				__finally {
					db_free(&dbv);
				}

	return NULL;
}

LRESULT CALLBACK PopupProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	POPUP_DATA_HEADER *ppdh = (POPUP_DATA_HEADER*)PUGetPluginData(wnd);
	LPCSTR acc;

	if (MESSAGE_CLOSEPOPUP == msg)
		PUDeletePopup(wnd);

	switch (msg) {
	case UM_INITPOPUP:
		SetProp(wnd, PLUGIN_DATA_PROP_NAME, (HANDLE)ppdh);
		return 0;

	case UM_FREEPLUGINDATA:
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
	bool result = false;
	HWND handle = 0;

	if (ReadCheckbox(0, IDC_POPUPSINFULLSCREEN, (DWORD)TlsGetValue(itlsSettings))) {
		handle = CreateWindowEx(WS_EX_TOOLWINDOW, TEMP_WINDOW_CLASS_NAME, NULL, WS_OVERLAPPED | WS_VISIBLE,
				-100, -100, 10, 10, NULL, NULL, NULL, NULL);
		if (handle) {
			ShowWindow(handle, SW_MINIMIZE);
			ShowWindow(handle, SW_RESTORE);
		}
	}
	result = PUAddPopupT(data) == 0;

	if (handle) DestroyWindow(handle);

	return result;
}

void FormatPseudocontactDisplayName(LPTSTR buff, LPCTSTR jid, LPCTSTR unreadCount)
{
	if (lstrcmp(unreadCount,  _T("0")))
		wsprintf(buff, _T("%s [%s]"), jid, unreadCount); //!!!!!!!!!!!
	else
		wsprintf(buff, _T("%s"), jid); //!!!!!!!!!!!
}

BOOL UsePopups()
{
	return ServiceExists(MS_POPUP_QUERY) &&
		CallService(MS_POPUP_QUERY, PUQS_GETSTATUS, 0) &&
		ReadCheckbox(0, IDC_POPUPSENABLED, (DWORD)TlsGetValue(itlsSettings));
}

void ShowNotification(LPCSTR acc, POPUPDATAT *data, LPCTSTR jid, LPCTSTR url, LPCTSTR unreadCount)
{
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
	int lurl = (lstrlen(url) + 1) * sizeof(TCHAR);
	int ljid = (lstrlen(jid) + 1) * sizeof(TCHAR);
	
	POPUP_DATA_HEADER *ppdh = (POPUP_DATA_HEADER*)malloc(sizeof(POPUP_DATA_HEADER) + lurl + ljid);
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
	mir_sntprintf(&data.lptzText[0], SIZEOF(data.lptzText), TranslateT(NUMBER_EMAILS_MESSAGE), unreadCount);

	ShowNotification(acc, &data, jid, url, unreadCount);
}

void UnreadThreadNotification(LPCSTR acc, LPCTSTR jid, LPCTSTR url, LPCTSTR unreadCount, const MAIL_THREAD_NOTIFICATION *mtn)
{
	POPUPDATAT data = {0};

	FormatPseudocontactDisplayName(&data.lptzContactName[0], jid, unreadCount);
	LPTSTR senders = (LPTSTR)malloc(SENDER_COUNT * 100 * sizeof(TCHAR));
	LPTSTR currSender = senders;

	for (int i = 0; i < SENDER_COUNT && mtn->senders[i].addr; i++) {
		mir_sntprintf(currSender, SENDER_COUNT * 100, _T("    %s <%s>\n"), mtn->senders[i].name, mtn->senders[i].addr);
		currSender += lstrlen(currSender);
	}

	if (ReadCheckbox(0, IDC_ADDSNIP, (DWORD)TlsGetValue(itlsSettings)))
		mir_sntprintf(&data.lptzText[0], SIZEOF(data.lptzText), TranslateTS(FULL_NOTIFICATION_FORMAT), mtn->subj, senders, mtn->snip);
	else
		mir_sntprintf(&data.lptzText[0], SIZEOF(data.lptzText), TranslateTS(SHORT_NOTIFICATION_FORMAT), mtn->subj, senders);

	free(senders);

	ShowNotification(acc, &data, jid, url, unreadCount);
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

	if (!lstrcmpi(ppis->url, ppdh->url) && !lstrcmpi(ppis->jid, ppdh->jid))
		SendMessage(hwnd, MESSAGE_CLOSEPOPUP, 0, 0);

	return TRUE;
}

void CloseNotifications(LPCSTR acc, LPCTSTR url, LPCTSTR jid, BOOL PopupsOnly)
{
	POPUP_IDENT_STRINGS pis = {url, jid};
	EnumWindows(ClosePopupFunc, (LPARAM)&pis);
}
