/*
Copyright (c) 2015 Miranda NG project (http://miranda-ng.org)

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
	GCREGISTER gcr = { sizeof(gcr) };
	gcr.iMaxText = 0;
	gcr.ptszDispName = m_tszUserName;
	gcr.pszModule = m_szModuleName;
	CallServiceSync(MS_GC_REGISTER, 0, (LPARAM)&gcr);

	HookProtoEvent(ME_GC_EVENT, &CSkypeProto::OnGroupChatEventHook);
	HookProtoEvent(ME_GC_BUILDMENU, &CSkypeProto::OnGroupChatMenuHook);

	CreateProtoService(PS_JOINCHAT, &CSkypeProto::OnJoinChatRoom);
	CreateProtoService(PS_LEAVECHAT, &CSkypeProto::OnLeaveChatRoom);
}

void CSkypeProto::CloseAllChatChatSessions()
{
	GC_INFO gci = { 0 };
	gci.Flags = GCF_BYINDEX | GCF_ID | GCF_DATA;
	gci.pszModule = m_szModuleName;

	int count = CallServiceSync(MS_GC_GETSESSIONCOUNT, 0, (LPARAM)m_szModuleName);
	for (int i = 0; i < count; i++)
	{
		gci.iItem = i;
		if (!CallServiceSync(MS_GC_GETINFO, 0, (LPARAM)&gci))
		{
			GCDEST gcd = { m_szModuleName, gci.pszID, GC_EVENT_CONTROL };
			GCEVENT gce = { sizeof(gce), &gcd };
			CallServiceSync(MS_GC_EVENT, SESSION_OFFLINE, (LPARAM)&gce);
			CallServiceSync(MS_GC_EVENT, SESSION_TERMINATE, (LPARAM)&gce);
		}
	}
}

MCONTACT CSkypeProto::FindChatRoom(const char *chatname)
{
	MCONTACT hContact = NULL;
	for (hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName))
	{
		if (!isChatRoom(hContact))
			continue;

		ptrA cChatname(getStringA(hContact, "ChatRoomID"));
		if (!mir_strcmpi(chatname, cChatname))
			break;
	}
	return hContact;
}

void CSkypeProto::StartChatRoom(const TCHAR *tid, const TCHAR *tname)
{
	// Create the group chat session
	GCSESSION gcw = { sizeof(gcw) };
	gcw.iType = GCW_CHATROOM;
	gcw.ptszID = tid;
	gcw.pszModule = m_szModuleName;
	gcw.ptszName = tname;
	CallServiceSync(MS_GC_NEWSESSION, 0, (LPARAM)&gcw);

	// Send setting events
	GCDEST gcd = { m_szModuleName, tid, GC_EVENT_ADDGROUP };
	GCEVENT gce = { sizeof(gce), &gcd };

	// Create a user statuses
	gce.ptszStatus = TranslateT("Admin");
	CallServiceSync(MS_GC_EVENT, NULL, reinterpret_cast<LPARAM>(&gce));
	gce.ptszStatus = TranslateT("User");
	CallServiceSync(MS_GC_EVENT, NULL, reinterpret_cast<LPARAM>(&gce));

	// Finish initialization
	gcd.iType = GC_EVENT_CONTROL;
	gce.time = time(NULL);
	gce.pDest = &gcd;

	bool hideChats = getBool("HideChats", 1);

	CallServiceSync(MS_GC_EVENT, (hideChats ? WINDOW_HIDDEN : SESSION_INITDONE), reinterpret_cast<LPARAM>(&gce));
	CallServiceSync(MS_GC_EVENT, SESSION_ONLINE, reinterpret_cast<LPARAM>(&gce));
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

	for (size_t i = 0; i < conversations.size(); i++)
	{
		const JSONNode &conversation = conversations.at(i);
		const JSONNode &lastMessage = conversation["lastMessage"];
		const JSONNode &threadProperties = conversation["threadProperties"];
		if (!lastMessage)
			continue;

		std::string conversationLink = lastMessage["conversationLink"].as_string();
		if (conversationLink.find("/19:") != -1)
		{
			CMStringA skypename(UrlToSkypename(conversationLink.c_str()));
			CMString topic(threadProperties["topic"].as_mstring());
			SendRequest(new GetChatInfoRequest(skypename, li), &CSkypeProto::OnGetChatInfo, topic.Detach());
		}
	}
}

/* Hooks */

