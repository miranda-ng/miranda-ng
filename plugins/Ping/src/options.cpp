#include "stdafx.h"

PingOptions options;

/////////////////////////////////////////////////////////////////////////////////////////
// host edit

PINGLIST temp_list;
PINGADDRESS add_edit_addr;

INT_PTR CALLBACK DlgProcDestEdit(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		for (int i = ID_STATUS_OFFLINE; i <= ID_STATUS_MAX; i++) {
			wchar_t *ret = Clist_GetStatusModeDescription(i, 0);
			SendDlgItemMessage(hwndDlg, IDC_COMBO_DESTSTAT, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)ret);
			SendDlgItemMessage(hwndDlg, IDC_COMBO_DESTSTAT2, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)ret);
		}

		SendDlgItemMessage(hwndDlg, IDC_COMBO_DESTSTAT, CB_SETCURSEL, 1, 0);
		SendDlgItemMessage(hwndDlg, IDC_COMBO_DESTSTAT2, CB_SETCURSEL, 0, 0);

		SetDlgItemText(hwndDlg, IDC_ED_DESTADDR, add_edit_addr.pszName);
		SetDlgItemText(hwndDlg, IDC_ED_DESTLAB, add_edit_addr.pszLabel);
		SetDlgItemText(hwndDlg, IDC_ED_COMMAND, add_edit_addr.pszCommand);
		SetDlgItemText(hwndDlg, IDC_ED_PARAMS, add_edit_addr.pszParams);

		CheckDlgButton(hwndDlg, IDC_CHK_DESTTCP, add_edit_addr.port != -1 ? BST_CHECKED : BST_UNCHECKED);
		if (add_edit_addr.port != -1) {
			EnableWindow(GetDlgItem(hwndDlg, IDC_ED_DESTPORT), TRUE);
			SetDlgItemInt(hwndDlg, IDC_ED_DESTPORT, add_edit_addr.port, FALSE);
		}

		SendDlgItemMessage(hwndDlg, IDC_COMBO_DESTPROTO, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)TranslateT("<none>"));
		SendDlgItemMessage(hwndDlg, IDC_COMBO_DESTPROTO, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)TranslateT("<all>"));

		for (auto &pa : Accounts())
			SendDlgItemMessage(hwndDlg, IDC_COMBO_DESTPROTO, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)pa->tszAccountName);

		if (add_edit_addr.pszProto[0] == '\0') {
			SendDlgItemMessage(hwndDlg, IDC_COMBO_DESTPROTO, CB_SETCURSEL, 0, 0);
		}
		else {
			SendDlgItemMessage(hwndDlg, IDC_COMBO_DESTPROTO, CB_SELECTSTRING, 0, (LPARAM)add_edit_addr.pszProto);
			EnableWindow(GetDlgItem(hwndDlg, IDC_COMBO_DESTSTAT), TRUE);
			SendDlgItemMessage(hwndDlg, IDC_COMBO_DESTSTAT, CB_SETCURSEL, (WPARAM)(add_edit_addr.set_status - ID_STATUS_OFFLINE), 0);

			EnableWindow(GetDlgItem(hwndDlg, IDC_COMBO_DESTSTAT2), TRUE);
			SendDlgItemMessage(hwndDlg, IDC_COMBO_DESTSTAT2, CB_SETCURSEL, (WPARAM)(add_edit_addr.get_status - ID_STATUS_OFFLINE), 0);
		}

		// ? doesn't work? ?
		SetFocus(GetDlgItem(hwndDlg, IDC_ED_DESTLAB));
		return FALSE;

	case WM_COMMAND:
		if (HIWORD(wParam) == LBN_SELCHANGE && LOWORD(wParam) == IDC_COMBO_DESTPROTO) {
			int sel = SendDlgItemMessage(hwndDlg, IDC_COMBO_DESTPROTO, CB_GETCURSEL, 0, 0);
			if (sel != CB_ERR) {
				EnableWindow(GetDlgItem(hwndDlg, IDC_COMBO_DESTSTAT), sel != 0);
				EnableWindow(GetDlgItem(hwndDlg, IDC_COMBO_DESTSTAT2), sel != 0);
			}
		}

		if (HIWORD(wParam) == BN_CLICKED) {
			switch (LOWORD(wParam)) {
			case IDC_CHK_DESTTCP:
				EnableWindow(GetDlgItem(hwndDlg, IDC_ED_DESTPORT), IsDlgButtonChecked(hwndDlg, IDC_CHK_DESTTCP));
				break;

			case IDOK:
				GetDlgItemText(hwndDlg, IDC_ED_DESTADDR, add_edit_addr.pszName, _countof(add_edit_addr.pszName));
				GetDlgItemText(hwndDlg, IDC_ED_DESTLAB, add_edit_addr.pszLabel, _countof(add_edit_addr.pszLabel));
				GetDlgItemText(hwndDlg, IDC_ED_COMMAND, add_edit_addr.pszCommand, _countof(add_edit_addr.pszCommand));
				GetDlgItemText(hwndDlg, IDC_ED_PARAMS, add_edit_addr.pszParams, _countof(add_edit_addr.pszParams));

				if (SendDlgItemMessage(hwndDlg, IDC_COMBO_DESTPROTO, CB_GETCURSEL, 0, 0) != -1) {
					GetDlgItemTextA(hwndDlg, IDC_COMBO_DESTPROTO, add_edit_addr.pszProto, _countof(add_edit_addr.pszProto));
					if (!mir_strcmp(add_edit_addr.pszProto, Translate("<none>")))
						add_edit_addr.pszProto[0] = '\0';
					else {
						int sel = SendDlgItemMessage(hwndDlg, IDC_COMBO_DESTSTAT, CB_GETCURSEL, 0, 0);
						if (sel != CB_ERR)
							add_edit_addr.set_status = ID_STATUS_OFFLINE + sel;

						sel = SendDlgItemMessage(hwndDlg, IDC_COMBO_DESTSTAT2, CB_GETCURSEL, 0, 0);
						if (sel != CB_ERR)
							add_edit_addr.get_status = ID_STATUS_OFFLINE + sel;
					}
				}
				else add_edit_addr.pszProto[0] = '\0';

				if (IsDlgButtonChecked(hwndDlg, IDC_CHK_DESTTCP)) {
					BOOL tr;
					int port = GetDlgItemInt(hwndDlg, IDC_ED_DESTPORT, &tr, FALSE);
					if (tr) add_edit_addr.port = port;
					else add_edit_addr.port = -1;
				}
				else add_edit_addr.port = -1;

				EndDialog(hwndDlg, IDOK);
				RefreshWindow();
				break;

			case IDCANCEL:
				EndDialog(hwndDlg, IDCANCEL);
				break;
			}
		}

		return TRUE;
	}
	return FALSE;
}

