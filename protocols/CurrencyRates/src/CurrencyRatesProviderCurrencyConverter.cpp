#include "stdafx.h"
#include "CurrencyRatesProviderCurrencyConverter.h"

typedef boost::shared_ptr<CAdvProviderSettings> TAdvSettingsPtr;
typedef std::map<const ICurrencyRatesProvider *, TAdvSettingsPtr> TAdvSettings;

TAdvSettings g_aAdvSettings;

CAdvProviderSettings *get_adv_settings(const ICurrencyRatesProvider *m_pProvider, bool bCreateIfNonExist)
{
	TAdvSettings::iterator i = g_aAdvSettings.find(m_pProvider);
	if (i != g_aAdvSettings.end())
		return i->second.get();

	if (true == bCreateIfNonExist) {
		TAdvSettingsPtr pAdvSet(new CAdvProviderSettings(m_pProvider));
		g_aAdvSettings.insert(std::make_pair(m_pProvider, pAdvSet));
		return pAdvSet.get();
	}

	return nullptr;
}

void remove_adv_settings(const ICurrencyRatesProvider *m_pProvider)
{
	TAdvSettings::iterator i = g_aAdvSettings.find(m_pProvider);
	if (i != g_aAdvSettings.end())
		g_aAdvSettings.erase(i);
}

CMStringW build_url(const CMStringW &rsURL, const CMStringW &from, const CMStringW &to)
{
	CMStringW res = rsURL + L"?q=" + from + L"_" + to + L"&compact=ultra";
	ptrA szApiKey(g_plugin.getStringA(DB_KEY_ApiKey));
	if (szApiKey != nullptr)
		res.AppendFormat(L"&apiKey=%S", szApiKey.get());
	return res;
}

CMStringW build_url(MCONTACT hContact, const CMStringW &rsURL)
{
	CMStringW sFrom = g_plugin.getMStringW(hContact, DB_STR_FROM_ID);
	CMStringW sTo = g_plugin.getMStringW(hContact, DB_STR_TO_ID);
	return build_url(rsURL, sFrom, sTo);
}

bool parse_response(const CMStringW &rsJSON, double &dRate)
{
	JSONNode root = JSONNode::parse(_T2A(rsJSON));
	if (!root)
		return false;

	dRate = root.at(json_index_t(0)).as_float();
	return true;
}

using TWatchedRates = std::vector<CCurrencyRatesProviderCurrencyConverter::TRateInfo>;
TWatchedRates g_aWatchedRates;

class COptionsDlg : public CDlgBase
{
	CCurrencyRatesProviderCurrencyConverter* get_provider()
	{
		for (auto &it : g_apProviders)
			if (auto p = dynamic_cast<CCurrencyRatesProviderCurrencyConverter*>(it))
				return p;

		assert(!"We should never get here!");
		return nullptr;
	};

	CMStringW make_currencyrate_name(const CCurrencyRate &rCurrencyRate)
	{
		auto &rsDesc = rCurrencyRate.GetName();
		return((false == rsDesc.IsEmpty()) ? rsDesc : rCurrencyRate.GetSymbol());
	};

	CMStringW make_contact_name(const CMStringW &rsSymbolFrom, const CMStringW &rsSymbolTo)
	{
		return rsSymbolFrom + L"/" + rsSymbolTo;
	};

	CMStringW make_rate_name(const CCurrencyRatesProviderCurrencyConverter::TRateInfo &ri)
	{
		if ((false == ri.first.GetName().IsEmpty()) && (false == ri.second.GetName().IsEmpty()))
			return make_contact_name(ri.first.GetName(), ri.second.GetName());

		return make_contact_name(ri.first.GetSymbol(), ri.second.GetSymbol());
	};

	CCurrencyRatesProviderCurrencyConverter *m_pProvider;

