#include "common.h"

void CDropbox::InitMenus()
{
	CLISTMENUITEM mi = { 0 };
	mi.cbSize = sizeof(CLISTMENUITEM);
	mi.flags = CMIF_TCHAR;

	mi.pszService = MODULE"/SendFilesToDropbox";
	mi.ptszName = LPGENT("Send files to Dropbox");
	mi.position = -2000020000 + CMI_SEND_FILES;
	mi.icolibItem = LoadSkinnedIconHandle(SKINICON_EVENT_FILE);
	ContactMenuItems[CMI_SEND_FILES] = Menu_AddContactMenuItem(&mi);
	CreateServiceFunction(mi.pszService, SendFilesToDropbox);

	mi.pszService = MODULE"/RequestAuthorization";
	mi.ptszName = LPGENT("Request authorization");
	mi.position = 1000030000 + CMI_AUTH_REQUEST;
	mi.icolibItem = LoadSkinnedIconHandle(SKINICON_AUTH_REQUEST);
	ContactMenuItems[CMI_AUTH_REQUEST] = Menu_AddContactMenuItem(&mi);
	CreateServiceFunction(mi.pszService, RequestApiAuthorization);

	mi.pszService = MODULE"/RevokeAuthorization";
	mi.ptszName = LPGENT("Revoke authorization");
	mi.position = 1000030000 + CMI_AUTH_REVOKE;
	mi.icolibItem = LoadSkinnedIconHandle(SKINICON_AUTH_REVOKE);
	ContactMenuItems[CMI_AUTH_REVOKE] = Menu_AddContactMenuItem(&mi);
	CreateServiceFunction(mi.pszService, RevokeApiAuthorization);
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
	if (!hContact)
		return 0;

	Menu_DisableItem(ContactMenuItems[CMI_SEND_FILES], FALSE);

	Menu_ShowItem(ContactMenuItems[CMI_AUTH_REQUEST], FALSE);
	Menu_ShowItem(ContactMenuItems[CMI_AUTH_REVOKE], FALSE);
	Menu_ShowItem(ContactMenuItems[CMI_SEND_FILES], FALSE);

	WORD status = db_get_w(hContact, GetContactProto(hContact), "Status", ID_STATUS_OFFLINE);

	if (hContact == GetDefaultContact())
	{
		if (!HasAccessToken())
			Menu_ShowItem(ContactMenuItems[CMI_AUTH_REQUEST], TRUE);
		else
			Menu_ShowItem(ContactMenuItems[CMI_AUTH_REVOKE], TRUE);
	}
	else if (status != ID_STATUS_OFFLINE && HasAccessToken())
		Menu_ShowItem(ContactMenuItems[CMI_SEND_FILES], TRUE);

	if (INSTANCE->hContactTransfer)
		Menu_DisableItem(ContactMenuItems[CMI_SEND_FILES], TRUE);

	return 0;
}