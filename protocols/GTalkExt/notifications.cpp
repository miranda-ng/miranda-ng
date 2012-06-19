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
#include "avatar.h"
#include "inbox.h"

static const LPTSTR TEMP_WINDOW_CLASS_NAME = _T("AntiShittyFullscreenDetectionWindowClass");
static const LPTSTR _T(NUMBER_EMAILS_MESSAGE) = _T("You've received an e-mail\n%s unread threads");

static const LPTSTR PLUGIN_DATA_PROP_NAME = _T("{DB5CE833-C3AC-4851-831C-DDEBD9FA0508}");
static const LPTSTR EVT_DELETED_HOOK_PROP_NAME = _T("{87CBD2BC-8806-413C-8FD5-1D61ABCA4AF8}");

#define EVENT_DELETED_MSG RegisterWindowMessage(_T("{B9B00536-86A0-4BCE-B2FE-4ABD409C22AE}"))
#define MESSAGE_CLOSEPOPUP RegisterWindowMessage(_T("{7A60EA87-3E77-41DF-8A69-59B147F0C9C6}"))

static const LPSTR CLIST_MODULE_NAME = "CList";
static const LPSTR CONTACT_DISPLAY_NAME_SETTING = "MyHandle";
static const LPSTR STATUS_MSG_SETTING = "StatusMsg";
static const LPSTR UNREAD_THREADS_SETTING = "UnreadThreads";

struct POPUP_DATA_HEADER {
	BOOL MarkRead;
	HANDLE hDbEvent;
	HANDLE hContact;
	LPTSTR jid;
	LPTSTR url;
};

extern DWORD itlsSettings;
BOOL isOriginalPopups = FALSE;

LRESULT CALLBACK WndProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
		case WM_NCCREATE:
			return 1;

		case WM_GETMINMAXINFO:
			PMINMAXINFO info = (PMINMAXINFO)lParam;
			info->ptMaxPosition.x = -100;
			info->ptMaxPosition.y = -100;
			info->ptMaxSize.x = 10;
			info->ptMaxSize.y = 10;
			info->ptMaxTrackSize.x = 10;
			info->ptMaxTrackSize.y = 10;
			info->ptMinTrackSize.x = 10;
			info->ptMinTrackSize.y = 10;
			return 0;
	}
	return DefWindowProc(wnd, msg, wParam, lParam);
}

LPCSTR GetJidAcc(LPCTSTR jid)
{
	int count = 0;
	PROTOCOLDESCRIPTOR **protos;
	CallService(MS_PROTO_ENUMPROTOCOLS, (WPARAM)&count, (LPARAM)&protos);

	DBVARIANT dbv;
	for (int i = 0; i < count; i++)
		if (getJabberApi(protos[i]->szName))
			if (!DBGetContactSettingTString(0, protos[i]->szName, "jid", &dbv))
				__try {
					if (!lstrcmpi(jid, dbv.ptszVal)) return protos[i]->szName;
				}
				__finally {
					DBFreeVariant(&dbv);
				}

	return NULL;
}

void MarkEventRead(HANDLE hCnt, HANDLE hEvt)
{
	DWORD settings = (DWORD)TlsGetValue(itlsSettings);
	if (ReadCheckbox(0, IDC_POPUPSENABLED, settings) &&
		ReadCheckbox(0, IDC_PSEUDOCONTACTENABLED, settings) &&
		ReadCheckbox(0, IDC_MARKEVENTREAD, settings) &&
		(CallService(MS_DB_EVENT_MARKREAD, (WPARAM)hCnt, (LPARAM)hEvt) != (INT_PTR)-1))
			CallService(MS_CLIST_REMOVEEVENT, (WPARAM)hCnt, (LPARAM)hEvt);

}

int OnEventDeleted(WPARAM hContact, LPARAM hDbEvent, LPARAM wnd)
{
	if (DBGetContactSettingByte((HANDLE)hContact, SHORT_PLUGIN_NAME, PSEUDOCONTACT_FLAG, 0)) {
		CallService(MS_CLIST_REMOVEEVENT, hContact, hDbEvent);
		PostMessage((HWND)wnd, EVENT_DELETED_MSG, hContact, hDbEvent);
	}

	return 0;
}

