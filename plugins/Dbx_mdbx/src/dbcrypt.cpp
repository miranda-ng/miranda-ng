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

class CSelectCryptoDialog : public CDlgBase
{
	CCtrlCombo m_combo;
	CCtrlData m_descr;
	CCtrlCheck m_chkTotalCrypt;
	CRYPTO_PROVIDER **m_provs;
	size_t m_provscount;
	CRYPTO_PROVIDER *m_selected;
	bool m_bTotalEncryption;

	bool OnInitDialog() override
	{
		for (size_t i = 0; i < m_provscount; i++) {
			CRYPTO_PROVIDER *prov = m_provs[i];
			m_combo.AddStringA(prov->pszName, i);
		}
		m_combo.SetCurSel(0);
		m_descr.SetText(m_provs[0]->szDescr.w);
		return true;
	}

	bool OnApply() override
	{
		m_selected = m_provs[m_combo.GetItemData(m_combo.GetCurSel())];
		m_bTotalEncryption = m_chkTotalCrypt.GetState() != 0;
		return true;
	}

	void OnComboChanged(CCtrlCombo*)
	{
		m_descr.SetText(m_provs[m_combo.GetItemData(m_combo.GetCurSel())]->szDescr.w);
	}

public:
	CSelectCryptoDialog(CRYPTO_PROVIDER **provs, size_t count) :
		CDlgBase(g_plugin, IDD_SELECT_CRYPTOPROVIDER),
		m_combo(this, IDC_SELECTCRYPT_COMBO),
		m_descr(this, IDC_CRYPTOPROVIDER_DESCR),
		m_chkTotalCrypt(this, IDC_CHECK_TOTALCRYPT),
		m_provs(provs),
		m_provscount(count),
		m_selected(nullptr)
	{
		m_combo.OnChange = Callback(this, &CSelectCryptoDialog::OnComboChanged);
	}

	inline CRYPTO_PROVIDER* GetSelected() 
	{	return m_selected;
	}
	inline bool TotalSelected()
	{	return m_bTotalEncryption;
	}
};

char DBKey_Crypto_Provider[] = "Provider";
char DBKey_Crypto_Key[] = "Key";
char DBKey_Crypto_IsEncrypted[] = "EncryptedDB";

CRYPTO_PROVIDER* CDbxMDBX::SelectProvider()
{
	CRYPTO_PROVIDER **ppProvs, *pProv;
	int iNumProvs;
	Crypto_EnumProviders(&iNumProvs, &ppProvs);

	if (iNumProvs == 0)
		return nullptr;

	bool bTotalCrypt = false;

	if (iNumProvs > 1) {
		CSelectCryptoDialog dlg(ppProvs, iNumProvs);
		dlg.DoModal();
		pProv = dlg.GetSelected();
		bTotalCrypt = dlg.TotalSelected();
	}
	else pProv = ppProvs[0];

	{
		txn_ptr trnlck(this);
		MDBX_val key = { DBKey_Crypto_Provider, sizeof(DBKey_Crypto_Provider) }, value = { pProv->pszName, mir_strlen(pProv->pszName) + 1 };
		if (mdbx_put(trnlck, m_dbCrypto, &key, &value, MDBX_UPSERT) != MDBX_SUCCESS)
			return nullptr;

		key.iov_len = sizeof(DBKey_Crypto_IsEncrypted); key.iov_base = DBKey_Crypto_IsEncrypted; value.iov_len = sizeof(bool); value.iov_base = &bTotalCrypt;
		if (mdbx_put(trnlck, m_dbCrypto, &key, &value, MDBX_UPSERT) != MDBX_SUCCESS)
			return nullptr;
	}

	DBFlush();
	return pProv;
}

/////////////////////////////////////////////////////////////////////////////////////////

class CEnterPasswordDialog : public CDlgBase
{
	friend class CDbxMDBX;

	CTimer m_timer;
	CCtrlData m_header;
	CCtrlData m_language;
	CCtrlEdit m_passwordEdit;

