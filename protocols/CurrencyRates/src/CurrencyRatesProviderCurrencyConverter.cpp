#include "stdafx.h"
#include "CurrencyRatesProviderCurrencyConverter.h"

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

INT_PTR CALLBACK OptDlgProc(HWND hdlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	auto get_provider = []()->CCurrencyRatesProviderCurrencyConverter*
	{
		for (auto &pProvider : g_apProviders)
			if (auto p = dynamic_cast<CCurrencyRatesProviderCurrencyConverter*>(pProvider))
				return p;

		assert(!"We should never get here!");
		return nullptr;
	};

	auto make_currencyrate_name = [](const CCurrencyRate &rCurrencyRate)->CMStringW
	{
		auto &rsDesc = rCurrencyRate.GetName();
		return((false == rsDesc.IsEmpty()) ? rsDesc : rCurrencyRate.GetSymbol());
	};

	auto make_contact_name = [](const CMStringW &rsSymbolFrom, const CMStringW &rsSymbolTo)->CMStringW 
	{
		return rsSymbolFrom + L"/" + rsSymbolTo;
	};


	auto make_rate_name = [make_contact_name](const CCurrencyRatesProviderCurrencyConverter::TRateInfo &ri)->CMStringW 
	{
		if ((false == ri.first.GetName().IsEmpty()) && (false == ri.second.GetName().IsEmpty()))
			return make_contact_name(ri.first.GetName(), ri.second.GetName());

		return make_contact_name(ri.first.GetSymbol(), ri.second.GetSymbol());
	};


	auto pProvider = get_provider();

	CCommonDlgProcData d(pProvider);
	CommonOptionDlgProc(hdlg, message, wParam, lParam, d);

	switch (message) {
	case WM_NOTIFY:
		{
			LPNMHDR pNMHDR = reinterpret_cast<LPNMHDR>(lParam);
			switch (pNMHDR->code) {
			case PSN_APPLY:
				{
					if (pProvider) {
						TWatchedRates aTemp(g_aWatchedRates);
						TWatchedRates aRemove;
						size_t cWatchedRates = pProvider->GetWatchedRateCount();
						for (size_t i = 0; i < cWatchedRates; ++i) {
							CCurrencyRatesProviderCurrencyConverter::TRateInfo ri;
							if (true == pProvider->GetWatchedRateInfo(i, ri)) {
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

						for (auto &it : aRemove) pProvider->WatchForRate(it, false);
						for (auto &it : aTemp)   pProvider->WatchForRate(it, true);
						pProvider->RefreshSettings();
					}
				}
				break;
			}
		}
		break;

	case WM_INITDIALOG:
		TranslateDialogDefault(hdlg);
		{
			g_aWatchedRates.clear();

			HWND hcbxFrom = ::GetDlgItem(hdlg, IDC_COMBO_CONVERT_FROM);
			HWND hcbxTo = ::GetDlgItem(hdlg, IDC_COMBO_CONVERT_INTO);

			CCurrencyRateSection rSection;
			const auto& rCurrencyRates = pProvider->GetCurrencyRates();
			if (rCurrencyRates.GetSectionCount() > 0) {
				rSection = rCurrencyRates.GetSection(0);
			}

			auto cCurrencyRates = rSection.GetCurrencyRateCount();
			for (auto i = 0u; i < cCurrencyRates; ++i) {
				const auto& rCurrencyRate = rSection.GetCurrencyRate(i);
				CMStringW sName = make_currencyrate_name(rCurrencyRate);
				::SendMessage(hcbxFrom, CB_ADDSTRING, 0, LPARAM(sName.c_str()));
				::SendMessage(hcbxTo, CB_ADDSTRING, 0, LPARAM(sName.c_str()));
			}

			auto cWatchedRates = pProvider->GetWatchedRateCount();
			for (auto i = 0u; i < cWatchedRates; ++i) {
				CCurrencyRatesProviderCurrencyConverter::TRateInfo ri;
				if (true == pProvider->GetWatchedRateInfo(i, ri)) {
					g_aWatchedRates.push_back(ri);
					CMStringW sRate = make_rate_name(ri);
					::SendDlgItemMessage(hdlg, IDC_LIST_RATES, LB_ADDSTRING, 0, LPARAM(sRate.c_str()));
				}
			}

			::EnableWindow(::GetDlgItem(hdlg, IDC_BUTTON_ADD), FALSE);
			::EnableWindow(::GetDlgItem(hdlg, IDC_BUTTON_REMOVE), FALSE);
		}
		return TRUE;

	case WM_COMMAND:
		switch (HIWORD(wParam)) {
		case CBN_SELCHANGE:
			switch (LOWORD(wParam)) {
			case IDC_COMBO_REFRESH_RATE:
				break;
			case IDC_COMBO_CONVERT_FROM:
			case IDC_COMBO_CONVERT_INTO:
				{
					int nFrom = static_cast<int>(::SendDlgItemMessage(hdlg, IDC_COMBO_CONVERT_FROM, CB_GETCURSEL, 0, 0));
					int nTo = static_cast<int>(::SendDlgItemMessage(hdlg, IDC_COMBO_CONVERT_INTO, CB_GETCURSEL, 0, 0));
					bool bEnableAddButton = ((CB_ERR != nFrom) && (CB_ERR != nTo) && (nFrom != nTo));
					EnableWindow(GetDlgItem(hdlg, IDC_BUTTON_ADD), bEnableAddButton);
				}
				break;
			case IDC_LIST_RATES:
				{
					int nSel = ::SendDlgItemMessage(hdlg, IDC_LIST_RATES, LB_GETCURSEL, 0, 0);
					::EnableWindow(::GetDlgItem(hdlg, IDC_BUTTON_REMOVE), (LB_ERR != nSel));
				}
				break;
			}
			break;

		case BN_CLICKED:
			switch (LOWORD(wParam)) {
			case IDC_BUTTON_ADD:
				{
					size_t nFrom = static_cast<size_t>(::SendDlgItemMessage(hdlg, IDC_COMBO_CONVERT_FROM, CB_GETCURSEL, 0, 0));
					size_t nTo = static_cast<size_t>(::SendDlgItemMessage(hdlg, IDC_COMBO_CONVERT_INTO, CB_GETCURSEL, 0, 0));
					if ((CB_ERR != nFrom) && (CB_ERR != nTo) && (nFrom != nTo)) {
						CCurrencyRateSection rSection;
						const auto& rCurrencyRates = pProvider->GetCurrencyRates();
						if (rCurrencyRates.GetSectionCount() > 0) {
							rSection = rCurrencyRates.GetSection(0);
						}

						auto cCurrencyRates = rSection.GetCurrencyRateCount();
						if ((nFrom < cCurrencyRates) && (nTo < cCurrencyRates)) {
							CCurrencyRatesProviderCurrencyConverter::TRateInfo ri;
							ri.first = rSection.GetCurrencyRate(nFrom);
							ri.second = rSection.GetCurrencyRate(nTo);

							g_aWatchedRates.push_back(ri);

							CMStringW sRate = make_rate_name(ri);
							::SendDlgItemMessage(hdlg, IDC_LIST_RATES, LB_ADDSTRING, 0, LPARAM(sRate.c_str()));
							PropSheet_Changed(::GetParent(hdlg), hdlg);
						}
					}
				}
				break;

			case IDC_BUTTON_REMOVE:
				HWND hWnd = ::GetDlgItem(hdlg, IDC_LIST_RATES);
				int nSel = ::SendMessage(hWnd, LB_GETCURSEL, 0, 0);
				if (LB_ERR != nSel) {
					if ((LB_ERR != ::SendMessage(hWnd, LB_DELETESTRING, nSel, 0))
						&& (nSel < static_cast<int>(g_aWatchedRates.size()))) {

						TWatchedRates::iterator i = g_aWatchedRates.begin();
						std::advance(i, nSel);
						g_aWatchedRates.erase(i);
						PropSheet_Changed(::GetParent(hdlg), hdlg);
					}
				}

				nSel = ::SendMessage(hWnd, LB_GETCURSEL, 0, 0);
				::EnableWindow(::GetDlgItem(hdlg, IDC_BUTTON_REMOVE), (LB_ERR != nSel));
				break;
			}
			break;
		}
		break;
	}

	return FALSE;
}

CCurrencyRatesProviderCurrencyConverter::CCurrencyRatesProviderCurrencyConverter()
{
}

CCurrencyRatesProviderCurrencyConverter::~CCurrencyRatesProviderCurrencyConverter()
{
}

void CCurrencyRatesProviderCurrencyConverter::ShowPropertyPage(WPARAM wp, OPTIONSDIALOGPAGE &odp)
{
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_DIALOG_OPT_GOOGLE);
	odp.pfnDlgProc = OptDlgProc;
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

		db_delete_contact(hContact);
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
