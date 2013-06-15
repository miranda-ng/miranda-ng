#include "skype.h"

HANDLE CSkypeProto::hChooserMenu;
HANDLE CSkypeProto::contactMenuItems[CMI_TEMS_COUNT];
HANDLE CSkypeProto::contactMenuServices[CMI_TEMS_COUNT];

INT_PTR CSkypeProto::MenuChooseService(WPARAM wParam, LPARAM lParam)
{
	if (lParam)
		*(void**)lParam = (void*)wParam;

	return 0;
}

void CSkypeProto::ShowMenuItem(HANDLE hMenuItem, BOOL show)
{
	CLISTMENUITEM clmi = {0};
	clmi.cbSize = sizeof(CLISTMENUITEM);
	clmi.flags = CMIM_FLAGS;
	if (!show)
		clmi.flags |= CMIF_HIDDEN;

	::CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenuItem, (LPARAM)&clmi);
}

int CSkypeProto::OnPrebuildContactMenu(WPARAM wParam, LPARAM)
{
	HANDLE hContact = (HANDLE)wParam;
	
	if (hContact == NULL)
		return 0;

	if (this->IsOnline() && !this->IsChatRoom(hContact))
	{
		bool ctrlPressed = (::GetKeyState(VK_CONTROL) & 0x8000) != 0;

		bool authNeed = ::db_get_b(hContact, this->m_szModuleName, "Auth", 0) > 0;
		bool grantNeed = ::db_get_b(hContact, this->m_szModuleName, "Grant", 0) > 0;

		CSkypeProto::ShowMenuItem(CSkypeProto::contactMenuItems[CMI_AUTH_REQUEST], ctrlPressed || authNeed);
		CSkypeProto::ShowMenuItem(CSkypeProto::contactMenuItems[CMI_AUTH_GRANT], ctrlPressed || grantNeed);
		CSkypeProto::ShowMenuItem(CSkypeProto::contactMenuItems[CMI_AUTH_REVOKE], ctrlPressed || (!grantNeed && !authNeed));
	}

	return 0;
}

CSkypeProto* CSkypeProto::GetInstanceByHContact(HANDLE hContact)
{
	char *proto = (char *)::CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);

	if (proto == NULL)
		return NULL;

	for (int i = 0; i < CSkypeProto::instanceList.getCount(); i++)
		if ( !::strcmp(proto, CSkypeProto::instanceList[i]->m_szModuleName))
			return CSkypeProto::instanceList[i];

	return NULL;
}

template<int (__cdecl CSkypeProto::*Service)(WPARAM, LPARAM)>
INT_PTR GlobalService(WPARAM wParam, LPARAM lParam)
{
	CSkypeProto *ppro = CSkypeProto::GetInstanceByHContact((HANDLE)wParam);
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
	SEString sid(::mir_u2a(mir_ptr<wchar_t>(::db_get_wsa(hContact, this->m_szModuleName, SKYPE_SETTINGS_SID))));
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
	SEString sid(::mir_u2a(mir_ptr<wchar_t>(::db_get_wsa(hContact, this->m_szModuleName, SKYPE_SETTINGS_SID))));
	if (this->GetContact(sid, contact))
	{
		if (contact->SetBuddyStatus(false))
		{
			::db_set_b(hContact, this->m_szModuleName, "Grant", 1);
		}
		this->contactList.remove_val(contact);
	}

	return 0;
}

INT_PTR CSkypeProto::InviteCommand(WPARAM, LPARAM)
{
	this->StartChat();

	return 0;
}

int CSkypeProto::PrebuildContactMenu(WPARAM wParam, LPARAM lParam)
{
	for (size_t i = 0; i < SIZEOF(CSkypeProto::contactMenuItems); i++)
		CSkypeProto::ShowMenuItem(CSkypeProto::contactMenuItems[i], false);

	CSkypeProto* ppro = CSkypeProto::GetInstanceByHContact((HANDLE)wParam);
	return (ppro) ? ppro->OnPrebuildContactMenu(wParam, lParam) : 0;
}

void  CSkypeProto::InitMenus()
{
	TMenuParam mnu = {0};
	mnu.cbSize = sizeof(mnu);
	mnu.name = "SkypeAccountChooser";
	mnu.ExecService = "Skype/MenuChoose";
	hChooserMenu = (HANDLE)::CallService(MO_CREATENEWMENUOBJECT, 0, (LPARAM)&mnu);

	//////////////////////////////////////////////////////////////////////////////////////
	// Contact menu initialization

	CLISTMENUITEM mi = { 0 };
	mi.cbSize = sizeof(CLISTMENUITEM);
	mi.flags = CMIF_TCHAR;
	
	// "Revoke authorization"
	mi.pszService = "Skype/RevokeAuth";
	mi.ptszName = LPGENT("Revoke authorization");
	mi.position = -2000001000 - CMI_AUTH_REVOKE;
	mi.icolibItem = ::LoadSkinnedIconHandle(SKINICON_AUTH_REVOKE);
	CSkypeProto::contactMenuItems[CMI_AUTH_REVOKE] = ::Menu_AddContactMenuItem(&mi);
	CSkypeProto::contactMenuServices[CMI_AUTH_REVOKE] = ::CreateServiceFunction(mi.pszService, GlobalService<&CSkypeProto::RevokeAuth>);

	// "Request authorization"
	mi.pszService = "Skype/RequestAuth";
	mi.ptszName = LPGENT("Request authorization");
	mi.position = -2000001000 - CMI_AUTH_REQUEST;
	mi.icolibItem = ::LoadSkinnedIconHandle(SKINICON_AUTH_REQUEST);
	CSkypeProto::contactMenuItems[CMI_AUTH_REQUEST] = ::Menu_AddContactMenuItem(&mi);
	CSkypeProto::contactMenuServices[CMI_AUTH_REQUEST] = ::CreateServiceFunction(mi.pszService, GlobalService<&CSkypeProto::RequestAuth>);

	// "Grant authorization"
	mi.pszService = "Skype/GrantAuth";
	mi.ptszName = LPGENT("Grant authorization");
	mi.position = -2000001000 - CMI_AUTH_GRANT;
	mi.icolibItem = ::LoadSkinnedIconHandle(SKINICON_AUTH_GRANT);
	CSkypeProto::contactMenuItems[CMI_AUTH_GRANT] = ::Menu_AddContactMenuItem(&mi);
	CSkypeProto::contactMenuServices[CMI_AUTH_GRANT] = ::CreateServiceFunction(mi.pszService, GlobalService<&CSkypeProto::GrantAuth>);
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
	::strcpy(tDest, "/InviteCommand");
	this->CreateServiceObj(tDest, &CSkypeProto::InviteCommand);
	mi.ptszName = LPGENT("Create conference");
	mi.position = 200000 + SMI_CHAT_INVITE;
	mi.icolibItem = CSkypeProto::GetIconHandle("confInvite");
	::Menu_AddProtoMenuItem(&mi);
}