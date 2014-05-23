#include "gmail.h"

LRESULT WINAPI FlatComboProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	WNDPROC	OldComboProc = (WNDPROC)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	RECT rect;
	HDC hDcCombo;
	switch (msg) {
	case WM_PAINT:
		CallWindowProc(OldComboProc, hwnd, msg, wParam, lParam);
		hDcCombo = GetWindowDC(hwnd);
		GetClientRect(hwnd, &rect);
		FrameRect(hDcCombo, &rect, GetSysColorBrush(COLOR_3DFACE));
		InflateRect(&rect, -1, -1);
		DrawEdge(hDcCombo, &rect, BDR_RAISEDOUTER, BF_FLAT | BF_TOPLEFT);
		InflateRect(&rect, -1, -1);
		FrameRect(hDcCombo, &rect, 0);
		return 0;
	}
	return CallWindowProc(OldComboProc, hwnd, msg, wParam, lParam);
}

static INT_PTR CALLBACK DlgProcOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	int ShowControl;
	char str[MAX_PATH] = { 0 };
	char *tail;
	static int curIndex = 0;
	HWND hwndCombo = GetDlgItem(hwndDlg, IDC_NAME);

	if (acc_num) {
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
		TranslateDialogDefault(hwndDlg);
		optionWindowIsOpen = TRUE;
		BuildList();
		// Remember old window procedure
		SetWindowLongPtr(hwndCombo, GWLP_USERDATA, GetWindowLongPtr(hwndCombo, GWLP_WNDPROC));
		// Perform the subclass
		SetWindowLongPtr(hwndCombo, GWLP_WNDPROC, (LONG_PTR)FlatComboProc);

		for (int i = 0; i < acc_num; i++)
			SendMessageA(hwndCombo, CB_ADDSTRING, 0, (LONG)acc[i].name);
		SendMessage(hwndCombo, CB_SETCURSEL, curIndex, 0);
		if (curIndex < acc_num)
			SetDlgItemTextA(hwndDlg, IDC_PASS, acc[curIndex].pass);

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
			if (!db_get_s(NULL, pluginName, "OpenUsePrgPath", &dbv)) {
				lstrcpyA(str, dbv.pszVal);
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
			{
				OPENFILENAME OpenFileName;
				TCHAR szName[_MAX_PATH];
				memset(&OpenFileName, 0, sizeof(OPENFILENAME));
				GetDlgItemText(hwndDlg, IDC_PRG, szName, _MAX_PATH);
				OpenFileName.lStructSize = sizeof(OPENFILENAME);
				OpenFileName.hwndOwner = hwndDlg;
				OpenFileName.lpstrFilter = _T("Executables (*.exe;*.com;*.bat)\0*.exe;*.com;*.bat\0\0");
				OpenFileName.lpstrFile = szName;
				OpenFileName.nMaxFile = _MAX_PATH;
				OpenFileName.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_FILEMUSTEXIST;
				if (!GetOpenFileName(&OpenFileName))
					return 0;
				SetDlgItemText(hwndDlg, IDC_PRG, szName);
			}
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;
		case IDC_BTNADD:
			acc_num++;
			acc = (Account *)realloc(acc, acc_num * sizeof(Account));
			curIndex = SendMessageA(hwndCombo, CB_ADDSTRING, 0, (LONG)"");
			memset(&acc[curIndex], 0, sizeof(Account));
			SendMessage(hwndCombo, CB_SETCURSEL, curIndex, 0);
			SetDlgItemTextA(hwndDlg, IDC_PASS, "");
			SetFocus(hwndCombo);
			acc[curIndex].hContact = CallService(MS_DB_CONTACT_ADD, 0, 0);
			CallService(MS_PROTO_ADDTOCONTACT, (WPARAM)acc[curIndex].hContact, (LPARAM)pluginName);
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;
		
		case IDC_BTNSAV:
			if (GetDlgItemTextA(hwndDlg, IDC_NAME, acc[curIndex].name, 64)) {
				tail = strstr(acc[curIndex].name, "@");
				if (tail && lstrcmpA(tail + 1, "gmail.com") != 0)
					lstrcpyA(acc[curIndex].hosted, tail + 1);
				SendMessageA(hwndCombo, CB_DELETESTRING, curIndex, 0);
				SendMessageA(hwndCombo, CB_INSERTSTRING, curIndex, (LONG_PTR)acc[curIndex].name);
				SendMessageA(hwndCombo, CB_SETCURSEL, curIndex, 0);
				db_set_s(acc[curIndex].hContact, pluginName, "name", acc[curIndex].name);
				db_set_s(acc[curIndex].hContact, pluginName, "Nick", acc[curIndex].name);
				GetDlgItemTextA(hwndDlg, IDC_PASS, acc[curIndex].pass, 64);
				db_set_s(acc[curIndex].hContact, pluginName, "Password", acc[curIndex].pass);
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			}
			break;

		case IDC_BTNDEL:
			acc_num--;
			SendMessage(hwndCombo, CB_DELETESTRING, curIndex, 0);
			DeleteResults(acc[curIndex].results.next);
			acc[curIndex].results.next = NULL;
			CallService(MS_DB_CONTACT_DELETE, (WPARAM)acc[curIndex].hContact, 0);
			for (int i = curIndex; i < acc_num; i++)
				acc[i] = acc[i + 1];
			curIndex = 0;
			SendMessage(hwndCombo, CB_SETCURSEL, 0, 0);
			SetDlgItemTextA(hwndDlg, IDC_PASS, acc[0].pass);
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;

		case IDC_NAME:
			if (HIWORD(wParam) == CBN_SELCHANGE) {
				curIndex = SendMessage(hwndCombo, CB_GETCURSEL, 0, 0);
				SetDlgItemTextA(hwndDlg, IDC_PASS, acc[curIndex].pass);
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			}
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
			SendMessage(hwndDlg, WM_COMMAND, IDC_BTNSAV, 0);
			opt.circleTime = GetDlgItemInt(hwndDlg, IDC_CIRCLE, NULL, FALSE);
			if (opt.circleTime > 0) {
				KillTimer(NULL, hTimer);
				hTimer = SetTimer(NULL, 0, opt.circleTime * 60000, TimerProc);
				db_set_dw(NULL, pluginName, "circleTime", opt.circleTime);
			}
			opt.notifierOnTray = IsDlgButtonChecked(hwndDlg, IDC_OPTTRAY);
			opt.notifierOnPop = IsDlgButtonChecked(hwndDlg, IDC_OPTPOP);
			db_set_dw(NULL, pluginName, "notifierOnTray", opt.notifierOnTray);
			db_set_dw(NULL, pluginName, "notifierOnPop", opt.notifierOnPop);

			opt.popupDuration = GetDlgItemInt(hwndDlg, IDC_DURATION, NULL, TRUE);
			db_set_dw(NULL, pluginName, "popupDuration", opt.popupDuration);

			opt.popupBgColor = SendDlgItemMessage(hwndDlg, IDC_BGCOLOR, CPM_GETCOLOUR, 0, opt.popupBgColor);
			opt.popupTxtColor = SendDlgItemMessage(hwndDlg, IDC_TEXTCOLOR, CPM_GETCOLOUR, 0, opt.popupBgColor);
			db_set_dw(NULL, pluginName, "popupBgColor", opt.popupBgColor);
			db_set_dw(NULL, pluginName, "popupTxtColor", opt.popupTxtColor);

			if (IsDlgButtonChecked(hwndDlg, IDC_SYSDEF) == BST_CHECKED)
				opt.OpenUsePrg = 0;
			else if (IsDlgButtonChecked(hwndDlg, IDC_USEIE) == BST_CHECKED)
				opt.OpenUsePrg = 1;
			else if (IsDlgButtonChecked(hwndDlg, IDC_STARTPRG) == BST_CHECKED) {
				opt.OpenUsePrg = 2;
			}
			GetDlgItemTextA(hwndDlg, IDC_PRG, str, MAX_PATH);

			db_set_dw(NULL, pluginName, "OpenUsePrg", opt.OpenUsePrg);
			db_set_s(NULL, pluginName, "OpenUsePrgPath", str);

			opt.ShowCustomIcon = IsDlgButtonChecked(hwndDlg, IDC_SHOWICON);
			opt.UseOnline = IsDlgButtonChecked(hwndDlg, IDC_ONLINE);
			if (IsDlgButtonChecked(hwndDlg, IDC_AUTOLOGIN) == BST_CHECKED)
				opt.AutoLogin = 0;
			else if (IsDlgButtonChecked(hwndDlg, IDC_AUTOLOGIN) == BST_UNCHECKED)
				opt.AutoLogin = 1;
			else if (IsDlgButtonChecked(hwndDlg, IDC_AUTOLOGIN) == BST_INDETERMINATE)
				opt.AutoLogin = 2;
			opt.LogThreads = IsDlgButtonChecked(hwndDlg, IDC_LOGTHREADS);
			db_set_dw(NULL, pluginName, "ShowCustomIcon", opt.ShowCustomIcon);
			db_set_dw(NULL, pluginName, "UseOnline", opt.UseOnline);
			db_set_dw(NULL, pluginName, "AutoLogin", opt.AutoLogin);
			db_set_dw(NULL, pluginName, "LogThreads", opt.LogThreads);

			ID_STATUS_NONEW = opt.UseOnline ? ID_STATUS_ONLINE : ID_STATUS_OFFLINE;
			for (int i = 0; i < acc_num; i++)
				db_set_w(acc[i].hContact, pluginName, "Status", ID_STATUS_NONEW);
		}
		return TRUE;

	case WM_DESTROY:
		optionWindowIsOpen = FALSE;
		return TRUE;
	}
	return FALSE;
}

int OptInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp;

	ZeroMemory(&odp, sizeof(odp));
	odp.cbSize = sizeof(odp);
	odp.position = -790000000;
	odp.hInstance = hInst;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT);
	odp.pszTitle = LPGEN("GmailNotifier");
	odp.pszGroup = LPGEN("Network");
	odp.flags = ODPF_BOLDGROUPS;
	odp.pfnDlgProc = DlgProcOpts;
	Options_AddPage(wParam, &odp);
	return 0;
}
