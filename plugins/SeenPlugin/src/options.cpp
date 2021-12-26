/*
"Last Seen mod" plugin for Miranda IM
Copyright ( C ) 2002-03  micron-x
Copyright ( C ) 2005-07  Y.B.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or ( at your option ) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "stdafx.h"

struct helpstring
{
	wchar_t letter;
	const wchar_t *pwszHelp;
};

static helpstring section1[] = 
{
	{ 'Y', LPGENW("year (4 digits)") },
	{ 'y', LPGENW("year (2 digits)") },
	{ 'm', LPGENW("month") },
	{ 'E', LPGENW("name of month") },
	{ 'e', LPGENW("short name of month") },
	{ 'd', LPGENW("day") },
	{ 'W', LPGENW("weekday (full)") },
	{ 'w', LPGENW("weekday (abbreviated)") },
};

static helpstring section2[] =
{
	{ 'H', LPGENW("hours (24)") },
	{ 'h', LPGENW("hours (12)") },
	{ 'p', LPGENW("AM/PM") },
	{ 'M', LPGENW("minutes") },
	{ 'S', LPGENW("seconds") },
};

static helpstring section3[] =
{
	{ 'n', LPGENW("username") },
	{ 'N', LPGENW("nick") },
	{ 'u', LPGENW("UIN/handle") },
	{ 'G', LPGENW("group") },
	{ 's', LPGENW("status") },
	{ 'T', LPGENW("status message") },
	{ 'o', LPGENW("old status") },
	{ 'i', LPGENW("external IP") },
	{ 'r', LPGENW("internal IP") },
	{ 'C', LPGENW("client info") },
	{ 'P', LPGENW("protocol") },
	{ 'A', LPGENW("account") },
};

static helpstring section4[] =
{
	{ 't', LPGENW("tabulator") },
	{ 'b', LPGENW("line break") },
};

static void addSection(CMStringW &str, const wchar_t *pwszTitle, const helpstring *strings, int count)
{
	str.Append(TranslateW(pwszTitle)); str.AppendChar('\n');
	
	for (int i=0; i < count; i++)
		str.AppendFormat(L"%%%c: \t %s \n", strings[i].letter, TranslateW(strings[i].pwszHelp));

	str.AppendChar('\n');
}

class CPopupOptsDlg : public CDlgBase
{
	UI_MESSAGE_MAP(CPopupOptsDlg, CDlgBase);
		UI_MESSAGE(WM_COMMAND, OnCommand);
	UI_MESSAGE_MAP_END();

	CCtrlCheck chkPopups;
	CCtrlButton btnReset;

public:
	CPopupOptsDlg() :
		CDlgBase(g_plugin, IDD_POPUPS),
		btnReset(this, IDC_DEFAULTCOL),
		chkPopups(this, IDC_POPUPS)
	{
		CreateLink(chkPopups, g_plugin.bUsePopups);

		btnReset.OnClick = Callback(this, &CPopupOptsDlg::onClick_Reset);
		chkPopups.OnChange = Callback(this, &CPopupOptsDlg::onChange_Popups);
	}

	bool OnInitDialog() override
	{
		int hasPopups = Popup_Enabled();
		ShowWindow(GetDlgItem(m_hwnd, IDC_POPUPSTAMP), hasPopups ? SW_SHOW : SW_HIDE);
		ShowWindow(GetDlgItem(m_hwnd, IDC_LABTEXT), hasPopups ? SW_SHOW : SW_HIDE);
		ShowWindow(GetDlgItem(m_hwnd, IDC_LABTTITLE), hasPopups ? SW_SHOW : SW_HIDE);
		ShowWindow(GetDlgItem(m_hwnd, IDC_POPUPSTAMPTEXT), hasPopups ? SW_SHOW : SW_HIDE);
		chkPopups.Enable(hasPopups);

		for (int i = ID_STATUS_OFFLINE; i <= ID_STATUS_MAX; i++) {
			char szSetting[100];
			mir_snprintf(szSetting, "Col_%d", i - ID_STATUS_OFFLINE);
			uint32_t sett = g_plugin.getDword(szSetting, StatusColors15bits[i - ID_STATUS_OFFLINE]);

			COLORREF back, text;
			GetColorsFromDWord(&back, &text, sett);
			SendDlgItemMessage(m_hwnd, i, CPM_SETCOLOUR, 0, back);
			SendDlgItemMessage(m_hwnd, i + 20, CPM_SETCOLOUR, 0, text);
		}

		SetDlgItemText(m_hwnd, IDC_POPUPSTAMP, g_plugin.getMStringW("PopupStamp", DEFAULT_POPUPSTAMP));
		SetDlgItemText(m_hwnd, IDC_POPUPSTAMPTEXT, g_plugin.getMStringW("PopupStampText", DEFAULT_POPUPSTAMPTEXT));
		return true;
	}

	bool OnApply() override
	{
		wchar_t szstamp[256];
		GetDlgItemText(m_hwnd, IDC_POPUPSTAMP, szstamp, _countof(szstamp));
		g_plugin.setWString("PopupStamp", szstamp);

		GetDlgItemText(m_hwnd, IDC_POPUPSTAMPTEXT, szstamp, _countof(szstamp));
		g_plugin.setWString("PopupStampText", szstamp);

		for (int i = ID_STATUS_OFFLINE; i <= ID_STATUS_MAX; i++) {
			COLORREF back = SendDlgItemMessage(m_hwnd, i, CPM_GETCOLOUR, 0, 0);
			COLORREF text = SendDlgItemMessage(m_hwnd, i + 20, CPM_GETCOLOUR, 0, 0);
			uint32_t sett = GetDWordFromColors(back, text);

			char szSetting[100];
			mir_snprintf(szSetting, "Col_%d", i - ID_STATUS_OFFLINE);
			if (sett != StatusColors15bits[i - ID_STATUS_OFFLINE])
				g_plugin.setDword(szSetting, sett);
			else
				g_plugin.delSetting(szSetting);
		}
		return true;
	}

	LRESULT OnCommand(UINT, WPARAM wParam, LPARAM)
	{
		if (HIWORD(wParam) == CPN_COLOURCHANGED) {
			uint16_t idText, idBack;
			if (LOWORD(wParam) > ID_STATUS_MAX) // we have clicked a text color
				idText = wParam, idBack = wParam - 20;
			else
				idText = wParam + 20, idBack = wParam;

			POPUPDATAW ppd;
			ppd.colorBack = SendDlgItemMessage(m_hwnd, idBack, CPM_GETCOLOUR, 0, 0);
			ppd.colorText = SendDlgItemMessage(m_hwnd, idText, CPM_GETCOLOUR, 0, 0);
			uint32_t temp = GetDWordFromColors(ppd.colorBack, ppd.colorText);
			GetColorsFromDWord(&ppd.colorBack, &ppd.colorText, temp);
			SendDlgItemMessage(m_hwnd, idBack, CPM_SETCOLOUR, 0, ppd.colorBack);
			SendDlgItemMessage(m_hwnd, idText, CPM_SETCOLOUR, 0, ppd.colorText);
			ppd.lchIcon = Skin_LoadProtoIcon(nullptr, idBack);

			wchar_t szstamp[256];
			GetDlgItemText(m_hwnd, IDC_POPUPSTAMP, szstamp, _countof(szstamp));
			wcsncpy(ppd.lpwzContactName, ParseString(szstamp, NULL), MAX_CONTACTNAME);

			GetDlgItemText(m_hwnd, IDC_POPUPSTAMPTEXT, szstamp, _countof(szstamp));
			wcsncpy(ppd.lpwzText, ParseString(szstamp, NULL), MAX_SECONDLINE);

			PUAddPopupW(&ppd);
			NotifyChange();
		}
		return FALSE;
	}

	void onChange_Popups(CCtrlCheck *)
	{
		bool hasPopups = chkPopups.GetState();
		EnableWindow(GetDlgItem(m_hwnd, IDC_POPUPSTAMP), hasPopups);
		EnableWindow(GetDlgItem(m_hwnd, IDC_POPUPSTAMPTEXT), hasPopups);
		for (int i = ID_STATUS_OFFLINE; i <= ID_STATUS_MAX; i++) {
			EnableWindow(GetDlgItem(m_hwnd, i), hasPopups);
			EnableWindow(GetDlgItem(m_hwnd, i + 20), hasPopups);
		}
	}

	void onClick_Reset(CCtrlButton *)
	{
		for (int i = ID_STATUS_OFFLINE; i <= ID_STATUS_MAX; i++) {
			uint32_t sett = StatusColors15bits[i - ID_STATUS_OFFLINE];
			COLORREF back, text;
			GetColorsFromDWord(&back, &text, sett);
			SendDlgItemMessage(m_hwnd, i, CPM_SETCOLOUR, 0, back);
			SendDlgItemMessage(m_hwnd, i + 20, CPM_SETCOLOUR, 0, text);
		}
	}
};

/////////////////////////////////////////////////////////////////////////////////////////

class CMainOptsDlg : public CDlgBase
{
	CCtrlCheck chkFile, chkMissed, chkHistory, chkMenuItem, chkUserInfo;
	CCtrlButton btnVariables;
	CCtrlListView protocols;

public:
	CMainOptsDlg() :
		CDlgBase(g_plugin, IDD_SETTINGS),
		chkFile(this, IDC_FILE),
		chkMissed(this, IDC_MISSEDONES),
		chkHistory(this, IDC_HISTORY),
		chkMenuItem(this, IDC_MENUITEM),
		chkUserInfo(this, IDC_USERINFO),
		protocols(this, IDC_PROTOCOLLIST),
		btnVariables(this, IDC_VARIABLES)
	{
		btnVariables.OnClick = Callback(this, &CMainOptsDlg::onClick_Variables);

		chkFile.OnChange = Callback(this, &CMainOptsDlg::onChanged_File);
		chkMissed.OnChange = Callback(this, &CMainOptsDlg::onChanged_Missed);
		chkHistory.OnChange = Callback(this, &CMainOptsDlg::onChanged_History);
		chkMenuItem.OnChange = Callback(this, &CMainOptsDlg::onChanged_MenuItem);
		chkUserInfo.OnChange = Callback(this, &CMainOptsDlg::onChanged_UserInfo);		
	}

	bool OnInitDialog() override
	{
		chkFile.SetState(g_bFileActive);                           onChanged_File(0);
		chkMissed.SetState(g_plugin.getByte("MissedOnes", 0));     onChanged_Missed(0);
		chkHistory.SetState(g_plugin.getByte("KeepHistory", 0));   onChanged_History(0);
		chkMenuItem.SetState(g_plugin.getByte("MenuItem", 1));     onChanged_MenuItem(0);
		chkUserInfo.SetState(g_plugin.getByte("UserinfoTab", 1));  onChanged_UserInfo(0);

		CheckDlgButton(m_hwnd, IDC_IGNOREOFFLINE, g_plugin.getByte("IgnoreOffline", 1) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_SHOWICON, g_plugin.getByte("ShowIcon", 1) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_COUNT, g_plugin.getByte("MissedOnes_Count", 0) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_IDLESUPPORT, g_plugin.getByte("IdleSupport", 1) ? BST_CHECKED : BST_UNCHECKED);

		SetDlgItemText(m_hwnd, IDC_MENUSTAMP, g_plugin.getMStringW("MenuStamp", DEFAULT_MENUSTAMP));
		SetDlgItemText(m_hwnd, IDC_USERSTAMP, g_plugin.getMStringW("UserStamp", DEFAULT_USERSTAMP));
		SetDlgItemText(m_hwnd, IDC_FILESTAMP, g_plugin.getMStringW("FileStamp", DEFAULT_FILESTAMP));
		SetDlgItemText(m_hwnd, IDC_FILENAME, g_plugin.getMStringW("FileName", DEFAULT_FILENAME));
		SetDlgItemText(m_hwnd, IDC_HISTORYSTAMP, g_plugin.getMStringW("HistoryStamp", DEFAULT_HISTORYSTAMP));

		SetDlgItemInt(m_hwnd, IDC_HISTORYSIZE, g_plugin.getWord("HistoryMax", 10 - 1) - 1, FALSE);

		// load protocol list
		protocols.SetExtendedListViewStyle(LVS_EX_CHECKBOXES);

		RECT rc;
		GetClientRect(protocols.GetHwnd(), &rc);

		LVCOLUMN lvc = {};
		lvc.mask = LVCF_FMT | LVCF_WIDTH;
		lvc.fmt = LVCFMT_IMAGE | LVCFMT_LEFT;
		lvc.cx = 120;
		protocols.InsertColumn(0, &lvc);

		for (auto &pa : Accounts().rev_iter()) {
			if (CallProtoService(pa->szModuleName, PS_GETCAPS, PFLAGNUM_2, 0) == 0)
				continue;

			int idx = protocols.AddItem(pa->tszAccountName, 0, (LPARAM)mir_strdup(pa->szModuleName));
			if (IsWatchedProtocol(pa->szModuleName))
				protocols.SetCheckState(idx, true);
		}
		return true;
	}

	bool OnApply() override
	{
		wchar_t szstamp[256];
		GetDlgItemText(m_hwnd, IDC_MENUSTAMP, szstamp, _countof(szstamp));
		g_plugin.setWString("MenuStamp", szstamp);

		GetDlgItemText(m_hwnd, IDC_USERSTAMP, szstamp, _countof(szstamp));
		g_plugin.setWString("UserStamp", szstamp);

		GetDlgItemText(m_hwnd, IDC_FILESTAMP, szstamp, _countof(szstamp));
		g_plugin.setWString("FileStamp", szstamp);

		GetDlgItemText(m_hwnd, IDC_FILENAME, szstamp, _countof(szstamp));
		g_plugin.setWString("FileName", szstamp);

		GetDlgItemText(m_hwnd, IDC_HISTORYSTAMP, szstamp, _countof(szstamp));
		g_plugin.setWString("HistoryStamp", szstamp);

		g_plugin.setWord("HistoryMax", (uint16_t)(GetDlgItemInt(m_hwnd, IDC_HISTORYSIZE, nullptr, FALSE) + 1));

		if (chkMenuItem.IsChanged()) {
			bool bchecked = chkMenuItem.GetState();
			g_plugin.setByte("MenuItem", bchecked);
			if (hmenuitem == nullptr && bchecked)
				InitMenuitem();
		}
		
		if (chkUserInfo.IsChanged()) {
			bool bchecked = chkUserInfo.GetState();
			g_plugin.setByte("UserinfoTab", bchecked);
			if (bchecked)
				ehuserinfo = HookEvent(ME_USERINFO_INITIALISE, UserinfoInit);
			else
				UnhookEvent(ehuserinfo);
		}

		if (chkFile.IsChanged()) {
			bool bchecked = chkFile.GetState();
			if (g_bFileActive != bchecked) {
				g_bFileActive = bchecked;
				g_plugin.setByte("FileOutput", bchecked);
				if (bchecked)
					InitFileOutput();
				else
					UninitFileOutput();
			}
		}

		g_plugin.setByte("KeepHistory", chkHistory.GetState());
		g_plugin.setByte("IgnoreOffline", IsDlgButtonChecked(m_hwnd, IDC_IGNOREOFFLINE));

		if (chkMissed.IsChanged()) {
			bool bchecked = chkMissed.GetState();
			g_plugin.setByte("MissedOnes", bchecked);
			if (bchecked)
				ehmissed_proto = HookEvent(ME_PROTO_ACK, ModeChange_mo);
			else {
				UnhookEvent(ehmissed_proto);
				ehmissed_proto = nullptr;
			}
		}

		g_plugin.setByte("ShowIcon", IsDlgButtonChecked(m_hwnd, IDC_SHOWICON));
		g_plugin.setByte("MissedOnes_Count", IsDlgButtonChecked(m_hwnd, IDC_COUNT));
		g_plugin.setByte("IdleSupport", IsDlgButtonChecked(m_hwnd, IDC_IDLESUPPORT));

		// save protocol list
		CMStringA watchedProtocols;

		int nItems = protocols.GetItemCount();
		for (int i=0; i < nItems; i++) {
			if (!protocols.GetCheckState(i))
				continue;

			char *szProto = (char *)protocols.GetItemData(i);
			if (!watchedProtocols.IsEmpty())
				watchedProtocols.AppendChar('\n');
			watchedProtocols.Append(szProto);
		}
		g_plugin.setString("WatchedAccounts", watchedProtocols);

		UnloadWatchedProtos();
		LoadWatchedProtos();
		return true;
	}

	void OnDestroy() override
	{
		// free protocol list 
		int nItems = protocols.GetItemCount();
		for (int i = 0; i < nItems; i++)
			mir_free((void *)(char *)protocols.GetItemData(i));
	}

	void onChanged_MenuItem(CCtrlCheck *)
	{
		bool bChecked = chkMenuItem.GetState();
		EnableWindow(GetDlgItem(m_hwnd, IDC_MENUSTAMP), bChecked);
		EnableWindow(GetDlgItem(m_hwnd, IDC_SHOWICON), bChecked);
	}

	void onChanged_UserInfo(CCtrlCheck *)
	{
		EnableWindow(GetDlgItem(m_hwnd, IDC_USERSTAMP), chkUserInfo.GetState());
	}

	void onChanged_File(CCtrlCheck *)
	{
		bool bChecked = chkFile.GetState();
		EnableWindow(GetDlgItem(m_hwnd, IDC_FILESTAMP), bChecked);
		EnableWindow(GetDlgItem(m_hwnd, IDC_FILENAME), bChecked);
	}

	void onChanged_History(CCtrlCheck *)
	{
		bool bChecked = chkHistory.GetState();
		EnableWindow(GetDlgItem(m_hwnd, IDC_HISTORYSTAMP), bChecked);
		EnableWindow(GetDlgItem(m_hwnd, IDC_HISTORYSIZE), bChecked);
	}

	void onChanged_Missed(CCtrlCheck *)
	{
		EnableWindow(GetDlgItem(m_hwnd, IDC_COUNT), chkMissed.GetState());
	}

	void onClick_Variables(CCtrlButton*) {
		CMStringW str;
		addSection(str, LPGENW("-- Date --"), section1, _countof(section1));
		addSection(str, LPGENW("-- Time --"), section2, _countof(section2));
		addSection(str, LPGENW("-- User --"), section3, _countof(section3));
		addSection(str, LPGENW("-- Format --"), section4, _countof(section4));
		str.AppendFormat(L"%s\t%s\n\t%s", TranslateT("Note:"), TranslateT("Use # for empty string"), TranslateT("instead of <unknown>"));
		MessageBoxW(m_hwnd, str, TranslateT("Last Seen variables"), MB_OK | MB_TOPMOST);
	}
};

int OptionsInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.position = 100000000;
	odp.flags = ODPF_BOLDGROUPS;

	odp.szGroup.a = LPGEN("Contacts");
	odp.szTitle.a = LPGEN("Last seen");
	odp.pDialog = new CMainOptsDlg();
	g_plugin.addOptions(wParam, &odp);

	odp.szGroup.a = LPGEN("Popups");
	odp.szTitle.a = LPGEN("Last seen");
	odp.pDialog = new CPopupOptsDlg();
	g_plugin.addOptions(wParam, &odp);
	return 0;
}
