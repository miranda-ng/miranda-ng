#include "StdAfx.h"

#define WINDOW_PREFIX_SETTINGS "Edit Settings_"

namespace
{
	LPCTSTR g_pszVariableQuoteName = _T("%quotename%");
	LPCTSTR g_pszVariableUserProfile = _T("%miranda_userdata%");

	void update_file_controls(HWND hDlg)
	{
		bool bEnable = (1 == ::IsDlgButtonChecked(hDlg,IDC_CHECK_EXTERNAL_FILE));

		::EnableWindow(::GetDlgItem(hDlg,IDC_EDIT_FILE_NAME),bEnable);
		::EnableWindow(::GetDlgItem(hDlg,IDC_STATIC_SELECT_FILE),bEnable);
		::EnableWindow(::GetDlgItem(hDlg,IDC_BUTTON_BROWSE),bEnable);
		::EnableWindow(::GetDlgItem(hDlg,IDC_STATIC_LOG_FILE_FORMAT),bEnable);
		::EnableWindow(::GetDlgItem(hDlg,IDC_EDIT_LOG_FILE_FORMAT),bEnable);
		::EnableWindow(::GetDlgItem(hDlg,IDC_BUTTON_LOG_FILE_DESCRIPTION),bEnable);
		::EnableWindow(::GetDlgItem(hDlg,IDC_CHECK_LOG_FILE_CONDITION),bEnable);
	}

	void update_history_controls(HWND hDlg)
	{
		bool bEnable = (1 == ::IsDlgButtonChecked(hDlg,IDC_CHECK_INTERNAL_HISTORY));

		::EnableWindow(::GetDlgItem(hDlg,IDC_STATIC_HISTORY_FORMAT),bEnable);
		::EnableWindow(::GetDlgItem(hDlg,IDC_EDIT_HISTORY_FORMAT),bEnable);
		::EnableWindow(::GetDlgItem(hDlg,IDC_BUTTON_HISTORY_DESCRIPTION),bEnable);
		::EnableWindow(::GetDlgItem(hDlg,IDC_CHECK_HISTORY_CONDITION),bEnable);
	}

	void update_popup_controls(HWND hDlg)
	{
		bool bEnable = (1 == ::IsDlgButtonChecked(hDlg,IDC_CHECK_SHOW_POPUP));
		::EnableWindow(::GetDlgItem(hDlg,IDC_EDIT_POPUP_FORMAT),bEnable);
		::EnableWindow(::GetDlgItem(hDlg,IDC_CHECK_SHOW_POPUP_ONLY_VALUE_CHANGED),bEnable);
		::EnableWindow(::GetDlgItem(hDlg,IDC_STATIC_POPUP_FORMAT),bEnable);
		::EnableWindow(::GetDlgItem(hDlg,IDC_BUTTON_POPUP_FORMAT_DESCRIPTION),bEnable);
		::EnableWindow(::GetDlgItem(hDlg,IDC_BUTTON_POPUP_SETTINGS),bEnable);
	}

	bool enable_popup_controls(HWND hDlg)
	{
		bool bIsPopupServiceEnabled = 1 == ServiceExists(MS_POPUP_ADDPOPUPT);
		::EnableWindow(::GetDlgItem(hDlg,IDC_CHECK_SHOW_POPUP),bIsPopupServiceEnabled);
		::EnableWindow(::GetDlgItem(hDlg,IDC_EDIT_POPUP_FORMAT),bIsPopupServiceEnabled);
		::EnableWindow(::GetDlgItem(hDlg,IDC_CHECK_SHOW_POPUP_ONLY_VALUE_CHANGED),bIsPopupServiceEnabled);
		::EnableWindow(::GetDlgItem(hDlg,IDC_STATIC_POPUP_FORMAT),bIsPopupServiceEnabled);
		::EnableWindow(::GetDlgItem(hDlg,IDC_BUTTON_POPUP_FORMAT_DESCRIPTION),bIsPopupServiceEnabled);
		::EnableWindow(::GetDlgItem(hDlg,IDC_BUTTON_POPUP_SETTINGS),bIsPopupServiceEnabled);

		return bIsPopupServiceEnabled;
	}

	void update_all_controls(HWND hDlg)
	{
		bool bIsCheckedContactSpec = (1 == ::IsDlgButtonChecked(hDlg,IDC_CHECK_CONTACT_SPECIFIC));
		bool bIsCheckedExternal = (1 == ::IsDlgButtonChecked(hDlg,IDC_CHECK_EXTERNAL_FILE));

		::EnableWindow(::GetDlgItem(hDlg,IDC_CHECK_EXTERNAL_FILE),bIsCheckedContactSpec);
		::EnableWindow(::GetDlgItem(hDlg,IDC_EDIT_FILE_NAME),(bIsCheckedContactSpec&&bIsCheckedExternal));
		::EnableWindow(::GetDlgItem(hDlg,IDC_STATIC_SELECT_FILE),(bIsCheckedContactSpec&&bIsCheckedExternal));
		::EnableWindow(::GetDlgItem(hDlg,IDC_BUTTON_BROWSE),(bIsCheckedContactSpec&&bIsCheckedExternal));
		::EnableWindow(::GetDlgItem(hDlg,IDC_STATIC_LOG_FILE_FORMAT),(bIsCheckedContactSpec&&bIsCheckedExternal));
		::EnableWindow(::GetDlgItem(hDlg,IDC_EDIT_LOG_FILE_FORMAT),(bIsCheckedContactSpec&&bIsCheckedExternal));
		::EnableWindow(::GetDlgItem(hDlg,IDC_BUTTON_LOG_FILE_DESCRIPTION),(bIsCheckedContactSpec&&bIsCheckedExternal));
		::EnableWindow(::GetDlgItem(hDlg,IDC_CHECK_LOG_FILE_CONDITION),(bIsCheckedContactSpec&&bIsCheckedExternal));

		bool bIsCheckedHistory = (1 == ::IsDlgButtonChecked(hDlg,IDC_CHECK_INTERNAL_HISTORY));
		::EnableWindow(::GetDlgItem(hDlg,IDC_CHECK_INTERNAL_HISTORY),bIsCheckedContactSpec);
		::EnableWindow(::GetDlgItem(hDlg,IDC_STATIC_HISTORY_FORMAT),(bIsCheckedContactSpec&&bIsCheckedHistory));
		::EnableWindow(::GetDlgItem(hDlg,IDC_EDIT_HISTORY_FORMAT),(bIsCheckedContactSpec&&bIsCheckedHistory));
		::EnableWindow(::GetDlgItem(hDlg,IDC_BUTTON_HISTORY_DESCRIPTION),(bIsCheckedContactSpec&&bIsCheckedHistory));
		::EnableWindow(::GetDlgItem(hDlg,IDC_CHECK_HISTORY_CONDITION),(bIsCheckedContactSpec&&bIsCheckedHistory));

		bool bIsPopupServiceEnabled = 1 == ServiceExists(MS_POPUP_ADDPOPUPT);
		bool bIsCheckedShowPopup = (1 == ::IsDlgButtonChecked(hDlg,IDC_CHECK_SHOW_POPUP));
		::EnableWindow(::GetDlgItem(hDlg,IDC_CHECK_SHOW_POPUP),(bIsCheckedContactSpec&&bIsPopupServiceEnabled));
		::EnableWindow(::GetDlgItem(hDlg,IDC_EDIT_POPUP_FORMAT),(bIsCheckedContactSpec&&bIsPopupServiceEnabled&&bIsCheckedShowPopup));
		::EnableWindow(::GetDlgItem(hDlg,IDC_CHECK_SHOW_POPUP_ONLY_VALUE_CHANGED),(bIsCheckedContactSpec&&bIsPopupServiceEnabled&&bIsCheckedShowPopup));
		::EnableWindow(::GetDlgItem(hDlg,IDC_STATIC_POPUP_FORMAT),(bIsCheckedContactSpec&&bIsPopupServiceEnabled&&bIsCheckedShowPopup));
		::EnableWindow(::GetDlgItem(hDlg,IDC_BUTTON_POPUP_FORMAT_DESCRIPTION),(bIsCheckedContactSpec&&bIsPopupServiceEnabled&&bIsCheckedShowPopup));
		::EnableWindow(::GetDlgItem(hDlg,IDC_BUTTON_POPUP_SETTINGS),(bIsCheckedContactSpec&&bIsPopupServiceEnabled));
	}

