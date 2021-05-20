#include "StdAfx.h"

// extern HANDLE g_hWindowListEditSettings;
extern HGENMENU g_hMenuEditSettings;
extern HGENMENU g_hMenuOpenLogFile;
#ifdef CHART_IMPLEMENT
extern HGENMENU g_hMenuChart;
#endif
extern HGENMENU g_hMenuRefresh, g_hMenuRoot;

#define WINDOW_PREFIX_INFO "Currency Rate Info"

MCONTACT g_hContact;

inline bool IsMyContact(MCONTACT hContact)
{
	return nullptr != GetContactProviderPtr(hContact);
}

inline MCONTACT get_contact(HWND hWnd)
{
	return MCONTACT(GetWindowLongPtr(hWnd, GWLP_USERDATA));
}

static bool get_fetch_time(time_t& rTime, MCONTACT hContact)
{
	rTime = g_plugin.getDword(hContact, DB_STR_CURRENCYRATE_FETCH_TIME, -1);
	return (rTime != -1);
}

INT_PTR CALLBACK CurrencyRateInfoDlgProcImpl(MCONTACT hContact, HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		assert(hContact);

		TranslateDialogDefault(hdlg);
		{
			::SetDlgItemTextW(hdlg, IDC_STATIC_CURRENCYRATE_NAME, GetContactName(hContact));

			double dRate = 0.0;
			if (true == CurrencyRates_DBReadDouble(hContact, MODULENAME, DB_STR_CURRENCYRATE_PREV_VALUE, dRate)) {
				wchar_t str[40];
				swprintf_s(str, L"%.6lf", dRate);
				::SetDlgItemTextW(hdlg, IDC_EDIT_PREVIOUS_RATE, str);
			}

			dRate = 0.0;
			if (true == CurrencyRates_DBReadDouble(hContact, MODULENAME, DB_STR_CURRENCYRATE_CURR_VALUE, dRate)) {
				wchar_t str[40];
				swprintf_s(str, L"%.6lf", dRate);
				::SetDlgItemTextW(hdlg, IDC_EDIT_RATE, str);
			}

			time_t nFetchTime;
			if (true == get_fetch_time(nFetchTime, hContact)) {
				wchar_t szTime[50] = { 0 };
				if (0 == _tctime_s(szTime, 50, &nFetchTime)) {
					::SetDlgItemTextW(hdlg, IDC_EDIT_RATE_FETCH_TIME, szTime);
				}
			}

			const ICurrencyRatesProvider::CProviderInfo& pi = GetContactProviderPtr(hContact)->GetInfo();
			CMStringW provInfo(FORMAT, L"%s <a href=\"%s\">%s</a>", TranslateT("Info provided by"), pi.m_sURL.c_str(), pi.m_sName.c_str());
			::SetDlgItemTextW(hdlg, IDC_SYSLINK_PROVIDER, provInfo);
		}
		return TRUE;

	case WM_NOTIFY:
		LPNMHDR pNMHDR = reinterpret_cast<LPNMHDR>(lParam);
		switch (pNMHDR->code) {
		case NM_CLICK:
			if (IDC_SYSLINK_PROVIDER == wParam) {
				PNMLINK pNMLink = reinterpret_cast<PNMLINK>(pNMHDR);
				::ShellExecute(hdlg, L"open", pNMLink->item.szUrl, nullptr, nullptr, SW_SHOWNORMAL);
			}
			break;
		}
		break;
	}
	return FALSE;
}

INT_PTR CALLBACK CurrencyRateInfoDlgProc(HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return CurrencyRateInfoDlgProcImpl(g_hContact, hdlg, msg, wParam, lParam);
}

int CurrencyRatesEventFunc_OnUserInfoInit(WPARAM wp, LPARAM hContact)
{
	if (NULL == hContact)
		return 0;

	if (false == IsMyContact(hContact))
		return 0;

	g_hContact = hContact;

	OPTIONSDIALOGPAGE odp = {};
	odp.pfnDlgProc = CurrencyRateInfoDlgProc;
	odp.position = -2000000000;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_DIALOG_CURRENCYRATE_INFO);
	odp.szTitle.a = LPGEN("Currency Rate");
	g_plugin.addUserInfo(wp, &odp);
	return 0;
}


INT_PTR CurrencyRatesMenu_EditSettings(WPARAM wp, LPARAM)
{
	MCONTACT hContact = MCONTACT(wp);
	if (NULL != hContact)
		ShowSettingsDlg(hContact);
	return 0;
}

