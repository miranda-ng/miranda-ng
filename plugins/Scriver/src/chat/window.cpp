/*
Chat module plugin for Miranda IM

Copyright (C) 2003 Jörgen Persson
Copyright 2003-2009 Miranda ICQ/IM project,

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

#include "../stdafx.h"

struct MESSAGESUBDATA
{
	time_t lastEnterTime;
	wchar_t *szSearchQuery;
	wchar_t *szSearchResult;
	SESSION_INFO *lastSession;
};

static LRESULT CALLBACK SplitterSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	RECT rc;

	switch (msg) {
	case WM_NCHITTEST:
		return HTCLIENT;

	case WM_SETCURSOR:
		GetClientRect(hwnd, &rc);
		SetCursor(rc.right > rc.bottom ? hCurSplitNS : hCurSplitWE);
		return TRUE;

	case WM_LBUTTONDOWN:
		SetCapture(hwnd);
		return 0;

	case WM_MOUSEMOVE:
		if (GetCapture() == hwnd) {
			GetClientRect(hwnd, &rc);
			SendMessage(GetParent(hwnd), GC_SPLITTERMOVED, rc.right > rc.bottom ? (short)HIWORD(GetMessagePos()) + rc.bottom / 2 : (short)LOWORD(GetMessagePos()) + rc.right / 2, (LPARAM)hwnd);
		}
		return 0;

	case WM_LBUTTONUP:
		ReleaseCapture();
		PostMessage(GetParent(hwnd), WM_SIZE, 0, 0);
		return 0;
	}
	return mir_callNextSubclass(hwnd, SplitterSubclassProc, msg, wParam, lParam);
}

static void InitButtons(HWND hwndDlg, SESSION_INFO *si)
{
	SendDlgItemMessage(hwndDlg, IDC_CHAT_SHOWNICKLIST, BM_SETIMAGE, IMAGE_ICON, (LPARAM)GetCachedIcon(si->bNicklistEnabled ? "chat_nicklist" : "chat_nicklist2"));
	SendDlgItemMessage(hwndDlg, IDC_CHAT_FILTER, BM_SETIMAGE, IMAGE_ICON, (LPARAM)GetCachedIcon(si->bFilterEnabled ? "chat_filter" : "chat_filter2"));

	MODULEINFO *pInfo = pci->MM_FindModule(si->pszModule);
	if (pInfo) {
		EnableWindow(GetDlgItem(hwndDlg, IDC_CHAT_BOLD), pInfo->bBold);
		EnableWindow(GetDlgItem(hwndDlg, IDC_CHAT_ITALICS), pInfo->bItalics);
		EnableWindow(GetDlgItem(hwndDlg, IDC_CHAT_UNDERLINE), pInfo->bUnderline);
		EnableWindow(GetDlgItem(hwndDlg, IDC_CHAT_COLOR), pInfo->bColor);
		EnableWindow(GetDlgItem(hwndDlg, IDC_CHAT_BKGCOLOR), pInfo->bBkgColor);
		if (si->iType == GCW_CHATROOM)
			EnableWindow(GetDlgItem(hwndDlg, IDC_CHAT_CHANMGR), pInfo->bChanMgr);
	}
}

static void TabAutoComplete(HWND hwnd, MESSAGESUBDATA *dat, SESSION_INFO *si)
{
	LRESULT lResult = (LRESULT)SendMessage(hwnd, EM_GETSEL, 0, 0);
	int start = LOWORD(lResult), end = HIWORD(lResult);
	SendMessage(hwnd, EM_SETSEL, end, end);

	GETTEXTEX gt = { 0 };
	gt.codepage = 1200;
	int iLen = GetRichTextLength(hwnd, gt.codepage, TRUE);
	if (iLen <= 0)
		return;

	bool isTopic = false, isRoom = false;
	wchar_t *pszName = NULL;
	wchar_t* pszText = (wchar_t*)mir_alloc(iLen + 100 * sizeof(wchar_t));
	gt.cb = iLen + 99 * sizeof(wchar_t);
	gt.flags = GT_DEFAULT;

	SendMessage(hwnd, EM_GETTEXTEX, (WPARAM)&gt, (LPARAM)pszText);
	if (start > 1 && pszText[start - 1] == ' ' && pszText[start - 2] == ':')
		start -= 2;

	if (dat->szSearchResult != NULL) {
		int cbResult = (int)mir_wstrlen(dat->szSearchResult);
		if (start >= cbResult && !wcsnicmp(dat->szSearchResult, pszText + start - cbResult, cbResult)) {
			start -= cbResult;
			goto LBL_SkipEnd;
		}
	}

	while (start > 0 && pszText[start - 1] != ' ' && pszText[start - 1] != 13 && pszText[start - 1] != VK_TAB)
		start--;

LBL_SkipEnd:
	while (end < iLen && pszText[end] != ' ' && pszText[end] != 13 && pszText[end - 1] != VK_TAB)
		end++;

	if (pszText[start] == '#')
		isRoom = true;
	else {
		int topicStart = start;
		while (topicStart >0 && (pszText[topicStart - 1] == ' ' || pszText[topicStart - 1] == 13 || pszText[topicStart - 1] == VK_TAB))
			topicStart--;
		if (topicStart > 5 && wcsstr(&pszText[topicStart - 6], L"/topic") == &pszText[topicStart - 6])
			isTopic = true;
	}

	if (dat->szSearchQuery == NULL) {
		dat->szSearchQuery = (wchar_t*)mir_alloc(sizeof(wchar_t)*(end - start + 1));
		mir_wstrncpy(dat->szSearchQuery, pszText + start, end - start + 1);
		dat->szSearchResult = mir_wstrdup(dat->szSearchQuery);
		dat->lastSession = NULL;
	}

	if (isTopic)
		pszName = si->ptszTopic;
	else if (isRoom) {
		dat->lastSession = SM_FindSessionAutoComplete(si->pszModule, si, dat->lastSession, dat->szSearchQuery, dat->szSearchResult);
		if (dat->lastSession != NULL)
			pszName = dat->lastSession->ptszName;
	}
	else pszName = pci->UM_FindUserAutoComplete(si->pUsers, dat->szSearchQuery, dat->szSearchResult);

	mir_free(pszText);
	replaceStrW(dat->szSearchResult, NULL);

	if (pszName == NULL) {
		if (end != start) {
			SendMessage(hwnd, EM_SETSEL, start, end);
			SendMessage(hwnd, EM_REPLACESEL, FALSE, (LPARAM)dat->szSearchQuery);
		}
		replaceStrW(dat->szSearchQuery, NULL);
	}
	else {
		dat->szSearchResult = mir_wstrdup(pszName);
		if (end != start) {
			ptrW szReplace;
			if (!isRoom && !isTopic && g_Settings.bAddColonToAutoComplete && start == 0) {
				szReplace = (wchar_t*)mir_alloc((mir_wstrlen(pszName) + 4) * sizeof(wchar_t));
				mir_wstrcpy(szReplace, pszName);
				mir_wstrcat(szReplace, L": ");
				pszName = szReplace;
			}
			SendMessage(hwnd, EM_SETSEL, start, end);
			SendMessage(hwnd, EM_REPLACESEL, FALSE, (LPARAM)pszName);
		}
	}
}

static LRESULT CALLBACK MessageSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	BOOL isShift = GetKeyState(VK_SHIFT) & 0x8000;
	BOOL isCtrl = GetKeyState(VK_CONTROL) & 0x8000;
	BOOL isAlt = GetKeyState(VK_MENU) & 0x8000;

	SESSION_INFO *si = (SESSION_INFO*)GetWindowLongPtr(GetParent(hwnd), GWLP_USERDATA);
	MESSAGESUBDATA *dat = (MESSAGESUBDATA *)GetWindowLongPtr(hwnd, GWLP_USERDATA);

	int result = InputAreaShortcuts(hwnd, msg, wParam, lParam, si);
	if (result != -1)
		return result;

	switch (msg) {
	case EM_SUBCLASSED:
		dat = (MESSAGESUBDATA*)mir_calloc(sizeof(MESSAGESUBDATA));
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)dat);
		return 0;

	case WM_MOUSEWHEEL:
		if ((GetWindowLongPtr(hwnd, GWL_STYLE) & WS_VSCROLL) == 0)
			SendDlgItemMessage(GetParent(hwnd), IDC_LOG, WM_MOUSEWHEEL, wParam, lParam);

		dat->lastEnterTime = 0;
		return TRUE;

	case EM_REPLACESEL:
		PostMessage(hwnd, EM_ACTIVATE, 0, 0);
		break;

	case EM_ACTIVATE:
		SetActiveWindow(GetParent(hwnd));
		break;

	case WM_KEYDOWN:
		if (wParam == VK_RETURN) {
			mir_free(dat->szSearchQuery);
			dat->szSearchQuery = NULL;
			mir_free(dat->szSearchResult);
			dat->szSearchResult = NULL;
			if ((isCtrl != 0) ^ (0 != db_get_b(NULL, SRMMMOD, SRMSGSET_SENDONENTER, SRMSGDEFSET_SENDONENTER))) {
				PostMessage(GetParent(hwnd), WM_COMMAND, IDOK, 0);
				return 0;
			}
			if (db_get_b(NULL, SRMMMOD, SRMSGSET_SENDONDBLENTER, SRMSGDEFSET_SENDONDBLENTER)) {
				if (dat->lastEnterTime + 2 < time(NULL))
					dat->lastEnterTime = time(NULL);
				else {
					SendMessage(hwnd, WM_KEYDOWN, VK_BACK, 0);
					SendMessage(hwnd, WM_KEYUP, VK_BACK, 0);
					PostMessage(GetParent(hwnd), WM_COMMAND, IDOK, 0);
					return 0;
				}
			}
		}
		else dat->lastEnterTime = 0;

		if (wParam == VK_TAB && isShift && !isCtrl) { // SHIFT-TAB (go to nick list)
			SetFocus(GetDlgItem(GetParent(hwnd), IDC_CHAT_LIST));
			return TRUE;
		}

		if (wParam == VK_TAB && !isCtrl && !isShift) {    //tab-autocomplete
			SendMessage(hwnd, WM_SETREDRAW, FALSE, 0);
			TabAutoComplete(hwnd, dat, si);
			SendMessage(hwnd, WM_SETREDRAW, TRUE, 0);
			RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE);
			return 0;
		}
		if (wParam != VK_RIGHT && wParam != VK_LEFT) {
			mir_free(dat->szSearchQuery);
			dat->szSearchQuery = NULL;
			mir_free(dat->szSearchResult);
			dat->szSearchResult = NULL;
		}
		if (wParam == 0x49 && isCtrl && !isAlt) { // ctrl-i (italics)
			CheckDlgButton(GetParent(hwnd), IDC_CHAT_ITALICS, IsDlgButtonChecked(GetParent(hwnd), IDC_CHAT_ITALICS) == BST_UNCHECKED ? BST_CHECKED : BST_UNCHECKED);
			SendMessage(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(IDC_CHAT_ITALICS, 0), 0);
			return TRUE;
		}

		if (wParam == 0x42 && isCtrl && !isAlt) { // ctrl-b (bold)
			CheckDlgButton(GetParent(hwnd), IDC_CHAT_BOLD, IsDlgButtonChecked(GetParent(hwnd), IDC_CHAT_BOLD) == BST_UNCHECKED ? BST_CHECKED : BST_UNCHECKED);
			SendMessage(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(IDC_CHAT_BOLD, 0), 0);
			return TRUE;
		}

		if (wParam == 0x55 && isCtrl && !isAlt) { // ctrl-u (paste clean text)
			CheckDlgButton(GetParent(hwnd), IDC_CHAT_UNDERLINE, IsDlgButtonChecked(GetParent(hwnd), IDC_CHAT_UNDERLINE) == BST_UNCHECKED ? BST_CHECKED : BST_UNCHECKED);
			SendMessage(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(IDC_CHAT_UNDERLINE, 0), 0);
			return TRUE;
		}

		if (wParam == 0x4b && isCtrl && !isAlt) { // ctrl-k (paste clean text)
			CheckDlgButton(GetParent(hwnd), IDC_CHAT_COLOR, IsDlgButtonChecked(GetParent(hwnd), IDC_CHAT_COLOR) == BST_UNCHECKED ? BST_CHECKED : BST_UNCHECKED);
			SendMessage(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(IDC_CHAT_COLOR, 0), 0);
			return TRUE;
		}

		if (wParam == VK_SPACE && isCtrl && !isAlt) { // ctrl-space (paste clean text)
			CheckDlgButton(GetParent(hwnd), IDC_CHAT_BKGCOLOR, BST_UNCHECKED);
			CheckDlgButton(GetParent(hwnd), IDC_CHAT_COLOR, BST_UNCHECKED);
			CheckDlgButton(GetParent(hwnd), IDC_CHAT_BOLD, BST_UNCHECKED);
			CheckDlgButton(GetParent(hwnd), IDC_CHAT_UNDERLINE, BST_UNCHECKED);
			CheckDlgButton(GetParent(hwnd), IDC_CHAT_ITALICS, BST_UNCHECKED);
			SendMessage(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(IDC_CHAT_BKGCOLOR, 0), 0);
			SendMessage(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(IDC_CHAT_COLOR, 0), 0);
			SendMessage(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(IDC_CHAT_BOLD, 0), 0);
			SendMessage(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(IDC_CHAT_UNDERLINE, 0), 0);
			SendMessage(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(IDC_CHAT_ITALICS, 0), 0);
			return TRUE;
		}

		if (wParam == 0x4c && isCtrl && !isAlt) { // ctrl-l (paste clean text)
			CheckDlgButton(GetParent(hwnd), IDC_CHAT_BKGCOLOR, IsDlgButtonChecked(GetParent(hwnd), IDC_CHAT_BKGCOLOR) == BST_UNCHECKED ? BST_CHECKED : BST_UNCHECKED);
			SendMessage(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(IDC_CHAT_BKGCOLOR, 0), 0);
			return TRUE;
		}

		if (wParam == 0x46 && isCtrl && !isAlt) { // ctrl-f (paste clean text)
			if (IsWindowEnabled(GetDlgItem(GetParent(hwnd), IDC_CHAT_FILTER)))
				SendMessage(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(IDC_CHAT_FILTER, 0), 0);
			return TRUE;
		}

		if (wParam == 0x4e && isCtrl && !isAlt) { // ctrl-n (nicklist)
			if (IsWindowEnabled(GetDlgItem(GetParent(hwnd), IDC_CHAT_SHOWNICKLIST)))
				SendMessage(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(IDC_CHAT_SHOWNICKLIST, 0), 0);
			return TRUE;
		}

		if (wParam == 0x48 && isCtrl && !isAlt) { // ctrl-h (history)
			SendMessage(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(IDC_HISTORY, 0), 0);
			return TRUE;
		}

		if (wParam == 0x4f && isCtrl && !isAlt) { // ctrl-o (options)
			if (IsWindowEnabled(GetDlgItem(GetParent(hwnd), IDC_CHAT_CHANMGR)))
				SendMessage(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(IDC_CHAT_CHANMGR, 0), 0);
			return TRUE;
		}

		if (((wParam == VK_INSERT && isShift) || (wParam == 'V' && isCtrl)) && !isAlt) { // ctrl-v (paste clean text)
			SendMessage(hwnd, EM_PASTESPECIAL, CF_UNICODETEXT, 0);
			return TRUE;
		}

		if (wParam == VK_NEXT || wParam == VK_PRIOR) {
			HWND htemp = GetParent(hwnd);
			SendDlgItemMessage(htemp, IDC_LOG, msg, wParam, lParam);
			return TRUE;
		}
		break;

	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_KILLFOCUS:
		dat->lastEnterTime = 0;
		break;

	case WM_CONTEXTMENU:
		InputAreaContextMenu(hwnd, wParam, lParam, si->hContact);
		return TRUE;

	case WM_KEYUP:
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
		{
			UINT u = 0;
			UINT u2 = 0;
			COLORREF cr;

			LoadMsgDlgFont(MSGFONTID_MESSAGEAREA, NULL, &cr);

			CHARFORMAT2 cf;
			cf.cbSize = sizeof(CHARFORMAT2);
			cf.dwMask = CFM_BOLD | CFM_ITALIC | CFM_UNDERLINE | CFM_BACKCOLOR | CFM_COLOR;
			SendMessage(hwnd, EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);

			if (pci->MM_FindModule(si->pszModule) && pci->MM_FindModule(si->pszModule)->bColor) {
				int index = pci->GetColorIndex(si->pszModule, cf.crTextColor);
				u = IsDlgButtonChecked(GetParent(hwnd), IDC_CHAT_COLOR);

				if (index >= 0) {
					si->bFGSet = TRUE;
					si->iFG = index;
				}

				if (u == BST_UNCHECKED && cf.crTextColor != cr)
					CheckDlgButton(GetParent(hwnd), IDC_CHAT_COLOR, BST_CHECKED);
				else if (u == BST_CHECKED && cf.crTextColor == cr)
					CheckDlgButton(GetParent(hwnd), IDC_CHAT_COLOR, BST_UNCHECKED);
			}

			if (pci->MM_FindModule(si->pszModule) && pci->MM_FindModule(si->pszModule)->bBkgColor) {
				int index = pci->GetColorIndex(si->pszModule, cf.crBackColor);
				COLORREF crB = db_get_dw(NULL, SRMMMOD, SRMSGSET_INPUTBKGCOLOUR, SRMSGDEFSET_INPUTBKGCOLOUR);
				u = IsDlgButtonChecked(GetParent(hwnd), IDC_CHAT_BKGCOLOR);

				if (index >= 0) {
					si->bBGSet = TRUE;
					si->iBG = index;
				}
				if (u == BST_UNCHECKED && cf.crBackColor != crB)
					CheckDlgButton(GetParent(hwnd), IDC_CHAT_BKGCOLOR, BST_CHECKED);
				else if (u == BST_CHECKED && cf.crBackColor == crB)
					CheckDlgButton(GetParent(hwnd), IDC_CHAT_BKGCOLOR, BST_UNCHECKED);
			}

			if (pci->MM_FindModule(si->pszModule) && pci->MM_FindModule(si->pszModule)->bBold) {
				u = IsDlgButtonChecked(GetParent(hwnd), IDC_CHAT_BOLD);
				u2 = cf.dwEffects;
				u2 &= CFE_BOLD;
				if (u == BST_UNCHECKED && u2)
					CheckDlgButton(GetParent(hwnd), IDC_CHAT_BOLD, BST_CHECKED);
				else if (u == BST_CHECKED && u2 == 0)
					CheckDlgButton(GetParent(hwnd), IDC_CHAT_BOLD, BST_UNCHECKED);
			}

			if (pci->MM_FindModule(si->pszModule) && pci->MM_FindModule(si->pszModule)->bItalics) {
				u = IsDlgButtonChecked(GetParent(hwnd), IDC_CHAT_ITALICS);
				u2 = cf.dwEffects;
				u2 &= CFE_ITALIC;
				if (u == BST_UNCHECKED && u2)
					CheckDlgButton(GetParent(hwnd), IDC_CHAT_ITALICS, BST_CHECKED);
				else if (u == BST_CHECKED && u2 == 0)
					CheckDlgButton(GetParent(hwnd), IDC_CHAT_ITALICS, BST_UNCHECKED);
			}

			if (pci->MM_FindModule(si->pszModule) && pci->MM_FindModule(si->pszModule)->bUnderline) {
				u = IsDlgButtonChecked(GetParent(hwnd), IDC_CHAT_UNDERLINE);
				u2 = cf.dwEffects;
				u2 &= CFE_UNDERLINE;
				if (u == BST_UNCHECKED && u2)
					CheckDlgButton(GetParent(hwnd), IDC_CHAT_UNDERLINE, BST_CHECKED);
				else if (u == BST_CHECKED && u2 == 0)
					CheckDlgButton(GetParent(hwnd), IDC_CHAT_UNDERLINE, BST_UNCHECKED);
			}
		}
		break;

	case WM_DESTROY:
		mir_free(dat->szSearchQuery);
		mir_free(dat->szSearchResult);
		mir_free(dat);
		return 0;
	}

	return mir_callNextSubclass(hwnd, MessageSubclassProc, msg, wParam, lParam);
}

static INT_PTR CALLBACK FilterWndProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static SESSION_INFO *si = NULL;
	switch (uMsg) {
	case WM_INITDIALOG:
		si = (SESSION_INFO *)lParam;
		CheckDlgButton(hwndDlg, IDC_CHAT_1, si->iLogFilterFlags & GC_EVENT_ACTION ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHAT_2, si->iLogFilterFlags & GC_EVENT_MESSAGE ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHAT_3, si->iLogFilterFlags & GC_EVENT_NICK ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHAT_4, si->iLogFilterFlags & GC_EVENT_JOIN ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHAT_5, si->iLogFilterFlags & GC_EVENT_PART ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHAT_6, si->iLogFilterFlags & GC_EVENT_TOPIC ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHAT_7, si->iLogFilterFlags & GC_EVENT_ADDSTATUS ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHAT_8, si->iLogFilterFlags & GC_EVENT_INFORMATION ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHAT_9, si->iLogFilterFlags & GC_EVENT_QUIT ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHAT_10, si->iLogFilterFlags & GC_EVENT_KICK ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHAT_11, si->iLogFilterFlags & GC_EVENT_NOTICE ? BST_CHECKED : BST_UNCHECKED);
		break;

	case WM_CTLCOLOREDIT:
	case WM_CTLCOLORSTATIC:
		SetTextColor((HDC)wParam, RGB(60, 60, 150));
		SetBkColor((HDC)wParam, GetSysColor(COLOR_WINDOW));
		return (INT_PTR)GetSysColorBrush(COLOR_WINDOW);

	case WM_ACTIVATE:
		if (LOWORD(wParam) == WA_INACTIVE) {
			int iFlags = 0;

			if (IsDlgButtonChecked(hwndDlg, IDC_CHAT_1) == BST_CHECKED)
				iFlags |= GC_EVENT_ACTION;
			if (IsDlgButtonChecked(hwndDlg, IDC_CHAT_2) == BST_CHECKED)
				iFlags |= GC_EVENT_MESSAGE;
			if (IsDlgButtonChecked(hwndDlg, IDC_CHAT_3) == BST_CHECKED)
				iFlags |= GC_EVENT_NICK;
			if (IsDlgButtonChecked(hwndDlg, IDC_CHAT_4) == BST_CHECKED)
				iFlags |= GC_EVENT_JOIN;
			if (IsDlgButtonChecked(hwndDlg, IDC_CHAT_5) == BST_CHECKED)
				iFlags |= GC_EVENT_PART;
			if (IsDlgButtonChecked(hwndDlg, IDC_CHAT_6) == BST_CHECKED)
				iFlags |= GC_EVENT_TOPIC;
			if (IsDlgButtonChecked(hwndDlg, IDC_CHAT_7) == BST_CHECKED)
				iFlags |= GC_EVENT_ADDSTATUS;
			if (IsDlgButtonChecked(hwndDlg, IDC_CHAT_8) == BST_CHECKED)
				iFlags |= GC_EVENT_INFORMATION;
			if (IsDlgButtonChecked(hwndDlg, IDC_CHAT_9) == BST_CHECKED)
				iFlags |= GC_EVENT_QUIT;
			if (IsDlgButtonChecked(hwndDlg, IDC_CHAT_10) == BST_CHECKED)
				iFlags |= GC_EVENT_KICK;
			if (IsDlgButtonChecked(hwndDlg, IDC_CHAT_11) == BST_CHECKED)
				iFlags |= GC_EVENT_NOTICE;

			if (iFlags & GC_EVENT_ADDSTATUS)
				iFlags |= GC_EVENT_REMOVESTATUS;

			SendMessage(si->hWnd, GC_CHANGEFILTERFLAG, 0, iFlags);
			if (si->bFilterEnabled)
				SendMessage(si->hWnd, GC_REDRAWLOG, 0, 0);
			PostMessage(hwndDlg, WM_CLOSE, 0, 0);
		}
		break;

	case WM_CLOSE:
		DestroyWindow(hwndDlg);
		break;
	}

	return FALSE;
}

static LRESULT CALLBACK ButtonSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_RBUTTONUP:
		if (db_get_b(NULL, CHAT_MODULE, "RightClickFilter", 0) != 0) {
			if (GetDlgItem(GetParent(hwnd), IDC_CHAT_FILTER) == hwnd)
				SendMessage(GetParent(hwnd), GC_SHOWFILTERMENU, 0, 0);
			if (GetDlgItem(GetParent(hwnd), IDC_CHAT_COLOR) == hwnd)
				SendMessage(GetParent(hwnd), GC_SHOWCOLORCHOOSER, 0, IDC_CHAT_COLOR);
			if (GetDlgItem(GetParent(hwnd), IDC_CHAT_BKGCOLOR) == hwnd)
				SendMessage(GetParent(hwnd), GC_SHOWCOLORCHOOSER, 0, IDC_CHAT_BKGCOLOR);
		}
		break;
	}

	return mir_callNextSubclass(hwnd, ButtonSubclassProc, msg, wParam, lParam);
}

static LRESULT CALLBACK LogSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static BOOL inMenu = FALSE;
	SESSION_INFO *si = (SESSION_INFO*)GetWindowLongPtr(GetParent(hwnd), GWLP_USERDATA);
	int result = InputAreaShortcuts(hwnd, msg, wParam, lParam, si);
	if (result != -1)
		return result;

	CHARRANGE sel;

	switch (msg) {
	case WM_MEASUREITEM:
		MeasureMenuItem(wParam, lParam);
		return TRUE;

	case WM_DRAWITEM:
		return DrawMenuItem(wParam, lParam);

	case WM_SETCURSOR:
		if (inMenu) {
			SetCursor(LoadCursor(NULL, IDC_ARROW));
			return TRUE;
		}
		break;

	case WM_LBUTTONUP:
		SendMessage(hwnd, EM_EXGETSEL, 0, (LPARAM)&sel);
		if (sel.cpMin != sel.cpMax) {
			SendMessage(hwnd, WM_COPY, 0, 0);
			sel.cpMin = sel.cpMax;
			SendMessage(hwnd, EM_EXSETSEL, 0, (LPARAM)&sel);
		}
		SetFocus(GetDlgItem(GetParent(hwnd), IDC_MESSAGE));
		break;

	case WM_ACTIVATE:
		if (LOWORD(wParam) == WA_INACTIVE) {
			SendMessage(hwnd, EM_EXGETSEL, 0, (LPARAM)&sel);
			if (sel.cpMin != sel.cpMax) {
				sel.cpMin = sel.cpMax;
				SendMessage(hwnd, EM_EXSETSEL, 0, (LPARAM)&sel);
			}
		}
		break;

	case WM_CONTEXTMENU:
		POINT pt;
		POINTL ptl;
		SendMessage(hwnd, EM_EXGETSEL, 0, (LPARAM)&sel);
		if (lParam == 0xFFFFFFFF) {
			SendMessage(hwnd, EM_POSFROMCHAR, (WPARAM)&pt, (LPARAM)sel.cpMax);
			ClientToScreen(hwnd, &pt);
		}
		else {
			pt.x = GET_X_LPARAM(lParam);
			pt.y = GET_Y_LPARAM(lParam);
		}
		ptl.x = (LONG)pt.x;
		ptl.y = (LONG)pt.y;
		ScreenToClient(hwnd, (LPPOINT)&ptl);
		{
			ptrW pszWord(GetRichTextWord(hwnd, &ptl));
			inMenu = TRUE;

			CHARRANGE all = { 0, -1 };
			HMENU hMenu = NULL;
			UINT uID = CreateGCMenu(hwnd, &hMenu, 1, pt, si, NULL, pszWord);
			inMenu = FALSE;
			switch (uID) {
			case 0:
				PostMessage(GetParent(hwnd), WM_MOUSEACTIVATE, 0, 0);
				break;

			case ID_COPYALL:
				SendMessage(hwnd, EM_EXGETSEL, 0, (LPARAM)&sel);
				SendMessage(hwnd, EM_EXSETSEL, 0, (LPARAM)&all);
				SendMessage(hwnd, WM_COPY, 0, 0);
				SendMessage(hwnd, EM_EXSETSEL, 0, (LPARAM)&sel);
				PostMessage(GetParent(hwnd), WM_MOUSEACTIVATE, 0, 0);
				break;

			case IDM_CLEAR:
				if (si) {
					SetWindowText(hwnd, L"");
					pci->LM_RemoveAll(&si->pLog, &si->pLogEnd);
					si->iEventCount = 0;
					si->LastTime = 0;
					PostMessage(GetParent(hwnd), WM_MOUSEACTIVATE, 0, 0);
				}
				break;

			case IDM_SEARCH_GOOGLE:
			case IDM_SEARCH_BING:
			case IDM_SEARCH_YANDEX:
			case IDM_SEARCH_YAHOO:
			case IDM_SEARCH_WIKIPEDIA:
			case IDM_SEARCH_FOODNETWORK:
			case IDM_SEARCH_GOOGLE_MAPS:
			case IDM_SEARCH_GOOGLE_TRANSLATE:
				SearchWord(pszWord, uID - IDM_SEARCH_GOOGLE + SEARCHENGINE_GOOGLE);
				PostMessage(GetParent(hwnd), WM_MOUSEACTIVATE, 0, 0);
				break;

			default:
				PostMessage(GetParent(hwnd), WM_MOUSEACTIVATE, 0, 0);
				pci->DoEventHookAsync(GetParent(hwnd), si->ptszID, si->pszModule, GC_USER_LOGMENU, NULL, NULL, uID);
				break;
			}
			DestroyGCMenu(&hMenu, 5);
		}
		break;

	case WM_CHAR:
		SetFocus(GetDlgItem(GetParent(hwnd), IDC_MESSAGE));
		SendDlgItemMessage(GetParent(hwnd), IDC_MESSAGE, WM_CHAR, wParam, lParam);
		break;
	}

	return mir_callNextSubclass(hwnd, LogSubclassProc, msg, wParam, lParam);
}

static LRESULT CALLBACK NicklistSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	SESSION_INFO *si = (SESSION_INFO*)GetWindowLongPtr(GetParent(hwnd), GWLP_USERDATA);
	int result = InputAreaShortcuts(hwnd, msg, wParam, lParam, si);
	if (result != -1)
		return result;

	switch (msg) {
	case WM_ERASEBKGND:
		{
			HDC dc = (HDC)wParam;
			if (dc) {
				int index = SendMessage(hwnd, LB_GETTOPINDEX, 0, 0);
				if (index == LB_ERR || si->nUsersInNicklist <= 0)
					return 0;

				int items = si->nUsersInNicklist - index;
				int height = SendMessage(hwnd, LB_GETITEMHEIGHT, 0, 0);

				if (height != LB_ERR) {
					RECT rc = { 0 };
					GetClientRect(hwnd, &rc);

					if (rc.bottom - rc.top > items * height) {
						rc.top = items*height;
						FillRect(dc, &rc, pci->hListBkgBrush);
					}
				}
			}
		}
		return 1;

	case WM_RBUTTONDOWN:
		SendMessage(hwnd, WM_LBUTTONDOWN, wParam, lParam);
		break;

	case WM_RBUTTONUP:
		SendMessage(hwnd, WM_LBUTTONUP, wParam, lParam);
		break;

	case WM_MEASUREITEM:
		{
			MEASUREITEMSTRUCT *mis = (MEASUREITEMSTRUCT *)lParam;
			if (mis->CtlType == ODT_MENU)
				return Menu_MeasureItem(lParam);
		}
		return FALSE;

	case WM_DRAWITEM:
		{
			DRAWITEMSTRUCT *dis = (DRAWITEMSTRUCT *)lParam;
			if (dis->CtlType == ODT_MENU)
				return Menu_DrawItem(lParam);
		}
		return FALSE;

	case WM_CONTEXTMENU:
		{
			int height = 0;

			TVHITTESTINFO hti;
			hti.pt.x = GET_X_LPARAM(lParam);
			hti.pt.y = GET_Y_LPARAM(lParam);
			if (hti.pt.x == -1 && hti.pt.y == -1) {
				int index = SendMessage(hwnd, LB_GETCURSEL, 0, 0);
				int top = SendMessage(hwnd, LB_GETTOPINDEX, 0, 0);
				height = SendMessage(hwnd, LB_GETITEMHEIGHT, 0, 0);
				hti.pt.x = 4;
				hti.pt.y = (index - top)*height + 1;
			}
			else ScreenToClient(hwnd, &hti.pt);

			DWORD item = (DWORD)(SendMessage(hwnd, LB_ITEMFROMPOINT, 0, MAKELPARAM(hti.pt.x, hti.pt.y)));
			if (HIWORD(item) == 1)
				item = (DWORD)(-1);
			else
				item &= 0xFFFF;

			USERINFO *ui = pci->SM_GetUserFromIndex(si->ptszID, si->pszModule, (int)item);
			if (ui) {
				HMENU hMenu = 0;
				UINT uID;
				USERINFO uinew;

				memcpy(&uinew, ui, sizeof(USERINFO));
				if (hti.pt.x == -1 && hti.pt.y == -1)
					hti.pt.y += height - 4;
				ClientToScreen(hwnd, &hti.pt);
				uID = CreateGCMenu(hwnd, &hMenu, 0, hti.pt, si, uinew.pszUID, uinew.pszNick);

				switch (uID) {
				case 0:
					break;

				case ID_MESS:
					pci->DoEventHookAsync(GetParent(hwnd), si->ptszID, si->pszModule, GC_USER_PRIVMESS, ui, nullptr, 0);
					break;

				default:
					pci->DoEventHookAsync(GetParent(hwnd), si->ptszID, si->pszModule, GC_USER_NICKLISTMENU, ui, nullptr, uID);
					break;
				}
				DestroyGCMenu(&hMenu, 1);
				return TRUE;
			}
		}
		break;

	case WM_GETDLGCODE:
		{
			BOOL isAlt = GetKeyState(VK_MENU) & 0x8000;
			BOOL isCtrl = (GetKeyState(VK_CONTROL) & 0x8000) && !isAlt;

			LPMSG lpmsg;
			if ((lpmsg = (LPMSG)lParam) != NULL) {
				if (lpmsg->message == WM_KEYDOWN
					&& (lpmsg->wParam == VK_RETURN || lpmsg->wParam == VK_ESCAPE || (lpmsg->wParam == VK_TAB && (isAlt || isCtrl))))
					return DLGC_WANTALLKEYS;
			}
		}
		break;

	case WM_KEYDOWN:
		if (wParam == VK_RETURN) {
			int index = SendMessage(hwnd, LB_GETCURSEL, 0, 0);
			if (index != LB_ERR) {
				USERINFO *ui = pci->SM_GetUserFromIndex(si->ptszID, si->pszModule, index);
				pci->DoEventHookAsync(GetParent(hwnd), si->ptszID, si->pszModule, GC_USER_PRIVMESS, ui, nullptr, 0);
			}
			break;
		}
		if (wParam == VK_ESCAPE || wParam == VK_UP || wParam == VK_DOWN || wParam == VK_NEXT ||
			wParam == VK_PRIOR || wParam == VK_TAB || wParam == VK_HOME || wParam == VK_END) {
			si->szSearch[0] = 0;
		}
		break;

	case WM_CHAR:
	case WM_UNICHAR:
		/*
		* simple incremental search for the user (nick) - list control
		* typing esc or movement keys will clear the current search string
		*/
		if (wParam == 27 && si->szSearch[0]) {						// escape - reset everything
			si->szSearch[0] = 0;
			break;
		}
		else if (wParam == '\b' && si->szSearch[0])					// backspace
			si->szSearch[mir_wstrlen(si->szSearch) - 1] = '\0';
		else if (wParam < ' ')
			break;
		else {
			wchar_t szNew[2];
			szNew[0] = (wchar_t)wParam;
			szNew[1] = '\0';
			if (mir_wstrlen(si->szSearch) >= _countof(si->szSearch) - 2) {
				MessageBeep(MB_OK);
				break;
			}
			mir_wstrcat(si->szSearch, szNew);
		}
		if (si->szSearch[0]) {
			// iterate over the (sorted) list of nicknames and search for the
			// string we have
			int iItems = SendMessage(hwnd, LB_GETCOUNT, 0, 0);
			for (int i = 0; i < iItems; i++) {
				USERINFO *ui = pci->UM_FindUserFromIndex(si->pUsers, i);
				if (ui) {
					if (!wcsnicmp(ui->pszNick, si->szSearch, mir_wstrlen(si->szSearch))) {
						SendMessage(hwnd, LB_SETCURSEL, i, 0);
						InvalidateRect(hwnd, NULL, FALSE);
						return 0;
					}
				}
			}

			MessageBeep(MB_OK);
			si->szSearch[mir_wstrlen(si->szSearch) - 1] = '\0';
			return 0;
		}
		break;

	case WM_MOUSEMOVE:
		Chat_HoverMouse(si, hwnd, lParam, ServiceExists("mToolTip/HideTip"));
		break;
	}

	return mir_callNextSubclass(hwnd, NicklistSubclassProc, msg, wParam, lParam);
}

