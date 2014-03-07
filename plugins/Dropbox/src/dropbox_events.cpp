#include "common.h"

int CDropbox::OnModulesLoaded(void *obj, WPARAM wParam, LPARAM lParam)
{
	CDropbox *instance = (CDropbox*)obj;

	HookEventObj(ME_FILEDLG_CANCELED, OnFileDoalogCancelled, obj);
	HookEventObj(ME_FILEDLG_SUCCEEDED, OnFileDoalogSuccessed, obj);

	NETLIBUSER nlu = { sizeof(nlu) };
	nlu.flags = NUF_INCOMING | NUF_OUTGOING | NUF_HTTPCONNS | NUF_TCHAR;
	nlu.szSettingsModule = MODULE;
	nlu.szSettingsModule = MODULE;
	nlu.ptszDescriptiveName = L"Dropbox";

	instance->hNetlibUser = (HANDLE)CallService(MS_NETLIB_REGISTERUSER, 0, (LPARAM)&nlu);

	if (ServiceExists(MS_BB_ADDBUTTON))
	{
		BBButton bbd = { sizeof(bbd) };
		bbd.pszModuleName = MODULE;

		bbd.bbbFlags = BBBF_ISIMBUTTON | BBBF_ISRSIDEBUTTON;
		bbd.ptszTooltip = TranslateT("Send files to Dropbox");
		bbd.hIcon = LoadSkinnedIconHandle(SKINICON_EVENT_FILE);
		bbd.dwButtonID = BBB_ID_FILE_SEND;
		bbd.dwDefPos = 100 + bbd.dwButtonID;
		CallService(MS_BB_ADDBUTTON, 0, (LPARAM)&bbd);

		HookEventObj(ME_MSG_WINDOWEVENT, OnSrmmWindowOpened, obj);
		HookEventObj(ME_MSG_BUTTONPRESSED, OnTabSrmmButtonPressed, obj);
	}

	return 0;
}

int CDropbox::OnPreShutdown(void *obj, WPARAM wParam, LPARAM lParam)
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

int CDropbox::OnContactDeleted(void *obj, WPARAM hContact, LPARAM lParam)
{
	CDropbox *instance = (CDropbox*)obj;

	if (lstrcmpiA(GetContactProto(hContact), MODULE) == 0)
	{
		if (instance->HasAccessToken())
			instance->DestroyAcceessToken();
	}

	return 0;
}

int CDropbox::OnOptionsInitialized(void *obj, WPARAM wParam, LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = { sizeof(odp) };
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

int CDropbox::OnSrmmWindowOpened(void *obj, WPARAM wParam, LPARAM lParam)
{
	CDropbox *instance = (CDropbox*)obj;

	MessageWindowEventData *ev = (MessageWindowEventData*)lParam;
	if (ev->uType == MSG_WINDOW_EVT_OPENING && ev->hContact)
	{
		WORD status = db_get_w(ev->hContact, GetContactProto(ev->hContact), "Status", ID_STATUS_OFFLINE);

		BBButton bbd = { sizeof(bbd) };
		bbd.pszModuleName = MODULE;
		if (ev->hContact == instance->GetDefaultContact() || !instance->HasAccessToken() || status == ID_STATUS_OFFLINE)
			bbd.bbbFlags = BBSF_HIDDEN | BBSF_DISABLED;
		else if (instance->hTransferContact)
			bbd.bbbFlags = BBSF_DISABLED;

		bbd.dwButtonID = BBB_ID_FILE_SEND;
		CallService(MS_BB_SETBUTTONSTATE, ev->hContact, (LPARAM)&bbd);
	}

	return 0;
}

int CDropbox::OnTabSrmmButtonPressed(void *obj, WPARAM wParam, LPARAM lParam)
{
	CDropbox *instance = (CDropbox*)obj;

	CustomButtonClickData *cbc = (CustomButtonClickData *)lParam;
	if (!strcmp(cbc->pszModule, MODULE) && cbc->dwButtonId == BBB_ID_FILE_SEND && cbc->hContact)
	{
		instance->hTransferContact = cbc->hContact;

		HWND hwnd = (HWND)CallService(MS_FILE_SENDFILE, instance->GetDefaultContact(), 0);

		instance->dcftp[hwnd] = cbc->hContact;
	}

	return 0; 
}

int CDropbox::OnFileDoalogCancelled(void *obj, WPARAM hContact, LPARAM lParam)
{
	CDropbox *instance = (CDropbox*)obj;

	HWND hwnd = (HWND)lParam;
	if (instance->hTransferContact == instance->dcftp[hwnd])
	{
		instance->dcftp.erase((HWND)lParam);
		instance->hTransferContact = 0;
	}

	return 0;
}

int CDropbox::OnFileDoalogSuccessed(void *obj, WPARAM hContact, LPARAM lParam)
{
	CDropbox *instance = (CDropbox*)obj;

	HWND hwnd = (HWND)lParam;
	if (instance->hTransferContact == instance->dcftp[hwnd])
		instance->dcftp.erase((HWND)lParam);

	return 0;
}