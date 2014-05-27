#include "StdAfx.h"

namespace
{
	enum ETreeCheckBoxState
	{
		// tree check box state
		TCBS_NOSTATEBOX = 0,
		TCBS_UNCHECKED = 1,
		TCBS_CHECKED = 2,
	};

	enum
	{
		TREE_VIEW_CHECK_STATE_CHANGE = WM_USER + 100,
		IMAGE_INDEX_SECTION = 0,
		IMAGE_INDEX_QUOTE = 1
	};

	// 	typedef CQuotesProviders::TQuotesProviders TQuotesProviders;
	// 	typedef CQuotesProviders::TQuotesProviderPtr TQuotesProviderPtr;

	HTREEITEM tree_insert_item(HWND hwndTree,
		const tstring& rsName,
		HTREEITEM htiParent,
		int nImage,
		LPARAM lp = 0)
	{
// 		USES_CONVERSION;
		TVINSERTSTRUCT tvi;
		ZeroMemory(&tvi,sizeof(tvi));

		tvi.hParent = htiParent;
		tvi.hInsertAfter = TVI_LAST;
		tvi.item.mask = TVIF_TEXT|TVIF_PARAM|TVIF_IMAGE|TVIF_SELECTEDIMAGE;

// 		CA2T name(rsName.c_str());
		
		tvi.item.pszText = const_cast<LPTSTR>(rsName.c_str());//name;
		tvi.item.lParam = lp;
		tvi.item.iImage = nImage;
		tvi.item.iSelectedImage = nImage;
		return TreeView_InsertItem(hwndTree,&tvi);
	}

	bool add_quote_to_tree(const CQuotesProviderDukasCopy::CQuote& q,HWND hwndTree,HTREEITEM htiParent,const CQuotesProviderDukasCopy* pQuotesProvier)
	{
		bool bChecked = pQuotesProvier->IsQuoteWatched(q);
		HTREEITEM hti = tree_insert_item(hwndTree,((false == q.GetName().empty()) ? q.GetName() : q.GetSymbol()),htiParent,IMAGE_INDEX_QUOTE);
		if(hti && bChecked)
		{
			HWND hDlg = ::GetParent(hwndTree);
			assert(::IsWindow(hDlg));
			::PostMessage(hDlg,TREE_VIEW_CHECK_STATE_CHANGE,MAKEWPARAM(0,TCBS_CHECKED),reinterpret_cast<LPARAM>(hti));
		}

		return (NULL != hti && bChecked);
	}

	void add_section_to_tree(const CQuotesProviderDukasCopy::CQuoteSection& qs,
		HWND hwndTree,
		HTREEITEM htiParent,
		const CQuotesProviderDukasCopy* pQuotesProvier,
		bool& rbIsChecked,
		bool& rbIsExpended,
		bool bExpand = false)
	{
		rbIsChecked = false;
		rbIsExpended = false;
		HTREEITEM hti = tree_insert_item(hwndTree,qs.GetName(),htiParent,IMAGE_INDEX_SECTION);

		size_t cCheckedItems = 0;
		size_t cSection = qs.GetSectionCount();
		for(size_t i = 0;i < cSection;++i)
		{
			bool bIsChecked = false;
			bool bIsExpanded = false;
			CQuotesProviderDukasCopy::CQuoteSection other = qs.GetSection(i);
			add_section_to_tree(other,hwndTree,hti,pQuotesProvier,bIsChecked,bIsExpanded);

			if(bIsChecked)
			{
				++cCheckedItems;
			}

			if(bIsExpanded)
			{
				bExpand = true;
			}
		}

		size_t cQuotes = qs.GetQuoteCount();
		for(size_t i = 0;i < cQuotes;++i)
		{
			CQuotesProviderDukasCopy::CQuote q = qs.GetQuote(i);
			if(true == add_quote_to_tree(q,hwndTree,hti,pQuotesProvier))
			{
				++ cCheckedItems;
			}
		}

		if(bExpand || cCheckedItems > 0)
		{
			rbIsExpended = true;
			TreeView_Expand(hwndTree,hti,TVE_EXPAND);
		}

		if(cCheckedItems == (cSection+cQuotes))
		{
			rbIsChecked = true;
			HWND hDlg = ::GetParent(hwndTree);
			assert(::IsWindow(hDlg));
			::PostMessage(hDlg,TREE_VIEW_CHECK_STATE_CHANGE,MAKEWPARAM(0,TCBS_CHECKED),reinterpret_cast<LPARAM>(hti));
		}
	}

