/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-20 Miranda NG team,
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
		return (CRYPTO_PROVIDER*)m_combo.GetItemData(m_combo.GetCurSel());
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
		if (!m_crypto->setKey((const BYTE*)key.data(), key.length())) {
			CEnterPasswordDialog dlg(this);
			while (true) {
				if (!dlg.DoModal())
					return 4;

				m_crypto->setPassword(pass_ptrA(mir_utf8encodeW(dlg.m_newPass)));
				if (m_crypto->setKey((const BYTE*)key.data(), key.length())) {
					m_bUsesPassword = true;
					SecureZeroMemory(&dlg.m_newPass, sizeof(dlg.m_newPass));
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
