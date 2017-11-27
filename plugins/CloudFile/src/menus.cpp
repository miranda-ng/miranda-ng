#include "stdafx.h"

HGENMENU hContactMenu;

INT_PTR UploadMenuCommand(void *obj, WPARAM hContact, LPARAM)
{
	CCloudService *service = (CCloudService*)obj;

	service->OpenUploadDialog(hContact);

	return 0;
}

void InitializeMenus()
{
	CMenuItem mi;
	SET_UID(mi, 0x93d4495b, 0x259b, 0x4fba, 0xbc, 0x14, 0xf9, 0x46, 0x2c, 0xda, 0xfc, 0x6d);
	mi.name.a = LPGEN("Upload to...");

	ptrA defaultService(db_get_sa(NULL, MODULE, "DefaultService"));
	if (defaultService) {
		CCloudServiceSearch search(defaultService);
		CCloudService *service = Services.find(&search);
		if (service) {
			mi.name.a = LPGEN("Upload");
			mi.pszService = MODULE "/Default/Upload";
			CreateServiceFunctionObj(mi.pszService, UploadMenuCommand, service);
		}
	}

	mi.position = -2000019999;
	mi.hIcon = LoadIconEx(IDI_UPLOAD);
	hContactMenu = Menu_AddContactMenuItem(&mi);

	if (defaultService)
		return;

	UNSET_UID(mi);
	
	mi.root = hContactMenu;

	size_t count = Services.getCount();
	
	for (size_t i = 0; i < count; i++) {
		CCloudService *service = Services[i];

		CMStringA serviceName = MODULE;
		serviceName.AppendFormat("/%s/Upload", service->GetModule());
		mi.pszService = serviceName.GetBuffer();

		mi.flags = CMIF_SYSTEM | CMIF_UNICODE;
		mi.name.w = (wchar_t*)service->GetText();
		mi.position = i;
		mi.hIcolibItem = GetIconHandle(Services[i]->GetIconId());
		Menu_AddContactMenuItem(&mi);
		CreateServiceFunctionObj(mi.pszService, UploadMenuCommand, service);
	}
}

int OnPrebuildContactMenu(WPARAM hContact, LPARAM)
{
	Menu_ShowItem(hContactMenu, CanSendToContact(hContact));

	return 0;
}