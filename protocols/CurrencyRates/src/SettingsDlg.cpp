#include "StdAfx.h"

#define WINDOW_PREFIX_SETTINGS "Edit Settings_"

const wchar_t g_pszVariableCurrencyRateName[] = L"%currencyratename%";
const wchar_t g_pszVariableUserProfile[] = L"%miranda_userdata%";

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

void update_popup_controls(HWND hDlg)
{
	bool bEnable = (1 == ::IsDlgButtonChecked(hDlg, IDC_CHECK_SHOW_POPUP));
	::EnableWindow(::GetDlgItem(hDlg, IDC_EDIT_POPUP_FORMAT), bEnable);
	::EnableWindow(::GetDlgItem(hDlg, IDC_CHECK_SHOW_POPUP_ONLY_VALUE_CHANGED), bEnable);
	::EnableWindow(::GetDlgItem(hDlg, IDC_STATIC_POPUP_FORMAT), bEnable);
	::EnableWindow(::GetDlgItem(hDlg, IDC_BUTTON_POPUP_FORMAT_DESCRIPTION), bEnable);
	::EnableWindow(::GetDlgItem(hDlg, IDC_BUTTON_POPUP_SETTINGS), bEnable);
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
	::EnableWindow(::GetDlgItem(hDlg, IDC_BUTTON_POPUP_SETTINGS), (bIsCheckedContactSpec));
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

	BOOL b = GetOpenFileName(&ofn);
	if (TRUE == b) {
		SetDlgItemText(hDlg, IDC_EDIT_FILE_NAME, ofn.lpstrFile);
	}
}

struct CSettingWindowParam
{
	CSettingWindowParam(MCONTACT hContact) : m_hContact(hContact), m_pPopupSettings(nullptr) {}
	~CSettingWindowParam() { delete m_pPopupSettings; }

	MCONTACT m_hContact;
	CPopupSettings *m_pPopupSettings;
};

inline CSettingWindowParam *get_param(HWND hWnd)
{
	return reinterpret_cast<CSettingWindowParam *>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
}

void update_popup_controls_settings(HWND hDlg)
{
	bool bIsColoursEnabled = 1 == IsDlgButtonChecked(hDlg, IDC_RADIO_USER_DEFINED_COLOURS);
	::EnableWindow(::GetDlgItem(hDlg, IDC_BGCOLOR), bIsColoursEnabled);
	::EnableWindow(::GetDlgItem(hDlg, IDC_TEXTCOLOR), bIsColoursEnabled);

	bool bIsDelayEnabled = 1 == IsDlgButtonChecked(hDlg, IDC_DELAYCUSTOM);
	::EnableWindow(::GetDlgItem(hDlg, IDC_DELAY), bIsDelayEnabled);

}

