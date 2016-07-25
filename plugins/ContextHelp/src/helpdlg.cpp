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


extern HINSTANCE hInst;
HWND hwndHelpDlg;

static int HelpDialogResize(HWND, LPARAM, UTILRESIZECONTROL *urc)
{
	switch (urc->wId) {
	case IDC_CTLTEXT:
#ifdef EDITOR
	case IDC_DLGID:
	case IDC_MODULE:
#endif
		return RD_ANCHORX_WIDTH | RD_ANCHORY_TOP;
	case IDC_TEXT:
		return RD_ANCHORX_WIDTH | RD_ANCHORY_HEIGHT;
	}
	return RD_ANCHORX_LEFT | RD_ANCHORY_TOP;
}

#ifndef EDITOR
INT_PTR CALLBACK ShadowDlgProc(HWND hwndDlg, UINT msg, WPARAM, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		{
			BOOL(WINAPI *pfnSetLayeredWindowAttributes)(HWND, COLORREF, BYTE, DWORD);
			*(FARPROC*)&pfnSetLayeredWindowAttributes = GetProcAddress(GetModuleHandleA("USER32"), "SetLayeredWindowAttributes");
			if (!pfnSetLayeredWindowAttributes) {
				*(HANDLE*)lParam = NULL; // hwndShadowDlg reset
				DestroyWindow(hwndDlg);
				return FALSE;
			}
			EnableWindow(hwndDlg, FALSE);
			SetWindowLongPtr(hwndDlg, GWL_EXSTYLE, GetWindowLongPtr(hwndDlg, GWL_EXSTYLE) | WS_EX_LAYERED);
			pfnSetLayeredWindowAttributes(hwndDlg, RGB(0, 0, 0), 96, LWA_ALPHA);
			return FALSE;
		}
	case WM_CTLCOLORDLG:
		return (INT_PTR)GetSysColorBrush(COLOR_WINDOWFRAME);
	}
	return FALSE;
}

// in client coordinates
int GetCharRangeRect(HWND hwndEdit, LONG *cpMin, LONG cpMax, RECT *rcRange)
{
	LONG cpLineBreak;

	LONG nLine, nLinePrev;
	if (*cpMin>cpMax)
		return 1;
	nLine = SendMessage(hwndEdit, EM_EXLINEFROMCHAR, 0, *cpMin);
	for (cpLineBreak = *cpMin + 1; cpLineBreak <= cpMax; cpLineBreak++) {
		nLinePrev = nLine;
		nLine = SendMessage(hwndEdit, EM_EXLINEFROMCHAR, 0, cpLineBreak);
		if (nLine != nLinePrev)
			break;
	}
	cpMax = cpLineBreak - 1;

	POINTL pt;
	if (SendMessage(hwndEdit, EM_SETTYPOGRAPHYOPTIONS, 0, 0)) { // test for richedit v3.0
		SendMessage(hwndEdit, EM_POSFROMCHAR, (WPARAM)&pt, *cpMin);
		rcRange->left = pt.x;
		rcRange->top = pt.y;
		SendMessage(hwndEdit, EM_POSFROMCHAR, (WPARAM)&pt, cpMax);
		rcRange->right = pt.x;
		rcRange->bottom = pt.y;
	}
	else {
		DWORD pos;
		pos = SendMessage(hwndEdit, EM_POSFROMCHAR, (WPARAM)*cpMin, 0);
		POINTSTOPOINT(pt, MAKEPOINTS(pos));
		rcRange->left = pt.x;
		rcRange->top = pt.y;
		pos = SendMessage(hwndEdit, EM_POSFROMCHAR, (WPARAM)cpMax, 0);
		POINTSTOPOINT(pt, MAKEPOINTS(pos));
		rcRange->right = pt.x;
		rcRange->bottom = pt.y;
	}

	FORMATRANGE fr;
	ZeroMemory(&fr, sizeof(fr));
	fr.chrg.cpMin = *cpMin;
	fr.chrg.cpMax = cpMax;
	fr.hdc = fr.hdcTarget = GetDC(hwndEdit);
	if (fr.hdc == NULL)
		return 1;
	SendMessage(hwndEdit, EM_FORMATRANGE, 0, (LPARAM)&fr);
	PostMessage(hwndEdit, EM_FORMATRANGE, 0, 0); // clear memory
	rcRange->bottom += MulDiv(fr.rc.bottom, GetDeviceCaps(fr.hdc, LOGPIXELSY), 1440); // twips to pixels
	ReleaseDC(hwndEdit, fr.hdc);

	*cpMin = cpLineBreak;
	return 0;
}

