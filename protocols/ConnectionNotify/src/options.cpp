#include "stdafx.h"

BOOL bOptionsOpen = FALSE;
static CONNECTION *connExceptionsTmp = nullptr;
static CONNECTION *connCurrentEditModal = nullptr;

void fillExceptionsListView(HWND hwndDlg)
{
	LVITEM lvI = { 0 };

	int i = 0;
	CONNECTION *tmp = connExceptionsTmp;
	HWND hwndList = GetDlgItem(hwndDlg, IDC_LIST_EXCEPTIONS);
	ListView_DeleteAllItems(hwndList);

	// Some code to create the list-view control.
	// Initialize LVITEM members that are common to all
	// items. 
	lvI.mask = LVIF_TEXT;
	while (tmp) {
		wchar_t tmpAddress[25];
		lvI.iItem = i++;
		lvI.iSubItem = 0;
		lvI.pszText = tmp->PName;
		ListView_InsertItem(hwndList, &lvI);
		lvI.iSubItem = 1;
		if (tmp->intIntPort == -1)
			mir_snwprintf(tmpAddress, L"%s:*", tmp->strIntIp);
		else
			mir_snwprintf(tmpAddress, L"%s:%d", tmp->strIntIp, tmp->intIntPort);
		lvI.pszText = tmpAddress;
		ListView_SetItem(hwndList, &lvI);
		lvI.iSubItem = 2;
		if (tmp->intExtPort == -1)
			mir_snwprintf(tmpAddress, L"%s:*", tmp->strExtIp);
		else
			mir_snwprintf(tmpAddress, L"%s:%d", tmp->strExtIp, tmp->intExtPort);
		lvI.pszText = tmpAddress;
		ListView_SetItem(hwndList, &lvI);
		lvI.iSubItem = 3;
		lvI.pszText = tmp->Pid ? LPGENW("Show") : LPGENW("Hide");
		ListView_SetItem(hwndList, &lvI);

		tmp = tmp->next;
	}

}

//filter editor dialog box procedure opened modally from options dialog
static INT_PTR CALLBACK FilterEditProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
	case WM_INITDIALOG:
		{
			CONNECTION *conn = (CONNECTION *)lParam;
			TranslateDialogDefault(hWnd);
			connCurrentEditModal = conn;
			SetDlgItemText(hWnd, ID_TEXT_NAME, conn->PName);
			SetDlgItemText(hWnd, ID_TXT_LOCAL_IP, conn->strIntIp);
			SetDlgItemText(hWnd, ID_TXT_REMOTE_IP, conn->strExtIp);

			if (conn->intIntPort == -1)
				SetDlgItemText(hWnd, ID_TXT_LOCAL_PORT, L"*");
			else
				SetDlgItemInt(hWnd, ID_TXT_LOCAL_PORT, conn->intIntPort, FALSE);

			if (conn->intExtPort == -1)
				SetDlgItemText(hWnd, ID_TXT_REMOTE_PORT, L"*");
			else
				SetDlgItemInt(hWnd, ID_TXT_REMOTE_PORT, conn->intExtPort, FALSE);

			SendDlgItemMessage(hWnd, ID_CBO_ACTION, CB_ADDSTRING, 0, (LPARAM)TranslateT("Always show popup"));
			SendDlgItemMessage(hWnd, ID_CBO_ACTION, CB_ADDSTRING, 0, (LPARAM)TranslateT("Never show popup"));
			SendDlgItemMessage(hWnd, ID_CBO_ACTION, CB_SETCURSEL, conn->Pid == 0 ? 1 : 0, 0);
		}
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case ID_OK:
			wchar_t tmpPort[6];
			GetDlgItemText(hWnd, ID_TXT_LOCAL_PORT, tmpPort, _countof(tmpPort));
			if (tmpPort[0] == '*')
				connCurrentEditModal->intIntPort = -1;
			else
				connCurrentEditModal->intIntPort = GetDlgItemInt(hWnd, ID_TXT_LOCAL_PORT, nullptr, FALSE);

			GetDlgItemText(hWnd, ID_TXT_REMOTE_PORT, tmpPort, _countof(tmpPort));
			if (tmpPort[0] == '*')
				connCurrentEditModal->intExtPort = -1;
			else
				connCurrentEditModal->intExtPort = GetDlgItemInt(hWnd, ID_TXT_REMOTE_PORT, nullptr, FALSE);

			GetDlgItemText(hWnd, ID_TXT_LOCAL_IP, connCurrentEditModal->strIntIp, _countof(connCurrentEditModal->strIntIp));
			GetDlgItemText(hWnd, ID_TXT_REMOTE_IP, connCurrentEditModal->strExtIp, _countof(connCurrentEditModal->strExtIp));
			GetDlgItemText(hWnd, ID_TEXT_NAME, connCurrentEditModal->PName, _countof(connCurrentEditModal->PName));

			connCurrentEditModal->Pid = !(BOOL)SendDlgItemMessage(hWnd, ID_CBO_ACTION, CB_GETCURSEL, 0, 0);

			connCurrentEditModal = nullptr;
			EndDialog(hWnd, IDOK);
			return TRUE;

		case ID_CANCEL:
			connCurrentEditModal = nullptr;
			EndDialog(hWnd, IDCANCEL);
			return TRUE;
		}
		return FALSE;

	case WM_CLOSE:
		connCurrentEditModal = nullptr;
		EndDialog(hWnd, IDCANCEL);
		break;
	}
	return FALSE;
}

