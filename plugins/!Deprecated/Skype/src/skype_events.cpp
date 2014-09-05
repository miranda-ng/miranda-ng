#include "skype.h"

int CSkypeProto::OnModulesLoaded(WPARAM, LPARAM)
{
	/*if (::ServiceExists(MS_ASSOCMGR_ADDNEWURLTYPE))
	{
		::CreateServiceFunction(MODULE"/ParseSkypeURI", &CSkypeProto::ParseSkypeUri);
		::AssocMgr_AddNewUrlTypeT("skype:", TranslateT("Skype URI API"), g_hInstance, IDI_SKYPE, MODULE"/ParseSkypeURI", 0);
	}*/

	return 0;
}

int CSkypeProto::OnProtoModulesLoaded(WPARAM, LPARAM)
{
	this->InitNetLib();
	this->InitChatModule();
	this->InitInstanceHookList();

	if (::ServiceExists(MS_BB_ADDBUTTON))
	{
		BBButton bbd = { sizeof(bbd) };
		bbd.pszModuleName = MODULE;

		bbd.bbbFlags = BBBF_ISCHATBUTTON | BBBF_ISRSIDEBUTTON;
		bbd.ptszTooltip = ::TranslateT("Invite contacts to conference");
		bbd.hIcon = CSkypeProto::GetSkinIconHandle("addContacts");
		bbd.dwButtonID = BBB_ID_CONF_INVITE;
		bbd.dwDefPos = 100 + bbd.dwButtonID;
		::CallService(MS_BB_ADDBUTTON, 0, (LPARAM)&bbd);

		bbd.bbbFlags = BBBF_ISIMBUTTON | BBBF_ISRSIDEBUTTON;
		bbd.ptszTooltip = ::TranslateT("Spawn conference");
		bbd.hIcon = CSkypeProto::GetSkinIconHandle("conference");
		bbd.dwButtonID = BBB_ID_CONF_SPAWN;
		bbd.dwDefPos = 100 + bbd.dwButtonID;
		::CallService(MS_BB_ADDBUTTON, 0, (LPARAM)&bbd);

		this->HookProtoEvent(ME_MSG_WINDOWEVENT, &CSkypeProto::OnSrmmWindowOpen);
	}

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
	MCONTACT hContact = (MCONTACT)wParam;
	if (hContact)
	{
		if (this->isChatRoom(hContact))
		{
			this->OnLeaveChat(wParam, 0);
			ptrW cid(::db_get_wsa(hContact, this->m_szModuleName, SKYPE_SETTINGS_SID));
			if (cid != NULL)
			{
				ConversationRef conversation;
				if (this->GetConversationByIdentity((char *)_T2A(cid), conversation))
					conversation->Delete();
			}
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
	char *title = ::mir_t2a(this->m_tszUserName);

	OPTIONSDIALOGPAGE odp = { sizeof(odp) };
	odp.hInstance = g_hInstance;
	odp.pszTitle = title;
	odp.dwInitParam = LPARAM(this);
	odp.flags = ODPF_BOLDGROUPS;
	odp.pszGroup = LPGEN("Network");

	odp.pszTab = LPGEN("Account");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_MAIN);
	odp.pfnDlgProc = CSkypeProto::SkypeMainOptionsProc;
	::Options_AddPage(wParam, &odp);

	odp.pszTab = LPGEN("Blocked contacts");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_BLOCKED);
	odp.pfnDlgProc = CSkypeProto::SkypeBlockedOptionsProc;
	::Options_AddPage(wParam, &odp);

	odp.pszTab = LPGEN("Privacy");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_PRIVACY);
	odp.pfnDlgProc = CSkypeProto::SkypePrivacyOptionsProc;
	::Options_AddPage(wParam, &odp);

	::mir_free(title);
	return 0;
}

