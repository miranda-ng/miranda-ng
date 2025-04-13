#include "stdafx.h"

static int OnProtoAck(WPARAM, LPARAM lParam)
{
	ACKDATA *ack = (ACKDATA*)lParam;

	if (ack->type != ACKTYPE_STATUS)
		return 0;

	for (auto &hContact : Contacts(ack->szModule)) {
		MessageWindowData msgw;
		if (Srmm_GetWindowData(hContact, msgw) || !(msgw.uState & MSG_WINDOW_STATE_EXISTS))
			continue;

		BBButton bbd = {};
		bbd.pszModuleName = MODULENAME;
		bbd.dwButtonID = BBB_ID_FILE_SEND;
		bbd.bbbFlags = CanSendToContact(hContact)
			? BBSF_RELEASED
			: BBSF_DISABLED;
		Srmm_SetButtonState(hContact, &bbd);
	}

	return 0;
}

static int OnFileDialogCanceled(WPARAM hContact, LPARAM)
{
	for (auto &service : g_arServices) {
		auto it = service->InterceptedContacts.find(hContact);
		if (it != service->InterceptedContacts.end())
			service->InterceptedContacts.erase(it);
	}
	return 0;
}

int OnModulesLoaded(WPARAM, LPARAM)
{
	HookEvent(ME_PROTO_ACK, OnProtoAck);
	HookEvent(ME_FILEDLG_CANCELED, OnFileDialogCanceled);

	HookTemporaryEvent(ME_MSG_TOOLBARLOADED, OnSrmmToolbarLoaded);
	return 0;
}
