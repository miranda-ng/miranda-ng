#include "skype_proto.h"

int CSkypeProto::OnModulesLoaded(WPARAM, LPARAM)
{
	this->InitChat();
	this->InitNetLib();
	this->InitCustomFolders();
	this->InitInstanceHookList();

	if (::ServiceExists(MS_BB_ADDBUTTON))
	{
		BBButton bbd = { sizeof(bbd) };
		bbd.pszModuleName = MODULE;

		bbd.bbbFlags = BBBF_ISCHATBUTTON | BBBF_ISRSIDEBUTTON;
		bbd.ptszTooltip = ::TranslateT("Invite to conference");
		bbd.hIcon = CSkypeProto::GetIconHandle("confInvite");
		bbd.dwButtonID = BBB_ID_CONF_INVITE;
		bbd.dwDefPos = 100 + bbd.dwButtonID;
		::CallService(MS_BB_ADDBUTTON, 0, (LPARAM)&bbd);

		bbd.bbbFlags = BBBF_ISIMBUTTON | BBBF_ISRSIDEBUTTON;
		bbd.ptszTooltip = ::TranslateT("Spawn conference");
		bbd.hIcon = CSkypeProto::GetIconHandle("confSpawn");
		bbd.dwButtonID = BBB_ID_CONF_SPAWN;
		bbd.dwDefPos = 100 + bbd.dwButtonID;
		::CallService(MS_BB_ADDBUTTON, 0, (LPARAM)&bbd);

		HookEvent(ME_MSG_WINDOWEVENT, &CSkypeProto::OnSrmmWindowOpen);
	}

	g_skype->SetOnMessageCallback(
		(CSkype::OnMessaged)&CSkypeProto::OnSkypeEvent, 
		this);

	return 0;
}

int CSkypeProto::OnPreShutdown(WPARAM, LPARAM)
{
	if (::ServiceExists(MS_BB_REMOVEBUTTON))
	{
		BBButton bbd = { sizeof(bbd) };
		bbd.pszModuleName = MODULE;

		bbd.dwButtonID = BBB_ID_CONF_INVITE;
		::CallService(MS_BB_REMOVEBUTTON, 0, (LPARAM)&bbd);

		bbd.dwButtonID = BBB_ID_CONF_SPAWN;
		::CallService(MS_BB_REMOVEBUTTON, 0, (LPARAM)&bbd);
	}

	this->SetStatus(ID_STATUS_OFFLINE);

	this->UninitNetLib();
	return 0;
}

int CSkypeProto::OnContactDeleted(WPARAM wParam, LPARAM lParam)
{
	HANDLE hContact = (HANDLE)wParam;
	if (hContact && this->IsOnline())
	{
		if (this->IsChatRoom(hContact))
		{
			mir_ptr<wchar_t> chatID(::db_get_wsa(hContact, this->m_szModuleName, "ChatRoomID"));
			this->LeaveChat(chatID);

			CConversation::Ref conversation;
			g_skype->GetConversationByIdentity(::mir_utf8encodeW(chatID), conversation);
			conversation->RetireFrom();
			conversation->Delete();
		}
		else
			this->RevokeAuth(wParam, lParam);
	}

	return 0;
}

INT_PTR __cdecl CSkypeProto::OnAccountManagerInit(WPARAM wParam, LPARAM lParam)
{
	return (int)::CreateDialogParam(
		g_hInstance,
		MAKEINTRESOURCE(IDD_ACCMGR),
		(HWND)lParam,
		&CSkypeProto::SkypeMainOptionsProc,
		(LPARAM)this);
}

int __cdecl CSkypeProto::OnOptionsInit(WPARAM wParam, LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = {0};
	odp.cbSize = sizeof(odp);
	odp.hInstance = g_hInstance;
	odp.ptszTitle = m_tszUserName;
	odp.dwInitParam = LPARAM(this);
	odp.flags = ODPF_BOLDGROUPS | ODPF_TCHAR | ODPF_DONTTRANSLATE;

	odp.position = 271828;
	odp.ptszGroup = LPGENT("Network");
	odp.ptszTab = LPGENT("Account");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_MAIN);
	odp.pfnDlgProc = SkypeMainOptionsProc;
	::Options_AddPage(wParam, &odp);

	return 0;
}

