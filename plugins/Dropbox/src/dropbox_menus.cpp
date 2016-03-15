#include "stdafx.h"

INT_PTR CDropbox::SendFilesToDropboxCommand(void *obj, WPARAM hContact, LPARAM)
{
	CDropbox *instance = (CDropbox*)obj;
	if (!instance->HasAccessToken())
		return 1;

	auto it = instance->interceptedContacts.find(hContact);
	if (it == instance->interceptedContacts.end())
	{
		HWND hwnd = (HWND)CallService(MS_FILE_SENDFILE, hContact, 0);
		instance->interceptedContacts[hContact] = hwnd;
	}
	else
		SetActiveWindow(it->second);

	return 0;
}

void CDropbox::InitializeMenus()
{
	CMenuItem mi;
	SET_UID(mi, 0x19af0aaf, 0x37d0, 0x4b88, 0xa5, 0x92, 0xf1, 0x6f, 0x54, 0xfd, 0x67, 0xb5);
	mi.pszService = MODULE"/SendFilesToDropbox";
	mi.name.a = LPGEN("Upload files to Dropbox");
	mi.position = -2000020000 + CMI_SEND_FILES;
	mi.hIcolibItem = GetIconHandleByName("upload");
	contactMenuItems[CMI_SEND_FILES] = Menu_AddContactMenuItem(&mi);
	CreateServiceFunctionObj(mi.pszService, SendFilesToDropboxCommand, this);
}

int CDropbox::OnPrebuildContactMenu(WPARAM hContact, LPARAM)
{
	bool bShow = false;

	char *proto = GetContactProto(hContact);
	if (proto != NULL) {
		bool bHasIM = (CallProtoService(proto, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_IMSEND) != 0;
		if (bHasIM && HasAccessToken() && hContact != GetDefaultContact() && !IsAccountIntercepted(proto)) {
			bool isProtoOnline = CallProtoService(proto, PS_GETSTATUS, 0, 0) > ID_STATUS_OFFLINE;
			WORD status = db_get_w(hContact, proto, "Status", ID_STATUS_OFFLINE);
			bool canSendOffline = (CallProtoService(proto, PS_GETCAPS, PFLAGNUM_4, 0) & PF4_IMSENDOFFLINE) > 0;
			if (isProtoOnline && (status != ID_STATUS_OFFLINE || canSendOffline))
				bShow = true;
		}
	}

	Menu_ShowItem(contactMenuItems[CMI_SEND_FILES], bShow);
	return 0;
}
