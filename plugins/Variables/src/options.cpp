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

#include "variables.h"

static INT_PTR CALLBACK SetOptsDlgProc(HWND hwndDlg,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch( msg ) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{
			DBVARIANT dbv;
			if (!db_get_ts( NULL, MODULENAME, SETTING_STARTUPTEXT, &dbv )) {
				SetDlgItemText(hwndDlg, IDC_FORMATTEXT, dbv.ptszVal);
				db_free(&dbv);
			}
		}
		CheckDlgButton(hwndDlg, IDC_PARSEATSTARTUP, db_get_b(NULL, MODULENAME, SETTING_PARSEATSTARTUP, 0) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_STRIPCRLF, db_get_b(NULL, MODULENAME, SETTING_STRIPCRLF, 0) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_STRIPWS, db_get_b(NULL, MODULENAME, SETTING_STRIPWS, 0) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_STRIPALL, db_get_b(NULL, MODULENAME, SETTING_STRIPALL, 0) ? BST_CHECKED : BST_UNCHECKED);
		EnableWindow(GetDlgItem(hwndDlg, IDC_STRIPCRLF), IsDlgButtonChecked(hwndDlg, IDC_STRIPALL)?FALSE:TRUE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_STRIPWS), IsDlgButtonChecked(hwndDlg, IDC_STRIPALL)?FALSE:TRUE);
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
				SendMessage(GetParent(hwndDlg),PSM_CHANGED,0,0);
				gParseOpts.bStripEOL = IsDlgButtonChecked(hwndDlg, IDC_STRIPCRLF);
				gParseOpts.bStripWS = IsDlgButtonChecked(hwndDlg, IDC_STRIPWS);
				gParseOpts.bStripAll = IsDlgButtonChecked(hwndDlg, IDC_STRIPALL);
				break;
			}
			if (LOWORD(wParam) == IDC_FORMATTEXT && IsDlgButtonChecked( hwndDlg, IDC_AUTOPARSE ))
				SendMessage(hwndDlg, VARM_PARSE, 0, 0);
		}
		switch (LOWORD(wParam)) {
		case IDC_PARSE:
			SendMessage(hwndDlg, VARM_PARSE, 0, 0);
			break;

		case IDC_AUTOPARSE:
			SendMessage(hwndDlg, VARM_PARSE, 0, 0);
			if (IsDlgButtonChecked( hwndDlg, IDC_AUTOPARSE ))
				SetTimer(hwndDlg, IDT_PARSE, 1000, NULL);
			else
				KillTimer(hwndDlg, IDT_PARSE);
			break;

		case IDC_SHOWHELP:
			variables_showhelp(hwndDlg, IDC_FORMATTEXT, VHF_FULLDLG|VHF_SETLASTSUBJECT, NULL, NULL);
			break;

		case IDC_STRIPALL:
			EnableWindow(GetDlgItem(hwndDlg, IDC_STRIPCRLF), IsDlgButtonChecked(hwndDlg, IDC_STRIPALL)?FALSE:TRUE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_STRIPWS), IsDlgButtonChecked(hwndDlg, IDC_STRIPALL)?FALSE:TRUE);
			break;
		}
		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->code == PSN_APPLY) {
			int len = SendDlgItemMessage(hwndDlg, IDC_FORMATTEXT, WM_GETTEXTLENGTH, 0, 0);
			if (len >= 0) {
				TCHAR *szFormatText = (TCHAR*)mir_calloc((len+1)* sizeof(TCHAR));
				if (szFormatText == NULL)
					break;

				if (GetDlgItemText(hwndDlg, IDC_FORMATTEXT, szFormatText, len+1) != 0)
					db_set_ts(NULL, MODULENAME, SETTING_STARTUPTEXT, szFormatText);

				mir_free(szFormatText);
			}
			db_set_b(NULL, MODULENAME, SETTING_PARSEATSTARTUP, (BYTE)(IsDlgButtonChecked(hwndDlg, IDC_PARSEATSTARTUP)?1:0));
			db_set_b(NULL, MODULENAME, SETTING_STRIPCRLF, (BYTE)(IsDlgButtonChecked(hwndDlg, IDC_STRIPCRLF)?1:0));
			db_set_b(NULL, MODULENAME, SETTING_STRIPWS, (BYTE)(IsDlgButtonChecked(hwndDlg, IDC_STRIPWS)?1:0));
			db_set_b(NULL, MODULENAME, SETTING_STRIPALL, (BYTE)(IsDlgButtonChecked(hwndDlg, IDC_STRIPALL)?1:0));
		}
		break;

	case VARM_PARSE:
		{
			TCHAR *string = Hlp_GetDlgItemText(hwndDlg, IDC_FORMATTEXT);
			if (string != NULL) {
				TCHAR *newString = variables_parsedup(string, NULL, NULL);
				if (newString != NULL) {
					SetDlgItemText(hwndDlg, IDC_RESULT, newString);
					mir_free(newString);
				}
				mir_free(string);
		}	}	
		break;

	case WM_TIMER:
		if (IsDlgButtonChecked(hwndDlg, IDC_AUTOPARSE))
			SendMessage(hwndDlg, VARM_PARSE, 0, 0);
		break;

	case WM_DESTROY:
		setParseOptions(NULL);
		break;
	}

	return FALSE;
}

int OptionsInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = { sizeof(odp) };
	odp.position      = 150000000;
	odp.pszGroup      = LPGEN("Services");
	odp.groupPosition = 910000000;
	odp.hInstance     = hInst;
	odp.pszTemplate   = MAKEINTRESOURCEA(IDD_OPTS_DIALOG);
	odp.pszTitle      = LPGEN("Variables");
	odp.pfnDlgProc    = SetOptsDlgProc;
	odp.flags         = ODPF_BOLDGROUPS;
	Options_AddPage(wParam, &odp);
	return 0;
}
