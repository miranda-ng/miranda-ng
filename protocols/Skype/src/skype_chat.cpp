#include "skype_proto.h"
#include <m_chat.h>
#include <m_message.h>
#include <m_history.h>

bool CSkypeProto::IsChatRoom(HANDLE hContact)
{
	return ::DBGetContactSettingByte(hContact, this->m_szModuleName, "ChatRoom", 0) > 0;
}

HANDLE CSkypeProto::GetChatRoomByID(const char *cid)
{
	HANDLE hContact = ::db_find_first();
	while (hContact)
	{
		if  (this->IsProtoContact(hContact) && this->IsChatRoom(hContact))
		{
			char *chatID = ::DBGetString(hContact, this->m_szModuleName, "ChatRoomID");
			if (chatID && ::strcmp(cid, chatID) == 0)
				return hContact;
		}

		hContact = ::db_find_next(hContact);
	}

	return 0;
}

HANDLE CSkypeProto::AddChatRoomByID(const char* cid, const char* name, DWORD flags)
{
	HANDLE hContact = this->GetChatRoomByID(cid);
	if ( !hContact)
	{
		hContact = (HANDLE)::CallService(MS_DB_CONTACT_ADD, 0, 0);
		::CallService(MS_PROTO_ADDTOCONTACT, (WPARAM)hContact, (LPARAM)this->m_szModuleName);

		::DBWriteContactSettingByte(hContact, this->m_szModuleName, "ChatRoom", 1);
		::DBWriteContactSettingString(hContact, this->m_szModuleName, "ChatRoomID", cid);
		::DBWriteContactSettingString(hContact, this->m_szModuleName, "Nick", name);
		::DBWriteContactSettingWord(hContact, this->m_szModuleName, "Status", ID_STATUS_OFFLINE);
		::DBWriteContactSettingWord(hContact, this->m_szModuleName, "ApparentMode", ID_STATUS_OFFLINE);
		
		wchar_t *defaultGroup = ::DBGetStringW(NULL, "Chat", "AddToGroup");
		if (defaultGroup)
		{
			::DBWriteContactSettingWString(hContact, "CList", "Group", defaultGroup);		
		}
	}

	return hContact;
}

void CSkypeProto::ChatValidateContact(HANDLE hItem, HWND hwndList, const char *contacts)
{
	if ( !this->IsProtoContact(hItem) || this->IsChatRoom(hItem)) 
	{
		/*HANDLE hContact = (HANDLE)::SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_NEXT, (LPARAM)hItem);
		char *sid = ::DBGetString(hContact, this->m_szModuleName, "sid");
		if (!sid)
			::SendMessage(hwndList, CLM_DELETEITEM, (WPARAM)hItem, 0);
		else if(contacts && ::strstr(contacts, sid))*/
			::SendMessage(hwndList, CLM_DELETEITEM, (WPARAM)hItem, 0);
	}
}

void CSkypeProto::ChatPrepare(HANDLE hItem, HWND hwndList, const char *contacts)
{
	if (hItem == NULL)
		hItem = (HANDLE)::SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_ROOT, 0);

	while (hItem) 
	{
		HANDLE hItemN = (HANDLE)::SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_NEXT, (LPARAM)hItem);

		if (IsHContactGroup(hItem))
		{
			HANDLE hItemT = (HANDLE)::SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_CHILD, (LPARAM)hItem);
			if (hItemT) this->ChatPrepare(hItemT, hwndList, contacts);
		}
		else if (IsHContactContact(hItem))
			this->ChatValidateContact(hItem, hwndList, contacts);

		hItem = hItemN;
   }
}

void CSkypeProto::GetInviteContacts(HANDLE hItem, HWND hwndList, SEStringList &chatTargets)
{
	if (hItem == NULL)
		hItem = (HANDLE)::SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_ROOT, 0);

	while (hItem) 
	{
		if (IsHContactGroup(hItem))
		{
			HANDLE hItemT = (HANDLE)SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_CHILD, (LPARAM)hItem);
			if (hItemT) this->GetInviteContacts(hItemT, hwndList, chatTargets);
		}
		else
		{
			int chk = SendMessage(hwndList, CLM_GETCHECKMARK, (WPARAM)hItem, 0);
			if (chk)
			{
				if (IsHContactInfo(hItem))
				{
					TCHAR buf[128] = _T("");
					SendMessage(hwndList, CLM_GETITEMTEXT, (WPARAM)hItem, (LPARAM)buf);

					if (buf[0]) chatTargets.append(mir_t2a(buf));
				}
				else 
				{
					char *sid = ::DBGetString(hItem, this->m_szModuleName, "sid");
					if (sid) chatTargets.append(sid);
				}
			}
		}
		hItem = (HANDLE)SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_NEXT, (LPARAM)hItem);
	}
}