int CSkypeProto::OnGroupChatEventHook(WPARAM, LPARAM lParam)
{
	GCHOOK *gch = (GCHOOK*)lParam;
	if (!gch)
	{
		return 1;
	}
	else if (mir_strcmp(gch->pDest->pszModule, m_szModuleName) != 0)
	{
		return 0;
	}

	ptrA chat_id(mir_t2a(gch->pDest->ptszID));

	switch (gch->pDest->iType)
	{
	case GC_USER_MESSAGE:
	{
		OnSendChatMessage(gch->pDest->ptszID, gch->ptszText);
		break;
	}

	case GC_USER_PRIVMESS:
	{
		MCONTACT hContact = FindContact(_T2A(gch->ptszUID));
		if (hContact == NULL)
		{
			hContact = AddContact(_T2A(gch->ptszUID), true);
			setWord(hContact, "Status", ID_STATUS_ONLINE);
			db_set_b(hContact, "CList", "Hidden", 1);
			setTString(hContact, "Nick", gch->ptszUID);
			db_set_dw(hContact, "Ignore", "Mask1", 0);
		}
		CallService(MS_MSG_SENDMESSAGET, hContact, 0);
		break;
	}

	case GC_USER_LOGMENU:
	{
		switch (gch->dwData)
		{
		case 10: {
			CSkypeInviteDlg dlg(this);

			{ mir_cslock lck(m_InviteDialogsLock); m_InviteDialogs.insert(&dlg); }

			if(!dlg.DoModal()) break;
			MCONTACT hContact = dlg.m_hContact;
			if (hContact != NULL)
			{
				ptrA username(db_get_sa(hContact, m_szModuleName, SKYPE_SETTINGS_ID));
				SendRequest(new InviteUserToChatRequest(chat_id, username, "User", li));
			}

			{ mir_cslock lck(m_InviteDialogsLock); m_InviteDialogs.remove(&dlg); }

			break;
		}
		case 20:
			OnLeaveChatRoom(FindChatRoom(chat_id), NULL);
			break;
		case 30:
			CMString newTopic = ChangeTopicForm();
			if (!newTopic.IsEmpty())
				SendRequest(new SetChatPropertiesRequest(chat_id, "topic", ptrA(mir_utf8encodeT(newTopic.GetBuffer())), li));
			break;
		}
		break;
	}

	case GC_USER_NICKLISTMENU:
	{
		ptrA user_id;
		if (gch->dwData == 10 || gch->dwData == 30 || gch->dwData == 40)
		{
			user_id = mir_t2a_cp(gch->ptszUID, CP_UTF8);
		}

		switch (gch->dwData)
		{
		case 10:
			SendRequest(new KickUserRequest(chat_id, user_id, li));
			break;
		case 30:
			SendRequest(new InviteUserToChatRequest(chat_id, user_id, "Admin", li));
			break;
		case 40:
			SendRequest(new InviteUserToChatRequest(chat_id, user_id, "User", li));
			break;
		}

		break;
	}
	}
	return 0;
}

INT_PTR CSkypeProto::OnJoinChatRoom(WPARAM hContact, LPARAM)
{
	if (hContact)
	{
		ptrT idT(getTStringA(hContact, "ChatRoomID"));
		ptrT nameT(getTStringA(hContact, "Nick"));
		StartChatRoom(idT, nameT != NULL ? nameT : idT);
	}
	return 0;
}

