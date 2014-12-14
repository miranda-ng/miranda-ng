/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-14 Miranda NG project (http://miranda-ng.org)
Copyright (c) 2000-03 Miranda ICQ/IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

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

static INT_PTR CALLBACK DlgProcCluiOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		CheckDlgButton(hwndDlg, IDC_BRINGTOFRONT,
			db_get_b(NULL, "CList", "BringToFront", SETTING_BRINGTOFRONT_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_ONTOP, db_get_b(NULL, "CList", "OnTop", SETTING_ONTOP_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_TOOLWND,
			db_get_b(NULL, "CList", "ToolWindow", SETTING_TOOLWINDOW_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_MIN2TRAY,
			db_get_b(NULL, "CList", "Min2Tray", SETTING_MIN2TRAY_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
		if (IsDlgButtonChecked(hwndDlg, IDC_TOOLWND))
			EnableWindow(GetDlgItem(hwndDlg, IDC_MIN2TRAY), FALSE);
		CheckDlgButton(hwndDlg, IDC_SHOWCAPTION,
			db_get_b(NULL, "CLUI", "ShowCaption", SETTING_SHOWCAPTION_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_SHOWMAINMENU,
			db_get_b(NULL, "CLUI", "ShowMainMenu", SETTING_SHOWMAINMENU_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CLIENTDRAG,
			db_get_b(NULL, "CLUI", "ClientAreaDrag", SETTING_CLIENTDRAG_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
		if (BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_SHOWCAPTION)) {
			EnableWindow(GetDlgItem(hwndDlg, IDC_MIN2TRAY), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_TOOLWND), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_TITLETEXT), FALSE);
		}
		CheckDlgButton(hwndDlg, IDC_FADEINOUT, db_get_b(NULL, "CLUI", "FadeInOut", 0) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_AUTOSIZE, db_get_b(NULL, "CLUI", "AutoSize", 0) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_DROPSHADOW, db_get_b(NULL, "CList", "WindowShadow", 0) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_ONDESKTOP, db_get_b(NULL, "CList", "OnDesktop", 0) ? BST_CHECKED : BST_UNCHECKED);
		SendDlgItemMessage(hwndDlg, IDC_MAXSIZESPIN, UDM_SETRANGE, 0, MAKELONG(100, 0));
		SendDlgItemMessage(hwndDlg, IDC_MAXSIZESPIN, UDM_SETPOS, 0, db_get_b(NULL, "CLUI", "MaxSizeHeight", 75));
		CheckDlgButton(hwndDlg, IDC_AUTOSIZEUPWARD, db_get_b(NULL, "CLUI", "AutoSizeUpward", 0) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_AUTOHIDE,
			db_get_b(NULL, "CList", "AutoHide", SETTING_AUTOHIDE_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
		SendDlgItemMessage(hwndDlg, IDC_HIDETIMESPIN, UDM_SETRANGE, 0, MAKELONG(900, 1));
		SendDlgItemMessage(hwndDlg, IDC_HIDETIMESPIN, UDM_SETPOS, 0,
			MAKELONG(db_get_w(NULL, "CList", "HideTime", SETTING_HIDETIME_DEFAULT), 0));
		EnableWindow(GetDlgItem(hwndDlg, IDC_HIDETIME), IsDlgButtonChecked(hwndDlg, IDC_AUTOHIDE));
		EnableWindow(GetDlgItem(hwndDlg, IDC_HIDETIMESPIN), IsDlgButtonChecked(hwndDlg, IDC_AUTOHIDE));
		EnableWindow(GetDlgItem(hwndDlg, IDC_STATIC01), IsDlgButtonChecked(hwndDlg, IDC_AUTOHIDE));
		if (BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_AUTOSIZE)) {
			EnableWindow(GetDlgItem(hwndDlg, IDC_STATIC21), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_STATIC22), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_MAXSIZEHEIGHT), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_MAXSIZESPIN), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_AUTOSIZEUPWARD), FALSE);
		}

		{	DBVARIANT dbv;
			if ( !db_get_ts(NULL, "CList", "TitleText", &dbv)) {
				SetDlgItemText(hwndDlg, IDC_TITLETEXT, dbv.ptszVal);
				db_free( &dbv );
			}
			else SetDlgItemTextA(hwndDlg, IDC_TITLETEXT, MIRANDANAME);
		}
		CheckDlgButton(hwndDlg, IDC_TRANSPARENT, db_get_b(NULL, "CList", "Transparent", SETTING_TRANSPARENT_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);

		if (BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_TRANSPARENT)) {
			EnableWindow(GetDlgItem(hwndDlg, IDC_STATIC11), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_STATIC12), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_TRANSACTIVE), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_TRANSINACTIVE), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_ACTIVEPERC), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_INACTIVEPERC), FALSE);
		}
		SendDlgItemMessage(hwndDlg, IDC_TRANSACTIVE, TBM_SETRANGE, FALSE, MAKELONG(1, 255));
		SendDlgItemMessage(hwndDlg, IDC_TRANSINACTIVE, TBM_SETRANGE, FALSE, MAKELONG(1, 255));
		SendDlgItemMessage(hwndDlg, IDC_TRANSACTIVE, TBM_SETPOS, TRUE, db_get_b(NULL, "CList", "Alpha", SETTING_ALPHA_DEFAULT));
		SendDlgItemMessage(hwndDlg, IDC_TRANSINACTIVE, TBM_SETPOS, TRUE,
			db_get_b(NULL, "CList", "AutoAlpha", SETTING_AUTOALPHA_DEFAULT));
		SendMessage(hwndDlg, WM_HSCROLL, 0x12345678, 0);
		return TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_AUTOHIDE) {
			EnableWindow(GetDlgItem(hwndDlg, IDC_HIDETIME), IsDlgButtonChecked(hwndDlg, IDC_AUTOHIDE));
			EnableWindow(GetDlgItem(hwndDlg, IDC_HIDETIMESPIN), IsDlgButtonChecked(hwndDlg, IDC_AUTOHIDE));
			EnableWindow(GetDlgItem(hwndDlg, IDC_STATIC01), IsDlgButtonChecked(hwndDlg, IDC_AUTOHIDE));
		}
		else if (LOWORD(wParam) == IDC_TRANSPARENT) {
			EnableWindow(GetDlgItem(hwndDlg, IDC_STATIC11), IsDlgButtonChecked(hwndDlg, IDC_TRANSPARENT));
			EnableWindow(GetDlgItem(hwndDlg, IDC_STATIC12), IsDlgButtonChecked(hwndDlg, IDC_TRANSPARENT));
			EnableWindow(GetDlgItem(hwndDlg, IDC_TRANSACTIVE), IsDlgButtonChecked(hwndDlg, IDC_TRANSPARENT));
			EnableWindow(GetDlgItem(hwndDlg, IDC_TRANSINACTIVE), IsDlgButtonChecked(hwndDlg, IDC_TRANSPARENT));
			EnableWindow(GetDlgItem(hwndDlg, IDC_ACTIVEPERC), IsDlgButtonChecked(hwndDlg, IDC_TRANSPARENT));
			EnableWindow(GetDlgItem(hwndDlg, IDC_INACTIVEPERC), IsDlgButtonChecked(hwndDlg, IDC_TRANSPARENT));
		}
		else if (LOWORD(wParam) == IDC_AUTOSIZE) {
			EnableWindow(GetDlgItem(hwndDlg, IDC_STATIC21), IsDlgButtonChecked(hwndDlg, IDC_AUTOSIZE));
			EnableWindow(GetDlgItem(hwndDlg, IDC_STATIC22), IsDlgButtonChecked(hwndDlg, IDC_AUTOSIZE));
			EnableWindow(GetDlgItem(hwndDlg, IDC_MAXSIZEHEIGHT), IsDlgButtonChecked(hwndDlg, IDC_AUTOSIZE));
			EnableWindow(GetDlgItem(hwndDlg, IDC_MAXSIZESPIN), IsDlgButtonChecked(hwndDlg, IDC_AUTOSIZE));
			EnableWindow(GetDlgItem(hwndDlg, IDC_AUTOSIZEUPWARD), IsDlgButtonChecked(hwndDlg, IDC_AUTOSIZE));
		}
		else if (LOWORD(wParam) == IDC_TOOLWND) {
			EnableWindow(GetDlgItem(hwndDlg, IDC_MIN2TRAY), BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_TOOLWND));
		}
		else if (LOWORD(wParam) == IDC_SHOWCAPTION) {
			EnableWindow(GetDlgItem(hwndDlg, IDC_TOOLWND), IsDlgButtonChecked(hwndDlg, IDC_SHOWCAPTION));
			EnableWindow(GetDlgItem(hwndDlg, IDC_MIN2TRAY), BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_TOOLWND)
				&& IsDlgButtonChecked(hwndDlg, IDC_SHOWCAPTION));
			EnableWindow(GetDlgItem(hwndDlg, IDC_TITLETEXT), IsDlgButtonChecked(hwndDlg, IDC_SHOWCAPTION));
		}

		if ((LOWORD(wParam) == IDC_HIDETIME || LOWORD(wParam) == IDC_TITLETEXT || LOWORD(wParam) == IDC_MAXSIZEHEIGHT) &&
			(HIWORD(wParam) != EN_CHANGE || (HWND) lParam != GetFocus()))
			return 0;

		// Enable apply button
		SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		break;

	case WM_HSCROLL:
		{
			char str[10];
			mir_snprintf(str, SIZEOF(str), "%d%%", 100 * SendDlgItemMessage(hwndDlg, IDC_TRANSINACTIVE, TBM_GETPOS, 0, 0) / 255);
			SetDlgItemTextA(hwndDlg, IDC_INACTIVEPERC, str);
			mir_snprintf(str, SIZEOF(str), "%d%%", 100 * SendDlgItemMessage(hwndDlg, IDC_TRANSACTIVE, TBM_GETPOS, 0, 0) / 255);
			SetDlgItemTextA(hwndDlg, IDC_ACTIVEPERC, str);
		}
		if (wParam != 0x12345678)
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		break;
	case WM_NOTIFY:
		if (((LPNMHDR) lParam)->code == PSN_APPLY) {
			db_set_b(NULL, "CList", "OnTop", (BYTE) IsDlgButtonChecked(hwndDlg, IDC_ONTOP));
			db_set_b(NULL, "CList", "ToolWindow", (BYTE) IsDlgButtonChecked(hwndDlg, IDC_TOOLWND));
			db_set_b(NULL, "CList", "BringToFront", (BYTE) IsDlgButtonChecked(hwndDlg, IDC_BRINGTOFRONT));
			db_set_b(NULL, "CLUI", "FadeInOut", (BYTE) IsDlgButtonChecked(hwndDlg, IDC_FADEINOUT));
			db_set_b(NULL, "CLUI", "AutoSize", (BYTE) IsDlgButtonChecked(hwndDlg, IDC_AUTOSIZE));
			db_set_b(NULL, "CLUI", "MaxSizeHeight", (BYTE) GetDlgItemInt(hwndDlg, IDC_MAXSIZEHEIGHT, NULL, FALSE));
			db_set_b(NULL, "CLUI", "AutoSizeUpward", (BYTE) IsDlgButtonChecked(hwndDlg, IDC_AUTOSIZEUPWARD));
			db_set_b(NULL, "CList", "AutoHide", (BYTE) IsDlgButtonChecked(hwndDlg, IDC_AUTOHIDE));
			db_set_w(NULL, "CList", "HideTime", (WORD) SendDlgItemMessage(hwndDlg, IDC_HIDETIMESPIN, UDM_GETPOS, 0, 0));
			db_set_b(NULL, "CList", "Transparent", (BYTE) IsDlgButtonChecked(hwndDlg, IDC_TRANSPARENT));
			db_set_b(NULL, "CList", "Alpha", (BYTE) SendDlgItemMessage(hwndDlg, IDC_TRANSACTIVE, TBM_GETPOS, 0, 0));
			db_set_b(NULL, "CList", "AutoAlpha", (BYTE) SendDlgItemMessage(hwndDlg, IDC_TRANSINACTIVE, TBM_GETPOS, 0, 0));
			db_set_b(NULL, "CList", "WindowShadow", (BYTE) IsDlgButtonChecked(hwndDlg, IDC_DROPSHADOW));
			db_set_b(NULL, "CList", "OnDesktop", (BYTE) IsDlgButtonChecked(hwndDlg, IDC_ONDESKTOP));
			db_set_b(NULL, "CLUI", "ShowCaption", (BYTE) IsDlgButtonChecked(hwndDlg, IDC_SHOWCAPTION));
			db_set_b(NULL, "CLUI", "ShowMainMenu", (BYTE) IsDlgButtonChecked(hwndDlg, IDC_SHOWMAINMENU));
			db_set_b(NULL, "CLUI", "ClientAreaDrag", (BYTE) IsDlgButtonChecked(hwndDlg, IDC_CLIENTDRAG));
			db_set_b(NULL, "CList", "Min2Tray", (BYTE) IsDlgButtonChecked(hwndDlg, IDC_MIN2TRAY));
			{
				TCHAR title[256];
				GetDlgItemText(hwndDlg, IDC_TITLETEXT, title, SIZEOF(title));
				db_set_ts(NULL, "CList", "TitleText", title);
				SetWindowText(pcli->hwndContactList, title);
			}
			pcli->pfnLoadCluiGlobalOpts();
			SetWindowPos(pcli->hwndContactList, IsDlgButtonChecked(hwndDlg, IDC_ONTOP) ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0,
				SWP_NOMOVE | SWP_NOSIZE);
			if (IsDlgButtonChecked(hwndDlg, IDC_TOOLWND)) {
				// Window must be hidden to dynamically remove the taskbar button.
				// See http://msdn.microsoft.com/library/en-us/shellcc/platform/shell/programmersguide/shell_int/shell_int_programming/taskbar.asp
				WINDOWPLACEMENT p;
				p.length = sizeof(p);
				GetWindowPlacement(pcli->hwndContactList, &p);
				ShowWindow(pcli->hwndContactList, SW_HIDE);
				SetWindowLongPtr(pcli->hwndContactList, GWL_EXSTYLE,
					GetWindowLongPtr(pcli->hwndContactList, GWL_EXSTYLE) | WS_EX_TOOLWINDOW | WS_EX_WINDOWEDGE);
				SetWindowPlacement(pcli->hwndContactList, &p);
			}
			else SetWindowLongPtr(pcli->hwndContactList, GWL_EXSTYLE, GetWindowLongPtr(pcli->hwndContactList, GWL_EXSTYLE) & ~WS_EX_TOOLWINDOW);

			if (IsDlgButtonChecked(hwndDlg, IDC_ONDESKTOP)) {
				HWND hProgMan = FindWindowA("Progman", NULL);
				if (IsWindow(hProgMan))
					SetParent(pcli->hwndContactList, hProgMan);
			}
			else SetParent(pcli->hwndContactList, NULL);

			if (IsDlgButtonChecked(hwndDlg, IDC_SHOWCAPTION))
				SetWindowLongPtr(pcli->hwndContactList, GWL_STYLE,
				    GetWindowLongPtr(pcli->hwndContactList, GWL_STYLE) | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX);
			else
				SetWindowLongPtr(pcli->hwndContactList, GWL_STYLE,
				    GetWindowLongPtr(pcli->hwndContactList, GWL_STYLE) & ~(WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX));
			if (BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_SHOWMAINMENU))
				SetMenu(pcli->hwndContactList, NULL);
			else
				SetMenu(pcli->hwndContactList, pcli->hMenuMain);
			SetWindowPos(pcli->hwndContactList, 0, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
			RedrawWindow(pcli->hwndContactList, NULL, NULL, RDW_FRAME | RDW_INVALIDATE);
			if (IsIconic(pcli->hwndContactList) && BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_TOOLWND))
				ShowWindow(pcli->hwndContactList, IsDlgButtonChecked(hwndDlg, IDC_MIN2TRAY) ? SW_HIDE : SW_SHOW);
			if (IsDlgButtonChecked(hwndDlg, IDC_TRANSPARENT)) {
				SetWindowLongPtr(pcli->hwndContactList, GWL_EXSTYLE, GetWindowLongPtr(pcli->hwndContactList, GWL_EXSTYLE) | WS_EX_LAYERED);
				SetLayeredWindowAttributes(pcli->hwndContactList, RGB(0, 0, 0), (BYTE)db_get_b(NULL, "CList", "AutoAlpha", SETTING_AUTOALPHA_DEFAULT), LWA_ALPHA);
			}
			else SetWindowLongPtr(pcli->hwndContactList, GWL_EXSTYLE, GetWindowLongPtr(pcli->hwndContactList, GWL_EXSTYLE) & ~WS_EX_LAYERED);

			SendMessage(pcli->hwndContactTree, WM_SIZE, 0, 0);        //forces it to send a cln_listsizechanged
			return TRUE;
		}
		break;
	}
	return FALSE;
}

