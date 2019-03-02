#include "stdafx.h"

static void SaveButton(HWND hwndDlg, HWND hwndCombo, int curIndex)
{
	if (curIndex < 0 || curIndex >= g_accs.getCount())
		return;

	Account &acc = g_accs[curIndex];
	if (GetDlgItemTextA(hwndDlg, IDC_NAME, acc.name, _countof(acc.name))) {
		char *tail = strstr(acc.name, "@");
		if (tail && mir_strcmp(tail + 1, "gmail.com") != 0)
			mir_strcpy(acc.hosted, tail + 1);
		SendMessageA(hwndCombo, CB_DELETESTRING, curIndex, 0);
		SendMessageA(hwndCombo, CB_INSERTSTRING, curIndex, (LPARAM)acc.name);
		SendMessageA(hwndCombo, CB_SETCURSEL, curIndex, 0);
		g_plugin.setString(acc.hContact, "name", acc.name);
		g_plugin.setString(acc.hContact, "Nick", acc.name);

		GetDlgItemTextA(hwndDlg, IDC_PASS, acc.pass, _countof(acc.pass));
		g_plugin.setString(acc.hContact, "Password", acc.pass);
	}
}

static INT_PTR CALLBACK DlgProcOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	int ShowControl;
	char str[MAX_PATH] = { 0 };
	static int curIndex = 0;
	static bool bInit = false;
	HWND hwndCombo = GetDlgItem(hwndDlg, IDC_NAME);

	if (g_accs.getCount()) {
		EnableWindow(hwndCombo, TRUE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_PASS), TRUE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_BTNSAV), TRUE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_BTNDEL), TRUE);
	}
	else {
		EnableWindow(hwndCombo, FALSE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_PASS), FALSE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_BTNSAV), FALSE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_BTNDEL), FALSE);
	}

	switch (msg) {
	case WM_INITDIALOG:
		bInit = true;
		TranslateDialogDefault(hwndDlg);
		optionWindowIsOpen = TRUE;
		BuildList();

		for (auto &it : g_accs)
			SendMessageA(hwndCombo, CB_ADDSTRING, 0, (LONG_PTR)it->name);
		SendMessage(hwndCombo, CB_SETCURSEL, curIndex, 0);
		if (curIndex < g_accs.getCount())
			SetDlgItemTextA(hwndDlg, IDC_PASS, g_accs[curIndex].pass);

		SetDlgItemInt(hwndDlg, IDC_CIRCLE, opt.circleTime, FALSE);
		if (opt.notifierOnTray)
			CheckDlgButton(hwndDlg, IDC_OPTTRAY, BST_CHECKED);
		if (opt.notifierOnPop) {
			CheckDlgButton(hwndDlg, IDC_OPTPOP, BST_CHECKED);
			ShowWindow(GetDlgItem(hwndDlg, IDC_DURATION), SW_SHOW);
			ShowWindow(GetDlgItem(hwndDlg, IDC_BGCOLOR), SW_SHOW);
			ShowWindow(GetDlgItem(hwndDlg, IDC_TEXTCOLOR), SW_SHOW);
			ShowWindow(GetDlgItem(hwndDlg, IDC_STATIC_DURATION), SW_SHOW);
			ShowWindow(GetDlgItem(hwndDlg, IDC_STATIC_COLOR), SW_SHOW);
			ShowWindow(GetDlgItem(hwndDlg, IDC_STATIC_LESS), SW_SHOW);
			ShowWindow(GetDlgItem(hwndDlg, IDC_STATIC_SEC), SW_SHOW);
		}

		SetDlgItemInt(hwndDlg, IDC_DURATION, opt.popupDuration, TRUE);
		SendDlgItemMessage(hwndDlg, IDC_BGCOLOR, CPM_SETCOLOUR, 0, opt.popupBgColor);
		SendDlgItemMessage(hwndDlg, IDC_TEXTCOLOR, CPM_SETCOLOUR, 0, opt.popupTxtColor);

		if (opt.OpenUsePrg == 0)
			CheckDlgButton(hwndDlg, IDC_SYSDEF, BST_CHECKED);
		else if (opt.OpenUsePrg == 1)
			CheckDlgButton(hwndDlg, IDC_USEIE, BST_CHECKED);
		else if (opt.OpenUsePrg == 2) {
			CheckDlgButton(hwndDlg, IDC_STARTPRG, BST_CHECKED);
			ShowWindow(GetDlgItem(hwndDlg, IDC_PRG), SW_SHOW);
			ShowWindow(GetDlgItem(hwndDlg, IDC_PRGBROWSE), SW_SHOW);
		}
		{
			DBVARIANT dbv;
			if (!g_plugin.getString("OpenUsePrgPath", &dbv)) {
				mir_strcpy(str, dbv.pszVal);
				db_free(&dbv);
			}
		}
		SetDlgItemTextA(hwndDlg, IDC_PRG, str);

		if (opt.UseOnline)
			CheckDlgButton(hwndDlg, IDC_ONLINE, BST_CHECKED);
		if (opt.ShowCustomIcon)
			CheckDlgButton(hwndDlg, IDC_SHOWICON, BST_CHECKED);
		if (opt.AutoLogin == 0)
			CheckDlgButton(hwndDlg, IDC_AUTOLOGIN, BST_CHECKED);
		else if (opt.AutoLogin == 1)
			CheckDlgButton(hwndDlg, IDC_AUTOLOGIN, BST_UNCHECKED);
		else if (opt.AutoLogin == 2)
			CheckDlgButton(hwndDlg, IDC_AUTOLOGIN, BST_INDETERMINATE);
		if (opt.LogThreads)
			CheckDlgButton(hwndDlg, IDC_LOGTHREADS, BST_CHECKED);

		bInit = false;
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_SYSDEF:
		case IDC_USEIE:
		case IDC_STARTPRG:
			ShowControl = IsDlgButtonChecked(hwndDlg, IDC_STARTPRG) ? SW_SHOW : SW_HIDE;
			ShowWindow(GetDlgItem(hwndDlg, IDC_PRG), ShowControl);
			ShowWindow(GetDlgItem(hwndDlg, IDC_PRGBROWSE), ShowControl);
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;

		case IDC_OPTPOP:
			ShowControl = IsDlgButtonChecked(hwndDlg, IDC_OPTPOP) ? SW_SHOW : SW_HIDE;
			ShowWindow(GetDlgItem(hwndDlg, IDC_DURATION), ShowControl);
			ShowWindow(GetDlgItem(hwndDlg, IDC_BGCOLOR), ShowControl);
			ShowWindow(GetDlgItem(hwndDlg, IDC_TEXTCOLOR), ShowControl);
			ShowWindow(GetDlgItem(hwndDlg, IDC_STATIC_DURATION), ShowControl);
			ShowWindow(GetDlgItem(hwndDlg, IDC_STATIC_COLOR), ShowControl);
			ShowWindow(GetDlgItem(hwndDlg, IDC_STATIC_LESS), ShowControl);
			ShowWindow(GetDlgItem(hwndDlg, IDC_STATIC_SEC), ShowControl);
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;

		case IDC_PRGBROWSE:
			wchar_t szName[_MAX_PATH];
			GetDlgItemText(hwndDlg, IDC_PRG, szName, _countof(szName));
			{
				OPENFILENAME OpenFileName = {};
				OpenFileName.lStructSize = sizeof(OPENFILENAME);
				OpenFileName.hwndOwner = hwndDlg;
				OpenFileName.lpstrFilter = L"Executables (*.exe;*.com;*.bat)\0*.exe;*.com;*.bat\0\0";
				OpenFileName.lpstrFile = szName;
				OpenFileName.nMaxFile = _countof(szName);
				OpenFileName.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_FILEMUSTEXIST;
				if (!GetOpenFileName(&OpenFileName))
					return 0;
				SetDlgItemText(hwndDlg, IDC_PRG, szName);
			}
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;

		case IDC_BTNADD:
			{
				Account *p = new Account();
				p->hContact = db_add_contact();
				Proto_AddToContact(p->hContact, MODULENAME);
				g_accs.insert(p);

				curIndex = SendMessageA(hwndCombo, CB_ADDSTRING, 0, (LPARAM)"");
				SendMessage(hwndCombo, CB_SETCURSEL, curIndex, 0);
				SetDlgItemTextA(hwndDlg, IDC_PASS, "");
				SetFocus(hwndCombo);
			}
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;

		case IDC_BTNSAV:
			SaveButton(hwndDlg, hwndCombo, curIndex);
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;

		case IDC_BTNDEL:
			{
				SendMessage(hwndCombo, CB_DELETESTRING, curIndex, 0);

				Account &acc = g_accs[curIndex];
				DeleteResults(acc.results.next);
				db_delete_contact(acc.hContact);
				g_accs.remove(curIndex);

				curIndex = 0;
				SendMessage(hwndCombo, CB_SETCURSEL, 0, 0);
				if (g_accs.getCount())
					SetDlgItemTextA(hwndDlg, IDC_PASS, g_accs[0].pass);
			}
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;

		case IDC_NAME:
			if (HIWORD(wParam) == CBN_SELCHANGE) {
				curIndex = SendMessage(hwndCombo, CB_GETCURSEL, 0, 0);
				SetDlgItemTextA(hwndDlg, IDC_PASS, g_accs[curIndex].pass);
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			}
			break;

		case IDC_CIRCLE:
		case IDC_DURATION:
			if (!bInit && (HIWORD(wParam) == EN_CHANGE))
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;

		case IDC_ONLINE:
		case IDC_SHOWICON:
		case IDC_AUTOLOGIN:
		case IDC_LOGTHREADS:
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		}

		return TRUE;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code) {
		case PSN_APPLY:
			SaveButton(hwndDlg, hwndCombo, curIndex);
			opt.circleTime = GetDlgItemInt(hwndDlg, IDC_CIRCLE, nullptr, FALSE);
			if (opt.circleTime > 0) {
				KillTimer(nullptr, hTimer);
				hTimer = SetTimer(nullptr, 0, opt.circleTime * 60000, TimerProc);
				g_plugin.setDword("circleTime", opt.circleTime);
			}
			opt.notifierOnTray = IsDlgButtonChecked(hwndDlg, IDC_OPTTRAY);
			opt.notifierOnPop = IsDlgButtonChecked(hwndDlg, IDC_OPTPOP);
			g_plugin.setDword("notifierOnTray", opt.notifierOnTray);
			g_plugin.setDword("notifierOnPop", opt.notifierOnPop);

			opt.popupDuration = GetDlgItemInt(hwndDlg, IDC_DURATION, nullptr, TRUE);
			g_plugin.setDword("popupDuration", opt.popupDuration);

			opt.popupBgColor = SendDlgItemMessage(hwndDlg, IDC_BGCOLOR, CPM_GETCOLOUR, 0, opt.popupBgColor);
			opt.popupTxtColor = SendDlgItemMessage(hwndDlg, IDC_TEXTCOLOR, CPM_GETCOLOUR, 0, opt.popupBgColor);
			g_plugin.setDword("popupBgColor", opt.popupBgColor);
			g_plugin.setDword("popupTxtColor", opt.popupTxtColor);

			if (IsDlgButtonChecked(hwndDlg, IDC_SYSDEF) == BST_CHECKED)
				opt.OpenUsePrg = 0;
			else if (IsDlgButtonChecked(hwndDlg, IDC_USEIE) == BST_CHECKED)
				opt.OpenUsePrg = 1;
			else if (IsDlgButtonChecked(hwndDlg, IDC_STARTPRG) == BST_CHECKED) {
				opt.OpenUsePrg = 2;
			}
			GetDlgItemTextA(hwndDlg, IDC_PRG, str, _countof(str));

			g_plugin.setDword("OpenUsePrg", opt.OpenUsePrg);
			g_plugin.setString("OpenUsePrgPath", str);

			opt.ShowCustomIcon = IsDlgButtonChecked(hwndDlg, IDC_SHOWICON);
			opt.UseOnline = IsDlgButtonChecked(hwndDlg, IDC_ONLINE);
			if (IsDlgButtonChecked(hwndDlg, IDC_AUTOLOGIN) == BST_CHECKED)
				opt.AutoLogin = 0;
			else if (IsDlgButtonChecked(hwndDlg, IDC_AUTOLOGIN) == BST_UNCHECKED)
				opt.AutoLogin = 1;
			else if (IsDlgButtonChecked(hwndDlg, IDC_AUTOLOGIN) == BST_INDETERMINATE)
				opt.AutoLogin = 2;
			opt.LogThreads = IsDlgButtonChecked(hwndDlg, IDC_LOGTHREADS);
			g_plugin.setDword("ShowCustomIcon", opt.ShowCustomIcon);
			g_plugin.setDword("UseOnline", opt.UseOnline);
			g_plugin.setDword("AutoLogin", opt.AutoLogin);
			g_plugin.setDword("LogThreads", opt.LogThreads);

			ID_STATUS_NONEW = opt.UseOnline ? ID_STATUS_ONLINE : ID_STATUS_OFFLINE;
			for (auto &it : g_accs)
				g_plugin.setWord(it->hContact, "Status", ID_STATUS_NONEW);
		}
		return TRUE;

	case WM_CLOSE:
		optionWindowIsOpen = FALSE;
		return TRUE;
	}
	return FALSE;
}

int OptInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.position = -790000000;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT);
	odp.szTitle.a = LPGEN("GmailNotifier");
	odp.szGroup.a = LPGEN("Network");
	odp.flags = ODPF_BOLDGROUPS;
	odp.pfnDlgProc = DlgProcOpts;
	g_plugin.addOptions(wParam, &odp);
	return 0;
}
