/*
Copyright (c) 2015-22 Miranda NG team (https://miranda-ng.org)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

void CSkypeProto::InitGroupChatModule()
{
	GCREGISTER gcr = {};
	gcr.iMaxText = 0;
	gcr.ptszDispName = m_tszUserName;
	gcr.pszModule = m_szModuleName;
	Chat_Register(&gcr);

	HookProtoEvent(ME_GC_EVENT, &CSkypeProto::OnGroupChatEventHook);
	HookProtoEvent(ME_GC_BUILDMENU, &CSkypeProto::OnGroupChatMenuHook);

	CreateProtoService(PS_JOINCHAT, &CSkypeProto::OnJoinChatRoom);
	CreateProtoService(PS_LEAVECHAT, &CSkypeProto::OnLeaveChatRoom);
}

SESSION_INFO* CSkypeProto::StartChatRoom(const wchar_t *tid, const wchar_t *tname)
{
	// Create the group chat session
	SESSION_INFO *si = Chat_NewSession(GCW_CHATROOM, m_szModuleName, tid, tname);
	if (!si)
		return nullptr;

	// Create a user statuses
	Chat_AddGroup(si, TranslateT("Admin"));
	Chat_AddGroup(si, TranslateT("User"));

	// Finish initialization
	Chat_Control(m_szModuleName, tid, (getBool("HideChats", 1) ? WINDOW_HIDDEN : SESSION_INITDONE));
	Chat_Control(m_szModuleName, tid, SESSION_ONLINE);

	PushRequest(new GetChatInfoRequest(tid));
	return si;
}

void CSkypeProto::OnLoadChats(NETLIBHTTPREQUEST *response, AsyncHttpRequest*)
{
	JsonReply reply(response);
	if (reply.error())
		return;

	auto &root = reply.data();
	const JSONNode &metadata = root["_metadata"];
	const JSONNode &conversations = root["conversations"].as_array();

	int totalCount = metadata["totalCount"].as_int();
	std::string syncState = metadata["syncState"].as_string();

	if (totalCount >= 99 || conversations.size() >= 99)
		ReadHistoryRest(syncState.c_str());

	for (auto &it : conversations) {
		auto &props = it["threadProperties"];
		if (it["lastMessage"] && props["members"] && !props["lastleaveat"])
			StartChatRoom(it["id"].as_mstring(), props["topic"].as_mstring());
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// Group chat invitation dialog

class CSkypeInviteDlg : public CSkypeDlgBase
{
	CCtrlCombo m_combo;

public:
	MCONTACT m_hContact = 0;

	CSkypeInviteDlg(CSkypeProto *proto) :
		CSkypeDlgBase(proto, IDD_GC_INVITE),
		m_combo(this, IDC_CONTACT)
	{}

	bool OnInitDialog() override
	{
		for (auto &hContact : m_proto->AccContacts())
			if (!m_proto->isChatRoom(hContact))
				m_combo.AddString(Clist_GetContactDisplayName(hContact), hContact);
		return true;
	}

	bool OnApply() override
	{
		m_hContact = m_combo.GetCurData();
		return true;
	}
};

int CSkypeProto::OnGroupChatEventHook(WPARAM, LPARAM lParam)
{
	GCHOOK *gch = (GCHOOK*)lParam;
	if (!gch)
		return 0;

	auto *si = gch->si;
	if (mir_strcmp(si->pszModule, m_szModuleName) != 0)
		return 0;

	T2Utf chat_id(si->ptszID), user_id(gch->ptszUID);

	switch (gch->iType) {
	case GC_USER_MESSAGE:
		SendChatMessage(si, gch->ptszText);
		break;

	case GC_USER_PRIVMESS:
		{
			MCONTACT hContact = FindContact(user_id);
			if (hContact == NULL) {
				hContact = AddContact(user_id, T2Utf(gch->ptszNick), true);
				setWord(hContact, "Status", ID_STATUS_ONLINE);
				Contact::Hide(hContact);
			}
			CallService(MS_MSG_SENDMESSAGEW, hContact, 0);
		}
		break;

	case GC_USER_LOGMENU:
		switch (gch->dwData) {
		case 10:
			{
				CSkypeInviteDlg dlg(this);
				if (dlg.DoModal())
					if (dlg.m_hContact != NULL)
						PushRequest(new InviteUserToChatRequest(chat_id, getId(dlg.m_hContact), "User"));
			}
			break;

		case 20:
			OnLeaveChatRoom(si->hContact, NULL);
			break;

		case 30:
			CMStringW newTopic = ChangeTopicForm();
			if (!newTopic.IsEmpty())
				PushRequest(new SetChatPropertiesRequest(chat_id, "topic", T2Utf(newTopic.GetBuffer())));
			break;
		}
		break;

	case GC_USER_NICKLISTMENU:
		switch (gch->dwData) {
		case 10:
			PushRequest(new KickUserRequest(chat_id, user_id));
			break;
		case 30:
			PushRequest(new InviteUserToChatRequest(chat_id, user_id, "Admin"));
			break;
		case 40:
			PushRequest(new InviteUserToChatRequest(chat_id, user_id, "User"));
			break;
		case 50:
			ptrW tnick_old(GetChatContactNick(si->hContact, gch->ptszUID, gch->ptszText));

			ENTER_STRING pForm = {};
			pForm.type = ESF_COMBO;
			pForm.caption = TranslateT("Enter new nickname");
			pForm.szModuleName = m_szModuleName;
			pForm.szDataPrefix = "renamenick_";

			if (EnterString(&pForm)) {
				if (si->hContact == NULL)
					break; // This probably shouldn't happen, but if chat is NULL for some reason, do nothing

				ptrW tnick_new(pForm.ptszResult);
				bool reset = mir_wstrlen(tnick_new) == 0;
				if (reset) {
					// User fill blank name, which means we reset the custom nick
					db_unset(si->hContact, "UsersNicks", user_id);
					tnick_new = GetChatContactNick(si->hContact, gch->ptszUID, gch->ptszText);
				}

				if (!mir_wstrcmp(tnick_old, tnick_new))
					break; // New nick is same, do nothing

				GCEVENT gce = { m_szModuleName, 0, GC_EVENT_NICK };
				gce.pszID.w = si->ptszID;
				gce.dwFlags = GCEF_ADDTOLOG;
				gce.pszNick.w = tnick_old;
				gce.bIsMe = IsMe(user_id);
				gce.pszUID.w = gch->ptszUID;
				gce.pszText.w= tnick_new;
				gce.time = time(0);
				Chat_Event(&gce);

				if (!reset)
					db_set_ws(si->hContact, "UsersNicks", user_id, tnick_new);
			}
			break;
		}
		break;
	}
	return 1;
}

INT_PTR CSkypeProto::OnJoinChatRoom(WPARAM hContact, LPARAM)
{
	if (hContact) {
		ptrW idT(getWStringA(hContact, "ChatRoomID"));
		ptrW nameT(getWStringA(hContact, "Nick"));
		StartChatRoom(idT, nameT != NULL ? nameT : idT);
	}
	return 0;
}

INT_PTR CSkypeProto::OnLeaveChatRoom(WPARAM hContact, LPARAM)
{
	if (!IsOnline())
		return 1;

	if (hContact && IDYES == MessageBox(nullptr, TranslateT("This chat is going to be destroyed forever with all its contents. This action cannot be undone. Are you sure?"), TranslateT("Warning"), MB_YESNO | MB_ICONQUESTION)) {
		ptrW idT(getWStringA(hContact, "ChatRoomID"));
		Chat_Control(m_szModuleName, idT, SESSION_OFFLINE);
		Chat_Terminate(m_szModuleName, idT);

		PushRequest(new KickUserRequest(_T2A(idT), m_szSkypename));

		db_delete_contact(hContact);
	}
	return 0;
}

/* CHAT EVENT */

