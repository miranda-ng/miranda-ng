#include "stdafx.h"

class COptionsDlg : public CDlgBase
{
	int curIndex = 0;

	CCtrlEdit edtDuration, edtCircle;
	CCtrlCheck chkPopups, chkOnline, chkTray, chkPopup, chkShowIcon, chkLogThreads;
	CCtrlColor clrText, clrBack;
	CCtrlCombo m_combo;
	CCtrlButton btnAdd, btnDel, btnReg;

public: 
	COptionsDlg() :
		CDlgBase(g_plugin, IDD_OPT),
		m_combo(this, IDC_NAME),
		btnAdd(this, IDC_BTNADD),
		btnDel(this, IDC_BTNDEL),
		btnReg(this, IDC_REGISTER),
		clrBack(this, IDC_BGCOLOR),
		clrText(this, IDC_TEXTCOLOR),
		chkTray(this, IDC_OPTTRAY),
		chkPopup(this, IDC_OPTPOP),
		chkOnline(this, IDC_ONLINE),
		chkPopups(this, IDC_OPTPOP),
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

		m_combo.OnSelChanged = Callback(this, &COptionsDlg::onSelChanged);

		btnAdd.OnClick = Callback(this, &COptionsDlg::onClick_Add);
		btnDel.OnClick = Callback(this, &COptionsDlg::onClick_Del);
		btnReg.OnClick = Callback(this, &COptionsDlg::onClick_Reg);

		chkPopups.OnChange = Callback(this, &COptionsDlg::onChange_Popups);
	}

	void OnChange() override
	{
		bool bEnabled = g_accs.getCount();
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

		if (g_plugin.bNotifierOnPop) {
			ShowWindow(GetDlgItem(m_hwnd, IDC_DURATION), SW_SHOW);
			ShowWindow(GetDlgItem(m_hwnd, IDC_BGCOLOR), SW_SHOW);
			ShowWindow(GetDlgItem(m_hwnd, IDC_TEXTCOLOR), SW_SHOW);
			ShowWindow(GetDlgItem(m_hwnd, IDC_STATIC_DURATION), SW_SHOW);
			ShowWindow(GetDlgItem(m_hwnd, IDC_STATIC_COLOR), SW_SHOW);
			ShowWindow(GetDlgItem(m_hwnd, IDC_STATIC_LESS), SW_SHOW);
			ShowWindow(GetDlgItem(m_hwnd, IDC_STATIC_SEC), SW_SHOW);
		}

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
		es.caption = TranslateT("Enter your Google email");
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

	void onSelChanged(CCtrlCombo*)
	{
		curIndex = m_combo.GetCurSel();
		if (curIndex != -1) {
			auto *p = (Account *)m_combo.GetItemData(curIndex);
			btnReg.SetText(p->szRefreshToken.IsEmpty() ? TranslateT("Register") : TranslateT("Unregister"));
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
