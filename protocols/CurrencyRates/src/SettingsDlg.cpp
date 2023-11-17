#include "StdAfx.h"

#define WINDOW_PREFIX_SETTINGS "Edit Settings_"

void update_file_controls(HWND hDlg)
{
	bool bEnable = (1 == ::IsDlgButtonChecked(hDlg, IDC_CHECK_EXTERNAL_FILE));

	::EnableWindow(::GetDlgItem(hDlg, IDC_EDIT_FILE_NAME), bEnable);
	::EnableWindow(::GetDlgItem(hDlg, IDC_STATIC_SELECT_FILE), bEnable);
	::EnableWindow(::GetDlgItem(hDlg, IDC_BUTTON_BROWSE), bEnable);
	::EnableWindow(::GetDlgItem(hDlg, IDC_STATIC_LOG_FILE_FORMAT), bEnable);
	::EnableWindow(::GetDlgItem(hDlg, IDC_EDIT_LOG_FILE_FORMAT), bEnable);
	::EnableWindow(::GetDlgItem(hDlg, IDC_BUTTON_LOG_FILE_DESCRIPTION), bEnable);
	::EnableWindow(::GetDlgItem(hDlg, IDC_CHECK_LOG_FILE_CONDITION), bEnable);
}

void update_history_controls(HWND hDlg)
{
	bool bEnable = (1 == ::IsDlgButtonChecked(hDlg, IDC_CHECK_INTERNAL_HISTORY));

	::EnableWindow(::GetDlgItem(hDlg, IDC_STATIC_HISTORY_FORMAT), bEnable);
	::EnableWindow(::GetDlgItem(hDlg, IDC_EDIT_HISTORY_FORMAT), bEnable);
	::EnableWindow(::GetDlgItem(hDlg, IDC_BUTTON_HISTORY_DESCRIPTION), bEnable);
	::EnableWindow(::GetDlgItem(hDlg, IDC_CHECK_HISTORY_CONDITION), bEnable);
}

void update_all_controls(HWND hDlg)
{
	bool bIsCheckedContactSpec = (1 == ::IsDlgButtonChecked(hDlg, IDC_CHECK_CONTACT_SPECIFIC));
	bool bIsCheckedExternal = (1 == ::IsDlgButtonChecked(hDlg, IDC_CHECK_EXTERNAL_FILE));

	::EnableWindow(::GetDlgItem(hDlg, IDC_CHECK_EXTERNAL_FILE), bIsCheckedContactSpec);
	::EnableWindow(::GetDlgItem(hDlg, IDC_EDIT_FILE_NAME), (bIsCheckedContactSpec && bIsCheckedExternal));
	::EnableWindow(::GetDlgItem(hDlg, IDC_STATIC_SELECT_FILE), (bIsCheckedContactSpec && bIsCheckedExternal));
	::EnableWindow(::GetDlgItem(hDlg, IDC_BUTTON_BROWSE), (bIsCheckedContactSpec && bIsCheckedExternal));
	::EnableWindow(::GetDlgItem(hDlg, IDC_STATIC_LOG_FILE_FORMAT), (bIsCheckedContactSpec && bIsCheckedExternal));
	::EnableWindow(::GetDlgItem(hDlg, IDC_EDIT_LOG_FILE_FORMAT), (bIsCheckedContactSpec && bIsCheckedExternal));
	::EnableWindow(::GetDlgItem(hDlg, IDC_BUTTON_LOG_FILE_DESCRIPTION), (bIsCheckedContactSpec && bIsCheckedExternal));
	::EnableWindow(::GetDlgItem(hDlg, IDC_CHECK_LOG_FILE_CONDITION), (bIsCheckedContactSpec && bIsCheckedExternal));

	bool bIsCheckedHistory = (1 == ::IsDlgButtonChecked(hDlg, IDC_CHECK_INTERNAL_HISTORY));
	::EnableWindow(::GetDlgItem(hDlg, IDC_CHECK_INTERNAL_HISTORY), bIsCheckedContactSpec);
	::EnableWindow(::GetDlgItem(hDlg, IDC_STATIC_HISTORY_FORMAT), (bIsCheckedContactSpec && bIsCheckedHistory));
	::EnableWindow(::GetDlgItem(hDlg, IDC_EDIT_HISTORY_FORMAT), (bIsCheckedContactSpec && bIsCheckedHistory));
	::EnableWindow(::GetDlgItem(hDlg, IDC_BUTTON_HISTORY_DESCRIPTION), (bIsCheckedContactSpec && bIsCheckedHistory));
	::EnableWindow(::GetDlgItem(hDlg, IDC_CHECK_HISTORY_CONDITION), (bIsCheckedContactSpec && bIsCheckedHistory));

	bool bIsCheckedShowPopup = (1 == ::IsDlgButtonChecked(hDlg, IDC_CHECK_SHOW_POPUP));
	::EnableWindow(::GetDlgItem(hDlg, IDC_CHECK_SHOW_POPUP), (bIsCheckedContactSpec));
	::EnableWindow(::GetDlgItem(hDlg, IDC_EDIT_POPUP_FORMAT), (bIsCheckedContactSpec && bIsCheckedShowPopup));
	::EnableWindow(::GetDlgItem(hDlg, IDC_CHECK_SHOW_POPUP_ONLY_VALUE_CHANGED), (bIsCheckedContactSpec && bIsCheckedShowPopup));
	::EnableWindow(::GetDlgItem(hDlg, IDC_STATIC_POPUP_FORMAT), (bIsCheckedContactSpec && bIsCheckedShowPopup));
	::EnableWindow(::GetDlgItem(hDlg, IDC_BUTTON_POPUP_FORMAT_DESCRIPTION), (bIsCheckedContactSpec && bIsCheckedShowPopup));
}

