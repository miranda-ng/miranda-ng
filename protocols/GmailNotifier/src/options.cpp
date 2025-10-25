#include "stdafx.h"

class COptionsDlg : public CDlgBase
{
	int curIndex = 0;

	CCtrlEdit edtDuration, edtCircle;
	CCtrlCheck radio1, radio2, radio3;
	CCtrlCheck chkPopups, chkOnline, chkTray, chkPopup, chkLogThreads, chkAutoLogin;
	CCtrlColor clrText, clrBack;
	CCtrlCombo m_combo;
	CCtrlButton btnBrowse, btnAdd, btnEdit, btnDel, btnReg;

public:
	COptionsDlg() :
		CDlgBase(g_plugin, IDD_OPT),
		m_combo(this, IDC_NAME),
		radio1(this, IDC_SYSDEF),
		radio2(this, IDC_STARTPRG),
		radio3(this, IDC_NOTHING),
		btnAdd(this, IDC_BTNADD),
		btnEdit(this, IDC_BTNEDIT),
		btnDel(this, IDC_BTNDEL),
		btnReg(this, IDC_REGISTER),
		btnBrowse(this, IDC_PRGBROWSE),
		clrBack(this, IDC_BGCOLOR),
		clrText(this, IDC_TEXTCOLOR),
		chkTray(this, IDC_OPTTRAY),
		chkPopup(this, IDC_OPTPOP),
		chkOnline(this, IDC_ONLINE),
		chkPopups(this, IDC_OPTPOP),
		chkAutoLogin(this, IDC_AUTOLOGIN),
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
		CreateLink(edtDuration, g_plugin.popupDuration);
		CreateLink(chkAutoLogin, g_plugin.AutoLogin);
		CreateLink(chkLogThreads, g_plugin.bLogThreads);

		m_combo.OnSelChanged = Callback(this, &COptionsDlg::onSelChanged);

		btnAdd.OnClick = Callback(this, &COptionsDlg::onClick_Add);
		btnEdit.OnClick = Callback(this, &COptionsDlg::onClick_Edit);
		btnDel.OnClick = Callback(this, &COptionsDlg::onClick_Del);
		btnReg.OnClick = Callback(this, &COptionsDlg::onClick_Reg);
		btnBrowse.OnClick = Callback(this, &COptionsDlg::onClick_Browse);

