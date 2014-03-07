#include "common.h"

void CDropbox::InitializeMenus()
{
	CLISTMENUITEM mi = { 0 };
	mi.cbSize = sizeof(CLISTMENUITEM);
	mi.flags = CMIF_TCHAR;

	mi.pszService = MODULE"/SendFilesToDropbox";
	mi.ptszName = LPGENT("Send files to Dropbox");
	mi.position = -2000020000 + CMI_SEND_FILES;
	mi.icolibItem = LoadSkinnedIconHandle(SKINICON_EVENT_FILE);
	contactMenuItems[CMI_SEND_FILES] = Menu_AddContactMenuItem(&mi);
	CreateServiceFunctionObj(mi.pszService, SendFilesToDropbox, this);
}

int CDropbox::OnPrebuildContactMenu(void *obj, WPARAM hContact, LPARAM lParam)
{
	CDropbox *instance = (CDropbox*)obj;

	if (!hContact)
		return 0;

	Menu_ShowItem(instance->contactMenuItems[CMI_SEND_FILES], FALSE);

	WORD status = db_get_w(hContact, GetContactProto(hContact), "Status", ID_STATUS_OFFLINE);

	if (hContact != instance->GetDefaultContact() && status != ID_STATUS_OFFLINE && instance->HasAccessToken() && !instance->hTransferContact)
		Menu_ShowItem(instance->contactMenuItems[CMI_SEND_FILES], TRUE);

	return 0;
}