INT_PTR CSkypeProto::OnLeaveChatRoom(WPARAM hContact, LPARAM)
{
	if (!IsOnline())
		return 1;
	if (hContact && IDYES == MessageBox(NULL, TranslateT("This chat is going to be destroyed forever with all its contents. This action cannot be undone. Are you sure?"), TranslateT("Warning"), MB_YESNO | MB_ICONQUESTION))
	{
		ptrT idT(getTStringA(hContact, "ChatRoomID"));

		GCDEST gcd = { m_szModuleName, NULL, GC_EVENT_CONTROL };
		gcd.ptszID = idT;

		GCEVENT gce = { sizeof(gce), &gcd };
		gce.time = ::time(NULL);

		CallServiceSync(MS_GC_EVENT, SESSION_OFFLINE, reinterpret_cast<LPARAM>(&gce));
		CallServiceSync(MS_GC_EVENT, SESSION_TERMINATE, reinterpret_cast<LPARAM>(&gce));

		SendRequest(new KickUserRequest(_T2A(idT), li.szSkypename, li));

		CallService(MS_DB_CONTACT_DELETE, (WPARAM)hContact, 0);
	}
	return 0;
}

/* CHAT EVENT */

void CSkypeProto::OnChatEvent(const JSONNode &node)
{
	//CMStringA szMessageId = node["clientmessageid"] ? node["clientmessageid"].as_string().c_str() : node["skypeeditedid"].as_string().c_str();
	CMStringA szConversationName(UrlToSkypename(node["conversationLink"].as_string().c_str()));
	CMStringA szFromSkypename(UrlToSkypename(node["from"].as_string().c_str()));

	CMString szTopic(node["threadtopic"].as_mstring());

	time_t timestamp = IsoToUnixTime(node["composetime"].as_string().c_str());

	std::string strContent = node["content"].as_string();
	int nEmoteOffset = node["skypeemoteoffset"].as_int();

	
	if (FindChatRoom(szConversationName) == NULL)
		SendRequest(new GetChatInfoRequest(szConversationName, li), &CSkypeProto::OnGetChatInfo, szTopic.Detach());

	std::string messageType = node["messagetype"].as_string();
	if (messageType == "Text" || messageType == "RichText")
	{
		AddMessageToChat(_A2T(szConversationName), _A2T(szFromSkypename), strContent.c_str(), nEmoteOffset != NULL, nEmoteOffset, timestamp);
	}
	else if (messageType == "ThreadActivity/AddMember")
	{
		ptrA xinitiator, xtarget, initiator;
		//content = <addmember><eventtime>1429186229164</eventtime><initiator>8:initiator</initiator><target>8:user</target></addmember>

		HXML xml = xmlParseString(ptrT(mir_utf8decodeT(strContent.c_str())), 0, _T("addmember"));
		if (xml == NULL)
			return;

		for (int i = 0; i < xmlGetChildCount(xml); i++)
		{
			HXML xmlNode = xmlGetNthChild(xml, L"target", i);
			if (xmlNode == NULL)
				break;

			xtarget = mir_t2a(xmlGetText(xmlNode));

			CMStringA target = ParseUrl(xtarget, "8:");
			AddChatContact(_A2T(szConversationName), target, target, L"User");
		}
		xmlDestroyNode(xml);
	}
	else if (messageType == "ThreadActivity/DeleteMember")
	{
		ptrA xinitiator, xtarget;
		//content = <addmember><eventtime>1429186229164</eventtime><initiator>8:initiator</initiator><target>8:user</target></addmember>

		HXML xml = xmlParseString(ptrT(mir_utf8decodeT(strContent.c_str())), 0, _T("deletemember"));
		if (xml != NULL) {
			HXML xmlNode = xmlGetChildByPath(xml, _T("initiator"), 0);
			xinitiator = node != NULL ? mir_t2a(xmlGetText(xmlNode)) : NULL;

			xmlNode = xmlGetChildByPath(xml, _T("target"), 0);
			xtarget = xmlNode != NULL ? mir_t2a(xmlGetText(xmlNode)) : NULL;

			xmlDestroyNode(xml);
		}
		if (xtarget == NULL)
			return;

		CMStringA target = ParseUrl(xtarget, "8:");
		CMStringA initiator = ParseUrl(xinitiator, "8:");
		RemoveChatContact(_A2T(szConversationName), target, target, true, initiator);

	}
	else if (messageType == "ThreadActivity/TopicUpdate")
	{
		//content=<topicupdate><eventtime>1429532702130</eventtime><initiator>8:user</initiator><value>test topic</value></topicupdate>
		ptrA xinitiator, value;
		HXML xml = xmlParseString(ptrT(mir_utf8decodeT(strContent.c_str())), 0, _T("topicupdate"));
		if (xml != NULL) {
			HXML xmlNode = xmlGetChildByPath(xml, _T("initiator"), 0);
			xinitiator = xmlNode != NULL ? mir_t2a(xmlGetText(xmlNode)) : NULL;

			xmlNode = xmlGetChildByPath(xml, _T("value"), 0);
			value = xmlNode != NULL ? mir_t2a(xmlGetText(xmlNode)) : NULL;

			xmlDestroyNode(xml);
		}

		CMStringA initiator = ParseUrl(xinitiator, "8:");
		RenameChat(szConversationName, value);
		ChangeChatTopic(szConversationName, value, initiator);
	}
	else if (messageType == "ThreadActivity/RoleUpdate")
	{
		//content=<roleupdate><eventtime>1429551258363</eventtime><initiator>8:user</initiator><target><id>8:user1</id><role>admin</role></target></roleupdate>
		ptrA xinitiator, xId, xRole;
		HXML xml = xmlParseString(ptrT(mir_utf8decodeT(strContent.c_str())), 0, _T("roleupdate"));
		if (xml != NULL) {
			HXML xmlNode = xmlGetChildByPath(xml, _T("initiator"), 0);
			xinitiator = xmlNode != NULL ? mir_t2a(xmlGetText(xmlNode)) : NULL;

			xmlNode = xmlGetChildByPath(xml, _T("target"), 0);
			if (xmlNode != NULL)
			{
				HXML xmlId = xmlGetChildByPath(xmlNode, _T("id"), 0);
				HXML xmlRole = xmlGetChildByPath(xmlNode, _T("role"), 0);
				xId = xmlId != NULL ? mir_t2a(xmlGetText(xmlId)) : NULL;
				xRole = xmlRole != NULL ? mir_t2a(xmlGetText(xmlRole)) : NULL;
			}
			xmlDestroyNode(xml);
			
			CMStringA initiator = ParseUrl(xinitiator, "8:");
			CMStringA id = ParseUrl(xId, "8:");

			GCDEST gcd = { m_szModuleName, _A2T(szConversationName), !mir_strcmpi(xRole, "Admin") ? GC_EVENT_ADDSTATUS : GC_EVENT_REMOVESTATUS };
			GCEVENT gce = { sizeof(gce), &gcd };
			ptrT tszId(mir_a2t(id));
			ptrT tszRole(mir_a2t(xRole));
			ptrT tszInitiator(mir_a2t(initiator));
			gce.pDest = &gcd;
			gce.dwFlags = GCEF_ADDTOLOG;
			gce.ptszNick = tszId;
			gce.ptszUID = tszId;
			gce.ptszText = tszInitiator;
			gce.time = time(NULL);
			gce.bIsMe = IsMe(id);
			gce.ptszStatus = TranslateT("Admin");
			CallServiceSync(MS_GC_EVENT, 0, (LPARAM)&gce);
		}
	}
}

