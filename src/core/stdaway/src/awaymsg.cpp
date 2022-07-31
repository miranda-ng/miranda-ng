/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org),
Copyright (c) 2000-12 Miranda IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "stdafx.h"

int LoadAwayMessageSending(void);

static HGENMENU hAwayMsgMenuItem;
static MWindowList hWindowList;

#define HM_AWAYMSG (WM_USER+10)

class CReadAwayMsgDlg : public CDlgBase
{
	MCONTACT m_hContact;
	HANDLE m_hSeq;
	HANDLE m_hAwayMsgEvent;

	UI_MESSAGE_MAP(CReadAwayMsgDlg, CDlgBase);
	UI_MESSAGE(HM_AWAYMSG, OnProtoAck);
	UI_MESSAGE_MAP_END();

	INT_PTR OnProtoAck(UINT, WPARAM, LPARAM lParam)
	{
		ACKDATA *ack = (ACKDATA *)lParam;
		if (ack->hContact != m_hContact || ack->type != ACKTYPE_AWAYMSG || ack->result != ACKRESULT_SUCCESS)
			return 0;

		if (m_hAwayMsgEvent && ack->hProcess == m_hSeq) {
			UnhookEvent(m_hAwayMsgEvent);
			m_hAwayMsgEvent = nullptr;
		}

		m_msg.SetText((const wchar_t *)ack->lParam);

		m_retr.Hide();
		m_msg.Show();
		SetDlgItemText(m_hwnd, IDOK, TranslateT("&Close"));
		return 0;
	}

	CCtrlBase m_retr, m_msg;

public:
	CReadAwayMsgDlg(MCONTACT hContact) :
		CDlgBase(g_plugin, IDD_READAWAYMSG),
		m_msg(this, IDC_MSG),
		m_retr(this, IDC_RETRIEVING)
	{
		m_hContact = db_mc_getMostOnline(hContact);
		if (m_hContact == 0)
			m_hContact = hContact;
	}

	bool OnInitDialog() override
	{
		m_hSeq = (HANDLE)ProtoChainSend(m_hContact, PSS_GETAWAYMSG, 0, 0);
		if (m_hSeq == nullptr) {
			ACKDATA ack = {};
			ack.hContact = m_hContact;
			ack.type = ACKTYPE_AWAYMSG;
			ack.result = ACKRESULT_SUCCESS;
			SendMessage(m_hwnd, HM_AWAYMSG, 0, (LPARAM)&ack);
			return false;
		}

		m_hAwayMsgEvent = HookEventMessage(ME_PROTO_ACK, m_hwnd, HM_AWAYMSG);
		WindowList_Add(hWindowList, m_hwnd, m_hContact);

		wchar_t *contactName = Clist_GetContactDisplayName(m_hContact);
		char *szProto = Proto_GetBaseAccountName(m_hContact);
		uint16_t dwStatus = db_get_w(m_hContact, szProto, "Status", ID_STATUS_OFFLINE);
		wchar_t *status = Clist_GetStatusModeDescription(dwStatus, 0);

		wchar_t str[256], format[128];
		GetWindowText(m_hwnd, format, _countof(format));
		mir_snwprintf(str, format, status, contactName);
		SetWindowText(m_hwnd, str);
				
		mir_snwprintf(str, ptrW(m_retr.GetText()).get(), status);
		m_retr.SetText(str);

		Window_SetProtoIcon_IcoLib(m_hwnd, szProto, dwStatus);

		Utils_RestoreWindowPosition(m_hwnd, m_hContact, MODULENAME, "AwayMsgDlg");
		return true;
	}

	void OnDestroy()
	{
		if (m_hAwayMsgEvent)
			UnhookEvent(m_hAwayMsgEvent);
		Utils_SaveWindowPosition(m_hwnd, m_hContact, MODULENAME, "AwayMsgDlg");
		WindowList_Remove(hWindowList, m_hwnd);
		Window_FreeIcon_IcoLib(m_hwnd);
	}
};

static INT_PTR GetMessageCommand(WPARAM hContact, LPARAM)
{
	if (HWND hwnd = WindowList_Find(hWindowList, hContact)) {
		SetForegroundWindow(hwnd);
		SetFocus(hwnd);
	}
	else (new CReadAwayMsgDlg(hContact))->Show();
	return 0;
}

static int AwayMsgPreBuildMenu(WPARAM hContact, LPARAM)
{
	char *szProto = Proto_GetBaseAccountName(hContact);
	if (szProto != nullptr) {
		if (!Contact::IsGroupChat(hContact, szProto)) {
			int status = db_get_w(hContact, szProto, "Status", ID_STATUS_OFFLINE);
			if (CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_MODEMSGRECV) {
				if (CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_3, 0) & Proto_Status2Flag(status)) {
					wchar_t str[128];
					mir_snwprintf(str, TranslateT("Re&ad %s message"), Clist_GetStatusModeDescription(status, 0));
					Menu_ModifyItem(hAwayMsgMenuItem, str, Skin_GetProtoIcon(szProto, status), CMIF_NOTOFFLINE);
					return 0;
				}
			}
		}
	}

	Menu_ShowItem(hAwayMsgMenuItem, false);
	return 0;
}

static int AwayMsgPreShutdown(WPARAM, LPARAM)
{
	if (hWindowList) {
		WindowList_BroadcastAsync(hWindowList, WM_CLOSE, 0, 0);
		WindowList_Destroy(hWindowList);
	}
	return 0;
}

int LoadAwayMsgModule(void)
{
	hWindowList = WindowList_Create();
	CreateServiceFunction(MS_AWAYMSG_SHOWAWAYMSG, GetMessageCommand);

	CMenuItem mi(&g_plugin);
	SET_UID(mi, 0xd3282acc, 0x9ff1, 0x4ede, 0x8a, 0x1e, 0x36, 0x72, 0x3f, 0x44, 0x4f, 0x84);
	mi.position = -2000005000;
	mi.flags = CMIF_NOTOFFLINE;
	mi.name.a = LPGEN("Re&ad status message");
	mi.pszService = MS_AWAYMSG_SHOWAWAYMSG;
	hAwayMsgMenuItem = Menu_AddContactMenuItem(&mi);
	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, AwayMsgPreBuildMenu);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, AwayMsgPreShutdown);
	return LoadAwayMessageSending();
}
