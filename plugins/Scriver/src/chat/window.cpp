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

#include "../commonheaders.h"

static ToolbarButton toolbarButtons[] = {
	{ LPGENT("Bold"), IDC_CHAT_BOLD, 0, 4, 24 },
	{ LPGENT("Italic"), IDC_CHAT_ITALICS, 0, 0, 24 },
	{ LPGENT("Underline"), IDC_CHAT_UNDERLINE, 0, 0, 24 },
	{ LPGENT("Text color"), IDC_CHAT_COLOR, 0, 0, 24 },
	{ LPGENT("Background color"), IDC_CHAT_BKGCOLOR, 0, 0, 24 },
	{ LPGENT("Smiley"), IDC_CHAT_SMILEY, 0, 8, 24 },
	{ LPGENT("History"), IDC_CHAT_HISTORY, 1, 0, 24 },
	{ LPGENT("Filter"), IDC_CHAT_FILTER, 1, 0, 24 },
	{ LPGENT("Manager"), IDC_CHAT_CHANMGR, 1, 0, 24 },
	{ LPGENT("Nick list"), IDC_CHAT_SHOWNICKLIST, 1, 0, 24 },
	{ LPGENT("Send"), IDOK, 1, 0, 38 },
};

struct MESSAGESUBDATA
{
	time_t lastEnterTime;
	TCHAR*  szSearchQuery;
	TCHAR*  szSearchResult;
	SESSION_INFO *lastSession;
};

