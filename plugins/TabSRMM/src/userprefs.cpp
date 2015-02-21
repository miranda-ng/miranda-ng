/////////////////////////////////////////////////////////////////////////////////////////
// Miranda NG: the free IM client for Microsoft* Windows*
//
// Copyright (ñ) 2012-15 Miranda NG project,
// Copyright (c) 2000-09 Miranda ICQ/IM project,
// all portions of this codebase are copyrighted to the people
// listed in contributors.txt.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// you should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// part of tabSRMM messaging plugin for Miranda.
//
// (C) 2005-2010 by silvercircle _at_ gmail _dot_ com and contributors
//
// global/local message log options
// local (per user) template overrides
// view mode (ieview/default)
// text formatting

#include "commonheaders.h"

#define UPREF_ACTION_APPLYOPTIONS 1
#define UPREF_ACTION_REMAKELOG 2
#define UPREF_ACTION_SWITCHLOGVIEWER 4

static HWND hCpCombo;

static BOOL CALLBACK FillCpCombo(LPCTSTR str)
{
	int i;
	UINT cp = _ttoi(str);
	for (i = 0; cpTable[i].cpName != NULL && cpTable[i].cpId != cp; i++);
	if (cpTable[i].cpName != NULL) {
		LRESULT iIndex = SendMessage(hCpCombo, CB_ADDSTRING, -1, (LPARAM)TranslateTS(cpTable[i].cpName));
		SendMessage(hCpCombo, CB_SETITEMDATA, (WPARAM)iIndex, cpTable[i].cpId);
	}
	return TRUE;
}

static int have_ieview = 0, have_hpp = 0;

