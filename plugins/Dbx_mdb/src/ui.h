static IconItem iconList[] =
{
	{ LPGEN("Logo"), "logo", IDI_LOGO },
	{ LPGEN("Password"), "password", IDI_ICONPASS }
};

#define MS_DB_CHANGEPASSWORD "DB/UI/ChangePassword"

class COptionsDialog : public CDlgBase
{
	CCtrlCheck m_chkStandart;
	CCtrlCheck m_chkTotal;
	CCtrlButton m_btnChangePass;
	CDbxMdb *m_db;

	void OnInitDialog()
	{
		m_chkStandart.SetState(!m_db->isEncrypted());
		m_chkTotal.SetState(m_db->isEncrypted());
		m_btnChangePass.SetTextA(Translate(m_db->GetMenuTitle()));
	}

	void OnApply()
	{
		m_db->EnableEncryption(m_chkTotal.GetState() != 0);
		m_chkStandart.SetState(!m_db->isEncrypted());
		m_chkTotal.SetState(m_db->isEncrypted());
	}

	void ChangePass(CCtrlButton*)
	{
		CallService(MS_DB_CHANGEPASSWORD, 0, 0);
	}

public:
	COptionsDialog(CDbxMdb *db) :
		CDlgBase(g_hInst, IDD_OPTIONS),
		m_chkStandart(this, IDC_STANDARD),
		m_chkTotal(this, IDC_TOTAL),
		m_btnChangePass(this, IDC_USERPASS),
		m_db(db)
	{
		m_btnChangePass.OnClick = Callback(this, &COptionsDialog::ChangePass);
	}
};

class CSelectCryptoDialog : public CDlgBase
{
	CCtrlCombo m_combo;
	CCtrlData m_descr;
	CCtrlCheck m_chkTotalCrypt;
	CRYPTO_PROVIDER **m_provs;
	size_t m_provscount;
	CRYPTO_PROVIDER *m_selected;
	bool m_bTotalEncryption;

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
		m_bTotalEncryption = m_chkTotalCrypt.GetState() != 0;
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
		m_chkTotalCrypt(this, IDC_CHECK_TOTALCRYPT),
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
	inline bool TotalSelected()
	{
		return m_bTotalEncryption;
	}
};

struct DlgChangePassParam
{
	CDbxMdb *db;
	TCHAR newPass[100];
	unsigned short wrongPass;
};

class CEnterPasswordDialog : public CDlgBase
{
	CCtrlData m_header;
	CCtrlData m_language;
	CCtrlEdit m_passwordEdit;
	CCtrlButton m_buttonOK;

	DlgChangePassParam *m_param;

	INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam)
	{
		if (msg == WM_TIMER)
		{
			UINT_PTR LangID = (UINT_PTR)GetKeyboardLayout(0);
			char Lang[3] = { 0 };
			GetLocaleInfoA(MAKELCID((LangID & 0xffffffff), SORT_DEFAULT), LOCALE_SABBREVLANGNAME, Lang, 2);
			Lang[0] = toupper(Lang[0]);
			Lang[1] = tolower(Lang[1]);
			m_language.SetTextA(Lang);
			return FALSE;
		}
		else if (msg == WM_CTLCOLORSTATIC)
		{
			if ((HWND)lParam == m_language.GetHwnd()) {
				SetTextColor((HDC)wParam, GetSysColor(COLOR_HIGHLIGHTTEXT));
				SetBkMode((HDC)wParam, TRANSPARENT);
				return (INT_PTR)GetSysColorBrush(COLOR_HIGHLIGHT);
			}
		}
		return CDlgBase::DlgProc(msg, wParam, lParam);
	}

	void OnInitDialog()
	{
		m_header.SendMsg(WM_SETICON, ICON_SMALL, (LPARAM)LoadIcon(g_hInst, MAKEINTRESOURCE(iconList[0].defIconID)));
		if (m_param->wrongPass)
		{
			if (m_param->wrongPass > 2)
			{
				m_passwordEdit.Disable();
				m_buttonOK.Disable();
				m_header.SetText(TranslateT("Too many errors!"));
			}
			else
			{
				m_header.SetText(TranslateT("Password is not correct!"));
			}
		}
		else
		{
			m_header.SetText(TranslateT("Please type in your password"));
		}
		SetTimer(m_hwnd, 1, 200, NULL);
	}

	void OnOK(CCtrlButton*)
	{
		m_passwordEdit.GetText(m_param->newPass, _countof(m_param->newPass));
		EndDialog(m_hwnd, -128);
	}

	void OnDestroy()
	{
		KillTimer(m_hwnd, 1);
	}

public:
	CEnterPasswordDialog(DlgChangePassParam *param) :
		CDlgBase(g_hInst, IDD_LOGIN),
		m_header(this, IDC_HEADERBAR),
		m_language(this, IDC_LANG),
		m_passwordEdit(this, IDC_USERPASS),
		m_buttonOK(this, IDOK),
		m_param(param)
	{
		m_buttonOK.OnClick = Callback(this, &CEnterPasswordDialog::OnOK);
	}

};