static INT_PTR CALLBACK DlgProcSBarOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		CheckDlgButton(hwndDlg, IDC_SHOWSBAR, db_get_b(NULL, "CLUI", "ShowSBar", 1) ? BST_CHECKED : BST_UNCHECKED);
		{
			BYTE showOpts = db_get_b(NULL, "CLUI", "SBarShow", 1);
			CheckDlgButton(hwndDlg, IDC_SHOWICON, showOpts & 1 ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_SHOWPROTO, showOpts & 2 ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_SHOWSTATUS, showOpts & 4 ? BST_CHECKED : BST_UNCHECKED);
		}
		CheckDlgButton(hwndDlg, IDC_RIGHTSTATUS, db_get_b(NULL, "CLUI", "SBarRightClk", 0) ? BST_UNCHECKED : BST_CHECKED);
		CheckDlgButton(hwndDlg, IDC_RIGHTMIRANDA, IsDlgButtonChecked(hwndDlg, IDC_RIGHTSTATUS) == BST_UNCHECKED ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_EQUALSECTIONS, db_get_b(NULL, "CLUI", "EqualSections", 0) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_SBPANELBEVEL, db_get_b(NULL, "CLUI", "SBarBevel", 1) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_SHOWGRIP, db_get_b(NULL, "CLUI", "ShowGrip", 1) ? BST_CHECKED : BST_UNCHECKED);
		if (BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_SHOWSBAR)) {
			EnableWindow(GetDlgItem(hwndDlg, IDC_SHOWICON), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_SHOWPROTO), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_SHOWSTATUS), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_RIGHTSTATUS), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_RIGHTMIRANDA), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_EQUALSECTIONS), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_SBPANELBEVEL), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_SHOWGRIP), FALSE);
		}
		return TRUE;
	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_SHOWSBAR) {
			EnableWindow(GetDlgItem(hwndDlg, IDC_SHOWICON), IsDlgButtonChecked(hwndDlg, IDC_SHOWSBAR));
			EnableWindow(GetDlgItem(hwndDlg, IDC_SHOWPROTO), IsDlgButtonChecked(hwndDlg, IDC_SHOWSBAR));
			EnableWindow(GetDlgItem(hwndDlg, IDC_SHOWSTATUS), IsDlgButtonChecked(hwndDlg, IDC_SHOWSBAR));
			EnableWindow(GetDlgItem(hwndDlg, IDC_RIGHTSTATUS), IsDlgButtonChecked(hwndDlg, IDC_SHOWSBAR));
			EnableWindow(GetDlgItem(hwndDlg, IDC_RIGHTMIRANDA), IsDlgButtonChecked(hwndDlg, IDC_SHOWSBAR));
			EnableWindow(GetDlgItem(hwndDlg, IDC_EQUALSECTIONS), IsDlgButtonChecked(hwndDlg, IDC_SHOWSBAR));
			EnableWindow(GetDlgItem(hwndDlg, IDC_SBPANELBEVEL), IsDlgButtonChecked(hwndDlg, IDC_SHOWSBAR));
			EnableWindow(GetDlgItem(hwndDlg, IDC_SHOWGRIP), IsDlgButtonChecked(hwndDlg, IDC_SHOWSBAR));
		}
		SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		break;
	case WM_NOTIFY:
		if (((LPNMHDR) lParam)->code == PSN_APPLY ) {
			db_set_b(NULL, "CLUI", "ShowSBar", (BYTE) IsDlgButtonChecked(hwndDlg, IDC_SHOWSBAR));
			db_set_b(NULL, "CLUI", "SBarShow",
				(BYTE) ((IsDlgButtonChecked(hwndDlg, IDC_SHOWICON) ? 1 : 0) |
				(IsDlgButtonChecked(hwndDlg, IDC_SHOWPROTO) ? 2 : 0) |
				(IsDlgButtonChecked(hwndDlg, IDC_SHOWSTATUS) ? 4 : 0)));
			db_set_b(NULL, "CLUI", "SBarRightClk", (BYTE) IsDlgButtonChecked(hwndDlg, IDC_RIGHTMIRANDA));
			db_set_b(NULL, "CLUI", "EqualSections", (BYTE) IsDlgButtonChecked(hwndDlg, IDC_EQUALSECTIONS));
			db_set_b(NULL, "CLUI", "SBarBevel", (BYTE) IsDlgButtonChecked(hwndDlg, IDC_SBPANELBEVEL));
			pcli->pfnLoadCluiGlobalOpts();
			if (db_get_b(NULL, "CLUI", "ShowGrip", 1) != (BYTE) IsDlgButtonChecked(hwndDlg, IDC_SHOWGRIP)) {
				HWND parent = GetParent(pcli->hwndStatus);
				int flags = WS_CHILD | CCS_BOTTOM;
				db_set_b(NULL, "CLUI", "ShowGrip", (BYTE) IsDlgButtonChecked(hwndDlg, IDC_SHOWGRIP));
				ShowWindow(pcli->hwndStatus, SW_HIDE);
				DestroyWindow(pcli->hwndStatus);
				flags |= db_get_b(NULL, "CLUI", "ShowSBar", 1) ? WS_VISIBLE : 0;
				flags |= db_get_b(NULL, "CLUI", "ShowGrip", 1) ? SBARS_SIZEGRIP : 0;
				pcli->hwndStatus = CreateWindow(STATUSCLASSNAME, NULL, flags, 0, 0, 0, 0, parent, NULL, g_hInst, NULL);
			}
			if (IsDlgButtonChecked(hwndDlg, IDC_SHOWSBAR))
				ShowWindow(pcli->hwndStatus, SW_SHOW);
			else
				ShowWindow(pcli->hwndStatus, SW_HIDE);
			SendMessage(pcli->hwndContactList, WM_SIZE, 0, 0);
			return TRUE;
		}
		break;
	}
	return FALSE;
}

/****************************************************************************************/

static UINT expertOnlyControls[] =
{
	IDC_BRINGTOFRONT, IDC_AUTOSIZE, IDC_STATIC21, IDC_MAXSIZEHEIGHT, IDC_MAXSIZESPIN,
	IDC_STATIC22, IDC_AUTOSIZEUPWARD, IDC_SHOWMAINMENU, IDC_SHOWCAPTION, IDC_CLIENTDRAG
};

int CluiOptInit(WPARAM wParam, LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = { sizeof(odp) };
	odp.hInstance = g_hInst;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_CLUI);
	odp.pszTitle = LPGEN("Window");
	odp.pszGroup = LPGEN("Contact list");
	odp.pfnDlgProc = DlgProcCluiOpts;
	odp.flags = ODPF_BOLDGROUPS;
	Options_AddPage(wParam, &odp);

	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_SBAR);
	odp.pszTitle = LPGEN("Status bar");
	odp.pfnDlgProc = DlgProcSBarOpts;
	odp.flags = ODPF_BOLDGROUPS;
	Options_AddPage(wParam, &odp);
	return 0;
}
