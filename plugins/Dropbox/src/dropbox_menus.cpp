#include "stdafx.h"

INT_PTR CDropbox::SendFilesToDropboxCommand(void *obj, WPARAM hContact, LPARAM)
{
	CDropbox *instance = (CDropbox*)obj;
	if (!instance->HasAccessToken())
		return 1;

	instance->hTransferContact = hContact;
	instance->hTransferWindow = (HWND)CallService(MS_FILE_SENDFILE, instance->GetDefaultContact(), 0);

	DisableSrmmButton(hContact);
	return 0;
}

void CDropbox::InitializeMenus()
{
	CLISTMENUITEM mi = { 0 };
	mi.cbSize = sizeof(CLISTMENUITEM);
	mi.pszService = MODULE"/SendFilesToDropbox";
	mi.pszName = LPGEN("Upload files to Dropbox");
	mi.position = -2000020000 + CMI_SEND_FILES;
	mi.icolibItem = GetIconHandleByName("upload");
	contactMenuItems[CMI_SEND_FILES] = Menu_AddContactMenuItem(&mi);
	CreateServiceFunctionObj(mi.pszService, SendFilesToDropboxCommand, this);
}

int CDropbox::OnPrebuildContactMenu(WPARAM hContact, LPARAM)
{
	if (!hContact)
		return 0;

	BOOL bShow = FALSE;

	if (HasAccessToken() && !hTransferContact && hContact != GetDefaultContact())
	{
		char *proto = GetContactProto(hContact);
		bool isContact = db_get_b(hContact, proto, "ChatRoom", 0) == 0;
		if (proto && isContact)
		{
			bool isProtoOnline = CallProtoService(proto, PS_GETSTATUS, 0, 0) > ID_STATUS_OFFLINE;
			WORD status = db_get_w(hContact, proto, "Status", ID_STATUS_OFFLINE);
			bool canSendOffline = (CallProtoService(proto, PS_GETCAPS, PFLAGNUM_4, 0) & PF4_IMSENDOFFLINE) > 0;
			if (isProtoOnline && (status != ID_STATUS_OFFLINE || canSendOffline))
				bShow = TRUE;
		}
	}

	Menu_ShowItem(contactMenuItems[CMI_SEND_FILES], bShow);
	return 0;
}
