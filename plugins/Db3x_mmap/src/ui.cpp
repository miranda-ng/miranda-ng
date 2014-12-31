/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org)
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

struct DlgChangePassParam
{
	CDb3Mmap *db;
	TCHAR newPass[100];
	int wrongPass;
};

#define MS_DB_CHANGEPASSWORD "DB/UI/ChangePassword"

static IconItem iconList[] =
{
	{ LPGEN("Logo"), "logo", IDI_LOGO },
	{ LPGEN("Password"), "password", IDI_ICONPASS }
};

static HGENMENU hSetPwdMenu;

static UINT oldLangID;
void LanguageChanged(HWND hwndDlg)
{
	UINT LangID = (UINT)GetKeyboardLayout(0);
	char Lang[3] = { 0 };
	if (LangID != oldLangID) {
		oldLangID = LangID;
		GetLocaleInfoA(MAKELCID((LangID & 0xffffffff), SORT_DEFAULT), LOCALE_SABBREVLANGNAME, Lang, 2);
		Lang[0] = toupper(Lang[0]);
		Lang[1] = tolower(Lang[1]);
		SetDlgItemTextA(hwndDlg, IDC_LANG, Lang);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

static INT_PTR CALLBACK sttEnterPassword(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	DlgChangePassParam *param = (DlgChangePassParam*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (uMsg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		SendDlgItemMessage(hwndDlg, IDC_HEADERBAR, WM_SETICON, ICON_SMALL, (LPARAM)LoadIcon(g_hInst, MAKEINTRESOURCE(iconList[0].defIconID)));

		param = (DlgChangePassParam*)lParam;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);

		if (param->wrongPass) {
			if (param->wrongPass > 2) {
				HWND hwndCtrl = GetDlgItem(hwndDlg, IDC_USERPASS);
				EnableWindow(hwndCtrl, FALSE);
				hwndCtrl = GetDlgItem(hwndDlg, IDOK);
				EnableWindow(hwndCtrl, FALSE);
				SetDlgItemText(hwndDlg, IDC_HEADERBAR, TranslateT("Too many errors!"));
			}
			else SetDlgItemText(hwndDlg, IDC_HEADERBAR, TranslateT("Password is not correct!"));
		}
		else SetDlgItemText(hwndDlg, IDC_HEADERBAR, TranslateT("Please type in your password"));

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
			GetDlgItemText(hwndDlg, IDC_USERPASS, param->newPass, SIZEOF(param->newPass));
			EndDialog(hwndDlg, IDOK);
		}
		break;

	case WM_TIMER:
		LanguageChanged(hwndDlg);
		return FALSE;

	case WM_DESTROY:
		KillTimer(hwndDlg, 1);
		DestroyIcon((HICON)SendMessage(hwndDlg, WM_GETICON, ICON_SMALL, 0));
	}

	return FALSE;
}

bool CDb3Mmap::EnterPassword(const BYTE *pKey, const size_t keyLen)
{
	DlgChangePassParam param = { this };
	while (true) {
		// Esc pressed
		if (IDOK != DialogBoxParam(g_hInst, MAKEINTRESOURCE(IDD_LOGIN), 0, sttEnterPassword, (LPARAM)&param))
			return false;

		m_crypto->setPassword(ptrA(mir_utf8encodeT(param.newPass)));
		if (m_crypto->setKey(pKey, keyLen)) {
			m_bUsesPassword = true;
			SecureZeroMemory(&param, sizeof(param));
			return true;
		}

		param.wrongPass++;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

static bool CheckOldPassword(HWND hwndDlg, CDb3Mmap *db)
{
	if (db->usesPassword()) {
		TCHAR buf[100];
		GetDlgItemText(hwndDlg, IDC_OLDPASS, buf, SIZEOF(buf));
		ptrA oldPass(mir_utf8encodeT(buf));
		if (!db->m_crypto->checkPassword(oldPass)) {
			SetDlgItemText(hwndDlg, IDC_HEADERBAR, TranslateT("Wrong old password entered!"));
			return false;
		}
	}
	return true;
}

static INT_PTR CALLBACK sttChangePassword(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	DlgChangePassParam *param = (DlgChangePassParam*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	TCHAR buf[100];

	switch (uMsg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		SendDlgItemMessage(hwndDlg, IDC_HEADERBAR, WM_SETICON, ICON_SMALL, (LPARAM)Skin_GetIconByHandle(iconList[0].hIcolib, true));

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

		case IDREMOVE:
			if (!CheckOldPassword(hwndDlg, param->db)) {
			LBL_Error:
				SendDlgItemMessage(hwndDlg, IDC_HEADERBAR, WM_NCPAINT, 0, 0);
				SetDlgItemTextA(hwndDlg, IDC_USERPASS1, "");
				SetDlgItemTextA(hwndDlg, IDC_USERPASS2, "");
			}
			else {
				param->db->WriteSignature(dbSignatureU);
				param->db->SetPassword(NULL);
				param->db->StoreKey();
				EndDialog(hwndDlg, IDREMOVE);
			}
			break;

		case IDOK:
			TCHAR buf2[100];
			GetDlgItemText(hwndDlg, IDC_USERPASS1, buf2, SIZEOF(buf2));
			if (_tcslen(buf2) < 3) {
				SetDlgItemText(hwndDlg, IDC_HEADERBAR, TranslateT("Password is too short!"));
				goto LBL_Error;
			}

			GetDlgItemText(hwndDlg, IDC_USERPASS2, buf, SIZEOF(buf));
			if (_tcscmp(buf2, buf)) {
				SetDlgItemText(hwndDlg, IDC_HEADERBAR, TranslateT("Passwords do not match!"));
				goto LBL_Error;
			}

			if (!CheckOldPassword(hwndDlg, param->db))
				goto LBL_Error;

			param->db->WriteSignature(dbSignatureE);
			param->db->SetPassword(buf2);
			param->db->StoreKey();
			SecureZeroMemory(buf2, sizeof(buf2));
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

static INT_PTR ChangePassword(void* obj, WPARAM, LPARAM)
{
	CDb3Mmap *db = (CDb3Mmap*)obj;
	DlgChangePassParam param = { db };
	DialogBoxParam(g_hInst, MAKEINTRESOURCE(db->usesPassword() ? IDD_CHANGEPASS : IDD_NEWPASS), 0, sttChangePassword, (LPARAM)&param);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CALLBACK DlgProcOptions(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CDb3Mmap *db = (CDb3Mmap *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);

		db = (CDb3Mmap*)lParam;
		CheckRadioButton(hwndDlg, IDC_STANDARD, IDC_TOTAL, IDC_STANDARD + db->isEncrypted());
		return TRUE;

	case WM_COMMAND:
		if (HIWORD(wParam) == BN_CLICKED && (HWND)lParam == GetFocus()) {
			if (LOWORD(wParam) == IDC_USERPASS)
				CallService(MS_DB_CHANGEPASSWORD, 0, 0);
			else
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		}
		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->code == PSN_APPLY) {
			if (IsDlgButtonChecked(hwndDlg, IDC_TOTAL) != (UINT)db->isEncrypted()) {
				db->ToggleEncryption();
				CheckRadioButton(hwndDlg, IDC_STANDARD, IDC_TOTAL, IDC_STANDARD + db->isEncrypted());
			}
			break;
		}
		break;
	}

	return FALSE;
}

static int OnOptionsInit(PVOID obj, WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = { sizeof(odp) };
	odp.position = -790000000;
	odp.hInstance = g_hInst;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS);
	odp.flags = ODPF_BOLDGROUPS;
	odp.pszTitle = LPGEN("Database");
	odp.pfnDlgProc = DlgProcOptions;
	odp.dwInitParam = (LPARAM)obj;
	Options_AddPage(wParam, &odp);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CDb3Mmap::UpdateMenuItem()
{
	CLISTMENUITEM mi = { sizeof(mi) };
	mi.flags = CMIM_NAME;
	mi.icolibItem = iconList[1].hIcolib;
	mi.pszName = GetMenuTitle();
	Menu_ModifyItem(hSetPwdMenu, &mi);
}

static int OnModulesLoaded(PVOID obj, WPARAM, LPARAM)
{
	CDb3Mmap *db = (CDb3Mmap*)obj;

	Icon_Register(g_hInst, LPGEN("Database"), iconList, SIZEOF(iconList), "mmap");

	HookEventObj(ME_OPT_INITIALISE, OnOptionsInit, db);

	// main menu item
	CLISTMENUITEM mi = { sizeof(mi) };
	mi.pszName = LPGEN("Database");
	mi.position = 500000000;
	mi.flags = CMIF_ROOTHANDLE;
	mi.icolibItem = iconList[0].hIcolib;
	HGENMENU hMenuRoot = Menu_AddMainMenuItem(&mi);

	mi.icolibItem = iconList[1].hIcolib;
	mi.pszName = db->GetMenuTitle();
	mi.hParentMenu = hMenuRoot;
	mi.pszService = MS_DB_CHANGEPASSWORD;
	hSetPwdMenu = Menu_AddMainMenuItem(&mi);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CDb3Mmap::InitDialogs()
{
	hService = CreateServiceFunctionObj(MS_DB_CHANGEPASSWORD, ChangePassword, this);
	hHook = HookEventObj(ME_SYSTEM_MODULESLOADED, OnModulesLoaded, this);
}
