/*
Copyright (C) 2010 Mataes

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this file; see the file license.txt.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.
*/

#include "stdafx.h"

static int GetUpdateMode()
{
	int UpdateMode = g_plugin.getByte(DB_SETTING_UPDATE_MODE, -1);

	// Check if there is url for custom mode
	if (UpdateMode == UPDATE_MODE_CUSTOM) {
		ptrW url(g_plugin.getWStringA(DB_SETTING_UPDATE_URL));
		if (url == NULL || !wcslen(url)) {
			// No url for custom mode, reset that setting so it will be determined automatically
			g_plugin.delSetting(DB_SETTING_UPDATE_MODE);
			UpdateMode = -1;
		}
	}

	if (UpdateMode < 0 || UpdateMode >= UPDATE_MODE_MAX_VALUE) {
		// Missing or unknown mode, determine correct from version of running core
		char coreVersion[512];
		Miranda_GetVersionText(coreVersion, _countof(coreVersion));
		UpdateMode = (strstr(coreVersion, "alpha") == nullptr) ? UPDATE_MODE_STABLE : UPDATE_MODE_TRUNK;
	}

	return UpdateMode;
}

wchar_t* GetDefaultUrl()
{
	int bits = g_plugin.bChangePlatform ? DEFAULT_OPP_BITS : DEFAULT_BITS;
	const wchar_t *pwszProto = g_plugin.bUseHttps ? L"https" : L"http";

	wchar_t url[MAX_PATH];
	switch (GetUpdateMode()) {
	case UPDATE_MODE_STABLE:
		mir_snwprintf(url, DEFAULT_UPDATE_URL, pwszProto, bits);
		return mir_wstrdup(url);
	case UPDATE_MODE_STABLE_SYMBOLS:
		mir_snwprintf(url, DEFAULT_UPDATE_URL_STABLE_SYMBOLS, pwszProto, bits);
		return mir_wstrdup(url);
	case UPDATE_MODE_TRUNK:
		mir_snwprintf(url, DEFAULT_UPDATE_URL_TRUNK, pwszProto, bits);
		return mir_wstrdup(url);
	case UPDATE_MODE_TRUNK_SYMBOLS:
		mir_snwprintf(url, DEFAULT_UPDATE_URL_TRUNK_SYMBOLS, pwszProto, bits);
		return mir_wstrdup(url);
	default:
		return g_plugin.getWStringA(DB_SETTING_UPDATE_URL);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// Plugin updater's options

class COptionsDlg : public CDlgBase
{
	CCtrlSpin spinBackups;
	CCtrlCombo cmbPeriod;
	CCtrlCheck chkPeriod, chkStable, chkStableSym, chkTrunk, chkTrunkSym, chkCustom;
	CCtrlCheck chkHttps, chkPlatform, chkStartup, chkAutoRestart, chkOnlyOnce, chkBackup, chkSilent;

	const wchar_t* GetHttps()
	{
		return chkHttps.GetState() ? L"https" : L"http";
	}

	int GetBits()
	{
		return chkPlatform.GetState() ? DEFAULT_OPP_BITS : DEFAULT_BITS;
	}

	void UpdateUrl()
	{
		wchar_t defurl[MAX_PATH];
		if (chkStable.GetState()) {
			mir_snwprintf(defurl, DEFAULT_UPDATE_URL, GetHttps(), GetBits());
			SetDlgItemText(m_hwnd, IDC_CUSTOMURL, defurl);
		}
		else if (chkStableSym.GetState()) {
			mir_snwprintf(defurl, DEFAULT_UPDATE_URL_STABLE_SYMBOLS, GetHttps(), GetBits());
			SetDlgItemText(m_hwnd, IDC_CUSTOMURL, defurl);
		}
		else if (chkTrunk.GetState()) {
			mir_snwprintf(defurl, DEFAULT_UPDATE_URL_TRUNK, GetHttps(), GetBits());
			SetDlgItemText(m_hwnd, IDC_CUSTOMURL, defurl);
		}
		else if (chkTrunkSym.GetState()) {
			mir_snwprintf(defurl, DEFAULT_UPDATE_URL_TRUNK_SYMBOLS, GetHttps(), GetBits());
			SetDlgItemText(m_hwnd, IDC_CUSTOMURL, defurl);
		}
	}

public:
	COptionsDlg() :
		CDlgBase(g_plugin, IDD_OPT_UPDATENOTIFY),
		cmbPeriod(this, IDC_PERIODMEASURE),
		chkHttps(this, IDC_USE_HTTPS),
		chkBackup(this, IDC_BACKUP),
		chkSilent(this, IDC_SILENTMODE),
		chkPeriod(this, IDC_UPDATEONPERIOD),
		chkStartup(this, IDC_UPDATEONSTARTUP),
		chkPlatform(this, IDC_CHANGE_PLATFORM),
		chkOnlyOnce(this, IDC_ONLYONCEADAY),
		chkAutoRestart(this, IDC_AUTORESTART),

		chkTrunk(this, IDC_TRUNK),
		chkTrunkSym(this, IDC_TRUNK_SYMBOLS),
		chkStable(this, IDC_STABLE),
		chkStableSym(this, IDC_STABLE_SYMBOLS),
		chkCustom(this, IDC_CUSTOM),

		spinBackups(this, IDC_BACKUPS_SPIN, 10, 1)
	{
		CreateLink(chkHttps, g_plugin.bUseHttps);
		CreateLink(chkBackup, g_plugin.bBackup);
		CreateLink(chkPeriod, g_plugin.bUpdateOnPeriod);
		CreateLink(chkSilent, g_plugin.bSilentMode);
		CreateLink(chkStartup, g_plugin.bUpdateOnStartup);
		CreateLink(chkOnlyOnce, g_plugin.bOnlyOnceADay);
		CreateLink(chkAutoRestart, g_plugin.bAutoRestart);
		CreateLink(spinBackups, g_plugin.iNumberBackups);

		chkPlatform.OnChange = chkHttps.OnChange = Callback(this, &COptionsDlg::onChange_Url);
		chkPeriod.OnChange = Callback(this, &COptionsDlg::onChange_Period);
		chkStartup.OnChange = Callback(this, &COptionsDlg::onChange_Startup);

		chkTrunk.OnChange = Callback(this, &COptionsDlg::onChange_Trunk);
		chkTrunkSym.OnChange = Callback(this, &COptionsDlg::onChange_TrunkSymbols);
		chkStable.OnChange = Callback(this, &COptionsDlg::onChange_Stable);
		chkStableSym.OnChange = Callback(this, &COptionsDlg::onChange_StableSymbols);
		chkCustom.OnChange = Callback(this, &COptionsDlg::onChange_Custom);
	}

	bool OnInitDialog() override
	{
		chkPlatform.SetState(g_plugin.bChangePlatform);

		if (g_plugin.getByte(DB_SETTING_NEED_RESTART, 0))
			ShowWindow(GetDlgItem(m_hwnd, IDC_NEEDRESTARTLABEL), SW_SHOW);

		SendDlgItemMessage(m_hwnd, IDC_PERIODSPIN, UDM_SETRANGE, 0, MAKELONG(99, 1));
		SendDlgItemMessage(m_hwnd, IDC_PERIODSPIN, UDM_SETPOS, 0, (LPARAM)g_plugin.iPeriod);

		if (ServiceExists(MS_AB_BACKUP)) {
			chkBackup.Enable();
			chkBackup.SetText(TranslateT("Backup database before update"));
		}

		Edit_LimitText(GetDlgItem(m_hwnd, IDC_PERIOD), 2);

		if (g_plugin.getByte(DB_SETTING_DONT_SWITCH_TO_STABLE, 0)) {
			chkStable.Disable();
			
			// Reset setting if needed
			int UpdateMode = g_plugin.getByte(DB_SETTING_UPDATE_MODE, UPDATE_MODE_STABLE);
			if (UpdateMode == UPDATE_MODE_STABLE)
				g_plugin.setByte(DB_SETTING_UPDATE_MODE, UPDATE_MODE_TRUNK);
			chkStable.SetText(LPGENW("Stable version (incompatible with current development version)"));
		}
		TranslateDialogDefault(m_hwnd);

		cmbPeriod.AddString(TranslateT("hours"), 0);
		cmbPeriod.AddString(TranslateT("days"), 1);
		cmbPeriod.SetCurSel(g_plugin.iPeriodMeasure);

		switch (GetUpdateMode()) {
		case UPDATE_MODE_STABLE:
			chkStable.SetState(true);
			UpdateUrl();
			break;
		case UPDATE_MODE_STABLE_SYMBOLS:
			chkStableSym.SetState(true);
			UpdateUrl();
			break;
		case UPDATE_MODE_TRUNK:
			chkTrunk.SetState(true);
			UpdateUrl();
			break;
		case UPDATE_MODE_TRUNK_SYMBOLS:
			chkTrunkSym.SetState(true);
			UpdateUrl();
			break;
		default:
			chkCustom.SetState(true);
			EnableWindow(GetDlgItem(m_hwnd, IDC_CUSTOMURL), TRUE);
			chkPlatform.Disable();

			ptrW url(g_plugin.getWStringA(DB_SETTING_UPDATE_URL));
			if (url == NULL)
				url = GetDefaultUrl();
			SetDlgItemText(m_hwnd, IDC_CUSTOMURL, url);
		}

#ifndef _WIN64
		chkPlatform.SetText(TranslateT("Change platform to 64-bit"));
		{
			BOOL bIsWow64 = FALSE;
			IsWow64Process(GetCurrentProcess(), &bIsWow64);
			if (!bIsWow64)
				chkPlatform.Hide();
		}
#endif
		return true;
	}

	bool OnApply() override
	{
		g_plugin.iPeriodMeasure = cmbPeriod.GetCurData();

		wchar_t buffer[3] = { 0 };
		Edit_GetText(GetDlgItem(m_hwnd, IDC_PERIOD), buffer, _countof(buffer));
		g_plugin.iPeriod = _wtoi(buffer);

		g_plugin.InitTimer(0);

		int iNewMode;
		bool bNoSymbols = false;
		if (chkStable.GetState()) {
			iNewMode = UPDATE_MODE_STABLE;
			bNoSymbols = true;
		}
		else if (chkStableSym.GetState()) {
			iNewMode = UPDATE_MODE_STABLE_SYMBOLS;
		}
		else if (chkTrunk.GetState()) {
			iNewMode = UPDATE_MODE_TRUNK;
			bNoSymbols = true;
		}
		else if (chkTrunkSym.GetState()) {
			iNewMode = UPDATE_MODE_TRUNK_SYMBOLS;
		}
		else {
			wchar_t wszUrl[100];
			GetDlgItemText(m_hwnd, IDC_CUSTOMURL, wszUrl, _countof(wszUrl));
			g_plugin.setWString(DB_SETTING_UPDATE_URL, wszUrl);
			iNewMode = UPDATE_MODE_CUSTOM;
		}

		bool bStartUpdate = false;

		// Repository was changed, force recheck
		if (g_plugin.getByte(DB_SETTING_UPDATE_MODE, UPDATE_MODE_STABLE) != iNewMode) {
			g_plugin.setByte(DB_SETTING_UPDATE_MODE, iNewMode);
			if (!bNoSymbols)
				g_plugin.bForceRedownload = true;
			bStartUpdate = true;
		}

		if (chkPlatform.GetState()) {
			g_plugin.bForceRedownload = bStartUpdate = true;
			g_plugin.setByte(DB_SETTING_CHANGEPLATFORM, g_plugin.bChangePlatform = 1);
		}
		else g_plugin.setByte(DB_SETTING_CHANGEPLATFORM, g_plugin.bChangePlatform = 0);

		// if user selected update channel without symbols, remove PDBs
		if (bNoSymbols) {
			CMStringW wszPath(VARSW(L"%miranda_path%"));
			wszPath.AppendChar('\\');

			WIN32_FIND_DATA ffd;
			HANDLE hFind = FindFirstFile(wszPath + L"*.pdb", &ffd);
			if (hFind != INVALID_HANDLE_VALUE) {
				do {
					DeleteFileW(wszPath + ffd.cFileName);
				} while (FindNextFile(hFind, &ffd) != 0);
			}
			FindClose(hFind);
		}

		if (PU::PrepareEscalation())
			RemoveBackupFolders();

		// if user tried to change the channel, run the update dialog immediately
		if (bStartUpdate) {
			Netlib_Log(g_hNetlibUser, "Platform changed, let's check for updates");
			DoCheck(false);
		}

		return true;
	}

	void onChange_Startup(CCtrlCheck *)
	{
		chkOnlyOnce.Enable(chkStartup.GetState());
	}

	void onChange_Period(CCtrlCheck *)
	{
		bool value = chkPeriod.GetState();
		EnableWindow(GetDlgItem(m_hwnd, IDC_PERIOD), value);
		EnableWindow(GetDlgItem(m_hwnd, IDC_PERIODSPIN), value);
		cmbPeriod.Enable(value);
	}

	void onChange_TrunkSymbols(CCtrlCheck *)
	{
		chkPlatform.Enable();
		EnableWindow(GetDlgItem(m_hwnd, IDC_CUSTOMURL), FALSE);
		UpdateUrl();
	}

	void onChange_Trunk(CCtrlCheck *)
	{
		chkPlatform.Enable();
		EnableWindow(GetDlgItem(m_hwnd, IDC_CUSTOMURL), FALSE);
		UpdateUrl();
	}

	void onChange_Stable(CCtrlCheck *)
	{
		chkPlatform.Enable();
		EnableWindow(GetDlgItem(m_hwnd, IDC_CUSTOMURL), FALSE);
		UpdateUrl();
	}

	void onChange_StableSymbols(CCtrlCheck *)
	{
		chkPlatform.Enable();
		EnableWindow(GetDlgItem(m_hwnd, IDC_CUSTOMURL), FALSE);
		UpdateUrl();
	}

	void onChange_Custom(CCtrlCheck *)
	{
		chkPlatform.Disable();
		EnableWindow(GetDlgItem(m_hwnd, IDC_CUSTOMURL), TRUE);
		{
			ptrW url(g_plugin.getWStringA(DB_SETTING_UPDATE_URL));
			if (url == NULL)
				url = GetDefaultUrl();
			SetDlgItemText(m_hwnd, IDC_CUSTOMURL, url);
		}
	}

	void onChange_Url(CCtrlCheck *)
	{
		UpdateUrl();
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// Popup options

struct
{
	wchar_t *Text;
	int Action;
}
static PopupActions[] =
{
	LPGENW("Close popup"), PCA_CLOSEPOPUP,
	LPGENW("Do nothing"), PCA_DONOTHING
};

class CPopupOptDlg : public CDlgBase
{
	CCtrlCheck chOwnColors, chkWinColors, chkPopupColors, chkErrors, chkInfo;
	CCtrlButton btnPreview;

public:
	CPopupOptDlg() :
		CDlgBase(g_plugin, IDD_POPUP),
		btnPreview(this, IDC_PREVIEW),
		chkInfo(this, IDC_INFO_MESSAGES),
		chkErrors(this, IDC_ERRORS),
		chOwnColors(this, IDC_USEOWNCOLORS),
		chkWinColors(this, IDC_USEWINCOLORS),
		chkPopupColors(this, IDC_USEPOPUPCOLORS)
	{
		btnPreview.OnClick = Callback(this, &CPopupOptDlg::onClick_Preview);

		chkInfo.OnChange = Callback(this, &CPopupOptDlg::onChange_Info);
		chkErrors.OnChange = Callback(this, &CPopupOptDlg::onChange_Errors);

		chOwnColors.OnChange = Callback(this, &CPopupOptDlg::onChange_OwnColors);
		chkWinColors.OnChange = Callback(this, &CPopupOptDlg::onChange_WinColors);
		chkPopupColors.OnChange = Callback(this, &CPopupOptDlg::onChange_PopupColors);
	}

	bool OnInitDialog() override
	{
		//Colors
		if (g_plugin.PopupDefColors == byCOLOR_OWN)
			chkWinColors.SetState(true);
		else if (g_plugin.PopupDefColors == byCOLOR_WINDOWS)
			chOwnColors.SetState(true);
		else if (g_plugin.PopupDefColors == byCOLOR_POPUP)
			chkPopupColors.SetState(true);

		for (auto &it : PopupsList) {
			SendDlgItemMessage(m_hwnd, it.ctrl2, CPM_SETCOLOUR, 0, it.colorText);
			SendDlgItemMessage(m_hwnd, it.ctrl3, CPM_SETCOLOUR, 0, it.colorBack);
			EnableWindow(GetDlgItem(m_hwnd, it.ctrl2), g_plugin.PopupDefColors == byCOLOR_OWN);
			EnableWindow(GetDlgItem(m_hwnd, it.ctrl3), g_plugin.PopupDefColors == byCOLOR_OWN);
		}

		// Timeout
		SendDlgItemMessage(m_hwnd, IDC_TIMEOUT_VALUE, EM_LIMITTEXT, 4, 0);
		SendDlgItemMessage(m_hwnd, IDC_TIMEOUT_VALUE_SPIN, UDM_SETRANGE32, -1, 9999);
		SetDlgItemInt(m_hwnd, IDC_TIMEOUT_VALUE, g_plugin.PopupTimeout, TRUE);

		// Mouse actions
		for (auto &it : PopupActions) {
			SendDlgItemMessage(m_hwnd, IDC_LC, CB_SETITEMDATA, SendDlgItemMessage(m_hwnd, IDC_LC, CB_ADDSTRING, 0, (LPARAM)TranslateW(it.Text)), it.Action);
			SendDlgItemMessage(m_hwnd, IDC_RC, CB_SETITEMDATA, SendDlgItemMessage(m_hwnd, IDC_RC, CB_ADDSTRING, 0, (LPARAM)TranslateW(it.Text)), it.Action);
		}
		SendDlgItemMessage(m_hwnd, IDC_LC, CB_SETCURSEL, g_plugin.PopupLeftClickAction, 0);
		SendDlgItemMessage(m_hwnd, IDC_RC, CB_SETCURSEL, g_plugin.PopupRightClickAction, 0);

		// Popups notified
		for (auto &it : PopupsList) {
			char str[20] = { 0 };
			mir_snprintf(str, "Popups%d", int(&it - PopupsList));
			CheckDlgButton(m_hwnd, it.ctrl1, (g_plugin.getByte(str, DEFAULT_POPUP_ENABLED)) ? BST_CHECKED : BST_UNCHECKED);
		}
		return true;
	}

	bool OnApply() override
	{
		for (auto &it : PopupsList) {
			int i = int(&it - PopupsList);
			char szSetting[20] = { 0 };
			mir_snprintf(szSetting, "Popups%d", i);
			g_plugin.setByte(szSetting, (uint8_t)(IsDlgButtonChecked(m_hwnd, it.ctrl1)));

			mir_snprintf(szSetting, "Popups%iTx", i);
			g_plugin.setDword(szSetting, it.colorText = SendDlgItemMessage(m_hwnd, it.ctrl2, CPM_GETCOLOUR, 0, 0));

			mir_snprintf(szSetting, "Popups%iBg", i);
			g_plugin.setDword(szSetting, it.colorBack = SendDlgItemMessage(m_hwnd, it.ctrl3, CPM_GETCOLOUR, 0, 0));
		}

		g_plugin.PopupTimeout = GetDlgItemInt(m_hwnd, IDC_TIMEOUT_VALUE, nullptr, TRUE);
		g_plugin.PopupLeftClickAction = (uint8_t)SendDlgItemMessage(m_hwnd, IDC_LC, CB_GETCURSEL, 0, 0);
		g_plugin.PopupRightClickAction = (uint8_t)SendDlgItemMessage(m_hwnd, IDC_RC, CB_GETCURSEL, 0, 0);

		if (chkWinColors.GetState())
			g_plugin.PopupDefColors = byCOLOR_OWN;
		else if (chOwnColors.GetState())
			g_plugin.PopupDefColors = byCOLOR_WINDOWS;
		else
			g_plugin.PopupDefColors = byCOLOR_POPUP;
		return true;
	}

	void OnReset() override
	{
		// Restore the options stored in memory.
		InitPopupList();
	}

	INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam) override
	{
		if (msg == WM_COMMAND) {
			uint16_t idCtrl = LOWORD(wParam), wNotifyCode = HIWORD(wParam);
			if (wNotifyCode == CPN_COLOURCHANGED) {
				if (idCtrl > 40070) {
					//It's a color picker change. idCtrl is the control id.
					COLORREF color = SendDlgItemMessage(m_hwnd, idCtrl, CPM_GETCOLOUR, 0, 0);
					int ctlID = idCtrl;
					if ((ctlID > 41070) && (ctlID < 42070)) //It's 41071 or above => Text color.
						PopupsList[ctlID - 41071].colorText = color;
					else if (ctlID > 42070)//Background color.
						PopupsList[ctlID - 42071].colorBack = color;
				}

				NotifyChange();
				return TRUE;
			}
		}

		return CDlgBase::DlgProc(msg, wParam, lParam);
	}

	void onChange_OwnColors(CCtrlCheck *)
	{
		for (auto &it : PopupsList) {
			EnableWindow(GetDlgItem(m_hwnd, it.ctrl2), TRUE); //Text
			EnableWindow(GetDlgItem(m_hwnd, it.ctrl3), TRUE); //Background
		}
	}

	void onChange_WinColors(CCtrlCheck *)
	{
		// Use Windows colors
		for (auto &it : PopupsList) {
			EnableWindow(GetDlgItem(m_hwnd, it.ctrl2), FALSE); //Text
			EnableWindow(GetDlgItem(m_hwnd, it.ctrl3), FALSE); //Background
		}
	}

	void onChange_PopupColors(CCtrlCheck *)
	{
		// Use Popup colors
		for (auto &it : PopupsList) {
			EnableWindow(GetDlgItem(m_hwnd, it.ctrl2), FALSE); //Text
			EnableWindow(GetDlgItem(m_hwnd, it.ctrl3), FALSE); //Background
		}
	}

	void onClick_Preview(CCtrlButton *)
	{
		LPCTSTR Title = TranslateT("Plugin Updater");
		LPCTSTR Text = TranslateT("Test");
		for (auto &it : PopupsList)
			if (IsDlgButtonChecked(m_hwnd, it.ctrl1))
				ShowPopup(Title, Text, int(&it - PopupsList));
	}

	void onChange_Errors(CCtrlCheck *)
	{
		EnableWindow(GetDlgItem(m_hwnd, IDC_ERRORS_MSG), !chkErrors.GetState());
	}
	
	void onChange_Info(CCtrlCheck *)
	{
		EnableWindow(GetDlgItem(m_hwnd, IDC_INFO_MESSAGES_MSG), !chkInfo.GetState());
	}
};

int OptInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.position = 100000000;
	odp.flags = ODPF_BOLDGROUPS | ODPF_UNICODE;

	odp.szGroup.w = LPGENW("Services");
	odp.szTitle.w = LPGENW("Plugin Updater");
	odp.pDialog = new COptionsDlg();
	g_plugin.addOptions(wParam, &odp);

	odp.szGroup.w = LPGENW("Popups");
	odp.szTitle.w = LPGENW("Plugin Updater");
	odp.pDialog = new CPopupOptDlg();
	g_plugin.addOptions(wParam, &odp);
	return 0;
}