int __cdecl CSkypeProto::OnUserInfoInit(WPARAM wParam, LPARAM lParam)
{
	if ((!this->IsProtoContact((HANDLE)lParam)) && lParam)
		return 0;

	OPTIONSDIALOGPAGE odp = {0};
	odp.cbSize = sizeof(odp);
	odp.flags = ODPF_TCHAR | ODPF_USERINFOTAB | ODPF_DONTTRANSLATE;
	odp.hInstance = g_hInstance;
	odp.dwInitParam = LPARAM(this);
	odp.position = -1900000000;
	odp.ptszTitle = this->m_tszUserName;

	HANDLE hContact = (HANDLE)lParam;
	if (hContact) 
	{
		char *szProto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
		if (szProto != NULL && !strcmp(szProto, m_szModuleName)) 
		{
			odp.pfnDlgProc = SkypeDlgProc;
			odp.pszTemplate = MAKEINTRESOURCEA(IDD_INFO_SKYPE);
			UserInfo_AddPage(wParam, &odp);
		}
	} 
	else 
	{
		NeedUpdate = 0;
		odp.pfnDlgProc = ContactSkypeDlgProc;
		odp.pszTemplate = MAKEINTRESOURCEA(IDD_OWNINFO_CONTACT);
		odp.ptszTab = LPGENT("Contacts");
		UserInfo_AddPage(wParam, &odp);

		odp.pfnDlgProc = HomeSkypeDlgProc;
		odp.pszTemplate = MAKEINTRESOURCEA(IDD_OWNINFO_HOME);
		odp.ptszTab = LPGENT("Home");
		UserInfo_AddPage(wParam, &odp);

		odp.pfnDlgProc = PersonalSkypeDlgProc;
		odp.pszTemplate = MAKEINTRESOURCEA(IDD_OWNINFO_PERSONAL);
		odp.ptszTab = LPGENT("General");
		UserInfo_AddPage(wParam, &odp);
	}

	return 0;
}

int __cdecl CSkypeProto::OnSrmmWindowOpen(WPARAM, LPARAM lParam)
{
	MessageWindowEventData *ev = (MessageWindowEventData*)lParam;
	if (ev->uType == MSG_WINDOW_EVT_OPENING && ev->hContact) 
	{ 
		BBButton bbd = { sizeof(bbd) };
		bbd.pszModuleName = MODULE;
		bbd.bbbFlags = (!strcmp( GetContactProto(ev->hContact), this->m_szModuleName)) ? 0 : BBSF_HIDDEN | BBSF_DISABLED;

		bbd.dwButtonID = BBB_ID_CONF_INVITE;
		::CallService(MS_BB_SETBUTTONSTATE, (WPARAM)ev->hContact, (LPARAM)&bbd);

		bbd.dwButtonID = BBB_ID_CONF_SPAWN;
		::CallService(MS_BB_SETBUTTONSTATE, (WPARAM)ev->hContact, (LPARAM)&bbd);
	} 
	return 0; 
}

int __cdecl CSkypeProto::OnTabSRMMButtonPressed(WPARAM wParam, LPARAM lParam)
{
	HANDLE hContact = (HANDLE)wParam;
	CustomButtonClickData *cbcd = (CustomButtonClickData *)lParam;
	
	switch (cbcd->dwButtonId)
	{
	case BBB_ID_CONF_INVITE:
		if (this->IsOnline() && this->IsChatRoom(hContact))
		{
			StringList targets = this->GetChatUsers(mir_ptr<wchar_t>(::db_get_wsa(hContact, this->m_szModuleName, "ChatRoomID")));

			this->StartChat(targets);
		}
		break;

	case BBB_ID_CONF_SPAWN:
		if (this->IsOnline() && !this->IsChatRoom(hContact))
		{
			StringList targets;
			targets.insert( mir_ptr<wchar_t>(::db_get_wsa(hContact, this->m_szModuleName, SKYPE_SETTINGS_LOGIN)));

			this->StartChat(targets);
		}
		break;
	}

	return 1;
}

