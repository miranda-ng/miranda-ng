/*
Copyright (C) 2012-23 Miranda NG team (https://miranda-ng.org)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

typedef boost::shared_ptr<CAdvProviderSettings> TAdvSettingsPtr;
typedef std::map<const ICurrencyRatesProvider *, TAdvSettingsPtr> TAdvSettings;

TAdvSettings g_aAdvSettings;

CAdvProviderSettings *get_adv_settings(const ICurrencyRatesProvider *m_pProvider, bool bCreateIfNonExist)
{
	TAdvSettings::iterator i = g_aAdvSettings.find(m_pProvider);
	if (i != g_aAdvSettings.end())
		return i->second.get();

	if (true == bCreateIfNonExist) {
		TAdvSettingsPtr pAdvSet(new CAdvProviderSettings(m_pProvider));
		g_aAdvSettings.insert(std::make_pair(m_pProvider, pAdvSet));
		return pAdvSet.get();
	}

	return nullptr;
}

void remove_adv_settings(const ICurrencyRatesProvider *m_pProvider)
{
	TAdvSettings::iterator i = g_aAdvSettings.find(m_pProvider);
	if (i != g_aAdvSettings.end())
		g_aAdvSettings.erase(i);
}

class COptionsDlg : public CDlgBase
{
	CCurrencyRatesProviderBase* get_provider()
	{
		for (auto &it : g_apProviders)
			if (auto p = dynamic_cast<CCurrencyRatesProviderBase *>(it))
				return p;

		assert(!"We should never get here!");
		return nullptr;
	};

	CMStringW make_currencyrate_name(const CCurrencyRate &rCurrencyRate)
	{
		auto &rsDesc = rCurrencyRate.GetName();
		return((false == rsDesc.IsEmpty()) ? rsDesc : rCurrencyRate.GetSymbol());
	};

	CMStringW make_contact_name(const CMStringW &rsSymbolFrom, const CMStringW &rsSymbolTo)
	{
		return rsSymbolFrom + L"/" + rsSymbolTo;
	};

	CMStringW make_rate_name(const CCurrencyRatesProviderBase::TRateInfo &ri)
	{
		if ((false == ri.first.GetName().IsEmpty()) && (false == ri.second.GetName().IsEmpty()))
			return make_contact_name(ri.first.GetName(), ri.second.GetName());

		return make_contact_name(ri.first.GetSymbol(), ri.second.GetSymbol());
	};

	using TWatchedRates = std::vector<CCurrencyRatesProviderBase::TRateInfo>;
	TWatchedRates g_aWatchedRates;

	CCurrencyRatesProviderBase *m_pProvider;

	CCtrlCombo cmbFrom, cmbTo;
	CCtrlButton btnAdd, btnRemove, btnDescr, btnAdvanced;
	CCtrlListBox m_list;

public:
	COptionsDlg() :
		CDlgBase(g_plugin, IDD_DIALOG_OPT_GOOGLE),
		m_pProvider(get_provider()),
		m_list(this, IDC_LIST_RATES),
		btnAdd(this, IDC_BUTTON_ADD),
		btnDescr(this, IDC_BUTTON_DESCRIPTION),
		btnRemove(this, IDC_BUTTON_REMOVE),
		btnAdvanced(this, IDC_BUTTON_ADVANCED_SETTINGS),
		cmbTo(this, IDC_COMBO_CONVERT_INTO),
		cmbFrom(this, IDC_COMBO_CONVERT_FROM)
	{
		btnAdd.OnClick = Callback(this, &COptionsDlg::onClick_Add);
		btnDescr.OnClick = Callback(this, &COptionsDlg::onClick_Descr);
		btnRemove.OnClick = Callback(this, &COptionsDlg::onClick_Remove);
		btnAdvanced.OnClick = Callback(this, &COptionsDlg::onClick_Advanced);

		cmbTo.OnSelChanged = cmbFrom.OnSelChanged = Callback(this, &COptionsDlg::onSelChange_From);
		m_list.OnSelChange = Callback(this, &COptionsDlg::onSelChange_Rates);
	}

	bool OnInitDialog() override
	{
		// set contact list display format
		::SetDlgItemTextW(m_hwnd, IDC_EDIT_CONTACT_LIST_FORMAT, g_plugin.getMStringW(DB_KEY_DisplayNameFormat, DB_DEF_DisplayNameFormat));

		// set status message display format
		::SetDlgItemTextW(m_hwnd, IDC_EDIT_STATUS_MESSAGE_FORMAT, g_plugin.getMStringW(DB_KEY_StatusMsgFormat, DB_DEF_StatusMsgFormat));

		// set tendency format
		::SetDlgItemTextW(m_hwnd, IDC_EDIT_TENDENCY_FORMAT, g_plugin.getMStringW(DB_KEY_TendencyFormat, DB_DEF_TendencyFormat));

		// set api key
		::SetDlgItemTextW(m_hwnd, IDC_EDIT_PERSONAL_KEY, g_plugin.getMStringW(DB_KEY_ApiKey));

		// refresh rate
		HWND hwndCombo = ::GetDlgItem(m_hwnd, IDC_COMBO_REFRESH_RATE);
		LPCTSTR pszRefreshRateTypes[] = { TranslateT("Seconds"), TranslateT("Minutes"), TranslateT("Hours") };
		for (int i = 0; i < _countof(pszRefreshRateTypes); ++i)
			::SendMessage(hwndCombo, CB_ADDSTRING, 0, LPARAM(pszRefreshRateTypes[i]));

		int nRefreshRateType = g_plugin.getWord(DB_KEY_RefreshRateType, RRT_MINUTES);
		if (nRefreshRateType < RRT_SECONDS || nRefreshRateType > RRT_HOURS)
			nRefreshRateType = RRT_MINUTES;

		UINT nRate = g_plugin.getWord(DB_KEY_RefreshRateValue, 1);
		switch (nRefreshRateType) {
		case RRT_SECONDS:
		case RRT_MINUTES:
			if (nRate < 1 || nRate > 60)
				nRate = 1;

			spin_set_range(::GetDlgItem(m_hwnd, IDC_SPIN_REFRESH_RATE), 1, 60);
			break;
		case RRT_HOURS:
			if (nRate < 1 || nRate > 24)
				nRate = 1;

			spin_set_range(::GetDlgItem(m_hwnd, IDC_SPIN_REFRESH_RATE), 1, 24);
			break;
		}

		::SendMessage(hwndCombo, CB_SETCURSEL, nRefreshRateType, 0);
		::SetDlgItemInt(m_hwnd, IDC_EDIT_REFRESH_RATE, nRate, FALSE);

		g_aWatchedRates.clear();

		CCurrencyRateSection rSection;
		const auto &rCurrencyRates = m_pProvider->GetCurrencyRates();
		if (rCurrencyRates.GetSectionCount() > 0) {
			rSection = rCurrencyRates.GetSection(0);
		}

		auto cCurrencyRates = rSection.GetCurrencyRateCount();
		for (auto i = 0u; i < cCurrencyRates; ++i) {
			const auto &rCurrencyRate = rSection.GetCurrencyRate(i);
			CMStringW sName = make_currencyrate_name(rCurrencyRate);
			cmbFrom.AddString(sName);
			cmbTo.AddString(sName);
		}

		for (auto &cc : Contacts(MODULENAME)) {
			CCurrencyRatesProviderBase::TRateInfo ri;
			if (true == m_pProvider->GetWatchedRateInfo(cc, ri)) {
				g_aWatchedRates.push_back(ri);
				CMStringW sRate = make_rate_name(ri);
				m_list.AddString(sRate);
			}
		}

		btnAdd.Disable();
		btnRemove.Disable();
		return true;
	}

	bool OnApply() override
	{
		BOOL bOk = FALSE;
		UINT nRefreshRate = ::GetDlgItemInt(m_hwnd, IDC_EDIT_REFRESH_RATE, &bOk, FALSE);
		ERefreshRateType nType = static_cast<ERefreshRateType>(::SendDlgItemMessage(m_hwnd, IDC_COMBO_REFRESH_RATE, CB_GETCURSEL, 0, 0));

		g_plugin.setWord(DB_KEY_RefreshRateType, nType);
		g_plugin.setWord(DB_KEY_RefreshRateValue, nRefreshRate);

		g_plugin.setWString(DB_KEY_DisplayNameFormat, get_window_text(::GetDlgItem(m_hwnd, IDC_EDIT_CONTACT_LIST_FORMAT)));
		g_plugin.setWString(DB_KEY_StatusMsgFormat, get_window_text(::GetDlgItem(m_hwnd, IDC_EDIT_STATUS_MESSAGE_FORMAT)));
		g_plugin.setWString(DB_KEY_TendencyFormat, get_window_text(::GetDlgItem(m_hwnd, IDC_EDIT_TENDENCY_FORMAT)));
		g_plugin.setWString(DB_KEY_ApiKey, get_window_text(::GetDlgItem(m_hwnd, IDC_EDIT_PERSONAL_KEY)));

		CAdvProviderSettings *pAdvSet = get_adv_settings(m_pProvider, false);
		if (pAdvSet)
			pAdvSet->SaveToDb();

		TWatchedRates aTemp(g_aWatchedRates);
		TWatchedRates aRemove;

		for (auto &cc : Contacts(MODULENAME)) {
			CCurrencyRatesProviderBase::TRateInfo ri;
			if (true == m_pProvider->GetWatchedRateInfo(cc, ri)) {
				auto it = std::find_if(aTemp.begin(), aTemp.end(), [&ri](const auto &other)->bool
					{
						return ((0 == mir_wstrcmpi(ri.first.GetID().c_str(), other.first.GetID().c_str()))
							&& ((0 == mir_wstrcmpi(ri.second.GetID().c_str(), other.second.GetID().c_str()))));
					});
				if (it == aTemp.end()) {
					aRemove.push_back(ri);
				}
				else {
					aTemp.erase(it);
				}
			}
		}

		for (auto &it : aRemove)
			m_pProvider->WatchForRate(it, false);
		for (auto &it : aTemp)
			m_pProvider->WatchForRate(it, true);
		m_pProvider->RefreshSettings();
		return true;
	}

	void OnDestroy() override
	{
		remove_adv_settings(m_pProvider);
	}

	LRESULT DlgProc(UINT msg, WPARAM wParam, LPARAM lParam) override
	{
		if (msg == WM_NOTIFY) {
			LPNMHDR pNMHDR = reinterpret_cast<LPNMHDR>(lParam);
			switch (pNMHDR->code) {
			case PSN_KILLACTIVE:
				BOOL bOk = FALSE;
				UINT nRefreshRate = ::GetDlgItemInt(m_hwnd, IDC_EDIT_REFRESH_RATE, &bOk, FALSE);
				ERefreshRateType nType = static_cast<ERefreshRateType>(::SendDlgItemMessage(m_hwnd, IDC_COMBO_REFRESH_RATE, CB_GETCURSEL, 0, 0));
				switch (nType) {
				case RRT_MINUTES:
				case RRT_SECONDS:
					if (FALSE == bOk || nRefreshRate < 1 || nRefreshRate > 60) {
						prepare_edit_ctrl_for_error(::GetDlgItem(m_hwnd, IDC_EDIT_REFRESH_RATE));
						CurrencyRates_MessageBox(m_hwnd, TranslateT("Enter integer value between 1 and 60."), MB_OK | MB_ICONERROR);
						bOk = FALSE;
					}
					break;
				case RRT_HOURS:
					if (FALSE == bOk || nRefreshRate < 1 || nRefreshRate > 24) {
						prepare_edit_ctrl_for_error(::GetDlgItem(m_hwnd, IDC_EDIT_REFRESH_RATE));
						CurrencyRates_MessageBox(m_hwnd, TranslateT("Enter integer value between 1 and 24."), MB_OK | MB_ICONERROR);
						bOk = FALSE;
					}
					break;
				}

				if (TRUE == bOk) {
					HWND hEdit = ::GetDlgItem(m_hwnd, IDC_EDIT_CONTACT_LIST_FORMAT);
					assert(IsWindow(hEdit));

					CMStringW s = get_window_text(hEdit);
					if (s.IsEmpty()) {
						prepare_edit_ctrl_for_error(hEdit);
						CurrencyRates_MessageBox(m_hwnd, TranslateT("Enter text to display in contact list."), MB_OK | MB_ICONERROR);
						bOk = FALSE;
					}
				}

				::SetWindowLongPtr(m_hwnd, DWLP_MSGRESULT, (TRUE == bOk) ? FALSE : TRUE);
				break;
			}
		}

		return CDlgBase::DlgProc(msg, wParam, lParam);
	}

	void onSelChange_From(CCtrlCombo *)
	{
		int nFrom = cmbFrom.GetCurSel();
		int nTo = cmbTo.GetCurSel();
		btnAdd.Enable((CB_ERR != nFrom) && (CB_ERR != nTo) && (nFrom != nTo));
	}

	void onSelChange_Rates(CCtrlCombo *)
	{
		int nType = m_list.GetCurSel();
		btnRemove.Enable(LB_ERR != nType);

		switch (nType) {
		case RRT_SECONDS:
		case RRT_MINUTES:
			spin_set_range(::GetDlgItem(m_hwnd, IDC_SPIN_REFRESH_RATE), 1, 60);
			break;
		case RRT_HOURS:
			spin_set_range(::GetDlgItem(m_hwnd, IDC_SPIN_REFRESH_RATE), 1, 24);
			BOOL bOk = FALSE;
			UINT nRefreshRate = ::GetDlgItemInt(m_hwnd, IDC_EDIT_REFRESH_RATE, &bOk, FALSE);
			if (TRUE == bOk && nRefreshRate > 24)
				::SetDlgItemInt(m_hwnd, IDC_EDIT_REFRESH_RATE, 24, FALSE);
			break;
		}
	}

	void onClick_Add(CCtrlButton *)
	{
		size_t nFrom = cmbFrom.GetCurSel();
		size_t nTo = cmbTo.GetCurSel();
		if ((CB_ERR != nFrom) && (CB_ERR != nTo) && (nFrom != nTo)) {
			CCurrencyRateSection rSection;
			const auto &rCurrencyRates = m_pProvider->GetCurrencyRates();
			if (rCurrencyRates.GetSectionCount() > 0)
				rSection = rCurrencyRates.GetSection(0);

			auto cCurrencyRates = rSection.GetCurrencyRateCount();
			if ((nFrom < cCurrencyRates) && (nTo < cCurrencyRates)) {
				CCurrencyRatesProviderBase::TRateInfo ri;
				ri.first = rSection.GetCurrencyRate(nFrom);
				ri.second = rSection.GetCurrencyRate(nTo);

				g_aWatchedRates.push_back(ri);

				CMStringW sRate = make_rate_name(ri);
				m_list.AddString(sRate);
				NotifyChange();
			}
		}
	}

	void onClick_Advanced(CCtrlButton *)
	{
		CAdvProviderSettings *pAdvSet = get_adv_settings(m_pProvider, true);
		assert(pAdvSet);
		if (true == ShowSettingsDlg(m_hwnd, pAdvSet))
			NotifyChange();
	}

	void onClick_Descr(CCtrlButton *)
	{
		show_variable_list(m_hwnd, m_pProvider);
	}

	void onClick_Remove(CCtrlButton *)
	{
		int nSel = m_list.GetCurSel();
		if (LB_ERR != nSel) {
			m_list.DeleteString(nSel);
			if (nSel < static_cast<int>(g_aWatchedRates.size())) {
				g_aWatchedRates.erase(g_aWatchedRates.begin() + nSel);
				NotifyChange();
			}
		}

		nSel = m_list.GetCurSel();
		btnRemove.Enable(LB_ERR != nSel);
	}
};

int CurrencyRatesEventFunc_OptInitialise(WPARAM wp, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.position = 910000000;
	odp.szTitle.a = LPGEN("Currency Rates");
	odp.szGroup.a = LPGEN("Network");
	odp.flags = ODPF_USERINFOTAB;

	odp.pDialog = new COptionsDlg();
	odp.szTab.a = LPGEN("General");
	g_plugin.addOptions(wp, &odp);
	return 0;
}
