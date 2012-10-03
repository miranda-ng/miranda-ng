#include "skype_proto.h"

HANDLE CSkypeProto::hPrebuildMenuHook;

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

	for (int i = 0; i < g_Instances.getCount(); i++)
		if (!strcmp(proto, g_Instances[i]->m_szModuleName))
			return g_Instances[i];

	return NULL;
}

int CSkypeProto::PrebuildContactMenu(WPARAM wParam, LPARAM lParam)
{
	/*sttEnableMenuItem( g_hMenuRequestAuth, FALSE );
	sttEnableMenuItem( g_hMenuGrantAuth, FALSE );
	sttEnableMenuItem( g_hMenuRevokeAuth, FALSE );
	sttEnableMenuItem( g_hMenuCommands, FALSE );
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
	CSkypeProto::hPrebuildMenuHook = ::HookEvent(ME_CLIST_PREBUILDCONTACTMENU, CSkypeProto::PrebuildContactMenu);

	//List_InsertPtr( &arServices, CreateServiceFunction( "Jabber/MenuChoose", JabberMenuChooseService ));

	/*TMenuParam mnu = {0};
	mnu.cbSize = sizeof(mnu);
	mnu.name = "JabberAccountChooser";
	mnu.ExecService = "Jabber/MenuChoose";
	hChooserMenu = (HANDLE)CallService( MO_CREATENEWMENUOBJECT, 0, (LPARAM)&mnu );

	TMO_MenuItem tmi = { 0 };
	tmi.cbSize = sizeof( tmi );
	tmi.flags = CMIF_ICONFROMICOLIB;
	tmi.pszName = "Cancel";
	tmi.position = 9999999;
	tmi.hIcolibItem = LoadSkinnedIconHandle(SKINICON_OTHER_DELETE);
	CallService( MO_ADDNEWMENUITEM, (WPARAM)hChooserMenu, ( LPARAM )&tmi );*/
}

void  CSkypeProto::UninitMenus()
{
	::UnhookEvent(CSkypeProto::hPrebuildMenuHook);
}