	std::vector<TCHAR> get_filter()
	{
		std::vector<TCHAR> aFilter;
		LPCTSTR pszFilterParts[] = {LPGENT("Log Files (*.txt,*.log)"), _T("*.txt;*.log"), LPGENT("All files (*.*)"), _T("*.*")};
		for(int i = 0;i < sizeof(pszFilterParts)/sizeof(pszFilterParts[0]);++i)
		{
			tstring sPart = TranslateTS(pszFilterParts[i]);
			std::copy(sPart.begin(),sPart.end(),std::back_inserter(aFilter));
			aFilter.push_back(_T('\0'));

		}
		aFilter.push_back(_T('\0'));
		return aFilter;
	}
	void select_log_file(HWND hDlg)
	{
		std::vector<TCHAR> aFileBuffer(_MAX_PATH*2,_T('\0'));
		LPTSTR pszFile = &*aFileBuffer.begin();

		std::vector<TCHAR> aFilterBuffer = get_filter();
		LPCTSTR pszFilter = &*aFilterBuffer.begin();

		OPENFILENAME ofn = {0};
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = hDlg;
		ofn.lpstrFile = pszFile;
		ofn.nMaxFile = (DWORD)aFileBuffer.size();
		ofn.lpstrFilter = pszFilter;
		ofn.nFilterIndex = 1;
		ofn.hInstance = g_hInstance;
		ofn.lpstrDefExt = _T("log");
		ofn.Flags = OFN_PATHMUSTEXIST|OFN_HIDEREADONLY|OFN_EXPLORER;

		BOOL b = GetOpenFileName(&ofn);
		if(TRUE == b)
		{
			SetDlgItemText(hDlg,IDC_EDIT_FILE_NAME,ofn.lpstrFile);
		}
	}

	struct CSettingWindowParam
	{
		CSettingWindowParam(MCONTACT hContact) : m_hContact(hContact),m_pPopupSettings(NULL){}
		~CSettingWindowParam(){delete m_pPopupSettings;}

		MCONTACT m_hContact;
		CPopupSettings* m_pPopupSettings;
	};

	inline CSettingWindowParam* get_param(HWND hWnd)
	{
		return reinterpret_cast<CSettingWindowParam*>(GetWindowLongPtr(hWnd,GWLP_USERDATA));
	}

	void update_popup_controls_settings(HWND hDlg)
	{
		bool bIsColoursEnabled = 1 == IsDlgButtonChecked(hDlg,IDC_RADIO_USER_DEFINED_COLOURS);
		::EnableWindow(::GetDlgItem(hDlg,IDC_BGCOLOR),bIsColoursEnabled);
		::EnableWindow(::GetDlgItem(hDlg,IDC_TEXTCOLOR),bIsColoursEnabled);

		bool bIsDelayEnabled = 1 == IsDlgButtonChecked(hDlg,IDC_DELAYCUSTOM);
		::EnableWindow(::GetDlgItem(hDlg,IDC_DELAY),bIsDelayEnabled);

	}

	INT_PTR CALLBACK EditPopupSettingsDlgProc(HWND hWnd,UINT msg,WPARAM wp,LPARAM lp)
	{
		switch(msg)
		{
		case WM_INITDIALOG:
			{
				CPopupSettings* pSettings = reinterpret_cast<CPopupSettings*>(lp);
				TranslateDialogDefault( hWnd );
				::SendDlgItemMessage(hWnd,IDC_BGCOLOR,CPM_SETCOLOUR,0,pSettings->GetColourBk());
				::SendDlgItemMessage(hWnd,IDC_TEXTCOLOR,CPM_SETCOLOUR,0,pSettings->GetColourText());

				::CheckDlgButton(hWnd,IDC_CHECK_DONT_USE_POPUPHISTORY,pSettings->GetHistoryFlag());

				::CheckRadioButton(hWnd,IDC_RADIO_DEFAULT_COLOURS,IDC_RADIO_USER_DEFINED_COLOURS,(CPopupSettings::colourDefault == pSettings->GetColourMode()) ? IDC_RADIO_DEFAULT_COLOURS : IDC_RADIO_USER_DEFINED_COLOURS);
				UINT n;
				switch(pSettings->GetDelayMode())
				{
				default:
					assert(!"Unknown delay mode. Please, fix it");
				case CPopupSettings::delayFromPopup:
					n = IDC_DELAYFROMPU;
					break;
				case CPopupSettings::delayCustom:
					n = IDC_DELAYCUSTOM;
					break;
				case CPopupSettings::delayPermanent:
					n = IDC_DELAYPERMANENT;
					break;
				}
				::CheckRadioButton(hWnd,IDC_DELAYFROMPU,IDC_DELAYPERMANENT,n);

				::SetDlgItemInt(hWnd,IDC_DELAY,pSettings->GetDelayTimeout(),FALSE);

				update_popup_controls_settings(hWnd);

				::SetWindowLongPtr(hWnd,GWLP_USERDATA,reinterpret_cast<LONG_PTR>(pSettings));
			}
			return TRUE;
		case WM_COMMAND:
			switch(LOWORD(wp))
			{
			case IDC_RADIO_DEFAULT_COLOURS:
			case IDC_RADIO_USER_DEFINED_COLOURS:
			case IDC_DELAYFROMPU:
			case IDC_DELAYCUSTOM:
			case IDC_DELAYPERMANENT:
				update_popup_controls_settings(hWnd);
				break;

			case IDCANCEL:
				::EndDialog(hWnd,IDCANCEL);
				break;
			case IDOK:
				{
					CPopupSettings* pSettings = reinterpret_cast<CPopupSettings*>(GetWindowLongPtr(hWnd,GWLP_USERDATA));

					bool bError = false;
					BOOL bOk = FALSE;
					UINT nDelay = ::GetDlgItemInt(hWnd,IDC_DELAY,&bOk,FALSE);
					CPopupSettings::EDelayMode nModeDelay = pSettings->GetDelayMode();
					if(1 == ::IsDlgButtonChecked(hWnd,IDC_DELAYFROMPU))
					{
						nModeDelay = CPopupSettings::delayFromPopup;
					}
					else if(1 == ::IsDlgButtonChecked(hWnd,IDC_DELAYCUSTOM))
					{
						if(TRUE == bOk)
						{
							nModeDelay = CPopupSettings::delayCustom;
						}
						else
						{
							prepare_edit_ctrl_for_error(::GetDlgItem(hWnd,IDC_DELAY));
							Quotes_MessageBox(hWnd,TranslateT("Enter integer value"),MB_OK|MB_ICONERROR);
							bError = true;
						}
					}
					else if(1 == ::IsDlgButtonChecked(hWnd,IDC_DELAYPERMANENT))
					{
						nModeDelay = CPopupSettings::delayPermanent;
					}
					if(false == bError)
					{
						pSettings->SetDelayMode(nModeDelay);
						if(TRUE == bOk)
						{
							pSettings->SetDelayTimeout(nDelay);
						}
						pSettings->SetHistoryFlag((1 == IsDlgButtonChecked(hWnd,IDC_CHECK_DONT_USE_POPUPHISTORY)));

						if(1 == ::IsDlgButtonChecked(hWnd,IDC_RADIO_DEFAULT_COLOURS))
						{
							pSettings->SetColourMode(CPopupSettings::colourDefault);
						}
						else if(1 == ::IsDlgButtonChecked(hWnd,IDC_RADIO_USER_DEFINED_COLOURS))
						{
							pSettings->SetColourMode(CPopupSettings::colourUserDefined);
						}

						pSettings->SetColourBk(static_cast<COLORREF>(::SendDlgItemMessage(hWnd,IDC_BGCOLOR,CPM_GETCOLOUR,0,0)));
						pSettings->SetColourText(static_cast<COLORREF>(::SendDlgItemMessage(hWnd,IDC_TEXTCOLOR,CPM_GETCOLOUR,0,0)));

						::EndDialog(hWnd,IDOK);
					}
				}
				break;
			}
			break;
		}

		return FALSE;
	}

