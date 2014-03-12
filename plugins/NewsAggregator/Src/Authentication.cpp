/* 
Copyright (C) 2012 Mataes

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this file; see the file license.txt.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  
*/

#include "common.h"

void CreateAuthString(char *auth, MCONTACT hContact, HWND hwndDlg)
{
	DBVARIANT dbv;
	char *user = NULL, *pass = NULL;
	TCHAR *tlogin = NULL, *tpass = NULL, buf[MAX_PATH] = {0};
	if (hContact && db_get_b(hContact, MODULE, "UseAuth", 0)) {
		if (!db_get_ts(hContact, MODULE, "Login", &dbv)) {
			tlogin = mir_tstrdup(dbv.ptszVal);
			db_free(&dbv);
		}
		tpass = db_get_tsa(hContact, MODULE, "Password");
	}
	else if (hwndDlg && IsDlgButtonChecked(hwndDlg, IDC_USEAUTH)) {
		GetDlgItemText(hwndDlg, IDC_LOGIN, buf, SIZEOF(buf));
		tlogin = mir_tstrdup(buf);
		GetDlgItemText(hwndDlg, IDC_PASSWORD, buf, SIZEOF(buf));
		tpass = mir_tstrdup(buf);
	}
	user = mir_t2a(tlogin);
	pass = mir_t2a(tpass);

	char str[MAX_PATH];
	int len = mir_snprintf(str, SIZEOF(str), "%s:%s", user, pass);
	mir_free(user);
	mir_free(pass);
	mir_free(tlogin);
	mir_free(tpass);

	mir_snprintf(auth, 250, "Basic %s", ptrA(mir_base64_encode((PBYTE)str, len)));
}

INT_PTR CALLBACK AuthenticationProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		{
			TranslateDialogDefault(hwndDlg);
			ItemInfo &SelItem = *(ItemInfo*)lParam;
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)&SelItem);

			if (SelItem.hwndList) {
				TCHAR str[MAX_PATH];
				if (GetDlgItemText(SelItem.hwndList, IDC_FEEDTITLE, str, SIZEOF(str)))
					SetDlgItemText(hwndDlg, IDC_FEEDNAME, str);
				else {
					GetDlgItemText(SelItem.hwndList, IDC_FEEDURL, str, SIZEOF(str));
					SetDlgItemText(hwndDlg, IDC_FEEDNAME, str);
				}
			}
			else if (SelItem.hContact) {
				DBVARIANT dbv;
				if (!db_get_ts(SelItem.hContact, MODULE, "Nick", &dbv)) {
					SetDlgItemText(hwndDlg, IDC_FEEDNAME, dbv.ptszVal);
					db_free(&dbv);
				}
				else if (!db_get_ts(SelItem.hContact, MODULE, "URL", &dbv)) {
					SetDlgItemText(hwndDlg, IDC_FEEDNAME, dbv.ptszVal);
					db_free(&dbv);
				}
			}
		}
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
			case IDOK:
			{
				ItemInfo &SelItem = *(ItemInfo*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
				TCHAR username[MAX_PATH];
				char passw[MAX_PATH];
				if (!GetDlgItemText(hwndDlg, IDC_FEEDUSERNAME, username, SIZEOF(username))) {
					MessageBox(hwndDlg, TranslateT("Enter your username"), TranslateT("Error"), MB_OK | MB_ICONERROR);
					break;
				}
				if (!GetDlgItemTextA(hwndDlg, IDC_FEEDPASSWORD, passw, SIZEOF(passw))) {
					MessageBox(hwndDlg, TranslateT("Enter your password"), TranslateT("Error"), MB_OK | MB_ICONERROR);
					break;
				}
				if (SelItem.hwndList) {
					CheckDlgButton(SelItem.hwndList, IDC_USEAUTH, BST_CHECKED);
					EnableWindow(GetDlgItem(SelItem.hwndList, IDC_LOGIN), TRUE);
					EnableWindow(GetDlgItem(SelItem.hwndList, IDC_PASSWORD), TRUE);
					SetDlgItemText(SelItem.hwndList, IDC_LOGIN, username);
					SetDlgItemTextA(SelItem.hwndList, IDC_PASSWORD, passw);
				}
				else if (SelItem.hContact) {
					db_set_b(SelItem.hContact, MODULE, "UseAuth", 1);
					db_set_ts(SelItem.hContact, MODULE, "Login", username);
					db_set_s(SelItem.hContact, MODULE, "Password", passw);
				}
				EndDialog(hwndDlg, IDOK);
				return TRUE;
			}

			case IDCANCEL:
			{
				EndDialog(hwndDlg, IDCANCEL);
				return TRUE;
			}
		}
		break;
	}
	return FALSE;
}