int __cdecl CSkypeProto::OnUserInfoInit(WPARAM wParam, LPARAM lParam)
{
	if ((!this->IsProtoContact((MCONTACT)lParam)) && lParam)
		return 0;

	OPTIONSDIALOGPAGE odp = { sizeof(odp) };
	odp.flags = ODPF_TCHAR | ODPF_USERINFOTAB | ODPF_DONTTRANSLATE;
	odp.hInstance = g_hInstance;
	odp.dwInitParam = LPARAM(this);
	odp.position = -1900000000;
	odp.ptszTitle = this->m_tszUserName;

	MCONTACT hContact = (MCONTACT)lParam;
	if (hContact) {
		char *szProto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, hContact, 0);
		if (szProto != NULL && !strcmp(szProto, m_szModuleName)) {
			odp.pfnDlgProc = SkypeDlgProc;
			odp.pszTemplate = MAKEINTRESOURCEA(IDD_INFO_SKYPE);
			UserInfo_AddPage(wParam, &odp);
		}
	} 
	else {
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

		odp.pfnDlgProc = AccountSkypeDlgProc;
		odp.pszTemplate = MAKEINTRESOURCEA(IDD_OWNINFO_ACCOUNT);
		odp.ptszTab = LPGENT("Skype account");
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
		bbd.bbbFlags = (!::strcmp(::GetContactProto(ev->hContact), this->m_szModuleName)) ? 0 : BBSF_HIDDEN | BBSF_DISABLED;

		bbd.dwButtonID = BBB_ID_CONF_INVITE;
		::CallService(MS_BB_SETBUTTONSTATE, (WPARAM)ev->hContact, (LPARAM)&bbd);

		bbd.dwButtonID = BBB_ID_CONF_SPAWN;
		::CallService(MS_BB_SETBUTTONSTATE, (WPARAM)ev->hContact, (LPARAM)&bbd);
	} 
	return 0; 
}

int __cdecl CSkypeProto::OnTabSRMMButtonPressed(WPARAM wParam, LPARAM lParam)
{
	MCONTACT hContact = (MCONTACT)wParam;
	CustomButtonClickData *cbcd = (CustomButtonClickData *)lParam;
	
	switch (cbcd->dwButtonId)
	{
	case BBB_ID_CONF_INVITE:
		if (this->IsOnline() && this->isChatRoom(hContact))
			this->ChatRoomInvite(hContact);
		break;

	case BBB_ID_CONF_SPAWN:
		if (this->IsOnline() && !this->isChatRoom(hContact))
		{
			SEStringList targets;
			ptrW sid(::db_get_wsa(hContact, this->m_szModuleName, SKYPE_SETTINGS_SID));
			targets.append((char *)_T2A(sid));

			ConversationRef conversation, conference;
			this->GetConversationByParticipants(targets, conversation);

			StringList invitedContacts(sid);
			ChatRoomParam *param = new ChatRoomParam(NULL, invitedContacts, this);
			
			if (::DialogBoxParam(g_hInstance, MAKEINTRESOURCE(IDD_CHATROOM_CREATE), NULL, CSkypeProto::ChatRoomProc, (LPARAM)param) == IDOK && param->invitedContacts.size() > 0)
			{				
				for (size_t i = 0; i < param->invitedContacts.size(); i++)
				{
					SEString contact(_T2A(param->invitedContacts[i]));
					if ( !targets.contains(contact))
						targets.append(contact);
				}
				conversation->SpawnConference(targets, conference);
			}
		}
		break;
	}

	return 1;
}

void CSkypeProto::OnMessage(
	const MessageRef & message,
	const bool & changesInboxTimestamp,
	const MessageRef & supersedesHistoryMessage,
	const ConversationRef & conversation)
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
		this->OnFileEvent(conversation, message);
		break;

	case CMessage::POSTED_CONTACTS:
			this->OnContactsEvent(conversation, message);
		break;

	//case CMessage::REQUESTED_AUTH:
	//	break;

	//case CMessage::GRANTED_AUTH:
	//	break;

	//case CMessage::BLOCKED:
	//	break;
	}
}