static LPCTSTR pszFilterParts[] = { LPGENW("Log Files (*.txt,*.log)"), L"*.txt;*.log", LPGENW("All files (*.*)"), L"*.*" };

void select_log_file(HWND hDlg)
{
	wchar_t pszFile[MAX_PATH];

	CMStringW pszFilter;
	for (auto &it : pszFilterParts) {
		pszFilter += it;
		pszFilter.AppendChar(0);
	}
	pszFilter.AppendChar(0);

	OPENFILENAME ofn = { 0 };
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hDlg;
	ofn.lpstrFile = pszFile;
	ofn.nMaxFile = _countof(pszFile);
	ofn.lpstrFilter = pszFilter;
	ofn.nFilterIndex = 1;
	ofn.hInstance = g_plugin.getInst();
	ofn.lpstrDefExt = L"log";
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_EXPLORER;

	if (GetOpenFileName(&ofn))
		SetDlgItemText(hDlg, IDC_EDIT_FILE_NAME, pszFile);
}

struct CSettingWindowParam
{
	CSettingWindowParam(MCONTACT hContact) : 
		m_hContact(hContact)
	{}

	MCONTACT m_hContact;
};

inline CSettingWindowParam *get_param(HWND hWnd)
{
	return reinterpret_cast<CSettingWindowParam *>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
}

