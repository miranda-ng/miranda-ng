// -----------------------------------------------------------------------------
// ICQ plugin for Miranda NG
// -----------------------------------------------------------------------------
// Copyright © 2018-24 Miranda NG team
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
// -----------------------------------------------------------------------------

#include "stdafx.h"

SESSION_INFO* CIcqProto::GcCreate(const wchar_t *pwszId, const wchar_t *pwszNick)
{
	auto *si = Chat_NewSession(GCW_CHATROOM, m_szModuleName, pwszId, pwszNick);
	if (si == nullptr)
		return nullptr;

	if (si->pStatuses == 0) {
		Chat_AddGroup(si, TranslateT("admin"));
		Chat_AddGroup(si, TranslateT("member"));
	}
	
	Chat_Control(si, m_bHideGroupchats ? WINDOW_HIDDEN : SESSION_INITDONE);
	Chat_Control(si, SESSION_ONLINE);

	// #3420 ICQ server will place our group chat into its own group
	Clist_SetGroup(si->hContact, nullptr);

	if (si->ptszTopic) {
		GCEVENT gce = { si, GC_EVENT_TOPIC };
		gce.time = ::time(0);
		gce.pszText.w = si->ptszTopic; si->ptszTopic = 0;
		Chat_Event(&gce);

		mir_free((void*)gce.pszText.w);
	}

	return si;
}

