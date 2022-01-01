/*

Facebook plugin for Miranda NG
Copyright © 2019-22 Miranda NG team

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "stdafx.h"

/////////////////////////////////////////////////////////////////////////////////////////
// Invitation dialog

class CGroupchatInviteDlg : public CFBDlgBase
{
	CCtrlClc m_clc;
	SESSION_INFO *m_si;

	void FilterList(CCtrlClc *)
	{
		for (auto &hContact : Contacts()) {
			char *proto = Proto_GetBaseAccountName(hContact);
			if (mir_strcmp(proto, m_proto->m_szModuleName) || m_proto->isChatRoom(hContact))
				if (HANDLE hItem = m_clc.FindContact(hContact))
					m_clc.DeleteItem(hItem);
		}
	}

	void ResetListOptions(CCtrlClc *)
	{
		m_clc.SetHideEmptyGroups(1);
		m_clc.SetHideOfflineRoot(1);
		m_clc.SetOfflineModes(PF2_NONE);
	}

public:
	CGroupchatInviteDlg(FacebookProto *ppro, SESSION_INFO *si) :
		CFBDlgBase(ppro, IDD_GROUPCHAT_INVITE),
		m_si(si),
		m_clc(this, IDC_CLIST)
	{
		m_clc.OnNewContact =
			m_clc.OnListRebuilt = Callback(this, &CGroupchatInviteDlg::FilterList);
		m_clc.OnOptionsChanged = Callback(this, &CGroupchatInviteDlg::ResetListOptions);
	}

	bool OnInitDialog() override
	{
		SetWindowLongPtr(m_clc.GetHwnd(), GWL_STYLE,
			GetWindowLongPtr(m_clc.GetHwnd(), GWL_STYLE) | CLS_SHOWHIDDEN | CLS_HIDEOFFLINE | CLS_CHECKBOXES | CLS_HIDEEMPTYGROUPS | CLS_USEGROUPS | CLS_GREYALTERNATE | CLS_GROUPCHECKBOXES);
		m_clc.SendMsg(CLM_SETEXSTYLE, CLS_EX_DISABLEDRAGDROP | CLS_EX_TRACKSELECT, 0);

		ResetListOptions(&m_clc);
		FilterList(&m_clc);
		return true;
	}

	bool OnApply() override
	{
		JSONNode list(JSON_ARRAY);

		for (auto &hContact : m_proto->AccContacts()) {
			if (m_proto->isChatRoom(hContact))
				continue;

			if (HANDLE hItem = m_clc.FindContact(hContact)) {
				if (m_clc.GetCheck(hItem)) {
					JSONNode user; user << CHAR_PARAM("type", "id") << CHAR_PARAM("id", m_proto->getMStringA(hContact, DBKEY_ID));
					list << user;
				}
			}
		}

		auto *pReq = m_proto->CreateRequest(FB_API_URL_PARTS, "addMembers", "POST");
		pReq << CHAR_PARAM("to", list.write().c_str()) << WCHAR_PARAM("id", CMStringW(FORMAT, L"t_%s", m_si->ptszID));
		pReq->CalcSig();

		JsonReply reply(m_proto->ExecuteRequest(pReq));
		return true;
	}
};

void FacebookProto::Chat_InviteUser(SESSION_INFO *si)
{
	CGroupchatInviteDlg dlg(this, si);
	if (si->pDlg)
		dlg.SetParent(((CDlgBase *)si->pDlg)->GetHwnd());
	dlg.DoModal();
}

/////////////////////////////////////////////////////////////////////////////////////////
// Group chats

enum ChatMenuItems
{
	IDM_INVITE = 10, IDM_LEAVE,

	IDM_KICK = 20
};

static gc_item sttLogListItems[] =
{
	{ LPGENW("&Invite a user"), IDM_INVITE, MENU_ITEM },
	{ nullptr, 0, MENU_SEPARATOR },
	{ LPGENW("&Leave/destroy chat"), IDM_LEAVE, MENU_ITEM },
};

static gc_item sttNickListItems[] =
{
	{ LPGENW("&Kick user"), IDM_KICK, MENU_ITEM },
};

int FacebookProto::GroupchatMenuHook(WPARAM, LPARAM lParam)
{
	GCMENUITEMS *gcmi = (GCMENUITEMS *)lParam;
	if (gcmi == nullptr)
		return 0;

	if (mir_strcmpi(gcmi->pszModule, m_szModuleName))
		return 0;

	if (SESSION_INFO *si = g_chatApi.SM_FindSession(gcmi->pszID, gcmi->pszModule)) {
		if (gcmi->Type == MENU_ON_LOG)
			Chat_AddMenuItems(gcmi->hMenu, _countof(sttLogListItems), sttLogListItems, &g_plugin);
		if (gcmi->Type == MENU_ON_NICKLIST)
			Chat_AddMenuItems(gcmi->hMenu, _countof(sttNickListItems), sttNickListItems, &g_plugin);
	}

	return 0;
}

int FacebookProto::GroupchatEventHook(WPARAM, LPARAM lParam)
{
	GCHOOK *gch = (GCHOOK *)lParam;
	if (gch == nullptr)
		return 0;

	if (mir_strcmpi(gch->si->pszModule, m_szModuleName))
		return 0;

	SESSION_INFO *si = g_chatApi.SM_FindSession(gch->si->ptszID, gch->si->pszModule);
	if (si == nullptr)
		return 1;

	switch (gch->iType) {
	case GC_USER_MESSAGE:
		rtrimw(gch->ptszText);
		if (!mir_wstrlen(gch->ptszText))
			break;

		if (m_bOnline) {
			wchar_t *wszText = NEWWSTR_ALLOCA(gch->ptszText);
			Chat_UnescapeTags(wszText);

			int mid = SendMsg(si->hContact, 0, T2Utf(wszText));

			mir_cslock lck(m_csOwnMessages);
			for (auto &msg : arOwnMessages)
				if (msg->reqId == mid)
					msg->wszText = wszText;
		}
		break;

	case GC_USER_PRIVMESS:
		Chat_SendPrivateMessage(gch);
		break;

	case GC_USER_LOGMENU:
		Chat_ProcessLogMenu(si, gch);
		break;

	case GC_USER_NICKLISTMENU:
		Chat_ProcessNickMenu(si, gch);
		break;
	}

	return 1;
}

void FacebookProto::Chat_ProcessLogMenu(SESSION_INFO *si, GCHOOK *gch)
{
	switch (gch->dwData) {
	case IDM_INVITE:
		Chat_InviteUser(si);
		break;

	case IDM_LEAVE:
		Chat_Leave(si);
		break;
	}
}

void FacebookProto::Chat_ProcessNickMenu(SESSION_INFO *si, GCHOOK *gch)
{
	switch (gch->dwData) {
	case IDM_KICK:
		Chat_KickUser(si, gch->ptszUID);
		break;
	}
}

void FacebookProto::Chat_SendPrivateMessage(GCHOOK *gch)
{
	auto *pUser = FindUser(_wtoi64(gch->ptszUID));
	if (pUser == nullptr) {
		pUser = AddContact(gch->ptszUID, true);
		setWString(pUser->hContact, "Nick", gch->ptszNick);
		db_set_b(pUser->hContact, "CList", "Hidden", 1);
		db_set_dw(pUser->hContact, "Ignore", "Mask1", 0);
	}

	CallService(MS_MSG_SENDMESSAGE, pUser->hContact, 0);
}

int FacebookProto::Chat_KickUser(SESSION_INFO *si, const wchar_t *pwszUid)
{
	auto *pReq = CreateRequest(FB_API_URL_PARTS, "removeMembers", "DELETE");
	pReq << WCHAR_PARAM("id", CMStringW(FORMAT, L"t_%s", si->ptszID));
	if (pwszUid != nullptr) {
		JSONNode list(JSON_ARRAY);
		JSONNode user; user << CHAR_PARAM("type", "id") << WCHAR_PARAM("id", pwszUid);
		list << user;
		pReq << CHAR_PARAM("to", list.write().c_str());
	}
	pReq->CalcSig();

	JsonReply reply(ExecuteRequest(pReq));
	return reply.error();
}

/////////////////////////////////////////////////////////////////////////////////////////

static void __cdecl DestroyRoomThread(SESSION_INFO *si)
{
	::Sleep(100);
	Chat_Terminate(si->pszModule, si->ptszID, true);
}

void FacebookProto::Chat_Leave(SESSION_INFO *si)
{
	if (Chat_KickUser(si, nullptr) == 0)
		mir_forkThread<SESSION_INFO>(DestroyRoomThread, si);
}
