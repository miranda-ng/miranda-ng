#include "stdafx.h"

int CDropbox::OnModulesLoaded(WPARAM, LPARAM)
{
	HookEventObj(ME_DB_CONTACT_DELETED, GlobalEvent<&CDropbox::OnContactDeleted>, this);
	HookEventObj(ME_OPT_INITIALISE, GlobalEvent<&CDropbox::OnOptionsInitialized>, this);
	HookEventObj(ME_CLIST_PREBUILDCONTACTMENU, GlobalEvent<&CDropbox::OnPrebuildContactMenu>, this);

	HookEventObj(ME_MSG_WINDOWEVENT, GlobalEvent<&CDropbox::OnSrmmWindowOpened>, this);
	HookEventObj(ME_FILEDLG_CANCELED, GlobalEvent<&CDropbox::OnFileDialogCancelled>, this);
	HookEventObj(ME_FILEDLG_SUCCEEDED, GlobalEvent<&CDropbox::OnFileDialogSuccessed>, this);

	NETLIBUSER nlu = { sizeof(nlu) };
	nlu.flags = NUF_INCOMING | NUF_OUTGOING | NUF_HTTPCONNS | NUF_TCHAR;
	nlu.szSettingsModule = MODULE;
	nlu.szSettingsModule = MODULE;
	nlu.ptszDescriptiveName = L"Dropbox";

	hNetlibConnection = (HANDLE)CallService(MS_NETLIB_REGISTERUSER, 0, (LPARAM)&nlu);

	GetDefaultContact();

	WORD status = ProtoGetStatus(0, 0);
	ProtoBroadcastAck(MODULE, NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)ID_STATUS_OFFLINE, status);

	if (ServiceExists(MS_BB_ADDBUTTON))
	{
		BBButton bbd = { sizeof(bbd) };
		bbd.pszModuleName = MODULE;

		bbd.bbbFlags = BBBF_ISIMBUTTON | BBBF_ISRSIDEBUTTON;
		bbd.ptszTooltip = TranslateT("Upload files to Dropbox");
		bbd.hIcon = GetIconHandleByName("upload");
		bbd.dwButtonID = BBB_ID_FILE_SEND;
		bbd.dwDefPos = 100 + bbd.dwButtonID;
		CallService(MS_BB_ADDBUTTON, 0, (LPARAM)&bbd);

		HookEventObj(ME_MSG_BUTTONPRESSED, GlobalEvent<&CDropbox::OnTabSrmmButtonPressed>, this);
	}

	return 0;
}

int CDropbox::OnPreShutdown(WPARAM, LPARAM)
{
	if (ServiceExists(MS_BB_ADDBUTTON))
	{
		BBButton bbd = { sizeof(bbd) };
		bbd.pszModuleName = MODULE;

		bbd.dwButtonID = BBB_ID_FILE_SEND;
		CallService(MS_BB_REMOVEBUTTON, 0, (LPARAM)&bbd);
	}

	return 0;
}

int CDropbox::OnContactDeleted(WPARAM hContact, LPARAM)
{
	if (mir_strcmpi(GetContactProto(hContact), MODULE) == 0)
	{
		if (HasAccessToken())
			DestroyAccessToken();
		hDefaultContact = NULL;
	}
	return 0;
}

int CDropbox::OnSrmmWindowOpened(WPARAM, LPARAM lParam)
{
	MessageWindowEventData *ev = (MessageWindowEventData*)lParam;
	if (ev->uType == MSG_WINDOW_EVT_OPENING && ev->hContact)
	{
		char *proto = GetContactProto(ev->hContact);
		bool isProtoOnline = CallProtoService(proto, PS_GETSTATUS, 0, 0) > ID_STATUS_OFFLINE;
		WORD status = db_get_w(ev->hContact, proto, "Status", ID_STATUS_OFFLINE);
		bool canSendOffline = (CallProtoService(proto, PS_GETCAPS, PFLAGNUM_4, 0) & PF4_IMSENDOFFLINE) > 0;

		BBButton bbd = { sizeof(bbd) };
		bbd.pszModuleName = MODULE;
		bbd.dwButtonID = BBB_ID_FILE_SEND;
		bbd.bbbFlags = BBSF_RELEASED;
		if (!HasAccessToken() || ev->hContact == GetDefaultContact() || !HasAccessToken())
			bbd.bbbFlags = BBSF_HIDDEN | BBSF_DISABLED;
		else if (!isProtoOnline || (status == ID_STATUS_OFFLINE && !canSendOffline))
			bbd.bbbFlags = BBSF_DISABLED;

		CallService(MS_BB_SETBUTTONSTATE, ev->hContact, (LPARAM)&bbd);
	}

	return 0;
}

