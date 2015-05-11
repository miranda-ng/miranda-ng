#include "stdafx.h"

int CDropbox::OnModulesLoaded(void *obj, WPARAM, LPARAM)
{
	HookEventObj(ME_DB_CONTACT_DELETED, OnContactDeleted, obj);
	HookEventObj(ME_OPT_INITIALISE, OnOptionsInitialized, obj);
	HookEventObj(ME_CLIST_PREBUILDCONTACTMENU, OnPrebuildContactMenu, obj);

	HookEventObj(ME_MSG_WINDOWEVENT, OnSrmmWindowOpened, obj);
	HookEventObj(ME_FILEDLG_CANCELED, OnFileDialogCancelled, obj);
	HookEventObj(ME_FILEDLG_SUCCEEDED, OnFileDialogSuccessed, obj);

	NETLIBUSER nlu = { sizeof(nlu) };
	nlu.flags = NUF_INCOMING | NUF_OUTGOING | NUF_HTTPCONNS | NUF_TCHAR;
	nlu.szSettingsModule = MODULE;
	nlu.szSettingsModule = MODULE;
	nlu.ptszDescriptiveName = L"Dropbox";

	CDropbox *instance = (CDropbox*)obj;

	instance->hNetlibConnection = (HANDLE)CallService(MS_NETLIB_REGISTERUSER, 0, (LPARAM)&nlu);

	instance->GetDefaultContact();

	if (ServiceExists(MS_BB_ADDBUTTON)) {
		BBButton bbd = { sizeof(bbd) };
		bbd.pszModuleName = MODULE;

		bbd.bbbFlags = BBBF_ISIMBUTTON | BBBF_ISRSIDEBUTTON;
		bbd.ptszTooltip = TranslateT("Send files to Dropbox");
		bbd.hIcon = GetIconHandle(IDI_DROPBOX);
		bbd.dwButtonID = BBB_ID_FILE_SEND;
		bbd.dwDefPos = 100 + bbd.dwButtonID;
		CallService(MS_BB_ADDBUTTON, 0, (LPARAM)&bbd);

		HookEventObj(ME_MSG_BUTTONPRESSED, OnTabSrmmButtonPressed, obj);
	}

	return 0;
}

int CDropbox::OnPreShutdown(void *, WPARAM, LPARAM)
{
	if (ServiceExists(MS_BB_ADDBUTTON)) {
		BBButton bbd = { sizeof(bbd) };
		bbd.pszModuleName = MODULE;

		bbd.dwButtonID = BBB_ID_FILE_SEND;
		CallService(MS_BB_REMOVEBUTTON, 0, (LPARAM)&bbd);
	}

	return 0;
}

int CDropbox::OnContactDeleted(void *obj, WPARAM hContact, LPARAM)
{
	CDropbox *instance = (CDropbox*)obj;

	if (mir_strcmpi(GetContactProto(hContact), MODULE) == 0)
		if (instance->HasAccessToken())
			instance->DestroyAccessToken();

	return 0;
}

int CDropbox::OnOptionsInitialized(void *obj, WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = { 0 };
	odp.position = 100000000;
	odp.hInstance = g_hInstance;
	odp.flags = ODPF_BOLDGROUPS;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS_MAIN);
	odp.pszGroup = LPGEN("Network");
	odp.pszTitle = "Dropbox";
	odp.pfnDlgProc = MainOptionsProc;
	odp.dwInitParam = (LPARAM)obj;

	Options_AddPage(wParam, &odp);

	return 0;
}

