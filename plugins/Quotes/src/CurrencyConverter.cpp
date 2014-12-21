#include "StdAfx.h"

#define WINDOW_PREFIX "CurrenyConverter_"

#define DB_STR_CC_QUOTE_FROM_ID "CurrencyConverter_FromID"
#define DB_STR_CC_QUOTE_TO_ID "CurrencyConverter_ToID"
#define DB_STR_CC_AMOUNT "CurrencyConverter_Amount"

namespace
{
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


	CQuotesProviderGoogle::CQuoteSection get_quotes(const CQuotesProviderGoogle* pProvider = NULL)
	{
		if (NULL == pProvider)
		{
			pProvider = get_google_provider();
		}

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

	inline tstring make_quote_name(const CQuotesProviderGoogle::CQuote& rQuote)
	{
		const tstring& rsDesc = rQuote.GetName();
		return((false == rsDesc.empty()) ? rsDesc : rQuote.GetSymbol());
	}

	inline void update_convert_button(HWND hDlg)
	{
		int nFrom = static_cast<int>(::SendDlgItemMessage(hDlg, IDC_COMBO_CONVERT_FROM, CB_GETCURSEL, 0, 0));
		int nTo = static_cast<int>(::SendDlgItemMessage(hDlg, IDC_COMBO_CONVERT_INTO, CB_GETCURSEL, 0, 0));
		bool bEnableButton = ((CB_ERR != nFrom)
			&& (CB_ERR != nTo)
			&& (nFrom != nTo)
			&& (GetWindowTextLength(GetDlgItem(hDlg, IDC_EDIT_VALUE)) > 0));
		EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_CONVERT), bEnableButton);
	}

	inline void update_swap_button(HWND hDlg)
	{
		int nFrom = static_cast<int>(::SendDlgItemMessage(hDlg, IDC_COMBO_CONVERT_FROM, CB_GETCURSEL, 0, 0));
		int nTo = static_cast<int>(::SendDlgItemMessage(hDlg, IDC_COMBO_CONVERT_INTO, CB_GETCURSEL, 0, 0));
		bool bEnableButton = ((CB_ERR != nFrom)
			&& (CB_ERR != nTo)
			&& (nFrom != nTo));
		EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_SWAP), bEnableButton);
	}

	inline tstring double2str(double dValue)
	{
		tostringstream output;
		output.imbue(GetSystemLocale());
		output << std::fixed << std::setprecision(2) << dValue;
		return output.str();
	}

	inline bool str2double(const tstring& s, double& d)
	{
		tistringstream input(s);
		input.imbue(GetSystemLocale());
		input >> d;
		return ((false == input.bad()) && (false == input.fail()));
	}


	INT_PTR CALLBACK CurrencyConverterDlgProc(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp)
	{
		switch (msg)
		{
		case WM_INITDIALOG:
		{
			HANDLE hWL = CModuleInfo::GetInstance().GetWindowList(WINDOW_PREFIX, false);
			assert(hWL);
			WindowList_Add(hWL, hDlg, NULL);

			TranslateDialogDefault(hDlg);

			::SendMessage(hDlg, WM_SETICON, FALSE, reinterpret_cast<LPARAM>(Quotes_LoadIconEx(ICON_STR_CURRENCY_CONVERTER)));
			::SendMessage(hDlg, WM_SETICON, TRUE, reinterpret_cast<LPARAM>(Quotes_LoadIconEx(ICON_STR_CURRENCY_CONVERTER, true)));

			HWND hcbxFrom = ::GetDlgItem(hDlg, IDC_COMBO_CONVERT_FROM);
			HWND hcbxTo = ::GetDlgItem(hDlg, IDC_COMBO_CONVERT_INTO);

			tstring sFromQuoteID = Quotes_DBGetStringT(NULL, QUOTES_MODULE_NAME, DB_STR_CC_QUOTE_FROM_ID);
			tstring sToQuoteID = Quotes_DBGetStringT(NULL, QUOTES_MODULE_NAME, DB_STR_CC_QUOTE_TO_ID);

			const CQuotesProviderGoogle* pProvider = get_google_provider();
			const CQuotesProviderGoogle::CQuoteSection& rSection = get_quotes(pProvider);
			size_t cQuotes = rSection.GetQuoteCount();
			for (size_t i = 0; i < cQuotes; ++i)
			{
				const CQuotesProviderGoogle::CQuote& rQuote = rSection.GetQuote(i);
				tstring sName = make_quote_name(rQuote);
				LPCTSTR pszName = sName.c_str();
				LRESULT nFrom = ::SendMessage(hcbxFrom, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(pszName));
				LRESULT nTo = ::SendMessage(hcbxTo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(pszName));

				if (0 == quotes_stricmp(rQuote.GetID().c_str(), sFromQuoteID.c_str()))
				{
					::SendMessage(hcbxFrom, CB_SETCURSEL, nFrom, 0);
				}

				if (0 == quotes_stricmp(rQuote.GetID().c_str(), sToQuoteID.c_str()))
				{
					::SendMessage(hcbxTo, CB_SETCURSEL, nTo, 0);
				}
			}

			double dAmount = 1.0;
			Quotes_DBReadDouble(NULL, QUOTES_MODULE_NAME, DB_STR_CC_AMOUNT, dAmount);
			::SetDlgItemText(hDlg, IDC_EDIT_VALUE, double2str(dAmount).c_str());

			const IQuotesProvider::CProviderInfo& pi = pProvider->GetInfo();
			tostringstream o;
			o << TranslateT("Info provided by") << _T(" <a href=\"") << pi.m_sURL << _T("\">") << pi.m_sName << _T("</a>");

			::SetDlgItemText(hDlg, IDC_SYSLINK_PROVIDER, o.str().c_str());

			::SendDlgItemMessage(hDlg, IDC_BUTTON_SWAP, BM_SETIMAGE, IMAGE_ICON,
				reinterpret_cast<LPARAM>(Quotes_LoadIconEx(ICON_STR_SWAP)));

			update_convert_button(hDlg);
			update_swap_button(hDlg);

			Utils_RestoreWindowPositionNoSize(hDlg, NULL, QUOTES_PROTOCOL_NAME, WINDOW_PREFIX);
			::ShowWindow(hDlg, SW_SHOW);
		}
		return (TRUE);
		case WM_CLOSE:
		{
			HANDLE hWL = CModuleInfo::GetInstance().GetWindowList(WINDOW_PREFIX, false);
			assert(hWL);
			WindowList_Remove(hWL, hDlg);
			Utils_SaveWindowPosition(hDlg, NULL, QUOTES_PROTOCOL_NAME, WINDOW_PREFIX);
			EndDialog(hDlg, 0);
		}
		return (TRUE);
		case WM_COMMAND:
			switch (LOWORD(wp))
			{
			case IDC_COMBO_CONVERT_FROM:
			case IDC_COMBO_CONVERT_INTO:
				if (CBN_SELCHANGE == HIWORD(wp))
				{
					update_convert_button(hDlg);
					update_swap_button(hDlg);
				}
				return TRUE;
			case IDC_EDIT_VALUE:
				if (EN_CHANGE == HIWORD(wp))
				{
					update_convert_button(hDlg);
				}
				return TRUE;
			case IDCANCEL:
			{
				SendMessage(hDlg, WM_CLOSE, 0, 0);
			}
			return (TRUE);
			case IDC_BUTTON_SWAP:
			{
				HWND wndFrom = ::GetDlgItem(hDlg, IDC_COMBO_CONVERT_FROM);
				HWND wndTo = ::GetDlgItem(hDlg, IDC_COMBO_CONVERT_INTO);
				WPARAM nFrom = ::SendMessage(wndFrom, CB_GETCURSEL, 0, 0);
				WPARAM nTo = ::SendMessage(wndTo, CB_GETCURSEL, 0, 0);

				::SendMessage(wndFrom, CB_SETCURSEL, nTo, 0);
				::SendMessage(wndTo, CB_SETCURSEL, nFrom, 0);
			}
			return (TRUE);
			case IDC_BUTTON_CONVERT:
			{
				HWND hwndAmount = GetDlgItem(hDlg, IDC_EDIT_VALUE);
				tstring sText = get_window_text(hwndAmount);

				double dAmount = 1.0;
				if ((true == str2double(sText, dAmount)) && (dAmount > 0.0))
				{
					Quotes_DBWriteDouble(NULL, QUOTES_MODULE_NAME, DB_STR_CC_AMOUNT, dAmount);

					size_t nFrom = static_cast<size_t>(::SendDlgItemMessage(hDlg, IDC_COMBO_CONVERT_FROM, CB_GETCURSEL, 0, 0));
					size_t nTo = static_cast<size_t>(::SendDlgItemMessage(hDlg, IDC_COMBO_CONVERT_INTO, CB_GETCURSEL, 0, 0));
					if ((CB_ERR != nFrom) && (CB_ERR != nTo) && (nFrom != nTo))
					{
						const CQuotesProviderGoogle::CQuoteSection& rSection = get_quotes();
						size_t cQuotes = rSection.GetQuoteCount();
						if ((nFrom < cQuotes) && (nTo < cQuotes))
						{
							CQuotesProviderGoogle::CRateInfo ri;
							CQuotesProviderGoogle::CQuote from = rSection.GetQuote(nFrom);
							CQuotesProviderGoogle::CQuote to = rSection.GetQuote(nTo);

							db_set_ts(NULL, QUOTES_MODULE_NAME, DB_STR_CC_QUOTE_FROM_ID, from.GetID().c_str());
							db_set_ts(NULL, QUOTES_MODULE_NAME, DB_STR_CC_QUOTE_TO_ID, to.GetID().c_str());

							const CQuotesProviderGoogle* pProvider = get_google_provider();
							assert(pProvider);
							if (pProvider)
							{
								tstring sResult;
								std::string sError;
								try
								{
									double dResult = pProvider->Convert(dAmount, from, to);
									tostringstream ss;
									ss.imbue(GetSystemLocale());
									ss << std::fixed << std::setprecision(2) << dAmount << " " << from.GetName() << " = " << dResult << " " << to.GetName();
									sResult = ss.str();
								}
								catch (std::exception& e)
								{
									sError = e.what();
									//Quotes_MessageBox(hDlg,sResult.c_str());
								}

								if (false == sError.empty())
								{
									//USES_CONVERSION;
									sResult = quotes_a2t(sError.c_str());//A2T(sError.c_str());
								}

								SetDlgItemText(hDlg, IDC_EDIT_RESULT, sResult.c_str());
							}
						}
					}
				}
				else
				{
					Quotes_MessageBox(hDlg, TranslateT("Enter positive number."), MB_OK | MB_ICONERROR);
					prepare_edit_ctrl_for_error(GetDlgItem(hDlg, IDC_EDIT_VALUE));
				}
			}
			return (TRUE);
			}
			return (FALSE);
		case WM_NOTIFY:
		{
			LPNMHDR pNMHDR = reinterpret_cast<LPNMHDR>(lp);
			switch (pNMHDR->code)
			{
			case NM_CLICK:
				if (IDC_SYSLINK_PROVIDER == wp)
				{
					PNMLINK pNMLink = reinterpret_cast<PNMLINK>(pNMHDR);
					::ShellExecute(hDlg, _T("open"), pNMLink->item.szUrl, NULL, NULL, SW_SHOWNORMAL);
				}
				break;
			}
		}
		break;
		}
		return (FALSE);
	}
}

INT_PTR QuotesMenu_CurrencyConverter(WPARAM, LPARAM)
{
	HANDLE hWL = CModuleInfo::GetInstance().GetWindowList(WINDOW_PREFIX, true);
	HWND hWnd = WindowList_Find(hWL, NULL);
	if (NULL != hWnd)
	{
		SetForegroundWindow(hWnd);
		SetFocus(hWnd);
	}
	else
	{
		CreateDialogParam(g_hInstance, MAKEINTRESOURCE(IDD_CURRENCY_CONVERTER), NULL, CurrencyConverterDlgProc, 0);
	}

	return 0;
}
