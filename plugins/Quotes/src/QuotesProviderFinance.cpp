#include "stdafx.h"

void CQuotesProviderFinance::GetWatchedQuotes(TQuotes& raQuotes)const
{
	raQuotes.clear();
	BOOST_FOREACH(MCONTACT hContact,m_aContacts)
	{
		tstring sID = Quotes_DBGetStringT(hContact,QUOTES_MODULE_NAME,DB_STR_QUOTE_ID);
		tstring sSymbol = Quotes_DBGetStringT(hContact,QUOTES_MODULE_NAME,DB_STR_QUOTE_SYMBOL,sID.c_str());
		tstring sDescr = Quotes_DBGetStringT(hContact,QUOTES_MODULE_NAME,DB_STR_QUOTE_DESCRIPTION);
		CQuotesProviderBase::CQuote quote(sID,sSymbol,sDescr);

		raQuotes.push_back(quote);
	}
}

namespace
{
	inline tstring get_quote_id(MCONTACT hContact)
	{
		return Quotes_DBGetStringT(hContact,QUOTES_MODULE_NAME,DB_STR_QUOTE_ID);
	}

	inline bool is_quote_id_equal(MCONTACT hContact,const tstring& sID)
	{
		return sID == get_quote_id(hContact);
	}
}

bool CQuotesProviderFinance::WatchForQuote(const CQuote& rQuote,bool bWatch)
{
	const tstring& sQuoteID = rQuote.GetID();
	TContracts::iterator i = std::find_if(m_aContacts.begin(),m_aContacts.end(),
		boost::bind(is_quote_id_equal,_1,sQuoteID));

	if ((false == bWatch) && (i != m_aContacts.end()))
	{
		MCONTACT hContact = *i;
		{// for CCritSection
			CGuard<CLightMutex> cs(m_cs);
			m_aContacts.erase(i);
		}

		CallService(MS_DB_CONTACT_DELETE, WPARAM(hContact), 0);
		return true;
	}
	else if ((true == bWatch) && (i == m_aContacts.end()))
	{
		MCONTACT hContact = CreateNewContact(rQuote.GetSymbol());
		if(hContact)
		{
			db_set_ts(hContact,QUOTES_PROTOCOL_NAME,DB_STR_QUOTE_ID,sQuoteID.c_str());
			if(false == rQuote.GetName().empty())
			{
				db_set_ts(hContact,QUOTES_PROTOCOL_NAME,DB_STR_QUOTE_DESCRIPTION,rQuote.GetName().c_str());
			}

			return true;
		}
	}

	return false;
}

MCONTACT CQuotesProviderFinance::GetContactByQuoteID(const tstring& rsQuoteID)const
{
	CGuard<CLightMutex> cs(m_cs);

	TContracts::const_iterator i = std::find_if(m_aContacts.begin(),m_aContacts.end(),
		boost::bind(std::equal_to<tstring>(),rsQuoteID,boost::bind(get_quote_id,_1)));
	if(i != m_aContacts.end())
	{
		return *i;
	}
	else
	{
		return NULL;
	}
}

void CQuotesProviderFinance::Accept(CQuotesProviderVisitor& visitor)const
{
	CQuotesProviderBase::Accept(visitor);
	visitor.Visit(*this);
}

namespace
{
	inline tstring make_quote_name(const CQuotesProviderBase::CQuote& rQuote)
	{
		const tstring& rsDesc = rQuote.GetName();
		return((false == rsDesc.empty()) ? rsDesc : rQuote.GetSymbol());
	}
	
	int add_quote_to_wnd(const CQuotesProviderBase::CQuote& rQuote,HWND hwnd)
	{
		tstring sName = make_quote_name(rQuote);
		int nIndex = ::SendMessage(hwnd,LB_ADDSTRING,0,reinterpret_cast<LPARAM>(sName.c_str()));
		if(nIndex >= 0)
		{
			CQuotesProviderBase::CQuote* pQuote = new CQuotesProviderBase::CQuote(rQuote);
			if(LB_ERR == ::SendMessage(hwnd,LB_SETITEMDATA,nIndex,reinterpret_cast<LPARAM>(pQuote)))
			{
				delete pQuote;
			}
		}
		return nIndex;
	}

// 	typedef CQuotesProviderFinance::TQuotes TQuotes;
// 	TQuotes g_aWatchedQuotes;

// 	inline bool cmp_quotes(const tstring& rsQuoteId,const CQuotesProviderBase::CQuote& quote)
// 	{
// 		return (0 == quotes_stricmp(rsQuoteId.c_str(),quote.GetID().c_str()));
// 	}

