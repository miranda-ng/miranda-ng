#include "stdafx.h"

int OnModulesLoaded(WPARAM, LPARAM)
{
	// options
	HookEvent(ME_OPT_INITIALISE, OnOptionsInitialized);
	// srfile
	size_t count = Services.getCount();
	for (size_t i = 0; i < count; i++) {
		CCloudService *service = Services[i];

		HookEventObj(ME_FILEDLG_CANCELED, OnFileDialogCanceled, service);
	}

	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, OnPrebuildContactMenu);

	HookEvent(ME_MSG_WINDOWEVENT, OnSrmmWindowOpened);
	HookEvent(ME_MSG_BUTTONPRESSED, OnSrmmButtonPressed);

	HookTemporaryEvent(ME_MSG_TOOLBARLOADED, OnSrmmToolbarLoaded);

	return 0;
}

int OnProtoAck(WPARAM, LPARAM lParam)
{
	ACKDATA *ack = (ACKDATA*)lParam;

	if (ack->type != ACKTYPE_STATUS)
		return 0;

	for (MCONTACT hContact = db_find_first(ack->szModule); hContact; hContact = db_find_next(hContact, ack->szModule)) {
		MessageWindowData msgw;
		if (Srmm_GetWindowData(hContact, msgw) || !(msgw.uState & MSG_WINDOW_STATE_EXISTS))
			continue;

		BBButton bbd = {};
		bbd.pszModuleName = MODULE;
		bbd.dwButtonID = BBB_ID_FILE_SEND;
		bbd.bbbFlags = CanSendToContact(hContact)
			? BBSF_RELEASED
			: BBSF_DISABLED;
		Srmm_SetButtonState(hContact, &bbd);
	}

	return 0;
}

int OnFileDialogCanceled(void* obj, WPARAM hContact, LPARAM)
{
	CCloudService *service = (CCloudService*)obj;

	auto it = service->InterceptedContacts.find(hContact);
	if (it != service->InterceptedContacts.end())
		service->InterceptedContacts.erase(it);

	return 0;
}
