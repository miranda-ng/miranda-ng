#include "stdafx.h"

class COptionsDlg : public CDlgBase
{
	int curIndex = 0;

	CCtrlEdit edtDuration, edtCircle;
	CCtrlCheck chkProxy1, chkProxy2, chkProxy3, chkPopups, chkOnline, chkTray, chkPopup, chkShowIcon, chkLogThreads;
	CCtrlColor clrText, clrBack;
	CCtrlCombo m_combo;
	CCtrlButton btnBrowse, btnAdd, btnDel, btnSave;

public: 
	COptionsDlg() :
		CDlgBase(g_plugin, IDD_OPT),
		m_combo(this, IDC_NAME),
		btnAdd(this, IDC_BTNADD),
		btnDel(this, IDC_BTNDEL),
		btnSave(this, IDC_BTNSAV),
		btnBrowse(this, IDC_PRGBROWSE),
		clrBack(this, IDC_BGCOLOR),
		clrText(this, IDC_TEXTCOLOR),
		chkTray(this, IDC_OPTTRAY),
		chkPopup(this, IDC_OPTPOP),
		chkOnline(this, IDC_ONLINE),
		chkPopups(this, IDC_OPTPOP),
		chkProxy1(this, IDC_SYSDEF),
		chkProxy2(this, IDC_USEIE),
		chkProxy3(this, IDC_STARTPRG),
		chkShowIcon(this, IDC_SHOWICON),
		chkLogThreads(this, IDC_LOGTHREADS),
		edtCircle(this, IDC_CIRCLE),
		edtDuration(this, IDC_DURATION)
	{
		CreateLink(clrBack, g_plugin.popupBgColor);
		CreateLink(clrText, g_plugin.popupTxtColor);
		CreateLink(chkTray, g_plugin.bNotifierOnTray);
		CreateLink(chkPopup, g_plugin.bNotifierOnPop);
		CreateLink(chkOnline, g_plugin.bUseOnline);
		CreateLink(edtCircle, g_plugin.circleTime);
		CreateLink(chkShowIcon, g_plugin.bShowCustomIcon);
		CreateLink(edtDuration, g_plugin.popupDuration);
		CreateLink(chkLogThreads, g_plugin.bLogThreads);

		m_combo.OnSelChanged = Callback(this, &COptionsDlg::onSelChanged_Name);

		btnAdd.OnClick = Callback(this, &COptionsDlg::onClick_Add);
		btnDel.OnClick = Callback(this, &COptionsDlg::onClick_Del);
		btnSave.OnClick = Callback(this, &COptionsDlg::onClick_Save);
		btnBrowse.OnClick = Callback(this, &COptionsDlg::onClick_Browse);

		chkPopups.OnChange = Callback(this, &COptionsDlg::onChange_Popups);
		chkProxy1.OnChange = chkProxy2.OnChange = chkProxy2.OnChange = Callback(this, &COptionsDlg::onChange_Proxy);	
	}

	void OnChange() override
	{
		bool bEnabled = g_accs.getCount();
		btnDel.Enable(bEnabled);
		btnSave.Enable(bEnabled);
		m_combo.Enable(bEnabled);
		EnableWindow(GetDlgItem(m_hwnd, IDC_PASS), bEnabled);
	}

