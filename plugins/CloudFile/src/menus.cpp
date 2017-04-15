#include "stdafx.h"

HGENMENU hContactMenu;

INT_PTR UploadMenuCommand(void *obj, WPARAM hContact, LPARAM)
{
	CCloudService *service = (CCloudService*)obj;

	auto it = service->InterceptedContacts.find(hContact);
	if (it == service->InterceptedContacts.end())
	{
		HWND hwnd = (HWND)CallService(MS_FILE_SENDFILE, hContact, 0);
		service->InterceptedContacts[hContact] = hwnd;
	}
	else
		SetActiveWindow(it->second);

	return 0;
}

void InitializeMenus()
{
	CMenuItem mi;
	SET_UID(mi, 0x93d4495b, 0x259b, 0x4fba, 0xbc, 0x14, 0xf9, 0x46, 0x2c, 0xda, 0xfc, 0x6d);
	mi.name.a = LPGEN("Upload files to ...");
	mi.position = -2000020001;
	mi.hIcon = LoadIconEx(IDI_UPLOAD);
	hContactMenu = Menu_AddContactMenuItem(&mi);

	UNSET_UID(mi);
	mi.flags |= CMIF_SYSTEM | CMIF_UNICODE;
	mi.root = hContactMenu;

	size_t count = Services.getCount();
	for (size_t i = 0; i < count; i++) {
		CCloudService *service = Services[i];

		CMStringA serviceName(CMStringDataFormat::FORMAT, "%s/%s/Upload", MODULE, service->GetModule());
		mi.pszService = serviceName.GetBuffer();
		mi.name.w = (wchar_t*)service->GetText();
		mi.position = i;
		mi.hIcolibItem = Services[i]->GetIcon();
		Menu_AddContactMenuItem(&mi);
		CreateServiceFunctionObj(mi.pszService, UploadMenuCommand, service);
	}
}

int OnPrebuildContactMenu(WPARAM hContact, LPARAM)
{
	bool bShow = false;
	char *proto = GetContactProto(hContact);
	if (proto) {
		bool bHasIM = (CallProtoService(proto, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_IMSEND) != 0;
		if (bHasIM) {
			bool isProtoOnline = CallProtoService(proto, PS_GETSTATUS, 0, 0) > ID_STATUS_OFFLINE;
			WORD status = db_get_w(hContact, proto, "Status", ID_STATUS_OFFLINE);
			bool canSendOffline = (CallProtoService(proto, PS_GETCAPS, PFLAGNUM_4, 0) & PF4_IMSENDOFFLINE) > 0;
			if (isProtoOnline && (status != ID_STATUS_OFFLINE || canSendOffline))
				bShow = true;
		}
	}
	Menu_ShowItem(hContactMenu, bShow);

	return 0;
}