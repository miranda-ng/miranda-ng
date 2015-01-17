/*
Exchange notifier plugin for Miranda IM

Copyright © 2006 Cristian Libotean, Attila Vajda

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "commonheaders.h"
#include "dlg_handlers.h"

#define MIN_EMAILS_WIDTH 300
#define MIN_EMAILS_HEIGHT 250

static WNDPROC OldListProc;

INT_PTR CALLBACK DlgProcOptions(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static int bInitializing; //true when dialog is being created
	
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hWnd);
		{
			bInitializing = 1;
			DBVARIANT dbv = {0};
			dbv.type = DBVT_ASCIIZ;

			int bCheck = db_get_b(NULL, ModuleName, "Check", 1);

			TCHAR buffer[4096];
			GetStringFromDatabase("Username", _T(""), buffer, SIZEOF(buffer));
			SetDlgItemText(hWnd, IDC_USER_EDIT, buffer);

			GetStringFromDatabase("Password", _T(""), buffer, SIZEOF(buffer));
			SetDlgItemText(hWnd, IDC_PASSWORD_EDIT, buffer);

			GetStringFromDatabase("Server", _T(""), buffer, SIZEOF(buffer));
			SetDlgItemText(hWnd, IDC_SERVER_EDIT, buffer);

			SetDlgItemInt(hWnd, IDC_PORT_EDIT, db_get_dw(NULL, ModuleName, "Port", EXCHANGE_PORT), FALSE);

			SetDlgItemInt(hWnd, IDC_INTERVAL_EDIT, db_get_dw(NULL, ModuleName, "Interval", DEFAULT_INTERVAL), FALSE);

			CheckDlgButton(hWnd, IDC_RECONNECT, (db_get_b(NULL, ModuleName, "Reconnect", 0)) ? BST_CHECKED : BST_UNCHECKED);

			SetDlgItemInt(hWnd, IDC_RECONNECT_INTERVAL, db_get_dw(NULL, ModuleName, "ReconnectInterval", DEFAULT_RECONNECT_INTERVAL), FALSE);
			CheckDlgButton(hWnd, IDC_USE_POPUPS, (BOOL) db_get_b(NULL, ModuleName, "UsePopups", 0) ? BST_CHECKED : BST_UNCHECKED);
			EnableWindow(GetDlgItem(hWnd, IDC_USE_POPUPS), ServiceExists(MS_POPUP_ADDPOPUPT)); //disable the popups checkbox if no popup module is present

			CheckDlgButton(hWnd, IDC_CHECK_EMAILS, (bCheck) ? BST_CHECKED : BST_UNCHECKED);
			EnableWindow(GetDlgItem(hWnd, IDC_INTERVAL_EDIT), bCheck);

			int portCheck = db_get_b(NULL, ModuleName, "UsePortCheck", 1);
			CheckDlgButton(hWnd, IDC_USE_PORTCHECK, (portCheck) ? BST_CHECKED : BST_UNCHECKED);
			EnableWindow(GetDlgItem(hWnd, IDC_PORT_EDIT), portCheck);

			int retries = db_get_b(NULL, ModuleName, "MaxRetries", MAX_EXCHANGE_CONNECT_RETRIES);
			SetDlgItemInt(hWnd,IDC_MAX_RETRIES,retries,FALSE);

			EnableWindow(GetDlgItem(hWnd, IDC_RECONNECT_INTERVAL), IsDlgButtonChecked(hWnd, IDC_RECONNECT));

			bInitializing = 0;
		}
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_USER_EDIT:
		case IDC_PASSWORD_EDIT:
		case IDC_SERVER_EDIT:
		case IDC_PORT_EDIT:
		case IDC_INTERVAL_EDIT:
		case IDC_RECONNECT_INTERVAL:
		case IDC_MAX_RETRIES:
			if ((!bInitializing) && (HIWORD(wParam) == EN_CHANGE))// || (HIWORD(wParam) == CBN_SELENDOK))
				SendMessage(GetParent(hWnd), PSM_CHANGED, 0, 0);
			break;

		case IDC_USE_POPUPS:
		case IDC_CHECK_EMAILS:
		case IDC_RECONNECT:
		case IDC_USE_PORTCHECK:
			int portCheck = IsDlgButtonChecked(hWnd, IDC_USE_PORTCHECK);
			SendMessage(GetParent(hWnd), PSM_CHANGED, 0, 0);

			EnableWindow(GetDlgItem(hWnd, IDC_INTERVAL_EDIT), IsDlgButtonChecked(hWnd, IDC_CHECK_EMAILS));
			EnableWindow(GetDlgItem(hWnd, IDC_RECONNECT_INTERVAL), IsDlgButtonChecked(hWnd, IDC_RECONNECT));
			EnableWindow(GetDlgItem(hWnd, IDC_PORT_EDIT), portCheck);
			break;
		}
		break;

	case WM_NOTIFY:
		switch(((LPNMHDR)lParam)->idFrom) {
		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_APPLY:
				db_set_b(NULL, ModuleName, "Check", IsDlgButtonChecked(hWnd, IDC_CHECK_EMAILS));

				TCHAR buffer[4096];
				GetDlgItemText(hWnd, IDC_USER_EDIT, buffer, SIZEOF(buffer));
				db_set_ts(NULL, ModuleName, "Username", buffer);

				GetDlgItemText(hWnd, IDC_PASSWORD_EDIT, buffer, SIZEOF(buffer));
				db_set_ts(NULL, ModuleName, "Password", buffer);

				GetDlgItemText(hWnd, IDC_SERVER_EDIT, buffer, SIZEOF(buffer));
				db_set_ts(NULL, ModuleName, "Server", buffer);

				GetDlgItemText(hWnd, IDC_PORT_EDIT, buffer, SIZEOF(buffer));
				db_set_dw(NULL, ModuleName, "Port", GetDlgItemInt(hWnd,IDC_PORT_EDIT,NULL,FALSE));

				db_set_dw(NULL, ModuleName, "Interval", GetDlgItemInt(hWnd,IDC_INTERVAL_EDIT,NULL,FALSE));
				db_set_dw(NULL, ModuleName, "ReconnectInterval", GetDlgItemInt(hWnd,IDC_RECONNECT_INTERVAL,NULL,FALSE));

				db_set_b(NULL, ModuleName, "Reconnect", IsDlgButtonChecked(hWnd, IDC_RECONNECT));

				db_set_b(NULL, ModuleName, "UsePopups", IsDlgButtonChecked(hWnd, IDC_USE_POPUPS));
				db_set_b(NULL, ModuleName, "UsePortCheck", IsDlgButtonChecked(hWnd, IDC_USE_PORTCHECK));

				db_set_b(NULL, ModuleName, "MaxRetries", GetDlgItemInt(hWnd,IDC_MAX_RETRIES,NULL,FALSE));

				exchangeServer.Reconnect(); //login info may be changed
				UpdateTimers(); //interval might get changed
			}
		}

		break;
	}

	return 0;
}

#include "commctrl.h"

void AddAnchorWindowToDeferList(HDWP &hdWnds, HWND window, RECT *rParent, WINDOWPOS *wndPos, int anchors)
{
	RECT rChild = AnchorCalcPos(window, rParent, wndPos, anchors);
	hdWnds = DeferWindowPos(hdWnds, window, HWND_NOTOPMOST, rChild.left, rChild.top, rChild.right - rChild.left, rChild.bottom - rChild.top, SWP_NOZORDER);
}

int CALLBACK ListSubclassProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_KEYUP:
		if (wParam == VK_ESCAPE)
			SendMessage(GetParent(hWnd), WM_CLOSE, 0, 0);

		break;

	case WM_SYSKEYDOWN:
		if (wParam == 'X')
			SendMessage(GetParent(hWnd), WM_CLOSE, 0, 0);						

		break;

	case WM_LBUTTONDBLCLK:
		int count = ListView_GetItemCount(hWnd);
		for (int i = 0; i < count; i++) {
			if (ListView_GetItemState(hWnd, i, LVIS_SELECTED)) {
				TCHAR emailID[4096]; //uhh
				ListView_GetItemText(hWnd, i, 2, emailID, SIZEOF(emailID));
				exchangeServer.OpenMessage(emailID);
			}
		}
		break;
	}

	return CallWindowProc(OldListProc, hWnd, msg, wParam, lParam);
}

INT_PTR CALLBACK DlgProcEmails(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hWnd);
		{
			SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hiMailIcon);

			LVCOLUMN col = {0};
			HWND hList = GetDlgItem(hWnd, IDC_EMAILS_LIST);
			OldListProc = (WNDPROC) SetWindowLongPtr(hList, GWLP_WNDPROC, (LONG_PTR) ListSubclassProc);
			ListView_SetExtendedListViewStyle(hList, LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES);
			col.mask = LVCF_TEXT | LVCF_WIDTH;
			col.cx = 100;
			col.pszText = TranslateT("Entry ID");
			ListView_InsertColumn(hList, 0, &col);
			col.pszText = TranslateT("Subject");
			col.cx = 300;
			ListView_InsertColumn(hList, 0, &col);
			col.cx = 200;
			col.iSubItem = 1;
			col.pszText = TranslateT("Sender");
			ListView_InsertColumn(hList, 0, &col);
		}
		return TRUE;

	case WM_DESTROY:
		hEmailsDlg = NULL;
		break;

	case WM_CLOSE:
		DestroyWindow(hWnd); //close the window - no need to keep it in memory
		break;

	case EXM_UPDATE_EMAILS:
		{
			HWND hList = GetDlgItem(hWnd, IDC_EMAILS_LIST);
			ListView_DeleteAllItems(hList);
			int count = GetWindowLongPtr(hWnd, GWLP_USERDATA);
			LVITEM item = {0};
			TEmailHeader email = {0};
			email.cbSize = sizeof(TEmailHeader);
			TCHAR sender[1024] = _T("");
			TCHAR subject[1024] = _T("");
			//char buffer[4096];
			email.cSender = sizeof(sender);
			email.cSubject = sizeof(subject);
			email.szSender = sender;
			email.szSubject = subject;
			item.mask = LVIF_TEXT;

			for (int i = 0; i < count; i++)
			{
				exchangeServer.GetEmailHeader(i, &email);
				item.iItem = i;
				item.iSubItem = 0;
				item.pszText = email.szSender;
				ListView_InsertItem(hList, &item);
				ListView_SetItemText(hList, i, 1, email.szSubject);
				ListView_SetItemText(hList, i, 2, mir_a2t(email.emailID));
			}
			SetFocus(hList);
		}
		break;

	case WM_SHOWWINDOW:
		if (wParam)
			SendMessage(hWnd, EXM_UPDATE_EMAILS, 0, 0);
		break;

	case WM_KEYUP:
		if (wParam == VK_ESCAPE)
			SendMessage(hWnd, WM_CLOSE, 0, 0);
		break;

	case WM_SYSKEYDOWN:
		if (wParam == 'X')
			SendMessage(hWnd, WM_CLOSE, 0, 0);
		break;

	case WM_WINDOWPOSCHANGING:
		{
			HDWP hdWnds = BeginDeferWindowPos(3);
			RECT rParent;
			WINDOWPOS *wndPos = (WINDOWPOS *) lParam;

			if ((!wndPos) || (wndPos->flags & SWP_NOSIZE))
				break;

			GetWindowRect(hWnd, &rParent);
			if (wndPos->cx < MIN_EMAILS_WIDTH)
				wndPos->cx = MIN_EMAILS_WIDTH;
			if (wndPos->cy < MIN_EMAILS_HEIGHT)
				wndPos->cy = MIN_EMAILS_HEIGHT;

			AddAnchorWindowToDeferList(hdWnds, GetDlgItem(hWnd, IDC_EMAILS_LIST), &rParent, wndPos, ANCHOR_ALL);
			AddAnchorWindowToDeferList(hdWnds, GetDlgItem(hWnd, IDC_CLOSE), &rParent, wndPos, ANCHOR_BOTTOM | ANCHOR_RIGHT);
			AddAnchorWindowToDeferList(hdWnds, GetDlgItem(hWnd, IDC_MARK_READ), &rParent, wndPos, ANCHOR_BOTTOM | ANCHOR_LEFT);

			EndDeferWindowPos(hdWnds);
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_CLOSE:
			SendMessage(hWnd, WM_CLOSE, 0, 0);
			break;

		case IDC_MARK_READ:
			HWND hList = GetDlgItem(hWnd, IDC_EMAILS_LIST);
			int count = ListView_GetItemCount(hList);
			for (int i = 0; i < count; i++) {
				if (ListView_GetCheckState(hList, i)) {
					TCHAR emailID[2048]; //uhh ohh
					LVITEM item = {0};
					item.iItem = i;
					item.mask = LVIF_TEXT;
					item.iSubItem = 2;
					item.cchTextMax = SIZEOF(emailID);
					item.pszText = emailID;
					ListView_GetItem(hList, &item);
					exchangeServer.MarkEmailAsRead(emailID);
				}
			}
			count = exchangeServer.GetUnreadEmailsCount();
			if (count > 0)
			{
				SetWindowLongPtr(hWnd, GWLP_USERDATA, count);
				SendMessage(hWnd, EXM_UPDATE_EMAILS, 0, 0);
			}
			else SendMessage(hWnd, WM_CLOSE, 0, 0);
		}

		break;
	}

	return 0;
}

LRESULT CALLBACK DlgProcPopup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_COMMAND:
		switch (HIWORD(wParam)) {
		case STN_CLICKED:
			{
				int count = (int) PUGetPluginData(hWnd);
				ShowEmailsWindow(count);
				PUDeletePopup(hWnd);
				break;
			}
		}
		break;

	case WM_CONTEXTMENU:
		PUDeletePopup(hWnd);
		break;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}