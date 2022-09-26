/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team,
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

/////////////////////////////////////////////////////////////////////////////////////////
// SRMM log container

#include "stdafx.h"
#include "chat.h"

#define EVENTTYPE_STATUSCHANGE 25368
#define EVENTTYPE_ERRMSG 25366

CRtfLogWindow::CRtfLogWindow(CMsgDialog &pDlg) :
	CSrmmLogWindow(pDlg),
	m_rtf(*(CCtrlRichEdit*)pDlg.FindControl(IDC_SRMM_LOG))
{
}

CRtfLogWindow::~CRtfLogWindow()
{
}

/////////////////////////////////////////////////////////////////////////////////////////

EXTERN_C MIR_APP_DLL(LRESULT) CALLBACK stubLogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CRtfLogWindow *pLog = (CRtfLogWindow *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	if (pLog != nullptr)
		return pLog->WndProc(msg, wParam, lParam);

	return mir_callNextSubclass(hwnd, stubLogProc, msg, wParam, lParam);
}

void CRtfLogWindow::Attach()
{
	SetWindowLongPtr(m_rtf.GetHwnd(), GWLP_USERDATA, LPARAM(this));
	m_rtf.SetReadOnly(true);

	mir_subclassWindow(m_rtf.GetHwnd(), stubLogProc);
}

/////////////////////////////////////////////////////////////////////////////////////////

void CRtfLogWindow::Detach()
{
	mir_unsubclassWindow(m_rtf.GetHwnd(), stubLogProc);
}

/////////////////////////////////////////////////////////////////////////////////////////

bool CRtfLogWindow::AtBottom()
{
	if (!(GetWindowLongPtr(m_rtf.GetHwnd(), GWL_STYLE) & WS_VSCROLL))
		return false;

	SCROLLINFO si = {};
	si.cbSize = sizeof(si);
	si.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;
	GetScrollInfo(m_rtf.GetHwnd(), SB_VERT, &si);
	return (si.nPos + (int)si.nPage + 5) >= si.nMax;
}

void CRtfLogWindow::Clear()
{
	m_rtf.SetText(L"");
}

HWND CRtfLogWindow::GetHwnd()
{
	return m_rtf.GetHwnd();
}