	CCtrlCombo cmbFrom, cmbTo;
	CCtrlButton btnAdd, btnRemove, btnDescr, btnAdvanced;
	CCtrlListBox m_list;

public:
	COptionsDlg() :
		CDlgBase(g_plugin, IDD_DIALOG_OPT_GOOGLE),
		m_pProvider(get_provider()),
		m_list(this, IDC_LIST_RATES),
		btnAdd(this, IDC_BUTTON_ADD),
		btnDescr(this, IDC_BUTTON_DESCRIPTION),
		btnRemove(this, IDC_BUTTON_REMOVE),
		btnAdvanced(this, IDC_BUTTON_ADVANCED_SETTINGS),
		cmbTo(this, IDC_COMBO_CONVERT_INTO),
		cmbFrom(this, IDC_COMBO_CONVERT_FROM)
	{
		btnAdd.OnClick = Callback(this, &COptionsDlg::onClick_Add);
		btnDescr.OnClick = Callback(this, &COptionsDlg::onClick_Descr);
		btnRemove.OnClick = Callback(this, &COptionsDlg::onClick_Remove);
		btnAdvanced.OnClick = Callback(this, &COptionsDlg::onClick_Advanced);

		cmbTo.OnSelChanged = cmbFrom.OnSelChanged = Callback(this, &COptionsDlg::onSelChange_From);
		m_list.OnSelChange = Callback(this, &COptionsDlg::onSelChange_Rates);
	}

	bool OnInitDialog() override
	{
		// set contact list display format
		::SetDlgItemTextW(m_hwnd, IDC_EDIT_CONTACT_LIST_FORMAT, g_plugin.getMStringW(DB_KEY_DisplayNameFormat, DB_DEF_DisplayNameFormat));

		// set status message display format
		::SetDlgItemTextW(m_hwnd, IDC_EDIT_STATUS_MESSAGE_FORMAT, g_plugin.getMStringW(DB_KEY_StatusMsgFormat, DB_DEF_StatusMsgFormat));

		// set tendency format
		::SetDlgItemTextW(m_hwnd, IDC_EDIT_TENDENCY_FORMAT, g_plugin.getMStringW(DB_KEY_TendencyFormat, DB_DEF_TendencyFormat));

		// set api key
		::SetDlgItemTextW(m_hwnd, IDC_EDIT_PERSONAL_KEY, g_plugin.getMStringW(DB_KEY_ApiKey));

		// refresh rate
		HWND hwndCombo = ::GetDlgItem(m_hwnd, IDC_COMBO_REFRESH_RATE);
		LPCTSTR pszRefreshRateTypes[] = { TranslateT("Seconds"), TranslateT("Minutes"), TranslateT("Hours") };
		for (int i = 0; i < _countof(pszRefreshRateTypes); ++i)
			::SendMessage(hwndCombo, CB_ADDSTRING, 0, LPARAM(pszRefreshRateTypes[i]));

		int nRefreshRateType = g_plugin.getWord(DB_KEY_RefreshRateType, RRT_MINUTES);
		if (nRefreshRateType < RRT_SECONDS || nRefreshRateType > RRT_HOURS)
			nRefreshRateType = RRT_MINUTES;

		UINT nRate = g_plugin.getWord(DB_KEY_RefreshRateValue, 1);
		switch (nRefreshRateType) {
		case RRT_SECONDS:
		case RRT_MINUTES:
			if (nRate < 1 || nRate > 60)
				nRate = 1;

			spin_set_range(::GetDlgItem(m_hwnd, IDC_SPIN_REFRESH_RATE), 1, 60);
			break;
		case RRT_HOURS:
			if (nRate < 1 || nRate > 24)
				nRate = 1;

			spin_set_range(::GetDlgItem(m_hwnd, IDC_SPIN_REFRESH_RATE), 1, 24);
			break;
		}

		::SendMessage(hwndCombo, CB_SETCURSEL, nRefreshRateType, 0);
		::SetDlgItemInt(m_hwnd, IDC_EDIT_REFRESH_RATE, nRate, FALSE);

		g_aWatchedRates.clear();

		CCurrencyRateSection rSection;
		const auto &rCurrencyRates = m_pProvider->GetCurrencyRates();
		if (rCurrencyRates.GetSectionCount() > 0) {
			rSection = rCurrencyRates.GetSection(0);
		}

		auto cCurrencyRates = rSection.GetCurrencyRateCount();
		for (auto i = 0u; i < cCurrencyRates; ++i) {
			const auto &rCurrencyRate = rSection.GetCurrencyRate(i);
			CMStringW sName = make_currencyrate_name(rCurrencyRate);
			cmbFrom.AddString(sName);
			cmbTo.AddString(sName);
		}

		auto cWatchedRates = m_pProvider->GetWatchedRateCount();
		for (auto i = 0u; i < cWatchedRates; ++i) {
			CCurrencyRatesProviderCurrencyConverter::TRateInfo ri;
			if (true == m_pProvider->GetWatchedRateInfo(i, ri)) {
				g_aWatchedRates.push_back(ri);
				CMStringW sRate = make_rate_name(ri);
				m_list.AddString(sRate);
			}
		}

		btnAdd.Disable();
		btnRemove.Disable();
		return true;
	}