INT_PTR CALLBACK EditPopupSettingsDlgProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg) {
	case WM_INITDIALOG:
		{
			CPopupSettings *pSettings = reinterpret_cast<CPopupSettings *>(lp);
			TranslateDialogDefault(hWnd);
			::SendDlgItemMessage(hWnd, IDC_BGCOLOR, CPM_SETCOLOUR, 0, pSettings->GetColourBk());
			::SendDlgItemMessage(hWnd, IDC_TEXTCOLOR, CPM_SETCOLOUR, 0, pSettings->GetColourText());

			::CheckDlgButton(hWnd, IDC_CHECK_DONT_USE_POPUPHISTORY, pSettings->GetHistoryFlag() ? BST_CHECKED : BST_UNCHECKED);

			::CheckRadioButton(hWnd, IDC_RADIO_DEFAULT_COLOURS, IDC_RADIO_USER_DEFINED_COLOURS, (CPopupSettings::colourDefault == pSettings->GetColourMode()) ? IDC_RADIO_DEFAULT_COLOURS : IDC_RADIO_USER_DEFINED_COLOURS);
			UINT n;
			switch (pSettings->GetDelayMode()) {
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
			::CheckRadioButton(hWnd, IDC_DELAYFROMPU, IDC_DELAYPERMANENT, n);

			::SetDlgItemInt(hWnd, IDC_DELAY, pSettings->GetDelayTimeout(), FALSE);

			update_popup_controls_settings(hWnd);

			::SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pSettings));
		}
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wp)) {
		case IDC_RADIO_DEFAULT_COLOURS:
		case IDC_RADIO_USER_DEFINED_COLOURS:
		case IDC_DELAYFROMPU:
		case IDC_DELAYCUSTOM:
		case IDC_DELAYPERMANENT:
			update_popup_controls_settings(hWnd);
			break;

		case IDCANCEL:
			::EndDialog(hWnd, IDCANCEL);
			break;

		case IDOK:
			{
				CPopupSettings *pSettings = reinterpret_cast<CPopupSettings *>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

				bool bError = false;
				BOOL bOk = FALSE;
				UINT nDelay = ::GetDlgItemInt(hWnd, IDC_DELAY, &bOk, FALSE);
				CPopupSettings::EDelayMode nModeDelay = pSettings->GetDelayMode();
				if (1 == ::IsDlgButtonChecked(hWnd, IDC_DELAYFROMPU))
					nModeDelay = CPopupSettings::delayFromPopup;
				else if (1 == ::IsDlgButtonChecked(hWnd, IDC_DELAYCUSTOM)) {
					if (TRUE == bOk)
						nModeDelay = CPopupSettings::delayCustom;
					else {
						prepare_edit_ctrl_for_error(::GetDlgItem(hWnd, IDC_DELAY));
						CurrencyRates_MessageBox(hWnd, TranslateT("Enter integer value"), MB_OK | MB_ICONERROR);
						bError = true;
					}
				}
				else if (1 == ::IsDlgButtonChecked(hWnd, IDC_DELAYPERMANENT))
					nModeDelay = CPopupSettings::delayPermanent;

				if (false == bError) {
					pSettings->SetDelayMode(nModeDelay);
					if (TRUE == bOk)
						pSettings->SetDelayTimeout(nDelay);

					pSettings->SetHistoryFlag((1 == IsDlgButtonChecked(hWnd, IDC_CHECK_DONT_USE_POPUPHISTORY)));

					if (1 == ::IsDlgButtonChecked(hWnd, IDC_RADIO_DEFAULT_COLOURS))
						pSettings->SetColourMode(CPopupSettings::colourDefault);
					else if (1 == ::IsDlgButtonChecked(hWnd, IDC_RADIO_USER_DEFINED_COLOURS))
						pSettings->SetColourMode(CPopupSettings::colourUserDefined);

					pSettings->SetColourBk(static_cast<COLORREF>(::SendDlgItemMessage(hWnd, IDC_BGCOLOR, CPM_GETCOLOUR, 0, 0)));
					pSettings->SetColourText(static_cast<COLORREF>(::SendDlgItemMessage(hWnd, IDC_TEXTCOLOR, CPM_GETCOLOUR, 0, 0)));

					::EndDialog(hWnd, IDOK);
				}
			}
			break;
		}
		break;
	}

	return FALSE;
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

			BYTE bUseContactSpecific = g_plugin.getByte(hContact, DB_STR_CONTACT_SPEC_SETTINGS, 0);
			::CheckDlgButton(hWnd, IDC_CHECK_CONTACT_SPECIFIC, bUseContactSpecific ? BST_CHECKED : BST_UNCHECKED);

			auto pProvider = GetContactProviderPtr(hContact);
			CAdvProviderSettings setGlobal(pProvider);
			// log to history
			WORD dwLogMode = g_plugin.getWord(hContact, DB_STR_CURRENCYRATE_LOG, setGlobal.GetLogMode());
			UINT nCheck = (dwLogMode & lmInternalHistory) ? 1 : 0;
			::CheckDlgButton(hWnd, IDC_CHECK_INTERNAL_HISTORY, nCheck ? BST_CHECKED : BST_UNCHECKED);

			::SetDlgItemText(hWnd, IDC_EDIT_HISTORY_FORMAT, g_plugin.getMStringW(hContact, DB_STR_CURRENCYRATE_FORMAT_HISTORY, setGlobal.GetHistoryFormat()));

			WORD wOnlyIfChanged = g_plugin.getWord(hContact, DB_STR_CURRENCYRATE_HISTORY_CONDITION, setGlobal.GetHistoryOnlyChangedFlag());
			::CheckDlgButton(hWnd, IDC_CHECK_HISTORY_CONDITION, (1 == wOnlyIfChanged) ? BST_CHECKED : BST_UNCHECKED);

			// log to file
			nCheck = (dwLogMode & lmExternalFile) ? 1 : 0;
			::CheckDlgButton(hWnd, IDC_CHECK_EXTERNAL_FILE, nCheck ? BST_CHECKED : BST_UNCHECKED);

			CMStringW sLogFileName = g_plugin.getMStringW(hContact, DB_STR_CURRENCYRATE_LOG_FILE);
			if (sLogFileName.IsEmpty())
				sLogFileName = GenerateLogFileName(setGlobal.GetLogFileName(), g_plugin.getMStringW(hContact, DB_STR_CURRENCYRATE_SYMBOL), glfnResolveCurrencyRateName);
			::SetDlgItemText(hWnd, IDC_EDIT_FILE_NAME, sLogFileName);

			::SetDlgItemText(hWnd, IDC_EDIT_LOG_FILE_FORMAT, g_plugin.getMStringW(hContact, DB_STR_CURRENCYRATE_FORMAT_LOG_FILE, setGlobal.GetLogFormat()));

			wOnlyIfChanged = g_plugin.getWord(hContact, DB_STR_CURRENCYRATE_LOG_FILE_CONDITION, setGlobal.GetLogOnlyChangedFlag());
			::CheckDlgButton(hWnd, IDC_CHECK_LOG_FILE_CONDITION, (1 == wOnlyIfChanged) ? BST_CHECKED : BST_UNCHECKED);

			// popup
			nCheck = (dwLogMode & lmPopup) ? 1 : 0;
			::CheckDlgButton(hWnd, IDC_CHECK_SHOW_POPUP, nCheck ? BST_CHECKED : BST_UNCHECKED);

			::SetDlgItemText(hWnd, IDC_EDIT_POPUP_FORMAT, g_plugin.getMStringW(hContact, DB_STR_CURRENCYRATE_FORMAT_POPUP, setGlobal.GetPopupFormat().c_str()));

			bool bOnlyIfChanged = 1 == g_plugin.getByte(hContact, DB_STR_CURRENCYRATE_POPUP_CONDITION, setGlobal.GetShowPopupIfValueChangedFlag());
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
				show_variable_list(hWnd, GetContactProviderPtr(get_param(hWnd)->m_hContact));
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
		case IDC_CHECK_SHOW_POPUP:
			if (BN_CLICKED == HIWORD(wp))
				update_popup_controls(hWnd);
			break;
		case IDC_BUTTON_BROWSE:
			if (BN_CLICKED == HIWORD(wp))
				select_log_file(hWnd);
			break;

		case IDC_BUTTON_POPUP_SETTINGS:
			if (BN_CLICKED == HIWORD(wp)) {
				CSettingWindowParam *pParam = get_param(hWnd);
				if (!pParam->m_pPopupSettings) {
					pParam->m_pPopupSettings = new CPopupSettings();
					pParam->m_pPopupSettings->InitForContact(pParam->m_hContact);
				}

				DialogBoxParam(g_plugin.getInst(),
					MAKEINTRESOURCE(IDD_DIALOG_POPUP),
					hWnd,
					EditPopupSettingsDlgProc, LPARAM(pParam->m_pPopupSettings));
			}
			break;

		case IDOK:
			{
				CSettingWindowParam *pParam = get_param(hWnd);
				MCONTACT hContact = pParam->m_hContact;

				bool bUseContactSpec = 1 == ::IsDlgButtonChecked(hWnd, IDC_CHECK_CONTACT_SPECIFIC);

				WORD nLogMode = lmDisabled;
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

					if (pParam->m_pPopupSettings) {
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

//////////////////////////////////////////////////////////////////////////

INT_PTR CALLBACK EditSettingsPerProviderDlgProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg) {
	case WM_INITDIALOG:
		{
			TranslateDialogDefault(hWnd);
			CAdvProviderSettings *pAdvSettings = reinterpret_cast<CAdvProviderSettings *>(lp);

			::SetDlgItemText(hWnd, IDC_EDIT_NAME, pAdvSettings->GetProviderPtr()->GetInfo().m_sName.c_str());

			// log to history
			WORD dwLogMode = pAdvSettings->GetLogMode();
			UINT nCheck = (dwLogMode & lmInternalHistory) ? 1 : 0;
			::CheckDlgButton(hWnd, IDC_CHECK_INTERNAL_HISTORY, nCheck ? BST_CHECKED : BST_UNCHECKED);
			::SetDlgItemText(hWnd, IDC_EDIT_HISTORY_FORMAT, pAdvSettings->GetHistoryFormat().c_str());
			::CheckDlgButton(hWnd, IDC_CHECK_HISTORY_CONDITION, (pAdvSettings->GetHistoryOnlyChangedFlag()) ? BST_CHECKED : BST_UNCHECKED);

			// log to file
			nCheck = (dwLogMode & lmExternalFile) ? 1 : 0;
			::CheckDlgButton(hWnd, IDC_CHECK_EXTERNAL_FILE, nCheck ? BST_CHECKED : BST_UNCHECKED);
			::SetDlgItemText(hWnd, IDC_EDIT_FILE_NAME, pAdvSettings->GetLogFileName().c_str());
			::SetDlgItemText(hWnd, IDC_EDIT_LOG_FILE_FORMAT, pAdvSettings->GetLogFormat().c_str());
			::CheckDlgButton(hWnd, IDC_CHECK_LOG_FILE_CONDITION, (pAdvSettings->GetLogOnlyChangedFlag()) ? BST_CHECKED : BST_UNCHECKED);

			update_file_controls(hWnd);
			update_history_controls(hWnd);

			// popup
			nCheck = (dwLogMode & lmPopup) ? 1 : 0;
			::CheckDlgButton(hWnd, IDC_CHECK_SHOW_POPUP, nCheck ? BST_CHECKED : BST_UNCHECKED);
			::SetDlgItemText(hWnd, IDC_EDIT_POPUP_FORMAT, pAdvSettings->GetPopupFormat().c_str());
			::CheckDlgButton(hWnd, IDC_CHECK_SHOW_POPUP_ONLY_VALUE_CHANGED, (pAdvSettings->GetShowPopupIfValueChangedFlag()) ? BST_CHECKED : BST_UNCHECKED);

			update_popup_controls(hWnd);

			::SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pAdvSettings));
		}
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wp)) {
		case IDOK:
			{
				WORD nLogMode = lmDisabled;
				UINT nCheck = ::IsDlgButtonChecked(hWnd, IDC_CHECK_EXTERNAL_FILE);
				if (1 == nCheck) {
					nLogMode |= lmExternalFile;
				}

				nCheck = ::IsDlgButtonChecked(hWnd, IDC_CHECK_INTERNAL_HISTORY);
				if (1 == nCheck) {
					nLogMode |= lmInternalHistory;
				}

				nCheck = ::IsDlgButtonChecked(hWnd, IDC_CHECK_SHOW_POPUP);
				if (1 == nCheck) {
					nLogMode |= lmPopup;
				}

				bool bOk = true;
				HWND hwndLogFile = ::GetDlgItem(hWnd, IDC_EDIT_FILE_NAME);
				HWND hwndLogFileFrmt = ::GetDlgItem(hWnd, IDC_EDIT_LOG_FILE_FORMAT);

				CMStringW sLogFile = get_window_text(hwndLogFile);
				CMStringW sLogFileFormat = get_window_text(hwndLogFileFrmt);

				if ((nLogMode & lmExternalFile)) {
					if (true == sLogFile.IsEmpty()) {
						prepare_edit_ctrl_for_error(hwndLogFile);
						CurrencyRates_MessageBox(hWnd, TranslateT("Enter log file name."), MB_OK | MB_ICONERROR);
						bOk = false;
					}
					else if (true == sLogFileFormat.IsEmpty()) {
						prepare_edit_ctrl_for_error(hwndLogFileFrmt);
						CurrencyRates_MessageBox(hWnd, TranslateT("Enter log file format."), MB_OK | MB_ICONERROR);
						bOk = false;
					}
				}

				HWND hwndHistoryFrmt = ::GetDlgItem(hWnd, IDC_EDIT_HISTORY_FORMAT);
				CMStringW sHistoryFormat = get_window_text(hwndHistoryFrmt);
				if ((true == bOk) && (nLogMode & lmInternalHistory) && (true == sHistoryFormat.IsEmpty())) {
					prepare_edit_ctrl_for_error(hwndHistoryFrmt);
					CurrencyRates_MessageBox(hWnd, TranslateT("Enter history format."), MB_OK | MB_ICONERROR);
					bOk = false;
				}

				HWND hwndPopupFrmt = ::GetDlgItem(hWnd, IDC_EDIT_POPUP_FORMAT);
				CMStringW sPopupFormat = get_window_text(hwndPopupFrmt);
				if ((true == bOk) && (nLogMode & lmPopup) && (true == sPopupFormat.IsEmpty())) {
					prepare_edit_ctrl_for_error(hwndPopupFrmt);
					CurrencyRates_MessageBox(hWnd, TranslateT("Enter popup window format."), MB_OK | MB_ICONERROR);
					bOk = false;
				}

				if (true == bOk) {
					CAdvProviderSettings *pAdvSettings = reinterpret_cast<CAdvProviderSettings *>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

					pAdvSettings->SetLogMode(nLogMode);
					pAdvSettings->SetHistoryOnlyChangedFlag(1 == IsDlgButtonChecked(hWnd, IDC_CHECK_HISTORY_CONDITION));
					pAdvSettings->SetLogOnlyChangedFlag(1 == IsDlgButtonChecked(hWnd, IDC_CHECK_LOG_FILE_CONDITION));
					pAdvSettings->SetShowPopupIfValueChangedFlag(1 == IsDlgButtonChecked(hWnd, IDC_CHECK_SHOW_POPUP_ONLY_VALUE_CHANGED));
					pAdvSettings->SetLogFileName(sLogFile);
					pAdvSettings->SetLogFormat(sLogFileFormat);
					pAdvSettings->SetHistoryFormat(sHistoryFormat);
					pAdvSettings->SetPopupFormat(sPopupFormat);

					::EndDialog(hWnd, IDOK);
				}
			}
			break;

		case IDCANCEL:
			::EndDialog(hWnd, IDCANCEL);
			break;

		case IDC_BUTTON_HISTORY_DESCRIPTION:
		case IDC_BUTTON_LOG_FILE_DESCRIPTION:
		case IDC_BUTTON_POPUP_FORMAT_DESCRIPTION:
			if (BN_CLICKED == HIWORD(wp)) {
				const CAdvProviderSettings *pAdvSettings = reinterpret_cast<CAdvProviderSettings *>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
				show_variable_list(hWnd, pAdvSettings->GetProviderPtr());
			}
			break;

		case IDC_CHECK_EXTERNAL_FILE:
			if (BN_CLICKED == HIWORD(wp))
				update_file_controls(hWnd);
			break;

		case IDC_CHECK_INTERNAL_HISTORY:
			if (BN_CLICKED == HIWORD(wp))
				update_history_controls(hWnd);
			break;

		case IDC_CHECK_SHOW_POPUP:
			if (BN_CLICKED == HIWORD(wp))
				update_popup_controls(hWnd);
			break;

		case IDC_BUTTON_BROWSE:
			if (BN_CLICKED == HIWORD(wp))
				select_log_file(hWnd);
			break;

		case IDC_BUTTON_POPUP_SETTINGS:
			const CAdvProviderSettings *pAdvSettings = reinterpret_cast<CAdvProviderSettings *>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
			DialogBoxParam(g_plugin.getInst(),
				MAKEINTRESOURCE(IDD_DIALOG_POPUP),
				hWnd,
				EditPopupSettingsDlgProc, LPARAM(pAdvSettings->GetPopupSettingsPtr()));
			break;
		}
		break;
	}
	return FALSE;
}