void CSkypeProto::OnSendChatMessage(const TCHAR *chat_id, const TCHAR * tszMessage)
{
	if (!IsOnline())
		return;
	ptrA szChatId(mir_t2a(chat_id));
	ptrA szMessage(mir_utf8encodeT(tszMessage));
	if (strncmp(szMessage, "/me ", 4) == 0)
		SendRequest(new SendChatActionRequest(szChatId, GenerateMessageId(), szMessage, li));
	else
		SendRequest(new SendChatMessageRequest(szChatId, GenerateMessageId(), szMessage, li));
}

void CSkypeProto::AddMessageToChat(const TCHAR *chat_id, const TCHAR *from, const char *content, bool isAction, int emoteOffset, time_t timestamp, bool isLoading)
{
	GCDEST gcd = { m_szModuleName, chat_id, isAction ? GC_EVENT_ACTION : GC_EVENT_MESSAGE };
	GCEVENT gce = { sizeof(GCEVENT), &gcd };

	gce.bIsMe = IsMe(_T2A(from));
	gce.ptszNick = from;
	gce.time = timestamp;
	gce.ptszUID = from;
	ptrA szHtml(RemoveHtml(content));
	ptrT tszHtml(mir_utf8decodeT(szHtml));
	if (!isAction)
	{
		gce.ptszText = tszHtml;
		gce.dwFlags = GCEF_ADDTOLOG;
	}
	else
	{
		gce.ptszText = &tszHtml[emoteOffset];
	}

	if (isLoading) gce.dwFlags = GCEF_NOTNOTIFY;

	CallServiceSync(MS_GC_EVENT, 0, (LPARAM)&gce);
}

