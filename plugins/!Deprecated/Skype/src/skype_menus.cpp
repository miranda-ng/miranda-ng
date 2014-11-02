#include "skype.h"

HANDLE   CSkypeProto::hChooserMenu;
HGENMENU CSkypeProto::contactMenuItems[CMI_MAX];

INT_PTR CSkypeProto::MenuChooseService(WPARAM wParam, LPARAM lParam)
{
	if (lParam)
		*(void**)lParam = (void*)wParam;

	return 0;
}

int CSkypeProto::OnPrebuildContactMenu(WPARAM wParam, LPARAM)
{
	MCONTACT hContact = (MCONTACT)wParam;
	if ( !hContact)
		return 0;

	if ( !this->IsOnline() || ::lstrcmpA(::GetContactProto(hContact), m_szModuleName))
		return 0;

	if ( !this->isChatRoom(hContact))
	{
		bool ctrlPressed = (::GetKeyState(VK_CONTROL) & 0x8000) != 0;
		
		bool authNeed = this->getByte(hContact, "Auth", 0) > 0;
		bool grantNeed = this->getByte(hContact, "Grant", 0) > 0;
		
		::Menu_ShowItem(CSkypeProto::contactMenuItems[CMI_AUTH_REQUEST], ctrlPressed || authNeed);
		::Menu_ShowItem(CSkypeProto::contactMenuItems[CMI_AUTH_GRANT], ctrlPressed || grantNeed);
		::Menu_ShowItem(CSkypeProto::contactMenuItems[CMI_AUTH_REVOKE], ctrlPressed || (!grantNeed && !authNeed));
		::Menu_ShowItem(CSkypeProto::contactMenuItems[CMI_HISTORY], TRUE);

		{
			SEString sid(_T2A(::db_get_wsa(hContact, this->m_szModuleName, SKYPE_SETTINGS_SID)));
			
			ContactRef contact;
			this->GetContact(sid, contact);

			bool isBlocked = false;
			contact->IsMemberOfHardwiredGroup(ContactGroup::CONTACTS_BLOCKED_BY_ME, isBlocked);
		
			CLISTMENUITEM clmi = { sizeof(clmi) };
			clmi.cbSize = sizeof(CLISTMENUITEM);
			clmi.flags = CMIM_FLAGS;
			if (isBlocked)
			{
				clmi.flags |= CMIM_NAME | CMIM_ICON | CMIF_TCHAR;
				clmi.icolibItem = CSkypeProto::GetSkinIconHandle("contact");
				clmi.ptszName = LPGENT("Unblock this person...");
			}
			else
			{
				clmi.flags |= CMIM_NAME | CMIM_ICON | CMIF_TCHAR;
				clmi.icolibItem = CSkypeProto::GetSkinIconHandle("block");
				clmi.ptszName = LPGENT("Block this person...");
			}
			::CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)CSkypeProto::contactMenuItems[CMI_BLOCK], (LPARAM)&clmi);
		}
	}

	return 0;
}

template<int (__cdecl CSkypeProto::*Service)(WPARAM, LPARAM)>
INT_PTR GlobalService(WPARAM wParam, LPARAM lParam)
{
	CSkypeProto *ppro = CSkypeProto::GetContactInstance((MCONTACT)wParam);
	return ppro ? (ppro->*Service)(wParam, lParam) : 0;
}

int CSkypeProto::RequestAuth(WPARAM wParam, LPARAM lParam)
{
	return this->AuthRequest((MCONTACT)wParam, LPGENT("Hi! I'd like to add you to my contact list"));
}

int CSkypeProto::GrantAuth(WPARAM wParam, LPARAM lParam)
{
	CContact::Ref contact;
	MCONTACT hContact = (MCONTACT)wParam;
	SEString sid(_T2A(::db_get_wsa(hContact, this->m_szModuleName, SKYPE_SETTINGS_SID)));
	if (this->GetContact(sid, contact))
	{
		if (contact->SetBuddyStatus(true))
		{
			this->delSetting(hContact, "Auth");
			this->delSetting(hContact, "Grant");
		}
	}

	return 0;
}

int CSkypeProto::RevokeAuth(WPARAM wParam, LPARAM lParam)
{
	CContact::Ref contact;
	MCONTACT hContact = (MCONTACT)wParam;
	SEString sid(_T2A(::db_get_wsa(hContact, this->m_szModuleName, SKYPE_SETTINGS_SID)));
	if (this->GetContact(sid, contact))
	{
		if (contact->SetBuddyStatus(false))
			this->setByte(hContact, "Grant", 1);
		this->contactList.remove_val(contact);
	}

	return 0;
}

