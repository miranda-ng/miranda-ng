#include "skype_proto.h"

HANDLE CSkypeProto::hChooserMenu;
HANDLE CSkypeProto::hPrebuildMenuHook;
HANDLE CSkypeProto::g_hContactMenuSvc[CMITEMS_COUNT];
HANDLE CSkypeProto::g_hContactMenuItems[CMITEMS_COUNT];

INT_PTR CSkypeProto::MenuChooseService(WPARAM wParam, LPARAM lParam)
{
	if (lParam)
		*(void**)lParam = (void*)wParam;

	return 0;
}

static void sttEnableMenuItem(HANDLE hMenuItem, BOOL bEnable)
{
	CLISTMENUITEM clmi = {0};
	clmi.cbSize = sizeof(CLISTMENUITEM);
	clmi.flags = CMIM_FLAGS;
	if (!bEnable)
		clmi.flags |= CMIF_HIDDEN;

	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenuItem, (LPARAM)&clmi);
}

int CSkypeProto::OnPrebuildContactMenu(WPARAM wParam, LPARAM)
{
	HANDLE hContact = (HANDLE)wParam;
	
	if (hContact == NULL)
		return 0;

	if (this->IsOnline() && !this->GetSettingByte(hContact, "ChatRoom"))
	{
		bool ctrlPressed = (GetKeyState(VK_CONTROL) & 0x8000) != 0;

		bool authNeed = this->GetSettingByte(hContact, "Auth") > 0;
		bool grantNeed = this->GetSettingByte(hContact, "Grant") > 0;

		sttEnableMenuItem( g_hContactMenuItems[CMI_AUTH_REQUEST], ctrlPressed || authNeed);
		sttEnableMenuItem( g_hContactMenuItems[CMI_AUTH_GRANT], ctrlPressed || grantNeed);
		sttEnableMenuItem( g_hContactMenuItems[CMI_AUTH_REVOKE], ctrlPressed || (!grantNeed && !authNeed));
	}

	return 0;
}

CSkypeProto* CSkypeProto::GetInstanceByHContact(HANDLE hContact)
{
	char* proto = (char*)::CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0);
	if (proto == NULL)
		return NULL;

	for (int i = 0; i < CSkypeProto::instanceList.getCount(); i++)
		if (!strcmp(proto, CSkypeProto::instanceList[i]->m_szModuleName))
			return CSkypeProto::instanceList[i];

	return NULL;
}

template<int (__cdecl CSkypeProto::*Scn)(WPARAM, LPARAM)>
INT_PTR GlobalService(WPARAM wParam, LPARAM lParam)
{
	CSkypeProto *ppro = CSkypeProto::GetInstanceByHContact((HANDLE)wParam);
	return ppro ? (ppro->*Scn)(wParam, lParam) : 0;
}

int CSkypeProto::RequestAuth(WPARAM wParam, LPARAM lParam)
{
	// todo: set default auth request
	return this->AuthRequest((HANDLE)wParam, LPGENT("Hi! I\'d like to add you to my contact list"));
}

int CSkypeProto::GrantAuth(WPARAM wParam, LPARAM lParam)
{
	CContact::Ref contact;
	HANDLE hContact = (HANDLE)wParam;
	SEString sid(::mir_u2a(this->GetSettingString(hContact, "sid")));
	if (g_skype->GetContact(sid, contact))
	{
		if (contact->SetBuddyStatus(true))
		{
			this->DeleteSetting(hContact, "Auth");
			this->DeleteSetting(hContact, "Grant");
		}
	}

	return 0;
}

int CSkypeProto::RevokeAuth(WPARAM wParam, LPARAM lParam)
{
	CContact::Ref contact;
	HANDLE hContact = (HANDLE)wParam;
	SEString sid(::mir_u2a(this->GetSettingString(hContact, "sid")));
	if (g_skype->GetContact(sid, contact))
	{
		if (contact->SetBuddyStatus(false))
		{
			this->SetSettingByte(hContact, "Grant", 1);
		}
		this->contactList.remove_val(contact);
	}

	return 0;
}

int CSkypeProto::PrebuildContactMenu(WPARAM wParam, LPARAM lParam)
{
	for (size_t i=0; i<SIZEOF(g_hContactMenuItems); i++)
	{
		sttEnableMenuItem(g_hContactMenuItems[i], false);
	}

	CSkypeProto* ppro = CSkypeProto::GetInstanceByHContact((HANDLE)wParam);
	return (ppro) ? ppro->OnPrebuildContactMenu(wParam, lParam) : 0;
}

void  CSkypeProto::InitMenus()
{
	CSkypeProto::hPrebuildMenuHook = ::HookEvent(
		ME_CLIST_PREBUILDCONTACTMENU, 
		CSkypeProto::PrebuildContactMenu);

	TMenuParam mnu = {0};
	mnu.cbSize = sizeof(mnu);
	mnu.name = "SkypeAccountChooser";
	mnu.ExecService = "Skype/MenuChoose";
	hChooserMenu = (HANDLE)::CallService(MO_CREATENEWMENUOBJECT, 0, (LPARAM)&mnu);

	//////////////////////////////////////////////////////////////////////////////////////
	// Contact menu initialization

	CLISTMENUITEM mi = { 0 };
	mi.cbSize = sizeof(CLISTMENUITEM);

	// "Request authorization"
	mi.ptszName = LPGENT("Request authorization");
	mi.flags = CMIF_ICONFROMICOLIB | CMIF_TCHAR;
	mi.position = -2000001000;
	mi.icolibItem = CSkypeProto::GetIconHandle("authRequest");
	mi.pszService = "Skype/ReqAuth";
	g_hContactMenuItems[CMI_AUTH_REQUEST] = ::Menu_AddContactMenuItem(&mi);
	g_hContactMenuSvc[CMI_AUTH_REQUEST] = ::CreateServiceFunction(mi.pszService, GlobalService<&CSkypeProto::RequestAuth>);

	// "Grant authorization"
	mi.pszService = "Skype/GrantAuth";
	mi.ptszName = LPGENT("Grant authorization");
	mi.position = -2000001001;
	mi.icolibItem = CSkypeProto::GetIconHandle("authGrant");
	g_hContactMenuItems[CMI_AUTH_GRANT] = ::Menu_AddContactMenuItem(&mi);
	g_hContactMenuSvc[CMI_AUTH_GRANT] = ::CreateServiceFunction(mi.pszService, GlobalService<&CSkypeProto::GrantAuth>);

	// Revoke auth
	mi.pszService = "Skype/RevokeAuth";
	mi.ptszName = LPGENT("Revoke authorization");
	mi.position = -2000001002;
	mi.icolibItem = CSkypeProto::GetIconHandle("authRevoke");
	g_hContactMenuItems[CMI_AUTH_REVOKE] = ::Menu_AddContactMenuItem(&mi);
	g_hContactMenuSvc[CMI_AUTH_REVOKE] = ::CreateServiceFunction(mi.pszService, GlobalService<&CSkypeProto::RevokeAuth>);
}

void  CSkypeProto::UninitMenus()
{
	::UnhookEvent(CSkypeProto::hPrebuildMenuHook);
}