void CSkypeProto::OnGetChatInfo(const NETLIBHTTPREQUEST *response, void *p)
{
	TCHAR *topic = (TCHAR*)p;
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
	for (size_t i = 0; i < members.size(); i++)
	{
		const JSONNode &member = members.at(i);

		CMStringA username(UrlToSkypename(member["userLink"].as_string().c_str()));
		std::string role = member["role"].as_string();
		if (!IsChatContact(_A2T(chatId), username))
			AddChatContact(_A2T(chatId), username, username, _A2T(role.c_str()), true);
	}
	PushRequest(new GetHistoryRequest(chatId, 15, true, 0, li), &CSkypeProto::OnGetServerHistory);
	mir_free(topic);
}

void CSkypeProto::RenameChat(const char *chat_id, const char *name)
{
	ptrT tchat_id(mir_a2t(chat_id));
	ptrT tname(mir_utf8decodeT(name));

	GCDEST gcd = { m_szModuleName, tchat_id, GC_EVENT_CHANGESESSIONAME };
	GCEVENT gce = { sizeof(gce), &gcd };
	gce.ptszText = tname;
	CallService(MS_GC_EVENT, 0, reinterpret_cast<LPARAM>(&gce));
}

void CSkypeProto::ChangeChatTopic(const char *chat_id, const char *topic, const char *initiator)
{
	ptrT tchat_id(mir_a2t(chat_id));
	ptrT tname(mir_a2t(initiator));
	ptrT ttopic(mir_utf8decodeT(topic));

	GCDEST gcd = { m_szModuleName, tchat_id, GC_EVENT_TOPIC };
	GCEVENT gce = { sizeof(gce), &gcd };
	gce.ptszUID = tname;
	gce.ptszNick = tname;
	gce.ptszText = ttopic;
	CallService(MS_GC_EVENT, 0, reinterpret_cast<LPARAM>(&gce));
}

bool CSkypeProto::IsChatContact(const TCHAR *chat_id, const char *id)
{
	ptrA users(GetChatUsers(chat_id));
	return (users != NULL && strstr(users, id) != NULL);
}

char *CSkypeProto::GetChatUsers(const TCHAR *chat_id)
{
	GC_INFO gci = { 0 };
	gci.Flags = GCF_USERS;
	gci.pszModule = m_szModuleName;
	gci.pszID = chat_id;
	CallService(MS_GC_GETINFO, 0, (LPARAM)&gci);
	return gci.pszUsers;
}

