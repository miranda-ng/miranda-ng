/*
Copyright (c) 2015-17 Miranda NG project (https://miranda-ng.org)

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

void CSkypeProto::CloseAllChatChatSessions()
{
	GC_INFO gci = { 0 };
	gci.Flags = GCF_BYINDEX | GCF_ID;
	gci.pszModule = m_szModuleName;

	int count = pci->SM_GetCount(m_szModuleName);
	for (int i = 0; i < count; i++) {
		gci.iItem = i;
		if (!Chat_GetInfo(&gci)) {
			Chat_Control(m_szModuleName, gci.pszID, SESSION_OFFLINE);
			Chat_Terminate(m_szModuleName, gci.pszID);
		}
	}
}

MCONTACT CSkypeProto::FindChatRoom(const char *chatname)
{
	SESSION_INFO *si = pci->SM_FindSession(_A2T(chatname), m_szModuleName);
	return si ? si->hContact : 0;
}

void CSkypeProto::StartChatRoom(const wchar_t *tid, const wchar_t *tname)
{
	// Create the group chat session
	Chat_NewSession(GCW_CHATROOM, m_szModuleName, tid, tname);

	// Create a user statuses
	Chat_AddGroup(m_szModuleName, tid, TranslateT("Admin"));
	Chat_AddGroup(m_szModuleName, tid, TranslateT("User"));

	// Finish initialization
	Chat_Control(m_szModuleName, tid, (getBool("HideChats", 1) ? WINDOW_HIDDEN : SESSION_INITDONE));
	Chat_Control(m_szModuleName, tid, SESSION_ONLINE);
}

void CSkypeProto::OnLoadChats(const NETLIBHTTPREQUEST *response)
{
	if (response == NULL)
		return;

	JSONNode root = JSONNode::parse(response->pData);
	if (!root)
		return;

	const JSONNode &metadata = root["_metadata"];
	const JSONNode &conversations = root["conversations"].as_array();

	int totalCount = metadata["totalCount"].as_int();
	std::string syncState = metadata["syncState"].as_string();

	if (totalCount >= 99 || conversations.size() >= 99)
		PushRequest(new SyncHistoryFirstRequest(syncState.c_str(), li), &CSkypeProto::OnSyncHistory);

	for (size_t i = 0; i < conversations.size(); i++) {
		const JSONNode &conversation = conversations.at(i);
		const JSONNode &threadProperties = conversation["threadProperties"];
		const JSONNode &id = conversation["id"];

		if (!conversation["lastMessage"])
			continue;

		CMStringW topic(threadProperties["topic"].as_mstring());
		SendRequest(new GetChatInfoRequest(id.as_string().c_str(), li), &CSkypeProto::OnGetChatInfo, topic.Detach());
	}
}

/* Hooks */