	bool OnApply() override
	{
		BOOL bOk = FALSE;
		UINT nRefreshRate = ::GetDlgItemInt(m_hwnd, IDC_EDIT_REFRESH_RATE, &bOk, FALSE);
		ERefreshRateType nType = static_cast<ERefreshRateType>(::SendDlgItemMessage(m_hwnd, IDC_COMBO_REFRESH_RATE, CB_GETCURSEL, 0, 0));

		g_plugin.setWord(DB_KEY_RefreshRateType, nType);
		g_plugin.setWord(DB_KEY_RefreshRateValue, nRefreshRate);

		g_plugin.setWString(DB_KEY_DisplayNameFormat, get_window_text(::GetDlgItem(m_hwnd, IDC_EDIT_CONTACT_LIST_FORMAT)));
		g_plugin.setWString(DB_KEY_StatusMsgFormat, get_window_text(::GetDlgItem(m_hwnd, IDC_EDIT_STATUS_MESSAGE_FORMAT)));
		g_plugin.setWString(DB_KEY_TendencyFormat, get_window_text(::GetDlgItem(m_hwnd, IDC_EDIT_TENDENCY_FORMAT)));
		g_plugin.setWString(DB_KEY_ApiKey, get_window_text(::GetDlgItem(m_hwnd, IDC_EDIT_PERSONAL_KEY)));

		CAdvProviderSettings *pAdvSet = get_adv_settings(m_pProvider, false);
		if (pAdvSet)
			pAdvSet->SaveToDb();

		TWatchedRates aTemp(g_aWatchedRates);
		TWatchedRates aRemove;
		size_t cWatchedRates = m_pProvider->GetWatchedRateCount();
		for (size_t i = 0; i < cWatchedRates; ++i) {
			CCurrencyRatesProviderCurrencyConverter::TRateInfo ri;
			if (true == m_pProvider->GetWatchedRateInfo(i, ri)) {
				auto it = std::find_if(aTemp.begin(), aTemp.end(), [&ri](const auto& other)->bool
				{
					return ((0 == mir_wstrcmpi(ri.first.GetID().c_str(), other.first.GetID().c_str()))
						&& ((0 == mir_wstrcmpi(ri.second.GetID().c_str(), other.second.GetID().c_str()))));
				});
				if (it == aTemp.end()) {
					aRemove.push_back(ri);
				}
				else {
					aTemp.erase(it);
				}
			}
		}

		for (auto &it : aRemove)
			m_pProvider->WatchForRate(it, false);
		for (auto &it : aTemp)
			m_pProvider->WatchForRate(it, true);
		m_pProvider->RefreshSettings();
		return true;
	}

	void OnDestroy() override
	{
		remove_adv_settings(m_pProvider);
	}

