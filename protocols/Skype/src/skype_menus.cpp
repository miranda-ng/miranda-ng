#include "skype.h"

HGENMENU CSkypeProto::hChooserMenu;
HGENMENU CSkypeProto::contactMenuItems[CMI_MAX];

INT_PTR CSkypeProto::MenuChooseService(WPARAM wParam, LPARAM lParam)
{
	if (lParam)
		*(void**)lParam = (void*)wParam;

	return 0;
}

int CSkypeProto::OnPrebuildContactMenu(WPARAM wParam, LPARAM)
{
	HANDLE hContact = (HANDLE)wParam;
	if ( !hContact)
		return 0;

	if ( !this->IsOnline() || ::lstrcmpA(::GetContactProto(hContact), m_szModuleName))
		return 0;

	if ( !this->IsChatRoom(hContact))
	{
		bool ctrlPressed = (::GetKeyState(VK_CONTROL) & 0x8000) != 0;
		
		bool authNeed = ::db_get_b(hContact, this->m_szModuleName, "Auth", 0) > 0;
		bool grantNeed = ::db_get_b(hContact, this->m_szModuleName, "Grant", 0) > 0;
		bool ignored = ::db_get_b(hContact, this->m_szModuleName, "Ignore", 0) > 0;

		Menu_ShowItem(CSkypeProto::contactMenuItems[CMI_AUTH_REQUEST], ctrlPressed || authNeed);
		Menu_ShowItem(CSkypeProto::contactMenuItems[CMI_AUTH_GRANT], ctrlPressed || grantNeed);
		Menu_ShowItem(CSkypeProto::contactMenuItems[CMI_AUTH_REVOKE], ctrlPressed || (!grantNeed && !authNeed));
		Menu_ShowItem(CSkypeProto::contactMenuItems[CMI_BLOCK], ctrlPressed);

		CLISTMENUITEM clmi = { sizeof(clmi) };
		clmi.cbSize = sizeof(CLISTMENUITEM);
		clmi.flags = CMIM_FLAGS;
		//::CallService(MS_IGNORE_ISIGNORED, wParam, IGNOREEVENT_ALL);
		if (::db_get_b(hContact, this->m_szModuleName, "Ignore", 0) == 1)
			clmi.flags |= CMIF_CHECKED;
		
		::CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)CSkypeProto::contactMenuItems[CMI_IGNORE], (LPARAM)&clmi);
	}

	return 0;
}

template<int (__cdecl CSkypeProto::*Service)(WPARAM, LPARAM)>
INT_PTR GlobalService(WPARAM wParam, LPARAM lParam)
{
	CSkypeProto *ppro = CSkypeProto::GetContactInstance((HANDLE)wParam);
	return ppro ? (ppro->*Service)(wParam, lParam) : 0;
}

int CSkypeProto::RequestAuth(WPARAM wParam, LPARAM lParam)
{
	return this->AuthRequest((HANDLE)wParam, LPGENT("Hi! I\'d like to add you to my contact list"));
}

int CSkypeProto::GrantAuth(WPARAM wParam, LPARAM lParam)
{
	CContact::Ref contact;
	HANDLE hContact = (HANDLE)wParam;
	SEString sid(_T2A(::db_get_wsa(hContact, this->m_szModuleName, SKYPE_SETTINGS_SID)));
	if (this->GetContact(sid, contact))
	{
		if (contact->SetBuddyStatus(true))
		{
			::db_unset(hContact, this->m_szModuleName, "Auth");
			::db_unset(hContact, this->m_szModuleName, "Grant");
		}
	}

	return 0;
}

int CSkypeProto::RevokeAuth(WPARAM wParam, LPARAM lParam)
{
	CContact::Ref contact;
	HANDLE hContact = (HANDLE)wParam;
	SEString sid(_T2A(::db_get_wsa(hContact, this->m_szModuleName, SKYPE_SETTINGS_SID)));
	if (this->GetContact(sid, contact))
	{
		if (contact->SetBuddyStatus(false))
			::db_set_b(hContact, this->m_szModuleName, "Grant", 1);
		this->contactList.remove_val(contact);
	}

	return 0;
}

int CSkypeProto::PrebuildContactMenu(WPARAM wParam, LPARAM lParam)
{
	for (int i = 0; i < SIZEOF(CSkypeProto::contactMenuItems); i++)
		::Menu_ShowItem(CSkypeProto::contactMenuItems[i], false);

	CSkypeProto* ppro = CSkypeProto::GetContactInstance((HANDLE)wParam);
	return (ppro) ? ppro->OnPrebuildContactMenu(wParam, lParam) : 0;
}

void  CSkypeProto::InitMenus()
{
	TMenuParam mnu = {0};
	mnu.cbSize = sizeof(mnu);
	mnu.name = "SkypeAccountChooser";
	mnu.ExecService = "Skype/MenuChoose";
	hChooserMenu = (HGENMENU)::CallService(MO_CREATENEWMENUOBJECT, 0, (LPARAM)&mnu);

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

	// "Ignore"
	mi.pszService = MODULE"/Ignore";
	mi.ptszName = LPGENT("Ignore");
	mi.position = -200001000 + CMI_IGNORE;
	mi.icolibItem = NULL;//CSkypeProto::GetIconHandle("ignore");
	CSkypeProto::contactMenuItems[CMI_IGNORE] = ::Menu_AddContactMenuItem(&mi);
	::CreateServiceFunction(mi.pszService, GlobalService<&CSkypeProto::IgnoreCommand>);

	// "Block"
	mi.pszService = MODULE"/Block";
	mi.ptszName = LPGENT("Block");
	//mi.flags |= CMIF_HIDDEN;
	mi.position = -200001000 + CMI_BLOCK;
	mi.icolibItem = CSkypeProto::GetIconHandle("block");
	CSkypeProto::contactMenuItems[CMI_BLOCK] = ::Menu_AddContactMenuItem(&mi);
	::CreateServiceFunction(mi.pszService, GlobalService<&CSkypeProto::BlockCommand>);
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
		mi.icolibItem = CSkypeProto::GetIconHandle("main");
		hSkypeRoot = m_hMenuRoot = ::Menu_AddProtoMenuItem(&mi);
	}
	else 
	{
		if (m_hMenuRoot)
			::CallService(MS_CLIST_REMOVEMAINMENUITEM, (WPARAM)m_hMenuRoot, 0);
		m_hMenuRoot = NULL;
	}

	mi.hParentMenu = hSkypeRoot;
	mi.flags = CMIF_CHILDPOPUP | CMIF_TCHAR;	

	// Invite Command
	::strcpy(tDest, "/CreateChatRoom");
	this->CreateServiceObj(tDest, &CSkypeProto::CreateChatRoomCommand);
	mi.ptszName = LPGENT("Create conference");
	mi.position = 200000 + SMI_CHAT_CREATE;
	mi.icolibItem = CSkypeProto::GetIconHandle("conference");
	::Menu_AddProtoMenuItem(&mi);

	// Invite Command
	::strcpy(tDest, "/IgnoreList");
	this->CreateServiceObj(tDest, &CSkypeProto::OpenIgnoreListCommand);
	mi.ptszName = LPGENT("Ignore list");
	mi.position = 200000 + SMI_IGNORE_LIST;
	mi.icolibItem = CSkypeProto::GetIconHandle("ignore");
	::Menu_AddProtoMenuItem(&mi);
}