static HCURSOR hHandCursor;
static LRESULT CALLBACK HelpSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_KEYDOWN:
		if (GetKeyState(VK_CONTROL) & 0x8000 && wParam == 'C') {
			SendMessage(GetParent(hwnd), M_CLIPBOARDCOPY, 0, 0);
			return 0;
		}
		break;
	case WM_LBUTTONDBLCLK:
		DestroyWindow(GetParent(hwnd));
		return 0;
	case WM_CONTEXTMENU:
		return DefWindowProc(hwnd, msg, wParam, lParam); // redirect to parent
	case WM_SETCURSOR: // not available via EN_MSGFILTER
		if (GetDlgCtrlID((HWND)wParam) == IDC_TEXT) {
			POINT pt;
			DWORD pos;
			CHARRANGE rng;
			pos = GetMessagePos();
			POINTSTOPOINT(pt, MAKEPOINTS(pos));
			ScreenToClient((HWND)wParam, &pt);
			pos = SendMessage((HWND)wParam, EM_CHARFROMPOS, 0, (WPARAM)&pt);
			if (IsHyperlink(pos, &rng.cpMin, &rng.cpMax, NULL)) {
				RECT rc;
				while (!GetCharRangeRect((HWND)wParam, &rng.cpMin, rng.cpMax, &rc))
					if (PtInRect(&rc, pt)) {
						SetCursor(hHandCursor);
						return TRUE;
					}
			}
		}
		break;
	}
	return CallWindowProc((WNDPROC)GetWindowLongPtr(hwnd, GWLP_USERDATA), hwnd, msg, wParam, lParam);
}
#endif // !defined EDITOR

INT_PTR CALLBACK HelpDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HWND hwndCtl = (HWND)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	static LCID locale;
#ifndef EDITOR
	static HWND hwndShadowDlg;
	static HWND hwndToolTip;
#endif

	switch (msg) {
	case WM_INITDIALOG:
		hwndHelpDlg = hwndDlg;
#ifdef EDITOR
		TranslateDialogDefault(hwndDlg);
		SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_DLGID), GWL_STYLE, GetWindowLongPtr(GetDlgItem(hwndDlg, IDC_DLGID), GWL_STYLE) | SS_ENDELLIPSIS);
		SendDlgItemMessage(hwndDlg, IDC_TEXT, EM_SETEVENTMASK, 0, ENM_KEYEVENTS);
		{	RECT rcDlg, rcWork;
		if (SystemParametersInfo(SPI_GETWORKAREA, 0, &rcWork, FALSE) && GetWindowRect(hwndDlg, &rcDlg))
			SetWindowPos(hwndDlg, 0, rcDlg.left, rcWork.bottom - rcDlg.bottom + rcDlg.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
		}
#else
			{
				RECT rc, rcBuf;
				SendDlgItemMessage(hwndDlg, IDC_CTLTEXT, EM_GETRECT, 0, (LPARAM)&rcBuf);
				SendDlgItemMessage(hwndDlg, IDC_TEXT, EM_GETRECT, 0, (LPARAM)&rc);
				rc.left = rcBuf.left; // sync richedit offset with edit
				rc.right = rcBuf.right;
				SendDlgItemMessage(hwndDlg, IDC_TEXT, EM_SETRECTNP, 0, (LPARAM)&rc);
			}
			SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_CTLTEXT), GWLP_USERDATA, SetWindowLong(GetDlgItem(hwndDlg, IDC_CTLTEXT), GWLP_WNDPROC, (LONG_PTR)HelpSubclassProc));
			SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_CARETSUCKER), GWLP_USERDATA, SetWindowLong(GetDlgItem(hwndDlg, IDC_CARETSUCKER), GWLP_WNDPROC, (LONG_PTR)HelpSubclassProc));
			SendDlgItemMessage(hwndDlg, IDC_TEXT, EM_SETEVENTMASK, 0, ENM_KEYEVENTS | ENM_MOUSEEVENTS | ENM_REQUESTRESIZE);
			SendDlgItemMessage(hwndDlg, IDC_TEXT, EM_SETBKGNDCOLOR, 0, GetSysColor(COLOR_INFOBK));
			SendDlgItemMessage(hwndDlg, IDC_TEXT, EM_SETEDITSTYLE, SES_EXTENDBACKCOLOR, SES_EXTENDBACKCOLOR);
			SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_TEXT), GWLP_USERDATA, SetWindowLong(GetDlgItem(hwndDlg, IDC_TEXT), GWLP_WNDPROC, (LONG_PTR)HelpSubclassProc));
			hwndShadowDlg = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_SHADOW), hwndDlg, ShadowDlgProc, (LPARAM)&hwndShadowDlg);
			hwndToolTip = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL, WS_POPUP | TTS_ALWAYSTIP | TTS_NOPREFIX, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, hwndDlg, NULL, hInst, NULL);
			if (hwndToolTip != NULL) {
				SetWindowPos(hwndToolTip, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
				SendMessage(hwndToolTip, TTM_SETTIPBKCOLOR, GetSysColor(COLOR_WINDOW), 0); // yelleow on yellow looks silly
				SendMessage(hwndToolTip, TTM_SETDELAYTIME, TTDT_AUTOMATIC, GetDoubleClickTime() * 3);
			}
			hHandCursor = (HCURSOR)LoadImage(NULL, IDC_HAND, IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED);
			if (hHandCursor == NULL) // use fallback out of miranda32.exe
				hHandCursor = (HCURSOR)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(214), IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED);
