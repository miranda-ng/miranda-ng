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

#include "common.h"

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

int CSkypeProto::OnGroupChatEventHook(WPARAM, LPARAM lParam)
{
	GCHOOK *gch = (GCHOOK*)lParam;
	if (!gch)
	{
		return 1;
	}
	else if (strcmp(gch->pDest->pszModule, m_szModuleName) != 0)
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
			switch(gch->dwData)
			{
			case 10: {
				MCONTACT hContact = (MCONTACT)DialogBoxParam(g_hInstance, MAKEINTRESOURCE(IDD_GC_INVITE), NULL, InviteDlgProc, (LPARAM)this);
				if (hContact != NULL) 
				{
					ptrA username(db_get_sa(hContact, m_szModuleName, SKYPE_SETTINGS_ID));
					SendRequest(new InviteUserToChatRequest(RegToken, chat_id, username, "User", Server));
				}
				break;
			}
			case 20:
				//chat_leave(id);
				break;
			case 30:
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
				SendRequest(new KickUserRequest(RegToken, chat_id, user_id, Server));
				break;
			case 30:
				SendRequest(new InviteUserToChatRequest(RegToken, chat_id, user_id, "Admin", Server));
				break;
			case 40:
				SendRequest(new InviteUserToChatRequest(RegToken, chat_id, user_id, "User", Server));
				break;
			}

			break;

		}
	}
	return 0;
}
void CSkypeProto::StartChatRoom(const TCHAR *tid, const TCHAR *tname)
{
	// Create the group chat session
	GCSESSION gcw = { sizeof(gcw) };
	gcw.iType = GCW_PRIVMESS;
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

	// Add self contact
	//AddChatContact(tid, facy.self_.user_id.c_str(), facy.self_.real_name.c_str());
	CallServiceSync(MS_GC_EVENT, (hideChats ? WINDOW_HIDDEN : SESSION_INITDONE), reinterpret_cast<LPARAM>(&gce));
	CallServiceSync(MS_GC_EVENT, SESSION_ONLINE, reinterpret_cast<LPARAM>(&gce));

	//SendRequest(new GetChatInfoRequest(RegToken, ptrA(mir_t2a(tid)), Server), &CSkypeProto::OnGetChatInfo); 
}

int CSkypeProto::OnGroupChatMenuHook(WPARAM, LPARAM lParam)
{
	GCMENUITEMS *gcmi = (GCMENUITEMS*)lParam;
	if (stricmp(gcmi->pszModule, m_szModuleName)) return 0;

	if (gcmi->Type == MENU_ON_LOG)
	{
		static const struct gc_item Items[] =
		{
			{ LPGENT("&Invite user..."),		10, MENU_ITEM, FALSE },
			{ LPGENT("&Leave chat session"),	20, MENU_ITEM, FALSE },
			{ LPGENT("&Change topic"),			30, MENU_ITEM, FALSE }
		};
		gcmi->nItems = SIZEOF(Items);
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
				{ LPGENT("Kick &user"), 10, MENU_ITEM		},
				{ NULL,					 0, MENU_SEPARATOR	},
				{ LPGENT("Set &role"),	20, MENU_NEWPOPUP	},
				{ LPGENT("&Admin"),		30, MENU_POPUPITEM	},
				{ LPGENT("&User"),		40, MENU_POPUPITEM	}
			};
			gcmi->nItems = SIZEOF(Items);
			gcmi->Item = (gc_item*)Items;
		}
	}


	return 0;
}

INT_PTR CSkypeProto::OnJoinChatRoom(WPARAM hContact, LPARAM)
{
	if (hContact)
	{
		ptrT idT(getTStringA(hContact, "ChatRoomID"));
		StartChatRoom(idT, idT);
	}
	return 0;
}

