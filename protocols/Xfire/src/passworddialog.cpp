//fürs passwort dialog - dufte

#include "stdafx.h"
#include "passworddialog.h"

static char nick[255];
BOOL usenick = FALSE;

INT_PTR CALLBACK DlgPWProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static char* pw[255];
	switch (msg)
	{
	case WM_CLOSE:
	{
		GetDlgItemTextA(hwndDlg, IDC_PWSTRING, (LPSTR)pw, 254);
		if (usenick)
			GetDlgItemTextA(hwndDlg, IDC_PWNICK, (LPSTR)nick, 254);
		EndDialog(hwndDlg, (INT_PTR)pw);
		break;
	}
	case WM_INITDIALOG:
	{
		TranslateDialogDefault(hwndDlg);
		//passwort und nick leeren
		pw[0] = 0;
		nick[0] = 0;
		SendMessage(hwndDlg, WM_SETICON, (WPARAM)false, (LPARAM)LoadIcon(hinstance, MAKEINTRESOURCE(IDI_TM)));
		if (!usenick)
			EnableWindow(GetDlgItem(hwndDlg, IDC_PWNICK), FALSE);

		return TRUE;
	}
	case WM_COMMAND:
	{
		if (LOWORD(wParam) == IDC_BTN4) {
			SendMessage(hwndDlg, WM_CLOSE, 0, 0);
		}
		break;
	}
	}
	return FALSE;
}

void ShowPasswordDialog(char*pw, char*mynick) {
	if (mynick != NULL)
	{
		usenick = TRUE;
	}
	else
		usenick = FALSE;

	char* npw = (char*)DialogBox(hinstance, MAKEINTRESOURCE(IDD_PWDLG), NULL, DlgPWProc);
	strcpy(pw, npw);
	if (mynick)
	{
		strcpy(mynick, (char*)nick);
	}
}