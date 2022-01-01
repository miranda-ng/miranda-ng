/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org),
Copyright (c) 2000-12 Miranda IM project,
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

#include "stdafx.h"

static INT_PTR CALLBACK DlgProcCluiOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		CheckDlgButton(hwndDlg, IDC_BRINGTOFRONT, g_plugin.getByte("BringToFront", SETTING_BRINGTOFRONT_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_ONTOP, g_plugin.getByte("OnTop", SETTING_ONTOP_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_TOOLWND, g_plugin.getByte("ToolWindow", SETTING_TOOLWINDOW_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_MIN2TRAY, g_plugin.getByte("Min2Tray", SETTING_MIN2TRAY_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
		if (IsDlgButtonChecked(hwndDlg, IDC_TOOLWND))
			EnableWindow(GetDlgItem(hwndDlg, IDC_MIN2TRAY), FALSE);
		
		CheckDlgButton(hwndDlg, IDC_SHOWCAPTION, db_get_b(0, "CLUI", "ShowCaption", SETTING_SHOWCAPTION_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_SHOWMAINMENU, db_get_b(0, "CLUI", "ShowMainMenu", SETTING_SHOWMAINMENU_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CLIENTDRAG, db_get_b(0, "CLUI", "ClientAreaDrag", SETTING_CLIENTDRAG_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
		if (BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_SHOWCAPTION)) {
			EnableWindow(GetDlgItem(hwndDlg, IDC_MIN2TRAY), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_TOOLWND), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_TITLETEXT), FALSE);
		}

		CheckDlgButton(hwndDlg, IDC_FADEINOUT, db_get_b(0, "CLUI", "FadeInOut", 0) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_AUTOSIZE, db_get_b(0, "CLUI", "AutoSize", 0) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_DROPSHADOW, g_plugin.getByte("WindowShadow", 0) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_ONDESKTOP, g_plugin.getByte("OnDesktop", 0) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_DISABLEDOCKING, db_get_b(0, "CLUI", "DockToSides", 1) ? BST_CHECKED : BST_UNCHECKED);
		SendDlgItemMessage(hwndDlg, IDC_MAXSIZESPIN, UDM_SETRANGE, 0, MAKELONG(100, 0));
		SendDlgItemMessage(hwndDlg, IDC_MAXSIZESPIN, UDM_SETPOS, 0, db_get_b(0, "CLUI", "MaxSizeHeight", 75));
		CheckDlgButton(hwndDlg, IDC_AUTOSIZEUPWARD, db_get_b(0, "CLUI", "AutoSizeUpward", 0) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_AUTOHIDE,
			g_plugin.getByte("AutoHide", SETTING_AUTOHIDE_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
		SendDlgItemMessage(hwndDlg, IDC_HIDETIMESPIN, UDM_SETRANGE, 0, MAKELONG(900, 1));
		SendDlgItemMessage(hwndDlg, IDC_HIDETIMESPIN, UDM_SETPOS, 0,
			MAKELONG(g_plugin.getWord("HideTime", SETTING_HIDETIME_DEFAULT), 0));
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
		{
			ptrW wszTitle(g_plugin.getWStringA("TitleText"));
			if (wszTitle != nullptr)
				SetDlgItemText(hwndDlg, IDC_TITLETEXT, wszTitle);
			else
				SetDlgItemTextA(hwndDlg, IDC_TITLETEXT, MIRANDANAME);
		}
		CheckDlgButton(hwndDlg, IDC_TRANSPARENT, g_plugin.getByte("Transparent", SETTING_TRANSPARENT_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);

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
		SendDlgItemMessage(hwndDlg, IDC_TRANSACTIVE, TBM_SETPOS, TRUE, g_plugin.getByte("Alpha", SETTING_ALPHA_DEFAULT));
		SendDlgItemMessage(hwndDlg, IDC_TRANSINACTIVE, TBM_SETPOS, TRUE, g_plugin.getByte("AutoAlpha", SETTING_AUTOALPHA_DEFAULT));
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
			(HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus()))
			return 0;

		// Enable apply button
		SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		break;

	case WM_HSCROLL:
		{
			char str[10];
			mir_snprintf(str, "%d%%", 100 * SendDlgItemMessage(hwndDlg, IDC_TRANSINACTIVE, TBM_GETPOS, 0, 0) / 255);
			SetDlgItemTextA(hwndDlg, IDC_INACTIVEPERC, str);
			mir_snprintf(str, "%d%%", 100 * SendDlgItemMessage(hwndDlg, IDC_TRANSACTIVE, TBM_GETPOS, 0, 0) / 255);
			SetDlgItemTextA(hwndDlg, IDC_ACTIVEPERC, str);
		}
		if (wParam != 0x12345678)
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		break;
	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->code == PSN_APPLY) {
			g_plugin.setByte("OnTop", (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_ONTOP));
			g_plugin.setByte("ToolWindow", (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_TOOLWND));
			g_plugin.setByte("BringToFront", (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_BRINGTOFRONT));
			db_set_b(0, "CLUI", "FadeInOut", (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_FADEINOUT));
			db_set_b(0, "CLUI", "AutoSize", (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_AUTOSIZE));
			db_set_b(0, "CLUI", "MaxSizeHeight", (uint8_t)GetDlgItemInt(hwndDlg, IDC_MAXSIZEHEIGHT, nullptr, FALSE));
			db_set_b(0, "CLUI", "AutoSizeUpward", (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_AUTOSIZEUPWARD));
			g_plugin.setByte("AutoHide", (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_AUTOHIDE));
			g_plugin.setWord("HideTime", (uint16_t)SendDlgItemMessage(hwndDlg, IDC_HIDETIMESPIN, UDM_GETPOS, 0, 0));
			g_plugin.setByte("Transparent", (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_TRANSPARENT));
			g_plugin.setByte("Alpha", (uint8_t)SendDlgItemMessage(hwndDlg, IDC_TRANSACTIVE, TBM_GETPOS, 0, 0));
			g_plugin.setByte("AutoAlpha", (uint8_t)SendDlgItemMessage(hwndDlg, IDC_TRANSINACTIVE, TBM_GETPOS, 0, 0));
			g_plugin.setByte("WindowShadow", (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_DROPSHADOW));
			g_plugin.setByte("OnDesktop", (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_ONDESKTOP));
			db_set_b(0, "CLUI", "DockToSides", (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_DISABLEDOCKING));
			db_set_b(0, "CLUI", "ShowCaption", (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_SHOWCAPTION));
			db_set_b(0, "CLUI", "ShowMainMenu", (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_SHOWMAINMENU));
			db_set_b(0, "CLUI", "ClientAreaDrag", (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_CLIENTDRAG));
			g_plugin.setByte("Min2Tray", (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_MIN2TRAY));
			{
				wchar_t title[256];
				GetDlgItemText(hwndDlg, IDC_TITLETEXT, title, _countof(title));
				g_plugin.setWString("TitleText", title);
				SetWindowText(g_clistApi.hwndContactList, title);
			}

			g_clistApi.pfnLoadCluiGlobalOpts();
			SetWindowPos(g_clistApi.hwndContactList, IsDlgButtonChecked(hwndDlg, IDC_ONTOP) ? HWND_TOPMOST : HWND_NOTOPMOST,
				0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

			if (IsDlgButtonChecked(hwndDlg, IDC_TOOLWND)) {
				// Window must be hidden to dynamically remove the taskbar button.
				// See http://msdn.microsoft.com/library/en-us/shellcc/platform/shell/programmersguide/shell_int/shell_int_programming/taskbar.asp
				WINDOWPLACEMENT p;
				p.length = sizeof(p);
				GetWindowPlacement(g_clistApi.hwndContactList, &p);
				ShowWindow(g_clistApi.hwndContactList, SW_HIDE);
				SetWindowLongPtr(g_clistApi.hwndContactList, GWL_EXSTYLE,
					GetWindowLongPtr(g_clistApi.hwndContactList, GWL_EXSTYLE) & ~WS_EX_APPWINDOW | WS_EX_TOOLWINDOW | WS_EX_WINDOWEDGE);
				SetWindowPlacement(g_clistApi.hwndContactList, &p);
			}
			else
				SetWindowLongPtr(g_clistApi.hwndContactList, GWL_EXSTYLE, GetWindowLongPtr(g_clistApi.hwndContactList, GWL_EXSTYLE) & ~WS_EX_TOOLWINDOW | WS_EX_APPWINDOW);

			if (IsDlgButtonChecked(hwndDlg, IDC_ONDESKTOP)) {
				HWND hProgMan = FindWindow(L"Progman", nullptr);
				if (hProgMan)
					SetParent(g_clistApi.hwndContactList, hProgMan);
			}
			else
				SetParent(g_clistApi.hwndContactList, nullptr);

			if (IsDlgButtonChecked(hwndDlg, IDC_SHOWCAPTION)) {
				int style = GetWindowLongPtr(g_clistApi.hwndContactList, GWL_STYLE) | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
				SetWindowLongPtr(g_clistApi.hwndContactList, GWL_STYLE, style);
			}
			else {
				SetWindowLongPtr(g_clistApi.hwndContactList, GWL_STYLE,
					GetWindowLongPtr(g_clistApi.hwndContactList, GWL_STYLE) & ~(WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX));
			}

			if (BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_SHOWMAINMENU))
				SetMenu(g_clistApi.hwndContactList, nullptr);
			else
				SetMenu(g_clistApi.hwndContactList, g_clistApi.hMenuMain);

			SetWindowPos(g_clistApi.hwndContactList, nullptr, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
			RedrawWindow(g_clistApi.hwndContactList, nullptr, nullptr, RDW_FRAME | RDW_INVALIDATE);

			if (IsIconic(g_clistApi.hwndContactList) && BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_TOOLWND))
				ShowWindow(g_clistApi.hwndContactList, IsDlgButtonChecked(hwndDlg, IDC_MIN2TRAY) ? SW_HIDE : SW_SHOW);
			if (IsDlgButtonChecked(hwndDlg, IDC_TRANSPARENT)) {
				SetWindowLongPtr(g_clistApi.hwndContactList, GWL_EXSTYLE, GetWindowLongPtr(g_clistApi.hwndContactList, GWL_EXSTYLE) | WS_EX_LAYERED);
				SetLayeredWindowAttributes(g_clistApi.hwndContactList, RGB(0, 0, 0), (uint8_t)g_plugin.getByte("AutoAlpha", SETTING_AUTOALPHA_DEFAULT), LWA_ALPHA);
			}
			else
				SetWindowLongPtr(g_clistApi.hwndContactList, GWL_EXSTYLE, GetWindowLongPtr(g_clistApi.hwndContactList, GWL_EXSTYLE) & ~WS_EX_LAYERED);

			SendMessage(g_clistApi.hwndContactTree, WM_SIZE, 0, 0);        //forces it to send a cln_listsizechanged

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
		CheckDlgButton(hwndDlg, IDC_SHOWSBAR, db_get_b(0, "CLUI", "ShowSBar", 1) ? BST_CHECKED : BST_UNCHECKED);
		{
			uint8_t showOpts = db_get_b(0, "CLUI", "SBarShow", 1);
			CheckDlgButton(hwndDlg, IDC_SHOWICON, showOpts & 1 ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_SHOWPROTO, showOpts & 2 ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_SHOWSTATUS, showOpts & 4 ? BST_CHECKED : BST_UNCHECKED);
		}
		CheckDlgButton(hwndDlg, IDC_RIGHTSTATUS, db_get_b(0, "CLUI", "SBarRightClk", 0) ? BST_UNCHECKED : BST_CHECKED);
		CheckDlgButton(hwndDlg, IDC_RIGHTMIRANDA, IsDlgButtonChecked(hwndDlg, IDC_RIGHTSTATUS) == BST_UNCHECKED ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_EQUALSECTIONS, db_get_b(0, "CLUI", "EqualSections", 0) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_SBPANELBEVEL, db_get_b(0, "CLUI", "SBarBevel", 1) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_SHOWGRIP, db_get_b(0, "CLUI", "ShowGrip", 1) ? BST_CHECKED : BST_UNCHECKED);
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
		if (((LPNMHDR)lParam)->code == PSN_APPLY) {
			db_set_b(0, "CLUI", "ShowSBar", (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_SHOWSBAR));
			db_set_b(0, "CLUI", "SBarShow",
				(uint8_t)((IsDlgButtonChecked(hwndDlg, IDC_SHOWICON) ? 1 : 0) |
				(IsDlgButtonChecked(hwndDlg, IDC_SHOWPROTO) ? 2 : 0) |
				(IsDlgButtonChecked(hwndDlg, IDC_SHOWSTATUS) ? 4 : 0)));
			db_set_b(0, "CLUI", "SBarRightClk", (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_RIGHTMIRANDA));
			db_set_b(0, "CLUI", "EqualSections", (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_EQUALSECTIONS));
			db_set_b(0, "CLUI", "SBarBevel", (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_SBPANELBEVEL));
			g_clistApi.pfnLoadCluiGlobalOpts();
			if (db_get_b(0, "CLUI", "ShowGrip", 1) != (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_SHOWGRIP)) {
				HWND parent = GetParent(g_clistApi.hwndStatus);
				int flags = WS_CHILD | CCS_BOTTOM;
				db_set_b(0, "CLUI", "ShowGrip", (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_SHOWGRIP));
				ShowWindow(g_clistApi.hwndStatus, SW_HIDE);
				DestroyWindow(g_clistApi.hwndStatus);
				flags |= db_get_b(0, "CLUI", "ShowSBar", 1) ? WS_VISIBLE : 0;
				flags |= db_get_b(0, "CLUI", "ShowGrip", 1) ? SBARS_SIZEGRIP : 0;
				g_clistApi.hwndStatus = CreateWindow(STATUSCLASSNAME, nullptr, flags, 0, 0, 0, 0, parent, nullptr, g_plugin.getInst(), nullptr);
			}
			if (IsDlgButtonChecked(hwndDlg, IDC_SHOWSBAR))
				ShowWindow(g_clistApi.hwndStatus, SW_SHOW);
			else
				ShowWindow(g_clistApi.hwndStatus, SW_HIDE);
			SendMessage(g_clistApi.hwndContactList, WM_SIZE, 0, 0);
			return TRUE;
		}
		break;
	}
	return FALSE;
}

/****************************************************************************************/

int CluiOptInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.szTitle.a = LPGEN("Contact list");
	odp.flags = ODPF_BOLDGROUPS;

	odp.szTab.a = LPGEN("Window");
	odp.pfnDlgProc = DlgProcCluiOpts;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_CLUI);
	g_plugin.addOptions(wParam, &odp);

	odp.szTab.a = LPGEN("Status bar");
	odp.pfnDlgProc = DlgProcSBarOpts;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_SBAR);
	g_plugin.addOptions(wParam, &odp);
	return 0;
}