void CSkypeProto::OnChatEvent(const JSONNode &node)
{
	CMStringW wszChatId(UrlToSkypeId(node["conversationLink"].as_mstring()));
	CMStringW szFromId(UrlToSkypeId(node["from"].as_mstring()));

	CMStringW wszTopic(node["threadtopic"].as_mstring());
	CMStringW wszContent(node["content"].as_mstring());

	time_t timestamp = IsoToUnixTime(node["composetime"].as_string());

	int nEmoteOffset = node["skypeemoteoffset"].as_int();

	SESSION_INFO *si = g_chatApi.SM_FindSession(wszChatId, m_szModuleName);
	if (si == nullptr) {
		si = StartChatRoom(wszChatId, wszTopic);
		if (si == nullptr) {
			debugLogW(L"unable to create chat %s", wszChatId.c_str());
			return;
		}
	}

	std::string messageType = node["messagetype"].as_string();
	if (messageType == "Text" || messageType == "RichText") {
		CMStringW wszClearedContent(messageType == "RichText" ? RemoveHtml(wszContent) : wszContent);
		AddMessageToChat(si, szFromId, wszClearedContent, nEmoteOffset != NULL, nEmoteOffset, timestamp);
	}
	else if (messageType == "ThreadActivity/AddMember") {
		// <addmember><eventtime>1429186229164</eventtime><initiator>8:initiator</initiator><target>8:user</target></addmember>
		TiXmlDocument doc;
		if (0 != doc.Parse(T2Utf(wszContent)))
			return;

		if (auto *pRoot = doc.FirstChildElement("addMember")) {
			CMStringW target = Utf2T(XmlGetChildText(pRoot, "target"));
			AddChatContact(si, target, L"User");
		}
	}
	else if (messageType == "ThreadActivity/DeleteMember") {
		// <deletemember><eventtime>1429186229164</eventtime><initiator>8:initiator</initiator><target>8:user</target></deletemember>
		TiXmlDocument doc;
		if (0 != doc.Parse(T2Utf(wszContent)))
			return;

		if (auto *pRoot = doc.FirstChildElement("deletemember")) {
			CMStringW target = Utf2T(UrlToSkypeId(XmlGetChildText(pRoot, "target")));
			CMStringW initiator = Utf2T(XmlGetChildText(pRoot, "initiator"));
			RemoveChatContact(si, target, true, initiator);
		}
	}
	else if (messageType == "ThreadActivity/TopicUpdate") {
		// <topicupdate><eventtime>1429532702130</eventtime><initiator>8:user</initiator><value>test topic</value></topicupdate>
		TiXmlDocument doc;
		if (0 != doc.Parse(T2Utf(wszContent)))
			return;

		auto *pRoot = doc.FirstChildElement("topicupdate");
		if (pRoot) {
			CMStringW initiator = Utf2T(XmlGetChildText(pRoot, "initiator"));
			CMStringW value = Utf2T(XmlGetChildText(pRoot, "value"));
			Chat_ChangeSessionName(m_szModuleName, wszChatId, value);

			GCEVENT gce = { m_szModuleName, 0, GC_EVENT_TOPIC };
			gce.pszID.w = wszChatId;
			gce.pszUID.w = initiator;
			gce.pszNick.w = GetSkypeNick(initiator);
			gce.pszText.w = wszTopic;
			Chat_Event(&gce);
		}
	}
	else if (messageType == "ThreadActivity/RoleUpdate") {
		// <roleupdate><eventtime>1429551258363</eventtime><initiator>8:user</initiator><target><id>8:user1</id><role>admin</role></target></roleupdate>
		TiXmlDocument doc;
		if (0 != doc.Parse(T2Utf(wszContent)))
			return;

		auto *pRoot = doc.FirstChildElement("roleupdate");
		if (pRoot) {
			CMStringW initiator = Utf2T(UrlToSkypeId(XmlGetChildText(pRoot, "initiator")));

			auto *pTarget = pRoot->FirstChildElement("target");
			if (pTarget) {
				CMStringW id = Utf2T(UrlToSkypeId(XmlGetChildText(pTarget, "id")));
				const char *role = XmlGetChildText(pTarget, "role");

				GCEVENT gce = { m_szModuleName, 0, !mir_strcmpi(role, "Admin") ? GC_EVENT_ADDSTATUS : GC_EVENT_REMOVESTATUS };
				gce.pszID.w = wszChatId;
				gce.dwFlags = GCEF_ADDTOLOG;
				gce.pszNick.w = id;
				gce.pszUID.w = id;
				gce.pszText.w = initiator;
				gce.time = time(0);
				gce.bIsMe = IsMe(T2Utf(id));
				gce.pszStatus.w = TranslateT("Admin");
				Chat_Event(&gce);
			}
		}
	}
}