int GetTextPixelSize(wchar_t* pszText, HFONT hFont, BOOL bWidth)
{
	if (!pszText || !hFont)
		return 0;

	HDC hdc = GetDC(NULL);
	HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);

	RECT rc = { 0 };
	DrawText(hdc, pszText, -1, &rc, DT_CALCRECT);
	SelectObject(hdc, hOldFont);
	ReleaseDC(NULL, hdc);
	return bWidth ? rc.right - rc.left : rc.bottom - rc.top;
}

static void __cdecl phase2(void *lParam)
{
	Thread_SetName("Scriver: phase2");

	SESSION_INFO *si = (SESSION_INFO*)lParam;
	Sleep(30);
	if (si && si->hWnd)
		PostMessage(si->hWnd, GC_REDRAWLOG2, 0, 0);
}

/////////////////////////////////////////////////////////////////////////////////////////

class CChatRoomDlg : public CSrmmBaseDialog
{
	SESSION_INFO *m_si;

	CCtrlEdit m_message, m_log;
	CCtrlListBox m_nickList;

	void MessageDialogResize(int w, int h)
	{
		bool bNick = m_si->iType != GCW_SERVER && m_si->bNicklistEnabled;
		bool bToolbar = SendMessage(GetParent(m_hwnd), CM_GETTOOLBARSTATUS, 0, 0) != 0;
		int  hSplitterMinTop = TOOLBAR_HEIGHT + m_si->minLogBoxHeight, hSplitterMinBottom = m_si->minEditBoxHeight;
		int  toolbarHeight = bToolbar ? TOOLBAR_HEIGHT : 0;

		m_si->iSplitterY = m_si->desiredInputAreaHeight + SPLITTER_HEIGHT + 3;

		if (h - m_si->iSplitterY < hSplitterMinTop)
			m_si->iSplitterY = h - hSplitterMinTop;
		if (m_si->iSplitterY < hSplitterMinBottom)
			m_si->iSplitterY = hSplitterMinBottom;

		ShowWindow(GetDlgItem(m_hwnd, IDC_SPLITTERX), bNick ? SW_SHOW : SW_HIDE);
		if (m_si->iType != GCW_SERVER)
			ShowWindow(GetDlgItem(m_hwnd, IDC_CHAT_LIST), m_si->bNicklistEnabled ? SW_SHOW : SW_HIDE);
		else
			ShowWindow(GetDlgItem(m_hwnd, IDC_CHAT_LIST), SW_HIDE);

		if (m_si->iType == GCW_SERVER) {
			EnableWindow(GetDlgItem(m_hwnd, IDC_CHAT_SHOWNICKLIST), FALSE);
			EnableWindow(GetDlgItem(m_hwnd, IDC_CHAT_FILTER), FALSE);
			EnableWindow(GetDlgItem(m_hwnd, IDC_CHAT_CHANMGR), FALSE);
		}
		else {
			EnableWindow(GetDlgItem(m_hwnd, IDC_CHAT_SHOWNICKLIST), TRUE);
			EnableWindow(GetDlgItem(m_hwnd, IDC_CHAT_FILTER), TRUE);
			if (m_si->iType == GCW_CHATROOM)
				EnableWindow(GetDlgItem(m_hwnd, IDC_CHAT_CHANMGR), pci->MM_FindModule(m_si->pszModule)->bChanMgr);
		}

		HDWP hdwp = BeginDeferWindowPos(5);
		int toolbarTopY = bToolbar ? h - m_si->iSplitterY - toolbarHeight : h - m_si->iSplitterY;
		int logBottom = (m_si->hwndIeview != NULL) ? toolbarTopY / 2 : toolbarTopY;

		hdwp = DeferWindowPos(hdwp, GetDlgItem(m_hwnd, IDC_LOG), 0, 1, 0, bNick ? w - m_si->iSplitterX - 1 : w - 2, logBottom, SWP_NOZORDER);
		hdwp = DeferWindowPos(hdwp, GetDlgItem(m_hwnd, IDC_CHAT_LIST), 0, w - m_si->iSplitterX + 2, 0, m_si->iSplitterX - 3, toolbarTopY, SWP_NOZORDER);
		hdwp = DeferWindowPos(hdwp, GetDlgItem(m_hwnd, IDC_SPLITTERX), 0, w - m_si->iSplitterX, 1, 2, toolbarTopY - 1, SWP_NOZORDER);
		hdwp = DeferWindowPos(hdwp, GetDlgItem(m_hwnd, IDC_SPLITTERY), 0, 0, h - m_si->iSplitterY, w, SPLITTER_HEIGHT, SWP_NOZORDER);
		hdwp = DeferWindowPos(hdwp, GetDlgItem(m_hwnd, IDC_MESSAGE), 0, 1, h - m_si->iSplitterY + SPLITTER_HEIGHT, w - 2, m_si->iSplitterY - SPLITTER_HEIGHT - 1, SWP_NOZORDER);
		EndDeferWindowPos(hdwp);

		SetButtonsPos(m_hwnd, bToolbar);

		if (m_si->hwndIeview != NULL) {
			IEVIEWWINDOW ieWindow;
			ieWindow.cbSize = sizeof(IEVIEWWINDOW);
			ieWindow.iType = IEW_SETPOS;
			ieWindow.parent = m_hwnd;
			ieWindow.hwnd = m_si->hwndIeview;
			ieWindow.x = 0;
			ieWindow.y = logBottom + 1;
			ieWindow.cx = bNick ? w - m_si->iSplitterX : w;
			ieWindow.cy = logBottom;
			CallService(MS_IEVIEW_WINDOW, 0, (LPARAM)&ieWindow);
		}
		else RedrawWindow(GetDlgItem(m_hwnd, IDC_LOG), NULL, NULL, RDW_INVALIDATE);

		RedrawWindow(GetDlgItem(m_hwnd, IDC_CHAT_LIST), NULL, NULL, RDW_INVALIDATE);
		RedrawWindow(GetDlgItem(m_hwnd, IDC_MESSAGE), NULL, NULL, RDW_INVALIDATE);
	}

public:
	CChatRoomDlg(SESSION_INFO *si)
		: CSrmmBaseDialog(g_hInst, IDD_CHANNEL),
		m_si(si),
		m_log(this, IDC_LOG),
		m_message(this, IDC_MESSAGE),
		m_nickList(this, IDC_CHAT_LIST)
	{
		m_pLog = &m_log;
		m_pEntry = &m_message;
		m_autoClose = 0;
	}