static const COLORREF crCols[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};

void CSkypeProto::InitChat()
{
	GCREGISTER gcr = {0};
	gcr.cbSize = sizeof(gcr);
	gcr.dwFlags = GC_TYPNOTIF | GC_CHANMGR;
	gcr.iMaxText = 0;
	gcr.nColors = 16;
	gcr.pColors = (COLORREF*)crCols;
	gcr.pszModuleDispName = ::mir_u2a(this->m_tszUserName);
	gcr.pszModule = this->m_szModuleName;
	CallServiceSync(MS_GC_REGISTER, 0, (LPARAM)&gcr);

	this->HookEvent(ME_GC_EVENT, &CSkypeProto::OnGCEventHook);
	this->HookEvent(ME_GC_BUILDMENU, &CSkypeProto::OnGCMenuHook);
}

char *CSkypeProto::StartChat(const char *cid)
{
	char *chatID;
	SEString data;
	CConversation::Ref conversation;

	if (cid)
	{
		g_skype->GetConversationByIdentity(cid, conversation);
		conversation->GetJoinBlob(data);
		g_skype->GetConversationByBlob(data, conversation, false);
		conversation->Join();

		chatID = ::mir_strdup(cid);
	}
	else
	{
		g_skype->CreateConference(conversation);
		conversation->SetOption(CConversation::P_OPT_JOINING_ENABLED,	true);
		conversation->SetOption(CConversation::P_OPT_ENTRY_LEVEL_RANK,	CParticipant::WRITER);
		conversation->SetOption(CConversation::P_OPT_DISCLOSE_HISTORY,  1);

		conversation->GetPropIdentity(data);
		chatID = ::mir_strdup(data);
	}	

	conversation->GetPropDisplayname(data);
	char *chatName = ::mir_utf8decodeA(data);

	GCSESSION gcw = {0};
	gcw.cbSize = sizeof(gcw);
	gcw.iType = GCW_CHATROOM;
	gcw.pszModule = this->m_szModuleName;
	gcw.pszName = chatName;
	gcw.pszID = chatID;
	::CallServiceSync(MS_GC_NEWSESSION, 0, (LPARAM)&gcw);

	GCDEST gcd = { m_szModuleName, { NULL }, GC_EVENT_ADDGROUP };
	gcd.pszID = chatID;

	GCEVENT gce = {0};
	gce.cbSize = sizeof(GCEVENT);
	gce.pDest = &gcd;
	gce.pszStatus = ::Translate("Me");
	::CallServiceSync(MS_GC_EVENT, 0, (LPARAM)&gce);

	gcd.iType = GC_EVENT_ADDGROUP;
	gce.pszStatus = ::Translate("Others");
	::CallServiceSync(MS_GC_EVENT, 0, (LPARAM)&gce);

	gcd.iType = GC_EVENT_CONTROL;
	::CallServiceSync(MS_GC_EVENT, SESSION_INITDONE, (LPARAM)&gce);
	::CallServiceSync(MS_GC_EVENT, SESSION_ONLINE,   (LPARAM)&gce);
	::CallServiceSync(MS_GC_EVENT, WINDOW_VISIBLE,   (LPARAM)&gce);

	::mir_free(chatName);

	return chatID;
}

void CSkypeProto::JoinToChat(const char *cid, bool showWindow)
{
	char *chatID = ::mir_strdup(cid);

	SEString data;
	CConversation::Ref conversation;

	g_skype->GetConversationByIdentity(cid, conversation);
	conversation->GetJoinBlob(data);
	g_skype->GetConversationByBlob(data, conversation, false);
	conversation->Join();

	conversation->GetPropDisplayname(data);
	char *chatName = ::mir_utf8decodeA((const char *)data);

	GCSESSION gcw = {0};
	gcw.cbSize = sizeof(gcw);
	gcw.iType = GCW_CHATROOM;
	gcw.pszModule = this->m_szModuleName;
	gcw.pszName = chatName;
	gcw.pszID = chatID;
	::CallServiceSync(MS_GC_NEWSESSION, 0, (LPARAM)&gcw);

	GCDEST gcd = { m_szModuleName, { NULL }, GC_EVENT_ADDGROUP };
	gcd.pszID = chatID;

	GCEVENT gce = {0};
	gce.cbSize = sizeof(GCEVENT);
	gce.pDest = &gcd;
	gce.pszStatus = ::Translate("Me");
	::CallServiceSync(MS_GC_EVENT, 0, (LPARAM)&gce);

	gcd.iType = GC_EVENT_ADDGROUP;
	gce.pszStatus = ::Translate("Others");
	::CallServiceSync(MS_GC_EVENT, 0, (LPARAM)&gce);

	gcd.iType = GC_EVENT_CONTROL;
	::CallServiceSync(MS_GC_EVENT, showWindow ? SESSION_INITDONE : WINDOW_HIDDEN, (LPARAM)&gce);
	::CallServiceSync(MS_GC_EVENT, SESSION_ONLINE,   (LPARAM)&gce);
	//::CallServiceSync(MS_GC_EVENT, showWindow ? WINDOW_VISIBLE : WINDOW_HIDDEN,   (LPARAM)&gce);

	CParticipant::Refs participants;
	conversation->GetParticipants(participants, CConversation::OTHER_CONSUMERS);
	for (uint i = 0; i < participants.size(); i++)
	{
		participants[i]->GetPropIdentity(data);
		this->AddChatContact(cid, ::mir_strdup(data));
	}

	::mir_free(chatName);
	::mir_free(chatID);
}