	LRESULT DlgProc(UINT msg, WPARAM wParam, LPARAM lParam) override
	{
		if (msg == WM_NOTIFY) {
			LPNMHDR pNMHDR = reinterpret_cast<LPNMHDR>(lParam);
			switch (pNMHDR->code) {
			case PSN_KILLACTIVE:
				BOOL bOk = FALSE;
				UINT nRefreshRate = ::GetDlgItemInt(m_hwnd, IDC_EDIT_REFRESH_RATE, &bOk, FALSE);
				ERefreshRateType nType = static_cast<ERefreshRateType>(::SendDlgItemMessage(m_hwnd, IDC_COMBO_REFRESH_RATE, CB_GETCURSEL, 0, 0));
				switch (nType) {
				case RRT_MINUTES:
				case RRT_SECONDS:
					if (FALSE == bOk || nRefreshRate < 1 || nRefreshRate > 60) {
						prepare_edit_ctrl_for_error(::GetDlgItem(m_hwnd, IDC_EDIT_REFRESH_RATE));
						CurrencyRates_MessageBox(m_hwnd, TranslateT("Enter integer value between 1 and 60."), MB_OK | MB_ICONERROR);
						bOk = FALSE;
					}
					break;
				case RRT_HOURS:
					if (FALSE == bOk || nRefreshRate < 1 || nRefreshRate > 24) {
						prepare_edit_ctrl_for_error(::GetDlgItem(m_hwnd, IDC_EDIT_REFRESH_RATE));
						CurrencyRates_MessageBox(m_hwnd, TranslateT("Enter integer value between 1 and 24."), MB_OK | MB_ICONERROR);
						bOk = FALSE;
					}
					break;
				}

				if (TRUE == bOk) {
					HWND hEdit = ::GetDlgItem(m_hwnd, IDC_EDIT_CONTACT_LIST_FORMAT);
					assert(IsWindow(hEdit));

					CMStringW s = get_window_text(hEdit);
					if (s.IsEmpty()) {
						prepare_edit_ctrl_for_error(hEdit);
						CurrencyRates_MessageBox(m_hwnd, TranslateT("Enter text to display in contact list."), MB_OK | MB_ICONERROR);
						bOk = FALSE;
					}
				}

				::SetWindowLongPtr(m_hwnd, DWLP_MSGRESULT, (TRUE == bOk) ? FALSE : TRUE);
				break;
			}
		}

		return CDlgBase::DlgProc(msg, wParam, lParam);
	}

	void onSelChange_From(CCtrlCombo*)
	{
		int nFrom = cmbFrom.GetCurSel();
		int nTo = cmbTo.GetCurSel();
		btnAdd.Enable((CB_ERR != nFrom) && (CB_ERR != nTo) && (nFrom != nTo));
	}

	void onSelChange_Rates(CCtrlCombo *)
	{
		int nType = m_list.GetCurSel();
		btnRemove.Enable(LB_ERR != nType);

		switch (nType) {
		case RRT_SECONDS:
		case RRT_MINUTES:
			spin_set_range(::GetDlgItem(m_hwnd, IDC_SPIN_REFRESH_RATE), 1, 60);
			break;
		case RRT_HOURS:
			spin_set_range(::GetDlgItem(m_hwnd, IDC_SPIN_REFRESH_RATE), 1, 24);
			BOOL bOk = FALSE;
			UINT nRefreshRate = ::GetDlgItemInt(m_hwnd, IDC_EDIT_REFRESH_RATE, &bOk, FALSE);
			if (TRUE == bOk && nRefreshRate > 24)
				::SetDlgItemInt(m_hwnd, IDC_EDIT_REFRESH_RATE, 24, FALSE);
			break;
		}
	}

	void onClick_Add(CCtrlButton*)
	{
		size_t nFrom = cmbFrom.GetCurSel();
		size_t nTo = cmbTo.GetCurSel();
		if ((CB_ERR != nFrom) && (CB_ERR != nTo) && (nFrom != nTo)) {
			CCurrencyRateSection rSection;
			const auto& rCurrencyRates = m_pProvider->GetCurrencyRates();
			if (rCurrencyRates.GetSectionCount() > 0)
				rSection = rCurrencyRates.GetSection(0);

			auto cCurrencyRates = rSection.GetCurrencyRateCount();
			if ((nFrom < cCurrencyRates) && (nTo < cCurrencyRates)) {
				CCurrencyRatesProviderCurrencyConverter::TRateInfo ri;
				ri.first = rSection.GetCurrencyRate(nFrom);
				ri.second = rSection.GetCurrencyRate(nTo);

				g_aWatchedRates.push_back(ri);

				CMStringW sRate = make_rate_name(ri);
				m_list.AddString(sRate);
				NotifyChange();
			}
		}
	}

	void onClick_Advanced(CCtrlButton *)
	{
		CAdvProviderSettings *pAdvSet = get_adv_settings(m_pProvider, true);
		assert(pAdvSet);
		if (true == ShowSettingsDlg(m_hwnd, pAdvSet))
			NotifyChange();
	}

	void onClick_Descr(CCtrlButton *)
	{
		show_variable_list(m_hwnd, m_pProvider);
	}

