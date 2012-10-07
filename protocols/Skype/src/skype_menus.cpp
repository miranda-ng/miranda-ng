#include "skype_proto.h"

HANDLE CSkypeProto::hChooserMenu;
HANDLE CSkypeProto::hPrebuildMenuHook;

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
	CSkypeProto::hPrebuildMenuHook = ::HookEvent(
		ME_CLIST_PREBUILDCONTACTMENU, 
		CSkypeProto::PrebuildContactMenu);

	TMenuParam mnu = {0};
	mnu.cbSize = sizeof(mnu);
	mnu.name = "SkypeAccountChooser";
	mnu.ExecService = "Skype/MenuChoose";
	hChooserMenu = (HANDLE)::CallService(MO_CREATENEWMENUOBJECT, 0, (LPARAM)&mnu);
}

void  CSkypeProto::UninitMenus()
{
	::UnhookEvent(CSkypeProto::hPrebuildMenuHook);
}