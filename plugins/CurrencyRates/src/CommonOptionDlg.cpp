#include "StdAfx.h"

typedef boost::shared_ptr<CAdvProviderSettings> TAdvSettingsPtr;
typedef std::map<const ICurrencyRatesProvider*, TAdvSettingsPtr> TAdvSettings;

TAdvSettings g_aAdvSettings;

CAdvProviderSettings* get_adv_settings(const ICurrencyRatesProvider *pProvider, bool bCreateIfNonExist)
{
	TAdvSettings::iterator i = g_aAdvSettings.find(pProvider);
	if (i != g_aAdvSettings.end())
		return i->second.get();

	if (true == bCreateIfNonExist) {
		TAdvSettingsPtr pAdvSet(new CAdvProviderSettings(pProvider));
		g_aAdvSettings.insert(std::make_pair(pProvider, pAdvSet));
		return pAdvSet.get();
	}

	return nullptr;
}

void remove_adv_settings(const ICurrencyRatesProvider *pProvider)
{
	TAdvSettings::iterator i = g_aAdvSettings.find(pProvider);
	if (i != g_aAdvSettings.end())
		g_aAdvSettings.erase(i);
}

void CommonOptionDlgProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp, CCommonDlgProcData& rData)
{
	switch (msg) {
	case WM_INITDIALOG:
		{
			assert(rData.m_pCurrencyRatesProvider);

			// set contact list display format
			tstring sDspNameFrmt = CurrencyRates_DBGetStringW(NULL, CURRENCYRATES_MODULE_NAME, DB_KEY_DisplayNameFormat, DB_DEF_DisplayNameFormat);
			::SetDlgItemText(hWnd, IDC_EDIT_CONTACT_LIST_FORMAT, sDspNameFrmt.c_str());

			// set status message display format
			tstring sStatusMsgFrmt = CurrencyRates_DBGetStringW(NULL, CURRENCYRATES_MODULE_NAME, DB_KEY_StatusMsgFormat, DB_DEF_StatusMsgFormat);
			::SetDlgItemText(hWnd, IDC_EDIT_STATUS_MESSAGE_FORMAT, sStatusMsgFrmt.c_str());

			// set tendency format
			tstring sTendencyFrmt = CurrencyRates_DBGetStringW(NULL, CURRENCYRATES_MODULE_NAME, DB_KEY_TendencyFormat, DB_DEF_TendencyFormat);
			::SetDlgItemText(hWnd, IDC_EDIT_TENDENCY_FORMAT, sTendencyFrmt.c_str());

			// set api key
			tstring sApiKey = CurrencyRates_DBGetStringW(NULL, CURRENCYRATES_MODULE_NAME, DB_KEY_ApiKey, L"");
			::SetDlgItemText(hWnd, IDC_EDIT_PERSONAL_KEY, sApiKey.c_str());

			// refresh rate
			HWND hwndCombo = ::GetDlgItem(hWnd, IDC_COMBO_REFRESH_RATE);
			LPCTSTR pszRefreshRateTypes[] = { TranslateT("Seconds"), TranslateT("Minutes"), TranslateT("Hours") };
			for (int i = 0; i < _countof(pszRefreshRateTypes); ++i)
				::SendMessage(hwndCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(pszRefreshRateTypes[i]));

			int nRefreshRateType = db_get_w(0, CURRENCYRATES_MODULE_NAME, DB_KEY_RefreshRateType, RRT_MINUTES);
			if (nRefreshRateType < RRT_SECONDS || nRefreshRateType > RRT_HOURS)
				nRefreshRateType = RRT_MINUTES;

			UINT nRate = db_get_w(0, CURRENCYRATES_MODULE_NAME, DB_KEY_RefreshRateValue, 1);
			switch (nRefreshRateType) {
			default:
			case RRT_SECONDS:
			case RRT_MINUTES:
				if (nRate < 1 || nRate > 60)
					nRate = 1;

				spin_set_range(::GetDlgItem(hWnd, IDC_SPIN_REFRESH_RATE), 1, 60);
				break;
			case RRT_HOURS:
				if (nRate < 1 || nRate > 24)
					nRate = 1;

				spin_set_range(::GetDlgItem(hWnd, IDC_SPIN_REFRESH_RATE), 1, 24);
				break;
			}

			::SendMessage(hwndCombo, CB_SETCURSEL, nRefreshRateType, 0);
			::SetDlgItemInt(hWnd, IDC_EDIT_REFRESH_RATE, nRate, FALSE);

			PropSheet_UnChanged(::GetParent(hWnd), hWnd);
		}
		break;

	case WM_COMMAND:
		switch (HIWORD(wp)) {
		case CBN_SELCHANGE:
			if (IDC_COMBO_REFRESH_RATE == LOWORD(wp)) {
				ERefreshRateType nType = static_cast<ERefreshRateType>(::SendMessage(reinterpret_cast<HWND>(lp), CB_GETCURSEL, 0, 0));
				switch (nType) {
				default:
				case RRT_SECONDS:
				case RRT_MINUTES:
					spin_set_range(::GetDlgItem(hWnd, IDC_SPIN_REFRESH_RATE), 1, 60);
					break;
				case RRT_HOURS:
					spin_set_range(::GetDlgItem(hWnd, IDC_SPIN_REFRESH_RATE), 1, 24);
					BOOL bOk = FALSE;
					UINT nRefreshRate = ::GetDlgItemInt(hWnd, IDC_EDIT_REFRESH_RATE, &bOk, FALSE);
					if (TRUE == bOk && nRefreshRate > 24)
						::SetDlgItemInt(hWnd, IDC_EDIT_REFRESH_RATE, 24, FALSE);
					break;
				}

				PropSheet_Changed(::GetParent(hWnd), hWnd);
			}
			break;

		case EN_CHANGE:
			switch (LOWORD(wp)) {
			case IDC_EDIT_REFRESH_RATE:
			case IDC_EDIT_CONTACT_LIST_FORMAT:
			case IDC_EDIT_STATUS_MESSAGE_FORMAT:
			case IDC_EDIT_TENDENCY_FORMAT:
			case IDC_EDIT_PERSONAL_KEY:
				if (reinterpret_cast<HWND>(lp) == ::GetFocus())
					PropSheet_Changed(::GetParent(hWnd), hWnd);
				break;
			}
			break;

		case BN_CLICKED:
			switch (LOWORD(wp)) {
			case IDC_BUTTON_DESCRIPTION:
				show_variable_list(hWnd, rData.m_pCurrencyRatesProvider);
				break;
			case IDC_BUTTON_ADVANCED_SETTINGS:
				CAdvProviderSettings* pAdvSet = get_adv_settings(rData.m_pCurrencyRatesProvider, true);
				assert(pAdvSet);
				if (true == ShowSettingsDlg(hWnd, pAdvSet))
					PropSheet_Changed(::GetParent(hWnd), hWnd);
				break;
			}
			break;
		}
		break;

	case WM_NOTIFY:
		{
			LPNMHDR pNMHDR = reinterpret_cast<LPNMHDR>(lp);
			switch (pNMHDR->code) {
			case PSN_KILLACTIVE:
				{
					BOOL bOk = FALSE;
					UINT nRefreshRate = ::GetDlgItemInt(hWnd, IDC_EDIT_REFRESH_RATE, &bOk, FALSE);
					ERefreshRateType nType = static_cast<ERefreshRateType>(::SendDlgItemMessage(hWnd, IDC_COMBO_REFRESH_RATE, CB_GETCURSEL, 0, 0));
					switch (nType) {
					default:
					case RRT_MINUTES:
					case RRT_SECONDS:
						if (FALSE == bOk || nRefreshRate < 1 || nRefreshRate > 60) {
							prepare_edit_ctrl_for_error(::GetDlgItem(hWnd, IDC_EDIT_REFRESH_RATE));
							CurrencyRates_MessageBox(hWnd, TranslateT("Enter integer value between 1 and 60."), MB_OK | MB_ICONERROR);
							bOk = FALSE;
						}
						break;
					case RRT_HOURS:
						if (FALSE == bOk || nRefreshRate < 1 || nRefreshRate > 24) {
							prepare_edit_ctrl_for_error(::GetDlgItem(hWnd, IDC_EDIT_REFRESH_RATE));
							CurrencyRates_MessageBox(hWnd, TranslateT("Enter integer value between 1 and 24."), MB_OK | MB_ICONERROR);
							bOk = FALSE;
						}
						break;
					}

					if (TRUE == bOk) {
						HWND hEdit = ::GetDlgItem(hWnd, IDC_EDIT_CONTACT_LIST_FORMAT);
						assert(IsWindow(hEdit));

						tstring s = get_window_text(hEdit);
						if (true == s.empty()) {
							prepare_edit_ctrl_for_error(hEdit);
							CurrencyRates_MessageBox(hWnd, TranslateT("Enter text to display in contact list."), MB_OK | MB_ICONERROR);
							bOk = FALSE;
						}
					}

					::SetWindowLongPtr(hWnd, DWLP_MSGRESULT, (TRUE == bOk) ? FALSE : TRUE);
				}
				break;

			case PSN_APPLY:
				BOOL bOk = FALSE;
				UINT nRefreshRate = ::GetDlgItemInt(hWnd, IDC_EDIT_REFRESH_RATE, &bOk, FALSE);
				assert(TRUE == bOk);
				ERefreshRateType nType = static_cast<ERefreshRateType>(::SendDlgItemMessage(hWnd, IDC_COMBO_REFRESH_RATE, CB_GETCURSEL, 0, 0));

				assert(rData.m_pCurrencyRatesProvider);

				rData.m_bFireSetingsChangedEvent = true;
				db_set_w(0, CURRENCYRATES_MODULE_NAME, DB_KEY_RefreshRateType, nType);
				db_set_w(0, CURRENCYRATES_MODULE_NAME, DB_KEY_RefreshRateValue, nRefreshRate);

				tstring s = get_window_text(::GetDlgItem(hWnd, IDC_EDIT_CONTACT_LIST_FORMAT));
				db_set_ws(0, CURRENCYRATES_MODULE_NAME, DB_KEY_DisplayNameFormat, s.c_str());

				s = get_window_text(::GetDlgItem(hWnd, IDC_EDIT_STATUS_MESSAGE_FORMAT));
				db_set_ws(0, CURRENCYRATES_MODULE_NAME, DB_KEY_StatusMsgFormat, s.c_str());

				s = get_window_text(::GetDlgItem(hWnd, IDC_EDIT_TENDENCY_FORMAT));
				db_set_ws(0, CURRENCYRATES_MODULE_NAME, DB_KEY_TendencyFormat, s.c_str());

				s = get_window_text(::GetDlgItem(hWnd, IDC_EDIT_PERSONAL_KEY));
				db_set_ws(0, CURRENCYRATES_MODULE_NAME, DB_KEY_ApiKey, s.c_str());

				CAdvProviderSettings* pAdvSet = get_adv_settings(rData.m_pCurrencyRatesProvider, false);
				if (pAdvSet)
					pAdvSet->SaveToDb();
				break;
			}
		}
		break;

	case WM_DESTROY:
		remove_adv_settings(rData.m_pCurrencyRatesProvider);
		break;
	}
}
