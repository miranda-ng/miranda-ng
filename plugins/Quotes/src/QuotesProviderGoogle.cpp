#include "StdAfx.h"

CQuotesProviderGoogle::CQuotesProviderGoogle()
{
}

CQuotesProviderGoogle::~CQuotesProviderGoogle()
{
}

namespace
{
	inline tstring make_contact_name(const tstring& rsSymbolFrom, const tstring& rsSymbolTo)
	{
		tostringstream o;
		o << rsSymbolFrom << _T("/") << rsSymbolTo;
		return o.str();
	}

	inline bool is_rate_watched(MCONTACT hContact,
		const CQuotesProviderBase::CQuote& from,
		const CQuotesProviderBase::CQuote& to)
	{
		tstring sFrom = Quotes_DBGetStringT(hContact, QUOTES_PROTOCOL_NAME, DB_STR_FROM_ID);
		tstring sTo = Quotes_DBGetStringT(hContact, QUOTES_PROTOCOL_NAME, DB_STR_TO_ID);
		return ((0 == quotes_stricmp(from.GetID().c_str(), sFrom.c_str()))
			&& (0 == quotes_stricmp(to.GetID().c_str(), sTo.c_str())));
	}
}

bool CQuotesProviderGoogle::WatchForRate(const CRateInfo& ri,
	bool bWatch)
{
	TContracts::const_iterator i = std::find_if(m_aContacts.begin(), m_aContacts.end(),
		boost::bind(is_rate_watched, _1, ri.m_from, ri.m_to));
	if ((true == bWatch) && (i == m_aContacts.end()))
	{
		tstring sName = make_contact_name(ri.m_from.GetSymbol(), ri.m_to.GetSymbol());
		MCONTACT hContact = CreateNewContact(sName);
		if (hContact)
		{
			db_set_ts(hContact, QUOTES_PROTOCOL_NAME, DB_STR_FROM_ID, ri.m_from.GetID().c_str());
			db_set_ts(hContact, QUOTES_PROTOCOL_NAME, DB_STR_TO_ID, ri.m_to.GetID().c_str());
			if (false == ri.m_from.GetName().empty())
			{
				db_set_ts(hContact, QUOTES_PROTOCOL_NAME, DB_STR_FROM_DESCRIPTION, ri.m_from.GetName().c_str());
			}
			if (false == ri.m_to.GetName().empty())
			{
				db_set_ts(hContact, QUOTES_PROTOCOL_NAME, DB_STR_TO_DESCRIPTION, ri.m_to.GetName().c_str());
			}

			return true;
		}
	}
	else if ((false == bWatch) && (i != m_aContacts.end()))
	{
		MCONTACT hContact = *i;
		{// for CCritSection
			CGuard<CLightMutex> cs(m_cs);
			m_aContacts.erase(i);
		}

		CallService(MS_DB_CONTACT_DELETE, WPARAM(hContact), 0);
		return true;
	}

	return false;
}

size_t CQuotesProviderGoogle::GetWatchedRateCount()const
{
	return m_aContacts.size();
}

bool CQuotesProviderGoogle::GetWatchedRateInfo(size_t nIndex, CRateInfo& rRateInfo)
{
	if (nIndex < m_aContacts.size())
	{
		MCONTACT hContact = m_aContacts[nIndex];
		tstring sSymbolFrom = Quotes_DBGetStringT(hContact, QUOTES_PROTOCOL_NAME, DB_STR_FROM_ID);
		tstring sSymbolTo = Quotes_DBGetStringT(hContact, QUOTES_PROTOCOL_NAME, DB_STR_TO_ID);
		tstring sDescFrom = Quotes_DBGetStringT(hContact, QUOTES_PROTOCOL_NAME, DB_STR_FROM_DESCRIPTION);
		tstring sDescTo = Quotes_DBGetStringT(hContact, QUOTES_PROTOCOL_NAME, DB_STR_TO_DESCRIPTION);

		rRateInfo.m_from = CQuote(sSymbolFrom, sSymbolFrom, sDescFrom);
		rRateInfo.m_to = CQuote(sSymbolTo, sSymbolTo, sDescTo);
		return true;
	}
	else
	{
		return false;
	}
}

