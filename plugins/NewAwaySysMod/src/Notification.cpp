/*
	New Away System - plugin for Miranda IM
	Copyright (C) 2005-2007 Chervov Dmitry

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

#include "Common.h"
#include "Path.h"
#include "Properties.h"

void ShowMsg(TCHAR *FirstLine, TCHAR *SecondLine, bool IsErrorMsg, int Timeout)
{
	if (ServiceExists(MS_POPUP_ADDPOPUPT)) {
		POPUPDATAT ppd = { 0 };
		ppd.lchIcon = LoadIcon(NULL, IsErrorMsg ? IDI_EXCLAMATION : IDI_INFORMATION);
		mir_tstrncpy(ppd.lptzContactName, FirstLine, MAX_CONTACTNAME);
		mir_tstrncpy(ppd.lptzText, SecondLine, MAX_SECONDLINE);
		ppd.colorBack = IsErrorMsg ? 0x0202E3 : 0xE8F1FD;
		ppd.colorText = IsErrorMsg ? 0xE8F1FD : 0x000000;
		ppd.iSeconds = Timeout;
		PUAddPopupT(&ppd);
	}
	else MessageBox(NULL, SecondLine, FirstLine, MB_OK | (IsErrorMsg ? MB_ICONEXCLAMATION : MB_ICONINFORMATION));
}


static int CALLBACK MenuWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_MEASUREITEM:
		return CallService(MS_CLIST_MENUMEASUREITEM, wParam, lParam);

	case WM_DRAWITEM:
		return CallService(MS_CLIST_MENUDRAWITEM, wParam, lParam);
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}


static VOID CALLBACK ShowContactMenu(MCONTACT hContact)
{
	POINT pt;
	HWND hMenuWnd = CreateWindowEx(WS_EX_TOOLWINDOW, _T("static"), _T(MOD_NAME)_T("_MenuWindow"), 0, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, HWND_DESKTOP, NULL, g_hInstance, NULL);
	SetWindowLongPtr(hMenuWnd, GWLP_WNDPROC, (LONG_PTR)MenuWndProc);
	HMENU hMenu = (HMENU)CallService(MS_CLIST_MENUBUILDCONTACT, hContact, 0);
	GetCursorPos(&pt);
	SetForegroundWindow(hMenuWnd);
	CallService(MS_CLIST_MENUPROCESSCOMMAND, MAKEWPARAM(TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, pt.x, pt.y, 0, hMenuWnd, NULL), MPCF_CONTACTMENU), hContact);
	PostMessage(hMenuWnd, WM_NULL, 0, 0);
	DestroyMenu(hMenu);
	DestroyWindow(hMenuWnd);
}

void ShowLog(TCString &LogFilePath)
{
	int Result = (int)ShellExecute(NULL, _T("open"), LogFilePath, NULL, NULL, SW_SHOW);
	if (Result <= 32) {
		TCHAR szError[64];
		mir_sntprintf(szError, SIZEOF(szError), TranslateT("Error #%d"), Result);
		ShowMsg(szError, TranslateT("Can't open log file ") + LogFilePath, true);
	}
}