	virtual void OnInitDialog() override
	{
		NotifyLocalWinEvent(m_si->hContact, m_hwnd, MSG_WINDOW_EVT_OPENING);

		SetWindowLongPtr(m_hwnd, GWLP_USERDATA, (LONG_PTR)m_si);

		RichUtil_SubClass(m_message.GetHwnd());
		RichUtil_SubClass(m_log.GetHwnd());
		RichUtil_SubClass(m_nickList.GetHwnd());
		mir_subclassWindow(GetDlgItem(m_hwnd, IDC_SPLITTERX), SplitterSubclassProc);
		mir_subclassWindow(m_nickList.GetHwnd(), NicklistSubclassProc);
		mir_subclassWindow(m_log.GetHwnd(), LogSubclassProc);
		mir_subclassWindow(GetDlgItem(m_hwnd, IDC_CHAT_FILTER), ButtonSubclassProc);
		mir_subclassWindow(GetDlgItem(m_hwnd, IDC_CHAT_COLOR), ButtonSubclassProc);
		mir_subclassWindow(GetDlgItem(m_hwnd, IDC_CHAT_BKGCOLOR), ButtonSubclassProc);
		mir_subclassWindow(m_message.GetHwnd(), MessageSubclassProc);

		Srmm_CreateToolbarIcons(m_hwnd, BBBF_ISCHATBUTTON);

		RECT minEditInit;
		GetWindowRect(m_message.GetHwnd(), &minEditInit);
		m_si->minEditBoxHeight = minEditInit.bottom - minEditInit.top;
		m_si->minLogBoxHeight = m_si->minEditBoxHeight;

		m_message.SendMsg(EM_SUBCLASSED, 0, 0);
		m_message.SendMsg(EM_SETEVENTMASK, 0, ENM_MOUSEEVENTS | ENM_KEYEVENTS | ENM_CHANGE | ENM_REQUESTRESIZE);

		int mask = (int)m_log.SendMsg(EM_GETEVENTMASK, 0, 0);
		m_log.SendMsg(EM_SETEVENTMASK, 0, mask | ENM_LINK | ENM_MOUSEEVENTS);
		m_log.SendMsg(EM_LIMITTEXT, sizeof(wchar_t) * 0x7FFFFFFF, 0);
		m_log.SendMsg(EM_SETOLECALLBACK, 0, (LPARAM)&reOleCallback);
		m_log.SendMsg(EM_AUTOURLDETECT, 1, 0);

		if (db_get_b(NULL, CHAT_MODULE, "UseIEView", 0)) {
			IEVIEWWINDOW ieWindow = { sizeof(ieWindow) };
			ieWindow.iType = IEW_CREATE;
			ieWindow.dwMode = IEWM_CHAT;
			ieWindow.parent = m_hwnd;
			ieWindow.cx = 200;
			ieWindow.cy = 300;
			CallService(MS_IEVIEW_WINDOW, 0, (LPARAM)&ieWindow);

			m_si->hwndIeview = ieWindow.hwnd;

			IEVIEWEVENT iee = { sizeof(iee) };
			iee.iType = IEE_CLEAR_LOG;
			iee.hwnd = m_si->hwndIeview;
			iee.hContact = m_si->hContact;
			iee.pszProto = m_si->pszModule;
			CallService(MS_IEVIEW_EVENT, 0, (LPARAM)&iee);
		}

		m_log.SendMsg(EM_HIDESELECTION, TRUE, 0);

		SendMessage(m_hwnd, GC_SETWNDPROPS, 0, 0);
		SendMessage(m_hwnd, GC_UPDATESTATUSBAR, 0, 0);
		SendMessage(m_hwnd, DM_UPDATETITLEBAR, 0, 0);

		SendMessage(GetParent(m_hwnd), CM_ADDCHILD, (WPARAM)m_hwnd, m_si->hContact);
		PostMessage(m_hwnd, GC_UPDATENICKLIST, 0, 0);
		NotifyLocalWinEvent(m_si->hContact, m_hwnd, MSG_WINDOW_EVT_OPEN);
	}

