#include "stdafx.h"

int OnSrmmToolbarLoaded(WPARAM, LPARAM)
{
	BBButton bbd = {};
	bbd.pszModuleName = MODULE;
	bbd.bbbFlags = BBBF_ISIMBUTTON | BBBF_ISCHATBUTTON | BBBF_ISRSIDEBUTTON | BBBF_ISARROWBUTTON;

	CMStringW tooltip(FORMAT, TranslateT("Upload files to..."));
	bbd.pwszTooltip = tooltip;
	bbd.hIcon = GetIconHandle(IDI_UPLOAD);
	bbd.dwButtonID = BBB_ID_FILE_SEND;
	bbd.dwDefPos = 100 + bbd.dwButtonID;
	Srmm_AddButton(&bbd);

	return 0;
}

int OnSrmmWindowOpened(WPARAM, LPARAM lParam)
{
	MessageWindowEventData *ev = (MessageWindowEventData*)lParam;
	if (ev->uType == MSG_WINDOW_EVT_OPENING && ev->hContact) {
		BBButton bbd = {};
		bbd.pszModuleName = MODULE;
		bbd.dwButtonID = BBB_ID_FILE_SEND;
		bbd.bbbFlags = CanSendToContact(ev->hContact)
			? BBSF_RELEASED
			: BBSF_DISABLED;
		Srmm_SetButtonState(ev->hContact, &bbd);
	}

	return 0;
}

int OnSrmmButtonPressed(WPARAM, LPARAM lParam)
{
	CustomButtonClickData *cbc = (CustomButtonClickData*)lParam;

	if (mir_strcmp(cbc->pszModule, MODULE))
		return 0;

	if (cbc->dwButtonId != BBB_ID_FILE_SEND)
		return 0;

	if (cbc->flags != BBCF_ARROWCLICKED) {
		ptrA defaultService(db_get_sa(NULL, MODULE, "DefaultService"));
		if (defaultService) {
			CCloudServiceSearch search(defaultService);
			CCloudService *service = Services.find(&search);
			if (service)
				service->OpenUploadDialog(cbc->hContact);
			return 0;
		}
	}

	HMENU hMenu = CreatePopupMenu();

	size_t count = Services.getCount();
	for (size_t i = 0; i < count; i++) {
		CCloudService *service = Services[i];

		InsertMenu(hMenu, i, MF_STRING | MF_BYPOSITION, i + 1,TranslateW(service->GetText()));
		//HBITMAP hBitmap = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(service->GetIconId()), IMAGE_ICON, 16, 16, 0);
		//SetMenuItemBitmaps(hMenu, i, MF_BITMAP, hBitmap, hBitmap);
	}

	int pos = TrackPopupMenu(hMenu, TPM_RETURNCMD, cbc->pt.x, cbc->pt.y, 0, cbc->hwndFrom, NULL);
	if (pos > 0) {
		CCloudService *service = Services[pos - 1];

		service->OpenUploadDialog(cbc->hContact);
	}

	return 0;
}