static LRESULT CALLBACK SplitterSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_NCHITTEST:
		return HTCLIENT;

	case WM_SETCURSOR:
		{
			RECT rc;
			GetClientRect(hwnd, &rc);
			SetCursor(rc.right > rc.bottom ? hCurSplitNS : hCurSplitWE);
			return TRUE;
		}
		return 0;

	case WM_LBUTTONDOWN:
		SetCapture(hwnd);
		return 0;

	case WM_MOUSEMOVE:
		if (GetCapture() == hwnd) {
			RECT rc;
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
	SendDlgItemMessage(hwndDlg, IDC_CHAT_SMILEY, BM_SETIMAGE, IMAGE_ICON, (LPARAM)GetCachedIcon("chat_smiley"));
	SendDlgItemMessage(hwndDlg, IDC_CHAT_BOLD, BM_SETIMAGE, IMAGE_ICON, (LPARAM)GetCachedIcon("chat_bold"));
	SendDlgItemMessage(hwndDlg, IDC_CHAT_ITALICS, BM_SETIMAGE, IMAGE_ICON, (LPARAM)GetCachedIcon("chat_italics"));
	SendDlgItemMessage(hwndDlg, IDC_CHAT_UNDERLINE, BM_SETIMAGE, IMAGE_ICON, (LPARAM)GetCachedIcon("chat_underline"));
	SendDlgItemMessage(hwndDlg, IDC_CHAT_COLOR, BM_SETIMAGE, IMAGE_ICON, (LPARAM)GetCachedIcon("chat_fgcol"));
	SendDlgItemMessage(hwndDlg, IDC_CHAT_BKGCOLOR, BM_SETIMAGE, IMAGE_ICON, (LPARAM)GetCachedIcon("chat_bkgcol"));
	SendDlgItemMessage(hwndDlg, IDC_CHAT_HISTORY, BM_SETIMAGE, IMAGE_ICON, (LPARAM)GetCachedIcon("chat_history"));
	SendDlgItemMessage(hwndDlg, IDC_CHAT_CHANMGR, BM_SETIMAGE, IMAGE_ICON, (LPARAM)GetCachedIcon("chat_settings"));
	SendDlgItemMessage(hwndDlg, IDC_CHAT_SHOWNICKLIST, BM_SETIMAGE, IMAGE_ICON, (LPARAM)GetCachedIcon(si->bNicklistEnabled ? "chat_nicklist" : "chat_nicklist2"));
	SendDlgItemMessage(hwndDlg, IDC_CHAT_FILTER, BM_SETIMAGE, IMAGE_ICON, (LPARAM)GetCachedIcon(si->bFilterEnabled ? "chat_filter" : "chat_filter2"));
	SendDlgItemMessage(hwndDlg, IDOK, BM_SETIMAGE, IMAGE_ICON, (LPARAM)GetCachedIcon("scriver_SEND"));

	SendDlgItemMessage(hwndDlg, IDC_CHAT_SMILEY, BUTTONSETASFLATBTN, TRUE, 0);
	SendDlgItemMessage(hwndDlg, IDC_CHAT_BOLD, BUTTONSETASFLATBTN, TRUE, 0);
	SendDlgItemMessage(hwndDlg, IDC_CHAT_ITALICS, BUTTONSETASFLATBTN, TRUE, 0);
	SendDlgItemMessage(hwndDlg, IDC_CHAT_UNDERLINE, BUTTONSETASFLATBTN, TRUE, 0);
	SendDlgItemMessage(hwndDlg, IDC_CHAT_BKGCOLOR, BUTTONSETASFLATBTN, TRUE, 0);
	SendDlgItemMessage(hwndDlg, IDC_CHAT_COLOR, BUTTONSETASFLATBTN, TRUE, 0);
	SendDlgItemMessage(hwndDlg, IDC_CHAT_HISTORY, BUTTONSETASFLATBTN, TRUE, 0);
	SendDlgItemMessage(hwndDlg, IDC_CHAT_SHOWNICKLIST, BUTTONSETASFLATBTN, TRUE, 0);
	SendDlgItemMessage(hwndDlg, IDC_CHAT_CHANMGR, BUTTONSETASFLATBTN, TRUE, 0);
	SendDlgItemMessage(hwndDlg, IDC_CHAT_FILTER, BUTTONSETASFLATBTN, TRUE, 0);
	SendDlgItemMessage(hwndDlg, IDOK, BUTTONSETASFLATBTN, TRUE, 0);

	SendDlgItemMessage(hwndDlg, IDC_CHAT_SMILEY, BUTTONADDTOOLTIP, (WPARAM)LPGEN("Insert a smiley"), 0);
	SendDlgItemMessage(hwndDlg, IDC_CHAT_BOLD, BUTTONADDTOOLTIP, (WPARAM)LPGEN("Make the text bold (CTRL+B)"), 0);
	SendDlgItemMessage(hwndDlg, IDC_CHAT_ITALICS, BUTTONADDTOOLTIP, (WPARAM)LPGEN("Make the text italicized (CTRL+I)"), 0);
	SendDlgItemMessage(hwndDlg, IDC_CHAT_UNDERLINE, BUTTONADDTOOLTIP, (WPARAM)LPGEN("Make the text underlined (CTRL+U)"), 0);
	SendDlgItemMessage(hwndDlg, IDC_CHAT_BKGCOLOR, BUTTONADDTOOLTIP, (WPARAM)LPGEN("Select a background color for the text (CTRL+L)"), 0);
	SendDlgItemMessage(hwndDlg, IDC_CHAT_COLOR, BUTTONADDTOOLTIP, (WPARAM)LPGEN("Select a foreground color for the text (CTRL+K)"), 0);
	SendDlgItemMessage(hwndDlg, IDC_CHAT_HISTORY, BUTTONADDTOOLTIP, (WPARAM)LPGEN("Show the history (CTRL+H)"), 0);
	SendDlgItemMessage(hwndDlg, IDC_CHAT_SHOWNICKLIST, BUTTONADDTOOLTIP, (WPARAM)LPGEN("Show/hide the nick list (CTRL+N)"), 0);
	SendDlgItemMessage(hwndDlg, IDC_CHAT_CHANMGR, BUTTONADDTOOLTIP, (WPARAM)LPGEN("Control this room (CTRL+O)"), 0);
	SendDlgItemMessage(hwndDlg, IDC_CHAT_FILTER, BUTTONADDTOOLTIP, (WPARAM)LPGEN("Enable/disable the event filter (CTRL+F)"), 0);
	SendDlgItemMessage(hwndDlg, IDOK, BUTTONADDTOOLTIP, (WPARAM)LPGEN("Send Message"), 0);

	SendDlgItemMessage(hwndDlg, IDC_CHAT_BOLD, BUTTONSETASPUSHBTN, TRUE, 0);
	SendDlgItemMessage(hwndDlg, IDC_CHAT_ITALICS, BUTTONSETASPUSHBTN, TRUE, 0);
	SendDlgItemMessage(hwndDlg, IDC_CHAT_UNDERLINE, BUTTONSETASPUSHBTN, TRUE, 0);
	SendDlgItemMessage(hwndDlg, IDC_CHAT_COLOR, BUTTONSETASPUSHBTN, TRUE, 0);
	SendDlgItemMessage(hwndDlg, IDC_CHAT_BKGCOLOR, BUTTONSETASPUSHBTN, TRUE, 0);

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

static void MessageDialogResize(HWND hwndDlg, SESSION_INFO *si, int w, int h)
{
	int  logBottom, toolbarTopY;
	BOOL bNick = si->iType != GCW_SERVER && si->bNicklistEnabled;
	BOOL bToolbar = SendMessage(GetParent(hwndDlg), CM_GETTOOLBARSTATUS, 0, 0);
	int  buttonVisibility = bToolbar ? g_dat.chatBbuttonVisibility : 0;
	int  hSplitterMinTop = TOOLBAR_HEIGHT + si->windowData.minLogBoxHeight, hSplitterMinBottom = si->windowData.minEditBoxHeight;
	int  toolbarHeight = bToolbar ? IsToolbarVisible(SIZEOF(toolbarButtons), g_dat.chatBbuttonVisibility) ? TOOLBAR_HEIGHT : TOOLBAR_HEIGHT / 3 : 0;

	si->iSplitterY = si->desiredInputAreaHeight + SPLITTER_HEIGHT + 3;

	if (h - si->iSplitterY < hSplitterMinTop)
		si->iSplitterY = h - hSplitterMinTop;
	if (si->iSplitterY < hSplitterMinBottom)
		si->iSplitterY = hSplitterMinBottom;

	ShowToolbarControls(hwndDlg, SIZEOF(toolbarButtons), toolbarButtons, buttonVisibility, SW_SHOW);
	ShowWindow(GetDlgItem(hwndDlg, IDC_CHAT_SPLITTERX), bNick ? SW_SHOW : SW_HIDE);
	if (si->iType != GCW_SERVER)
		ShowWindow(GetDlgItem(hwndDlg, IDC_CHAT_LIST), si->bNicklistEnabled ? SW_SHOW : SW_HIDE);
	else
		ShowWindow(GetDlgItem(hwndDlg, IDC_CHAT_LIST), SW_HIDE);

	if (si->iType == GCW_SERVER) {
		EnableWindow(GetDlgItem(hwndDlg, IDC_CHAT_SHOWNICKLIST), FALSE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_CHAT_FILTER), FALSE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_CHAT_CHANMGR), FALSE);
	}
	else {
		EnableWindow(GetDlgItem(hwndDlg, IDC_CHAT_SHOWNICKLIST), TRUE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_CHAT_FILTER), TRUE);
		if (si->iType == GCW_CHATROOM)
			EnableWindow(GetDlgItem(hwndDlg, IDC_CHAT_CHANMGR), pci->MM_FindModule(si->pszModule)->bChanMgr);
	}

	HDWP hdwp = BeginDeferWindowPos(20);
	toolbarTopY = bToolbar ? h - si->iSplitterY - toolbarHeight : h - si->iSplitterY;
	if (si->windowData.hwndLog != NULL)
		logBottom = toolbarTopY / 2;
	else
		logBottom = toolbarTopY;

	hdwp = DeferWindowPos(hdwp, GetDlgItem(hwndDlg, IDC_CHAT_LOG), 0, 1, 0, bNick ? w - si->iSplitterX - 1 : w - 2, logBottom, SWP_NOZORDER);
	hdwp = DeferWindowPos(hdwp, GetDlgItem(hwndDlg, IDC_CHAT_LIST), 0, w - si->iSplitterX + 2, 0, si->iSplitterX - 3, toolbarTopY, SWP_NOZORDER);
	hdwp = DeferWindowPos(hdwp, GetDlgItem(hwndDlg, IDC_CHAT_SPLITTERX), 0, w - si->iSplitterX, 1, 2, toolbarTopY - 1, SWP_NOZORDER);
	hdwp = DeferWindowPos(hdwp, GetDlgItem(hwndDlg, IDC_CHAT_SPLITTERY), 0, 0, h - si->iSplitterY, w, SPLITTER_HEIGHT, SWP_NOZORDER);
	hdwp = DeferWindowPos(hdwp, GetDlgItem(hwndDlg, IDC_CHAT_MESSAGE), 0, 1, h - si->iSplitterY + SPLITTER_HEIGHT, w - 2, si->iSplitterY - SPLITTER_HEIGHT - 1, SWP_NOZORDER);
	hdwp = ResizeToolbar(hwndDlg, hdwp, w, toolbarTopY + 1, toolbarHeight - 1, SIZEOF(toolbarButtons), toolbarButtons, buttonVisibility);
	EndDeferWindowPos(hdwp);
	if (si->windowData.hwndLog != NULL) {
		RECT rect;
		POINT pt;
		IEVIEWWINDOW ieWindow;
		GetWindowRect(GetDlgItem(hwndDlg, IDC_CHAT_LOG), &rect);
		pt.x = 0;
		pt.y = rect.top;
		ScreenToClient(GetDlgItem(hwndDlg, IDC_CHAT_LOG), &pt);
		ieWindow.cbSize = sizeof(IEVIEWWINDOW);
		ieWindow.iType = IEW_SETPOS;
		ieWindow.parent = hwndDlg;
		ieWindow.hwnd = si->windowData.hwndLog;
		ieWindow.x = 0;
		ieWindow.y = logBottom + 1;
		ieWindow.cx = bNick ? w - si->iSplitterX : w;
		ieWindow.cy = logBottom;
		CallService(MS_IEVIEW_WINDOW, 0, (LPARAM)&ieWindow);
	}
	else RedrawWindow(GetDlgItem(hwndDlg, IDC_CHAT_LOG), NULL, NULL, RDW_INVALIDATE);

	RedrawWindow(GetDlgItem(hwndDlg, IDC_CHAT_LIST), NULL, NULL, RDW_INVALIDATE);
	RedrawWindow(GetDlgItem(hwndDlg, IDC_CHAT_MESSAGE), NULL, NULL, RDW_INVALIDATE);
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
	TCHAR *pszName = NULL;
	TCHAR* pszText = (TCHAR *)mir_alloc(iLen + 100 * sizeof(TCHAR));
	gt.cb = iLen + 99 * sizeof(TCHAR);
	gt.flags = GT_DEFAULT;

	SendMessage(hwnd, EM_GETTEXTEX, (WPARAM)&gt, (LPARAM)pszText);
	if (start > 1 && pszText[start - 1] == ' ' && pszText[start - 2] == ':')
		start -= 2;

	if (dat->szSearchResult != NULL) {
		int cbResult = (int)_tcslen(dat->szSearchResult);
		if (start >= cbResult && !_tcsncicmp(dat->szSearchResult, pszText + start - cbResult, cbResult)) {
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
		if (topicStart > 5 && _tcsstr(&pszText[topicStart - 6], _T("/topic")) == &pszText[topicStart - 6])
			isTopic = true;
	}

	if (dat->szSearchQuery == NULL) {
		dat->szSearchQuery = (TCHAR*)mir_alloc(sizeof(TCHAR)*(end - start + 1));
		mir_tstrncpy(dat->szSearchQuery, pszText + start, end - start + 1);
		dat->szSearchResult = mir_tstrdup(dat->szSearchQuery);
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
	replaceStrT(dat->szSearchResult, NULL);

	if (pszName == NULL) {
		if (end != start) {
			SendMessage(hwnd, EM_SETSEL, start, end);
			SendMessage(hwnd, EM_REPLACESEL, FALSE, (LPARAM)dat->szSearchQuery);
		}
		replaceStrT(dat->szSearchQuery, NULL);
	}
	else {
		dat->szSearchResult = mir_tstrdup(pszName);
		if (end != start) {
			ptrT szReplace;
			if (!isRoom && !isTopic && g_Settings.bAddColonToAutoComplete && start == 0) {
				szReplace = (TCHAR *)mir_alloc((_tcslen(pszName) + 4) * sizeof(TCHAR));
				_tcscpy(szReplace, pszName);
				_tcscat(szReplace, _T(": "));
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

	SESSION_INFO *Parentsi = (SESSION_INFO*)GetWindowLongPtr(GetParent(hwnd), GWLP_USERDATA);
	MESSAGESUBDATA *dat = (MESSAGESUBDATA *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	CommonWindowData *windowData = &Parentsi->windowData;

	int result = InputAreaShortcuts(hwnd, msg, wParam, lParam, windowData);
	if (result != -1)
		return result;

	switch (msg) {
	case EM_SUBCLASSED:
		dat = (MESSAGESUBDATA*)mir_calloc(sizeof(MESSAGESUBDATA));
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)dat);
		return 0;

	case WM_MOUSEWHEEL:
		if ((GetWindowLongPtr(hwnd, GWL_STYLE) & WS_VSCROLL) == 0)
			SendMessage(GetDlgItem(GetParent(hwnd), IDC_CHAT_LOG), WM_MOUSEWHEEL, wParam, lParam);

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
			TabAutoComplete(hwnd, dat, Parentsi);
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
			SendMessage(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(IDC_CHAT_HISTORY, 0), 0);
			return TRUE;
		}

		if (wParam == 0x4f && isCtrl && !isAlt) { // ctrl-o (options)
			if (IsWindowEnabled(GetDlgItem(GetParent(hwnd), IDC_CHAT_CHANMGR)))
				SendMessage(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(IDC_CHAT_CHANMGR, 0), 0);
			return TRUE;
		}

		if (((wParam == VK_INSERT && isShift) || (wParam == 'V' && isCtrl)) && !isAlt) { // ctrl-v (paste clean text)
			SendMessage(hwnd, EM_PASTESPECIAL, CF_TEXT, 0);
			return TRUE;
		}

		if (wParam == VK_NEXT || wParam == VK_PRIOR) {
			HWND htemp = GetParent(hwnd);
			SendDlgItemMessage(htemp, IDC_CHAT_LOG, msg, wParam, lParam);
			return TRUE;
		}
		break;

	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_KILLFOCUS:
		dat->lastEnterTime = 0;
		break;

	case WM_CONTEXTMENU:
		InputAreaContextMenu(hwnd, wParam, lParam, Parentsi->hContact);
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

			if (pci->MM_FindModule(Parentsi->pszModule) && pci->MM_FindModule(Parentsi->pszModule)->bColor) {
				int index = GetColorIndex(Parentsi->pszModule, cf.crTextColor);
				u = IsDlgButtonChecked(GetParent(hwnd), IDC_CHAT_COLOR);

				if (index >= 0) {
					Parentsi->bFGSet = TRUE;
					Parentsi->iFG = index;
				}

				if (u == BST_UNCHECKED && cf.crTextColor != cr)
					CheckDlgButton(GetParent(hwnd), IDC_CHAT_COLOR, BST_CHECKED);
				else if (u == BST_CHECKED && cf.crTextColor == cr)
					CheckDlgButton(GetParent(hwnd), IDC_CHAT_COLOR, BST_UNCHECKED);
			}

			if (pci->MM_FindModule(Parentsi->pszModule) && pci->MM_FindModule(Parentsi->pszModule)->bBkgColor) {
				int index = GetColorIndex(Parentsi->pszModule, cf.crBackColor);
				COLORREF crB = db_get_dw(NULL, SRMMMOD, SRMSGSET_INPUTBKGCOLOUR, SRMSGDEFSET_INPUTBKGCOLOUR);
				u = IsDlgButtonChecked(GetParent(hwnd), IDC_CHAT_BKGCOLOR);

				if (index >= 0) {
					Parentsi->bBGSet = TRUE;
					Parentsi->iBG = index;
				}
				if (u == BST_UNCHECKED && cf.crBackColor != crB)
					CheckDlgButton(GetParent(hwnd), IDC_CHAT_BKGCOLOR, BST_CHECKED);
				else if (u == BST_CHECKED && cf.crBackColor == crB)
					CheckDlgButton(GetParent(hwnd), IDC_CHAT_BKGCOLOR, BST_UNCHECKED);
			}

			if (pci->MM_FindModule(Parentsi->pszModule) && pci->MM_FindModule(Parentsi->pszModule)->bBold) {
				u = IsDlgButtonChecked(GetParent(hwnd), IDC_CHAT_BOLD);
				u2 = cf.dwEffects;
				u2 &= CFE_BOLD;
				if (u == BST_UNCHECKED && u2)
					CheckDlgButton(GetParent(hwnd), IDC_CHAT_BOLD, BST_CHECKED);
				else if (u == BST_CHECKED && u2 == 0)
					CheckDlgButton(GetParent(hwnd), IDC_CHAT_BOLD, BST_UNCHECKED);
			}

			if (pci->MM_FindModule(Parentsi->pszModule) && pci->MM_FindModule(Parentsi->pszModule)->bItalics) {
				u = IsDlgButtonChecked(GetParent(hwnd), IDC_CHAT_ITALICS);
				u2 = cf.dwEffects;
				u2 &= CFE_ITALIC;
				if (u == BST_UNCHECKED && u2)
					CheckDlgButton(GetParent(hwnd), IDC_CHAT_ITALICS, BST_CHECKED);
				else if (u == BST_CHECKED && u2 == 0)
					CheckDlgButton(GetParent(hwnd), IDC_CHAT_ITALICS, BST_UNCHECKED);
			}

			if (pci->MM_FindModule(Parentsi->pszModule) && pci->MM_FindModule(Parentsi->pszModule)->bUnderline) {
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
		CheckDlgButton(hwndDlg, IDC_CHAT_1, si->iLogFilterFlags & GC_EVENT_ACTION);
		CheckDlgButton(hwndDlg, IDC_CHAT_2, si->iLogFilterFlags & GC_EVENT_MESSAGE);
		CheckDlgButton(hwndDlg, IDC_CHAT_3, si->iLogFilterFlags & GC_EVENT_NICK);
		CheckDlgButton(hwndDlg, IDC_CHAT_4, si->iLogFilterFlags & GC_EVENT_JOIN);
		CheckDlgButton(hwndDlg, IDC_CHAT_5, si->iLogFilterFlags & GC_EVENT_PART);
		CheckDlgButton(hwndDlg, IDC_CHAT_6, si->iLogFilterFlags & GC_EVENT_TOPIC);
		CheckDlgButton(hwndDlg, IDC_CHAT_7, si->iLogFilterFlags & GC_EVENT_ADDSTATUS);
		CheckDlgButton(hwndDlg, IDC_CHAT_8, si->iLogFilterFlags & GC_EVENT_INFORMATION);
		CheckDlgButton(hwndDlg, IDC_CHAT_9, si->iLogFilterFlags & GC_EVENT_QUIT);
		CheckDlgButton(hwndDlg, IDC_CHAT_10, si->iLogFilterFlags & GC_EVENT_KICK);
		CheckDlgButton(hwndDlg, IDC_CHAT_11, si->iLogFilterFlags & GC_EVENT_NOTICE);
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

			SendMessage(si->hWnd, GC_CHANGEFILTERFLAG, 0, (LPARAM)iFlags);
			if (si->bFilterEnabled)
				SendMessage(si->hWnd, GC_REDRAWLOG, 0, 0);
			PostMessage(hwndDlg, WM_CLOSE, 0, 0);
		}
		break;

	case WM_CLOSE:
		DestroyWindow(hwndDlg);
		break;
	}

	return(FALSE);
}

static LRESULT CALLBACK ButtonSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_RBUTTONUP:
		if (db_get_b(NULL, CHAT_MODULE, "RightClickFilter", 0) != 0) {
			if (GetDlgItem(GetParent(hwnd), IDC_CHAT_FILTER) == hwnd)
				SendMessage(GetParent(hwnd), GC_SHOWFILTERMENU, 0, 0);
			if (GetDlgItem(GetParent(hwnd), IDC_CHAT_COLOR) == hwnd)
				SendMessage(GetParent(hwnd), GC_SHOWCOLORCHOOSER, 0, (LPARAM)IDC_CHAT_COLOR);
			if (GetDlgItem(GetParent(hwnd), IDC_CHAT_BKGCOLOR) == hwnd)
				SendMessage(GetParent(hwnd), GC_SHOWCOLORCHOOSER, 0, (LPARAM)IDC_CHAT_BKGCOLOR);
		}
		break;
	}

	return mir_callNextSubclass(hwnd, ButtonSubclassProc, msg, wParam, lParam);
}