LRESULT CALLBACK PopupProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (EVENT_DELETED_MSG == msg) {
		POPUP_DATA_HEADER *ppdh = (POPUP_DATA_HEADER*)PUGetPluginData(wnd);
		if ((HANDLE)lParam == ppdh->hDbEvent) ppdh->hDbEvent = NULL;
		return 0;
	}
	else
		if (MESSAGE_CLOSEPOPUP == msg) {
			POPUP_DATA_HEADER *ppdh = (POPUP_DATA_HEADER*)PUGetPluginData(wnd);
			ppdh->MarkRead = TRUE;
			PUDeletePopUp(wnd);
		}

	switch (msg) {
		case UM_INITPOPUP: {
			POPUP_DATA_HEADER *ppdh = (POPUP_DATA_HEADER*)PUGetPluginData(wnd);
			SetProp(wnd, PLUGIN_DATA_PROP_NAME, (HANDLE)ppdh);
			SetProp(wnd, EVT_DELETED_HOOK_PROP_NAME,
				HookEventParam(ME_DB_EVENT_DELETED, OnEventDeleted, (LPARAM)wnd));
			return 0;
		}

		case UM_FREEPLUGINDATA: {
			HANDLE hHook = GetProp(wnd, EVT_DELETED_HOOK_PROP_NAME);
			RemoveProp(wnd, EVT_DELETED_HOOK_PROP_NAME);
			UnhookEvent(hHook);

			LPCSTR acc = NULL;
			POPUP_DATA_HEADER *ppdh = (POPUP_DATA_HEADER*)PUGetPluginData(wnd);
			__try {
				if (ppdh->MarkRead && ppdh->hDbEvent && (acc = GetJidAcc(ppdh->jid))) {
					ReadNotificationSettings(acc);
					MarkEventRead(ppdh->hContact, ppdh->hDbEvent);
					CallService(MS_CLIST_REMOVEEVENT, (WPARAM)ppdh->hContact, (LPARAM)ppdh->hDbEvent);
				}

			}
			__finally {
				RemoveProp(wnd, PLUGIN_DATA_PROP_NAME);
				free(ppdh);
			}

			return 0;
		}

		case WM_LBUTTONUP: {
			LPCSTR acc = NULL;
			POPUP_DATA_HEADER *ppdh = (POPUP_DATA_HEADER*)PUGetPluginData(wnd);
			__try {
				if (!(acc = GetJidAcc(ppdh->jid))) return 0;

				ReadNotificationSettings(acc);
				OpenUrl(acc, ppdh->jid, ppdh->url);
			}
			__finally {
				CloseNotifications(acc, ppdh->url, ppdh->jid, TRUE);
			}
			return 0;
		}

		case WM_RBUTTONUP:
			SendMessage(wnd, MESSAGE_CLOSEPOPUP, 0, 0);
			return 0;
	}
	return DefWindowProc(wnd, msg, wParam, lParam);
}

HWND DoAddPopup(POPUPDATAT *data)
{
	WNDCLASS cls = {0};
	cls.lpfnWndProc = WndProc;
	cls.lpszClassName = TEMP_WINDOW_CLASS_NAME;

	HWND result = 0;
	HWND handle = 0;
	__try {
		if (ReadCheckbox(0, IDC_POPUPSINFULLSCREEN, (DWORD)TlsGetValue(itlsSettings))) {
			RegisterClass(&cls);
			handle = CreateWindowEx(WS_EX_TOOLWINDOW, TEMP_WINDOW_CLASS_NAME, NULL, WS_OVERLAPPED | WS_VISIBLE,
				-100, -100, 10, 10, NULL, NULL, NULL, NULL);
			if (handle) {
				ShowWindow(handle, SW_MINIMIZE);
				ShowWindow(handle, SW_RESTORE);
			}
		}
		result = (HWND)CallService(MS_POPUP_ADDPOPUPT, (WPARAM) data, APF_RETURN_HWND);
	}
	__finally {
		if (handle) DestroyWindow(handle);
	}

	return result;
}

void FormatPseudocontactDisplayName(LPTSTR buff, LPCTSTR jid, LPCTSTR unreadCount)
{
	if (lstrcmp(unreadCount,  _T("0")))
		wsprintf(buff, _T("%s [%s]"), jid, unreadCount);
	else
		wsprintf(buff, _T("%s"), jid);
}