	void add_provider_to_tree(const CQuotesProviderDukasCopy* pQuotesProvier,HWND hwndTree)
	{
		CQuotesProviderDukasCopy::CQuoteSection qs = pQuotesProvier->GetQuotes();
		bool bIsChecked = false;
		bool bIsExpanded = false;
		add_section_to_tree(qs,hwndTree,TVI_ROOT,pQuotesProvier,bIsChecked,bIsExpanded,true);
	}

	inline HTREEITEM tree_get_child_item(HWND hwndTree,HTREEITEM hti)
	{
		return reinterpret_cast<HTREEITEM>(::SendMessage(hwndTree, TVM_GETNEXTITEM, TVGN_CHILD,reinterpret_cast<LPARAM>(hti)));
	}

	inline HTREEITEM tree_get_next_sibling_item(HWND hwndTree,HTREEITEM hti)
	{
		return reinterpret_cast<HTREEITEM>(::SendMessage(hwndTree, TVM_GETNEXTITEM, TVGN_NEXT,reinterpret_cast<LPARAM>(hti)));
	}

	inline ETreeCheckBoxState tree_get_state_image(HWND hwndTree,HTREEITEM hti)
	{
		TVITEM   tvi;
		tvi.hItem = hti;
		tvi.mask = TVIF_STATE|TVIF_HANDLE;
		tvi.stateMask = TVIS_STATEIMAGEMASK;
		if(TRUE == ::SendMessage(hwndTree,TVM_GETITEM,0,reinterpret_cast<LPARAM>(&tvi)))
		{
			UINT nState = (tvi.state >> 12);
			return static_cast<ETreeCheckBoxState>(nState);
		}
		else
		{
			return TCBS_UNCHECKED;
		}
	}

	void tree_do_set_item_state(HWND hwndTree,HTREEITEM hti,ETreeCheckBoxState nState)
	{
		TVITEM tvi;
		ZeroMemory(&tvi,sizeof(tvi));

		tvi.mask = TVIF_STATE|TVIF_HANDLE;
		tvi.hItem = hti;

		tvi.stateMask = TVIS_STATEIMAGEMASK;
		tvi.state = INDEXTOSTATEIMAGEMASK(nState);

		::SendMessage(hwndTree,TVM_SETITEM,0,reinterpret_cast<LPARAM>(&tvi));
	}

	void tree_set_item_state(HWND hwndTree,HTREEITEM hti,ETreeCheckBoxState nState,bool bRecursively)
	{
		if(true == bRecursively)
		{
			for(hti = tree_get_child_item(hwndTree,hti);hti;hti = tree_get_next_sibling_item(hwndTree,hti))
			{
				tree_do_set_item_state(hwndTree,hti,nState);
				tree_set_item_state(hwndTree,hti,nState,bRecursively);
			}
		}
		else
		{
			tree_do_set_item_state(hwndTree,hti,nState);
		}
	}

	void save_quote_selection(HWND hwndTree,HTREEITEM h,const CQuotesProviderDukasCopy::CQuote& q,CQuotesProviderDukasCopy* pQuotesProvier)
	{
		ETreeCheckBoxState nState = tree_get_state_image(hwndTree,h);
		pQuotesProvier->WatchForQuote(q,(TCBS_CHECKED == nState));
	}

	void recursive_save_quote_section_selection(HWND hwndTree,HTREEITEM h,const CQuotesProviderDukasCopy::CQuoteSection& qs,CQuotesProviderDukasCopy* pQuotesProvier)
	{
		size_t cSection = qs.GetSectionCount();
		h = tree_get_child_item(hwndTree,h);
		for(size_t i = 0;h && (i < cSection);++i,h = tree_get_next_sibling_item(hwndTree,h))
		{
			CQuotesProviderDukasCopy::CQuoteSection other = qs.GetSection(i);
			recursive_save_quote_section_selection(hwndTree,h,other,pQuotesProvier);
		}

		size_t cQuotes = qs.GetQuoteCount();
		for(size_t i = 0;h && (i < cQuotes);++i,h = tree_get_next_sibling_item(hwndTree,h))
		{
			CQuotesProviderDukasCopy::CQuote q = qs.GetQuote(i);
			save_quote_selection(hwndTree,h,q,pQuotesProvier);
		}
	}

