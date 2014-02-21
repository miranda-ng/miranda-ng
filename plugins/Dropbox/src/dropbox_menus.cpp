#include "common.h"

HGENMENU CDropbox::ContactMenuItems[CMI_MAX];

void CDropbox::InitMenus()
{
	CLISTMENUITEM mi = { 0 };
	mi.cbSize = sizeof(CLISTMENUITEM);
	mi.flags = CMIF_TCHAR;

	/*mi.pszService = MODULE"/SendFilesToDropbox";
	mi.ptszName = LPGENT("Send files to Dropbox");
	mi.position = -201000000 + CMI_SEND_FILES;
	mi.icolibItem = LoadSkinnedIconHandle(SKINICON_EVENT_FILE);
	ContactMenuItems[CMI_SEND_FILES] = Menu_AddContactMenuItem(&mi);
	CreateServiceFunction(mi.pszService, SendFilesToDropbox);*/

	mi.pszService = MODULE"/RequestAuthorization";
	mi.ptszName = LPGENT("Request Authorization");
	mi.position = -201001000 + CMI_API_REQUEST_AUTH;
	mi.icolibItem = LoadSkinnedIconHandle(SKINICON_AUTH_REQUEST);
	ContactMenuItems[CMI_API_REQUEST_AUTH] = Menu_AddContactMenuItem(&mi);
	CreateServiceFunction(mi.pszService, RequestApiAuthorization);
}