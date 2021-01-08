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

STDMETHODIMP_(BOOL) MDatabaseCommon::StoreProvider(CRYPTO_PROVIDER *)
{
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////

BOOL MDatabaseCommon::IsSettingEncrypted(LPCSTR szModule, LPCSTR szSetting)
{
	if (!_strnicmp(szSetting, "password", 8))      return true;
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
