/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team,
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
#include "database.h"
#include "encrypt.h"

/////////////////////////////////////////////////////////////////////////////////////////
// Provider selection dialog

class CSelectCryptoDialog : public CDlgBase
{
	CCtrlCombo m_combo;
	CCtrlData m_descr;
	CRYPTO_PROVIDER *m_selected = nullptr;

	CRYPTO_PROVIDER *getCurrent()
	{
		return (CRYPTO_PROVIDER*)m_combo.GetCurData();
	}

	bool OnInitDialog() override
	{
		for (auto &p : arCryptoProviders)
			m_combo.AddStringA(p->pszName, LPARAM(p));

		m_combo.SetCurSel(0);
		m_descr.SetText(arCryptoProviders[0].szDescr.w);
		return true;
	}

	bool OnApply() override
	{
		m_selected = getCurrent();
		return true;
	}

	void OnComboChanged(CCtrlCombo*)
	{
		m_descr.SetText(getCurrent()->szDescr.w);
	}

public:
	CSelectCryptoDialog() :
		CDlgBase(g_plugin, IDD_SELECT_CRYPTOPROVIDER),
		m_combo(this, IDC_SELECTCRYPT_COMBO),
		m_descr(this, IDC_CRYPTOPROVIDER_DESCR)
	{
		m_combo.OnChange = Callback(this, &CSelectCryptoDialog::OnComboChanged);
	}

	inline CRYPTO_PROVIDER* GetSelected() 
	{	return m_selected;
	}
};

