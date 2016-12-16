/*
Chat module plugin for Miranda IM

Copyright 2000-12 Miranda IM, 2012-16 Miranda NG project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

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

static HKL hkl = NULL;

struct MESSAGESUBDATA
{
	SESSION_INFO *si;
	time_t lastEnterTime;
	wchar_t  szTabSave[20];
};

static wchar_t szTrimString[] = L":;,.!?\'\"><()[]- \r\n";

static LRESULT CALLBACK SplitterSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	RECT rc;

	switch (msg) {
	case WM_NCHITTEST:
		return HTCLIENT;

	case WM_SETCURSOR:
		GetClientRect(hwnd, &rc);
		SetCursor(rc.right > rc.bottom ? LoadCursor(NULL, IDC_SIZENS) : LoadCursor(NULL, IDC_SIZEWE));
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

static int RoomWndResize(HWND, LPARAM lParam, UTILRESIZECONTROL *urc)
{
	SESSION_INFO *si = (SESSION_INFO*)lParam;

	RECT rc;
	BOOL bControl = (BOOL)db_get_b(NULL, CHAT_MODULE, "ShowTopButtons", 1);
	BOOL bFormat = (BOOL)db_get_b(NULL, CHAT_MODULE, "ShowFormatButtons", 1);
	BOOL bToolbar = bFormat || bControl;
	BOOL bSend = (BOOL)db_get_b(NULL, CHAT_MODULE, "ShowSend", 0);
	BOOL bNick = si->iType != GCW_SERVER && si->bNicklistEnabled;

	switch (urc->wId) {
	case IDOK:
		GetWindowRect(si->hwndStatus, &rc);
		urc->rcItem.left = bSend ? 315 : urc->dlgNewSize.cx;
		urc->rcItem.top = urc->dlgNewSize.cy - si->iSplitterY + 23;
		urc->rcItem.bottom = urc->dlgNewSize.cy - (rc.bottom - rc.top) - 1;
		return RD_ANCHORX_RIGHT | RD_ANCHORY_CUSTOM;

	case IDC_LOG:
		urc->rcItem.top = 2;
		urc->rcItem.left = 0;
		urc->rcItem.right = bNick ? urc->dlgNewSize.cx - si->iSplitterX : urc->dlgNewSize.cx;
LBL_CalcBottom:
		urc->rcItem.bottom = urc->dlgNewSize.cy - si->iSplitterY;
		if (!bToolbar)
			urc->rcItem.bottom += 20;
		return RD_ANCHORX_CUSTOM | RD_ANCHORY_CUSTOM;

	case IDC_LIST:
		urc->rcItem.top = 2;
		urc->rcItem.right = urc->dlgNewSize.cx;
		urc->rcItem.left = urc->dlgNewSize.cx - si->iSplitterX + 2;
		goto LBL_CalcBottom;

	case IDC_SPLITTERX:
		urc->rcItem.top = 1;
		urc->rcItem.left = urc->dlgNewSize.cx - si->iSplitterX;
		urc->rcItem.right = urc->rcItem.left + 2;
		goto LBL_CalcBottom;

	case IDC_SPLITTERY:
		urc->rcItem.top = urc->dlgNewSize.cy - si->iSplitterY;
		if (!bToolbar)
			urc->rcItem.top += 20;
		urc->rcItem.bottom = urc->rcItem.top + 2;
		return RD_ANCHORX_WIDTH | RD_ANCHORY_CUSTOM;

	case IDC_MESSAGE:
		GetWindowRect(si->hwndStatus, &rc);
		urc->rcItem.right = bSend ? urc->dlgNewSize.cx - 64 : urc->dlgNewSize.cx;
		urc->rcItem.top = urc->dlgNewSize.cy - si->iSplitterY + 22;
		urc->rcItem.bottom = urc->dlgNewSize.cy - (rc.bottom - rc.top) - 1;
		return RD_ANCHORX_LEFT | RD_ANCHORY_CUSTOM;
	}
	return RD_ANCHORX_LEFT | RD_ANCHORY_TOP;
}

static LRESULT CALLBACK MessageSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	MESSAGESUBDATA *dat = (MESSAGESUBDATA*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	HWND hwndDlg = GetParent(hwnd);
	CHARRANGE sel;

	switch (msg) {
	case EM_SUBCLASSED:
		dat = (MESSAGESUBDATA*)mir_alloc(sizeof(MESSAGESUBDATA));

		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)dat);
		dat->si = (SESSION_INFO*)lParam;
		dat->szTabSave[0] = '\0';
		dat->lastEnterTime = 0;
		return 0;

	case WM_MOUSEWHEEL:
		SendDlgItemMessage(hwndDlg, IDC_LOG, WM_MOUSEWHEEL, wParam, lParam);
		dat->lastEnterTime = 0;
		return TRUE;

	case EM_REPLACESEL:
		PostMessage(hwnd, EM_ACTIVATE, 0, 0);
		break;

	case EM_ACTIVATE:
		SetActiveWindow(hwndDlg);
		break;

	case WM_CHAR:
		{
			BOOL isCtrl = GetKeyState(VK_CONTROL) & 0x8000;
			BOOL isAlt = GetKeyState(VK_MENU) & 0x8000;

			if (GetWindowLongPtr(hwnd, GWL_STYLE) & ES_READONLY)
				break;

			if (wParam == 9 && isCtrl && !isAlt) // ctrl-i (italics)
				return TRUE;

			if (wParam == VK_SPACE && isCtrl && !isAlt) // ctrl-space (paste clean text)
				return TRUE;

			if (wParam == '\n' || wParam == '\r') {
				if ((isCtrl != 0) ^ (0 != db_get_b(NULL, CHAT_MODULE, "SendOnEnter", 1))) {
					PostMessage(hwndDlg, WM_COMMAND, IDOK, 0);
					return 0;
				}
				if (db_get_b(NULL, CHAT_MODULE, "SendOnDblEnter", 0)) {
					if (dat->lastEnterTime + 2 < time(NULL))
						dat->lastEnterTime = time(NULL);
					else {
						SendMessage(hwnd, WM_KEYDOWN, VK_BACK, 0);
						SendMessage(hwnd, WM_KEYUP, VK_BACK, 0);
						PostMessage(hwndDlg, WM_COMMAND, IDOK, 0);
						return 0;
					}
				}
			}
			else dat->lastEnterTime = 0;

			if (wParam == 1 && isCtrl && !isAlt) {      //ctrl-a
				SendMessage(hwnd, EM_SETSEL, 0, -1);
				return 0;
			}
		}
		break;

	case WM_KEYDOWN:
		{
			static int start, end;
			BOOL isShift = GetKeyState(VK_SHIFT) & 0x8000;
			BOOL isCtrl = GetKeyState(VK_CONTROL) & 0x8000;
			BOOL isAlt = GetKeyState(VK_MENU) & 0x8000;
			if (wParam == VK_RETURN) {
				dat->szTabSave[0] = '\0';
				if ((isCtrl != 0) ^ (0 != db_get_b(NULL, CHAT_MODULE, "SendOnEnter", 1)))
					return 0;

				if (db_get_b(NULL, CHAT_MODULE, "SendOnDblEnter", 0))
					if (dat->lastEnterTime + 2 >= time(NULL))
						return 0;

				break;
			}

			if (wParam == VK_TAB && isShift && !isCtrl) { // SHIFT-TAB (go to nick list)
				SetFocus(GetDlgItem(hwndDlg, IDC_LIST));
				return TRUE;
			}

			if (wParam == VK_TAB && isCtrl && !isShift) { // CTRL-TAB (switch tab/window)
				if (g_Settings.bTabsEnable)
					SendMessage(GetParent(GetParent(hwndDlg)), GC_SWITCHNEXTTAB, 0, 0);
				else
					pci->ShowRoom(SM_GetNextWindow(dat->si), WINDOW_VISIBLE, TRUE);
				return TRUE;
			}

			if (wParam == VK_TAB && isCtrl && isShift) { // CTRL_SHIFT-TAB (switch tab/window)
				if (g_Settings.bTabsEnable)
					SendMessage(GetParent(GetParent(hwndDlg)), GC_SWITCHPREVTAB, 0, 0);
				else
					pci->ShowRoom(SM_GetPrevWindow(dat->si), WINDOW_VISIBLE, TRUE);
				return TRUE;
			}

			if (wParam <= '9' && wParam >= '1' && isCtrl && !isAlt) // CTRL + 1 -> 9 (switch tab)
				if (g_Settings.bTabsEnable)
					SendMessage(hwndDlg, GC_SWITCHTAB, 0, (int)wParam - (int)'1');

			if (wParam <= VK_NUMPAD9 && wParam >= VK_NUMPAD1 && isCtrl && !isAlt) // CTRL + 1 -> 9 (switch tab)
				if (g_Settings.bTabsEnable)
					SendMessage(hwndDlg, GC_SWITCHTAB, 0, (int)wParam - (int)VK_NUMPAD1);

			if (wParam == VK_TAB && !isCtrl && !isShift) {    //tab-autocomplete
				wchar_t* pszText = NULL;
				LRESULT lResult = (LRESULT)SendMessage(hwnd, EM_GETSEL, 0, 0);

				SendMessage(hwnd, WM_SETREDRAW, FALSE, 0);
				start = LOWORD(lResult);
				end = HIWORD(lResult);
				SendMessage(hwnd, EM_SETSEL, end, end);

				GETTEXTLENGTHEX gtl = {};
				gtl.flags = GTL_PRECISE;
				gtl.codepage = CP_ACP;
				int iLen = SendMessage(hwnd, EM_GETTEXTLENGTHEX, (WPARAM)&gtl, 0);
				if (iLen > 0) {
					pszText = (wchar_t *)mir_alloc(sizeof(wchar_t)*(iLen + 100));

					GETTEXTEX gt = {};
					gt.cb = iLen + 99;
					gt.flags = GT_DEFAULT;
					gt.codepage = 1200;

					SendMessage(hwnd, EM_GETTEXTEX, (WPARAM)&gt, (LPARAM)pszText);
					while (start > 0 && pszText[start - 1] != ' ' && pszText[start - 1] != 13 && pszText[start - 1] != VK_TAB)
						start--;
					while (end < iLen && pszText[end] != ' ' && pszText[end] != 13 && pszText[end - 1] != VK_TAB)
						end++;

					if (dat->szTabSave[0] == '\0')
						mir_wstrncpy(dat->szTabSave, pszText + start, end - start + 1);

					wchar_t *pszSelName = (wchar_t *)mir_alloc(sizeof(wchar_t)*(end - start + 1));
					mir_wstrncpy(pszSelName, pszText + start, end - start + 1);

					wchar_t *pszName = pci->UM_FindUserAutoComplete(dat->si->pUsers, dat->szTabSave, pszSelName);
					if (pszName == NULL) {
						pszName = dat->szTabSave;
						SendMessage(hwnd, EM_SETSEL, start, end);
						if (end != start)
							SendMessage(hwnd, EM_REPLACESEL, FALSE, (LPARAM)pszName);
						dat->szTabSave[0] = '\0';
					}
					else {
						SendMessage(hwnd, EM_SETSEL, start, end);
						if (end != start)
							SendMessage(hwnd, EM_REPLACESEL, FALSE, (LPARAM)pszName);
					}
					mir_free(pszText);
					mir_free(pszSelName);
				}

				SendMessage(hwnd, WM_SETREDRAW, TRUE, 0);
				RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE);
				return 0;
			}

			if (dat->szTabSave[0] != '\0' && wParam != VK_RIGHT && wParam != VK_LEFT && wParam != VK_SPACE && wParam != VK_RETURN && wParam != VK_BACK && wParam != VK_DELETE) {
				if (g_Settings.bAddColonToAutoComplete && start == 0)
					SendMessageA(hwnd, EM_REPLACESEL, FALSE, (LPARAM) ": ");

				dat->szTabSave[0] = '\0';
			}

			if (wParam == VK_F4 && isCtrl && !isAlt) { // ctrl-F4 (close tab)
				SendMessage(hwndDlg, GC_CLOSEWINDOW, 0, 0);
				return TRUE;
			}

			if (wParam == 0x49 && isCtrl && !isAlt) { // ctrl-i (italics)
				CheckDlgButton(hwndDlg, IDC_ITALICS, IsDlgButtonChecked(hwndDlg, IDC_ITALICS) == BST_UNCHECKED ? BST_CHECKED : BST_UNCHECKED);
				SendMessage(hwndDlg, WM_COMMAND, MAKEWPARAM(IDC_ITALICS, 0), 0);
				return TRUE;
			}

			if (wParam == 0x42 && isCtrl && !isAlt) { // ctrl-b (bold)
				CheckDlgButton(hwndDlg, IDC_BOLD, IsDlgButtonChecked(hwndDlg, IDC_BOLD) == BST_UNCHECKED ? BST_CHECKED : BST_UNCHECKED);
				SendMessage(hwndDlg, WM_COMMAND, MAKEWPARAM(IDC_BOLD, 0), 0);
				return TRUE;
			}

			if (wParam == 0x55 && isCtrl && !isAlt) { // ctrl-u (paste clean text)
				CheckDlgButton(hwndDlg, IDC_UNDERLINE, IsDlgButtonChecked(hwndDlg, IDC_UNDERLINE) == BST_UNCHECKED ? BST_CHECKED : BST_UNCHECKED);
				SendMessage(hwndDlg, WM_COMMAND, MAKEWPARAM(IDC_UNDERLINE, 0), 0);
				return TRUE;
			}

			if (wParam == 0x4b && isCtrl && !isAlt) { // ctrl-k (paste clean text)
				CheckDlgButton(hwndDlg, IDC_COLOR, IsDlgButtonChecked(hwndDlg, IDC_COLOR) == BST_UNCHECKED ? BST_CHECKED : BST_UNCHECKED);
				SendMessage(hwndDlg, WM_COMMAND, MAKEWPARAM(IDC_COLOR, 0), 0);
				return TRUE;
			}

			if (wParam == VK_SPACE && isCtrl && !isAlt) { // ctrl-space (paste clean text)
				CheckDlgButton(hwndDlg, IDC_BKGCOLOR, BST_UNCHECKED);
				CheckDlgButton(hwndDlg, IDC_COLOR, BST_UNCHECKED);
				CheckDlgButton(hwndDlg, IDC_BOLD, BST_UNCHECKED);
				CheckDlgButton(hwndDlg, IDC_UNDERLINE, BST_UNCHECKED);
				CheckDlgButton(hwndDlg, IDC_ITALICS, BST_UNCHECKED);
				SendMessage(hwndDlg, WM_COMMAND, MAKEWPARAM(IDC_BKGCOLOR, 0), 0);
				SendMessage(hwndDlg, WM_COMMAND, MAKEWPARAM(IDC_COLOR, 0), 0);
				SendMessage(hwndDlg, WM_COMMAND, MAKEWPARAM(IDC_BOLD, 0), 0);
				SendMessage(hwndDlg, WM_COMMAND, MAKEWPARAM(IDC_UNDERLINE, 0), 0);
				SendMessage(hwndDlg, WM_COMMAND, MAKEWPARAM(IDC_ITALICS, 0), 0);
				return TRUE;
			}

			if (wParam == 0x4c && isCtrl && !isAlt) { // ctrl-l (paste clean text)
				CheckDlgButton(hwndDlg, IDC_BKGCOLOR, IsDlgButtonChecked(hwndDlg, IDC_BKGCOLOR) == BST_UNCHECKED ? BST_CHECKED : BST_UNCHECKED);
				SendMessage(hwndDlg, WM_COMMAND, MAKEWPARAM(IDC_BKGCOLOR, 0), 0);
				return TRUE;
			}

			if (wParam == 0x46 && isCtrl && !isAlt) { // ctrl-f (paste clean text)
				if (IsWindowEnabled(GetDlgItem(hwndDlg, IDC_FILTER)))
					SendMessage(hwndDlg, WM_COMMAND, MAKEWPARAM(IDC_FILTER, 0), 0);
				return TRUE;
			}

			if (wParam == 0x4e && isCtrl && !isAlt) { // ctrl-n (nicklist)
				if (IsWindowEnabled(GetDlgItem(hwndDlg, IDC_SHOWNICKLIST)))
					SendMessage(hwndDlg, WM_COMMAND, MAKEWPARAM(IDC_SHOWNICKLIST, 0), 0);
				return TRUE;
			}

			if (wParam == 0x48 && isCtrl && !isAlt) { // ctrl-h (history)
				SendMessage(hwndDlg, WM_COMMAND, MAKEWPARAM(IDC_HISTORY, 0), 0);
				return TRUE;
			}

			if (wParam == 0x4f && isCtrl && !isAlt) { // ctrl-o (options)
				if (IsWindowEnabled(GetDlgItem(hwndDlg, IDC_CHANMGR)))
					SendMessage(hwndDlg, WM_COMMAND, MAKEWPARAM(IDC_CHANMGR, 0), 0);
				return TRUE;
			}

			if ((wParam == 45 && isShift || wParam == 0x56 && isCtrl) && !isAlt) { // ctrl-v (paste clean text)
				SendMessage(hwnd, EM_PASTESPECIAL, CF_TEXT, 0);
				return TRUE;
			}

			if (wParam == 0x57 && isCtrl && !isAlt) { // ctrl-w (close window)
				SendMessage(hwndDlg, GC_CLOSEWINDOW, 0, 0);
				return TRUE;
			}

			if (wParam == VK_NEXT || wParam == VK_PRIOR) {
				HWND htemp = hwndDlg;
				SendDlgItemMessage(htemp, IDC_LOG, msg, wParam, lParam);
				dat->lastEnterTime = 0;
				return TRUE;
			}

			if (wParam == VK_UP && isCtrl && !isAlt) {
				char* lpPrevCmd = pci->SM_GetPrevCommand(dat->si->ptszID, dat->si->pszModule);

				SendMessage(hwnd, WM_SETREDRAW, FALSE, 0);

				if (lpPrevCmd) {
					SETTEXTEX ste;
					ste.flags = ST_DEFAULT;
					ste.codepage = CP_ACP;
					SendMessage(hwnd, EM_SETTEXTEX, (WPARAM)&ste, (LPARAM)lpPrevCmd);
				}
				else SetWindowText(hwnd, L"");

				GETTEXTLENGTHEX gtl = {};
				gtl.flags = GTL_PRECISE;
				gtl.codepage = CP_ACP;
				int iLen = SendMessage(hwnd, EM_GETTEXTLENGTHEX, (WPARAM)&gtl, 0);
				SendMessage(hwnd, EM_SCROLLCARET, 0, 0);
				SendMessage(hwnd, WM_SETREDRAW, TRUE, 0);
				RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE);
				SendMessage(hwnd, EM_SETSEL, iLen, iLen);
				dat->lastEnterTime = 0;
				return TRUE;
			}

			if (wParam == VK_DOWN && isCtrl && !isAlt) {
				char* lpPrevCmd = pci->SM_GetNextCommand(dat->si->ptszID, dat->si->pszModule);
				SendMessage(hwnd, WM_SETREDRAW, FALSE, 0);

				if (lpPrevCmd) {
					SETTEXTEX ste;
					ste.flags = ST_DEFAULT;
					ste.codepage = CP_ACP;
					SendMessage(hwnd, EM_SETTEXTEX, (WPARAM)&ste, (LPARAM)lpPrevCmd);
				}
				else SetWindowText(hwnd, L"");

				GETTEXTLENGTHEX gtl = {};
				gtl.flags = GTL_PRECISE;
				gtl.codepage = CP_ACP;
				int iLen = SendMessage(hwnd, EM_GETTEXTLENGTHEX, (WPARAM)&gtl, 0);
				SendMessage(hwnd, EM_SCROLLCARET, 0, 0);
				SendMessage(hwnd, WM_SETREDRAW, TRUE, 0);
				RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE);
				SendMessage(hwnd, EM_SETSEL, iLen, iLen);
				dat->lastEnterTime = 0;
				return TRUE;
			}
		}
	
		// fall through
	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_KILLFOCUS:
		dat->lastEnterTime = 0;
		break;

	case WM_RBUTTONDOWN:
		{
			HMENU hSubMenu = GetSubMenu(g_hMenu, 4);
			TranslateMenu(hSubMenu);
			SendMessage(hwnd, EM_EXGETSEL, 0, (LPARAM)&sel);

			EnableMenuItem(hSubMenu, ID_MESSAGE_UNDO, SendMessage(hwnd, EM_CANUNDO, 0, 0) ? MF_ENABLED : MF_GRAYED);
			EnableMenuItem(hSubMenu, ID_MESSAGE_REDO, SendMessage(hwnd, EM_CANREDO, 0, 0) ? MF_ENABLED : MF_GRAYED);
			EnableMenuItem(hSubMenu, ID_MESSAGE_COPY, sel.cpMax != sel.cpMin ? MF_ENABLED : MF_GRAYED);
			EnableMenuItem(hSubMenu, ID_MESSAGE_CUT, sel.cpMax != sel.cpMin ? MF_ENABLED : MF_GRAYED);

			dat->lastEnterTime = 0;

			POINT pt;
			pt.x = (short)LOWORD(lParam);
			pt.y = (short)HIWORD(lParam);
			ClientToScreen(hwnd, &pt);

			CHARRANGE all = { 0, -1 };
			UINT uID = TrackPopupMenu(hSubMenu, TPM_RETURNCMD, pt.x, pt.y, 0, hwnd, NULL);
			switch (uID) {
			case 0:
				break;

			case ID_MESSAGE_UNDO:
				SendMessage(hwnd, EM_UNDO, 0, 0);
				break;

			case ID_MESSAGE_REDO:
				SendMessage(hwnd, EM_REDO, 0, 0);
				break;

			case ID_MESSAGE_COPY:
				SendMessage(hwnd, WM_COPY, 0, 0);
				break;

			case ID_MESSAGE_CUT:
				SendMessage(hwnd, WM_CUT, 0, 0);
				break;

			case ID_MESSAGE_PASTE:
				SendMessage(hwnd, EM_PASTESPECIAL, CF_TEXT, 0);
				break;

			case ID_MESSAGE_SELECTALL:
				SendMessage(hwnd, EM_EXSETSEL, 0, (LPARAM)&all);
				break;

			case ID_MESSAGE_CLEAR:
				SetWindowText(hwnd, L"");
				break;
			}
			PostMessage(hwnd, WM_KEYUP, 0, 0);
		}
		break;

	case WM_KEYUP:
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
		{
			CHARFORMAT2 cf;
			cf.cbSize = sizeof(CHARFORMAT2);
			cf.dwMask = CFM_BOLD | CFM_ITALIC | CFM_UNDERLINE | CFM_BACKCOLOR | CFM_COLOR;
			SendMessage(hwnd, EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);

			MODULEINFO *pmi = pci->MM_FindModule(dat->si->pszModule);
			if (pmi == NULL)
				break;

			if (pmi->bColor) {
				int index = GetColorIndex(dat->si->pszModule, cf.crTextColor);
				UINT u = IsDlgButtonChecked(hwndDlg, IDC_COLOR);

				if (index >= 0) {
					dat->si->bFGSet = TRUE;
					dat->si->iFG = index;
				}

				if (u == BST_UNCHECKED && cf.crTextColor != g_Settings.MessageAreaColor)
					CheckDlgButton(hwndDlg, IDC_COLOR, BST_CHECKED);
				else if (u == BST_CHECKED && cf.crTextColor == g_Settings.MessageAreaColor)
					CheckDlgButton(hwndDlg, IDC_COLOR, BST_UNCHECKED);
			}

			if (pmi->bBkgColor) {
				int index = GetColorIndex(dat->si->pszModule, cf.crBackColor);
				COLORREF crB = (COLORREF)db_get_dw(NULL, CHAT_MODULE, "ColorMessageBG", GetSysColor(COLOR_WINDOW));
				UINT u = IsDlgButtonChecked(hwndDlg, IDC_BKGCOLOR);

				if (index >= 0) {
					dat->si->bBGSet = TRUE;
					dat->si->iBG = index;
				}
				if (u == BST_UNCHECKED && cf.crBackColor != crB)
					CheckDlgButton(hwndDlg, IDC_BKGCOLOR, BST_CHECKED);
				else if (u == BST_CHECKED && cf.crBackColor == crB)
					CheckDlgButton(hwndDlg, IDC_BKGCOLOR, BST_UNCHECKED);
			}

			if (pmi->bBold) {
				UINT u = IsDlgButtonChecked(hwndDlg, IDC_BOLD);
				UINT u2 = cf.dwEffects;
				u2 &= CFE_BOLD;
				if (u == BST_UNCHECKED && u2)
					CheckDlgButton(hwndDlg, IDC_BOLD, BST_CHECKED);
				else if (u == BST_CHECKED && u2 == 0)
					CheckDlgButton(hwndDlg, IDC_BOLD, BST_UNCHECKED);
			}

			if (pmi->bItalics) {
				UINT u = IsDlgButtonChecked(hwndDlg, IDC_ITALICS);
				UINT u2 = cf.dwEffects;
				u2 &= CFE_ITALIC;
				if (u == BST_UNCHECKED && u2)
					CheckDlgButton(hwndDlg, IDC_ITALICS, BST_CHECKED);
				else if (u == BST_CHECKED && u2 == 0)
					CheckDlgButton(hwndDlg, IDC_ITALICS, BST_UNCHECKED);
			}

			if (pmi->bUnderline) {
				UINT u = IsDlgButtonChecked(hwndDlg, IDC_UNDERLINE);
				UINT u2 = cf.dwEffects;
				u2 &= CFE_UNDERLINE;
				if (u == BST_UNCHECKED && u2)
					CheckDlgButton(hwndDlg, IDC_UNDERLINE, BST_CHECKED);
				else if (u == BST_CHECKED && u2 == 0)
					CheckDlgButton(hwndDlg, IDC_UNDERLINE, BST_UNCHECKED);
			}
		}
		break;

	case WM_DESTROY:
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
		si = (SESSION_INFO*)lParam;
		CheckDlgButton(hwndDlg, IDC_1, si->iLogFilterFlags & GC_EVENT_ACTION ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_2, si->iLogFilterFlags & GC_EVENT_MESSAGE ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_3, si->iLogFilterFlags & GC_EVENT_NICK ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_4, si->iLogFilterFlags & GC_EVENT_JOIN ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_5, si->iLogFilterFlags & GC_EVENT_PART ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_6, si->iLogFilterFlags & GC_EVENT_TOPIC ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_7, si->iLogFilterFlags & GC_EVENT_ADDSTATUS ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_8, si->iLogFilterFlags & GC_EVENT_INFORMATION ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_9, si->iLogFilterFlags & GC_EVENT_QUIT ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_10, si->iLogFilterFlags & GC_EVENT_KICK ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_11, si->iLogFilterFlags & GC_EVENT_NOTICE ? BST_CHECKED : BST_UNCHECKED);
		break;

	case WM_CTLCOLOREDIT:
	case WM_CTLCOLORSTATIC:
		SetTextColor((HDC)wParam, RGB(60, 60, 150));
		SetBkColor((HDC)wParam, GetSysColor(COLOR_WINDOW));
		return (INT_PTR)GetSysColorBrush(COLOR_WINDOW);

	case WM_ACTIVATE:
		if (LOWORD(wParam) == WA_INACTIVE) {
			int iFlags = 0;

			if (IsDlgButtonChecked(hwndDlg, IDC_1) == BST_CHECKED)
				iFlags |= GC_EVENT_ACTION;
			if (IsDlgButtonChecked(hwndDlg, IDC_2) == BST_CHECKED)
				iFlags |= GC_EVENT_MESSAGE;
			if (IsDlgButtonChecked(hwndDlg, IDC_3) == BST_CHECKED)
				iFlags |= GC_EVENT_NICK;
			if (IsDlgButtonChecked(hwndDlg, IDC_4) == BST_CHECKED)
				iFlags |= GC_EVENT_JOIN;
			if (IsDlgButtonChecked(hwndDlg, IDC_5) == BST_CHECKED)
				iFlags |= GC_EVENT_PART;
			if (IsDlgButtonChecked(hwndDlg, IDC_6) == BST_CHECKED)
				iFlags |= GC_EVENT_TOPIC;
			if (IsDlgButtonChecked(hwndDlg, IDC_7) == BST_CHECKED)
				iFlags |= GC_EVENT_ADDSTATUS;
			if (IsDlgButtonChecked(hwndDlg, IDC_8) == BST_CHECKED)
				iFlags |= GC_EVENT_INFORMATION;
			if (IsDlgButtonChecked(hwndDlg, IDC_9) == BST_CHECKED)
				iFlags |= GC_EVENT_QUIT;
			if (IsDlgButtonChecked(hwndDlg, IDC_10) == BST_CHECKED)
				iFlags |= GC_EVENT_KICK;
			if (IsDlgButtonChecked(hwndDlg, IDC_11) == BST_CHECKED)
				iFlags |= GC_EVENT_NOTICE;

			if (iFlags & GC_EVENT_ADDSTATUS)
				iFlags |= GC_EVENT_REMOVESTATUS;

			SendMessage(GetParent(hwndDlg), GC_CHANGEFILTERFLAG, 0, iFlags);
			if (si->bFilterEnabled)
				SendMessage(GetParent(hwndDlg), GC_REDRAWLOG, 0, 0);
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
			if (GetDlgItem(GetParent(hwnd), IDC_FILTER) == hwnd)
				SendMessage(GetParent(hwnd), GC_SHOWFILTERMENU, 0, 0);
			if (GetDlgItem(GetParent(hwnd), IDC_COLOR) == hwnd)
				SendMessage(GetParent(hwnd), GC_SHOWCOLORCHOOSER, 0, IDC_COLOR);
			if (GetDlgItem(GetParent(hwnd), IDC_BKGCOLOR) == hwnd)
				SendMessage(GetParent(hwnd), GC_SHOWCOLORCHOOSER, 0, IDC_BKGCOLOR);
		}
		break;
	}

	return mir_callNextSubclass(hwnd, ButtonSubclassProc, msg, wParam, lParam);
}

static LRESULT CALLBACK LogSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CHARRANGE sel;

	switch (msg) {
	case WM_LBUTTONUP:
		SendMessage(hwnd, EM_EXGETSEL, 0, (LPARAM)&sel);
		if (sel.cpMin != sel.cpMax) {
			SendMessage(hwnd, WM_COPY, 0, 0);
			sel.cpMin = sel.cpMax;
			SendMessage(hwnd, EM_EXSETSEL, 0, (LPARAM)&sel);
		}
		SetFocus(GetDlgItem(GetParent(hwnd), IDC_MESSAGE));
		break;

	case WM_KEYDOWN:
		if (wParam == 0x57 && GetKeyState(VK_CONTROL) & 0x8000) { // ctrl-w (close window)
			PostMessage(GetParent(hwnd), GC_CLOSEWINDOW, 0, 0);
			return TRUE;
		}
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

	case WM_CHAR:
		SetFocus(GetDlgItem(GetParent(hwnd), IDC_MESSAGE));
		SendDlgItemMessage(GetParent(hwnd), IDC_MESSAGE, WM_CHAR, wParam, lParam);
		break;
	}

	return mir_callNextSubclass(hwnd, LogSubclassProc, msg, wParam, lParam);
}

static LRESULT CALLBACK NicklistSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	SESSION_INFO *si = (SESSION_INFO*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

	switch (msg) {
	case WM_ERASEBKGND:
		{
			HDC dc = (HDC)wParam;
			if (dc == NULL)
				return 0;

			int index = SendMessage(hwnd, LB_GETTOPINDEX, 0, 0);
			if (index == LB_ERR || si->nUsersInNicklist <= 0)
				return 0;

			int height = SendMessage(hwnd, LB_GETITEMHEIGHT, 0, 0);
			if (height == LB_ERR)
				return 0;

			RECT rc = {};
			GetClientRect(hwnd, &rc);

			int items = si->nUsersInNicklist - index;
			if (rc.bottom - rc.top > items * height) {
				rc.top = items * height;
				FillRect(dc, &rc, pci->hListBkgBrush);
			}
		}
		return 1;

	case WM_KEYDOWN:
		if (wParam == 0x57 && GetKeyState(VK_CONTROL) & 0x8000) { // ctrl-w (close window)
			PostMessage(GetParent(hwnd), GC_CLOSEWINDOW, 0, 0);
			return TRUE;
		}
		break;

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
		TVHITTESTINFO hti;
		{
			int height = 0;
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

			int item = LOWORD(SendDlgItemMessage(GetParent(hwnd), IDC_LIST, LB_ITEMFROMPOINT, 0, MAKELPARAM(hti.pt.x, hti.pt.y)));
			USERINFO *ui = pci->SM_GetUserFromIndex(si->ptszID, si->pszModule, item);
			if (ui) {
				USERINFO uinew;
				memcpy(&uinew, ui, sizeof(USERINFO));
				if (hti.pt.x == -1 && hti.pt.y == -1)
					hti.pt.y += height - 4;
				ClientToScreen(hwnd, &hti.pt);

				HMENU hMenu = 0;
				UINT uID = CreateGCMenu(hwnd, &hMenu, 0, hti.pt, si, uinew.pszUID, uinew.pszNick);
				switch (uID) {
				case 0:
					break;

				case ID_MESS:
					pci->DoEventHookAsync(GetParent(hwnd), si->ptszID, si->pszModule, GC_USER_PRIVMESS, ui->pszUID, NULL, 0);
					break;

				default:
					pci->DoEventHookAsync(GetParent(hwnd), si->ptszID, si->pszModule, GC_USER_NICKLISTMENU, ui->pszUID, NULL, uID);
					break;
				}
				DestroyGCMenu(&hMenu, 1);
				return TRUE;
			}
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
	RECT rc = {};
	DrawText(hdc, pszText, -1, &rc, DT_CALCRECT);
	SelectObject(hdc, hOldFont);
	ReleaseDC(NULL, hdc);
	return bWidth ? rc.right - rc.left : rc.bottom - rc.top;
}

static void __cdecl phase2(void * lParam)
{
	SESSION_INFO *si = (SESSION_INFO*)lParam;
	Sleep(30);
	if (si && si->hWnd)
		PostMessage(si->hWnd, GC_REDRAWLOG2, 0, 0);
}

/////////////////////////////////////////////////////////////////////////////////////////

CChatRoomDlg::CChatRoomDlg(SESSION_INFO *si) :
	CDlgBase(g_hInst, g_Settings.bTabsEnable ? IDD_CHANNEL_TAB : IDD_CHANNEL),
	m_si(si),
	m_nickList(this, IDC_LIST),
	m_message(this, IDC_MESSAGE),
	m_log(this, IDC_LOG),

	m_btnOk(this, IDOK),

	m_btnBold(this, IDC_BOLD),
	m_btnItalic(this, IDC_ITALICS),
	m_btnUnderline(this, IDC_UNDERLINE),

	m_btnColor(this, IDC_COLOR),
	m_btnBkColor(this, IDC_BKGCOLOR),

	m_btnFilter(this, IDC_FILTER),
	m_btnHistory(this, IDC_HISTORY),
	m_btnNickList(this, IDC_SHOWNICKLIST),
	m_btnChannelMgr(this, IDC_CHANMGR)
{
	m_autoClose = 0;

	m_btnBold.OnClick = Callback(this, &CChatRoomDlg::OnClick_Bold);
	m_btnItalic.OnClick = Callback(this, &CChatRoomDlg::OnClick_Bold);
	m_btnUnderline.OnClick = Callback(this, &CChatRoomDlg::OnClick_Bold);

	m_btnColor.OnClick = Callback(this, &CChatRoomDlg::OnClick_Color);
	m_btnBkColor.OnClick = Callback(this, &CChatRoomDlg::OnClick_BkColor);

	m_btnOk.OnClick = Callback(this, &CChatRoomDlg::OnClick_Ok);

	m_btnFilter.OnClick = Callback(this, &CChatRoomDlg::OnClick_Filter);
	m_btnHistory.OnClick = Callback(this, &CChatRoomDlg::OnClick_History);
	m_btnChannelMgr.OnClick = Callback(this, &CChatRoomDlg::OnClick_Options);
	m_btnNickList.OnClick = Callback(this, &CChatRoomDlg::OnClick_NickList);

	m_nickList.OnDblClick = Callback(this, &CChatRoomDlg::OnListDblclick);
}

void CChatRoomDlg::OnInitDialog()
{
	m_si->hWnd = m_hwnd;
	m_si->pDlg = this;

	if (g_Settings.bTabsEnable)
		SetWindowLongPtr(m_hwnd, GWL_EXSTYLE, GetWindowLongPtr(m_hwnd, GWL_EXSTYLE) | WS_EX_APPWINDOW);

	// initialize toolbar icons
	Srmm_CreateToolbarIcons(m_hwnd, BBBF_ISCHATBUTTON);
	m_btnBold.OnInit(); m_btnItalic.OnInit(); m_btnUnderline.OnInit();
	m_btnColor.OnInit(); m_btnBkColor.OnInit();
	m_btnFilter.OnInit(); m_btnHistory.OnInit(); m_btnChannelMgr.OnInit();

	WindowList_Add(pci->hWindowList, m_hwnd, m_si->hContact);

	NotifyLocalWinEvent(m_si->hContact, m_hwnd, MSG_WINDOW_EVT_OPENING);
	mir_subclassWindow(GetDlgItem(m_hwnd, IDC_SPLITTERX), SplitterSubclassProc);
	mir_subclassWindow(GetDlgItem(m_hwnd, IDC_SPLITTERY), SplitterSubclassProc);
	mir_subclassWindow(m_log.GetHwnd(), LogSubclassProc);
	mir_subclassWindow(m_btnFilter.GetHwnd(), ButtonSubclassProc);
	mir_subclassWindow(m_btnColor.GetHwnd(), ButtonSubclassProc);
	mir_subclassWindow(m_btnBkColor.GetHwnd(), ButtonSubclassProc);
	mir_subclassWindow(m_message.GetHwnd(), MessageSubclassProc);

	SetWindowLongPtr(m_nickList.GetHwnd(), GWLP_USERDATA, LPARAM(m_si));
	mir_subclassWindow(m_nickList.GetHwnd(), NicklistSubclassProc);

	SendDlgItemMessage(m_hwnd, IDC_MESSAGE, EM_SUBCLASSED, 0, LPARAM(m_si));
	SendDlgItemMessage(m_hwnd, IDC_LOG, EM_AUTOURLDETECT, 1, 0);

	int mask = (int)SendDlgItemMessage(m_hwnd, IDC_LOG, EM_GETEVENTMASK, 0, 0);
	SendDlgItemMessage(m_hwnd, IDC_LOG, EM_SETEVENTMASK, 0, mask | ENM_LINK | ENM_MOUSEEVENTS);
	SendDlgItemMessage(m_hwnd, IDC_LOG, EM_LIMITTEXT, sizeof(wchar_t) * 0x7FFFFFFF, 0);
	SendDlgItemMessage(m_hwnd, IDC_LOG, EM_SETOLECALLBACK, 0, (LPARAM)&reOleCallback);

	m_si->hwndStatus = CreateWindowEx(0, STATUSCLASSNAME, NULL, WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP | SBT_TOOLTIPS, 0, 0, 0, 0, m_hwnd, NULL, g_hInst, NULL);
	SendMessage(m_si->hwndStatus, SB_SETMINHEIGHT, GetSystemMetrics(SM_CYSMICON), 0);

	SendDlgItemMessage(m_hwnd, IDC_LOG, EM_HIDESELECTION, TRUE, 0);

	SendMessage(m_hwnd, GC_SETWNDPROPS, 0, 0);
	SendMessage(m_hwnd, GC_UPDATESTATUSBAR, 0, 0);
	SendMessage(m_hwnd, GC_UPDATETITLE, 0, 0);
	SetWindowPosition();

	SendMessage(m_hwnd, WM_SIZE, 0, 0);

	NotifyLocalWinEvent(m_si->hContact, m_hwnd, MSG_WINDOW_EVT_OPEN);
}

void CChatRoomDlg::OnDestroy()
{
	NotifyLocalWinEvent(m_si->hContact, m_hwnd, MSG_WINDOW_EVT_CLOSING);
	SaveWindowPosition(true);

	WindowList_Remove(pci->hWindowList, m_hwnd);

	m_si->pDlg = NULL;
	m_si->hWnd = NULL;
	m_si->wState &= ~STATE_TALK;
	DestroyWindow(m_si->hwndStatus); m_si->hwndStatus = NULL;

	NotifyLocalWinEvent(m_si->hContact, m_hwnd, MSG_WINDOW_EVT_CLOSE);
}

void CChatRoomDlg::OnClick_Bold(CCtrlButton *pButton)
{
	if (!pButton->Enabled())
		return;

	CHARFORMAT2 cf;
	cf.cbSize = sizeof(CHARFORMAT2);
	cf.dwMask = CFM_BOLD | CFM_ITALIC | CFM_UNDERLINE;
	cf.dwEffects = 0;
	switch (pButton->GetCtrlId()) {
		case IDC_BOLD: cf.dwEffects |= CFE_BOLD; break;
		case IDC_ITALICS: cf.dwEffects |= CFE_ITALIC; break;
		case IDC_UNDERLINE: cf.dwEffects |= CFE_UNDERLINE; break;
	}
	SendDlgItemMessage(m_hwnd, IDC_MESSAGE, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
}

void CChatRoomDlg::OnClick_Color(CCtrlButton *pButton)
{
	if (!pButton->Enabled())
		return;

	CHARFORMAT2 cf;
	cf.cbSize = sizeof(CHARFORMAT2);
	cf.dwEffects = 0;

	if (IsDlgButtonChecked(m_hwnd, IDC_COLOR)) {
		if (db_get_b(NULL, CHAT_MODULE, "RightClickFilter", 0) == 0)
			SendMessage(m_hwnd, GC_SHOWCOLORCHOOSER, 0, IDC_COLOR);
		else if (m_si->bFGSet) {
			cf.dwMask = CFM_COLOR;
			cf.crTextColor = pci->MM_FindModule(m_si->pszModule)->crColors[m_si->iFG];
			SendDlgItemMessage(m_hwnd, IDC_MESSAGE, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
		}
	}
	else {
		cf.dwMask = CFM_COLOR;
		cf.crTextColor = g_Settings.MessageAreaColor;
		SendDlgItemMessage(m_hwnd, IDC_MESSAGE, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
	}
}

void CChatRoomDlg::OnClick_BkColor(CCtrlButton *pButton)
{
	if (!pButton->Enabled())
		return;
	
	CHARFORMAT2 cf;
	cf.cbSize = sizeof(CHARFORMAT2);
	cf.dwEffects = 0;

	if (IsDlgButtonChecked(m_hwnd, IDC_BKGCOLOR)) {
		if (db_get_b(NULL, CHAT_MODULE, "RightClickFilter", 0) == 0)
			SendMessage(m_hwnd, GC_SHOWCOLORCHOOSER, 0, IDC_BKGCOLOR);
		else if (m_si->bBGSet) {
			cf.dwMask = CFM_BACKCOLOR;
			cf.crBackColor = pci->MM_FindModule(m_si->pszModule)->crColors[m_si->iBG];
			SendDlgItemMessage(m_hwnd, IDC_MESSAGE, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
		}
	}
	else {
		cf.dwMask = CFM_BACKCOLOR;
		cf.crBackColor = (COLORREF)db_get_dw(NULL, CHAT_MODULE, "ColorMessageBG", GetSysColor(COLOR_WINDOW));
		SendDlgItemMessage(m_hwnd, IDC_MESSAGE, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
	}
}

void CChatRoomDlg::OnClick_Filter(CCtrlButton *pButton)
{
	if (!pButton->Enabled())
		return;

	m_si->bFilterEnabled = !m_si->bFilterEnabled;
	SendDlgItemMessage(m_hwnd, IDC_FILTER, BM_SETIMAGE, IMAGE_ICON, (LPARAM)LoadIconEx(m_si->bFilterEnabled ? "filter" : "filter2", FALSE));
	if (m_si->bFilterEnabled && db_get_b(NULL, CHAT_MODULE, "RightClickFilter", 0) == 0)
		SendMessage(m_hwnd, GC_SHOWFILTERMENU, 0, 0);
	else
		SendMessage(m_hwnd, GC_REDRAWLOG, 0, 0);
}

void CChatRoomDlg::OnClick_History(CCtrlButton *pButton)
{
	if (!pButton->Enabled())
		return;

	MODULEINFO *pInfo = pci->MM_FindModule(m_si->pszModule);
	if (pInfo)
		ShellExecute(m_hwnd, NULL, pci->GetChatLogsFilename(m_si, 0), NULL, NULL, SW_SHOW);
}

void CChatRoomDlg::OnClick_NickList(CCtrlButton *pButton)
{
	if (!pButton->Enabled() || m_si->iType == GCW_SERVER)
		return;

	m_si->bNicklistEnabled = !m_si->bNicklistEnabled;
	pButton->SendMsg(BM_SETIMAGE, IMAGE_ICON, (LPARAM)LoadIconEx(m_si->bNicklistEnabled ? "nicklist" : "nicklist2", FALSE));

	SendMessage(m_hwnd, GC_SCROLLTOBOTTOM, 0, 0);
	SendMessage(m_hwnd, WM_SIZE, 0, 0);
}

void CChatRoomDlg::OnClick_Options(CCtrlButton *pButton)
{
	if (pButton->Enabled())
		pci->DoEventHookAsync(m_hwnd, m_si->ptszID, m_si->pszModule, GC_USER_CHANMGR, NULL, NULL, 0);
}

void CChatRoomDlg::OnClick_Ok(CCtrlButton *pButton)
{
	if (!pButton->Enabled())
		return;

	ptrA pszRtf(Message_GetFromStream(m_hwnd, m_si));
	if (pszRtf == NULL)
		return;

	MODULEINFO *mi = pci->MM_FindModule(m_si->pszModule);
	if (mi == NULL)
		return;

	pci->SM_AddCommand(m_si->ptszID, m_si->pszModule, pszRtf);

	CMStringW ptszText(ptrW(mir_utf8decodeW(pszRtf)));
	pci->DoRtfToTags(ptszText, mi->nColorCount, mi->crColors);
	ptszText.Trim();
	ptszText.Replace(L"%", L"%%");

	if (mi->bAckMsg) {
		EnableWindow(m_message.GetHwnd(), FALSE);
		SendDlgItemMessage(m_hwnd, IDC_MESSAGE, EM_SETREADONLY, TRUE, 0);
	}
	else SetDlgItemText(m_hwnd, IDC_MESSAGE, L"");

	EnableWindow(m_btnOk.GetHwnd(), FALSE);

	pci->DoEventHookAsync(m_hwnd, m_si->ptszID, m_si->pszModule, GC_USER_MESSAGE, NULL, ptszText, 0);

	SetFocus(m_message.GetHwnd());
}

void CChatRoomDlg::OnListDblclick(CCtrlListBox*)
{
	TVHITTESTINFO hti;
	hti.pt.x = (short)LOWORD(GetMessagePos());
	hti.pt.y = (short)HIWORD(GetMessagePos());
	ScreenToClient(m_nickList.GetHwnd(), &hti.pt);

	int item = LOWORD(SendDlgItemMessage(m_hwnd, IDC_LIST, LB_ITEMFROMPOINT, 0, MAKELPARAM(hti.pt.x, hti.pt.y)));
	USERINFO *ui = pci->SM_GetUserFromIndex(m_si->ptszID, m_si->pszModule, item);
	if (ui == NULL)
		return;

	if (GetKeyState(VK_SHIFT) & 0x8000) {
		int start = LOWORD(SendDlgItemMessage(m_hwnd, IDC_MESSAGE, EM_GETSEL, 0, 0));
		CMStringW buf(FORMAT, (start == 0) ? L"%s: " : L"%s ", ui->pszUID);
		SendDlgItemMessage(m_hwnd, IDC_MESSAGE, EM_REPLACESEL, FALSE, (LPARAM)buf.c_str());
		PostMessage(m_hwnd, WM_MOUSEACTIVATE, 0, 0);
	}
	else pci->DoEventHookAsync(m_hwnd, m_si->ptszID, m_si->pszModule, GC_USER_PRIVMESS, ui->pszUID, NULL, 0);
}

void CChatRoomDlg::SetWindowPosition()
{
	if (g_Settings.bTabsEnable)
		return;

	WINDOWPLACEMENT wp;
	wp.length = sizeof(wp);
	GetWindowPlacement(m_hwnd, &wp);

	RECT screen;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &screen, 0);

	if (m_si->iX) {
		wp.rcNormalPosition.left = m_si->iX;
		wp.rcNormalPosition.top = m_si->iY;
		wp.rcNormalPosition.right = wp.rcNormalPosition.left + m_si->iWidth;
		wp.rcNormalPosition.bottom = wp.rcNormalPosition.top + m_si->iHeight;
		wp.showCmd = SW_HIDE;
		SetWindowPlacement(m_hwnd, &wp);
		return;
	}
	
	if (db_get_b(NULL, CHAT_MODULE, "SavePosition", 0)) {
		if (RestoreWindowPosition(m_hwnd, m_si->hContact, CHAT_MODULE, "room", SW_HIDE))
			return;
		SetWindowPos(m_hwnd, 0, (screen.right - screen.left) / 2 - (550) / 2, (screen.bottom - screen.top) / 2 - (400) / 2, (550), (400), SWP_NOZORDER | SWP_HIDEWINDOW | SWP_NOACTIVATE);
	}
	else SetWindowPos(m_hwnd, 0, (screen.right - screen.left) / 2 - (550) / 2, (screen.bottom - screen.top) / 2 - (400) / 2, (550), (400), SWP_NOZORDER | SWP_HIDEWINDOW | SWP_NOACTIVATE);

	SESSION_INFO *pActive = pci->GetActiveSession();
	if (pActive && pActive->hWnd && db_get_b(NULL, CHAT_MODULE, "CascadeWindows", 1)) {
		RECT rcThis, rcNew;
		int dwFlag = SWP_NOZORDER | SWP_NOACTIVATE;
		if (!IsWindowVisible(m_hwnd))
			dwFlag |= SWP_HIDEWINDOW;

		GetWindowRect(m_hwnd, &rcThis);
		GetWindowRect(pActive->hWnd, &rcNew);

		int offset = GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYFRAME);
		SetWindowPos(m_hwnd, 0, rcNew.left + offset, rcNew.top + offset, rcNew.right - rcNew.left, rcNew.bottom - rcNew.top, dwFlag);
	}
}

void CChatRoomDlg::SaveWindowPosition(bool bUpdateSession)
{
	WINDOWPLACEMENT wp = {};
	wp.length = sizeof(wp);
	GetWindowPlacement(getCaptionWindow(), &wp);

	g_Settings.iX = wp.rcNormalPosition.left;
	g_Settings.iY = wp.rcNormalPosition.top;
	g_Settings.iWidth = wp.rcNormalPosition.right - wp.rcNormalPosition.left;
	g_Settings.iHeight = wp.rcNormalPosition.bottom - wp.rcNormalPosition.top;

	if (bUpdateSession) {
		m_si->iX = g_Settings.iX;
		m_si->iY = g_Settings.iY;
		m_si->iWidth = g_Settings.iWidth;
		m_si->iHeight = g_Settings.iHeight;
	}
}

INT_PTR CChatRoomDlg::DlgProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	SESSION_INFO *s;
	RECT rc;
	CHARRANGE sel;

	switch (uMsg) {
	case GC_SETWNDPROPS:
		SendDlgItemMessage(m_hwnd, IDC_SHOWNICKLIST, BM_SETIMAGE, IMAGE_ICON, (LPARAM)LoadIconEx(m_si->bNicklistEnabled ? "nicklist" : "nicklist2", FALSE));
		SendDlgItemMessage(m_hwnd, IDC_FILTER, BM_SETIMAGE, IMAGE_ICON, (LPARAM)LoadIconEx(m_si->bFilterEnabled ? "filter" : "filter2", FALSE));
		{
			MODULEINFO *mi = pci->MM_FindModule(m_si->pszModule);
			EnableWindow(m_btnBold.GetHwnd(), mi->bBold);
			EnableWindow(m_btnItalic.GetHwnd(), mi->bItalics);
			EnableWindow(m_btnUnderline.GetHwnd(), mi->bUnderline);
			EnableWindow(m_btnColor.GetHwnd(), mi->bColor);
			EnableWindow(m_btnBkColor.GetHwnd(), mi->bBkgColor);
			if (m_si->iType == GCW_CHATROOM)
				EnableWindow(m_btnChannelMgr.GetHwnd(), mi->bChanMgr);

			HICON hIcon = m_si->wStatus == ID_STATUS_ONLINE ? mi->hOnlineIcon : mi->hOfflineIcon;
			if (!hIcon) {
				pci->MM_IconsChanged();
				hIcon = (m_si->wStatus == ID_STATUS_ONLINE) ? mi->hOnlineIcon : mi->hOfflineIcon;
			}

			SendMessage(m_hwndParent, GC_FIXTABICONS, 0, 0);
			SendMessage(m_si->hwndStatus, SB_SETICON, 0, (LPARAM)hIcon);
		}
		Window_SetIcon_IcoLib(getCaptionWindow(), GetIconHandle("window"));

		SendDlgItemMessage(m_hwnd, IDC_LOG, EM_SETBKGNDCOLOR, 0, g_Settings.crLogBackground);

		CHARFORMAT2 cf;
		cf.cbSize = sizeof(CHARFORMAT2);
		cf.dwMask = CFM_COLOR | CFM_BOLD | CFM_UNDERLINE | CFM_BACKCOLOR;
		cf.dwEffects = 0;
		cf.crTextColor = g_Settings.MessageAreaColor;
		cf.crBackColor = (COLORREF)db_get_dw(NULL, CHAT_MODULE, "ColorMessageBG", GetSysColor(COLOR_WINDOW));
		SendDlgItemMessage(m_hwnd, IDC_MESSAGE, EM_SETBKGNDCOLOR, 0, db_get_dw(NULL, CHAT_MODULE, "ColorMessageBG", GetSysColor(COLOR_WINDOW)));
		SendDlgItemMessage(m_hwnd, IDC_MESSAGE, WM_SETFONT, (WPARAM)g_Settings.MessageAreaFont, MAKELPARAM(TRUE, 0));
		SendDlgItemMessage(m_hwnd, IDC_MESSAGE, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf);
		{
			// nicklist
			int ih = GetTextPixelSize(L"AQGglo", g_Settings.UserListFont, FALSE);
			int ih2 = GetTextPixelSize(L"AQGglo", g_Settings.UserListHeadingsFont, FALSE);
			int height = db_get_b(NULL, CHAT_MODULE, "NicklistRowDist", 12);
			int font = ih > ih2 ? ih : ih2;

			// make sure we have space for icon!
			if (g_Settings.bShowContactStatus)
				font = font > 16 ? font : 16;

			SendDlgItemMessage(m_hwnd, IDC_LIST, LB_SETITEMHEIGHT, 0, height > font ? height : font);
			InvalidateRect(m_nickList.GetHwnd(), NULL, TRUE);
		}
		SendMessage(m_hwnd, WM_SIZE, 0, 0);
		SendMessage(m_hwnd, GC_REDRAWLOG2, 0, 0);
		break;

	case GC_UPDATETITLE:
		wchar_t szTemp[100];
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

		SetWindowText(getCaptionWindow(), szTemp);
		break;

	case WM_CBD_LOADICONS:
		Srmm_UpdateToolbarIcons(m_hwnd);
		break;

	case WM_CBD_UPDATED:
		SetButtonsPos(m_hwnd, true);
		break;

	case GC_UPDATESTATUSBAR:
		{
			MODULEINFO *mi = pci->MM_FindModule(m_si->pszModule);
			wchar_t *ptszDispName = mi->ptszModDispName;
			int x = 12;
			x += GetTextPixelSize(ptszDispName, (HFONT)SendMessage(m_si->hwndStatus, WM_GETFONT, 0, 0), TRUE);
			x += GetSystemMetrics(SM_CXSMICON);
			int iStatusbarParts[2] = { x, -1 };
			SendMessage(m_si->hwndStatus, SB_SETPARTS, 2, (LPARAM)&iStatusbarParts);

			// stupid hack to make icons show. I dunno why this is needed currently
			HICON hIcon = m_si->wStatus == ID_STATUS_ONLINE ? mi->hOnlineIcon : mi->hOfflineIcon;
			if (!hIcon) {
				pci->MM_IconsChanged();
				hIcon = m_si->wStatus == ID_STATUS_ONLINE ? mi->hOnlineIcon : mi->hOfflineIcon;
			}

			SendMessage(m_si->hwndStatus, SB_SETICON, 0, (LPARAM)hIcon);
			SendMessage(m_hwndParent, GC_FIXTABICONS, 0, 0);

			SendMessage(m_si->hwndStatus, SB_SETTEXT, 0, (LPARAM)ptszDispName);

			SendMessage(m_si->hwndStatus, SB_SETTEXT, 1, (LPARAM)(m_si->ptszStatusbarText ? m_si->ptszStatusbarText : L""));
			SendMessage(m_si->hwndStatus, SB_SETTIPTEXT, 1, (LPARAM)(m_si->ptszStatusbarText ? m_si->ptszStatusbarText : L""));
		}
		return TRUE;

	case WM_SIZE:
		if (wParam == SIZE_MAXIMIZED)
			PostMessage(m_hwnd, GC_SCROLLTOBOTTOM, 0, 0);

		if (!IsIconic(m_hwnd)) {
			SendMessage(m_si->hwndStatus, WM_SIZE, 0, 0);

			BOOL bControl = (BOOL)db_get_b(NULL, CHAT_MODULE, "ShowTopButtons", 1);
			BOOL bFormat = (BOOL)db_get_b(NULL, CHAT_MODULE, "ShowFormatButtons", 1);
			BOOL bSend = (BOOL)db_get_b(NULL, CHAT_MODULE, "ShowSend", 0);
			BOOL bNick = m_si->iType != GCW_SERVER && m_si->bNicklistEnabled;

			ShowWindow(m_btnBold.GetHwnd(), bFormat ? SW_SHOW : SW_HIDE);
			ShowWindow(m_btnItalic.GetHwnd(), bFormat ? SW_SHOW : SW_HIDE);
			ShowWindow(m_btnUnderline.GetHwnd(), bFormat ? SW_SHOW : SW_HIDE);
			
			ShowWindow(m_btnColor.GetHwnd(), bFormat ? SW_SHOW : SW_HIDE);
			ShowWindow(m_btnBkColor.GetHwnd(), bFormat ? SW_SHOW : SW_HIDE);
			ShowWindow(m_btnHistory.GetHwnd(), bControl ? SW_SHOW : SW_HIDE);
			ShowWindow(m_btnNickList.GetHwnd(), bControl ? SW_SHOW : SW_HIDE);
			ShowWindow(m_btnFilter.GetHwnd(), bControl ? SW_SHOW : SW_HIDE);
			ShowWindow(m_btnChannelMgr.GetHwnd(), bControl ? SW_SHOW : SW_HIDE);
			ShowWindow(m_btnOk.GetHwnd(), bSend ? SW_SHOW : SW_HIDE);
			ShowWindow(GetDlgItem(m_hwnd, IDC_SPLITTERX), bNick ? SW_SHOW : SW_HIDE);
			if (m_si->iType != GCW_SERVER)
				ShowWindow(m_nickList.GetHwnd(), m_si->bNicklistEnabled ? SW_SHOW : SW_HIDE);
			else
				ShowWindow(m_nickList.GetHwnd(), SW_HIDE);

			if (m_si->iType == GCW_SERVER) {
				EnableWindow(m_btnNickList.GetHwnd(), FALSE);
				EnableWindow(m_btnFilter.GetHwnd(), FALSE);
				EnableWindow(m_btnChannelMgr.GetHwnd(), FALSE);
			}
			else {
				EnableWindow(m_btnNickList.GetHwnd(), TRUE);
				EnableWindow(m_btnFilter.GetHwnd(), TRUE);
				if (m_si->iType == GCW_CHATROOM)
					EnableWindow(m_btnChannelMgr.GetHwnd(), pci->MM_FindModule(m_si->pszModule)->bChanMgr);
			}

			Utils_ResizeDialog(m_hwnd, g_hInst, MAKEINTRESOURCEA(IDD_CHANNEL), RoomWndResize, (LPARAM)m_si);
			SetButtonsPos(m_hwnd, true);

			InvalidateRect(m_si->hwndStatus, NULL, TRUE);
			RedrawWindow(m_message.GetHwnd(), NULL, NULL, RDW_INVALIDATE);
			RedrawWindow(m_btnOk.GetHwnd(), NULL, NULL, RDW_INVALIDATE);
			SaveWindowPosition(false);
		}
		return TRUE;

	case GC_REDRAWWINDOW:
		InvalidateRect(m_hwnd, NULL, TRUE);
		break;

	case GC_REDRAWLOG:
		m_si->LastTime = 0;
		if (m_si->pLog) {
			LOGINFO * pLog = m_si->pLog;
			if (m_si->iEventCount > 60) {
				int index = 0;
				while (index < 59) {
					if (pLog->next == NULL)
						break;

					pLog = pLog->next;
					if (m_si->iType != GCW_CHATROOM || !m_si->bFilterEnabled || (m_si->iLogFilterFlags&pLog->iType) != 0)
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

	case WM_CTLCOLORLISTBOX:
		SetBkColor((HDC)wParam, g_Settings.crUserListBGColor);
		return (INT_PTR)pci->hListBkgBrush;

	case WM_MEASUREITEM:
		{
			MEASUREITEMSTRUCT *mis = (MEASUREITEMSTRUCT *)lParam;
			if (mis->CtlType == ODT_MENU)
				return Menu_MeasureItem(lParam);

			int ih = GetTextPixelSize(L"AQGgl'", g_Settings.UserListFont, FALSE);
			int ih2 = GetTextPixelSize(L"AQGg'", g_Settings.UserListHeadingsFont, FALSE);
			int font = ih > ih2 ? ih : ih2;
			int height = db_get_b(NULL, CHAT_MODULE, "NicklistRowDist", 12);

			// make sure we have space for icon!
			if (g_Settings.bShowContactStatus)
				font = font > 16 ? font : 16;

			mis->itemHeight = height > font ? height : font;
		}
		return TRUE;

	case WM_DRAWITEM:
		{
			DRAWITEMSTRUCT *dis = (DRAWITEMSTRUCT *)lParam;
			if (dis->CtlType == ODT_MENU)
				return Menu_DrawItem(lParam);

			if (dis->CtlID == IDC_LIST) {
				int index = dis->itemID;
				USERINFO *ui = pci->SM_GetUserFromIndex(m_si->ptszID, m_si->pszModule, index);
				if (ui) {
					int x_offset = 2;

					int height = dis->rcItem.bottom - dis->rcItem.top;
					if (height & 1)
						height++;

					int offset = (height == 10) ? 0 : height / 2 - 4;
					HFONT hFont = (ui->iStatusEx == 0) ? g_Settings.UserListFont : g_Settings.UserListHeadingsFont;
					HFONT hOldFont = (HFONT)SelectObject(dis->hDC, hFont);
					SetBkMode(dis->hDC, TRANSPARENT);

					if (dis->itemAction == ODA_FOCUS && dis->itemState & ODS_SELECTED)
						FillRect(dis->hDC, &dis->rcItem, pci->hListSelectedBkgBrush);
					else //if (dis->itemState & ODS_INACTIVE)
						FillRect(dis->hDC, &dis->rcItem, pci->hListBkgBrush);

					if (g_Settings.bShowContactStatus && g_Settings.bContactStatusFirst && ui->ContactStatus) {
						HICON hIcon = Skin_LoadProtoIcon(m_si->pszModule, ui->ContactStatus);
						DrawIconEx(dis->hDC, x_offset, dis->rcItem.top + offset - 3, hIcon, 16, 16, 0, NULL, DI_NORMAL);
						x_offset += 18;
					}
					DrawIconEx(dis->hDC, x_offset, dis->rcItem.top + offset, pci->SM_GetStatusIcon(m_si, ui), 10, 10, 0, NULL, DI_NORMAL);
					x_offset += 12;
					if (g_Settings.bShowContactStatus && !g_Settings.bContactStatusFirst && ui->ContactStatus) {
						HICON hIcon = Skin_LoadProtoIcon(m_si->pszModule, ui->ContactStatus);
						DrawIconEx(dis->hDC, x_offset, dis->rcItem.top + offset - 3, hIcon, 16, 16, 0, NULL, DI_NORMAL);
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
		{
			int i = SendDlgItemMessage(m_hwnd, IDC_LIST, LB_GETTOPINDEX, 0, 0);
			SendDlgItemMessage(m_hwnd, IDC_LIST, LB_SETCOUNT, m_si->nUsersInNicklist, 0);
			SendDlgItemMessage(m_hwnd, IDC_LIST, LB_SETTOPINDEX, i, 0);
			SendMessage(m_hwnd, GC_UPDATETITLE, 0, 0);
		}
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
			SetDlgItemText(m_hwnd, IDC_LOG, L"");
			return TRUE;

		case SESSION_TERMINATE:
			if (!g_Settings.bTabsEnable) {
				SaveWindowPosition(true);
				if (db_get_b(NULL, CHAT_MODULE, "SavePosition", 0)) {
					db_set_dw(m_si->hContact, CHAT_MODULE, "roomx", m_si->iX);
					db_set_dw(m_si->hContact, CHAT_MODULE, "roomy", m_si->iY);
					db_set_dw(m_si->hContact, CHAT_MODULE, "roomwidth", m_si->iWidth);
					db_set_dw(m_si->hContact, CHAT_MODULE, "roomheight", m_si->iHeight);
				}
			}
			
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
		{
			static int x = 0;

			RECT rcLog;
			GetWindowRect(m_log.GetHwnd(), &rcLog);

			if ((HWND)lParam == GetDlgItem(m_hwnd, IDC_SPLITTERX)) {
				int oldSplitterX;
				GetClientRect(m_hwnd, &rc);
				POINT pt = { wParam, 0 };
				ScreenToClient(m_hwnd, &pt);

				oldSplitterX = m_si->iSplitterX;
				m_si->iSplitterX = rc.right - pt.x + 1;
				if (m_si->iSplitterX < 35)
					m_si->iSplitterX = 35;
				if (m_si->iSplitterX > rc.right - rc.left - 35)
					m_si->iSplitterX = rc.right - rc.left - 35;
				g_Settings.iSplitterX = m_si->iSplitterX;
			}
			else if ((HWND)lParam == GetDlgItem(m_hwnd, IDC_SPLITTERY)) {
				BOOL bFormat = IsWindowVisible(m_btnBold.GetHwnd());
				int oldSplitterY;
				GetClientRect(m_hwnd, &rc);
				POINT pt = { 0, wParam };
				ScreenToClient(m_hwnd, &pt);

				oldSplitterY = m_si->iSplitterY;
				m_si->iSplitterY = bFormat ? rc.bottom - pt.y + 1 : rc.bottom - pt.y + 20;
				if (m_si->iSplitterY < 63)
					m_si->iSplitterY = 63;
				if (m_si->iSplitterY > rc.bottom - rc.top - 40)
					m_si->iSplitterY = rc.bottom - rc.top - 40;
				g_Settings.iSplitterY = m_si->iSplitterY;
			}
			if (x == 2) {
				PostMessage(m_hwnd, WM_SIZE, 0, 0);
				x = 0;
			}
			else x++;
		}
		break;

	case GC_FIREHOOK:
		if (lParam) {
			GCHOOK *gch = (GCHOOK *)lParam;
			NotifyEventHooks(pci->hSendEvent, 0, lParam);
			if (gch->pDest) {
				mir_free((void*)gch->pDest->ptszID);
				mir_free((void*)gch->pDest->pszModule);
				mir_free(gch->pDest);
			}
			mir_free((void*)gch->ptszText);
			mir_free((void*)gch->ptszUID);
			mir_free(gch);
		}
		break;

	case GC_CLOSEWINDOW:
		if (g_Settings.bTabsEnable)
			SendMessage(GetParent(m_hwndParent), GC_REMOVETAB, 0, (LPARAM)this);
		Close();
		break;

	case GC_CHANGEFILTERFLAG:
		m_si->iLogFilterFlags = lParam;
		break;

	case GC_SHOWFILTERMENU:
		{
			HWND hwnd = CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_FILTER), m_hwnd, FilterWndProc, (LPARAM)m_si);
			TranslateDialogDefault(hwnd);
			GetWindowRect(m_btnFilter.GetHwnd(), &rc);
			SetWindowPos(hwnd, HWND_TOP, rc.left - 85, (IsWindowVisible(m_btnFilter.GetHwnd()) || IsWindowVisible(m_btnBold.GetHwnd())) ? rc.top - 206 : rc.top - 186, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
		}
		break;

	case GC_SHOWCOLORCHOOSER:
		pci->ColorChooser(m_si, lParam == IDC_COLOR, m_hwnd, m_message.GetHwnd(), GetDlgItem(m_hwnd, lParam));
		break;

	case GC_SCROLLTOBOTTOM:
		if ((GetWindowLongPtr(m_log.GetHwnd(), GWL_STYLE) & WS_VSCROLL) != 0) {
			SCROLLINFO scroll = {};
			scroll.cbSize = sizeof(scroll);
			scroll.fMask = SIF_PAGE | SIF_RANGE;
			GetScrollInfo(m_log.GetHwnd(), SB_VERT, &scroll);

			scroll.fMask = SIF_POS;
			scroll.nPos = scroll.nMax - scroll.nPage + 1;
			SetScrollInfo(m_log.GetHwnd(), SB_VERT, &scroll, TRUE);

			sel.cpMin = sel.cpMax = GetRichTextLength(m_log.GetHwnd());
			SendDlgItemMessage(m_hwnd, IDC_LOG, EM_EXSETSEL, 0, (LPARAM)&sel);
			PostMessage(m_log.GetHwnd(), WM_VSCROLL, MAKEWPARAM(SB_BOTTOM, 0), 0);
		}
		break;

	case WM_TIMER:
		if (wParam == TIMERID_FLASHWND)
			FlashWindow(m_hwnd, TRUE);
		break;

	case WM_ACTIVATE:
		if (LOWORD(wParam) == WA_INACTIVE) {
			if (g_Settings.bTabsEnable) {
				m_si->wState &= ~GC_EVENT_HIGHLIGHT;
				m_si->wState &= ~STATE_TALK;
				SendMessage(m_hwndParent, GC_FIXTABICONS, 0, (LPARAM)this);
			}
			break;
		}
		if (LOWORD(wParam) != WA_ACTIVE)
			break;
		// fall through

	case WM_MOUSEACTIVATE:
		{
			WINDOWPLACEMENT wp = {};
			wp.length = sizeof(wp);
			GetWindowPlacement(m_hwnd, &wp);
			g_Settings.iX = wp.rcNormalPosition.left;
			g_Settings.iY = wp.rcNormalPosition.top;
			g_Settings.iWidth = wp.rcNormalPosition.right - wp.rcNormalPosition.left;
			g_Settings.iHeight = wp.rcNormalPosition.bottom - wp.rcNormalPosition.top;

			if (uMsg != WM_ACTIVATE)
				SetFocus(m_message.GetHwnd());

			pci->SetActiveSession(m_si->ptszID, m_si->pszModule);

			if (KillTimer(m_hwnd, TIMERID_FLASHWND))
				FlashWindow(m_hwnd, FALSE);
			if (db_get_w(m_si->hContact, m_si->pszModule, "ApparentMode", 0) != 0)
				db_set_w(m_si->hContact, m_si->pszModule, "ApparentMode", 0);
			if (pcli->pfnGetEvent(m_si->hContact, 0))
				pcli->pfnRemoveEvent(m_si->hContact, GC_FAKE_EVENT);
		}
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code) {
		case EN_MSGFILTER:
			if (((LPNMHDR)lParam)->idFrom == IDC_LOG && ((MSGFILTER *)lParam)->msg == WM_RBUTTONUP) {
				POINT pt = { GET_X_LPARAM(((ENLINK *)lParam)->lParam), GET_Y_LPARAM(((ENLINK *)lParam)->lParam) };
				ClientToScreen(((LPNMHDR)lParam)->hwndFrom, &pt);

				// fixing stuff for searches
				POINTL ptl = { (LONG)pt.x, (LONG)pt.y };
				ScreenToClient(m_log.GetHwnd(), (LPPOINT)&ptl);
				long iCharIndex = SendDlgItemMessage(m_hwnd, IDC_LOG, EM_CHARFROMPOS, 0, (LPARAM)&ptl);
				if (iCharIndex < 0)
					break;

				long start = SendDlgItemMessage(m_hwnd, IDC_LOG, EM_FINDWORDBREAK, WB_LEFT, iCharIndex);//-iChars;
				long end = SendDlgItemMessage(m_hwnd, IDC_LOG, EM_FINDWORDBREAK, WB_RIGHT, iCharIndex);//-iChars;

				wchar_t pszWord[4096]; pszWord[0] = '\0';
				if (end - start > 0) {
					TEXTRANGE tr;
					tr.lpstrText = pszWord;
					tr.chrg.cpMin = start;
					tr.chrg.cpMax = end;
					long iRes = SendDlgItemMessage(m_hwnd, IDC_LOG, EM_GETTEXTRANGE, 0, (LPARAM)&tr);
					if (iRes > 0)
						for (size_t iLen = mir_wstrlen(pszWord) - 1; wcschr(szTrimString, pszWord[iLen]); iLen--)
							pszWord[iLen] = 0;
				}

				CHARRANGE all = { 0, -1 };
				HMENU hMenu = 0;
				UINT uID = CreateGCMenu(m_hwnd, &hMenu, 1, pt, m_si, NULL, pszWord);
				switch (uID) {
				case 0:
					PostMessage(m_hwnd, WM_MOUSEACTIVATE, 0, 0);
					break;

				case ID_COPYALL:
					SendMessage(((LPNMHDR)lParam)->hwndFrom, EM_EXGETSEL, 0, (LPARAM)&sel);
					SendMessage(((LPNMHDR)lParam)->hwndFrom, EM_EXSETSEL, 0, (LPARAM)&all);
					SendMessage(((LPNMHDR)lParam)->hwndFrom, WM_COPY, 0, 0);
					SendMessage(((LPNMHDR)lParam)->hwndFrom, EM_EXSETSEL, 0, (LPARAM)&sel);
					PostMessage(m_hwnd, WM_MOUSEACTIVATE, 0, 0);
					break;

				case ID_CLEARLOG:
					s = pci->SM_FindSession(m_si->ptszID, m_si->pszModule);
					if (s) {
						SetDlgItemText(m_hwnd, IDC_LOG, L"");
						pci->LM_RemoveAll(&s->pLog, &s->pLogEnd);
						s->iEventCount = 0;
						s->LastTime = 0;
						m_si->iEventCount = 0;
						m_si->LastTime = 0;
						m_si->pLog = s->pLog;
						m_si->pLogEnd = s->pLogEnd;
						PostMessage(m_hwnd, WM_MOUSEACTIVATE, 0, 0);
					}
					break;

				case ID_SEARCH_GOOGLE:
					if (pszWord[0])
						Utils_OpenUrlW(CMStringW(FORMAT, L"http://www.google.com/search?q=%s", pszWord));

					PostMessage(m_hwnd, WM_MOUSEACTIVATE, 0, 0);
					break;

				case ID_SEARCH_WIKIPEDIA:
					if (pszWord[0])
						Utils_OpenUrlW(CMStringW(FORMAT, L"http://en.wikipedia.org/wiki/%s", pszWord));

					PostMessage(m_hwnd, WM_MOUSEACTIVATE, 0, 0);
					break;

				default:
					PostMessage(m_hwnd, WM_MOUSEACTIVATE, 0, 0);
					pci->DoEventHookAsync(m_hwnd, m_si->ptszID, m_si->pszModule, GC_USER_LOGMENU, NULL, NULL, uID);
					break;
				}
				DestroyGCMenu(&hMenu, 5);
			}
			break;

		case EN_LINK:
			if (((LPNMHDR)lParam)->idFrom == IDC_LOG) {
				switch (((ENLINK*)lParam)->msg) {
				case WM_RBUTTONDOWN:
				case WM_LBUTTONUP:
				case WM_LBUTTONDBLCLK:
					{
						SendMessage(((LPNMHDR)lParam)->hwndFrom, EM_EXGETSEL, 0, (LPARAM)&sel);
						if (sel.cpMin != sel.cpMax)
							break;

						TEXTRANGE tr;
						tr.chrg = ((ENLINK *)lParam)->chrg;
						tr.lpstrText = (LPTSTR)mir_alloc(sizeof(wchar_t)*(tr.chrg.cpMax - tr.chrg.cpMin + 1));
						SendMessage(((LPNMHDR)lParam)->hwndFrom, EM_GETTEXTRANGE, 0, (LPARAM)&tr);

						if (((ENLINK *)lParam)->msg == WM_RBUTTONDOWN) {
							HMENU hSubMenu;
							POINT pt;

							hSubMenu = GetSubMenu(g_hMenu, 2);
							TranslateMenu(hSubMenu);
							pt.x = (short)LOWORD(((ENLINK *)lParam)->lParam);
							pt.y = (short)HIWORD(((ENLINK *)lParam)->lParam);
							ClientToScreen(((NMHDR *)lParam)->hwndFrom, &pt);
							switch (TrackPopupMenu(hSubMenu, TPM_RETURNCMD, pt.x, pt.y, 0, m_hwnd, NULL)) {
							case ID_NEW:
								Utils_OpenUrlW(tr.lpstrText);
								break;

							case ID_CURR:
								Utils_OpenUrlW(tr.lpstrText, false);
								break;

							case ID_COPY:
								{
									HGLOBAL hData;
									if (!OpenClipboard(m_hwnd))
										break;
									EmptyClipboard();
									hData = GlobalAlloc(GMEM_MOVEABLE, sizeof(wchar_t)*(mir_wstrlen(tr.lpstrText) + 1));
									mir_wstrcpy((wchar_t*)GlobalLock(hData), tr.lpstrText);
									GlobalUnlock(hData);
									SetClipboardData(CF_UNICODETEXT, hData);
									CloseClipboard();
									SetFocus(m_message.GetHwnd());
									break;
								}
							}
							mir_free(tr.lpstrText);
							return TRUE;
						}

						Utils_OpenUrlW(tr.lpstrText);
						SetFocus(m_message.GetHwnd());
						mir_free(tr.lpstrText);
						break;
					}
				}
			}
			break;

		case TTN_NEEDTEXT:
			if (((LPNMHDR)lParam)->idFrom == (UINT_PTR)m_nickList.GetHwnd()) {
				LPNMTTDISPINFO lpttd = (LPNMTTDISPINFO)lParam;
				SESSION_INFO* parentdat = (SESSION_INFO*)GetWindowLongPtr(m_hwnd, GWLP_USERDATA);
				POINT p;
				GetCursorPos(&p);
				ScreenToClient(m_nickList.GetHwnd(), &p);
				int item = LOWORD(SendDlgItemMessage(m_hwnd, IDC_LIST, LB_ITEMFROMPOINT, 0, MAKELPARAM(p.x, p.y)));
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
		}
		break;

	case WM_COMMAND:
		if (HIWORD(wParam) == BN_CLICKED)
			if (LOWORD(wParam) >= MIN_CBUTTONID && LOWORD(wParam) <= MAX_CBUTTONID) {
				Srmm_ClickToolbarIcon(m_si->hContact, LOWORD(wParam), GetDlgItem(m_hwnd, LOWORD(wParam)), 0);
				break;
			}

		switch (LOWORD(wParam)) {
		case IDC_MESSAGE:
			EnableWindow(m_btnOk.GetHwnd(), GetRichTextLength(m_message.GetHwnd()) != 0);
			break;
		}
		break;

	case WM_KEYDOWN:
		SetFocus(m_message.GetHwnd());
		break;

	case WM_MOVE:
		SaveWindowPosition(false);
		break;

	case WM_GETMINMAXINFO:
		{
			MINMAXINFO *mmi = (MINMAXINFO*)lParam;
			mmi->ptMinTrackSize.x = m_si->iSplitterX + 43;
			if (mmi->ptMinTrackSize.x < 350)
				mmi->ptMinTrackSize.x = 350;

			mmi->ptMinTrackSize.y = m_si->iSplitterY + 80;
		}
		break;

	case WM_LBUTTONDBLCLK:
		if (LOWORD(lParam) < 30)
			PostMessage(m_hwnd, GC_SCROLLTOBOTTOM, 0, 0);
		break;
	}
	
	return CDlgBase::DlgProc(uMsg, wParam, lParam);
}