int CRtfLogWindow::GetType()
{
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

static DWORD CALLBACK StreamOutCallback(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb)
{
	CMStringW *str = (CMStringW *)dwCookie;
	str->Append((wchar_t*)pbBuff, cb / 2);
	*pcb = cb;
	return 0;
}

wchar_t* CRtfLogWindow::GetSelection()
{
	CHARRANGE sel;
	SendMessage(m_rtf.GetHwnd(), EM_EXGETSEL, 0, (LPARAM)&sel);
	if (sel.cpMin == sel.cpMax)
		return nullptr;

	CMStringW result;

	EDITSTREAM stream;
	memset(&stream, 0, sizeof(stream));
	stream.pfnCallback = StreamOutCallback;
	stream.dwCookie = (DWORD_PTR)&result;
	SendMessage(m_rtf.GetHwnd(), EM_STREAMOUT, SF_TEXT | SF_UNICODE | SFF_SELECTION, (LPARAM)&stream);
	return result.Detach();
}

/////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CRtfLogWindow::Notify(WPARAM, LPARAM lParam)
{
	LPNMHDR hdr = (LPNMHDR)lParam;
	if (hdr->code != EN_LINK)
		return FALSE;

	ENLINK *pLink = (ENLINK *)lParam;
	switch (pLink->msg) {
	case WM_SETCURSOR:
		SetCursor(g_hCurHyperlinkHand);
		SetWindowLongPtr(m_pDlg.m_hwnd, DWLP_MSGRESULT, TRUE);
		return TRUE;

	case WM_RBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_LBUTTONDBLCLK:
		CHARRANGE sel;
		m_rtf.SendMsg(EM_EXGETSEL, 0, (LPARAM)&sel);
		if (sel.cpMin != sel.cpMax)
			break;

		CMStringW wszText(' ', pLink->chrg.cpMax - pLink->chrg.cpMin + 1);

		TEXTRANGE tr;
		tr.chrg = pLink->chrg;
		tr.lpstrText = wszText.GetBuffer();
		m_rtf.SendMsg(EM_GETTEXTRANGE, 0, (LPARAM)&tr);
		if (wcschr(tr.lpstrText, '@') != nullptr && wcschr(tr.lpstrText, ':') == nullptr && wcschr(tr.lpstrText, '/') == nullptr)
			wszText.Insert(0, L"mailto:");

		if (pLink->msg == WM_RBUTTONDOWN) {
			HMENU hMenu = LoadMenu(g_plugin.getInst(), MAKEINTRESOURCE(IDR_CONTEXT));
			HMENU hSubMenu = GetSubMenu(hMenu, 6);
			TranslateMenu(hSubMenu);

			POINT pt = { GET_X_LPARAM(pLink->lParam), GET_Y_LPARAM(pLink->lParam) };
			ClientToScreen(((NMHDR *)lParam)->hwndFrom, &pt);

			switch (TrackPopupMenu(hSubMenu, TPM_RETURNCMD, pt.x, pt.y, 0, m_pDlg.m_hwnd, nullptr)) {
			case IDM_OPENLINK:
				Utils_OpenUrlW(wszText);
				break;

			case IDM_COPYLINK:
				Utils_ClipboardCopy(wszText);
				break;
			}

			DestroyMenu(hMenu);
			SetWindowLongPtr(m_pDlg.m_hwnd, DWLP_MSGRESULT, TRUE);
			return TRUE;
		}

		Utils_OpenUrlW(wszText);
		SetFocus(m_pDlg.m_message.GetHwnd());
	}

	return FALSE;
}

void CRtfLogWindow::Resize()
{
	bool bottomScroll = !m_pDlg.isChat();
	if (AtBottom())
		bottomScroll = true;

	// ::MoveWindow(m_rtf.GetHwnd(), x, y, cx, cy, true);

	if (bottomScroll)
		ScrollToBottom();
}

void CRtfLogWindow::ScrollToBottom()
{
	if (!(GetWindowLongPtr(m_rtf.GetHwnd(), GWL_STYLE) & WS_VSCROLL))
		return;

	SCROLLINFO si = {};
	si.cbSize = sizeof(si);
	si.fMask = SIF_PAGE | SIF_RANGE;
	GetScrollInfo(m_rtf.GetHwnd(), SB_VERT, &si);

	si.fMask = SIF_POS;
	si.nPos = si.nMax - si.nPage;
	SetScrollInfo(m_rtf.GetHwnd(), SB_VERT, &si, TRUE);
	m_rtf.SendMsg(WM_VSCROLL, MAKEWPARAM(SB_BOTTOM, 0), 0);
}

/////////////////////////////////////////////////////////////////////////////////////////

static wchar_t szTrimString[] = L":;,.!?\'\"><()[]- \r\n";

INT_PTR CRtfLogWindow::WndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	CHARRANGE sel;

	switch (msg) {
	case WM_ACTIVATE:
		if (LOWORD(wParam) == WA_INACTIVE) {
			m_rtf.SendMsg(EM_EXGETSEL, 0, (LPARAM)&sel);
			if (sel.cpMin != sel.cpMax) {
				sel.cpMin = sel.cpMax;
				m_rtf.SendMsg(EM_EXSETSEL, 0, (LPARAM)&sel);
			}
		}
		break;

	case WM_SETCURSOR:
		if (m_pDlg.m_bInMenu) {
			SetCursor(LoadCursor(nullptr, IDC_ARROW));
			return TRUE;
		}
		break;

	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		if (!(GetKeyState(VK_RMENU) & 0x8000)) {
			MSG message = { m_pDlg.m_hwnd, msg, wParam, lParam };
			LRESULT iButtonFrom = Hotkey_Check(&message, BB_HK_SECTION);
			if (iButtonFrom) {
				Srmm_ProcessToolbarHotkey(m_pDlg.m_hContact, iButtonFrom, m_pDlg.m_hwnd);
				return TRUE;
			}
		}
		break;

	case WM_CHAR:
		if (wParam >= ' ') {
			SetFocus(m_pDlg.m_message.GetHwnd());
			m_pDlg.m_message.SendMsg(WM_CHAR, wParam, lParam);
		}
		else if (wParam == '\t')
			SetFocus(m_pDlg.m_message.GetHwnd());
		break;

	case WM_CONTEXTMENU:
		POINT pt, ptl;
		m_rtf.SendMsg(EM_EXGETSEL, 0, (LPARAM)&sel);
		if (lParam == 0xFFFFFFFF) {
			m_rtf.SendMsg(EM_POSFROMCHAR, (WPARAM)&pt, (LPARAM)sel.cpMax);
			ClientToScreen(m_rtf.GetHwnd(), &pt);
		}
		else {
			pt.x = GET_X_LPARAM(lParam);
			pt.y = GET_Y_LPARAM(lParam);
		}
		ptl = pt;
		ScreenToClient(m_rtf.GetHwnd(), &ptl);
		{
			wchar_t *pszWord = (wchar_t *)_alloca(8192);
			pszWord[0] = '\0';

			// get a word under cursor
			if (sel.cpMin == sel.cpMax) {
				int iCharIndex = m_rtf.SendMsg(EM_CHARFROMPOS, 0, (LPARAM)&ptl);
				if (iCharIndex < 0)
					break;

				sel.cpMin = m_rtf.SendMsg(EM_FINDWORDBREAK, WB_LEFT, iCharIndex);
				sel.cpMax = m_rtf.SendMsg(EM_FINDWORDBREAK, WB_RIGHT, iCharIndex);
			}

			if (sel.cpMax > sel.cpMin) {
				TEXTRANGE tr = { 0 };
				tr.chrg = sel;
				tr.lpstrText = pszWord;
				int iRes = m_rtf.SendMsg(EM_GETTEXTRANGE, 0, (LPARAM)&tr);
				if (iRes > 0) {
					wchar_t *p = wcschr(pszWord, '\r');
					if (p)
						*p = 0;

					size_t iLen = mir_wstrlen(pszWord) - 1;
					while (wcschr(szTrimString, pszWord[iLen])) {
						pszWord[iLen] = '\0';
						iLen--;
					}
				}
			}

			CHARRANGE all = { 0, -1 };
			HMENU hMenu = LoadMenu(g_plugin.getInst(), MAKEINTRESOURCE(IDR_LOGMENU));
			HMENU hSubMenu = GetSubMenu(hMenu, 0);
			TranslateMenu(hSubMenu);
			m_pDlg.m_bInMenu = true;

			int flags = MF_BYPOSITION | (GetRichTextLength(m_rtf.GetHwnd()) == 0 ? MF_GRAYED : MF_ENABLED);
			EnableMenuItem(hSubMenu, 0, flags);
			EnableMenuItem(hSubMenu, 2, flags);

			if (pszWord && pszWord[0]) {
				CMStringW wszText(FORMAT, TranslateT("Look up '%s':"), pszWord);
				if (wszText.GetLength() > 30) {
					wszText.Truncate(30);
					wszText.AppendChar('\'');
				}
				ModifyMenu(hSubMenu, 4, MF_STRING | MF_BYPOSITION, 4, wszText);
			}
			else ModifyMenu(hSubMenu, 4, MF_STRING | MF_GRAYED | MF_BYPOSITION, 4, TranslateT("No word to look up"));

			UINT uID = Chat_CreateMenu(m_rtf.GetHwnd(), hSubMenu, pt, m_pDlg.m_si, nullptr);
			m_pDlg.m_bInMenu = false;
			DestroyMenu(hMenu);

			switch (uID) {
			case 0:
				PostMessage(m_pDlg.m_hwnd, WM_MOUSEACTIVATE, 0, 0);
				break;

			case IDM_COPYALL:
				m_rtf.SendMsg(EM_EXGETSEL, 0, (LPARAM)&sel);
				m_rtf.SendMsg(EM_EXSETSEL, 0, (LPARAM)&all);
				m_rtf.SendMsg(WM_COPY, 0, 0);
				m_rtf.SendMsg(EM_EXSETSEL, 0, (LPARAM)&sel);
				PostMessage(m_pDlg.m_hwnd, WM_MOUSEACTIVATE, 0, 0);
				break;

			case IDM_CLEAR:
				m_rtf.SetText(L"");
				if (auto *si = m_pDlg.m_si) {
					g_chatApi.LM_RemoveAll(&si->pLog, &si->pLogEnd);
					si->iEventCount = 0;
					si->LastTime = 0;
				}
				PostMessage(m_pDlg.m_hwnd, WM_MOUSEACTIVATE, 0, 0);
				break;

			case IDM_SEARCH_GOOGLE:
			case IDM_SEARCH_BING:
			case IDM_SEARCH_YANDEX:
			case IDM_SEARCH_YAHOO:
			case IDM_SEARCH_WIKIPEDIA:
			case IDM_SEARCH_FOODNETWORK:
			case IDM_SEARCH_GOOGLE_MAPS:
			case IDM_SEARCH_GOOGLE_TRANSLATE:
				{
					CMStringW szURL;
					switch (uID) {
					case IDM_SEARCH_WIKIPEDIA:
						szURL.Format(L"http://en.wikipedia.org/wiki/%s", pszWord);
						break;
					case IDM_SEARCH_YAHOO:
						szURL.Format(L"http://search.yahoo.com/search?p=%s&ei=UTF-8", pszWord);
						break;
					case IDM_SEARCH_FOODNETWORK:
						szURL.Format(L"http://search.foodnetwork.com/search/delegate.do?fnSearchString=%s", pszWord);
						break;
					case IDM_SEARCH_BING:
						szURL.Format(L"http://www.bing.com/search?q=%s&form=OSDSRC", pszWord);
						break;
					case IDM_SEARCH_GOOGLE_MAPS:
						szURL.Format(L"http://maps.google.com/maps?q=%s&ie=utf-8&oe=utf-8", pszWord);
						break;
					case IDM_SEARCH_GOOGLE_TRANSLATE:
						szURL.Format(L"http://translate.google.com/?q=%s&ie=utf-8&oe=utf-8", pszWord);
						break;
					case IDM_SEARCH_YANDEX:
						szURL.Format(L"http://yandex.ru/yandsearch?text=%s", pszWord);
						break;
					case IDM_SEARCH_GOOGLE:
						szURL.Format(L"http://www.google.com/search?q=%s&ie=utf-8&oe=utf-8", pszWord);
						break;
					}
					Utils_OpenUrlW(szURL);
				}
				PostMessage(m_pDlg.m_hwnd, WM_MOUSEACTIVATE, 0, 0);
				break;

			default:
				PostMessage(m_pDlg.m_hwnd, WM_MOUSEACTIVATE, 0, 0);
				Chat_DoEventHook(m_pDlg.m_si, GC_USER_LOGMENU, nullptr, nullptr, uID);
				break;
			}
		}
		return 0;
	}

	LRESULT res = mir_callNextSubclass(m_rtf.GetHwnd(), stubLogProc, msg, wParam, lParam);
	if (msg == WM_GETDLGCODE)
		return res & ~DLGC_HASSETSEL;
	return res;
}
