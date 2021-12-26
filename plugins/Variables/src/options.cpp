/*
Variables Plugin for Miranda-IM (www.miranda-im.org)
Copyright 2003-2006 P. Boon

This program is mir_free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "stdafx.h"

static INT_PTR CALLBACK SetOptsDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);

		DBVARIANT dbv;
		if (!g_plugin.getWString(SETTING_STARTUPTEXT, &dbv)) {
			SetDlgItemText(hwndDlg, IDC_FORMATTEXT, dbv.pwszVal);
			db_free(&dbv);
		}

		CheckDlgButton(hwndDlg, IDC_PARSEATSTARTUP, g_plugin.getByte(SETTING_PARSEATSTARTUP, 0) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_STRIPCRLF, g_plugin.getByte(SETTING_STRIPCRLF, 0) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_STRIPWS, g_plugin.getByte(SETTING_STRIPWS, 0) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_STRIPALL, g_plugin.getByte(SETTING_STRIPALL, 0) ? BST_CHECKED : BST_UNCHECKED);
		EnableWindow(GetDlgItem(hwndDlg, IDC_STRIPCRLF), IsDlgButtonChecked(hwndDlg, IDC_STRIPALL) ? FALSE : TRUE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_STRIPWS), IsDlgButtonChecked(hwndDlg, IDC_STRIPALL) ? FALSE : TRUE);
		variables_skin_helpbutton(hwndDlg, IDC_SHOWHELP);
		break;

	case WM_COMMAND:
		if ((HIWORD(wParam) == EN_CHANGE) || (HIWORD(wParam) == BN_CLICKED)) {
			switch (LOWORD(wParam)) {
			case IDC_PARSEATSTARTUP:
			case IDC_STRIPCRLF:
			case IDC_STRIPWS:
			case IDC_STRIPALL:
			case IDC_FORMATTEXT:
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				gParseOpts.bStripEOL = IsDlgButtonChecked(hwndDlg, IDC_STRIPCRLF);
				gParseOpts.bStripWS = IsDlgButtonChecked(hwndDlg, IDC_STRIPWS);
				gParseOpts.bStripAll = IsDlgButtonChecked(hwndDlg, IDC_STRIPALL);
				break;
			}
			if (LOWORD(wParam) == IDC_FORMATTEXT && IsDlgButtonChecked(hwndDlg, IDC_AUTOPARSE))
				SendMessage(hwndDlg, VARM_PARSE, 0, 0);
		}
		switch (LOWORD(wParam)) {
		case IDC_PARSE:
			SendMessage(hwndDlg, VARM_PARSE, 0, 0);
			break;

		case IDC_AUTOPARSE:
			SendMessage(hwndDlg, VARM_PARSE, 0, 0);
			if (IsDlgButtonChecked(hwndDlg, IDC_AUTOPARSE))
				SetTimer(hwndDlg, IDT_PARSE, 1000, nullptr);
			else
				KillTimer(hwndDlg, IDT_PARSE);
			break;

		case IDC_SHOWHELP:
			variables_showhelp(hwndDlg, IDC_FORMATTEXT, VHF_FULLDLG | VHF_SETLASTSUBJECT, nullptr, nullptr);
			break;

		case IDC_STRIPALL:
			EnableWindow(GetDlgItem(hwndDlg, IDC_STRIPCRLF), IsDlgButtonChecked(hwndDlg, IDC_STRIPALL) ? FALSE : TRUE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_STRIPWS), IsDlgButtonChecked(hwndDlg, IDC_STRIPALL) ? FALSE : TRUE);
			break;
		}
		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->code == PSN_APPLY) {
			int len = SendDlgItemMessage(hwndDlg, IDC_FORMATTEXT, WM_GETTEXTLENGTH, 0, 0);
			if (len >= 0) {
				wchar_t *szFormatText = (wchar_t*)mir_calloc((len + 1) * sizeof(wchar_t));
				if (szFormatText == nullptr)
					break;

				if (GetDlgItemText(hwndDlg, IDC_FORMATTEXT, szFormatText, len + 1) != 0)
					g_plugin.setWString(SETTING_STARTUPTEXT, szFormatText);

				mir_free(szFormatText);
			}
			g_plugin.setByte(SETTING_PARSEATSTARTUP, (uint8_t)(IsDlgButtonChecked(hwndDlg, IDC_PARSEATSTARTUP) ? 1 : 0));
			g_plugin.setByte(SETTING_STRIPCRLF, (uint8_t)(IsDlgButtonChecked(hwndDlg, IDC_STRIPCRLF) ? 1 : 0));
			g_plugin.setByte(SETTING_STRIPWS, (uint8_t)(IsDlgButtonChecked(hwndDlg, IDC_STRIPWS) ? 1 : 0));
			g_plugin.setByte(SETTING_STRIPALL, (uint8_t)(IsDlgButtonChecked(hwndDlg, IDC_STRIPALL) ? 1 : 0));
		}
		break;

	case VARM_PARSE:
		{
			wchar_t *string = Hlp_GetDlgItemText(hwndDlg, IDC_FORMATTEXT);
			if (string != nullptr) {
				wchar_t *newString = variables_parsedup(string, nullptr, NULL);
				if (newString != nullptr) {
					SetDlgItemText(hwndDlg, IDC_RESULT, newString);
					mir_free(newString);
				}
				mir_free(string);
			}
		}
		break;

	case WM_TIMER:
		if (IsDlgButtonChecked(hwndDlg, IDC_AUTOPARSE))
			SendMessage(hwndDlg, VARM_PARSE, 0, 0);
		break;

	case WM_DESTROY:
		setParseOptions(nullptr);
		break;
	}

	return FALSE;
}

int OptionsInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.flags = ODPF_BOLDGROUPS;
	odp.position = 150000000;
	odp.szGroup.a = LPGEN("Services");
	odp.szTitle.a = LPGEN("Variables");
	odp.pfnDlgProc = SetOptsDlgProc;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTS_DIALOG);
	g_plugin.addOptions(wParam, &odp);
	return 0;
}