void CSkypeProto::LeaveChat(const char *cid)
{
	char *chatID = ::mir_strdup(cid);

	GCDEST gcd = { m_szModuleName, { NULL }, GC_EVENT_CONTROL };
	gcd.pszID = chatID;

	GCEVENT gce = {0};
	gce.cbSize = sizeof(GCEVENT);
	gce.pDest = &gcd;
	::CallServiceSync(MS_GC_EVENT, SESSION_OFFLINE, (LPARAM)&gce);
	::CallServiceSync(MS_GC_EVENT, SESSION_TERMINATE, (LPARAM)&gce);

	::mir_free(chatID);
}

void CSkypeProto::RaiseChatEvent(const char *cid, const char *sid, int evt, const char *message)
{
	char *idt = ::mir_strdup(cid);
	char *snt = ::mir_strdup(sid);

	HANDLE hContact = this->GetContactBySid(sid);
	char *nick = hContact ? (char *)::CallService(MS_CLIST_GETCONTACTDISPLAYNAME, WPARAM(hContact), 0) : snt;

	GCDEST gcd = { this->m_szModuleName, { NULL },  evt };
	gcd.pszID = idt;

	GCEVENT gce = {0};
	gce.cbSize = sizeof(gce);
	gce.dwFlags = GCEF_ADDTOLOG;
	gce.pDest = &gcd;
	gce.pszNick = nick;
	gce.pszUID = snt;
	gce.bIsMe = ::stricmp(sid, this->login) == 0;
	gce.pszStatus = gce.bIsMe ? ::Translate("Me") : ::Translate("Others");
	gce.pszText = message;
	gce.time = time(NULL);
	::CallServiceSync(MS_GC_EVENT, 0, (LPARAM)&gce);

	::mir_free(snt);
	::mir_free(idt);
}

void CSkypeProto::SendChatMessage(const char *cid, const char *sid, const char *message)
{
	this->RaiseChatEvent(cid, sid, GC_EVENT_MESSAGE, message);
}

void CSkypeProto::AddChatContact(const char *cid, const char *sid)
{
	this->RaiseChatEvent(cid, sid, GC_EVENT_JOIN);
}

void CSkypeProto::KickChatContact(const char *cid, const char *sid)
{
	this->RaiseChatEvent(cid, sid, GC_EVENT_KICK);
}

void CSkypeProto::RemoveChatContact(const char *cid, const char *sid)
{
	this->RaiseChatEvent(cid, sid, GC_EVENT_QUIT);
}

INT_PTR __cdecl CSkypeProto::OnJoinChat(WPARAM wParam, LPARAM)
{
	HANDLE hContact = (HANDLE)wParam;
	if (hContact)
	{
		this->JoinToChat(::DBGetString(hContact, this->m_szModuleName, "ChatRoomID"));
	}

	return 0;
}

INT_PTR __cdecl CSkypeProto::OnLeaveChat(WPARAM wParam, LPARAM)
{
	HANDLE hContact = (HANDLE)wParam;
	if (hContact)
	{
		char *cid = ::DBGetString(hContact, this->m_szModuleName, "ChatRoomID");
		this->LeaveChat(cid);
		
		::mir_free(cid);
	}

	return 0;
}

