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

static HGENMENU hSetPwdMenu;

static UINT oldLangID;
void LanguageChanged(HWND hwndDlg)
{
	UINT_PTR LangID = (UINT_PTR)GetKeyboardLayout(0);
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

static bool CheckOldPassword(HWND hwndDlg, CDbxMDBX *db)
{
	if (db->usesPassword()) 
	{
		wchar_t buf[100];
		GetDlgItemText(hwndDlg, IDC_OLDPASS, buf, _countof(buf));
		pass_ptrA oldPass(mir_utf8encodeW(buf));
		if (!db->m_crypto->checkPassword(oldPass)) 
		{
			SetDlgItemText(hwndDlg, IDC_HEADERBAR, TranslateT("Wrong old password entered!"));
			return false;
		}
	}
	return true;
}

struct DlgChangePassParam
{
	CDbxMDBX *db;
	wchar_t newPass[100];
	unsigned short wrongPass;
};

static INT_PTR CALLBACK sttChangePassword(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	DlgChangePassParam *param = (DlgChangePassParam*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	wchar_t buf[100];

	switch (uMsg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		SendDlgItemMessage(hwndDlg, IDC_HEADERBAR, WM_SETICON, ICON_SMALL, (LPARAM)g_plugin.getIcon(IDI_LOGO, true));

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
				// param->db->WriteSignature(dbSignatureU);
				param->db->SetPassword(nullptr);
				param->db->StoreKey();
				EndDialog(hwndDlg, IDREMOVE);
			}
			break;

		case IDOK:
			wchar_t buf2[100];
			GetDlgItemText(hwndDlg, IDC_USERPASS1, buf2, _countof(buf2));
			if (wcslen(buf2) < 3) {
				SetDlgItemText(hwndDlg, IDC_HEADERBAR, TranslateT("Password is too short!"));
				goto LBL_Error;
			}

			GetDlgItemText(hwndDlg, IDC_USERPASS2, buf, _countof(buf));
			if (wcscmp(buf2, buf)) {
				SetDlgItemText(hwndDlg, IDC_HEADERBAR, TranslateT("Passwords do not match!"));
				goto LBL_Error;
			}

			if (!CheckOldPassword(hwndDlg, param->db))
				goto LBL_Error;

			// param->db->WriteSignature(dbSignatureE);
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
		Window_FreeIcon_IcoLib(GetDlgItem(hwndDlg, IDC_HEADERBAR));
	}

	return FALSE;
}

static INT_PTR ChangePassword(void* obj, WPARAM, LPARAM)
{
	CDbxMDBX *db = (CDbxMDBX*)obj;
	DlgChangePassParam param = { db };
	DialogBoxParam(g_plugin.getInst(), MAKEINTRESOURCE(db->usesPassword() ? IDD_CHANGEPASS : IDD_NEWPASS), nullptr, sttChangePassword, (LPARAM)&param);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

static INT_PTR CompactMe(void* obj, WPARAM, LPARAM)
{
	CDbxMDBX *db = (CDbxMDBX*)obj;
	if (!db->Compact())
		MessageBox(0, TranslateT("Database was compacted successfully"), TranslateT("Database"), MB_OK | MB_ICONINFORMATION);
	else
		MessageBox(0, TranslateT("Database compaction failed"), TranslateT("Database"), MB_OK | MB_ICONERROR);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

class COptionsDialog : public CDlgBase
{
	CCtrlCheck m_chkStandart;
	CCtrlCheck m_chkTotal;
	CCtrlButton m_btnChangePass;
	CDbxMDBX *m_db;

	bool OnInitDialog() override
	{
		m_chkStandart.SetState(!m_db->isEncrypted());
		m_chkTotal.SetState(m_db->isEncrypted());
		m_btnChangePass.SetTextA(Translate(m_db->GetMenuTitle()));
		return true;
	}

	bool OnApply() override
	{
		SetCursor(LoadCursor(nullptr, IDC_WAIT));
		m_db->EnableEncryption(m_chkTotal.GetState() != 0);
		SetCursor(LoadCursor(nullptr, IDC_ARROW));
		m_chkStandart.SetState(!m_db->isEncrypted());
		m_chkTotal.SetState(m_db->isEncrypted());
		return true;
	}

	void ChangePass(CCtrlButton*)
	{
		CallService(MS_DB_CHANGEPASSWORD, 0, 0);
	}

public:
	COptionsDialog(CDbxMDBX *db) :
		CDlgBase(g_plugin, IDD_OPTIONS),
		m_chkStandart(this, IDC_STANDARD),
		m_chkTotal(this, IDC_TOTAL),
		m_btnChangePass(this, IDC_USERPASS),
		m_db(db)
	{
		m_btnChangePass.OnClick = Callback(this, &COptionsDialog::ChangePass);
	}
};

static int OnOptionsInit(PVOID obj, WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = { sizeof(odp) };
	odp.position = -790000000;
	odp.flags = ODPF_BOLDGROUPS;
	odp.szTitle.a = LPGEN("Database");
	odp.pDialog = new COptionsDialog((CDbxMDBX*)obj);
	g_plugin.addOptions(wParam, &odp);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

static IconItem iconList[] =
{
	{ LPGEN("Logo"), "logo", IDI_LOGO },
	{ LPGEN("Compact"), "compact", IDI_COMPACT }
};

void CDbxMDBX::UpdateMenuItem()
{
	Menu_ModifyItem(hSetPwdMenu, _A2T(GetMenuTitle()), Skin_GetIconHandle(SKINICON_OTHER_KEYS));
}

static int OnModulesLoaded(PVOID obj, WPARAM, LPARAM)
{
	CDbxMDBX *db = (CDbxMDBX*)obj;

	g_plugin.registerIcon(LPGEN("Database"), iconList, "mdbx");

	HookEventObj(ME_OPT_INITIALISE, OnOptionsInit, db);

	CMenuItem mi(&g_plugin);

	// main menu item
	mi.root = g_plugin.addRootMenu(MO_MAIN, LPGENW("Database"), 500000000, iconList[0].hIcolib);
	Menu_ConfigureItem(mi.root, MCI_OPT_UID, "F7C5567C-D1EE-484B-B4F6-24677A5AAAEF");

	SET_UID(mi, 0x50321866, 0xba1, 0x46dd, 0xb3, 0xa6, 0xc3, 0xcc, 0x55, 0xf2, 0x42, 0x9e);
	mi.position = 1000000001;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_KEYS);
	mi.name.a = db->GetMenuTitle();
	mi.pszService = MS_DB_CHANGEPASSWORD;
	hSetPwdMenu = Menu_AddMainMenuItem(&mi);

	SET_UID(mi, 0x98c0caf3, 0xBfe5, 0x4e31, 0xac, 0xf0, 0xab, 0x95, 0xb2, 0x9b, 0x9f, 0x73);
	mi.position++;
	mi.hIcolibItem = iconList[1].hIcolib;
	mi.name.a = LPGEN("Compact");
	mi.pszService = MS_DB_COMPACT;
	hSetPwdMenu = Menu_AddMainMenuItem(&mi);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CDbxMDBX::InitDialogs()
{
	hService[0] = CreateServiceFunctionObj(MS_DB_CHANGEPASSWORD, ChangePassword, this);
	hService[1] = CreateServiceFunctionObj(MS_DB_COMPACT, CompactMe, this);

	hHook = HookEventObj(ME_SYSTEM_MODULESLOADED, OnModulesLoaded, this);
}