	bool OnInitDialog() override
	{
		optionWindowIsOpen = TRUE;
		BuildList();

		for (auto &it : g_accs)
			m_combo.AddString(_A2T(it->name));
		m_combo.SetCurSel(curIndex);
		if (curIndex < g_accs.getCount())
			SetDlgItemTextA(m_hwnd, IDC_PASS, g_accs[curIndex].pass);

		if (g_plugin.bNotifierOnPop) {
			ShowWindow(GetDlgItem(m_hwnd, IDC_DURATION), SW_SHOW);
			ShowWindow(GetDlgItem(m_hwnd, IDC_BGCOLOR), SW_SHOW);
			ShowWindow(GetDlgItem(m_hwnd, IDC_TEXTCOLOR), SW_SHOW);
			ShowWindow(GetDlgItem(m_hwnd, IDC_STATIC_DURATION), SW_SHOW);
			ShowWindow(GetDlgItem(m_hwnd, IDC_STATIC_COLOR), SW_SHOW);
			ShowWindow(GetDlgItem(m_hwnd, IDC_STATIC_LESS), SW_SHOW);
			ShowWindow(GetDlgItem(m_hwnd, IDC_STATIC_SEC), SW_SHOW);
		}

		if (g_plugin.OpenUsePrg == 0)
			chkProxy1.SetState(true);
		else if (g_plugin.OpenUsePrg == 1)
			chkProxy2.SetState(true);
		else if (g_plugin.OpenUsePrg == 2) {
			chkProxy3.SetState(true);
			ShowWindow(GetDlgItem(m_hwnd, IDC_PRG), SW_SHOW);
			ShowWindow(GetDlgItem(m_hwnd, IDC_PRGBROWSE), SW_SHOW);
		}

		ptrW szPrg(g_plugin.getWStringA("OpenUsePrgPath"));
		if (szPrg)
			SetDlgItemText(m_hwnd, IDC_PRG, szPrg);

		if (g_plugin.AutoLogin == 0)
			CheckDlgButton(m_hwnd, IDC_AUTOLOGIN, BST_CHECKED);
		else if (g_plugin.AutoLogin == 1)
			CheckDlgButton(m_hwnd, IDC_AUTOLOGIN, BST_UNCHECKED);
		else if (g_plugin.AutoLogin == 2)
			CheckDlgButton(m_hwnd, IDC_AUTOLOGIN, BST_INDETERMINATE);

		OnChange();
		return true;
	}

	bool OnApply() override
	{
		onClick_Save(0);

		if (chkProxy1.GetState())
			g_plugin.OpenUsePrg = 0;
		else if (chkProxy2.GetState())
			g_plugin.OpenUsePrg = 1;
		else if (chkProxy3.GetState()) {
			g_plugin.OpenUsePrg = 2;
		}

		char str[MAX_PATH] = { 0 };
		GetDlgItemTextA(m_hwnd, IDC_PRG, str, _countof(str));
		g_plugin.setString("OpenUsePrgPath", str);

		if (IsDlgButtonChecked(m_hwnd, IDC_AUTOLOGIN) == BST_CHECKED)
			g_plugin.AutoLogin = 0;
		else if (IsDlgButtonChecked(m_hwnd, IDC_AUTOLOGIN) == BST_UNCHECKED)
			g_plugin.AutoLogin = 1;
		else if (IsDlgButtonChecked(m_hwnd, IDC_AUTOLOGIN) == BST_INDETERMINATE)
			g_plugin.AutoLogin = 2;

		ID_STATUS_NONEW = g_plugin.bUseOnline ? ID_STATUS_ONLINE : ID_STATUS_OFFLINE;
		for (auto &it : g_accs)
			g_plugin.setWord(it->hContact, "Status", ID_STATUS_NONEW);

		if (g_plugin.circleTime > 0) {
			KillTimer(nullptr, hTimer);
			hTimer = SetTimer(nullptr, 0, g_plugin.circleTime * 60000, TimerProc);
		}
		return true;
	}

	void onChange_Proxy(CCtrlCheck *)
	{
		int ShowControl = IsDlgButtonChecked(m_hwnd, IDC_STARTPRG) ? SW_SHOW : SW_HIDE;
		ShowWindow(GetDlgItem(m_hwnd, IDC_PRG), ShowControl);
		ShowWindow(GetDlgItem(m_hwnd, IDC_PRGBROWSE), ShowControl);
	}

