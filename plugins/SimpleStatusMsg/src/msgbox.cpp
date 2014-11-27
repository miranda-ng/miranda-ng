/*

Simple Status Message plugin for Miranda IM
Copyright (C) 2006-2011 Bartosz 'Dezeath' Bia³ek, (C) 2005 Harven

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "commonheaders.h"

#define I_ICON_DEL		0
#define I_ICON_HIST		1
#define I_ICON_MSG		2
#define I_ICON_ADD		3
#define I_ICON_CLEAR 	4

struct MsgBoxData
{
	char	*m_szProto;
	int		m_iStatus;
	int		m_iInitialStatus;
	int		m_iStatusModes;
	int		m_iStatusMsgModes;
	HWND	status_cbex;
	HWND	recent_cbex;
	HICON	icon[5];
	HIMAGELIST status_icons;
	HIMAGELIST other_icons;
	int		m_iCountdown;
	int		curr_sel_msg;
	int		m_iDlgFlags;
	int		max_hist_msgs;
	int		num_def_msgs;
	BOOL	m_bPredefChanged;
	BOOL	m_bIsMsgHistory;
	BOOL	m_bOnStartup;
};

typedef struct
{
	clock_t ctLastDblClk;
	UINT uClocksPerDblClk;
} MsgEditCtrl;

HIMAGELIST AddOtherIconsToImageList(struct MsgBoxData *data)
{
	HIMAGELIST himlIcons = ImageList_Create(16, 16, ILC_COLOR32 | ILC_MASK, 4, 0);

	for (int i = 0; i < 5; ++i)
		ImageList_AddIcon(himlIcons, data->icon[i]);

	return himlIcons;
}

int statusicon_nr[9];

HIMAGELIST AddStatusIconsToImageList(const char *szProto, int status_flags)
{
	int num_icons = 1;
	int i;

	for (i = 0; i < 9; ++i)
		if (Proto_Status2Flag(ID_STATUS_ONLINE + i) & status_flags)
			num_icons++;

	HIMAGELIST himlIcons = ImageList_Create(16, 16, ILC_COLOR32 | ILC_MASK, num_icons, 0);
	HICON hicon = LoadSkinnedProtoIcon(szProto, ID_STATUS_OFFLINE);
	ImageList_AddIcon(himlIcons, hicon);
	Skin_ReleaseIcon(hicon);
	statusicon_nr[0] = 0;

	int j = 1;
	for (i = 0; i < 9; ++i) {
		if (Proto_Status2Flag(ID_STATUS_ONLINE + i) & status_flags) {
			hicon = LoadSkinnedProtoIcon(szProto, ID_STATUS_ONLINE + i);
			ImageList_AddIcon(himlIcons, hicon);
			Skin_ReleaseIcon(hicon);
			statusicon_nr[i + 1] = j;
			j++;
		}
		else
			statusicon_nr[i + 1] = 0;
	}

	return himlIcons;
}

HWND WINAPI CreateStatusComboBoxEx(HWND hwndDlg, struct MsgBoxData *data)
{
	int j = 0, cur_sel = 0;
	TCHAR *status_desc;

	if (!(data->m_iDlgFlags & DLG_SHOW_STATUS))
		return NULL;

	HWND handle = CreateWindowEx(0, WC_COMBOBOXEX, NULL,
					WS_TABSTOP | CBS_NOINTEGRALHEIGHT | WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST,
					0, 0, 0, 240, hwndDlg, NULL, g_hInst, NULL);

	COMBOBOXEXITEM cbei = {0};
	if (!(data->m_iDlgFlags & DLG_SHOW_STATUS_ICONS))
		cbei.mask = CBEIF_LPARAM | CBEIF_TEXT;
	else
		cbei.mask = CBEIF_LPARAM | CBEIF_TEXT | CBEIF_IMAGE | CBEIF_SELECTEDIMAGE;

	if (data->m_bOnStartup)
		status_desc = (TCHAR *)TranslateT("<startup>");
	else
		status_desc = (TCHAR *)TranslateT("<current>");
	cbei.iItem = j;
	cbei.pszText = (LPTSTR)status_desc;
	cbei.cchTextMax = sizeof(status_desc);

	if (data->m_szProto || data->m_iStatus == ID_STATUS_CURRENT) {
		if (data->m_bOnStartup)
			j = GetStartupStatus(data->m_szProto) - ID_STATUS_OFFLINE;
		else
			j = GetCurrentStatus(data->m_szProto) - ID_STATUS_OFFLINE;
	}
	else
		j = data->m_iStatus - ID_STATUS_OFFLINE;

	if (j < 0 || j > 9)
		j = 0; // valid status modes only

	if (data->m_iDlgFlags & DLG_SHOW_STATUS_ICONS) {
		cbei.iImage = statusicon_nr[j];
		cbei.iSelectedImage = statusicon_nr[j];
	}
	j = 0;
	cbei.lParam = (LPARAM)ID_STATUS_CURRENT;

	if (ID_STATUS_CURRENT == data->m_iInitialStatus)
		cur_sel = j;

	SendMessage(handle, CBEM_INSERTITEM, 0, (LPARAM)&cbei);
	j++;

	for (int i = 0; i < 10; ++i) {
		if ((Proto_Status2Flag(ID_STATUS_OFFLINE + i) & data->m_iStatusModes) || i == 0) {
			status_desc = pcli->pfnGetStatusModeDescription(ID_STATUS_OFFLINE + i, 0);
			cbei.iItem = j;
			cbei.pszText = (LPTSTR)status_desc;
			cbei.cchTextMax = sizeof(status_desc);
			if (data->m_iDlgFlags & DLG_SHOW_STATUS_ICONS) {
				cbei.iImage = j - 1;
				cbei.iSelectedImage = j - 1;
			}
			cbei.lParam = (LPARAM)ID_STATUS_OFFLINE + i;

			if (ID_STATUS_OFFLINE + i == data->m_iInitialStatus)
				cur_sel = j;

			if (SendMessage(handle, CBEM_INSERTITEM, 0, (LPARAM)&cbei) == -1)
				break;
			j++;
		}
	}

	if (!data->m_szProto && (data->m_iDlgFlags & DLG_SHOW_STATUS_PROFILES) && ServiceExists(MS_SS_GETPROFILECOUNT)) {
		int defaultProfile;
		int profileCount = (int)CallService(MS_SS_GETPROFILECOUNT, (WPARAM)&defaultProfile, 0);

		for (int i = 0; i < profileCount; ++i) {
			TCHAR tszProfileName[128];
			CallService(MS_SS_GETPROFILENAME, (WPARAM)i, (LPARAM)tszProfileName);

			cbei.iItem = j;
			cbei.pszText = (LPTSTR)tszProfileName;
			cbei.cchTextMax = SIZEOF(tszProfileName);
			if (data->m_iDlgFlags & DLG_SHOW_STATUS_ICONS) {
				int k = GetCurrentStatus(NULL) - ID_STATUS_OFFLINE;
				if (k < 0 || k > 9)
					k = 0; // valid status modes only
				cbei.iImage = statusicon_nr[k];
				cbei.iSelectedImage = statusicon_nr[k];
			}
			cbei.lParam	= (LPARAM)40083 + i;

			mir_free(status_desc);

			if (SendMessage(handle, CBEM_INSERTITEM, 0, (LPARAM)&cbei) == -1)
				break;
			j++;
		}
	}

	if (!(data->m_iDlgFlags & DLG_SHOW_STATUS_ICONS))
		SendMessage(handle, CB_SETITEMHEIGHT, 0, (LPARAM)16);
	else {
		SendMessage(handle, CB_SETITEMHEIGHT, 0, (LPARAM)18);
		SendMessage(handle, CBEM_SETIMAGELIST, 0, (LPARAM)data->status_icons);
	}
	SetWindowPos(handle, NULL, 11, 11, 112, 20, SWP_NOACTIVATE);
	SendMessage(handle, CB_SETCURSEL, (WPARAM)cur_sel, 0);
	SendMessage(handle, CB_SETITEMHEIGHT, (WPARAM)-1, (LPARAM)16);

	return handle;
}

#define HISTORY_MSG		1
#define CLEAR_HISTORY	2
#define PREDEFINED_MSG	3
#define ADD_MSG			4
#define DELETE_SELECTED	5
#define DEFAULT_MSG		6

HWND WINAPI CreateRecentComboBoxEx(HWND hwndDlg, struct MsgBoxData *data)
{
	int i, j;
	char buff[16];
	BOOL found = FALSE;
	DBVARIANT dbv;
	TCHAR text[128];

	HWND handle = CreateWindowEx(0, WC_COMBOBOXEX, NULL,
					WS_TABSTOP | CBS_NOINTEGRALHEIGHT | WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST,
					0, 0, 0, 300, hwndDlg, NULL, g_hInst, NULL);

	COMBOBOXEXITEM cbei = {0};
	if (!(data->m_iDlgFlags & DLG_SHOW_LIST_ICONS))
		cbei.mask = CBEIF_LPARAM | CBEIF_TEXT | CBEIF_INDENT;
	else
		cbei.mask = CBEIF_LPARAM | CBEIF_TEXT | CBEIF_IMAGE | CBEIF_SELECTEDIMAGE;

	j = db_get_w(NULL, "SimpleStatusMsg", "LMMsg", 1);

	for (i = 1; i <= data->max_hist_msgs; ++i) {
		// history messages
		if (j < 1)
			j = data->max_hist_msgs;
		mir_snprintf(buff, SIZEOF(buff), "SMsg%d", j);
		j--;
		if (!db_get_ts(NULL, "SimpleStatusMsg", buff, &dbv)) {
			if (dbv.ptszVal != NULL && dbv.ptszVal != '\0') {
				found = TRUE;
				cbei.iItem = -1;
				cbei.pszText = (LPTSTR)dbv.ptszVal;
				cbei.cchTextMax = sizeof(dbv.ptszVal);
				if (data->m_iDlgFlags & DLG_SHOW_LIST_ICONS) {
					cbei.iImage = I_ICON_HIST;
					cbei.iSelectedImage = I_ICON_HIST;
				}
				else
					cbei.iIndent = 0;
				cbei.lParam = MAKELPARAM(HISTORY_MSG, j + 1);

				if (SendMessage(handle, CBEM_INSERTITEM, 0, (LPARAM)&cbei) == -1) {
					db_free(&dbv);
					break;
				}
			}
			db_free(&dbv);
		}
	}

	data->m_bIsMsgHistory = found;

	if ((data->m_iDlgFlags & DLG_SHOW_BUTTONS) || (data->m_iDlgFlags & DLG_SHOW_BUTTONS_FLAT)) {
		if (found) {
			if (!IsWindowEnabled(GetDlgItem(hwndDlg, IDC_BCLEAR)))
				EnableWindow(GetDlgItem(hwndDlg, IDC_BCLEAR), TRUE);
		}
		else if (IsWindowEnabled(GetDlgItem(hwndDlg, IDC_BCLEAR)))
			EnableWindow(GetDlgItem(hwndDlg, IDC_BCLEAR), FALSE);
	}
	else if (data->m_iDlgFlags & DLG_SHOW_BUTTONS_INLIST) {
		if (found) {
			if (data->m_iDlgFlags & DLG_SHOW_LIST_ICONS) {
				mir_sntprintf(text, SIZEOF(text), TranslateT("Clear history"));
				cbei.iImage = I_ICON_CLEAR;
				cbei.iSelectedImage = I_ICON_CLEAR;
			}
			else {
				mir_sntprintf(text, SIZEOF(text), _T("## %s ##"), TranslateT("Clear history"));
				cbei.iIndent = 1;
			}
			cbei.iItem = -1;
			cbei.pszText = (LPTSTR)text;
			cbei.cchTextMax = SIZEOF(text);
			cbei.lParam = MAKELPARAM(CLEAR_HISTORY, 0);
			SendMessage(handle, CBEM_INSERTITEM, 0, (LPARAM)&cbei);
		}

		cbei.iItem = -1;
		if (data->m_iDlgFlags & DLG_SHOW_LIST_ICONS) {
			mir_sntprintf(text, SIZEOF(text), TranslateT("Add to predefined"));
			cbei.iImage = I_ICON_ADD;
			cbei.iSelectedImage = I_ICON_ADD;
		}
		else {
			mir_sntprintf(text, SIZEOF(text), _T("## %s ##"), TranslateT("Add to predefined"));
			cbei.iIndent = 1;
		}
		cbei.pszText = (LPTSTR)text;
		cbei.cchTextMax = SIZEOF(text);
		cbei.lParam = MAKELPARAM(ADD_MSG, 0);
		SendMessage(handle, CBEM_INSERTITEM, 0, (LPARAM)&cbei);

		if (data->m_iDlgFlags & DLG_SHOW_LIST_ICONS) {
			mir_sntprintf(text, SIZEOF(text), TranslateT("Delete selected"));
			cbei.iImage = I_ICON_DEL;
			cbei.iSelectedImage = I_ICON_DEL;
		}
		else {
			cbei.iIndent = 1;
			mir_sntprintf(text, SIZEOF(text), _T("## %s ##"), TranslateT("Delete selected"));
		}
		cbei.iItem = -1;
		cbei.pszText = (LPTSTR)text;
		cbei.cchTextMax = SIZEOF(text);
		cbei.lParam = MAKELPARAM(DELETE_SELECTED, 0);
		SendMessage(handle, CBEM_INSERTITEM, 0, (LPARAM)&cbei);
	}

	if ((data->m_iDlgFlags & DLG_SHOW_BUTTONS) || (data->m_iDlgFlags & DLG_SHOW_BUTTONS_FLAT)) {
		if (data->num_def_msgs || found) {
			if (!IsWindowEnabled(GetDlgItem(hwndDlg, IDC_BDEL)))
				EnableWindow(GetDlgItem(hwndDlg, IDC_BDEL), TRUE);
		}
		else if (IsWindowEnabled(GetDlgItem(hwndDlg, IDC_BDEL)))
			EnableWindow(GetDlgItem(hwndDlg, IDC_BDEL), FALSE);
	}

	for (i = 1; i <= data->num_def_msgs; ++i) {
		// predefined messages
		mir_snprintf(buff, SIZEOF(buff), "DefMsg%d", i);
		if (!db_get_ts(NULL, "SimpleStatusMsg", buff, &dbv)) {
			if (dbv.ptszVal) {
				if (!lstrlen(dbv.ptszVal)) {
					db_free(&dbv);
					continue;
				}

				cbei.iItem = -1;
				cbei.pszText = (LPTSTR)dbv.ptszVal;
				cbei.cchTextMax = sizeof(dbv.ptszVal);
				if (data->m_iDlgFlags & DLG_SHOW_LIST_ICONS) {
					cbei.iImage = I_ICON_MSG;
					cbei.iSelectedImage = I_ICON_MSG;
				}
				else
					cbei.iIndent = 0;
				cbei.lParam = MAKELPARAM(PREDEFINED_MSG, i);

				if (SendMessage(handle, CBEM_INSERTITEM, 0, (LPARAM)&cbei) == -1)
					break;
			}
			db_free(&dbv);
		}
	}

	if (db_get_b(NULL, "SimpleStatusMsg", "PutDefInList", 0)) {
		cbei.iItem = -1;
		cbei.pszText = (LPTSTR)GetDefaultMessage(data->m_iStatus);
		cbei.cchTextMax = sizeof(GetDefaultMessage(data->m_iStatus));
		if (data->m_iDlgFlags & DLG_SHOW_LIST_ICONS) {
			cbei.iImage = I_ICON_MSG;
			cbei.iSelectedImage = I_ICON_MSG;
		}
		else
			cbei.iIndent = 0;
		cbei.lParam = MAKELPARAM(DEFAULT_MSG, 0);

		SendMessage(handle, CBEM_INSERTITEM, 0, (LPARAM)&cbei);
	}

	if (data->m_iDlgFlags & DLG_SHOW_LIST_ICONS)
		SendMessage(handle, CBEM_SETIMAGELIST, 0, (LPARAM)data->other_icons);
	if (!(data->m_iDlgFlags & DLG_SHOW_STATUS)) {
		SetWindowPos(handle, NULL, 11, 11, 290, 20, SWP_NOACTIVATE);
		SendMessage(handle, CB_SETDROPPEDWIDTH, (WPARAM)290, 0);
	}
	else {
		SetWindowPos(handle, NULL, 127, 11, 174, 20, SWP_NOACTIVATE);
		SendMessage(handle, CB_SETDROPPEDWIDTH, (WPARAM)250, 0);
	}
	SendMessage(handle, CB_SETITEMHEIGHT, (WPARAM)-1, (LPARAM)16);
	SendMessage(handle, CB_SETITEMHEIGHT, 0, (LPARAM)16);

	if (((data->m_iDlgFlags & DLG_SHOW_BUTTONS) || (data->m_iDlgFlags & DLG_SHOW_BUTTONS_FLAT)) && !found && !data->num_def_msgs)
		EnableWindow(handle, FALSE);

	if (((!(data->m_iDlgFlags & DLG_SHOW_BUTTONS)) && (!(data->m_iDlgFlags & DLG_SHOW_BUTTONS_FLAT)) && (!(data->m_iDlgFlags & DLG_SHOW_BUTTONS_INLIST))) && !found && !data->num_def_msgs)
		EnableWindow(handle, FALSE);

	return handle;
}

VOID APIENTRY HandlePopupMenu(HWND hwnd, POINT pt, HWND edit_control)
{
	HMENU hmenu = LoadMenu(g_hInst, MAKEINTRESOURCE(IDR_EDITMENU));
	if (hmenu == NULL)
		return;

	HMENU hmenuTrackPopup = GetSubMenu(hmenu, 0);

	TranslateMenu(hmenuTrackPopup);

	ClientToScreen(hwnd, (LPPOINT)&pt);

	LPDWORD sel_s = NULL, sel_e = NULL;
	SendMessage(edit_control, EM_GETSEL, (WPARAM)&sel_s, (LPARAM)&sel_e);
	if (sel_s == sel_e) {
		EnableMenuItem(hmenuTrackPopup, IDM_COPY, MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(hmenuTrackPopup, IDM_CUT, MF_BYCOMMAND | MF_GRAYED);
	}
	if (SendMessage(edit_control, WM_GETTEXTLENGTH, 0, 0) == 0)
		EnableMenuItem(hmenuTrackPopup, IDM_DELETE, MF_BYCOMMAND | MF_GRAYED);

	if (ServiceExists(MS_VARS_FORMATSTRING))
		DeleteMenu(hmenuTrackPopup, ID__VARIABLES, MF_BYCOMMAND);
	else
		DeleteMenu(hmenuTrackPopup, 8, MF_BYPOSITION);

	DeleteMenu(hmenuTrackPopup, 7, MF_BYPOSITION);

	int m_selection = TrackPopupMenu(hmenuTrackPopup, TPM_LEFTALIGN | TPM_RETURNCMD, pt.x, pt.y, 0, hwnd, NULL);
	switch (m_selection) {
		case IDM_COPY:
			SendMessage(edit_control, WM_COPY, 0, 0);
			break;

		case IDM_CUT:
			SendMessage(edit_control, WM_CUT, 0, 0);
			break;

		case IDM_PASTE:
			SendMessage(edit_control, WM_PASTE, 0, 0);
			break;

		case IDM_SELECTALL:
			SendMessage(edit_control, EM_SETSEL, 0, -1);
			break;

		case IDM_DELETE:
			SendMessage(edit_control, WM_SETTEXT, 0, (LPARAM)"");
			SendMessage(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(IDC_EDIT1, EN_CHANGE), (LPARAM)edit_control);
			break;

		case ID__FORTUNEAWAYMSG:
			CallService(MS_UTILS_OPENURL, 1, (LPARAM)"http://miranda-ng.org/");
			break;

		case ID__VARIABLES:
			CallService(MS_UTILS_OPENURL, 1, (LPARAM)"http://miranda-ng.org/");
			break;

		case ID__VARIABLES_MOREVARIABLES:
		{
			VARHELPINFO vhi = {0};
			vhi.cbSize = sizeof(vhi);
			vhi.flags = VHF_FULLDLG | VHF_SETLASTSUBJECT;
			vhi.hwndCtrl = edit_control;
			vhi.szSubjectDesc = NULL;
			vhi.szExtraTextDesc = NULL;
			CallService(MS_VARS_SHOWHELPEX, (WPARAM)hwnd, (LPARAM)&vhi);
			break;
		}

		default:
			if (!OpenClipboard(GetParent(hwnd)))
				break;
			if (EmptyClipboard()) {
				TCHAR item_string[128];
				int len;

				GetMenuString(hmenu, m_selection, (LPTSTR)&item_string, 128, MF_BYCOMMAND);
				len = lstrlen(item_string);
				if (len) {
					LPTSTR lptstrCopy;
					HGLOBAL hglbCopy = GlobalAlloc(GMEM_MOVEABLE, (len + 1) * sizeof(TCHAR));
					if (hglbCopy == NULL) {
						CloseClipboard();
						break;
					}
					lptstrCopy = (LPTSTR)GlobalLock(hglbCopy);
					memcpy(lptstrCopy, item_string, len * sizeof(TCHAR));
					lptstrCopy[len] = (TCHAR)0;
					GlobalUnlock(hglbCopy);

					SetClipboardData(CF_UNICODETEXT, hglbCopy);

				}
			}
			CloseClipboard();
			SendMessage(edit_control, WM_PASTE, 0, 0);
			break;
	}
	DestroyMenu(hmenu);
}

static LRESULT CALLBACK EditBoxSubProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_CONTEXTMENU:
		{
			POINT pt = {(LONG)LOWORD(lParam), (LONG)HIWORD(lParam)};
			RECT rc;
			GetClientRect(hwndDlg, &rc);

			if (pt.x == -1 && pt.y == -1) {
				GetCursorPos(&pt);
				if (!PtInRect(&rc, pt)) {
					pt.x = rc.left + (rc.right - rc.left) / 2;
					pt.y = rc.top + (rc.bottom - rc.top) / 2;
				}
			}
			else
				ScreenToClient(hwndDlg, &pt);

			if (PtInRect(&rc, pt))
				HandlePopupMenu(hwndDlg, pt, GetDlgItem(GetParent(hwndDlg), IDC_EDIT1));

			return 0;
		}

	case WM_CHAR:
		if (wParam=='\n' && GetKeyState(VK_CONTROL) & 0x8000) {
			PostMessage(GetParent(hwndDlg), WM_COMMAND, IDC_OK, 0);
			return 0;
		}
		if (wParam == 1 && GetKeyState(VK_CONTROL) & 0x8000) {	// Ctrl + A
			SendMessage(hwndDlg, EM_SETSEL, 0, -1);
			return 0;
		}
		if (wParam == 23 && GetKeyState(VK_CONTROL) & 0x8000) {	// Ctrl + W
			SendMessage(GetParent(hwndDlg), WM_COMMAND, IDC_CANCEL, 0);
			return 0;
		}
		if (wParam == 127 && GetKeyState(VK_CONTROL) & 0x8000) {	// Ctrl + Backspace
			DWORD start, end;
			TCHAR *text;
			int textLen;
			SendMessage(hwndDlg, EM_GETSEL, (WPARAM)&end, (LPARAM)(PDWORD)NULL);
			SendMessage(hwndDlg, WM_KEYDOWN, VK_LEFT, 0);
			SendMessage(hwndDlg, EM_GETSEL, (WPARAM)&start, (LPARAM)(PDWORD)NULL);
			textLen = GetWindowTextLength(hwndDlg);
			text = (TCHAR *)mir_alloc(sizeof(TCHAR) * (textLen + 1));
			GetWindowText(hwndDlg, text, textLen + 1);
			MoveMemory(text + start, text + end, sizeof(TCHAR) * (textLen + 1 - end));
			SetWindowText(hwndDlg, text);
			mir_free(text);
			SendMessage(hwndDlg, EM_SETSEL, start, start);
			SendMessage(GetParent(hwndDlg), WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(hwndDlg), EN_CHANGE), (LPARAM)hwndDlg);
			return 0;
		}
		break;

	case WM_LBUTTONDBLCLK:
		{
			MsgEditCtrl *mec = (MsgEditCtrl *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
			if (mec != NULL) {
				mec->ctLastDblClk = clock();
				mec->uClocksPerDblClk = GetDoubleClickTime() * CLOCKS_PER_SEC / 1000;
				SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)mec);
			}
			break;
		}

	case WM_LBUTTONDOWN:
		{
			MsgEditCtrl *mec = (MsgEditCtrl *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
			if (mec != NULL && UINT(clock() - mec->ctLastDblClk) < mec->uClocksPerDblClk) {
				SendMessage(hwndDlg, EM_SETSEL, 0, -1);
				return 0;
			}
			break;
		}

	case WM_SETFOCUS:
		{
			MsgEditCtrl *mec = (MsgEditCtrl *)mir_calloc(sizeof(MsgEditCtrl));
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)mec);
			break;
		}

	case WM_KILLFOCUS:
		{
			MsgEditCtrl *mec = (MsgEditCtrl *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
			mir_free(mec);
			mec = NULL;
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)mec);
			break;
		}
	}

	return mir_callNextSubclass(hwndDlg, EditBoxSubProc, uMsg, wParam, lParam);
}

int AddToPredefined(HWND hwndDlg, struct MsgBoxData *data)
{
	COMBOBOXEXITEM newitem = {0};
	int len = 0, num_items;
	TCHAR msg[1024], text[1024];

	if (IsWindowEnabled(GetDlgItem(hwndDlg, IDC_EDIT1)))
		len = GetDlgItemText(hwndDlg, IDC_EDIT1, msg, SIZEOF(msg));

	if (!len)
		return -1;

	num_items = SendMessage(data->recent_cbex, CB_GETCOUNT, 0, 0) - 1;
	for (int i = 1; i <= data->num_def_msgs; i++, num_items--) {
		newitem.mask = CBEIF_LPARAM | CBEIF_TEXT;
		newitem.iItem = num_items;
		newitem.cchTextMax = SIZEOF(text);
		newitem.pszText = text;

		SendMessage(data->recent_cbex, CBEM_GETITEM, 0, (LPARAM)&newitem);
		if (LOWORD(newitem.lParam) == PREDEFINED_MSG && !lstrcmp(text, msg))
			return num_items;
	}

	data->num_def_msgs++;
	data->m_bPredefChanged = TRUE;

	newitem.iItem = -1;
	newitem.pszText = (LPTSTR)msg;
	newitem.cchTextMax = SIZEOF(msg);
	if (data->m_iDlgFlags & DLG_SHOW_LIST_ICONS) {
		newitem.mask = CBEIF_LPARAM | CBEIF_TEXT | CBEIF_IMAGE | CBEIF_SELECTEDIMAGE;
		newitem.iImage = I_ICON_MSG;
		newitem.iSelectedImage = I_ICON_MSG;
	}
	else {
		newitem.mask = CBEIF_LPARAM | CBEIF_TEXT | CBEIF_INDENT;
		newitem.iIndent = 0;
	}
	newitem.lParam = MAKELPARAM(PREDEFINED_MSG, 0);
	return SendMessage(data->recent_cbex, CBEM_INSERTITEM, 0, (LPARAM)&newitem);
}

void ClearHistory(struct MsgBoxData *data, int cur_sel)
{
	COMBOBOXEXITEM histitem = {0};
	int i, num_items;
	char text[16], buff2[80];

	for (i = 1; i <= data->max_hist_msgs; i++) {
		mir_snprintf(text, SIZEOF(text), "SMsg%d", i);
		db_set_ts(NULL, "SimpleStatusMsg", text, _T(""));
	}
	db_set_s(NULL, "SimpleStatusMsg", "LastMsg", "");
	for (i = 0; i < accounts->count; i++) {
		if (!IsAccountEnabled(accounts->pa[i]))
			continue;

		if (!CallProtoService(accounts->pa[i]->szModuleName, PS_GETCAPS, PFLAGNUM_3, 0))
			continue;

		if (!(CallProtoService(accounts->pa[i]->szModuleName, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_MODEMSGSEND))
			continue;

		mir_snprintf(buff2, SIZEOF(buff2), "Last%sMsg", accounts->pa[i]->szModuleName);
		db_set_s(NULL, "SimpleStatusMsg", buff2, "");
	}
	db_set_w(NULL, "SimpleStatusMsg", "LMMsg", (WORD)data->max_hist_msgs);
	SendMessage(data->recent_cbex, CB_SETCURSEL, -1, 0);
	num_items = SendMessage(data->recent_cbex, CB_GETCOUNT, 0, 0);
	if (num_items == CB_ERR)
		return;

	if ((!(data->m_iDlgFlags & DLG_SHOW_BUTTONS)) && (!(data->m_iDlgFlags & DLG_SHOW_BUTTONS_FLAT)))
		SendMessage(data->recent_cbex, CBEM_DELETEITEM, (WPARAM)cur_sel, 0);

	for (i = num_items; i >= 0; i--) {
		histitem.mask = CBEIF_LPARAM;
		histitem.iItem = i;
		SendMessage(data->recent_cbex, CBEM_GETITEM, 0, (LPARAM)&histitem);
		if (LOWORD(histitem.lParam) == HISTORY_MSG)
			SendMessage(data->recent_cbex, CBEM_DELETEITEM, (WPARAM)i, 0);
	}
}

void DisplayCharsCount(struct MsgBoxData *dlg_data, HWND hwndDlg)
{
	TCHAR msg[1024];
	TCHAR status_text[128];
	int len, lines = 1;

	if (dlg_data->m_iCountdown != -2)
		return;

	len = GetDlgItemText(hwndDlg, IDC_EDIT1, msg, SIZEOF(msg));
	if (db_get_b(NULL, "SimpleStatusMsg", "RemoveCR", 0)) {
		int index, num_lines = SendMessage(GetDlgItem(hwndDlg, IDC_EDIT1), EM_GETLINECOUNT, 0, 0);
		for (int i = 1; i < num_lines; ++i) {
			index = SendMessage(GetDlgItem(hwndDlg, IDC_EDIT1), EM_LINEINDEX, (WPARAM)i, 0);
			if (msg[index - 1] == '\n')
				lines++;
		}
	}
	mir_sntprintf(status_text, SIZEOF(status_text), TranslateT("OK (%d)"), len - (lines - 1));
	SendMessage(GetDlgItem(hwndDlg, IDC_OK), WM_SETTEXT, 0, (LPARAM)status_text);
}

void SetEditControlText(struct MsgBoxData *data, HWND hwndDlg, int iStatus)
{
	int flags, fcursel = CB_ERR, num_start;
	DBVARIANT dbv, dbv2;
	char setting[80];

	num_start = SendMessage(data->recent_cbex, CB_GETCOUNT, 0, 0);
	num_start -= data->num_def_msgs + 1;

	mir_snprintf(setting, SIZEOF(setting), "%sFlags", data->m_szProto ? data->m_szProto : "");
	flags = db_get_b(NULL, "SimpleStatusMsg", (char *)StatusModeToDbSetting(iStatus, setting), STATUS_DEFAULT);

	if (flags & STATUS_LAST_MSG) {
		if (data->m_szProto)
			mir_snprintf(setting, SIZEOF(setting), "Last%sMsg", data->m_szProto);
		else
			mir_snprintf(setting, SIZEOF(setting), "LastMsg");

		if (!db_get(NULL, "SimpleStatusMsg", setting, &dbv)) {
			if (dbv.pszVal && strlen(dbv.pszVal)) {
				if (!db_get_ts(NULL, "SimpleStatusMsg", dbv.pszVal, &dbv2)) {
					if (dbv2.ptszVal && lstrlen(dbv2.ptszVal)) {
						SetDlgItemText(hwndDlg, IDC_EDIT1, dbv2.ptszVal);
						fcursel = SendMessage(data->recent_cbex, CB_FINDSTRINGEXACT, num_start, (LPARAM)dbv2.ptszVal);
						if (fcursel != CB_ERR)
							SendMessage(data->recent_cbex, CB_SETCURSEL, (WPARAM)fcursel, 0);
					}
					db_free(&dbv2);
				}
			}
			db_free(&dbv);
		}
	}
	else if (flags & STATUS_DEFAULT_MSG) {
		SetDlgItemText(hwndDlg, IDC_EDIT1, GetDefaultMessage(iStatus));

		if (db_get_b(NULL, "SimpleStatusMsg", "PutDefInList", 0)) {
			fcursel = SendMessage(data->recent_cbex, CB_FINDSTRINGEXACT, num_start, (LPARAM)GetDefaultMessage(iStatus));
			if (fcursel != CB_ERR)
				SendMessage(data->recent_cbex, CB_SETCURSEL, (WPARAM)fcursel, 0);
		}
	}
	else if (flags & STATUS_THIS_MSG) {
		if (data->m_szProto)
			mir_snprintf(setting, SIZEOF(setting), "%sDefault", data->m_szProto);
		else
			mir_snprintf(setting, SIZEOF(setting), "Default");

		if (!db_get_ts(NULL, "SRAway", StatusModeToDbSetting(iStatus, setting), &dbv)) {
			SetDlgItemText(hwndDlg, IDC_EDIT1, dbv.ptszVal);
			fcursel = SendMessage(data->recent_cbex, CB_FINDSTRINGEXACT, num_start, (LPARAM)dbv.ptszVal);
			if (fcursel != CB_ERR)
				SendMessage(data->recent_cbex, CB_SETCURSEL, (WPARAM)fcursel, 0);
			db_free(&dbv);
		}
	}
	else if (flags & STATUS_LAST_STATUS_MSG) {
		if (data->m_szProto)
			mir_snprintf(setting, SIZEOF(setting), "%sMsg", data->m_szProto);
		else
			mir_snprintf(setting, SIZEOF(setting), "Msg");

		if (!db_get_ts(NULL, "SRAway", StatusModeToDbSetting(iStatus, setting), &dbv)) {
			SetDlgItemText(hwndDlg, IDC_EDIT1, dbv.ptszVal);
			fcursel = SendMessage(data->recent_cbex, CB_FINDSTRINGEXACT, num_start, (LPARAM)dbv.ptszVal);
			if (fcursel != CB_ERR)
				SendMessage(data->recent_cbex, CB_SETCURSEL, (WPARAM)fcursel, 0);
			db_free(&dbv);
		}
	}

	if (fcursel != CB_ERR)
		data->curr_sel_msg = fcursel;
}

void ChangeDlgStatus(HWND hwndDlg, struct MsgBoxData *msgbox_data, int iStatus)
{
	TCHAR szTitle[256], szProtoName[128];
	BOOL bDisabled = msgbox_data->m_szProto && !(CallProtoService(msgbox_data->m_szProto, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_MODEMSGSEND);

	mir_sntprintf(szProtoName, SIZEOF(szProtoName), msgbox_data->m_szProto ? ProtoGetAccount(msgbox_data->m_szProto)->tszAccountName : TranslateT("global"));
	if (iStatus == ID_STATUS_CURRENT) {
		if (msgbox_data->m_bOnStartup)
			mir_sntprintf(szTitle, SIZEOF(szTitle), TranslateT("%s message (%s)"), TranslateT("<startup>"), szProtoName);
		else
			mir_sntprintf(szTitle, SIZEOF(szTitle), TranslateT("%s message (%s)"), TranslateT("<current>"), szProtoName);
	}
	else if (iStatus > ID_STATUS_CURRENT) {
		TCHAR buff[128];

		char buff1[128];
		CallService(MS_SS_GETPROFILENAME, iStatus - 40083, (LPARAM)buff1);
		MultiByteToWideChar(CallService(MS_LANGPACK_GETCODEPAGE, 0, 0), 0, buff1, -1, buff, 128);

		mir_sntprintf(szTitle, SIZEOF(szTitle), TranslateT("%s message (%s)"), (TCHAR *)buff, szProtoName);
	}
	else
		mir_sntprintf(szTitle, SIZEOF(szTitle), TranslateT("%s message (%s)"), pcli->pfnGetStatusModeDescription(iStatus, 0), szProtoName);
	SetWindowText(hwndDlg, szTitle);

	if (iStatus == ID_STATUS_CURRENT)
		iStatus = msgbox_data->m_bOnStartup ? GetStartupStatus(msgbox_data->m_szProto) : GetCurrentStatus(msgbox_data->m_szProto);
	else if (iStatus > ID_STATUS_CURRENT)
		iStatus = GetCurrentStatus(NULL);

	Skin_ReleaseIcon((HICON)SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM)LoadSkinnedProtoIcon(msgbox_data->m_szProto, iStatus)));
	Skin_ReleaseIcon((HICON)SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, (LPARAM)LoadSkinnedProtoIcon(msgbox_data->m_szProto, iStatus)));

	if (!bDisabled && ((Proto_Status2Flag(iStatus) & msgbox_data->m_iStatusMsgModes)
		|| (iStatus == ID_STATUS_OFFLINE && (Proto_Status2Flag(ID_STATUS_INVISIBLE) & msgbox_data->m_iStatusMsgModes))))
	{
		int num_items = SendMessage(msgbox_data->recent_cbex, CB_GETCOUNT, 0, 0);
		int fcursel = CB_ERR, num_start = num_items - msgbox_data->num_def_msgs - 1;
		TCHAR msg[1024];

		if (!IsWindowEnabled(GetDlgItem(hwndDlg, IDC_EDIT1)))
			EnableWindow(GetDlgItem(hwndDlg, IDC_EDIT1), TRUE);

		if (!IsWindowEnabled(msgbox_data->recent_cbex) && num_items)
			EnableWindow(msgbox_data->recent_cbex, TRUE);

		// TODO what if num_start <= 0 ?
		if (GetDlgItemText(hwndDlg, IDC_EDIT1, msg, SIZEOF(msg)))
			fcursel = SendMessage(msgbox_data->recent_cbex, CB_FINDSTRINGEXACT, num_start, (LPARAM)msg);
		if (fcursel != CB_ERR) {
			SendMessage(msgbox_data->recent_cbex, CB_SETCURSEL, fcursel, 0);
			msgbox_data->curr_sel_msg = fcursel;
		}

		if ((msgbox_data->m_iDlgFlags & DLG_SHOW_BUTTONS) || (msgbox_data->m_iDlgFlags & DLG_SHOW_BUTTONS_FLAT)) {
			if (!GetDlgItemText(hwndDlg, IDC_EDIT1, msg, SIZEOF(msg))) {
				if (IsWindowEnabled(GetDlgItem(hwndDlg, IDC_BADD)))
					EnableWindow(GetDlgItem(hwndDlg, IDC_BADD), FALSE);
			}
			else if (!IsWindowEnabled(GetDlgItem(hwndDlg, IDC_BADD)))
				EnableWindow(GetDlgItem(hwndDlg, IDC_BADD), TRUE);
			if (num_items) {
				if (msgbox_data->curr_sel_msg == -1) {
					if (IsWindowEnabled(GetDlgItem(hwndDlg, IDC_BDEL)))
						EnableWindow(GetDlgItem(hwndDlg, IDC_BDEL), FALSE);
				}
				else {
					COMBOBOXEXITEM cbitem = {0};
					cbitem.mask = CBEIF_LPARAM | CBEIF_TEXT;
					cbitem.iItem = msgbox_data->curr_sel_msg;
					cbitem.cchTextMax = SIZEOF(msg);
					cbitem.pszText = msg;
					SendMessage(msgbox_data->recent_cbex, CBEM_GETITEM, 0, (LPARAM)&cbitem);
					if (LOWORD(cbitem.lParam) == PREDEFINED_MSG) {
						if (IsWindowEnabled(GetDlgItem(hwndDlg, IDC_BADD)))
							EnableWindow(GetDlgItem(hwndDlg, IDC_BADD), FALSE);
					}
					else if (!IsWindowEnabled(GetDlgItem(hwndDlg, IDC_BADD)))
						EnableWindow(GetDlgItem(hwndDlg, IDC_BADD), TRUE);

					if (!IsWindowEnabled(GetDlgItem(hwndDlg, IDC_BDEL)))
						EnableWindow(GetDlgItem(hwndDlg, IDC_BDEL), TRUE);
				}

				if (msgbox_data->m_bIsMsgHistory && !IsWindowEnabled(GetDlgItem(hwndDlg, IDC_BCLEAR)))
						EnableWindow(GetDlgItem(hwndDlg, IDC_BCLEAR), TRUE);
			}
		}
	}
	else {
		if (IsWindowEnabled(GetDlgItem(hwndDlg, IDC_EDIT1)))
			EnableWindow(GetDlgItem(hwndDlg, IDC_EDIT1), FALSE);
		if (IsWindowEnabled(msgbox_data->recent_cbex))
			EnableWindow(msgbox_data->recent_cbex, FALSE);

		if ((msgbox_data->m_iDlgFlags & DLG_SHOW_BUTTONS) || (msgbox_data->m_iDlgFlags & DLG_SHOW_BUTTONS_FLAT)) {
			if (IsWindowEnabled(GetDlgItem(hwndDlg, IDC_BADD)))
				EnableWindow(GetDlgItem(hwndDlg, IDC_BADD), FALSE);
			if (IsWindowEnabled(GetDlgItem(hwndDlg, IDC_BCLEAR)))
				EnableWindow(GetDlgItem(hwndDlg, IDC_BCLEAR), FALSE);
			if (IsWindowEnabled(GetDlgItem(hwndDlg, IDC_BDEL)))
				EnableWindow(GetDlgItem(hwndDlg, IDC_BDEL), FALSE);
		}
	}
}

#define DM_SIMPAWAY_SHUTDOWN (WM_USER + 10)
#define DM_SIMPAWAY_CHANGEICONS (WM_USER + 11)

INT_PTR CALLBACK AwayMsgBoxDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	struct MsgBoxData *msgbox_data = (struct MsgBoxData *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (uMsg) {
		case WM_INITDIALOG:
		{
			TCHAR szTitle[256], szFormat[256], szProtoName[128];
			struct MsgBoxInitData *init_data;
			struct MsgBoxData *copy_init_data;
			INITCOMMONCONTROLSEX icex = {0};
			BOOL bCurrentStatus = FALSE, bDisabled = FALSE;

			InitCommonControls();
			icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
			icex.dwICC = ICC_USEREX_CLASSES;
			InitCommonControlsEx(&icex);

			TranslateDialogDefault(hwndDlg);
			init_data = (struct MsgBoxInitData *)lParam;
			GetWindowText(hwndDlg, szFormat, SIZEOF(szFormat));
			mir_sntprintf(szProtoName, SIZEOF(szProtoName), init_data->m_szProto ? ProtoGetAccount(init_data->m_szProto)->tszAccountName : TranslateT("global"));

			if (init_data->m_iStatus == ID_STATUS_CURRENT) {
				if (init_data->m_bOnStartup)
					mir_sntprintf(szTitle, SIZEOF(szTitle), szFormat, TranslateT("<startup>"), szProtoName);
				else
					mir_sntprintf(szTitle, SIZEOF(szTitle), szFormat, TranslateT("<current>"), szProtoName);
			}
			else
				mir_sntprintf(szTitle, SIZEOF(szTitle), szFormat, pcli->pfnGetStatusModeDescription(init_data->m_iStatus, 0), szProtoName);
			SetWindowText(hwndDlg, szTitle);

			int icoStatus = ID_STATUS_OFFLINE;
			if (init_data->m_iStatus == ID_STATUS_CURRENT)
				icoStatus = init_data->m_bOnStartup ? GetStartupStatus(init_data->m_szProto) : GetCurrentStatus(init_data->m_szProto);
			else
				icoStatus = init_data->m_iStatus;
			if (icoStatus < ID_STATUS_OFFLINE)
				icoStatus = ID_STATUS_OFFLINE;
			SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM)LoadSkinnedProtoIcon(init_data->m_szProto, icoStatus));
			SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, (LPARAM)LoadSkinnedProtoIcon(init_data->m_szProto, icoStatus));

			copy_init_data = (struct MsgBoxData *)mir_alloc(sizeof(struct MsgBoxData));

			SendDlgItemMessage(hwndDlg, IDC_EDIT1, EM_LIMITTEXT, 1024, 0);

			HookEventMessage(ME_SYSTEM_PRESHUTDOWN, hwndDlg, DM_SIMPAWAY_SHUTDOWN);
			HookEventMessage(ME_SKIN2_ICONSCHANGED, hwndDlg, DM_SIMPAWAY_CHANGEICONS);

			copy_init_data->num_def_msgs = db_get_w(NULL, "SimpleStatusMsg", "DefMsgCount", 0);
			copy_init_data->max_hist_msgs = db_get_b(NULL, "SimpleStatusMsg", "MaxHist", 10);
			copy_init_data->m_iDlgFlags = db_get_b(NULL, "SimpleStatusMsg", "DlgFlags", DLG_SHOW_DEFAULT);
			copy_init_data->m_szProto = init_data->m_szProto;
			copy_init_data->m_iStatus = init_data->m_iStatus;
			copy_init_data->m_iStatusModes = init_data->m_iStatusModes;
			copy_init_data->m_iStatusMsgModes = init_data->m_iStatusMsgModes;
			copy_init_data->m_iInitialStatus = init_data->m_iStatus;
			copy_init_data->m_bOnStartup = init_data->m_bOnStartup;

			//Load Icons
			copy_init_data->icon[I_ICON_DEL] = LoadIconEx("cross");
			copy_init_data->icon[I_ICON_HIST] = LoadIconEx("recent");
			copy_init_data->icon[I_ICON_MSG] = LoadIconEx("predef");
			copy_init_data->icon[I_ICON_ADD] = LoadIconEx("add");
			copy_init_data->icon[I_ICON_CLEAR] = LoadIconEx("clear");
			if (copy_init_data->m_iDlgFlags & DLG_SHOW_STATUS_ICONS)
				copy_init_data->status_icons = AddStatusIconsToImageList(init_data->m_szProto, copy_init_data->m_iStatusModes);
			if (copy_init_data->m_iDlgFlags & DLG_SHOW_LIST_ICONS)
				copy_init_data->other_icons = AddOtherIconsToImageList(copy_init_data);

			if ((copy_init_data->m_iDlgFlags & DLG_SHOW_BUTTONS) || (copy_init_data->m_iDlgFlags & DLG_SHOW_BUTTONS_FLAT)) {
				SendMessage(GetDlgItem(hwndDlg, IDC_BADD), BUTTONADDTOOLTIP, (WPARAM)Translate("Add to predefined"), 0);
				SendMessage(GetDlgItem(hwndDlg, IDC_BADD), BM_SETIMAGE, IMAGE_ICON, (LPARAM)copy_init_data->icon[I_ICON_ADD]);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BADD), TRUE);
				ShowWindow(GetDlgItem(hwndDlg, IDC_BADD), TRUE);

				SendMessage(GetDlgItem(hwndDlg, IDC_BDEL), BUTTONADDTOOLTIP, (WPARAM)Translate("Delete selected"), 0);
				SendMessage(GetDlgItem(hwndDlg, IDC_BDEL), BM_SETIMAGE, IMAGE_ICON, (LPARAM)copy_init_data->icon[I_ICON_DEL]);
				ShowWindow(GetDlgItem(hwndDlg, IDC_BDEL), TRUE);

				SendMessage(GetDlgItem(hwndDlg, IDC_BCLEAR), BUTTONADDTOOLTIP, (WPARAM)Translate("Clear history"), 0);
				SendMessage(GetDlgItem(hwndDlg, IDC_BCLEAR), BM_SETIMAGE, IMAGE_ICON, (LPARAM)copy_init_data->icon[I_ICON_CLEAR]);
				ShowWindow(GetDlgItem(hwndDlg, IDC_BCLEAR), TRUE);

				if (copy_init_data->m_iDlgFlags & DLG_SHOW_BUTTONS_FLAT) {
					SendMessage(GetDlgItem(hwndDlg, IDC_BADD), BUTTONSETASFLATBTN, TRUE, 0);
					SendMessage(GetDlgItem(hwndDlg, IDC_BDEL), BUTTONSETASFLATBTN, TRUE, 0);
					SendMessage(GetDlgItem(hwndDlg, IDC_BCLEAR), BUTTONSETASFLATBTN, TRUE, 0);
				}
			}
			else {
				SetWindowPos(GetDlgItem(hwndDlg, IDC_OK), NULL, 52, 115, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
				SetWindowPos(GetDlgItem(hwndDlg, IDC_CANCEL), NULL, 160, 115, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
				ShowWindow(GetDlgItem(hwndDlg, IDC_BADD), FALSE);
				ShowWindow(GetDlgItem(hwndDlg, IDC_BDEL), FALSE);
				ShowWindow(GetDlgItem(hwndDlg, IDC_BCLEAR), FALSE);
			}
			copy_init_data->status_cbex = CreateStatusComboBoxEx(hwndDlg, copy_init_data);

			if (copy_init_data->m_iStatus == ID_STATUS_CURRENT) {
				if (copy_init_data->m_bOnStartup)
					copy_init_data->m_iStatus = GetStartupStatus(copy_init_data->m_szProto);
				else
					copy_init_data->m_iStatus = GetCurrentStatus(copy_init_data->m_szProto);
				if (copy_init_data->m_szProto == NULL)
					bCurrentStatus = TRUE;
			}

			copy_init_data->recent_cbex = CreateRecentComboBoxEx(hwndDlg, copy_init_data);
			copy_init_data->curr_sel_msg = -1;
			copy_init_data->m_bPredefChanged = FALSE;

			SetEditControlText(copy_init_data, hwndDlg, copy_init_data->m_iStatus);
			if ((copy_init_data->m_iDlgFlags & DLG_SHOW_BUTTONS) || (copy_init_data->m_iDlgFlags & DLG_SHOW_BUTTONS_FLAT)) {
				TCHAR msg[1024];

				if (!GetDlgItemText(hwndDlg, IDC_EDIT1, msg, SIZEOF(msg)))
					EnableWindow(GetDlgItem(hwndDlg, IDC_BADD), FALSE);

				if (copy_init_data->curr_sel_msg == -1) {
					if (IsWindowEnabled(GetDlgItem(hwndDlg, IDC_BDEL)))
						EnableWindow(GetDlgItem(hwndDlg, IDC_BDEL), FALSE);
				}
				else {
					COMBOBOXEXITEM cbitem = {0};
					cbitem.mask = CBEIF_LPARAM | CBEIF_TEXT;
					cbitem.iItem = copy_init_data->curr_sel_msg;
					cbitem.cchTextMax = SIZEOF(msg);
					cbitem.pszText = msg;

					SendMessage(copy_init_data->recent_cbex, CBEM_GETITEM, 0, (LPARAM)&cbitem);
					if (LOWORD(cbitem.lParam) == PREDEFINED_MSG) {
						if (IsWindowEnabled(GetDlgItem(hwndDlg, IDC_BADD)))
							EnableWindow(GetDlgItem(hwndDlg, IDC_BADD), FALSE);
					}
					else if (!IsWindowEnabled(GetDlgItem(hwndDlg, IDC_BADD)))
						EnableWindow(GetDlgItem(hwndDlg, IDC_BADD), TRUE);
				}
			}

			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)copy_init_data);

			if (copy_init_data->m_szProto && !(CallProtoService(copy_init_data->m_szProto, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_MODEMSGSEND))
				bDisabled = TRUE;

			if (!(((Proto_Status2Flag(copy_init_data->m_iStatus) & copy_init_data->m_iStatusMsgModes)
				|| (copy_init_data->m_iStatus == ID_STATUS_OFFLINE && (Proto_Status2Flag(ID_STATUS_INVISIBLE) & copy_init_data->m_iStatusMsgModes))) && !bDisabled))
			{
				if (IsWindowEnabled(GetDlgItem(hwndDlg, IDC_EDIT1)))
					EnableWindow(GetDlgItem(hwndDlg, IDC_EDIT1), FALSE);
				if (IsWindowEnabled(copy_init_data->recent_cbex))
					EnableWindow(copy_init_data->recent_cbex, FALSE);

				if ((copy_init_data->m_iDlgFlags & DLG_SHOW_BUTTONS) || (copy_init_data->m_iDlgFlags & DLG_SHOW_BUTTONS_FLAT)) {
					if (IsWindowEnabled(GetDlgItem(hwndDlg, IDC_BADD)))
						EnableWindow(GetDlgItem(hwndDlg, IDC_BADD), FALSE);
					if (IsWindowEnabled(GetDlgItem(hwndDlg, IDC_BCLEAR)))
						EnableWindow(GetDlgItem(hwndDlg, IDC_BCLEAR), FALSE);
					if (IsWindowEnabled(GetDlgItem(hwndDlg, IDC_BDEL)))
						EnableWindow(GetDlgItem(hwndDlg, IDC_BDEL), FALSE);
				}
			}

			if (db_get_b(NULL, "SimpleStatusMsg", "AutoClose", 1) && init_data->m_bOnEvent) {
				copy_init_data->m_iCountdown = db_get_b(NULL, "SimpleStatusMsg", "DlgTime", 5);
				SendMessage(hwndDlg, WM_TIMER, 0, 0);
				SetTimer(hwndDlg, 1, 1000, 0);
			}
			else {
				copy_init_data->m_iCountdown = -2;
				DisplayCharsCount(copy_init_data, hwndDlg);
			}

			if (bCurrentStatus)
				copy_init_data->m_iStatus = ID_STATUS_CURRENT;

			mir_subclassWindow( GetDlgItem(hwndDlg, IDC_EDIT1), EditBoxSubProc);
			if (!init_data->m_bOnEvent && IsWindowEnabled(GetDlgItem(hwndDlg, IDC_EDIT1))) {
				SetFocus(GetDlgItem(hwndDlg, IDC_EDIT1));
				SendMessage(GetDlgItem(hwndDlg, IDC_EDIT1), EM_SETSEL, 0, -1);
			}
			else
				SetFocus(GetDlgItem(hwndDlg, IDC_OK));

			mir_free(init_data);

			if (!db_get_b(NULL, "SimpleStatusMsg", "WinCentered", 1)) {
				WINDOWPLACEMENT wp;
				int x, y;

				wp.length = sizeof(wp);
				GetWindowPlacement(hwndDlg, &wp);

				x = (int)db_get_dw(NULL, "SimpleStatusMsg", "Winx", -1);
				y = (int)db_get_dw(NULL, "SimpleStatusMsg", "Winy", -1);

				if (x !=- 1) {
					OffsetRect(&wp.rcNormalPosition, x - wp.rcNormalPosition.left, y - wp.rcNormalPosition.top);
					wp.flags = 0;
					SetWindowPlacement(hwndDlg, &wp);
				}
			}
			return FALSE;
		}

		case WM_TIMER:
			if (msgbox_data->m_iCountdown == -1) {
				SendMessage(hwndDlg, WM_COMMAND, (WPARAM)IDC_OK, 0);
				msgbox_data->m_iCountdown = -2;
				DisplayCharsCount(msgbox_data, hwndDlg);
				break;
			}
			else {
				TCHAR str[64];
				mir_sntprintf(str, SIZEOF(str), TranslateT("Closing in %d"), msgbox_data->m_iCountdown);
				SetDlgItemText(hwndDlg, IDC_OK, str);
			}
			msgbox_data->m_iCountdown--;
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_OK:
				{
					TCHAR tszMsg[1024];
					int iStatus, iMsgLen = 0, iProfileStatus = 0;
					BOOL bCurrentStatus = FALSE;

					if (msgbox_data->m_iStatus == ID_STATUS_CURRENT) {
						msgbox_data->m_iStatus = msgbox_data->m_bOnStartup ? GetStartupStatus(msgbox_data->m_szProto) : GetCurrentStatus(msgbox_data->m_szProto);
						if (msgbox_data->m_szProto == NULL)
							bCurrentStatus = TRUE;
					}
					else if (msgbox_data->m_iStatus >= ID_STATUS_CURRENT) {
						iProfileStatus = msgbox_data->m_iStatus;
						msgbox_data->m_iStatus = GetCurrentStatus(NULL);
					}

					if (IsWindowEnabled(GetDlgItem(hwndDlg, IDC_EDIT1)))
						iMsgLen = GetDlgItemText(hwndDlg, IDC_EDIT1, tszMsg, SIZEOF(tszMsg));

					if (iMsgLen == 0) {
						char szSetting[80];
						if (msgbox_data->m_szProto) {
							mir_snprintf(szSetting, SIZEOF(szSetting), "Last%sMsg", msgbox_data->m_szProto);
							db_set_s(NULL, "SimpleStatusMsg", szSetting, "");

							mir_snprintf(szSetting, SIZEOF(szSetting), "%sMsg", msgbox_data->m_szProto);
							db_set_ts(NULL, "SRAway", StatusModeToDbSetting(msgbox_data->m_iStatus, szSetting), _T(""));
						}
						else {
							db_set_s(NULL, "SimpleStatusMsg", "LastMsg", "");
							for (int j = 0; j < accounts->count; j++) {
								if (!IsAccountEnabled(accounts->pa[j]))
									continue;

								if (!CallProtoService(accounts->pa[j]->szModuleName, PS_GETCAPS, PFLAGNUM_3, 0))
									continue;

								if (db_get_b(NULL, accounts->pa[j]->szModuleName, "LockMainStatus", 0))
									continue;

								if (!(CallProtoService(accounts->pa[j]->szModuleName, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_MODEMSGSEND))
									continue;

								mir_snprintf(szSetting, SIZEOF(szSetting), "Last%sMsg", accounts->pa[j]->szModuleName);
								db_set_s(NULL, "SimpleStatusMsg", szSetting, "");

								mir_snprintf(szSetting, SIZEOF(szSetting), "%sMsg", accounts->pa[j]->szModuleName);
								iStatus = msgbox_data->m_bOnStartup ? GetStartupStatus(accounts->pa[j]->szModuleName) : GetCurrentStatus(accounts->pa[j]->szModuleName);
								db_set_ts(NULL, "SRAway", StatusModeToDbSetting(iStatus, szSetting), _T(""));
							}

							db_set_ts(NULL, "SRAway", StatusModeToDbSetting(msgbox_data->m_iStatus, "Msg"), _T("")); // for compatibility with some plugins
						}

						if (bCurrentStatus)
							SetStatusMessage(msgbox_data->m_szProto, msgbox_data->m_iInitialStatus, ID_STATUS_CURRENT, 0, msgbox_data->m_bOnStartup);
						else if (iProfileStatus != 0)
							SetStatusMessage(msgbox_data->m_szProto, msgbox_data->m_iInitialStatus, iProfileStatus, 0, FALSE);
						else
							SetStatusMessage(msgbox_data->m_szProto, msgbox_data->m_iInitialStatus, msgbox_data->m_iStatus, 0, msgbox_data->m_bOnStartup);
					}
					else {
						char buff[64], buff2[80];
						DBVARIANT dbv;
						BOOL found = FALSE;

						for (int i = 1; i <= msgbox_data->max_hist_msgs; i++) {
							mir_snprintf(buff, SIZEOF(buff), "SMsg%d", i);
							if (!db_get_ts(NULL, "SimpleStatusMsg", buff, &dbv)) {
								if (!lstrcmp(dbv.ptszVal, tszMsg)) {
									found = TRUE;
									if (msgbox_data->m_szProto) {
										mir_snprintf(buff2, SIZEOF(buff2), "Last%sMsg", msgbox_data->m_szProto);
										db_set_s(NULL, "SimpleStatusMsg", buff2, buff);

										mir_snprintf(buff2, SIZEOF(buff2), "%sMsg", msgbox_data->m_szProto);
										db_set_ts(NULL, "SRAway", StatusModeToDbSetting(msgbox_data->m_iStatus, buff2), tszMsg);
									}
									else {
										db_set_s(NULL, "SimpleStatusMsg", "LastMsg", buff);
										for (int j = 0; j < accounts->count; j++) {
											if (!IsAccountEnabled(accounts->pa[j]))
												continue;

											if (!CallProtoService(accounts->pa[j]->szModuleName, PS_GETCAPS, PFLAGNUM_3, 0))
												continue;

											if (db_get_b(NULL, accounts->pa[j]->szModuleName, "LockMainStatus", 0))
												continue;

											if (!(CallProtoService(accounts->pa[j]->szModuleName, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_MODEMSGSEND))
												continue;

											mir_snprintf(buff2, SIZEOF(buff2), "Last%sMsg", accounts->pa[j]->szModuleName);
											db_set_s(NULL, "SimpleStatusMsg", buff2, buff);

											mir_snprintf(buff2, SIZEOF(buff2), "%sMsg", accounts->pa[j]->szModuleName);
											iStatus = msgbox_data->m_bOnStartup ? GetStartupStatus(accounts->pa[j]->szModuleName) : GetCurrentStatus(accounts->pa[j]->szModuleName);
											db_set_ts(NULL, "SRAway", StatusModeToDbSetting(iStatus, buff2), tszMsg);
										}
									}
									db_free(&dbv);
									break;
								}
								db_free(&dbv);
							}
						}

						if (!found) {
							int last_modified_msg = db_get_w(NULL, "SimpleStatusMsg", "LMMsg", msgbox_data->max_hist_msgs);

							if (last_modified_msg == msgbox_data->max_hist_msgs)
								last_modified_msg = 1;
							else
								last_modified_msg++;

							mir_snprintf(buff, SIZEOF(buff), "SMsg%d", last_modified_msg);
							db_set_ts(NULL, "SimpleStatusMsg", buff, tszMsg);

							if (msgbox_data->m_szProto) {
								mir_snprintf(buff2, SIZEOF(buff2), "Last%sMsg", msgbox_data->m_szProto);
								db_set_s(NULL, "SimpleStatusMsg", buff2, buff);

								mir_snprintf(buff2, SIZEOF(buff2), "%sMsg", msgbox_data->m_szProto);
								db_set_ts(NULL, "SRAway", StatusModeToDbSetting(msgbox_data->m_iStatus, buff2), tszMsg);
							}
							else {
								db_set_s(NULL, "SimpleStatusMsg", "LastMsg", buff);
								for (int j = 0; j < accounts->count; j++) {
									if (!IsAccountEnabled(accounts->pa[j]))
										continue;

									if (!CallProtoService(accounts->pa[j]->szModuleName, PS_GETCAPS, PFLAGNUM_3, 0))
										continue;

									if (db_get_b(NULL, accounts->pa[j]->szModuleName, "LockMainStatus", 0))
										continue;

									if (!(CallProtoService(accounts->pa[j]->szModuleName, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_MODEMSGSEND))
										continue;

									mir_snprintf(buff2, SIZEOF(buff2), "Last%sMsg", accounts->pa[j]->szModuleName);
									db_set_s(NULL, "SimpleStatusMsg", buff2, buff);

									mir_snprintf(buff2, SIZEOF(buff2), "%sMsg", accounts->pa[j]->szModuleName);
									iStatus = msgbox_data->m_bOnStartup ? GetStartupStatus(accounts->pa[j]->szModuleName) : GetCurrentStatus(accounts->pa[j]->szModuleName);
									db_set_ts(NULL, "SRAway", StatusModeToDbSetting(iStatus, buff2), tszMsg);
								}
							}
							db_set_w(NULL, "SimpleStatusMsg", "LMMsg", (WORD)last_modified_msg);
						}

						if (!msgbox_data->m_szProto)
							db_set_ts(NULL, "SRAway", StatusModeToDbSetting(msgbox_data->m_iStatus, "Msg"), tszMsg); // for compatibility with some plugins

						if (bCurrentStatus)
							SetStatusMessage(msgbox_data->m_szProto, msgbox_data->m_iInitialStatus, ID_STATUS_CURRENT, tszMsg, msgbox_data->m_bOnStartup);
						else if (iProfileStatus != 0)
							SetStatusMessage(msgbox_data->m_szProto, msgbox_data->m_iInitialStatus, iProfileStatus, tszMsg, FALSE);
						else
							SetStatusMessage(msgbox_data->m_szProto, msgbox_data->m_iInitialStatus, msgbox_data->m_iStatus, tszMsg, msgbox_data->m_bOnStartup);
					}
				}

				case IDCANCEL:
				case IDC_CANCEL:
					DestroyWindow(hwndDlg);
					return TRUE;

				case IDC_EDIT1:		// Notification from the edit control
					if (msgbox_data->m_iCountdown > -2) {
						KillTimer(hwndDlg, 1);
						msgbox_data->m_iCountdown = -2;
						DisplayCharsCount(msgbox_data, hwndDlg);
					}
					switch (HIWORD(wParam)) {
						case EN_CHANGE:
							DisplayCharsCount(msgbox_data, hwndDlg);
							SendMessage(msgbox_data->recent_cbex, CB_SETCURSEL, -1, 0);
							if ((msgbox_data->m_iDlgFlags & DLG_SHOW_BUTTONS) || (msgbox_data->m_iDlgFlags & DLG_SHOW_BUTTONS_FLAT)) {
								TCHAR msg[1024];

								if (IsWindowEnabled(GetDlgItem(hwndDlg, IDC_BDEL)))
									EnableWindow(GetDlgItem(hwndDlg, IDC_BDEL), FALSE);

								if (!GetDlgItemText(hwndDlg, IDC_EDIT1, msg, SIZEOF(msg))) {
									if (IsWindowEnabled(GetDlgItem(hwndDlg, IDC_BADD)))
										EnableWindow(GetDlgItem(hwndDlg, IDC_BADD), FALSE);
								}
								else if (!IsWindowEnabled(GetDlgItem(hwndDlg, IDC_BADD)))
									EnableWindow(GetDlgItem(hwndDlg, IDC_BADD), TRUE);
							}
							break;
					}
					break;
			}

			if ((HWND)lParam == msgbox_data->status_cbex) {
				if (msgbox_data->m_iCountdown > -2) {
					KillTimer(hwndDlg, 1);
					msgbox_data->m_iCountdown = -2;
					DisplayCharsCount(msgbox_data, hwndDlg);
				}
				switch (HIWORD(wParam)) {
					case CBN_SELENDOK:
					case CBN_SELCHANGE:
					{
						COMBOBOXEXITEM cbitem = {0};

						cbitem.mask = CBEIF_LPARAM;
						cbitem.iItem = SendMessage(msgbox_data->status_cbex, CB_GETCURSEL, 0, 0);
						SendMessage(msgbox_data->status_cbex, CBEM_GETITEM, 0, (LPARAM)&cbitem);

						msgbox_data->m_iStatus = cbitem.lParam;
						ChangeDlgStatus(hwndDlg, msgbox_data, (int)cbitem.lParam);

						if (HIWORD(wParam) == CBN_SELENDOK && IsWindowEnabled(GetDlgItem(hwndDlg, IDC_EDIT1)))
							SetFocus(GetDlgItem(hwndDlg, IDC_EDIT1));
						break;
					}
				}
			}

			if ((HWND)lParam == msgbox_data->recent_cbex) {
				if (msgbox_data->m_iCountdown > -2) {
					KillTimer(hwndDlg, 1);
					msgbox_data->m_iCountdown = -2;
					DisplayCharsCount(msgbox_data, hwndDlg);
				}
				switch (HIWORD(wParam)) {
					case CBN_SELENDOK:
					{
						TCHAR text[1024];
						int cur_sel = SendMessage(msgbox_data->recent_cbex, CB_GETCURSEL, 0, 0);
						COMBOBOXEXITEM cbitem = {0};

						cbitem.mask = CBEIF_LPARAM | CBEIF_TEXT;
						cbitem.iItem = cur_sel;
						cbitem.cchTextMax = SIZEOF(text);
						cbitem.pszText = text;

						SendMessage(msgbox_data->recent_cbex, CBEM_GETITEM, 0, (LPARAM)&cbitem);
						if (LOWORD(cbitem.lParam) == HISTORY_MSG || LOWORD(cbitem.lParam) == PREDEFINED_MSG || LOWORD(cbitem.lParam) == DEFAULT_MSG)
						{
							SetDlgItemText(hwndDlg, IDC_EDIT1, text);
							DisplayCharsCount(msgbox_data, hwndDlg);
							if ((msgbox_data->m_iDlgFlags & DLG_SHOW_BUTTONS) || (msgbox_data->m_iDlgFlags & DLG_SHOW_BUTTONS_FLAT)) {
								if (!IsWindowEnabled(GetDlgItem(hwndDlg, IDC_BDEL)))
									EnableWindow(GetDlgItem(hwndDlg, IDC_BDEL), TRUE);
								if (LOWORD(cbitem.lParam) == PREDEFINED_MSG) {
									if (IsWindowEnabled(GetDlgItem(hwndDlg, IDC_BADD)))
										EnableWindow(GetDlgItem(hwndDlg, IDC_BADD), FALSE);
								}
								else if (!IsWindowEnabled(GetDlgItem(hwndDlg, IDC_BADD)))
									EnableWindow(GetDlgItem(hwndDlg, IDC_BADD), TRUE);
							}
						}
						else if (LOWORD(cbitem.lParam) == CLEAR_HISTORY) {
							if (MessageBox(NULL, TranslateT("Are you sure you want to clear status message history?"), TranslateT("Confirm clearing history"), MB_ICONQUESTION | MB_YESNO) == IDYES)
								ClearHistory(msgbox_data, cur_sel);
							else if (IsWindowEnabled(GetDlgItem(hwndDlg, IDC_EDIT1))) {
								TCHAR msg[1024];
								int fcursel = CB_ERR, num_start;
								num_start = SendMessage(msgbox_data->recent_cbex, CB_GETCOUNT, 0, 0);
								num_start -= msgbox_data->num_def_msgs + 1;
								GetDlgItemText(hwndDlg, IDC_EDIT1, msg, SIZEOF(msg));
								fcursel = SendMessage(msgbox_data->recent_cbex, CB_FINDSTRINGEXACT, num_start, (LPARAM)msg);
								SendMessage(msgbox_data->recent_cbex, CB_SETCURSEL, fcursel, 0);
							}
						}
						else if (LOWORD(cbitem.lParam) == DELETE_SELECTED) {
							COMBOBOXEXITEM histitem = {0};
							BOOL scursel = FALSE;

							histitem.mask = CBEIF_LPARAM;
							histitem.iItem = msgbox_data->curr_sel_msg;
							SendMessage(msgbox_data->recent_cbex, CBEM_GETITEM, 0, (LPARAM)&histitem);

							if (LOWORD(histitem.lParam) == HISTORY_MSG) {
								char szSetting[16];
								mir_snprintf(szSetting, SIZEOF(szSetting), "SMsg%d", (int)HIWORD(histitem.lParam));
								db_set_ts(NULL, "SimpleStatusMsg", szSetting, _T(""));
								SendMessage(msgbox_data->recent_cbex, CBEM_DELETEITEM, (WPARAM)msgbox_data->curr_sel_msg, 0);
							}
							if (LOWORD(histitem.lParam) == PREDEFINED_MSG) {
								msgbox_data->m_bPredefChanged = TRUE;
								SendMessage(msgbox_data->recent_cbex, CBEM_DELETEITEM, (WPARAM)msgbox_data->curr_sel_msg, 0);
							}

							cur_sel = msgbox_data->curr_sel_msg;
							while (!scursel) {
								if (cur_sel - 1 >= 0)
									cur_sel--;
								else {
									scursel = TRUE;
									break;
								}
								histitem.mask = CBEIF_LPARAM;
								histitem.iItem = cur_sel;
								SendMessage(msgbox_data->recent_cbex, CBEM_GETITEM, 0, (LPARAM)&histitem);

								if ((LOWORD(histitem.lParam) != CLEAR_HISTORY) && (LOWORD(histitem.lParam) != DELETE_SELECTED) && (LOWORD(histitem.lParam) != ADD_MSG))
									scursel = TRUE;
							}
							msgbox_data->curr_sel_msg = cur_sel;
							SendMessage(msgbox_data->recent_cbex, CB_SETCURSEL, (WPARAM)cur_sel, 0);

							histitem.mask = CBEIF_LPARAM | CBEIF_TEXT;
							histitem.iItem = cur_sel;
							histitem.cchTextMax = SIZEOF(text);
							histitem.pszText = text;

							SendMessage(msgbox_data->recent_cbex, CBEM_GETITEM, 0, (LPARAM)&histitem);
							if (LOWORD(histitem.lParam) == HISTORY_MSG || LOWORD(histitem.lParam) == PREDEFINED_MSG || LOWORD(histitem.lParam) == DEFAULT_MSG)
							{
								SetDlgItemText(hwndDlg, IDC_EDIT1, text);
								DisplayCharsCount(msgbox_data, hwndDlg);
							}
						}
						else if (LOWORD(cbitem.lParam) == ADD_MSG) {
							int sel = AddToPredefined(hwndDlg, msgbox_data);
							if (sel != -1) {
								SendMessage(msgbox_data->recent_cbex, CB_SETCURSEL, (WPARAM)sel, 0);
								msgbox_data->curr_sel_msg = sel;
							}
							else
								SendMessage(msgbox_data->recent_cbex, CB_SETCURSEL, (WPARAM)msgbox_data->curr_sel_msg, 0);
							break;
						}
						msgbox_data->curr_sel_msg = cur_sel;

						if (IsWindowEnabled(GetDlgItem(hwndDlg, IDC_EDIT1))) {
							SetFocus(GetDlgItem(hwndDlg, IDC_EDIT1));
							SendMessage(GetDlgItem(hwndDlg, IDC_EDIT1), EM_SETSEL, 0, -1);
						}
						break;
					}
				}
			}

			if ((HWND)lParam == GetDlgItem(hwndDlg, IDC_BADD)) {
				switch (HIWORD(wParam)) {
					case BN_CLICKED:
					{
						int sel = AddToPredefined(hwndDlg, msgbox_data);
						if (sel != -1) {
							if (!IsWindowEnabled(msgbox_data->recent_cbex))
								EnableWindow(msgbox_data->recent_cbex, TRUE);
							if (!IsWindowEnabled(GetDlgItem(hwndDlg, IDC_BDEL)))
								EnableWindow(GetDlgItem(hwndDlg, IDC_BDEL), TRUE);
							if (IsWindowEnabled(GetDlgItem(hwndDlg, IDC_BADD)))
								EnableWindow(GetDlgItem(hwndDlg, IDC_BADD), FALSE);

							SendMessage(msgbox_data->recent_cbex, CB_SETCURSEL, (WPARAM)sel, 0);
							msgbox_data->curr_sel_msg = sel;
						}
						break;
					}
				}
			}

			if ((HWND)lParam == GetDlgItem(hwndDlg, IDC_BCLEAR)) {
				switch (HIWORD(wParam)) {
					case BN_CLICKED:
						if (MessageBox(NULL, TranslateT("Are you sure you want to clear status message history?"), TranslateT("Confirm clearing history"), MB_ICONQUESTION | MB_YESNO) == IDYES)
						{
							ClearHistory(msgbox_data, 0);

							int num_items = SendMessage(msgbox_data->recent_cbex, CB_GETCOUNT, 0, 0);
							if (!num_items) {
								if (IsWindowEnabled(msgbox_data->recent_cbex)) {
									EnableWindow(GetDlgItem(hwndDlg, IDC_BDEL), FALSE);
									EnableWindow(msgbox_data->recent_cbex, FALSE);
								}
							}
							EnableWindow(GetDlgItem(hwndDlg, IDC_BCLEAR), FALSE);
						}
						break;
				}
			}

			if ((HWND)lParam == GetDlgItem(hwndDlg, IDC_BDEL)) {
				switch (HIWORD(wParam)) {
					case BN_CLICKED:
					{
						int cur_sel;
						char buff[16];
						int left_items = 0;
						COMBOBOXEXITEM histitem = {0};

						cur_sel = SendMessage(msgbox_data->recent_cbex, CB_GETCURSEL, 0, 0);

						histitem.mask = CBEIF_LPARAM;
						histitem.iItem = msgbox_data->curr_sel_msg;

						SendMessage(msgbox_data->recent_cbex, CBEM_GETITEM, 0, (LPARAM)&histitem);

						if (LOWORD(histitem.lParam) == HISTORY_MSG) {
							mir_snprintf(buff, SIZEOF(buff), "SMsg%d", (int)HIWORD(histitem.lParam));
							db_set_ts(NULL, "SimpleStatusMsg", buff, _T(""));
						}
						else if (LOWORD(histitem.lParam) == PREDEFINED_MSG)
							msgbox_data->m_bPredefChanged = TRUE;
						left_items = SendMessage(msgbox_data->recent_cbex, CBEM_DELETEITEM, (WPARAM)msgbox_data->curr_sel_msg, 0);

						if (!left_items) {
							if (IsWindowEnabled(msgbox_data->recent_cbex))
								EnableWindow(msgbox_data->recent_cbex, FALSE);
							if (IsWindowEnabled(GetDlgItem(hwndDlg, IDC_BCLEAR)))
								EnableWindow(GetDlgItem(hwndDlg, IDC_BCLEAR), FALSE);
							EnableWindow(GetDlgItem(hwndDlg, IDC_BDEL), FALSE);
						}
						else {
							TCHAR text[1024];

							if (cur_sel - 1 >= 0)
								cur_sel--;
							msgbox_data->curr_sel_msg = cur_sel;
							SendMessage(msgbox_data->recent_cbex, CB_SETCURSEL, (WPARAM)cur_sel, 0);

							histitem.mask = CBEIF_LPARAM | CBEIF_TEXT;
							histitem.iItem = cur_sel;
							histitem.cchTextMax = SIZEOF(text);
							histitem.pszText = text;

							SendMessage(msgbox_data->recent_cbex, CBEM_GETITEM, 0, (LPARAM)&histitem);
							if (LOWORD(histitem.lParam) == HISTORY_MSG || LOWORD(histitem.lParam) == PREDEFINED_MSG || LOWORD(histitem.lParam) == DEFAULT_MSG)
							{
								SetDlgItemText(hwndDlg, IDC_EDIT1, text);
								DisplayCharsCount(msgbox_data, hwndDlg);
								EnableWindow(GetDlgItem(hwndDlg, IDC_BADD), LOWORD(histitem.lParam) == PREDEFINED_MSG ? FALSE : TRUE);
							}
						}
						break;
					}
				}
			}
			break;

		case DM_SIMPAWAY_SHUTDOWN:
			DestroyWindow(hwndDlg);
			break;

		case DM_SIMPAWAY_CHANGEICONS:
			ReleaseIconEx("cross");
			ReleaseIconEx("recent");
			ReleaseIconEx("predef");
			ReleaseIconEx("add");
			ReleaseIconEx("clear");
			msgbox_data->icon[I_ICON_DEL] = LoadIconEx("cross");
			msgbox_data->icon[I_ICON_HIST] = LoadIconEx("recent");
			msgbox_data->icon[I_ICON_MSG] = LoadIconEx("predef");
			msgbox_data->icon[I_ICON_ADD] = LoadIconEx("add");
			msgbox_data->icon[I_ICON_CLEAR] = LoadIconEx("clear");
			if (msgbox_data->m_iDlgFlags & DLG_SHOW_LIST_ICONS) {
				for (int i = 0; i < 5; ++i)
					ImageList_ReplaceIcon(msgbox_data->other_icons, i, msgbox_data->icon[i]);
			}
			if ((msgbox_data->m_iDlgFlags & DLG_SHOW_BUTTONS) || (msgbox_data->m_iDlgFlags & DLG_SHOW_BUTTONS_FLAT)) {
				SendMessage(GetDlgItem(hwndDlg, IDC_BADD), BM_SETIMAGE, IMAGE_ICON, (LPARAM)msgbox_data->icon[I_ICON_ADD]);
				SendMessage(GetDlgItem(hwndDlg, IDC_BCLEAR), BM_SETIMAGE, IMAGE_ICON, (LPARAM)msgbox_data->icon[I_ICON_CLEAR]);
				SendMessage(GetDlgItem(hwndDlg, IDC_BDEL), BM_SETIMAGE, IMAGE_ICON, (LPARAM)msgbox_data->icon[I_ICON_DEL]);
			}
			break;

		case WM_DESTROY:
			if (msgbox_data == NULL)
				break;
		{
			WINDOWPLACEMENT wp;
			wp.length = sizeof(wp);
			GetWindowPlacement(hwndDlg, &wp);
			db_set_dw(NULL, "SimpleStatusMsg", "Winx", wp.rcNormalPosition.left);
			db_set_dw(NULL, "SimpleStatusMsg", "Winy", wp.rcNormalPosition.top);

			if (msgbox_data->m_bPredefChanged) {
				int i, num_items, new_num_def_msgs = 0;
				COMBOBOXEXITEM cbitem = {0};
				TCHAR text[1024];
				char buff[64];

				num_items = SendMessage(msgbox_data->recent_cbex, CB_GETCOUNT, 0, 0);
				num_items--;
				for (i = 1; i <= msgbox_data->num_def_msgs; i++) {
					cbitem.mask = CBEIF_LPARAM | CBEIF_TEXT;
					cbitem.iItem = num_items;
					cbitem.cchTextMax = SIZEOF(text);
					cbitem.pszText = text;

					SendMessage(msgbox_data->recent_cbex, CBEM_GETITEM, 0, (LPARAM)&cbitem);
					mir_snprintf(buff, SIZEOF(buff), "DefMsg%d", i);
					if (LOWORD(cbitem.lParam) == PREDEFINED_MSG) {
						new_num_def_msgs++;
						db_set_ts(NULL, "SimpleStatusMsg", buff, text);
					}
					else
						db_unset(NULL, "SimpleStatusMsg", buff);
					num_items--;
				}
				db_set_w(NULL, "SimpleStatusMsg", "DefMsgCount", (WORD)new_num_def_msgs);
			}

			ImageList_Destroy(msgbox_data->status_icons);
			ImageList_Destroy(msgbox_data->other_icons);
			ReleaseIconEx("cross");
			ReleaseIconEx("recent");
			ReleaseIconEx("predef");
			ReleaseIconEx("add");
			ReleaseIconEx("clear");
			Skin_ReleaseIcon((HICON)SendMessage(hwndDlg, WM_SETICON, ICON_BIG, 0));
			Skin_ReleaseIcon((HICON)SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, 0));

			hwndSAMsgDialog = NULL;
			mir_free(msgbox_data);
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, 0);
			break;
		}
	}
	return FALSE;
}