HANDLE SetupPseudocontact(LPCTSTR jid, LPCTSTR unreadCount, LPCSTR acc, LPCTSTR displayName)
{
	HANDLE result = (HANDLE)DBGetContactSettingDword(0, acc, PSEUDOCONTACT_LINK, 0);
	if (!result || !DBGetContactSettingByte(result, SHORT_PLUGIN_NAME, PSEUDOCONTACT_FLAG, 0)) {
		result = (HANDLE)CallService(MS_DB_CONTACT_ADD, 0, 0);
		DBWriteContactSettingDword(0, acc, PSEUDOCONTACT_LINK, (DWORD)result);
		DBWriteContactSettingByte(result, SHORT_PLUGIN_NAME, PSEUDOCONTACT_FLAG, 1);
		CallService(MS_PROTO_ADDTOCONTACT, (WPARAM)result, (LPARAM)acc);
	}

	SetAvatar(result);

	BOOL allocateName = !displayName;
	__try {
		if (allocateName) {
			displayName = (LPCTSTR)malloc((lstrlen(jid) + lstrlen(unreadCount) + 3 + 1) * sizeof(TCHAR));
			FormatPseudocontactDisplayName((LPTSTR)displayName, jid, unreadCount);
		}

		DBWriteContactSettingTString(result, CLIST_MODULE_NAME, CONTACT_DISPLAY_NAME_SETTING, displayName);
	}
	__finally {
		if (allocateName) free((PVOID)displayName);
	}

	DBWriteContactSettingTString(result, CLIST_MODULE_NAME, STATUS_MSG_SETTING, TranslateTS(MAIL_NOTIFICATIONS));
	DBWriteContactSettingTString(result, SHORT_PLUGIN_NAME, UNREAD_THREADS_SETTING, unreadCount);

	return result;
}

HANDLE AddCListNotification(HANDLE hContact, LPCSTR acc, POPUPDATAT *data, LPCTSTR jid, LPCTSTR url, LPCTSTR unreadCount)
{
	int lurl = (lstrlen(url) + 1) * sizeof(WCHAR);
	LPSTR utf8 = (LPSTR)malloc(sizeof(data->lptzText) + sizeof(WCHAR) * 2 + lurl);
	__try {
		DBEVENTINFO dbei = {0};

		dbei.cbBlob = WideCharToMultiByte(CP_UTF8, 0, &data->lptzText[0], -1, utf8, sizeof(data->lptzText), NULL, NULL) - 1;

		if (utf8[dbei.cbBlob - 1] != 10) {
			utf8[dbei.cbBlob++] = 13;
			utf8[dbei.cbBlob++] = 10;
		}

		dbei.cbBlob += WideCharToMultiByte(CP_UTF8, 0, url, -1, utf8 + dbei.cbBlob, lurl, NULL, NULL);

		dbei.pBlob = (PBYTE)utf8;
		dbei.cbSize = sizeof(dbei);
		dbei.szModule = (LPSTR)acc;
		dbei.timestamp = time(NULL);
		dbei.flags = DBEF_UTF;
		dbei.eventType = EVENTTYPE_MESSAGE;
		return (HANDLE)CallService(MS_DB_EVENT_ADD, (WPARAM)hContact, (LPARAM)&dbei);
	}
	__finally {
		free(utf8);
	}
}

BOOL UsePopups()
{
	return ServiceExists(MS_POPUP_QUERY) &&
		CallService(MS_POPUP_QUERY, PUQS_GETSTATUS, 0) &&
		ReadCheckbox(0, IDC_POPUPSENABLED, (DWORD)TlsGetValue(itlsSettings));
}

void ShowNotification(LPCSTR acc, POPUPDATAT *data, LPCTSTR jid, LPCTSTR url, LPCTSTR unreadCount)
{
	HANDLE hCnt = SetupPseudocontact(jid, unreadCount, acc, &data->lptzContactName[0]);
	HANDLE hEvt = ReadCheckbox(0, IDC_PSEUDOCONTACTENABLED, (DWORD)TlsGetValue(itlsSettings))
		? AddCListNotification(hCnt, acc, data, jid, url, unreadCount) : NULL;

	if (!UsePopups()) return;

	extern HICON g_hPopupIcon;
	data->lchIcon = g_hPopupIcon;
	data->iSeconds = (int)DBGetContactSettingDword(0, SHORT_PLUGIN_NAME, TIMEOUT_SETTING, 0);
	data->colorBack = (COLORREF)DBGetContactSettingDword(0, SHORT_PLUGIN_NAME, BACK_COLOR_SETTING, 0);
	data->colorText = (COLORREF)DBGetContactSettingDword(0, SHORT_PLUGIN_NAME, TEXT_COLOR_SETTING, 0);
	if (data->colorBack == data->colorText) {
		data->colorBack = 0;
		data->colorText = 0;
	}

	data->PluginWindowProc = PopupProc;
	int lurl = (lstrlen(url) + 1) * sizeof(TCHAR);
	int ljid = (lstrlen(jid) + 1) * sizeof(TCHAR);
	data->PluginData = malloc(sizeof(POPUP_DATA_HEADER) + lurl + ljid);
	__try {
		POPUP_DATA_HEADER *ppdh = (POPUP_DATA_HEADER*)data->PluginData;

		ppdh->MarkRead = FALSE;
		ppdh->hContact = hCnt;
		ppdh->hDbEvent = hEvt;

		ppdh->jid = (LPTSTR)((PBYTE)ppdh + sizeof(*ppdh));
		memcpy(ppdh->jid, jid, ljid);

		ppdh->url = (LPTSTR)((PBYTE)ppdh->jid + ljid);
		memcpy(ppdh->url, url, lurl);

		HWND code = DoAddPopup(data);
		if ((code == (HWND)-1) || (isOriginalPopups && !code))
			return;
		data->PluginData = NULL; // freed in popup wndproc
	}
	__finally {
		free(data->PluginData);
	}
}

