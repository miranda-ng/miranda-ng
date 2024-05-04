/*
Copyright © 2016-22 Miranda NG team

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

enum {
	IDM_CANCEL,
	IDM_COPY_ID,

	IDM_CHANGENICK, IDM_CHANGETOPIC, IDM_RENAME, IDM_DESTROY, IDM_LEAVE,

	IDM_KICK, IDM_INVITE
};

static void sttDisableMenuItem(int nItems, gc_item *items, uint32_t id, bool disabled)
{
	for (int i = 0; i < nItems; i++)
		if (items[i].dwID == id)
			items[i].bDisabled = disabled;
}

static void sttShowGcMenuItem(int nItems, gc_item *items, uint32_t id, int type)
{
	for (int i = 0; i < nItems; i++)
		if (items[i].dwID == id)
			items[i].uType = type;
}

/////////////////////////////////////////////////////////////////////////////////////////

static int SortRolesByPosition(const CDiscordRole *p1, const CDiscordRole *p2)
{
	return p1->position - p2->position;
}

void BuildStatusList(const CDiscordGuild *pGuild, SESSION_INFO *si)
{
	Chat_AddGroup(si, L"@owner");

	LIST<CDiscordRole> roles(pGuild->arRoles.getCount(), SortRolesByPosition);
	for (auto &it : pGuild->arRoles)
		roles.insert(it);

	for (auto &it : roles)
		Chat_AddGroup(si, it->wszName);
}

/////////////////////////////////////////////////////////////////////////////////////////

static gc_item sttLogListItems[] =
{
	{ LPGENW("Change &nickname"), IDM_CHANGENICK, MENU_ITEM },
	{ LPGENW("Invite users"), IDM_INVITE, MENU_ITEM },
	{ LPGENW("Channel control"), FALSE, MENU_NEWPOPUP },
	{ LPGENW("Change &topic"), IDM_CHANGETOPIC, MENU_POPUPITEM },
	{ LPGENW("&Rename channel"), IDM_RENAME, MENU_POPUPITEM },
	{ nullptr, 0, MENU_POPUPSEPARATOR },
	{ LPGENW("Destroy channel"), IDM_DESTROY, MENU_POPUPITEM },
	{ LPGENW("Leave channel"), IDM_LEAVE, MENU_POPUPITEM },
};

static gc_item sttNicklistItems[] =
{
	{ LPGENW("Copy ID"), IDM_COPY_ID, MENU_ITEM },
	{ nullptr, 0, MENU_SEPARATOR },
	{ LPGENW("Kick user"), IDM_KICK, MENU_ITEM },
};

int CDiscordProto::GroupchatMenuHook(WPARAM, LPARAM lParam)
{
	GCMENUITEMS* gcmi = (GCMENUITEMS*)lParam;
	if (gcmi == nullptr)
		return 0;

	if (mir_strcmpi(gcmi->pszModule, m_szModuleName))
		return 0;

	CDiscordUser *pChat = FindUserByChannel(_wtoi64(gcmi->pszID));
	if (pChat == nullptr || pChat->si == nullptr)
		return 0;

	bool isOwner = getId(pChat->hContact, DB_KEY_OWNERID) == m_ownId;

	if (gcmi->Type == MENU_ON_LOG) {
		if (pChat->pGuild == nullptr)
			sttShowGcMenuItem(_countof(sttLogListItems), sttLogListItems, IDM_CHANGENICK, 0);

		sttShowGcMenuItem(_countof(sttLogListItems), sttLogListItems, IDM_LEAVE, isOwner ? 0 : MENU_POPUPITEM);
		sttShowGcMenuItem(_countof(sttLogListItems), sttLogListItems, IDM_DESTROY, isOwner ? MENU_POPUPITEM : 0);

		Chat_AddMenuItems(gcmi->hMenu, _countof(sttLogListItems), sttLogListItems, &g_plugin);
	}
	else if (gcmi->Type == MENU_ON_NICKLIST) {
		sttDisableMenuItem(_countof(sttNicklistItems), sttNicklistItems, IDM_KICK, !isOwner);

		Chat_AddMenuItems(gcmi->hMenu, _countof(sttNicklistItems), sttNicklistItems, &g_plugin);
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CDiscordProto::Chat_SendPrivateMessage(GCHOOK *gch)
{
	SnowFlake userId = _wtoi64(gch->ptszUID);

	MCONTACT hContact;
	CDiscordUser *pUser = FindUser(userId);
	if (pUser == nullptr) {
		PROTOSEARCHRESULT psr = { sizeof(psr) };
		psr.id.w = (wchar_t*)gch->ptszUID;
		psr.nick.w = (wchar_t*)gch->ptszNick;
		if ((hContact = AddToList(PALF_TEMPORARY, &psr)) == 0)
			return;

		setId(hContact, DB_KEY_ID, userId);
		setId(hContact, DB_KEY_CHANNELID, _wtoi64(gch->si->ptszID));
		setWString(hContact, DB_KEY_NICK, gch->ptszNick);
		Contact::Hide(hContact);
		db_set_dw(hContact, "Ignore", "Mask1", 0);
	}
	else hContact = pUser->hContact;

	CallService(MS_MSG_SENDMESSAGE, hContact, 0);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Invitation dialog

class CGroupchatInviteDlg : public CDiscordDlgBase
{
	CCtrlClc m_clc;
	SnowFlake m_iChatId;

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
		m_clc.SetHideEmptyGroups(true);
		m_clc.SetHideOfflineRoot(true);
	}

public:
	CGroupchatInviteDlg(CDiscordProto *ppro, SnowFlake chatId) :
		CDiscordDlgBase(ppro, IDD_GROUPCHAT_INVITE),
		m_clc(this, IDC_CLIST),
		m_iChatId(chatId)
	{
		m_clc.OnNewContact =
			m_clc.OnListRebuilt = Callback(this, &CGroupchatInviteDlg::FilterList);
		m_clc.OnOptionsChanged = Callback(this, &CGroupchatInviteDlg::ResetListOptions);
	}

	bool OnInitDialog() override
	{
		SetWindowLongPtr(m_clc.GetHwnd(), GWL_STYLE,
			GetWindowLongPtr(m_clc.GetHwnd(), GWL_STYLE) | CLS_SHOWHIDDEN | CLS_CHECKBOXES | CLS_HIDEEMPTYGROUPS | CLS_USEGROUPS | CLS_GREYALTERNATE | CLS_GROUPCHECKBOXES);
		m_clc.SendMsg(CLM_SETEXSTYLE, CLS_EX_DISABLEDRAGDROP | CLS_EX_TRACKSELECT, 0);
		ResetListOptions(&m_clc);
		FilterList(&m_clc);
		return true;
	}

	bool OnApply() override
	{
		// invite users from roster
		for (auto &hContact : m_proto->AccContacts()) {
			if (m_proto->isChatRoom(hContact))
				continue;

			if (HANDLE hItem = m_clc.FindContact(hContact)) {
				if (m_clc.GetCheck(hItem)) {
					CMStringA szUrl(FORMAT, "/channels/%lld/recipients/%lld", m_iChatId, m_proto->getId(hContact, DB_KEY_ID));
					m_proto->Push(new AsyncHttpRequest(m_proto, REQUEST_PUT, szUrl, 0));
				}
			}
		}
		return true;
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// Log menu

void CDiscordProto::LeaveChat(CDiscordUser *pChat)
{
	CMStringA szUrl(FORMAT, "/channels/%S", pChat->wszUsername.c_str());
	Push(new AsyncHttpRequest(this, REQUEST_DELETE, szUrl, nullptr));
}

INT_PTR CDiscordProto::SvcLeaveChat(WPARAM hContact, LPARAM)
{
	if (auto *pUser = FindUserByChannel(getId(hContact, DB_KEY_CHANNELID)))
		if (pUser->si)
			LeaveChat(pUser);
	return 0;
}

void CDiscordProto::Chat_ProcessLogMenu(GCHOOK *gch)
{
	CDiscordUser *pUser = FindUserByChannel(_wtoi64(gch->si->ptszID));
	if (pUser == nullptr)
		return;

	ENTER_STRING es = {};
	es.szModuleName = m_szModuleName;

	switch (gch->dwData) {
	case IDM_DESTROY:
		if (IDYES == MessageBox(nullptr, TranslateT("Do you really want to destroy this channel? This action is non-revertable."), m_tszUserName, MB_YESNO | MB_ICONQUESTION))
			LeaveChat(pUser);
		break;

	case IDM_LEAVE:
		LeaveChat(pUser);
		break;

	case IDM_RENAME:
		es.caption = TranslateT("Enter new channel name:");
		es.type = ESF_COMBO;
		es.szDataPrefix = "chat_rename";
		if (EnterString(&es)) {
			JSONNode root; root << WCHAR_PARAM("name", es.ptszResult);
			CMStringA szUrl(FORMAT, "/channels/%S", pUser->wszUsername.c_str());
			Push(new AsyncHttpRequest(this, REQUEST_PATCH, szUrl, nullptr, &root));
			mir_free(es.ptszResult);
		}
		break;

	case IDM_CHANGETOPIC:
		es.caption = TranslateT("Enter new topic:");
		es.type = ESF_RICHEDIT;
		es.szDataPrefix = "chat_topic";
		if (EnterString(&es)) {
			JSONNode root; root << WCHAR_PARAM("topic", es.ptszResult);
			CMStringA szUrl(FORMAT, "/channels/%S", pUser->wszUsername.c_str());
			Push(new AsyncHttpRequest(this, REQUEST_PATCH, szUrl, nullptr, &root));
			mir_free(es.ptszResult);
		}
		break;

	case IDM_CHANGENICK:
		es.caption = TranslateT("Enter your new nick name:");
		es.type = ESF_COMBO;
		es.szDataPrefix = "chat_nick";
		es.recentCount = 5;
		if (EnterString(&es)) {
			JSONNode root; root << WCHAR_PARAM("nick", es.ptszResult);
			CMStringA szUrl(FORMAT, "/guilds/%lld/members/@me/nick", pUser->pGuild->m_id);
			Push(new AsyncHttpRequest(this, REQUEST_PATCH, szUrl, nullptr, &root));
			mir_free(es.ptszResult);
		}
		break;

	case IDM_INVITE:
		CGroupchatInviteDlg dlg(this, pUser->channelId);
		if (gch->si->pDlg)
			dlg.SetParent(gch->si->pDlg->GetHwnd());
		dlg.DoModal();
		break;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// Nick list menu

void CDiscordProto::KickChatUser(CDiscordUser *pChat, const wchar_t *pszUID)
{
	CMStringA szUrl(FORMAT, "/channels/%lld/recipients/%S", pChat->channelId, pszUID);
	Push(new AsyncHttpRequest(this, REQUEST_DELETE, szUrl, 0));
}

void CDiscordProto::Chat_ProcessNickMenu(GCHOOK* gch)
{
	auto *pChannel = FindUserByChannel(_wtoi64(gch->si->ptszID));
	if (pChannel == nullptr)
		return;

	switch (gch->dwData) {
	case IDM_COPY_ID:
		CopyId(gch->ptszUID);
		break;

	case IDM_KICK:
		KickChatUser(pChannel, gch->ptszUID);
		break;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

int CDiscordProto::GroupchatEventHook(WPARAM, LPARAM lParam)
{
	GCHOOK *gch = (GCHOOK*)lParam;
	if (gch == nullptr)
		return 0;

	if (mir_strcmpi(gch->si->pszModule, m_szModuleName))
		return 0;

	switch (gch->iType) {
	case GC_USER_MESSAGE:
		if (m_bOnline && mir_wstrlen(gch->ptszText) > 0) {
			CMStringW wszText(gch->ptszText);
			wszText.TrimRight();

			int pos = wszText.Find(':');
			if (pos != -1) {
				auto wszWord = wszText.Left(pos);
				wszWord.Trim();
				if (auto *si = Chat_Find(gch->si->ptszID, gch->si->pszModule)) {
					USERINFO *pUser = nullptr;

					for (auto &U : si->getUserList())
						if (wszWord == U->pszNick) {
							pUser = U;
							break;
						}

					if (pUser) {
						wszText.Delete(0, pos);
						wszText.Insert(0, L"<@" + CMStringW(pUser->pszUID) + L">");
					}
				}
			}

			Chat_UnescapeTags(wszText.GetBuffer());

			JSONNode body; body << WCHAR_PARAM("content", wszText);
			CMStringA szUrl(FORMAT, "/channels/%S/messages", gch->si->ptszID);
			Push(new AsyncHttpRequest(this, REQUEST_POST, szUrl, nullptr, &body));
		}
		break;

	case GC_USER_PRIVMESS:
		Chat_SendPrivateMessage(gch);
		break;

	case GC_USER_LOGMENU:
		Chat_ProcessLogMenu(gch);
		break;

	case GC_USER_NICKLISTMENU:
		Chat_ProcessNickMenu(gch);
		break;

	case GC_USER_TYPNOTIFY:
		UserIsTyping(gch->si->hContact, (int)gch->dwData);
		break;
	}

	return 1;
}
