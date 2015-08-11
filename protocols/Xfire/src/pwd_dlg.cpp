//f�rs nick - dialog

#include "stdafx.h"
#include "pwd_dlg.h"

char password[256] = "";

INT_PTR CALLBACK DlgPwProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM)
{
	switch (msg) {
	case WM_INITDIALOG:
		SetWindowText(hwndDlg, LPGENT("Please enter server password..."));
		TranslateDialogDefault(hwndDlg);
		SendMessage(hwndDlg, WM_SETICON, (WPARAM)false, (LPARAM)LoadIcon(hinstance, MAKEINTRESOURCE(IDI_TM)));
		return TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK) {
			GetDlgItemTextA(hwndDlg, IDC_NICKNAME, password, _countof(password));
			EndDialog(hwndDlg, TRUE);
			return TRUE;
		}
		else if (LOWORD(wParam) == IDCANCEL) {
			EndDialog(hwndDlg, FALSE);
			return FALSE;
		}
	}
	return FALSE;
}

BOOL ShowPwdDlg(char* pw)
{
	// kein g�ltiges ziel f�r das eingegebene passwort
	if (pw == NULL)
		return FALSE;

	if (DialogBox(hinstance, MAKEINTRESOURCE(IDD_SETNICKNAME), NULL, DlgPwProc)) {
		// passwort kopieren
		if (*password == 0)
			return FALSE;

		strcpy_s(pw, 255, password);
		return TRUE;
	}

	return FALSE;
}