	INT_PTR CALLBACK EditSettingsPerContactDlgProc(HWND hWnd,UINT msg,WPARAM wp,LPARAM lp)
	{
		switch(msg)
		{
		case WM_INITDIALOG:
			{
				MCONTACT hContact = MCONTACT(lp);
				TranslateDialogDefault(hWnd);

				HANDLE hWL = CModuleInfo::GetInstance().GetWindowList(WINDOW_PREFIX_SETTINGS,false);
				assert(hWL);
				WindowList_Add(hWL,hWnd,hContact);

				tstring sName = GetContactName(hContact);
				::SetDlgItemText(hWnd,IDC_EDIT_NAME,sName.c_str());

				CQuotesProviders::TQuotesProviderPtr pProvider = CModuleInfo::GetQuoteProvidersPtr()->GetContactProviderPtr(hContact);

				BYTE bUseContactSpecific = db_get_b(hContact,QUOTES_PROTOCOL_NAME,DB_STR_CONTACT_SPEC_SETTINGS,0);
				::CheckDlgButton(hWnd,IDC_CHECK_CONTACT_SPECIFIC,bUseContactSpecific);

				CAdvProviderSettings setGlobal(pProvider.get());
				// log to history
				WORD dwLogMode = db_get_w(hContact,QUOTES_PROTOCOL_NAME,DB_STR_QUOTE_LOG,setGlobal.GetLogMode());
				UINT nCheck = (dwLogMode&lmInternalHistory) ? 1 : 0;
				::CheckDlgButton(hWnd,IDC_CHECK_INTERNAL_HISTORY,nCheck);

				tstring sHistoryFrmt = Quotes_DBGetStringT(hContact,QUOTES_PROTOCOL_NAME,DB_STR_QUOTE_FORMAT_HISTORY,setGlobal.GetHistoryFormat().c_str());
				::SetDlgItemText(hWnd,IDC_EDIT_HISTORY_FORMAT,sHistoryFrmt.c_str());

				WORD wOnlyIfChanged = db_get_w(hContact,QUOTES_PROTOCOL_NAME,DB_STR_QUOTE_HISTORY_CONDITION,setGlobal.GetHistoryOnlyChangedFlag());
				::CheckDlgButton(hWnd,IDC_CHECK_HISTORY_CONDITION,(1 == wOnlyIfChanged) ? 1 : 0);

				// log to file
				nCheck = (dwLogMode&lmExternalFile) ? 1 : 0;
				::CheckDlgButton(hWnd,IDC_CHECK_EXTERNAL_FILE,nCheck);

				tstring sLogFileName = Quotes_DBGetStringT(hContact,QUOTES_PROTOCOL_NAME,DB_STR_QUOTE_LOG_FILE);
				if(true == sLogFileName.empty())
				{
					sLogFileName = GenerateLogFileName(setGlobal.GetLogFileName(),Quotes_DBGetStringT(hContact,QUOTES_PROTOCOL_NAME,DB_STR_QUOTE_SYMBOL),glfnResolveQuoteName);
				}
				::SetDlgItemText(hWnd,IDC_EDIT_FILE_NAME,sLogFileName.c_str());

				tstring sLogFileFrmt = Quotes_DBGetStringT(hContact,QUOTES_PROTOCOL_NAME,DB_STR_QUOTE_FORMAT_LOG_FILE,setGlobal.GetLogFormat().c_str());
				::SetDlgItemText(hWnd,IDC_EDIT_LOG_FILE_FORMAT,sLogFileFrmt.c_str());

				wOnlyIfChanged = db_get_w(hContact,QUOTES_PROTOCOL_NAME,DB_STR_QUOTE_LOG_FILE_CONDITION,setGlobal.GetLogOnlyChangedFlag());
				::CheckDlgButton(hWnd,IDC_CHECK_LOG_FILE_CONDITION,(1 == wOnlyIfChanged) ? 1 : 0);

				// popup
				nCheck = (dwLogMode&lmPopup) ? 1 : 0;
				::CheckDlgButton(hWnd,IDC_CHECK_SHOW_POPUP,nCheck);
				tstring sPopupFrmt = Quotes_DBGetStringT(hContact,QUOTES_PROTOCOL_NAME,DB_STR_QUOTE_FORMAT_POPUP,setGlobal.GetPopupFormat().c_str());
				::SetDlgItemText(hWnd,IDC_EDIT_POPUP_FORMAT,sPopupFrmt.c_str());
				bool bOnlyIfChanged = 1 == db_get_b(hContact,QUOTES_PROTOCOL_NAME,DB_STR_QUOTE_POPUP_CONDITION,setGlobal.GetShowPopupIfValueChangedFlag());
				::CheckDlgButton(hWnd,IDC_CHECK_SHOW_POPUP_ONLY_VALUE_CHANGED,(true == bOnlyIfChanged) ? 1 : 0);

				update_all_controls(hWnd);

				CSettingWindowParam* pParam = new CSettingWindowParam(hContact);
				::SetWindowLongPtr(hWnd,GWLP_USERDATA,reinterpret_cast<LONG_PTR>(pParam));
				Utils_RestoreWindowPositionNoSize(hWnd,hContact,QUOTES_MODULE_NAME,WINDOW_PREFIX_SETTINGS);
				::ShowWindow(hWnd,SW_SHOW);
			}
			break;
		case WM_COMMAND:
			switch(LOWORD(wp)) 
			{
			case IDC_BUTTON_HISTORY_DESCRIPTION:
			case IDC_BUTTON_LOG_FILE_DESCRIPTION:
			case IDC_BUTTON_POPUP_FORMAT_DESCRIPTION:
				if(BN_CLICKED == HIWORD(wp))
				{
					CQuotesProviders::TQuotesProviderPtr pProvider = CModuleInfo::GetQuoteProvidersPtr()->GetContactProviderPtr(get_param(hWnd)->m_hContact);
					show_variable_list(hWnd,pProvider.get());
				}
				break;

			case IDC_CHECK_CONTACT_SPECIFIC:
				if(BN_CLICKED == HIWORD(wp))
				{
					update_all_controls(hWnd);
				}
				break;
			case IDC_CHECK_EXTERNAL_FILE:
				if(BN_CLICKED == HIWORD(wp))
				{
					update_file_controls(hWnd);
				}
				break;
			case IDC_CHECK_INTERNAL_HISTORY:
				if(BN_CLICKED == HIWORD(wp))
				{
					update_history_controls(hWnd);
				}
				break;
			case IDC_CHECK_SHOW_POPUP:
				if(BN_CLICKED == HIWORD(wp))
				{
					update_popup_controls(hWnd);
				}
				break;
			case IDC_BUTTON_BROWSE:
				if(BN_CLICKED == HIWORD(wp))
				{
					select_log_file(hWnd);
				}
				break;
			case IDC_BUTTON_POPUP_SETTINGS:
				if(BN_CLICKED == HIWORD(wp))
				{
					CSettingWindowParam* pParam = get_param(hWnd);
					if (!pParam->m_pPopupSettings)
					{
						CQuotesProviders::TQuotesProviderPtr pProvider = CModuleInfo::GetQuoteProvidersPtr()->GetContactProviderPtr(pParam->m_hContact);

						pParam->m_pPopupSettings = new CPopupSettings(pProvider.get());
						pParam->m_pPopupSettings->InitForContact(pParam->m_hContact);
					}

					DialogBoxParam(g_hInstance,
						MAKEINTRESOURCE(IDD_DIALOG_POPUP),
						hWnd,
						EditPopupSettingsDlgProc,reinterpret_cast<LPARAM>(pParam->m_pPopupSettings));
				}
				break;

			case IDOK:
				{
					CSettingWindowParam* pParam =  get_param(hWnd);
					MCONTACT hContact = pParam->m_hContact;

					bool bUseContactSpec = 1 == ::IsDlgButtonChecked(hWnd,IDC_CHECK_CONTACT_SPECIFIC);

					WORD nLogMode = lmDisabled;
					UINT nCheck = ::IsDlgButtonChecked(hWnd,IDC_CHECK_EXTERNAL_FILE);
					if(1 == nCheck)
					{
						nLogMode |= lmExternalFile;
					}

					nCheck = ::IsDlgButtonChecked(hWnd,IDC_CHECK_INTERNAL_HISTORY);
					if(1 == nCheck)
					{
						nLogMode |= lmInternalHistory;
					}

					nCheck = ::IsDlgButtonChecked(hWnd,IDC_CHECK_SHOW_POPUP);
					if(1 == nCheck)
					{
						nLogMode |= lmPopup;
					}

					bool bOk = true;
					HWND hwndLogFile = ::GetDlgItem(hWnd,IDC_EDIT_FILE_NAME);
					HWND hwndLogFileFrmt = ::GetDlgItem(hWnd,IDC_EDIT_LOG_FILE_FORMAT);
					HWND hwndHistoryFrmt = ::GetDlgItem(hWnd,IDC_EDIT_HISTORY_FORMAT);
					tstring sLogFile = get_window_text(hwndLogFile);
					tstring sLogFileFormat = get_window_text(hwndLogFileFrmt);
					tstring sHistoryFormat = get_window_text(hwndHistoryFrmt);
					if ((nLogMode&lmExternalFile))
					{
						if(true == sLogFile.empty())
						{
							prepare_edit_ctrl_for_error(hwndLogFile);
							Quotes_MessageBox(hWnd,TranslateT("Enter log file name."),MB_OK|MB_ICONERROR);
							bOk = false;
						}
						else if(true == sLogFileFormat.empty())
						{
							prepare_edit_ctrl_for_error(hwndLogFileFrmt);
							Quotes_MessageBox(hWnd,TranslateT("Enter log file format."),MB_OK|MB_ICONERROR);
							bOk = false;
						}
					}

					if ((true == bOk) && (nLogMode&lmInternalHistory) && (true == sHistoryFormat.empty()))
					{
						prepare_edit_ctrl_for_error(hwndHistoryFrmt);
						Quotes_MessageBox(hWnd,TranslateT("Enter history format."),MB_OK|MB_ICONERROR);
						bOk = false;
					}

					HWND hwndPopupFrmt = ::GetDlgItem(hWnd,IDC_EDIT_POPUP_FORMAT);
					tstring sPopupFormat = get_window_text(hwndPopupFrmt);
					if ((true == bOk) && (nLogMode&lmPopup) && (true == sPopupFormat.empty()))
					{
						prepare_edit_ctrl_for_error(hwndPopupFrmt);
						Quotes_MessageBox(hWnd,TranslateT("Enter popup window format."),MB_OK|MB_ICONERROR);
						bOk = false;
					}

					if(true == bOk)
					{
						UINT nIfChangedHistory = IsDlgButtonChecked(hWnd,IDC_CHECK_HISTORY_CONDITION);
						UINT nIfChangedFile = IsDlgButtonChecked(hWnd,IDC_CHECK_LOG_FILE_CONDITION);
						bool bIfChangedPopup = (1 == IsDlgButtonChecked(hWnd,IDC_CHECK_SHOW_POPUP_ONLY_VALUE_CHANGED));

						db_set_b(hContact,QUOTES_PROTOCOL_NAME,DB_STR_CONTACT_SPEC_SETTINGS,bUseContactSpec);
						db_set_w(hContact,QUOTES_PROTOCOL_NAME,DB_STR_QUOTE_LOG,nLogMode);
						db_set_w(hContact,QUOTES_PROTOCOL_NAME,DB_STR_QUOTE_LOG_FILE_CONDITION,nIfChangedFile);
						db_set_w(hContact,QUOTES_PROTOCOL_NAME,DB_STR_QUOTE_HISTORY_CONDITION,nIfChangedHistory);
						db_set_b(hContact,QUOTES_PROTOCOL_NAME,DB_STR_QUOTE_POPUP_CONDITION,bIfChangedPopup);
						db_set_ts(hContact,QUOTES_PROTOCOL_NAME,DB_STR_QUOTE_LOG_FILE,sLogFile.c_str());
						db_set_ts(hContact,QUOTES_PROTOCOL_NAME,DB_STR_QUOTE_FORMAT_LOG_FILE,sLogFileFormat.c_str());
						db_set_ts(hContact,QUOTES_PROTOCOL_NAME,DB_STR_QUOTE_FORMAT_HISTORY,sHistoryFormat.c_str());
						db_set_ts(hContact,QUOTES_PROTOCOL_NAME,DB_STR_QUOTE_FORMAT_POPUP,sPopupFormat.c_str());

						if(pParam->m_pPopupSettings)
						{
							pParam->m_pPopupSettings->SaveForContact(hContact);
						}

						::DestroyWindow(hWnd);
					}
				}
				break;
			case IDCANCEL:
				DestroyWindow(hWnd);
				break;
			}
			break;
		case WM_CLOSE:
			DestroyWindow(hWnd);
			break;
		case WM_DESTROY:
			{
				CSettingWindowParam* pParam = get_param(hWnd);
				SetWindowLongPtr(hWnd,GWLP_USERDATA,0);

				HANDLE hWL = CModuleInfo::GetInstance().GetWindowList(WINDOW_PREFIX_SETTINGS,false);
				assert(hWL);
				WindowList_Remove(hWL,hWnd);
				Utils_SaveWindowPosition(hWnd,pParam->m_hContact,QUOTES_MODULE_NAME,WINDOW_PREFIX_SETTINGS);
				delete pParam;
			}
			break;
		}

		return FALSE;
	}
}


