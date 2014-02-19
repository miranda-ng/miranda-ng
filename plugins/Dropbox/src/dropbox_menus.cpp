#include "dropbox.h"

HGENMENU CDropbox::ContactMenuItems[CMI_MAX];

void CDropbox::InitMenus()
{
	CLISTMENUITEM mi = { 0 };
	mi.cbSize = sizeof(CLISTMENUITEM);
	mi.flags = CMIF_TCHAR;

	mi.pszService = MODULE"/RequeriedAccess";
	mi.ptszName = LPGENT("Requeried access");
	mi.position = -201001000 + CMI_API_ACCESS_REQUERIED;
	mi.icolibItem = LoadSkinnedIconHandle(SKINICON_AUTH_REQUEST);
	ContactMenuItems[CMI_API_ACCESS_REQUERIED] = Menu_AddContactMenuItem(&mi);
	CreateServiceFunction(mi.pszService, RequeriedApiAccess);
}