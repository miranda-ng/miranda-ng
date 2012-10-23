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

int CSkypeProto::OnPrebuildContactMenu(WPARAM wParam, LPARAM)
{
	HANDLE hContact = (HANDLE)wParam;
	
	if (hContact == NULL)
		return 0;

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

INT_PTR SkypeMenuHandleRequestAuth(WPARAM wParam, LPARAM lParam)
{
	CSkypeProto* ppro = CSkypeProto::GetInstanceByHContact((HANDLE)wParam);
	return(ppro) ? ppro->OnMenuHandleRequestAuth(wParam, lParam) : 0;
}

INT_PTR SkypeMenuHandleGrantAuth(WPARAM wParam, LPARAM lParam)
{
	CSkypeProto* ppro = CSkypeProto::GetInstanceByHContact((HANDLE)wParam);
	return(ppro) ? ppro->OnMenuHandleGrantAuth(wParam, lParam) : 0;
}

INT_PTR SkypeMenuRevokeAuth(WPARAM wParam, LPARAM lParam)
{
	CSkypeProto* ppro = CSkypeProto::GetInstanceByHContact((HANDLE)wParam);
	return(ppro) ? ppro->OnMenuRevokeAuth(wParam, lParam) : 0;
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

int CSkypeProto::PrebuildContactMenu(WPARAM wParam, LPARAM lParam)
{
	sttEnableMenuItem( g_hContactMenuItems[CMI_AUTH_REQUEST], FALSE );
	sttEnableMenuItem( g_hContactMenuItems[CMI_AUTH_GRANT], FALSE );
	sttEnableMenuItem( g_hContactMenuItems[CMI_AUTH_REVOKE], FALSE );
	/*sttEnableMenuItem( g_hMenuCommands, FALSE );
	sttEnableMenuItem( g_hMenuSendNote, FALSE );
	sttEnableMenuItem( g_hMenuConvert, FALSE );
	sttEnableMenuItem( g_hMenuRosterAdd, FALSE );
	sttEnableMenuItem( g_hMenuLogin, FALSE );
	sttEnableMenuItem( g_hMenuRefresh, FALSE );
	sttEnableMenuItem( g_hMenuAddBookmark, FALSE );
	sttEnableMenuItem( g_hMenuResourcesRoot, FALSE );
	sttEnableMenuItem( g_hMenuDirectPresence[0], FALSE );*/

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
	mi.icolibItem = GetIconHandle("authReuest");
	mi.pszService = "Skype/ReqAuth";
	g_hContactMenuItems[CMI_AUTH_REQUEST] = Menu_AddContactMenuItem(&mi);
	g_hContactMenuSvc[CMI_AUTH_REQUEST] = CreateServiceFunction(mi.pszService, SkypeMenuHandleRequestAuth);

	// "Grant authorization"
	mi.pszService = "Skype/GrantAuth";
	mi.ptszName = LPGENT("Grant authorization");
	mi.position = -2000001001;
	mi.icolibItem = GetIconHandle("authGrant");
	g_hContactMenuItems[CMI_AUTH_GRANT] = Menu_AddContactMenuItem(&mi);
	g_hContactMenuSvc[CMI_AUTH_GRANT] = CreateServiceFunction(mi.pszService, SkypeMenuHandleGrantAuth);

	// Revoke auth
	mi.pszService = "Skype/RevokeAuth";
	mi.ptszName = LPGENT("Revoke authorization");
	mi.position = -2000001002;
	mi.icolibItem = GetIconHandle("authRevoke");
	g_hContactMenuItems[CMI_AUTH_REVOKE] = Menu_AddContactMenuItem(&mi);
	g_hContactMenuSvc[CMI_AUTH_REVOKE] = CreateServiceFunction(mi.pszService, SkypeMenuRevokeAuth);
}

void  CSkypeProto::UninitMenus()
{
	::UnhookEvent(CSkypeProto::hPrebuildMenuHook);
}