void ShowSettingsDlg(MCONTACT hContact)
{
	HANDLE hWL = CModuleInfo::GetInstance().GetWindowList(WINDOW_PREFIX_SETTINGS,true);
	assert(hWL);
	HWND hWnd = WindowList_Find(hWL,hContact);
	if(NULL != hWnd)
	{
		SetForegroundWindow(hWnd);
		SetFocus(hWnd);
	}
	else
	{
		CreateDialogParam(g_hInstance,MAKEINTRESOURCE(IDD_CONTACT_SETTINGS),NULL,EditSettingsPerContactDlgProc, LPARAM(hContact));
	}
}

//////////////////////////////////////////////////////////////////////////

namespace
{
	INT_PTR CALLBACK EditSettingsPerProviderDlgProc(HWND hWnd,UINT msg,WPARAM wp,LPARAM lp)
	{
		switch(msg)
		{
		case WM_INITDIALOG:
			{
				TranslateDialogDefault(hWnd);
				CAdvProviderSettings* pAdvSettings = reinterpret_cast<CAdvProviderSettings*>(lp);

				::SetDlgItemText(hWnd,IDC_EDIT_NAME,pAdvSettings->GetProviderPtr()->GetInfo().m_sName.c_str());

				// log to history
				WORD dwLogMode = pAdvSettings->GetLogMode();
				UINT nCheck = (dwLogMode&lmInternalHistory) ? 1 : 0;
				::CheckDlgButton(hWnd,IDC_CHECK_INTERNAL_HISTORY,nCheck);
				::SetDlgItemText(hWnd,IDC_EDIT_HISTORY_FORMAT,pAdvSettings->GetHistoryFormat().c_str());
				::CheckDlgButton(hWnd,IDC_CHECK_HISTORY_CONDITION,(true == pAdvSettings->GetHistoryOnlyChangedFlag()) ? 1 : 0);

				// log to file
				nCheck = (dwLogMode&lmExternalFile) ? 1 : 0;
				::CheckDlgButton(hWnd,IDC_CHECK_EXTERNAL_FILE,nCheck);
				::SetDlgItemText(hWnd,IDC_EDIT_FILE_NAME,pAdvSettings->GetLogFileName().c_str());
				::SetDlgItemText(hWnd,IDC_EDIT_LOG_FILE_FORMAT,pAdvSettings->GetLogFormat().c_str());
				::CheckDlgButton(hWnd,IDC_CHECK_LOG_FILE_CONDITION,(true == pAdvSettings->GetLogOnlyChangedFlag()) ? 1 : 0);

				update_file_controls(hWnd);
				update_history_controls(hWnd);

				// popup
				nCheck = (dwLogMode&lmPopup) ? 1 : 0;
				::CheckDlgButton(hWnd,IDC_CHECK_SHOW_POPUP,nCheck);
				::SetDlgItemText(hWnd,IDC_EDIT_POPUP_FORMAT,pAdvSettings->GetPopupFormat().c_str());
				::CheckDlgButton(hWnd,IDC_CHECK_SHOW_POPUP_ONLY_VALUE_CHANGED,(true == pAdvSettings->GetShowPopupIfValueChangedFlag()) ? 1 : 0);

				if(true == enable_popup_controls(hWnd))
				{
					update_popup_controls(hWnd);
				}

				::SetWindowLongPtr(hWnd,GWLP_USERDATA,reinterpret_cast<LONG_PTR>(pAdvSettings));
			}
			return TRUE;
		case WM_COMMAND:
			switch(LOWORD(wp))
			{
			case IDOK:
				{
					WORD nLogMode = lmDisabled;
					UINT nCheck = ::IsDlgButtonChecked(hWnd,IDC_CHECK_EXTERNAL_FILE);
					if(1 == nCheck)
					{
						nLogMode |= lmExternalFile;
					}

					nCheck = ::IsDlgButtonChecked(hWnd,IDC_CHECK_INTERNAL_HISTORY);
					if(1 == nCheck)
					{
						nLogMode |= lmInternalHistory;
					}

					nCheck = ::IsDlgButtonChecked(hWnd,IDC_CHECK_SHOW_POPUP);
					if(1 == nCheck)
					{
						nLogMode |= lmPopup;
					}

					bool bOk = true;
					HWND hwndLogFile = ::GetDlgItem(hWnd,IDC_EDIT_FILE_NAME);
					HWND hwndLogFileFrmt = ::GetDlgItem(hWnd,IDC_EDIT_LOG_FILE_FORMAT);
					
					tstring sLogFile = get_window_text(hwndLogFile);
					tstring sLogFileFormat = get_window_text(hwndLogFileFrmt);
					
					if ((nLogMode&lmExternalFile))
					{
						if(true == sLogFile.empty())
						{
							prepare_edit_ctrl_for_error(hwndLogFile);
							Quotes_MessageBox(hWnd,TranslateT("Enter log file name."),MB_OK|MB_ICONERROR);
							bOk = false;
						}
						else if(true == sLogFileFormat.empty())
						{
							prepare_edit_ctrl_for_error(hwndLogFileFrmt);
							Quotes_MessageBox(hWnd,TranslateT("Enter log file format."),MB_OK|MB_ICONERROR);
							bOk = false;
						}
					}

					HWND hwndHistoryFrmt = ::GetDlgItem(hWnd,IDC_EDIT_HISTORY_FORMAT);
					tstring sHistoryFormat = get_window_text(hwndHistoryFrmt);
					if ((true == bOk) && (nLogMode&lmInternalHistory) && (true == sHistoryFormat.empty()))
					{
						prepare_edit_ctrl_for_error(hwndHistoryFrmt);
						Quotes_MessageBox(hWnd,TranslateT("Enter history format."),MB_OK|MB_ICONERROR);
						bOk = false;
					}

					HWND hwndPopupFrmt = ::GetDlgItem(hWnd,IDC_EDIT_POPUP_FORMAT);
					tstring sPopupFormat = get_window_text(hwndPopupFrmt);
					if ((true == bOk) && (nLogMode&lmPopup) && (true == sPopupFormat.empty()))
					{
						prepare_edit_ctrl_for_error(hwndPopupFrmt);
						Quotes_MessageBox(hWnd,TranslateT("Enter popup window format."),MB_OK|MB_ICONERROR);
						bOk = false;
					}

					if(true == bOk)
					{
						CAdvProviderSettings* pAdvSettings = reinterpret_cast<CAdvProviderSettings*>(GetWindowLongPtr(hWnd,GWLP_USERDATA));

						pAdvSettings->SetLogMode(nLogMode);
						pAdvSettings->SetHistoryOnlyChangedFlag(1 == IsDlgButtonChecked(hWnd,IDC_CHECK_HISTORY_CONDITION));
						pAdvSettings->SetLogOnlyChangedFlag(1 == IsDlgButtonChecked(hWnd,IDC_CHECK_LOG_FILE_CONDITION));
						pAdvSettings->SetShowPopupIfValueChangedFlag(1 == IsDlgButtonChecked(hWnd,IDC_CHECK_SHOW_POPUP_ONLY_VALUE_CHANGED));
						pAdvSettings->SetLogFileName(sLogFile);
						pAdvSettings->SetLogFormat(sLogFileFormat);
						pAdvSettings->SetHistoryFormat(sHistoryFormat);
						pAdvSettings->SetPopupFormat(sPopupFormat);

						::EndDialog(hWnd,IDOK);
					}
				}
				break;
			case IDCANCEL:
				::EndDialog(hWnd,IDCANCEL);
				break;
			case IDC_BUTTON_HISTORY_DESCRIPTION:
			case IDC_BUTTON_LOG_FILE_DESCRIPTION:
			case IDC_BUTTON_POPUP_FORMAT_DESCRIPTION:
				if(BN_CLICKED == HIWORD(wp))
				{
					const CAdvProviderSettings* pAdvSettings = reinterpret_cast<CAdvProviderSettings*>(GetWindowLongPtr(hWnd,GWLP_USERDATA));
					show_variable_list(hWnd,pAdvSettings->GetProviderPtr());
				}
				break;
			case IDC_CHECK_EXTERNAL_FILE:
				if(BN_CLICKED == HIWORD(wp))
				{
					update_file_controls(hWnd);
				}
				break;
			case IDC_CHECK_INTERNAL_HISTORY:
				if(BN_CLICKED == HIWORD(wp))
				{
					update_history_controls(hWnd);
				}
				break;
			case IDC_CHECK_SHOW_POPUP:
				if(BN_CLICKED == HIWORD(wp))
				{
					update_popup_controls(hWnd);
				}
				break;
			case IDC_BUTTON_BROWSE:
				if(BN_CLICKED == HIWORD(wp))
				{
					select_log_file(hWnd);
				}
				break;
			case IDC_BUTTON_POPUP_SETTINGS:
				{
					const CAdvProviderSettings* pAdvSettings = reinterpret_cast<CAdvProviderSettings*>(GetWindowLongPtr(hWnd,GWLP_USERDATA));
					DialogBoxParam(g_hInstance,
						MAKEINTRESOURCE(IDD_DIALOG_POPUP),
						hWnd,
						EditPopupSettingsDlgProc,reinterpret_cast<LPARAM>(pAdvSettings->GetPopupSettingsPtr()));

				}
				break;
			}
			break;
		}
		return FALSE;
	}
}

