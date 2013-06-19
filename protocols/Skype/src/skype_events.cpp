#include "skype.h"

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
		bbd.ptszTooltip = ::TranslateT("Invite contacts to conference");
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

		bbd.bbbFlags = BBBF_ISCHATBUTTON | BBBF_ISRSIDEBUTTON;
		bbd.ptszTooltip = ::TranslateT("Bookmark");
		bbd.hIcon = CSkypeProto::GetIconHandle("bookmark");
		bbd.dwButtonID = BBB_ID_CONF_BOOKMARK;
		bbd.dwDefPos = 100 + bbd.dwButtonID;
		::CallService(MS_BB_ADDBUTTON, 0, (LPARAM)&bbd);

		this->HookEvent(ME_MSG_WINDOWEVENT, &CSkypeProto::OnSrmmWindowOpen);
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

		bbd.dwButtonID = BBB_ID_CONF_BOOKMARK;
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
			this->DeleteChatRoom(hContact);
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
		bbd.bbbFlags = (!::strcmp(::GetContactProto(ev->hContact), this->m_szModuleName)) ? 0 : BBSF_HIDDEN | BBSF_DISABLED;

		bbd.dwButtonID = BBB_ID_CONF_INVITE;
		::CallService(MS_BB_SETBUTTONSTATE, (WPARAM)ev->hContact, (LPARAM)&bbd);

		bbd.dwButtonID = BBB_ID_CONF_SPAWN;
		::CallService(MS_BB_SETBUTTONSTATE, (WPARAM)ev->hContact, (LPARAM)&bbd);

		bbd.bbbFlags = 0;
		if (::strcmp(::GetContactProto(ev->hContact), this->m_szModuleName) != 0)
			bbd.bbbFlags = BBSF_HIDDEN | BBSF_DISABLED;
		else if (this->IsChatRoomBookmarked(ev->hContact)) 
			bbd.bbbFlags = BBSF_DISABLED;
		bbd.dwButtonID = BBB_ID_CONF_BOOKMARK;
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
			this->InviteToChatRoom(hContact);
		break;

	case BBB_ID_CONF_SPAWN:
		if (this->IsOnline() && !this->IsChatRoom(hContact))
		{
			StringList targets;
			targets.insert(ptrW(::db_get_wsa(hContact, this->m_szModuleName, SKYPE_SETTINGS_SID)));

			this->StartChat(targets);
		}
		break;

	case BBB_ID_CONF_BOOKMARK:
		this->SetBookmarkCommand(wParam, 0);
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