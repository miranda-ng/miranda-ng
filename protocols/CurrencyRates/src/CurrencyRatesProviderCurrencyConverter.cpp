#include "stdafx.h"
#include "CurrencyRatesProviderCurrencyConverter.h"
#include <boost\property_tree\ptree.hpp>
#include <boost\property_tree\json_parser.hpp>

std::wstring build_url(const std::wstring &rsURL, const std::wstring &from, const std::wstring &to)
{
	std::wostringstream o;
	o << rsURL << L"?q=" << from << L"_" << to << "&compact=ultra";
	ptrA szApiKey(g_plugin.getStringA(DB_KEY_ApiKey));
	if (szApiKey != nullptr)
		o << "&apiKey=" << szApiKey.get();
	return o.str();
}

std::wstring build_url(MCONTACT hContact, const std::wstring &rsURL)
{
	std::wstring sFrom = CurrencyRates_DBGetStringW(hContact, MODULENAME, DB_STR_FROM_ID);
	std::wstring sTo = CurrencyRates_DBGetStringW(hContact, MODULENAME, DB_STR_TO_ID);
	return build_url(rsURL, sFrom, sTo);
}

bool parse_responce(const std::wstring &rsJSON, double &dRate)
{
	try {
		boost::property_tree::ptree pt;
		std::istringstream i_stream(currencyrates_t2a(rsJSON.c_str()));

		boost::property_tree::read_json(i_stream, pt);
		if (!pt.empty()) {
			auto pt_nested = pt.begin()->second;
			dRate = pt_nested.get_value<double>();
		}
		else {
			dRate = pt.get_value<double>();
		}

		return true;
	}
	catch (boost::property_tree::ptree_error&) {
	}
	return false;
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

	auto make_currencyrate_name = [](const CCurrencyRate &rCurrencyRate)->std::wstring
	{
		const std::wstring& rsDesc = rCurrencyRate.GetName();
		return((false == rsDesc.empty()) ? rsDesc : rCurrencyRate.GetSymbol());
	};

	auto make_contact_name = [](const std::wstring &rsSymbolFrom, const std::wstring &rsSymbolTo)->std::wstring
	{
		std::wostringstream o;
		o << rsSymbolFrom << L"/" << rsSymbolTo;
		return o.str();
	};


	auto make_rate_name = [make_contact_name](const CCurrencyRatesProviderCurrencyConverter::TRateInfo &ri)->std::wstring
	{
		if ((false == ri.first.GetName().empty()) && (false == ri.second.GetName().empty()))
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
				std::wstring sName = make_currencyrate_name(rCurrencyRate);
				LPCTSTR pszName = sName.c_str();
				::SendMessage(hcbxFrom, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(pszName));
				::SendMessage(hcbxTo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(pszName));
			}

			auto cWatchedRates = pProvider->GetWatchedRateCount();
			for (auto i = 0u; i < cWatchedRates; ++i) {
				CCurrencyRatesProviderCurrencyConverter::TRateInfo ri;
				if (true == pProvider->GetWatchedRateInfo(i, ri)) {
					g_aWatchedRates.push_back(ri);
					std::wstring sRate = make_rate_name(ri);
					LPCTSTR pszRateName = sRate.c_str();
					::SendDlgItemMessage(hdlg, IDC_LIST_RATES, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(pszRateName));
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

							std::wstring sRate = make_rate_name(ri);
							LPCTSTR pszRateName = sRate.c_str();
							::SendDlgItemMessage(hdlg, IDC_LIST_RATES, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(pszRateName));
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
	std::wstring sURL = GetURL();

	for (TContacts::const_iterator i = anContacts.begin(); i != anContacts.end() && IsOnline(); ++i) {
		MCONTACT hContact = *i;

		std::wstring sFullURL = build_url(hContact, sURL);
		if ((true == http.OpenURL(sFullURL)) && (true == IsOnline())) {
			std::wstring sHTML;
			if ((true == http.ReadResponce(sHTML)) && (true == IsOnline())) {
				double dRate = 0.0;
				if ((true == parse_responce(sHTML, dRate)) && (true == IsOnline())) {
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
	std::wstring sFullURL = build_url(GetURL(), from.GetID(), to.GetID());

	CHTTPSession http;
	if ((true == http.OpenURL(sFullURL))) {
		std::wstring sHTML;
		if ((true == http.ReadResponce(sHTML))) {
			double dResult = 0.0;
			if ((true == parse_responce(sHTML, dResult)))
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
	std::wstring sSymbolFrom = CurrencyRates_DBGetStringW(hContact, MODULENAME, DB_STR_FROM_ID);
	std::wstring sSymbolTo = CurrencyRates_DBGetStringW(hContact, MODULENAME, DB_STR_TO_ID);
	std::wstring sDescFrom = CurrencyRates_DBGetStringW(hContact, MODULENAME, DB_STR_FROM_DESCRIPTION);
	std::wstring sDescTo = CurrencyRates_DBGetStringW(hContact, MODULENAME, DB_STR_TO_DESCRIPTION);

	rRateInfo.first = CCurrencyRate(sSymbolFrom, sSymbolFrom, sDescFrom);
	rRateInfo.second = CCurrencyRate(sSymbolTo, sSymbolTo, sDescTo);
	return true;
}

bool CCurrencyRatesProviderCurrencyConverter::WatchForRate(const TRateInfo &ri, bool bWatch)
{
	auto i = std::find_if(m_aContacts.begin(), m_aContacts.end(), [&ri](auto hContact)->bool
	{
		std::wstring sFrom = CurrencyRates_DBGetStringW(hContact, MODULENAME, DB_STR_FROM_ID);
		std::wstring sTo = CurrencyRates_DBGetStringW(hContact, MODULENAME, DB_STR_TO_ID);
		return ((0 == mir_wstrcmpi(ri.first.GetID().c_str(), sFrom.c_str()))
			&& (0 == mir_wstrcmpi(ri.second.GetID().c_str(), sTo.c_str())));
	});

	auto make_contact_name = [](const std::wstring &rsSymbolFrom, const std::wstring &rsSymbolTo)->std::wstring
	{
		std::wostringstream o;
		o << rsSymbolFrom << L"/" << rsSymbolTo;
		return o.str();
	};


	if ((true == bWatch) && (i == m_aContacts.end())) {
		std::wstring sName = make_contact_name(ri.first.GetSymbol(), ri.second.GetSymbol());
		MCONTACT hContact = CreateNewContact(sName);
		if (hContact) {
			g_plugin.setWString(hContact, DB_STR_FROM_ID, ri.first.GetID().c_str());
			g_plugin.setWString(hContact, DB_STR_TO_ID, ri.second.GetID().c_str());
			if (false == ri.first.GetName().empty()) {
				g_plugin.setWString(hContact, DB_STR_FROM_DESCRIPTION, ri.first.GetName().c_str());
			}
			if (false == ri.second.GetName().empty()) {
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

MCONTACT CCurrencyRatesProviderCurrencyConverter::GetContactByID(const std::wstring& rsFromID, const std::wstring& rsToID) const
{
	mir_cslock lck(m_cs);

	auto i = std::find_if(m_aContacts.begin(), m_aContacts.end(), [rsFromID, rsToID](MCONTACT hContact)->bool
	{
		std::wstring sFrom = CurrencyRates_DBGetStringW(hContact, MODULENAME, DB_STR_FROM_ID);
		std::wstring sTo = CurrencyRates_DBGetStringW(hContact, MODULENAME, DB_STR_TO_ID);
		return ((0 == mir_wstrcmpi(rsFromID.c_str(), sFrom.c_str())) && (0 == mir_wstrcmpi(rsToID.c_str(), sTo.c_str())));
	});

	if (i != m_aContacts.end())
		return *i;

	return NULL;
}

void CCurrencyRatesProviderCurrencyConverter::FillFormat(TFormatSpecificators &array) const
{
	CSuper::FillFormat(array);

	array.push_back(CFormatSpecificator(L"%F", TranslateT("From Currency Full Name")));
	array.push_back(CFormatSpecificator(L"%f", TranslateT("From Currency Short Name")));
	array.push_back(CFormatSpecificator(L"%I", TranslateT("Into Currency Full Name")));
	array.push_back(CFormatSpecificator(L"%i", TranslateT("Into Currency Short Name")));
	array.push_back(CFormatSpecificator(L"%s", TranslateT("Short notation for \"%f/%i\"")));
}

std::wstring CCurrencyRatesProviderCurrencyConverter::FormatSymbol(MCONTACT hContact, wchar_t c, int nWidth) const
{
	switch (c) {
	case 'F':
		return CurrencyRates_DBGetStringW(hContact, MODULENAME, DB_STR_FROM_DESCRIPTION);

	case 'f':
		return CurrencyRates_DBGetStringW(hContact, MODULENAME, DB_STR_FROM_ID);

	case 'I':
		return CurrencyRates_DBGetStringW(hContact, MODULENAME, DB_STR_TO_DESCRIPTION);

	case 'i':
		return CurrencyRates_DBGetStringW(hContact, MODULENAME, DB_STR_TO_ID);
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