namespace
{
	tstring build_url(const tstring& rsURL, const tstring& from, const tstring& to, double dAmount)
	{
		tostringstream o;
		o << rsURL << _T("?a=") << std::fixed << dAmount << _T("&from=") << from << _T("&to=") << to;
		return o.str();
	}
	tstring build_url(MCONTACT hContact, const tstring& rsURL, double dAmount = 1.0)
	{
		tstring sFrom = Quotes_DBGetStringT(hContact, QUOTES_PROTOCOL_NAME, DB_STR_FROM_ID);
		tstring sTo = Quotes_DBGetStringT(hContact, QUOTES_PROTOCOL_NAME, DB_STR_TO_ID);
		return build_url(rsURL, sFrom, sTo, dAmount);
	}

	typedef IHTMLNode::THTMLNodePtr THTMLNodePtr;

	bool parse_html_node(const THTMLNodePtr& pNode, double& rdRate)
	{
		tstring sID = pNode->GetAttribute(_T("id"));
		if ((false == sID.empty()) && (0 == quotes_stricmp(sID.c_str(), _T("currency_converter_result"))))
		{
			size_t cChild = pNode->GetChildCount();
			// 			assert(1 == cChild);
			if (cChild > 0)
			{
				THTMLNodePtr pChild = pNode->GetChildPtr(0);
				tstring sRate = pChild->GetText();

				tistringstream input(sRate);
				input >> rdRate;

				return ((false == input.bad()) && (false == input.fail()));
			}
		}
		else
		{
			size_t cChild = pNode->GetChildCount();
			for (size_t i = 0; i < cChild; ++i)
			{
				THTMLNodePtr pChild = pNode->GetChildPtr(i);
				if (pChild && (true == parse_html_node(pChild, rdRate)))
				{
					return true;
				}
			}
		}

		return false;
	}

	bool parse_responce(const tstring& rsHTML, double& rdRate)
	{
		IHTMLEngine::THTMLParserPtr pHTMLParser = CModuleInfo::GetHTMLEngine()->GetParserPtr();
		THTMLNodePtr pRoot = pHTMLParser->ParseString(rsHTML);
		if (pRoot)
		{
			return parse_html_node(pRoot, rdRate);
		}
		else
		{
			return false;
		}
	}
}

void CQuotesProviderGoogle::RefreshQuotes(TContracts& anContacts)
{
	CHTTPSession http;
	tstring sURL = GetURL();

	bool bUseExtendedStatus = CModuleInfo::GetInstance().GetExtendedStatusFlag();

	for (TContracts::const_iterator i = anContacts.begin(); i != anContacts.end() && IsOnline(); ++i)
	{
		MCONTACT hContact = *i;

		if (bUseExtendedStatus)
		{
			SetContactStatus(hContact, ID_STATUS_OCCUPIED);
		}

		tstring sFullURL = build_url(hContact, sURL);
		// 		LogIt(Info,sFullURL);
		if ((true == http.OpenURL(sFullURL)) && (true == IsOnline()))
		{
			tstring sHTML;
			if ((true == http.ReadResponce(sHTML)) && (true == IsOnline()))
			{
				// 				LogIt(Info,sHTML);

				double dRate = 0.0;
				if ((true == parse_responce(sHTML, dRate)) && (true == IsOnline()))
				{
					WriteContactRate(hContact, dRate);
					continue;
				}
			}
		}

		SetContactStatus(hContact, ID_STATUS_NA);
	}
}

