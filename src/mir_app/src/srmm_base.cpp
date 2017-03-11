/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-17 Miranda NG project,
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

#include "chat.h"

extern HCURSOR g_hCurHyperlinkHand;

CSrmmBaseDialog::CSrmmBaseDialog(HINSTANCE hInst, int idDialog, SESSION_INFO *si)
	: CDlgBase(hInst, idDialog),
	m_si(si),
	m_pLog(nullptr),
	m_pEntry(nullptr),
	m_pFilter(nullptr),
	m_pColor(nullptr),
	m_pBkColor(nullptr),
	m_hContact(0)
{
	m_bFilterEnabled = db_get_b(0, CHAT_MODULE, "FilterEnabled", 0) != 0;
	m_bNicklistEnabled = db_get_b(0, CHAT_MODULE, "ShowNicklist", 1) != 0;
	m_iLogFilterFlags = db_get_dw(0, CHAT_MODULE, "FilterFlags", 0x03E0);

	if (si) {
		m_hContact = si->hContact;

		MODULEINFO *mi = chatApi.MM_FindModule(si->pszModule);
		if (mi == nullptr) {
			if (mi->bColor) {
				m_iFG = 4;
				m_bFGSet = true;
			}
			if (mi->bBkgColor) {
				m_iBG = 2;
				m_bBGSet = true;
			}
		}
	}
}

INT_PTR CSrmmBaseDialog::DlgProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_COMMAND:
		if (!lParam && Clist_MenuProcessCommand(LOWORD(wParam), MPCF_CONTACTMENU, m_hContact))
			return 0;

		if (wParam >= MIN_CBUTTONID && wParam <= MAX_CBUTTONID) {
			Srmm_ClickToolbarIcon(m_hContact, wParam, GetDlgItem(m_hwnd, wParam), 0);
			return 0;
		}
		break;

	case WM_NOTIFY:
		if (m_pLog != nullptr) {
			LPNMHDR hdr = (LPNMHDR)lParam;
			if (hdr->hwndFrom == m_pLog->GetHwnd() && hdr->code == EN_LINK) {
				ENLINK *pLink = (ENLINK*)lParam;
				switch (pLink->msg) {
				case WM_SETCURSOR:
					SetCursor(g_hCurHyperlinkHand);
					SetWindowLongPtr(m_hwnd, DWLP_MSGRESULT, TRUE);
					return TRUE;

				case WM_RBUTTONDOWN:
				case WM_LBUTTONUP:
				case WM_LBUTTONDBLCLK:
					CHARRANGE sel;
					m_pLog->SendMsg(EM_EXGETSEL, 0, (LPARAM)&sel);
					if (sel.cpMin != sel.cpMax)
						break;

					CMStringW wszText(' ', pLink->chrg.cpMax - pLink->chrg.cpMin + 1);

					TEXTRANGE tr;
					tr.chrg = pLink->chrg;
					tr.lpstrText = wszText.GetBuffer();
					m_pLog->SendMsg(EM_GETTEXTRANGE, 0, (LPARAM)&tr);
					if (wcschr(tr.lpstrText, '@') != nullptr && wcschr(tr.lpstrText, ':') == nullptr && wcschr(tr.lpstrText, '/') == nullptr)
						wszText.Insert(0, L"mailto:");

					if (pLink->msg == WM_RBUTTONDOWN) {
						HMENU hMenu = LoadMenu(g_hInst, MAKEINTRESOURCE(IDR_CONTEXT));
						HMENU hSubMenu = GetSubMenu(hMenu, 6);
						TranslateMenu(hSubMenu);

						POINT pt = { GET_X_LPARAM(pLink->lParam), GET_Y_LPARAM(pLink->lParam) };
						ClientToScreen(((NMHDR *)lParam)->hwndFrom, &pt);

						switch (TrackPopupMenu(hSubMenu, TPM_RETURNCMD, pt.x, pt.y, 0, m_hwnd, nullptr)) {
						case IDM_OPENLINK:
							Utils_OpenUrlW(wszText);
							break;

						case IDM_COPYLINK:
							if (OpenClipboard(m_hwnd)) {
								EmptyClipboard();
								HGLOBAL hData = GlobalAlloc(GMEM_MOVEABLE, (wszText.GetLength() + 1) * sizeof(wchar_t));
								mir_wstrcpy((wchar_t*)GlobalLock(hData), wszText);
								GlobalUnlock(hData);
								SetClipboardData(CF_UNICODETEXT, hData);
								CloseClipboard();
							}
							break;
						}

						DestroyMenu(hMenu);
						SetWindowLongPtr(m_hwnd, DWLP_MSGRESULT, TRUE);
						return TRUE;
					}

					Utils_OpenUrlW(wszText);
					if (m_pEntry != nullptr)
						SetFocus(m_pEntry->GetHwnd());
				}
			}
		}
		break;
	}

	return CDlgBase::DlgProc(msg, wParam, lParam);
}

void CSrmmBaseDialog::AddLog()
{
	if (m_si->pLogEnd)
		StreamInEvents(m_si->pLog, false);
	else
		ClearLog();
}

void CSrmmBaseDialog::ClearLog()
{
	if (m_pLog != nullptr)
		m_pLog->SetText(L"");
}

void CSrmmBaseDialog::DoEventHook(int iType, const USERINFO *pUser, const wchar_t *pszText, INT_PTR dwItem)
{
	GCDEST gcd = {};
	gcd.pszModule = m_si->pszModule;
	gcd.ptszID = m_si->ptszID;
	gcd.iType = iType;

	GCHOOK gch = {};
	if (pUser != nullptr) {
		gch.ptszUID = pUser->pszUID;
		gch.ptszNick = pUser->pszNick;
	}

	gch.ptszText = (LPWSTR)pszText;
	gch.dwData = dwItem;
	gch.pDest = &gcd;
	NotifyEventHooks(chatApi.hSendEvent, 0, (WPARAM)&gch);
}

void CSrmmBaseDialog::RedrawLog2()
{
	m_si->LastTime = 0;
	if (m_si->pLog)
		StreamInEvents(m_si->pLogEnd, TRUE);
}