CAdvProviderSettings::CAdvProviderSettings(const IQuotesProvider* pQuotesProvider)
					 : m_pQuotesProvider(pQuotesProvider),
					   m_wLogMode(lmDisabled),
					   m_bIsOnlyChangedHistory(false),
					   m_bIsOnlyChangedLogFile(false),
					   m_bShowPopupIfValueChanged(false),
					   m_pPopupSettings(nullptr)
{
	assert(m_pQuotesProvider);

	CQuotesProviderVisitorDbSettings visitor;
	m_pQuotesProvider->Accept(visitor);

	assert(visitor.m_pszDefLogFileFormat);
	assert(visitor.m_pszDefHistoryFormat);
	assert(visitor.m_pszDbLogMode);
	assert(visitor.m_pszDbHistoryFormat);
	assert(visitor.m_pszDbHistoryCondition);
	assert(visitor.m_pszDbLogFile);
	assert(visitor.m_pszDbLogFormat);
	assert(visitor.m_pszDbLogCondition);

	m_wLogMode = db_get_w(NULL,QUOTES_PROTOCOL_NAME,visitor.m_pszDbLogMode,static_cast<WORD>(lmDisabled));
	m_sFormatHistory = Quotes_DBGetStringT(NULL,QUOTES_PROTOCOL_NAME,visitor.m_pszDbHistoryFormat,visitor.m_pszDefHistoryFormat);
	m_bIsOnlyChangedHistory = 1 == db_get_b(NULL,QUOTES_PROTOCOL_NAME,visitor.m_pszDbHistoryCondition,0);

	m_sLogFileName = Quotes_DBGetStringT(NULL,QUOTES_PROTOCOL_NAME,visitor.m_pszDbLogFile);
	if(true == m_sLogFileName.empty())
	{
		m_sLogFileName = g_pszVariableUserProfile;
		m_sLogFileName += _T("\\Quotes\\");
		m_sLogFileName += g_pszVariableQuoteName;
		m_sLogFileName += _T(".log");
	}

	m_sFormatLogFile = Quotes_DBGetStringT(NULL,QUOTES_PROTOCOL_NAME,visitor.m_pszDbLogFormat,visitor.m_pszDefLogFileFormat);
	m_bIsOnlyChangedLogFile = (1 == db_get_b(NULL,QUOTES_PROTOCOL_NAME,visitor.m_pszDbLogCondition,0));

	m_sPopupFormat = Quotes_DBGetStringT(NULL,QUOTES_PROTOCOL_NAME,visitor.m_pszDbPopupFormat,visitor.m_pszDefPopupFormat);
	m_bShowPopupIfValueChanged = (1 == db_get_b(NULL,QUOTES_PROTOCOL_NAME,visitor.m_pszDbPopupCondition,0));
}