int CSkypeProto::PrebuildContactMenu(WPARAM wParam, LPARAM lParam)
{
	for (int i = 0; i < SIZEOF(CSkypeProto::contactMenuItems); i++)
		::Menu_ShowItem(CSkypeProto::contactMenuItems[i], false);

	CSkypeProto* ppro = CSkypeProto::GetContactInstance((MCONTACT)wParam);
	return (ppro) ? ppro->OnPrebuildContactMenu(wParam, lParam) : 0;
}

void  CSkypeProto::InitMenus()
{
	hChooserMenu = MO_CreateMenuObject("SkypeAccountChooser", LPGEN("Skype menu chooser"), 0, "Skype/MenuChoose");

	//////////////////////////////////////////////////////////////////////////////////////
	// Contact menu initialization

	CLISTMENUITEM mi = { 0 };
	mi.cbSize = sizeof(CLISTMENUITEM);
	mi.flags = CMIF_TCHAR | CMIF_NOTOFFLINE;

	// "Request authorization"
	mi.pszService = MODULE"/RequestAuth";
	mi.ptszName = LPGENT("Request authorization");
	mi.position = -201001000 + CMI_AUTH_REQUEST;
	mi.icolibItem = ::LoadSkinnedIconHandle(SKINICON_AUTH_REQUEST);
	CSkypeProto::contactMenuItems[CMI_AUTH_REQUEST] = ::Menu_AddContactMenuItem(&mi);
	::CreateServiceFunction(mi.pszService, GlobalService<&CSkypeProto::RequestAuth>);

	// "Grant authorization"
	mi.pszService = MODULE"/GrantAuth";
	mi.ptszName = LPGENT("Grant authorization");
	mi.position = -201001000 + CMI_AUTH_GRANT;
	mi.icolibItem = ::LoadSkinnedIconHandle(SKINICON_AUTH_GRANT);
	CSkypeProto::contactMenuItems[CMI_AUTH_GRANT] = ::Menu_AddContactMenuItem(&mi);
	::CreateServiceFunction(mi.pszService, GlobalService<&CSkypeProto::GrantAuth>);

	// "Revoke authorization"
	mi.pszService = MODULE"/RevokeAuth";
	mi.ptszName = LPGENT("Revoke authorization");
	mi.position = -201001000 + CMI_AUTH_REVOKE;
	mi.icolibItem = ::LoadSkinnedIconHandle(SKINICON_AUTH_REVOKE);
	CSkypeProto::contactMenuItems[CMI_AUTH_REVOKE] = ::Menu_AddContactMenuItem(&mi);
	::CreateServiceFunction(mi.pszService, GlobalService<&CSkypeProto::RevokeAuth>);

	// "Block"
	mi.pszService = MODULE"/Block";
	mi.ptszName = LPGENT("Block this person...");
	mi.position = -200001000 + CMI_BLOCK;
	mi.icolibItem = CSkypeProto::GetSkinIconHandle("block");
	CSkypeProto::contactMenuItems[CMI_BLOCK] = ::Menu_AddContactMenuItem(&mi);
	::CreateServiceFunction(mi.pszService, GlobalService<&CSkypeProto::BlockCommand>);

	mi.pszService = MODULE"/SyncHistory";
	mi.ptszName = LPGENT("View old messages...");
	mi.flags = CMIF_TCHAR | CMIF_ROOTPOPUP;
	mi.position = -200001000 + CMI_HISTORY;
	mi.icolibItem = ::LoadSkinnedIconHandle(SKINICON_OTHER_HISTORY);
	CSkypeProto::contactMenuItems[CMI_HISTORY] = ::Menu_AddContactMenuItem(&mi);
	
	mi.flags &= ~CMIF_ROOTPOPUP;

	mi.pszService = MODULE"/SyncHistoryDay";
	mi.ptszName = LPGENT("for last day");
	mi.flags |= CMIF_CHILDPOPUP;
	mi.position = -200001000 + CMI_HISTORY + 100;
	mi.hParentMenu = CSkypeProto::contactMenuItems[CMI_HISTORY];
	::Menu_AddContactMenuItem(&mi);
	::CreateServiceFunction(mi.pszService, GlobalService<&CSkypeProto::SyncLastDayHistoryCommand>);

	mi.pszService = MODULE"/SyncHistoryWeek";
	mi.ptszName = LPGENT("for last week");
	mi.flags |= CMIF_CHILDPOPUP;
	mi.position = -200001000 + CMI_HISTORY + 1001;
	mi.hParentMenu = CSkypeProto::contactMenuItems[CMI_HISTORY];
	::Menu_AddContactMenuItem(&mi);
	::CreateServiceFunction(mi.pszService, GlobalService<&CSkypeProto::SyncLastWeekHistoryCommand>);

	mi.pszService = MODULE"/SyncHistoryMonth";
	mi.ptszName = LPGENT("for last month");
	mi.flags |= CMIF_CHILDPOPUP;
	mi.position = -200001000 + CMI_HISTORY + 102;
	mi.hParentMenu = CSkypeProto::contactMenuItems[CMI_HISTORY];
	::Menu_AddContactMenuItem(&mi);
	::CreateServiceFunction(mi.pszService, GlobalService<&CSkypeProto::SyncLastMonthHistoryCommand>);

	mi.pszService = MODULE"/SyncHistory3Month";
	mi.ptszName = LPGENT("for last 3 month");
	mi.flags |= CMIF_CHILDPOPUP;
	mi.position = -200001000 + CMI_HISTORY + 103;
	mi.hParentMenu = CSkypeProto::contactMenuItems[CMI_HISTORY];
	::Menu_AddContactMenuItem(&mi);
	::CreateServiceFunction(mi.pszService, GlobalService<&CSkypeProto::SyncLast3MonthHistoryCommand>);

	mi.pszService = MODULE"/SyncHistoryYear";
	mi.ptszName = LPGENT("for last year");
	mi.flags |= CMIF_CHILDPOPUP;
	mi.position = -200001000 + CMI_HISTORY + 104;
	mi.hParentMenu = CSkypeProto::contactMenuItems[CMI_HISTORY];
	::Menu_AddContactMenuItem(&mi);
	::CreateServiceFunction(mi.pszService, GlobalService<&CSkypeProto::SyncLastYearHistoryCommand>);

	mi.pszService = MODULE"/SyncHistoryAllTime";
	mi.ptszName = LPGENT("for all time");
	mi.flags |= CMIF_CHILDPOPUP;
	mi.position = -200001000 + CMI_HISTORY + 105;
	mi.hParentMenu = CSkypeProto::contactMenuItems[CMI_HISTORY];
	::Menu_AddContactMenuItem(&mi);
	::CreateServiceFunction(mi.pszService, GlobalService<&CSkypeProto::SyncAllTimeHistoryCommand>);
}