INT_PTR CIcqProto::SvcLeaveChat(WPARAM hContact, LPARAM)
{
	CMStringW wszId(GetUserId(hContact));
	if (auto *si = Chat_Find(wszId, m_szModuleName))
		LeaveDestroyChat(si);

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CIcqProto::OnGetChatInfo(MHttpResponse *pReply, AsyncHttpRequest *pReq)
{
	RobustReply root(pReply);
	if (root.error() != 20000)
		return;

	SESSION_INFO *si = (SESSION_INFO *)pReq->pUserInfo;

	const JSONNode &results = root.results();
	for (auto &it : results["members"]) {
		CMStringW friendly = it["friendly"].as_mstring();
		CMStringW role = it["role"].as_mstring();
		CMStringW sn = it["sn"].as_mstring();

		GCEVENT gce = { si, GC_EVENT_JOIN };
		gce.dwFlags = GCEF_SILENT;
		gce.pszNick.w = friendly;
		gce.pszUID.w = sn;
		gce.time = ::time(0);
		gce.bIsMe = sn == m_szOwnId;
		gce.pszStatus.w = TranslateW(role);
		Chat_Event(&gce);
	}

	CMStringW wszAbout = results["about"].as_mstring(), wszTopic;
	CMStringW wszRules = results["rules"].as_mstring();
	if (!wszAbout.IsEmpty())
		wszTopic.AppendFormat(L"%s: %s", TranslateT("About"), wszAbout.c_str());
	if (!wszRules.IsEmpty()) {
		if (!wszTopic.IsEmpty())
			wszTopic.Append(L"\r\n");
		wszTopic.AppendFormat(L"%s: %s", TranslateT("Rules"), wszRules.c_str());
	}
	if (!wszTopic.IsEmpty()) {
		GCEVENT gce = { si, GC_EVENT_TOPIC };
		gce.time = ::time(0);
		gce.pszText.w = wszTopic;
		Chat_Event(&gce);
	}

	setId(si->hContact, "InfoVersion", _wtoi64(results["infoVersion"].as_mstring()));
	setId(si->hContact, "MembersVersion", _wtoi64(results["membersVersion"].as_mstring()));
}

void CIcqProto::RetrieveChatInfo(MCONTACT hContact)
{
	CMStringW wszChatID(GetUserId(hContact));
	if (auto *si = Chat_Find(wszChatID, m_szModuleName)) {
		auto *pReq = new AsyncRapiRequest(this, "getChatInfo", &CIcqProto::OnGetChatInfo);
		pReq->params << WCHAR_PARAM("sn", wszChatID) << INT_PARAM("memberLimit", 100) << CHAR_PARAM("aimSid", m_aimsid);
		pReq->pUserInfo = si;
		Push(pReq);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// Invitation dialog

class CGroupchatInviteDlg : public CIcqDlgBase
{
	CCtrlClc m_clc;
	SESSION_INFO *m_si;

	void FilterList(CCtrlClc*)
	{
		for (auto &hContact : Contacts()) {
			char *proto = Proto_GetBaseAccountName(hContact);
			if (mir_strcmp(proto, m_proto->m_szModuleName) || m_proto->isChatRoom(hContact))
				if (HANDLE hItem = m_clc.FindContact(hContact))
					m_clc.DeleteItem(hItem);
		}
	}

	void ResetListOptions(CCtrlClc*)
	{
		m_clc.SetHideEmptyGroups(1);
		m_clc.SetHideOfflineRoot(1);
	}

public:
	CGroupchatInviteDlg(CIcqProto *ppro, SESSION_INFO *si) :
		CIcqDlgBase(ppro, IDD_GROUPCHAT_INVITE),
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
		CMStringW szMembers;
		for (auto &hContact : m_proto->AccContacts()) {
			if (m_proto->isChatRoom(hContact))
				continue;

			if (HANDLE hItem = m_clc.FindContact(hContact)) {
				if (m_clc.GetCheck(hItem)) {
					if (!szMembers.IsEmpty())
						szMembers.AppendChar(',');
					szMembers.Append(m_proto->GetUserId(hContact));
				}
			}
		}

		m_proto->Push(new AsyncHttpRequest(CONN_MAIN, REQUEST_GET, "/mchat/AddChat")
			<< AIMSID(m_proto) << WCHAR_PARAM("chat_id", m_si->ptszID) << WCHAR_PARAM("members", szMembers));
		return true;
	}
};

void CIcqProto::InviteUserToChat(SESSION_INFO *si)
{
	CGroupchatInviteDlg dlg(this, si);
	if (si->pDlg)
		dlg.SetParent(((CDlgBase*)si->pDlg)->GetHwnd());
	dlg.DoModal();
}

void CIcqProto::OnLeaveChat(MHttpResponse*, AsyncHttpRequest *pReq)
{
	db_delete_contact(INT_PTR(pReq->pUserInfo));
}

void CIcqProto::LeaveDestroyChat(SESSION_INFO *si)
{
	auto *pReq = new AsyncHttpRequest(CONN_MAIN, REQUEST_GET, "/buddylist/hideChat", &CIcqProto::OnLeaveChat)
		<< AIMSID(this) << WCHAR_PARAM("buddy", si->ptszID) << INT64_PARAM("lastMsgId", getId(si->hContact, DB_KEY_LASTMSGID));
	pReq->pUserInfo = (void *)si->hContact;
	Push(pReq);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Group chats

static gc_item sttLogListItems[] =
{
	{ LPGENW("&Invite a user"), IDM_INVITE, MENU_ITEM },
	{ nullptr, 0, MENU_SEPARATOR },
	{ LPGENW("&Leave/destroy chat"), IDM_LEAVE, MENU_ITEM }
};

int CIcqProto::GcMenuHook(WPARAM, LPARAM lParam)
{
	GCMENUITEMS *gcmi = (GCMENUITEMS*)lParam;
	if (gcmi == nullptr)
		return 0;

	if (mir_strcmpi(gcmi->pszModule, m_szModuleName))
		return 0;

	SESSION_INFO *si = Chat_Find(gcmi->pszID, gcmi->pszModule);
	if (si == nullptr)
		return 0;

	if (gcmi->Type == MENU_ON_LOG)
		Chat_AddMenuItems(gcmi->hMenu, _countof(sttLogListItems), sttLogListItems, &g_plugin);

	return 0;
}

int CIcqProto::GcEventHook(WPARAM, LPARAM lParam)
{
	GCHOOK *gch = (GCHOOK*)lParam;
	if (gch == nullptr)
		return 0;

	if (mir_strcmpi(gch->si->pszModule, m_szModuleName))
		return 0;

	SESSION_INFO *si = gch->si;

	switch (gch->iType) {
	case GC_USER_MESSAGE:
		rtrimw(gch->ptszText);
		if (!mir_wstrlen(gch->ptszText))
			break;

		if (m_bOnline) {
			wchar_t *wszText = NEWWSTR_ALLOCA(gch->ptszText);
			Chat_UnescapeTags(wszText);
			SendMsg(si->hContact, 0, T2Utf(wszText));
		}
		break;

	case GC_USER_PRIVMESS:
		GcSendPrivateMessage(gch);
		break;

	case GC_USER_LOGMENU:
		GcProcessLogMenu(si, gch->dwData);
		break;
	}

	return 1;
}

void CIcqProto::GcProcessLogMenu(SESSION_INFO *si, int iChoice)
{
	switch (iChoice) {
	case IDM_INVITE:
		InviteUserToChat(si);
		break;

	case IDM_LEAVE:
		LeaveDestroyChat(si);
		break;
	}
}

void CIcqProto::GcSendPrivateMessage(GCHOOK *gch)
{
	MCONTACT hContact;
	auto *pUser = FindUser(gch->ptszUID);
	if (pUser == nullptr) {
		hContact = CreateContact(gch->ptszUID, true);
		setWString(hContact, "Nick", gch->ptszNick);
		Contact::Hide(hContact);
		db_set_dw(hContact, "Ignore", "Mask1", 0);
	}
	else hContact = pUser->m_hContact;

	CallService(MS_MSG_SENDMESSAGE, hContact, 0);
}

void CIcqProto::ProcessGroupChat(const JSONNode &ev)
{
	for (auto &it : ev["mchats"]) {
		CMStringW wszId(it["sender"].as_mstring());
		auto *si = Chat_Find(wszId, m_szModuleName);
		if (si == nullptr)
			continue;

		CMStringW method(it["method"].as_mstring());
		GCEVENT gce = { si, (method == "add_members") ? GC_EVENT_JOIN : GC_EVENT_PART };

		int iStart = 0;
		CMStringW members(it["members"].as_mstring());
		while (true) {
			CMStringW member = members.Tokenize(L",", iStart);
			if (member.IsEmpty())
				break;

			auto *pUser = FindUser(member);
			if (pUser == nullptr)
				continue;

			gce.pszNick.w = Clist_GetContactDisplayName(pUser->m_hContact);
			gce.pszUID.w = member;
			gce.time = ::time(0);
			gce.bIsMe = member == m_szOwnId;
			Chat_Event(&gce);
		}
	}
}
