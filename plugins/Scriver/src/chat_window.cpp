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

#include "stdafx.h"

struct MESSAGESUBDATA
{
	time_t lastEnterTime;
	wchar_t *szSearchQuery;
	wchar_t *szSearchResult;
	SESSION_INFO *lastSession;
};

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
	wchar_t *pszName = nullptr;
	wchar_t* pszText = (wchar_t*)mir_alloc(iLen + 100 * sizeof(wchar_t));
	gt.cb = iLen + 99 * sizeof(wchar_t);
	gt.flags = GT_DEFAULT;

	SendMessage(hwnd, EM_GETTEXTEX, (WPARAM)&gt, (LPARAM)pszText);
	if (start > 1 && pszText[start - 1] == ' ' && pszText[start - 2] == ':')
		start -= 2;

	if (dat->szSearchResult != nullptr) {
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

	if (dat->szSearchQuery == nullptr) {
		dat->szSearchQuery = (wchar_t*)mir_alloc(sizeof(wchar_t)*(end - start + 1));
		mir_wstrncpy(dat->szSearchQuery, pszText + start, end - start + 1);
		dat->szSearchResult = mir_wstrdup(dat->szSearchQuery);
		dat->lastSession = nullptr;
	}

	if (isTopic)
		pszName = si->ptszTopic;
	else if (isRoom) {
		dat->lastSession = SM_FindSessionAutoComplete(si->pszModule, si, dat->lastSession, dat->szSearchQuery, dat->szSearchResult);
		if (dat->lastSession != nullptr)
			pszName = dat->lastSession->ptszName;
	}
	else pszName = pci->UM_FindUserAutoComplete(si->pUsers, dat->szSearchQuery, dat->szSearchResult);

	mir_free(pszText);
	replaceStrW(dat->szSearchResult, nullptr);

	if (pszName == nullptr) {
		if (end != start) {
			SendMessage(hwnd, EM_SETSEL, start, end);
			SendMessage(hwnd, EM_REPLACESEL, FALSE, (LPARAM)dat->szSearchQuery);
		}
		replaceStrW(dat->szSearchQuery, nullptr);
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

LRESULT CALLBACK CChatRoomDlg::MessageSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	BOOL isShift = GetKeyState(VK_SHIFT) & 0x8000;
	BOOL isCtrl = GetKeyState(VK_CONTROL) & 0x8000;
	BOOL isAlt = GetKeyState(VK_MENU) & 0x8000;

	MESSAGESUBDATA *dat = (MESSAGESUBDATA *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	CChatRoomDlg *pDlg = (CChatRoomDlg*)GetWindowLongPtr(GetParent(hwnd), GWLP_USERDATA);

	int result = InputAreaShortcuts(hwnd, msg, wParam, lParam, pDlg);
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
			dat->szSearchQuery = nullptr;
			mir_free(dat->szSearchResult);
			dat->szSearchResult = nullptr;
			if ((isCtrl != 0) ^ (0 != db_get_b(0, SRMMMOD, SRMSGSET_SENDONENTER, SRMSGDEFSET_SENDONENTER))) {
				PostMessage(GetParent(hwnd), WM_COMMAND, IDOK, 0);
				return 0;
			}
			if (db_get_b(0, SRMMMOD, SRMSGSET_SENDONDBLENTER, SRMSGDEFSET_SENDONDBLENTER)) {
				if (dat->lastEnterTime + 2 < time(nullptr))
					dat->lastEnterTime = time(nullptr);
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
			TabAutoComplete(hwnd, dat, pDlg->m_si);
			SendMessage(hwnd, WM_SETREDRAW, TRUE, 0);
			RedrawWindow(hwnd, nullptr, nullptr, RDW_INVALIDATE);
			return 0;
		}
		if (wParam != VK_RIGHT && wParam != VK_LEFT) {
			mir_free(dat->szSearchQuery);
			dat->szSearchQuery = nullptr;
			mir_free(dat->szSearchResult);
			dat->szSearchResult = nullptr;
		}
		if (wParam == 0x49 && isCtrl && !isAlt) { // ctrl-i (italics)
			CheckDlgButton(GetParent(hwnd), IDC_ITALICS, IsDlgButtonChecked(GetParent(hwnd), IDC_ITALICS) == BST_UNCHECKED ? BST_CHECKED : BST_UNCHECKED);
			SendMessage(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(IDC_ITALICS, 0), 0);
			return TRUE;
		}

		if (wParam == 0x42 && isCtrl && !isAlt) { // ctrl-b (bold)
			CheckDlgButton(GetParent(hwnd), IDC_BOLD, IsDlgButtonChecked(GetParent(hwnd), IDC_BOLD) == BST_UNCHECKED ? BST_CHECKED : BST_UNCHECKED);
			SendMessage(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(IDC_BOLD, 0), 0);
			return TRUE;
		}

		if (wParam == 0x55 && isCtrl && !isAlt) { // ctrl-u (paste clean text)
			CheckDlgButton(GetParent(hwnd), IDC_UNDERLINE, IsDlgButtonChecked(GetParent(hwnd), IDC_UNDERLINE) == BST_UNCHECKED ? BST_CHECKED : BST_UNCHECKED);
			SendMessage(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(IDC_UNDERLINE, 0), 0);
			return TRUE;
		}

		if (wParam == 0x4b && isCtrl && !isAlt) { // ctrl-k (paste clean text)
			CheckDlgButton(GetParent(hwnd), IDC_COLOR, IsDlgButtonChecked(GetParent(hwnd), IDC_COLOR) == BST_UNCHECKED ? BST_CHECKED : BST_UNCHECKED);
			SendMessage(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(IDC_COLOR, 0), 0);
			return TRUE;
		}

		if (wParam == VK_SPACE && isCtrl && !isAlt) { // ctrl-space (paste clean text)
			CheckDlgButton(GetParent(hwnd), IDC_BKGCOLOR, BST_UNCHECKED);
			CheckDlgButton(GetParent(hwnd), IDC_COLOR, BST_UNCHECKED);
			CheckDlgButton(GetParent(hwnd), IDC_BOLD, BST_UNCHECKED);
			CheckDlgButton(GetParent(hwnd), IDC_UNDERLINE, BST_UNCHECKED);
			CheckDlgButton(GetParent(hwnd), IDC_ITALICS, BST_UNCHECKED);
			SendMessage(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(IDC_BKGCOLOR, 0), 0);
			SendMessage(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(IDC_COLOR, 0), 0);
			SendMessage(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(IDC_BOLD, 0), 0);
			SendMessage(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(IDC_UNDERLINE, 0), 0);
			SendMessage(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(IDC_ITALICS, 0), 0);
			return TRUE;
		}

		if (wParam == 0x4c && isCtrl && !isAlt) { // ctrl-l (paste clean text)
			CheckDlgButton(GetParent(hwnd), IDC_BKGCOLOR, IsDlgButtonChecked(GetParent(hwnd), IDC_BKGCOLOR) == BST_UNCHECKED ? BST_CHECKED : BST_UNCHECKED);
			SendMessage(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(IDC_BKGCOLOR, 0), 0);
			return TRUE;
		}

		if (wParam == 0x46 && isCtrl && !isAlt) { // ctrl-f (paste clean text)
			if (IsWindowEnabled(GetDlgItem(GetParent(hwnd), IDC_FILTER)))
				SendMessage(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(IDC_FILTER, 0), 0);
			return TRUE;
		}

		if (wParam == 0x4e && isCtrl && !isAlt) { // ctrl-n (nicklist)
			if (IsWindowEnabled(GetDlgItem(GetParent(hwnd), IDC_SHOWNICKLIST)))
				SendMessage(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(IDC_SHOWNICKLIST, 0), 0);
			return TRUE;
		}

		if (wParam == 0x48 && isCtrl && !isAlt) { // ctrl-h (history)
			SendMessage(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(IDC_HISTORY, 0), 0);
			return TRUE;
		}

		if (wParam == 0x4f && isCtrl && !isAlt) { // ctrl-o (options)
			if (IsWindowEnabled(GetDlgItem(GetParent(hwnd), IDC_CHANMGR)))
				SendMessage(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(IDC_CHANMGR, 0), 0);
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
		InputAreaContextMenu(hwnd, wParam, lParam, pDlg->m_hContact);
		return TRUE;

	case WM_KEYUP:
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
		{
			UINT u = 0;
			UINT u2 = 0;
			COLORREF cr;
			SESSION_INFO *si = pDlg->m_si;

			LoadMsgDlgFont(MSGFONTID_MESSAGEAREA, nullptr, &cr);

			CHARFORMAT2 cf;
			cf.cbSize = sizeof(CHARFORMAT2);
			cf.dwMask = CFM_BOLD | CFM_ITALIC | CFM_UNDERLINE | CFM_BACKCOLOR | CFM_COLOR;
			SendMessage(hwnd, EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);

			if (pci->MM_FindModule(si->pszModule) && pci->MM_FindModule(si->pszModule)->bColor) {
				int index = pci->GetColorIndex(si->pszModule, cf.crTextColor);
				u = IsDlgButtonChecked(GetParent(hwnd), IDC_COLOR);

				if (index >= 0) {
					pDlg->m_bFGSet = true;
					pDlg->m_iFG = index;
				}

				if (u == BST_UNCHECKED && cf.crTextColor != cr)
					CheckDlgButton(GetParent(hwnd), IDC_COLOR, BST_CHECKED);
				else if (u == BST_CHECKED && cf.crTextColor == cr)
					CheckDlgButton(GetParent(hwnd), IDC_COLOR, BST_UNCHECKED);
			}

			if (pci->MM_FindModule(si->pszModule) && pci->MM_FindModule(si->pszModule)->bBkgColor) {
				int index = pci->GetColorIndex(si->pszModule, cf.crBackColor);
				COLORREF crB = db_get_dw(0, SRMMMOD, SRMSGSET_INPUTBKGCOLOUR, SRMSGDEFSET_INPUTBKGCOLOUR);
				u = IsDlgButtonChecked(GetParent(hwnd), IDC_BKGCOLOR);

				if (index >= 0) {
					pDlg->m_bBGSet = TRUE;
					pDlg->m_iBG = index;
				}
				if (u == BST_UNCHECKED && cf.crBackColor != crB)
					CheckDlgButton(GetParent(hwnd), IDC_BKGCOLOR, BST_CHECKED);
				else if (u == BST_CHECKED && cf.crBackColor == crB)
					CheckDlgButton(GetParent(hwnd), IDC_BKGCOLOR, BST_UNCHECKED);
			}

			if (pci->MM_FindModule(si->pszModule) && pci->MM_FindModule(si->pszModule)->bBold) {
				u = IsDlgButtonChecked(GetParent(hwnd), IDC_BOLD);
				u2 = cf.dwEffects;
				u2 &= CFE_BOLD;
				if (u == BST_UNCHECKED && u2)
					CheckDlgButton(GetParent(hwnd), IDC_BOLD, BST_CHECKED);
				else if (u == BST_CHECKED && u2 == 0)
					CheckDlgButton(GetParent(hwnd), IDC_BOLD, BST_UNCHECKED);
			}

			if (pci->MM_FindModule(si->pszModule) && pci->MM_FindModule(si->pszModule)->bItalics) {
				u = IsDlgButtonChecked(GetParent(hwnd), IDC_ITALICS);
				u2 = cf.dwEffects;
				u2 &= CFE_ITALIC;
				if (u == BST_UNCHECKED && u2)
					CheckDlgButton(GetParent(hwnd), IDC_ITALICS, BST_CHECKED);
				else if (u == BST_CHECKED && u2 == 0)
					CheckDlgButton(GetParent(hwnd), IDC_ITALICS, BST_UNCHECKED);
			}

			if (pci->MM_FindModule(si->pszModule) && pci->MM_FindModule(si->pszModule)->bUnderline) {
				u = IsDlgButtonChecked(GetParent(hwnd), IDC_UNDERLINE);
				u2 = cf.dwEffects;
				u2 &= CFE_UNDERLINE;
				if (u == BST_UNCHECKED && u2)
					CheckDlgButton(GetParent(hwnd), IDC_UNDERLINE, BST_CHECKED);
				else if (u == BST_CHECKED && u2 == 0)
					CheckDlgButton(GetParent(hwnd), IDC_UNDERLINE, BST_UNCHECKED);
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

INT_PTR CALLBACK CChatRoomDlg::FilterWndProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static CChatRoomDlg *pDlg = nullptr;
	switch (uMsg) {
	case WM_INITDIALOG:
		pDlg = (CChatRoomDlg*)lParam;
		CheckDlgButton(hwndDlg, IDC_CHAT_1, pDlg->m_iLogFilterFlags & GC_EVENT_ACTION ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHAT_2, pDlg->m_iLogFilterFlags & GC_EVENT_MESSAGE ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHAT_3, pDlg->m_iLogFilterFlags & GC_EVENT_NICK ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHAT_4, pDlg->m_iLogFilterFlags & GC_EVENT_JOIN ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHAT_5, pDlg->m_iLogFilterFlags & GC_EVENT_PART ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHAT_6, pDlg->m_iLogFilterFlags & GC_EVENT_TOPIC ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHAT_7, pDlg->m_iLogFilterFlags & GC_EVENT_ADDSTATUS ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHAT_8, pDlg->m_iLogFilterFlags & GC_EVENT_INFORMATION ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHAT_9, pDlg->m_iLogFilterFlags & GC_EVENT_QUIT ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHAT_10, pDlg->m_iLogFilterFlags & GC_EVENT_KICK ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHAT_11, pDlg->m_iLogFilterFlags & GC_EVENT_NOTICE ? BST_CHECKED : BST_UNCHECKED);
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

			pDlg->m_iLogFilterFlags = iFlags;
			if (pDlg->m_bFilterEnabled)
				pDlg->RedrawLog();
			PostMessage(hwndDlg, WM_CLOSE, 0, 0);
		}
		break;

	case WM_CLOSE:
		DestroyWindow(hwndDlg);
		break;
	}

	return FALSE;
}

LRESULT CALLBACK CChatRoomDlg::LogSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static BOOL inMenu = FALSE;
	CChatRoomDlg *pDlg = (CChatRoomDlg*)GetWindowLongPtr(GetParent(hwnd), GWLP_USERDATA);

	int result = InputAreaShortcuts(hwnd, msg, wParam, lParam, pDlg);
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
			SetCursor(LoadCursor(nullptr, IDC_ARROW));
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

			SESSION_INFO *si = pDlg->m_si;
			CHARRANGE all = { 0, -1 };
			HMENU hMenu = nullptr;
			UINT uID = CreateGCMenu(hwnd, &hMenu, 1, pt, si, nullptr, pszWord);
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
				pDlg->DoEventHook(GC_USER_LOGMENU, nullptr, nullptr, uID);
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

LRESULT CALLBACK CChatRoomDlg::NicklistSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CChatRoomDlg *pDlg = (CChatRoomDlg*)GetWindowLongPtr(GetParent(hwnd), GWLP_USERDATA);

	int result = InputAreaShortcuts(hwnd, msg, wParam, lParam, pDlg);
	if (result != -1)
		return result;

	switch (msg) {
	case WM_ERASEBKGND:
		{
			HDC dc = (HDC)wParam;
			if (dc) {
				int index = SendMessage(hwnd, LB_GETTOPINDEX, 0, 0);
				if (index == LB_ERR || pDlg->m_si->nUsersInNicklist <= 0)
					return 0;

				int items = pDlg->m_si->nUsersInNicklist - index;
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

			USERINFO *ui = pci->SM_GetUserFromIndex(pDlg->m_si->ptszID, pDlg->m_si->pszModule, (int)item);
			if (ui) {
				HMENU hMenu = 0;
				UINT uID;
				USERINFO uinew;

				memcpy(&uinew, ui, sizeof(USERINFO));
				if (hti.pt.x == -1 && hti.pt.y == -1)
					hti.pt.y += height - 4;
				ClientToScreen(hwnd, &hti.pt);
				uID = CreateGCMenu(hwnd, &hMenu, 0, hti.pt, pDlg->m_si, uinew.pszUID, uinew.pszNick);

				switch (uID) {
				case 0:
					break;

				case ID_MESS:
					pDlg->DoEventHook(GC_USER_PRIVMESS, ui, nullptr, 0);
					break;

				default:
					pDlg->DoEventHook(GC_USER_NICKLISTMENU, ui, nullptr, uID);
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
			if ((lpmsg = (LPMSG)lParam) != nullptr) {
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
				USERINFO *ui = pci->SM_GetUserFromIndex(pDlg->m_si->ptszID, pDlg->m_si->pszModule, index);
				pDlg->DoEventHook(GC_USER_PRIVMESS, ui, nullptr, 0);
			}
			break;
		}
		if (wParam == VK_ESCAPE || wParam == VK_UP || wParam == VK_DOWN || wParam == VK_NEXT ||
			wParam == VK_PRIOR || wParam == VK_TAB || wParam == VK_HOME || wParam == VK_END) {
			pDlg->m_wszSearch[0] = 0;
		}
		break;

	case WM_CHAR:
	case WM_UNICHAR:
		/*
		* simple incremental search for the user (nick) - list control
		* typing esc or movement keys will clear the current search string
		*/
		if (wParam == 27 && pDlg->m_wszSearch[0]) {						// escape - reset everything
			pDlg->m_wszSearch[0] = 0;
			break;
		}
		else if (wParam == '\b' && pDlg->m_wszSearch[0])					// backspace
			pDlg->m_wszSearch[mir_wstrlen(pDlg->m_wszSearch) - 1] = '\0';
		else if (wParam < ' ')
			break;
		else {
			wchar_t szNew[2];
			szNew[0] = (wchar_t)wParam;
			szNew[1] = '\0';
			if (mir_wstrlen(pDlg->m_wszSearch) >= _countof(pDlg->m_wszSearch) - 2) {
				MessageBeep(MB_OK);
				break;
			}
			mir_wstrcat(pDlg->m_wszSearch, szNew);
		}
		if (pDlg->m_wszSearch[0]) {
			// iterate over the (sorted) list of nicknames and search for the
			// string we have
			int iItems = SendMessage(hwnd, LB_GETCOUNT, 0, 0);
			for (int i = 0; i < iItems; i++) {
				USERINFO *ui = pci->UM_FindUserFromIndex(pDlg->m_si->pUsers, i);
				if (ui) {
					if (!wcsnicmp(ui->pszNick, pDlg->m_wszSearch, mir_wstrlen(pDlg->m_wszSearch))) {
						SendMessage(hwnd, LB_SETCURSEL, i, 0);
						InvalidateRect(hwnd, nullptr, FALSE);
						return 0;
					}
				}
			}

			MessageBeep(MB_OK);
			pDlg->m_wszSearch[mir_wstrlen(pDlg->m_wszSearch) - 1] = '\0';
			return 0;
		}
		break;

	case WM_MOUSEMOVE:
		Chat_HoverMouse(pDlg->m_si, hwnd, lParam, ServiceExists("mToolTip/HideTip"));
		break;
	}

	return mir_callNextSubclass(hwnd, NicklistSubclassProc, msg, wParam, lParam);
}

int GetTextPixelSize(wchar_t* pszText, HFONT hFont, BOOL bWidth)
{
	if (!pszText || !hFont)
		return 0;

	HDC hdc = GetDC(nullptr);
	HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);

	RECT rc = { 0 };
	DrawText(hdc, pszText, -1, &rc, DT_CALCRECT);
	SelectObject(hdc, hOldFont);
	ReleaseDC(nullptr, hdc);
	return bWidth ? rc.right - rc.left : rc.bottom - rc.top;
}

static void __cdecl phase2(void *lParam)
{
	Thread_SetName("Scriver: phase2");

	SESSION_INFO *si = (SESSION_INFO*)lParam;
	Sleep(30);
	if (si && si->pDlg)
		si->pDlg->RedrawLog2();
}

void CChatRoomDlg::FixTabIcons()
{
	HICON hIcon;
	if (!(m_si->wState & GC_EVENT_HIGHLIGHT)) {
		if (m_si->wState & STATE_TALK)
			hIcon = (m_si->wStatus == ID_STATUS_ONLINE) ? pci->MM_FindModule(m_si->pszModule)->hOnlineTalkIcon : pci->MM_FindModule(m_si->pszModule)->hOfflineTalkIcon;
		else
			hIcon = (m_si->wStatus == ID_STATUS_ONLINE) ? pci->MM_FindModule(m_si->pszModule)->hOnlineIcon : pci->MM_FindModule(m_si->pszModule)->hOfflineIcon;
	}
	else hIcon = g_dat.hMsgIcon;

	TabControlData tcd = {};
	tcd.iFlags = TCDF_ICON;
	tcd.hIcon = hIcon;
	SendMessage(m_hwndParent, CM_UPDATETABCONTROL, (WPARAM)&tcd, (LPARAM)m_hwnd);
}

/////////////////////////////////////////////////////////////////////////////////////////

void CChatRoomDlg::MessageDialogResize(int w, int h)
{
	bool bNick = m_si->iType != GCW_SERVER && m_bNicklistEnabled;
	bool bToolbar = SendMessage(m_hwndParent, CM_GETTOOLBARSTATUS, 0, 0) != 0;
	int  hSplitterMinTop = TOOLBAR_HEIGHT + m_minLogBoxHeight, hSplitterMinBottom = m_minEditBoxHeight;
	int  toolbarHeight = bToolbar ? TOOLBAR_HEIGHT : 0;

	if (h - m_pParent->iSplitterY < hSplitterMinTop)
		m_pParent->iSplitterY = h - hSplitterMinTop;
	if (m_pParent->iSplitterY < hSplitterMinBottom)
		m_pParent->iSplitterY = hSplitterMinBottom;

	ShowWindow(m_splitterX.GetHwnd(), bNick ? SW_SHOW : SW_HIDE);
	if (m_si->iType != GCW_SERVER)
		ShowWindow(m_nickList.GetHwnd(), m_bNicklistEnabled ? SW_SHOW : SW_HIDE);
	else
		ShowWindow(m_nickList.GetHwnd(), SW_HIDE);

	if (m_si->iType == GCW_SERVER) {
		m_btnShowList.Enable(false);
		m_btnFilter.Enable(false);
		m_btnChanMgr.Enable(false);
	}
	else {
		m_btnShowList.Enable(true);
		m_btnFilter.Enable(true);
		if (m_si->iType == GCW_CHATROOM)
			m_btnChanMgr.Enable(pci->MM_FindModule(m_si->pszModule)->bChanMgr);
	}

	int toolbarTopY = bToolbar ? h - m_pParent->iSplitterY - toolbarHeight : h - m_pParent->iSplitterY;
	int logBottom = (m_hwndIeview != nullptr) ? toolbarTopY / 2 : toolbarTopY;

	HDWP hdwp = BeginDeferWindowPos(5);
	hdwp = DeferWindowPos(hdwp, m_log.GetHwnd(), 0, 1, 0, bNick ? w - m_pParent->iSplitterX - 1 : w - 2, logBottom, SWP_NOZORDER);
	hdwp = DeferWindowPos(hdwp, m_nickList.GetHwnd(), 0, w - m_pParent->iSplitterX + 2, 0, m_pParent->iSplitterX - 3, toolbarTopY, SWP_NOZORDER);
	hdwp = DeferWindowPos(hdwp, m_splitterX.GetHwnd(), 0, w - m_pParent->iSplitterX, 1, 2, toolbarTopY - 1, SWP_NOZORDER);
	hdwp = DeferWindowPos(hdwp, m_splitterY.GetHwnd(), 0, 0, h - m_pParent->iSplitterY, w, SPLITTER_HEIGHT, SWP_NOZORDER);
	hdwp = DeferWindowPos(hdwp, m_message.GetHwnd(), 0, 1, h - m_pParent->iSplitterY + SPLITTER_HEIGHT, w - 2, m_pParent->iSplitterY - SPLITTER_HEIGHT - 1, SWP_NOZORDER);
	EndDeferWindowPos(hdwp);

	SetButtonsPos(m_hwnd, m_hContact, bToolbar);

	if (m_hwndIeview != nullptr) {
		IEVIEWWINDOW ieWindow;
		ieWindow.cbSize = sizeof(IEVIEWWINDOW);
		ieWindow.iType = IEW_SETPOS;
		ieWindow.parent = m_hwnd;
		ieWindow.hwnd = m_hwndIeview;
		ieWindow.x = 0;
		ieWindow.y = logBottom + 1;
		ieWindow.cx = bNick ? w - m_pParent->iSplitterX : w;
		ieWindow.cy = logBottom;
		CallService(MS_IEVIEW_WINDOW, 0, (LPARAM)&ieWindow);
	}
	else RedrawWindow(m_log.GetHwnd(), nullptr, nullptr, RDW_INVALIDATE);

	RedrawWindow(m_nickList.GetHwnd(), nullptr, nullptr, RDW_INVALIDATE);
	RedrawWindow(m_message.GetHwnd(), nullptr, nullptr, RDW_INVALIDATE);
}

CChatRoomDlg::CChatRoomDlg(SESSION_INFO *si)
	: CScriverWindow(IDD_CHANNEL, si),

	m_log(this, IDC_LOG),
	m_message(this, IDC_MESSAGE),
	m_nickList(this, IDC_CHAT_LIST),

	m_splitterX(this, IDC_SPLITTERX),
	m_splitterY(this, IDC_SPLITTERY),

	m_btnOk(this, IDOK),
	m_btnBold(this, IDC_BOLD),
	m_btnColor(this, IDC_COLOR),
	m_btnFilter(this, IDC_FILTER),
	m_btnItalic(this, IDC_ITALICS),
	m_btnHistory(this, IDC_HISTORY),
	m_btnChanMgr(this, IDC_CHANMGR),
	m_btnBkColor(this, IDC_BKGCOLOR),
	m_btnShowList(this, IDC_SHOWNICKLIST),
	m_btnUnderline(this, IDC_UNDERLINE)
{
	m_pLog = &m_log;
	m_pEntry = &m_message;
	m_pColor = &m_btnColor;
	m_pBkColor = &m_btnBkColor;
	m_pFilter = &m_btnFilter;

	m_btnOk.OnClick = Callback(this, &CChatRoomDlg::onClick_Ok);
	m_btnFilter.OnClick = Callback(this, &CChatRoomDlg::onClick_Filter);
	m_btnHistory.OnClick = Callback(this, &CChatRoomDlg::onClick_History);
	m_btnChanMgr.OnClick = Callback(this, &CChatRoomDlg::onClick_ChanMgr);
	m_btnShowList.OnClick = Callback(this, &CChatRoomDlg::onClick_ShowList);

	m_btnBold.OnClick = Callback(this, &CChatRoomDlg::onClick_BIU);
	m_btnItalic.OnClick = Callback(this, &CChatRoomDlg::onClick_BIU);
	m_btnUnderline.OnClick = Callback(this, &CChatRoomDlg::onClick_BIU);

	m_btnColor.OnClick = Callback(this, &CChatRoomDlg::onClick_Color);
	m_btnBkColor.OnClick = Callback(this, &CChatRoomDlg::onClick_BkColor);

	m_nickList.OnDblClick = Callback(this, &CChatRoomDlg::onDblClick_List);

	m_message.OnChange = Callback(this, &CChatRoomDlg::onChange_Message);
	
	m_splitterX.OnChange = Callback(this, &CChatRoomDlg::OnSplitterX);
	m_splitterY.OnChange = Callback(this, &CChatRoomDlg::OnSplitterY);
}

void CChatRoomDlg::OnInitDialog()
{
	m_si->pDlg = this;

	NotifyLocalWinEvent(m_hContact, m_hwnd, MSG_WINDOW_EVT_OPENING);

	SetWindowLongPtr(m_hwnd, GWLP_USERDATA, (LONG_PTR)this);
	m_pParent = (ParentWindowData *)GetWindowLongPtr(m_hwndParent, GWLP_USERDATA);

	RichUtil_SubClass(m_message.GetHwnd());
	RichUtil_SubClass(m_log.GetHwnd());
	RichUtil_SubClass(m_nickList.GetHwnd());

	mir_subclassWindow(m_log.GetHwnd(), LogSubclassProc);
	mir_subclassWindow(m_message.GetHwnd(), MessageSubclassProc);
	mir_subclassWindow(m_nickList.GetHwnd(), NicklistSubclassProc);

	mir_subclassWindow(m_btnFilter.GetHwnd(), Srmm_ButtonSubclassProc);
	mir_subclassWindow(m_btnColor.GetHwnd(), Srmm_ButtonSubclassProc);
	mir_subclassWindow(m_btnBkColor.GetHwnd(), Srmm_ButtonSubclassProc);

	Srmm_CreateToolbarIcons(m_hwnd, BBBF_ISCHATBUTTON);

	RECT rc;
	GetWindowRect(m_message.GetHwnd(), &rc);
	m_minLogBoxHeight = m_minEditBoxHeight = rc.bottom - rc.top;
	if (m_pParent->iSplitterY == -1)
		m_pParent->iSplitterY = m_minEditBoxHeight;

	if (m_pParent->iSplitterX == -1) {
		GetWindowRect(m_nickList.GetHwnd(), &rc);
		m_pParent->iSplitterX = rc.right - rc.left;
	}

	m_message.SendMsg(EM_SUBCLASSED, 0, 0);
	m_message.SendMsg(EM_SETEVENTMASK, 0, ENM_MOUSEEVENTS | ENM_KEYEVENTS | ENM_CHANGE | ENM_REQUESTRESIZE);

	int mask = (int)m_log.SendMsg(EM_GETEVENTMASK, 0, 0);
	m_log.SendMsg(EM_SETEVENTMASK, 0, mask | ENM_LINK | ENM_MOUSEEVENTS);
	m_log.SendMsg(EM_LIMITTEXT, sizeof(wchar_t) * 0x7FFFFFFF, 0);
	m_log.SendMsg(EM_SETOLECALLBACK, 0, (LPARAM)&reOleCallback);
	m_log.SendMsg(EM_AUTOURLDETECT, 1, 0);

	if (db_get_b(0, CHAT_MODULE, "UseIEView", 0)) {
		IEVIEWWINDOW ieWindow = { sizeof(ieWindow) };
		ieWindow.iType = IEW_CREATE;
		ieWindow.dwMode = IEWM_CHAT;
		ieWindow.parent = m_hwnd;
		ieWindow.cx = 200;
		ieWindow.cy = 300;
		CallService(MS_IEVIEW_WINDOW, 0, (LPARAM)&ieWindow);

		m_hwndIeview = ieWindow.hwnd;

		IEVIEWEVENT iee = { sizeof(iee) };
		iee.iType = IEE_CLEAR_LOG;
		iee.hwnd = m_hwndIeview;
		iee.hContact = m_hContact;
		iee.pszProto = m_si->pszModule;
		CallService(MS_IEVIEW_EVENT, 0, (LPARAM)&iee);
	}

	m_log.SendMsg(EM_HIDESELECTION, TRUE, 0);

	UpdateOptions();
	UpdateStatusBar();
	UpdateTitle();

	SendMessage(m_hwndParent, CM_ADDCHILD, (WPARAM)this, 0);
	UpdateNickList();
	NotifyLocalWinEvent(m_hContact, m_hwnd, MSG_WINDOW_EVT_OPEN);
}

void CChatRoomDlg::OnDestroy()
{
	NotifyLocalWinEvent(m_hContact, m_hwnd, MSG_WINDOW_EVT_CLOSING);

	m_si->pDlg = nullptr;
	SetWindowLongPtr(m_hwnd, GWLP_USERDATA, 0);

	SendMessage(m_hwndParent, CM_REMOVECHILD, 0, (LPARAM)m_hwnd);
	if (m_hwndIeview != nullptr) {
		IEVIEWWINDOW ieWindow;
		ieWindow.cbSize = sizeof(IEVIEWWINDOW);
		ieWindow.iType = IEW_DESTROY;
		ieWindow.hwnd = m_hwndIeview;
		CallService(MS_IEVIEW_WINDOW, 0, (LPARAM)&ieWindow);
	}

	NotifyLocalWinEvent(m_hContact, m_hwnd, MSG_WINDOW_EVT_CLOSE);
}

void CChatRoomDlg::OnSplitterX(CSplitter *pSplitter)
{
	RECT rc;
	GetClientRect(m_hwnd, &rc);

	m_pParent->iSplitterX = rc.right - pSplitter->GetPos() + 1;
	if (m_pParent->iSplitterX < 35)
		m_pParent->iSplitterX = 35;
	if (m_pParent->iSplitterX > rc.right - rc.left - 35)
		m_pParent->iSplitterX = rc.right - rc.left - 35;
}

void CChatRoomDlg::OnSplitterY(CSplitter *pSplitter)
{
	RECT rc;
	GetClientRect(m_hwnd, &rc);
	m_pParent->iSplitterY = rc.bottom - pSplitter->GetPos();
}

void CChatRoomDlg::onDblClick_List(CCtrlListBox*)
{
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
		else DoEventHook(GC_USER_PRIVMESS, ui, nullptr, 0);
	}
}

void CChatRoomDlg::onClick_Ok(CCtrlButton *pButton)
{
	if (!pButton->Enabled())
		return;

	char *pszRtf = GetRichTextRTF(m_message.GetHwnd());
	if (pszRtf == nullptr)
		return;

	MODULEINFO *mi = pci->MM_FindModule(m_si->pszModule);
	if (mi == nullptr)
		return;

	TCmdList *cmdListNew = tcmdlist_last(cmdList);
	while (cmdListNew != nullptr && cmdListNew->temporary) {
		cmdList = tcmdlist_remove(cmdList, cmdListNew);
		cmdListNew = tcmdlist_last(cmdList);
	}

	// takes pszRtf to a queue, no leak here
	cmdList = tcmdlist_append(cmdList, pszRtf, 20, FALSE);

	CMStringW ptszText(ptrW(mir_utf8decodeW(pszRtf)));
	pci->DoRtfToTags(ptszText, mi->nColorCount, mi->crColors);
	ptszText.Trim();
	ptszText.Replace(L"%", L"%%");

	if (mi->bAckMsg) {
		EnableWindow(m_message.GetHwnd(), FALSE);
		m_message.SendMsg(EM_SETREADONLY, TRUE, 0);
	}
	else SetDlgItemText(m_hwnd, IDC_MESSAGE, L"");

	EnableWindow(m_btnOk.GetHwnd(), FALSE);

	DoEventHook(GC_USER_MESSAGE, nullptr, ptszText, 0);
	SetFocus(m_message.GetHwnd());
}

void CChatRoomDlg::onClick_History(CCtrlButton *pButton)
{
	if (!pButton->Enabled())
		return;
	
	MODULEINFO *pInfo = pci->MM_FindModule(m_si->pszModule);
	if (pInfo)
		ShellExecute(m_hwnd, nullptr, pci->GetChatLogsFilename(m_si, 0), nullptr, nullptr, SW_SHOW);
}

void CChatRoomDlg::onClick_ChanMgr(CCtrlButton *pButton)
{
	if (pButton->Enabled())
		DoEventHook(GC_USER_CHANMGR, nullptr, nullptr, 0);
}

void CChatRoomDlg::onClick_ShowList(CCtrlButton *pButton)
{
	if (!pButton->Enabled() || m_si->iType == GCW_SERVER)
		return;

	m_bNicklistEnabled = !m_bNicklistEnabled;
	pButton->SendMsg(BM_SETIMAGE, IMAGE_ICON, (LPARAM)GetCachedIcon(m_bNicklistEnabled ? "chat_nicklist" : "chat_nicklist2"));
	ScrollToBottom();
	SendMessage(m_hwnd, WM_SIZE, 0, 0);
}

void CChatRoomDlg::onClick_Filter(CCtrlButton *pButton)
{
	if (!pButton->Enabled())
		return;

	m_bFilterEnabled = !m_bFilterEnabled;
	pButton->SendMsg(BM_SETIMAGE, IMAGE_ICON, (LPARAM)GetCachedIcon(m_bFilterEnabled ? "chat_filter" : "chat_filter2"));
	if (m_bFilterEnabled && db_get_b(0, CHAT_MODULE, "RightClickFilter", 0) == 0)
		ShowFilterMenu();
	else
		RedrawLog();
}

void CChatRoomDlg::onClick_BIU(CCtrlButton *pButton)
{
	if (!pButton->Enabled())
		return;

	CHARFORMAT2 cf;
	cf.cbSize = sizeof(CHARFORMAT2);
	cf.dwMask = CFM_BOLD | CFM_ITALIC | CFM_UNDERLINE;
	cf.dwEffects = 0;

	if (IsDlgButtonChecked(m_hwnd, IDC_BOLD))
		cf.dwEffects |= CFE_BOLD;
	if (IsDlgButtonChecked(m_hwnd, IDC_ITALICS))
		cf.dwEffects |= CFE_ITALIC;
	if (IsDlgButtonChecked(m_hwnd, IDC_UNDERLINE))
		cf.dwEffects |= CFE_UNDERLINE;
	m_message.SendMsg(EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
}

void CChatRoomDlg::onClick_Color(CCtrlButton *pButton)
{
	if (!pButton->Enabled())
		return;
	
	MODULEINFO *pInfo = pci->MM_FindModule(m_si->pszModule);
	CHARFORMAT2 cf;
	cf.cbSize = sizeof(CHARFORMAT2);
	cf.dwEffects = 0;

	if (IsDlgButtonChecked(m_hwnd, IDC_COLOR)) {
		if (db_get_b(0, CHAT_MODULE, "RightClickFilter", 0) == 0)
			ShowColorChooser(IDC_COLOR);
		else if (m_bFGSet) {
			cf.dwMask = CFM_COLOR;
			cf.crTextColor = pInfo->crColors[m_iFG];
			m_message.SendMsg(EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
		}
	}
	else {
		COLORREF cr;
		LoadMsgDlgFont(MSGFONTID_MESSAGEAREA, nullptr, &cr);
		cf.dwMask = CFM_COLOR;
		cf.crTextColor = cr;
		m_message.SendMsg(EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
	}
}

void CChatRoomDlg::onClick_BkColor(CCtrlButton *pButton)
{
	if (!pButton->Enabled())
		return;
	
	MODULEINFO *pInfo = pci->MM_FindModule(m_si->pszModule);
	CHARFORMAT2 cf;
	cf.cbSize = sizeof(CHARFORMAT2);
	cf.dwEffects = 0;

	if (IsDlgButtonChecked(m_hwnd, IDC_BKGCOLOR)) {
		if (db_get_b(0, CHAT_MODULE, "RightClickFilter", 0) == 0)
			ShowColorChooser(IDC_BKGCOLOR);
		else if (m_bBGSet) {
			cf.dwMask = CFM_BACKCOLOR;
			cf.crBackColor = pInfo->crColors[m_iBG];
			m_message.SendMsg(EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
		}
	}
	else {
		cf.dwMask = CFM_BACKCOLOR;
		cf.crBackColor = (COLORREF)db_get_dw(0, SRMMMOD, SRMSGSET_INPUTBKGCOLOUR, SRMSGDEFSET_INPUTBKGCOLOUR);
		m_message.SendMsg(EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
	}
}

void CChatRoomDlg::onChange_Message(CCtrlEdit *pEdit)
{
	cmdListCurrent = nullptr;
	m_btnOk.Enable(GetRichTextLength(pEdit->GetHwnd(), 1200, FALSE) != 0);
}

/////////////////////////////////////////////////////////////////////////////////////////

void CChatRoomDlg::RedrawLog()
{
	m_si->LastTime = 0;
	if (m_si->pLog) {
		LOGINFO *pLog = m_si->pLog;
		if (m_si->iEventCount > 60) {
			int index = 0;
			while (index < 59) {
				if (pLog->next == nullptr)
					break;

				pLog = pLog->next;
				if ((m_si->iType != GCW_CHATROOM && m_si->iType != GCW_PRIVMESS) || !m_bFilterEnabled || (m_iLogFilterFlags & pLog->iType) != 0)
					index++;
			}
			StreamInEvents(pLog, true);
			mir_forkthread(phase2, m_si);
		}
		else StreamInEvents(m_si->pLogEnd, true);
	}
	else ClearLog();
}

void CChatRoomDlg::ScrollToBottom()
{
	if ((GetWindowLongPtr(m_log.GetHwnd(), GWL_STYLE) & WS_VSCROLL) == 0)
		return;

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

void CChatRoomDlg::ShowFilterMenu()
{
	HWND hwnd = CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_FILTER), m_hwnd, FilterWndProc, (LPARAM)this);
	TranslateDialogDefault(hwnd);

	RECT rc;
	GetWindowRect(m_btnFilter.GetHwnd(), &rc);
	SetWindowPos(hwnd, HWND_TOP, rc.left - 85, (IsWindowVisible(m_btnFilter.GetHwnd()) || IsWindowVisible(m_btnBold.GetHwnd())) ? rc.top - 206 : rc.top - 186, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
}

void CChatRoomDlg::UpdateNickList()
{
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
	InvalidateRect(m_nickList.GetHwnd(), nullptr, FALSE);
	UpdateWindow(m_nickList.GetHwnd());
	UpdateTitle();
}

void CChatRoomDlg::UpdateOptions()
{
	m_btnShowList.SendMsg(BM_SETIMAGE, IMAGE_ICON, (LPARAM)GetCachedIcon(m_bNicklistEnabled ? "chat_nicklist" : "chat_nicklist2"));
	m_btnFilter.SendMsg(BM_SETIMAGE, IMAGE_ICON, (LPARAM)GetCachedIcon(m_bFilterEnabled ? "chat_filter" : "chat_filter2"));
	{
		MODULEINFO *pInfo = pci->MM_FindModule(m_si->pszModule);
		if (pInfo) {
			m_btnBold.Enable(pInfo->bBold);
			m_btnItalic.Enable(pInfo->bItalics);
			m_btnUnderline.Enable(pInfo->bUnderline);
			m_btnColor.Enable(pInfo->bColor);
			m_btnBkColor.Enable(pInfo->bBkgColor);
			if (m_si->iType == GCW_CHATROOM)
				m_btnChanMgr.Enable(pInfo->bChanMgr);
		}
	}

	UpdateStatusBar();
	UpdateTitle();
	FixTabIcons();

	m_log.SendMsg(EM_SETBKGNDCOLOR, 0, g_Settings.crLogBackground);

	// messagebox
	COLORREF crFore;
	LoadMsgDlgFont(MSGFONTID_MESSAGEAREA, nullptr, &crFore);

	CHARFORMAT2 cf;
	cf.cbSize = sizeof(CHARFORMAT2);
	cf.dwMask = CFM_COLOR | CFM_BOLD | CFM_UNDERLINE | CFM_BACKCOLOR;
	cf.dwEffects = 0;
	cf.crTextColor = crFore;
	cf.crBackColor = db_get_dw(0, SRMMMOD, SRMSGSET_INPUTBKGCOLOUR, SRMSGDEFSET_INPUTBKGCOLOUR);
	m_message.SendMsg(EM_SETBKGNDCOLOR, 0, db_get_dw(0, SRMMMOD, SRMSGSET_INPUTBKGCOLOUR, SRMSGDEFSET_INPUTBKGCOLOUR));
	m_message.SendMsg(WM_SETFONT, (WPARAM)g_Settings.MessageBoxFont, MAKELPARAM(TRUE, 0));
	m_message.SendMsg(EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf);
	{
		// nicklist
		int ih = GetTextPixelSize(L"AQG_glo'", g_Settings.UserListFont, FALSE);
		int ih2 = GetTextPixelSize(L"AQG_glo'", g_Settings.UserListHeadingsFont, FALSE);
		int height = db_get_b(0, CHAT_MODULE, "NicklistRowDist", 12);
		int font = ih > ih2 ? ih : ih2;
		// make sure we have space for icon!
		if (db_get_b(0, CHAT_MODULE, "ShowContactStatus", 0))
			font = font > 16 ? font : 16;

		m_nickList.SendMsg(LB_SETITEMHEIGHT, 0, height > font ? height : font);
		InvalidateRect(m_nickList.GetHwnd(), nullptr, TRUE);
	}
	m_message.SendMsg(EM_REQUESTRESIZE, 0, 0);
	SendMessage(m_hwnd, WM_SIZE, 0, 0);
	RedrawLog2();
}

void CChatRoomDlg::UpdateStatusBar()
{
	MODULEINFO *mi = pci->MM_FindModule(m_si->pszModule);
	wchar_t szTemp[512];
	mir_snwprintf(szTemp, L"%s : %s", mi->ptszModDispName, m_si->ptszStatusbarText ? m_si->ptszStatusbarText : L"");

	StatusBarData sbd;
	sbd.iItem = 0;
	sbd.iFlags = SBDF_TEXT | SBDF_ICON;
	sbd.hIcon = m_si->wStatus == ID_STATUS_ONLINE ? mi->hOnlineIcon : mi->hOfflineIcon;
	sbd.pszText = szTemp;
	SendMessage(m_hwndParent, CM_UPDATESTATUSBAR, (WPARAM)&sbd, (LPARAM)m_hwnd);

	sbd.iItem = 1;
	sbd.hIcon = nullptr;
	sbd.pszText = L"";
	SendMessage(m_hwndParent, CM_UPDATESTATUSBAR, (WPARAM)&sbd, (LPARAM)m_hwnd);

	StatusIconData sid = { sizeof(sid) };
	sid.szModule = SRMMMOD;
	Srmm_ModifyIcon(m_hContact, &sid);
}

void CChatRoomDlg::UpdateTitle()
{
	TitleBarData tbd = {};
	if (g_dat.flags & SMF_STATUSICON) {
		MODULEINFO *mi = pci->MM_FindModule(m_si->pszModule);
		tbd.hIcon = (m_si->wStatus == ID_STATUS_ONLINE) ? mi->hOnlineIcon : mi->hOfflineIcon;
		tbd.hIconBig = (m_si->wStatus == ID_STATUS_ONLINE) ? mi->hOnlineIconBig : mi->hOfflineIconBig;
	}
	else {
		tbd.hIcon = GetCachedIcon("chat_window");
		tbd.hIconBig = g_dat.hIconChatBig;
	}
	tbd.hIconNot = (m_si->wState & (GC_EVENT_HIGHLIGHT | STATE_TALK)) ? GetCachedIcon("chat_overlay") : nullptr;

	wchar_t szTemp[512];
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
	SendMessage(m_hwndParent, CM_UPDATETITLEBAR, (WPARAM)&tbd, (LPARAM)m_hwnd);
	SendMessage(m_hwnd, DM_UPDATETABCONTROL, 0, 0);
}

/////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CChatRoomDlg::DlgProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static HMENU hToolbarMenu;
	RECT rc;
	POINT pt;
	HICON hIcon;
	TabControlData tcd;

	switch (uMsg) {
	case DM_SWITCHINFOBAR:
	case DM_SWITCHTOOLBAR:
		SendMessage(m_hwnd, WM_SIZE, 0, 0);
		break;

	case WM_SIZE:
		if (wParam == SIZE_MAXIMIZED)
			ScrollToBottom();

		if (IsIconic(m_hwnd)) break;

		if (wParam == SIZE_RESTORED || wParam == SIZE_MAXIMIZED) {
			GetClientRect(m_hwnd, &rc);
			MessageDialogResize(rc.right - rc.left, rc.bottom - rc.top);
		}
		break;

	case DM_UPDATETABCONTROL:
		tcd.iFlags = TCDF_TEXT;
		tcd.pszText = m_si->ptszName;
		SendMessage(m_hwndParent, CM_UPDATETABCONTROL, (WPARAM)&tcd, (LPARAM)m_hwnd);
		// fall through

	case DM_ACTIVATE:
		if (m_si->wState & STATE_TALK) {
			m_si->wState &= ~STATE_TALK;
			db_set_w(m_hContact, m_si->pszModule, "ApparentMode", 0);
		}

		if (m_si->wState & GC_EVENT_HIGHLIGHT) {
			m_si->wState &= ~GC_EVENT_HIGHLIGHT;

			if (pcli->pfnGetEvent(m_hContact, 0))
				pcli->pfnRemoveEvent(m_hContact, GC_FAKE_EVENT);
		}

		FixTabIcons();
		if (!m_si->pDlg) {
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
			int height = db_get_b(0, CHAT_MODULE, "NicklistRowDist", 12);
			// make sure we have space for icon!
			if (db_get_b(0, CHAT_MODULE, "ShowContactStatus", 0))
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
						DrawIconEx(dis->hDC, x_offset, dis->rcItem.top + offset - 3, hIcon, 16, 16, 0, nullptr, DI_NORMAL);
						IcoLib_ReleaseIcon(hIcon);
						x_offset += 18;
					}
					DrawIconEx(dis->hDC, x_offset, dis->rcItem.top + offset, pci->SM_GetStatusIcon(m_si, ui), 10, 10, 0, nullptr, DI_NORMAL);
					x_offset += 12;
					if (g_Settings.bShowContactStatus && !g_Settings.bContactStatusFirst && ui->ContactStatus) {
						hIcon = Skin_LoadProtoIcon(m_si->pszModule, ui->ContactStatus);
						DrawIconEx(dis->hDC, x_offset, dis->rcItem.top + offset - 3, hIcon, 16, 16, 0, nullptr, DI_NORMAL);
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

	case WM_ACTIVATE:
		if (LOWORD(wParam) != WA_ACTIVE)
			break;

		// fall through
	case WM_MOUSEACTIVATE:
		if (uMsg != WM_ACTIVATE)
			SetFocus(m_message.GetHwnd());

		pci->SetActiveSession(m_si);

		if (db_get_w(m_hContact, m_si->pszModule, "ApparentMode", 0) != 0)
			db_set_w(m_hContact, m_si->pszModule, "ApparentMode", 0);
		if (pcli->pfnGetEvent(m_hContact, 0))
			pcli->pfnRemoveEvent(m_hContact, GC_FAKE_EVENT);
		break;

	case WM_NOTIFY:
		{
			LPNMHDR pNmhdr = (LPNMHDR)lParam;
			switch (pNmhdr->code) {
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
					if (ui != nullptr) {
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

	case WM_KEYDOWN:
		SetFocus(m_message.GetHwnd());
		break;

	case WM_GETMINMAXINFO:
		{
			MINMAXINFO *mmi = (MINMAXINFO*)lParam;
			mmi->ptMinTrackSize.x = m_pParent->iSplitterX + 43;
			if (mmi->ptMinTrackSize.x < 350)
				mmi->ptMinTrackSize.x = 350;

			mmi->ptMinTrackSize.y = m_minLogBoxHeight + TOOLBAR_HEIGHT + m_minEditBoxHeight + 5;
		}
		break;

	case WM_LBUTTONDBLCLK:
		if (LOWORD(lParam) < 30)
			ScrollToBottom();
		else
			SendMessage(m_hwndParent, WM_SYSCOMMAND, SC_MINIMIZE, 0);
		break;

	case WM_LBUTTONDOWN:
		SendMessage(m_hwndParent, WM_LBUTTONDOWN, wParam, lParam);
		return TRUE;

	case DM_GETCONTEXTMENU:
		SetWindowLongPtr(m_hwnd, DWLP_MSGRESULT, (LPARAM)Menu_BuildContactMenu(m_hContact));
		return TRUE;

	case WM_CONTEXTMENU:
		if (m_hwndParent == (HWND)wParam) {
			HMENU hMenu = Menu_BuildContactMenu(m_hContact);
			GetCursorPos(&pt);
			TrackPopupMenu(hMenu, 0, pt.x, pt.y, 0, m_hwnd, nullptr);
			DestroyMenu(hMenu);
		}
		break;
	}
	return CScriverWindow::DlgProc(uMsg, wParam, lParam);
}

void ShowRoom(SESSION_INFO *si)
{
	if (si == nullptr)
		return;

	// Do we need to create a window?
	if (si->pDlg == nullptr) {
		HWND hParent = GetParentWindow(si->hContact, TRUE);

		CChatRoomDlg *pDlg = new CChatRoomDlg(si);
		pDlg->SetParent(hParent);
		pDlg->Show();
		
		pDlg->m_pParent = (ParentWindowData*)GetWindowLongPtr(hParent, GWLP_USERDATA);
		si->pDlg = pDlg;
	}
	SendMessage(si->pDlg->GetHwnd(), DM_UPDATETABCONTROL, -1, (LPARAM)si);
	SendMessage(GetParent(si->pDlg->GetHwnd()), CM_ACTIVATECHILD, 0, (LPARAM)si->pDlg->GetHwnd());
	SendMessage(GetParent(si->pDlg->GetHwnd()), CM_POPUPWINDOW, 0, (LPARAM)si->pDlg->GetHwnd());
	SendMessage(si->pDlg->GetHwnd(), WM_MOUSEACTIVATE, 0, 0);
	SetFocus(GetDlgItem(si->pDlg->GetHwnd(), IDC_MESSAGE));
}
