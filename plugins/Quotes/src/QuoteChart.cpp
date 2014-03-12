#include "StdAfx.h"

#ifdef CHART_IMPLEMENT

#define WINDOW_PREFIX "Quotes Chart_"
#define CHART_CTRL_CLASS _T("DioksinChart")

namespace
{
	struct CTimeConvert
	{
		static double Convert(const boost::posix_time::time_duration& v)
		{
			return boost::numeric_cast<double>(v.ticks());
		}

		static tstring ToString(const boost::posix_time::ptime& v)
		{
			tostringstream k;
			k.imbue(std::locale(GetSystemLocale(),new ttime_facet(_T("%d/%m/%y %H:%M:%S"))));
			k << v;
			return k.str();
		}
	};

	typedef CChart<boost::posix_time::ptime,double,CTimeConvert> TChart;

	inline TChart* get_chart_ptr(HWND hWnd)
	{
		TChart* pChart = reinterpret_cast<TChart*>(GetWindowLongPtr(hWnd,GWLP_USERDATA));
		return pChart;
	}

	bool read_log_file(MCONTACT hContact,TChart& rChart)
	{		
		tstring sLogFileName = GetContactLogFileName(hContact);
		if(false == sLogFileName.empty())
		{
			std::locale loc(GetSystemLocale(),new ttime_input_facet(_T("%d.%m.%y %H:%M:%S")));
			boost::posix_time::ptime oDateTime;
			double dRate;

			tifstream file(sLogFileName.c_str());
			file.imbue(loc);
			while((false == file.fail()) && (false == file.eof()))
			{
				tstring sLine;
				std::getline(file,sLine);

				tistringstream line(sLine);
				line.imbue(loc);

				tstring sName;
				std::getline(line,sName,_T('\t'));
				line >> oDateTime >> dRate;
				if ((false == line.fail()) && (true == line.eof()))
				{
					rChart.AddValue(oDateTime,dRate);
				}
			}

			return true;
		}
		return false;
	}


	enum
	{
		ID_CHART = 0x1969,

		srcLogFile = 0,
		srcHistory = 1,

		filterAll = 0,
		filterLastDay = 1,
		filterLastWeek = 2,
		filterLastMonth = 3,
		filterLastYear = 4,
		filterUserDefined = 5,

		CHART_SET_SOURCE = WM_USER + 1,
		CHART_SET_FILTER = WM_USER + 2,
	};

	LRESULT CALLBACK ChartWndProc(HWND hWnd,UINT msg,WPARAM wp,LPARAM lp) 
	{
		switch(msg)
		{
		case WM_CREATE:
			{
				CREATESTRUCT* pCS = reinterpret_cast<CREATESTRUCT*>(lp);
				MCONTACT hContact = reinterpret_cast<HANDLE>(pCS->lpCreateParams);

				TChart* pChart = new TChart;
				read_log_file(hContact,*pChart);

				::SetWindowLongPtr(hWnd,GWLP_USERDATA,reinterpret_cast<LONG_PTR>(pChart));
			}			
			return 0;

		case CHART_SET_SOURCE:
			break;

		case CHART_SET_FILTER:
			break;

		case WM_SIZE:
			{
				TChart* pChart = get_chart_ptr(hWnd);
				pChart->SetRect(0,0,LOWORD(lp),HIWORD(lp));
			}
			return 0;

		case WM_PAINT: 
			if(TRUE == ::GetUpdateRect(hWnd,NULL,FALSE))
			{
				PAINTSTRUCT ps;
				HDC hdc = ::BeginPaint(hWnd,&ps);
				if(NULL != hdc)
				{
					TChart* pChart = get_chart_ptr(hWnd);
					pChart->Draw(hdc);
					::EndPaint(hWnd,&ps);
				}
			}

			return 0; 
		case WM_DESTROY:
			{
				TChart* pChart = get_chart_ptr(hWnd);
				::SetWindowLongPtr(hWnd,GWLP_USERDATA,0);
				delete pChart;
			}
			break;
		}

		return ::DefWindowProc(hWnd,msg,wp,lp);
	}

	void register_chart_control()
	{
		static bool g_bRegister = false;
		if(g_bRegister)
		{
			return;
		}

		WNDCLASS wc; 

		wc.style = CS_HREDRAW | CS_VREDRAW; 
		wc.lpfnWndProc = ChartWndProc; 
		wc.cbClsExtra = 0; 
		wc.cbWndExtra = 0; 
		wc.hInstance = CModuleInfo::GetInstance().GetModuleHandle(); 
		wc.hIcon = NULL; 
		wc.hCursor = LoadCursor(NULL,IDC_ARROW); 
		wc.hbrBackground = static_cast<HBRUSH>(::GetStockObject(WHITE_BRUSH)); 
		wc.lpszMenuName =  NULL; 
		wc.lpszClassName = CHART_CTRL_CLASS; 

		if(RegisterClass(&wc))
		{
			g_bRegister = true;
		}
	}