void CSkypeProto::SendChatMessage(SESSION_INFO *si, const wchar_t *tszMessage)
{
	if (!IsOnline())
		return;

	wchar_t *buf = NEWWSTR_ALLOCA(tszMessage);
	rtrimw(buf);
	Chat_UnescapeTags(buf);

	T2Utf chat_id(si->ptszID);
	ptrA szMessage(mir_utf8encodeW(buf));

	if (strncmp(szMessage, "/me ", 4) == 0)
		PushRequest(new SendChatActionRequest(chat_id, time(0), szMessage));
	else
		PushRequest(new SendChatMessageRequest(chat_id, time(0), szMessage));
}

void CSkypeProto::AddMessageToChat(SESSION_INFO *si, const wchar_t *from, const wchar_t *content, bool isAction, int emoteOffset, time_t timestamp, bool isLoading)
{
	ptrW tnick(GetChatContactNick(si->hContact, from));

	GCEVENT gce = { m_szModuleName, 0, isAction ? GC_EVENT_ACTION : GC_EVENT_MESSAGE };
	gce.pszID.w = si->ptszID;
	gce.bIsMe = IsMe(from);
	gce.pszNick.w = tnick;
	gce.time = timestamp;
	gce.pszUID.w = from;

	CMStringW wszText(content);
	wszText.Replace(L"%", L"%%");

	if (!isAction) {
		gce.pszText.w = wszText;
		gce.dwFlags |= GCEF_ADDTOLOG;
	}
	else gce.pszText.w = wszText.c_str() + emoteOffset;

	if (isLoading)
		gce.dwFlags |= GCEF_NOTNOTIFY;

	Chat_Event(&gce);
}