INT_PTR CALLBACK EditSettingsPerContactDlgProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hWnd);
		{
			MCONTACT hContact = MCONTACT(lp);

			MWindowList hWL = CModuleInfo::GetWindowList(WINDOW_PREFIX_SETTINGS, false);
			assert(hWL);
			WindowList_Add(hWL, hWnd, hContact);

			::SetDlgItemText(hWnd, IDC_EDIT_NAME, GetContactName(hContact));

			uint8_t bUseContactSpecific = g_plugin.getByte(hContact, DB_STR_CONTACT_SPEC_SETTINGS, 0);
			::CheckDlgButton(hWnd, IDC_CHECK_CONTACT_SPECIFIC, bUseContactSpecific ? BST_CHECKED : BST_UNCHECKED);

			// log to history
			uint16_t dwLogMode = g_plugin.getWord(hContact, DB_STR_CURRENCYRATE_LOG, g_plugin.wLogMode);
			UINT nCheck = (dwLogMode & lmInternalHistory) ? 1 : 0;
			::CheckDlgButton(hWnd, IDC_CHECK_INTERNAL_HISTORY, nCheck ? BST_CHECKED : BST_UNCHECKED);

			::SetDlgItemText(hWnd, IDC_EDIT_HISTORY_FORMAT, g_plugin.getMStringW(hContact, DB_STR_CURRENCYRATE_FORMAT_HISTORY, g_plugin.wszHistoryFormat));

			uint16_t wOnlyIfChanged = g_plugin.getWord(hContact, DB_STR_CURRENCYRATE_HISTORY_CONDITION, g_plugin.bIsOnlyChangedHistory);
			::CheckDlgButton(hWnd, IDC_CHECK_HISTORY_CONDITION, (1 == wOnlyIfChanged) ? BST_CHECKED : BST_UNCHECKED);

			// log to file
			nCheck = (dwLogMode & lmExternalFile) ? 1 : 0;
			::CheckDlgButton(hWnd, IDC_CHECK_EXTERNAL_FILE, nCheck ? BST_CHECKED : BST_UNCHECKED);

			CMStringW sLogFileName = g_plugin.getMStringW(hContact, DB_STR_CURRENCYRATE_LOG_FILE);
			if (sLogFileName.IsEmpty())
				sLogFileName = GenerateLogFileName((wchar_t*)g_plugin.wszLogFileName, g_plugin.getMStringW(hContact, DB_STR_CURRENCYRATE_SYMBOL), glfnResolveCurrencyRateName);
			::SetDlgItemText(hWnd, IDC_EDIT_FILE_NAME, sLogFileName);

			::SetDlgItemText(hWnd, IDC_EDIT_LOG_FILE_FORMAT, g_plugin.getMStringW(hContact, DB_STR_CURRENCYRATE_FORMAT_LOG_FILE, g_plugin.wszLogFileFormat));

			wOnlyIfChanged = g_plugin.getWord(hContact, DB_STR_CURRENCYRATE_LOG_FILE_CONDITION, g_plugin.bIsOnlyChangedLogFile);
			::CheckDlgButton(hWnd, IDC_CHECK_LOG_FILE_CONDITION, (1 == wOnlyIfChanged) ? BST_CHECKED : BST_UNCHECKED);

			// popup
			nCheck = (dwLogMode & lmPopup) ? 1 : 0;
			::CheckDlgButton(hWnd, IDC_CHECK_SHOW_POPUP, nCheck ? BST_CHECKED : BST_UNCHECKED);

			::SetDlgItemText(hWnd, IDC_EDIT_POPUP_FORMAT, g_plugin.getMStringW(hContact, DB_STR_CURRENCYRATE_FORMAT_POPUP, g_plugin.wszPopupFormat));

			bool bOnlyIfChanged = 1 == g_plugin.getByte(hContact, DB_STR_CURRENCYRATE_POPUP_CONDITION, g_plugin.bShowPopupIfValueChanged);
			::CheckDlgButton(hWnd, IDC_CHECK_SHOW_POPUP_ONLY_VALUE_CHANGED, (bOnlyIfChanged) ? BST_CHECKED : BST_UNCHECKED);

			update_all_controls(hWnd);

			CSettingWindowParam *pParam = new CSettingWindowParam(hContact);
			::SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pParam));
			Utils_RestoreWindowPositionNoSize(hWnd, hContact, MODULENAME, WINDOW_PREFIX_SETTINGS);
			::ShowWindow(hWnd, SW_SHOW);
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wp)) {
		case IDC_BUTTON_HISTORY_DESCRIPTION:
		case IDC_BUTTON_LOG_FILE_DESCRIPTION:
		case IDC_BUTTON_POPUP_FORMAT_DESCRIPTION:
			if (BN_CLICKED == HIWORD(wp))
				show_variable_list(hWnd, g_pCurrentProvider);
			break;

		case IDC_CHECK_CONTACT_SPECIFIC:
			if (BN_CLICKED == HIWORD(wp))
				update_all_controls(hWnd);
			break;
		case IDC_CHECK_EXTERNAL_FILE:
			if (BN_CLICKED == HIWORD(wp))
				update_file_controls(hWnd);
			break;
		case IDC_CHECK_INTERNAL_HISTORY:
			if (BN_CLICKED == HIWORD(wp))
				update_history_controls(hWnd);
			break;
		case IDC_BUTTON_BROWSE:
			if (BN_CLICKED == HIWORD(wp))
				select_log_file(hWnd);
			break;

		case IDOK:
			{
				CSettingWindowParam *pParam = get_param(hWnd);
				MCONTACT hContact = pParam->m_hContact;

				bool bUseContactSpec = 1 == ::IsDlgButtonChecked(hWnd, IDC_CHECK_CONTACT_SPECIFIC);

				uint16_t nLogMode = lmDisabled;
				UINT nCheck = ::IsDlgButtonChecked(hWnd, IDC_CHECK_EXTERNAL_FILE);
				if (1 == nCheck)
					nLogMode |= lmExternalFile;

				nCheck = ::IsDlgButtonChecked(hWnd, IDC_CHECK_INTERNAL_HISTORY);
				if (1 == nCheck)
					nLogMode |= lmInternalHistory;

				nCheck = ::IsDlgButtonChecked(hWnd, IDC_CHECK_SHOW_POPUP);
				if (1 == nCheck)
					nLogMode |= lmPopup;

				bool bOk = true;
				HWND hwndLogFile = ::GetDlgItem(hWnd, IDC_EDIT_FILE_NAME);
				HWND hwndLogFileFrmt = ::GetDlgItem(hWnd, IDC_EDIT_LOG_FILE_FORMAT);
				HWND hwndHistoryFrmt = ::GetDlgItem(hWnd, IDC_EDIT_HISTORY_FORMAT);
				CMStringW sLogFile = get_window_text(hwndLogFile);
				CMStringW sLogFileFormat = get_window_text(hwndLogFileFrmt);
				CMStringW sHistoryFormat = get_window_text(hwndHistoryFrmt);
				if ((nLogMode & lmExternalFile)) {
					if (sLogFile.IsEmpty()) {
						prepare_edit_ctrl_for_error(hwndLogFile);
						CurrencyRates_MessageBox(hWnd, TranslateT("Enter log file name."), MB_OK | MB_ICONERROR);
						bOk = false;
					}
					else if (sLogFileFormat.IsEmpty()) {
						prepare_edit_ctrl_for_error(hwndLogFileFrmt);
						CurrencyRates_MessageBox(hWnd, TranslateT("Enter log file format."), MB_OK | MB_ICONERROR);
						bOk = false;
					}
				}

				if (bOk && (nLogMode & lmInternalHistory) && sHistoryFormat.IsEmpty()) {
					prepare_edit_ctrl_for_error(hwndHistoryFrmt);
					CurrencyRates_MessageBox(hWnd, TranslateT("Enter history format."), MB_OK | MB_ICONERROR);
					bOk = false;
				}

				HWND hwndPopupFrmt = ::GetDlgItem(hWnd, IDC_EDIT_POPUP_FORMAT);
				CMStringW sPopupFormat = get_window_text(hwndPopupFrmt);
				if (bOk && (nLogMode & lmPopup) && sPopupFormat.IsEmpty()) {
					prepare_edit_ctrl_for_error(hwndPopupFrmt);
					CurrencyRates_MessageBox(hWnd, TranslateT("Enter popup window format."), MB_OK | MB_ICONERROR);
					bOk = false;
				}

				if (true == bOk) {
					UINT nIfChangedHistory = IsDlgButtonChecked(hWnd, IDC_CHECK_HISTORY_CONDITION);
					UINT nIfChangedFile = IsDlgButtonChecked(hWnd, IDC_CHECK_LOG_FILE_CONDITION);
					bool bIfChangedPopup = (1 == IsDlgButtonChecked(hWnd, IDC_CHECK_SHOW_POPUP_ONLY_VALUE_CHANGED));

					g_plugin.setByte(hContact, DB_STR_CONTACT_SPEC_SETTINGS, bUseContactSpec);
					g_plugin.setWord(hContact, DB_STR_CURRENCYRATE_LOG, nLogMode);
					g_plugin.setWord(hContact, DB_STR_CURRENCYRATE_LOG_FILE_CONDITION, nIfChangedFile);
					g_plugin.setWord(hContact, DB_STR_CURRENCYRATE_HISTORY_CONDITION, nIfChangedHistory);
					g_plugin.setByte(hContact, DB_STR_CURRENCYRATE_POPUP_CONDITION, bIfChangedPopup);
					g_plugin.setWString(hContact, DB_STR_CURRENCYRATE_LOG_FILE, sLogFile.c_str());
					g_plugin.setWString(hContact, DB_STR_CURRENCYRATE_FORMAT_LOG_FILE, sLogFileFormat.c_str());
					g_plugin.setWString(hContact, DB_STR_CURRENCYRATE_FORMAT_HISTORY, sHistoryFormat.c_str());
					g_plugin.setWString(hContact, DB_STR_CURRENCYRATE_FORMAT_POPUP, sPopupFormat.c_str());
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
		CSettingWindowParam *pParam = get_param(hWnd);
		SetWindowLongPtr(hWnd, GWLP_USERDATA, 0);

		MWindowList hWL = CModuleInfo::GetWindowList(WINDOW_PREFIX_SETTINGS, false);
		assert(hWL);
		WindowList_Remove(hWL, hWnd);
		Utils_SaveWindowPosition(hWnd, pParam->m_hContact, MODULENAME, WINDOW_PREFIX_SETTINGS);
		delete pParam;
		break;
	}

	return FALSE;
}

void ShowSettingsDlg(MCONTACT hContact)
{
	MWindowList hWL = CModuleInfo::GetWindowList(WINDOW_PREFIX_SETTINGS, true);
	assert(hWL);
	HWND hWnd = WindowList_Find(hWL, hContact);
	if (nullptr != hWnd) {
		SetForegroundWindow(hWnd);
		SetFocus(hWnd);
	}
	else CreateDialogParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_CONTACT_SETTINGS), nullptr, EditSettingsPerContactDlgProc, LPARAM(hContact));
}