	void onChange_Popups(CCtrlCheck *)
	{
		int ShowControl = IsDlgButtonChecked(m_hwnd, IDC_OPTPOP) ? SW_SHOW : SW_HIDE;
		ShowWindow(GetDlgItem(m_hwnd, IDC_DURATION), ShowControl);
		ShowWindow(GetDlgItem(m_hwnd, IDC_BGCOLOR), ShowControl);
		ShowWindow(GetDlgItem(m_hwnd, IDC_TEXTCOLOR), ShowControl);
		ShowWindow(GetDlgItem(m_hwnd, IDC_STATIC_DURATION), ShowControl);
		ShowWindow(GetDlgItem(m_hwnd, IDC_STATIC_COLOR), ShowControl);
		ShowWindow(GetDlgItem(m_hwnd, IDC_STATIC_LESS), ShowControl);
		ShowWindow(GetDlgItem(m_hwnd, IDC_STATIC_SEC), ShowControl);
	}

	void onClick_Browse(CCtrlButton *)
	{
		wchar_t szName[_MAX_PATH];
		GetDlgItemText(m_hwnd, IDC_PRG, szName, _countof(szName));

		OPENFILENAME OpenFileName = {};
		OpenFileName.lStructSize = sizeof(OPENFILENAME);
		OpenFileName.hwndOwner = m_hwnd;
		OpenFileName.lpstrFilter = L"Executables (*.exe;*.com;*.bat)\0*.exe;*.com;*.bat\0\0";
		OpenFileName.lpstrFile = szName;
		OpenFileName.nMaxFile = _countof(szName);
		OpenFileName.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_FILEMUSTEXIST;
		if (GetOpenFileName(&OpenFileName))
			SetDlgItemText(m_hwnd, IDC_PRG, szName);
	}

	void onClick_Add(CCtrlButton *)
	{
		Account *p = new Account();
		p->hContact = db_add_contact();
		Proto_AddToContact(p->hContact, MODULENAME);
		g_accs.insert(p);

		curIndex = m_combo.AddString(L"");
		m_combo.SetCurSel(curIndex);
		SetDlgItemTextA(m_hwnd, IDC_PASS, "");
		SetFocus(m_combo.GetHwnd());
		NotifyChange();
	}

	void onClick_Del(CCtrlButton *)
	{
		m_combo.DeleteString(curIndex);

		Account &acc = g_accs[curIndex];
		DeleteResults(acc.results.next);
		db_delete_contact(acc.hContact);
		g_accs.remove(curIndex);

		m_combo.SetCurSel(curIndex = 0);
		if (g_accs.getCount())
			SetDlgItemTextA(m_hwnd, IDC_PASS, g_accs[0].pass);
		NotifyChange();
	}

	void onClick_Save(CCtrlButton *)
	{
		if (curIndex < 0 || curIndex >= g_accs.getCount())
			return;

		Account &acc = g_accs[curIndex];
		m_combo.GetTextA(acc.name, _countof(acc.name));
		if (!mir_strlen(acc.name))
			return;

		char *tail = strstr(acc.name, "@");
		if (tail && mir_strcmp(tail + 1, "gmail.com") != 0)
			mir_strcpy(acc.hosted, tail + 1);
		m_combo.DeleteString(curIndex);
		m_combo.InsertString(_A2T(acc.name), curIndex);
		m_combo.SetCurSel(curIndex);
		g_plugin.setString(acc.hContact, "name", acc.name);
		g_plugin.setString(acc.hContact, "Nick", acc.name);

		m_combo.GetTextA(acc.pass, _countof(acc.pass));
		g_plugin.setString(acc.hContact, "Password", acc.pass);
	}

	void onSelChanged_Name(CCtrlCombo*)
	{
		curIndex = m_combo.GetCurSel();
		SetDlgItemTextA(m_hwnd, IDC_PASS, g_accs[curIndex].pass);
		NotifyChange();
	}
};

int OptInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.flags = ODPF_BOLDGROUPS;
	odp.position = -790000000;
	odp.szTitle.a = LPGEN("GmailNotifier");
	odp.szGroup.a = LPGEN("Network");
	odp.pDialog = new COptionsDlg();
	g_plugin.addOptions(wParam, &odp);
	return 0;
}