	virtual void OnDestroy() override
	{
		NotifyLocalWinEvent(m_si->hContact, m_hwnd, MSG_WINDOW_EVT_CLOSING);

		m_si->hWnd = NULL;
		SetWindowLongPtr(m_hwnd, GWLP_USERDATA, 0);

		SendMessage(GetParent(m_hwnd), CM_REMOVECHILD, 0, (LPARAM)m_hwnd);
		if (m_si->hwndIeview != NULL) {
			IEVIEWWINDOW ieWindow;
			ieWindow.cbSize = sizeof(IEVIEWWINDOW);
			ieWindow.iType = IEW_DESTROY;
			ieWindow.hwnd = m_si->hwndIeview;
			CallService(MS_IEVIEW_WINDOW, 0, (LPARAM)&ieWindow);
		}

		NotifyLocalWinEvent(m_si->hContact, m_hwnd, MSG_WINDOW_EVT_CLOSE);
	}

	virtual INT_PTR DlgProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override
	{
		static HMENU hToolbarMenu;
		RECT rc;
		POINT pt;
		HICON hIcon;
		TabControlData tcd;
		TitleBarData tbd;
		wchar_t szTemp[512];

		switch (uMsg) {
		case GC_SETWNDPROPS:
			// LoadGlobalSettings();
			InitButtons(m_hwnd, m_si);

			SendMessage(m_hwnd, GC_UPDATESTATUSBAR, 0, 0);
			SendMessage(m_hwnd, DM_UPDATETITLEBAR, 0, 0);
			SendMessage(m_hwnd, GC_FIXTABICONS, 0, 0);

			m_log.SendMsg(EM_SETBKGNDCOLOR, 0, g_Settings.crLogBackground);
			{
				// messagebox
				COLORREF crFore;
				LoadMsgDlgFont(MSGFONTID_MESSAGEAREA, NULL, &crFore);

				CHARFORMAT2 cf;
				cf.cbSize = sizeof(CHARFORMAT2);
				cf.dwMask = CFM_COLOR | CFM_BOLD | CFM_UNDERLINE | CFM_BACKCOLOR;
				cf.dwEffects = 0;
				cf.crTextColor = crFore;
				cf.crBackColor = db_get_dw(NULL, SRMMMOD, SRMSGSET_INPUTBKGCOLOUR, SRMSGDEFSET_INPUTBKGCOLOUR);
				m_message.SendMsg(EM_SETBKGNDCOLOR, 0, db_get_dw(NULL, SRMMMOD, SRMSGSET_INPUTBKGCOLOUR, SRMSGDEFSET_INPUTBKGCOLOUR));
				m_message.SendMsg(WM_SETFONT, (WPARAM)g_Settings.MessageBoxFont, MAKELPARAM(TRUE, 0));
				m_message.SendMsg(EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf);

				// nicklist
				int ih = GetTextPixelSize(L"AQG_glo'", g_Settings.UserListFont, FALSE);
				int ih2 = GetTextPixelSize(L"AQG_glo'", g_Settings.UserListHeadingsFont, FALSE);
				int height = db_get_b(NULL, CHAT_MODULE, "NicklistRowDist", 12);
				int font = ih > ih2 ? ih : ih2;
				// make sure we have space for icon!
				if (db_get_b(NULL, CHAT_MODULE, "ShowContactStatus", 0))
					font = font > 16 ? font : 16;

				m_nickList.SendMsg(LB_SETITEMHEIGHT, 0, height > font ? height : font);
				InvalidateRect(m_nickList.GetHwnd(), NULL, TRUE);
			}
			m_message.SendMsg(EM_REQUESTRESIZE, 0, 0);
			SendMessage(m_hwnd, WM_SIZE, 0, 0);
			SendMessage(m_hwnd, GC_REDRAWLOG2, 0, 0);
			break;

		case DM_UPDATETITLEBAR:
			if (g_dat.flags & SMF_STATUSICON) {
				MODULEINFO *mi = pci->MM_FindModule(m_si->pszModule);
				tbd.hIcon = (m_si->wStatus == ID_STATUS_ONLINE) ? mi->hOnlineIcon : mi->hOfflineIcon;
				tbd.hIconBig = (m_si->wStatus == ID_STATUS_ONLINE) ? mi->hOnlineIconBig : mi->hOfflineIconBig;
			}
			else {
				tbd.hIcon = GetCachedIcon("chat_window");
				tbd.hIconBig = g_dat.hIconChatBig;
			}
			tbd.hIconNot = (m_si->wState & (GC_EVENT_HIGHLIGHT | STATE_TALK)) ? GetCachedIcon("chat_overlay") : NULL;

			switch (m_si->iType) {
			case GCW_CHATROOM:
				mir_snwprintf(szTemp,
					(m_si->nUsersInNicklist == 1) ? TranslateT("%s: chat room (%u user)") : TranslateT("%s: chat room (%u users)"),
					m_si->ptszName, m_si->nUsersInNicklist);
				break;
			case GCW_PRIVMESS:
				mir_snwprintf(szTemp,
					(m_si->nUsersInNicklist == 1) ? TranslateT("%s: message session") : TranslateT("%s: message session (%u users)"),
					m_si->ptszName, m_si->nUsersInNicklist);
				break;
			case GCW_SERVER:
				mir_snwprintf(szTemp, L"%s: Server", m_si->ptszName);
				break;
			}
			tbd.iFlags = TBDF_TEXT | TBDF_ICON;
			tbd.pszText = szTemp;
			SendMessage(GetParent(m_hwnd), CM_UPDATETITLEBAR, (WPARAM)&tbd, (LPARAM)m_hwnd);
			SendMessage(m_hwnd, DM_UPDATETABCONTROL, 0, 0);
			break;

		case GC_UPDATESTATUSBAR:
			{
				MODULEINFO *mi = pci->MM_FindModule(m_si->pszModule);
				hIcon = m_si->wStatus == ID_STATUS_ONLINE ? mi->hOnlineIcon : mi->hOfflineIcon;
				mir_snwprintf(szTemp, L"%s : %s", mi->ptszModDispName, m_si->ptszStatusbarText ? m_si->ptszStatusbarText : L"");

				StatusBarData sbd;
				sbd.iItem = 0;
				sbd.iFlags = SBDF_TEXT | SBDF_ICON;
				sbd.hIcon = hIcon;
				sbd.pszText = szTemp;
				SendMessage(GetParent(m_hwnd), CM_UPDATESTATUSBAR, (WPARAM)&sbd, (LPARAM)m_hwnd);

				sbd.iItem = 1;
				sbd.hIcon = NULL;
				sbd.pszText = L"";
				SendMessage(GetParent(m_hwnd), CM_UPDATESTATUSBAR, (WPARAM)&sbd, (LPARAM)m_hwnd);

				StatusIconData sid = { sizeof(sid) };
				sid.szModule = SRMMMOD;
				Srmm_ModifyIcon(m_si->hContact, &sid);
			}
			break;

		case DM_SWITCHINFOBAR:
		case DM_SWITCHTOOLBAR:
			SendMessage(m_hwnd, WM_SIZE, 0, 0);
			break;

		case WM_SIZE:
			if (wParam == SIZE_MAXIMIZED)
				PostMessage(m_hwnd, GC_SCROLLTOBOTTOM, 0, 0);

			if (IsIconic(m_hwnd)) break;

			if (wParam == SIZE_RESTORED || wParam == SIZE_MAXIMIZED) {
				GetClientRect(m_hwnd, &rc);
				MessageDialogResize(rc.right - rc.left, rc.bottom - rc.top);
			}
			break;

		case GC_REDRAWWINDOW:
			InvalidateRect(m_hwnd, NULL, TRUE);
			break;

		case GC_REDRAWLOG:
			m_si->LastTime = 0;
			if (m_si->pLog) {
				LOGINFO *pLog = m_si->pLog;
				if (m_si->iEventCount > 60) {
					int index = 0;
					while (index < 59) {
						if (pLog->next == NULL)
							break;

						pLog = pLog->next;
						if ((m_si->iType != GCW_CHATROOM && m_si->iType != GCW_PRIVMESS) || !m_si->bFilterEnabled || (m_si->iLogFilterFlags&pLog->iType) != 0)
							index++;
					}
					Log_StreamInEvent(m_hwnd, pLog, m_si, TRUE);
					mir_forkthread(phase2, m_si);
				}
				else Log_StreamInEvent(m_hwnd, m_si->pLogEnd, m_si, TRUE);
			}
			else SendMessage(m_hwnd, GC_CONTROL_MSG, WINDOW_CLEARLOG, 0);
			break;

		case GC_REDRAWLOG2:
			m_si->LastTime = 0;
			if (m_si->pLog)
				Log_StreamInEvent(m_hwnd, m_si->pLogEnd, m_si, TRUE);
			break;

		case GC_ADDLOG:
			if (m_si->pLogEnd)
				Log_StreamInEvent(m_hwnd, m_si->pLog, m_si, FALSE);
			else
				SendMessage(m_hwnd, GC_CONTROL_MSG, WINDOW_CLEARLOG, 0);
			break;

		case DM_UPDATETABCONTROL:
			tcd.iFlags = TCDF_TEXT;
			tcd.pszText = m_si->ptszName;
			SendMessage(GetParent(m_hwnd), CM_UPDATETABCONTROL, (WPARAM)&tcd, (LPARAM)m_hwnd);
			// fall through

		case GC_FIXTABICONS:
			if (!(m_si->wState & GC_EVENT_HIGHLIGHT)) {
				if (m_si->wState & STATE_TALK)
					hIcon = (m_si->wStatus == ID_STATUS_ONLINE) ? pci->MM_FindModule(m_si->pszModule)->hOnlineTalkIcon : pci->MM_FindModule(m_si->pszModule)->hOfflineTalkIcon;
				else
					hIcon = (m_si->wStatus == ID_STATUS_ONLINE) ? pci->MM_FindModule(m_si->pszModule)->hOnlineIcon : pci->MM_FindModule(m_si->pszModule)->hOfflineIcon;
			}
			else hIcon = g_dat.hMsgIcon;

			tcd.iFlags = TCDF_ICON;
			tcd.hIcon = hIcon;
			SendMessage(GetParent(m_hwnd), CM_UPDATETABCONTROL, (WPARAM)&tcd, (LPARAM)m_hwnd);
			break;

		case GC_SETMESSAGEHIGHLIGHT:
			m_si->wState |= GC_EVENT_HIGHLIGHT;
			SendMessage(m_si->hWnd, GC_FIXTABICONS, 0, 0);
			SendMessage(m_hwnd, DM_UPDATETITLEBAR, 0, 0);
			if (g_Settings.bFlashWindowHighlight && GetActiveWindow() != m_hwnd && GetForegroundWindow() != GetParent(m_hwnd))
				SendMessage(GetParent(m_si->hWnd), CM_STARTFLASHING, 0, 0);
			break;

		case GC_SETTABHIGHLIGHT:
			SendMessage(m_si->hWnd, GC_FIXTABICONS, 0, 0);
			SendMessage(m_hwnd, DM_UPDATETITLEBAR, 0, 0);
			if (g_Settings.bFlashWindow && GetActiveWindow() != GetParent(m_hwnd) && GetForegroundWindow() != GetParent(m_hwnd))
				SendMessage(GetParent(m_si->hWnd), CM_STARTFLASHING, 0, 0);
			break;

		case DM_ACTIVATE:
			if (m_si->wState & STATE_TALK) {
				m_si->wState &= ~STATE_TALK;
				db_set_w(m_si->hContact, m_si->pszModule, "ApparentMode", 0);
			}

			if (m_si->wState & GC_EVENT_HIGHLIGHT) {
				m_si->wState &= ~GC_EVENT_HIGHLIGHT;

				if (pcli->pfnGetEvent(m_si->hContact, 0))
					pcli->pfnRemoveEvent(m_si->hContact, GC_FAKE_EVENT);
			}

			SendMessage(m_hwnd, GC_FIXTABICONS, 0, 0);
			if (!m_si->hWnd) {
				ShowRoom(m_si);
				SendMessage(m_hwnd, WM_MOUSEACTIVATE, 0, 0);
			}
			break;

		case WM_CTLCOLORLISTBOX:
			SetBkColor((HDC)wParam, g_Settings.crUserListBGColor);
			return (INT_PTR)pci->hListBkgBrush;

		case WM_MEASUREITEM:
			if (!MeasureMenuItem(wParam, lParam)) {
				MEASUREITEMSTRUCT *mis = (MEASUREITEMSTRUCT *)lParam;
				if (mis->CtlType == ODT_MENU)
					return Menu_MeasureItem(lParam);

				int ih = GetTextPixelSize(L"AQGgl'", g_Settings.UserListFont, FALSE);
				int ih2 = GetTextPixelSize(L"AQGg'", g_Settings.UserListHeadingsFont, FALSE);
				int font = ih > ih2 ? ih : ih2;
				int height = db_get_b(NULL, CHAT_MODULE, "NicklistRowDist", 12);
				// make sure we have space for icon!
				if (db_get_b(NULL, CHAT_MODULE, "ShowContactStatus", 0))
					font = font > 16 ? font : 16;
				mis->itemHeight = height > font ? height : font;
			}
			return TRUE;

		case WM_DRAWITEM:
			if (!DrawMenuItem(wParam, lParam)) {
				DRAWITEMSTRUCT *dis = (DRAWITEMSTRUCT *)lParam;
				if (dis->CtlType == ODT_MENU)
					return Menu_DrawItem(lParam);

				if (dis->CtlID == IDC_CHAT_LIST) {
					int index = dis->itemID;
					USERINFO *ui = pci->SM_GetUserFromIndex(m_si->ptszID, m_si->pszModule, index);
					if (ui) {
						int x_offset = 2;

						int height = dis->rcItem.bottom - dis->rcItem.top;
						if (height & 1)
							height++;

						int offset = (height == 10) ? 0 : height / 2 - 5;
						HFONT hFont = (ui->iStatusEx == 0) ? g_Settings.UserListFont : g_Settings.UserListHeadingsFont;
						HFONT hOldFont = (HFONT)SelectObject(dis->hDC, hFont);
						SetBkMode(dis->hDC, TRANSPARENT);

						if (dis->itemAction == ODA_FOCUS && dis->itemState & ODS_SELECTED)
							FillRect(dis->hDC, &dis->rcItem, pci->hListSelectedBkgBrush);
						else //if (dis->itemState & ODS_INACTIVE)
							FillRect(dis->hDC, &dis->rcItem, pci->hListBkgBrush);

						if (g_Settings.bShowContactStatus && g_Settings.bContactStatusFirst && ui->ContactStatus) {
							hIcon = Skin_LoadProtoIcon(m_si->pszModule, ui->ContactStatus);
							DrawIconEx(dis->hDC, x_offset, dis->rcItem.top + offset - 3, hIcon, 16, 16, 0, NULL, DI_NORMAL);
							IcoLib_ReleaseIcon(hIcon);
							x_offset += 18;
						}
						DrawIconEx(dis->hDC, x_offset, dis->rcItem.top + offset, pci->SM_GetStatusIcon(m_si, ui), 10, 10, 0, NULL, DI_NORMAL);
						x_offset += 12;
						if (g_Settings.bShowContactStatus && !g_Settings.bContactStatusFirst && ui->ContactStatus) {
							hIcon = Skin_LoadProtoIcon(m_si->pszModule, ui->ContactStatus);
							DrawIconEx(dis->hDC, x_offset, dis->rcItem.top + offset - 3, hIcon, 16, 16, 0, NULL, DI_NORMAL);
							IcoLib_ReleaseIcon(hIcon);
							x_offset += 18;
						}

						SetTextColor(dis->hDC, ui->iStatusEx == 0 ? g_Settings.crUserListColor : g_Settings.crUserListHeadingsColor);
						TextOut(dis->hDC, dis->rcItem.left + x_offset, dis->rcItem.top, ui->pszNick, (int)mir_wstrlen(ui->pszNick));
						SelectObject(dis->hDC, hOldFont);
					}
					return TRUE;
				}
			}
			break;

		case GC_UPDATENICKLIST:
			m_nickList.SendMsg(WM_SETREDRAW, FALSE, 0);
			m_nickList.SendMsg(LB_RESETCONTENT, 0, 0);
			for (int index = 0; index < m_si->nUsersInNicklist; index++) {
				USERINFO *ui = pci->SM_GetUserFromIndex(m_si->ptszID, m_si->pszModule, index);
				if (ui) {
					char szIndicator = SM_GetStatusIndicator(m_si, ui);
					if (szIndicator > '\0') {
						static wchar_t ptszBuf[128];
						mir_snwprintf(ptszBuf, L"%c%s", szIndicator, ui->pszNick);
						m_nickList.SendMsg(LB_ADDSTRING, 0, (LPARAM)ptszBuf);
					}
					else m_nickList.SendMsg(LB_ADDSTRING, 0, (LPARAM)ui->pszNick);
				}
			}
			m_nickList.SendMsg(WM_SETREDRAW, TRUE, 0);
			InvalidateRect(m_nickList.GetHwnd(), NULL, FALSE);
			UpdateWindow(m_nickList.GetHwnd());
			SendMessage(m_hwnd, DM_UPDATETITLEBAR, 0, 0);
			break;

		case GC_CONTROL_MSG:
			switch (wParam) {
			case SESSION_OFFLINE:
				SendMessage(m_hwnd, GC_UPDATESTATUSBAR, 0, 0);
				SendMessage(m_si->hWnd, GC_UPDATENICKLIST, 0, 0);
				return TRUE;

			case SESSION_ONLINE:
				SendMessage(m_hwnd, GC_UPDATESTATUSBAR, 0, 0);
				return TRUE;

			case WINDOW_HIDDEN:
				SendMessage(m_hwnd, GC_CLOSEWINDOW, 0, 0);
				return TRUE;

			case WINDOW_CLEARLOG:
				m_log.SetText(L"");
				return TRUE;

			case SESSION_TERMINATE:
				if (pcli->pfnGetEvent(m_si->hContact, 0))
					pcli->pfnRemoveEvent(m_si->hContact, GC_FAKE_EVENT);
				m_si->wState &= ~STATE_TALK;
				db_set_w(m_si->hContact, m_si->pszModule, "ApparentMode", 0);
				SendMessage(m_hwnd, GC_CLOSEWINDOW, 0, 0);
				return TRUE;

			case WINDOW_MINIMIZE:
				ShowWindow(m_hwnd, SW_MINIMIZE);
				goto LABEL_SHOWWINDOW;

			case WINDOW_MAXIMIZE:
				ShowWindow(m_hwnd, SW_MAXIMIZE);
				goto LABEL_SHOWWINDOW;

			case SESSION_INITDONE:
				if (db_get_b(NULL, CHAT_MODULE, "PopupOnJoin", 0) != 0)
					return TRUE;
				// fall through
			case WINDOW_VISIBLE:
				if (IsIconic(m_hwnd))
					ShowWindow(m_hwnd, SW_NORMAL);
			LABEL_SHOWWINDOW:
				SendMessage(m_hwnd, WM_SIZE, 0, 0);
				SendMessage(m_hwnd, GC_REDRAWLOG, 0, 0);
				SendMessage(m_hwnd, GC_UPDATENICKLIST, 0, 0);
				SendMessage(m_hwnd, GC_UPDATESTATUSBAR, 0, 0);
				ShowWindow(m_hwnd, SW_SHOW);
				SendMessage(m_hwnd, WM_SIZE, 0, 0);
				SetForegroundWindow(m_hwnd);
				return TRUE;
			}
			break;

		case GC_SPLITTERMOVED:
			if ((HWND)lParam == GetDlgItem(m_hwnd, IDC_SPLITTERX)) {
				GetClientRect(m_hwnd, &rc);
				pt.x = wParam; pt.y = 0;
				ScreenToClient(m_hwnd, &pt);

				m_si->iSplitterX = rc.right - pt.x + 1;
				if (m_si->iSplitterX < 35)
					m_si->iSplitterX = 35;
				if (m_si->iSplitterX > rc.right - rc.left - 35)
					m_si->iSplitterX = rc.right - rc.left - 35;
				g_Settings.iSplitterX = m_si->iSplitterX;
			}
			else if ((HWND)lParam == GetDlgItem(m_hwnd, IDC_SPLITTERY)) {
				GetClientRect(m_hwnd, &rc);
				pt.x = 0; pt.y = wParam;
				ScreenToClient(m_hwnd, &pt);
				m_si->iSplitterY = rc.bottom - pt.y;
				g_Settings.iSplitterY = m_si->iSplitterY;
			}
			PostMessage(m_hwnd, WM_SIZE, 0, 0);
			break;

		case GC_FIREHOOK:
			if (lParam) {
				NotifyEventHooks(pci->hSendEvent, 0, lParam);
				GCHOOK *gch = (GCHOOK*)lParam;
				if (gch->pDest) {
					mir_free((void*)gch->pDest->ptszID);
					mir_free((void*)gch->pDest->pszModule);
					mir_free(gch->pDest);
				}
				mir_free(gch->ptszText);
				mir_free(gch->ptszUID);
				mir_free(gch->ptszNick);
				mir_free(gch);
			}
			break;

		case GC_CHANGEFILTERFLAG:
			m_si->iLogFilterFlags = lParam;
			break;

		case GC_SHOWFILTERMENU:
			{
				HWND hwnd = CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_FILTER), m_hwnd, FilterWndProc, (LPARAM)m_si);
				TranslateDialogDefault(hwnd);
				GetWindowRect(GetDlgItem(m_hwnd, IDC_CHAT_FILTER), &rc);
				SetWindowPos(hwnd, HWND_TOP, rc.left - 85, (IsWindowVisible(GetDlgItem(m_hwnd, IDC_CHAT_FILTER)) || IsWindowVisible(GetDlgItem(m_hwnd, IDC_CHAT_BOLD))) ? rc.top - 206 : rc.top - 186, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
			}
			break;

		case GC_SHOWCOLORCHOOSER:
			pci->ColorChooser(m_si, lParam == IDC_CHAT_COLOR, m_hwnd, m_message.GetHwnd(), GetDlgItem(m_hwnd, lParam));
			break;

		case GC_SCROLLTOBOTTOM:
			if ((GetWindowLongPtr(m_log.GetHwnd(), GWL_STYLE) & WS_VSCROLL) != 0) {
				SCROLLINFO sci = { 0 };
				sci.cbSize = sizeof(sci);
				sci.fMask = SIF_PAGE | SIF_RANGE;
				GetScrollInfo(m_log.GetHwnd(), SB_VERT, &sci);

				sci.fMask = SIF_POS;
				sci.nPos = sci.nMax - sci.nPage + 1;
				SetScrollInfo(m_log.GetHwnd(), SB_VERT, &sci, TRUE);

				CHARRANGE sel;
				sel.cpMin = sel.cpMax = GetRichTextLength(m_log.GetHwnd(), CP_ACP, FALSE);
				m_log.SendMsg(EM_EXSETSEL, 0, (LPARAM)&sel);
				PostMessage(m_log.GetHwnd(), WM_VSCROLL, MAKEWPARAM(SB_BOTTOM, 0), 0);
			}
			break;

		case WM_ACTIVATE:
			if (LOWORD(wParam) != WA_ACTIVE)
				break;

			//fall through
		case WM_MOUSEACTIVATE:
			if (uMsg != WM_ACTIVATE)
				SetFocus(m_message.GetHwnd());

			pci->SetActiveSession(m_si->ptszID, m_si->pszModule);

			if (db_get_w(m_si->hContact, m_si->pszModule, "ApparentMode", 0) != 0)
				db_set_w(m_si->hContact, m_si->pszModule, "ApparentMode", 0);
			if (pcli->pfnGetEvent(m_si->hContact, 0))
				pcli->pfnRemoveEvent(m_si->hContact, GC_FAKE_EVENT);
			break;

		case WM_NOTIFY:
			{
				LPNMHDR pNmhdr = (LPNMHDR)lParam;
				switch (pNmhdr->code) {
				case EN_REQUESTRESIZE:
					if (pNmhdr->idFrom == IDC_MESSAGE) {
						REQRESIZE *rr = (REQRESIZE *)lParam;
						int height = rr->rc.bottom - rr->rc.top + 1;
						if (height < g_dat.minInputAreaHeight)
							height = g_dat.minInputAreaHeight;

						if (m_si->desiredInputAreaHeight != height) {
							m_si->desiredInputAreaHeight = height;
							SendMessage(m_hwnd, WM_SIZE, 0, 0);
							PostMessage(m_hwnd, GC_SCROLLTOBOTTOM, 0, 0);
						}
					}
					break;

				case EN_MSGFILTER:
					if (pNmhdr->idFrom == IDC_LOG && ((MSGFILTER *)lParam)->msg == WM_RBUTTONUP) {
						SetWindowLongPtr(m_hwnd, DWLP_MSGRESULT, TRUE);
						return TRUE;
					}
					break;

				case TTN_NEEDTEXT:
					if (pNmhdr->idFrom == (UINT_PTR)m_nickList.GetHwnd()) {
						LPNMTTDISPINFO lpttd = (LPNMTTDISPINFO)lParam;
						SESSION_INFO* parentdat = (SESSION_INFO*)GetWindowLongPtr(m_hwnd, GWLP_USERDATA);

						POINT p;
						GetCursorPos(&p);
						ScreenToClient(m_nickList.GetHwnd(), &p);
						int item = LOWORD(m_nickList.SendMsg(LB_ITEMFROMPOINT, 0, MAKELPARAM(p.x, p.y)));
						USERINFO *ui = pci->SM_GetUserFromIndex(parentdat->ptszID, parentdat->pszModule, item);
						if (ui != NULL) {
							static wchar_t ptszBuf[1024];
							mir_snwprintf(ptszBuf, L"%s: %s\r\n%s: %s\r\n%s: %s",
								TranslateT("Nickname"), ui->pszNick,
								TranslateT("Unique ID"), ui->pszUID,
								TranslateT("Status"), pci->TM_WordToString(parentdat->pStatuses, ui->Status));
							lpttd->lpszText = ptszBuf;
						}
					}
					break;
				}
			}
			break;

		case WM_COMMAND:
			if (!lParam && Clist_MenuProcessCommand(LOWORD(wParam), MPCF_CONTACTMENU, m_si->hContact))
				break;

			if (HIWORD(wParam) == BN_CLICKED)
				if (LOWORD(wParam) >= MIN_CBUTTONID && LOWORD(wParam) <= MAX_CBUTTONID) {
					Srmm_ClickToolbarIcon(m_si->hContact, LOWORD(wParam), GetDlgItem(m_hwnd, LOWORD(wParam)), 0);
					break;
				}

			switch (LOWORD(wParam)) {
			case IDC_CHAT_LIST:
				if (HIWORD(wParam) == LBN_DBLCLK) {
					TVHITTESTINFO hti;
					hti.pt.x = (short)LOWORD(GetMessagePos());
					hti.pt.y = (short)HIWORD(GetMessagePos());
					ScreenToClient(m_nickList.GetHwnd(), &hti.pt);

					int item = LOWORD(m_nickList.SendMsg(LB_ITEMFROMPOINT, 0, MAKELPARAM(hti.pt.x, hti.pt.y)));
					USERINFO *ui = pci->SM_GetUserFromIndex(m_si->ptszID, m_si->pszModule, item);
					if (ui) {
						if (GetKeyState(VK_SHIFT) & 0x8000) {
							LRESULT lResult = (LRESULT)m_message.SendMsg(EM_GETSEL, 0, 0);
							int start = LOWORD(lResult);
							size_t dwNameLenMax = (mir_wstrlen(ui->pszUID) + 4);
							wchar_t* pszName = (wchar_t*)alloca(sizeof(wchar_t) * dwNameLenMax);
							if (start == 0)
								mir_snwprintf(pszName, dwNameLenMax, L"%s: ", ui->pszUID);
							else
								mir_snwprintf(pszName, dwNameLenMax, L"%s ", ui->pszUID);

							m_message.SendMsg(EM_REPLACESEL, FALSE, (LPARAM)pszName);
							PostMessage(m_hwnd, WM_MOUSEACTIVATE, 0, 0);
						}
						else pci->DoEventHookAsync(m_hwnd, m_si->ptszID, m_si->pszModule, GC_USER_PRIVMESS, ui, nullptr, 0);
					}

					return TRUE;
				}

				if (HIWORD(wParam) == LBN_KILLFOCUS)
					RedrawWindow(m_nickList.GetHwnd(), NULL, NULL, RDW_INVALIDATE);
				break;

			case IDOK:
				if (IsWindowEnabled(GetDlgItem(m_hwnd, IDOK))) {
					char *pszRtf = GetRichTextRTF(m_message.GetHwnd());
					if (pszRtf == NULL)
						break;

					MODULEINFO *mi = pci->MM_FindModule(m_si->pszModule);
					if (mi == NULL)
						break;

					TCmdList *cmdListNew = tcmdlist_last(m_si->cmdList);
					while (cmdListNew != NULL && cmdListNew->temporary) {
						m_si->cmdList = tcmdlist_remove(m_si->cmdList, cmdListNew);
						cmdListNew = tcmdlist_last(m_si->cmdList);
					}

					// takes pszRtf to a queue, no leak here
					m_si->cmdList = tcmdlist_append(m_si->cmdList, pszRtf, 20, FALSE);

					CMStringW ptszText(ptrW(mir_utf8decodeW(pszRtf)));
					pci->DoRtfToTags(ptszText, mi->nColorCount, mi->crColors);
					ptszText.Trim();
					ptszText.Replace(L"%", L"%%");

					if (mi->bAckMsg) {
						EnableWindow(m_message.GetHwnd(), FALSE);
						m_message.SendMsg(EM_SETREADONLY, TRUE, 0);
					}
					else SetDlgItemText(m_hwnd, IDC_MESSAGE, L"");

					EnableWindow(GetDlgItem(m_hwnd, IDOK), FALSE);

					pci->DoEventHookAsync(m_hwnd, m_si->ptszID, m_si->pszModule, GC_USER_MESSAGE, NULL, ptszText, 0);
					SetFocus(m_message.GetHwnd());
				}
				break;

			case IDC_CHAT_SHOWNICKLIST:
				if (!IsWindowEnabled(GetDlgItem(m_hwnd, IDC_CHAT_SHOWNICKLIST)))
					break;
				if (m_si->iType == GCW_SERVER)
					break;

				m_si->bNicklistEnabled = !m_si->bNicklistEnabled;

				SendDlgItemMessage(m_hwnd, IDC_CHAT_SHOWNICKLIST, BM_SETIMAGE, IMAGE_ICON, (LPARAM)GetCachedIcon(m_si->bNicklistEnabled ? "chat_nicklist" : "chat_nicklist2"));
				SendMessage(m_hwnd, GC_SCROLLTOBOTTOM, 0, 0);
				SendMessage(m_hwnd, WM_SIZE, 0, 0);
				break;

			case IDC_MESSAGE:
				if (HIWORD(wParam) == EN_CHANGE) {
					m_si->cmdListCurrent = NULL;
					EnableWindow(GetDlgItem(m_hwnd, IDOK), GetRichTextLength(m_message.GetHwnd(), 1200, FALSE) != 0);
				}
				break;

			case IDC_HISTORY:
				if (IsWindowEnabled(GetDlgItem(m_hwnd, IDC_HISTORY))) {
					MODULEINFO *pInfo = pci->MM_FindModule(m_si->pszModule);
					if (pInfo)
						ShellExecute(m_hwnd, NULL, pci->GetChatLogsFilename(m_si, 0), NULL, NULL, SW_SHOW);
				}
				break;

			case IDC_CHAT_CHANMGR:
				if (!IsWindowEnabled(GetDlgItem(m_hwnd, IDC_CHAT_CHANMGR)))
					break;
				pci->DoEventHookAsync(m_hwnd, m_si->ptszID, m_si->pszModule, GC_USER_CHANMGR, NULL, NULL, 0);
				break;

			case IDC_CHAT_FILTER:
				if (!IsWindowEnabled(GetDlgItem(m_hwnd, IDC_CHAT_FILTER)))
					break;

				m_si->bFilterEnabled = !m_si->bFilterEnabled;
				SendDlgItemMessage(m_hwnd, IDC_CHAT_FILTER, BM_SETIMAGE, IMAGE_ICON, (LPARAM)GetCachedIcon(m_si->bFilterEnabled ? "chat_filter" : "chat_filter2"));
				if (m_si->bFilterEnabled && db_get_b(NULL, CHAT_MODULE, "RightClickFilter", 0) == 0) {
					SendMessage(m_hwnd, GC_SHOWFILTERMENU, 0, 0);
					break;
				}
				SendMessage(m_hwnd, GC_REDRAWLOG, 0, 0);
				break;

			case IDC_CHAT_BKGCOLOR:
				if (IsWindowEnabled(GetDlgItem(m_hwnd, IDC_CHAT_BKGCOLOR))) {
					MODULEINFO *pInfo = pci->MM_FindModule(m_si->pszModule);
					CHARFORMAT2 cf;
					cf.cbSize = sizeof(CHARFORMAT2);
					cf.dwEffects = 0;

					if (IsDlgButtonChecked(m_hwnd, IDC_CHAT_BKGCOLOR)) {
						if (db_get_b(NULL, CHAT_MODULE, "RightClickFilter", 0) == 0)
							SendMessage(m_hwnd, GC_SHOWCOLORCHOOSER, 0, IDC_CHAT_BKGCOLOR);
						else if (m_si->bBGSet) {
							cf.dwMask = CFM_BACKCOLOR;
							cf.crBackColor = pInfo->crColors[m_si->iBG];
							if (pInfo->bSingleFormat)
								m_message.SendMsg(EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf);
							else
								m_message.SendMsg(EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
						}
					}
					else {
						cf.dwMask = CFM_BACKCOLOR;
						cf.crBackColor = (COLORREF)db_get_dw(NULL, SRMMMOD, SRMSGSET_INPUTBKGCOLOUR, SRMSGDEFSET_INPUTBKGCOLOUR);
						if (pInfo->bSingleFormat)
							m_message.SendMsg(EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf);
						else
							m_message.SendMsg(EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
					}
				}
				break;

			case IDC_CHAT_COLOR:
				{
					MODULEINFO *pInfo = pci->MM_FindModule(m_si->pszModule);
					CHARFORMAT2 cf;
					cf.cbSize = sizeof(CHARFORMAT2);
					cf.dwEffects = 0;

					if (!IsWindowEnabled(GetDlgItem(m_hwnd, IDC_CHAT_COLOR)))
						break;

					if (IsDlgButtonChecked(m_hwnd, IDC_CHAT_COLOR)) {
						if (db_get_b(NULL, CHAT_MODULE, "RightClickFilter", 0) == 0)
							SendMessage(m_hwnd, GC_SHOWCOLORCHOOSER, 0, IDC_CHAT_COLOR);
						else if (m_si->bFGSet) {
							cf.dwMask = CFM_COLOR;
							cf.crTextColor = pInfo->crColors[m_si->iFG];
							if (pInfo->bSingleFormat)
								m_message.SendMsg(EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf);
							else
								m_message.SendMsg(EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
						}
					}
					else {
						COLORREF cr;
						LoadMsgDlgFont(MSGFONTID_MESSAGEAREA, NULL, &cr);
						cf.dwMask = CFM_COLOR;
						cf.crTextColor = cr;
						if (pInfo->bSingleFormat)
							m_message.SendMsg(EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf);
						else
							m_message.SendMsg(EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
					}
				}
				break;

			case IDC_CHAT_BOLD:
			case IDC_CHAT_ITALICS:
			case IDC_CHAT_UNDERLINE:
				{
					MODULEINFO *pInfo = pci->MM_FindModule(m_si->pszModule);
					CHARFORMAT2 cf;
					cf.cbSize = sizeof(CHARFORMAT2);
					cf.dwMask = CFM_BOLD | CFM_ITALIC | CFM_UNDERLINE;
					cf.dwEffects = 0;

					if (LOWORD(wParam) == IDC_CHAT_BOLD && !IsWindowEnabled(GetDlgItem(m_hwnd, IDC_CHAT_BOLD)))
						break;
					if (LOWORD(wParam) == IDC_CHAT_ITALICS && !IsWindowEnabled(GetDlgItem(m_hwnd, IDC_CHAT_ITALICS)))
						break;
					if (LOWORD(wParam) == IDC_CHAT_UNDERLINE && !IsWindowEnabled(GetDlgItem(m_hwnd, IDC_CHAT_UNDERLINE)))
						break;
					if (IsDlgButtonChecked(m_hwnd, IDC_CHAT_BOLD))
						cf.dwEffects |= CFE_BOLD;
					if (IsDlgButtonChecked(m_hwnd, IDC_CHAT_ITALICS))
						cf.dwEffects |= CFE_ITALIC;
					if (IsDlgButtonChecked(m_hwnd, IDC_CHAT_UNDERLINE))
						cf.dwEffects |= CFE_UNDERLINE;
					if (pInfo->bSingleFormat)
						m_message.SendMsg(EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf);
					else
						m_message.SendMsg(EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
				}
				break;

			case IDCANCEL:
				PostMessage(m_hwnd, WM_CLOSE, 0, 0);
			}
			break;

		case WM_KEYDOWN:
			SetFocus(m_message.GetHwnd());
			break;

		case WM_GETMINMAXINFO:
			{
				MINMAXINFO* mmi = (MINMAXINFO*)lParam;
				mmi->ptMinTrackSize.x = m_si->iSplitterX + 43;
				if (mmi->ptMinTrackSize.x < 350)
					mmi->ptMinTrackSize.x = 350;

				mmi->ptMinTrackSize.y = m_si->minLogBoxHeight + TOOLBAR_HEIGHT + m_si->minEditBoxHeight + 5;
			}
			break;

		case WM_LBUTTONDBLCLK:
			if (LOWORD(lParam) < 30)
				PostMessage(m_hwnd, GC_SCROLLTOBOTTOM, 0, 0);
			else
				SendMessage(GetParent(m_hwnd), WM_SYSCOMMAND, SC_MINIMIZE, 0);
			break;

		case WM_LBUTTONDOWN:
			SendMessage(GetParent(m_hwnd), WM_LBUTTONDOWN, wParam, lParam);
			return TRUE;

		case DM_GETCONTEXTMENU:
			SetWindowLongPtr(m_hwnd, DWLP_MSGRESULT, (LPARAM)Menu_BuildContactMenu(m_si->hContact));
			return TRUE;

		case WM_CONTEXTMENU:
			if (GetParent(m_hwnd) == (HWND)wParam) {
				HMENU hMenu = Menu_BuildContactMenu(m_si->hContact);
				GetCursorPos(&pt);
				TrackPopupMenu(hMenu, 0, pt.x, pt.y, 0, m_hwnd, NULL);
				DestroyMenu(hMenu);
			}
			break;

		case GC_CLOSEWINDOW:
			Close();
			break;
		}
		return CSrmmBaseDialog::DlgProc(uMsg, wParam, lParam);
	}
};

void ShowRoom(SESSION_INFO *si)
{
	if (si == NULL)
		return;

	// Do we need to create a window?
	if (si->hWnd == NULL) {
		HWND hParent = GetParentWindow(si->hContact, TRUE);

		CChatRoomDlg *pDlg = new CChatRoomDlg(si);
		pDlg->SetParent(hParent);
		pDlg->Show();
		
		si->parent = (ParentWindowData*)GetWindowLongPtr(hParent, GWLP_USERDATA);
		si->hWnd = pDlg->GetHwnd();
	}
	SendMessage(si->hWnd, DM_UPDATETABCONTROL, -1, (LPARAM)si);
	SendMessage(GetParent(si->hWnd), CM_ACTIVATECHILD, 0, (LPARAM)si->hWnd);
	SendMessage(GetParent(si->hWnd), CM_POPUPWINDOW, 0, (LPARAM)si->hWnd);
	SendMessage(si->hWnd, WM_MOUSEACTIVATE, 0, 0);
	SetFocus(GetDlgItem(si->hWnd, IDC_MESSAGE));
}