INT_PTR CSkypeProto::OnLeaveChatRoom(WPARAM hContact, LPARAM)
{
	if (hContact)
	{
		ptrT idT(getTStringA(hContact, "ChatRoomID"));

		GCDEST gcd = { m_szModuleName, NULL, GC_EVENT_CONTROL };
		gcd.ptszID = idT;

		GCEVENT gce = { sizeof(gce), &gcd };
		gce.time = ::time(NULL);

		CallServiceSync(MS_GC_EVENT, SESSION_OFFLINE, reinterpret_cast<LPARAM>(&gce));
		CallServiceSync(MS_GC_EVENT, SESSION_TERMINATE, reinterpret_cast<LPARAM>(&gce));
	}
	return 0;
}

INT_PTR CSkypeProto::SvcDestroyChat(WPARAM hContact, LPARAM)
{
	debugLogA("CVkProto::SvcDestroyKickChat");
	if (!IsOnline())
		return 1;

	ptrA chatId(db_get_sa(hContact, m_szModuleName, SKYPE_SETTINGS_ID));

	SendRequest(new KickUserRequest(RegToken, chatId, SelfSkypeName, Server));

	CallService(MS_DB_CONTACT_DELETE, (WPARAM)hContact, 0);

	return 0;
}

/* CHAT EVENT */