int CSkypeProto::OnGroupChatEventHook(WPARAM, LPARAM lParam)
{
	GCHOOK *gch = (GCHOOK*)lParam;
	if (!gch)
		return 1;

	if (mir_strcmp(gch->pszModule, m_szModuleName) != 0)
		return 0;

	_T2A chat_id(gch->ptszID);

	switch (gch->iType) {
	case GC_USER_MESSAGE:
		OnSendChatMessage(gch->ptszID, gch->ptszText);
		break;

	case GC_USER_PRIVMESS:
		{
			MCONTACT hContact = FindContact(_T2A(gch->ptszUID));
			if (hContact == NULL) {
				hContact = AddContact(_T2A(gch->ptszUID), true);
				setWord(hContact, "Status", ID_STATUS_ONLINE);
				db_set_b(hContact, "CList", "Hidden", 1);
				setWString(hContact, "Nick", gch->ptszUID);
				db_set_dw(hContact, "Ignore", "Mask1", 0);
			}
			CallService(MS_MSG_SENDMESSAGEW, hContact, 0);
		}
		break;

	case GC_USER_LOGMENU:
		switch (gch->dwData) {
		case 10:
			{
				CSkypeInviteDlg dlg(this);
				{
					mir_cslock lck(m_InviteDialogsLock);
					m_InviteDialogs.insert(&dlg);
				}

				if (!dlg.DoModal())
					break;

				MCONTACT hContact = dlg.m_hContact;
				if (hContact != NULL)
					SendRequest(new InviteUserToChatRequest(chat_id, Contacts[hContact], "User", li));

				{
					mir_cslock lck(m_InviteDialogsLock);
					m_InviteDialogs.remove(&dlg);
				}
			}
			break;

		case 20:
			OnLeaveChatRoom(FindChatRoom(chat_id), NULL);
			break;

		case 30:
			CMStringW newTopic = ChangeTopicForm();
			if (!newTopic.IsEmpty())
				SendRequest(new SetChatPropertiesRequest(chat_id, "topic", T2Utf(newTopic.GetBuffer()), li));
			break;
		}
		break;

	case GC_USER_NICKLISTMENU:
		{
			_T2A user_id(gch->ptszUID);

			switch (gch->dwData) {
			case 10:
				SendRequest(new KickUserRequest(chat_id, user_id, li));
				break;
			case 30:
				SendRequest(new InviteUserToChatRequest(chat_id, user_id, "Admin", li));
				break;
			case 40:
				SendRequest(new InviteUserToChatRequest(chat_id, user_id, "User", li));
				break;
			case 50:
				ptrW tnick_old(GetChatContactNick(chat_id, _T2A(gch->ptszUID), _T2A(gch->ptszText)));

				ENTER_STRING pForm = { sizeof(pForm) };
				pForm.type = ESF_COMBO;
				pForm.recentCount = 0;
				pForm.caption = TranslateT("Enter new nickname");
				pForm.ptszInitVal = tnick_old;
				pForm.szModuleName = m_szModuleName;
				pForm.szDataPrefix = "renamenick_";

				if (EnterString(&pForm)) {
					MCONTACT hChatContact = FindChatRoom(chat_id);
					if (hChatContact == NULL)
						break; // This probably shouldn't happen, but if chat is NULL for some reason, do nothing

					ptrW tnick_new(pForm.ptszResult);
					bool reset = mir_wstrlen(tnick_new) == 0;
					if (reset) {
						// User fill blank name, which means we reset the custom nick
						db_unset(hChatContact, "UsersNicks", _T2A(gch->ptszUID));
						tnick_new = GetChatContactNick(chat_id, _T2A(gch->ptszUID), _T2A(gch->ptszText));
					}

					if (!mir_wstrcmp(tnick_old, tnick_new))
						break; // New nick is same, do nothing

					GCEVENT gce = { m_szModuleName, gch->ptszID, GC_EVENT_NICK };
					gce.ptszNick = tnick_old;
					gce.bIsMe = IsMe(user_id);
					gce.ptszUID = gch->ptszUID;
					gce.ptszText = tnick_new;
					gce.dwFlags = GCEF_ADDTOLOG;
					gce.time = time(NULL);
					Chat_Event(&gce);

					if (!reset)
						db_set_ws(hChatContact, "UsersNicks", _T2A(gch->ptszUID), tnick_new);
				}
				break;
			}
			break;
		}
	}
	return 0;
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

	if (hContact && IDYES == MessageBox(NULL, TranslateT("This chat is going to be destroyed forever with all its contents. This action cannot be undone. Are you sure?"), TranslateT("Warning"), MB_YESNO | MB_ICONQUESTION)) {
		ptrW idT(getWStringA(hContact, "ChatRoomID"));
		Chat_Control(m_szModuleName, idT, SESSION_OFFLINE);
		Chat_Terminate(m_szModuleName, idT);

		SendRequest(new KickUserRequest(_T2A(idT), li.szSkypename, li));

		db_delete_contact(hContact);
	}
	return 0;
}

/* CHAT EVENT */

