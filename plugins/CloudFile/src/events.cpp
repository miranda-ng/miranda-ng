#include "stdafx.h"

int OnModulesLoaded(WPARAM, LPARAM)
{
	// init
	InitServices();
	InitializeIcons();
	InitializeMenus();
	// netlib
	NETLIBUSER nlu = {};
	nlu.flags = NUF_INCOMING | NUF_OUTGOING | NUF_HTTPCONNS | NUF_UNICODE;
	nlu.szSettingsModule = MODULE;
	nlu.szDescriptiveName.w = _A2W("MODULE");
	hNetlibConnection = Netlib_RegisterUser(&nlu);
	// options
	HookEvent(ME_OPT_INITIALISE, OnOptionsInitialized);
	// srfile
	size_t count = Services.getCount();
	for (size_t i = 0; i < count; i++) {
		CCloudService *service = Services[i];

		HookEventObj(ME_FILEDLG_CANCELED, OnFileDialogCanceled, service);
	}
	// menus
	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, OnPrebuildContactMenu);
	// srmm
	HookEvent(ME_MSG_TOOLBARLOADED, OnSrmmToolbarLoaded);
	HookEvent(ME_MSG_WINDOWEVENT, OnSrmmWindowOpened);
	HookEvent(ME_MSG_BUTTONPRESSED, OnSrmmButtonPressed);

	return 0;
}

int OnProtoAck(WPARAM, LPARAM lParam)
{
	ACKDATA *ack = (ACKDATA*)lParam;

	if (!mir_strcmp(ack->szModule, MODULE))
		return 0; // don't rebroadcast our own acks

	if (ack->type == ACKTYPE_STATUS) {
		WORD status = ack->lParam;
		bool canSendOffline = (CallProtoService(ack->szModule, PS_GETCAPS, PFLAGNUM_4, 0) & PF4_IMSENDOFFLINE) > 0;

		for (MCONTACT hContact = db_find_first(ack->szModule); hContact; hContact = db_find_next(hContact, ack->szModule)) {
			MessageWindowData msgw;
			if (!Srmm_GetWindowData(hContact, msgw) && msgw.uState & MSG_WINDOW_STATE_EXISTS) {
				BBButton bbd = {};
				bbd.pszModuleName = MODULE;
				bbd.dwButtonID = BBB_ID_FILE_SEND;
				bbd.bbbFlags = BBSF_RELEASED;

				if (status == ID_STATUS_OFFLINE && !canSendOffline)
					bbd.bbbFlags = BBSF_DISABLED;

				Srmm_SetButtonState(hContact, &bbd);
			}
		}
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