		chkPopups.OnChange = Callback(this, &COptionsDlg::onChange_Popups);
		radio1.OnChange = radio2.OnChange = radio3.OnChange = Callback(this, &COptionsDlg::onChange_Program);
	}

	void OnChange() override
	{
		bool bEnabled = g_accs.getCount();
		btnEdit.Enable(bEnabled);
		btnDel.Enable(bEnabled);
		btnReg.Enable(bEnabled);
		m_combo.Enable(bEnabled);
	}

	bool OnInitDialog() override
	{
		for (auto &it : g_accs)
			m_combo.AddString(_A2T(it->szName), LPARAM(it));
		m_combo.SetCurSel(curIndex);
		onSelChanged(0);

		if (g_plugin.OpenUsePrg == 0)
			radio1.SetState(true);
		else if (g_plugin.OpenUsePrg == 1)
			radio2.SetState(true);
		else if (g_plugin.OpenUsePrg == 2) {
			radio3.SetState(true);
			ShowWindow(GetDlgItem(m_hwnd, IDC_PRG), SW_SHOW);
			ShowWindow(GetDlgItem(m_hwnd, IDC_PRGBROWSE), SW_SHOW);
		}

		ptrW szPrg(g_plugin.getWStringA("OpenUsePrgPath"));
		if (szPrg)
			SetDlgItemTextW(m_hwnd, IDC_PRG, szPrg);

		if (g_plugin.bNotifierOnPop) {
			ShowWindow(GetDlgItem(m_hwnd, IDC_DURATION), SW_SHOW);
			ShowWindow(GetDlgItem(m_hwnd, IDC_BGCOLOR), SW_SHOW);
			ShowWindow(GetDlgItem(m_hwnd, IDC_TEXTCOLOR), SW_SHOW);
			ShowWindow(GetDlgItem(m_hwnd, IDC_STATIC_DURATION), SW_SHOW);
			ShowWindow(GetDlgItem(m_hwnd, IDC_STATIC_COLOR), SW_SHOW);
			ShowWindow(GetDlgItem(m_hwnd, IDC_STATIC_LESS), SW_SHOW);
			ShowWindow(GetDlgItem(m_hwnd, IDC_STATIC_SEC), SW_SHOW);
		}

		OnChange();
		return true;
	}

	bool OnApply() override
	{
		if (radio1.IsChecked())
			g_plugin.OpenUsePrg = 0;
		else if (radio2.IsChecked())
			g_plugin.OpenUsePrg = 1;
		else if (radio3.IsChecked())
			g_plugin.OpenUsePrg = 2;

		char str[MAX_PATH] = { 0 };
		GetDlgItemTextA(m_hwnd, IDC_PRG, str, _countof(str));
		g_plugin.setString("OpenUsePrgPath", str);

		ID_STATUS_NONEW = g_plugin.bUseOnline ? ID_STATUS_ONLINE : ID_STATUS_OFFLINE;
		for (auto &it : g_accs)
			g_plugin.setWord(it->hContact, "Status", ID_STATUS_NONEW);

		if (g_plugin.circleTime > 0) {
			KillTimer(nullptr, hTimer);
			hTimer = SetTimer(nullptr, 0, g_plugin.circleTime * 60000, TimerProc);
		}
		return true;
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

	void onClick_Add(CCtrlButton *)
	{
		ENTER_STRING es = {};
		es.type = ESF_NOT_EMPTY;
		es.caption = TranslateT("Enter account name");
		if (!EnterString(&es))
			return;

		Account *p = new Account(db_add_contact());
		Proto_AddToContact(p->hContact, MODULENAME);
		g_accs.insert(p);

		p->szName = _T2A(es.ptszResult);
		g_plugin.setString(p->hContact, "name", p->szName);
		g_plugin.setString(p->hContact, "Nick", p->szName);

		curIndex = m_combo.AddString(es.ptszResult, LPARAM(p));
		m_combo.SetCurSel(curIndex);
		SetFocus(m_combo.GetHwnd());
		NotifyChange();
	}

	void onClick_Edit(CCtrlButton *)
	{
		Account &acc = g_accs[curIndex];
		CMStringW wszNick(g_plugin.getMStringW(acc.hContact, "Nick"));

		ENTER_STRING es = {};
		es.type = ESF_NOT_EMPTY;
		es.caption = TranslateT("Enter your Google email");
		es.ptszInitVal = wszNick;
		if (!EnterString(&es))
			return;

		m_combo.DeleteString(curIndex);
		m_combo.InsertString(es.ptszResult, curIndex, LPARAM(&acc));
		m_combo.SetCurSel(curIndex);

		acc.szName = _T2A(es.ptszResult);
		g_plugin.setString(acc.hContact, "name", acc.szName);
		g_plugin.setString(acc.hContact, "Nick", acc.szName);
		NotifyChange();
	}

	void onClick_Del(CCtrlButton *)
	{
		m_combo.DeleteString(curIndex);

		Account &acc = g_accs[curIndex];
		{
			mir_cslock lck(acc.csLock);
			if (acc.Registered())
				acc.Unregister();

			db_delete_contact(acc.hContact, CDF_FROM_SERVER);
			g_accs.remove(curIndex);
		}

		m_combo.SetCurSel(curIndex = 0);
		NotifyChange();
	}

	void onClick_Reg(CCtrlButton *)
	{
		Account &acc = g_accs[curIndex];
		{
			mir_cslock lck(acc.csLock);
			if (acc.Registered())
				acc.Unregister();
			else
				acc.Register();
		}

		onSelChanged(0);
	}

	void onClick_Browse(CCtrlButton *)
	{
		wchar_t szName[_MAX_PATH];
		GetDlgItemTextW(m_hwnd, IDC_PRG, szName, _countof(szName));

		OPENFILENAME OpenFileName = {};
		OpenFileName.lStructSize = sizeof(OPENFILENAME);
		OpenFileName.hwndOwner = m_hwnd;
		OpenFileName.lpstrFilter = L"Executables (*.exe;*.com;*.bat)\0*.exe;*.com;*.bat\0\0";
		OpenFileName.lpstrFile = szName;
		OpenFileName.nMaxFile = _countof(szName);
		OpenFileName.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_FILEMUSTEXIST;
		if (GetOpenFileNameW(&OpenFileName))
			SetDlgItemTextW(m_hwnd, IDC_PRG, szName);
	}
	
	void onChange_Program(CCtrlCheck *)
	{
		int ShowControl = radio2.IsChecked();
		ShowWindow(GetDlgItem(m_hwnd, IDC_PRG), ShowControl);
		ShowWindow(GetDlgItem(m_hwnd, IDC_PRGBROWSE), ShowControl);
	}
	
	void onSelChanged(CCtrlCombo *)
	{
		curIndex = m_combo.GetCurSel();
		if (curIndex != -1) {
			auto *p = (Account *)m_combo.GetItemData(curIndex);
			btnReg.SetText(p->szRefreshToken.IsEmpty() ? TranslateT("Authorize") : TranslateT("Deauthorize"));
		}
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
