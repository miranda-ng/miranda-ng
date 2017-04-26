#include "stdafx.h"

int OnSrmmToolbarLoaded(WPARAM, LPARAM)
{
	BBButton bbd = {};
	bbd.pszModuleName = MODULE;
	bbd.bbbFlags = BBBF_ISIMBUTTON | BBBF_ISCHATBUTTON | BBBF_ISRSIDEBUTTON;

	CMStringW tooltip(CMStringDataFormat::FORMAT, TranslateT("Upload files to ..."));
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
		char *proto = GetContactProto(ev->hContact);
		bool isProtoOnline = CallProtoService(proto, PS_GETSTATUS, 0, 0) > ID_STATUS_OFFLINE;
		WORD status = db_get_w(ev->hContact, proto, "Status", ID_STATUS_OFFLINE);
		bool canSendOffline = (CallProtoService(proto, PS_GETCAPS, PFLAGNUM_4, 0) & PF4_IMSENDOFFLINE) > 0;

		BBButton bbd = {};
		bbd.pszModuleName = MODULE;
		bbd.dwButtonID = BBB_ID_FILE_SEND;
		bbd.bbbFlags = BBSF_RELEASED;
		if (!isProtoOnline || (status == ID_STATUS_OFFLINE && !canSendOffline))
			bbd.bbbFlags = BBSF_DISABLED;

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

	HMENU hMenu = CreatePopupMenu();

	size_t count = Services.getCount();
	for (size_t i = 0; i < count; i++) {
		CCloudService *service = Services[i];

		if (!db_get_b(NULL, service->GetModule(), "IsEnable", TRUE))
			continue;

		InsertMenu(hMenu, i, MF_STRING, i + 1, TranslateW(service->GetText()));
		//HBITMAP hBitmap = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(service->GetIconId()), IMAGE_ICON, 16, 16, 0);
		//SetMenuItemBitmaps(hMenu, i, MF_BITMAP, hBitmap, hBitmap);
	}

	int ind = TrackPopupMenu(hMenu, TPM_RETURNCMD, cbc->pt.x, cbc->pt.y, 0, cbc->hwndFrom, NULL);
	if (ind > 0) {
		CCloudService *service = Services[ind - 1];

		auto it = service->InterceptedContacts.find(cbc->hContact);
		if (it == service->InterceptedContacts.end())
		{
			HWND hwnd = (HWND)CallService(MS_FILE_SENDFILE, cbc->hContact, 0);
			service->InterceptedContacts[cbc->hContact] = hwnd;
		}
		else
			SetActiveWindow(it->second);
	}

	return 0;
}