void CSkypeProto::OnChatEvent(const JSONNode &node)
{
	//CMStringA szMessageId = node["clientmessageid"] ? node["clientmessageid"].as_string().c_str() : node["skypeeditedid"].as_string().c_str();
	CMStringA szConversationName(UrlToSkypename(node["conversationLink"].as_string().c_str()));
	CMStringA szFromSkypename(UrlToSkypename(node["from"].as_string().c_str()));

	CMStringW szTopic(node["threadtopic"].as_mstring());

	time_t timestamp = IsoToUnixTime(node["composetime"].as_string().c_str());

	std::string strContent = node["content"].as_string();
	int nEmoteOffset = node["skypeemoteoffset"].as_int();


	if (FindChatRoom(szConversationName) == NULL)
		SendRequest(new GetChatInfoRequest(szConversationName, li), &CSkypeProto::OnGetChatInfo, szTopic.Detach());

	std::string messageType = node["messagetype"].as_string();
	if (messageType == "Text" || messageType == "RichText") {
		ptrA szClearedContent(messageType == "RichText" ? RemoveHtml(strContent.c_str()) : mir_strdup(strContent.c_str()));
		AddMessageToChat(_A2T(szConversationName), _A2T(szFromSkypename), szClearedContent, nEmoteOffset != NULL, nEmoteOffset, timestamp);
	}
	else if (messageType == "ThreadActivity/AddMember") {
		ptrA xinitiator, xtarget, initiator;
		//content = <addmember><eventtime>1429186229164</eventtime><initiator>8:initiator</initiator><target>8:user</target></addmember>

		HXML xml = xmlParseString(ptrW(mir_utf8decodeW(strContent.c_str())), 0, L"addmember");
		if (xml == NULL)
			return;

		for (int i = 0; i < xmlGetChildCount(xml); i++) {
			HXML xmlNode = xmlGetNthChild(xml, L"target", i);
			if (xmlNode == NULL)
				break;

			xtarget = mir_u2a(xmlGetText(xmlNode));

			CMStringA target = ParseUrl(xtarget, "8:");
			AddChatContact(_A2T(szConversationName), target, target, L"User");
		}
		xmlDestroyNode(xml);
	}
	else if (messageType == "ThreadActivity/DeleteMember") {
		ptrA xinitiator, xtarget;
		//content = <addmember><eventtime>1429186229164</eventtime><initiator>8:initiator</initiator><target>8:user</target></addmember>

		HXML xml = xmlParseString(ptrW(mir_utf8decodeW(strContent.c_str())), 0, L"deletemember");
		if (xml != NULL) {
			HXML xmlNode = xmlGetChildByPath(xml, L"initiator", 0);
			xinitiator = node != NULL ? mir_u2a(xmlGetText(xmlNode)) : NULL;

			xmlNode = xmlGetChildByPath(xml, L"target", 0);
			xtarget = xmlNode != NULL ? mir_u2a(xmlGetText(xmlNode)) : NULL;

			xmlDestroyNode(xml);
		}
		if (xtarget == NULL)
			return;

		CMStringA target = ParseUrl(xtarget, "8:");
		CMStringA initiator = ParseUrl(xinitiator, "8:");
		RemoveChatContact(_A2T(szConversationName), target, target, true, initiator);

	}
	else if (messageType == "ThreadActivity/TopicUpdate") {
		//content=<topicupdate><eventtime>1429532702130</eventtime><initiator>8:user</initiator><value>test topic</value></topicupdate>
		ptrA xinitiator, value;
		HXML xml = xmlParseString(ptrW(mir_utf8decodeW(strContent.c_str())), 0, L"topicupdate");
		if (xml != NULL) {
			HXML xmlNode = xmlGetChildByPath(xml, L"initiator", 0);
			xinitiator = xmlNode != NULL ? mir_u2a(xmlGetText(xmlNode)) : NULL;

			xmlNode = xmlGetChildByPath(xml, L"value", 0);
			value = xmlNode != NULL ? mir_u2a(xmlGetText(xmlNode)) : NULL;

			xmlDestroyNode(xml);
		}

		CMStringA initiator = ParseUrl(xinitiator, "8:");
		RenameChat(szConversationName, value);
		ChangeChatTopic(szConversationName, value, initiator);
	}
	else if (messageType == "ThreadActivity/RoleUpdate") {
		//content=<roleupdate><eventtime>1429551258363</eventtime><initiator>8:user</initiator><target><id>8:user1</id><role>admin</role></target></roleupdate>
		ptrA xinitiator, xId, xRole;
		HXML xml = xmlParseString(ptrW(mir_utf8decodeW(strContent.c_str())), 0, L"roleupdate");
		if (xml != NULL) {
			HXML xmlNode = xmlGetChildByPath(xml, L"initiator", 0);
			xinitiator = xmlNode != NULL ? mir_u2a(xmlGetText(xmlNode)) : NULL;

			xmlNode = xmlGetChildByPath(xml, L"target", 0);
			if (xmlNode != NULL) {
				HXML xmlId = xmlGetChildByPath(xmlNode, L"id", 0);
				HXML xmlRole = xmlGetChildByPath(xmlNode, L"role", 0);
				xId = xmlId != NULL ? mir_u2a(xmlGetText(xmlId)) : NULL;
				xRole = xmlRole != NULL ? mir_u2a(xmlGetText(xmlRole)) : NULL;
			}
			xmlDestroyNode(xml);

			CMStringA initiator = ParseUrl(xinitiator, "8:");
			CMStringA id = ParseUrl(xId, "8:");
			ptrW tszId(mir_a2u(id));
			ptrW tszRole(mir_a2u(xRole));
			ptrW tszInitiator(mir_a2u(initiator));

			GCEVENT gce = { m_szModuleName, _A2T(szConversationName), !mir_strcmpi(xRole, "Admin") ? GC_EVENT_ADDSTATUS : GC_EVENT_REMOVESTATUS };
			gce.dwFlags = GCEF_ADDTOLOG;
			gce.ptszNick = tszId;
			gce.ptszUID = tszId;
			gce.ptszText = tszInitiator;
			gce.time = time(NULL);
			gce.bIsMe = IsMe(id);
			gce.ptszStatus = TranslateT("Admin");
			Chat_Event(&gce);
		}
	}
}