namespace
{
	bool get_log_file(MCONTACT hContact, CMStringW &rsLogfile)
	{
		rsLogfile = GetContactLogFileName(hContact);
		return !rsLogfile.IsEmpty();
	}
}

INT_PTR CurrencyRatesMenu_OpenLogFile(WPARAM wp, LPARAM)
{
	MCONTACT hContact = MCONTACT(wp);
	if (NULL == hContact)
		return 0;

	CMStringW sLogFileName;
	if ((true == get_log_file(hContact, sLogFileName)) && (false == sLogFileName.IsEmpty()))
		::ShellExecute(nullptr, L"open", sLogFileName.c_str(), nullptr, nullptr, SW_SHOWNORMAL);

	return 0;
}

INT_PTR CurrencyRatesMenu_RefreshContact(WPARAM wp, LPARAM)
{
	MCONTACT hContact = MCONTACT(wp);
	if (NULL == hContact)
		return 0;

	ICurrencyRatesProvider *pProvider = GetContactProviderPtr(hContact);
	if (pProvider)
		pProvider->RefreshContact(hContact);
	return 0;
}

static INT_PTR CALLBACK CurrencyRateInfoDlgProc1(HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	MCONTACT hContact = NULL;
	MWindowList hWL;

	switch (msg) {
	case WM_INITDIALOG:
		hContact = MCONTACT(lParam);
		hWL = CModuleInfo::GetWindowList(WINDOW_PREFIX_INFO, false);
		assert(hWL);
		WindowList_Add(hWL, hdlg, hContact);

		::SetWindowLongPtr(hdlg, GWLP_USERDATA, hContact);
		Utils_RestoreWindowPositionNoSize(hdlg, hContact, MODULENAME, WINDOW_PREFIX_INFO);
		::ShowWindow(hdlg, SW_SHOW);
		break;

	case WM_CLOSE:
		DestroyWindow(hdlg);
		return FALSE;

	case WM_DESTROY:
		hContact = get_contact(hdlg);
		if (hContact) {
			SetWindowLongPtr(hdlg, GWLP_USERDATA, 0);

			hWL = CModuleInfo::GetWindowList(WINDOW_PREFIX_INFO, false);
			assert(hWL);
			WindowList_Remove(hWL, hdlg);
			Utils_SaveWindowPosition(hdlg, hContact, MODULENAME, WINDOW_PREFIX_INFO);
		}
		return FALSE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK) {
			::DestroyWindow(hdlg);
			return FALSE;
		}

	default:
		hContact = get_contact(hdlg);
		break;
	}

	return CurrencyRateInfoDlgProcImpl(hContact, hdlg, msg, wParam, lParam);
}

int CurrencyRates_OnContactDoubleClick(WPARAM wp, LPARAM/* lp*/)
{
	MCONTACT hContact = MCONTACT(wp);
	if (GetContactProviderPtr(hContact)) {
		MWindowList hWL = CModuleInfo::GetWindowList(WINDOW_PREFIX_INFO, true);
		assert(hWL);
		HWND hWnd = WindowList_Find(hWL, hContact);
		if (nullptr != hWnd) {
			SetForegroundWindow(hWnd);
			SetFocus(hWnd);
		}
		else if (true == IsMyContact(hContact))
			CreateDialogParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_DIALOG_CURRENCYRATE_INFO_1), nullptr, CurrencyRateInfoDlgProc1, LPARAM(hContact));

		return 1;
	}

	return 0;
}

int CurrencyRates_PrebuildContactMenu(WPARAM wp, LPARAM)
{
	Menu_EnableItem(g_hMenuEditSettings, false);
	Menu_EnableItem(g_hMenuOpenLogFile, false);
	#ifdef CHART_IMPLEMENT
	Menu_EnableItem(g_hMenuChart, false);
	#endif
	Menu_EnableItem(g_hMenuRefresh, false);

	MCONTACT hContact = MCONTACT(wp);
	char *szProto = Proto_GetBaseAccountName(hContact);
	if (mir_strcmp(szProto, MODULENAME)) {
		Menu_ShowItem(g_hMenuRoot, false);
		return 0;
	}

	Menu_ShowItem(g_hMenuRoot, true);
	Menu_EnableItem(g_hMenuEditSettings, true);

	Menu_EnableItem(g_hMenuRefresh, true);

	CMStringW sLogFileName;
	if (get_log_file(hContact, sLogFileName) && !sLogFileName.IsEmpty() && !_waccess(sLogFileName, 04)) {
		#ifdef CHART_IMPLEMENT
		Menu_EnableItem(g_hMenuChart, true);
		#endif
		Menu_EnableItem(g_hMenuOpenLogFile, true);
	}

	return 0;
}
