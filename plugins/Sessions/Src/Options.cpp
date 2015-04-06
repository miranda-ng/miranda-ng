/*
Sessions Management plugin for Miranda IM

Copyright (C) 2007-2008 Danil Mozhar

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "sessions.h"

HICON hIcon;
HICON hMarked, hNotMarked;

HWND hComboBox = NULL;
HWND hComboBoxEdit = NULL;

HWND hOpClistControl = NULL;

static BOOL bOptionsInit;
int opses_count;
BOOL bSesssionNameChanged = 0;

MCONTACT session_list_t[255] = { 0 };

HWND g_opHdlg;

int width;
HDC hdc = NULL;
RECT lprect;
RECT rc;
POINT pt;
BOOL bChecked = FALSE;

int OpLoadSessionContacts(WPARAM, LPARAM lparam)
{
	memset(session_list_t, 0, sizeof(session_list_t));

	for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
		if (LoadContactsFromMask(hContact, 1, lparam)) {
			int i = GetInSessionOrder(hContact, 1, lparam);
			session_list_t[i] = hContact;
		}
	}

	int i;
	for (i = 0; session_list_t[i] != 0; i++)
		SendDlgItemMessage(g_opHdlg, IDC_OPCLIST, LB_ADDSTRING, 0, CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)session_list_t[i], GCDNF_TCHAR));

	return i;
}

static LRESULT CALLBACK ComboBoxSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_ERASEBKGND:
		return TRUE;

	case EM_SETSEL:
		if (!hOpClistControl)
			return HideCaret(hwnd);
		break;

	case WM_GETDLGCODE:
		if (!hOpClistControl)
			return DLGC_WANTARROWS;
		break;

	case WM_SETCURSOR:
		if (!hOpClistControl) {
			SetCursor(LoadCursor(NULL, IDC_ARROW));
			return TRUE;
		}
		break;

	case WM_LBUTTONDOWN:
		if (hOpClistControl)
			break;
		HideCaret(hwnd);

	case WM_LBUTTONDBLCLK:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONDBLCLK:
		SendMessage(hComboBox, CB_SHOWDROPDOWN, 1, 0);
		return TRUE;

	case WM_NCLBUTTONDBLCLK:
	case WM_NCLBUTTONDOWN:
		if (!bChecked) {
			MarkUserDefSession(opses_count, 1);
			hIcon = hMarked;
			bChecked = TRUE;
			RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_FRAME);
		}
		else {
			MarkUserDefSession(opses_count, 0);
			hIcon = hNotMarked;
			bChecked = FALSE;
			RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_FRAME);
		}
		break;

	case WM_MOUSEMOVE:
		if (hOpClistControl)
			break;

	case WM_NCMOUSEMOVE:
		return TRUE;

	case WM_NCPAINT:
		hdc = GetDC(hwnd);
		GetClientRect(hwnd, &rc);
		rc.left = rc.right;
		rc.right = rc.left + 16;
		FillRect(hdc, &rc, (HBRUSH)GetSysColor(COLOR_WINDOW));
		DrawIconEx(hdc, rc.left, 0, hIcon, 16, 16, 0, NULL, DI_NORMAL);
		ReleaseDC(hwnd, hdc);
		break;

	case WM_NCCALCSIZE:
	{
		NCCALCSIZE_PARAMS *ncsParam = (NCCALCSIZE_PARAMS*)lParam;
		ncsParam->rgrc[0].right -= 16;
	}
	break;

	case WM_NCHITTEST:
		LRESULT lr = mir_callNextSubclass(hwnd, ComboBoxSubclassProc, msg, wParam, lParam);
		if (lr == HTNOWHERE)
			lr = HTOBJECT;
		return lr;
	}
	return mir_callNextSubclass(hwnd, ComboBoxSubclassProc, msg, wParam, lParam);
}

static INT_PTR CALLBACK OptionsProc(HWND hdlg, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hdlg);
		{
			COMBOBOXINFO cbi = { 0 };
			cbi.cbSize = sizeof(cbi);

			opses_count = 0;
			g_opHdlg = hdlg;
			bOptionsInit = TRUE;
			hMarked = Skin_GetIconByHandle(iconList[1].hIcolib);
			hNotMarked = Skin_GetIconByHandle(iconList[2].hIcolib);

			hIcon = (bChecked = IsMarkedUserDefSession(opses_count)) ? hMarked : hNotMarked;

			SetDlgItemInt(hdlg, IDC_TRACK, g_ses_limit = db_get_b(0, MODNAME, "TrackCount", 10), FALSE);
			SendDlgItemMessage(hdlg, IDC_SPIN1, UDM_SETRANGE, 0, MAKELONG(10, 1));
			SendDlgItemMessage(hdlg, IDC_SPIN1, UDM_SETPOS, 0, GetDlgItemInt(hdlg, IDC_TRACK, NULL, FALSE));

			SendDlgItemMessage(hdlg, IDC_OPCLIST, LB_RESETCONTENT, 0, 0);
			SetDlgItemInt(hdlg, IDC_STARTDELAY, db_get_w(NULL, MODNAME, "StartupModeDelay", 1500), FALSE);
			int startupmode = db_get_b(NULL, MODNAME, "StartupMode", 3);
			int exitmode = db_get_b(NULL, MODNAME, "ShutdownMode", 2);

			g_bExclHidden = db_get_b(NULL, MODNAME, "ExclHidden", 0) != 0;
			g_bWarnOnHidden = db_get_b(NULL, MODNAME, "WarnOnHidden", 0) != 0;
			g_bOtherWarnings = db_get_b(NULL, MODNAME, "OtherWarnings", 1) != 0;
			g_bCrashRecovery = db_get_b(NULL, MODNAME, "CrashRecovery", 0) != 0;

			CheckDlgButton(hdlg, IDC_EXCLHIDDEN, g_bExclHidden ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hdlg, IDC_LASTHIDDENWARN, g_bWarnOnHidden ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hdlg, IDC_WARNINGS, g_bOtherWarnings ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hdlg, IDC_CRASHRECOVERY, g_bCrashRecovery ? BST_CHECKED : BST_UNCHECKED);

			if (startupmode == 1)
				CheckDlgButton(hdlg, IDC_STARTDIALOG, BST_CHECKED);
			else if (startupmode == 3) {
				CheckDlgButton(hdlg, IDC_STARTDIALOG, BST_CHECKED);
				CheckDlgButton(hdlg, IDC_CHECKLAST, BST_CHECKED);
			}
			else if (startupmode == 2) {
				CheckDlgButton(hdlg, IDC_RLOADLAST, BST_CHECKED);
				EnableWindow(GetDlgItem(hdlg, IDC_CHECKLAST), FALSE);
			}
			else if (startupmode == 0) {
				CheckDlgButton(hdlg, IDC_RNOTHING, BST_CHECKED);
				EnableWindow(GetDlgItem(hdlg, IDC_STARTDELAY), FALSE);
				EnableWindow(GetDlgItem(hdlg, IDC_STATICOP), FALSE);
				EnableWindow(GetDlgItem(hdlg, IDC_STATICMS), FALSE);
				EnableWindow(GetDlgItem(hdlg, IDC_CHECKLAST), FALSE);
			}

			if (exitmode == 0) {
				CheckDlgButton(hdlg, IDC_REXDSAVE, BST_CHECKED);
				EnableWindow(GetDlgItem(hdlg, IDC_EXSTATIC1), FALSE);
				EnableWindow(GetDlgItem(hdlg, IDC_EXSTATIC2), FALSE);
				EnableWindow(GetDlgItem(hdlg, IDC_TRACK), FALSE);
				EnableWindow(GetDlgItem(hdlg, IDC_SPIN1), FALSE);
			}
			else if (exitmode == 1)
				CheckDlgButton(hdlg, IDC_REXASK, BST_CHECKED);
			else if (exitmode == 2)
				CheckDlgButton(hdlg, IDC_REXSAVE, BST_CHECKED);

			LoadSessionToCombobox(hdlg, 1, 255, "UserSessionDsc", 0);
			if (SendDlgItemMessage(hdlg, IDC_LIST, CB_GETCOUNT, 0, 0)) {
				EnableWindow(GetDlgItem(hdlg, IDC_EDIT), TRUE);
				SendDlgItemMessage(hdlg, IDC_LIST, CB_SETCURSEL, 0, 0);
				if (!OpLoadSessionContacts(0, opses_count))
					EnableWindow(GetDlgItem(hdlg, IDC_DEL), FALSE);
			}

			GetComboBoxInfo(GetDlgItem(hdlg, IDC_LIST), &cbi);
			mir_subclassWindow(cbi.hwndItem, ComboBoxSubclassProc);

			hComboBoxEdit = cbi.hwndItem;
			hComboBox = cbi.hwndCombo;

			SetWindowPos(hComboBoxEdit, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

			bOptionsInit = FALSE;
		}
		break;

	case WM_CTLCOLORLISTBOX:
		switch (GetDlgCtrlID((HWND)lparam)) {
		case IDC_OPCLIST:
			SetBkMode((HDC)wparam, TRANSPARENT);
			return (BOOL)CreateSolidBrush(GetSysColor(COLOR_3DFACE));
		}
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lparam)->code) {
		case PSN_APPLY:
		{
			int iDelay = GetDlgItemInt(hdlg, IDC_STARTDELAY, NULL, FALSE);
			db_set_w(0, MODNAME, "StartupModeDelay", (WORD)iDelay);
			db_set_b(0, MODNAME, "TrackCount", (BYTE)(g_ses_limit = GetDlgItemInt(hdlg, IDC_TRACK, NULL, FALSE)));
			if (IsDlgButtonChecked(hdlg, IDC_REXSAVE))
				db_set_b(NULL, MODNAME, "ShutdownMode", 2);
			else if (IsDlgButtonChecked(hdlg, IDC_REXDSAVE))
				db_set_b(NULL, MODNAME, "ShutdownMode", 0);
			else if (IsDlgButtonChecked(hdlg, IDC_REXASK))
				db_set_b(NULL, MODNAME, "ShutdownMode", 1);

			if (IsDlgButtonChecked(hdlg, IDC_STARTDIALOG)) {
				if (BST_UNCHECKED == IsDlgButtonChecked(hdlg, IDC_CHECKLAST))
					db_set_b(NULL, MODNAME, "StartupMode", 1);
				else
					db_set_b(NULL, MODNAME, "StartupMode", 3);
			}
			else if (IsDlgButtonChecked(hdlg, IDC_RLOADLAST))
				db_set_b(NULL, MODNAME, "StartupMode", 2);
			else if (IsDlgButtonChecked(hdlg, IDC_RNOTHING))
				db_set_b(NULL, MODNAME, "StartupMode", 0);

			db_set_b(NULL, MODNAME, "ExclHidden", (BYTE)(IsDlgButtonChecked(hdlg, IDC_EXCLHIDDEN) ? (g_bExclHidden = 1) : (g_bExclHidden = 0)));
			db_set_b(NULL, MODNAME, "WarnOnHidden", (BYTE)(IsDlgButtonChecked(hdlg, IDC_LASTHIDDENWARN) ? (g_bWarnOnHidden = 1) : (g_bWarnOnHidden = 0)));
			db_set_b(NULL, MODNAME, "OtherWarnings", (BYTE)(IsDlgButtonChecked(hdlg, IDC_WARNINGS) ? (g_bOtherWarnings = 1) : (g_bOtherWarnings = 0)));
			db_set_b(NULL, MODNAME, "CrashRecovery", (BYTE)(IsDlgButtonChecked(hdlg, IDC_CRASHRECOVERY) ? (g_bCrashRecovery = 1) : (g_bCrashRecovery = 0)));
		}
		return 1;

		case CLN_CHECKCHANGED:
			if (((LPNMHDR)lparam)->idFrom == IDC_EMCLIST) {
				int iSelection = (int)((NMCLISTCONTROL *)lparam)->hItem;
				MCONTACT hContact = db_find_first();
				for (; hContact; hContact = db_find_next(hContact))
					if (SendDlgItemMessage(hdlg, IDC_EMCLIST, CLM_FINDCONTACT, hContact, 0) == iSelection)
						break;
				if (hContact)
					EnableWindow(GetDlgItem(hdlg, IDC_SAVE), TRUE);
				else
					EnableWindow(GetDlgItem(hdlg, IDC_SAVE), FALSE);
			}
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wparam)) {
		case IDC_LIST:
			switch (HIWORD(wparam)) {
			case CBN_EDITCHANGE:
				EnableWindow(GetDlgItem(hdlg, IDC_SAVE), TRUE);
				bSesssionNameChanged = TRUE;
				break;

			case CBN_SELCHANGE:
			{
				HWND hCombo = GetDlgItem(hdlg, IDC_LIST);
				int index = SendMessage(hCombo, CB_GETCURSEL, 0, 0);
				if (index != CB_ERR) {
					opses_count = SendMessage(hCombo, CB_GETITEMDATA, (WPARAM)index, 0);
					SendDlgItemMessage(hdlg, IDC_OPCLIST, LB_RESETCONTENT, 0, 0);
					if (IsMarkedUserDefSession(opses_count)) {
						hIcon = hMarked;
						bChecked = TRUE;
						RedrawWindow(hComboBoxEdit, NULL, NULL, RDW_INVALIDATE | RDW_NOCHILDREN | RDW_UPDATENOW | RDW_FRAME);
					}
					else {
						hIcon = hNotMarked;
						bChecked = FALSE;
						RedrawWindow(hComboBoxEdit, NULL, NULL, RDW_INVALIDATE | RDW_NOCHILDREN | RDW_UPDATENOW | RDW_FRAME);
					}
					OpLoadSessionContacts(0, opses_count);
					if (!hOpClistControl)
						EnableWindow(GetDlgItem(hdlg, IDC_DEL), TRUE);
					else {
						for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact))
							SendMessage(hOpClistControl, CLM_SETCHECKMARK, hContact, 0);

						for (int i = 0; session_list_t[i] > 0; i++) {
							MCONTACT hContact = (MCONTACT)SendMessage(hOpClistControl, CLM_FINDCONTACT, (WPARAM)session_list_t[i], 0);
							SendMessage(hOpClistControl, CLM_SETCHECKMARK, hContact, 1);
						}
						EnableWindow(GetDlgItem(hdlg, IDC_SAVE), FALSE);
					}
				}
			}
			}
			break;

		case IDC_EDIT:
			if (!hOpClistControl) {
				ShowWindow(GetDlgItem(hdlg, IDC_OPCLIST), SW_HIDE);
				EnableWindow(GetDlgItem(hdlg, IDC_DEL), FALSE);
				//EnableWindow(GetDlgItem(hdlg,IDC_SAVE),TRUE);
				SetDlgItemText(hdlg, IDC_EDIT, TranslateT("View"));
				hOpClistControl = CreateWindowEx(WS_EX_STATICEDGE, _T(CLISTCONTROL_CLASS), _T(""),
					WS_TABSTOP | WS_VISIBLE | WS_CHILD,
					14, 198, 161, 163, hdlg, (HMENU)IDC_EMCLIST, g_hInst, 0);

				SetWindowLongPtr(hOpClistControl, GWL_STYLE,
					GetWindowLongPtr(hOpClistControl, GWL_STYLE) | CLS_CHECKBOXES | CLS_HIDEEMPTYGROUPS | CLS_USEGROUPS | CLS_GREYALTERNATE | CLS_GROUPCHECKBOXES);
				SendMessage(hOpClistControl, CLM_SETEXSTYLE, CLS_EX_DISABLEDRAGDROP | CLS_EX_TRACKSELECT, 0);

				SendMessage(hOpClistControl, WM_TIMER, TIMERID_REBUILDAFTER, 0);

				for (int i = 0; session_list_t[i] > 0; i++) {
					HANDLE hItem = (HANDLE)SendMessage(hOpClistControl, CLM_FINDCONTACT, (WPARAM)session_list_t[i], 0);
					SendMessage(hOpClistControl, CLM_SETCHECKMARK, (WPARAM)hItem, 1);
				}
			}
			else {
				ShowWindow(GetDlgItem(hdlg, IDC_OPCLIST), SW_SHOWNA);
				EnableWindow(GetDlgItem(hdlg, IDC_DEL), TRUE);
				EnableWindow(GetDlgItem(hdlg, IDC_SAVE), FALSE);
				SetDlgItemText(hdlg, IDC_EDIT, TranslateT("Edit"));
				DestroyWindow(hOpClistControl);
				hOpClistControl = NULL;
			}
			break;

		case IDC_SAVE:
		{
			int i = 0;
			for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
				BYTE res = (BYTE)SendDlgItemMessage(hdlg, IDC_EMCLIST, CLM_GETCHECKMARK,
					SendDlgItemMessage(hdlg, IDC_EMCLIST, CLM_FINDCONTACT, hContact, 0), 0);
				if (res) {
					SetSessionMark(hContact, 1, '1', opses_count);
					SetInSessionOrder(hContact, 1, opses_count, i);
					i++;
				}
				else {
					SetSessionMark(hContact, 1, '0', opses_count);
					SetInSessionOrder(hContact, 1, opses_count, 0);
				}
			}
			if (bSesssionNameChanged) {
				if (GetWindowTextLength(hComboBoxEdit)) {
					TCHAR szUserSessionName[MAX_PATH] = { '\0' };
					GetWindowText(hComboBoxEdit, szUserSessionName, SIZEOF(szUserSessionName));
					RenameUserDefSession(opses_count, szUserSessionName);
					SendDlgItemMessage(hdlg, IDC_LIST, CB_RESETCONTENT, 0, 0);
					LoadSessionToCombobox(hdlg, 1, 255, "UserSessionDsc", 0);
				}
				bSesssionNameChanged = FALSE;
			}
			EnableWindow(GetDlgItem(hdlg, IDC_SAVE), FALSE);
		}
		break;

		case IDC_DEL:
			DelUserDefSession(opses_count);

			SendDlgItemMessage(hdlg, IDC_OPCLIST, LB_RESETCONTENT, 0, 0);
			SendDlgItemMessage(hdlg, IDC_LIST, CB_RESETCONTENT, 0, 0);

			LoadSessionToCombobox(hdlg, 1, 255, "UserSessionDsc", 0);

			opses_count = 0;

			if (SendDlgItemMessage(hdlg, IDC_LIST, CB_GETCOUNT, 0, 0)) {
				EnableWindow(GetDlgItem(hdlg, IDC_EDIT), TRUE);
				SendDlgItemMessage(hdlg, IDC_LIST, CB_SETCURSEL, 0, 0);
				if (!OpLoadSessionContacts(0, opses_count))
					EnableWindow(GetDlgItem(hdlg, IDC_DEL), FALSE);
			}
			else {
				EnableWindow(GetDlgItem(hdlg, IDC_EDIT), FALSE);
				EnableWindow(GetDlgItem(hdlg, IDC_DEL), FALSE);
			}
			break;

		case IDC_STARTDIALOG:
			EnableWindow(GetDlgItem(hdlg, IDC_STARTDELAY), TRUE);
			EnableWindow(GetDlgItem(hdlg, IDC_STATICOP), TRUE);
			EnableWindow(GetDlgItem(hdlg, IDC_STATICMS), TRUE);
			EnableWindow(GetDlgItem(hdlg, IDC_CHECKLAST), TRUE);
			SendMessage(GetParent(hdlg), PSM_CHANGED, 0, 0);
			break;

		case IDC_RLOADLAST:
			EnableWindow(GetDlgItem(hdlg, IDC_STARTDELAY), TRUE);
			EnableWindow(GetDlgItem(hdlg, IDC_STATICOP), TRUE);
			EnableWindow(GetDlgItem(hdlg, IDC_STATICMS), TRUE);
			EnableWindow(GetDlgItem(hdlg, IDC_CHECKLAST), FALSE);
			SendMessage(GetParent(hdlg), PSM_CHANGED, 0, 0);
			break;

		case IDC_RNOTHING:
			EnableWindow(GetDlgItem(hdlg, IDC_STARTDELAY), FALSE);
			EnableWindow(GetDlgItem(hdlg, IDC_STATICOP), FALSE);
			EnableWindow(GetDlgItem(hdlg, IDC_STATICMS), FALSE);
			EnableWindow(GetDlgItem(hdlg, IDC_CHECKLAST), FALSE);
			SendMessage(GetParent(hdlg), PSM_CHANGED, 0, 0);
			break;

		case IDC_REXSAVE:
			EnableWindow(GetDlgItem(hdlg, IDC_EXSTATIC1), TRUE);
			EnableWindow(GetDlgItem(hdlg, IDC_EXSTATIC2), TRUE);
			EnableWindow(GetDlgItem(hdlg, IDC_TRACK), TRUE);
			EnableWindow(GetDlgItem(hdlg, IDC_SPIN1), TRUE);
			SendMessage(GetParent(hdlg), PSM_CHANGED, 0, 0);
			break;

		case IDC_REXDSAVE:
			EnableWindow(GetDlgItem(hdlg, IDC_EXSTATIC1), FALSE);
			EnableWindow(GetDlgItem(hdlg, IDC_EXSTATIC2), FALSE);
			EnableWindow(GetDlgItem(hdlg, IDC_TRACK), FALSE);
			EnableWindow(GetDlgItem(hdlg, IDC_SPIN1), FALSE);
			SendMessage(GetParent(hdlg), PSM_CHANGED, 0, 0);
			break;

		case IDC_REXASK:
			EnableWindow(GetDlgItem(hdlg, IDC_EXSTATIC1), TRUE);
			EnableWindow(GetDlgItem(hdlg, IDC_EXSTATIC2), TRUE);
			EnableWindow(GetDlgItem(hdlg, IDC_TRACK), TRUE);
			EnableWindow(GetDlgItem(hdlg, IDC_SPIN1), TRUE);
			SendMessage(GetParent(hdlg), PSM_CHANGED, 0, 0);
			break;
		}

		if (HIWORD(wparam) == CBN_DROPDOWN && !hOpClistControl) {
			SendMessage(hComboBoxEdit, EM_SETSEL, 0, 0);
			SendMessage(hComboBoxEdit, EM_SCROLLCARET, 0, 0);
			SendMessage(hComboBoxEdit, WM_KILLFOCUS, 0, 0);
			HideCaret(hComboBoxEdit);
		}

		if ((HIWORD(wparam) != CBN_DROPDOWN) && (LOWORD(wparam) == IDC_LIST) && !hOpClistControl) {
			SendMessage(hComboBoxEdit, EM_SCROLLCARET, 0, 0);
			HideCaret(hComboBoxEdit);
		}

		if ((LOWORD(wparam) == IDC_STARTDELAY) && (HIWORD(wparam) != EN_CHANGE || (HWND)lparam != GetFocus()))
			return 0;

		if (lparam && !bOptionsInit && (HIWORD(wparam) == BN_CLICKED) && (GetFocus() == (HWND)lparam) &&
			((LOWORD(wparam) == IDC_CHECKLAST) || ((LOWORD(wparam) >= IDC_EXCLHIDDEN) && (LOWORD(wparam) <= IDC_CRASHRECOVERY))))
			SendMessage(GetParent(hdlg), PSM_CHANGED, 0, 0);

		return 0;

	case WM_CLOSE:
		EndDialog(hdlg, 0);
		return 0;
	}
	return 0;
}

int OptionsInit(WPARAM wparam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = { sizeof(odp) };
	odp.position = 955000000;
	odp.hInstance = g_hInst;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS);
	odp.pszTitle = MODNAME;
	odp.pfnDlgProc = OptionsProc;
	odp.pszGroup = LPGEN("Message sessions");
	odp.flags = ODPF_BOLDGROUPS;
	Options_AddPage(wparam, &odp);
	return 0;
}