int CDropbox::OnSrmmWindowOpened(void *obj, WPARAM, LPARAM lParam)
{
	MessageWindowEventData *ev = (MessageWindowEventData*)lParam;
	if (ev->uType == MSG_WINDOW_EVT_OPENING && ev->hContact) {
		CDropbox *instance = (CDropbox*)obj;
		char *proto = GetContactProto(ev->hContact);
		bool isProtoOnline = CallProtoService(proto, PS_GETSTATUS, 0, 0) > ID_STATUS_OFFLINE;
		WORD status = db_get_w(ev->hContact, proto, "Status", ID_STATUS_OFFLINE);
		bool canSendOffline = (CallProtoService(proto, PS_GETCAPS, PFLAGNUM_4, 0) & PF4_IMSENDOFFLINE) > 0;

		BBButton bbd = { sizeof(bbd) };
		bbd.pszModuleName = MODULE;
		bbd.dwButtonID = BBB_ID_FILE_SEND;
		bbd.bbbFlags = BBSF_RELEASED;
		if (!instance->HasAccessToken() || ev->hContact == instance->GetDefaultContact() || !instance->HasAccessToken())
			bbd.bbbFlags = BBSF_HIDDEN | BBSF_DISABLED;
		else if (!isProtoOnline || (status == ID_STATUS_OFFLINE && !canSendOffline))
			bbd.bbbFlags = BBSF_DISABLED;

		CallService(MS_BB_SETBUTTONSTATE, ev->hContact, (LPARAM)&bbd);
	}

	return 0;
}

int CDropbox::OnTabSrmmButtonPressed(void *obj, WPARAM, LPARAM lParam)
{
	CDropbox *instance = (CDropbox*)obj;

	CustomButtonClickData *cbc = (CustomButtonClickData *)lParam;
	if (!strcmp(cbc->pszModule, MODULE) && cbc->dwButtonId == BBB_ID_FILE_SEND && cbc->hContact) {
		instance->hTransferContact = cbc->hContact;
		instance->hTransferWindow = (HWND)CallService(MS_FILE_SENDFILE, instance->GetDefaultContact(), 0);

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

void __stdcall EnableTabSrmmButtonAsync(void *arg)
{
	BBButton bbd = { sizeof(bbd) };
	bbd.pszModuleName = MODULE;
	bbd.dwButtonID = BBB_ID_FILE_SEND;
	bbd.bbbFlags = BBSF_RELEASED;
	MCONTACT hContact = (MCONTACT)arg;
	CallService(MS_BB_SETBUTTONSTATE, hContact, (LPARAM)&bbd);
}

int CDropbox::OnFileDialogCancelled(void *obj, WPARAM, LPARAM lParam)
{
	CDropbox *instance = (CDropbox*)obj;

	HWND hwnd = (HWND)lParam;
	if (instance->hTransferWindow == hwnd) {
		CallFunctionAsync(EnableTabSrmmButtonAsync, (void*)instance->hTransferContact);
		instance->hTransferWindow = 0;
	}

	return 0;
}

int CDropbox::OnFileDialogSuccessed(void *obj, WPARAM, LPARAM lParam)
{
	CDropbox *instance = (CDropbox*)obj;

	HWND hwnd = (HWND)lParam;
	if (instance->hTransferWindow == hwnd) {
		CallFunctionAsync(EnableTabSrmmButtonAsync, (void*)instance->hTransferContact);
		instance->hTransferWindow = 0;
	}

	return 0;
}

int CDropbox::OnProtoAck(void *, WPARAM, LPARAM lParam)
{
	ACKDATA *ack = (ACKDATA*)lParam;

	if (!strcmp(ack->szModule, MODULE))
		return 0; // don't rebroadcast our own acks

	if (ack->type == ACKTYPE_STATUS/* && ((int)ack->lParam != (int)ack->hProcess)*/) {
		WORD status = ack->lParam;
		bool canSendOffline = (CallProtoService(ack->szModule, PS_GETCAPS, PFLAGNUM_4, 0) & PF4_IMSENDOFFLINE) > 0;

		MessageWindowInputData msgwi = { sizeof(msgwi) };
		msgwi.uFlags = MSG_WINDOW_UFLAG_MSG_BOTH;

		for (MCONTACT hContact = db_find_first(ack->szModule); hContact; hContact = db_find_next(hContact, ack->szModule)) {
			msgwi.hContact = hContact;

			MessageWindowData msgw;
			msgw.cbSize = sizeof(msgw);

			if (!CallService(MS_MSG_GETWINDOWDATA, (WPARAM)&msgwi, (LPARAM)&msgw) && msgw.uState & MSG_WINDOW_STATE_EXISTS) {
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
