/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org),
Copyright (c) 2000-10 Miranda ICQ/IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

Portions of this code modified for Simple Status Message plugin
Copyright (C) 2006-2011 Bartosz 'Dezeath' Białek, (C) 2005 Harven

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "stdafx.h"

#define HM_AWAYMSG  (WM_USER + 10)

static MWindowList hWindowList, hWindowList2;

class CReadAwayMsgDlg : public CDlgBase
{
	CCtrlButton btnCopy;

	MCONTACT m_hContact;
	HANDLE m_hSeq = nullptr;
	HANDLE m_hAwayMsgEvent = nullptr;

public:
	CReadAwayMsgDlg(MCONTACT hContact) :
		CDlgBase(g_plugin, IDD_READAWAYMSG),
		btnCopy(this, IDC_COPY),
		m_hContact(hContact)
	{
		btnCopy.OnClick = Callback(this, &CReadAwayMsgDlg::onClick_Copy);
	}

	bool OnInitDialog() override
	{
		m_hSeq = (HANDLE)ProtoChainSend(m_hContact, PSS_GETAWAYMSG, 0, 0);
		m_hAwayMsgEvent = m_hSeq ? HookEventMessage(ME_PROTO_ACK, m_hwnd, HM_AWAYMSG) : nullptr;
		WindowList_Add(hWindowList, m_hwnd, m_hContact);

		wchar_t str[256], format[128];
		wchar_t *contactName = Clist_GetContactDisplayName(m_hContact, 0);
		char *szProto = Proto_GetBaseAccountName(m_hContact);
		uint16_t dwStatus = db_get_w(m_hContact, szProto, "Status", ID_STATUS_OFFLINE);
		wchar_t *status = Clist_GetStatusModeDescription(dwStatus, 0);

		GetWindowText(m_hwnd, format, _countof(format));
		mir_snwprintf(str, format, status, contactName);
		SetWindowText(m_hwnd, str);
		if (m_hSeq) {
			GetDlgItemText(m_hwnd, IDC_RETRIEVING, format, _countof(format));
			mir_snwprintf(str, format, status);
		}
		else {
			mir_snwprintf(str, TranslateT("Failed to retrieve %s message."), status);
			SetDlgItemText(m_hwnd, IDOK, TranslateT("&Close"));
		}
		SetDlgItemText(m_hwnd, IDC_RETRIEVING, str);
		Window_SetProtoIcon_IcoLib(m_hwnd, szProto, dwStatus);
		EnableWindow(GetDlgItem(m_hwnd, IDC_COPY), FALSE);

		Utils_RestoreWindowPosition(m_hwnd, m_hContact, "SRAway", "AwayMsgDlg");
		return true;
	}

	void OnDestroy() override
	{
		UnhookEvent(m_hAwayMsgEvent);
		Utils_SaveWindowPosition(m_hwnd, m_hContact, "SRAway", "AwayMsgDlg");
		WindowList_Remove(hWindowList, m_hwnd);
		Window_FreeIcon_IcoLib(m_hwnd);
	}

	INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam) override
	{
		if (msg == HM_AWAYMSG) {
			ACKDATA *ack = (ACKDATA *)lParam;
			if (ack->hContact != m_hContact || ack->type != ACKTYPE_AWAYMSG)
				return 0;
			if (ack->result != ACKRESULT_SUCCESS)
				return 0;
			if (m_hAwayMsgEvent && ack->hProcess == m_hSeq) {
				UnhookEvent(m_hAwayMsgEvent);
				m_hAwayMsgEvent = nullptr;
			}

			CMStringW wszMsg((wchar_t *)ack->lParam);
			wszMsg.Replace(L"\n", L"\r\n");
			SetDlgItemText(m_hwnd, IDC_MSG, wszMsg);

			EnableWindow(GetDlgItem(m_hwnd, IDC_COPY), !wszMsg.IsEmpty());
			ShowWindow(GetDlgItem(m_hwnd, IDC_RETRIEVING), SW_HIDE);
			ShowWindow(GetDlgItem(m_hwnd, IDC_MSG), SW_SHOW);
			SetDlgItemText(m_hwnd, IDOK, TranslateT("&Close"));
		}

		return CDlgBase::DlgProc(msg, wParam, lParam);
	}

	void onClick_Copy(CCtrlButton*)
	{
		wchar_t msg[1024];
		GetDlgItemText(m_hwnd, IDC_MSG, msg, _countof(msg));
		Utils_ClipboardCopy(msg);
	}
};

