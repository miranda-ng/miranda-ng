/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-14 Miranda NG project (http://miranda-ng.org),
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

extern HANDLE hExtraImageApplying;
extern SIZE g_oldSize;
extern POINT g_oldPos;
extern COLORREF g_CLUISkinnedBkColorRGB;

static int opt_clui_changed = 0;

void AddToTaskBar(HWND hWnd);

INT_PTR CALLBACK DlgProcCluiOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		{
			opt_clui_changed = 0;
			TranslateDialogDefault(hwndDlg);
			CheckDlgButton(hwndDlg, IDC_BRINGTOFRONT, cfg::getByte("CList", "BringToFront", SETTING_BRINGTOFRONT_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_ALWAYSHIDEONTASKBAR, cfg::getByte("CList", "AlwaysHideOnTB", 1) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_ONTOP, cfg::getByte("CList", "OnTop", SETTING_ONTOP_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_SHOWMAINMENU, cfg::getByte("CLUI", "ShowMainMenu", SETTING_SHOWMAINMENU_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_CLIENTDRAG, cfg::getByte("CLUI", "ClientAreaDrag", SETTING_CLIENTDRAG_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_FADEINOUT, cfg::dat.fadeinout ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_AUTOSIZE, cfg::dat.autosize ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_DROPSHADOW, cfg::getByte("CList", "WindowShadow", 0) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_ONDESKTOP, cfg::getByte("CList", "OnDesktop", 0) ? BST_CHECKED : BST_UNCHECKED);

			SendDlgItemMessage(hwndDlg, IDC_BORDERSTYLE, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Title bar"));
			SendDlgItemMessage(hwndDlg, IDC_BORDERSTYLE, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Tool Window"));
			SendDlgItemMessage(hwndDlg, IDC_BORDERSTYLE, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Thin border"));
			SendDlgItemMessage(hwndDlg, IDC_BORDERSTYLE, CB_INSERTSTRING, -1, (LPARAM)TranslateT("No border"));
			SendDlgItemMessage(hwndDlg, IDC_BORDERSTYLE, CB_SETCURSEL, cfg::getByte("CLUI", "WindowStyle", SETTING_WINDOWSTYLE_TOOLWINDOW), 0);

			SendDlgItemMessage(hwndDlg, IDC_MAXSIZESPIN, UDM_SETRANGE, 0, MAKELONG(100, 0));
			SendDlgItemMessage(hwndDlg, IDC_MAXSIZESPIN, UDM_SETPOS, 0, cfg::getByte("CLUI", "MaxSizeHeight", 75));

			SendDlgItemMessage(hwndDlg, IDC_CLIPBORDERSPIN, UDM_SETRANGE, 0, MAKELONG(10, 0));
			SendDlgItemMessage(hwndDlg, IDC_CLIPBORDERSPIN, UDM_SETPOS, 0, cfg::dat.bClipBorder);

			SendDlgItemMessage(hwndDlg, IDC_CLEFTSPIN, UDM_SETRANGE, 0, MAKELONG(255, 0));
			SendDlgItemMessage(hwndDlg, IDC_CRIGHTSPIN, UDM_SETRANGE, 0, MAKELONG(255, 0));
			SendDlgItemMessage(hwndDlg, IDC_CTOPSPIN, UDM_SETRANGE, 0, MAKELONG(255, 0));
			SendDlgItemMessage(hwndDlg, IDC_CBOTTOMSPIN, UDM_SETRANGE, 0, MAKELONG(255, 0));

			SendDlgItemMessage(hwndDlg, IDC_CLEFTSPIN, UDM_SETPOS, 0, cfg::dat.bCLeft - (cfg::dat.dwFlags & CLUI_FRAME_CLISTSUNKEN ? 3 : 0));
			SendDlgItemMessage(hwndDlg, IDC_CRIGHTSPIN, UDM_SETPOS, 0, cfg::dat.bCRight - (cfg::dat.dwFlags & CLUI_FRAME_CLISTSUNKEN ? 3 : 0));
			SendDlgItemMessage(hwndDlg, IDC_CTOPSPIN, UDM_SETPOS, 0, cfg::dat.bCTop);
			SendDlgItemMessage(hwndDlg, IDC_CBOTTOMSPIN, UDM_SETPOS, 0, cfg::dat.bCBottom);

			CheckDlgButton(hwndDlg, IDC_AUTOSIZEUPWARD, cfg::getByte("CLUI", "AutoSizeUpward", 0) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_AUTOHIDE, cfg::getByte("CList", "AutoHide", SETTING_AUTOHIDE_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
			SendDlgItemMessage(hwndDlg, IDC_HIDETIMESPIN, UDM_SETRANGE, 0, MAKELONG(900, 1));
			SendDlgItemMessage(hwndDlg, IDC_HIDETIMESPIN, UDM_SETPOS, 0, MAKELONG(cfg::getWord("CList", "HideTime", SETTING_HIDETIME_DEFAULT), 0));
			Utils::enableDlgControl(hwndDlg, IDC_HIDETIME, IsDlgButtonChecked(hwndDlg, IDC_AUTOHIDE));
			Utils::enableDlgControl(hwndDlg, IDC_HIDETIMESPIN, IsDlgButtonChecked(hwndDlg, IDC_AUTOHIDE));
			Utils::enableDlgControl(hwndDlg, IDC_STATIC01, IsDlgButtonChecked(hwndDlg, IDC_AUTOHIDE));
			if (BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_AUTOSIZE)) {
				Utils::enableDlgControl(hwndDlg, IDC_STATIC21, FALSE);
				Utils::enableDlgControl(hwndDlg, IDC_STATIC22, FALSE);
				Utils::enableDlgControl(hwndDlg, IDC_MAXSIZEHEIGHT, FALSE);
				Utils::enableDlgControl(hwndDlg, IDC_MAXSIZESPIN, FALSE);
				Utils::enableDlgControl(hwndDlg, IDC_AUTOSIZEUPWARD, FALSE);
			} {
				DBVARIANT dbv;
				if (!cfg::getTString(NULL, "CList", "TitleText", &dbv)) {
					SetDlgItemText(hwndDlg, IDC_TITLETEXT, dbv.ptszVal);
					db_free(&dbv);
				}
				else
					SetDlgItemTextA(hwndDlg, IDC_TITLETEXT, MIRANDANAME);
			}

			CheckDlgButton(hwndDlg, IDC_TRANSPARENT, cfg::dat.isTransparent ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_FULLTRANSPARENT, cfg::dat.bFullTransparent ? BST_CHECKED : BST_UNCHECKED);

			if (BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_TRANSPARENT)) {
				Utils::enableDlgControl(hwndDlg, IDC_STATIC11, FALSE);
				Utils::enableDlgControl(hwndDlg, IDC_STATIC12, FALSE);
				Utils::enableDlgControl(hwndDlg, IDC_TRANSACTIVE, FALSE);
				Utils::enableDlgControl(hwndDlg, IDC_TRANSINACTIVE, FALSE);
				Utils::enableDlgControl(hwndDlg, IDC_ACTIVEPERC, FALSE);
				Utils::enableDlgControl(hwndDlg, IDC_INACTIVEPERC, FALSE);
			}
			SendDlgItemMessage(hwndDlg, IDC_TRANSACTIVE, TBM_SETRANGE, FALSE, MAKELONG(1, 255));
			SendDlgItemMessage(hwndDlg, IDC_TRANSINACTIVE, TBM_SETRANGE, FALSE, MAKELONG(1, 255));
			SendDlgItemMessage(hwndDlg, IDC_TRANSACTIVE, TBM_SETPOS, TRUE, cfg::dat.alpha);
			SendDlgItemMessage(hwndDlg, IDC_TRANSINACTIVE, TBM_SETPOS, TRUE, cfg::dat.autoalpha);
			SendMessage(hwndDlg, WM_HSCROLL, 0x12345678, 0);

			CheckDlgButton(hwndDlg, IDC_ROUNDEDBORDER, cfg::dat.dwFlags & CLUI_FRAME_ROUNDEDFRAME ? BST_CHECKED : BST_UNCHECKED);
			SendDlgItemMessage(hwndDlg, IDC_FRAMEGAPSPIN, UDM_SETRANGE, 0, MAKELONG(10, 0));
			SendDlgItemMessage(hwndDlg, IDC_FRAMEGAPSPIN, UDM_SETPOS, 0, (LPARAM)cfg::dat.gapBetweenFrames);
		}
		return TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_AUTOHIDE) {
			Utils::enableDlgControl(hwndDlg, IDC_HIDETIME, IsDlgButtonChecked(hwndDlg, IDC_AUTOHIDE));
			Utils::enableDlgControl(hwndDlg, IDC_HIDETIMESPIN, IsDlgButtonChecked(hwndDlg, IDC_AUTOHIDE));
			Utils::enableDlgControl(hwndDlg, IDC_STATIC01, IsDlgButtonChecked(hwndDlg, IDC_AUTOHIDE));
		}
		else if (LOWORD(wParam) == IDC_TRANSPARENT) {
			Utils::enableDlgControl(hwndDlg, IDC_STATIC11, IsDlgButtonChecked(hwndDlg, IDC_TRANSPARENT));
			Utils::enableDlgControl(hwndDlg, IDC_STATIC12, IsDlgButtonChecked(hwndDlg, IDC_TRANSPARENT));
			Utils::enableDlgControl(hwndDlg, IDC_TRANSACTIVE, IsDlgButtonChecked(hwndDlg, IDC_TRANSPARENT));
			Utils::enableDlgControl(hwndDlg, IDC_TRANSINACTIVE, IsDlgButtonChecked(hwndDlg, IDC_TRANSPARENT));
			Utils::enableDlgControl(hwndDlg, IDC_ACTIVEPERC, IsDlgButtonChecked(hwndDlg, IDC_TRANSPARENT));
			Utils::enableDlgControl(hwndDlg, IDC_INACTIVEPERC, IsDlgButtonChecked(hwndDlg, IDC_TRANSPARENT));
		}
		else if (LOWORD(wParam) == IDC_AUTOSIZE) {
			Utils::enableDlgControl(hwndDlg, IDC_STATIC21, IsDlgButtonChecked(hwndDlg, IDC_AUTOSIZE));
			Utils::enableDlgControl(hwndDlg, IDC_STATIC22, IsDlgButtonChecked(hwndDlg, IDC_AUTOSIZE));
			Utils::enableDlgControl(hwndDlg, IDC_MAXSIZEHEIGHT, IsDlgButtonChecked(hwndDlg, IDC_AUTOSIZE));
			Utils::enableDlgControl(hwndDlg, IDC_MAXSIZESPIN, IsDlgButtonChecked(hwndDlg, IDC_AUTOSIZE));
			Utils::enableDlgControl(hwndDlg, IDC_AUTOSIZEUPWARD, IsDlgButtonChecked(hwndDlg, IDC_AUTOSIZE));
		}
		if ((LOWORD(wParam) == IDC_FRAMEGAP || LOWORD(wParam) == IDC_HIDETIME || LOWORD(wParam) == IDC_CLIPBORDER || LOWORD(wParam) == IDC_TITLETEXT ||
			LOWORD(wParam) == IDC_MAXSIZEHEIGHT || LOWORD(wParam) == IDC_CLEFT || LOWORD(wParam) == IDC_CRIGHT || LOWORD(wParam) == IDC_CTOP
			|| LOWORD(wParam) == IDC_CBOTTOM) && (HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus()))
			return 0;
		if (LOWORD(wParam) == IDC_BORDERSTYLE && (HIWORD(wParam) != CBN_SELCHANGE || (HWND)lParam != GetFocus()))
			return 0;
		SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		opt_clui_changed = 1;
		break;

	case WM_HSCROLL:
		{
			char str[10];
			mir_snprintf(str, SIZEOF(str), "%d%%", 100 * SendDlgItemMessage(hwndDlg, IDC_TRANSINACTIVE, TBM_GETPOS, 0, 0) / 255);
			SetDlgItemTextA(hwndDlg, IDC_INACTIVEPERC, str);
			mir_snprintf(str, SIZEOF(str), "%d%%", 100 * SendDlgItemMessage(hwndDlg, IDC_TRANSACTIVE, TBM_GETPOS, 0, 0) / 255);
			SetDlgItemTextA(hwndDlg, IDC_ACTIVEPERC, str);
		}
		if (wParam != 0x12345678) {
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			opt_clui_changed = 1;
		}
		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->code == PSN_APPLY) {
			BOOL translated;
			BYTE oldFading;
			BYTE windowStyle = (BYTE)SendDlgItemMessage(hwndDlg, IDC_BORDERSTYLE, CB_GETCURSEL, 0, 0);

			if (!opt_clui_changed)
				return TRUE;

			cfg::writeByte("CLUI", "FadeInOut", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_FADEINOUT));
			cfg::dat.fadeinout = IsDlgButtonChecked(hwndDlg, IDC_FADEINOUT) ? 1 : 0;
			oldFading = cfg::dat.fadeinout;
			cfg::dat.fadeinout = FALSE;

			cfg::writeByte("CLUI", "WindowStyle", windowStyle);
			cfg::dat.gapBetweenFrames = GetDlgItemInt(hwndDlg, IDC_FRAMEGAP, &translated, FALSE);

			cfg::writeDword("CLUIFrames", "GapBetweenFrames", cfg::dat.gapBetweenFrames);
			cfg::writeByte("CList", "OnTop", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_ONTOP));
			SetWindowPos(pcli->hwndContactList, IsDlgButtonChecked(hwndDlg, IDC_ONTOP) ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

			cfg::dat.bCLeft = (BYTE)SendDlgItemMessage(hwndDlg, IDC_CLEFTSPIN, UDM_GETPOS, 0, 0);
			cfg::dat.bCRight = (BYTE)SendDlgItemMessage(hwndDlg, IDC_CRIGHTSPIN, UDM_GETPOS, 0, 0);
			cfg::dat.bCTop = (BYTE)SendDlgItemMessage(hwndDlg, IDC_CTOPSPIN, UDM_GETPOS, 0, 0);
			cfg::dat.bCBottom = (BYTE)SendDlgItemMessage(hwndDlg, IDC_CBOTTOMSPIN, UDM_GETPOS, 0, 0);

			cfg::writeDword("CLUI", "clmargins", MAKELONG(MAKEWORD(cfg::dat.bCLeft, cfg::dat.bCRight), MAKEWORD(cfg::dat.bCTop, cfg::dat.bCBottom)));
			SendMessage(pcli->hwndContactList, WM_SIZE, 0, 0);

			cfg::writeByte("CList", "BringToFront", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_BRINGTOFRONT));
			cfg::writeByte("CList", "AlwaysHideOnTB", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_ALWAYSHIDEONTASKBAR));

			if (windowStyle != SETTING_WINDOWSTYLE_DEFAULT) {
				LONG style;
				// Window must be hidden to dynamically remove the taskbar button.
				// See http://msdn.microsoft.com/library/en-us/shellcc/platform/shell/programmersguide/shell_int/shell_int_programming/taskbar.asp
				WINDOWPLACEMENT p;
				p.length = sizeof(p);
				GetWindowPlacement(pcli->hwndContactList, &p);
				ShowWindow(pcli->hwndContactList, SW_HIDE);

				style = GetWindowLongPtr(pcli->hwndContactList, GWL_EXSTYLE);
				style |= WS_EX_TOOLWINDOW | WS_EX_WINDOWEDGE;
				style &= ~WS_EX_APPWINDOW;
				SetWindowLongPtr(pcli->hwndContactList, GWL_EXSTYLE, style);

				SetWindowPlacement(pcli->hwndContactList, &p);
				ShowWindow(pcli->hwndContactList, SW_SHOW);
			}
			else {
				LONG style;
				style = GetWindowLongPtr(pcli->hwndContactList, GWL_EXSTYLE);
				style &= ~(WS_EX_TOOLWINDOW | WS_EX_WINDOWEDGE);
				if (cfg::getByte("CList", "AlwaysHideOnTB", 1))
					style &= ~WS_EX_APPWINDOW;
				else {
					style |= WS_EX_APPWINDOW;
					AddToTaskBar(pcli->hwndContactList);
				}
				SetWindowLongPtr(pcli->hwndContactList, GWL_EXSTYLE, style);
			}

			cfg::dat.bClipBorder = (BYTE)GetDlgItemInt(hwndDlg, IDC_CLIPBORDER, &translated, FALSE);
			cfg::writeDword("CLUI", "Frameflags", cfg::dat.dwFlags);
			cfg::writeByte("CLUI", "clipborder", cfg::dat.bClipBorder);

			cfg::writeByte("CLUI", "ShowMainMenu", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_SHOWMAINMENU));
			cfg::writeByte("CLUI", "ClientAreaDrag", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_CLIENTDRAG));

			ApplyCLUIBorderStyle();

			if (BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_SHOWMAINMENU))
				SetMenu(pcli->hwndContactList, NULL);
			else
				SetMenu(pcli->hwndContactList, pcli->hMenuMain);

			{
				TCHAR title[256];
				GetDlgItemText(hwndDlg, IDC_TITLETEXT, title, SIZEOF(title));
				cfg::writeTString(NULL, "CList", "TitleText", title);
				SetWindowText(pcli->hwndContactList, title);
			}
			cfg::dat.dwFlags = IsDlgButtonChecked(hwndDlg, IDC_ROUNDEDBORDER) ? cfg::dat.dwFlags | CLUI_FRAME_ROUNDEDFRAME : cfg::dat.dwFlags & ~CLUI_FRAME_ROUNDEDFRAME;
			cfg::writeByte("CLUI", "AutoSize", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_AUTOSIZE));

			if ((cfg::dat.autosize = IsDlgButtonChecked(hwndDlg, IDC_AUTOSIZE) ? 1 : 0)) {
				SendMessage(pcli->hwndContactList, WM_SIZE, 0, 0);
				SendMessage(pcli->hwndContactTree, WM_SIZE, 0, 0);
			}

			cfg::writeByte("CLUI", "MaxSizeHeight", (BYTE)GetDlgItemInt(hwndDlg, IDC_MAXSIZEHEIGHT, NULL, FALSE));
			cfg::writeByte("CLUI", "AutoSizeUpward", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_AUTOSIZEUPWARD));
			cfg::writeByte("CList", "AutoHide", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_AUTOHIDE));
			cfg::writeWord("CList", "HideTime", (WORD)SendDlgItemMessage(hwndDlg, IDC_HIDETIMESPIN, UDM_GETPOS, 0, 0));

			cfg::writeByte("CList", "Transparent", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_TRANSPARENT));
			cfg::dat.isTransparent = IsDlgButtonChecked(hwndDlg, IDC_TRANSPARENT) ? 1 : 0;
			cfg::writeByte("CList", "Alpha", (BYTE)SendDlgItemMessage(hwndDlg, IDC_TRANSACTIVE, TBM_GETPOS, 0, 0));
			cfg::dat.alpha = (BYTE)SendDlgItemMessage(hwndDlg, IDC_TRANSACTIVE, TBM_GETPOS, 0, 0);
			cfg::writeByte("CList", "AutoAlpha", (BYTE)SendDlgItemMessage(hwndDlg, IDC_TRANSINACTIVE, TBM_GETPOS, 0, 0));
			cfg::dat.autoalpha = (BYTE)SendDlgItemMessage(hwndDlg, IDC_TRANSINACTIVE, TBM_GETPOS, 0, 0);
			cfg::writeByte("CList", "WindowShadow", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_DROPSHADOW));
			cfg::writeByte("CList", "OnDesktop", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_ONDESKTOP));
			cfg::writeDword("CLUI", "Frameflags", cfg::dat.dwFlags);
			cfg::dat.bFullTransparent = IsDlgButtonChecked(hwndDlg, IDC_FULLTRANSPARENT) ? 1 : 0;
			cfg::writeByte("CLUI", "fulltransparent", (BYTE)cfg::dat.bFullTransparent);

			if (cfg::dat.bLayeredHack)
				SetWindowLongPtr(pcli->hwndContactList, GWL_EXSTYLE, GetWindowLongPtr(pcli->hwndContactList, GWL_EXSTYLE) | WS_EX_LAYERED);

			if (g_CLUISkinnedBkColorRGB)
				cfg::dat.colorkey = g_CLUISkinnedBkColorRGB;
			else if (cfg::dat.bClipBorder == 0 && !(cfg::dat.dwFlags & CLUI_FRAME_ROUNDEDFRAME))
				cfg::dat.colorkey = cfg::getDword("CLC", "BkColour", CLCDEFAULT_BKCOLOUR);
			else {
				SendMessage(pcli->hwndContactList, WM_SIZE, 0, 0);
				cfg::dat.colorkey = RGB(255, 0, 255);
			}
			if (cfg::dat.isTransparent || cfg::dat.bFullTransparent) {
				SetWindowLongPtr(pcli->hwndContactList, GWL_EXSTYLE, GetWindowLongPtr(pcli->hwndContactList, GWL_EXSTYLE) & ~WS_EX_LAYERED);
				SetWindowLongPtr(pcli->hwndContactList, GWL_EXSTYLE, GetWindowLongPtr(pcli->hwndContactList, GWL_EXSTYLE) | WS_EX_LAYERED);
				SetLayeredWindowAttributes(pcli->hwndContactList, 0, 255, LWA_ALPHA | LWA_COLORKEY);
				SetLayeredWindowAttributes(pcli->hwndContactList,
					(COLORREF)(cfg::dat.bFullTransparent ? cfg::dat.colorkey : 0),
					(BYTE)(cfg::dat.isTransparent ? cfg::dat.autoalpha : 255),
					(DWORD)((cfg::dat.isTransparent ? LWA_ALPHA : 0L) | (cfg::dat.bFullTransparent ? LWA_COLORKEY : 0L)));
			}
			else {
				SetLayeredWindowAttributes(pcli->hwndContactList, RGB(0, 0, 0), (BYTE)255, LWA_ALPHA);
				if (!cfg::dat.bLayeredHack)
					SetWindowLongPtr(pcli->hwndContactList, GWL_EXSTYLE, GetWindowLongPtr(pcli->hwndContactList, GWL_EXSTYLE) & ~WS_EX_LAYERED);
			}

			ConfigureCLUIGeometry(1);
			ShowWindow(pcli->hwndContactList, SW_SHOW);
			SendMessage(pcli->hwndContactList, WM_SIZE, 0, 0);
			SetWindowPos(pcli->hwndContactList, 0, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
			RedrawWindow(pcli->hwndContactList, NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW);
			cfg::dat.fadeinout = oldFading;
			opt_clui_changed = 0;

			pcli->pfnClcOptionsChanged();
			pcli->pfnClcBroadcast(CLM_AUTOREBUILD, 0, 0);
			return TRUE;
		}
		break;
	}
	return FALSE;
}