	CQuotesProviderBase::CQuote* get_quote_ptr_from_lb_index(HWND hwndListBox,int nIndex)
	{
		LRESULT lResult = ::SendMessage(hwndListBox,LB_GETITEMDATA,nIndex,0);
		return (((LB_ERR != lResult) && (0 != lResult)) ? (reinterpret_cast<CQuotesProviderBase::CQuote*>(lResult)) : nullptr);
	}

	int is_quote_added(HWND hwndList,const tstring& rsQuoteID)
	{
		int cItems = ::SendMessage(hwndList,LB_GETCOUNT,0,0);
		for(int i = 0;i < cItems;++i)
		{
			const CQuotesProviderBase::CQuote* pQuote = get_quote_ptr_from_lb_index(hwndList,i);
			if ((nullptr != pQuote) 
				&& ((0 == quotes_stricmp(rsQuoteID.c_str(),pQuote->GetID().c_str()))
				|| (0 == quotes_stricmp(rsQuoteID.c_str(),pQuote->GetName().c_str()))
				|| (0 == quotes_stricmp(rsQuoteID.c_str(),pQuote->GetSymbol().c_str()))))
			{
				return i;
			}
		}
		return LB_ERR;
	}

	INT_PTR CALLBACK  GoogleFinanceOptDlgProc(HWND hDlg,UINT message,WPARAM wParam,LPARAM lParam)
	{
		CQuotesProviderFinance* pProvider = nullptr;
		if(WM_INITDIALOG == message)
		{
			pProvider = reinterpret_cast<CQuotesProviderFinance*>(lParam);
			SetWindowLongPtr(hDlg,GWLP_USERDATA,lParam);
		}
		else
		{
			pProvider = reinterpret_cast<CQuotesProviderFinance*>(GetWindowLongPtr(hDlg,GWLP_USERDATA));
		}

		CCommonDlgProcData d(pProvider);
		CommonOptionDlgProc(hDlg,message,wParam,lParam,d);

		switch(message)
		{
		case WM_INITDIALOG:
			{
				TranslateDialogDefault(hDlg);

				CQuotesProviderFinance::TQuotes aQuotes;
				pProvider->GetWatchedQuotes(aQuotes);

				HWND hwndList = GetDlgItem(hDlg,IDC_LIST_RATES);
				std::for_each(aQuotes.begin(),aQuotes.end(),
					boost::bind(add_quote_to_wnd,_1,hwndList));

				::EnableWindow(::GetDlgItem(hDlg,IDC_BUTTON_ADD),FALSE);
				::EnableWindow(::GetDlgItem(hDlg,IDC_BUTTON_REMOVE),FALSE);
			}
			return (TRUE);
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
			case IDC_EDIT_QUOTE:
				if(EN_CHANGE == HIWORD(wParam))
				{
					::EnableWindow(::GetDlgItem(hDlg,IDC_BUTTON_ADD),GetWindowTextLength(GetDlgItem(hDlg,IDC_EDIT_QUOTE)) > 0);
				}
				return (TRUE);
			case IDC_BUTTON_ADD:
				if(BN_CLICKED == HIWORD(wParam))
				{
					HWND hEdit = GetDlgItem(hDlg,IDC_EDIT_QUOTE);
					tstring sQuoteSymbol = get_window_text(hEdit);
					assert(false == sQuoteSymbol.empty());
					HWND hwndList = GetDlgItem(hDlg,IDC_LIST_RATES);
					if(LB_ERR == is_quote_added(hwndList,sQuoteSymbol))
					{
						CQuotesProviderBase::CQuote quote(sQuoteSymbol,sQuoteSymbol);
						if(add_quote_to_wnd(quote,hwndList) >= 0)
						{
							SetDlgItemText(hDlg,IDC_EDIT_QUOTE,_T(""));
							SetFocus(hEdit);
							PropSheet_Changed(::GetParent(hDlg),hDlg);
						}
						else
						{
							::MessageBeep(MB_ICONERROR);
						}
					}
				}
				return (TRUE);
			case IDC_BUTTON_REMOVE:
				if(BN_CLICKED == HIWORD(wParam))
				{
					HWND hWnd = ::GetDlgItem(hDlg,IDC_LIST_RATES);
					int nSel = ::SendMessage(hWnd,LB_GETCURSEL,0,0);
					if(LB_ERR != nSel)
					{
						CQuotesProviderBase::CQuote* pQuote = get_quote_ptr_from_lb_index(hWnd,nSel);
						delete pQuote;
						if(LB_ERR != ::SendMessage(hWnd,LB_DELETESTRING,nSel,0))
						{
							PropSheet_Changed(::GetParent(hDlg),hDlg);
						}
					}

					nSel = ::SendMessage(hWnd,LB_GETCURSEL,0,0);
					::EnableWindow(::GetDlgItem(hDlg,IDC_BUTTON_REMOVE),(LB_ERR != nSel));
				}
				return (TRUE);
			case IDC_LIST_RATES:
				if(CBN_SELCHANGE == HIWORD(wParam))
				{
					int nSel = ::SendMessage(::GetDlgItem(hDlg,IDC_LIST_RATES),LB_GETCURSEL,0,0);
					::EnableWindow(::GetDlgItem(hDlg,IDC_BUTTON_REMOVE),(LB_ERR != nSel));
				}
				return (TRUE);
			}
			return (FALSE);

		case WM_NOTIFY:
			{
				LPNMHDR pNMHDR = reinterpret_cast<LPNMHDR>(lParam);
				switch(pNMHDR->code)
				{
				case PSN_APPLY:
					if(pProvider)
					{
						CQuotesProviderFinance::TQuotes aTemp;
						pProvider->GetWatchedQuotes(aTemp);

						typedef std::vector<const CQuotesProviderBase::CQuote*> TQuotesPtr;
						TQuotesPtr apCurrent;
						HWND hwndListBox = GetDlgItem(hDlg,IDC_LIST_RATES);
						int cItems = ::SendMessage(hwndListBox,LB_GETCOUNT,0,0);
						for(int i = 0;i < cItems;++i)
						{
							const CQuotesProviderBase::CQuote* pQuote = get_quote_ptr_from_lb_index(hwndListBox,i);
							if(pQuote)
							{
								apCurrent.push_back(pQuote);
							}
						}
						
						std::for_each(aTemp.begin(),aTemp.end(),
							[&apCurrent,pProvider](const CQuotesProviderBase::CQuote& quote)
							{
								if(apCurrent.end() == std::find_if(apCurrent.begin(),apCurrent.end(),
									[&quote](const CQuotesProviderBase::CQuote* pQuote){return 0 == quotes_stricmp(pQuote->GetID().c_str(),quote.GetID().c_str());}))
								{
									pProvider->WatchForQuote(quote,false);
								}
							});

						std::for_each(apCurrent.begin(),apCurrent.end(),
							[&aTemp,pProvider](const CQuotesProviderBase::CQuote* pQuote)
							{
								if(aTemp.end() == 
									std::find_if(aTemp.begin(),aTemp.end(),
									[pQuote](const CQuotesProviderBase::CQuote& quote){return 0 == quotes_stricmp(pQuote->GetID().c_str(),quote.GetID().c_str());}))
								{
									pProvider->WatchForQuote(*pQuote,true);
								}

							});

						pProvider->RefreshSettings();
					}

					return (TRUE);
				}
			}
			return (FALSE);
		case WM_DESTROY:
			HWND hwndListBox = GetDlgItem(hDlg,IDC_LIST_RATES);
			int cItems = ::SendMessage(hwndListBox,LB_GETCOUNT,0,0);
			for(int i = 0;i < cItems;++i)
			{
				const CQuotesProviderBase::CQuote* pQuote = get_quote_ptr_from_lb_index(hwndListBox,i);
				delete pQuote;
			}
			return (FALSE);
		}
		return (FALSE);
	}
}

void CQuotesProviderFinance::ShowPropertyPage(WPARAM wp, OPTIONSDIALOGPAGE &odp)
{
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_DIALOG_OPT_FINANCE);
	odp.pfnDlgProc = GoogleFinanceOptDlgProc;
	odp.dwInitParam = reinterpret_cast<DWORD>(static_cast<CQuotesProviderFinance*>(this));
	odp.ptszTab = const_cast<LPTSTR>(GetInfo().m_sName.c_str());
	Options_AddPage(wp, &odp);	
}