void  CSkypeProto::UninitMenus()
{
}

void CSkypeProto::OnInitStatusMenu()
{
	char text[ 200 ];
	strcpy(text, m_szModuleName);
	char* tDest = text + strlen(text);

	CLISTMENUITEM mi = { sizeof(mi) };
	mi.pszService = text;

	HGENMENU hSkypeRoot = ::MO_GetProtoRootMenu(m_szModuleName);
	if (!hSkypeRoot)
	{
		mi.ptszName = m_tszUserName;
		mi.position = -1999901006;
		mi.hParentMenu = HGENMENU_ROOT;
		mi.flags = CMIF_ROOTPOPUP | CMIF_TCHAR | CMIF_KEEPUNTRANSLATED;
		mi.icolibItem = CSkypeProto::GetSkinIconHandle("main");
		hSkypeRoot = m_hMenuRoot = ::Menu_AddProtoMenuItem(&mi);
	}
	else 
	{
		if (m_hMenuRoot)
			::CallService(MO_REMOVEMENUITEM, (WPARAM)m_hMenuRoot, 0);
		m_hMenuRoot = NULL;
	}

	mi.hParentMenu = hSkypeRoot;
	mi.flags = CMIF_CHILDPOPUP | CMIF_TCHAR;	

	// Invite Command
	::strcpy(tDest, "/CreateChatRoom");
	this->CreateProtoService(tDest, &CSkypeProto::CreateChatRoomCommand);
	mi.ptszName = LPGENT("Create conference");
	mi.position = 200000 + SMI_CHAT_CREATE;
	mi.icolibItem = CSkypeProto::GetSkinIconHandle("conference");
	::Menu_AddProtoMenuItem(&mi);

	// Invite Command
	::strcpy(tDest, "/BlockedeList");
	this->CreateProtoService(tDest, &CSkypeProto::OpenBlockedListCommand);
	mi.ptszName = LPGENT("Blocked contacts");
	mi.position = 200000 + SMI_IGNORE_LIST;
	mi.icolibItem = CSkypeProto::GetSkinIconHandle("block");
	::Menu_AddProtoMenuItem(&mi);
}