CMStringW GenerateLogFileName(const CMStringW &rsLogFilePattern, const CMStringW &rsCurrencyRateSymbol, int nFlags)
{
	CMStringW sPath = rsLogFilePattern;
	if (nFlags & glfnResolveCurrencyRateName) {
		assert(false == rsCurrencyRateSymbol.IsEmpty());

		CMStringW s = rsCurrencyRateSymbol;
		FixInvalidChars(s);
		sPath.Replace(L"%currencyratename%", s);
	}

	if (nFlags & glfnResolveUserProfile) {
		wchar_t *ptszParsedName = Utils_ReplaceVarsW(sPath.c_str());
		if (ptszParsedName) {
			sPath = ptszParsedName;
			mir_free(ptszParsedName);
		}
	}

	return sPath;
}

CMStringW GetContactLogFileName(MCONTACT hContact)
{
	CMStringW result;

	if (g_pCurrentProvider) {
		CMStringW sPattern;
		bool bUseContactSpecific = (g_plugin.getByte(hContact, DB_STR_CONTACT_SPEC_SETTINGS, 0) > 0);
		if (bUseContactSpecific)
			sPattern = g_plugin.getMStringW(hContact, DB_STR_CURRENCYRATE_LOG_FILE);
		else {
			sPattern = g_plugin.wszLogFileName;
		}

		result = GenerateLogFileName(sPattern, g_plugin.getMStringW(hContact, DB_STR_CURRENCYRATE_SYMBOL));
	}

	return result;
}

CMStringW GetContactName(MCONTACT hContact)
{
	CMStringW sDescription = g_plugin.getMStringW(hContact, DB_STR_CURRENCYRATE_DESCRIPTION);
	if (sDescription.IsEmpty())
		sDescription = g_plugin.getMStringW(hContact, DB_STR_CURRENCYRATE_SYMBOL);

	return sDescription;
}