//options page on miranda called
INT_PTR CALLBACK DlgProcConnectionNotifyOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HWND hwndList;
	switch (msg) {
	case WM_INITDIALOG:
		{
			LVCOLUMN lvc = { 0 };
			LVITEM lvI = { 0 };

			bOptionsOpen = TRUE;
			TranslateDialogDefault(hwndDlg);//translate miranda function

			LoadSettings();

			SetDlgItemInt(hwndDlg, IDC_INTERVAL, g_plugin.iInterval, FALSE);
			SetDlgItemInt(hwndDlg, IDC_INTERVAL1, g_plugin.iInterval1, TRUE);
			CheckDlgButton(hwndDlg, IDC_SETCOLOURS, g_plugin.bSetColours ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_RESOLVEIP, g_plugin.bResolveIp ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, ID_CHK_DEFAULTACTION, g_plugin.iDefaultAction ? BST_CHECKED : BST_UNCHECKED);

			SendDlgItemMessage(hwndDlg, IDC_BGCOLOR, CPM_SETCOLOUR, 0, (LPARAM)g_plugin.BgColor);
			SendDlgItemMessage(hwndDlg, IDC_FGCOLOR, CPM_SETCOLOUR, 0, (LPARAM)g_plugin.FgColor);
			if (!g_plugin.bSetColours) {
				HWND hwnd = GetDlgItem(hwndDlg, IDC_BGCOLOR);
				CheckDlgButton(hwndDlg, IDC_SETCOLOURS, BST_UNCHECKED);
				EnableWindow(hwnd, FALSE);
				hwnd = GetDlgItem(hwndDlg, IDC_FGCOLOR);
				EnableWindow(hwnd, FALSE);
			}
			SendDlgItemMessage(hwndDlg, ID_ADD, BM_SETIMAGE, IMAGE_ICON, (LPARAM)LoadImage(g_plugin.getInst(), MAKEINTRESOURCE(IDI_ICON6), IMAGE_ICON, 16, 16, 0));
			SendDlgItemMessage(hwndDlg, ID_DELETE, BM_SETIMAGE, IMAGE_ICON, (LPARAM)LoadImage(g_plugin.getInst(), MAKEINTRESOURCE(IDI_ICON3), IMAGE_ICON, 16, 16, 0));
			SendDlgItemMessage(hwndDlg, ID_DOWN, BM_SETIMAGE, IMAGE_ICON, (LPARAM)LoadImage(g_plugin.getInst(), MAKEINTRESOURCE(IDI_ICON4), IMAGE_ICON, 16, 16, 0));
			SendDlgItemMessage(hwndDlg, ID_UP, BM_SETIMAGE, IMAGE_ICON, (LPARAM)LoadImage(g_plugin.getInst(), MAKEINTRESOURCE(IDI_ICON5), IMAGE_ICON, 16, 16, 0));

			// initialise and fill listbox
			hwndList = GetDlgItem(hwndDlg, IDC_STATUS);
			ListView_DeleteAllItems(hwndList);
			SendMessage(hwndList, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES);

			// Initialize the LVCOLUMN structure.
			// The mask specifies that the format, width, text, and
			// subitem members of the structure are valid. 
			lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
			lvc.fmt = LVCFMT_LEFT;
			lvc.iSubItem = 0;
			lvc.pszText = TranslateT("Status");
			lvc.cx = 120;     // width of column in pixels
			ListView_InsertColumn(hwndList, 0, &lvc);

			// Some code to create the list-view control.
			// Initialize LVITEM members that are common to all
			// items. 
			lvI.mask = LVIF_TEXT;
			for (int i = 0; i < MAX_STATUS_COUNT; i++) {
				lvI.pszText = Clist_GetStatusModeDescription(ID_STATUS_ONLINE + i, 0);
				lvI.iItem = i;
				ListView_InsertItem(hwndList, &lvI);
				ListView_SetCheckState(hwndList, i, g_plugin.iStatus[i]);
			}

			connExceptionsTmp = LoadSettingsConnections();
			hwndList = GetDlgItem(hwndDlg, IDC_LIST_EXCEPTIONS);
			SendMessage(hwndList, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);

			lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
			lvc.fmt = LVCFMT_LEFT;
			lvc.iSubItem = 0;
			lvc.cx = 120;     // width of column in pixels
			lvc.pszText = TranslateT("Application");
			ListView_InsertColumn(hwndList, 1, &lvc);
			lvc.pszText = TranslateT("Internal socket");
			ListView_InsertColumn(hwndList, 2, &lvc);
			lvc.pszText = TranslateT("External socket");
			ListView_InsertColumn(hwndList, 3, &lvc);
			lvc.pszText = TranslateT("Action");
			lvc.cx = 50;
			ListView_InsertColumn(hwndList, 4, &lvc);

			// fill exceptions list
			fillExceptionsListView(hwndDlg);
		}
		break;

	case WM_COMMAND://user changed something, so get changes to variables
		PostMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		switch (LOWORD(wParam)) {
		case IDC_INTERVAL: g_plugin.iInterval = GetDlgItemInt(hwndDlg, IDC_INTERVAL, nullptr, FALSE); break;
		case IDC_INTERVAL1: g_plugin.iInterval1 = GetDlgItemInt(hwndDlg, IDC_INTERVAL1, nullptr, TRUE); break;
		case IDC_RESOLVEIP: g_plugin.bResolveIp = 0 != IsDlgButtonChecked(hwndDlg, IDC_RESOLVEIP); break;
		case ID_CHK_DEFAULTACTION: g_plugin.iDefaultAction = (uint8_t)IsDlgButtonChecked(hwndDlg, ID_CHK_DEFAULTACTION); break;
		case ID_ADD:
			{
				CONNECTION *cur = (CONNECTION *)mir_alloc(sizeof(CONNECTION));
				memset(cur, 0, sizeof(CONNECTION));
				cur->intExtPort = -1;
				cur->intIntPort = -1;
				cur->Pid = 0;
				cur->PName[0] = '*';
				cur->strExtIp[0] = '*';
				cur->strIntIp[0] = '*';

				if (DialogBoxParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_FILTER_DIALOG), hwndDlg, FilterEditProc, (LPARAM)cur) == IDCANCEL) {
					mir_free(cur);
					cur = nullptr;
				}
				else {
					cur->next = connExceptionsTmp;
					connExceptionsTmp = cur;
				}

				fillExceptionsListView(hwndDlg);
				ListView_SetItemState(GetDlgItem(hwndDlg, IDC_LIST_EXCEPTIONS), 0, LVNI_FOCUSED | LVIS_SELECTED, LVNI_FOCUSED | LVIS_SELECTED);
				SetFocus(GetDlgItem(hwndDlg, IDC_LIST_EXCEPTIONS));
			}
			break;

		case ID_DELETE:
			{
				int pos, pos1;
				CONNECTION *cur = connExceptionsTmp, *pre = nullptr;

				pos = (int)ListView_GetNextItem(GetDlgItem(hwndDlg, IDC_LIST_EXCEPTIONS), -1, LVNI_FOCUSED);
				if (pos == -1)break;
				pos1 = pos;
				while (pos--) {
					pre = cur;
					cur = cur->next;
				}
				if (pre == nullptr)
					connExceptionsTmp = connExceptionsTmp->next;
				else
					(pre)->next = cur->next;
				mir_free(cur);
				fillExceptionsListView(hwndDlg);
				ListView_SetItemState(GetDlgItem(hwndDlg, IDC_LIST_EXCEPTIONS), pos1, LVNI_FOCUSED | LVIS_SELECTED, LVNI_FOCUSED | LVIS_SELECTED);
				SetFocus(GetDlgItem(hwndDlg, IDC_LIST_EXCEPTIONS));
			}
			break;

		case ID_UP:
			{
				int pos, pos1;
				CONNECTION *cur = nullptr, *pre = nullptr, *prepre = nullptr;

				cur = connExceptionsTmp;

				pos = (int)ListView_GetNextItem(GetDlgItem(hwndDlg, IDC_LIST_EXCEPTIONS), -1, LVNI_FOCUSED);
				if (pos == -1)break;
				pos1 = pos;
				while (pos--) {
					prepre = pre;
					pre = cur;
					cur = cur->next;
				}
				if (prepre != nullptr) {
					pre->next = cur->next;
					cur->next = pre;
					prepre->next = cur;
				}
				else if (pre != nullptr) {
					pre->next = cur->next;
					cur->next = pre;
					connExceptionsTmp = cur;
				}
				fillExceptionsListView(hwndDlg);
				ListView_SetItemState(GetDlgItem(hwndDlg, IDC_LIST_EXCEPTIONS), pos1 - 1, LVNI_FOCUSED | LVIS_SELECTED, LVNI_FOCUSED | LVIS_SELECTED);
				SetFocus(GetDlgItem(hwndDlg, IDC_LIST_EXCEPTIONS));
			}
			break;

		case ID_DOWN:
			{
				int pos, pos1;
				CONNECTION *cur = nullptr, *pre = nullptr;

				cur = connExceptionsTmp;

				pos = (int)ListView_GetNextItem(GetDlgItem(hwndDlg, IDC_LIST_EXCEPTIONS), -1, LVNI_FOCUSED);
				if (pos == -1)break;
				pos1 = pos;
				while (pos--) {
					pre = cur;
					cur = cur->next;
				}
				if (cur == connExceptionsTmp && cur->next != nullptr) {
					connExceptionsTmp = cur->next;
					cur->next = cur->next->next;
					connExceptionsTmp->next = cur;
				}
				else if (cur->next != nullptr) {
					CONNECTION *tmp = cur->next->next;
					pre->next = cur->next;
					cur->next->next = cur;
					cur->next = tmp;
				}
				fillExceptionsListView(hwndDlg);
				ListView_SetItemState(GetDlgItem(hwndDlg, IDC_LIST_EXCEPTIONS), pos1 + 1, LVNI_FOCUSED | LVIS_SELECTED, LVNI_FOCUSED | LVIS_SELECTED);
				SetFocus(GetDlgItem(hwndDlg, IDC_LIST_EXCEPTIONS));
			}
			break;

		case IDC_SETCOLOURS:
			g_plugin.bSetColours = 0 != IsDlgButtonChecked(hwndDlg, IDC_SETCOLOURS);
			EnableWindow(GetDlgItem(hwndDlg, IDC_BGCOLOR), g_plugin.bSetColours);
			EnableWindow(GetDlgItem(hwndDlg, IDC_FGCOLOR), g_plugin.bSetColours);
			break;

		case IDC_BGCOLOR:
			g_plugin.BgColor = (COLORREF)SendDlgItemMessage(hwndDlg, IDC_BGCOLOR, CPM_GETCOLOUR, 0, 0);
			break;
		
		case IDC_FGCOLOR:
			g_plugin.FgColor = (COLORREF)SendDlgItemMessage(hwndDlg, IDC_FGCOLOR, CPM_GETCOLOUR, 0, 0);
			break;
		}
		break;

	case WM_NOTIFY://apply changes so write it to db
		switch (((LPNMHDR)lParam)->idFrom) {
		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_RESET:
				LoadSettings();
				deleteConnectionsTable(connExceptionsTmp);
				connExceptionsTmp = LoadSettingsConnections();
				return TRUE;

			case PSN_APPLY:
				g_plugin.setDword("Interval", g_plugin.iInterval);
				g_plugin.setDword("PopupInterval", g_plugin.iInterval1);
				g_plugin.setByte("PopupSetColours", g_plugin.bSetColours);
				g_plugin.setDword("PopupBgColor", g_plugin.BgColor);
				g_plugin.setDword("PopupFgColor", g_plugin.FgColor);
				g_plugin.setByte("ResolveIp", g_plugin.bResolveIp);
				g_plugin.setByte("FilterDefaultAction", g_plugin.iDefaultAction);

				for (int i = 0; i < MAX_STATUS_COUNT; i++) {
					char buff[128];
					mir_snprintf(buff, "Status%d", i);
					g_plugin.iStatus[i] = (ListView_GetCheckState(GetDlgItem(hwndDlg, IDC_STATUS), i) ? TRUE : FALSE);
					g_plugin.setByte(buff, g_plugin.iStatus[i] ? 1 : 0);
				}

				if (WAIT_OBJECT_0 == WaitForSingleObject(hExceptionsMutex, 100)) {
					deleteConnectionsTable(connExceptions);
					saveSettingsConnections(connExceptionsTmp);
					connExceptions = connExceptionsTmp;
					connExceptionsTmp = LoadSettingsConnections();
					ReleaseMutex(hExceptionsMutex);
				}
				return TRUE;
			}
			break;
		}

		if (GetDlgItem(hwndDlg, IDC_LIST_EXCEPTIONS) == ((LPNMHDR)lParam)->hwndFrom) {
			switch (((LPNMHDR)lParam)->code) {
			case NM_DBLCLK:
				{
					int pos, pos1;
					CONNECTION *cur = nullptr;

					cur = connExceptionsTmp;

					pos = (int)ListView_GetNextItem(GetDlgItem(hwndDlg, IDC_LIST_EXCEPTIONS), -1, LVNI_FOCUSED);
					if (pos == -1)break;
					pos1 = pos;
					while (pos--) {
						cur = cur->next;
					}
					DialogBoxParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_FILTER_DIALOG), hwndDlg, FilterEditProc, (LPARAM)cur);
					fillExceptionsListView(hwndDlg);
					ListView_SetItemState(GetDlgItem(hwndDlg, IDC_LIST_EXCEPTIONS), pos1, LVNI_FOCUSED | LVIS_SELECTED, LVNI_FOCUSED | LVIS_SELECTED);
					SetFocus(GetDlgItem(hwndDlg, IDC_LIST_EXCEPTIONS));
					break;
				}
			}
		}

		if (GetDlgItem(hwndDlg, IDC_STATUS) == ((LPNMHDR)lParam)->hwndFrom) {
			switch (((LPNMHDR)lParam)->code) {
			case LVN_ITEMCHANGED:
				NMLISTVIEW *nmlv = (NMLISTVIEW *)lParam;
				if ((nmlv->uNewState ^ nmlv->uOldState) & LVIS_STATEIMAGEMASK)
					SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				break;
			}
		}
		break;

	case WM_DESTROY:
		bOptionsOpen = FALSE;
		deleteConnectionsTable(connExceptionsTmp);
		connExceptionsTmp = nullptr;
		return TRUE;
	}
	return 0;
}

// options page on miranda called
int ConnectionNotifyOptInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_DIALOG);
	odp.szTitle.w = _A2W(PLUGINNAME);
	odp.szGroup.w = LPGENW("Plugins");
	odp.flags = ODPF_BOLDGROUPS | ODPF_UNICODE;
	odp.pfnDlgProc = DlgProcConnectionNotifyOpts;//callback function name
	g_plugin.addOptions(wParam, &odp);
	return 0;
}
