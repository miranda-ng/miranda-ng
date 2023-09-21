#include "stdafx.h"

int OnSrmmToolbarLoaded(WPARAM, LPARAM)
{
	BBButton bbd = {};
	bbd.pszModuleName = MODULENAME;
	bbd.bbbFlags = BBBF_ISIMBUTTON | BBBF_ISCHATBUTTON | BBBF_ISRSIDEBUTTON | BBBF_ISARROWBUTTON;

	CMStringW tooltip(FORMAT, TranslateT("Upload files to..."));
	bbd.pwszTooltip = tooltip;
	bbd.hIcon = g_plugin.getIconHandle(IDI_UPLOAD);
	bbd.dwButtonID = BBB_ID_FILE_SEND;
	bbd.dwDefPos = 100 + bbd.dwButtonID;
	Srmm_AddButton(&bbd, &g_plugin);
	return 0;
}

int OnSrmmWindowOpened(WPARAM uType, LPARAM lParam)
{
	auto *pDlg = (CSrmmBaseDialog *)lParam;

	if (uType == MSG_WINDOW_EVT_OPENING && pDlg->m_hContact) {
		BBButton bbd = {};
		bbd.pszModuleName = MODULENAME;
		bbd.dwButtonID = BBB_ID_FILE_SEND;
		bbd.bbbFlags = CanSendToContact(pDlg->m_hContact) ? BBSF_RELEASED : BBSF_DISABLED;
		Srmm_SetButtonState(pDlg->m_hContact, &bbd);
	}

	return 0;
}

int OnSrmmButtonPressed(WPARAM, LPARAM lParam)
{
	CustomButtonClickData *cbc = (CustomButtonClickData*)lParam;

	if (mir_strcmp(cbc->pszModule, MODULENAME))
		return 0;

	if (cbc->dwButtonId != BBB_ID_FILE_SEND)
		return 0;

	if (cbc->flags != BBCF_ARROWCLICKED) {
		ptrA defaultService(g_plugin.getStringA("DefaultService"));
		if (defaultService) {
			CCloudService *service = FindService(defaultService);
			if (service)
				service->OpenUploadDialog(cbc->hContact);
			return 0;
		}
	}

	HMENU hMenu = CreatePopupMenu();
	for (auto &it : Services)
		AppendMenu(hMenu, MF_STRING, Services.indexOf(&it) + 1, TranslateW(it->GetUserName()));

	int pos = TrackPopupMenu(hMenu, TPM_RETURNCMD, cbc->pt.x, cbc->pt.y, 0, cbc->hwndFrom, nullptr);
	DestroyMenu(hMenu);

	if (pos > 0) {
		CCloudService *service = Services[pos - 1];
		service->OpenUploadDialog(cbc->hContact);
	}

	return 0;
}