	int m_wrongPass = 0;
	wchar_t m_newPass[100];
	CDbxMDBX *m_db;

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
	CEnterPasswordDialog(CDbxMDBX *db) :
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
		m_header.SendMsg(WM_SETICON, ICON_SMALL, (LPARAM)g_plugin.getIcon(IDI_LOGO, true));

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

int CDbxMDBX::InitCrypt()
{
	if (m_crypto != nullptr)
		return 0;

	CRYPTO_PROVIDER *pProvider;
	MDBX_val key = { DBKey_Crypto_Provider, sizeof(DBKey_Crypto_Provider) }, value;

	int rc = mdbx_get(StartTran(), m_dbCrypto, &key, &value);
	if (rc == MDBX_SUCCESS) {
		pProvider = Crypto_GetProvider((const char*)value.iov_base);
		if (pProvider == nullptr)
			pProvider = SelectProvider();
	}
	else pProvider = SelectProvider();

	if (pProvider == nullptr)
		return 1;

	if ((m_crypto = pProvider->pFactory()) == nullptr)
		return 3;

	key.iov_len = sizeof(DBKey_Crypto_Key); key.iov_base = DBKey_Crypto_Key;
	rc = mdbx_get(StartTran(), m_dbCrypto, &key, &value);
	if (rc == MDBX_SUCCESS && (value.iov_len == m_crypto->getKeyLength())) {
		if (!m_crypto->setKey((const BYTE*)value.iov_base, value.iov_len)) {
			CEnterPasswordDialog dlg(this);
			while (true) {
				if (!dlg.DoModal())
					return 4;
				
				m_crypto->setPassword(pass_ptrA(mir_utf8encodeW(dlg.m_newPass)));
				if (m_crypto->setKey((const BYTE*)value.iov_base, value.iov_len)) {
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
		StoreKey();
	}

	key.iov_len = sizeof(DBKey_Crypto_IsEncrypted); key.iov_base = DBKey_Crypto_IsEncrypted;
	if (mdbx_get(StartTran(), m_dbCrypto, &key, &value) == MDBX_SUCCESS)
		m_bEncrypted = *(const bool *)value.iov_base;
	else
		m_bEncrypted = false;

	InitDialogs();
	return 0;
}

void CDbxMDBX::StoreKey()
{
	size_t iKeyLength = m_crypto->getKeyLength();
	BYTE *pKey = (BYTE*)_alloca(iKeyLength);
	m_crypto->getKey(pKey, iKeyLength);
	{
		txn_ptr trnlck(this);
		MDBX_val key = { DBKey_Crypto_Key, sizeof(DBKey_Crypto_Key) }, value = { pKey, iKeyLength };
		int rc = mdbx_put(trnlck, m_dbCrypto, &key, &value, MDBX_UPSERT);
		/* FIXME: throw an exception */
		assert(rc == MDBX_SUCCESS);
		UNREFERENCED_PARAMETER(rc);
	}

	SecureZeroMemory(pKey, iKeyLength);
	DBFlush();
}

void CDbxMDBX::SetPassword(const wchar_t *ptszPassword)
{
	if (ptszPassword == nullptr || *ptszPassword == 0) {
		m_bUsesPassword = false;
		m_crypto->setPassword(nullptr);
	}
	else {
		m_bUsesPassword = true;
		m_crypto->setPassword(pass_ptrA(mir_utf8encodeW(ptszPassword)));
	}
	UpdateMenuItem();
}

/////////////////////////////////////////////////////////////////////////////////////////

int CDbxMDBX::EnableEncryption(bool bEncrypted)
{
	if (m_bEncrypted == bEncrypted)
		return 0;

	std::vector<MEVENT> lstEvents;

	MDBX_stat st;
	mdbx_dbi_stat(StartTran(), m_dbEvents, &st, sizeof(st));
	lstEvents.reserve(st.ms_entries);

	{
		MDBX_val key, data;
		cursor_ptr pCursor(StartTran(), m_dbEvents);

		while (mdbx_cursor_get(pCursor, &key, &data, MDBX_NEXT) == MDBX_SUCCESS) {
			const MEVENT hDbEvent = *(const MEVENT *)key.iov_base;
			lstEvents.push_back(hDbEvent);
		}
	}

	do {
		size_t portion = min(lstEvents.size(), 1000);

		txn_ptr trnlck(this);
		for (size_t i = 0; i < portion; i++) {
			MEVENT &hDbEvent = lstEvents[i];
			MDBX_val key = { &hDbEvent, sizeof(MEVENT) }, data;
			int rc = mdbx_get(trnlck, m_dbEvents, &key, &data);
			if (rc != MDBX_SUCCESS) {
				if (rc != MDBX_NOTFOUND)
					assert(rc == MDBX_SUCCESS);
				continue;
			}

			const DBEvent *dbEvent = (const DBEvent*)data.iov_base;
			const BYTE    *pBlob = (BYTE*)(dbEvent + 1);

			if (((dbEvent->flags & DBEF_ENCRYPTED) != 0) != bEncrypted) {
				mir_ptr<BYTE> pNewBlob;
				size_t nNewBlob;
				uint32_t dwNewFlags;

				if (dbEvent->flags & DBEF_ENCRYPTED) {
					pNewBlob = (BYTE*)m_crypto->decodeBuffer(pBlob, dbEvent->cbBlob, &nNewBlob);
					dwNewFlags = dbEvent->flags & (~DBEF_ENCRYPTED);
				}
				else {
					pNewBlob = m_crypto->encodeBuffer(pBlob, dbEvent->cbBlob, &nNewBlob);
					dwNewFlags = dbEvent->flags | DBEF_ENCRYPTED;
				}

				data.iov_len = sizeof(DBEvent) + nNewBlob;
				mir_ptr<BYTE> pData((BYTE*)mir_alloc(data.iov_len));
				data.iov_base = pData.get();

				DBEvent *pNewDBEvent = (DBEvent *)data.iov_base;
				*pNewDBEvent = *dbEvent;
				pNewDBEvent->cbBlob = (uint16_t)nNewBlob;
				pNewDBEvent->flags = dwNewFlags;
				memcpy(pNewDBEvent + 1, pNewBlob, nNewBlob);

				if (mdbx_put(trnlck, m_dbEvents, &key, &data, MDBX_UPSERT) != MDBX_SUCCESS)
					return 1;
			}
		}

		lstEvents.erase(lstEvents.begin(), lstEvents.begin()+portion);
	}
		while (lstEvents.size() > 0);

	{
		txn_ptr trnlck(this);
		MDBX_val key = { DBKey_Crypto_IsEncrypted, sizeof(DBKey_Crypto_IsEncrypted) }, value = { &bEncrypted, sizeof(bool) };
		if (mdbx_put(trnlck, m_dbCrypto, &key, &value, MDBX_UPSERT) != MDBX_SUCCESS)
			return 1;
	}

	DBFlush();
	m_bEncrypted = bEncrypted;
	return 0;
}