	void onClick_Remove(CCtrlButton *)
	{
		int nSel = m_list.GetCurSel();
		if (LB_ERR != nSel) {
			m_list.DeleteString(nSel);
			if (nSel < static_cast<int>(g_aWatchedRates.size())) {
				g_aWatchedRates.erase(g_aWatchedRates.begin() + nSel);
				NotifyChange();
			}
		}

		nSel = m_list.GetCurSel();
		btnRemove.Enable(LB_ERR != nSel);
	}
};

CCurrencyRatesProviderCurrencyConverter::CCurrencyRatesProviderCurrencyConverter()
{
}

CCurrencyRatesProviderCurrencyConverter::~CCurrencyRatesProviderCurrencyConverter()
{
}

void CCurrencyRatesProviderCurrencyConverter::ShowPropertyPage(WPARAM wp, OPTIONSDIALOGPAGE &odp)
{
	odp.pDialog = new COptionsDlg();
	odp.szTab.w = const_cast<LPTSTR>(GetInfo().m_sName.c_str());
	g_plugin.addOptions(wp, &odp);
}

void CCurrencyRatesProviderCurrencyConverter::RefreshCurrencyRates(TContacts &anContacts)
{
	CHTTPSession http;
	CMStringW sURL = GetURL();

	for (TContacts::const_iterator i = anContacts.begin(); i != anContacts.end() && IsOnline(); ++i) {
		MCONTACT hContact = *i;

		CMStringW sFullURL = build_url(hContact, sURL);
		if ((true == http.OpenURL(sFullURL)) && (true == IsOnline())) {
			CMStringW sHTML;
			if ((true == http.ReadResponce(sHTML)) && (true == IsOnline())) {
				double dRate = 0.0;
				if ((true == parse_response(sHTML, dRate)) && (true == IsOnline())) {
					WriteContactRate(hContact, dRate);
					continue;
				}
			}
		}

		SetContactStatus(hContact, ID_STATUS_NA);
	}
}

double CCurrencyRatesProviderCurrencyConverter::Convert(double dAmount, const CCurrencyRate &from, const CCurrencyRate &to) const
{
	CMStringW sFullURL = build_url(GetURL(), from.GetID(), to.GetID());

	CHTTPSession http;
	if ((true == http.OpenURL(sFullURL))) {
		CMStringW sHTML;
		if ((true == http.ReadResponce(sHTML))) {
			double dResult = 0.0;
			if ((true == parse_response(sHTML, dResult)))
				return dResult * dAmount;

			throw std::runtime_error(Translate("Error occurred during HTML parsing."));
		}
		else throw std::runtime_error(Translate("Error occurred during site access."));
	}
	else throw std::runtime_error(Translate("Error occurred during site access."));

	return 0.0;
}

size_t CCurrencyRatesProviderCurrencyConverter::GetWatchedRateCount() const
{
	return m_aContacts.size();
}

bool CCurrencyRatesProviderCurrencyConverter::GetWatchedRateInfo(size_t nIndex, TRateInfo &rRateInfo)
{
	if (nIndex >= m_aContacts.size())
		return false;

	MCONTACT hContact = m_aContacts[nIndex];
	CMStringW sSymbolFrom = g_plugin.getMStringW(hContact, DB_STR_FROM_ID);
	CMStringW sSymbolTo = g_plugin.getMStringW(hContact, DB_STR_TO_ID);
	CMStringW sDescFrom = g_plugin.getMStringW(hContact, DB_STR_FROM_DESCRIPTION);
	CMStringW sDescTo = g_plugin.getMStringW(hContact, DB_STR_TO_DESCRIPTION);

	rRateInfo.first = CCurrencyRate(sSymbolFrom, sSymbolFrom, sDescFrom);
	rRateInfo.second = CCurrencyRate(sSymbolTo, sSymbolTo, sDescTo);
	return true;
}

