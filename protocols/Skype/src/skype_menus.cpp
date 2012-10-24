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

	if (this->IsOnline() && !DBGetContactSettingByte(hContact, m_szModuleName, "ChatRoom", 0))
	{
		bool ctrlPressed = (GetKeyState(VK_CONTROL) & 0x8000) != 0;

		//BYTE type = DBGetContactSettingByte(hContact, m_szModuleName, FACEBOOK_KEY_CONTACT_TYPE, 0);

		sttEnableMenuItem( g_hContactMenuItems[CMI_AUTH_REQUEST], ctrlPressed /*|| type == FACEBOOK_CONTACT_NONE || !type */);
		sttEnableMenuItem( g_hContactMenuItems[CMI_AUTH_GRANT], ctrlPressed /*|| type == FACEBOOK_CONTACT_APPROVE */);
		sttEnableMenuItem( g_hContactMenuItems[CMI_AUTH_REVOKE], ctrlPressed /*|| type == FACEBOOK_CONTACT_FRIEND */);
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
	if (this->IsOnline() && wParam)
	{
		HANDLE hContact = (HANDLE)wParam;
		TCHAR* szMessage = (TCHAR*)lParam;
		CContact::Ref contact;
		SEString sid(::mir_u2a(this->GetSettingString(hContact, "sid")));
		g_skype->GetContact(sid, contact);

		contact->SendAuthRequest(::mir_u2a(szMessage));
		this->DeleteSetting(hContact, "Grant");
		
		return 0;
	}

	return 1;
}

int CSkypeProto::GrantAuth(WPARAM wParam, LPARAM lParam)
{
	CContact::Ref contact;
	HANDLE hContact = (HANDLE)wParam;
	SEString sid(::mir_u2a(this->GetSettingString(hContact, "sid")));
	g_skype->GetContact(sid, contact);
	contact->SetBuddyStatus(true/*Contact::AUTHORIZED_BY_ME*/);

	return 0;
}

int CSkypeProto::RevokeAuth(WPARAM wParam, LPARAM lParam)
{
	CContact::Ref contact;
	HANDLE hContact = (HANDLE)wParam;
	SEString sid(::mir_u2a(this->GetSettingString(hContact, "sid")));
	g_skype->GetContact(sid, contact);
	contact->SetBuddyStatus(false/*CContact::BLOCKED_BY_ME*/);

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
	g_hContactMenuItems[CMI_AUTH_REQUEST] = Menu_AddContactMenuItem(&mi);
	g_hContactMenuSvc[CMI_AUTH_REQUEST] = CreateServiceFunction(mi.pszService, GlobalService<&CSkypeProto::RequestAuth>);

	// "Grant authorization"
	mi.pszService = "Skype/GrantAuth";
	mi.ptszName = LPGENT("Grant authorization");
	mi.position = -2000001001;
	mi.icolibItem = CSkypeProto::GetIconHandle("authGrant");
	g_hContactMenuItems[CMI_AUTH_GRANT] = Menu_AddContactMenuItem(&mi);
	g_hContactMenuSvc[CMI_AUTH_GRANT] = CreateServiceFunction(mi.pszService, GlobalService<&CSkypeProto::GrantAuth>);

	// Revoke auth
	mi.pszService = "Skype/RevokeAuth";
	mi.ptszName = LPGENT("Revoke authorization");
	mi.position = -2000001002;
	mi.icolibItem = CSkypeProto::GetIconHandle("authRevoke");
	g_hContactMenuItems[CMI_AUTH_REVOKE] = Menu_AddContactMenuItem(&mi);
	g_hContactMenuSvc[CMI_AUTH_REVOKE] = CreateServiceFunction(mi.pszService, GlobalService<&CSkypeProto::RevokeAuth>);
}

void  CSkypeProto::UninitMenus()
{
	::UnhookEvent(CSkypeProto::hPrebuildMenuHook);
}