namespace
{
	inline tstring make_quote_name(const CQuotesProviderGoogle::CQuote& rQuote)
	{
		const tstring& rsDesc = rQuote.GetName();
		return((false == rsDesc.empty()) ? rsDesc : rQuote.GetSymbol());
	}

	CQuotesProviderGoogle* get_google_provider()
	{
		CModuleInfo::TQuotesProvidersPtr& pProviders = CModuleInfo::GetQuoteProvidersPtr();
		const CQuotesProviders::TQuotesProviders& rapQuotesProviders = pProviders->GetProviders();
		for (CQuotesProviders::TQuotesProviders::const_iterator i = rapQuotesProviders.begin(); i != rapQuotesProviders.end(); ++i)
		{
			const CQuotesProviders::TQuotesProviderPtr& pProvider = *i;
			CQuotesProviderGoogle* pGoogle = dynamic_cast<CQuotesProviderGoogle*>(pProvider.get());
			if (pGoogle)
			{
				return pGoogle;
			}
		}

		assert(!"We should never get here!");
		return NULL;
	}

	CQuotesProviderGoogle::CQuoteSection get_quotes()
	{
		const CQuotesProviderGoogle* pProvider = get_google_provider();
		if (pProvider)
		{
			const CQuotesProviderGoogle::CQuoteSection& rQuotes = pProvider->GetQuotes();
			if (rQuotes.GetSectionCount() > 0)
			{
				return rQuotes.GetSection(0);
			}
		}

		return CQuotesProviderGoogle::CQuoteSection();
	}

	tstring make_rate_name(const CQuotesProviderGoogle::CQuote& rFrom,
		const CQuotesProviderGoogle::CQuote& rTo)
	{
		if ((false == rFrom.GetName().empty()) && (false == rTo.GetName().empty()))
		{
			return make_contact_name(rFrom.GetName(), rTo.GetName());
		}
		else
		{
			return make_contact_name(rFrom.GetSymbol(), rTo.GetSymbol());
		}
	}

	typedef std::vector<CQuotesProviderGoogle::CRateInfo> TWatchedRates;
	TWatchedRates g_aWatchedRates;

	bool is_equal_rate(const CQuotesProviderGoogle::CRateInfo& riL, const CQuotesProviderGoogle::CRateInfo& riR)
	{
		return ((0 == quotes_stricmp(riL.m_from.GetID().c_str(), riR.m_from.GetID().c_str()))
			&& ((0 == quotes_stricmp(riL.m_to.GetID().c_str(), riR.m_to.GetID().c_str()))));
	}

