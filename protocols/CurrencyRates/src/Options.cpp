/*
Copyright (C) 2012-24 Miranda NG team (https://miranda-ng.org)

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

/////////////////////////////////////////////////////////////////////////////////////////
// Pair adding dialog

class CPairAddDialog : public CDlgBase
{
	CCtrlCombo cmbFrom, cmbTo;
	CCtrlButton btnOk;

public:
	CCurrencyRatesProviderBase::TRateInfo ri;

	CPairAddDialog() :
		CDlgBase(g_plugin, IDD_ADD_PAIR),
		btnOk(this, IDOK),
		cmbTo(this, IDC_COMBO_CONVERT_INTO),
		cmbFrom(this, IDC_COMBO_CONVERT_FROM)
	{
		cmbTo.OnSelChanged = cmbFrom.OnSelChanged = Callback(this, &CPairAddDialog::onSelChange_From);
	}

	bool OnInitDialog() override
	{
		auto &rSection = g_pCurrentProvider->GetSection();
		for (auto &it : rSection.GetCurrencyRates()) {
			CMStringW sName = it.MakeName();
			cmbFrom.AddString(TranslateW(sName), LPARAM(&it));
			cmbTo.AddString(TranslateW(sName), LPARAM(&it));
		}

		onSelChange_From(0);
		return true;
	}

	bool OnApply() override
	{
		auto *c1 = (CCurrencyRate*)cmbFrom.GetCurData();
		auto *c2 = (CCurrencyRate*)cmbTo.GetCurData();
		if (c1 && c2) {
			ri.first = *c1;
			ri.second = *c2;
		}
		return true;
	}

	void onSelChange_From(CCtrlCombo *)
	{
		int nFrom = cmbFrom.GetCurSel();
		int nTo = cmbTo.GetCurSel();
		btnOk.Enable((CB_ERR != nFrom) && (CB_ERR != nTo) && (nFrom != nTo));
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// General options dialog

class COptionsDlg : public CDlgBase
{
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

	CCtrlEdit edtKey, edtDisplayFormat, edtStatusFormat, edtTendencyFormat;
	CCtrlCombo cmbProvider, cmbRefresh;
	CCtrlButton btnAdd, btnRemove, btnDescr, btnGetKey;
	CCtrlListBox m_list;

public:
	COptionsDlg() :
		CDlgBase(g_plugin, IDD_OPTIONS_GENERAL),
		m_pProvider(g_pCurrentProvider),
		m_list(this, IDC_LIST_RATES),
		cmbRefresh(this, IDC_COMBO_REFRESH_RATE),
		cmbProvider(this, IDC_PROVIDER),
		btnAdd(this, IDC_BUTTON_ADD),
		btnDescr(this, IDC_BUTTON_DESCRIPTION),
		btnGetKey(this, IDC_GET_KEY),
		btnRemove(this, IDC_BUTTON_REMOVE),
		edtKey(this, IDC_EDIT_PERSONAL_KEY),
		edtDisplayFormat(this, IDC_EDIT_CONTACT_LIST_FORMAT),
		edtStatusFormat(this, IDC_EDIT_STATUS_MESSAGE_FORMAT),
		edtTendencyFormat(this, IDC_EDIT_TENDENCY_FORMAT)
	{
		CreateLink(edtKey, g_plugin.wszApiKey);
		CreateLink(edtStatusFormat, g_plugin.wszStatusFormat);
		CreateLink(edtDisplayFormat, g_plugin.wszDisplayFormat);
		CreateLink(edtTendencyFormat, g_plugin.wszTendencyFormat);

		btnAdd.OnClick = Callback(this, &COptionsDlg::onClick_Add);
		btnDescr.OnClick = Callback(this, &COptionsDlg::onClick_Descr);
		btnGetKey.OnClick = Callback(this, &COptionsDlg::onClick_GetKey);
		btnRemove.OnClick = Callback(this, &COptionsDlg::onClick_Remove);

		m_list.OnSelChange = Callback(this, &COptionsDlg::onSelChange_Rates);

		cmbRefresh.OnSelChanged = Callback(this, &COptionsDlg::onSelChange_Refresh);
		cmbProvider.OnSelChanged = Callback(this, &COptionsDlg::onSelChange_Provider);
	}

	bool OnInitDialog() override
	{
		for (auto &it : g_apProviders) {
			int idx = cmbProvider.AddString(it->GetInfo().m_sName, (LPARAM)it);
			if (it == g_pCurrentProvider)
				cmbProvider.SetCurSel(idx);
		}
		onSelChange_Provider(0);

		// refresh rate
		cmbRefresh.AddString(TranslateT("Seconds"));
		cmbRefresh.AddString(TranslateT("Minutes"));
		cmbRefresh.AddString(TranslateT("Hours"));

		int nRefreshRateType = g_plugin.wRateType;
		if (nRefreshRateType < RRT_SECONDS || nRefreshRateType > RRT_HOURS)
			nRefreshRateType = RRT_MINUTES;

		UINT nRate = g_plugin.wRateValue;
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
		cmbRefresh.SetCurSel(nRefreshRateType);
		::SetDlgItemInt(m_hwnd, IDC_EDIT_REFRESH_RATE, nRate, FALSE);

		g_aWatchedRates.clear();

		for (auto &cc : Contacts(MODULENAME)) {
			CCurrencyRatesProviderBase::TRateInfo ri;
			if (true == m_pProvider->GetWatchedRateInfo(cc, ri)) {
				g_aWatchedRates.push_back(ri);
				CMStringW sRate = make_rate_name(ri);
				m_list.AddString(sRate);
			}
		}

		btnRemove.Disable();
		return true;
	}

	bool OnApply() override
	{
		BOOL bOk = FALSE;
		UINT nRefreshRate = ::GetDlgItemInt(m_hwnd, IDC_EDIT_REFRESH_RATE, &bOk, FALSE);

		g_plugin.wRateType = cmbRefresh.GetCurSel();
		g_plugin.wRateValue = nRefreshRate;

		TWatchedRates aTemp(g_aWatchedRates);
		TWatchedRates aRemove;

		for (auto &cc : Contacts(MODULENAME)) {
			CCurrencyRatesProviderBase::TRateInfo ri;
			if (true == m_pProvider->GetWatchedRateInfo(cc, ri)) {
				auto it = std::find_if(aTemp.begin(), aTemp.end(), [&ri](const auto &other)->bool
					{
						return ((0 == mir_wstrcmpi(ri.first.GetID(), other.first.GetID()))
							&& ((0 == mir_wstrcmpi(ri.second.GetID(), other.second.GetID()))));
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

		g_pCurrentProvider = (CCurrencyRatesProviderBase *)cmbProvider.GetCurData();
		g_plugin.setWString(DB_STR_PROVIDER, g_pCurrentProvider->GetInfo().m_sName);
		return true;
	}

	void onSelChange_Rates(CCtrlListBox *)
	{
		btnRemove.Enable(LB_ERR != m_list.GetCurSel());
	}

	void onSelChange_Provider(CCtrlCombo *)
	{
		auto *pProvider = (CCurrencyRatesProviderBase *)cmbProvider.GetCurData();
		bool bEnabled = pProvider->HasAuth();
		edtKey.Enable(bEnabled);
		btnGetKey.Enable(bEnabled);
	}

	void onSelChange_Refresh(CCtrlCombo *)
	{
		switch (cmbRefresh.GetCurSel()) {
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
		CPairAddDialog dlg;
		dlg.SetParent(m_hwnd);
		if (dlg.DoModal() == IDOK) {
			g_aWatchedRates.push_back(dlg.ri);

			CMStringW sRate = make_rate_name(dlg.ri);
			m_list.AddString(sRate);
			NotifyChange();
		}
	}

	void onClick_Descr(CCtrlButton *)
	{
		show_variable_list(m_hwnd, m_pProvider);
	}

	void onClick_GetKey(CCtrlButton *)
	{
		if (auto *pProvider = (CCurrencyRatesProviderBase *)cmbProvider.GetCurData())
			Utils_OpenUrlW(pProvider->GetInfo().m_sURL);
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

/////////////////////////////////////////////////////////////////////////////////////////
// Advanced settings dialog

void select_log_file(HWND hDlg);
void update_file_controls(HWND hDlg);
void update_history_controls(HWND hDlg);

class CAdvancedOptDlg : public CDlgBase
{
	CCtrlEdit edtHistoryFormat, edtLogFilename, edtLogFormat;
	CCtrlCheck chkExternalFile, chkHistory, chkHistoryCond, chkLogFileCond;
	CCtrlButton btnBrowse, btnDescr1, btnDescr2;

public:
	CAdvancedOptDlg() :
		CDlgBase(g_plugin, IDD_OPTIONS_ADVANCED),
		btnDescr1(this, IDC_BUTTON_HISTORY_DESCRIPTION),
		btnDescr2(this, IDC_BUTTON_LOG_FILE_DESCRIPTION),
		btnBrowse(this, IDC_BUTTON_BROWSE),
		chkHistory(this, IDC_CHECK_INTERNAL_HISTORY),
		chkHistoryCond(this, IDC_CHECK_HISTORY_CONDITION),
		chkLogFileCond(this, IDC_CHECK_LOG_FILE_CONDITION),
		chkExternalFile(this, IDC_CHECK_EXTERNAL_FILE),
		edtLogFormat(this, IDC_EDIT_LOG_FILE_FORMAT),
		edtLogFilename(this, IDC_EDIT_FILE_NAME),
		edtHistoryFormat(this, IDC_EDIT_HISTORY_FORMAT)
	{
		CreateLink(edtLogFormat, g_plugin.wszLogFileFormat);
		CreateLink(edtLogFilename, g_plugin.wszLogFileName);
		CreateLink(edtHistoryFormat, g_plugin.wszHistoryFormat);
		CreateLink(chkHistoryCond, g_plugin.bIsOnlyChangedHistory);
		CreateLink(chkLogFileCond, g_plugin.bIsOnlyChangedLogFile);

		btnDescr1.OnClick = btnDescr2.OnClick = Callback(this, &CAdvancedOptDlg::onClick_Description);
		btnBrowse.OnClick = Callback(this, &CAdvancedOptDlg::onClick_Browse);

		chkHistory.OnChange = Callback(this, &CAdvancedOptDlg::onChange_History);
		chkExternalFile.OnChange = Callback(this, &CAdvancedOptDlg::onChange_ExternalFile);
	}

	bool OnInitDialog() override
	{
		// log to history
		uint16_t dwLogMode = g_plugin.wLogMode;
		chkHistory.SetState((dwLogMode & lmInternalHistory) != 0);

		// log to file
		chkExternalFile.SetState((dwLogMode & lmExternalFile) != 0);
		return true;
	}

	bool OnApply() override
	{
		uint16_t nLogMode = lmDisabled;

		if (chkExternalFile.IsChecked()) nLogMode |= lmExternalFile;
		if (chkHistory.IsChecked())      nLogMode |= lmInternalHistory;

		bool bOk = true;
		ptrW sLogFile(edtLogFilename.GetText());
		ptrW sLogFileFormat(edtLogFormat.GetText());

		if ((nLogMode & lmExternalFile)) {
			if (!mir_wstrlen(sLogFile)) {
				prepare_edit_ctrl_for_error(edtLogFilename.GetHwnd());
				CurrencyRates_MessageBox(m_hwnd, TranslateT("Enter log file name."), MB_OK | MB_ICONERROR);
				bOk = false;
			}
			else if (!mir_wstrlen(sLogFileFormat)) {
				prepare_edit_ctrl_for_error(edtLogFormat.GetHwnd());
				CurrencyRates_MessageBox(m_hwnd, TranslateT("Enter log file format."), MB_OK | MB_ICONERROR);
				bOk = false;
			}
		}

		ptrW sHistoryFormat(edtHistoryFormat.GetText());
		if ((true == bOk) && (nLogMode & lmInternalHistory) && (!mir_wstrlen(sHistoryFormat))) {
			prepare_edit_ctrl_for_error(edtHistoryFormat.GetHwnd());
			CurrencyRates_MessageBox(m_hwnd, TranslateT("Enter history format."), MB_OK | MB_ICONERROR);
			bOk = false;
		}

		if (!bOk)
			return false;

		g_plugin.wLogMode = nLogMode;
		return true;
	}

	void onClick_Description(CCtrlButton *)
	{
		show_variable_list(m_hwnd, g_pCurrentProvider);
	}

	void onChange_ExternalFile(CCtrlCheck *)
	{
		update_file_controls(m_hwnd);
	}

	void onChange_History(CCtrlCheck *)
	{
		update_history_controls(m_hwnd);
	}

	void onClick_Browse(CCtrlButton *)
	{
		select_log_file(m_hwnd);
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// Popup options

class CPopupOptDlg : public CDlgBase
{
	CCtrlEdit edtPopupFormat, edtDelay;
	CCtrlColor clrBack, clrText;
	CCtrlCheck chkPopup, chkChanged, chkHistory;
	CCtrlButton btnPreview;

public:
	CPopupOptDlg() :
		CDlgBase(g_plugin, IDD_OPTIONS_POPUP),
		clrBack(this, IDC_BGCOLOR),
		clrText(this, IDC_TEXTCOLOR),
		chkPopup(this, IDC_CHECK_SHOW_POPUP),
		chkHistory(this, IDC_CHECK_DONT_USE_POPUPHISTORY),
		chkChanged(this, IDC_CHECK_SHOW_POPUP_ONLY_VALUE_CHANGED),
		edtDelay(this, IDC_DELAY),
		edtPopupFormat(this, IDC_EDIT_POPUP_FORMAT),
		btnPreview(this, IDC_PREV)
	{
		CreateLink(chkPopup, g_plugin.bUsePopups);
		CreateLink(chkHistory, g_plugin.bUseHistory);
		CreateLink(chkChanged, g_plugin.bShowPopupIfValueChanged);
		
		CreateLink(clrBack, g_plugin.rgbBkg);
		CreateLink(clrText, g_plugin.rgbText);

		CreateLink(edtDelay, g_plugin.wDelay);
		CreateLink(edtPopupFormat, g_plugin.wszPopupFormat);

		chkPopup.OnChange = Callback(this, &CPopupOptDlg::onChange_Popup);
		btnPreview.OnClick = Callback(this, &CPopupOptDlg::onClick_Preview);
	}

	bool OnInitDialog() override
	{
		::CheckRadioButton(m_hwnd, IDC_RADIO_DEFAULT_COLOURS, IDC_RADIO_USER_DEFINED_COLOURS, (colourDefault == g_plugin.modeColour) ? IDC_RADIO_DEFAULT_COLOURS : IDC_RADIO_USER_DEFINED_COLOURS);
		UINT n;
		switch (g_plugin.modeDelay) {
		case delayFromPopup:
			n = IDC_DELAYFROMPU;
			break;
		case delayCustom:
			n = IDC_DELAYCUSTOM;
			break;
		case delayPermanent:
		default:
			n = IDC_DELAYPERMANENT;
			break;
		}
		::CheckRadioButton(m_hwnd, IDC_DELAYFROMPU, IDC_DELAYPERMANENT, n);
		return true;
	}

	bool OnApply() override
	{
		uint8_t nModeDelay;
		if (1 == ::IsDlgButtonChecked(m_hwnd, IDC_DELAYFROMPU))
			nModeDelay = delayFromPopup;
		else if (1 == ::IsDlgButtonChecked(m_hwnd, IDC_DELAYCUSTOM))
			nModeDelay = delayCustom;
		else
			nModeDelay = delayPermanent;
		g_plugin.modeDelay = nModeDelay;

		if (1 == ::IsDlgButtonChecked(m_hwnd, IDC_RADIO_DEFAULT_COLOURS))
			g_plugin.modeColour = colourDefault;
		else 
			g_plugin.modeColour = colourUserDefined;
		return true;
	}

	void OnChange() override
	{
		bool bIsColoursEnabled = 1 == IsDlgButtonChecked(m_hwnd, IDC_RADIO_USER_DEFINED_COLOURS);
		::EnableWindow(::GetDlgItem(m_hwnd, IDC_BGCOLOR), bIsColoursEnabled);
		::EnableWindow(::GetDlgItem(m_hwnd, IDC_TEXTCOLOR), bIsColoursEnabled);

		bool bIsDelayEnabled = 1 == IsDlgButtonChecked(m_hwnd, IDC_DELAYCUSTOM);
		::EnableWindow(::GetDlgItem(m_hwnd, IDC_DELAY), bIsDelayEnabled);
	}

	void onChange_Popup(CCtrlCheck *)
	{
		bool bEnable = chkPopup.IsChecked();
		::EnableWindow(::GetDlgItem(m_hwnd, IDC_EDIT_POPUP_FORMAT), bEnable);
		::EnableWindow(::GetDlgItem(m_hwnd, IDC_CHECK_SHOW_POPUP_ONLY_VALUE_CHANGED), bEnable);
		::EnableWindow(::GetDlgItem(m_hwnd, IDC_STATIC_POPUP_FORMAT), bEnable);
		::EnableWindow(::GetDlgItem(m_hwnd, IDC_BUTTON_POPUP_FORMAT_DESCRIPTION), bEnable);
	}

	void onClick_Preview(CCtrlButton *)
	{
		show_popup(0, 0, 1, L"");
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// Module entry point

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

	odp.pDialog = new CAdvancedOptDlg();
	odp.szTab.a = LPGEN("Advanced");
	g_plugin.addOptions(wp, &odp);

	odp.position = 910000002;
	odp.szGroup.a = LPGEN("Popups");
	odp.szTab.a = nullptr;
	odp.pDialog = new CPopupOptDlg();
	g_plugin.addOptions(wp, &odp);
	return 0;
}