int CDropbox::OnTabSrmmButtonPressed(WPARAM, LPARAM lParam)
{
	CustomButtonClickData *cbc = (CustomButtonClickData *)lParam;
	if (!mir_strcmp(cbc->pszModule, MODULE) && cbc->dwButtonId == BBB_ID_FILE_SEND && cbc->hContact)
	{
		hTransferContact = cbc->hContact;
		hTransferWindow = (HWND)CallService(MS_FILE_SENDFILE, GetDefaultContact(), 0);

		DisableSrmmButton(cbc->hContact);
	}

	return 0;
}

void CDropbox::DisableSrmmButton(MCONTACT hContact)
{
	BBButton bbd = { sizeof(bbd) };
	bbd.pszModuleName = MODULE;
	bbd.dwButtonID = BBB_ID_FILE_SEND;
	bbd.bbbFlags = BBSF_DISABLED;
	CallService(MS_BB_SETBUTTONSTATE, hContact, (LPARAM)&bbd);
}

void __stdcall EnableTabSrmmButtonSync(void *arg)
{
	BBButton bbd = { sizeof(bbd) };
	bbd.pszModuleName = MODULE;
	bbd.dwButtonID = BBB_ID_FILE_SEND;
	bbd.bbbFlags = BBSF_RELEASED;
	MCONTACT hContact = (MCONTACT)arg;
	CallService(MS_BB_SETBUTTONSTATE, hContact, (LPARAM)&bbd);
}

int CDropbox::OnFileDialogCancelled(WPARAM, LPARAM lParam)
{
	HWND hwnd = (HWND)lParam;
	if (hTransferWindow == hwnd)
	{
		CallFunctionAsync(EnableTabSrmmButtonSync, (void*)hTransferContact);
		hTransferWindow = 0;
	}

	return 0;
}

int CDropbox::OnFileDialogSuccessed(WPARAM, LPARAM lParam)
{
	HWND hwnd = (HWND)lParam;
	if (hTransferWindow == hwnd)
	{
		CallFunctionAsync(EnableTabSrmmButtonSync, (void*)hTransferContact);
		hTransferWindow = 0;
	}

	return 0;
}

int CDropbox::OnProtoAck(WPARAM, LPARAM lParam)
{
	ACKDATA *ack = (ACKDATA*)lParam;

	if (!mir_strcmp(ack->szModule, MODULE))
		return 0; // don't rebroadcast our own acks

	if (ack->type == ACKTYPE_STATUS)
	{
		WORD status = ack->lParam;
		bool canSendOffline = (CallProtoService(ack->szModule, PS_GETCAPS, PFLAGNUM_4, 0) & PF4_IMSENDOFFLINE) > 0;

		MessageWindowInputData msgwi = { sizeof(msgwi) };
		msgwi.uFlags = MSG_WINDOW_UFLAG_MSG_BOTH;

		for (MCONTACT hContact = db_find_first(ack->szModule); hContact; hContact = db_find_next(hContact, ack->szModule))
		{
			msgwi.hContact = hContact;

			MessageWindowData msgw;
			msgw.cbSize = sizeof(msgw);

			if (!CallService(MS_MSG_GETWINDOWDATA, (WPARAM)&msgwi, (LPARAM)&msgw) && msgw.uState & MSG_WINDOW_STATE_EXISTS)
			{
				BBButton bbd = { sizeof(bbd) };
				bbd.pszModuleName = MODULE;
				bbd.dwButtonID = BBB_ID_FILE_SEND;
				bbd.bbbFlags = BBSF_RELEASED;

				if (status == ID_STATUS_OFFLINE && !canSendOffline)
					bbd.bbbFlags = BBSF_DISABLED;

				CallService(MS_BB_SETBUTTONSTATE, hContact, (LPARAM)&bbd);
			}
		}
	}

	return 0;
}