bool Edit(HWND hwnd, PINGADDRESS &addr)
{
	add_edit_addr = addr;
	if (DialogBox(g_plugin.getInst(), MAKEINTRESOURCE(IDD_DIALOG3), hwnd, DlgProcDestEdit) == IDOK) {
		addr = add_edit_addr;
		return true;
	}

	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////
// main ping options 

class CMainOptsDlg : public CDlgBase
{
	CCtrlCheck chkLog;
	CCtrlButton btnViewLog, btnBrowse;

public:
	CMainOptsDlg() :
		CDlgBase(g_plugin, IDD_DIALOG1),
		chkLog(this, IDC_CHK_LOG),
		btnBrowse(this, IDC_BTN_LOGBROWSE),
		btnViewLog(this, IDC_BTN_VIEWLOG)
	{
		chkLog.OnChange = Callback(this, &CMainOptsDlg::onChange_Log);

		btnBrowse.OnClick = Callback(this, &CMainOptsDlg::onClick_Browse);
		btnViewLog.OnClick = Callback(this, &CMainOptsDlg::onClick_ViewLog);
	}

	bool OnInitDialog() override
	{
		if (ServiceExists(MS_CLIST_FRAMES_ADDFRAME))
			EnableWindow(GetDlgItem(m_hwnd, IDC_CHK_ATTACH), FALSE);

		CheckDlgButton(m_hwnd, IDC_CHK_ATTACH, options.attach_to_clist ? BST_CHECKED : BST_UNCHECKED);

		SetDlgItemInt(m_hwnd, IDC_PPM, options.ping_period, FALSE);
		SetDlgItemInt(m_hwnd, IDC_PT, options.ping_timeout, FALSE);
		CheckDlgButton(m_hwnd, IDC_CHECKPOPUP, options.show_popup ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_CHECKPOPUP2, options.show_popup2 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_CHK_BLOCK, options.block_reps ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_CHK_LOG, options.logging ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_CHK_LOGCSV, options.log_csv ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_CHK_NOTESTICON, options.no_test_icon ? BST_CHECKED : BST_UNCHECKED);

		SendDlgItemMessage(m_hwnd, IDC_SP_INDENT, UDM_SETRANGE, 0, (LPARAM)MAKELONG(500, 0));
		SendDlgItemMessage(m_hwnd, IDC_SP_INDENT, UDM_SETPOS, 0, options.indent);
		SendDlgItemMessage(m_hwnd, IDC_SP_ROWHEIGHT, UDM_SETRANGE, 0, (LPARAM)MAKELONG(500, 6));
		SendDlgItemMessage(m_hwnd, IDC_SP_ROWHEIGHT, UDM_SETPOS, 0, options.row_height);

		SetDlgItemInt(m_hwnd, IDC_RPT, options.retries, FALSE);

		SetDlgItemText(m_hwnd, IDC_ED_FILENAME, options.log_filename);
		if (!options.logging) {
			EnableWindow(GetDlgItem(m_hwnd, IDC_ED_FILENAME), FALSE);
			EnableWindow(GetDlgItem(m_hwnd, IDC_BTN_LOGBROWSE), FALSE);
			EnableWindow(GetDlgItem(m_hwnd, IDC_CHK_LOGCSV), FALSE);
		}
		return true;
	}

	bool OnApply() override
	{
		BOOL trans_success;
		uint32_t new_ping_period = GetDlgItemInt(m_hwnd, IDC_PPM, &trans_success, FALSE);
		if (trans_success)
			options.ping_period = new_ping_period;

		uint32_t new_ping_timeout = GetDlgItemInt(m_hwnd, IDC_PT, &trans_success, FALSE);
		if (trans_success)
			options.ping_timeout = new_ping_timeout;

		options.show_popup = IsDlgButtonChecked(m_hwnd, IDC_CHECKPOPUP) == BST_CHECKED;
		options.show_popup2 = IsDlgButtonChecked(m_hwnd, IDC_CHECKPOPUP2) == BST_CHECKED;
		options.block_reps = IsDlgButtonChecked(m_hwnd, IDC_CHK_BLOCK) == BST_CHECKED;
		options.logging = IsDlgButtonChecked(m_hwnd, IDC_CHK_LOG) == BST_CHECKED;
		options.log_csv = IsDlgButtonChecked(m_hwnd, IDC_CHK_LOGCSV) == BST_CHECKED;
		GetDlgItemText(m_hwnd, IDC_ED_FILENAME, options.log_filename, _countof(options.log_filename));

		options.no_test_icon = IsDlgButtonChecked(m_hwnd, IDC_CHK_NOTESTICON) == BST_CHECKED;

		options.indent = SendDlgItemMessage(m_hwnd, IDC_SP_INDENT, UDM_GETPOS, 0, 0);
		options.row_height = SendDlgItemMessage(m_hwnd, IDC_SP_ROWHEIGHT, UDM_GETPOS, 0, 0);

		uint32_t new_retries = GetDlgItemInt(m_hwnd, IDC_RPT, &trans_success, FALSE);
		if (trans_success)
			options.retries = new_retries;

		bool new_attach = (IsDlgButtonChecked(m_hwnd, IDC_CHK_ATTACH) == BST_CHECKED);
		if (!ServiceExists(MS_CLIST_FRAMES_ADDFRAME) && options.attach_to_clist != new_attach)
			AttachToClist(new_attach);

		options.attach_to_clist = new_attach;

		SaveOptions();

		RefreshWindow();

		Log(L"options changed");

		if (hWakeEvent)
			SetEvent(hWakeEvent);
		return true;
	}

	void onChange_Log(CCtrlButton *)
	{
		bool bEnabled = IsDlgButtonChecked(m_hwnd, IDC_CHK_LOG);
		EnableWindow(GetDlgItem(m_hwnd, IDC_ED_FILENAME), bEnabled);
		EnableWindow(GetDlgItem(m_hwnd, IDC_BTN_LOGBROWSE), bEnabled);
		EnableWindow(GetDlgItem(m_hwnd, IDC_CHK_LOGCSV), bEnabled);
	}

	void onClick_ViewLog(CCtrlButton *)
	{
		CallService(MODULENAME "/ViewLogData", 0, 0);
	}

	void onClick_Browse(CCtrlButton *)
	{
		wchar_t filter[MAX_PATH];
		mir_snwprintf(filter, L"%s%c*.txt%c%s%c*.*%c", TranslateT("Text Files (*.txt)"), 0, 0, TranslateT("All Files"), 0, 0);
		OPENFILENAME ofn = { 0 };
		ofn.lStructSize = sizeof(ofn);
		ofn.lpstrFile = options.log_filename;
		ofn.hwndOwner = m_hwnd;
		ofn.nMaxFile = _countof(options.log_filename);
		ofn.lpstrTitle = TranslateT("Open log file");
		ofn.lpstrFilter = filter;
		ofn.nFilterIndex = 1;
		ofn.lpstrFileTitle = nullptr;
		ofn.nMaxFileTitle = 0;
		ofn.lpstrInitialDir = nullptr;
		ofn.Flags = OFN_PATHMUSTEXIST;

		if (GetOpenFileName(&ofn) == TRUE) {
			SetDlgItemText(m_hwnd, IDC_ED_FILENAME, ofn.lpstrFile);
			SendMessage(GetParent(m_hwnd), PSM_CHANGED, 0, 0);
		}
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// ping hosts list window

class CHostOptsDlg : public CDlgBase
{
	CCtrlButton btnAdd, btnEdit, btnRemove, btnDown, btnUp;
	CCtrlListBox m_list;

public:
	CHostOptsDlg() :
		CDlgBase(g_plugin, IDD_DIALOG2),
		m_list(this, IDC_LST_DEST),
		btnUp(this, IDC_BTN_DESTUP),
		btnAdd(this, IDC_BTN_DESTADD),
		btnDown(this, IDC_BTN_DESTDOWN),
		btnEdit(this, IDC_BTN_DESTEDIT),
		btnRemove(this, IDC_BTN_DESTREM)
	{
		m_list.OnSelChange = Callback(this, &CHostOptsDlg::onSelChange_List);

		btnAdd.OnClick = Callback(this, &CHostOptsDlg::onClick_Add);
		btnEdit.OnClick = Callback(this, &CHostOptsDlg::onClick_Edit);
		btnRemove.OnClick = Callback(this, &CHostOptsDlg::onClick_Remove);

		btnUp.OnClick = Callback(this, &CHostOptsDlg::onClick_Up);
		btnDown.OnClick = Callback(this, &CHostOptsDlg::onClick_Down);
	}

	bool OnInitDialog() override
	{
		mir_cslock lck(data_list_cs);
		temp_list = data_list;

		for (auto &it : temp_list)
			m_list.InsertString(it.pszLabel, -1, (LPARAM)&it);

		return true;
	}

	bool OnApply() override
	{
		SetAndSavePingList(temp_list);
		GetPingList(temp_list);

		RefreshWindow();

		// the following will be affected due to list rebuild event
		if (hWakeEvent)
			SetEvent(hWakeEvent);
		return true;
	}

	void onSelChange_List(CCtrlListBox*)
	{
		int sel = m_list.GetCurSel();
		if (sel != LB_ERR) {
			btnEdit.Enable();
			btnRemove.Enable();

			btnUp.Enable(sel > 0);
			btnDown.Enable(sel < m_list.GetCount() - 1);
		}
	}

	void onClick_Edit(CCtrlButton*)
	{
		int sel = m_list.GetCurSel();
		if (sel != LB_ERR) {
			PINGADDRESS *item = (PINGADDRESS *)m_list.GetItemData(sel);
			PINGADDRESS temp = *item;
			if (Edit(m_hwnd, temp)) {
				*item = temp;
				m_list.DeleteString(sel);
				m_list.InsertString(item->pszLabel, sel, (LPARAM)item);
				m_list.SetCurSel(sel);

				onSelChange_List(0);
				NotifyChange();
			}
		}
	}

	void onClick_Add(CCtrlButton *)
	{
		memset(&add_edit_addr, 0, sizeof(add_edit_addr));
		add_edit_addr.cbSize = sizeof(add_edit_addr);
		add_edit_addr.port = -1;
		add_edit_addr.set_status = ID_STATUS_ONLINE;
		add_edit_addr.get_status = ID_STATUS_OFFLINE;
		add_edit_addr.status = PS_NOTRESPONDING;
		add_edit_addr.item_id = 0;
		add_edit_addr.index = (int)temp_list.size();

		if (DialogBox(g_plugin.getInst(), MAKEINTRESOURCE(IDD_DIALOG3), m_hwnd, DlgProcDestEdit) == IDOK) {
			temp_list.push_back(add_edit_addr);

			int index = m_list.InsertString(add_edit_addr.pszLabel, -1, (LPARAM)&temp_list.back());
			m_list.SetCurSel(index);

			onSelChange_List(0);
			NotifyChange();
		}
	}

	void onClick_Remove(CCtrlButton *)
	{
		int sel = m_list.GetCurSel();
		if (sel != LB_ERR) {
			PINGADDRESS *item = (PINGADDRESS *)m_list.GetItemData(sel);
			m_list.DeleteString(sel);
			temp_list.remove(*item);
		}

		btnEdit.Disable();
		btnRemove.Disable();
		btnDown.Disable();
		btnUp.Disable();

		NotifyChange();
		RefreshWindow();
	}

	void onClick_Down(CCtrlButton *)
	{
		int sel2 = m_list.GetCurSel();
		if (sel2 == LB_ERR)
			return;

		PINGADDRESS *item = (PINGADDRESS *)m_list.GetItemData(sel2), *item2 = (PINGADDRESS *)m_list.GetItemData(sel2 + 1);
		if (item && item2) {
			add_edit_addr = *item;
			*item = *item2;
			*item2 = add_edit_addr;

			// keep indexes the same, as they're used for sorting the binary tree
			int index = item->index, index2 = item2->index;
			item->index = index2;
			item2->index = index;

			m_list.DeleteString(sel2);
			m_list.InsertString(item->pszLabel, sel2, (LPARAM)item);

			m_list.DeleteString(sel2 + 1);
			m_list.InsertString(item2->pszLabel, sel2 + 1, (LPARAM)item2);

			m_list.SetCurSel(sel2 + 1);

			btnUp.Enable(sel2 + 1 > 0);
			btnDown.Enable(sel2 + 1 < m_list.GetCount() - 1);
			NotifyChange();
		}
	}

	void onClick_Up(CCtrlButton *)
	{
		int sel2 = SendDlgItemMessage(m_hwnd, IDC_LST_DEST, LB_GETCURSEL, 0, 0);
		if (sel2 == LB_ERR)
			return;
		
		PINGADDRESS *item = (PINGADDRESS *)m_list.GetItemData(sel2), *item2 = (PINGADDRESS *)m_list.GetItemData(sel2 - 1);
		if (item && item2) {
			add_edit_addr = *item;
			*item = *item2;
			*item2 = add_edit_addr;

			// keep indexes the same, as they're used for sorting the binary tree
			int index = item->index, index2 = item2->index;
			item->index = index2;
			item2->index = index;

			m_list.DeleteString(sel2);
			m_list.InsertString(item->pszLabel, sel2, (LPARAM)item);

			m_list.DeleteString(sel2 - 1);
			m_list.InsertString(item2->pszLabel, sel2 - 1, (LPARAM)item2);

			m_list.SetCurSel(sel2 - 1);

			btnUp.Enable(sel2 - 1 > 0);
			btnDown.Enable(sel2 - 1 < m_list.GetCount() - 1);
			NotifyChange();
		}
	}
};

int PingOptInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.flags = ODPF_BOLDGROUPS | ODPF_UNICODE;
	odp.szGroup.w = LPGENW("Network");
	odp.szTitle.w = LPGENW("Ping");

	odp.szTab.w = LPGENW("Settings");
	odp.pDialog = new CMainOptsDlg();
	g_plugin.addOptions(wParam, &odp);

	odp.szTab.w = LPGENW("Hosts");
	odp.pDialog = new CHostOptsDlg();
	g_plugin.addOptions(wParam, &odp);
	return 0;
}

void LoadOptions()
{
	options.ping_period = g_plugin.getDword("PingPeriod", DEFAULT_PING_PERIOD);

	options.ping_timeout = g_plugin.getDword("PingTimeout", DEFAULT_PING_TIMEOUT);
	options.show_popup = (g_plugin.getByte("ShowPopup", DEFAULT_SHOW_POPUP ? 1 : 0) == 1);
	options.show_popup2 = (g_plugin.getByte("ShowPopup2", DEFAULT_SHOW_POPUP2 ? 1 : 0) == 1);
	options.block_reps = (g_plugin.getByte("BlockReps", DEFAULT_BLOCK_REPS ? 1 : 0) == 1);
	options.logging = (g_plugin.getByte("LoggingEnabled", DEFAULT_LOGGING_ENABLED ? 1 : 0) == 1);

	options.no_test_icon = (g_plugin.getByte("NoTestStatus", DEFAULT_NO_TEST_ICON ? 1 : 0) == 1);

	options.indent = g_plugin.getWord("Indent", 0);
	options.row_height = g_plugin.getWord("RowHeight", GetSystemMetrics(SM_CYSMICON));

	options.retries = g_plugin.getDword("Retries", 0);

	GetLogFilename(options.log_filename, _countof(options.log_filename));

	ICMP::get_instance()->set_timeout(options.ping_timeout * 1000);

	options.attach_to_clist = (g_plugin.getByte("AttachToClist", DEFAULT_ATTACH_TO_CLIST ? 1 : 0) == 1);
	options.log_csv = (g_plugin.getByte("LogCSV", 0) == 1);
}

void SaveOptions()
{
	g_plugin.setDword("PingPeriod", options.ping_period);
	g_plugin.setDword("PingTimeout", options.ping_timeout);

	g_plugin.setByte("ShowPopup", options.show_popup ? 1 : 0);
	g_plugin.setByte("ShowPopup2", options.show_popup2 ? 1 : 0);
	g_plugin.setByte("BlockReps", options.block_reps ? 1 : 0);
	g_plugin.setByte("LoggingEnabled", options.logging ? 1 : 0);

	g_plugin.setByte("NoTestStatus", options.no_test_icon ? 1 : 0);

	g_plugin.setWord("Indent", options.indent);
	g_plugin.setWord("RowHeight", options.row_height);

	g_plugin.setDword("Retries", (uint32_t)options.retries);
	g_plugin.setWString("LogFilename", options.log_filename);

	ICMP::get_instance()->set_timeout(options.ping_timeout * 1000);

	g_plugin.setByte("AttachToClist", options.attach_to_clist ? 1 : 0);
	g_plugin.setByte("LogCSV", options.log_csv ? 1 : 0);
}
