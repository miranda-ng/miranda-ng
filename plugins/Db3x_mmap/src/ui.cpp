/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-20 Miranda NG team (https://miranda-ng.org)
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

#include "stdafx.h"

#define MS_DB_CHANGEPASSWORD "DB/UI/ChangePassword"

static IconItem iconList[] =
{
	{ LPGEN("Logo"), "logo", IDI_LOGO }
};

static HGENMENU hSetPwdMenu;

static ULONG_PTR oldLangID;
void LanguageChanged(HWND hwndDlg)
{
	ULONG_PTR LangID = (ULONG_PTR)GetKeyboardLayout(0);
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

static bool CheckOldPassword(HWND hwndDlg, CDb3Mmap *db)
{
	if (db->usesPassword()) {
		wchar_t buf[100];
		GetDlgItemText(hwndDlg, IDC_OLDPASS, buf, _countof(buf));
		if (!db->getCrypt()->checkPassword(T2Utf(buf))) {
			SetDlgItemText(hwndDlg, IDC_HEADERBAR, TranslateT("Wrong old password entered!"));
			return false;
		}
	}
	return true;
}

struct DlgChangePassParam
{
	CDb3Mmap *db;
	wchar_t newPass[100];
	int wrongPass;
};

static INT_PTR CALLBACK sttChangePassword(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	DlgChangePassParam *param = (DlgChangePassParam*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	wchar_t buf[100];

	switch (uMsg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		SendDlgItemMessage(hwndDlg, IDC_HEADERBAR, WM_SETICON, ICON_SMALL, (LPARAM)IcoLib_GetIconByHandle(iconList[0].hIcolib, true));

		param = (DlgChangePassParam*)lParam;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);

		oldLangID = 0;
		SetTimer(hwndDlg, 1, 200, nullptr);
		LanguageChanged(hwndDlg);
		return TRUE;

	case WM_CTLCOLORSTATIC:
		if ((HWND)lParam == GetDlgItem(hwndDlg, IDC_LANG)) {
			SetTextColor((HDC)wParam, GetSysColor(COLOR_HIGHLIGHTTEXT));
			SetBkMode((HDC)wParam, TRANSPARENT);
			return (INT_PTR)GetSysColorBrush(COLOR_HIGHLIGHT);
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
				param->db->SetPassword(nullptr);
				param->db->StoreCryptoKey();
				EndDialog(hwndDlg, IDREMOVE);
			}
			break;

		case IDOK:
			wchar_t buf2[100];
			GetDlgItemText(hwndDlg, IDC_USERPASS1, buf2, _countof(buf2));
			if (mir_wstrlen(buf2) < 3) {
				SetDlgItemText(hwndDlg, IDC_HEADERBAR, TranslateT("Password is too short!"));
				goto LBL_Error;
			}

			GetDlgItemText(hwndDlg, IDC_USERPASS2, buf, _countof(buf));
			if (mir_wstrcmp(buf2, buf)) {
				SetDlgItemText(hwndDlg, IDC_HEADERBAR, TranslateT("Passwords do not match!"));
				goto LBL_Error;
			}

			if (!CheckOldPassword(hwndDlg, param->db))
				goto LBL_Error;

			param->db->WriteSignature(dbSignatureE);
			param->db->SetPassword(buf2);
			param->db->StoreCryptoKey();
			SecureZeroMemory(buf2, sizeof(buf2));
			EndDialog(hwndDlg, IDOK);
		}
		break;

	case WM_TIMER:
		LanguageChanged(hwndDlg);
		return FALSE;

	case WM_DESTROY:
		KillTimer(hwndDlg, 1);
		Window_FreeIcon_IcoLib(hwndDlg);
	}

	return FALSE;
}

static INT_PTR ChangePassword(void* obj, WPARAM, LPARAM)
{
	CDb3Mmap *db = (CDb3Mmap*)obj;
	DlgChangePassParam param = { db };
	DialogBoxParam(g_plugin.getInst(), MAKEINTRESOURCE(db->usesPassword() ? IDD_CHANGEPASS : IDD_NEWPASS), nullptr, sttChangePassword, (LPARAM)&param);
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
	OPTIONSDIALOGPAGE odp = {};
	odp.position = -790000000;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS);
	odp.flags = ODPF_BOLDGROUPS;
	odp.szTitle.a = LPGEN("Database");
	odp.pfnDlgProc = DlgProcOptions;
	odp.dwInitParam = (LPARAM)obj;
	g_plugin.addOptions(wParam, &odp);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CDb3Mmap::UpdateMenuItem()
{
	Menu_ModifyItem(hSetPwdMenu, GetMenuTitle(), Skin_GetIconHandle(SKINICON_OTHER_KEYS));
}

static int OnModulesLoaded(PVOID obj, WPARAM, LPARAM)
{
	CDb3Mmap *db = (CDb3Mmap*)obj;

	g_plugin.registerIcon(LPGEN("Database"), iconList, "mmap");

	HookEventObj(ME_OPT_INITIALISE, OnOptionsInit, db);

	CMenuItem mi(&g_plugin);

	// main menu item
	mi.root = g_plugin.addRootMenu(MO_MAIN, LPGENW("Database"), 500000000, iconList[0].hIcolib);
	Menu_ConfigureItem(mi.root, MCI_OPT_UID, "F7C5567C-D1EE-484B-B4F6-24677A5AAAEF");

	SET_UID(mi, 0x50321866, 0xba1, 0x46dd, 0xb3, 0xa6, 0xc3, 0xcc, 0x55, 0xf2, 0x42, 0x9e);
	mi.flags = CMIF_UNICODE;
	mi.hIcolibItem = iconList[0].hIcolib;
	mi.name.w = db->GetMenuTitle();
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