CAdvProviderSettings::~CAdvProviderSettings()
{
	delete m_pPopupSettings;
}

const IQuotesProvider* CAdvProviderSettings::GetProviderPtr()const
{
	return m_pQuotesProvider;
}

void CAdvProviderSettings::SaveToDb()const
{
	CQuotesProviderVisitorDbSettings visitor;
	m_pQuotesProvider->Accept(visitor);

	assert(visitor.m_pszDbLogMode);
	assert(visitor.m_pszDbHistoryFormat);
	assert(visitor.m_pszDbHistoryCondition);
	assert(visitor.m_pszDbLogFile);
	assert(visitor.m_pszDbLogFormat);
	assert(visitor.m_pszDbLogCondition);
	assert(visitor.m_pszDbPopupColourMode);
	assert(visitor.m_pszDbPopupBkColour);
	assert(visitor.m_pszDbPopupTextColour);
	assert(visitor.m_pszDbPopupDelayMode);
	assert(visitor.m_pszDbPopupDelayTimeout);
	assert(visitor.m_pszDbPopupHistoryFlag);

	db_set_w(NULL,QUOTES_PROTOCOL_NAME,visitor.m_pszDbLogMode,m_wLogMode);
	db_set_ts(NULL,QUOTES_PROTOCOL_NAME,visitor.m_pszDbHistoryFormat,m_sFormatHistory.c_str());
	db_set_b(NULL,QUOTES_PROTOCOL_NAME,visitor.m_pszDbHistoryCondition,m_bIsOnlyChangedHistory);
	db_set_ts(NULL,QUOTES_PROTOCOL_NAME,visitor.m_pszDbLogFile,m_sLogFileName.c_str());
	db_set_ts(NULL,QUOTES_PROTOCOL_NAME,visitor.m_pszDbLogFormat,m_sFormatLogFile.c_str());
	db_set_b(NULL,QUOTES_PROTOCOL_NAME,visitor.m_pszDbLogCondition,m_bIsOnlyChangedLogFile);
	db_set_ts(NULL,QUOTES_PROTOCOL_NAME,visitor.m_pszDbPopupFormat,m_sPopupFormat.c_str());
	db_set_b(NULL,QUOTES_PROTOCOL_NAME,visitor.m_pszDbPopupCondition,m_bShowPopupIfValueChanged);

	if(nullptr != m_pPopupSettings)
	{
		db_set_b(NULL,QUOTES_PROTOCOL_NAME,visitor.m_pszDbPopupColourMode,static_cast<BYTE>(m_pPopupSettings->GetColourMode()));
		db_set_dw(NULL,QUOTES_PROTOCOL_NAME,visitor.m_pszDbPopupBkColour,m_pPopupSettings->GetColourBk());
		db_set_dw(NULL,QUOTES_PROTOCOL_NAME,visitor.m_pszDbPopupTextColour,m_pPopupSettings->GetColourText());
		db_set_b(NULL,QUOTES_PROTOCOL_NAME,visitor.m_pszDbPopupDelayMode,static_cast<BYTE>(m_pPopupSettings->GetDelayMode()));
		db_set_w(NULL,QUOTES_PROTOCOL_NAME,visitor.m_pszDbPopupDelayTimeout,m_pPopupSettings->GetDelayTimeout());
		db_set_b(NULL,QUOTES_PROTOCOL_NAME,visitor.m_pszDbPopupHistoryFlag,m_pPopupSettings->GetHistoryFlag());
	}
}