void CSkypeProto::AddChatContact(const TCHAR *tchat_id, const char *id, const char *name, const TCHAR *role, bool isChange)
{
	if (IsChatContact(tchat_id, id))
		return;

	ptrT tnick(mir_a2t_cp(name, CP_UTF8));
	ptrT tid(mir_a2t(id));

	GCDEST gcd = { m_szModuleName, tchat_id, GC_EVENT_JOIN };
	GCEVENT gce = { sizeof(gce), &gcd };
	gce.pDest = &gcd;
	gce.dwFlags = GCEF_ADDTOLOG;
	gce.ptszNick = tnick;
	gce.ptszUID = tid;
	gce.time = !isChange ? time(NULL) : NULL;
	gce.bIsMe = IsMe(id);
	gce.ptszStatus = TranslateTS(role);

	CallServiceSync(MS_GC_EVENT, 0, reinterpret_cast<LPARAM>(&gce));
}

void CSkypeProto::RemoveChatContact(const TCHAR *tchat_id, const char *id, const char *name, bool isKick, const char *initiator)
{
	if (IsMe(id))
		return;

	ptrT tnick(mir_a2t_cp(name, CP_UTF8));
	ptrT tid(mir_a2t(id));
	ptrT tinitiator(mir_a2t(initiator));

	GCDEST gcd = { m_szModuleName, tchat_id, isKick ? GC_EVENT_KICK : GC_EVENT_PART };
	GCEVENT gce = { sizeof(gce), &gcd };
	if (isKick)
	{
		gce.ptszUID = tid;
		gce.ptszNick = tnick;
		gce.ptszStatus = tinitiator;
		gce.time = time(NULL);
	}
	else
	{
		gce.dwFlags = GCEF_ADDTOLOG;
		gce.ptszNick = tnick;
		gce.ptszUID = tid;
		gce.time = time(NULL);
		gce.bIsMe = IsMe(id);
	}

	CallServiceSync(MS_GC_EVENT, 0, reinterpret_cast<LPARAM>(&gce));
}

INT_PTR CSkypeProto::SvcCreateChat(WPARAM, LPARAM)
{
	if (IsOnline())
	{
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

	if (gcmi->Type == MENU_ON_LOG)
	{
		static const struct gc_item Items[] =
		{
			{ LPGENT("&Invite user..."),     10, MENU_ITEM, FALSE },
			{ LPGENT("&Leave chat session"), 20, MENU_ITEM, FALSE },
			{ LPGENT("&Change topic"),       30, MENU_ITEM, FALSE }
		};
		gcmi->nItems = _countof(Items);
		gcmi->Item = (gc_item*)Items;
	}
	else if (gcmi->Type == MENU_ON_NICKLIST)
	{
		if (IsMe(_T2A(gcmi->pszUID)))
		{
			gcmi->nItems = 0;
			gcmi->Item = NULL;
		}
		else
		{
			static const struct gc_item Items[] =
			{
				{ LPGENT("Kick &user"),  10, MENU_ITEM      },
				{ NULL,                  0,  MENU_SEPARATOR },
				{ LPGENT("Set &role"),   20, MENU_NEWPOPUP  },
				{ LPGENT("&Admin"),      30, MENU_POPUPITEM },
				{ LPGENT("&User"),       40, MENU_POPUPITEM }
			};
			gcmi->nItems = _countof(Items);
			gcmi->Item = (gc_item*)Items;
		}
	}


	return 0;
}

CMString CSkypeProto::ChangeTopicForm()
{
	CMString caption(FORMAT, _T("[%s] %s"), _A2T(m_szModuleName), TranslateT("Enter new chatroom topic"));
	ENTER_STRING pForm = { sizeof(pForm) };
	pForm.type = ESF_MULTILINE;
	pForm.caption = caption;
	pForm.ptszInitVal = NULL;
	pForm.szModuleName = m_szModuleName;
	return (!EnterString(&pForm)) ? CMString() : CMString(ptrT(pForm.ptszResult));
}