void UnreadMailNotification(LPCSTR acc, LPCTSTR jid, LPCTSTR url, LPCTSTR unreadCount)
{
	POPUPDATAT data = {0};

	FormatPseudocontactDisplayName(&data.lptzContactName[0], jid, unreadCount);
	wsprintf(&data.lptzText[0], TranslateT(NUMBER_EMAILS_MESSAGE), unreadCount);

	ShowNotification(acc, &data, jid, url, unreadCount);
}

void UnreadThreadNotification(LPCSTR acc, LPCTSTR jid, LPCTSTR url, LPCTSTR unreadCount, const MAIL_THREAD_NOTIFICATION *mtn)
{
	POPUPDATAT data = {0};

	FormatPseudocontactDisplayName(&data.lptzContactName[0], jid, unreadCount);
	LPTSTR senders = (LPTSTR)malloc(SENDER_COUNT * 100 * sizeof(TCHAR));
	LPTSTR currSender = senders;
	__try {
		for (int i = 0; i < SENDER_COUNT && mtn->senders[i].addr; i++) {
			wsprintf(currSender, _T("    %s <%s>\n"), mtn->senders[i].name, mtn->senders[i].addr);
			currSender += lstrlen(currSender);
		}

		if (ReadCheckbox(0, IDC_ADDSNIP, (DWORD)TlsGetValue(itlsSettings)))
			wsprintf(&data.lptzText[0], TranslateTS(FULL_NOTIFICATION_FORMAT), mtn->subj, senders, mtn->snip);
		else
			wsprintf(&data.lptzText[0], TranslateTS(SHORT_NOTIFICATION_FORMAT), mtn->subj, senders);
	}
	__finally {
		free(senders);
	}

	ShowNotification(acc, &data, jid, url, unreadCount);
}

void ClearNotificationContactHistory(LPCSTR acc)
{
	HANDLE hEvent = 0;
	HANDLE hContact = (HANDLE)DBGetContactSettingDword(0, acc, PSEUDOCONTACT_LINK, 0);
	if (hContact && DBGetContactSettingByte(hContact, SHORT_PLUGIN_NAME, PSEUDOCONTACT_FLAG, 0))
		while ((hEvent = (HANDLE)CallService(MS_DB_EVENT_FINDLAST, (WPARAM)hContact, 0)) &&
			!CallService(MS_DB_EVENT_DELETE, (WPARAM)hContact, (LPARAM)hEvent)) {};
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
	if (pid != GetCurrentProcessId()) return TRUE;

	POPUP_IDENT_STRINGS *ppis = (POPUP_IDENT_STRINGS*)lParam;
	POPUP_DATA_HEADER *ppdh = (POPUP_DATA_HEADER*)GetProp(hwnd, PLUGIN_DATA_PROP_NAME);
	if (!ppdh) return TRUE;

	if (!lstrcmpi(ppis->url, ppdh->url) && !lstrcmpi(ppis->jid, ppdh->jid))
		SendMessage(hwnd, MESSAGE_CLOSEPOPUP, 0, 0);

	return TRUE;
}

void CloseNotifications(LPCSTR acc, LPCTSTR url, LPCTSTR jid, BOOL PopupsOnly)
{
	DWORD settings = (DWORD)TlsGetValue(itlsSettings);
	if (acc &&
		!PopupsOnly &&
		ReadCheckbox(0, IDC_PSEUDOCONTACTENABLED, settings) &&
		ReadCheckbox(0, IDC_CLEARPSEUDOCONTACTLOG, settings))
		ClearNotificationContactHistory(acc);

	POPUP_IDENT_STRINGS pis = {url, jid};
	EnumWindows(ClosePopupFunc, (LPARAM)&pis);
}