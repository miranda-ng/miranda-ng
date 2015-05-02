#include "stdafx.h"

INT_PTR CDropbox::SendFilesToDropboxCommand(void *obj, WPARAM hContact, LPARAM)
{
	CDropbox *instance = (CDropbox*)obj;

	if (!instance->HasAccessToken())
		return 1;

	instance->hTransferContact = hContact;

	HWND hwnd = (HWND)CallService(MS_FILE_SENDFILE, instance->GetDefaultContact(), 0);

	instance->dcftp[hwnd] = hContact;

	BBButton bbd = { sizeof(bbd) };
	bbd.pszModuleName = MODULE;
	bbd.dwButtonID = BBB_ID_FILE_SEND;
	bbd.bbbFlags = BBSF_DISABLED;

	CallService(MS_BB_SETBUTTONSTATE, hContact, (LPARAM)&bbd);

	return 0;
}

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
	CreateServiceFunctionObj(mi.pszService, SendFilesToDropboxCommand, this);
}

int CDropbox::OnPrebuildContactMenu(void *obj, WPARAM hContact, LPARAM)
{
	CDropbox *instance = (CDropbox*)obj;

	if (!hContact)
		return 0;

	Menu_ShowItem(instance->contactMenuItems[CMI_SEND_FILES], FALSE);

	if (!instance->HasAccessToken())
		return 0;

	char *proto = GetContactProto(hContact);
	bool isProtoOnline = CallProtoService(proto, PS_GETSTATUS, 0, 0) > ID_STATUS_OFFLINE;
	WORD status = db_get_w(hContact, proto, "Status", ID_STATUS_OFFLINE);
	bool canSendOffline = (CallProtoService(proto, PS_GETCAPS, PFLAGNUM_4, 0) & PF4_IMSENDOFFLINE) > 0;

	if (hContact != instance->GetDefaultContact() && isProtoOnline && (status != ID_STATUS_OFFLINE || canSendOffline) && instance->HasAccessToken() && !instance->hTransferContact)
		Menu_ShowItem(instance->contactMenuItems[CMI_SEND_FILES], TRUE);

	return 0;
}