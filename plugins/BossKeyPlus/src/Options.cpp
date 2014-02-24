/*
	BossKey - Hide Miranda from your boss :)
	Copyright (C) 2002-2003 Goblineye Entertainment, (C) 2007-2010 Billy_Bons

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

#include "BossKey.h"

bool g_fOptionsOpen;
bool g_fReqRass;
extern HGENMENU g_hMenuItem;

INT_PTR CALLBACK MainOptDlg(HWND hwndDlg,UINT msg,WPARAM wParam,LPARAM lParam)
{
	static bool s_fRedraw;

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{
			g_fOptionsOpen = true;
			s_fRedraw = false;

			// set icon and tooltip for variables help button
			variables_skin_helpbutton(hwndDlg, IDC_MAINOPT_VARHELP);

			SendDlgItemMessage(hwndDlg,IDC_MAINOPT_PASS,EM_LIMITTEXT,MAXPASSLEN,0); // limit password length

			DBVARIANT dbVar;

			if (!db_get_s(NULL,MOD_NAME,"password",&dbVar)) {
				SetDlgItemTextA(hwndDlg,IDC_MAINOPT_PASS,dbVar.pszVal);
				db_free(&dbVar);
			}

			CheckDlgButton(hwndDlg,IDC_MAINOPT_SETONLINEBACK,(g_wMask & OPT_SETONLINEBACK) ? (BST_CHECKED) : (BST_UNCHECKED));
			CheckDlgButton(hwndDlg,IDC_MAINOPT_CHANGESTATUSBOX,(g_wMask & OPT_CHANGESTATUS) ? (BST_CHECKED) : (BST_UNCHECKED));
			CheckDlgButton(hwndDlg,IDC_MAINOPT_DISABLESNDS,(g_wMask & OPT_DISABLESNDS) ? (BST_CHECKED) : (BST_UNCHECKED));
			CheckDlgButton(hwndDlg,IDC_MAINOPT_REQPASS,(g_wMask & OPT_REQPASS) ? (BST_CHECKED) : (BST_UNCHECKED));
			CheckDlgButton(hwndDlg,IDC_MAINOPT_ONLINEONLY,(g_wMask & OPT_ONLINEONLY) ? (BST_CHECKED) : (BST_UNCHECKED));
			CheckDlgButton(hwndDlg,IDC_MAINOPT_USEDEFMSG,(g_wMask & OPT_USEDEFMSG) ? (BST_CHECKED) : (BST_UNCHECKED));
			CheckDlgButton(hwndDlg,IDC_MAINOPT_TRAYICON,(g_wMask & OPT_TRAYICON) ? (BST_CHECKED) : (BST_UNCHECKED));

			for (int i=ID_STATUS_OFFLINE; i <= ID_STATUS_OUTTOLUNCH; i++)
				SendDlgItemMessage(hwndDlg, IDC_MAINOPT_CHGSTS, CB_INSERTSTRING, -1, (LPARAM)pcli->pfnGetStatusModeDescription(i, 0));

			SendDlgItemMessage(hwndDlg, IDC_MAINOPT_CHGSTS, CB_SETCURSEL, db_get_b(NULL, MOD_NAME, "stattype", 2), 0);

			SendMessage(hwndDlg,WM_USER + 60,0,0);
			SendMessage(hwndDlg,WM_USER + 50,0,0);

			s_fRedraw = true;
		}
		return true;

	case WM_NOTIFY:
		switch (((NMHDR*)lParam)->code) {
		case PSN_APPLY:
			WORD wMask = 0;
			// we apply changes here
			// this plugin ain't that big, no need for a seperate routine

			// write down status type
			if (IsDlgButtonChecked(hwndDlg,IDC_MAINOPT_CHANGESTATUSBOX) == BST_CHECKED) {
				db_set_b(NULL,MOD_NAME,"stattype",(BYTE)SendDlgItemMessage(hwndDlg,IDC_MAINOPT_CHGSTS,CB_GETCURSEL,0,0));

				// status msg, if needed
				if (IsWindowEnabled(GetDlgItem(hwndDlg,IDC_MAINOPT_STATMSG))) { // meaning we should save it
					TCHAR tszMsg[1025];
					GetDlgItemText(hwndDlg,IDC_MAINOPT_STATMSG,tszMsg,1024);
					if (tszMsg[0] != 0)
						db_set_ts(NULL,MOD_NAME,"statmsg",tszMsg);
					else // delete current setting
						db_unset(NULL,MOD_NAME,"statmsg");
				}
				wMask |= OPT_CHANGESTATUS;
			}

			// checkbox
			if (IsDlgButtonChecked(hwndDlg,IDC_MAINOPT_REQPASS) == BST_CHECKED) {
				char szPass[MAXPASSLEN+1];
				GetDlgItemTextA(hwndDlg,IDC_MAINOPT_PASS,szPass,MAXPASSLEN+1);
				if (szPass[0] != 0){
					db_set_s(NULL,MOD_NAME,"password",szPass);
					wMask |= OPT_REQPASS;
				}
			}
			if (IsDlgButtonChecked(hwndDlg,IDC_MAINOPT_SETONLINEBACK) == BST_CHECKED) wMask |= OPT_SETONLINEBACK;
			if (IsDlgButtonChecked(hwndDlg,IDC_MAINOPT_DISABLESNDS) == BST_CHECKED) wMask	|= OPT_DISABLESNDS;
			if (IsDlgButtonChecked(hwndDlg,IDC_MAINOPT_ONLINEONLY) == BST_CHECKED) wMask |= OPT_ONLINEONLY;
			if (IsDlgButtonChecked(hwndDlg,IDC_MAINOPT_USEDEFMSG) == BST_CHECKED)  wMask |= OPT_USEDEFMSG;
			if (IsDlgButtonChecked(hwndDlg,IDC_MAINOPT_TRAYICON) == BST_CHECKED)  wMask |= OPT_TRAYICON;

			db_set_w(NULL,MOD_NAME,"optsmask",wMask);
			g_wMask = wMask;
			return true;
		}
		break;

	case WM_USER+50: // we're told to checkout the selection state of the combobox, and enable/disable accordingly
		{
			BYTE bSelection = (BYTE)SendDlgItemMessage(hwndDlg,IDC_MAINOPT_CHGSTS,CB_GETCURSEL,0,0);
			WORD wMode = STATUS_ARR_TO_ID[bSelection];
			if (IsDlgButtonChecked(hwndDlg,IDC_MAINOPT_USEDEFMSG) == BST_CHECKED) {
				TCHAR *ptszDefMsg = GetDefStatusMsg(wMode, 0);
				SetDlgItemText(hwndDlg, IDC_MAINOPT_STATMSG, ptszDefMsg);
				if(ptszDefMsg)
					mir_free(ptszDefMsg);
			}
			else {
				DBVARIANT dbVar;
				SendDlgItemMessage(hwndDlg,IDC_MAINOPT_STATMSG,EM_LIMITTEXT,1024,0);
				if (!db_get_ts(NULL,MOD_NAME,"statmsg",&dbVar)) {
					SetDlgItemText(hwndDlg,IDC_MAINOPT_STATMSG,dbVar.ptszVal);
					db_free(&dbVar);
				}
			}
			EnableWindow(GetDlgItem(hwndDlg,IDC_MAINOPT_STATMSG),(IsDlgButtonChecked(hwndDlg,IDC_MAINOPT_CHANGESTATUSBOX) == BST_CHECKED) && (IsDlgButtonChecked(hwndDlg,IDC_MAINOPT_USEDEFMSG) != BST_CHECKED));
			ShowWindow(GetDlgItem(hwndDlg, IDC_MAINOPT_VARHELP), IsWindowEnabled(GetDlgItem(hwndDlg,IDC_MAINOPT_STATMSG)) && ServiceExists(MS_VARS_FORMATSTRING));
		}
		return true;

	case WM_USER+60:
		{
			bool fEnable = IsDlgButtonChecked(hwndDlg,IDC_MAINOPT_CHANGESTATUSBOX) == BST_CHECKED;
			EnableWindow(GetDlgItem(hwndDlg,IDC_MAINOPT_CHGSTS),fEnable);
			EnableWindow(GetDlgItem(hwndDlg,IDC_MAINOPT_SETONLINEBACK),fEnable);
			EnableWindow(GetDlgItem(hwndDlg,IDC_MAINOPT_ONLINEONLY),fEnable);
			EnableWindow(GetDlgItem(hwndDlg,IDC_MAINOPT_USEDEFMSG),fEnable);			
			EnableWindow(GetDlgItem(hwndDlg,IDC_MAINOPT_PASS),IsDlgButtonChecked(hwndDlg,IDC_MAINOPT_REQPASS) == BST_CHECKED);
		}
		return true;

	case WM_COMMAND:
		switch (HIWORD(wParam)) {
		case CBN_SELCHANGE:
			if (LOWORD(wParam) == IDC_MAINOPT_CHGSTS)
				SendMessage(hwndDlg,WM_USER + 50,0,0);

			SendMessage(GetParent(hwndDlg),PSM_CHANGED,(WPARAM)hwndDlg,0);
			return 0;

		case BN_CLICKED:
			switch(LOWORD(wParam)) {
			case IDC_MAINOPT_DISABLESNDS:
			case IDC_MAINOPT_SETONLINEBACK:
			case IDC_MAINOPT_ONLINEONLY:
			case IDC_MAINOPT_TRAYICON:
				SendMessage(GetParent(hwndDlg),PSM_CHANGED,(WPARAM)hwndDlg,0);
				return true;

			case IDC_MAINOPT_USEDEFMSG:
				SendMessage(hwndDlg,WM_USER + 50,0,0);
				SendMessage(GetParent(hwndDlg),PSM_CHANGED,(WPARAM)hwndDlg,0);
				return true;

			case IDC_MAINOPT_CHANGESTATUSBOX:
				SendMessage(hwndDlg,WM_USER + 60,0,0);
				SendMessage(hwndDlg,WM_USER + 50,0,0);
				SendMessage(GetParent(hwndDlg),PSM_CHANGED,(WPARAM)hwndDlg,0);
				return true;

			case IDC_MAINOPT_REQPASS:
				SendMessage(hwndDlg,WM_USER + 60,0,0);
				SendMessage(GetParent(hwndDlg),PSM_CHANGED,(WPARAM)hwndDlg,0);
				return true;

			case IDC_MAINOPT_VARHELP:
				variables_showhelp(hwndDlg, IDC_MAINOPT_STATMSG, VHF_INPUT|VHF_HELP, 0, 0);
				SendMessage(GetParent(hwndDlg),PSM_CHANGED,(WPARAM)hwndDlg,0);
				return true;

			case IDC_MAINOPT_LNK_HOTKEY:
				OPENOPTIONSDIALOG ood = {0};
				ood.cbSize = sizeof(ood);
				ood.pszGroup = "Customize";
				ood.pszPage = "Hotkeys";
				Options_Open(&ood);
				return (true);
			}
			return 0;

		case EN_UPDATE:
			switch( LOWORD( wParam )) {
			case IDC_MAINOPT_STATMSG:
			case IDC_MAINOPT_PASS:
				if (s_fRedraw)
					SendMessage(GetParent(hwndDlg),PSM_CHANGED,(WPARAM)hwndDlg,0);
			}
		}
		break;

	case WM_DESTROY:
		g_fOptionsOpen = false;
		return 0;
	}
	return(false);
}

INT_PTR CALLBACK AdvOptDlg(HWND hwndDlg,UINT msg,WPARAM wParam,LPARAM lParam)
{
	static bool inFocus = false;

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{
			g_fOptionsOpen = true;

			minutes = db_get_b(NULL,MOD_NAME,"time",10);
			TCHAR szMinutes[4] = {0};
			_itot(minutes, szMinutes, 10);
			SendDlgItemMessage(hwndDlg,IDC_MAINOPT_TIME,EM_LIMITTEXT,2,0);
			SendDlgItemMessage(hwndDlg, IDC_MAINOPT_SPIN_TIME, UDM_SETRANGE32, 1, (LPARAM)99);
			SetDlgItemText(hwndDlg, IDC_MAINOPT_TIME, szMinutes);			
			CheckDlgButton(hwndDlg,IDC_MAINOPT_HIDEIFLOCK,(g_wMaskAdv & OPT_HIDEIFLOCK) ? (BST_CHECKED) : (BST_UNCHECKED));
			CheckDlgButton(hwndDlg,IDC_MAINOPT_MENUITEM,(g_wMaskAdv & OPT_MENUITEM) ? (BST_CHECKED) : (BST_UNCHECKED));
			CheckDlgButton(hwndDlg,IDC_MAINOPT_HIDEIFWINIDLE,(g_wMaskAdv & OPT_HIDEIFWINIDLE) ? (BST_CHECKED) : (BST_UNCHECKED));
			CheckDlgButton(hwndDlg,IDC_MAINOPT_HIDEIFMIRIDLE,(g_wMaskAdv & OPT_HIDEIFMIRIDLE) ? (BST_CHECKED) : (BST_UNCHECKED));
			CheckDlgButton(hwndDlg,IDC_MAINOPT_HIDEIFSCRSVR,(g_wMaskAdv & OPT_HIDEIFSCRSVR) ? (BST_CHECKED) : (BST_UNCHECKED));
			CheckDlgButton(hwndDlg,IDC_MAINOPT_HIDEONSTART,(g_wMaskAdv & OPT_HIDEONSTART) ? (BST_CHECKED) : (BST_UNCHECKED));
			CheckDlgButton(hwndDlg,IDC_MAINOPT_RESTORE,(g_wMaskAdv & OPT_RESTORE) ? (BST_CHECKED) : (BST_UNCHECKED));
		}
		return true;

	case WM_NOTIFY:
		{
			NMHDR* nmhdr = (NMHDR*)lParam;
			switch (nmhdr->code) {
			case PSN_APPLY:
				WORD wMaskAdv = 0;
				if (IsDlgButtonChecked(hwndDlg,IDC_MAINOPT_HIDEIFLOCK) == BST_CHECKED)		wMaskAdv |= OPT_HIDEIFLOCK;
				if (IsDlgButtonChecked(hwndDlg,IDC_MAINOPT_HIDEIFWINIDLE) == BST_CHECKED)	wMaskAdv |= OPT_HIDEIFWINIDLE;
				if (IsDlgButtonChecked(hwndDlg,IDC_MAINOPT_HIDEIFMIRIDLE) == BST_CHECKED)	wMaskAdv |= OPT_HIDEIFMIRIDLE;
				if (IsDlgButtonChecked(hwndDlg,IDC_MAINOPT_HIDEIFSCRSVR) == BST_CHECKED)	wMaskAdv |= OPT_HIDEIFSCRSVR;
				if (IsDlgButtonChecked(hwndDlg,IDC_MAINOPT_HIDEONSTART) == BST_CHECKED)		wMaskAdv |= OPT_HIDEONSTART;
				if (IsDlgButtonChecked(hwndDlg,IDC_MAINOPT_RESTORE) == BST_CHECKED)			wMaskAdv |= OPT_RESTORE;

				if (IsDlgButtonChecked(hwndDlg,IDC_MAINOPT_MENUITEM) == BST_CHECKED ) {
					if (g_hMenuItem == 0)
						BossKeyMenuItemInit();
					wMaskAdv |= OPT_MENUITEM;
				}
				else if (g_hMenuItem != 0)
					BossKeyMenuItemUnInit();

				TCHAR szMinutes[4] = {0};
				GetDlgItemText(hwndDlg,IDC_MAINOPT_TIME,szMinutes,3);
				minutes = _ttoi(szMinutes);
				if(minutes<1) minutes = 1;
				db_set_b(NULL,MOD_NAME,"time",minutes);
				db_set_w(NULL,MOD_NAME,"optsmaskadv",wMaskAdv);
				g_wMaskAdv = wMaskAdv;
				return true;
			}
		}
		break;

	case WM_COMMAND:
		switch (HIWORD(wParam)) {
		case BN_CLICKED:
			switch(LOWORD(wParam)) {
			case IDC_MAINOPT_MENUITEM:
			case IDC_MAINOPT_HIDEIFLOCK:
			case IDC_MAINOPT_HIDEIFSCRSVR:
			case IDC_MAINOPT_HIDEONSTART:
			case IDC_MAINOPT_RESTORE:
				SendMessage(GetParent(hwndDlg),PSM_CHANGED,(WPARAM)hwndDlg,0);
				return true;

			case IDC_MAINOPT_HIDEIFWINIDLE:
				if (IsDlgButtonChecked(hwndDlg,IDC_MAINOPT_HIDEIFWINIDLE) == BST_CHECKED)
					CheckDlgButton(hwndDlg,IDC_MAINOPT_HIDEIFMIRIDLE,BST_UNCHECKED);
				SendMessage(GetParent(hwndDlg),PSM_CHANGED,(WPARAM)hwndDlg,0);
				return true;

			case IDC_MAINOPT_HIDEIFMIRIDLE:
				if (IsDlgButtonChecked(hwndDlg,IDC_MAINOPT_HIDEIFMIRIDLE) == BST_CHECKED)
					CheckDlgButton(hwndDlg,IDC_MAINOPT_HIDEIFWINIDLE,BST_UNCHECKED);
				SendMessage(GetParent(hwndDlg),PSM_CHANGED,(WPARAM)hwndDlg,0);
				return true;
			}
			return 0;

		case EN_SETFOCUS:
			inFocus = true;
			return 0;

		case EN_UPDATE:
			switch( LOWORD( wParam )) {
			case IDC_MAINOPT_TIME:
				if(inFocus)
					SendMessage(GetParent(hwndDlg),PSM_CHANGED,(WPARAM)hwndDlg,0);
			}
			break;
		}
		break;

	case WM_DESTROY:
		g_fOptionsOpen = false;
		return 0;
	}
	return false;
}

int OptsDlgInit(WPARAM wParam,LPARAM)
{
	OPTIONSDIALOGPAGE optDi = { sizeof(optDi) };
	optDi.position = 920000000;
	optDi.pfnDlgProc = MainOptDlg;
	optDi.pszTemplate = MAKEINTRESOURCEA(IDD_OPTDIALOGMAIN);
	optDi.hInstance = g_hInstance;
	optDi.ptszTitle = LPGENT("BossKey");
	optDi.ptszGroup = LPGENT("Events");
	optDi.ptszTab	= LPGENT("Main");
	optDi.flags = ODPF_BOLDGROUPS|ODPF_TCHAR;
	Options_AddPage(wParam, &optDi);

	optDi.pfnDlgProc = AdvOptDlg;
	optDi.pszTemplate = MAKEINTRESOURCEA(IDD_OPTDIALOGADV);
	optDi.ptszTab	= LPGENT("Advanced");
	Options_AddPage(wParam, &optDi);
	return 0;
}