void CSkypeProto::OnGetChatInfo(NETLIBHTTPREQUEST *response, AsyncHttpRequest*)
{
	JsonReply reply(response);
	if (reply.error())
		return;

	auto &root = reply.data();
	const JSONNode &properties = root["properties"];
	if (!properties["capabilities"] || properties["capabilities"].empty())
		return;

	CMStringW wszChatId(UrlToSkypeId(root["messages"].as_mstring()));
	auto *si = g_chatApi.SM_FindSession(wszChatId, m_szModuleName);
	if (si == nullptr)
		return;

	for (auto &member : root["members"]) {
		CMStringW username(UrlToSkypeId(member["userLink"].as_mstring()));
		CMStringW role = member["role"].as_mstring();
		AddChatContact(si, username, role, true);
	}
	
	PushRequest(new GetHistoryRequest(T2Utf(si->ptszID), 100, 0, true));
}

wchar_t* CSkypeProto::GetChatContactNick(MCONTACT hContact, const wchar_t *id, const wchar_t *name)
{
	// Check if there is custom nick for this chat contact
	if (hContact)
		if (auto *tname = db_get_wsa(hContact, "UsersNicks", T2Utf(id)))
			return tname;

	// Check if we have this contact in database
	if (IsMe(id)) {
		// Return my nick
		if (auto *tname = getWStringA("Nick"))
			return tname;
	}
	else {
		hContact = FindContact(id);
		if (hContact != NULL) {
			// Primarily return custom name
			if (auto *tname = db_get_wsa(hContact, "CList", "MyHandle"))
				return tname;

			// If not exists, then user nick
			if (auto *tname = getWStringA(hContact, "Nick"))
				return tname;
		}
	}

	// Return default value as nick - given name or user id
	if (name != nullptr)
		return mir_wstrdup(name);
	return mir_wstrdup(GetSkypeNick(id));
}

void CSkypeProto::AddChatContact(SESSION_INFO *si, const wchar_t *id, const wchar_t *role, bool isChange)
{
	ptrW szNick(GetChatContactNick(si->hContact, id));

	GCEVENT gce = { m_szModuleName, 0, GC_EVENT_JOIN };
	gce.pszID.w = si->ptszID;
	gce.dwFlags = GCEF_ADDTOLOG;
	gce.pszNick.w = szNick;
	gce.pszUID.w = id;
	gce.time = !isChange ? time(0) : NULL;
	gce.bIsMe = IsMe(id);
	gce.pszStatus.w = TranslateW(role);
	Chat_Event(&gce);
}

