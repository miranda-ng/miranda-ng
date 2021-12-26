/*
* A plugin for Miranda IM which displays web page text in a window 
* Copyright (C) 2005 Vincent Joyce.
*
* Miranda IM: the free icq client for MS Windows  Copyright (C) 2000-2
* Richard Hughes, Roland Rabien & Tristan Van de Vreede
*
* This program is free software; you can redistribute it and/or modify it
* under the terms of the GNU General Public License as published by the Free
* Software Foundation; either version 2 of the License, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
* or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
* for more details.
*
* You should have received a copy of the GNU General Public License along
* with this program; if not, write to the Free Software Foundation, Inc., 59
* Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "stdafx.h"
#include "webview.h"

static char *fontSizes[] = { "8", "10", "14", "16", "18", "20", "24", "28" };

#define M_FILLSCRIPTCOMBO    (WM_USER+16)

/////////////////////////////////////////////////////////////////////////////////////////

static int CALLBACK EnumFontScriptsProc(ENUMLOGFONTEX * lpelfe, NEWTEXTMETRICEX*, int, LPARAM lParam)
{
	if (SendMessage((HWND)lParam, CB_FINDSTRINGEXACT, -1, (LPARAM)lpelfe->elfScript) == CB_ERR) {
		int i = SendMessage((HWND)lParam, CB_ADDSTRING, 0, (LPARAM)lpelfe->elfScript);

		SendMessage((HWND)lParam, CB_SETITEMDATA, i, lpelfe->elfLogFont.lfCharSet);
	}
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// copied and modified from NewStatusNotify

static INT_PTR CALLBACK DlgPopUpOpts(HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	char str[512];
	uint32_t  BGColour = 0;
	uint32_t  TextColour = 0;
	uint32_t  delay = 0;
	static int test = 0;

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hdlg);

		delay = g_plugin.getDword(POP_DELAY_KEY, 0);

		// setting popup delay option
		_itoa(delay, str, 10);
		SetDlgItemTextA(hdlg, IDC_DELAY, str);

		BGColour = g_plugin.getDword(POP_BG_CLR_KEY, Def_color_bg);
		TextColour = g_plugin.getDword(POP_TXT_CLR_KEY, Def_color_txt);

		// Colours. First step is configuring the colours.
		SendDlgItemMessage(hdlg, IDC_POP_BGCOLOUR, CPM_SETCOLOUR, 0, BGColour);
		SendDlgItemMessage(hdlg, IDC_POP_TEXTCOLOUR, CPM_SETCOLOUR, 0, TextColour);
		// Second step is disabling them if we want to use default Windows
		// ones.
		CheckDlgButton(hdlg, IDC_POP_USEWINCOLORS, g_plugin.getByte(POP_USEWINCLRS_KEY, 0) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hdlg, IDC_POP_USESAMECOLORS, g_plugin.getByte(POP_USESAMECLRS_KEY, 1) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hdlg, IDC_POP_USECUSTCOLORS, g_plugin.getByte(POP_USECUSTCLRS_KEY, 0) ? BST_CHECKED : BST_UNCHECKED);

		if (IsDlgButtonChecked(hdlg, IDC_POP_USEWINCOLORS) || IsDlgButtonChecked(hdlg, IDC_POP_USESAMECOLORS)) {
			EnableWindow(GetDlgItem(hdlg, IDC_POP_BGCOLOUR), 0);
			EnableWindow(GetDlgItem(hdlg, IDC_POP_TEXTCOLOUR), 0);
		}

		CheckDlgButton(hdlg, IDC_LCLK_WINDOW, g_plugin.getByte(LCLK_WINDOW_KEY, 0) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hdlg, IDC_LCLK_WEB_PGE, g_plugin.getByte(LCLK_WEB_PGE_KEY, 0) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hdlg, IDC_LCLK_DISMISS, g_plugin.getByte(LCLK_DISMISS_KEY, 1) ? BST_CHECKED : BST_UNCHECKED);

		CheckDlgButton(hdlg, IDC_RCLK_WINDOW, g_plugin.getByte(RCLK_WINDOW_KEY, 0) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hdlg, IDC_RCLK_WEB_PGE, g_plugin.getByte(RCLK_WEB_PGE_KEY, 1) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hdlg, IDC_RCLK_DISMISS, g_plugin.getByte(RCLK_DISMISS_KEY, 0) ? BST_CHECKED : BST_UNCHECKED);
		return TRUE;

	case WM_COMMAND:
		// enable the "apply" button 
		if (HIWORD(wParam) == BN_CLICKED && GetFocus() == (HWND)lParam)
			SendMessage(GetParent(hdlg), PSM_CHANGED, 0, 0);
		// These are simple clicks: we don't save, but we tell the Options Page 
		// to enable the "Apply" button.
		switch (LOWORD(wParam)) {
		case IDC_POP_BGCOLOUR: // Fall through
		case IDC_POP_TEXTCOLOUR:
			// select new colors
			if (HIWORD(wParam) == CPN_COLOURCHANGED)
				SendMessage(GetParent(hdlg), PSM_CHANGED, 0, 0);
			break;

		case IDC_POP_USESAMECOLORS:
			// use same color as data window - enable/disable color selection
			// controls
			EnableWindow(GetDlgItem(hdlg, IDC_POP_BGCOLOUR), !((uint8_t)IsDlgButtonChecked(hdlg, IDC_POP_USESAMECOLORS)));
			EnableWindow(GetDlgItem(hdlg, IDC_POP_TEXTCOLOUR), !((uint8_t)IsDlgButtonChecked(hdlg, IDC_POP_USESAMECOLORS)));
			SendMessage(GetParent(hdlg), PSM_CHANGED, 0, 0);
			break;

		case IDC_POP_USEWINCOLORS:
			// use window color - enable/disable color selection controls
			EnableWindow(GetDlgItem(hdlg, IDC_POP_BGCOLOUR), !((uint8_t)IsDlgButtonChecked(hdlg, IDC_POP_USEWINCOLORS)));
			EnableWindow(GetDlgItem(hdlg, IDC_POP_TEXTCOLOUR), !((uint8_t)IsDlgButtonChecked(hdlg, IDC_POP_USEWINCOLORS)));
			SendMessage(GetParent(hdlg), PSM_CHANGED, 0, 0);
			break;

		case IDC_POP_USECUSTCOLORS:
			EnableWindow(GetDlgItem(hdlg, IDC_POP_BGCOLOUR), ((uint8_t)IsDlgButtonChecked(hdlg, IDC_POP_USECUSTCOLORS)));
			EnableWindow(GetDlgItem(hdlg, IDC_POP_TEXTCOLOUR), ((uint8_t)IsDlgButtonChecked(hdlg, IDC_POP_USECUSTCOLORS)));
			break;

		case IDC_PD1:
			SetDlgItemText(hdlg, IDC_DELAY, L"0");
			break;
		case IDC_PD2:
			// Popup delay = permanent
			SetDlgItemText(hdlg, IDC_DELAY, L"-1");
			break;

		case IDC_DELAY:
			if (HIWORD(wParam) == EN_CHANGE)
				test++;
			if (test > 1) {
				//CheckRadioButton(hdlg, IDC_PD1, IDC_PD3, IDC_PD3);
				SendMessage(GetParent(hdlg), PSM_CHANGED, 0, 0);
			}
			break;

		case IDC_PREVIEW:
			wchar_t str3[512];
			POPUPDATAW ppd;

			GetDlgItemText(hdlg, IDC_DELAY, str3, _countof(str3));

			if (IsDlgButtonChecked(hdlg, IDC_POP_USECUSTCOLORS)) {
				BGColour = (SendDlgItemMessage(hdlg, IDC_POP_BGCOLOUR, CPM_GETCOLOUR, 0, 0));
				TextColour = (SendDlgItemMessage(hdlg, IDC_POP_TEXTCOLOUR, CPM_GETCOLOUR, 0, 0));
			}
			if (IsDlgButtonChecked(hdlg, IDC_POP_USEWINCOLORS)) {
				BGColour = GetSysColor(COLOR_BTNFACE);
				TextColour = GetSysColor(COLOR_WINDOWTEXT);
			}
			if (IsDlgButtonChecked(hdlg, IDC_POP_USESAMECOLORS)) {
				BGColour = BackgoundClr;
				TextColour = TextClr;
			}
			ppd.lchContact = NULL;
			mir_wstrcpy(ppd.lpwzContactName, _A2W(MODULENAME));
			ppd.lchIcon = LoadIcon(g_plugin.getInst(), MAKEINTRESOURCE(IDI_SITE));
			mir_wstrcpy(ppd.lpwzText, TranslateT("This is a preview popup."));
			ppd.colorBack = BGColour;
			ppd.colorText = TextColour;
			ppd.PluginWindowProc = nullptr;
			ppd.iSeconds = _wtol(str3);
			// display popups
			PUAddPopupW(&ppd);
		}
		break;

	case WM_NOTIFY: // Here we have pressed either the OK or the APPLY button.
		switch (((LPNMHDR)lParam)->code) {
		case PSN_APPLY:
			int popupdelayval = 0;
			wchar_t str2[512];
			GetDlgItemText(hdlg, IDC_DELAY, str2, _countof(str2));

			popupdelayval = _wtol(str2);
			g_plugin.setDword(POP_DELAY_KEY, popupdelayval);

			g_plugin.setByte(LCLK_WINDOW_KEY, (uint8_t)IsDlgButtonChecked(hdlg, IDC_LCLK_WINDOW));
			g_plugin.setByte(LCLK_WEB_PGE_KEY, (uint8_t)IsDlgButtonChecked(hdlg, IDC_LCLK_WEB_PGE));
			g_plugin.setByte(LCLK_DISMISS_KEY, (uint8_t)IsDlgButtonChecked(hdlg, IDC_LCLK_DISMISS));

			g_plugin.setByte(RCLK_WINDOW_KEY, (uint8_t)IsDlgButtonChecked(hdlg, IDC_RCLK_WINDOW));
			g_plugin.setByte(RCLK_WEB_PGE_KEY, (uint8_t)IsDlgButtonChecked(hdlg, IDC_RCLK_WEB_PGE));
			g_plugin.setByte(RCLK_DISMISS_KEY, (uint8_t)IsDlgButtonChecked(hdlg, IDC_RCLK_DISMISS));

			g_plugin.setByte(POP_USECUSTCLRS_KEY, (uint8_t)IsDlgButtonChecked(hdlg, IDC_POP_USECUSTCOLORS));
			g_plugin.setByte(POP_USEWINCLRS_KEY, (uint8_t)IsDlgButtonChecked(hdlg, IDC_POP_USEWINCOLORS));
			g_plugin.setByte(POP_USESAMECLRS_KEY, (uint8_t)IsDlgButtonChecked(hdlg, IDC_POP_USESAMECOLORS));

			BGColour = (SendDlgItemMessage(hdlg, IDC_POP_BGCOLOUR, CPM_GETCOLOUR, 0, 0));
			TextColour = (SendDlgItemMessage(hdlg, IDC_POP_TEXTCOLOUR, CPM_GETCOLOUR, 0, 0));

			g_plugin.setDword(POP_BG_CLR_KEY, BGColour);
			g_plugin.setDword(POP_TXT_CLR_KEY, TextColour);

			test = 0;
			return TRUE;
		}
		break;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////

static INT_PTR CALLBACK DlgProcOpt(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	uint32_t timerval, delaytime, oldcolor;
	DBVARIANT dbv;
	static int test = 0;
	static int test2 = 0;

	switch (uMsg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{
			test = 0;

			SendDlgItemMessage(hwndDlg, IDC_SPIN1, UDM_SETRANGE, 0, MAKELONG(999, 0));
			SendDlgItemMessage(hwndDlg, IDC_SPIN2, UDM_SETRANGE, 0, MAKELONG(120, 0));

			SetDlgItemInt(hwndDlg, IDC_TIME, g_plugin.getDword(REFRESH_KEY, TIME), FALSE);
			SetDlgItemInt(hwndDlg, IDC_START_DELAY, g_plugin.getWord(START_DELAY_KEY, 0), FALSE);

			mir_forkthread(FillFontListThread, hwndDlg);

			CheckDlgButton(hwndDlg, IDC_SUPPRESS, g_plugin.getByte(SUPPRESS_ERR_KEY, 0) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_UPDATE_ONSTART, g_plugin.getByte(UPDATE_ONSTART_KEY, 0) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_UPDATE_ON_OPEN, g_plugin.getByte(UPDATE_ON_OPEN_KEY, 0) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_HIDE_STATUS_ICON, g_plugin.getByte(HIDE_STATUS_ICON_KEY, 0) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_FONT_BOLD, g_plugin.getByte(FONT_BOLD_KEY, 0) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_FONT_ITALIC, g_plugin.getByte(FONT_ITALIC_KEY, 0) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_FONT_UNDERLINE, g_plugin.getByte(FONT_UNDERLINE_KEY, 0) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_ERROR_POPUP, g_plugin.getByte(ERROR_POPUP_KEY, 0) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_UPDATE_ONALERT, g_plugin.getByte(UPDATE_ONALERT_KEY, 0) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_SAVE_INDIVID_POS, g_plugin.getByte(SAVE_INDIVID_POS_KEY, 0) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_NO_PROTECT, g_plugin.getByte(NO_PROTECT_KEY, 0) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_DATAPOPUP, g_plugin.getByte(DATA_POPUP_KEY, 0) ? BST_CHECKED : BST_UNCHECKED);

			if (!g_plugin.getWString(FONT_FACE_KEY, &dbv)) {
				SetDlgItemText(hwndDlg, IDC_TYPEFACE, dbv.pwszVal);
				db_free(&dbv);
			}
			else SetDlgItemText(hwndDlg, IDC_TYPEFACE, Def_font_face);

			for (int i = 0; i < _countof(fontSizes); i++)
				SendDlgItemMessage(hwndDlg, IDC_FONTSIZE, CB_ADDSTRING, 0, (LPARAM)fontSizes[i]);

			SendMessage(hwndDlg, M_FILLSCRIPTCOMBO, wParam, 0);

			SetDlgItemInt(hwndDlg, IDC_FONTSIZE, g_plugin.getByte(FONT_SIZE_KEY, 14), FALSE);

			EnableWindow(GetDlgItem(hwndDlg, IDC_FIND_BUTTON), 0);
			EnableWindow(GetDlgItem(hwndDlg, IDC_ALERT_BUTTON), 0);

			SendDlgItemMessage(hwndDlg, IDC_BGCOLOR, CPM_SETCOLOUR, 0, (BackgoundClr));
			SendDlgItemMessage(hwndDlg, IDC_TXTCOLOR, CPM_SETCOLOUR, 0, (TextClr));

			/*
			* record bg value for later comparison
			*/
			oldcolor = BackgoundClr;

			if (g_plugin.getByte(SUPPRESS_ERR_KEY, 0)) {
				CheckDlgButton(hwndDlg, IDC_SUPPRESS, BST_CHECKED);
				EnableWindow(GetDlgItem(hwndDlg, IDC_ERROR_POPUP), 0);
			}
			else {
				CheckDlgButton(hwndDlg, IDC_SUPPRESS, BST_UNCHECKED);
				EnableWindow(GetDlgItem(hwndDlg, IDC_ERROR_POPUP), 1);
			}

			EnableWindow(GetDlgItem(hwndDlg, IDC_ERROR_POPUP), 0);

			if (g_plugin.getByte(UPDATE_ONSTART_KEY, 0)) {
				EnableWindow(GetDlgItem(hwndDlg, IDC_START_DELAY), 1);
				EnableWindow(GetDlgItem(hwndDlg, IDC_SPIN2), 1);
				EnableWindow(GetDlgItem(hwndDlg, IDC_STARTDELAYTXT), 1);
				EnableWindow(GetDlgItem(hwndDlg, IDC_STDELAYSECTXT), 1);
			}
			else {
				EnableWindow(GetDlgItem(hwndDlg, IDC_START_DELAY), 0);
				EnableWindow(GetDlgItem(hwndDlg, IDC_SPIN2), 0);
				EnableWindow(GetDlgItem(hwndDlg, IDC_STARTDELAYTXT), 0);
				EnableWindow(GetDlgItem(hwndDlg, IDC_STDELAYSECTXT), 0);
			}
		}
		break;

	case M_FILLSCRIPTCOMBO: // fill the script combo box and set the
		// selection to the value for fontid wParam
		{
			LOGFONT lf = { 0 };
			int i;
			HDC hdc = GetDC(hwndDlg);

			lf.lfCharSet = DEFAULT_CHARSET;
			GetDlgItemText(hwndDlg, IDC_TYPEFACE, lf.lfFaceName, _countof(lf.lfFaceName));
			lf.lfPitchAndFamily = 0;
			SendDlgItemMessage(hwndDlg, IDC_SCRIPT, CB_RESETCONTENT, 0, 0);
			EnumFontFamiliesEx(hdc, &lf, (FONTENUMPROC)EnumFontScriptsProc, (LPARAM)GetDlgItem(hwndDlg, IDC_SCRIPT), 0);
			ReleaseDC(hwndDlg, hdc);
			for (i = SendDlgItemMessage(hwndDlg, IDC_SCRIPT, CB_GETCOUNT, 0, 0) - 1; i >= 0; i--) {
				if (SendDlgItemMessage(hwndDlg, IDC_SCRIPT, CB_GETITEMDATA, i, 0) == (uint8_t)((g_plugin.getByte(FONT_SCRIPT_KEY, 0)))) {
					SendDlgItemMessage(hwndDlg, IDC_SCRIPT, CB_SETCURSEL, i, 0);
					break;
				}
			}
			if (i < 0)
				SendDlgItemMessage(hwndDlg, IDC_SCRIPT, CB_SETCURSEL, 0, 0);
		}
		break;

	case WM_COMMAND:
		if (HIWORD(wParam) == BN_CLICKED && GetFocus() == (HWND)lParam)
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);

		switch (LOWORD(wParam)) {
		case IDC_TXTCOLOR:
			TextClr = SendDlgItemMessage(hwndDlg, IDC_TXTCOLOR, CPM_GETCOLOUR, 0, 0);
			g_plugin.setDword(TXT_COLOR_KEY, TextClr);
			if (HIWORD(wParam) == CPN_COLOURCHANGED) {
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				TxtclrLoop();
			}
			break;

		case IDC_BGCOLOR:
			BackgoundClr = SendDlgItemMessage(hwndDlg, IDC_BGCOLOR, CPM_GETCOLOUR, 0, 0);
			g_plugin.setDword(BG_COLOR_KEY, BackgoundClr);
			if (HIWORD(wParam) == CPN_COLOURCHANGED) {
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				BGclrLoop();
			}
			break;

		case IDC_HIDE_STATUS_ICON:
			ShowWindow(GetDlgItem(hwndDlg, IDC_RESTART), SW_SHOW);
			break;

		case IDC_SUPPRESS:
			EnableWindow(GetDlgItem(hwndDlg, IDC_ERROR_POPUP), (!(IsDlgButtonChecked(hwndDlg, IDC_SUPPRESS))));
			break;

		case IDC_UPDATE_ONSTART:
			EnableWindow(GetDlgItem(hwndDlg, IDC_START_DELAY), ((IsDlgButtonChecked(hwndDlg, IDC_UPDATE_ONSTART))));
			EnableWindow(GetDlgItem(hwndDlg, IDC_SPIN2), ((IsDlgButtonChecked(hwndDlg, IDC_UPDATE_ONSTART))));
			EnableWindow(GetDlgItem(hwndDlg, IDC_STARTDELAYTXT), ((IsDlgButtonChecked(hwndDlg, IDC_UPDATE_ONSTART))));
			EnableWindow(GetDlgItem(hwndDlg, IDC_STDELAYSECTXT), ((IsDlgButtonChecked(hwndDlg, IDC_UPDATE_ONSTART))));
			break;

		case IDC_TYPEFACE:
			SendMessage(hwndDlg, M_FILLSCRIPTCOMBO, wParam, 0);
		case IDC_FONTSIZE:
		case IDC_SCRIPT:
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;

		case IDC_START_DELAY:
			if (HIWORD(wParam) == EN_CHANGE)
				test++;
			if (test > 1)
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;

		case IDC_TIME:
			if (HIWORD(wParam) == EN_CHANGE)
				test2++;
			if (test2 > 2)
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;
		} // end WM_COMMAND
		break;

	case WM_DESTROY:
		test = 0;
		test2 = 0;
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code) {
		case PSN_APPLY:
			g_plugin.setByte(SUPPRESS_ERR_KEY, (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_SUPPRESS));
			g_plugin.setByte(UPDATE_ONSTART_KEY, (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_UPDATE_ONSTART));
			g_plugin.setByte(UPDATE_ON_OPEN_KEY, (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_UPDATE_ON_OPEN));
			g_plugin.setByte(HIDE_STATUS_ICON_KEY, (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_HIDE_STATUS_ICON));
			g_plugin.setByte(FONT_BOLD_KEY, (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_FONT_BOLD));
			g_plugin.setByte(FONT_ITALIC_KEY, (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_FONT_ITALIC));
			g_plugin.setByte(FONT_UNDERLINE_KEY, (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_FONT_UNDERLINE));
			g_plugin.setByte(UPDATE_ONALERT_KEY, (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_UPDATE_ONALERT));
			g_plugin.setByte(SAVE_INDIVID_POS_KEY, (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_SAVE_INDIVID_POS));
			g_plugin.setByte(NO_PROTECT_KEY, (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_NO_PROTECT));
			g_plugin.setByte(DATA_POPUP_KEY, (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_DATAPOPUP));

			wchar_t str[100];
			GetDlgItemText(hwndDlg, IDC_TYPEFACE, str, _countof(str));
			g_plugin.setWString(FONT_FACE_KEY, str);

			g_plugin.setByte(FONT_SIZE_KEY, (GetDlgItemInt(hwndDlg, IDC_FONTSIZE, nullptr, FALSE)));
			g_plugin.setByte(FONT_SCRIPT_KEY, ((uint8_t)SendDlgItemMessage(hwndDlg, IDC_SCRIPT, CB_GETITEMDATA, SendDlgItemMessage(hwndDlg, IDC_SCRIPT, CB_GETCURSEL, 0, 0), 0)));

			g_plugin.setByte(ERROR_POPUP_KEY, (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_ERROR_POPUP));

			timerval = GetDlgItemInt(hwndDlg, IDC_TIME, nullptr, FALSE);
			g_plugin.setDword(REFRESH_KEY, timerval);
			g_plugin.setDword(COUNTDOWN_KEY, timerval);


			delaytime = GetDlgItemInt(hwndDlg, IDC_START_DELAY, nullptr, FALSE);
			g_plugin.setDword(START_DELAY_KEY, delaytime);

			BackgoundClr = (SendDlgItemMessage(hwndDlg, IDC_BGCOLOR, CPM_GETCOLOUR, 0, 0));
			TextClr = (SendDlgItemMessage(hwndDlg, IDC_TXTCOLOR, CPM_GETCOLOUR, 0, 0));

			if ((g_plugin.getDword(REFRESH_KEY, 0) != 0)) {
				KillTimer(nullptr, timerId);
				KillTimer(nullptr, Countdown);
				timerId = SetTimer(nullptr, 0, ((g_plugin.getDword(REFRESH_KEY, 0)) * MINUTE), timerfunc);
				Countdown = SetTimer(nullptr, 0, MINUTE, Countdownfunc);
			}
			if ((g_plugin.getDword(REFRESH_KEY, 0) == 0)) {
				KillTimer(nullptr, timerId);
				KillTimer(nullptr, Countdown);
			}
			test = 0;
		}
		break; // end apply
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// module entry point

int OptInitialise(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.szTitle.a = MODULENAME;
	odp.flags = ODPF_BOLDGROUPS;

	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT);
	odp.szGroup.a = LPGEN("Network");
	odp.pfnDlgProc = DlgProcOpt;
	g_plugin.addOptions(wParam, &odp);

	// if popup service exists
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_POPUP);
	odp.szGroup.a = LPGEN("Popups");
	odp.pfnDlgProc = DlgPopUpOpts;
	g_plugin.addOptions(wParam, &odp);
	return 0;
}