void CSkypeProto::OnChatEvent(JSONNODE *node)
{
	ptrA clientMsgId(mir_t2a(ptrT(json_as_string(json_get(node, "clientmessageid")))));
	ptrA skypeEditedId(mir_t2a(ptrT(json_as_string(json_get(node, "skypeeditedid")))));
	
	ptrA fromLink(mir_t2a(ptrT(json_as_string(json_get(node, "from")))));
	ptrA from(ContactUrlToName(fromLink));

	time_t timestamp = IsoToUnixTime(ptrT(json_as_string(json_get(node, "composetime"))));

	ptrA content(mir_t2a(ptrT(json_as_string(json_get(node, "content")))));
	int emoteOffset = atoi(ptrA(mir_t2a(ptrT(json_as_string(json_get(node, "skypeemoteoffset"))))));

	ptrA conversationLink(mir_t2a(ptrT(json_as_string(json_get(node, "conversationLink")))));
	ptrA chatname(ChatUrlToName(conversationLink));

	ptrT topic(json_as_string(json_get(node, "threadtopic")));
	
	if (FindChatRoom(chatname) == NULL) StartChatRoom(_A2T(chatname), topic);
	
	ptrA messageType(mir_t2a(ptrT(json_as_string(json_get(node, "messagetype")))));
	if (!mir_strcmpi(messageType, "Text") || !mir_strcmpi(messageType, "RichText"))
	{
		AddMessageToChat(_A2T(chatname), _A2T(from), content, emoteOffset != NULL, emoteOffset, timestamp);
	}
	else if (!mir_strcmpi(messageType, "ThreadActivity/AddMember"))
	{
		ptrA xinitiator, xtarget, initiator, target;
		//content = <addmember><eventtime>1429186229164</eventtime><initiator>8:initiator</initiator><target>8:user</target></addmember>

		HXML xml = xi.parseString(ptrT(mir_a2t(content)), 0, _T("addmember"));
		if (xml != NULL) {

			HXML xmlNode = xi.getChildByPath(xml, _T("target"), 0);
			xtarget = xmlNode != NULL ? mir_t2a(xi.getText(xmlNode)) : NULL;

			xi.destroyNode(xml);
		}

		target = ParseUrl(xtarget, "8:");

		AddChatContact(_A2T(chatname), target, target, L"User");
	}
	else if (!mir_strcmpi(messageType, "ThreadActivity/DeleteMember"))
	{
		ptrA xinitiator, xtarget, initiator, target;
		//content = <addmember><eventtime>1429186229164</eventtime><initiator>8:initiator</initiator><target>8:user</target></addmember>

		HXML xml = xi.parseString(ptrT(mir_a2t(content)), 0, _T("deletemember"));
		if (xml != NULL) {
			HXML xmlNode = xi.getChildByPath(xml, _T("initiator"), 0);
			xinitiator = node != NULL ? mir_t2a(xi.getText(xmlNode)) : NULL;

			xmlNode = xi.getChildByPath(xml, _T("target"), 0);
			xtarget = xmlNode != NULL ? mir_t2a(xi.getText(xmlNode)) : NULL;

			xi.destroyNode(xml);
		}
		if(xtarget == NULL)
			return;

		target = ParseUrl(xtarget, "8:");

		bool isKick = false;
		initiator = ParseUrl(xinitiator, "8:");
		isKick = true;

		if (isKick)
		{
			RemoveChatContact(_A2T(chatname), target, target, true, initiator);
		}
		else
		{
			RemoveChatContact(_A2T(chatname), target, target);
		}
	}
	else if (!mir_strcmpi(messageType, "ThreadActivity/TopicUpdate"))
	{
		//content=<topicupdate><eventtime>1429532702130</eventtime><initiator>8:user</initiator><value>test topic</value></topicupdate>
		ptrA xinitiator, value, initiator;
		HXML xml = xi.parseString(ptrT(mir_a2t(content)), 0, _T("topicupdate"));
		if (xml != NULL) {
			HXML xmlNode = xi.getChildByPath(xml, _T("initiator"), 0);
			xinitiator = xmlNode != NULL ? mir_t2a(xi.getText(xmlNode)) : NULL;

			xmlNode = xi.getChildByPath(xml, _T("value"), 0);
			value = xmlNode != NULL ? mir_t2a(xi.getText(xmlNode)) : NULL;

			xi.destroyNode(xml);
		}
		initiator = ParseUrl(xinitiator, "8:");
		
		RenameChat(chatname, value);
		ChangeChatTopic(chatname, value, initiator);
	}
	else if (!mir_strcmpi(messageType, "ThreadActivity/RoleUpdate"))
	{
		//content=<roleupdate><eventtime>1429551258363</eventtime><initiator>8:user</initiator><target><id>8:user1</id><role>admin</role></target></roleupdate>
		ptrA xinitiator, xId, initiator, id, xRole;
		HXML xml = xi.parseString(ptrT(mir_a2t(content)), 0, _T("roleupdate"));
		if (xml != NULL) {
			HXML xmlNode = xi.getChildByPath(xml, _T("initiator"), 0);
			xinitiator = xmlNode != NULL ? mir_t2a(xi.getText(xmlNode)) : NULL;

			xmlNode = xi.getChildByPath(xml, _T("target"), 0);
			if (xmlNode != NULL)
			{
				HXML xmlId = xi.getChildByPath(xmlNode, _T("id"), 0);
				HXML xmlRole = xi.getChildByPath(xmlNode, _T("role"), 0);
				xId = xmlNode != NULL ? mir_t2a(xi.getText(xmlId)) : NULL;
				xRole = xmlNode != NULL ? mir_t2a(xi.getText(xmlRole)) : NULL;
			}
			xi.destroyNode(xml);

			initiator = ParseUrl(xinitiator, "8:");
			id = ParseUrl(xId, "8:");
			
			GCDEST gcd = { m_szModuleName, _A2T(chatname), !mir_strcmpi(xRole, "Admin") ? GC_EVENT_ADDSTATUS : GC_EVENT_REMOVESTATUS};
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
	ptrA szMessage(mir_t2a(tszMessage));
	if (strncmp(szMessage, "/me ", 4) == 0)
		SendRequest(new SendChatActionRequest(RegToken, szChatId, time(NULL), szMessage, Server));
	else 
		SendRequest(new SendChatMessageRequest(RegToken, szChatId, time(NULL), szMessage, Server));
}

void CSkypeProto::AddMessageToChat(const TCHAR *chat_id, const TCHAR *from, const char *content, bool isAction, int emoteOffset, time_t timestamp, bool isLoading)
{
	GCDEST gcd = { m_szModuleName, chat_id, isAction? GC_EVENT_ACTION : GC_EVENT_MESSAGE };
	GCEVENT gce = { sizeof(GCEVENT), &gcd };

	gce.bIsMe = IsMe(_T2A(from));
	gce.ptszNick = from;
	gce.time = timestamp;
	gce.ptszUID = from;
	ptrA szHtml(RemoveHtml(content));
	ptrT tszHtml(mir_a2t(szHtml));
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

	JSONROOT root(response->pData);
	JSONNODE *members = json_get(root, "members");
	JSONNODE *properties = json_get(root, "properties");

	if (json_empty(json_get(properties, "capabilities")))
		return;

	ptrA chatId(ChatUrlToName(mir_t2a(ptrT(json_as_string(json_get(root, "messages"))))));
	StartChatRoom(_A2T(chatId), mir_tstrdup(topic));
	for (size_t i = 0; i < json_size(members); i++)
	{
		JSONNODE *member = json_at(members, i);

		ptrA username(ContactUrlToName(ptrA(mir_t2a(ptrT(json_as_string(json_get(member, "userLink")))))));
		ptrT role(json_as_string(json_get(member, "role")));
		if (!IsChatContact(_A2T(chatId), username))
			AddChatContact(_A2T(chatId), username, username, role, true);
	}
	PushRequest(new GetHistoryRequest(RegToken, chatId, 15, true, 0, Server), &CSkypeProto::OnGetServerHistory);
	mir_free(topic);
}

void CSkypeProto::RenameChat(const char *chat_id, const char *name)
{
	ptrT tchat_id(mir_a2t(chat_id));
	ptrT tname(mir_a2t_cp(name, CP_UTF8));

	GCDEST gcd = { m_szModuleName, tchat_id, GC_EVENT_CHANGESESSIONAME };
	GCEVENT gce = { sizeof(gce), &gcd };
	gce.ptszText = tname;
	CallService(MS_GC_EVENT, 0, reinterpret_cast<LPARAM>(&gce));
}

void CSkypeProto::ChangeChatTopic(const char *chat_id, const char *topic, const char *initiator)
{
	ptrT tchat_id(mir_a2t(chat_id));
	ptrT tname(mir_a2t(initiator));
	ptrT ttopic(mir_a2t(topic));

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

	// mir_free(gci.pszUsers);
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
	gce.time = !isChange ? time(NULL): NULL;
	gce.bIsMe = IsMe(id);
	gce.ptszStatus = TranslateTS(role);

	CallServiceSync(MS_GC_EVENT, 0, reinterpret_cast<LPARAM>(&gce));
}

void CSkypeProto::RemoveChatContact(const TCHAR *tchat_id, const char *id, const char *name, bool isKick, const char *initiator)
{
	if(IsMe(id))
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
	if (!IsOnline())
		return 1;
	DialogBoxParam(g_hInstance, MAKEINTRESOURCE(IDD_GC_CREATE), NULL, GcCreateDlgProc, (LPARAM)this);
	return 0;
}


/* Dialogs */
INT_PTR CSkypeProto::GcCreateDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CSkypeProto *ppro = (CSkypeProto*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	NMCLISTCONTROL* nmc;

	switch (msg) 
	{
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);

		ppro = (CSkypeProto*)lParam;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
		{
			HWND hwndClist = GetDlgItem(hwndDlg, IDC_CLIST);
			SetWindowLongPtr(hwndClist, GWL_STYLE,
				GetWindowLongPtr(hwndClist, GWL_STYLE) | CLS_CHECKBOXES | CLS_HIDEEMPTYGROUPS | CLS_USEGROUPS | CLS_GREYALTERNATE | CLS_GROUPCHECKBOXES);
			SendMessage(hwndClist, CLM_SETEXSTYLE, CLS_EX_DISABLEDRAGDROP | CLS_EX_TRACKSELECT, 0);

			ResetOptions(hwndDlg);
		}
		return TRUE;

	case WM_NOTIFY:
		nmc = (NMCLISTCONTROL*)lParam;
		if (nmc->hdr.idFrom == IDC_CLIST && nmc->hdr.code == CLN_LISTREBUILT)
			FilterContacts(hwndDlg, ppro);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDCANCEL:
			EndDialog(hwndDlg, 0);
			return TRUE;

		case IDOK:
			HWND hwndClist = GetDlgItem(hwndDlg, IDC_CLIST);
			LIST<char>uids(1);
			for (MCONTACT hContact = db_find_first(ppro->m_szModuleName); hContact; hContact = db_find_next(hContact, ppro->m_szModuleName)) {
				if (ppro->isChatRoom(hContact))
					continue;

				if (int hItem = SendMessage(hwndClist, CLM_FINDCONTACT, hContact, 0)) {
					if (SendMessage(hwndClist, CLM_GETCHECKMARK, (WPARAM)hItem, 0)) {
						uids.insert(db_get_sa(hContact, ppro->m_szModuleName, SKYPE_SETTINGS_ID));
					}
				}
			}
			uids.insert(ppro->getStringA(SKYPE_SETTINGS_ID));

			TCHAR tszTitle[1024];
			GetDlgItemText(hwndDlg, IDC_TITLE, tszTitle, SIZEOF(tszTitle));
			ppro->SendRequest(new CreateChatroomRequest(ppro->RegToken, uids, ppro->getStringA(SKYPE_SETTINGS_ID), ppro->Server));
			for (int i = 0; i < uids.getCount(); i++)
				mir_free(uids[i]);
			uids.destroy();
			EndDialog(hwndDlg, 0);
			return TRUE;
		}
	}
	return FALSE;
}