void CSkypeProto::OnChatEvent(CConversation::Ref &conversation, CMessage::Ref &message)
{
	CMessage::TYPE messageType;
	message->GetPropType(messageType);

	switch (messageType)
	{
	case CMessage::POSTED_EMOTE:
	case CMessage::POSTED_TEXT:
		{
			SEString author;
			message->GetPropAuthor(author);
			
			if (::wcsicmp(mir_ptr<wchar_t>(::mir_utf8decodeW(author)), this->login) == 0)
				this->OnChatMessageSent(conversation, message);
			else
				this->OnChatMessageReceived(conversation, message);
		}
		break;

	case CMessage::ADDED_CONSUMERS:
		{
			SEString data;

			conversation->GetPropIdentity(data);
			wchar_t *cid = ::mir_utf8decodeW(data);

			HANDLE hContact = this->AddChatRoom(conversation);
			if ( !this->IsContactOnline(hContact))
			{
				this->JoinToChat(conversation);
			}
			else
			{
				StringList alreadyInChat(this->GetChatUsers(cid));

				message->GetPropIdentities(data);
				StringList needToAdd(::mir_utf8decodeW(data));

				CParticipant::Refs participants;
				conversation->GetParticipants(participants, CConversation::OTHER_CONSUMERS);
				for (uint i = 0; i < participants.size(); i++)
				{
					participants[i]->GetPropIdentity(data);
					std::wstring sid = ::mir_utf8decodeW(data);
					
					if (needToAdd.contains(sid.c_str()) && !alreadyInChat.contains(sid.c_str()))
					{
						CContact::Ref contact;
						g_skype->GetContact(std::string(::mir_utf8encodeW(sid.c_str())).c_str(), contact);

						CContact::AVAILABILITY status;
						contact->GetPropAvailability(status);

						CParticipant::RANK rank;
						participants[i]->GetPropRank(rank);

						this->AddChatContact(
							cid, 
							sid.c_str(), 
							CSkypeProto::Roles[rank],
							CSkypeProto::SkypeToMirandaStatus(status));
					}
				}
			}
		}
		break;

	case CMessage::RETIRED:
		{
			SEString data;

			conversation->GetPropIdentity(data);
			wchar_t *cid = ::mir_utf8decodeW(data);

			StringList alreadyInChat(this->GetChatUsers(cid));
			
			message->GetPropAuthor(data);	
			wchar_t *sid = ::mir_utf8decodeW(data);
			if (::wcsicmp(sid, this->login) != 0)
				if (alreadyInChat.contains(sid))
					this->RemoveChatContact(cid, sid);
		}
		break;

	case CMessage::RETIRED_OTHERS:
		{
			SEString data;

			conversation->GetPropIdentity(data);
			mir_ptr<wchar_t> cid( ::mir_utf8decodeW(data));

			message->GetPropIdentities(data);

			StringList alreadyInChat(this->GetChatUsers(cid));
			StringList needToKick(::mir_utf8decodeW(data));
				
			for (size_t i = 0; i < needToKick.size(); i++)
			{
				const wchar_t *sid = needToKick[i];
				if (::wcsicmp(sid, this->login) == 0)
				{
					HANDLE hContact = this->GetChatRoomByCid(cid);
					this->ShowNotification(::TranslateT("You have been kicked from the chat room"), 0, hContact);
					this->LeaveChat(cid);					
				}
				else if ( !alreadyInChat.contains(sid))
					this->KickChatContact(cid, sid);
			}
		}
		break;

	case CMessage::SPAWNED_CONFERENCE:
		{
			SEString data;
			conversation->GetPropIdentity(data);
			char *cid = ::mir_strdup(data);

			HANDLE hContact = this->AddChatRoom(conversation);
			if ( !this->IsContactOnline(hContact))
			{
				this->JoinToChat(conversation);
			}
		}
		break;

	case CMessage::STARTED_LIVESESSION:
		{
			conversation->LeaveLiveSession();

			SEString data;

			conversation->GetPropIdentity(data);
			mir_ptr<wchar_t> cid( ::mir_utf8decodeW(data));
			HANDLE hContact = this->GetChatRoomByCid(cid);
			
			this->RaiseChatEvent(
				cid, 
				this->login, 
				/*GC_EVENT_INFORMATION*/ 0x0100, 
				/*GCEF_ADDTOLOG*/ 0x0001, 
				0, 
				NULL, 
				::TranslateT("Group call"));
		}
		break;

	case CMessage::ENDED_LIVESESSION:
		{
			SEString data;

			conversation->GetPropIdentity(data);
			mir_ptr<wchar_t> cid( ::mir_utf8decodeW(data));
			HANDLE hContact = this->GetChatRoomByCid(cid);
			
			this->RaiseChatEvent(
				cid, 
				this->login, 
				/*GC_EVENT_INFORMATION*/ 0x0100, 
				/*GCEF_ADDTOLOG*/ 0x0001, 
				0, 
				NULL, 
				::TranslateT("The call is completed"));
		}
		break;
	}
}

void CSkypeProto::OnSkypeEvent(CConversation::Ref conversation, CMessage::Ref message)
{
	CMessage::TYPE messageType;
	message->GetPropType(messageType);

	switch (messageType)
	{
	case CMessage::POSTED_EMOTE:
	case CMessage::POSTED_TEXT:
	case CMessage::STARTED_LIVESESSION:
	case CMessage::ENDED_LIVESESSION:
		{
			CConversation::TYPE type;
			conversation->GetPropType(type);
			if (type == 0 || type == CConversation::DIALOG)
				this->OnMessageEvent(conversation, message);
			else
				this->OnChatEvent(conversation, message);
		}
		break;

	case CMessage::ADDED_CONSUMERS:
	case CMessage::RETIRED:
	case CMessage::RETIRED_OTHERS:
	case CMessage::SPAWNED_CONFERENCE:
		this->OnChatEvent(conversation, message);
		break;

	case CMessage::POSTED_FILES:
		this->OnFile(conversation, message);
		break;

	//case CMessage::REQUESTED_AUTH:
	//	break;

	//case CMessage::GRANTED_AUTH:
	//	break;

	//case CMessage::BLOCKED:
	//	break;
	}
}