CAdvProviderSettings::CAdvProviderSettings(const ICurrencyRatesProvider *pCurrencyRatesProvider) :
	m_pCurrencyRatesProvider(pCurrencyRatesProvider),
	m_wLogMode(lmDisabled),
	m_bIsOnlyChangedHistory(false),
	m_bIsOnlyChangedLogFile(false),
	m_bShowPopupIfValueChanged(false),
	m_pPopupSettings(nullptr)
{
	assert(m_pCurrencyRatesProvider);

	m_wLogMode = g_plugin.getWord(DB_KEY_LogMode, static_cast<WORD>(lmDisabled));
	m_sFormatHistory = g_plugin.getMStringW(DB_KEY_HistoryFormat, DB_DEF_HistoryFormat);
	m_bIsOnlyChangedHistory = 1 == g_plugin.getByte(DB_KEY_HistoryCondition, 0);

	m_sLogFileName = g_plugin.getMStringW(DB_KEY_LogFile);
	if (true == m_sLogFileName.IsEmpty()) {
		m_sLogFileName = g_pszVariableUserProfile;
		m_sLogFileName += L"\\CurrencyRates\\";
		m_sLogFileName += g_pszVariableCurrencyRateName;
		m_sLogFileName += L".log";
	}

	m_sFormatLogFile = g_plugin.getMStringW(DB_KEY_LogFormat, DB_DEF_LogFormat);
	m_bIsOnlyChangedLogFile = (1 == g_plugin.getByte(DB_KEY_LogCondition, 0));

	m_sPopupFormat = g_plugin.getMStringW(DB_KEY_PopupFormat, DB_DEF_PopupFormat);
	m_bShowPopupIfValueChanged = (1 == g_plugin.getByte(DB_KEY_PopupCondition, 0));
}

