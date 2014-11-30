//fürs nick - dialog

#include "stdafx.h"
#include "setnickname.h"

INT_PTR CALLBACK DlgNickProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_INITDIALOG:
	{
		TranslateDialogDefault(hwndDlg);
		SendMessage(hwndDlg, WM_SETICON, (WPARAM)false, (LPARAM)LoadIcon(hinstance, MAKEINTRESOURCE(IDI_TM)));

		DBVARIANT dbv;
		if (!db_get(NULL, protocolname, "Nick", &dbv)) {
			SetDlgItemTextA(hwndDlg, IDC_NICKNAME, dbv.pszVal);
			db_free(&dbv);
		}
		return TRUE;
	}
	case WM_COMMAND:
	{
		if (LOWORD(wParam) == IDOK)
		{
			char nick[255];
			GetDlgItemTextA(hwndDlg, IDC_NICKNAME, nick, SIZEOF(nick));

			CallService(XFIRE_SET_NICK, 0, (LPARAM)nick);

			EndDialog(hwndDlg, TRUE);
			return TRUE;
		}
		else if (LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hwndDlg, FALSE);
			return FALSE;
		}
	}
	}
	return FALSE;
}

BOOL ShowSetNick() {
	return DialogBox(hinstance, MAKEINTRESOURCE(IDD_SETNICKNAME), NULL, DlgNickProc);
}