	void recursive_save_selection(HWND hwndTree,CQuotesProviderDukasCopy* pQuotesProvider)
	{
		// 		CModuleInfo::TQuotesProvidersPtr& pProviders = CModuleInfo::GetQuoteProvidersPtr();
		// 		const TQuotesProviders& rapQuotesProviders = pProviders->GetProviders();
		// 
		// 		TQuotesProviders::const_iterator i = rapQuotesProviders.begin();
		// 		TQuotesProviders::const_iterator iE = rapQuotesProviders.end();
		// 		for(HTREEITEM h = tree_get_child_item(hwndTree,TVI_ROOT);h && (i!=iE);++i,h = tree_get_next_sibling_item(hwndTree,h))
		// 		{
		// 			const TQuotesProviderPtr& pQuotesProvier = *i;
		CQuotesProviderDukasCopy::CQuoteSection qs = pQuotesProvider->GetQuotes();
		recursive_save_quote_section_selection(hwndTree,tree_get_child_item(hwndTree,TVI_ROOT),qs,pQuotesProvider);
		// 		}
	}

	class CImageListWrapper
	{
	public:
		CImageListWrapper()
			: m_hImageList(ImageList_Create(::GetSystemMetrics(SM_CXSMICON),
			::GetSystemMetrics(SM_CYSMICON),
			ILC_COLOR24|ILC_MASK,2,0))
		{
			if(m_hImageList)
			{
				ImageList_AddIcon(m_hImageList,Quotes_LoadIconEx(ICON_STR_SECTION));
				ImageList_AddIcon(m_hImageList,Quotes_LoadIconEx(ICON_STR_QUOTE));
			}
		}

		~CImageListWrapper()
		{
			if(m_hImageList)
			{
				ImageList_Destroy(m_hImageList);
			}
		}

		operator HIMAGELIST()const
		{
			return m_hImageList;
		}

	private:
		HIMAGELIST m_hImageList;
	};

	HIMAGELIST get_image_list()
	{
		static CImageListWrapper wrapper;
		return wrapper;
	}

	CQuotesProviderDukasCopy* get_dukas_copy_provider()
	{
		CModuleInfo::TQuotesProvidersPtr& pProviders = CModuleInfo::GetQuoteProvidersPtr();
		const CQuotesProviders::TQuotesProviders& rapQuotesProviders = pProviders->GetProviders();
		for(CQuotesProviders::TQuotesProviders::const_iterator i = rapQuotesProviders.begin();i != rapQuotesProviders.end();++i)
		{
			const CQuotesProviders::TQuotesProviderPtr& pProvider = *i;
			CQuotesProviderDukasCopy* pDukas = dynamic_cast<CQuotesProviderDukasCopy*>(pProvider.get());
			if(pDukas)
			{
				return pDukas;
			}
		}

		assert(!"We should never get here!");
		return NULL;
	}

