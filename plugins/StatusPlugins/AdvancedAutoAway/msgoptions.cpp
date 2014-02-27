/*
    AdvancedAutoAway Plugin for Miranda-IM (www.miranda-im.org)
    Copyright 2003-2006 P. Boon

    This program is free software; you can redistribute it and/or modify
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
#include "../commonstatus.h"
#include "advancedautoaway.h"
#include "../resource.h"

extern char *StatusModeToDbSetting(int status,const char *suffix);

void DisableDialog(HWND hwndDlg)
{
	EnableWindow(GetDlgItem(hwndDlg, IDC_STATUS), FALSE);
	EnableWindow(GetDlgItem(hwndDlg, IDC_STATUSMSG), FALSE);
	EnableWindow(GetDlgItem(hwndDlg, IDC_VARIABLESHELP), FALSE);
	EnableWindow(GetDlgItem(hwndDlg, IDC_RADUSECUSTOM), FALSE);
	EnableWindow(GetDlgItem(hwndDlg, IDC_RADUSEMIRANDA), FALSE);
}

INT_PTR CALLBACK DlgProcAutoAwayMsgOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static AAMSGSETTING** settings;
	static int last, count;

	switch( msg ) {
	case WM_INITDIALOG:
		TranslateDialogDefault( hwndDlg );
		{
			ShowWindow(GetDlgItem(hwndDlg, IDC_VARIABLESHELP), ServiceExists(MS_VARS_SHOWHELP)?SW_SHOW:SW_HIDE);
			count = 0;
			last = -1;
		
			PROTOACCOUNT** proto;
			int protoCount = 0;
			ProtoEnumAccounts(&protoCount, &proto);
			if (protoCount <= 0)
			{
				DisableDialog(hwndDlg);
				break;
			}

			DWORD protoModeMsgFlags = 0;
			for (int i=0; i < protoCount; i++)
				if (CallProtoService(proto[i]->szModuleName, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_MODEMSGSEND & ~PF1_INDIVMODEMSG)
					protoModeMsgFlags |= CallProtoService( proto[i]->szModuleName, PS_GETCAPS, PFLAGNUM_3, 0 );

			if (protoModeMsgFlags == 0)
			{
				DisableDialog(hwndDlg);
				break;
			}

			settings = ( AAMSGSETTING** )malloc(sizeof(AAMSGSETTING*));
			count = 0;
			for (int i=0; i < SIZEOF(statusModeList); i++ ) {
				if ( !( protoModeMsgFlags & Proto_Status2Flag( statusModeList[i] )))
					continue;

				int j = SendDlgItemMessage(hwndDlg, IDC_STATUS, CB_ADDSTRING, 0, (LPARAM)pcli->pfnGetStatusModeDescription(statusModeList[i], 0));
				SendDlgItemMessage( hwndDlg, IDC_STATUS, CB_SETITEMDATA, j, statusModeList[i] );
				settings = ( AAMSGSETTING** )realloc(settings, (count+1)*sizeof(AAMSGSETTING*));
				settings[count] = ( AAMSGSETTING* )malloc(sizeof(AAMSGSETTING));
				settings[count]->status = statusModeList[i];

				DBVARIANT dbv;
				if ( !db_get(NULL, MODULENAME, StatusModeToDbSetting(statusModeList[i],SETTING_STATUSMSG), &dbv)) {
					settings[count]->msg = ( char* )malloc(strlen(dbv.pszVal) + 1);
					strcpy(settings[count]->msg, dbv.pszVal);
					db_free(&dbv);
				}
				else settings[count]->msg = NULL;

				settings[count]->useCustom = db_get_b(NULL, MODULENAME, StatusModeToDbSetting(statusModeList[i], SETTING_MSGCUSTOM), FALSE);
				count += 1;
			}
			SendDlgItemMessage(hwndDlg,IDC_STATUS,CB_SETCURSEL,0,0);
			SendMessage(hwndDlg,WM_COMMAND,MAKEWPARAM(IDC_STATUS,CBN_SELCHANGE),0);
		}
		break;

	case WM_COMMAND:
		if ( ((HIWORD(wParam) == EN_CHANGE) || (HIWORD(wParam) == BN_CLICKED)) && ((HWND)lParam == GetFocus()))
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		
		switch(LOWORD(wParam)) {
		case IDC_RADUSEMIRANDA:
			CheckDlgButton(hwndDlg, IDC_RADUSECUSTOM, !IsDlgButtonChecked(hwndDlg, IDC_RADUSEMIRANDA));
			EnableWindow(GetDlgItem(hwndDlg, IDC_STATUSMSG), IsDlgButtonChecked(hwndDlg, IDC_RADUSECUSTOM));
			EnableWindow(GetDlgItem(hwndDlg, IDC_VARIABLESHELP), IsDlgButtonChecked(hwndDlg, IDC_RADUSECUSTOM));
			settings[SendDlgItemMessage(hwndDlg,IDC_STATUS,CB_GETCURSEL,0,0)]->useCustom = IsDlgButtonChecked(hwndDlg, IDC_RADUSECUSTOM);
			break;

		case IDC_RADUSECUSTOM:
			CheckDlgButton(hwndDlg, IDC_RADUSEMIRANDA, !IsDlgButtonChecked(hwndDlg, IDC_RADUSECUSTOM));
			EnableWindow(GetDlgItem(hwndDlg, IDC_STATUSMSG), IsDlgButtonChecked(hwndDlg, IDC_RADUSECUSTOM));
			EnableWindow(GetDlgItem(hwndDlg, IDC_VARIABLESHELP), IsDlgButtonChecked(hwndDlg, IDC_RADUSECUSTOM));
			settings[SendDlgItemMessage(hwndDlg,IDC_STATUS,CB_GETCURSEL,0,0)]->useCustom = IsDlgButtonChecked(hwndDlg, IDC_RADUSECUSTOM);
			break;

		case IDC_STATUS:
			if ( HIWORD( wParam ) == CBN_SELCHANGE ) {
				int i = SendDlgItemMessage(hwndDlg,IDC_STATUS,CB_GETCURSEL,0,0);
				int len = SendDlgItemMessage(hwndDlg, IDC_STATUSMSG, WM_GETTEXTLENGTH, 0, 0);
				if ( last != -1 ) {
					if (settings[last]->msg == NULL)
						settings[last]->msg = ( char* )malloc(len+1);
					else
						settings[last]->msg = ( char* )realloc(settings[last]->msg, len+1);
					SendDlgItemMessageA(hwndDlg, IDC_STATUSMSG, WM_GETTEXT, (LPARAM)(len+1), (WPARAM)settings[last]->msg);
				}

				if (i != -1) {
					if (settings[i]->msg != NULL)
						SetDlgItemTextA(hwndDlg, IDC_STATUSMSG, settings[i]->msg);
					else {
						ptrT msg((TCHAR*)CallService(MS_AWAYMSG_GETSTATUSMSGT, settings[i]->status, 0));
						SetDlgItemText(hwndDlg, IDC_STATUSMSG, (msg != NULL) ? msg : _T(""));
					}

					if (settings[i]->useCustom) {
						EnableWindow(GetDlgItem(hwndDlg, IDC_STATUSMSG), TRUE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_VARIABLESHELP), TRUE);
						CheckDlgButton(hwndDlg, IDC_RADUSECUSTOM, TRUE);
						CheckDlgButton(hwndDlg, IDC_RADUSEMIRANDA, FALSE);
					}
					else {
						EnableWindow(GetDlgItem(hwndDlg, IDC_STATUSMSG), FALSE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_VARIABLESHELP), FALSE);
						CheckDlgButton(hwndDlg, IDC_RADUSEMIRANDA, TRUE);
						CheckDlgButton(hwndDlg, IDC_RADUSECUSTOM, FALSE);
					}
				}
				last = i;
			}
			break;

		case IDC_VARIABLESHELP:
			CallService(MS_VARS_SHOWHELP, (WPARAM)GetDlgItem(hwndDlg, IDC_STATUSMSG), 0);
			break;
		}
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code) {
		case PSN_APPLY:
			SendMessage(hwndDlg,WM_COMMAND,MAKEWPARAM(IDC_STATUS,CBN_SELCHANGE),0);
			for (int i=0; i < count; i++ ) {
				db_set_b(NULL, MODULENAME, StatusModeToDbSetting(settings[i]->status,SETTING_MSGCUSTOM), (BYTE)settings[i]->useCustom);
				if ( (settings[i]->useCustom) && (settings[i]->msg != NULL) && (settings[i]->msg[0] != '\0'))
					db_set_s(NULL, MODULENAME, StatusModeToDbSetting(settings[i]->status,SETTING_STATUSMSG), settings[i]->msg);
			}
			break;
		}
		break;
	
	case WM_DESTROY:
		for (int i=0; i < count; i++ ) {
			free(settings[i]->msg);
			free(settings[i]);
		}
		free(settings);
		break;
	}
	
	return FALSE;
}