#endif 
			SendDlgItemMessage(hwndDlg, IDC_TEXT, EM_SETLANGOPTIONS, 0, SendDlgItemMessage(hwndDlg, IDC_TEXT, EM_GETLANGOPTIONS, 0, 0) | IMF_UIFONTS);
			SetWindowPos(hwndDlg, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
			return TRUE;
	case WM_SIZE:
		{
			Utils_ResizeDialog(hwndDlg, hInst, MAKEINTRESOURCEA(IDD_HELP), HelpDialogResize);
			InvalidateRect(hwndDlg, NULL, TRUE);
#ifdef EDITOR
			break;
#endif
		}
#ifndef EDITOR
	case WM_MOVE:
		if (IsWindow(hwndShadowDlg)) {
			RECT rc;
			HRGN hRgnShadow, hRgnDlg;
			if (!GetWindowRect(hwndDlg, &rc))
				break;
			hRgnShadow = CreateRectRgnIndirect(&rc);
			if (hRgnShadow == NULL)
				break;
			OffsetRgn(hRgnShadow, 5, 5);
			hRgnDlg = CreateRectRgnIndirect(&rc);
			if (hRgnDlg == NULL)
				break;
			if (CombineRgn(hRgnShadow, hRgnShadow, hRgnDlg, RGN_DIFF) == ERROR) {
				DeleteObject(hRgnShadow);
				DeleteObject(hRgnDlg);
				break;
			}
			DeleteObject(hRgnDlg);
			OffsetRgn(hRgnShadow, -rc.left - 5, -rc.top - 5);
			SetWindowRgn(hwndShadowDlg, hRgnShadow, FALSE); // system gets ownership of hRgnShadow
			SetWindowPos(hwndShadowDlg, HWND_TOPMOST, rc.left + 5, rc.top + 5, rc.right - rc.left, rc.bottom - rc.top, SWP_SHOWWINDOW);
			SetWindowPos(hwndDlg, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		}
		break;
	case WM_KEYDOWN:
		if (GetKeyState(VK_CONTROL) & 0x8000 && wParam == 'C') {
			SendMessage(hwndDlg, M_CLIPBOARDCOPY, 0, 0);
			return TRUE;
		}
		break;
	case WM_CONTEXTMENU:
		{
			POINT pt;
			POINTSTOPOINT(pt, MAKEPOINTS(lParam));
			HMENU hMenu = CreatePopupMenu();
			AppendMenu(hMenu, MF_STRING, WM_COPY, TranslateT("&Copy"));
			if (TrackPopupMenuEx(hMenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_HORPOSANIMATION | TPM_VERPOSANIMATION | TPM_RIGHTBUTTON | TPM_RETURNCMD | TPM_NONOTIFY, pt.x, pt.y, hwndDlg, NULL))
				SendMessage(hwndDlg, M_CLIPBOARDCOPY, 0, 0);
			DestroyMenu(hMenu);
		}
		return 0;
	case WM_LBUTTONDBLCLK:
		DestroyWindow(hwndDlg);
		return TRUE;
	case M_CLIPBOARDCOPY:
		{
			HWND hwnd = GetFocus();
			if (hwnd == GetDlgItem(hwndDlg, IDC_CTLTEXT) || hwnd == GetDlgItem(hwndDlg, IDC_TEXT)) {
				CHARRANGE sel;
				ZeroMemory(&sel, sizeof(sel));
				SendMessage(hwnd, EM_GETSEL, (WPARAM)&sel.cpMin, (LPARAM)&sel.cpMax);
				if (sel.cpMin != sel.cpMax) {
					SendMessage(hwnd, WM_COPY, 0, 0);
					return TRUE;
				}
			}
		}
		if (OpenClipboard(hwndDlg)) {
			HGLOBAL hglb;
			int cch, len;
			EmptyClipboard();
			hglb = GlobalAlloc(GMEM_MOVEABLE, sizeof(LCID));
			if (hglb != NULL) {
				LCID *plocale = (LCID*)GlobalLock(hglb); // look at !!
				if (plocale != NULL) {
					*plocale = locale;
					GlobalUnlock(hglb);
					if (!SetClipboardData(CF_LOCALE, hglb))
						GlobalFree(hglb); // shell takes ownership
				}
				else
					GlobalFree(hglb);
			}
			cch = GetWindowTextLength(GetDlgItem(hwndDlg, IDC_CTLTEXT)) + GetWindowTextLength(GetDlgItem(hwndDlg, IDC_TEXT)) + 3;
			hglb = GlobalAlloc(GMEM_MOVEABLE, (cch + 1)*sizeof(TCHAR));
			if (hglb != NULL) {
				TCHAR *pszText = (TCHAR*)GlobalLock(hglb);
				if (pszText != NULL) {
					if (!GetWindowText(GetDlgItem(hwndDlg, IDC_CTLTEXT), pszText, cch - 2)) pszText[0] = _T('\0');
					len = lstrlen(pszText);
					if (GetWindowText(GetDlgItem(hwndDlg, IDC_TEXT), pszText + len + 2, cch - 2 - len) && len) {
						pszText[len] = _T('\r');
						pszText[len + 1] = _T('\n');
					}
					GlobalUnlock(hglb);
					if (!SetClipboardData(CF_UNICODETEXT, hglb))
						GlobalFree(hglb); // shell takes ownership
				}
				else GlobalFree(hglb);
			}
			CloseClipboard();
		}
		return TRUE;
	case WM_CTLCOLORDLG:
	case WM_CTLCOLORSTATIC:
		SetTextColor((HDC)wParam, GetSysColor(COLOR_INFOTEXT));
		SetBkColor((HDC)wParam, GetSysColor(COLOR_INFOBK));
		return (INT_PTR)GetSysColorBrush(COLOR_INFOBK);
	case WM_ACTIVATE:
		if (LOWORD(wParam) != WA_INACTIVE)
			break;
		if (GetParent((HWND)lParam) == hwndDlg)
			break;
		// fall through
	case WM_SYSCOLORCHANGE:
	case WM_ACTIVATEAPP:
		PostMessage(hwndDlg, WM_CLOSE, 0, 0); // no DestroyWindow() here! would cause recursion
		break;
#endif // !defined EDITOR
	case M_CHANGEHELPCONTROL:
		if (hwndCtl == (HWND)lParam)
			break;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
		hwndCtl = (HWND)lParam;
		SetDlgItemText(hwndDlg, IDC_CTLTEXT, NULL);
#ifdef EDITOR
		{
			TCHAR text[1024];
			char *szModule, *szDlgId;

			GetControlTitle(hwndCtl, text, _countof(text));
			SetDlgItemText(hwndDlg, IDC_CTLTEXT, text);
			mir_sntprintf(text, sizeof(text), TranslateT("Control ID: %d"), GetControlID(hwndCtl));
			SetDlgItemText(hwndDlg, IDC_CTLID, text);

			szDlgId = CreateDialogIdString(GetControlDialog(hwndCtl));
			mir_sntprintf(text, sizeof(text), TranslateT("Dialog ID: %hs"), (szDlgId != NULL) ? szDlgId : Translate("Unknown"));
			mir_free(szDlgId); // does NULL check
			SetDlgItemText(hwndDlg, IDC_DLGID, text);

			mir_sntprintf(text, sizeof(text), TranslateT("Type: %s"), TranslateTS(szControlTypeNames[GetControlType(hwndCtl)]));
			SetDlgItemText(hwndDlg, IDC_CTLTYPE, text);

			szModule = GetControlModuleName(hwndCtl);
			mir_sntprintf(text, sizeof(text), TranslateT("Module: %hs"), szModule ? szModule : Translate("Unknown"));
			mir_free(szModule); // does NULL check
			SetDlgItemText(hwndDlg, IDC_MODULE, text);
		}
#endif // defined EDITOR
		SetDlgItemText(hwndDlg, IDC_TEXT, NULL);
		SendMessage(hwndDlg, M_LOADHELP, 0, 0);
#ifdef EDITOR
		ShowWindow(hwndDlg, SW_SHOWNORMAL);
#else
		SendDlgItemMessage(hwndDlg, IDC_TEXT, EM_REQUESTRESIZE, 0, 0);
		return FALSE;
#endif
		return TRUE;
	case M_HELPLOADFAILED:
		if (hwndCtl != (HWND)lParam)
			break;
#ifdef EDITOR
		EnableWindow(GetDlgItem(hwndDlg, IDC_TEXT), TRUE);
		{
			TCHAR text[2024];
			GetControlTitle(hwndCtl, text, _countof(text));
			SetDlgItemText(hwndDlg, IDC_CTLTEXT, text);
		}
#else
		SetDlgItemText(hwndDlg, IDC_CTLTEXT, TranslateT("No help pack installed!"));
#endif
		SetDlgItemText(hwndDlg, IDC_TEXT, NULL);
		MessageBeep(MB_ICONERROR);
		break;
#ifdef EDITOR
	case M_SAVECOMPLETE:
#endif
	case M_HELPLOADED:
		if (hwndCtl != (HWND)lParam)
			break;
	case M_LOADHELP:
		{
			TCHAR *szTitle;
			char *szText;
			char *szDlgId, *szModule;
			UINT codepage;
			BOOL isRTL;
			int id, loading;
#ifdef EDITOR
			EnableWindow(GetDlgItem(hwndDlg, IDC_TEXT), TRUE);
#endif
			szDlgId = CreateDialogIdString(GetControlDialog(hwndCtl));
			szModule = GetControlModuleName(hwndCtl);
			id = GetControlID(hwndCtl);
#ifndef EDITOR
			// show id string instead of help text when 'ctrl' key pressed
			if (msg == M_LOADHELP && GetAsyncKeyState(VK_CONTROL) & 0x8000) {
				char *buf = CreateControlIdentifier(szDlgId ? szDlgId : "unknown", szModule ? szModule : "unknown", id, hwndCtl);
				HWND hwnd = GetDlgItem(hwndDlg, IDC_CTLTEXT);
				SetWindowTextA(hwnd, buf); // accepts NULL
				SetDlgItemText(hwndDlg, IDC_TEXT, NULL);
				mir_free(buf); // does NULL check
				mir_free(szDlgId); // does NULL check
				mir_free(szModule); // does NULL check
				break;
			}
#endif	
			if (szDlgId == NULL || szModule == NULL) {
				SetDlgItemText(hwndDlg, IDC_TEXT, NULL);
#ifndef EDITOR
				SetDlgItemText(hwndDlg, IDC_CTLTEXT, TranslateT("No help available for this item."));
#endif
				mir_free(szDlgId); // does NULL check
				mir_free(szModule); // does NULL check
				break;
			}

			loading = GetControlHelp(hwndCtl, szDlgId, szModule, id, &szTitle, &szText, NULL, &locale, &codepage, &isRTL, (msg == M_HELPLOADED) ? GCHF_DONTLOAD : 0);
			if (!loading) {
				if (szText)
					StreamInHtml(GetDlgItem(hwndDlg, IDC_TEXT), szText, codepage, RGB(255, 0, 0));
				else
					SetDlgItemText(hwndDlg, IDC_TEXT, NULL);
				if (szTitle) {
					TCHAR buf[128];
					RECT rc;
					HFONT hFontPrev;
					DWORD exStyle;
					HWND hwndCtlText;
					HDC hdc;
					hwndCtlText = GetDlgItem(hwndDlg, IDC_CTLTEXT);
					exStyle = GetWindowLongPtr(GetDlgItem(hwndDlg, IDC_CTLTEXT), GWL_EXSTYLE);
					hdc = GetDC(hwndCtlText);
					{
						DWORD(WINAPI *pfnGetLayout)(HDC); // obey right-to-left languages 
						*(FARPROC*)&pfnGetLayout = GetProcAddress(GetModuleHandleA("GDI32"), "GetLayout");
						if (pfnGetLayout) isRTL = (isRTL && !pfnGetLayout(hdc));
						if (isRTL)
							exStyle |= WS_EX_RTLREADING | WS_EX_RIGHT;
						else
							exStyle &= ~(WS_EX_RTLREADING | WS_EX_RIGHT);
					}
					mir_sntprintf(buf, sizeof(buf) - 4, L"%s", szTitle);
					if (hdc != NULL && hwndCtlText != NULL) {
						SendMessage(hwndCtlText, EM_GETRECT, 0, (LPARAM)&rc);
						hFontPrev = (HFONT)SelectObject(hdc, (HFONT)SendMessage(hwndCtlText, WM_GETFONT, 0, 0)); // look at !!
						// doesn't actually draw the string due to DT_CALCRECT
						DrawTextEx(hdc, buf, -1, &rc, DT_MODIFYSTRING | DT_CALCRECT | DT_EDITCONTROL | DT_END_ELLIPSIS | DT_INTERNAL | (isRTL ? (DT_RTLREADING | DT_RIGHT) : DT_LEFT) | DT_NOPREFIX | DT_SINGLELINE, NULL);
						SelectObject(hdc, hFontPrev);
						ReleaseDC(hwndCtlText, hdc);
					}
					SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_CTLTEXT), GWL_EXSTYLE, exStyle);
					SetWindowText(hwndCtlText, buf);
				}
				else
					SetDlgItemText(hwndDlg, IDC_CTLTEXT, NULL);
			}
			else {
				if (msg == M_HELPLOADED) {
					SetDlgItemText(hwndDlg, IDC_TEXT, NULL);
#ifndef EDITOR
					SetDlgItemText(hwndDlg, IDC_CTLTEXT, TranslateT("No help available for this item."));
#endif
				}
				else {
#ifdef EDITOR
					EnableWindow(GetDlgItem(hwndDlg, IDC_TEXT), FALSE);
					SetDlgItemText(hwndDlg, IDC_TEXT, TranslateT("Loading..."));
#else
					SetDlgItemText(hwndDlg, IDC_CTLTEXT, TranslateT("Loading..."));
					SetDlgItemText(hwndDlg, IDC_TEXT, NULL);
#endif
				}
			}
			mir_free(szDlgId);
			mir_free(szModule);
			break;
		}
	case WM_NOTIFY:
		switch (((NMHDR*)lParam)->idFrom) {
		case IDC_TEXT:
			switch (((NMHDR*)lParam)->code) {
#ifdef EDITOR
			case EN_MSGFILTER:
				switch (((MSGFILTER*)lParam)->msg) {
				case WM_CHAR:
					switch (((MSGFILTER*)lParam)->wParam) {
					case 'B' - 'A' + 1:
					case 'I' - 'A' + 1:
					case 'U' - 'A' + 1:
					case 'H' - 'A' + 1:
					case 'L' - 'A' + 1:
					case 'S' - 'A' + 1:
					case 'G' - 'A' + 1:
						SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, TRUE);
						return TRUE;
					}
					break;
				case WM_KEYDOWN:
					{
						CHARFORMAT cf;
						int changes = 0;

						ZeroMemory(&cf, sizeof(cf));
						if (!(GetKeyState(VK_CONTROL) & 0x8000))
							break;
						cf.cbSize = sizeof(cf);
						SendDlgItemMessage(hwndDlg, IDC_TEXT, EM_GETCHARFORMAT, TRUE, (LPARAM)&cf);
						switch (((MSGFILTER*)lParam)->wParam) {
						case 'B':
							cf.dwEffects ^= CFE_BOLD;
							cf.dwMask = CFM_BOLD;
							changes = 1;
							break;
						case 'I':
							cf.dwEffects ^= CFE_ITALIC;
							cf.dwMask = CFM_ITALIC;
							changes = 1;
							break;
						case 'U':
							cf.dwEffects ^= CFE_UNDERLINE;
							cf.dwMask = CFM_UNDERLINE;
							changes = 1;
							break;
						case 'L':
							{
								CHOOSECOLOR cc;
								COLORREF custCol[16];
								ZeroMemory(&custCol, sizeof(custCol));
								ZeroMemory(&cc, sizeof(cc));
								cc.lStructSize = sizeof(cc);
								cc.hwndOwner = hwndDlg;
								cc.lpCustColors = custCol;
								cc.rgbResult = cf.crTextColor;
								cc.Flags = CC_ANYCOLOR | CC_FULLOPEN | CC_RGBINIT;
								if (!ChooseColor(&cc))
									break;
								cf.crTextColor = 0;
								cf.dwEffects = 0;
								if (cc.rgbResult)
									cf.crTextColor = cc.rgbResult;
								else cf.dwEffects = CFE_AUTOCOLOR;
								cf.dwMask = CFM_COLOR;
								changes = 1;
								break;
							}
						case 'H':
							cf.dwEffects ^= CFE_STRIKEOUT;
							cf.dwMask = CFM_STRIKEOUT;
							changes = 1;
							break;
						case VK_OEM_PLUS:
							cf.yHeight = ((GetKeyState(VK_SHIFT) & 0x8000) ? TEXTSIZE_BIG : TEXTSIZE_NORMAL) * 10;
							cf.dwMask = CFM_SIZE;
							changes = 1;
							break;
						case VK_OEM_MINUS:
							cf.yHeight = TEXTSIZE_SMALL * 10;
							cf.dwMask = CFM_SIZE;
							changes = 1;
							break;
						case 'S':
							{
								TCHAR szTitle[1024];
								char *szText, *szDlgId, *szModule;
								if (!GetDlgItemText(hwndDlg, IDC_CTLTEXT, szTitle, _countof(szTitle)))
									break;
								szDlgId = CreateDialogIdString(GetControlDialog(hwndCtl));
								if (szDlgId == NULL)
									break;
								szText = StreamOutHtml(GetDlgItem(hwndDlg, IDC_TEXT));
								szModule = GetControlModuleName(hwndCtl);
								if (szModule == NULL) {
									mir_free(szDlgId);
									break;
								}
								SetControlHelp(szDlgId, szModule, GetControlID(hwndCtl), szTitle, szText, GetControlType(hwndCtl));
								mir_free(szText);
								mir_free(szDlgId);
								mir_free(szModule);
								SaveDialogCache();
								SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, TRUE);
								return TRUE;
							}
						case 'G':
							SendMessage(hwndDlg, M_LOADHELP, 0, 0);
							SetWindowLong(hwndDlg, DWL_MSGRESULT, TRUE);
							return TRUE;
						}
						if (changes) {
							SendDlgItemMessage(hwndDlg, IDC_TEXT, EM_SETCHARFORMAT, SCF_WORD | SCF_SELECTION, (LPARAM)&cf);
							SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, TRUE);
							return TRUE;
						}
						break;
					}
				}
				break;