static LRESULT CALLBACK LogSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static BOOL inMenu = FALSE;
	SESSION_INFO *si = (SESSION_INFO*)GetWindowLongPtr(GetParent(hwnd), GWLP_USERDATA);
	int result = InputAreaShortcuts(hwnd, msg, wParam, lParam, &si->windowData);
	if (result != -1)
		return result;

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
		{
			CHARRANGE sel;
			SendMessage(hwnd, EM_EXGETSEL, 0, (LPARAM)&sel);
			if (sel.cpMin != sel.cpMax) {
				SendMessage(hwnd, WM_COPY, 0, 0);
				sel.cpMin = sel.cpMax;
				SendMessage(hwnd, EM_EXSETSEL, 0, (LPARAM)&sel);
			}
			SetFocus(GetDlgItem(GetParent(hwnd), IDC_CHAT_MESSAGE));
		}
		break;

	case WM_ACTIVATE:
		if (LOWORD(wParam) == WA_INACTIVE) {
			CHARRANGE sel;
			SendMessage(hwnd, EM_EXGETSEL, 0, (LPARAM)&sel);
			if (sel.cpMin != sel.cpMax) {
				sel.cpMin = sel.cpMax;
				SendMessage(hwnd, EM_EXSETSEL, 0, (LPARAM)&sel);
			}
		}
		break;

	case WM_CONTEXTMENU:
		{
			CHARRANGE sel, all = { 0, -1 };
			POINT pt;
			POINTL ptl;

			SendMessage(hwnd, EM_EXGETSEL, 0, (LPARAM)&sel);
			if (lParam == 0xFFFFFFFF) {
				SendMessage(hwnd, EM_POSFROMCHAR, (WPARAM)&pt, (LPARAM)sel.cpMax);
				ClientToScreen(hwnd, &pt);
			}
			else {
				pt.x = (short)LOWORD(lParam);
				pt.y = (short)HIWORD(lParam);
			}
			ptl.x = (LONG)pt.x;
			ptl.y = (LONG)pt.y;
			ScreenToClient(hwnd, (LPPOINT)&ptl);
			TCHAR *pszWord = GetRichTextWord(hwnd, &ptl);
			inMenu = TRUE;
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
					SetWindowText(hwnd, _T(""));
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
				pci->DoEventHookAsync(GetParent(hwnd), si->ptszID, si->pszModule, GC_USER_LOGMENU, NULL, NULL, (LPARAM)uID);
				break;
			}
			DestroyGCMenu(&hMenu, 5);
			mir_free(pszWord);
		}
		break;

	case WM_CHAR:
		SetFocus(GetDlgItem(GetParent(hwnd), IDC_CHAT_MESSAGE));
		SendMessage(GetDlgItem(GetParent(hwnd), IDC_CHAT_MESSAGE), WM_CHAR, wParam, lParam);
		break;
	}

	return mir_callNextSubclass(hwnd, LogSubclassProc, msg, wParam, lParam);
}

static void ProcessNickListHovering(HWND hwnd, int hoveredItem, POINT * pt, SESSION_INFO * parentdat)
{
	static int currentHovered = -1;
	static HWND hwndToolTip = NULL;
	static HWND oldParent = NULL;

	if (hoveredItem == currentHovered)
		return;

	currentHovered = hoveredItem;

	if (oldParent != hwnd && hwndToolTip) {
		SendMessage(hwndToolTip, TTM_DELTOOL, 0, 0);
		DestroyWindow(hwndToolTip);
		hwndToolTip = NULL;
	}
	if (hoveredItem == -1) {
		SendMessage(hwndToolTip, TTM_ACTIVATE, 0, 0);
		return;
	}

	BOOL bNewTip = FALSE;
	if (!hwndToolTip) {
		hwndToolTip = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL,
			WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,
			CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
			hwnd, NULL, g_hInst, NULL);
		bNewTip = TRUE;
	}

	RECT clientRect;
	GetClientRect(hwnd, &clientRect);
	TOOLINFO ti = { sizeof(TOOLINFO) };
	ti.uFlags = TTF_SUBCLASS;
	ti.hinst = g_hInst;
	ti.hwnd = hwnd;
	ti.uId = 1;
	ti.rect = clientRect;

	TCHAR tszBuf[1024]; tszBuf[0] = 0;
	USERINFO *ui = pci->SM_GetUserFromIndex(parentdat->ptszID, parentdat->pszModule, currentHovered);
	if (ui) {
		if (ProtoServiceExists(parentdat->pszModule, MS_GC_PROTO_GETTOOLTIPTEXT)) {
			TCHAR *p = (TCHAR*)ProtoCallService(parentdat->pszModule, MS_GC_PROTO_GETTOOLTIPTEXT, (WPARAM)parentdat->ptszID, (LPARAM)ui->pszUID);
			if (p != NULL) {
				_tcsncpy_s(tszBuf, p, _TRUNCATE);
				mir_free(p);
			}
		}

		if (tszBuf[0] == 0)
			mir_sntprintf(tszBuf, SIZEOF(tszBuf), _T("%s: %s\r\n%s: %s\r\n%s: %s"),
				TranslateT("Nickname"), ui->pszNick,
				TranslateT("Unique ID"), ui->pszUID,
				TranslateT("Status"), pci->TM_WordToString(parentdat->pStatuses, ui->Status));

		ti.lpszText = tszBuf;
	}

	SendMessage(hwndToolTip, bNewTip ? TTM_ADDTOOL : TTM_UPDATETIPTEXT, 0, (LPARAM)&ti);
	SendMessage(hwndToolTip, TTM_ACTIVATE, ti.lpszText != NULL, 0);
	SendMessage(hwndToolTip, TTM_SETMAXTIPWIDTH, 0, 400);
}

static LRESULT CALLBACK NicklistSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	SESSION_INFO *si = (SESSION_INFO*)GetWindowLongPtr(GetParent(hwnd), GWLP_USERDATA);
	int result = InputAreaShortcuts(hwnd, msg, wParam, lParam, &si->windowData);
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
				return CallService(MS_CLIST_MENUMEASUREITEM, wParam, lParam);
		}
		return FALSE;

	case WM_DRAWITEM:
		{
			DRAWITEMSTRUCT *dis = (DRAWITEMSTRUCT *)lParam;
			if (dis->CtlType == ODT_MENU)
				return CallService(MS_CLIST_MENUDRAWITEM, wParam, lParam);
		}
		return FALSE;

	case WM_CONTEXTMENU:
		{
			int height = 0;

			TVHITTESTINFO hti;
			hti.pt.x = (short)LOWORD(lParam);
			hti.pt.y = (short)HIWORD(lParam);
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
					pci->DoEventHookAsync(GetParent(hwnd), si->ptszID, si->pszModule, GC_USER_PRIVMESS, ui->pszUID, NULL, 0);
					break;

				default:
					pci->DoEventHookAsync(GetParent(hwnd), si->ptszID, si->pszModule, GC_USER_NICKLISTMENU, ui->pszUID, NULL, (LPARAM)uID);
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
				pci->DoEventHookAsync(GetParent(hwnd), si->ptszID, si->pszModule, GC_USER_PRIVMESS, ui->pszUID, NULL, 0);
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
			si->szSearch[mir_tstrlen(si->szSearch) - 1] = '\0';
		else if (wParam < ' ')
			break;
		else {
			TCHAR szNew[2];
			szNew[0] = (TCHAR)wParam;
			szNew[1] = '\0';
			if (mir_tstrlen(si->szSearch) >= SIZEOF(si->szSearch) - 2) {
				MessageBeep(MB_OK);
				break;
			}
			_tcscat(si->szSearch, szNew);
		}
		if (si->szSearch[0]) {
			/*
			* iterate over the (sorted) list of nicknames and search for the
			* string we have
			*/
			int iItems = SendMessage(hwnd, LB_GETCOUNT, 0, 0);
			for (int i = 0; i < iItems; i++) {
				USERINFO *ui = pci->UM_FindUserFromIndex(si->pUsers, i);
				if (ui) {
					if (!_tcsnicmp(ui->pszNick, si->szSearch, mir_tstrlen(si->szSearch))) {
						SendMessage(hwnd, LB_SETCURSEL, i, 0);
						InvalidateRect(hwnd, NULL, FALSE);
						return 0;
					}
				}
			}

			MessageBeep(MB_OK);
			si->szSearch[mir_tstrlen(si->szSearch) - 1] = '\0';
			return 0;
		}
		break;

	case WM_MOUSEMOVE:
		POINT pt = { LOWORD(lParam), HIWORD(lParam) };
		RECT clientRect;
		GetClientRect(hwnd, &clientRect);
		BOOL bInClient = PtInRect(&clientRect, pt);
		// Mouse capturing/releasing
		if (bInClient && GetCapture() != hwnd)
			SetCapture(hwnd);
		else if (!bInClient)
			ReleaseCapture();

		if (bInClient) {
			// hit test item under mouse
			DWORD nItemUnderMouse = (DWORD)SendMessage(hwnd, LB_ITEMFROMPOINT, 0, lParam);
			if (HIWORD(nItemUnderMouse) == 1)
				nItemUnderMouse = (DWORD)(-1);
			else
				nItemUnderMouse &= 0xFFFF;

			ProcessNickListHovering(hwnd, (int)nItemUnderMouse, &pt, si);
		}
		else ProcessNickListHovering(hwnd, -1, &pt, NULL);
	}

	return mir_callNextSubclass(hwnd, NicklistSubclassProc, msg, wParam, lParam);
}