static INT_PTR CALLBACK DlgProcUserPrefs(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	MCONTACT hContact = (MCONTACT)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{
			hContact = lParam;
			DWORD maxhist = M.GetDword(hContact, "maxhist", 0);
			BYTE bIEView = M.GetByte(hContact, "ieview", 0);
			BYTE bHPP = M.GetByte(hContact, "hpplog", 0);
			int iLocalFormat = M.GetDword(hContact, "sendformat", 0);
			BYTE bSplit = M.GetByte(hContact, "splitoverride", 0);
			BYTE bInfoPanel = M.GetByte(hContact, "infopanel", 0);
			BYTE bAvatarVisible = M.GetByte(hContact, "hideavatar", -1);

			have_ieview = ServiceExists(MS_IEVIEW_WINDOW);
			have_hpp = ServiceExists("History++/ExtGrid/NewWindow");

			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)lParam);

			SendDlgItemMessage(hwndDlg, IDC_INFOPANEL, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Use global setting"));
			SendDlgItemMessage(hwndDlg, IDC_INFOPANEL, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Always on"));
			SendDlgItemMessage(hwndDlg, IDC_INFOPANEL, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Always off"));
			SendDlgItemMessage(hwndDlg, IDC_INFOPANEL, CB_SETCURSEL, bInfoPanel == 0 ? 0 : (bInfoPanel == 1 ? 1 : 2), 0);

			SendDlgItemMessage(hwndDlg, IDC_SHOWAVATAR, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Use global setting"));
			SendDlgItemMessage(hwndDlg, IDC_SHOWAVATAR, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Show always (if present)"));
			SendDlgItemMessage(hwndDlg, IDC_SHOWAVATAR, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Never show it at all"));
			SendDlgItemMessage(hwndDlg, IDC_SHOWAVATAR, CB_SETCURSEL, bAvatarVisible == 0xff ? 0 : (bAvatarVisible == 1 ? 1 : 2), 0);

			SendDlgItemMessage(hwndDlg, IDC_IEVIEWMODE, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Use global setting"));
			SendDlgItemMessage(hwndDlg, IDC_IEVIEWMODE, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Force default message log"));

			SendDlgItemMessage(hwndDlg, IDC_IEVIEWMODE, CB_SETITEMDATA, 0, 0);
			SendDlgItemMessage(hwndDlg, IDC_IEVIEWMODE, CB_SETITEMDATA, 1, 1);

			if (have_hpp) {
				SendDlgItemMessage(hwndDlg, IDC_IEVIEWMODE, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Force History++"));
				SendDlgItemMessage(hwndDlg, IDC_IEVIEWMODE, CB_SETITEMDATA, 2, 2);
			}

			if (have_ieview) {
				SendDlgItemMessage(hwndDlg, IDC_IEVIEWMODE, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Force IEView"));
				SendDlgItemMessage(hwndDlg, IDC_IEVIEWMODE, CB_SETITEMDATA, have_hpp ? 3 : 2, 3);
			}

			if (bIEView == 0 && bHPP == 0)
				SendDlgItemMessage(hwndDlg, IDC_IEVIEWMODE, CB_SETCURSEL, 0, 0);
			else if (bIEView == 0xff && bHPP == 0xff)
				SendDlgItemMessage(hwndDlg, IDC_IEVIEWMODE, CB_SETCURSEL, 1, 0);
			else {
				if (bHPP == 1)
					SendDlgItemMessage(hwndDlg, IDC_IEVIEWMODE, CB_SETCURSEL, have_hpp ? 2 : 0, 0);
				if (bIEView == 1)
					SendDlgItemMessage(hwndDlg, IDC_IEVIEWMODE, CB_SETCURSEL, (have_hpp && have_ieview) ? 3 : (have_ieview ? 2 : 0), 0);
			}

			SendDlgItemMessage(hwndDlg, IDC_TEXTFORMATTING, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Use global setting"));
			SendDlgItemMessage(hwndDlg, IDC_TEXTFORMATTING, CB_INSERTSTRING, -1, (LPARAM)TranslateT("BBCode"));
			SendDlgItemMessage(hwndDlg, IDC_TEXTFORMATTING, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Force off"));

			SendDlgItemMessage(hwndDlg, IDC_TEXTFORMATTING, CB_SETCURSEL, iLocalFormat == 0 ? 0 : (iLocalFormat == -1 ? 2 : (1)), 0);

			if (CheckMenuItem(PluginConfig.g_hMenuFavorites, (UINT_PTR)lParam, MF_BYCOMMAND | MF_UNCHECKED) == -1)
				CheckDlgButton(hwndDlg, IDC_ISFAVORITE, BST_UNCHECKED);
			else
				CheckDlgButton(hwndDlg, IDC_ISFAVORITE, BST_CHECKED);

			CheckDlgButton(hwndDlg, IDC_PRIVATESPLITTER, bSplit);
			CheckDlgButton(hwndDlg, IDC_TEMPLOVERRIDE, db_get_b(hContact, TEMPLATES_MODULE, "enabled", 0) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_RTLTEMPLOVERRIDE, db_get_b(hContact, RTLTEMPLATES_MODULE, "enabled", 0) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_LOADONLYACTUAL, M.GetByte(hContact, "ActualHistory", 0) ? BST_CHECKED : BST_UNCHECKED);

			SendDlgItemMessage(hwndDlg, IDC_TRIMSPIN, UDM_SETRANGE, 0, MAKELONG(1000, 5));
			SendDlgItemMessage(hwndDlg, IDC_TRIMSPIN, UDM_SETPOS, 0, maxhist);
			Utils::enableDlgControl(hwndDlg, IDC_TRIMSPIN, maxhist != 0);
			Utils::enableDlgControl(hwndDlg, IDC_TRIM, maxhist != 0);
			CheckDlgButton(hwndDlg, IDC_ALWAYSTRIM2, maxhist != 0 ? BST_CHECKED : BST_UNCHECKED);

			hCpCombo = GetDlgItem(hwndDlg, IDC_CODEPAGES);
			DWORD sCodePage = M.GetDword(hContact, "ANSIcodepage", 0);
			EnumSystemCodePages((CODEPAGE_ENUMPROC)FillCpCombo, CP_INSTALLED);
			SendDlgItemMessage(hwndDlg, IDC_CODEPAGES, CB_INSERTSTRING, 0, (LPARAM)TranslateT("Use default codepage"));
			if (sCodePage == 0)
				SendDlgItemMessage(hwndDlg, IDC_CODEPAGES, CB_SETCURSEL, 0, 0);
			else {
				for (int i = 0; i < SendDlgItemMessage(hwndDlg, IDC_CODEPAGES, CB_GETCOUNT, 0, 0); i++)
					if (SendDlgItemMessage(hwndDlg, IDC_CODEPAGES, CB_GETITEMDATA, i, 0) == (LRESULT)sCodePage)
						SendDlgItemMessage(hwndDlg, IDC_CODEPAGES, CB_SETCURSEL, i, 0);
			}
			CheckDlgButton(hwndDlg, IDC_FORCEANSI, M.GetByte(hContact, "forceansi", 0) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_IGNORETIMEOUTS, M.GetByte(hContact, "no_ack", 0) ? BST_CHECKED : BST_UNCHECKED);

			ShowWindow(hwndDlg, SW_SHOW);
		}
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_ALWAYSTRIM2:
			Utils::enableDlgControl(hwndDlg, IDC_TRIMSPIN, IsDlgButtonChecked(hwndDlg, IDC_ALWAYSTRIM2) != 0);
			Utils::enableDlgControl(hwndDlg, IDC_TRIM, IsDlgButtonChecked(hwndDlg, IDC_ALWAYSTRIM2) != 0);
			break;

		case WM_USER + 100:
			TWindowData *dat = 0;
			DWORD	*pdwActionToTake = (DWORD *)lParam;
			unsigned int iOldIEView = 0;
			HWND	hWnd = M.FindWindow(hContact);
			DWORD	sCodePage = M.GetDword(hContact, "ANSIcodepage", 0);
			BYTE	bOldInfoPanel = M.GetByte(hContact, "infopanel", 0);

			if (hWnd) {
				dat = (TWindowData*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
				if (dat)
					iOldIEView = GetIEViewMode(dat->hContact);
			}
			int iIndex = SendDlgItemMessage(hwndDlg, IDC_IEVIEWMODE, CB_GETCURSEL, 0, 0);
			int iMode = SendDlgItemMessage(hwndDlg, IDC_IEVIEWMODE, CB_GETITEMDATA, iIndex, 0);

			if (iIndex != CB_ERR && (iMode >= 0 && iMode <= 3)) {
				switch (iMode) {
				case 0:
					db_set_b(hContact, SRMSGMOD_T, "ieview", 0);
					db_set_b(hContact, SRMSGMOD_T, "hpplog", 0);
					break;
				case 1:
					db_set_b(hContact, SRMSGMOD_T, "ieview", -1);
					db_set_b(hContact, SRMSGMOD_T, "hpplog", -1);
					break;
				case 2:
					db_set_b(hContact, SRMSGMOD_T, "ieview", -1);
					db_set_b(hContact, SRMSGMOD_T, "hpplog", 1);
					break;
				case 3:
					db_set_b(hContact, SRMSGMOD_T, "ieview", 1);
					db_set_b(hContact, SRMSGMOD_T, "hpplog", -1);
					break;
				}
				if (hWnd && dat) {
					unsigned int iNewIEView = GetIEViewMode(dat->hContact);
					if (iNewIEView != iOldIEView) {
						if (pdwActionToTake)
							*pdwActionToTake |= UPREF_ACTION_SWITCHLOGVIEWER;
					}
				}
			}
			if ((iIndex = SendDlgItemMessage(hwndDlg, IDC_TEXTFORMATTING, CB_GETCURSEL, 0, 0)) != CB_ERR) {
				if (iIndex == 0)
					db_unset(hContact, SRMSGMOD_T, "sendformat");
				else
					db_set_dw(hContact, SRMSGMOD_T, "sendformat", iIndex == 2 ? -1 : 1);
			}

			iIndex = SendDlgItemMessage(hwndDlg, IDC_CODEPAGES, CB_GETCURSEL, 0, 0);
			DWORD	newCodePage = (DWORD)SendDlgItemMessage(hwndDlg, IDC_CODEPAGES, CB_GETITEMDATA, iIndex, 0);
			if (newCodePage != sCodePage) {
				db_set_dw(hContact, SRMSGMOD_T, "ANSIcodepage", newCodePage);
				if (hWnd && dat) {
					dat->codePage = newCodePage;
					SendMessage(hWnd, DM_UPDATETITLE, 0, 1);
				}
			}
			if ((IsDlgButtonChecked(hwndDlg, IDC_FORCEANSI) ? 1 : 0) != M.GetByte(hContact, "forceansi", 0)) {
				db_set_b(hContact, SRMSGMOD_T, "forceansi", (BYTE)(IsDlgButtonChecked(hwndDlg, IDC_FORCEANSI) ? 1 : 0));
				if (hWnd && dat)
					dat->sendMode = IsDlgButtonChecked(hwndDlg, IDC_FORCEANSI) ? dat->sendMode | SMODE_FORCEANSI : dat->sendMode & ~SMODE_FORCEANSI;
			}
			if (IsDlgButtonChecked(hwndDlg, IDC_ISFAVORITE)) {
				if (!M.GetByte(hContact, "isFavorite", 0))
					AddContactToFavorites(hContact, NULL, NULL, NULL, 0, 0, 1, PluginConfig.g_hMenuFavorites);
			}
			else DeleteMenu(PluginConfig.g_hMenuFavorites, hContact, MF_BYCOMMAND);

			db_set_b(hContact, SRMSGMOD_T, "isFavorite", (WORD)(IsDlgButtonChecked(hwndDlg, IDC_ISFAVORITE) ? 1 : 0));
			db_set_b(hContact, SRMSGMOD_T, "splitoverride", (BYTE)(IsDlgButtonChecked(hwndDlg, IDC_PRIVATESPLITTER) ? 1 : 0));

			db_set_b(hContact, TEMPLATES_MODULE, "enabled", (BYTE)(IsDlgButtonChecked(hwndDlg, IDC_TEMPLOVERRIDE)));
			db_set_b(hContact, RTLTEMPLATES_MODULE, "enabled", (BYTE)(IsDlgButtonChecked(hwndDlg, IDC_RTLTEMPLOVERRIDE)));

			BYTE bAvatarVisible = (BYTE)SendDlgItemMessage(hwndDlg, IDC_SHOWAVATAR, CB_GETCURSEL, 0, 0);
			if (bAvatarVisible == 0)
				db_unset(hContact, SRMSGMOD_T, "hideavatar");
			else
				db_set_b(hContact, SRMSGMOD_T, "hideavatar", (BYTE)(bAvatarVisible == 1 ? 1 : 0));

			BYTE bInfoPanel = (BYTE)SendDlgItemMessage(hwndDlg, IDC_INFOPANEL, CB_GETCURSEL, 0, 0);
			if (bInfoPanel != bOldInfoPanel) {
				db_set_b(hContact, SRMSGMOD_T, "infopanel", (BYTE)(bInfoPanel == 0 ? 0 : (bInfoPanel == 1 ? 1 : -1)));
				if (hWnd && dat)
					SendMessage(hWnd, DM_SETINFOPANEL, 0, 0);
			}
			if (IsDlgButtonChecked(hwndDlg, IDC_ALWAYSTRIM2))
				db_set_dw(hContact, SRMSGMOD_T, "maxhist", (DWORD)SendDlgItemMessage(hwndDlg, IDC_TRIMSPIN, UDM_GETPOS, 0, 0));
			else
				db_set_dw(hContact, SRMSGMOD_T, "maxhist", 0);

			if (IsDlgButtonChecked(hwndDlg, IDC_LOADONLYACTUAL)) {
				db_set_b(hContact, SRMSGMOD_T, "ActualHistory", 1);
				if (hWnd && dat) dat->bActualHistory = TRUE;
			}
			else {
				db_set_b(hContact, SRMSGMOD_T, "ActualHistory", 0);
				if (hWnd && dat) dat->bActualHistory = FALSE;
			}

			if (IsDlgButtonChecked(hwndDlg, IDC_IGNORETIMEOUTS)) {
				db_set_b(hContact, SRMSGMOD_T, "no_ack", 1);
				if (hWnd && dat)
					dat->sendMode |= SMODE_NOACK;
			}
			else {
				db_unset(hContact, SRMSGMOD_T, "no_ack");
				if (hWnd && dat)
					dat->sendMode &= ~SMODE_NOACK;
			}
			if (hWnd && dat) {
				SendMessage(hWnd, DM_CONFIGURETOOLBAR, 0, 1);
				ShowPicture(dat, FALSE);
				SendMessage(hWnd, WM_SIZE, 0, 0);
				DM_ScrollToBottom(dat, 0, 1);
			}
			DestroyWindow(hwndDlg);
			break;
		}
		break;
	}
	return FALSE;
}

static struct _checkboxes
{
	UINT	uId;
	UINT	uFlag;
}
checkboxes[] = {
	IDC_UPREFS_GRID, MWF_LOG_GRID,
	IDC_UPREFS_SHOWICONS, MWF_LOG_SHOWICONS,
	IDC_UPREFS_SHOWSYMBOLS, MWF_LOG_SYMBOLS,
	IDC_UPREFS_INOUTICONS, MWF_LOG_INOUTICONS,
	IDC_UPREFS_SHOWTIMESTAMP, MWF_LOG_SHOWTIME,
	IDC_UPREFS_SHOWDATES, MWF_LOG_SHOWDATES,
	IDC_UPREFS_SHOWSECONDS, MWF_LOG_SHOWSECONDS,
	IDC_UPREFS_LOCALTIME, MWF_LOG_LOCALTIME,
	IDC_UPREFS_INDENT, MWF_LOG_INDENT,
	IDC_UPREFS_GROUPING, MWF_LOG_GROUPMODE,
	IDC_UPREFS_BBCODE, MWF_LOG_BBCODE,
	IDC_UPREFS_RTL, MWF_LOG_RTL,
	IDC_UPREFS_NORMALTEMPLATES, MWF_LOG_NORMALTEMPLATES,
	0, 0
};

/////////////////////////////////////////////////////////////////////////////////////////
// loads message log and other "per contact" flags
// it uses the global flag value (0, mwflags) and then merges per contact settings
// based on the mask value.
//
// ALWAYS mask dat->dwFlags with MWF_LOG_ALL to only affect real flag bits and
// ignore temporary bits.

int TSAPI LoadLocalFlags(TWindowData *dat)
{
	if (dat == NULL)
		return NULL;

	int i = 0;
	DWORD	dwMask = M.GetDword(dat->hContact, "mwmask", 0);
	DWORD	dwLocal = M.GetDword(dat->hContact, "mwflags", 0);
	DWORD	dwGlobal = M.GetDword("mwflags", 0);

	dat->dwFlags &= ~MWF_LOG_ALL;
	if (dat->pContainer->theme.isPrivate)
		dat->dwFlags |= (dat->pContainer->theme.dwFlags & MWF_LOG_ALL);
	else
		dat->dwFlags |= (dwGlobal & MWF_LOG_ALL);
	while (checkboxes[i].uId) {
		DWORD	maskval = checkboxes[i].uFlag;
		if (dwMask & maskval)
			dat->dwFlags = (dwLocal & maskval) ? dat->dwFlags | maskval : dat->dwFlags & ~maskval;
		i++;
	}
	return(dat->dwFlags & MWF_LOG_ALL);
}

/////////////////////////////////////////////////////////////////////////////////////////
// dialog procedure for the user preferences dialog (2nd page,
// "per contact" message log options)
//
// @params: Win32 window procedure conform
// @return LRESULT

static INT_PTR CALLBACK DlgProcUserPrefsLogOptions(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	MCONTACT hContact = (MCONTACT)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	switch (msg) {
	case WM_INITDIALOG:
		hContact = lParam;
		TranslateDialogDefault(hwndDlg);
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)hContact);
		SendMessage(hwndDlg, WM_COMMAND, WM_USER + 200, 0);
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case WM_USER + 200: {
			DWORD	dwLocalFlags, dwLocalMask, maskval;
			int i = 0;

			dwLocalFlags = M.GetDword(hContact, "mwflags", 0);
			dwLocalMask = M.GetDword(hContact, "mwmask", 0);

			while (checkboxes[i].uId) {
				maskval = checkboxes[i].uFlag;

				if (dwLocalMask & maskval)
					CheckDlgButton(hwndDlg, checkboxes[i].uId, (dwLocalFlags & maskval) ? BST_CHECKED : BST_UNCHECKED);
				else
					CheckDlgButton(hwndDlg, checkboxes[i].uId, BST_INDETERMINATE);
				i++;
			}
			if (M.GetByte("logstatuschanges", 0) == M.GetByte(hContact, "logstatuschanges", 0))
				CheckDlgButton(hwndDlg, IDC_UPREFS_LOGSTATUS, BST_INDETERMINATE);
			else
				CheckDlgButton(hwndDlg, IDC_UPREFS_LOGSTATUS, M.GetByte(hContact, "logstatuschanges", 0) ? BST_CHECKED : BST_UNCHECKED);
			break;
		}
		case WM_USER + 100: {
			int i = 0;
			LRESULT state;
			HWND	hwnd = M.FindWindow(hContact);
			DWORD	*dwActionToTake = (DWORD *)lParam, dwMask = 0, dwFlags = 0, maskval;

			TWindowData *dat = NULL;
			if (hwnd)
				dat = (TWindowData*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

			while (checkboxes[i].uId) {
				maskval = checkboxes[i].uFlag;

				state = IsDlgButtonChecked(hwndDlg, checkboxes[i].uId);
				if (state != BST_INDETERMINATE) {
					dwMask |= maskval;
					dwFlags = (state == BST_CHECKED) ? (dwFlags | maskval) : (dwFlags & ~maskval);
				}
				i++;
			}
			state = IsDlgButtonChecked(hwndDlg, IDC_UPREFS_LOGSTATUS);
			if (state != BST_INDETERMINATE)
				db_set_b(hContact, SRMSGMOD_T, "logstatuschanges", (BYTE)state);

			if (dwMask) {
				db_set_dw(hContact, SRMSGMOD_T, "mwmask", dwMask);
				db_set_dw(hContact, SRMSGMOD_T, "mwflags", dwFlags);
			}
			else {
				db_unset(hContact, SRMSGMOD_T, "mwmask");
				db_unset(hContact, SRMSGMOD_T, "mwflags");
			}

			if (hwnd && dat) {
				if (dwMask)
					*dwActionToTake |= (DWORD)UPREF_ACTION_REMAKELOG;
				if ((dat->dwFlags & MWF_LOG_RTL) != (dwFlags & MWF_LOG_RTL))
					*dwActionToTake |= (DWORD)UPREF_ACTION_APPLYOPTIONS;
			}
			break;
		}
		case IDC_REVERTGLOBAL:
			db_unset(hContact, SRMSGMOD_T, "mwmask");
			db_unset(hContact, SRMSGMOD_T, "mwflags");
			SendMessage(hwndDlg, WM_COMMAND, WM_USER + 200, 0);
			break;
		}
		break;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// dialog procedure for the user preferences dialog. Handles the top
// level window (a tab control with 2 subpages)
//
// @params: like any Win32 window procedure
//
// @return LRESULT (ignored for dialog procs, use DWLP_MSGRESULT)

INT_PTR CALLBACK DlgProcUserPrefsFrame(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	MCONTACT hContact = (MCONTACT)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	TCITEM tci;

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);

		hContact = lParam;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)hContact);

		WindowList_Add(PluginConfig.hUserPrefsWindowList, hwndDlg, hContact);
		{
			RECT rcClient;
			GetClientRect(hwndDlg, &rcClient);

			TCHAR szBuffer[180];
			mir_sntprintf(szBuffer, SIZEOF(szBuffer), TranslateT("Set messaging options for %s"), pcli->pfnGetContactDisplayName(hContact, 0));
			SetWindowText(hwndDlg, szBuffer);

			memset(&tci, 0, sizeof(tci));
			tci.mask = TCIF_PARAM | TCIF_TEXT;
			tci.lParam = (LPARAM)CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_USERPREFS), hwndDlg, DlgProcUserPrefs, hContact);
			tci.pszText = TranslateT("General");
			TabCtrl_InsertItem(GetDlgItem(hwndDlg, IDC_OPTIONSTAB), 0, &tci);
			MoveWindow((HWND)tci.lParam, 6, DPISCALEY_S(32), rcClient.right - 12, rcClient.bottom - DPISCALEY_S(80), 1);
			ShowWindow((HWND)tci.lParam, SW_SHOW);
			EnableThemeDialogTexture((HWND)tci.lParam, ETDT_ENABLETAB);

			tci.lParam = (LPARAM)CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_USERPREFS1), hwndDlg, DlgProcUserPrefsLogOptions, hContact);
			tci.pszText = TranslateT("Message Log");
			TabCtrl_InsertItem(GetDlgItem(hwndDlg, IDC_OPTIONSTAB), 1, &tci);
			MoveWindow((HWND)tci.lParam, 6, DPISCALEY_S(32), rcClient.right - 12, rcClient.bottom - DPISCALEY_S(80), 1);
			ShowWindow((HWND)tci.lParam, SW_HIDE);
			EnableThemeDialogTexture((HWND)tci.lParam, ETDT_ENABLETAB);
		}
		TabCtrl_SetCurSel(GetDlgItem(hwndDlg, IDC_OPTIONSTAB), 0);
		ShowWindow(hwndDlg, SW_SHOW);
		return TRUE;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case IDC_OPTIONSTAB:
			switch (((LPNMHDR)lParam)->code) {
			case TCN_SELCHANGING:
				tci.mask = TCIF_PARAM;
				TabCtrl_GetItem(GetDlgItem(hwndDlg, IDC_OPTIONSTAB), TabCtrl_GetCurSel(GetDlgItem(hwndDlg, IDC_OPTIONSTAB)), &tci);
				ShowWindow((HWND)tci.lParam, SW_HIDE);
				break;

			case TCN_SELCHANGE:
				tci.mask = TCIF_PARAM;
				TabCtrl_GetItem(GetDlgItem(hwndDlg, IDC_OPTIONSTAB), TabCtrl_GetCurSel(GetDlgItem(hwndDlg, IDC_OPTIONSTAB)), &tci);
				ShowWindow((HWND)tci.lParam, SW_SHOW);
				break;
			}
			break;
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDCANCEL:
			DestroyWindow(hwndDlg);
			break;

		case IDOK:
			DWORD	dwActionToTake = 0;			// child pages request which action to take
			HWND	hwnd = M.FindWindow(hContact);

			tci.mask = TCIF_PARAM;

			int count = TabCtrl_GetItemCount(GetDlgItem(hwndDlg, IDC_OPTIONSTAB));
			for (int i = 0; i < count; i++) {
				TabCtrl_GetItem(GetDlgItem(hwndDlg, IDC_OPTIONSTAB), i, &tci);
				SendMessage((HWND)tci.lParam, WM_COMMAND, WM_USER + 100, (LPARAM)&dwActionToTake);
			}
			if (hwnd) {
				TWindowData *dat = (TWindowData*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
				if (dat) {
					DWORD dwOldFlags = (dat->dwFlags & MWF_LOG_ALL);
					SetDialogToType(hwnd);
					LoadLocalFlags(dat);
					if ((dat->dwFlags & MWF_LOG_ALL) != dwOldFlags) {
						bool	fShouldHide = true;
						if (IsIconic(dat->pContainer->hwnd))
							fShouldHide = false;
						else
							ShowWindow(dat->pContainer->hwnd, SW_HIDE);
						SendMessage(hwnd, DM_OPTIONSAPPLIED, 0, 0);
						SendMessage(hwnd, DM_DEFERREDREMAKELOG, (WPARAM)hwnd, 0);
						if (fShouldHide)
							ShowWindow(dat->pContainer->hwnd, SW_SHOWNORMAL);
					}
				}
			}
			DestroyWindow(hwndDlg);
			break;
		}
		break;

	case WM_DESTROY:
		WindowList_Remove(PluginConfig.hUserPrefsWindowList, hwndDlg);
		break;
	}
	return FALSE;
}