	INT_PTR CALLBACK GoogleOptDlgProc(HWND hdlg, UINT message, WPARAM wParam, LPARAM lParam)
	{
		CQuotesProviderGoogle* pProvider = get_google_provider();

		CCommonDlgProcData d(pProvider);
		CommonOptionDlgProc(hdlg, message, wParam, lParam, d);

		switch (message)
		{
		case WM_NOTIFY:
		{
			LPNMHDR pNMHDR = reinterpret_cast<LPNMHDR>(lParam);
			switch (pNMHDR->code)
			{
			case PSN_APPLY:
			{
				if (pProvider)
				{
					TWatchedRates aTemp(g_aWatchedRates);
					TWatchedRates aRemove;
					size_t cWatchedRates = pProvider->GetWatchedRateCount();
					for (size_t i = 0; i < cWatchedRates; ++i)
					{
						CQuotesProviderGoogle::CRateInfo ri;
						if (true == pProvider->GetWatchedRateInfo(i, ri))
						{
							TWatchedRates::iterator it =
								std::find_if(aTemp.begin(), aTemp.end(),
								boost::bind(is_equal_rate, _1, boost::cref(ri)));
							if (it == aTemp.end())
							{
								aRemove.push_back(ri);
							}
							else
							{
								aTemp.erase(it);
							}
						}
					}

					std::for_each(aRemove.begin(), aRemove.end(), boost::bind(&CQuotesProviderGoogle::WatchForRate, pProvider, _1, false));
					std::for_each(aTemp.begin(), aTemp.end(), boost::bind(&CQuotesProviderGoogle::WatchForRate, pProvider, _1, true));

					pProvider->RefreshSettings();
				}
			}
			break;
			}
		}
		break;
		case WM_INITDIALOG:
		{
			TranslateDialogDefault(hdlg);

			g_aWatchedRates.clear();

			HWND hcbxFrom = ::GetDlgItem(hdlg, IDC_COMBO_CONVERT_FROM);
			HWND hcbxTo = ::GetDlgItem(hdlg, IDC_COMBO_CONVERT_INTO);

			const CQuotesProviderGoogle::CQuoteSection& rSection = get_quotes();
			size_t cQuotes = rSection.GetQuoteCount();
			for (size_t i = 0; i < cQuotes; ++i)
			{
				const CQuotesProviderGoogle::CQuote& rQuote = rSection.GetQuote(i);
				tstring sName = make_quote_name(rQuote);
				LPCTSTR pszName = sName.c_str();
				::SendMessage(hcbxFrom, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(pszName));
				::SendMessage(hcbxTo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(pszName));
			}

			CQuotesProviderGoogle* pProvider = get_google_provider();
			if (pProvider)
			{
				size_t cWatchedRates = pProvider->GetWatchedRateCount();
				for (size_t i = 0; i < cWatchedRates; ++i)
				{
					CQuotesProviderGoogle::CRateInfo ri;
					if (true == pProvider->GetWatchedRateInfo(i, ri))
					{
						g_aWatchedRates.push_back(ri);
						tstring sRate = make_rate_name(ri.m_from, ri.m_to);
						LPCTSTR pszRateName = sRate.c_str();
						::SendDlgItemMessage(hdlg, IDC_LIST_RATES, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(pszRateName));
					}
				}
			}

			::EnableWindow(::GetDlgItem(hdlg, IDC_BUTTON_ADD), FALSE);
			::EnableWindow(::GetDlgItem(hdlg, IDC_BUTTON_REMOVE), FALSE);
		}
		return TRUE;
		case WM_COMMAND:
			switch (HIWORD(wParam))
			{
			case CBN_SELCHANGE:
				switch (LOWORD(wParam))
				{
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
				switch (LOWORD(wParam))
				{
				case IDC_BUTTON_ADD:
				{
					size_t nFrom = static_cast<size_t>(::SendDlgItemMessage(hdlg, IDC_COMBO_CONVERT_FROM, CB_GETCURSEL, 0, 0));
					size_t nTo = static_cast<size_t>(::SendDlgItemMessage(hdlg, IDC_COMBO_CONVERT_INTO, CB_GETCURSEL, 0, 0));
					if ((CB_ERR != nFrom) && (CB_ERR != nTo) && (nFrom != nTo))
					{
						const CQuotesProviderGoogle::CQuoteSection& rSection = get_quotes();
						size_t cQuotes = rSection.GetQuoteCount();
						if ((nFrom < cQuotes) && (nTo < cQuotes))
						{
							CQuotesProviderGoogle::CRateInfo ri;
							ri.m_from = rSection.GetQuote(nFrom);
							ri.m_to = rSection.GetQuote(nTo);

							g_aWatchedRates.push_back(ri);

							tstring sRate = make_rate_name(ri.m_from, ri.m_to);
							LPCTSTR pszRateName = sRate.c_str();
							::SendDlgItemMessage(hdlg, IDC_LIST_RATES, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(pszRateName));
							PropSheet_Changed(::GetParent(hdlg), hdlg);
						}
					}
				}
				break;
				case IDC_BUTTON_REMOVE:
				{
					HWND hWnd = ::GetDlgItem(hdlg, IDC_LIST_RATES);
					int nSel = ::SendMessage(hWnd, LB_GETCURSEL, 0, 0);
					if (LB_ERR != nSel)
					{
						if ((LB_ERR != ::SendMessage(hWnd, LB_DELETESTRING, nSel, 0))
							&& (nSel < static_cast<int>(g_aWatchedRates.size())))
						{

							TWatchedRates::iterator i = g_aWatchedRates.begin();
							std::advance(i, nSel);
							g_aWatchedRates.erase(i);
							PropSheet_Changed(::GetParent(hdlg), hdlg);
						}
					}

					nSel = ::SendMessage(hWnd, LB_GETCURSEL, 0, 0);
					::EnableWindow(::GetDlgItem(hdlg, IDC_BUTTON_REMOVE), (LB_ERR != nSel));
				}
				break;
				}
				break;
				// 			case LBN_SELCHANGE:
				// 				switch(LOWORD(lParam))
				// 				{
				// 				case IDC_LIST_RATES:
				// 					{
				// 						int nSel = ::SendDlgItemMessage(hdlg, IDC_LIST_RATES, LB_GETCURSEL, 0, 0);
				// 						::EnableWindow(::GetDlgItem(hdlg,IDC_BUTTON_REMOVE),(-1 != nSel));
				// 					}
				// 				}
				// 				break;
			}
			break;

		}

		return FALSE;
	}
}

void CQuotesProviderGoogle::ShowPropertyPage(WPARAM wp, OPTIONSDIALOGPAGE &odp)
{
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_DIALOG_OPT_GOOGLE);
	odp.pfnDlgProc = GoogleOptDlgProc;
	odp.ptszTab = const_cast<LPTSTR>(GetInfo().m_sName.c_str());
	Options_AddPage(wp, &odp);
}