int __cdecl CSkypeProto::OnGCEventHook(WPARAM, LPARAM lParam) 
{
	GCHOOK *gch = (GCHOOK *)lParam;
	if (!gch) return 1;

	if (::strcmp(gch->pDest->pszModule, this->m_szModuleName)) 
		return 0;

	char *chatID = ::mir_strdup(gch->pDest->pszID);
	char *sid = ::mir_strdup(gch->pszUID);

	switch (gch->pDest->iType) 
	{
		case GC_SESSION_TERMINATE:
			{
				CConversation::Ref conversation;
				if (g_skype->GetConversationByIdentity(chatID, conversation, false))
				{
					Participant::Refs participants;
					conversation->GetParticipants(participants, CConversation::MYSELF);
					participants[0]->Retire();
				}
			}
			break;

		case GC_USER_MESSAGE:
			if (gch->pszText && gch->pszText[0]) 
			{
				CConversation::Ref conversation;
				if (g_skype->GetConversationByIdentity(chatID, conversation, false))
				{
					CMessage::Ref message;
					char *text = ::mir_utf8encode(gch->pszText);
					conversation->PostText(text, message);
				}
			}
			break;

		case GC_USER_CHANMGR: 
			DialogBoxParam(
				g_hInstance, 
				MAKEINTRESOURCE(IDD_CHATROOM_INVITE), 
				NULL, 
				CSkypeProto::InviteToChatProc, 
				LPARAM(new InviteChatParam(chatID, NULL, this)));
			break;

		case GC_USER_PRIVMESS:
			::CallService(MS_MSG_SENDMESSAGE, (WPARAM)this->GetContactBySid(sid), 0);
			break;

		case GC_USER_LOGMENU:
			switch(gch->dwData) 
			{
			case 10:
				::DialogBoxParam(
					g_hInstance,
					MAKEINTRESOURCE(IDD_CHATROOM_INVITE), 
					NULL, 
					CSkypeProto::InviteToChatProc, 
					LPARAM(new InviteChatParam(chatID, NULL, this)));
				break;

			case 20:
				this->LeaveChat(chatID);
				break;
			}
			break;
		
		case GC_USER_NICKLISTMENU: 
			switch (gch->dwData) 
			{
			case 10:
				::CallService(MS_USERINFO_SHOWDIALOG, (WPARAM)this->GetContactBySid(sid), 0);
				break;

			case 20:
				CallService(MS_HISTORY_SHOWCONTACTHISTORY, (WPARAM)this->GetContactBySid(sid), 0);
				break;

			case 110:
				this->LeaveChat(chatID);
				break;
			}
			break;

		case GC_USER_TYPNOTIFY: 
			break;
	}
	::mir_free(sid);
	::mir_free(chatID);

	return 0;
}

int __cdecl CSkypeProto::OnGCMenuHook(WPARAM, LPARAM lParam) 
{
	GCMENUITEMS *gcmi = (GCMENUITEMS*) lParam;

	if (gcmi == NULL || ::stricmp(gcmi->pszModule, this->m_szModuleName)) return 0;

	if (gcmi->Type == MENU_ON_LOG) 
	{
		static const struct gc_item Items[] = 
		{
			{ TranslateT("&Invite user..."), 10, MENU_ITEM, FALSE },
			{ TranslateT("&Leave chat session"), 20, MENU_ITEM, FALSE }
		};
		gcmi->nItems = SIZEOF(Items);
		gcmi->Item = (gc_item*)Items;
	}
	else if (gcmi->Type == MENU_ON_NICKLIST) 
	{
		char* id = mir_t2a(gcmi->pszUID);
		if  (!::stricmp(this->login, id)) 
		{
			static const struct gc_item Items[] = 
			{
				{ TranslateT("User &details"), 10, MENU_ITEM, FALSE },
				{ TranslateT("User &history"), 20, MENU_ITEM, FALSE },
				{ _T(""), 100, MENU_SEPARATOR, FALSE },
				{ TranslateT("&Leave chat session"), 110, MENU_ITEM, FALSE }
			};
			gcmi->nItems = SIZEOF(Items);
			gcmi->Item = (gc_item*)Items;
		}
		else 
		{
			static const struct gc_item Items[] = 
			{
				{ TranslateT("User &details"), 10, MENU_ITEM, FALSE },
				{ TranslateT("User &history"), 20, MENU_ITEM, FALSE }
			};
			gcmi->nItems = SIZEOF(Items);
			gcmi->Item = (gc_item*)Items;
		}
		mir_free(id);
	}

	return 0;
}


char *CSkypeProto::GetChatUsers(const char *cid)
{
	GC_INFO gci = {0};
	gci.Flags = USERS;
	gci.pszModule = this->m_szModuleName;
	gci.pszID = ::mir_a2t(cid);
	::CallService(MS_GC_GETINFO, 0, (LPARAM)(GC_INFO *) &gci);

	::mir_free(gci.pszID);

	//StringList users(gci.pszUsers, " ");

	return gci.pszUsers;
}