#include "stdafx.h"

INT_PTR CALLBACK CDropbox::MainOptionsProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CDropbox *instance = (CDropbox*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (msg)
	{
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		instance = (CDropbox*)lParam;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);

		EnableWindow(GetDlgItem(hwndDlg, IDC_AUTHORIZE), FALSE);
		{
			LOGFONT lf;
			HFONT hFont = (HFONT)SendDlgItemMessage(hwndDlg, IDC_AUTH_STATUS, WM_GETFONT, 0, 0);
			GetObject(hFont, sizeof(lf), &lf);
			lf.lfWeight = FW_BOLD;
			SendDlgItemMessage(hwndDlg, IDC_AUTH_STATUS, WM_SETFONT, (WPARAM)CreateFontIndirect(&lf), 0);

			if (instance->HasAccessToken())
				SetDlgItemText(hwndDlg, IDC_AUTH_STATUS, TranslateT("you are already authorized"));
			else
				SetDlgItemText(hwndDlg, IDC_AUTH_STATUS, TranslateT("you are not authorized yet"));

			CheckDlgButton(hwndDlg, IDC_USE_SHORT_LINKS, db_get_b(NULL, MODULE, "UseSortLinks", 1) ? BST_CHECKED : BST_UNCHECKED);
			if (db_get_b(NULL, MODULE, "UrlAutoSend", 1))
				CheckDlgButton(hwndDlg, IDC_URL_AUTOSEND, BST_CHECKED);
			else if (db_get_b(NULL, MODULE, "UrlPasteToMessageInputArea", 1))
				CheckDlgButton(hwndDlg, IDC_URL_COPYTOMIA, BST_CHECKED);
			CheckDlgButton(hwndDlg, IDC_URL_COPYTOCB, db_get_b(NULL, MODULE, "UrlCopyToClipboard", 0) ? BST_CHECKED : BST_UNCHECKED);
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
			if ((HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus())) return 0;

			char requestToken[128];
			GetDlgItemTextA(hwndDlg, IDC_REQUEST_CODE, requestToken, SIZEOF(requestToken));
			EnableWindow(GetDlgItem(hwndDlg, IDC_AUTHORIZE), strlen(requestToken) != 0);
			break;

		case IDC_AUTHORIZE:
			mir_forkthreadowner(CDropbox::RequestAccessTokenAsync, instance, hwndDlg, 0);
			break;

		case IDC_USE_SHORT_LINKS:
		case IDC_URL_AUTOSEND:
		case IDC_URL_COPYTOML:
		case IDC_URL_COPYTOCB:
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;

		}
		break;

	case WM_NOTIFY:
		if (reinterpret_cast<NMHDR*>(lParam)->code == PSN_APPLY)
		{
			db_set_b(NULL, MODULE, "UseSortLinks", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_USE_SHORT_LINKS));
			db_set_b(NULL, MODULE, "UrlAutoSend", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_URL_AUTOSEND));
			db_set_b(NULL, MODULE, "UrlPasteToMessageInputArea", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_URL_COPYTOMIA));
			db_set_b(NULL, MODULE, "UrlCopyToClipboard", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_URL_COPYTOCB));
		}
		break;
	}
	return FALSE;
}
