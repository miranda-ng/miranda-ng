

class CSelectCryptoDialog : public CDlgBase
{
	CCtrlCombo m_combo;
	CCtrlData m_descr;
	CRYPTO_PROVIDER **m_provs;
	size_t m_provscount;
	CRYPTO_PROVIDER *m_selected;

	void OnInitDialog()
	{
		for (size_t i = 0; i < m_provscount; i++)
		{
			CRYPTO_PROVIDER *prov = m_provs[i];
			m_combo.AddStringA(prov->pszName, i);
		}
		m_combo.SetCurSel(0);
		m_descr.SetText(m_provs[0]->ptszDescr);
	}

	void OnClose()
	{
		m_selected = m_provs[ m_combo.GetItemData(m_combo.GetCurSel()) ];
	}

	void OnComboChanged(CCtrlCombo*)
	{
		m_descr.SetText(m_provs[m_combo.GetItemData(m_combo.GetCurSel())]->ptszDescr);
	}

public:
	CSelectCryptoDialog(CRYPTO_PROVIDER **provs, size_t count) :
		CDlgBase(g_hInst, IDD_SELECT_CRYPTOPROVIDER),
		m_combo(this, IDC_SELECTCRYPT_COMBO),
		m_descr(this, IDC_CRYPTOPROVIDER_DESCR),
		m_provs(provs),
		m_provscount(count),
		m_selected(nullptr)
	{
		m_combo.OnChange = Callback(this, &CSelectCryptoDialog::OnComboChanged);
	}

	inline CRYPTO_PROVIDER* GetSelected()
	{
		return m_selected;
	}
};