static INT_PTR GetMessageCommand(WPARAM wParam, LPARAM)
{
	if (HWND hwnd = WindowList_Find(hWindowList, wParam)) {
		SetForegroundWindow(hwnd);
		SetFocus(hwnd);
	}
	else {
		CReadAwayMsgDlg *pDlg = new CReadAwayMsgDlg(wParam);
		pDlg->Show();
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

class CCopyAwayMsgDlg : public CDlgBase
{
	MCONTACT m_hContact;
	HANDLE m_hSeq;
	HANDLE m_hAwayMsgEvent;

public:
	CCopyAwayMsgDlg(MCONTACT hContact) :
		CDlgBase(g_plugin, IDD_COPY),
		m_hContact(hContact)
	{}

	bool OnInitDialog() override
	{
		m_hSeq = (HANDLE)ProtoChainSend(m_hContact, PSS_GETAWAYMSG, 0, 0);
		m_hAwayMsgEvent = m_hSeq ? HookEventMessage(ME_PROTO_ACK, m_hwnd, HM_AWAYMSG) : nullptr;
		WindowList_Add(hWindowList2, m_hwnd, m_hContact);

		wchar_t str[256], format[128];
		GetWindowText(m_hwnd, format, _countof(format));
		mir_snwprintf(str, format, Clist_GetContactDisplayName(m_hContact));
		SetWindowText(m_hwnd, str);
		return (m_hSeq != 0);
	}

	void OnDestroy() override
	{
		UnhookEvent(m_hAwayMsgEvent);
		WindowList_Remove(hWindowList2, m_hwnd);
	}

	INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam) override
	{
		if (msg == HM_AWAYMSG) {
			ACKDATA *ack = (ACKDATA *)lParam;
			if (ack->hContact != m_hContact || ack->type != ACKTYPE_AWAYMSG) {
				Close();
				return 0;
			}
			if (ack->result != ACKRESULT_SUCCESS) {
				Close();
				return 0;
			}
			if (m_hAwayMsgEvent && ack->hProcess == m_hSeq) {
				UnhookEvent(m_hAwayMsgEvent);
				m_hAwayMsgEvent = nullptr;
			}

			CMStringW wszMsg((wchar_t *)ack->lParam);
			wszMsg.Replace(L"\n", L"\r\n");
			Utils_ClipboardCopy(wszMsg);

			Close();
		}

		return CDlgBase::DlgProc(msg, wParam, lParam);
	}
};

static INT_PTR CopyAwayMsgCommand(WPARAM wParam, LPARAM)
{
	if (HWND hwnd = WindowList_Find(hWindowList2, wParam)) {
		SetForegroundWindow(hwnd);
		SetFocus(hwnd);
	}
	else {
		CCopyAwayMsgDlg *pDlg = new CCopyAwayMsgDlg(wParam);
		pDlg->Show();
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

static char* StrFindURL(char *pszStr)
{
	char *pszURL = nullptr;

	if (pszStr != nullptr && *pszStr != '\0') {
		pszURL = strstr(pszStr, "www.");
		if (pszURL == nullptr)
			pszURL = strstr(pszStr, "http://");
		if (pszURL == nullptr)
			pszURL = strstr(pszStr, "https://");
		if (pszURL == nullptr)
			pszURL = strstr(pszStr, "ftp://");
	}

	return pszURL;
}

static INT_PTR GoToURLMsgCommand(WPARAM wParam, LPARAM)
{
	ptrA szMsg(db_get_sa(wParam, "CList", "StatusMsg"));

	char *szURL = StrFindURL(szMsg);
	if (szURL != nullptr) {
		int i;
		for (i = 0; szURL[i] != ' ' && szURL[i] != '\n' && szURL[i] != '\r' && szURL[i] != '\t' && szURL[i] != '\0'; i++);

		char *szMsgURL = (char *)mir_alloc(i + 1);
		if (szMsgURL) {
			mir_strncpy(szMsgURL, szURL, i + 1);
			Utils_OpenUrl(szMsgURL);
			mir_free(szMsgURL);
		}
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// contact menu items

static HGENMENU hAwayMsgMenuItem, hCopyMsgMenuItem, hGoToURLMenuItem;

static int AwayMsgPreBuildMenu(WPARAM hContact, LPARAM)
{
	Menu_ShowItem(hCopyMsgMenuItem, false);
	Menu_ShowItem(hGoToURLMenuItem, false);
	Menu_ShowItem(hAwayMsgMenuItem, false);

	char *szProto = Proto_GetBaseAccountName(hContact);
	if (szProto == nullptr || Contact::IsGroupChat(hContact, szProto))
		return 0;

	int iStatus = db_get_w(hContact, szProto, "Status", ID_STATUS_OFFLINE);
	if (CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_MODEMSGRECV) {
		if (CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_3, 0) & Proto_Status2Flag(iStatus == ID_STATUS_OFFLINE ? ID_STATUS_INVISIBLE : iStatus)) {
			wchar_t str[128];
			HANDLE hIcon = Skin_GetProtoIcon(szProto, iStatus);
			mir_snwprintf(str, TranslateT("Re&ad %s message"), Clist_GetStatusModeDescription(iStatus, 0));
			Menu_ModifyItem(hAwayMsgMenuItem, str, hIcon, 0);
			IcoLib_ReleaseIcon((HICON)hIcon);

			ptrA szMsg(db_get_sa(hContact, "CList", "StatusMsg"));
			if (szMsg != nullptr) {
				mir_snwprintf(str, TranslateT("Copy %s message"), Clist_GetStatusModeDescription(iStatus, 0));
				Menu_ModifyItem(hCopyMsgMenuItem, str);

				if (StrFindURL(szMsg) != nullptr) {
					mir_snwprintf(str, TranslateT("&Go to URL in %s message"), Clist_GetStatusModeDescription(iStatus, 0));
					Menu_ModifyItem(hGoToURLMenuItem, str);
				}
			}
		}
	}

	return 0;
}

int AwayMsgPreShutdown(void)
{
	if (hWindowList) {
		WindowList_Broadcast(hWindowList, WM_CLOSE, 0, 0);
		WindowList_Destroy(hWindowList);
	}
	if (hWindowList2) {
		WindowList_Broadcast(hWindowList2, WM_CLOSE, 0, 0);
		WindowList_Destroy(hWindowList2);
	}
	return 0;
}

int LoadAwayMsgModule(void)
{
	hWindowList = WindowList_Create();
	hWindowList2 = WindowList_Create();

	CMenuItem mi(&g_plugin);
	mi.flags = CMIF_UNICODE;

	SET_UID(mi, 0xd3282acc, 0x9ff1, 0x4ede, 0x8a, 0x1e, 0x36, 0x72, 0x3f, 0x44, 0x4f, 0x84);
	CreateServiceFunction(MS_AWAYMSG_SHOWAWAYMSG, GetMessageCommand);
	mi.position = -2000005000;
	mi.name.w = LPGENW("Re&ad Away message");
	mi.pszService = MS_AWAYMSG_SHOWAWAYMSG;
	hAwayMsgMenuItem = Menu_AddContactMenuItem(&mi);

	SET_UID(mi, 0x311124e9, 0xb477, 0x42ef, 0x84, 0xd2, 0xc, 0x6c, 0x50, 0x3f, 0x4a, 0x84);
	CreateServiceFunction(MS_SIMPLESTATUSMSG_COPYMSG, CopyAwayMsgCommand);
	mi.position = -2000006000;
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_COPY);
	mi.name.w = LPGENW("Copy Away message");
	mi.pszService = MS_SIMPLESTATUSMSG_COPYMSG;
	hCopyMsgMenuItem = Menu_AddContactMenuItem(&mi);

	SET_UID(mi, 0xe2c75070, 0x455d, 0x455f, 0xbf, 0x53, 0x86, 0x64, 0xbc, 0x14, 0xa1, 0xbe);
	CreateServiceFunction(MS_SIMPLESTATUSMSG_GOTOURLMSG, GoToURLMsgCommand);
	mi.position = -2000007000;
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_GOTOURL);
	mi.name.w = LPGENW("&Go to URL in Away message");
	mi.pszService = MS_SIMPLESTATUSMSG_GOTOURLMSG;
	hGoToURLMenuItem = Menu_AddContactMenuItem(&mi);

	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, AwayMsgPreBuildMenu);
	return 0;
}
