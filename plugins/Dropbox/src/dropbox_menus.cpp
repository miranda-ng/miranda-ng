#include "common.h"

HGENMENU CDropbox::ContactMenuItems[CMI_MAX];

void CDropbox::InitMenus()
{
	CLISTMENUITEM mi = { 0 };
	mi.cbSize = sizeof(CLISTMENUITEM);
	mi.flags = CMIF_TCHAR;

	mi.pszService = MODULE"/SendFilesToDropbox";
	mi.ptszName = LPGENT("Send files to Dropbox");
	mi.position = -201002000 + CMI_SEND_FILES;
	mi.icolibItem = LoadSkinnedIconHandle(SKINICON_EVENT_FILE);
	ContactMenuItems[CMI_SEND_FILES] = Menu_AddContactMenuItem(&mi);
	CreateServiceFunction(mi.pszService, SendFilesToDropbox);

	mi.pszService = MODULE"/RequestAuthorization";
	mi.ptszName = LPGENT("Request Authorization");
	mi.position = -201001000 + CMI_API_REQUEST_AUTH;
	mi.icolibItem = LoadSkinnedIconHandle(SKINICON_AUTH_REQUEST);
	ContactMenuItems[CMI_API_REQUEST_AUTH] = Menu_AddContactMenuItem(&mi);
	CreateServiceFunction(mi.pszService, RequestApiAuthorization);
}

void CDropbox::Menu_DisableItem(HGENMENU hMenuItem, BOOL bDisable)
{
	CLISTMENUITEM clmi = { sizeof(clmi) };
	clmi.cbSize = sizeof(CLISTMENUITEM);
	clmi.flags = CMIM_FLAGS;
	if (bDisable)
		clmi.flags |= CMIF_GRAYED;

	Menu_ModifyItem(hMenuItem, &clmi);
}

int CDropbox::OnPrebuildContactMenu(WPARAM hContact, LPARAM lParam)
{
	if ( !hContact)
		return 0;

	//bool ctrlPressed = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
	WORD status = CallContactService(hContact, PS_GETSTATUS, 0, 0);

	if (hContact == INSTANCE->GetDefaultContact() || status == ID_STATUS_OFFLINE)
		Menu_ShowItem(INSTANCE->ContactMenuItems[CMI_SEND_FILES], FALSE);
	else
	{
		Menu_DisableItem(INSTANCE->ContactMenuItems[CMI_SEND_FILES], INSTANCE->hContactTransfer);
		if (strcmp(GetContactProto(hContact), MODULE))
		{
			Menu_ShowItem(INSTANCE->ContactMenuItems[CMI_API_REQUEST_AUTH], FALSE);
		}
	}

	return 0;
}