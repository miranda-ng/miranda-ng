//fürs nick - dialog

#include "stdafx.h"
#include "pwd_dlg.h"

char password[256]="";

BOOL CALLBACK DlgPwProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_INITDIALOG:
		{
			SetWindowText(hwndDlg, "Please enter server password ...");
			TranslateDialogDefault(hwndDlg);
			SendMessage(hwndDlg,WM_SETICON, (WPARAM)false, (LPARAM)LoadIcon(hinstance, MAKEINTRESOURCE(IDI_TM)));
			return TRUE;
		}
		case WM_COMMAND:
		{
			if(LOWORD(wParam) == IDOK)
			{
				GetDlgItemText(hwndDlg,IDC_NICKNAME,password,sizeof(password));
				EndDialog(hwndDlg,TRUE);
				return TRUE;
			}
			else if(LOWORD(wParam) == IDCANCEL)
			{
				EndDialog(hwndDlg,FALSE);
				return FALSE;
			}
		}
	}
	return FALSE;
}

BOOL ShowPwdDlg(char* pw) {
	//kein gültiges ziel für das eingegebene passwort
	if(&pw==NULL)
		return FALSE;
	
	if(DialogBox(hinstance,MAKEINTRESOURCE(IDD_SETNICKNAME),NULL,DlgPwProc))
	{
		//passwort kopieren
		if(*password == 0)
			return FALSE;

		strcpy_s(pw,255,password);
		return TRUE;
	}

	return FALSE;
}