CAdvProviderSettings::~CAdvProviderSettings()
{
	delete m_pPopupSettings;
}

const ICurrencyRatesProvider *CAdvProviderSettings::GetProviderPtr() const
{
	return m_pCurrencyRatesProvider;
}

void CAdvProviderSettings::SaveToDb() const
{
	g_plugin.setWord(DB_KEY_LogMode, m_wLogMode);
	g_plugin.setWString(DB_KEY_HistoryFormat, m_sFormatHistory.c_str());
	g_plugin.setByte(DB_KEY_HistoryCondition, m_bIsOnlyChangedHistory);
	g_plugin.setWString(DB_KEY_LogFile, m_sLogFileName.c_str());
	g_plugin.setWString(DB_KEY_LogFormat, m_sFormatLogFile.c_str());
	g_plugin.setByte(DB_KEY_LogCondition, m_bIsOnlyChangedLogFile);
	g_plugin.setWString(DB_KEY_PopupFormat, m_sPopupFormat.c_str());
	g_plugin.setByte(DB_KEY_PopupCondition, m_bShowPopupIfValueChanged);

	if (nullptr != m_pPopupSettings) {
		g_plugin.setByte(DB_KEY_PopupColourMode, static_cast<BYTE>(m_pPopupSettings->GetColourMode()));
		g_plugin.setDword(DB_KEY_PopupBkColour, m_pPopupSettings->GetColourBk());
		g_plugin.setDword(DB_KEY_PopupTextColour, m_pPopupSettings->GetColourText());
		g_plugin.setByte(DB_KEY_PopupDelayMode, static_cast<BYTE>(m_pPopupSettings->GetDelayMode()));
		g_plugin.setWord(DB_KEY_PopupDelayTimeout, m_pPopupSettings->GetDelayTimeout());
		g_plugin.setByte(DB_KEY_PopupHistoryFlag, m_pPopupSettings->GetHistoryFlag());
	}
}