void CSkypeProto::RemoveChatContact(SESSION_INFO *si, const wchar_t *id, bool isKick, const wchar_t *initiator)
{
	if (IsMe(id))
		return;

	ptrW szNick(GetChatContactNick(si->hContact, id));
	ptrW szInitiator(GetChatContactNick(si->hContact, initiator));
	
	GCEVENT gce = { m_szModuleName, 0, isKick ? GC_EVENT_KICK : GC_EVENT_PART };
	gce.pszID.w = si->ptszID;
	gce.pszNick.w = szNick;
	gce.pszUID.w = id;
	gce.time = time(0);
	if (isKick)
		gce.pszStatus.w = szInitiator;
	else {
		gce.dwFlags += GCEF_ADDTOLOG;
		gce.bIsMe = IsMe(id);
	}

	Chat_Event(&gce);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Group chat creation dialog

class CSkypeGCCreateDlg : public CSkypeDlgBase
{
	CCtrlClc m_clc;

public:
	LIST<char> m_ContactsList;

	CSkypeGCCreateDlg(CSkypeProto *proto) :
		CSkypeDlgBase(proto, IDD_GC_CREATE),
		m_clc(this, IDC_CLIST),
		m_ContactsList(1)
	{
		m_clc.OnListRebuilt = Callback(this, &CSkypeGCCreateDlg::FilterList);
	}

	~CSkypeGCCreateDlg()
	{
		CSkypeProto::FreeList(m_ContactsList);
		m_ContactsList.destroy();
	}

	bool OnInitDialog() override
	{
		SetWindowLongPtr(m_clc.GetHwnd(), GWL_STYLE,
			GetWindowLongPtr(m_clc.GetHwnd(), GWL_STYLE) | CLS_CHECKBOXES | CLS_HIDEEMPTYGROUPS | CLS_USEGROUPS | CLS_GREYALTERNATE);
		m_clc.SendMsg(CLM_SETEXSTYLE, CLS_EX_DISABLEDRAGDROP | CLS_EX_TRACKSELECT, 0);

		ResetListOptions(&m_clc);
		return true;
	}

	bool OnApply() override
	{
		for (auto &hContact : m_proto->AccContacts()) {
			if (!m_proto->isChatRoom(hContact))
				if (HANDLE hItem = m_clc.FindContact(hContact))
					if (m_clc.GetCheck(hItem))
						m_ContactsList.insert(m_proto->getId(hContact).Detach());
		}

		m_ContactsList.insert(m_proto->m_szSkypename.GetBuffer());
		return true;
	}

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
};

INT_PTR CSkypeProto::SvcCreateChat(WPARAM, LPARAM)
{
	if (IsOnline()) {
		CSkypeGCCreateDlg dlg(this);
		if (dlg.DoModal()) {
			PushRequest(new CreateChatroomRequest(dlg.m_ContactsList, this));
			return 0;
		}
	}
	return 1;
}

/* Menus */

int CSkypeProto::OnGroupChatMenuHook(WPARAM, LPARAM lParam)
{
	GCMENUITEMS *gcmi = (GCMENUITEMS*)lParam;
	if (mir_strcmpi(gcmi->pszModule, m_szModuleName)) return 0;

	if (gcmi->Type == MENU_ON_LOG) {
		static const struct gc_item Items[] =
		{
			{ LPGENW("&Invite user..."),     10, MENU_ITEM, FALSE },
			{ LPGENW("&Leave chat session"), 20, MENU_ITEM, FALSE },
			{ LPGENW("&Change topic..."),    30, MENU_ITEM, FALSE }
		};
		Chat_AddMenuItems(gcmi->hMenu, _countof(Items), Items, &g_plugin);
	}
	else if (gcmi->Type == MENU_ON_NICKLIST) {
		static const struct gc_item Items[] =
		{
			{ LPGENW("Kick &user"),     10, MENU_ITEM      },
			{ nullptr,                     0,  MENU_SEPARATOR },
			{ LPGENW("Set &role"),      20, MENU_NEWPOPUP  },
			{ LPGENW("&Admin"),         30, MENU_POPUPITEM },
			{ LPGENW("&User"),          40, MENU_POPUPITEM },
			{ LPGENW("Change nick..."), 50, MENU_ITEM },
		};
		Chat_AddMenuItems(gcmi->hMenu, _countof(Items), Items, &g_plugin);
	}

	return 0;
}

CMStringW CSkypeProto::ChangeTopicForm()
{
	CMStringW caption(FORMAT, L"[%s] %s", _A2T(m_szModuleName).get(), TranslateT("Enter new chatroom topic"));
	ENTER_STRING pForm = {};
	pForm.type = ESF_MULTILINE;
	pForm.caption = caption;
	pForm.szModuleName = m_szModuleName;
	return (!EnterString(&pForm)) ? CMStringW() : CMStringW(ptrW(pForm.ptszResult));
}