void CSkypeProto::OnSendChatMessage(const wchar_t *chat_id, const wchar_t * tszMessage)
{
	if (!IsOnline())
		return;

	wchar_t *buf = NEWWSTR_ALLOCA(tszMessage);
	rtrimw(buf);
	Chat_UnescapeTags(buf);

	ptrA szChatId(mir_u2a(chat_id));
	ptrA szMessage(mir_utf8encodeW(buf));

	if (strncmp(szMessage, "/me ", 4) == 0)
		SendRequest(new SendChatActionRequest(szChatId, time(NULL), szMessage, li));
	else
		SendRequest(new SendChatMessageRequest(szChatId, time(NULL), szMessage, li));
}

void CSkypeProto::AddMessageToChat(const wchar_t *chat_id, const wchar_t *from, const char *content, bool isAction, int emoteOffset, time_t timestamp, bool isLoading)
{
	ptrW tnick(GetChatContactNick(_T2A(chat_id), _T2A(from), _T2A(from)));

	GCEVENT gce = { m_szModuleName, chat_id, isAction ? GC_EVENT_ACTION : GC_EVENT_MESSAGE };
	gce.bIsMe = IsMe(_T2A(from));
	gce.ptszNick = tnick;
	gce.time = timestamp;
	gce.ptszUID = from;

	CMStringW tszText(ptrW(mir_utf8decodeW(content)));
	tszText.Replace(L"%", L"%%");

	if (!isAction) {
		gce.ptszText = tszText;
		gce.dwFlags = GCEF_ADDTOLOG;
	}
	else gce.ptszText = &(tszText.GetBuffer())[emoteOffset];

	if (isLoading)
		gce.dwFlags |= GCEF_NOTNOTIFY;

	Chat_Event(&gce);
}

void CSkypeProto::OnGetChatInfo(const NETLIBHTTPREQUEST *response, void *p)
{
	ptrW topic((wchar_t*)p); // memory must be freed in any case
	if (response == NULL || response->pData == NULL)
		return;

	JSONNode root = JSONNode::parse(response->pData);
	if (!root)
		return;

	const JSONNode &members = root["members"];
	const JSONNode &properties = root["properties"];
	if (!properties["capabilities"] || properties["capabilities"].empty())
		return;

	CMStringA chatId(UrlToSkypename(root["messages"].as_string().c_str()));
	StartChatRoom(_A2T(chatId), topic);
	for (size_t i = 0; i < members.size(); i++) {
		const JSONNode &member = members.at(i);

		CMStringA username(UrlToSkypename(member["userLink"].as_string().c_str()));
		std::string role = member["role"].as_string();
		if (!IsChatContact(_A2T(chatId), username))
			AddChatContact(_A2T(chatId), username, username, _A2T(role.c_str()), true);
	}
	PushRequest(new GetHistoryRequest(chatId, 15, true, 0, li), &CSkypeProto::OnGetServerHistory);
}

void CSkypeProto::RenameChat(const char *chat_id, const char *name)
{
	ptrW tchat_id(mir_a2u(chat_id));
	ptrW tname(mir_utf8decodeW(name));
	Chat_ChangeSessionName(m_szModuleName, tchat_id, tname);
}

void CSkypeProto::ChangeChatTopic(const char *chat_id, const char *topic, const char *initiator)
{
	ptrW tchat_id(mir_a2u(chat_id));
	ptrW tname(mir_a2u(initiator));
	ptrW ttopic(mir_utf8decodeW(topic));

	GCEVENT gce = { m_szModuleName, tchat_id, GC_EVENT_TOPIC };
	gce.ptszUID = tname;
	gce.ptszNick = tname;
	gce.ptszText = ttopic;
	Chat_Event(&gce);
}

bool CSkypeProto::IsChatContact(const wchar_t *chat_id, const char *id)
{
	ptrA users(GetChatUsers(chat_id));
	return (users != NULL && strstr(users, id) != NULL);
}

char *CSkypeProto::GetChatUsers(const wchar_t *chat_id)
{
	GC_INFO gci = { 0 };
	gci.Flags = GCF_USERS;
	gci.pszModule = m_szModuleName;
	gci.pszID = chat_id;
	Chat_GetInfo(&gci);
	return gci.pszUsers;
}