WORD CAdvProviderSettings::GetLogMode()const
{
	return m_wLogMode;
}

void CAdvProviderSettings::SetLogMode(WORD wMode)
{
	m_wLogMode = wMode;
}

tstring CAdvProviderSettings::GetHistoryFormat()const
{
	return m_sFormatHistory;
}

void CAdvProviderSettings::SetHistoryFormat(const tstring& rsFormat)
{
	m_sFormatHistory = rsFormat;
}

bool CAdvProviderSettings::GetHistoryOnlyChangedFlag()const
{
	return m_bIsOnlyChangedHistory;
}

void CAdvProviderSettings::SetHistoryOnlyChangedFlag(bool bMode)
{
	m_bIsOnlyChangedHistory = bMode;
}

tstring CAdvProviderSettings::GetLogFileName()const
{
	return m_sLogFileName;
}

void CAdvProviderSettings::SetLogFileName(const tstring& rsFile)
{
	m_sLogFileName = rsFile;
}

tstring CAdvProviderSettings::GetLogFormat()const
{
	return m_sFormatLogFile;
}

void CAdvProviderSettings::SetLogFormat(const tstring& rsFormat)
{
	m_sFormatLogFile = rsFormat;
}

bool CAdvProviderSettings::GetLogOnlyChangedFlag()const
{
	return m_bIsOnlyChangedLogFile;
}

void CAdvProviderSettings::SetLogOnlyChangedFlag(bool bMode)
{
	m_bIsOnlyChangedLogFile = bMode;
}

const tstring& CAdvProviderSettings::GetPopupFormat() const
{
	return m_sPopupFormat; 
}

void CAdvProviderSettings::SetPopupFormat(const tstring& val)
{
	m_sPopupFormat = val; 
}

bool CAdvProviderSettings::GetShowPopupIfValueChangedFlag() const
{
	return m_bShowPopupIfValueChanged; 
}

void CAdvProviderSettings::SetShowPopupIfValueChangedFlag(bool val)
{
	m_bShowPopupIfValueChanged = val; 
}

CPopupSettings* CAdvProviderSettings::GetPopupSettingsPtr()const
{
	if(nullptr == m_pPopupSettings)
	{
		m_pPopupSettings = new CPopupSettings(m_pQuotesProvider);
	}

	return m_pPopupSettings;
}

CPopupSettings::CPopupSettings(const IQuotesProvider* pQuotesProvider)
				: m_modeColour(colourDefault),
				 m_modeDelay(delayFromPopup),
				 m_rgbBkg(GetDefColourBk()),
				 m_rgbText(GetDefColourText()),
				 m_wDelay(3),
				 m_bUseHistory(false)

{
	CQuotesProviderVisitorDbSettings visitor;
	pQuotesProvider->Accept(visitor);

	assert(visitor.m_pszDbPopupColourMode);
	assert(visitor.m_pszDbPopupBkColour);
	assert(visitor.m_pszDbPopupTextColour);
	assert(visitor.m_pszDbPopupDelayMode);
	assert(visitor.m_pszDbPopupDelayTimeout);
	assert(visitor.m_pszDbPopupHistoryFlag);

	BYTE m = db_get_b(NULL,QUOTES_PROTOCOL_NAME,visitor.m_pszDbPopupColourMode,static_cast<BYTE>(m_modeColour));
	if(m >= colourDefault && m <= colourUserDefined)
	{
		m_modeColour = static_cast<EColourMode>(m);
	}

	m_rgbBkg = db_get_dw(NULL,QUOTES_PROTOCOL_NAME,visitor.m_pszDbPopupBkColour,m_rgbBkg);
	m_rgbText = db_get_dw(NULL,QUOTES_PROTOCOL_NAME,visitor.m_pszDbPopupTextColour,m_rgbText);

	m = db_get_b(NULL,QUOTES_PROTOCOL_NAME,visitor.m_pszDbPopupDelayMode,static_cast<BYTE>(m_modeDelay));
	if(m >= delayFromPopup && m <= delayPermanent)
	{
		m_modeDelay = static_cast<EDelayMode>(m);
	}
	m_wDelay = db_get_w(NULL,QUOTES_PROTOCOL_NAME,visitor.m_pszDbPopupDelayTimeout,m_wDelay);
	m_bUseHistory = (1 == db_get_b(NULL,QUOTES_PROTOCOL_NAME,visitor.m_pszDbPopupHistoryFlag,m_bUseHistory));
}

/*static */
COLORREF CPopupSettings::GetDefColourBk()
{
	return ::GetSysColor(COLOR_BTNFACE);
}

/*static */
COLORREF CPopupSettings::GetDefColourText()
{
	return ::GetSysColor(COLOR_BTNTEXT);
}