CPopupSettings *CAdvProviderSettings::GetPopupSettingsPtr() const
{
	if (nullptr == m_pPopupSettings)
		m_pPopupSettings = new CPopupSettings();

	return m_pPopupSettings;
}

/////////////////////////////////////////////////////////////////////////////////////////
// class CPopupSettings

CPopupSettings::CPopupSettings() :
	m_modeColour(colourDefault),
	m_modeDelay(delayFromPopup),
	m_rgbBkg(GetDefColourBk()),
	m_rgbText(GetDefColourText()),
	m_wDelay(3),
	m_bUseHistory(false)

{
	BYTE m = g_plugin.getByte(DB_KEY_PopupColourMode, static_cast<BYTE>(m_modeColour));
	if (m >= colourDefault && m <= colourUserDefined)
		m_modeColour = static_cast<EColourMode>(m);

	m_rgbBkg = g_plugin.getDword(DB_KEY_PopupBkColour, m_rgbBkg);
	m_rgbText = g_plugin.getDword(DB_KEY_PopupTextColour, m_rgbText);

	m = g_plugin.getByte(DB_KEY_PopupDelayMode, static_cast<BYTE>(m_modeDelay));
	if (m >= delayFromPopup && m <= delayPermanent)
		m_modeDelay = static_cast<EDelayMode>(m);

	m_wDelay = g_plugin.getWord(DB_KEY_PopupDelayTimeout, m_wDelay);
	m_bUseHistory = (1 == g_plugin.getByte(DB_KEY_PopupHistoryFlag, m_bUseHistory));
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
	BYTE m = g_plugin.getByte(hContact, DB_STR_CURRENCYRATE_POPUP_COLOUR_MODE, static_cast<BYTE>(m_modeColour));
	if (m >= CPopupSettings::colourDefault && m <= CPopupSettings::colourUserDefined) {
		m_modeColour = static_cast<CPopupSettings::EColourMode>(m);
	}

	m_rgbBkg = g_plugin.getDword(hContact, DB_STR_CURRENCYRATE_POPUP_COLOUR_BK, m_rgbBkg);
	m_rgbText = g_plugin.getDword(hContact, DB_STR_CURRENCYRATE_POPUP_COLOUR_TEXT, m_rgbText);

	m = g_plugin.getByte(hContact, DB_STR_CURRENCYRATE_POPUP_DELAY_MODE, static_cast<BYTE>(m_modeDelay));
	if (m >= CPopupSettings::delayFromPopup && m <= CPopupSettings::delayPermanent) {
		m_modeDelay = static_cast<CPopupSettings::EDelayMode>(m);
	}
	m_wDelay = g_plugin.getWord(hContact, DB_STR_CURRENCYRATE_POPUP_DELAY_TIMEOUT, m_wDelay);
	m_bUseHistory = 1 == g_plugin.getByte(hContact, DB_STR_CURRENCYRATE_POPUP_HISTORY_FLAG, m_bUseHistory);
}

