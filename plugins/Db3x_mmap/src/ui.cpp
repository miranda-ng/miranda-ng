/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright 2012-13 Miranda NG project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "commonheaders.h"

#define MS_DB_CHANGEPASSWORD "DB/ChangePassword"

static IconItem iconList[] =
{
	{ LPGEN("Logo"),     "logo",     IDI_LOGO     },
	{ LPGEN("Password"), "password", IDI_ICONPASS }
};

static HGENMENU hSetPwdMenu;

static int oldLangID;
void LanguageChanged(HWND hDlg)
{
	UINT LangID = (UINT)GetKeyboardLayout(0);
	char Lang[3] = { 0 };
	if (LangID != oldLangID) {
		oldLangID = LangID;
		GetLocaleInfoA(MAKELCID((LangID & 0xffffffff), SORT_DEFAULT), LOCALE_SABBREVLANGNAME, Lang, 2);
		Lang[0] = toupper(Lang[0]);
		Lang[1] = tolower(Lang[1]);
		SetDlgItemTextA(hDlg, IDC_LANG, Lang);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

struct DlgChangePassParam
{
	CDb3Mmap *db;
	TCHAR newPass[100];
};

static INT_PTR CALLBACK sttChangePassword(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	DlgChangePassParam *param = (DlgChangePassParam*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (uMsg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		SendMessage(GetDlgItem(hwndDlg, IDC_HEADERBAR), WM_SETICON, ICON_SMALL, (LPARAM)Skin_GetIconByHandle(iconList[0].hIcolib));

		param = (DlgChangePassParam*)lParam;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);

		oldLangID = 0;
		SetTimer(hwndDlg, 1, 200, NULL);
		LanguageChanged(hwndDlg);
		return TRUE;

	case WM_CTLCOLORSTATIC:
		if ((HWND)lParam == GetDlgItem(hwndDlg, IDC_LANG)) {
			SetTextColor((HDC)wParam, GetSysColor(COLOR_HIGHLIGHTTEXT));
			SetBkMode((HDC)wParam, TRANSPARENT);
			return (BOOL)GetSysColorBrush(COLOR_HIGHLIGHT);
		}
		return FALSE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDCANCEL:
			EndDialog(hwndDlg, IDCANCEL);
			break;

		case IDOK:
			GetDlgItemText(hwndDlg, IDC_USERPASS1, param->newPass, SIZEOF(param->newPass));
			if (_tcslen(param->newPass) < 3) {
				SetDlgItemText(hwndDlg, IDC_HEADERBAR, TranslateT("Password is too short!"));
LBL_Error:
				SendDlgItemMessage(hwndDlg, IDC_HEADERBAR, WM_NCPAINT, 0, 0);
				SetDlgItemTextA(hwndDlg, IDC_USERPASS1, "");
				SetDlgItemTextA(hwndDlg, IDC_USERPASS2, "");
				break;
			}

			TCHAR buf2[100];
			GetDlgItemText(hwndDlg, IDC_USERPASS2, buf2, SIZEOF(buf2));
			if (_tcscmp(param->newPass, buf2)) {
				SetWindowText(GetDlgItem(hwndDlg, IDC_HEADERBAR), TranslateT("Passwords do not match!"));
				goto LBL_Error;
			}

			if (param->db->isEncrypted()) {
				GetDlgItemText(hwndDlg, IDC_OLDPASS, buf2, SIZEOF(buf2));
				ptrA oldPass(mir_utf8encodeT(buf2));
				if (!param->db->m_crypto->checkPassword(oldPass)) {
					SetWindowText(GetDlgItem(hwndDlg, IDC_HEADERBAR), TranslateT("Wrong old password entered!"));
					goto LBL_Error;
				}
			}
			EndDialog(hwndDlg, IDOK);
		}
		break;

	case WM_TIMER:
		LanguageChanged(hwndDlg);
		return FALSE;

	case WM_DESTROY:
		KillTimer(hwndDlg, 1);
		Skin_ReleaseIcon((HICON)SendMessage(hwndDlg, WM_GETICON, ICON_SMALL, 0));
	}

	return FALSE;
}

static INT_PTR ChangePassword(void* obj, LPARAM, LPARAM)
{
	CDb3Mmap *db = (CDb3Mmap*)obj;
	DlgChangePassParam param = { db };
	if (IDOK == DialogBoxParam(g_hInst, MAKEINTRESOURCE(db->isEncrypted() ? IDD_CHANGEPASS : IDD_NEWPASS), 0, sttChangePassword, (LPARAM)&param)) {
		ptrA newPass(mir_utf8encodeT(param.newPass));
		db->m_crypto->setPassword(newPass);
		// db->StoreKey();
	}
		
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

static int OnModulesLoaded(PVOID obj, WPARAM, LPARAM)
{
	CDb3Mmap *db = (CDb3Mmap*)obj;

	Icon_Register(g_hInst, LPGEN("Database"), iconList, SIZEOF(iconList), "mmap");

	// main menu item
	CLISTMENUITEM mi = { sizeof(mi) };
	mi.flags = CMIM_ALL | CMIF_TCHAR;
	mi.icolibItem = iconList[1].hIcolib;
	mi.ptszName = (db->isEncrypted()) ? LPGENT("Change password") : LPGENT("Set password");
	mi.ptszPopupName = LPGENT("Database");
	mi.pszService = MS_DB_CHANGEPASSWORD;
	mi.position = 500000000;
	hSetPwdMenu = Menu_AddMainMenuItem(&mi);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CDb3Mmap::InitDialogs()
{
	CreateServiceFunctionObj(MS_DB_CHANGEPASSWORD, ChangePassword, this);
	HookEventObj(ME_SYSTEM_MODULESLOADED, OnModulesLoaded, this);
}
