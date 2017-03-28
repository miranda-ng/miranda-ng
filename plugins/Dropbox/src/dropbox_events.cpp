#include "stdafx.h"

int CDropbox::OnToolbarLoaded(WPARAM, LPARAM)
{
	BBButton bbd = {};
	bbd.pszModuleName = MODULE;
	bbd.bbbFlags = BBBF_ISIMBUTTON | BBBF_ISCHATBUTTON | BBBF_ISRSIDEBUTTON;
	bbd.pwszTooltip = TranslateT("Upload files to Dropbox");
	bbd.hIcon = GetIconHandleByName("upload");
	bbd.dwButtonID = BBB_ID_FILE_SEND;
	bbd.dwDefPos = 100 + bbd.dwButtonID;
	Srmm_AddButton(&bbd);
	return 0;
}

int CDropbox::OnModulesLoaded(WPARAM, LPARAM)
{
	HookEventObj(ME_DB_CONTACT_DELETED, GlobalEvent<&CDropbox::OnContactDeleted>, this);
	HookEventObj(ME_OPT_INITIALISE, GlobalEvent<&CDropbox::OnOptionsInitialized>, this);
	HookEventObj(ME_CLIST_PREBUILDCONTACTMENU, GlobalEvent<&CDropbox::OnPrebuildContactMenu>, this);
	HookEventObj(ME_MSG_TOOLBARLOADED, GlobalEvent<&CDropbox::OnToolbarLoaded>, this);

	HookEventObj(ME_MSG_WINDOWEVENT, GlobalEvent<&CDropbox::OnSrmmWindowOpened>, this);
	HookEventObj(ME_FILEDLG_CANCELED, GlobalEvent<&CDropbox::OnFileDialogCancelled>, this);

	NETLIBUSER nlu = {};
	nlu.flags = NUF_INCOMING | NUF_OUTGOING | NUF_HTTPCONNS | NUF_UNICODE;
	nlu.szSettingsModule = MODULE;
	nlu.szDescriptiveName.w = L"Dropbox";
	hNetlibConnection = Netlib_RegisterUser(&nlu);

	GetDefaultContact();

	WORD status = ProtoGetStatus(0, 0);
	ProtoBroadcastAck(MODULE, NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)ID_STATUS_OFFLINE, status);

	HookEventObj(ME_MSG_BUTTONPRESSED, GlobalEvent<&CDropbox::OnTabSrmmButtonPressed>, this);
	return 0;
}

int CDropbox::OnContactDeleted(WPARAM hContact, LPARAM)
{
	if (mir_strcmpi(GetContactProto(hContact), MODULE) == 0) {
		if (HasAccessToken())
			DestroyAccessToken();
		hDefaultContact = NULL;
	}
	return 0;
}

int CDropbox::OnSrmmWindowOpened(WPARAM, LPARAM lParam)
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
		if (!HasAccessToken() || ev->hContact == GetDefaultContact() || IsAccountIntercepted(proto))
			bbd.bbbFlags = BBSF_HIDDEN | BBSF_DISABLED;
		else if (!isProtoOnline || (status == ID_STATUS_OFFLINE && !canSendOffline))
			bbd.bbbFlags = BBSF_DISABLED;

		Srmm_SetButtonState(ev->hContact, &bbd);
	}

	return 0;
}

int CDropbox::OnTabSrmmButtonPressed(WPARAM, LPARAM lParam)
{
	CustomButtonClickData *cbc = (CustomButtonClickData *)lParam;
	if (!mir_strcmp(cbc->pszModule, MODULE) && cbc->dwButtonId == BBB_ID_FILE_SEND && cbc->hContact) {
		auto it = interceptedContacts.find(cbc->hContact);
		if (it == interceptedContacts.end())
		{
			HWND hwnd = (HWND)CallService(MS_FILE_SENDFILE, cbc->hContact, 0);
			interceptedContacts[cbc->hContact] = hwnd;
		}
		else
			FlashWindow(it->second, FALSE);
	}

	return 0;
}

int CDropbox::OnFileDialogCancelled(WPARAM hContact, LPARAM)
{
	auto it = interceptedContacts.find(hContact);
	if (it != interceptedContacts.end())
		interceptedContacts.erase(it);

	return 0;
}

int CDropbox::OnProtoAck(WPARAM, LPARAM lParam)
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