bool CCurrencyRatesProviderCurrencyConverter::WatchForRate(const TRateInfo &ri, bool bWatch)
{
	auto i = std::find_if(m_aContacts.begin(), m_aContacts.end(), [&ri](auto hContact)->bool
	{
		CMStringW sFrom = g_plugin.getMStringW(hContact, DB_STR_FROM_ID);
		CMStringW sTo = g_plugin.getMStringW(hContact, DB_STR_TO_ID);
		return !mir_wstrcmpi(ri.first.GetID().c_str(), sFrom.c_str()) && !mir_wstrcmpi(ri.second.GetID().c_str(), sTo.c_str());
	});

	auto make_contact_name = [](const CMStringW &rsSymbolFrom, const CMStringW &rsSymbolTo)->CMStringW
	{
		return rsSymbolFrom + L"/" + rsSymbolTo;
	};

	if ((true == bWatch) && (i == m_aContacts.end())) {
		CMStringW sName = make_contact_name(ri.first.GetSymbol(), ri.second.GetSymbol());
		MCONTACT hContact = CreateNewContact(sName);
		if (hContact) {
			g_plugin.setWString(hContact, DB_STR_FROM_ID, ri.first.GetID().c_str());
			g_plugin.setWString(hContact, DB_STR_TO_ID, ri.second.GetID().c_str());
			if (false == ri.first.GetName().IsEmpty()) {
				g_plugin.setWString(hContact, DB_STR_FROM_DESCRIPTION, ri.first.GetName().c_str());
			}
			if (false == ri.second.GetName().IsEmpty()) {
				g_plugin.setWString(hContact, DB_STR_TO_DESCRIPTION, ri.second.GetName().c_str());
			}

			return true;
		}
	}
	else if ((false == bWatch) && (i != m_aContacts.end())) {
		MCONTACT hContact = *i;
		{// for CCritSection
			mir_cslock lck(m_cs);
			m_aContacts.erase(i);
		}

		db_delete_contact(hContact, true);
		return true;
	}

	return false;
}

MCONTACT CCurrencyRatesProviderCurrencyConverter::GetContactByID(const CMStringW &rsFromID, const CMStringW &rsToID) const
{
	mir_cslock lck(m_cs);

	auto i = std::find_if(m_aContacts.begin(), m_aContacts.end(), [rsFromID, rsToID](MCONTACT hContact)->bool
	{
		CMStringW sFrom = g_plugin.getMStringW(hContact, DB_STR_FROM_ID);
		CMStringW sTo = g_plugin.getMStringW(hContact, DB_STR_TO_ID);
		return !mir_wstrcmpi(rsFromID.c_str(), sFrom.c_str()) && !mir_wstrcmpi(rsToID.c_str(), sTo.c_str());
	});

	if (i != m_aContacts.end())
		return *i;

	return NULL;
}

void CCurrencyRatesProviderCurrencyConverter::FillFormat(TFormatSpecificators &array) const
{
	CSuper::FillFormat(array);

	array.push_back(CFormatSpecificator(L"%F", LPGENW("From Currency Full Name")));
	array.push_back(CFormatSpecificator(L"%f", LPGENW("From Currency Short Name")));
	array.push_back(CFormatSpecificator(L"%I", LPGENW("Into Currency Full Name")));
	array.push_back(CFormatSpecificator(L"%i", LPGENW("Into Currency Short Name")));
	array.push_back(CFormatSpecificator(L"%s", LPGENW("Short notation for \"%f/%i\"")));
}

CMStringW CCurrencyRatesProviderCurrencyConverter::FormatSymbol(MCONTACT hContact, wchar_t c, int nWidth) const
{
	switch (c) {
	case 'F':
		return g_plugin.getMStringW(hContact, DB_STR_FROM_DESCRIPTION);

	case 'f':
		return g_plugin.getMStringW(hContact, DB_STR_FROM_ID);

	case 'I':
		return g_plugin.getMStringW(hContact, DB_STR_TO_DESCRIPTION);

	case 'i':
		return g_plugin.getMStringW(hContact, DB_STR_TO_ID);
	}

	return CSuper::FormatSymbol(hContact, c, nWidth);
}

MCONTACT CCurrencyRatesProviderCurrencyConverter::ImportContact(const TiXmlNode *pRoot)
{
	const char *sFromID = nullptr, *sToID = nullptr;

	for (auto *pNode : TiXmlFilter(pRoot, "Setting")) {
		TNameValue Item = parse_setting_node(pNode);
		if (!mir_strcmpi(Item.first, DB_STR_FROM_ID))
			sFromID = Item.second;
		else if (!mir_strcmpi(Item.first, DB_STR_TO_ID))
			sToID = Item.second;
	}

	if (sFromID && sToID)
		return GetContactByID(Utf2T(sFromID).get(), Utf2T(sToID).get());

	return 0;
}
