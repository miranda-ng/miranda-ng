/*
 * astyle --force-indent=tab=4 --brackets=linux --indent-switches
 *		  --pad=oper --one-line=keep-blocks  --unpad=paren
 *
 * Miranda IM: the free IM client for Microsoft* Windows*
 *
 * Copyright 2000-2010 Miranda ICQ/IM project,
 * all portions of this codebase are copyrighted to the people
 * listed in contributors.txt.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * you should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * part of clist_ng plugin for Miranda.
 *
 * (C) 2005-2010 by silvercircle _at_ gmail _dot_ com and contributors
 *
 * $Id: cluiopts.cpp 138 2010-11-01 10:51:15Z silvercircle $
 *
 */

#include <commonheaders.h>

extern WNDPROC OldStatusBarProc;
extern HANDLE hExtraImageApplying;
extern SIZE g_oldSize;
extern POINT g_oldPos;
extern COLORREF g_CLUISkinnedBkColorRGB;

static int opt_clui_changed = 0;

INT_PTR CALLBACK cfg::DlgProcCluiOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		{
            opt_clui_changed = 0;
			TranslateDialogDefault(hwndDlg);
			CheckDlgButton(hwndDlg, IDC_BRINGTOFRONT, cfg::getByte("CList", "BringToFront", SETTING_BRINGTOFRONT_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_ALWAYSHIDEONTASKBAR, cfg::getByte("CList", "AlwaysHideOnTB", 1) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_ONTOP, cfg::getByte("CList", "OnTop", SETTING_ONTOP_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_CLIENTDRAG, cfg::getByte("CLUI", "ClientAreaDrag", SETTING_CLIENTDRAG_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_FADEINOUT, cfg::dat.fadeinout ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_AUTOSIZE, cfg::dat.autosize);
			CheckDlgButton(hwndDlg, IDC_ONDESKTOP, cfg::getByte("CList", "OnDesktop", 0) ? BST_CHECKED : BST_UNCHECKED);

			SendDlgItemMessage(hwndDlg, IDC_MAXSIZESPIN, UDM_SETRANGE, 0, MAKELONG(100, 0));
			SendDlgItemMessage(hwndDlg, IDC_MAXSIZESPIN, UDM_SETPOS, 0, cfg::getByte("CLUI", "MaxSizeHeight", 75));

			SendDlgItemMessage(hwndDlg, IDC_CLUIFRAMESBDR, CPM_SETCOLOUR, 0, cfg::getDword("CLUI", "clr_frameborder", RGB(40, 40, 40)));

			CheckDlgButton(hwndDlg, IDC_AUTOSIZEUPWARD, cfg::getByte("CLUI", "AutoSizeUpward", 0) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_AUTOHIDE, cfg::getByte("CList", "AutoHide", SETTING_AUTOHIDE_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
			SendDlgItemMessage(hwndDlg, IDC_HIDETIMESPIN, UDM_SETRANGE, 0, MAKELONG(900, 1));
			SendDlgItemMessage(hwndDlg, IDC_HIDETIMESPIN, UDM_SETPOS, 0, MAKELONG(cfg::getWord("CList", "HideTime", SETTING_HIDETIME_DEFAULT), 0));
			Utils::enableDlgControl(hwndDlg, IDC_HIDETIME, IsDlgButtonChecked(hwndDlg, IDC_AUTOHIDE));
			Utils::enableDlgControl(hwndDlg, IDC_HIDETIMESPIN, IsDlgButtonChecked(hwndDlg, IDC_AUTOHIDE));
			Utils::enableDlgControl(hwndDlg, IDC_STATIC01, IsDlgButtonChecked(hwndDlg, IDC_AUTOHIDE));
			if (!IsDlgButtonChecked(hwndDlg, IDC_AUTOSIZE)) {
				Utils::enableDlgControl(hwndDlg, IDC_STATIC21, FALSE);
				Utils::enableDlgControl(hwndDlg, IDC_STATIC22, FALSE);
				Utils::enableDlgControl(hwndDlg, IDC_MAXSIZEHEIGHT, FALSE);
				Utils::enableDlgControl(hwndDlg, IDC_MAXSIZESPIN, FALSE);
				Utils::enableDlgControl(hwndDlg, IDC_AUTOSIZEUPWARD, FALSE);
			}
			CheckDlgButton(hwndDlg, IDC_TRANSPARENT, cfg::dat.isTransparent ? BST_CHECKED : BST_UNCHECKED);
			if (!IsDlgButtonChecked(hwndDlg, IDC_TRANSPARENT)) {
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

			CheckDlgButton(hwndDlg, IDC_USEAERO, Skin::settings.fUseAero ? BST_CHECKED : BST_UNCHECKED);
			SendDlgItemMessage(hwndDlg, IDC_FRAMEGAPSPIN, UDM_SETRANGE, 0, MAKELONG(10, 0));
			SendDlgItemMessage(hwndDlg, IDC_FRAMEGAPSPIN, UDM_SETPOS, 0, (LPARAM)cfg::dat.gapBetweenFrames);

			return TRUE;
		}
	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_AUTOHIDE) {
			Utils::enableDlgControl(hwndDlg, IDC_HIDETIME, IsDlgButtonChecked(hwndDlg, IDC_AUTOHIDE));
			Utils::enableDlgControl(hwndDlg, IDC_HIDETIMESPIN, IsDlgButtonChecked(hwndDlg, IDC_AUTOHIDE));
			Utils::enableDlgControl(hwndDlg, IDC_STATIC01, IsDlgButtonChecked(hwndDlg, IDC_AUTOHIDE));
		} else if (LOWORD(wParam) == IDC_TRANSPARENT) {
			Utils::enableDlgControl(hwndDlg, IDC_STATIC11, IsDlgButtonChecked(hwndDlg, IDC_TRANSPARENT));
			Utils::enableDlgControl(hwndDlg, IDC_STATIC12, IsDlgButtonChecked(hwndDlg, IDC_TRANSPARENT));
			Utils::enableDlgControl(hwndDlg, IDC_TRANSACTIVE, IsDlgButtonChecked(hwndDlg, IDC_TRANSPARENT));
			Utils::enableDlgControl(hwndDlg, IDC_TRANSINACTIVE, IsDlgButtonChecked(hwndDlg, IDC_TRANSPARENT));
			Utils::enableDlgControl(hwndDlg, IDC_ACTIVEPERC, IsDlgButtonChecked(hwndDlg, IDC_TRANSPARENT));
			Utils::enableDlgControl(hwndDlg, IDC_INACTIVEPERC, IsDlgButtonChecked(hwndDlg, IDC_TRANSPARENT));
		} else if (LOWORD(wParam) == IDC_AUTOSIZE) {
			Utils::enableDlgControl(hwndDlg, IDC_STATIC21, IsDlgButtonChecked(hwndDlg, IDC_AUTOSIZE));
			Utils::enableDlgControl(hwndDlg, IDC_STATIC22, IsDlgButtonChecked(hwndDlg, IDC_AUTOSIZE));
			Utils::enableDlgControl(hwndDlg, IDC_MAXSIZEHEIGHT, IsDlgButtonChecked(hwndDlg, IDC_AUTOSIZE));
			Utils::enableDlgControl(hwndDlg, IDC_MAXSIZESPIN, IsDlgButtonChecked(hwndDlg, IDC_AUTOSIZE));
			Utils::enableDlgControl(hwndDlg, IDC_AUTOSIZEUPWARD, IsDlgButtonChecked(hwndDlg, IDC_AUTOSIZE));
		}
		if ((LOWORD(wParam) == IDC_FRAMEGAP || LOWORD(wParam) == IDC_HIDETIME || LOWORD(wParam) == IDC_ROWGAP ||
			LOWORD(wParam) == IDC_MAXSIZEHEIGHT) && (HIWORD(wParam) != EN_CHANGE || (HWND) lParam != GetFocus()))
			return 0;
		SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
        opt_clui_changed = 1;
		break;

	case WM_HSCROLL:
		{
			char str[10];
			wsprintfA(str, "%d%%", 100 * SendDlgItemMessage(hwndDlg, IDC_TRANSINACTIVE, TBM_GETPOS, 0, 0) / 255);
			SetDlgItemTextA(hwndDlg, IDC_INACTIVEPERC, str);
			wsprintfA(str, "%d%%", 100 * SendDlgItemMessage(hwndDlg, IDC_TRANSACTIVE, TBM_GETPOS, 0, 0) / 255);
			SetDlgItemTextA(hwndDlg, IDC_ACTIVEPERC, str);
		}
		if (wParam != 0x12345678) {
            SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
            opt_clui_changed = 1;
        }
		break;
	case WM_NOTIFY:
		switch (((LPNMHDR) lParam)->code) {
		case PSN_APPLY:
			{
				BOOL translated;
				BYTE oldFading;
				COLORREF clr_cluiframes;

                if(!opt_clui_changed)
                    return TRUE;

                cfg::writeByte("CLUI", "FadeInOut", (BYTE) IsDlgButtonChecked(hwndDlg, IDC_FADEINOUT));
				cfg::dat.fadeinout = IsDlgButtonChecked(hwndDlg, IDC_FADEINOUT) ? 1 : 0;
				oldFading = cfg::dat.fadeinout;
				cfg::dat.fadeinout = FALSE;

				cfg::dat.gapBetweenFrames = GetDlgItemInt(hwndDlg, IDC_FRAMEGAP, &translated, FALSE);

				cfg::writeDword("CLUIFrames", "GapBetweenFrames", cfg::dat.gapBetweenFrames);
				cfg::writeByte("CList", "OnTop", (BYTE) IsDlgButtonChecked(hwndDlg, IDC_ONTOP));
				SetWindowPos(pcli->hwndContactList, IsDlgButtonChecked(hwndDlg, IDC_ONTOP) ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

				SendMessage(pcli->hwndContactList, WM_SIZE, 0, 0);

				cfg::writeByte("CList", "BringToFront", (BYTE) IsDlgButtonChecked(hwndDlg, IDC_BRINGTOFRONT));
				cfg::writeByte("CList", "AlwaysHideOnTB", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_ALWAYSHIDEONTASKBAR));

				cfg::writeDword("CLUI", "Frameflags", cfg::dat.dwFlags);

				cfg::writeByte("CLUI", "ClientAreaDrag", (BYTE) IsDlgButtonChecked(hwndDlg, IDC_CLIENTDRAG));

				clr_cluiframes = (COLORREF)SendDlgItemMessage(hwndDlg, IDC_CLUIFRAMESBDR, CPM_GETCOLOUR, 0, 0);

				if(CLUI::hPenFrames)
					DeleteObject(CLUI::hPenFrames);
				CLUI::hPenFrames = CreatePen(PS_SOLID, 1, clr_cluiframes);
				cfg::writeDword("CLUI", "clr_frameborder", clr_cluiframes);

				CLUI::applyBorderStyle();

				cfg::writeByte("CLUI", "AutoSize", (BYTE) IsDlgButtonChecked(hwndDlg, IDC_AUTOSIZE));

				if((cfg::dat.autosize = IsDlgButtonChecked(hwndDlg, IDC_AUTOSIZE) ? 1 : 0)) {
					SendMessage(pcli->hwndContactList, WM_SIZE, 0, 0);
					SendMessage(pcli->hwndContactTree, WM_SIZE, 0, 0);
				}

				cfg::writeByte("CLUI", "MaxSizeHeight", (BYTE) GetDlgItemInt(hwndDlg, IDC_MAXSIZEHEIGHT, NULL, FALSE));
				cfg::writeByte("CLUI", "AutoSizeUpward", (BYTE) IsDlgButtonChecked(hwndDlg, IDC_AUTOSIZEUPWARD));
				cfg::writeByte("CList", "AutoHide", (BYTE) IsDlgButtonChecked(hwndDlg, IDC_AUTOHIDE));
				cfg::writeWord("CList", "HideTime", (WORD) SendDlgItemMessage(hwndDlg, IDC_HIDETIMESPIN, UDM_GETPOS, 0, 0));

				cfg::writeByte("CList", "Transparent", (BYTE) IsDlgButtonChecked(hwndDlg, IDC_TRANSPARENT));
				cfg::dat.isTransparent = IsDlgButtonChecked(hwndDlg, IDC_TRANSPARENT) ? 1 : 0;
				cfg::writeByte("CList", "Alpha", (BYTE) SendDlgItemMessage(hwndDlg, IDC_TRANSACTIVE, TBM_GETPOS, 0, 0));
				cfg::dat.alpha = (BYTE) SendDlgItemMessage(hwndDlg, IDC_TRANSACTIVE, TBM_GETPOS, 0, 0);
				cfg::writeByte("CList", "AutoAlpha", (BYTE) SendDlgItemMessage(hwndDlg, IDC_TRANSINACTIVE, TBM_GETPOS, 0, 0));
				cfg::dat.autoalpha = (BYTE) SendDlgItemMessage(hwndDlg, IDC_TRANSINACTIVE, TBM_GETPOS, 0, 0);
				cfg::writeByte("CList", "OnDesktop", (BYTE) IsDlgButtonChecked(hwndDlg, IDC_ONDESKTOP));
				cfg::writeDword("CLUI", "Frameflags", cfg::dat.dwFlags);

				cfg::writeByte(SKIN_DB_MODULE, "sfUseAero", IsDlgButtonChecked(hwndDlg, IDC_USEAERO));
				if(g_CLUISkinnedBkColorRGB)
					cfg::dat.colorkey = g_CLUISkinnedBkColorRGB;
				else {
					SendMessage(pcli->hwndContactList, WM_SIZE, 0, 0);
					cfg::dat.colorkey = RGB(255, 0, 255);
				}

				Api::updateState();
				if(!cfg::isAero) {
					if (cfg::dat.isTransparent || Skin::metrics.fHaveColorkey) {
						SetLayeredWindowAttributes(pcli->hwndContactList, 0, 255, LWA_ALPHA | LWA_COLORKEY);
						SetLayeredWindowAttributes(pcli->hwndContactList,
							(COLORREF)(Skin::metrics.fHaveColorkey ? cfg::dat.colorkey : 0),
							(BYTE)(cfg::dat.isTransparent ? cfg::dat.autoalpha : 255),
							(DWORD)((cfg::dat.isTransparent ? LWA_ALPHA : 0L) | (Skin::metrics.fHaveColorkey ? LWA_COLORKEY : 0L)));
					} else
						SetLayeredWindowAttributes(pcli->hwndContactList, RGB(0, 0, 0), (BYTE)255, LWA_ALPHA);
				}
                CLUI::configureGeometry(1);
                ShowWindow(pcli->hwndContactList, SW_SHOW);
                SendMessage(pcli->hwndContactList, WM_SIZE, 0, 0);
                SetWindowPos(pcli->hwndContactList, 0, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
                RedrawWindow(pcli->hwndContactList, NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW);
				cfg::dat.fadeinout = oldFading;
				SFL_SetState(cfg::dat.bUseFloater & CLUI_FLOATER_AUTOHIDE ? (cfg::getByte("CList", "State", SETTING_STATE_NORMAL) == SETTING_STATE_NORMAL ? 0 : 1) : 1);
                opt_clui_changed = 0;
				return TRUE;
			}
		}
		break;
	}
	return FALSE;
}

static int opt_sbar_changed = 0;

INT_PTR CALLBACK cfg::DlgProcSBarOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
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
		CheckDlgButton(hwndDlg, IDC_RIGHTMIRANDA, !IsDlgButtonChecked(hwndDlg, IDC_RIGHTSTATUS) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_EQUALSECTIONS, cfg::dat.bEqualSections ? BST_CHECKED : BST_UNCHECKED);
        CheckDlgButton(hwndDlg, IDC_MARKLOCKED, cfg::getByte("CLUI", "sbar_showlocked", 1));

		if (!IsDlgButtonChecked(hwndDlg, IDC_SHOWSBAR)) {
			Utils::enableDlgControl(hwndDlg, IDC_SHOWICON, FALSE);
			Utils::enableDlgControl(hwndDlg, IDC_SHOWPROTO, FALSE);
			Utils::enableDlgControl(hwndDlg, IDC_SHOWSTATUS, FALSE);
			Utils::enableDlgControl(hwndDlg, IDC_RIGHTSTATUS, FALSE);
			Utils::enableDlgControl(hwndDlg, IDC_RIGHTMIRANDA, FALSE);
			Utils::enableDlgControl(hwndDlg, IDC_EQUALSECTIONS, FALSE);
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
		}
		SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
        opt_sbar_changed = 1;
		break;
	case WM_NOTIFY:
		switch (((LPNMHDR) lParam)->code) {
        case PSN_APPLY:
            if(!opt_sbar_changed)
                return TRUE;

            cfg::writeByte("CLUI", "ShowSBar", (BYTE) IsDlgButtonChecked(hwndDlg, IDC_SHOWSBAR));
            cfg::writeByte("CLUI", "SBarShow", (BYTE) ((IsDlgButtonChecked(hwndDlg, IDC_SHOWICON) ? 1 : 0) | (IsDlgButtonChecked(hwndDlg, IDC_SHOWPROTO) ? 2 : 0) | (IsDlgButtonChecked(hwndDlg, IDC_SHOWSTATUS) ? 4 : 0)));
            cfg::writeByte("CLUI", "SBarRightClk", (BYTE) IsDlgButtonChecked(hwndDlg, IDC_RIGHTMIRANDA));
            cfg::writeByte("CLUI", "EqualSections", (BYTE) IsDlgButtonChecked(hwndDlg, IDC_EQUALSECTIONS));
            cfg::writeByte("CLUI", "sbar_showlocked", (BYTE) IsDlgButtonChecked(hwndDlg, IDC_MARKLOCKED));

			cfg::dat.bEqualSections = IsDlgButtonChecked(hwndDlg, IDC_EQUALSECTIONS) ? 1 : 0;
			if (IsDlgButtonChecked(hwndDlg, IDC_SHOWSBAR)) {
				ShowWindow(pcli->hwndStatus, SW_SHOW);
                SendMessage(pcli->hwndStatus, WM_SIZE, 0, 0);
				cfg::dat.dwFlags |= CLUI_FRAME_SBARSHOW;
			} else {
				ShowWindow(pcli->hwndStatus, SW_HIDE);
				cfg::dat.dwFlags &= ~CLUI_FRAME_SBARSHOW;
			}
			cfg::writeDword("CLUI", "Frameflags", cfg::dat.dwFlags);
			CLUI::configureGeometry(1);
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

DWORD CLUI::getWindowStyle(BYTE style)
{
	DWORD dwBasic = WS_CLIPCHILDREN;

	if(style == SETTING_WINDOWSTYLE_THINBORDER)
		return dwBasic | WS_BORDER;
	else if(style == SETTING_WINDOWSTYLE_TOOLWINDOW || style == 0)
		return dwBasic | (WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_POPUPWINDOW | WS_THICKFRAME);
	else if(style == SETTING_WINDOWSTYLE_NOBORDER)
		return dwBasic;

	return dwBasic;
}

void CLUI::applyBorderStyle()
{
    BYTE 			windowStyle = Skin::metrics.bWindowStyle;
	WINDOWPLACEMENT p = {0};
	DWORD 			style;
	bool			minToTray = TRUE;

	p.length = sizeof(p);
	GetWindowPlacement(pcli->hwndContactList, &p);
	ShowWindow(pcli->hwndContactList, SW_HIDE);

	Skin::metrics.fHaveFrame = true;

	if (windowStyle == SETTING_WINDOWSTYLE_DEFAULT || windowStyle == SETTING_WINDOWSTYLE_TOOLWINDOW) {
		SetWindowLong(pcli->hwndContactList, GWL_STYLE, GetWindowLong(pcli->hwndContactList, GWL_STYLE) | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_POPUPWINDOW | WS_THICKFRAME);
		if(SETTING_WINDOWSTYLE_DEFAULT == windowStyle) {
			SetWindowLong(pcli->hwndContactList, GWL_STYLE, GetWindowLong(pcli->hwndContactList, GWL_STYLE) & ~(WS_MAXIMIZEBOX/* | WS_MINIMIZEBOX*/));
			minToTray = FALSE;
		}
	} else if(windowStyle == SETTING_WINDOWSTYLE_THINBORDER) {
		SetWindowLong(pcli->hwndContactList, GWL_STYLE, GetWindowLong(pcli->hwndContactList, GWL_STYLE) & ~(WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_POPUPWINDOW | WS_THICKFRAME));
		SetWindowLong(pcli->hwndContactList, GWL_STYLE, GetWindowLong(pcli->hwndContactList, GWL_STYLE) | WS_BORDER | WS_CLIPCHILDREN);
	}
	else {
        SetWindowLong(pcli->hwndContactList, GWL_STYLE, GetWindowLong(pcli->hwndContactList, GWL_STYLE) & ~(WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_POPUPWINDOW | WS_THICKFRAME));
        SetWindowLong(pcli->hwndContactList, GWL_STYLE, GetWindowLong(pcli->hwndContactList, GWL_STYLE) | WS_CLIPCHILDREN);
    	Skin::metrics.fHaveFrame = false;
    }

	style = GetWindowLong(pcli->hwndContactList, GWL_EXSTYLE);
	if (windowStyle != SETTING_WINDOWSTYLE_DEFAULT)
	{
		style |= WS_EX_TOOLWINDOW | WS_EX_WINDOWEDGE;
		style &= ~WS_EX_APPWINDOW;
	}
	else
	{
		style &= ~(WS_EX_TOOLWINDOW | WS_EX_WINDOWEDGE);
		if (cfg::getByte("CList", "AlwaysHideOnTB", 1))
			style &= ~WS_EX_APPWINDOW;
		else
			style |= WS_EX_APPWINDOW;
	}

	SetWindowLong(pcli->hwndContactList, GWL_EXSTYLE, style);
	p.showCmd = SW_HIDE;
	SetWindowPlacement(pcli->hwndContactList, &p);

	cfg::writeByte(0, "CList", "Min2Tray", minToTray);
}