#else // defined EDITOR
			case EN_MSGFILTER:
				{
					MSGFILTER *msgf = (MSGFILTER*)lParam;
					switch (msgf->msg) {
					case WM_LBUTTONUP:
						{
							POINT pt;
							DWORD pos;
							CHARRANGE sel;
							char *pszLink;
							HWND hwndEdit = msgf->nmhdr.hwndFrom;

							ZeroMemory(&sel, sizeof(sel));
							SendMessage(msgf->nmhdr.hwndFrom, EM_EXGETSEL, 0, (LPARAM)&sel);
							if (sel.cpMin != sel.cpMax)
								break;
							POINTSTOPOINT(pt, MAKEPOINTS(msgf->lParam));
							pos = SendMessage(hwndEdit, EM_CHARFROMPOS, 0, (WPARAM)&pt);
							if (IsHyperlink(pos, NULL, NULL, &pszLink)) {
								Utils_OpenUrl(pszLink); // pszLink is MBCS string in CP_ACP
								SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, TRUE);
								return TRUE;
							}
						}
						break;
					case WM_MOUSEMOVE: // register hyperlink tooltips when current
						if (hwndToolTip != NULL) {
							POINTL pt;
							DWORD pos;
							CHARRANGE rng;
							char *pszLink;

							POINTSTOPOINT(pt, MAKEPOINTS(msgf->lParam));
							pos = SendMessage(msgf->nmhdr.hwndFrom, EM_CHARFROMPOS, 0, (WPARAM)&pt);
							if (IsHyperlink(pos, &rng.cpMin, &rng.cpMax, &pszLink)) { // pszLink is MBCS in CP_ACP
								TTTOOLINFOA ti = { 0 };
								ti.cbSize = sizeof(ti);
								ti.hwnd = msgf->nmhdr.hwndFrom;
								ti.uId = (UINT)rng.cpMin;
								if (!SendMessage(hwndToolTip, TTM_GETTOOLINFOA, 0, (LPARAM)&ti)) {
									LONG cpRectMin;
									ti.uFlags = TTF_SUBCLASS;
									ti.lpszText = pszLink;
									cpRectMin = rng.cpMin;
									while (!GetCharRangeRect(ti.hwnd, &rng.cpMin, rng.cpMax, &ti.rect)) {
										ti.uId = (UINT)cpRectMin;
										SendMessage(hwndToolTip, TTM_ADDTOOLA, 0, (LPARAM)&ti);
										cpRectMin = rng.cpMin;
									}
								}
							}
						}
						break;
					}
				}
				break;
			case EN_REQUESTRESIZE:
				{
					RECT rcDlg, rcEdit, rcCtl, rcNew;
					REQRESIZE *rr = (REQRESIZE*)lParam;
					POINT ptScreenBottomRight;

					if (!GetWindowRect(hwndDlg, &rcDlg))
						break;
					if (!GetWindowRect(hwndCtl, &rcCtl))
						break;
					if (!GetWindowRect(GetDlgItem(hwndDlg, IDC_TEXT), &rcEdit))
						break;
					rcNew.left = rcCtl.left + 30;
					rcNew.top = rcCtl.bottom + 10;
					rcNew.right = rcNew.left + (rr->rc.right - rr->rc.left) + (rcDlg.right - rcDlg.left) - (rcEdit.right - rcEdit.left) + (GetWindowLongPtr(GetDlgItem(hwndDlg, IDC_TEXT), GWL_STYLE)&WS_VSCROLL ? GetSystemMetrics(SM_CXVSCROLL) : 0);
					if (GetWindowTextLength(GetDlgItem(hwndDlg, IDC_TEXT)))
						rcNew.bottom = rcNew.top + min(GetSystemMetrics(SM_CYSCREEN) / 5, (rr->rc.bottom - rr->rc.top) + (rcDlg.bottom - rcDlg.top) - (rcEdit.bottom - rcEdit.top));
					else
						rcNew.bottom = rcNew.top + min(GetSystemMetrics(SM_CYSCREEN) / 5, (rcDlg.bottom - rcDlg.top) - (rcEdit.bottom - rcEdit.top));
					if (GetSystemMetrics(SM_CXVIRTUALSCREEN)) {
						ptScreenBottomRight.x = GetSystemMetrics(SM_CXVIRTUALSCREEN) + GetSystemMetrics(SM_XVIRTUALSCREEN);
						ptScreenBottomRight.y = GetSystemMetrics(SM_CYVIRTUALSCREEN) + GetSystemMetrics(SM_YVIRTUALSCREEN);
					}
					else {
						ptScreenBottomRight.x = GetSystemMetrics(SM_CXSCREEN);
						ptScreenBottomRight.y = GetSystemMetrics(SM_CYSCREEN);
					}
					if (rcNew.right >= ptScreenBottomRight.x)
						OffsetRect(&rcNew, ptScreenBottomRight.x - rcNew.right, 0);
					if (rcNew.bottom >= ptScreenBottomRight.y)
						OffsetRect(&rcNew, 0, ptScreenBottomRight.y - rcNew.bottom);
					SetWindowPos(hwndDlg, 0, rcNew.left, rcNew.top, rcNew.right - rcNew.left, rcNew.bottom - rcNew.top, SWP_NOZORDER);
					ShowWindow(hwndDlg, SW_SHOWNORMAL);
					break;
				}
#endif // defined EDITOR
			}
		}
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDCANCEL: // WM_CLOSE
			DestroyWindow(hwndDlg);
			return TRUE;
		}
		break;
	case WM_DESTROY:
#ifndef EDITOR
		FreeHyperlinkData();
		if (IsWindow(hwndShadowDlg))
			DestroyWindow(hwndShadowDlg);
		if (hwndToolTip != NULL)
			DestroyWindow(hwndToolTip);
#endif
		hwndHelpDlg = NULL;
		break;
	}
	return FALSE;
}