wchar_t* CSkypeProto::GetChatContactNick(const char *chat_id, const char *id, const char *name)
{
	// Check if there is custom nick for this chat contact
	if (chat_id != NULL) {
		if (wchar_t *tname = db_get_wsa(FindChatRoom(chat_id), "UsersNicks", id))
			return tname;
	}

	// Check if we have this contact in database
	if (IsMe(id)) {
		// Return my nick
		if (wchar_t *tname = getWStringA(NULL, "Nick"))
			return tname;
	}
	else {
		MCONTACT hContact = FindContact(id);
		if (hContact != NULL) {
			// Primarily return custom name
			if (wchar_t *tname = db_get_wsa(hContact, "CList", "MyHandle"))
				return tname;

			// If not exists, then user nick
			if (wchar_t *tname = getWStringA(hContact, "Nick"))
				return tname;
		}
	}

	// Return default value as nick - given name or user id
	if (name != NULL)
		return mir_a2u_cp(name, CP_UTF8);
	else
		return mir_a2u(id);
}

void CSkypeProto::AddChatContact(const wchar_t *tchat_id, const char *id, const char *name, const wchar_t *role, bool isChange)
{
	if (IsChatContact(tchat_id, id))
		return;

	ptrW tnick(GetChatContactNick(_T2A(tchat_id), id, name));
	ptrW tid(mir_a2u(id));

	GCEVENT gce = { m_szModuleName, tchat_id, GC_EVENT_JOIN };
	gce.dwFlags = GCEF_ADDTOLOG;
	gce.ptszNick = tnick;
	gce.ptszUID = tid;
	gce.time = !isChange ? time(NULL) : NULL;
	gce.bIsMe = IsMe(id);
	gce.ptszStatus = TranslateW(role);

	Chat_Event(&gce);
}

void CSkypeProto::RemoveChatContact(const wchar_t *tchat_id, const char *id, const char *name, bool isKick, const char *initiator)
{
	if (IsMe(id))
		return;

	ptrW tnick(GetChatContactNick(_T2A(tchat_id), id, name));
	ptrW tinitiator(GetChatContactNick(_T2A(tchat_id), initiator, initiator));
	ptrW tid(mir_a2u(id));

	GCEVENT gce = { m_szModuleName, tchat_id, isKick ? GC_EVENT_KICK : GC_EVENT_PART };
	if (isKick) {
		gce.ptszUID = tid;
		gce.ptszNick = tnick;
		gce.ptszStatus = tinitiator;
		gce.time = time(NULL);
	}
	else {
		gce.dwFlags = GCEF_ADDTOLOG;
		gce.ptszNick = tnick;
		gce.ptszUID = tid;
		gce.time = time(NULL);
		gce.bIsMe = IsMe(id);
	}

	Chat_Event(&gce);
}

INT_PTR CSkypeProto::SvcCreateChat(WPARAM, LPARAM)
{
	if (IsOnline()) {
		CSkypeGCCreateDlg dlg(this);

		{ mir_cslock lck(m_GCCreateDialogsLock); m_GCCreateDialogs.insert(&dlg); }

		if (!dlg.DoModal()) { return 1; }

		SendRequest(new CreateChatroomRequest(dlg.m_ContactsList, li));

		{ mir_cslock lck(m_GCCreateDialogsLock); m_GCCreateDialogs.remove(&dlg); }
		return 0;
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
		Chat_AddMenuItems(gcmi->hMenu, _countof(Items), Items);
	}
	else if (gcmi->Type == MENU_ON_NICKLIST) {
		static const struct gc_item Items[] =
		{
			{ LPGENW("Kick &user"),     10, MENU_ITEM      },
			{ NULL,                     0,  MENU_SEPARATOR },
			{ LPGENW("Set &role"),      20, MENU_NEWPOPUP  },
			{ LPGENW("&Admin"),         30, MENU_POPUPITEM },
			{ LPGENW("&User"),          40, MENU_POPUPITEM },
			{ LPGENW("Change nick..."), 50, MENU_ITEM },
		};
		Chat_AddMenuItems(gcmi->hMenu, _countof(Items), Items);
	}

	return 0;
}

CMStringW CSkypeProto::ChangeTopicForm()
{
	CMStringW caption(FORMAT, L"[%s] %s", _A2T(m_szModuleName), TranslateT("Enter new chatroom topic"));
	ENTER_STRING pForm = { sizeof(pForm) };
	pForm.type = ESF_MULTILINE;
	pForm.caption = caption;
	pForm.ptszInitVal = NULL;
	pForm.szModuleName = m_szModuleName;
	return (!EnterString(&pForm)) ? CMStringW() : CMStringW(ptrW(pForm.ptszResult));
}
