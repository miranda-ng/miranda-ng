/*
Scriver

Copyright (c) 2000-12 Miranda ICQ/IM project,

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

#include "commonheaders.h"

extern HANDLE hHookWinPopup;

enum KB_ACTIONS {KB_PREV_TAB = 1, KB_NEXT_TAB, KB_SWITCHTOOLBAR,
				 KB_SWITCHSTATUSBAR, KB_SWITCHTITLEBAR, KB_SWITCHINFOBAR, KB_MINIMIZE, KB_CLOSE, KB_CLEAR_LOG,
				 KB_TAB1, KB_TAB2, KB_TAB3, KB_TAB4, KB_TAB5, KB_TAB6, KB_TAB7, KB_TAB8, KB_TAB9, KB_SEND_ALL, KB_PASTESEND, KB_QUOTE};

void InputAreaContextMenu(HWND hwnd, WPARAM wParam, LPARAM lParam, MCONTACT hContact)
{
	POINT pt;
	CHARRANGE sel, all = { 0, -1 };

	HMENU hMenu = LoadMenu(g_hInst, MAKEINTRESOURCE(IDR_CONTEXT));
	HMENU hSubMenu = GetSubMenu(hMenu, 2);
	TranslateMenu(hSubMenu);
	SendMessage(hwnd, EM_EXGETSEL, 0, (LPARAM)&sel);
	if (sel.cpMin == sel.cpMax) {
		EnableMenuItem(hSubMenu, IDM_CUT, MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(hSubMenu, IDM_COPY, MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(hSubMenu, IDM_DELETE, MF_BYCOMMAND | MF_GRAYED);
	}
	if (!SendMessage(hwnd, EM_CANUNDO, 0, 0))
		EnableMenuItem(hSubMenu, IDM_UNDO, MF_BYCOMMAND | MF_GRAYED);

	if (!SendMessage(hwnd, EM_CANREDO, 0, 0))
		EnableMenuItem(hSubMenu, IDM_REDO, MF_BYCOMMAND | MF_GRAYED);

	if (!SendMessage(hwnd, EM_CANPASTE, 0, 0)) {
		EnableMenuItem(hSubMenu, IDM_PASTESEND, MF_BYCOMMAND | MF_GRAYED);
		if (!IsClipboardFormatAvailable(CF_HDROP))
			EnableMenuItem(hSubMenu, IDM_PASTE, MF_BYCOMMAND | MF_GRAYED);
	}
	if (lParam == 0xFFFFFFFF) {
		SendMessage(hwnd, EM_POSFROMCHAR, (WPARAM)& pt, (LPARAM)sel.cpMax);
		ClientToScreen(hwnd, &pt);
	}
	else {
		pt.x = (short)LOWORD(lParam);
		pt.y = (short)HIWORD(lParam);
	}

	// First notification
	MessageWindowPopupData mwpd;
	mwpd.cbSize = sizeof(mwpd);
	mwpd.uType = MSG_WINDOWPOPUP_SHOWING;
	mwpd.uFlags = MSG_WINDOWPOPUP_INPUT;
	mwpd.hContact = hContact;
	mwpd.hwnd = hwnd;
	mwpd.hMenu = hSubMenu;
	mwpd.selection = 0;
	mwpd.pt = pt;
	NotifyEventHooks(hHookWinPopup, 0, (LPARAM)&mwpd);

	int selection = TrackPopupMenu(hSubMenu, TPM_RETURNCMD, pt.x, pt.y, 0, GetParent(hwnd), NULL);

	// Second notification
	mwpd.selection = selection;
	mwpd.uType = MSG_WINDOWPOPUP_SELECTED;
	NotifyEventHooks(hHookWinPopup, 0, (LPARAM)&mwpd);

	switch (selection) {
	case IDM_UNDO:
		SendMessage(hwnd, WM_UNDO, 0, 0);
		break;
	case IDM_REDO:
		SendMessage(hwnd, EM_REDO, 0, 0);
		break;
	case IDM_CUT:
		SendMessage(hwnd, WM_CUT, 0, 0);
		break;
	case IDM_COPY:
		SendMessage(hwnd, WM_COPY, 0, 0);
		break;
	case IDM_PASTE:
		SendMessage(hwnd, EM_PASTESPECIAL, CF_TEXT, 0);
		break;
	case IDM_PASTESEND:
		SendMessage(hwnd, EM_PASTESPECIAL, CF_TEXT, 0);
		PostMessage(GetParent(hwnd), WM_COMMAND, IDOK, 0);
		break;
	case IDM_DELETE:
		SendMessage(hwnd, EM_REPLACESEL, TRUE, 0);
		break;
	case IDM_SELECTALL:
		SendMessage(hwnd, EM_EXSETSEL, 0, (LPARAM)&all);
		break;
	case IDM_CLEAR:
		SetWindowText(hwnd, _T(""));
		break;
	}
	DestroyMenu(hMenu);
}

int InputAreaShortcuts(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, CommonWindowData *windowData)
{
	BOOL isShift = GetKeyState(VK_SHIFT) & 0x8000;
	BOOL isAlt = GetKeyState(VK_MENU) & 0x8000;
	BOOL isCtrl = (GetKeyState(VK_CONTROL) & 0x8000) && !isAlt;

	MSG amsg;
	amsg.hwnd = hwnd;
	amsg.message = msg;
	amsg.wParam = wParam;
	amsg.lParam = lParam;
	int action = CallService(MS_HOTKEY_CHECK, (WPARAM)&amsg, (LPARAM)"Messaging");

	switch (action) {
	case KB_PREV_TAB:
		SendMessage(GetParent(GetParent(hwnd)), CM_ACTIVATEPREV, 0, (LPARAM)GetParent(hwnd));
		return FALSE;
	case KB_NEXT_TAB:
		SendMessage(GetParent(GetParent(hwnd)), CM_ACTIVATENEXT, 0, (LPARAM)GetParent(hwnd));
		return FALSE;
	case KB_SWITCHSTATUSBAR:
		SendMessage(GetParent(GetParent(hwnd)), DM_SWITCHSTATUSBAR, 0, 0);
		return FALSE;
	case KB_SWITCHTITLEBAR:
		SendMessage(GetParent(GetParent(hwnd)), DM_SWITCHTITLEBAR, 0, 0);
		return FALSE;
	case KB_SWITCHINFOBAR:
		SendMessage(GetParent(GetParent(hwnd)), DM_SWITCHINFOBAR, 0, 0);
		return FALSE;
	case KB_SWITCHTOOLBAR:
		SendMessage(GetParent(GetParent(hwnd)), DM_SWITCHTOOLBAR, 0, 0);
		return FALSE;
	case KB_MINIMIZE:
		ShowWindow(GetParent(GetParent(hwnd)), SW_MINIMIZE);
		return FALSE;
	case KB_CLOSE:
		SendMessage(GetParent(hwnd), WM_CLOSE, 0, 0);
		return FALSE;
	case KB_CLEAR_LOG:
		SendMessage(GetParent(hwnd), DM_CLEARLOG, 0, 0);
		return FALSE;
	case KB_TAB1:
	case KB_TAB2:
	case KB_TAB3:
	case KB_TAB4:
	case KB_TAB5:
	case KB_TAB6:
	case KB_TAB7:
	case KB_TAB8:
	case KB_TAB9:
		SendMessage(GetParent(GetParent(hwnd)), CM_ACTIVATEBYINDEX, 0, action - KB_TAB1);
		return FALSE;
	case KB_SEND_ALL:
		PostMessage(GetParent(hwnd), WM_COMMAND, IDC_SENDALL, 0);
		return FALSE;
	case KB_QUOTE:
		PostMessage(GetParent(hwnd), WM_COMMAND, IDC_QUOTE, 0);
		return FALSE;
	case KB_PASTESEND:
		if (SendMessage(hwnd, EM_CANPASTE, 0, 0)) {
			SendMessage(hwnd, EM_PASTESPECIAL, CF_TEXT, 0);
			PostMessage(GetParent(hwnd), WM_COMMAND, IDOK, 0);
		}
		return FALSE;
	}

	switch (msg) {
	case WM_KEYDOWN:
		if (wParam >= '1' && wParam <= '9' && isCtrl) {
			SendMessage(GetParent(GetParent(hwnd)), CM_ACTIVATEBYINDEX, 0, wParam - '1');
			return 0;
		}

		if (wParam == 'I' && isCtrl) // ctrl-i (italics)
			return FALSE;

		if (wParam == VK_SPACE && isCtrl) // ctrl-space (paste clean text)
			return FALSE;

		if (wParam == 'R' && isCtrl && isShift) {     // ctrl-shift-r
			SendMessage(GetParent(hwnd), DM_SWITCHRTL, 0, 0);
			return FALSE;
		}

		if ((wParam == VK_UP || wParam == VK_DOWN) && isCtrl && !db_get_b(NULL, SRMMMOD, SRMSGSET_AUTOCLOSE, SRMSGDEFSET_AUTOCLOSE)) {
			if (windowData->cmdList) {
				TCmdList *cmdListNew = NULL;
				if (wParam == VK_UP) {
					if (windowData->cmdListCurrent == NULL) {
						cmdListNew = tcmdlist_last(windowData->cmdList);
						while (cmdListNew != NULL && cmdListNew->temporary) {
							windowData->cmdList = tcmdlist_remove(windowData->cmdList, cmdListNew);
							cmdListNew = tcmdlist_last(windowData->cmdList);
						}
						if (cmdListNew != NULL) {
							char *textBuffer;
							if (windowData->flags & CWDF_RTF_INPUT)
								textBuffer = GetRichTextRTF(hwnd);
							else
								textBuffer = GetRichTextEncoded(hwnd, windowData->codePage);

							if (textBuffer != NULL) {
								windowData->cmdList = tcmdlist_append(windowData->cmdList, textBuffer, 20, TRUE);
								mir_free(textBuffer);
							}
						}
					}
					else if (windowData->cmdListCurrent->prev != NULL)
						cmdListNew = windowData->cmdListCurrent->prev;
				}
				else {
					if (windowData->cmdListCurrent != NULL) {
						if (windowData->cmdListCurrent->next != NULL)
							cmdListNew = windowData->cmdListCurrent->next;
						else if (!windowData->cmdListCurrent->temporary)
							SetWindowText(hwnd, _T(""));
					}
				}
				if (cmdListNew != NULL) {
					int iLen;
					SendMessage(hwnd, WM_SETREDRAW, FALSE, 0);
					if (windowData->flags & CWDF_RTF_INPUT)
						iLen = SetRichTextRTF(hwnd, cmdListNew->szCmd);
					else
						iLen = SetRichTextEncoded(hwnd, cmdListNew->szCmd, windowData->codePage);

					SendMessage(hwnd, EM_SCROLLCARET, 0, 0);
					SendMessage(hwnd, WM_SETREDRAW, TRUE, 0);
					RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE);
					SendMessage(hwnd, EM_SETSEL, iLen, iLen);
					windowData->cmdListCurrent = cmdListNew;
				}
			}
			return FALSE;
		}
		break;

	case WM_SYSKEYDOWN:
		if ((wParam == VK_LEFT) && isAlt) {
			SendMessage(GetParent(GetParent(hwnd)), CM_ACTIVATEPREV, 0, (LPARAM)GetParent(hwnd));
			return 0;
		}
		if ((wParam == VK_RIGHT) && isAlt) {
			SendMessage(GetParent(GetParent(hwnd)), CM_ACTIVATENEXT, 0, (LPARAM)GetParent(hwnd));
			return 0;
		}
		break;
	case WM_SYSKEYUP:
		if ((wParam == VK_LEFT) && isAlt)
			return 0;

		if ((wParam == VK_RIGHT) && isAlt)
			return 0;
		break;
	}

	return -1;
}

void RegisterKeyBindings()
{
	char strDesc[64], strName[64];
	HOTKEYDESC desc = { sizeof(desc) };
	desc.pszSection = LPGEN("Messaging");
	desc.pszName = "Scriver/Nav/Previous Tab";
	desc.pszDescription = LPGEN("Navigate: Previous Tab");
	desc.lParam = KB_PREV_TAB;
	desc.DefHotKey = HOTKEYCODE(HOTKEYF_CONTROL | HOTKEYF_SHIFT, VK_TAB);
	Hotkey_Register(&desc);
	desc.DefHotKey = HOTKEYCODE(HOTKEYF_CONTROL, VK_PRIOR);
	Hotkey_Register(&desc);
	desc.DefHotKey = HOTKEYCODE(HOTKEYF_ALT, VK_LEFT);
	Hotkey_Register(&desc);

	desc.pszName = "Scriver/Nav/Next Tab";
	desc.pszDescription = LPGEN("Navigate: Next Tab");
	desc.lParam = KB_NEXT_TAB;
	desc.DefHotKey = HOTKEYCODE(HOTKEYF_CONTROL, VK_TAB);
	Hotkey_Register(&desc);
	desc.DefHotKey = HOTKEYCODE(HOTKEYF_CONTROL, VK_NEXT);
	Hotkey_Register(&desc);
	desc.DefHotKey = HOTKEYCODE(HOTKEYF_ALT, VK_RIGHT);
	Hotkey_Register(&desc);

	desc.pszName = strName;
	desc.pszDescription = strDesc;
	for (int i = 0; i < 9; i++) {
		mir_snprintf(strName, SIZEOF(strName), "Scriver/Nav/Tab %d", i + 1);
		mir_snprintf(strDesc, SIZEOF(strDesc), Translate("Navigate: Tab %d"), i + 1);
		desc.lParam = KB_TAB1 + i;
		desc.DefHotKey = HOTKEYCODE(HOTKEYF_ALT, '1' + i);
		Hotkey_Register(&desc);
	}

	desc.pszName = "Scriver/Wnd/Toggle Statusbar";
	desc.pszDescription = LPGEN("Window: Toggle status bar");
	desc.lParam = KB_SWITCHSTATUSBAR;
	desc.DefHotKey = HOTKEYCODE(HOTKEYF_CONTROL | HOTKEYF_SHIFT, 'S');
	Hotkey_Register(&desc);

	desc.pszName = "Scriver/Wnd/Toggle Titlebar";
	desc.pszDescription = LPGEN("Window: Toggle title bar");
	desc.lParam = KB_SWITCHTITLEBAR;
	desc.DefHotKey = HOTKEYCODE(HOTKEYF_CONTROL | HOTKEYF_SHIFT, 'M');
	Hotkey_Register(&desc);

	desc.pszName = "Scriver/Wnd/Toggle Toolbar";
	desc.pszDescription = LPGEN("Window: Toggle toolbar");
	desc.lParam = KB_SWITCHTOOLBAR;
	desc.DefHotKey = HOTKEYCODE(HOTKEYF_CONTROL | HOTKEYF_SHIFT, 'T');
	Hotkey_Register(&desc);

	desc.pszName = "Scriver/Wnd/Toggle Infobar";
	desc.pszDescription = LPGEN("Window: Toggle info bar");
	desc.lParam = KB_SWITCHINFOBAR;
	desc.DefHotKey = HOTKEYCODE(HOTKEYF_CONTROL | HOTKEYF_SHIFT, 'N');
	Hotkey_Register(&desc);

	desc.pszName = "Scriver/Wnd/Clear Log";
	desc.pszDescription = LPGEN("Window: Clear log");
	desc.lParam = KB_CLEAR_LOG;
	desc.DefHotKey = HOTKEYCODE(HOTKEYF_CONTROL, 'L');
	Hotkey_Register(&desc);

	desc.pszName = "Scriver/Wnd/Minimize";
	desc.pszDescription = LPGEN("Window: Minimize");
	desc.lParam = KB_MINIMIZE;
	desc.DefHotKey = HOTKEYCODE(HOTKEYF_SHIFT, VK_ESCAPE);
	Hotkey_Register(&desc);

	desc.pszName = "Scriver/Wnd/Close Tab";
	desc.pszDescription = LPGEN("Window: Close tab");
	desc.lParam = KB_CLOSE;
	desc.DefHotKey = HOTKEYCODE(HOTKEYF_CONTROL, VK_F4);
	Hotkey_Register(&desc);
	desc.DefHotKey = HOTKEYCODE(HOTKEYF_CONTROL, 'W');
	Hotkey_Register(&desc);

	desc.pszName = "Scriver/Action/Quote";
	desc.pszDescription = LPGEN("Action: Quote");
	desc.lParam = KB_QUOTE;
	desc.DefHotKey = HOTKEYCODE(HOTKEYF_CONTROL, 'Q');
	Hotkey_Register(&desc);

	desc.pszName = "Scriver/Action/Send All";
	desc.pszDescription = LPGEN("Action: Send to all");
	desc.lParam = KB_SEND_ALL;
	desc.DefHotKey = HOTKEYCODE(HOTKEYF_CONTROL | HOTKEYF_SHIFT, VK_RETURN);
	Hotkey_Register(&desc);

	desc.pszName = "Scriver/Action/PasteSend";
	desc.pszDescription = LPGEN("Action: Paste and send");
	desc.lParam = KB_PASTESEND;
	desc.DefHotKey = HOTKEYCODE(HOTKEYF_CONTROL | HOTKEYF_SHIFT, VK_INSERT);
	Hotkey_Register(&desc);
}

BOOL HandleLinkClick(HINSTANCE hInstance, HWND hwndDlg, HWND hwndFocus, ENLINK *lParam)
{
	CHARRANGE sel;
	SendMessage(lParam->nmhdr.hwndFrom, EM_EXGETSEL, 0, (LPARAM)&sel);
	if (sel.cpMin != sel.cpMax)
		return FALSE;

	TEXTRANGE tr;
	tr.chrg = lParam->chrg;
	tr.lpstrText = (LPWSTR)mir_alloc(sizeof(TCHAR)*(tr.chrg.cpMax - tr.chrg.cpMin + 8));
	SendMessage(lParam->nmhdr.hwndFrom, EM_GETTEXTRANGE, 0, (LPARAM)&tr);
	if (_tcschr(tr.lpstrText, _T('@')) != NULL && _tcschr(tr.lpstrText, _T(':')) == NULL && _tcschr(tr.lpstrText, _T('/')) == NULL) {
		MoveMemory(tr.lpstrText + 7, tr.lpstrText, sizeof(TCHAR)*(tr.chrg.cpMax - tr.chrg.cpMin + 1));
		memcpy(tr.lpstrText, _T("mailto:"), sizeof(TCHAR) * 7);
	}

	BOOL bOpenLink = TRUE;

	if (((ENLINK*)lParam)->msg == WM_RBUTTONDOWN) {
		bOpenLink = FALSE;
		HMENU hMenu = LoadMenu(hInstance, MAKEINTRESOURCE(IDR_CONTEXT));
		HMENU hSubMenu = GetSubMenu(hMenu, 1);
		TranslateMenu(hSubMenu);
		POINT pt;
		pt.x = (short)LOWORD(((ENLINK*)lParam)->lParam);
		pt.y = (short)HIWORD(((ENLINK*)lParam)->lParam);
		ClientToScreen(((NMHDR*)lParam)->hwndFrom, &pt);
		switch (TrackPopupMenu(hSubMenu, TPM_RETURNCMD, pt.x, pt.y, 0, hwndDlg, NULL)) {
		case IDM_OPENLINK:
			bOpenLink = TRUE;
			break;

		case IDM_COPYLINK:
			if (!OpenClipboard(hwndDlg))
				break;
			EmptyClipboard();
			HGLOBAL hData = GlobalAlloc(GMEM_MOVEABLE, sizeof(TCHAR)*(mir_tstrlen(tr.lpstrText) + 1));
			mir_tstrcpy((LPWSTR)GlobalLock(hData), tr.lpstrText);
			GlobalUnlock(hData);
			SetClipboardData(CF_UNICODETEXT, hData);
			CloseClipboard();
			break;
		}
		DestroyMenu(hMenu);
	}
	if (bOpenLink)
		CallService(MS_UTILS_OPENURL, OUF_TCHAR | OUF_NEWWINDOW, (LPARAM)tr.lpstrText);

	SetFocus(hwndFocus);
	mir_free(tr.lpstrText);
	return TRUE;
}