void CQuotesProviderGoogle::Accept(CQuotesProviderVisitor& visitor)const
{
	CQuotesProviderBase::Accept(visitor);
	visitor.Visit(*this);
}

double CQuotesProviderGoogle::Convert(double dAmount, const CQuote& from, const CQuote& to)const
{
	tstring sFullURL = build_url(GetURL(), from.GetID(), to.GetID(), dAmount);
	// 	LogIt(Info,sFullURL);

	CHTTPSession http;
	if ((true == http.OpenURL(sFullURL)))
	{
		tstring sHTML;
		if ((true == http.ReadResponce(sHTML)))
		{
			// 			LogIt(Info,sHTML);

			double dResult = 0.0;
			if ((true == parse_responce(sHTML, dResult)))
			{
				return dResult;
			}
			else
			{
				throw std::runtime_error(Translate("Error occurred during html parsing."));
			}
		}
		else
		{
			throw std::runtime_error(Translate("Error occurred during site access."));
		}
	}
	else
	{
		throw std::runtime_error(Translate("Error occurred during site access."));
	}

	return 0.0;
}

namespace
{
	bool is_equal_ids(MCONTACT hContact, const tstring& rsFromID, const tstring& rsToID)
	{
		tstring sFrom = Quotes_DBGetStringT(hContact, QUOTES_PROTOCOL_NAME, DB_STR_FROM_ID);
		tstring sTo = Quotes_DBGetStringT(hContact, QUOTES_PROTOCOL_NAME, DB_STR_TO_ID);
		return ((0 == quotes_stricmp(rsFromID.c_str(), sFrom.c_str()))
			&& (0 == quotes_stricmp(rsToID.c_str(), sTo.c_str())));
	}
}

MCONTACT CQuotesProviderGoogle::GetContactByID(const tstring& rsFromID, const tstring& rsToID)const
{
	CGuard<CLightMutex> cs(m_cs);

	TContracts::const_iterator i = std::find_if(m_aContacts.begin(), m_aContacts.end(),
		boost::bind(is_equal_ids, _1, boost::cref(rsFromID), boost::cref(rsToID)));
	if (i != m_aContacts.end())
	{
		return *i;
	}
	else
	{
		return NULL;
	}
}