	bool screen_2_client(HWND hWnd,LPRECT pRect)
	{
		POINT pt;
		pt.x = pRect->left;
		pt.y = pRect->top;
		bool bResult = TRUE == ::ScreenToClient(hWnd,&pt);
		pRect->left = pt.x;
		pRect->top = pt.y;
		pt.x = pRect->right;
		pt.y = pRect->bottom;
		bResult |= TRUE == ::ScreenToClient(hWnd,&pt);
		pRect->right = pt.x;
		pRect->bottom = pt.y;
		return bResult;
	} 

	inline HANDLE get_contact(HWND hWnd)
	{
		MCONTACT hContact = reinterpret_cast<HANDLE>(GetWindowLongPtr(hWnd,GWLP_USERDATA));
		return hContact;
	}

	void update_filter_controls(HWND hDlg)
	{
		int nSel = ::SendDlgItemMessage(hDlg,IDC_COMBO_FILTER,CB_GETCURSEL,0,0);

		::ShowWindow(::GetDlgItem(hDlg,IDC_EDIT_FROM),(filterUserDefined == nSel) ? SW_SHOW : SW_HIDE);
		::ShowWindow(::GetDlgItem(hDlg,IDC_EDIT_TO),(filterUserDefined == nSel) ? SW_SHOW : SW_HIDE);
	}