void CPopupSettings::SaveForContact(MCONTACT hContact) const
{
	g_plugin.setByte(hContact, DB_STR_CURRENCYRATE_POPUP_COLOUR_MODE, static_cast<BYTE>(m_modeColour));
	g_plugin.setDword(hContact, DB_STR_CURRENCYRATE_POPUP_COLOUR_BK, m_rgbBkg);
	g_plugin.setDword(hContact, DB_STR_CURRENCYRATE_POPUP_COLOUR_TEXT, m_rgbText);
	g_plugin.setByte(hContact, DB_STR_CURRENCYRATE_POPUP_DELAY_MODE, static_cast<BYTE>(m_modeDelay));
	g_plugin.setWord(hContact, DB_STR_CURRENCYRATE_POPUP_DELAY_TIMEOUT, m_wDelay);
	g_plugin.setByte(hContact, DB_STR_CURRENCYRATE_POPUP_HISTORY_FLAG, m_bUseHistory);
}

CPopupSettings::EColourMode CPopupSettings::GetColourMode() const
{
	return m_modeColour;
}

void CPopupSettings::SetColourMode(EColourMode nMode)
{
	m_modeColour = nMode;
}

COLORREF CPopupSettings::GetColourBk() const
{
	return m_rgbBkg;
}

void CPopupSettings::SetColourBk(COLORREF rgb)
{
	m_rgbBkg = rgb;
}

