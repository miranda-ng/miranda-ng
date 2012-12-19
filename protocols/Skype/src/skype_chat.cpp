#include "skype_proto.h"
#include <m_chat.h>
#include <m_message.h>
#include <m_history.h>

bool CSkypeProto::IsChatRoom(HANDLE hContact)
{
	return ::DBGetContactSettingByte(hContact, this->m_szModuleName, "ChatRoom", 0) > 0;
}

void CSkypeProto::ChatValidateContact(HANDLE hItem, HWND hwndList)
{
	if ( !this->IsProtoContact(hItem) || this->IsChatRoom(hItem)) 
		::SendMessage(hwndList, CLM_DELETEITEM, (WPARAM)hItem, 0);
}

void CSkypeProto::ChatPrepare(HANDLE hItem, HWND hwndList)
{
	if (hItem == NULL)
		hItem = (HANDLE)::SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_ROOT, 0);

	while (hItem) 
	{
		HANDLE hItemN = (HANDLE)::SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_NEXT, (LPARAM)hItem);

		if (IsHContactGroup(hItem))
		{
			HANDLE hItemT = (HANDLE)::SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_CHILD, (LPARAM)hItem);
			if (hItemT) this->ChatPrepare(hItemT, hwndList);
		}
		else if (IsHContactContact(hItem))
			this->ChatValidateContact(hItem, hwndList);

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

void CSkypeProto::StartChat(HANDLE hContact, SEStringList &chatTargets)
{
	char *chatID;
	SEString data;
	CConversation::Ref conversation;	

	if (hContact)
	{
		chatID = ::DBGetString(hContact, this->m_szModuleName, "ChatRoomID");
		g_skype->GetConversationByIdentity(chatID, conversation);
		conversation->GetJoinBlob(data);
		char *blob = ::mir_strdup((const char *)data);
		g_skype->GetConversationByBlob(blob, conversation, false);
		conversation->Join();

		CParticipant::Refs participants;
		conversation->GetParticipants(participants, CConversation::OTHER_CONSUMERS);
		for (uint i = 0; i < participants.size(); i++)
		{
			participants[i]->GetPropIdentity(data);
			chatTargets.append(data);
		}
	}
	else if ( !g_skype->GetConversationByParticipants(chatTargets, conversation))
	{
		g_skype->CreateConference(conversation);
		conversation->SetOption(CConversation::P_OPT_JOINING_ENABLED,  true);
		conversation->AddConsumers(chatTargets);

		conversation->GetPropIdentity(data);
		chatID = ::mir_strdup((const char *)data);
	}

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

	for (uint i = 0; i < chatTargets.size(); i++)
	{
		HANDLE hContact = this->GetContactBySid(chatTargets[i]);
		gcd.iType = GC_EVENT_JOIN;
		gce.pszNick = ::DBGetString(hContact, this->m_szModuleName, "Nick");
		gce.pszUID = chatTargets[i];
		::CallServiceSync(MS_GC_EVENT, 0, (LPARAM)&gce);
	}

	gcd.iType = GC_EVENT_CONTROL;
	::CallServiceSync(MS_GC_EVENT, SESSION_INITDONE, (LPARAM)&gce);
	::CallServiceSync(MS_GC_EVENT, SESSION_ONLINE,   (LPARAM)&gce);
	::CallServiceSync(MS_GC_EVENT, WINDOW_VISIBLE,   (LPARAM)&gce);

	::mir_free(chatName);
	::mir_free(chatID);
}

void CSkypeProto::ChatEvent(const char *chatID, const char *sid, int evt, const char *msg)
{
	char *idt = ::mir_strdup(chatID);
	char *snt = ::mir_strdup(sid);

	HANDLE hContact = this->GetContactBySid(sid);
	char *nick = hContact ? (char *)::CallService(MS_CLIST_GETCONTACTDISPLAYNAME, WPARAM(hContact), 0) : snt;

	GCDEST gcd = { m_szModuleName, { NULL },  evt };
	gcd.pszID = idt;

	GCEVENT gce = {0};
	gce.cbSize = sizeof(gce);
	gce.dwFlags = GCEF_ADDTOLOG;
	gce.pDest = &gcd;
	gce.pszNick = nick;
	gce.pszUID = snt;
	gce.bIsMe = ::stricmp(sid, this->login) == 0;
	gce.pszStatus = gce.bIsMe ? ::Translate("Me") : ::Translate("Others");
	gce.pszText = msg;
	gce.time = time(NULL);
	::CallServiceSync(MS_GC_EVENT, 0, (LPARAM)&gce);

	::mir_free(snt);
	::mir_free(idt);
}

INT_PTR __cdecl CSkypeProto::OnJoinChat(WPARAM wParam, LPARAM)
{
	HANDLE hContact = (HANDLE)wParam;
	SEStringList list;
	this->StartChat(hContact, list);

	return 0;
}

INT_PTR __cdecl CSkypeProto::OnLeaveChat(WPARAM wParam, LPARAM)
{
	HANDLE hContact = (HANDLE)wParam;
	char *chatID = ::DBGetString(hContact, this->m_szModuleName, "ChatID");
	this->ChatLeave(chatID);

	return 0;
}

void CSkypeProto::ChatLeave(const char *chatID)
{
	char *idt = ::mir_strdup(chatID);

	GCDEST gcd = { m_szModuleName, { NULL }, GC_EVENT_CONTROL };
	gcd.pszID = idt;

	GCEVENT gce = {0};
	gce.cbSize = sizeof(GCEVENT);
	//gce.dwFlags = GCEF_REMOVECONTACT;
	gce.pDest = &gcd;
	::CallServiceSync(MS_GC_EVENT, SESSION_OFFLINE, (LPARAM)&gce);
	::CallServiceSync(MS_GC_EVENT, SESSION_TERMINATE, (LPARAM)&gce);

	::mir_free(idt);
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
				this->ChatLeave(chatID);
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
				this->ChatLeave(chatID);
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