	INT_PTR CALLBACK ChartDlgProc(HWND hDlg,UINT msg,WPARAM wp,LPARAM lp)
	{
		switch(msg)
		{
		case WM_INITDIALOG:
			{
				MCONTACT hContact = reinterpret_cast<HANDLE>(lp);

				TranslateDialogDefault(hDlg);

				tstring sName = get_window_text(hDlg);
				sName += _T(" - ");
				sName += GetContactName(hContact);
				::SetWindowText(hDlg,sName.c_str());

				HANDLE hWL = CModuleInfo::GetInstance().GetWindowList(WINDOW_PREFIX,false);
				assert(hWL);
				WindowList_Add(hWL,hDlg,hContact);

				::SetWindowLongPtr(hDlg,GWLP_USERDATA,reinterpret_cast<LONG_PTR>(hContact));

				static LPCTSTR szSources[] = {LPGENT("Log File"), LPGENT("Miranda's History")};
				static LPCTSTR szFilters[] = {LPGENT("All"), LPGENT("Last Day"), LPGENT("Last Week"), LPGENT("Last Month"), LPGENT("Last Year"), LPGENT("User-Defined")};

				for(int i = 0;i < sizeof(szSources)/sizeof(szSources[0]);++i)
				{
					LPCTSTR p = TranslateTS(szSources[i]);
					::SendDlgItemMessage(hDlg,IDC_COMBO_DATA_SOURCE,CB_INSERTSTRING,-1,reinterpret_cast<LPARAM>(p));
				}

				int nSel = db_get_b(hContact,QUOTES_PROTOCOL_NAME,"Chart_Source",srcLogFile);
				::SendDlgItemMessage(hDlg,IDC_COMBO_DATA_SOURCE,CB_SETCURSEL,nSel,0);

				for(int i = 0;i < sizeof(szFilters)/sizeof(szFilters[0]);++i)
				{
					LPCTSTR p = TranslateTS(szSources[i]);
					::SendDlgItemMessage(hDlg,IDC_COMBO_FILTER,CB_INSERTSTRING,-1,reinterpret_cast<LPARAM>(szFilters[i]));
				}

				nSel = db_get_b(hContact,QUOTES_PROTOCOL_NAME,"Chart_Filter",filterAll);
				::SendDlgItemMessage(hDlg,IDC_COMBO_FILTER,CB_SETCURSEL,nSel,0);

				update_filter_controls(hDlg);

				register_chart_control();
				HWND hwndImage = ::GetDlgItem(hDlg,IDC_STATIC_IMAGE);
				RECT rcImage;
				::GetWindowRect(hwndImage,&rcImage);
				screen_2_client(hDlg,&rcImage);
				//BOOL bResult = ShowWindow(hwndImage,SW_HIDE);
				//assert(bResult);

				HWND hChart = ::CreateWindowEx(0L,CHART_CTRL_CLASS,NULL,WS_CHILDWINDOW|WS_VISIBLE,
					rcImage.left,rcImage.top,rcImage.right-rcImage.left,rcImage.bottom-rcImage.top,
					hDlg,reinterpret_cast<HMENU>(ID_CHART),CModuleInfo::GetInstance().GetModuleHandle(),hContact);
				assert(NULL != hChart);

				Utils_RestoreWindowPosition(hDlg,hContact,QUOTES_MODULE_NAME,WINDOW_PREFIX);
				BOOL bResult = ::ShowWindow(hDlg,SW_SHOW);
				assert(bResult);
			}
			return (TRUE);
		case WM_CLOSE:
			{
				MCONTACT hContact = get_contact(hDlg);
				SetWindowLongPtr(hDlg,GWLP_USERDATA,0);

// 				save_options(hDlg,hContact);

				HANDLE hWL = CModuleInfo::GetInstance().GetWindowList(WINDOW_PREFIX,false);
				assert(hWL);
				WindowList_Remove(hWL,hDlg);
				Utils_SaveWindowPosition(hDlg,hContact,QUOTES_MODULE_NAME,WINDOW_PREFIX);

				HWND hwndChart = ::GetDlgItem(hDlg,ID_CHART);
				BOOL bResult = ::DestroyWindow(hwndChart);
				assert(bResult);
				
				::EndDialog(hDlg,0);
			}
			return (TRUE);
		case WM_COMMAND:
			switch(LOWORD(wp))
			{
			case IDCANCEL:
				{
					SendMessage(hDlg, WM_CLOSE, 0, 0);
				}
				return (TRUE);
			case IDC_COMBO_FILTER:
				if(CBN_SELCHANGE == HIWORD(wp))
				{
					::SendDlgItemMessage(hDlg,ID_CHART,CHART_SET_FILTER,::SendDlgItemMessage(hDlg,IDC_COMBO_FILTER,CB_GETCURSEL,0,0),0);
					update_filter_controls(hDlg);
				}
				break;
			case IDC_COMBO_DATA_SOURCE:
				if(CBN_SELCHANGE == HIWORD(wp))
				{
					::SendDlgItemMessage(hDlg,ID_CHART,CHART_SET_SOURCE,::SendDlgItemMessage(hDlg,IDC_COMBO_DATA_SOURCE,CB_GETCURSEL,0,0),0);
				}
				break;
			}
			return (FALSE);
		case WM_NOTIFY:
			{
				LPNMHDR pNMHDR = reinterpret_cast<LPNMHDR>(lp);
				switch(pNMHDR->code)
				{
				case NM_CLICK:
					if(IDC_SYSLINK_PROVIDER == wp)
					{
						PNMLINK pNMLink = reinterpret_cast<PNMLINK>(pNMHDR);
						::ShellExecute(hDlg,_T("open"),pNMLink->item.szUrl,NULL,NULL,SW_SHOWNORMAL);					
					}
					break;
				}
			}
			break;
// 		case WM_ERASEBKGND:
// 			{
// 				HDC hdc = reinterpret_cast<HDC>(wp);
// 				TChart* pChart = get_chart_ptr(hDlg);
// 				pChart->DrawBackground(hdc);
// 				return TRUE;
// 			}
// 			break;
		case WM_SIZE:
			{
				enum{ INDENT = 7};
				
				int nWidth = LOWORD(lp);
				int nHeight = HIWORD(lp);

				HWND hwndChart = GetDlgItem(hDlg,ID_CHART);
				HWND hwndLink = GetDlgItem(hDlg,IDC_SYSLINK_PROVIDER);
				HWND hwndClose = GetDlgItem(hDlg,IDCANCEL);

				RECT rcDlg;
				GetClientRect(hDlg,&rcDlg);

				RECT rcChart;
				GetWindowRect(hwndChart,&rcChart);
				screen_2_client(hDlg,&rcChart);

				RECT rcLink;
				GetWindowRect(hwndLink,&rcLink);
				screen_2_client(hDlg,&rcLink);
				SetWindowPos(hwndLink,NULL,rcDlg.left + INDENT,
					rcDlg.bottom-INDENT-(rcLink.bottom-rcLink.top),
					0,0,SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
				
				RECT rcClose;
				GetWindowRect(hwndClose,&rcClose);
				screen_2_client(hDlg,&rcClose);
				SetWindowPos(hwndClose,NULL,rcDlg.right - INDENT - (rcClose.right-rcClose.left),
					rcDlg.bottom-INDENT-(rcClose.bottom-rcClose.top),
					0,0,SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);

				SetWindowPos(hwndChart,NULL,rcDlg.left + INDENT,
											rcChart.top,
											(nWidth-INDENT*2),
											nHeight-(rcClose.bottom-rcClose.top)-INDENT*2-rcChart.top,
											SWP_NOZORDER|SWP_NOACTIVATE);

			}
			break;
		}
		return (FALSE);
	}
}

INT_PTR QuotesMenu_Chart(WPARAM wp,LPARAM lp)
{
	MCONTACT hContact = reinterpret_cast<HANDLE>(wp);
	if(NULL == hContact)
	{
		return 0;
	}

	HANDLE hWL = CModuleInfo::GetInstance().GetWindowList(WINDOW_PREFIX,true);
	assert(hWL);
	HWND hWnd = WindowList_Find(hWL,hContact);
	if(NULL != hWnd) 
	{
		SetForegroundWindow(hWnd);
		SetFocus(hWnd);
	}
	else
	{
		CreateDialogParam(CModuleInfo::GetModuleHandle(), MAKEINTRESOURCE(IDD_DUKASCOPY_CHART), NULL, ChartDlgProc, reinterpret_cast<LPARAM>(hContact));
	}

	return 0;
}

#endif //CHART_IMPLEMENT