static int opt_sbar_changed = 0;

INT_PTR CALLBACK DlgProcSBarOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		opt_sbar_changed = 0;
		TranslateDialogDefault(hwndDlg);
		CheckDlgButton(hwndDlg, IDC_SHOWSBAR, cfg::getByte("CLUI", "ShowSBar", 1) ? BST_CHECKED : BST_UNCHECKED); {
			BYTE showOpts = cfg::getByte("CLUI", "SBarShow", 1);
			CheckDlgButton(hwndDlg, IDC_SHOWICON, showOpts & 1 ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_SHOWPROTO, showOpts & 2 ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_SHOWSTATUS, showOpts & 4 ? BST_CHECKED : BST_UNCHECKED);
		}
		CheckDlgButton(hwndDlg, IDC_RIGHTSTATUS, cfg::getByte("CLUI", "SBarRightClk", 0) ? BST_UNCHECKED : BST_CHECKED);
		CheckDlgButton(hwndDlg, IDC_RIGHTMIRANDA, BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_RIGHTSTATUS) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_EQUALSECTIONS, cfg::dat.bEqualSections ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_SBPANELBEVEL, cfg::getByte("CLUI", "SBarBevel", 1) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_SHOWGRIP, cfg::getByte("CLUI", "ShowGrip", 1) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_SKINBACKGROUND, cfg::dat.bSkinnedStatusBar ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_SHOWXSTATUS, cfg::dat.bShowXStatusOnSbar ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_MARKLOCKED, cfg::getByte("CLUI", "sbar_showlocked", 1) ? BST_CHECKED : BST_UNCHECKED);

		if (BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_SHOWSBAR)) {
			Utils::enableDlgControl(hwndDlg, IDC_SHOWICON, FALSE);
			Utils::enableDlgControl(hwndDlg, IDC_SHOWPROTO, FALSE);
			Utils::enableDlgControl(hwndDlg, IDC_SHOWSTATUS, FALSE);
			Utils::enableDlgControl(hwndDlg, IDC_RIGHTSTATUS, FALSE);
			Utils::enableDlgControl(hwndDlg, IDC_RIGHTMIRANDA, FALSE);
			Utils::enableDlgControl(hwndDlg, IDC_EQUALSECTIONS, FALSE);
			Utils::enableDlgControl(hwndDlg, IDC_SBPANELBEVEL, FALSE);
			Utils::enableDlgControl(hwndDlg, IDC_SHOWGRIP, FALSE);
			Utils::enableDlgControl(hwndDlg, IDC_SKINBACKGROUND, FALSE);
		}
		return TRUE;
	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_SHOWSBAR) {
			Utils::enableDlgControl(hwndDlg, IDC_SHOWICON, IsDlgButtonChecked(hwndDlg, IDC_SHOWSBAR));
			Utils::enableDlgControl(hwndDlg, IDC_SHOWPROTO, IsDlgButtonChecked(hwndDlg, IDC_SHOWSBAR));
			Utils::enableDlgControl(hwndDlg, IDC_SHOWSTATUS, IsDlgButtonChecked(hwndDlg, IDC_SHOWSBAR));
			Utils::enableDlgControl(hwndDlg, IDC_RIGHTSTATUS, IsDlgButtonChecked(hwndDlg, IDC_SHOWSBAR));
			Utils::enableDlgControl(hwndDlg, IDC_RIGHTMIRANDA, IsDlgButtonChecked(hwndDlg, IDC_SHOWSBAR));
			Utils::enableDlgControl(hwndDlg, IDC_EQUALSECTIONS, IsDlgButtonChecked(hwndDlg, IDC_SHOWSBAR));
			Utils::enableDlgControl(hwndDlg, IDC_SBPANELBEVEL, IsDlgButtonChecked(hwndDlg, IDC_SHOWSBAR));
			Utils::enableDlgControl(hwndDlg, IDC_SHOWGRIP, IsDlgButtonChecked(hwndDlg, IDC_SHOWSBAR));
			Utils::enableDlgControl(hwndDlg, IDC_SKINBACKGROUND, IsDlgButtonChecked(hwndDlg, IDC_SHOWSBAR));
		}
		SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		opt_sbar_changed = 1;
		break;
	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code) {
		case PSN_APPLY:
			if (!opt_sbar_changed)
				return TRUE;

			cfg::writeByte("CLUI", "ShowSBar", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_SHOWSBAR));
			cfg::writeByte("CLUI", "SBarShow", (BYTE)((IsDlgButtonChecked(hwndDlg, IDC_SHOWICON) ? 1 : 0) | (IsDlgButtonChecked(hwndDlg, IDC_SHOWPROTO) ? 2 : 0) | (IsDlgButtonChecked(hwndDlg, IDC_SHOWSTATUS) ? 4 : 0)));
			cfg::writeByte("CLUI", "SBarRightClk", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_RIGHTMIRANDA));
			cfg::writeByte("CLUI", "EqualSections", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_EQUALSECTIONS));
			cfg::writeByte("CLUI", "sb_skinned", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_SKINBACKGROUND));
			cfg::writeByte("CLUI", "sbar_showlocked", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_MARKLOCKED));

			cfg::dat.bEqualSections = IsDlgButtonChecked(hwndDlg, IDC_EQUALSECTIONS) ? 1 : 0;
			cfg::dat.bSkinnedStatusBar = IsDlgButtonChecked(hwndDlg, IDC_SKINBACKGROUND) ? 1 : 0;
			cfg::dat.bShowXStatusOnSbar = IsDlgButtonChecked(hwndDlg, IDC_SHOWXSTATUS) ? 1 : 0;
			cfg::writeByte("CLUI", "xstatus_sbar", (BYTE)cfg::dat.bShowXStatusOnSbar);
			cfg::writeByte("CLUI", "SBarBevel", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_SBPANELBEVEL));
			if (cfg::getByte("CLUI", "ShowGrip", 1) != (BYTE)IsDlgButtonChecked(hwndDlg, IDC_SHOWGRIP)) {
				HWND parent = GetParent(pcli->hwndStatus);
				int flags = WS_CHILD | CCS_BOTTOM;
				cfg::writeByte("CLUI", "ShowGrip", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_SHOWGRIP));
				ShowWindow(pcli->hwndStatus, SW_HIDE);
				mir_unsubclassWindow(pcli->hwndStatus, NewStatusBarWndProc);
				DestroyWindow(pcli->hwndStatus);

				flags |= cfg::getByte("CLUI", "ShowSBar", 1) ? WS_VISIBLE : 0;
				flags |= cfg::getByte("CLUI", "ShowGrip", 1) ? SBARS_SIZEGRIP : 0;

				pcli->hwndStatus = CreateWindow(STATUSCLASSNAME, NULL, flags, 0, 0, 0, 0, parent, NULL, g_hInst, NULL);
				mir_subclassWindow(pcli->hwndStatus, NewStatusBarWndProc);
			}
			if (IsDlgButtonChecked(hwndDlg, IDC_SHOWSBAR)) {
				ShowWindow(pcli->hwndStatus, SW_SHOW);
				SendMessage(pcli->hwndStatus, WM_SIZE, 0, 0);
				cfg::dat.dwFlags |= CLUI_FRAME_SBARSHOW;
			}
			else {
				ShowWindow(pcli->hwndStatus, SW_HIDE);
				cfg::dat.dwFlags &= ~CLUI_FRAME_SBARSHOW;
			}
			cfg::writeDword("CLUI", "Frameflags", cfg::dat.dwFlags);
			ConfigureCLUIGeometry(1);
			SendMessage(pcli->hwndContactList, WM_SIZE, 0, 0);
			CluiProtocolStatusChanged(0, 0);
			PostMessage(pcli->hwndContactList, CLUIINTM_REDRAW, 0, 0);
			opt_sbar_changed = 0;
			return TRUE;
		}
		break;
	}
	return FALSE;
}

