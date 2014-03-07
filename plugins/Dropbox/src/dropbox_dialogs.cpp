#include "common.h"

INT_PTR CALLBACK CDropbox::TokenRequestProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	char *token = reinterpret_cast<char*>(::GetWindowLongPtr(hwndDlg, GWLP_USERDATA));

	switch (msg)
	{
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);

		token = (char*)lParam;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
		{
			//::SendMessage(hwndDlg, WM_SETICON, ICON_BIG,	(LPARAM)CSkypeProto::IconList[0].Handle);
			//::SendMessage(hwndDlg, WM_SETICON, ICON_SMALL,	(LPARAM)CSkypeProto::IconList[0].Handle);

			/*wchar_t title[MAX_PATH];
			::mir_sntprintf(
			title,
			MAX_PATH,
			::TranslateT("Enter a password for %s:"),
			param->login);*/
			//::SetDlgItemText(hwndDlg, IDC_INSTRUCTION, title);

			SendDlgItemMessage(hwndDlg, IDC_TOKEN, EM_LIMITTEXT, 128 - 1, 0);
		}
		break;

	case WM_CLOSE:
		EndDialog(hwndDlg, 0);
		break;

	case WM_COMMAND:
	{
		switch (LOWORD(wParam))
		{
		case IDOK:
		{
			char data[128];
			GetDlgItemTextA(hwndDlg, IDC_TOKEN, data, SIZEOF(data));
			strcpy(token, data);

			EndDialog(hwndDlg, IDOK);
		}
			break;

		case IDCANCEL:
			EndDialog(hwndDlg, IDCANCEL);
			break;
		}
	}
		break;
	}

	return FALSE;
}

INT_PTR CALLBACK CDropbox::MainOptionsProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CDropbox *instance = (CDropbox*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (msg)
	{
	case WM_INITDIALOG:
	{
		TranslateDialogDefault(hwndDlg);

		instance = (CDropbox*)lParam;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);

		CheckDlgButton(hwndDlg, IDC_USE_SHORT_LINKS, db_get_b(NULL, MODULE, "UseSortLinks", 1));
		EnableWindow(GetDlgItem(hwndDlg, IDC_AUTHORIZE), FALSE);

		LOGFONT lf;
		HFONT hFont = (HFONT)SendDlgItemMessage(hwndDlg, IDC_AUTH_STATUS, WM_GETFONT, 0, 0);
		GetObject(hFont, sizeof(lf), &lf);
		lf.lfWeight = FW_BOLD;
		SendDlgItemMessage(hwndDlg, IDC_AUTH_STATUS, WM_SETFONT, (WPARAM)CreateFontIndirect(&lf), 0);

		if (instance->HasAccessToken())
			SetDlgItemText(hwndDlg, IDC_AUTH_STATUS, TranslateT("you are already authorized"));
		else
			SetDlgItemText(hwndDlg, IDC_AUTH_STATUS, TranslateT("you are not authorized yet"));
	}
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_GETAUTH:
			CallService(MS_UTILS_OPENURL, 0, (LPARAM)DROPBOX_WWW_URL DROPBOX_API_VER "/oauth2/authorize?response_type=code&client_id=" DROPBOX_API_KEY);
			SetFocus(GetDlgItem(hwndDlg, IDC_REQUEST_CODE));
			break;

		case IDC_REQUEST_CODE:
		{
			if ((HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus())) return 0;

			char requestToken[128];
			GetDlgItemTextA(hwndDlg, IDC_REQUEST_CODE, requestToken, SIZEOF(requestToken));
			EnableWindow(GetDlgItem(hwndDlg, IDC_AUTHORIZE), strlen(requestToken) != 0);
		}
			break;

		case IDC_AUTHORIZE:
			mir_forkthreadowner(CDropbox::RequestAcceessTokenAsync, instance, hwndDlg, 0);
			break;

		case IDC_USE_SHORT_LINKS:
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;

		}
		break;

	case WM_NOTIFY:
		if (reinterpret_cast<NMHDR*>(lParam)->code == PSN_APPLY)
			db_set_b(NULL, MODULE, "UseSortLinks", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_USE_SHORT_LINKS));
		break;
	}
	return FALSE;
}