INT_PTR CSkypeProto::InviteDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{
			CSkypeProto *ppro = (CSkypeProto*)lParam;
			HWND hwndCombo = GetDlgItem(hwndDlg, IDC_CONTACT);
			for (MCONTACT hContact = db_find_first(ppro->m_szModuleName); hContact; hContact = db_find_next(hContact, ppro->m_szModuleName)) {
				if (ppro->isChatRoom(hContact)) continue; 
				TCHAR *ptszNick = pcli->pfnGetContactDisplayName(hContact, 0);
				int idx = SendMessage(hwndCombo, CB_ADDSTRING, 0, LPARAM(ptszNick));
				SendMessage(hwndCombo, CB_SETITEMDATA, idx, hContact);
			}
			SendMessage(hwndCombo, CB_SETCURSEL, 0, 0);
		}
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDCANCEL:
			EndDialog(hwndDlg, 0);
			return TRUE;

		case IDOK:
			int idx = SendDlgItemMessage(hwndDlg, IDC_CONTACT, CB_GETCURSEL, 0, 0);
			if (idx != -1)
				EndDialog(hwndDlg, SendDlgItemMessage(hwndDlg, IDC_CONTACT, CB_GETITEMDATA, idx, 0));
			else
				EndDialog(hwndDlg, 0);
			return TRUE;
		}		
	}

	return 0;
}

void CSkypeProto::FilterContacts(HWND hwndDlg, CSkypeProto *ppro)
{
	HWND hwndClist = GetDlgItem(hwndDlg, IDC_CLIST);
	for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
		char *proto = GetContactProto(hContact);
		if (mir_strcmp(proto, ppro->m_szModuleName) || ppro->isChatRoom(hContact))
			if (HANDLE hItem = (HANDLE)SendMessage(hwndClist, CLM_FINDCONTACT, hContact, 0))
				SendMessage(hwndClist, CLM_DELETEITEM, (WPARAM)hItem, 0);
	}
}

void CSkypeProto::ResetOptions(HWND hwndDlg)
{
	HWND hwndClist = GetDlgItem(hwndDlg, IDC_CLIST);
	SendMessage(hwndClist, CLM_SETHIDEEMPTYGROUPS, 1, 0);
	SendMessage(hwndClist, CLM_GETHIDEOFFLINEROOT, 1, 0);
}