	INT_PTR CALLBACK EconomicRatesDlgProc(HWND hdlg,UINT message,WPARAM wParam,LPARAM lParam)
	{
		CCommonDlgProcData d(get_dukas_copy_provider());
		CommonOptionDlgProc(hdlg,message,wParam,lParam,d);

		switch(message)
		{
		case WM_INITDIALOG:
			{
				TranslateDialogDefault(hdlg);

				HWND hwndTree = ::GetDlgItem(hdlg,IDC_TREE_ECONOMIC_RATES);
				HIMAGELIST hImage = get_image_list();
				if(hImage)
				{
					TreeView_SetImageList(hwndTree,hImage,TVSIL_NORMAL);
				}

				const CQuotesProviderDukasCopy* pDukasProvider = get_dukas_copy_provider();
				if(pDukasProvider)
				{
					add_provider_to_tree(pDukasProvider,hwndTree);
				}
				// 			Window_SetIcon_IcoLib(hdlg, SKINICON_OTHER_MIRANDA);
			}
			return TRUE;

		case WM_NOTIFY:
			{
				LPNMHDR pNMHDR = reinterpret_cast<LPNMHDR>(lParam);
				switch(pNMHDR->code)
				{
				case TVN_KEYDOWN:
					if(IDC_TREE_ECONOMIC_RATES == wParam)
					{
						LPNMTVKEYDOWN pKeyDown = reinterpret_cast<LPNMTVKEYDOWN>(lParam);
						if(VK_SPACE == pKeyDown->wVKey)
						{
							HTREEITEM hti = TreeView_GetSelection(::GetDlgItem(hdlg,IDC_TREE_ECONOMIC_RATES));
							::PostMessage(hdlg,TREE_VIEW_CHECK_STATE_CHANGE,MAKEWPARAM(1,0),reinterpret_cast<LPARAM>(hti));
							PropSheet_Changed(::GetParent(hdlg),hdlg);
						}
					}
					break;
				case NM_CLICK:
					if(IDC_TREE_ECONOMIC_RATES == wParam)
					{
						DWORD pos = ::GetMessagePos();

						HWND hwndTree = ::GetDlgItem(hdlg,IDC_TREE_ECONOMIC_RATES);

						TVHITTESTINFO tvhti;
						tvhti.pt.x = LOWORD(pos);
						tvhti.pt.y = HIWORD(pos);
						::ScreenToClient(hwndTree,&(tvhti.pt));

						HTREEITEM hti = reinterpret_cast<HTREEITEM>(::SendMessage(hwndTree,TVM_HITTEST,0,reinterpret_cast<LPARAM>(&tvhti)));
						if(hti && (tvhti.flags&TVHT_ONITEMSTATEICON))
						{
							::PostMessage(hdlg,TREE_VIEW_CHECK_STATE_CHANGE,MAKEWPARAM(1,0),reinterpret_cast<LPARAM>(hti));
							PropSheet_Changed(::GetParent(hdlg),hdlg);
						}
					}
					break;
				case PSN_APPLY:
					{
						CQuotesProviderDukasCopy* pDukasProvider = get_dukas_copy_provider();
						if(pDukasProvider)
						{
							recursive_save_selection(::GetDlgItem(hdlg,IDC_TREE_ECONOMIC_RATES),pDukasProvider);
							pDukasProvider->RefreshSettings();
						}
					}
					break;
				}
			}
			return TRUE;

		case TREE_VIEW_CHECK_STATE_CHANGE:
			{
				HWND hwndTree = ::GetDlgItem(hdlg,IDC_TREE_ECONOMIC_RATES);
				HTREEITEM hti = reinterpret_cast<HTREEITEM>(lParam);

				ETreeCheckBoxState nState;

				bool bRecursively = 1 == LOWORD(wParam);
				if(bRecursively)
				{
					nState = tree_get_state_image(hwndTree,hti);
				}
				else
				{
					nState = static_cast<ETreeCheckBoxState>(HIWORD(wParam));
				}

				tree_set_item_state(hwndTree,hti,nState,bRecursively);
			}
			break;
			// 		case WM_CLOSE:
			// 			DestroyWindow(hdlg);
			// 			break;
			// 		case WM_DESTROY:
			// 			g_hwndEconomicRates = NULL;
			// 			break;
		}

		return FALSE;
	}
}

void ShowDukasCopyPropPage(CQuotesProviderDukasCopy* pProvider,WPARAM wp,OPTIONSDIALOGPAGE& odp)
{
	const IQuotesProvider::CProviderInfo& pi = pProvider->GetInfo();

	odp.pszTemplate = MAKEINTRESOURCEA(IDD_DIALOG_ECONOMIC_RATES);
	odp.pfnDlgProc = EconomicRatesDlgProc;
// #if MIRANDA_VER >= 0x0600
	//odp.ptszTab = TranslateTS(const_cast<LPTSTR>(pi.m_sName.c_str()));
	odp.ptszTab = const_cast<LPTSTR>(pi.m_sName.c_str());
// #else
// 	tostringstream o;
// 	o << TranslateTS(QUOTES_PROTOCOL_NAME) << _T(" - ") << TranslateTS(pi.m_sName.c_str());
// 	tstring sTitle = o.str();
// 	odp.ptszTitle = TranslateTS(const_cast<LPTSTR>(sTitle.c_str()));
// #endif

	Options_AddPage(wp, &odp);
}
