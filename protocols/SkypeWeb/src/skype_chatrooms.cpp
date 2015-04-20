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

static LPCTSTR sttStatuses[] = { LPGENT("User"), LPGENT("Admin") };

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
			ptrA msg(mir_t2a(gch->ptszText));

			if (IsOnline()) {
				debugLogA("  > Chat - Outgoing message");
				SendRequest(new SendChatMessageRequest(RegToken, chat_id, time(NULL), msg, Server));
			}

			break;
		}

		case GC_USER_PRIVMESS:
		{
			/*facebook_user fbu;
			fbu.user_id = _T2A(hook->ptszUID, CP_UTF8);

			// Find this contact in list or add new temporary contact
			MCONTACT hContact = AddToContactList(&fbu, CONTACT_NONE, false, true);

			if (!hContact)
				break;

			CallService(MS_MSG_SENDMESSAGET, hContact, 0);*/
			break;
		}

		case GC_USER_LOGMENU:
		{
			switch(gch->dwData)
			{
			case 10:
				MCONTACT hContact;
				hContact = (MCONTACT)DialogBoxParam(g_hInstance, MAKEINTRESOURCE(IDD_GC_INVITE), NULL, InviteDlgProc, (LPARAM)this);
				if (hContact != NULL) 
				{
					ptrA username(db_get_sa(hContact, m_szModuleName, SKYPE_SETTINGS_ID));
					SendRequest(new InviteUserToChatRequest(RegToken, chat_id, username, "User", Server));
				}
				break;

			case 20:
				//chat_leave(id);
				break;
			}
			break;
		}

		case GC_USER_NICKLISTMENU:
		{
			ptrA user_id;
			if (gch->dwData == 10 || gch->dwData == 20) 
			{
				user_id = mir_t2a_cp(gch->ptszUID, CP_UTF8);
			}

			switch (gch->dwData)
			{
			case 10:
				SendRequest(new KickUserRequest(RegToken, chat_id, user_id, Server));
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

	SendRequest(new GetChatInfoRequest(RegToken, ptrA(mir_t2a(tid)), Server), &CSkypeProto::OnGetChatInfo); 
}

int CSkypeProto::OnGroupChatMenuHook(WPARAM, LPARAM lParam)
{
	GCMENUITEMS *gcmi = (GCMENUITEMS*)lParam;
	if (stricmp(gcmi->pszModule, m_szModuleName)) return 0;

	if (gcmi->Type == MENU_ON_LOG)
	{
		static const struct gc_item Items[] =
		{
			{ LPGENT("&Invite user..."), 10, MENU_ITEM, FALSE },
			{ LPGENT("&Leave chat session"), 20, MENU_ITEM, FALSE }
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
				{ LPGENT("Kick &user"), 10, MENU_ITEM, FALSE }
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
	}
	return 0;
}

INT_PTR CSkypeProto::OnLeaveChatRoom(WPARAM hContact, LPARAM)
{
	if (hContact)
	{
		ptrT idT(hContact ? getTStringA(hContact, "ChatRoomID") : NULL);

		GCDEST gcd = { m_szModuleName, NULL, GC_EVENT_CONTROL };
		gcd.ptszID = idT;

		GCEVENT gce = { sizeof(gce), &gcd };
		gce.time = ::time(NULL);

		CallServiceSync(MS_GC_EVENT, SESSION_OFFLINE, reinterpret_cast<LPARAM>(&gce));
		CallServiceSync(MS_GC_EVENT, SESSION_TERMINATE, reinterpret_cast<LPARAM>(&gce));
	}
	return 0;
}

/* CHAT EVENT */

void CSkypeProto::OnChatEvent(JSONNODE *node)
{
	ptrA clientMsgId(mir_t2a(ptrT(json_as_string(json_get(node, "clientmessageid")))));
	ptrA skypeEditedId(mir_t2a(ptrT(json_as_string(json_get(node, "skypeeditedid")))));
	
	ptrA from(mir_t2a(ptrT(json_as_string(json_get(node, "from")))));

	time_t timestamp = IsoToUnixTime(ptrT(json_as_string(json_get(node, "composetime"))));

	ptrA content(mir_t2a(ptrT(json_as_string(json_get(node, "content")))));
	//int emoteOffset = json_as_int(json_get(node, "skypeemoteoffset"));

	ptrA conversationLink(mir_t2a(ptrT(json_as_string(json_get(node, "conversationLink")))));
	ptrA chatname(ChatUrlToName(conversationLink));

	ptrT topic(json_as_string(json_get(node, "threadtopic")));
	
	//StartChatRoom(_A2T(chatname), topic);
	
	ptrA messageType(mir_t2a(ptrT(json_as_string(json_get(node, "messagetype")))));
	if (!mir_strcmpi(messageType, "Text") || !mir_strcmpi(messageType, "RichText"))
	{
		GCDEST gcd = { m_szModuleName, ptrT(mir_a2t(chatname)), GC_EVENT_MESSAGE };
		GCEVENT gce = { sizeof(GCEVENT), &gcd };
		gce.bIsMe = IsMe(ContactUrlToName(from));
		gce.ptszUID = mir_a2t(ContactUrlToName(from));
		gce.time = timestamp;
		gce.ptszNick = mir_a2t(ContactUrlToName(from));
		gce.ptszText = mir_a2t(RemoveHtml(content));
		gce.dwFlags = GCEF_ADDTOLOG;
		CallServiceSync(MS_GC_EVENT, 0, (LPARAM)&gce);
	}
	else if (!mir_strcmpi(messageType, "ThreadActivity/AddMember"))
	{
		ptrA xinitiator, xtarget, initiator, target;
		//content = <addmember><eventtime>1429186229164</eventtime><initiator>8:initiator</initiator><target>8:user</target></addmember>

		HXML xml = xi.parseString(ptrT(mir_a2t(content)), 0, _T("addmember"));
		if (xml != NULL) {

			HXML xmlNode = xi.getChildByPath(xml, _T("target"), 0);
			xtarget = node != NULL ? mir_t2a(xi.getText(xmlNode)) : NULL;

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
			xtarget = node != NULL ? mir_t2a(xi.getText(xmlNode)) : NULL;

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
			GCDEST gcd = { m_szModuleName, ptrT(mir_a2t(chatname)), GC_EVENT_KICK };
			GCEVENT gce = { sizeof(GCEVENT), &gcd };
			gce.ptszUID = ptrT(mir_a2t(target));
			gce.ptszNick = ptrT(mir_a2t(target));
			gce.ptszStatus = ptrT(mir_a2t(initiator));
			gce.time = timestamp;
			CallServiceSync(MS_GC_EVENT, 0, (LPARAM)&gce);
		}
		else
		{
			RemoveChatContact(_A2T(chatname), target, target);
		}
	}
	else if (!mir_strcmpi(messageType, "ThreadActivity/TopicUpdate"))
	{

	}
	else if (!mir_strcmpi(messageType, "ThreadActivity/RoleUpdate"))
	{

	}
}

void CSkypeProto::OnGetChatInfo(const NETLIBHTTPREQUEST *response)
{
	if (response == NULL || response->pData == NULL)
		return;

	JSONROOT root(response->pData);
	JSONNODE *members = json_get(root, "members");
	JSONNODE *properties = json_get(root, "properties");
	ptrA topic(mir_t2a(json_as_string(json_get(properties, "topic"))));
	ptrA chatId(ChatUrlToName(ptrA(mir_t2a(ptrT(json_as_string(json_get(root, "messages")))))));
	//RenameChat(chatId, topic);
	for (size_t i = 0; i < json_size(members); i++)
	{
		JSONNODE *member = json_at(members, i);

		ptrA username(ContactUrlToName(ptrA(mir_t2a(ptrT(json_as_string(json_get(member, "userLink")))))));
		ptrT role(json_as_string(json_get(member, "role")));
		if (!IsChatContact(_A2T(chatId), username))
			AddChatContact(_A2T(chatId), username, username, role);
	}
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

void CSkypeProto::AddChatContact(const TCHAR *tchat_id, const char *id, const char *name, const TCHAR *role)
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
	gce.time = time(NULL);
	gce.bIsMe = IsMe(id);
	gce.ptszStatus = TranslateTS(role);

	CallServiceSync(MS_GC_EVENT, 0, reinterpret_cast<LPARAM>(&gce));
}

void CSkypeProto::RemoveChatContact(const TCHAR *tchat_id, const char *id, const char *name)
{
	if(IsMe(id))
		return;
	
	ptrT tnick(mir_a2t_cp(name, CP_UTF8));
	ptrT tid(mir_a2t(id));

	GCDEST gcd = { m_szModuleName, tchat_id, GC_EVENT_PART };
	GCEVENT gce = { sizeof(gce), &gcd };
	gce.dwFlags = GCEF_ADDTOLOG;
	gce.ptszNick = tnick;
	gce.ptszUID = tid;
	gce.time = time(NULL);
	gce.bIsMe = false;

	CallServiceSync(MS_GC_EVENT, 0, reinterpret_cast<LPARAM>(&gce));
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