void CPopupSettings::InitForContact(MCONTACT hContact)
{
	BYTE m = db_get_b(hContact,QUOTES_PROTOCOL_NAME,DB_STR_QUOTE_POPUP_COLOUR_MODE,static_cast<BYTE>(m_modeColour));
	if(m >= CPopupSettings::colourDefault && m <= CPopupSettings::colourUserDefined)
	{
		m_modeColour = static_cast<CPopupSettings::EColourMode>(m);
	}

	m_rgbBkg = db_get_dw(hContact,QUOTES_PROTOCOL_NAME,DB_STR_QUOTE_POPUP_COLOUR_BK,m_rgbBkg);
	m_rgbText = db_get_dw(hContact,QUOTES_PROTOCOL_NAME,DB_STR_QUOTE_POPUP_COLOUR_TEXT,m_rgbText);

	m = db_get_b(hContact,QUOTES_PROTOCOL_NAME,DB_STR_QUOTE_POPUP_DELAY_MODE,static_cast<BYTE>(m_modeDelay));
	if(m >= CPopupSettings::delayFromPopup && m <= CPopupSettings::delayPermanent)
	{
		m_modeDelay = static_cast<CPopupSettings::EDelayMode>(m);
	}
	m_wDelay = db_get_w(hContact,QUOTES_PROTOCOL_NAME,DB_STR_QUOTE_POPUP_DELAY_TIMEOUT,m_wDelay);
	m_bUseHistory = 1 == db_get_b(hContact,QUOTES_PROTOCOL_NAME,DB_STR_QUOTE_POPUP_HISTORY_FLAG,m_bUseHistory);
}

void CPopupSettings::SaveForContact(MCONTACT hContact)const
{
	db_set_b(hContact,QUOTES_PROTOCOL_NAME,DB_STR_QUOTE_POPUP_COLOUR_MODE,static_cast<BYTE>(m_modeColour));
	db_set_dw(hContact,QUOTES_PROTOCOL_NAME,DB_STR_QUOTE_POPUP_COLOUR_BK,m_rgbBkg);
	db_set_dw(hContact,QUOTES_PROTOCOL_NAME,DB_STR_QUOTE_POPUP_COLOUR_TEXT,m_rgbText);
	db_set_b(hContact,QUOTES_PROTOCOL_NAME,DB_STR_QUOTE_POPUP_DELAY_MODE,static_cast<BYTE>(m_modeDelay));
	db_set_w(hContact,QUOTES_PROTOCOL_NAME,DB_STR_QUOTE_POPUP_DELAY_TIMEOUT,m_wDelay);
	db_set_b(hContact,QUOTES_PROTOCOL_NAME,DB_STR_QUOTE_POPUP_HISTORY_FLAG,m_bUseHistory);
}

CPopupSettings::EColourMode CPopupSettings::GetColourMode()const
{
	return m_modeColour;
}

void CPopupSettings::SetColourMode(EColourMode nMode)
{
	m_modeColour = nMode;
}

COLORREF CPopupSettings::GetColourBk()const
{
	return m_rgbBkg;
}

void CPopupSettings::SetColourBk(COLORREF rgb)
{
	m_rgbBkg = rgb;
}

COLORREF CPopupSettings::GetColourText()const
{
	return m_rgbText;
}

void CPopupSettings::SetColourText(COLORREF rgb)
{
	m_rgbText = rgb;
}

CPopupSettings::EDelayMode CPopupSettings::GetDelayMode()const
{
	return m_modeDelay;
}

void CPopupSettings::SetDelayMode(EDelayMode nMode)
{
	m_modeDelay = nMode;
}

WORD CPopupSettings::GetDelayTimeout()const
{
	return m_wDelay;
}

void CPopupSettings::SetDelayTimeout(WORD delay)
{
	m_wDelay = delay;
}

bool CPopupSettings::GetHistoryFlag()const
{
	return m_bUseHistory;
}

void CPopupSettings::SetHistoryFlag(bool flag)
{
	m_bUseHistory = flag;
}

bool ShowSettingsDlg(HWND hWndParent,CAdvProviderSettings* pAdvSettings)
{
	assert(pAdvSettings);

	return (IDOK == DialogBoxParam(g_hInstance,
								   MAKEINTRESOURCE(IDD_PROVIDER_ADV_SETTINGS),
								   hWndParent,
								   EditSettingsPerProviderDlgProc,
								   reinterpret_cast<LPARAM>(pAdvSettings)));
}

namespace
{
	void replace_invalid_char(tstring::value_type& rChar,tstring::value_type repl)
	{
		static const TCHAR charInvalidSigns[] = {_T('\\'), _T('/'), _T(':'), _T('*'), _T('?'), _T('\"'), _T('<'), _T('>'), _T('|')};

		for(int i = 0; i < sizeof(charInvalidSigns)/sizeof(charInvalidSigns[0]);++i)
		{
			if(rChar == charInvalidSigns[i])
			{
				rChar = repl;
				break;
			}
		}
	}

}

tstring GenerateLogFileName(const tstring& rsLogFilePattern,
							const tstring& rsQuoteSymbol,
							int nFlags/* = glfnResolveAll*/)
{
	tstring sPath = rsLogFilePattern;
	if(nFlags&glfnResolveQuoteName)
	{
		assert(false == rsQuoteSymbol.empty());

		tstring::size_type n = sPath.find(g_pszVariableQuoteName);
		if(tstring::npos != n)
		{
			tstring s = rsQuoteSymbol;
			std::for_each(s.begin(),s.end(),boost::bind(replace_invalid_char,_1,_T('_')));
			sPath.replace(n,mir_tstrlen(g_pszVariableQuoteName),s.c_str());
		}
	}

	if(nFlags&glfnResolveUserProfile)
	{
		REPLACEVARSDATA dat = {0};
		dat.cbSize = sizeof(dat);
		dat.dwFlags = RVF_TCHAR;

		TCHAR* ptszParsedName = reinterpret_cast<TCHAR*>(CallService(MS_UTILS_REPLACEVARS,
			reinterpret_cast<WPARAM>(sPath.c_str()),reinterpret_cast<LPARAM>(&dat)));
		if(ptszParsedName)
		{
			sPath = ptszParsedName;
			mir_free(ptszParsedName);
		}
	}

	return sPath;
}

tstring GetContactLogFileName(MCONTACT hContact)
{
	tstring result;

	const CQuotesProviders::TQuotesProviderPtr& pProvider = CModuleInfo::GetQuoteProvidersPtr()->GetContactProviderPtr(hContact);
	if(pProvider)
	{
		tstring sPattern;
		bool bUseContactSpecific = (db_get_b(hContact,QUOTES_PROTOCOL_NAME,DB_STR_CONTACT_SPEC_SETTINGS,0) > 0);
		if(bUseContactSpecific)
		{
			sPattern = Quotes_DBGetStringT(hContact,QUOTES_PROTOCOL_NAME,DB_STR_QUOTE_LOG_FILE);
		}
		else
		{
			CAdvProviderSettings global_settings(pProvider.get());
			sPattern = global_settings.GetLogFileName();
		}

		result = GenerateLogFileName(sPattern,Quotes_DBGetStringT(hContact,QUOTES_PROTOCOL_NAME,DB_STR_QUOTE_SYMBOL));
	}

	return result;
}

tstring GetContactName(MCONTACT hContact)
{
	tstring sDescription = Quotes_DBGetStringT(hContact,QUOTES_PROTOCOL_NAME,DB_STR_QUOTE_DESCRIPTION);
	if(sDescription.empty())
	{
		sDescription = Quotes_DBGetStringT(hContact,QUOTES_PROTOCOL_NAME,DB_STR_QUOTE_SYMBOL);
	}
	return sDescription;
}
