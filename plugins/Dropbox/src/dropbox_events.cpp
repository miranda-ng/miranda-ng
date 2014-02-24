#include "common.h"

int CDropbox::OnModulesLoaded(WPARAM wParam, LPARAM lParam)
{
	HookEvent(ME_FILEDLG_CANCELED, CDropbox::OnFileDoalogCancelled);
	HookEvent(ME_FILEDLG_SUCCEEDED, CDropbox::OnFileDoalogSuccessed);

	NETLIBUSER nlu = { sizeof(nlu) };
	nlu.flags = NUF_INCOMING | NUF_OUTGOING | NUF_HTTPCONNS | NUF_TCHAR;
	nlu.szSettingsModule = MODULE;
	nlu.szSettingsModule = MODULE;
	nlu.ptszDescriptiveName = L"Dropbox";

	INSTANCE->hNetlibUser = (HANDLE)CallService(MS_NETLIB_REGISTERUSER, 0, (LPARAM)&nlu);

	MCONTACT hContact = GetDefaultContact();
	if (!hContact)
	{
		hContact = (MCONTACT)CallService(MS_DB_CONTACT_ADD, 0, 0);
		if (!CallService(MS_PROTO_ADDTOCONTACT, hContact, (LPARAM)MODULE))
		{
			db_set_s(hContact, MODULE, "Nick", MODULE);
		}
	}

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

		HookEvent(ME_MSG_WINDOWEVENT, CDropbox::OnSrmmWindowOpened);
		HookEvent(ME_MSG_BUTTONPRESSED, CDropbox::OnSrmmButtonPressed);
	}

	return 0;
}

int CDropbox::OnOptionsInit(WPARAM wParam, LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = { sizeof(odp) };
	odp.position = 100000000;
	odp.hInstance = g_hInstance;
	odp.flags = ODPF_BOLDGROUPS;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS_MAIN);
	odp.pszGroup = LPGEN("Network");
	odp.pszTitle = LPGEN("Dropbox");
	odp.pfnDlgProc = MainOptionsProc;

	//Options_AddPage(wParam, &odp);

	return 0;
}

int CDropbox::OnContactDeleted(WPARAM hContact, LPARAM lParam)
{
	if (lstrcmpiA(GetContactProto(hContact), MODULE) == 0)
	{
		if (INSTANCE->HasAccessToken())
			INSTANCE->DestroyAcceessToken(hContact);
	}

	return 0;
}

int CDropbox::OnSrmmWindowOpened(WPARAM wParam, LPARAM lParam)
{
	MessageWindowEventData *ev = (MessageWindowEventData*)lParam;
	if (ev->uType == MSG_WINDOW_EVT_OPENING && ev->hContact)
	{
		WORD status = db_get_w(ev->hContact, GetContactProto(ev->hContact), "Status", ID_STATUS_OFFLINE);

		BBButton bbd = { sizeof(bbd) };
		bbd.pszModuleName = MODULE;
		if (ev->hContact == INSTANCE->GetDefaultContact())
			bbd.bbbFlags = BBSF_HIDDEN | BBSF_DISABLED;
		else if (status == ID_STATUS_OFFLINE)
			bbd.bbbFlags = BBSF_DISABLED;

		bbd.dwButtonID = BBB_ID_FILE_SEND;
		CallService(MS_BB_SETBUTTONSTATE, (WPARAM)ev->hContact, (LPARAM)&bbd);
	}

	return 0;
}

int CDropbox::OnSrmmButtonPressed(WPARAM wParam, LPARAM lParam)
{
	CustomButtonClickData *cbc = (CustomButtonClickData *)lParam;
	if (!strcmp(cbc->pszModule, MODULE) && cbc->dwButtonId == BBB_ID_FILE_SEND && cbc->hContact)
	{
		INSTANCE->hContactTransfer = cbc->hContact;

		HWND hwnd =  (HWND)CallService(MS_FILE_SENDFILE, INSTANCE->GetDefaultContact(), 0);

		dcftp[hwnd] = cbc->hContact;
	}

	return 0; 
}