CRYPTO_PROVIDER* MDatabaseCommon::SelectProvider()
{
	if (arCryptoProviders.getCount() == 0)
		return nullptr;

	CRYPTO_PROVIDER *pProv;
	if (arCryptoProviders.getCount() > 1) {
		CSelectCryptoDialog dlg;
		dlg.DoModal();
		pProv = dlg.GetSelected();
	}
	else pProv = &arCryptoProviders[0];

	return (StoreProvider(pProv)) ? pProv : nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////

BOOL MDatabaseCommon::IsSettingEncrypted(LPCSTR szModule, LPCSTR szSetting)
{
	if (!_strnicmp(szSetting, "password", 8))          return true;
	if (!mir_strcmp(szSetting, "NLProxyAuthPassword")) return true;
	if (!mir_strcmp(szSetting, "LNPassword"))          return true;
	if (!mir_strcmp(szSetting, "FileProxyPassword"))   return true;
	if (!mir_strcmp(szSetting, "TokenSecret"))         return true;

	if (!mir_strcmp(szModule, "SecureIM")) {
		if (!mir_strcmp(szSetting, "pgp"))              return true;
		if (!mir_strcmp(szSetting, "pgpPrivKey"))       return true;
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////

static HGENMENU hSetPwdMenu;

__forceinline wchar_t *GetMenuTitle(bool bUsesPassword)
{
	return bUsesPassword ? LPGENW("Change/remove password") : LPGENW("Set password");
}

void MDatabaseCommon::SetPassword(const wchar_t *ptszPassword)
{
	if (mir_wstrlen(ptszPassword) == 0) {
		m_bUsesPassword = false;
		m_crypto->setPassword(nullptr);
	}
	else {
		m_bUsesPassword = true;
		m_crypto->setPassword(pass_ptrA(mir_utf8encodeW(ptszPassword)));
	}

	Menu_ModifyItem(hSetPwdMenu, GetMenuTitle(m_bUsesPassword), Skin_GetIconHandle(SKINICON_OTHER_KEYS));
}

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

static bool CheckOldPassword(HWND hwndDlg, MDatabaseCommon *db)
{
	if (db->usesPassword()) {
		wchar_t buf[100];
		GetDlgItemText(hwndDlg, IDC_OLDPASS, buf, _countof(buf));
		pass_ptrA oldPass(mir_utf8encodeW(buf));
		if (!db->getCrypt()->checkPassword(oldPass)) {
			SetDlgItemText(hwndDlg, IDC_HEADERBAR, TranslateT("Wrong old password entered!"));
			return false;
		}
	}
	return true;
}

static INT_PTR CALLBACK sttChangePassword(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	MDatabaseCommon *db = (MDatabaseCommon *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (uMsg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		SendDlgItemMessage(hwndDlg, IDC_HEADERBAR, WM_SETICON, ICON_SMALL, (LPARAM)g_plugin.getIcon(IDI_DATABASE, true));
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
			if (!CheckOldPassword(hwndDlg, db)) {
LBL_Error:
				SendDlgItemMessage(hwndDlg, IDC_HEADERBAR, WM_NCPAINT, 0, 0);
				SetDlgItemTextA(hwndDlg, IDC_USERPASS1, "");
				SetDlgItemTextA(hwndDlg, IDC_USERPASS2, "");
			}
			else {
				db->SetPassword(nullptr);
				db->StoreCryptoKey();
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

			wchar_t buf[100];
			GetDlgItemText(hwndDlg, IDC_USERPASS2, buf, _countof(buf));
			if (wcscmp(buf2, buf)) {
				SetDlgItemText(hwndDlg, IDC_HEADERBAR, TranslateT("Passwords do not match!"));
				goto LBL_Error;
			}

			if (!CheckOldPassword(hwndDlg, db))
				goto LBL_Error;

			db->SetPassword(buf2);
			db->StoreCryptoKey();
			SecureZeroMemory(buf, sizeof(buf));
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
	MDatabaseCommon *db = (MDatabaseCommon*)obj;
	DialogBoxParam(g_plugin.getInst(), MAKEINTRESOURCE(db->usesPassword() ? IDD_CHANGEPASS : IDD_NEWPASS), nullptr, sttChangePassword, (LPARAM)db);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Options

class CDatabaseOptionsDialog : public CDlgBase
{
	CCtrlCheck m_chkStandart, m_chkTotal;
	CCtrlButton m_btnChangePass;
	MDatabaseCommon *m_db;

public:
	CDatabaseOptionsDialog(MDatabaseCommon *db) :
		CDlgBase(g_plugin, IDD_OPT_DATABASE),
		m_db(db),
		m_chkTotal(this, IDC_TOTAL),
		m_chkStandart(this, IDC_STANDARD),
		m_btnChangePass(this, IDC_USERPASS1)
	{
		m_btnChangePass.OnClick = Callback(this, &CDatabaseOptionsDialog::onClick_ChangePass);
	}

	bool OnInitDialog() override
	{
		m_chkStandart.SetState(!m_db->isEncrypted());
		m_chkTotal.SetState(m_db->isEncrypted());
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

	void onClick_ChangePass(CCtrlButton *)
	{
		ChangePassword(m_db, 0, 0);
	}
};

static int OnOptionsInit(PVOID obj, WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = { sizeof(odp) };
	odp.position = -790000000;
	odp.flags = ODPF_BOLDGROUPS;
	odp.szTitle.a = LPGEN("Database");
	odp.pDialog = new CDatabaseOptionsDialog((MDatabaseCommon*)obj);
	g_plugin.addOptions(wParam, &odp);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

void InitCryptMenuItem(CMenuItem &mi)
{
	auto *pDb = db_get_current();

	HookEventObj(ME_OPT_INITIALISE, OnOptionsInit, pDb);

	SET_UID(mi, 0x50321866, 0xba1, 0x46dd, 0xb3, 0xa6, 0xc3, 0xcc, 0x55, 0xf2, 0x42, 0x9e);
	mi.flags = CMIF_UNICODE;
	mi.position = 1000000001;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_KEYS);
	mi.name.w = GetMenuTitle(pDb->usesPassword());
	mi.pszService = "DB/UI/ChangePassword";
	hSetPwdMenu = Menu_AddMainMenuItem(&mi);
	CreateServiceFunctionObj(mi.pszService, ChangePassword, pDb);
}

/////////////////////////////////////////////////////////////////////////////////////////

class CEnterPasswordDialog : public CDlgBase
{
	friend class MDatabaseCommon;

	CTimer m_timer;
	CCtrlData m_header;
	CCtrlData m_language;
	CCtrlEdit m_passwordEdit;

	int m_wrongPass = 0;
	wchar_t m_newPass[100];
	MDatabaseCommon *m_db;

	void OnTimer(CTimer*)
	{
		UINT_PTR LangID = (UINT_PTR)GetKeyboardLayout(0);
		char Lang[3] = { 0 };
		GetLocaleInfoA(MAKELCID((LangID & 0xffffffff), SORT_DEFAULT), LOCALE_SABBREVLANGNAME, Lang, 2);
		Lang[0] = toupper(Lang[0]);
		Lang[1] = tolower(Lang[1]);
		m_language.SetTextA(Lang);
	}

	INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam) override
	{
		if (msg == WM_CTLCOLORSTATIC) {
			if ((HWND)lParam == m_language.GetHwnd()) {
				SetTextColor((HDC)wParam, GetSysColor(COLOR_HIGHLIGHTTEXT));
				SetBkMode((HDC)wParam, TRANSPARENT);
				return (INT_PTR)GetSysColorBrush(COLOR_HIGHLIGHT);
			}
		}
		return CDlgBase::DlgProc(msg, wParam, lParam);
	}

public:
	CEnterPasswordDialog(MDatabaseCommon *db) :
		CDlgBase(g_plugin, IDD_LOGIN),
		m_timer(this, 1),
		m_header(this, IDC_HEADERBAR),
		m_language(this, IDC_LANG),
		m_passwordEdit(this, IDC_USERPASS),
		m_db(db)
	{
		m_newPass[0] = 0;
		m_timer.OnEvent = Callback(this, &CEnterPasswordDialog::OnTimer);
	}

	~CEnterPasswordDialog()
	{
		SecureZeroMemory(m_newPass, sizeof(m_newPass));
	}

	bool OnInitDialog() override
	{
		m_header.SendMsg(WM_SETICON, ICON_SMALL, (LPARAM)g_plugin.getIcon(IDI_DATABASE, true));

		if (m_wrongPass) {
			if (m_wrongPass > 2) {
				m_passwordEdit.Disable();
				EnableWindow(GetDlgItem(m_hwnd, IDOK), false);
				m_header.SetText(TranslateT("Too many errors!"));
			}
			else m_header.SetText(TranslateT("Password is not correct!"));
		}
		else m_header.SetText(TranslateT("Please type in your password"));

		m_timer.Start(200);
		return true;
	}

	bool OnApply() override
	{
		m_passwordEdit.GetText(m_newPass, _countof(m_newPass));
		return true;
	}

	void OnDestroy() override
	{
		Window_FreeIcon_IcoLib(m_header.GetHwnd());
	}
};

int MDatabaseCommon::InitCrypt()
{
	if (m_crypto != nullptr)
		return 0;

	CRYPTO_PROVIDER *pProvider = ReadProvider();
	if (pProvider == nullptr)
		pProvider = SelectProvider();
	if (pProvider == nullptr)
		return 1;

	if ((m_crypto = pProvider->pFactory()) == nullptr)
		return 3;

	MBinBuffer key;
	BOOL bSuccess = ReadCryptoKey(key);
	if (bSuccess && (key.length() == m_crypto->getKeyLength())) {
		// first try to set a key without password
		if (!m_crypto->setKey(nullptr, (const uint8_t*)key.data(), key.length())) {
			CEnterPasswordDialog dlg(this);
			while (true) {
				if (!dlg.DoModal())
					return 4;

				pass_ptrA szPassword(mir_utf8encodeW(dlg.m_newPass));
				if (m_crypto->setKey(szPassword, (const uint8_t*)key.data(), key.length())) {
					m_bUsesPassword = true;
					break;
				}
				dlg.m_wrongPass++;
			}
		}
	}
	else {
		if (!m_crypto->generateKey())
			return 6;
		StoreCryptoKey();
	}

	m_bEncrypted = ReadEncryption();
	return 0;
}
