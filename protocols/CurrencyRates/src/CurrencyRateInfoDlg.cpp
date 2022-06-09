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

/////////////////////////////////////////////////////////////////////////////////////////
// User info dialog

class CurrencyRateInfoDlg : public CUserInfoPageDlg
{
	CCtrlHyperlink url;

public:
	CurrencyRateInfoDlg(int idDialog) :
		CUserInfoPageDlg(g_plugin, idDialog),
		url(this, IDC_SYSLINK_PROVIDER)
	{
	}

	bool OnRefresh() override
	{
		::SetDlgItemTextW(m_hwnd, IDC_STATIC_CURRENCYRATE_NAME, GetContactName(m_hContact));

		double dRate = 0.0;
		if (true == CurrencyRates_DBReadDouble(m_hContact, MODULENAME, DB_STR_CURRENCYRATE_PREV_VALUE, dRate)) {
			wchar_t str[40];
			swprintf_s(str, L"%.6lf", dRate);
			::SetDlgItemTextW(m_hwnd, IDC_EDIT_PREVIOUS_RATE, str);
		}

		dRate = 0.0;
		if (true == CurrencyRates_DBReadDouble(m_hContact, MODULENAME, DB_STR_CURRENCYRATE_CURR_VALUE, dRate)) {
			wchar_t str[40];
			swprintf_s(str, L"%.6lf", dRate);
			::SetDlgItemTextW(m_hwnd, IDC_EDIT_RATE, str);
		}

		time_t nFetchTime = g_plugin.getDword(m_hContact, DB_STR_CURRENCYRATE_FETCH_TIME, -1);
		if (nFetchTime != -1) {
			wchar_t szTime[50] = { 0 };
			if (0 == _tctime_s(szTime, 50, &nFetchTime)) {
				::SetDlgItemTextW(m_hwnd, IDC_EDIT_RATE_FETCH_TIME, szTime);
			}
		}

		auto &pi = GetContactProviderPtr(m_hContact)->GetInfo();
		CMStringW provInfo(FORMAT, L"%s <a href=\"%s\">%s</a>", TranslateT("Info provided by"), pi.m_sURL.c_str(), pi.m_sName.c_str());
		::SetDlgItemTextW(m_hwnd, IDC_SYSLINK_PROVIDER, provInfo);
		return false;
	}

	void onClick_Url(CCtrlHyperlink *pLink)
	{
		::ShellExecute(m_hwnd, L"open", ptrW(pLink->GetText()), nullptr, nullptr, SW_SHOWNORMAL);
	}
};

int CurrencyRatesEventFunc_OnUserInfoInit(WPARAM wp, LPARAM hContact)
{
	if (NULL == hContact)
		return 0;

	if (false == IsMyContact(hContact))
		return 0;

	g_hContact = hContact;

	USERINFOPAGE uip = {};
	uip.position = -2000000000;
	uip.pDialog = new CurrencyRateInfoDlg(IDD_DIALOG_CURRENCYRATE_INFO);
	uip.szTitle.a = LPGEN("Currency Rate");
	g_plugin.addUserInfo(wp, &uip);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Additional info dialog

struct CurrencyRateInfoDlg2 : public CurrencyRateInfoDlg
{
	CurrencyRateInfoDlg2(MCONTACT hContact) :
		CurrencyRateInfoDlg(IDD_DIALOG_CURRENCYRATE_INFO_1)
	{
		m_hContact = hContact;
	}

	bool OnInitDialog() override
	{
		MWindowList hWL = CModuleInfo::GetWindowList(WINDOW_PREFIX_INFO, false);
		WindowList_Add(hWL, m_hwnd, m_hContact);

		Utils_RestoreWindowPositionNoSize(m_hwnd, m_hContact, MODULENAME, WINDOW_PREFIX_INFO);
		OnRefresh();
		return true;
	}

	void OnDestroy() override
	{
		MWindowList hWL = CModuleInfo::GetWindowList(WINDOW_PREFIX_INFO, false);
		WindowList_Remove(hWL, m_hwnd);

		Utils_SaveWindowPosition(m_hwnd, m_hContact, MODULENAME, WINDOW_PREFIX_INFO);
	}
};

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
			(new CurrencyRateInfoDlg2(hContact))->Show();

		return 1;
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

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