int GetTextPixelSize(TCHAR* pszText, HFONT hFont, BOOL bWidth)
{
	if (!pszText || !hFont)
		return 0;

	HDC hdc = GetDC(NULL);
	HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);

	RECT rc = { 0 };
	int i = DrawText(hdc, pszText, -1, &rc, DT_CALCRECT);
	SelectObject(hdc, hOldFont);
	ReleaseDC(NULL, hdc);
	return bWidth ? rc.right - rc.left : rc.bottom - rc.top;
}

static void __cdecl phase2(void *lParam)
{
	SESSION_INFO *si = (SESSION_INFO*)lParam;
	Sleep(30);
	if (si && si->hWnd)
		PostMessage(si->hWnd, GC_REDRAWLOG3, 0, 0);
}

static INT_PTR CALLBACK RoomWndProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static HMENU hToolbarMenu;

	SESSION_INFO *si = (SESSION_INFO *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	if (!si && uMsg != WM_INITDIALOG)
		return FALSE;

	switch (uMsg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{
			SESSION_INFO *psi = (SESSION_INFO*)lParam;
			NotifyLocalWinEvent(psi->hContact, hwndDlg, MSG_WINDOW_EVT_OPENING);

			TranslateDialogDefault(hwndDlg);
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)psi);
			si = psi;
			RichUtil_SubClass(GetDlgItem(hwndDlg, IDC_CHAT_MESSAGE));
			RichUtil_SubClass(GetDlgItem(hwndDlg, IDC_CHAT_LOG));
			RichUtil_SubClass(GetDlgItem(hwndDlg, IDC_CHAT_LIST));
			mir_subclassWindow(GetDlgItem(hwndDlg, IDC_CHAT_SPLITTERX), SplitterSubclassProc);
			mir_subclassWindow(GetDlgItem(hwndDlg, IDC_CHAT_LIST), NicklistSubclassProc);
			mir_subclassWindow(GetDlgItem(hwndDlg, IDC_CHAT_LOG), LogSubclassProc);
			mir_subclassWindow(GetDlgItem(hwndDlg, IDC_CHAT_FILTER), ButtonSubclassProc);
			mir_subclassWindow(GetDlgItem(hwndDlg, IDC_CHAT_COLOR), ButtonSubclassProc);
			mir_subclassWindow(GetDlgItem(hwndDlg, IDC_CHAT_BKGCOLOR), ButtonSubclassProc);
			mir_subclassWindow(GetDlgItem(hwndDlg, IDC_CHAT_MESSAGE), MessageSubclassProc);

			RECT minEditInit;
			GetWindowRect(GetDlgItem(hwndDlg, IDC_CHAT_MESSAGE), &minEditInit);
			si->windowData.minEditBoxHeight = minEditInit.bottom - minEditInit.top;
			si->windowData.minLogBoxHeight = si->windowData.minEditBoxHeight;

			SendDlgItemMessage(hwndDlg, IDC_CHAT_MESSAGE, EM_SUBCLASSED, 0, 0);
			SendDlgItemMessage(hwndDlg, IDC_CHAT_LOG, EM_AUTOURLDETECT, 1, 0);
			int mask = (int)SendDlgItemMessage(hwndDlg, IDC_CHAT_LOG, EM_GETEVENTMASK, 0, 0);
			SendDlgItemMessage(hwndDlg, IDC_CHAT_LOG, EM_SETEVENTMASK, 0, mask | ENM_LINK | ENM_MOUSEEVENTS);
			SendDlgItemMessage(hwndDlg, IDC_CHAT_MESSAGE, EM_SETEVENTMASK, 0, ENM_MOUSEEVENTS | ENM_KEYEVENTS | ENM_CHANGE | ENM_REQUESTRESIZE);
			SendDlgItemMessage(hwndDlg, IDC_CHAT_LOG, EM_LIMITTEXT, (WPARAM)sizeof(TCHAR)* 0x7FFFFFFF, 0);
			SendDlgItemMessage(hwndDlg, IDC_CHAT_LOG, EM_SETOLECALLBACK, 0, (LPARAM)&reOleCallback);

			if (db_get_b(NULL, CHAT_MODULE, "UseIEView", 0)) {
				IEVIEWWINDOW ieWindow = { sizeof(ieWindow) };
				ieWindow.iType = IEW_CREATE;
				ieWindow.dwMode = IEWM_CHAT;
				ieWindow.parent = hwndDlg;
				ieWindow.cx = 200;
				ieWindow.cy = 300;
				CallService(MS_IEVIEW_WINDOW, 0, (LPARAM)&ieWindow);

				si->windowData.hwndLog = ieWindow.hwnd;

				IEVIEWEVENT iee = { sizeof(iee) };
				iee.iType = IEE_CLEAR_LOG;
				iee.hwnd = si->windowData.hwndLog;
				iee.hContact = si->hContact;
				iee.codepage = si->windowData.codePage;
				iee.pszProto = si->pszModule;
				CallService(MS_IEVIEW_EVENT, 0, (LPARAM)&iee);
			}

			EnableWindow(GetDlgItem(hwndDlg, IDC_CHAT_SMILEY), TRUE);

			SendMessage(GetDlgItem(hwndDlg, IDC_CHAT_LOG), EM_HIDESELECTION, TRUE, 0);

			SendMessage(hwndDlg, GC_SETWNDPROPS, 0, 0);
			SendMessage(hwndDlg, DM_UPDATESTATUSBAR, 0, 0);
			SendMessage(hwndDlg, DM_UPDATETITLEBAR, 0, 0);

			SendMessage(GetParent(hwndDlg), CM_ADDCHILD, (WPARAM)hwndDlg, (LPARAM)psi->hContact);
			PostMessage(hwndDlg, GC_UPDATENICKLIST, 0, 0);
			NotifyLocalWinEvent(psi->hContact, hwndDlg, MSG_WINDOW_EVT_OPEN);
		}
		break;

	case GC_SETWNDPROPS:
		// LoadGlobalSettings();
		InitButtons(hwndDlg, si);

		SendMessage(hwndDlg, DM_UPDATESTATUSBAR, 0, 0);
		SendMessage(hwndDlg, DM_UPDATETITLEBAR, 0, 0);
		SendMessage(hwndDlg, GC_FIXTABICONS, 0, 0);

		SendMessage(GetDlgItem(hwndDlg, IDC_CHAT_LOG), EM_SETBKGNDCOLOR, 0, g_Settings.crLogBackground);
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
			SendMessage(GetDlgItem(hwndDlg, IDC_CHAT_MESSAGE), EM_SETBKGNDCOLOR, 0, db_get_dw(NULL, SRMMMOD, SRMSGSET_INPUTBKGCOLOUR, SRMSGDEFSET_INPUTBKGCOLOUR));
			SendDlgItemMessage(hwndDlg, IDC_CHAT_MESSAGE, WM_SETFONT, (WPARAM)g_Settings.MessageBoxFont, MAKELPARAM(TRUE, 0));
			SendDlgItemMessage(hwndDlg, IDC_CHAT_MESSAGE, EM_SETCHARFORMAT, (WPARAM)SCF_ALL, (LPARAM)&cf);
		
			// nicklist
			int ih = GetTextPixelSize(_T("AQG_glo'"), g_Settings.UserListFont, FALSE);
			int ih2 = GetTextPixelSize(_T("AQG_glo'"), g_Settings.UserListHeadingsFont, FALSE);
			int height = db_get_b(NULL, CHAT_MODULE, "NicklistRowDist", 12);
			int font = ih > ih2 ? ih : ih2;
			// make sure we have space for icon!
			if (db_get_b(NULL, CHAT_MODULE, "ShowContactStatus", 0))
				font = font > 16 ? font : 16;

			SendMessage(GetDlgItem(hwndDlg, IDC_CHAT_LIST), LB_SETITEMHEIGHT, 0, (LPARAM)height > font ? height : font);
			InvalidateRect(GetDlgItem(hwndDlg, IDC_CHAT_LIST), NULL, TRUE);
		}
		SendDlgItemMessage(hwndDlg, IDC_MESSAGE, EM_REQUESTRESIZE, 0, 0);
		SendMessage(hwndDlg, WM_SIZE, 0, 0);
		SendMessage(hwndDlg, GC_REDRAWLOG2, 0, 0);
		break;

	case DM_UPDATETITLEBAR:
		{
			TitleBarData tbd = { 0 };
			TCHAR szTemp[100];
			if (g_dat.flags & SMF_STATUSICON) {
				MODULEINFO *mi = pci->MM_FindModule(si->pszModule);
				tbd.hIcon = (si->wStatus == ID_STATUS_ONLINE) ? mi->hOnlineIcon : mi->hOfflineIcon;
				tbd.hIconBig = (si->wStatus == ID_STATUS_ONLINE) ? mi->hOnlineIconBig : mi->hOfflineIconBig;
			}
			else {
				tbd.hIcon = GetCachedIcon("chat_window");
				tbd.hIconBig = g_dat.hIconChatBig;
			}
			tbd.hIconNot = (si->wState & (GC_EVENT_HIGHLIGHT | STATE_TALK)) ? GetCachedIcon("chat_overlay") : NULL;

			switch (si->iType) {
			case GCW_CHATROOM:
				mir_sntprintf(szTemp, SIZEOF(szTemp),
					(si->nUsersInNicklist == 1) ? TranslateT("%s: Chat Room (%u user)") : TranslateT("%s: Chat Room (%u users)"),
					si->ptszName, si->nUsersInNicklist);
				break;
			case GCW_PRIVMESS:
				mir_sntprintf(szTemp, SIZEOF(szTemp),
					(si->nUsersInNicklist == 1) ? TranslateT("%s: Message Session") : TranslateT("%s: Message Session (%u users)"),
					si->ptszName, si->nUsersInNicklist);
				break;
			case GCW_SERVER:
				mir_sntprintf(szTemp, SIZEOF(szTemp), _T("%s: Server"), si->ptszName);
				break;
			}
			tbd.iFlags = TBDF_TEXT | TBDF_ICON;
			tbd.pszText = szTemp;
			SendMessage(GetParent(hwndDlg), CM_UPDATETITLEBAR, (WPARAM)&tbd, (LPARAM)hwndDlg);
			SendMessage(hwndDlg, DM_UPDATETABCONTROL, 0, 0);
		}
		break;

	case DM_UPDATESTATUSBAR:
		{
			MODULEINFO *mi = pci->MM_FindModule(si->pszModule);
			TCHAR szTemp[512];
			HICON hIcon = si->wStatus == ID_STATUS_ONLINE ? mi->hOnlineIcon : mi->hOfflineIcon;
			mir_sntprintf(szTemp, SIZEOF(szTemp), _T("%s : %s"), mi->ptszModDispName, si->ptszStatusbarText ? si->ptszStatusbarText : _T(""));

			StatusBarData sbd;
			sbd.iItem = 0;
			sbd.iFlags = SBDF_TEXT | SBDF_ICON;
			sbd.hIcon = hIcon;
			sbd.pszText = szTemp;
			SendMessage(GetParent(hwndDlg), CM_UPDATESTATUSBAR, (WPARAM)&sbd, (LPARAM)hwndDlg);

			sbd.iItem = 1;
			sbd.hIcon = NULL;
			sbd.pszText = _T("");
			SendMessage(GetParent(hwndDlg), CM_UPDATESTATUSBAR, (WPARAM)&sbd, (LPARAM)hwndDlg);

			StatusIconData sid = { sizeof(sid) };
			sid.szModule = SRMMMOD;
			Srmm_ModifyIcon(si->hContact, &sid);
		}
		break;

	case DM_GETCODEPAGE:
		SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, si->windowData.codePage);
		return TRUE;

	case DM_SETCODEPAGE:
		si->windowData.codePage = (int)lParam;
		SendMessage(hwndDlg, GC_REDRAWLOG2, 0, 0);
		break;

	case DM_SWITCHINFOBAR:
	case DM_SWITCHTOOLBAR:
		SendMessage(hwndDlg, WM_SIZE, 0, 0);
		break;

	case WM_SIZE:
		if (wParam == SIZE_MAXIMIZED)
			PostMessage(hwndDlg, GC_SCROLLTOBOTTOM, 0, 0);

		if (IsIconic(hwndDlg)) break;

		if (wParam == SIZE_RESTORED || wParam == SIZE_MAXIMIZED) {
			int dlgWidth, dlgHeight;
			RECT rc;
			dlgWidth = LOWORD(lParam);
			dlgHeight = HIWORD(lParam);
			GetClientRect(hwndDlg, &rc);

			dlgWidth = rc.right - rc.left;
			dlgHeight = rc.bottom - rc.top;
			MessageDialogResize(hwndDlg, si, dlgWidth, dlgHeight);
		}
		break;

	case GC_REDRAWWINDOW:
		InvalidateRect(hwndDlg, NULL, TRUE);
		break;

	case GC_REDRAWLOG:
		si->LastTime = 0;
		if (si->pLog) {
			LOGINFO *pLog = si->pLog;
			if (si->iEventCount > 60) {
				int index = 0;
				while (index < 59) {
					if (pLog->next == NULL)
						break;

					pLog = pLog->next;
					if ((si->iType != GCW_CHATROOM && si->iType != GCW_PRIVMESS) || !si->bFilterEnabled || (si->iLogFilterFlags&pLog->iType) != 0)
						index++;
				}
				Log_StreamInEvent(hwndDlg, pLog, si, TRUE);
				mir_forkthread(phase2, si);
			}
			else Log_StreamInEvent(hwndDlg, si->pLogEnd, si, TRUE);
		}
		else SendMessage(hwndDlg, GC_EVENT_CONTROL + WM_USER + 500, WINDOW_CLEARLOG, 0);
		break;

	case GC_REDRAWLOG2:
		si->LastTime = 0;
		if (si->pLog)
			Log_StreamInEvent(hwndDlg, si->pLogEnd, si, TRUE);
		break;

	case GC_REDRAWLOG3:
		si->LastTime = 0;
		if (si->pLog)
			Log_StreamInEvent(hwndDlg, si->pLogEnd, si, TRUE);
		break;

	case GC_ADDLOG:
		if (si->pLogEnd)
			Log_StreamInEvent(hwndDlg, si->pLog, si, FALSE);
		else
			SendMessage(hwndDlg, GC_EVENT_CONTROL + WM_USER + 500, WINDOW_CLEARLOG, 0);
		break;

	case DM_UPDATETABCONTROL:
		{
			TabControlData tcd;
			tcd.iFlags = TCDF_TEXT;
			tcd.pszText = si->ptszName;
			SendMessage(GetParent(hwndDlg), CM_UPDATETABCONTROL, (WPARAM)&tcd, (LPARAM)hwndDlg);
		}
	case GC_FIXTABICONS:
		{
			HICON hIcon;
			if (!(si->wState & GC_EVENT_HIGHLIGHT)) {
				if (si->wState & STATE_TALK)
					hIcon = (si->wStatus == ID_STATUS_ONLINE) ? pci->MM_FindModule(si->pszModule)->hOnlineTalkIcon : pci->MM_FindModule(si->pszModule)->hOfflineTalkIcon;
				else
					hIcon = (si->wStatus == ID_STATUS_ONLINE) ? pci->MM_FindModule(si->pszModule)->hOnlineIcon : pci->MM_FindModule(si->pszModule)->hOfflineIcon;
			}
			else hIcon = g_dat.hMsgIcon;

			TabControlData tcd;
			tcd.iFlags = TCDF_ICON;
			tcd.hIcon = hIcon;
			SendMessage(GetParent(hwndDlg), CM_UPDATETABCONTROL, (WPARAM)&tcd, (LPARAM)hwndDlg);
		}
		break;

	case GC_SETMESSAGEHIGHLIGHT:
		si->wState |= GC_EVENT_HIGHLIGHT;
		SendMessage(si->hWnd, GC_FIXTABICONS, 0, 0);
		SendMessage(hwndDlg, DM_UPDATETITLEBAR, 0, 0);
		if (g_Settings.bFlashWindowHighlight && GetActiveWindow() != hwndDlg && GetForegroundWindow() != GetParent(hwndDlg))
			SendMessage(GetParent(si->hWnd), CM_STARTFLASHING, 0, 0);
		break;

	case GC_SETTABHIGHLIGHT:
		SendMessage(si->hWnd, GC_FIXTABICONS, 0, 0);
		SendMessage(hwndDlg, DM_UPDATETITLEBAR, 0, 0);
		if (g_Settings.bFlashWindow && GetActiveWindow() != GetParent(hwndDlg) && GetForegroundWindow() != GetParent(hwndDlg))
			SendMessage(GetParent(si->hWnd), CM_STARTFLASHING, 0, 0);
		break;

	case DM_ACTIVATE:
		if (si->wState & STATE_TALK) {
			si->wState &= ~STATE_TALK;
			db_set_w(si->hContact, si->pszModule, "ApparentMode", 0);
		}

		if (si->wState & GC_EVENT_HIGHLIGHT) {
			si->wState &= ~GC_EVENT_HIGHLIGHT;

			if (CallService(MS_CLIST_GETEVENT, (WPARAM)si->hContact, 0))
				CallService(MS_CLIST_REMOVEEVENT, (WPARAM)si->hContact, (LPARAM)GC_FAKE_EVENT);
		}

		SendMessage(hwndDlg, GC_FIXTABICONS, 0, 0);
		if (!si->hWnd) {
			ShowRoom(si, (WPARAM)WINDOW_VISIBLE, TRUE);
			SendMessage(hwndDlg, WM_MOUSEACTIVATE, 0, 0);
		}
		break;

	case GC_ACKMESSAGE:
		SendDlgItemMessage(hwndDlg, IDC_CHAT_MESSAGE, EM_SETREADONLY, FALSE, 0);
		SendDlgItemMessage(hwndDlg, IDC_CHAT_MESSAGE, WM_SETTEXT, 0, (LPARAM)_T(""));
		return TRUE;

	case WM_CTLCOLORLISTBOX:
		SetBkColor((HDC)wParam, g_Settings.crUserListBGColor);
		return (INT_PTR)pci->hListBkgBrush;

	case WM_MEASUREITEM:
		if (!MeasureMenuItem(wParam, lParam)) {
			MEASUREITEMSTRUCT *mis = (MEASUREITEMSTRUCT *)lParam;
			if (mis->CtlType == ODT_MENU)
				return CallService(MS_CLIST_MENUMEASUREITEM, wParam, lParam);

			int ih = GetTextPixelSize(_T("AQGgl'"), g_Settings.UserListFont, FALSE);
			int ih2 = GetTextPixelSize(_T("AQGg'"), g_Settings.UserListHeadingsFont, FALSE);
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
				return CallService(MS_CLIST_MENUDRAWITEM, wParam, lParam);

			if (dis->CtlID == IDC_CHAT_LIST) {
				int index = dis->itemID;
				USERINFO *ui = pci->SM_GetUserFromIndex(si->ptszID, si->pszModule, index);
				if (ui) {
					int x_offset = 2;

					int height = dis->rcItem.bottom - dis->rcItem.top;
					if (height & 1)
						height++;

					int offset = (height == 10) ? 0 : height / 2 - 5;
					HICON hIcon = pci->SM_GetStatusIcon(si, ui);
					HFONT hFont = (ui->iStatusEx == 0) ? g_Settings.UserListFont : g_Settings.UserListHeadingsFont;
					HFONT hOldFont = (HFONT)SelectObject(dis->hDC, hFont);
					SetBkMode(dis->hDC, TRANSPARENT);

					if (dis->itemAction == ODA_FOCUS && dis->itemState & ODS_SELECTED)
						FillRect(dis->hDC, &dis->rcItem, pci->hListSelectedBkgBrush);
					else //if (dis->itemState & ODS_INACTIVE)
						FillRect(dis->hDC, &dis->rcItem, pci->hListBkgBrush);

					if (g_Settings.bShowContactStatus && g_Settings.bContactStatusFirst && ui->ContactStatus) {
						HICON hIcon = LoadSkinnedProtoIcon(si->pszModule, ui->ContactStatus);
						DrawIconEx(dis->hDC, x_offset, dis->rcItem.top + offset - 3, hIcon, 16, 16, 0, NULL, DI_NORMAL);
						Skin_ReleaseIcon(hIcon);
						x_offset += 18;
					}
					DrawIconEx(dis->hDC, x_offset, dis->rcItem.top + offset, hIcon, 10, 10, 0, NULL, DI_NORMAL);
					x_offset += 12;
					if (g_Settings.bShowContactStatus && !g_Settings.bContactStatusFirst && ui->ContactStatus) {
						HICON hIcon = LoadSkinnedProtoIcon(si->pszModule, ui->ContactStatus);
						DrawIconEx(dis->hDC, x_offset, dis->rcItem.top + offset - 3, hIcon, 16, 16, 0, NULL, DI_NORMAL);
						Skin_ReleaseIcon(hIcon);
						x_offset += 18;
					}

					SetTextColor(dis->hDC, ui->iStatusEx == 0 ? g_Settings.crUserListColor : g_Settings.crUserListHeadingsColor);
					TextOut(dis->hDC, dis->rcItem.left + x_offset, dis->rcItem.top, ui->pszNick, mir_tstrlen(ui->pszNick));
					SelectObject(dis->hDC, hOldFont);
				}
				return TRUE;
			}
		}

	case GC_UPDATENICKLIST:
		SendDlgItemMessage(hwndDlg, IDC_CHAT_LIST, WM_SETREDRAW, FALSE, 0);
		SendMessage(GetDlgItem(hwndDlg, IDC_CHAT_LIST), LB_RESETCONTENT, 0, 0);
		for (int index = 0; index < si->nUsersInNicklist; index++) {
			USERINFO *ui = pci->SM_GetUserFromIndex(si->ptszID, si->pszModule, index);
			if (ui) {
				char szIndicator = SM_GetStatusIndicator(si, ui);
				if (szIndicator > '\0') {
					static TCHAR ptszBuf[128];
					mir_sntprintf(ptszBuf, SIZEOF(ptszBuf), _T("%c%s"), szIndicator, ui->pszNick);
					SendMessage(GetDlgItem(hwndDlg, IDC_CHAT_LIST), LB_ADDSTRING, 0, (LPARAM)ptszBuf);
				}
				else SendMessage(GetDlgItem(hwndDlg, IDC_CHAT_LIST), LB_ADDSTRING, 0, (LPARAM)ui->pszNick);
			}
		}
		SendDlgItemMessage(hwndDlg, IDC_CHAT_LIST, WM_SETREDRAW, TRUE, 0);
		InvalidateRect(GetDlgItem(hwndDlg, IDC_CHAT_LIST), NULL, FALSE);
		UpdateWindow(GetDlgItem(hwndDlg, IDC_CHAT_LIST));
		SendMessage(hwndDlg, DM_UPDATETITLEBAR, 0, 0);
		break;

	case GC_EVENT_CONTROL + WM_USER + 500:
		switch (wParam) {
		case SESSION_OFFLINE:
			SendMessage(hwndDlg, DM_UPDATESTATUSBAR, 0, 0);
			SendMessage(si->hWnd, GC_UPDATENICKLIST, 0, 0);
			return TRUE;

		case SESSION_ONLINE:
			SendMessage(hwndDlg, DM_UPDATESTATUSBAR, 0, 0);
			return TRUE;

		case WINDOW_HIDDEN:
			SendMessage(hwndDlg, GC_CLOSEWINDOW, 0, 0);
			return TRUE;

		case WINDOW_CLEARLOG:
			SetDlgItemText(hwndDlg, IDC_CHAT_LOG, _T(""));
			return TRUE;

		case SESSION_TERMINATE:
			if (CallService(MS_CLIST_GETEVENT, (WPARAM)si->hContact, 0))
				CallService(MS_CLIST_REMOVEEVENT, (WPARAM)si->hContact, (LPARAM)GC_FAKE_EVENT);
			si->wState &= ~STATE_TALK;
			db_set_w(si->hContact, si->pszModule, "ApparentMode", 0);
			SendMessage(hwndDlg, GC_CLOSEWINDOW, 0, 0);
			return TRUE;

		case WINDOW_MINIMIZE:
			ShowWindow(hwndDlg, SW_MINIMIZE);
			goto LABEL_SHOWWINDOW;

		case WINDOW_MAXIMIZE:
			ShowWindow(hwndDlg, SW_MAXIMIZE);
			goto LABEL_SHOWWINDOW;

		case SESSION_INITDONE:
			if (db_get_b(NULL, CHAT_MODULE, "PopupOnJoin", 0) != 0)
				return TRUE;
			// fall through
		case WINDOW_VISIBLE:
			if (IsIconic(hwndDlg))
				ShowWindow(hwndDlg, SW_NORMAL);
LABEL_SHOWWINDOW:
			SendMessage(hwndDlg, WM_SIZE, 0, 0);
			SendMessage(hwndDlg, GC_REDRAWLOG, 0, 0);
			SendMessage(hwndDlg, GC_UPDATENICKLIST, 0, 0);
			SendMessage(hwndDlg, DM_UPDATESTATUSBAR, 0, 0);
			ShowWindow(hwndDlg, SW_SHOW);
			SendMessage(hwndDlg, WM_SIZE, 0, 0);
			SetForegroundWindow(hwndDlg);
			return TRUE;
		}
		break;

	case GC_SPLITTERMOVED:
		if ((HWND)lParam == GetDlgItem(hwndDlg, IDC_CHAT_SPLITTERX)) {
			POINT pt;
			RECT rc;
			GetClientRect(hwndDlg, &rc);
			pt.x = wParam; pt.y = 0;
			ScreenToClient(hwndDlg, &pt);

			int oldSplitterX = si->iSplitterX;
			si->iSplitterX = rc.right - pt.x + 1;
			if (si->iSplitterX < 35)
				si->iSplitterX = 35;
			if (si->iSplitterX > rc.right - rc.left - 35)
				si->iSplitterX = rc.right - rc.left - 35;
			g_Settings.iSplitterX = si->iSplitterX;
		}
		else if ((HWND)lParam == GetDlgItem(hwndDlg, IDC_CHAT_SPLITTERY)) {
			POINT pt;
			RECT rc;
			GetClientRect(hwndDlg, &rc);
			pt.x = 0; pt.y = wParam;
			ScreenToClient(hwndDlg, &pt);
			int oldSplitterY = si->iSplitterY;
			si->iSplitterY = rc.bottom - pt.y;
			g_Settings.iSplitterY = si->iSplitterY;
		}
		PostMessage(hwndDlg, WM_SIZE, 0, 0);
		break;

	case GC_FIREHOOK:
		if (lParam) {
			GCHOOK *gch = (GCHOOK *)lParam;
			NotifyEventHooks(pci->hSendEvent, 0, (WPARAM)gch);
			if (gch->pDest) {
				mir_free((void*)gch->pDest->ptszID);
				mir_free((void*)gch->pDest->pszModule);
				mir_free(gch->pDest);
			}
			mir_free(gch->ptszText);
			mir_free(gch->ptszUID);
			mir_free(gch);
		}
		break;

	case GC_CHANGEFILTERFLAG:
		si->iLogFilterFlags = lParam;
		break;

	case GC_SHOWFILTERMENU:
		{
			RECT rc;
			HWND hwnd = CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_FILTER), hwndDlg, FilterWndProc, (LPARAM)si);
			TranslateDialogDefault(hwnd);
			GetWindowRect(GetDlgItem(hwndDlg, IDC_CHAT_FILTER), &rc);
			SetWindowPos(hwnd, HWND_TOP, rc.left - 85, (IsWindowVisible(GetDlgItem(hwndDlg, IDC_CHAT_FILTER)) || IsWindowVisible(GetDlgItem(hwndDlg, IDC_CHAT_BOLD))) ? rc.top - 206 : rc.top - 186, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
		}
		break;

	case GC_SHOWCOLORCHOOSER:
		{
			RECT rc;
			BOOL bFG = lParam == IDC_CHAT_COLOR ? TRUE : FALSE;
			COLORCHOOSER * pCC = (COLORCHOOSER *)mir_alloc(sizeof(COLORCHOOSER));
			GetWindowRect(GetDlgItem(hwndDlg, bFG ? IDC_CHAT_COLOR : IDC_CHAT_BKGCOLOR), &rc);
			pCC->hWndTarget = GetDlgItem(hwndDlg, IDC_CHAT_MESSAGE);
			pCC->pModule = pci->MM_FindModule(si->pszModule);
			pCC->xPosition = rc.left + 3;
			pCC->yPosition = IsWindowVisible(GetDlgItem(hwndDlg, IDC_CHAT_COLOR)) ? rc.top - 1 : rc.top + 20;
			pCC->bForeground = bFG;
			pCC->si = si;
			CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_COLORCHOOSER), hwndDlg, DlgProcColorToolWindow, (LPARAM)pCC);
		}
		break;

	case GC_SCROLLTOBOTTOM:
		{
			SCROLLINFO si = { 0 };
			if ((GetWindowLongPtr(GetDlgItem(hwndDlg, IDC_CHAT_LOG), GWL_STYLE) & WS_VSCROLL) != 0) {
				CHARRANGE sel;
				si.cbSize = sizeof(si);
				si.fMask = SIF_PAGE | SIF_RANGE;
				GetScrollInfo(GetDlgItem(hwndDlg, IDC_CHAT_LOG), SB_VERT, &si);
				si.fMask = SIF_POS;
				si.nPos = si.nMax - si.nPage + 1;
				SetScrollInfo(GetDlgItem(hwndDlg, IDC_CHAT_LOG), SB_VERT, &si, TRUE);
				sel.cpMin = sel.cpMax = GetRichTextLength(GetDlgItem(hwndDlg, IDC_CHAT_LOG), CP_ACP, FALSE);
				SendMessage(GetDlgItem(hwndDlg, IDC_CHAT_LOG), EM_EXSETSEL, 0, (LPARAM)&sel);
				PostMessage(GetDlgItem(hwndDlg, IDC_CHAT_LOG), WM_VSCROLL, MAKEWPARAM(SB_BOTTOM, 0), 0);
			}
		}
		break;

	case WM_ACTIVATE:
		if (LOWORD(wParam) != WA_ACTIVE)
			break;

		//fall through
	case WM_MOUSEACTIVATE:
		if (uMsg != WM_ACTIVATE)
			SetFocus(GetDlgItem(hwndDlg, IDC_CHAT_MESSAGE));

		pci->SetActiveSession(si->ptszID, si->pszModule);

		if (db_get_w(si->hContact, si->pszModule, "ApparentMode", 0) != 0)
			db_set_w(si->hContact, si->pszModule, "ApparentMode", 0);
		if (CallService(MS_CLIST_GETEVENT, (WPARAM)si->hContact, 0))
			CallService(MS_CLIST_REMOVEEVENT, (WPARAM)si->hContact, (LPARAM)GC_FAKE_EVENT);
		break;

	case WM_NOTIFY:
		{
			LPNMHDR pNmhdr = (LPNMHDR)lParam;
			switch (pNmhdr->code) {
			case EN_REQUESTRESIZE:
				if (pNmhdr->idFrom == IDC_CHAT_MESSAGE) {
					REQRESIZE *rr = (REQRESIZE *)lParam;
					int height = rr->rc.bottom - rr->rc.top + 1;
					if (height < g_dat.minInputAreaHeight)
						height = g_dat.minInputAreaHeight;

					if (si->desiredInputAreaHeight != height) {
						si->desiredInputAreaHeight = height;
						SendMessage(hwndDlg, WM_SIZE, 0, 0);
						PostMessage(hwndDlg, GC_SCROLLTOBOTTOM, 0, 0);
					}
				}
				break;

			case EN_MSGFILTER:
				if (pNmhdr->idFrom == IDC_CHAT_LOG && ((MSGFILTER *)lParam)->msg == WM_RBUTTONUP) {
					SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, TRUE);
					return TRUE;
				}
				break;

			case EN_LINK:
				if (pNmhdr->idFrom == IDC_CHAT_LOG) {
					switch (((ENLINK *)lParam)->msg) {
					case WM_RBUTTONDOWN:
					case WM_LBUTTONUP:
					case WM_LBUTTONDBLCLK:
						if (HandleLinkClick(g_hInst, hwndDlg, GetDlgItem(hwndDlg, IDC_CHAT_MESSAGE), (ENLINK*)lParam)) {
							SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, TRUE);
							return TRUE;
						}
						break;
					}
				}
				break;

			case TTN_NEEDTEXT:
				if (pNmhdr->idFrom == (UINT_PTR)GetDlgItem(hwndDlg, IDC_CHAT_LIST)) {
					LPNMTTDISPINFO lpttd = (LPNMTTDISPINFO)lParam;
					SESSION_INFO* parentdat = (SESSION_INFO*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

					POINT p;
					GetCursorPos(&p);
					ScreenToClient(GetDlgItem(hwndDlg, IDC_CHAT_LIST), &p);
					int item = LOWORD(SendMessage(GetDlgItem(hwndDlg, IDC_CHAT_LIST), LB_ITEMFROMPOINT, 0, MAKELPARAM(p.x, p.y)));
					USERINFO *ui = pci->SM_GetUserFromIndex(parentdat->ptszID, parentdat->pszModule, item);
					if (ui != NULL) {
						static TCHAR ptszBuf[1024];
						mir_sntprintf(ptszBuf, SIZEOF(ptszBuf), _T("%s: %s\r\n%s: %s\r\n%s: %s"),
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
		if (!lParam && CallService(MS_CLIST_MENUPROCESSCOMMAND, MAKEWPARAM(LOWORD(wParam), MPCF_CONTACTMENU), (LPARAM)si->hContact))
			break;

		switch (LOWORD(wParam)) {
		case IDC_CHAT_LIST:
			if (HIWORD(wParam) == LBN_DBLCLK) {
				TVHITTESTINFO hti;
				hti.pt.x = (short)LOWORD(GetMessagePos());
				hti.pt.y = (short)HIWORD(GetMessagePos());
				ScreenToClient(GetDlgItem(hwndDlg, IDC_CHAT_LIST), &hti.pt);

				int item = LOWORD(SendMessage(GetDlgItem(hwndDlg, IDC_CHAT_LIST), LB_ITEMFROMPOINT, 0, MAKELPARAM(hti.pt.x, hti.pt.y)));
				USERINFO *ui = pci->SM_GetUserFromIndex(si->ptszID, si->pszModule, item);
				if (ui) {
					if (GetKeyState(VK_SHIFT) & 0x8000) {
						LRESULT lResult = (LRESULT)SendMessage(GetDlgItem(hwndDlg, IDC_CHAT_MESSAGE), EM_GETSEL, 0, 0);
						int start = LOWORD(lResult);
						TCHAR* pszName = (TCHAR*)alloca(sizeof(TCHAR)*(mir_tstrlen(ui->pszUID) + 3));
						if (start == 0)
							mir_sntprintf(pszName, mir_tstrlen(ui->pszUID) + 3, _T("%s: "), ui->pszUID);
						else
							mir_sntprintf(pszName, mir_tstrlen(ui->pszUID) + 2, _T("%s "), ui->pszUID);

						SendMessage(GetDlgItem(hwndDlg, IDC_CHAT_MESSAGE), EM_REPLACESEL, FALSE, (LPARAM)pszName);
						PostMessage(hwndDlg, WM_MOUSEACTIVATE, 0, 0);
					}
					else pci->DoEventHookAsync(hwndDlg, si->ptszID, si->pszModule, GC_USER_PRIVMESS, ui->pszUID, NULL, 0);
				}

				return TRUE;
			}

			if (HIWORD(wParam) == LBN_KILLFOCUS)
				RedrawWindow(GetDlgItem(hwndDlg, IDC_CHAT_LIST), NULL, NULL, RDW_INVALIDATE);
			break;

		case IDOK:
			if (IsWindowEnabled(GetDlgItem(hwndDlg, IDOK))) {
				char *pszRtf = GetRichTextRTF(GetDlgItem(hwndDlg, IDC_CHAT_MESSAGE));

				TCmdList *cmdListNew = tcmdlist_last(si->windowData.cmdList);
				while (cmdListNew != NULL && cmdListNew->temporary) {
					si->windowData.cmdList = tcmdlist_remove(si->windowData.cmdList, cmdListNew);
					cmdListNew = tcmdlist_last(si->windowData.cmdList);
				}

				si->windowData.cmdList = tcmdlist_append(si->windowData.cmdList, pszRtf, 20, FALSE);
				TCHAR *ptszText = DoRtfToTags(pszRtf, si);
				rtrimt(ptszText);

				if (pci->MM_FindModule(si->pszModule)->bAckMsg) {
					EnableWindow(GetDlgItem(hwndDlg, IDC_CHAT_MESSAGE), FALSE);
					SendDlgItemMessage(hwndDlg, IDC_CHAT_MESSAGE, EM_SETREADONLY, TRUE, 0);
				}
				else SendDlgItemMessage(hwndDlg, IDC_CHAT_MESSAGE, WM_SETTEXT, 0, (LPARAM)_T(""));

				EnableWindow(GetDlgItem(hwndDlg, IDOK), FALSE);

				pci->DoEventHookAsync(hwndDlg, si->ptszID, si->pszModule, GC_USER_MESSAGE, NULL, ptszText, 0);
				mir_free(pszRtf);
				mir_free(ptszText);
				SetFocus(GetDlgItem(hwndDlg, IDC_CHAT_MESSAGE));
			}
			break;

		case IDC_CHAT_SHOWNICKLIST:
			if (!IsWindowEnabled(GetDlgItem(hwndDlg, IDC_CHAT_SHOWNICKLIST)))
				break;
			if (si->iType == GCW_SERVER)
				break;

			si->bNicklistEnabled = !si->bNicklistEnabled;

			SendDlgItemMessage(hwndDlg, IDC_CHAT_SHOWNICKLIST, BM_SETIMAGE, IMAGE_ICON, (LPARAM)GetCachedIcon(si->bNicklistEnabled ? "chat_nicklist" : "chat_nicklist2"));
			SendMessage(hwndDlg, GC_SCROLLTOBOTTOM, 0, 0);
			SendMessage(hwndDlg, WM_SIZE, 0, 0);
			break;

		case IDC_CHAT_MESSAGE:
			if (HIWORD(wParam) == EN_CHANGE) {
				si->windowData.cmdListCurrent = NULL;
				EnableWindow(GetDlgItem(hwndDlg, IDOK), GetRichTextLength(GetDlgItem(hwndDlg, IDC_CHAT_MESSAGE), si->windowData.codePage, FALSE) != 0);
			}
			break;

		case IDC_CHAT_SMILEY:
			{
				RECT rc;
				GetWindowRect(GetDlgItem(hwndDlg, IDC_CHAT_SMILEY), &rc);

				SMADD_SHOWSEL3 smaddInfo;
				smaddInfo.cbSize = sizeof(SMADD_SHOWSEL3);
				smaddInfo.hwndParent = GetParent(hwndDlg);
				smaddInfo.hwndTarget = GetDlgItem(hwndDlg, IDC_CHAT_MESSAGE);
				smaddInfo.targetMessage = EM_REPLACESEL;
				smaddInfo.targetWParam = TRUE;
				smaddInfo.Protocolname = si->pszModule;
				//smaddInfo.Direction = 3;
				smaddInfo.Direction = 0;
				smaddInfo.xPosition = rc.left;
				smaddInfo.yPosition = rc.bottom;
				smaddInfo.hContact = si->hContact;
				CallService(MS_SMILEYADD_SHOWSELECTION, 0, (LPARAM)&smaddInfo);
			}
			break;

		case IDC_CHAT_HISTORY:
			if (IsWindowEnabled(GetDlgItem(hwndDlg, IDC_CHAT_HISTORY))) {
				MODULEINFO *pInfo = pci->MM_FindModule(si->pszModule);
				if (pInfo)
					ShellExecute(hwndDlg, NULL, GetChatLogsFilename(si->hContact, 0), NULL, NULL, SW_SHOW);
			}
			break;

		case IDC_CHAT_CHANMGR:
			if (!IsWindowEnabled(GetDlgItem(hwndDlg, IDC_CHAT_CHANMGR)))
				break;
			pci->DoEventHookAsync(hwndDlg, si->ptszID, si->pszModule, GC_USER_CHANMGR, NULL, NULL, 0);
			break;

		case IDC_CHAT_FILTER:
			if (!IsWindowEnabled(GetDlgItem(hwndDlg, IDC_CHAT_FILTER)))
				break;

			si->bFilterEnabled = !si->bFilterEnabled;
			SendDlgItemMessage(hwndDlg, IDC_CHAT_FILTER, BM_SETIMAGE, IMAGE_ICON, (LPARAM)GetCachedIcon(si->bFilterEnabled ? "chat_filter" : "chat_filter2"));
			if (si->bFilterEnabled && db_get_b(NULL, CHAT_MODULE, "RightClickFilter", 0) == 0) {
				SendMessage(hwndDlg, GC_SHOWFILTERMENU, 0, 0);
				break;
			}
			SendMessage(hwndDlg, GC_REDRAWLOG, 0, 0);
			break;

		case IDC_CHAT_BKGCOLOR:
			if (IsWindowEnabled(GetDlgItem(hwndDlg, IDC_CHAT_BKGCOLOR))) {
				MODULEINFO *pInfo = pci->MM_FindModule(si->pszModule);
				CHARFORMAT2 cf;
				cf.cbSize = sizeof(CHARFORMAT2);
				cf.dwEffects = 0;

				if (IsDlgButtonChecked(hwndDlg, IDC_CHAT_BKGCOLOR)) {
					if (db_get_b(NULL, CHAT_MODULE, "RightClickFilter", 0) == 0)
						SendMessage(hwndDlg, GC_SHOWCOLORCHOOSER, 0, (LPARAM)IDC_CHAT_BKGCOLOR);
					else if (si->bBGSet) {
						cf.dwMask = CFM_BACKCOLOR;
						cf.crBackColor = pInfo->crColors[si->iBG];
						if (pInfo->bSingleFormat)
							SendDlgItemMessage(hwndDlg, IDC_CHAT_MESSAGE, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf);
						else
							SendDlgItemMessage(hwndDlg, IDC_CHAT_MESSAGE, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
					}
				}
				else {
					cf.dwMask = CFM_BACKCOLOR;
					cf.crBackColor = (COLORREF)db_get_dw(NULL, SRMMMOD, SRMSGSET_INPUTBKGCOLOUR, SRMSGDEFSET_INPUTBKGCOLOUR);
					if (pInfo->bSingleFormat)
						SendDlgItemMessage(hwndDlg, IDC_CHAT_MESSAGE, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf);
					else
						SendDlgItemMessage(hwndDlg, IDC_CHAT_MESSAGE, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
				}
			}
			break;

		case IDC_CHAT_COLOR:
			{
				MODULEINFO *pInfo = pci->MM_FindModule(si->pszModule);
				CHARFORMAT2 cf;
				cf.cbSize = sizeof(CHARFORMAT2);
				cf.dwEffects = 0;

				if (!IsWindowEnabled(GetDlgItem(hwndDlg, IDC_CHAT_COLOR)))
					break;

				if (IsDlgButtonChecked(hwndDlg, IDC_CHAT_COLOR)) {
					if (db_get_b(NULL, CHAT_MODULE, "RightClickFilter", 0) == 0)
						SendMessage(hwndDlg, GC_SHOWCOLORCHOOSER, 0, (LPARAM)IDC_CHAT_COLOR);
					else if (si->bFGSet) {
						cf.dwMask = CFM_COLOR;
						cf.crTextColor = pInfo->crColors[si->iFG];
						if (pInfo->bSingleFormat)
							SendDlgItemMessage(hwndDlg, IDC_CHAT_MESSAGE, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf);
						else
							SendDlgItemMessage(hwndDlg, IDC_CHAT_MESSAGE, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
					}
				}
				else {
					COLORREF cr;
					LoadMsgDlgFont(MSGFONTID_MESSAGEAREA, NULL, &cr);
					cf.dwMask = CFM_COLOR;
					cf.crTextColor = cr;
					if (pInfo->bSingleFormat)
						SendDlgItemMessage(hwndDlg, IDC_CHAT_MESSAGE, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf);
					else
						SendDlgItemMessage(hwndDlg, IDC_CHAT_MESSAGE, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
				}
			}
			break;

		case IDC_CHAT_BOLD:
		case IDC_CHAT_ITALICS:
		case IDC_CHAT_UNDERLINE:
			{
				MODULEINFO *pInfo = pci->MM_FindModule(si->pszModule);
				CHARFORMAT2 cf;
				cf.cbSize = sizeof(CHARFORMAT2);
				cf.dwMask = CFM_BOLD | CFM_ITALIC | CFM_UNDERLINE;
				cf.dwEffects = 0;

				if (LOWORD(wParam) == IDC_CHAT_BOLD && !IsWindowEnabled(GetDlgItem(hwndDlg, IDC_CHAT_BOLD)))
					break;
				if (LOWORD(wParam) == IDC_CHAT_ITALICS && !IsWindowEnabled(GetDlgItem(hwndDlg, IDC_CHAT_ITALICS)))
					break;
				if (LOWORD(wParam) == IDC_CHAT_UNDERLINE && !IsWindowEnabled(GetDlgItem(hwndDlg, IDC_CHAT_UNDERLINE)))
					break;
				if (IsDlgButtonChecked(hwndDlg, IDC_CHAT_BOLD))
					cf.dwEffects |= CFE_BOLD;
				if (IsDlgButtonChecked(hwndDlg, IDC_CHAT_ITALICS))
					cf.dwEffects |= CFE_ITALIC;
				if (IsDlgButtonChecked(hwndDlg, IDC_CHAT_UNDERLINE))
					cf.dwEffects |= CFE_UNDERLINE;
				if (pInfo->bSingleFormat)
					SendDlgItemMessage(hwndDlg, IDC_CHAT_MESSAGE, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf);
				else
					SendDlgItemMessage(hwndDlg, IDC_CHAT_MESSAGE, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
			}
			break;

		case IDCANCEL:
			PostMessage(hwndDlg, WM_CLOSE, 0, 0);
		}
		break;

	case WM_KEYDOWN:
		SetFocus(GetDlgItem(hwndDlg, IDC_CHAT_MESSAGE));
		break;

	case WM_GETMINMAXINFO:
		{
			MINMAXINFO* mmi = (MINMAXINFO*)lParam;
			mmi->ptMinTrackSize.x = si->iSplitterX + 43;
			if (mmi->ptMinTrackSize.x < 350)
				mmi->ptMinTrackSize.x = 350;

			mmi->ptMinTrackSize.y = si->windowData.minLogBoxHeight + TOOLBAR_HEIGHT + si->windowData.minEditBoxHeight + 5;
		}
		break;

	case WM_LBUTTONDBLCLK:
		if (LOWORD(lParam) < 30)
			PostMessage(hwndDlg, GC_SCROLLTOBOTTOM, 0, 0);
		else
			SendMessage(GetParent(hwndDlg), WM_SYSCOMMAND, SC_MINIMIZE, 0);
		break;

	case WM_LBUTTONDOWN:
		SendMessage(GetParent(hwndDlg), WM_LBUTTONDOWN, wParam, lParam);
		return TRUE;

	case WM_RBUTTONUP:
		{
			hToolbarMenu = CreatePopupMenu();
			for (int i = 0; i < SIZEOF(toolbarButtons); i++) {
				MENUITEMINFO mii = { sizeof(mii) };
				mii.fMask = MIIM_ID | MIIM_STRING | MIIM_STATE | MIIM_DATA | MIIM_BITMAP;
				mii.fType = MFT_STRING;
				mii.fState = (g_dat.chatBbuttonVisibility & (1 << i)) ? MFS_CHECKED : MFS_UNCHECKED;
				mii.wID = i + 1;
				mii.dwItemData = (ULONG_PTR)g_dat.hChatButtonIconList;
				mii.hbmpItem = HBMMENU_CALLBACK;
				mii.dwTypeData = TranslateTS((toolbarButtons[i].name));
				InsertMenuItem(hToolbarMenu, i, TRUE, &mii);
			}

			POINT pt;
			pt.x = (short)LOWORD(GetMessagePos());
			pt.y = (short)HIWORD(GetMessagePos());
			int res = TrackPopupMenu(hToolbarMenu, TPM_RETURNCMD, pt.x, pt.y, 0, hwndDlg, NULL);
			if (res > 0) {
				g_dat.chatBbuttonVisibility ^= (1 << (res - 1));
				db_set_dw(NULL, SRMMMOD, SRMSGSET_CHATBUTTONVISIBILITY, g_dat.chatBbuttonVisibility);
				pci->SM_BroadcastMessage(NULL, GC_SETWNDPROPS, 0, 0, TRUE);
			}
			DestroyMenu(hToolbarMenu);
		}
		return TRUE;

	case DM_GETCONTEXTMENU:
		{
			HMENU hMenu = (HMENU)CallService(MS_CLIST_MENUBUILDCONTACT, (WPARAM)si->hContact, 0);
			SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, (LONG_PTR)hMenu);
		}
		return TRUE;

	case WM_CONTEXTMENU:
		if (GetParent(hwndDlg) == (HWND)wParam) {
			POINT pt;
			HMENU hMenu = (HMENU)CallService(MS_CLIST_MENUBUILDCONTACT, (WPARAM)si->hContact, 0);
			GetCursorPos(&pt);
			TrackPopupMenu(hMenu, 0, pt.x, pt.y, 0, hwndDlg, NULL);
			DestroyMenu(hMenu);
		}
		break;

	case WM_CLOSE:
		SendMessage(hwndDlg, GC_CLOSEWINDOW, 0, 0);
		break;

	case GC_CLOSEWINDOW:
		DestroyWindow(hwndDlg);
		break;

	case WM_DESTROY:
		NotifyLocalWinEvent(si->hContact, hwndDlg, MSG_WINDOW_EVT_CLOSING);
		si->hWnd = NULL;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, 0);

		SendMessage(GetParent(hwndDlg), CM_REMOVECHILD, 0, (LPARAM)hwndDlg);
		if (si->windowData.hwndLog != NULL) {
			IEVIEWWINDOW ieWindow;
			ieWindow.cbSize = sizeof(IEVIEWWINDOW);
			ieWindow.iType = IEW_DESTROY;
			ieWindow.hwnd = si->windowData.hwndLog;
			CallService(MS_IEVIEW_WINDOW, 0, (LPARAM)&ieWindow);
		}

		NotifyLocalWinEvent(si->hContact, hwndDlg, MSG_WINDOW_EVT_CLOSE);
		break;
	}
	return FALSE;
}

void ShowRoom(SESSION_INFO *si, WPARAM wp, BOOL bSetForeground)
{
	if (si == NULL)
		return;

	si->windowData.hContact = si->hContact;

	//Do we need to create a window?
	if (si->hWnd == NULL) {
		HWND hParent = GetParentWindow(si->hContact, TRUE);
		si->hWnd = CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_CHANNEL), hParent, RoomWndProc, (LPARAM)si);
	}
	SendMessage(si->hWnd, DM_UPDATETABCONTROL, -1, (LPARAM)si);
	SendMessage(GetParent(si->hWnd), CM_ACTIVATECHILD, 0, (LPARAM)si->hWnd);
	SendMessage(GetParent(si->hWnd), CM_POPUPWINDOW, 0, (LPARAM)si->hWnd);
	SendMessage(si->hWnd, WM_MOUSEACTIVATE, 0, 0);
	SetFocus(GetDlgItem(si->hWnd, IDC_CHAT_MESSAGE));
}
