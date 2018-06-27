#include "stdafx.h"
#include "QuotesProviderCurrencyConverter.h"
#include <boost\property_tree\ptree.hpp>
#include <boost\property_tree\json_parser.hpp>

namespace
{
	tstring build_url(const tstring& rsURL, const tstring& from, const tstring& to)
	{
		tostringstream o;
		o << rsURL << L"?q=" << from << L"_" << to << "&compact=ultra";
		return o.str();
	}

	tstring build_url(MCONTACT hContact, const tstring& rsURL)
	{
		tstring sFrom = Quotes_DBGetStringT(hContact, QUOTES_PROTOCOL_NAME, DB_STR_FROM_ID);
		tstring sTo = Quotes_DBGetStringT(hContact, QUOTES_PROTOCOL_NAME, DB_STR_TO_ID);
		return build_url(rsURL, sFrom, sTo);
	}

	bool parse_responce(const tstring& rsJSON, double& dRate)
	{
		try
		{
			boost::property_tree::ptree pt;
			std::istringstream i_stream(quotes_t2a(rsJSON.c_str()));

			boost::property_tree::read_json(i_stream, pt);
			if (!pt.empty())
			{
				auto pt_nested = pt.begin()->second;
				dRate = pt_nested.get_value<double>();
			}
			else
			{
				dRate = pt.get_value<double>();
			}

			return true;
		}
		catch (boost::property_tree::ptree_error& )
		{
		}		
		return false;
	}

	using TWatchedRates =  std::vector<CQuotesProviderCurrencyConverter::TRateInfo>;
	TWatchedRates g_aWatchedRates;

