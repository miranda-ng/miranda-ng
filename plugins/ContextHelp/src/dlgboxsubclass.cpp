/*
Miranda IM Help Plugin
Copyright (C) 2002 Richard Hughes, 2005-2007 H. Herkenrath

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program (Help-License.txt); if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
#include "stdafx.h"


#define SC_CONTEXTHELP_SEPARATOR  SC_SEPARATOR+1
#define SC_CONTEXTHELP_DIALOG     SC_CONTEXTHELP+1

#define PROP_CONTEXTSTATE         L"HelpPlugin_ContextState"
#define PROPF_MENUFORCED          0x01  // always show help context menu for ctl (override default)
#define PROPF_MENUDISABLED        0x02  // never show help context menu for ctl
#define PROPF_AUTOTIPFORCED       0x04  // always show autotip for ctl (override default)
#define PROPF_AUTOTIPDISABLED     0x08  // never show autotip for ctl

extern HINSTANCE hInst;
extern HWND hwndHelpDlg;
extern WORD settingAutoTipDelay;
static HHOOK hMessageHook, hKeyboardHook, hEatNextMouseHook = NULL;
static HANDLE hServiceShowHelp, hServiceSetContext;

struct DlgBoxSubclassData {
	HWND hwndDlg;
	WNDPROC pfnOldWndProc;
	DWORD flags;
} static *dlgBoxSubclass = NULL;
static int dlgBoxSubclassCount = 0;
static CRITICAL_SECTION csDlgBoxSubclass;

#define DBSDF_MINIMIZABLE  0x01  // WS_MINIMIZEBOX style was set on hooked window
#define DBSDF_MAXIMIZABLE  0x02  // WS_MAXIMIZEBOX style was set on hooked window

struct FindChildAtPointData {
	HWND hwnd;
	POINT pt;
	int bestArea;
};

// ChildWindowFromPoint() messes up with group boxes
static BOOL CALLBACK FindChildAtPointEnumProc(HWND hwnd, LPARAM lParam)
{
	if (IsWindowVisible(hwnd)) {
		struct FindChildAtPointData *fcap = (struct FindChildAtPointData*)lParam;
		RECT rcVisible, rc, rcParent;
		GetWindowRect(hwnd, &rc);
		GetWindowRect(GetParent(hwnd), &rcParent);
		IntersectRect(&rcVisible, &rcParent, &rc);
		if (PtInRect(&rcVisible, fcap->pt)) {
			int thisArea = (rc.bottom - rc.top)*(rc.right - rc.left);
			if (thisArea && (thisArea<fcap->bestArea || fcap->bestArea == 0)) {
				fcap->bestArea = thisArea;
				fcap->hwnd = hwnd;
			}
		}
	}
	return TRUE;
}

// IsChild() messes up with owned windows
int IsRealChild(HWND hwndParent, HWND hwnd)
{
	while (hwnd != NULL) {
		if (hwnd == hwndParent)
			return 1;
		if (hwndParent == GetWindow(hwnd, GW_OWNER))
			return 0;
		hwnd = GetParent(hwnd);
	}

	return 0;
}

static BOOL CALLBACK RemovePropForAllChildsEnumProc(HWND hwnd, LPARAM lParam)
{
	RemoveProp(hwnd, (TCHAR*)lParam);

	return TRUE;
}

static HWND hwndMouseMoveDlg = NULL;
static UINT idMouseMoveTimer = 0;
static LONG cursorPos = MAKELONG(-1, -1);
static int openedAutoTip = 0;

static void CALLBACK NoMouseMoveForDelayTimerProc(HWND hwnd, UINT, UINT_PTR idTimer, DWORD)
{
	POINT pt;
	HWND hwndCtl;
	struct FindChildAtPointData fcap;

	KillTimer(hwnd, idTimer);
	if (idMouseMoveTimer != idTimer)
		return;
	idMouseMoveTimer = 0;
	if (!settingAutoTipDelay || !IsWindow(hwndMouseMoveDlg))
		return;

	ZeroMemory(&fcap, sizeof(fcap));
	if (!GetCursorPos(&pt))
		return;
	// ChildWindowFromPoint() messes up with group boxes
	fcap.hwnd = NULL;
	fcap.pt = pt;
	EnumChildWindows(hwndMouseMoveDlg, FindChildAtPointEnumProc, (LPARAM)&fcap);
	hwndCtl = fcap.hwnd;
	if (hwndCtl == NULL) {
		ScreenToClient(hwndMouseMoveDlg, &pt);
		hwndCtl = ChildWindowFromPointEx(hwndMouseMoveDlg, pt, CWP_SKIPINVISIBLE);
		if (hwndCtl == NULL)
			return;
	}

	LONG_PTR flags = (LONG_PTR)GetProp(hwndCtl, PROP_CONTEXTSTATE);
	if (flags&PROPF_AUTOTIPDISABLED)
		return;
	flags = SendMessage(hwndCtl, WM_GETDLGCODE, (WPARAM)VK_RETURN, (LPARAM)NULL);
	if (flags&DLGC_HASSETSEL || flags&DLGC_WANTALLKEYS)
		return; // autotips on edits are annoying

	CURSORINFO ci;
	BOOL(WINAPI *pfnGetCursorInfo)(CURSORINFO*);
	ci.cbSize = sizeof(ci);
	*(FARPROC*)&pfnGetCursorInfo = GetProcAddress(GetModuleHandleA("USER32"), "GetCursorInfo");
	// if(pfnGetCursorInfo && IsWinVer2000Plus()) // call not safe for WinNT4
	if (pfnGetCursorInfo(&ci) && !(ci.flags&CURSOR_SHOWING))
		return;

	if (IsRealChild(hwndMouseMoveDlg, hwndCtl) && hwndHelpDlg == NULL) {
		hwndHelpDlg = CreateDialog(hInst, MAKEINTRESOURCE(IDD_HELP), NULL, HelpDlgProc);
		if (hwndHelpDlg == NULL)
			return;
		openedAutoTip = 1;
		PostMessage(hwndHelpDlg, M_CHANGEHELPCONTROL, 0, (LPARAM)hwndCtl);
	}
}

static LRESULT CALLBACK KeyboardInputHookProc(int code, WPARAM wParam, LPARAM lParam)
{
	if (code == HC_ACTION && idMouseMoveTimer != 0) {
		KillTimer(NULL, idMouseMoveTimer);
		idMouseMoveTimer = 0;
	}
	return CallNextHookEx(hKeyboardHook, code, wParam, lParam);
}

// workaround for WM_HELP (SC_CONTEXTHELP causes an additional WM_LBUTTONUP when selecting)
static LRESULT CALLBACK EatNextMouseButtonUpHookProc(int code, WPARAM wParam, LPARAM lParam)
{
	if (code >= 0 && wParam == WM_LBUTTONUP) {
		UnhookWindowsHookEx(hEatNextMouseHook); // unhook ourselves
		hEatNextMouseHook = NULL;
		return -1;
	}
	return CallNextHookEx(hEatNextMouseHook, code, wParam, lParam);
}

static LRESULT CALLBACK DialogBoxSubclassProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	int i;

	EnterCriticalSection(&csDlgBoxSubclass);
	for (i = 0; i<dlgBoxSubclassCount; i++)
		if (dlgBoxSubclass[i].hwndDlg == hwndDlg)
			break;
	if (i == dlgBoxSubclassCount) {
		LeaveCriticalSection(&csDlgBoxSubclass);
		return 0;
	}
	WNDPROC pfnWndProc = dlgBoxSubclass[i].pfnOldWndProc;
	DWORD flags = dlgBoxSubclass[i].flags;
	if (msg == WM_NCDESTROY) {
		struct DlgBoxSubclassData *buf;
		MoveMemory(dlgBoxSubclass + i, dlgBoxSubclass + i + 1, sizeof(struct DlgBoxSubclassData)*(dlgBoxSubclassCount - i - 1));
		dlgBoxSubclassCount--;
		buf = (struct DlgBoxSubclassData*)mir_realloc(dlgBoxSubclass, sizeof(struct DlgBoxSubclassData)*dlgBoxSubclassCount);
		if (buf != NULL)
			dlgBoxSubclass = buf;
		else if (!dlgBoxSubclassCount)
			dlgBoxSubclass = NULL;
	}
	LeaveCriticalSection(&csDlgBoxSubclass);

	switch (msg) {
	case WM_INITMENUPOPUP:
		if (flags&DBSDF_MINIMIZABLE || flags&DBSDF_MAXIMIZABLE) {
			HMENU hMenu = GetSystemMenu(hwndDlg, FALSE);
			if ((HMENU)wParam != hMenu)
				break;
			int isMin = IsIconic(hwndDlg);
			int isMax = IsZoomed(hwndDlg);
			EnableMenuItem(hMenu, SC_RESTORE, MF_BYCOMMAND | (isMin || isMax) ? MF_ENABLED : MF_GRAYED);
			EnableMenuItem(hMenu, SC_MINIMIZE, MF_BYCOMMAND | (flags&DBSDF_MINIMIZABLE && !isMin) ? MF_ENABLED : MF_GRAYED);
			EnableMenuItem(hMenu, SC_MAXIMIZE, MF_BYCOMMAND | (flags&DBSDF_MAXIMIZABLE && !isMax) ? MF_ENABLED : MF_GRAYED);
			EnableMenuItem(hMenu, SC_SIZE, MF_BYCOMMAND | (GetWindowLongPtr(hwndDlg, GWL_STYLE)&WS_THICKFRAME && !isMin && !isMax) ? MF_ENABLED : MF_GRAYED);
		}
		break;
	case WM_MOUSEMOVE: // TrackMouseEvent() would disturb too much
		if (!settingAutoTipDelay)
			break;
		if (cursorPos == lParam)
			break;
		cursorPos = lParam;
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_MOUSEWHEEL:
		if (!settingAutoTipDelay)
			break;
		if (msg != WM_MOUSEMOVE && !idMouseMoveTimer)
			break;
		if (openedAutoTip && IsWindow(hwndHelpDlg))
			DestroyWindow(hwndHelpDlg);
		openedAutoTip = 0;
		hwndMouseMoveDlg = hwndDlg;
		if (hwndHelpDlg == NULL)
			idMouseMoveTimer = SetTimer(NULL, idMouseMoveTimer, settingAutoTipDelay, NoMouseMoveForDelayTimerProc);
		break;
	case WM_CAPTURECHANGED:
		if ((HWND)lParam == hwndDlg)
			break;
	case WM_SHOWWINDOW:
	case WM_WINDOWPOSCHANGING:
	case WM_MOVING:
	case WM_SIZING:
	case WM_CANCELMODE:
	case WM_CHILDACTIVATE:
	case WM_MOUSEACTIVATE:
	case WM_ACTIVATEAPP:
	case WM_ACTIVATE:
		if (idMouseMoveTimer)
			KillTimer(NULL, idMouseMoveTimer);
		idMouseMoveTimer = 0;
		break;
	case WM_SYSCOMMAND:
		if ((UINT)wParam == SC_CONTEXTHELP_DIALOG) { // alt. "What's this dialog?"
			if (idMouseMoveTimer)
				KillTimer(NULL, idMouseMoveTimer);
			idMouseMoveTimer = 0;
			if (hwndHelpDlg == NULL) {
				hwndHelpDlg = CreateDialog(hInst, MAKEINTRESOURCE(IDD_HELP), NULL, HelpDlgProc);
				if (hwndHelpDlg == NULL)
					break;
			}
			SendMessage(hwndHelpDlg, M_CHANGEHELPCONTROL, 0, (LPARAM)hwndDlg);
			return 0;
		}
		break;
	case WM_CONTEXTMENU:
		{
			POINT pt;
			struct FindChildAtPointData fcap;

			// workaround for badly coded plugins that do display a context menu
			// and pass the message to DefWindowProc afterwards (doing a "break;").
			if (GetTickCount() - GetMessageTime()>10)
				return 0;

			if (idMouseMoveTimer)
				KillTimer(NULL, idMouseMoveTimer);
			idMouseMoveTimer = 0;

			ZeroMemory(&fcap, sizeof(fcap));
			POINTSTOPOINT(pt, MAKEPOINTS(lParam));
			// ChildWindowFromPoint() messes up with group boxes
			fcap.hwnd = NULL;
			fcap.pt = pt;
			EnumChildWindows(hwndDlg, FindChildAtPointEnumProc, (LPARAM)&fcap);
			HWND hwndCtl = fcap.hwnd;
			if (hwndCtl == NULL) {
				ScreenToClient(hwndDlg, &pt);
				hwndCtl = ChildWindowFromPointEx(hwndDlg, pt, CWP_SKIPINVISIBLE);
				if (hwndCtl == NULL)
					break;
				POINTSTOPOINT(pt, MAKEPOINTS(lParam));
			}
			{
				LONG_PTR flags = (LONG_PTR)GetProp(hwndCtl, PROP_CONTEXTSTATE);
				if (flags&PROPF_MENUDISABLED)
					break;
				else if (!(flags&PROPF_MENUFORCED)) {
					int type = GetControlType(hwndCtl);
					// showing a context menu on these looks silly (multi components)
					if (type == CTLTYPE_TOOLBAR || type == CTLTYPE_LISTVIEW || type == CTLTYPE_TREEVIEW || type == CTLTYPE_STATUSBAR || type == CTLTYPE_CLC)
						break;
				}
			}
			if (IsRealChild(hwndDlg, hwndCtl)) {
				HMENU hMenu = CreatePopupMenu();
				AppendMenu(hMenu, MF_STRING, SC_CONTEXTHELP, (hwndCtl == hwndDlg) ? TranslateT("&What's this dialog?") : TranslateT("&What's this?"));
				if (TrackPopupMenuEx(hMenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_HORPOSANIMATION | TPM_VERPOSANIMATION | TPM_RIGHTBUTTON | TPM_RETURNCMD | TPM_NONOTIFY, pt.x, pt.y, hwndDlg, NULL)) {
					if (hwndHelpDlg == NULL) {
						hwndHelpDlg = CreateDialog(hInst, MAKEINTRESOURCE(IDD_HELP), NULL, HelpDlgProc);
						if (hwndHelpDlg == NULL) {
							DestroyMenu(hMenu);
							break;
						}
					}
					SendMessage(hwndHelpDlg, M_CHANGEHELPCONTROL, 0, (LPARAM)hwndCtl);
				}
				DestroyMenu(hMenu);
			}
			return 0;
		}
	case WM_HELP:
		{
			HELPINFO *hi = (HELPINFO*)lParam;
			if (hi->iContextType != HELPINFO_WINDOW) break;
			// fix for SHBrowseForFolder() dialog, which sends unhandled help to parent
			if (!IsRealChild(hwndDlg, (HWND)hi->hItemHandle))
				break;

			if (idMouseMoveTimer)
				KillTimer(NULL, idMouseMoveTimer);
			idMouseMoveTimer = 0;

			if (!IsWindow(hwndHelpDlg)) {
				hwndHelpDlg = CreateDialog(hInst, MAKEINTRESOURCE(IDD_HELP), NULL, HelpDlgProc);
				if (hwndHelpDlg == NULL)
					break;
			}
			SendMessage(hwndHelpDlg, M_CHANGEHELPCONTROL, 0, (LPARAM)hi->hItemHandle);
			// we need to eat the next WM_LBUTTONDOWN (if invoked by mouse)
			if (GetKeyState(GetSystemMetrics(SM_SWAPBUTTON) ? VK_RBUTTON : VK_LBUTTON) & 0x8000 && hEatNextMouseHook == NULL)
				hEatNextMouseHook = SetWindowsHookEx(WH_MOUSE, EatNextMouseButtonUpHookProc, NULL, GetCurrentThreadId());
			return TRUE;
		}
	case WM_NCDESTROY:
		if (idMouseMoveTimer)
			KillTimer(NULL, idMouseMoveTimer);
		idMouseMoveTimer = 0;
		EnumChildWindows(hwndDlg, RemovePropForAllChildsEnumProc, (LPARAM)PROP_CONTEXTSTATE);
		{
			TCHAR text[64];
			mir_sntprintf(text, _countof(text), L"unhooked window 0x%X for context help\n", hwndDlg);
			OutputDebugString(text);
		}
		SetWindowLongPtr(hwndDlg, GWLP_WNDPROC, (LONG_PTR)pfnWndProc);
		break;
	}
	return CallWindowProc(pfnWndProc, hwndDlg, msg, wParam, lParam);
}

static LRESULT CALLBACK HelpSendMessageHookProc(int code, WPARAM wParam, LPARAM lParam)
{
	if (code >= 0) {
		CWPSTRUCT *msg = (CWPSTRUCT*)lParam;
		switch (msg->message) {
		case WM_INITDIALOG: // dialogs and message boxes
			if (GetClassLong(msg->hwnd, GCW_ATOM) != 32770) // class="#32770"
				break;
			if (msg->hwnd == hwndHelpDlg || (DLGPROC)GetWindowLongPtr(msg->hwnd, DWLP_DLGPROC) == HelpDlgProc)
				break;
#ifndef EDITOR
			if ((DLGPROC)GetWindowLongPtr(msg->hwnd, DWLP_DLGPROC) == ShadowDlgProc)
				break;
#endif
			{
				LONG_PTR style, exStyle;
				struct DlgBoxSubclassData *buf;

				exStyle = GetWindowLongPtr(msg->hwnd, GWL_EXSTYLE);
				if (exStyle&WS_EX_CONTEXTHELP)
					break;
				style = GetWindowLongPtr(msg->hwnd, GWL_STYLE);

				EnterCriticalSection(&csDlgBoxSubclass);
				buf = (struct DlgBoxSubclassData*)mir_realloc(dlgBoxSubclass, sizeof(struct DlgBoxSubclassData)*(dlgBoxSubclassCount + 1));
				if (buf == NULL) {
					LeaveCriticalSection(&csDlgBoxSubclass);
					break;
				}
				dlgBoxSubclass = buf;
				dlgBoxSubclass[dlgBoxSubclassCount].hwndDlg = msg->hwnd;
				dlgBoxSubclass[dlgBoxSubclassCount].pfnOldWndProc = (WNDPROC)SetWindowLongPtr(msg->hwnd, GWLP_WNDPROC, (LONG_PTR)DialogBoxSubclassProc);
				dlgBoxSubclass[dlgBoxSubclassCount].flags = 0;

				// WS_EX_CONTEXTHELP cannot be used in conjunction WS_MINIMIZEBOX or WS_MAXIMIZEBOX
				// solution: switch off WS_MINIMIZEBOX or WS_MAXIMIZEBOX when only one of them is present
				if (!(style & WS_MINIMIZEBOX) || !(style & WS_MAXIMIZEBOX)) {
					if (style & WS_MINIMIZEBOX)
						dlgBoxSubclass[dlgBoxSubclassCount].flags |= DBSDF_MINIMIZABLE;
					if (style & WS_MAXIMIZEBOX)
						dlgBoxSubclass[dlgBoxSubclassCount].flags |= DBSDF_MAXIMIZABLE;
					SetWindowLongPtr(msg->hwnd, GWL_STYLE, style&(~(WS_MINIMIZEBOX | WS_MAXIMIZEBOX)));
					SetWindowLongPtr(msg->hwnd, GWL_EXSTYLE, (LONG_PTR)exStyle | WS_EX_CONTEXTHELP);
				}
				dlgBoxSubclassCount++;
				LeaveCriticalSection(&csDlgBoxSubclass);
			}
				{
					HMENU hMenu = GetSystemMenu(msg->hwnd, FALSE);
					if (hMenu != NULL && AppendMenu(hMenu, MF_SEPARATOR, SC_CONTEXTHELP_SEPARATOR, NULL)) {
						AppendMenu(hMenu, MF_STRING, SC_CONTEXTHELP, TranslateT("&What's this?"));
						AppendMenu(hMenu, MF_STRING, SC_CONTEXTHELP_DIALOG, TranslateT("&What's this dialog?"));
					}
				}
				{
					TCHAR text[64];
					mir_sntprintf(text, _countof(text), L"hooked window 0x%X for context help\n", msg->hwnd);
					OutputDebugString(text);
				}
				break;
		}
	}
	return CallNextHookEx(hMessageHook, code, wParam, lParam);
}

static INT_PTR ServiceShowHelp(WPARAM wParam, LPARAM)
{
	if (!IsWindow((HWND)wParam))
		return 1;
	if (idMouseMoveTimer)
		KillTimer(NULL, idMouseMoveTimer);
	idMouseMoveTimer = 0;
	if (hwndHelpDlg == NULL) {
		hwndHelpDlg = CreateDialog(hInst, MAKEINTRESOURCE(IDD_HELP), NULL, HelpDlgProc);
		if (hwndHelpDlg == NULL)
			return 2;
	}
	PostMessage(hwndHelpDlg, M_CHANGEHELPCONTROL, 0, (LPARAM)wParam);

	return 0;
}

static INT_PTR ServiceSetContextState(WPARAM wParam, LPARAM lParam)
{
	int i;
	LONG flags;
	EnterCriticalSection(&csDlgBoxSubclass);
	for (i = 0; i < dlgBoxSubclassCount; i++)
		if (IsRealChild(dlgBoxSubclass[i].hwndDlg, (HWND)wParam))
			break;
	if (i == dlgBoxSubclassCount) {
		LeaveCriticalSection(&csDlgBoxSubclass);

		return 2;
	}
	LeaveCriticalSection(&csDlgBoxSubclass);
	flags = (lParam&HCSF_CONTEXTMENU) ? PROPF_MENUFORCED : PROPF_MENUDISABLED;
	flags |= (lParam&HCSF_AUTOTIP) ? PROPF_AUTOTIPFORCED : PROPF_AUTOTIPDISABLED;

	return !SetProp((HWND)wParam, PROP_CONTEXTSTATE, (HANDLE)flags);
}

int InstallDialogBoxHook(void)
{
	InitializeCriticalSection(&csDlgBoxSubclass);
	hServiceShowHelp = CreateServiceFunction(MS_HELP_SHOWHELP, ServiceShowHelp);
	hServiceSetContext = CreateServiceFunction(MS_HELP_SETCONTEXTSTATE, ServiceSetContextState);
	hMessageHook = SetWindowsHookEx(WH_CALLWNDPROC, HelpSendMessageHookProc, NULL, GetCurrentThreadId()); // main thread
	hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardInputHookProc, NULL, GetCurrentThreadId()); // main thread

	return hMessageHook == NULL;
}

int RemoveDialogBoxHook(void)
{
	DestroyServiceFunction(hServiceShowHelp); // does NULL check
	DestroyServiceFunction(hServiceSetContext); // does NULL check
	UnhookWindowsHookEx(hMessageHook);
	if (hKeyboardHook)
		UnhookWindowsHookEx(hKeyboardHook);
	if (hEatNextMouseHook)
		UnhookWindowsHookEx(hEatNextMouseHook);
	DeleteCriticalSection(&csDlgBoxSubclass);
	for (int i = 0; i<dlgBoxSubclassCount; i++)
		SetWindowLongPtr(dlgBoxSubclass[i].hwndDlg, GWLP_WNDPROC, (LONG_PTR)dlgBoxSubclass[i].pfnOldWndProc);
	mir_free(dlgBoxSubclass); // does NULL check

	return 0;
}