COLORREF CPopupSettings::GetColourText() const
{
	return m_rgbText;
}

void CPopupSettings::SetColourText(COLORREF rgb)
{
	m_rgbText = rgb;
}

CPopupSettings::EDelayMode CPopupSettings::GetDelayMode() const
{
	return m_modeDelay;
}

void CPopupSettings::SetDelayMode(EDelayMode nMode)
{
	m_modeDelay = nMode;
}

WORD CPopupSettings::GetDelayTimeout() const
{
	return m_wDelay;
}

void CPopupSettings::SetDelayTimeout(WORD delay)
{
	m_wDelay = delay;
}

bool CPopupSettings::GetHistoryFlag() const
{
	return m_bUseHistory;
}

void CPopupSettings::SetHistoryFlag(bool flag)
{
	m_bUseHistory = flag;
}

bool ShowSettingsDlg(HWND hWndParent, CAdvProviderSettings *pAdvSettings)
{
	assert(pAdvSettings);

	return (IDOK == DialogBoxParam(g_plugin.getInst(),
		MAKEINTRESOURCE(IDD_PROVIDER_ADV_SETTINGS),
		hWndParent,
		EditSettingsPerProviderDlgProc,
		LPARAM(pAdvSettings)));
}

CMStringW GenerateLogFileName(const CMStringW &rsLogFilePattern, const CMStringW &rsCurrencyRateSymbol, int nFlags)
{
	CMStringW sPath = rsLogFilePattern;
	if (nFlags & glfnResolveCurrencyRateName) {
		assert(false == rsCurrencyRateSymbol.IsEmpty());

		CMStringW s = rsCurrencyRateSymbol;
		FixInvalidChars(s);
		sPath.Replace(g_pszVariableCurrencyRateName, s);
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

	auto pProvider = GetContactProviderPtr(hContact);
	if (pProvider) {
		CMStringW sPattern;
		bool bUseContactSpecific = (g_plugin.getByte(hContact, DB_STR_CONTACT_SPEC_SETTINGS, 0) > 0);
		if (bUseContactSpecific)
			sPattern = g_plugin.getMStringW(hContact, DB_STR_CURRENCYRATE_LOG_FILE);
		else {
			CAdvProviderSettings global_settings(pProvider);
			sPattern = global_settings.GetLogFileName();
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