	INT_PTR CALLBACK OptDlgProc(HWND hdlg, UINT message, WPARAM wParam, LPARAM lParam)
	{
		auto  get_provider = []()->CQuotesProviderCurrencyConverter*
		{
			auto pProviders = CModuleInfo::GetQuoteProvidersPtr();
			const auto& rapQuotesProviders = pProviders->GetProviders();
			for (auto i = rapQuotesProviders.begin(); i != rapQuotesProviders.end(); ++i) {
				const auto& pProvider = *i;
				if (auto p = dynamic_cast<CQuotesProviderCurrencyConverter*>(pProvider.get()))
				{
					return p;
				}
			}

			assert(!"We should never get here!");
			return nullptr;
		};

		auto make_quote_name = [](const CQuotesProviderBase::CQuote& rQuote)->tstring
		{
			const tstring& rsDesc = rQuote.GetName();
			return((false == rsDesc.empty()) ? rsDesc : rQuote.GetSymbol());
		};

		auto make_contact_name = [](const tstring& rsSymbolFrom, const tstring& rsSymbolTo)->tstring
		{
			tostringstream o;
			o << rsSymbolFrom << L"/" << rsSymbolTo;
			return o.str();
		};


		auto make_rate_name = [make_contact_name](const CQuotesProviderCurrencyConverter::TRateInfo& ri)->tstring
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
						CQuotesProviderCurrencyConverter::TRateInfo ri;
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

					std::for_each(aRemove.begin(), aRemove.end(), [pProvider](const auto& ri) {pProvider->WatchForRate(ri, false); });
					std::for_each(aTemp.begin(), aTemp.end(), [pProvider](const auto& ri) {pProvider->WatchForRate(ri, true); });
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

				CQuotesProviderBase::CQuoteSection rSection;
				const auto& rQuotes = pProvider->GetQuotes();
				if (rQuotes.GetSectionCount() > 0)
				{
					rSection = rQuotes.GetSection(0);
				}

				auto cQuotes = rSection.GetQuoteCount();
				for (auto i = 0u; i < cQuotes; ++i) 
				{
					const auto& rQuote = rSection.GetQuote(i);
					tstring sName = make_quote_name(rQuote);
					LPCTSTR pszName = sName.c_str();
					::SendMessage(hcbxFrom, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(pszName));
					::SendMessage(hcbxTo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(pszName));
				}

				auto cWatchedRates = pProvider->GetWatchedRateCount();
				for (auto i = 0u; i < cWatchedRates; ++i) 
				{
					CQuotesProviderCurrencyConverter::TRateInfo ri;
					if (true == pProvider->GetWatchedRateInfo(i, ri)) 
					{
						g_aWatchedRates.push_back(ri);
						tstring sRate = make_rate_name(ri);
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
					if ((CB_ERR != nFrom) && (CB_ERR != nTo) && (nFrom != nTo)) 
					{
						CQuotesProviderBase::CQuoteSection rSection;
						const auto& rQuotes = pProvider->GetQuotes();
						if (rQuotes.GetSectionCount() > 0)
						{
							rSection = rQuotes.GetSection(0);
						}

						auto cQuotes = rSection.GetQuoteCount();
						if ((nFrom < cQuotes) && (nTo < cQuotes)) 
						{
							CQuotesProviderCurrencyConverter::TRateInfo ri;
							ri.first = rSection.GetQuote(nFrom);
							ri.second = rSection.GetQuote(nTo);

							g_aWatchedRates.push_back(ri);

							tstring sRate = make_rate_name(ri);
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

}


CQuotesProviderCurrencyConverter::CQuotesProviderCurrencyConverter()
{
}


CQuotesProviderCurrencyConverter::~CQuotesProviderCurrencyConverter()
{
}

void CQuotesProviderCurrencyConverter::Accept(CQuotesProviderVisitor& visitor)const
{
	CQuotesProviderBase::Accept(visitor);
	visitor.Visit(*this);
}

void CQuotesProviderCurrencyConverter::ShowPropertyPage(WPARAM wp, OPTIONSDIALOGPAGE& odp)
{
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_DIALOG_OPT_GOOGLE);
	odp.pfnDlgProc = OptDlgProc;
	odp.szTab.w = const_cast<LPTSTR>(GetInfo().m_sName.c_str());
	g_plugin.addOptions(wp, &odp);
}

void CQuotesProviderCurrencyConverter::RefreshQuotes(TContracts& anContacts)
{
	CHTTPSession http;
	tstring sURL = GetURL();

	for (TContracts::const_iterator i = anContacts.begin(); i != anContacts.end() && IsOnline(); ++i) {
		MCONTACT hContact = *i;

		tstring sFullURL = build_url(hContact, sURL);
		if ((true == http.OpenURL(sFullURL)) && (true == IsOnline())) {
			tstring sHTML;
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

double CQuotesProviderCurrencyConverter::Convert(double dAmount, const CQuote& from, const CQuote& to)const
{
	tstring sFullURL = build_url(GetURL(), from.GetID(), to.GetID());

	CHTTPSession http;
	if ((true == http.OpenURL(sFullURL))) {
		tstring sHTML;
		if ((true == http.ReadResponce(sHTML))) {
			double dResult = 0.0;
			if ((true == parse_responce(sHTML, dResult)))
			{
				return dResult*dAmount;
			}

			throw std::runtime_error(Translate("Error occurred during HTML parsing."));
		}
		else throw std::runtime_error(Translate("Error occurred during site access."));
	}
	else throw std::runtime_error(Translate("Error occurred during site access."));

	return 0.0;
}

size_t CQuotesProviderCurrencyConverter::GetWatchedRateCount()const
{
	return m_aContacts.size();
}

bool CQuotesProviderCurrencyConverter::GetWatchedRateInfo(size_t nIndex, TRateInfo& rRateInfo)
{
	if(nIndex < m_aContacts.size()) {
		MCONTACT hContact = m_aContacts[nIndex];
		tstring sSymbolFrom = Quotes_DBGetStringT(hContact, QUOTES_PROTOCOL_NAME, DB_STR_FROM_ID);
		tstring sSymbolTo = Quotes_DBGetStringT(hContact, QUOTES_PROTOCOL_NAME, DB_STR_TO_ID);
		tstring sDescFrom = Quotes_DBGetStringT(hContact, QUOTES_PROTOCOL_NAME, DB_STR_FROM_DESCRIPTION);
		tstring sDescTo = Quotes_DBGetStringT(hContact, QUOTES_PROTOCOL_NAME, DB_STR_TO_DESCRIPTION);

		rRateInfo.first = CQuote(sSymbolFrom, sSymbolFrom, sDescFrom);
		rRateInfo.second = CQuote(sSymbolTo, sSymbolTo, sDescTo);
		return true;
	}
	else {
		return false;
	}
}

bool CQuotesProviderCurrencyConverter::WatchForRate(const TRateInfo& ri, bool bWatch)
{
	auto i = std::find_if(m_aContacts.begin(), m_aContacts.end(), [&ri](auto hContact)->bool
	{
		tstring sFrom = Quotes_DBGetStringT(hContact, QUOTES_PROTOCOL_NAME, DB_STR_FROM_ID);
		tstring sTo = Quotes_DBGetStringT(hContact, QUOTES_PROTOCOL_NAME, DB_STR_TO_ID);
		return ((0 == mir_wstrcmpi(ri.first.GetID().c_str(), sFrom.c_str()))
			&& (0 == mir_wstrcmpi(ri.second.GetID().c_str(), sTo.c_str())));
	});

	auto make_contact_name = [](const tstring& rsSymbolFrom, const tstring& rsSymbolTo)->tstring
	{
		tostringstream o;
		o << rsSymbolFrom << L"/" << rsSymbolTo;
		return o.str();
	};


	if ((true == bWatch) && (i == m_aContacts.end())) 
	{
		tstring sName = make_contact_name(ri.first.GetSymbol(), ri.second.GetSymbol());
		MCONTACT hContact = CreateNewContact(sName);
		if (hContact) 
		{
			db_set_ws(hContact, QUOTES_PROTOCOL_NAME, DB_STR_FROM_ID, ri.first.GetID().c_str());
			db_set_ws(hContact, QUOTES_PROTOCOL_NAME, DB_STR_TO_ID, ri.second.GetID().c_str());
			if (false == ri.first.GetName().empty()) 
			{
				db_set_ws(hContact, QUOTES_PROTOCOL_NAME, DB_STR_FROM_DESCRIPTION, ri.first.GetName().c_str());
			}
			if (false == ri.second.GetName().empty()) 
			{
				db_set_ws(hContact, QUOTES_PROTOCOL_NAME, DB_STR_TO_DESCRIPTION, ri.second.GetName().c_str());
			}

			return true;
		}
	}
	else if ((false == bWatch) && (i != m_aContacts.end()))
	{
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

MCONTACT CQuotesProviderCurrencyConverter::GetContactByID(const tstring& rsFromID, const tstring& rsToID)const
{
	mir_cslock lck(m_cs);

	auto i = std::find_if(m_aContacts.begin(), m_aContacts.end(), [rsFromID, rsToID](MCONTACT hContact)->bool
	{
		tstring sFrom = Quotes_DBGetStringT(hContact, QUOTES_PROTOCOL_NAME, DB_STR_FROM_ID);
		tstring sTo = Quotes_DBGetStringT(hContact, QUOTES_PROTOCOL_NAME, DB_STR_TO_ID);
		return ((0 == mir_wstrcmpi(rsFromID.c_str(), sFrom.c_str())) && (0 == mir_wstrcmpi(rsToID.c_str(), sTo.c_str())));

	});
	if (i != m_aContacts.end())
		return *i;

	return NULL;
}