DWORD GetCLUIWindowStyle(BYTE style)
{
	DWORD dwBasic = WS_CLIPCHILDREN;

	if (style == SETTING_WINDOWSTYLE_THINBORDER)
		return dwBasic | WS_BORDER;
	else if (style == SETTING_WINDOWSTYLE_TOOLWINDOW || style == 0)
		return dwBasic | (WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_POPUPWINDOW | WS_THICKFRAME);
	else if (style == SETTING_WINDOWSTYLE_NOBORDER)
		return dwBasic;

	return dwBasic;
}

void ApplyCLUIBorderStyle()
{
	BYTE windowStyle = cfg::getByte("CLUI", "WindowStyle", SETTING_WINDOWSTYLE_TOOLWINDOW);
	WINDOWPLACEMENT p;
	bool minToTray = TRUE;

	p.length = sizeof(p);
	GetWindowPlacement(pcli->hwndContactList, &p);
	ShowWindow(pcli->hwndContactList, SW_HIDE);

	if (windowStyle == SETTING_WINDOWSTYLE_DEFAULT || windowStyle == SETTING_WINDOWSTYLE_TOOLWINDOW) {
		SetWindowLongPtr(pcli->hwndContactList, GWL_STYLE, GetWindowLongPtr(pcli->hwndContactList, GWL_STYLE) | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_POPUPWINDOW | WS_THICKFRAME);
		if (SETTING_WINDOWSTYLE_DEFAULT == windowStyle) {
			SetWindowLongPtr(pcli->hwndContactList, GWL_STYLE, GetWindowLongPtr(pcli->hwndContactList, GWL_STYLE) & ~(WS_MAXIMIZEBOX/* | WS_MINIMIZEBOX*/));
			minToTray = FALSE;
		}
	}
	else if (windowStyle == SETTING_WINDOWSTYLE_THINBORDER) {
		SetWindowLongPtr(pcli->hwndContactList, GWL_STYLE, GetWindowLongPtr(pcli->hwndContactList, GWL_STYLE) & ~(WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_POPUPWINDOW | WS_THICKFRAME));
		SetWindowLongPtr(pcli->hwndContactList, GWL_STYLE, GetWindowLongPtr(pcli->hwndContactList, GWL_STYLE) | WS_BORDER | WS_CLIPCHILDREN);
	}
	else {
		SetWindowLongPtr(pcli->hwndContactList, GWL_STYLE, GetWindowLongPtr(pcli->hwndContactList, GWL_STYLE) & ~(WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_POPUPWINDOW | WS_THICKFRAME));
		SetWindowLongPtr(pcli->hwndContactList, GWL_STYLE, GetWindowLongPtr(pcli->hwndContactList, GWL_STYLE) | WS_CLIPCHILDREN);
	}
	p.showCmd = SW_HIDE;
	SetWindowPlacement(pcli->hwndContactList, &p);

	cfg::writeByte(0, "CList", "Min2Tray", minToTray);
}
