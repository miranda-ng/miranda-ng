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

CSrmmBaseDialog::CSrmmBaseDialog(HINSTANCE hInst, int idDialog, SESSION_INFO *si) :
	CDlgBase(hInst, idDialog),
	m_si(si),
	m_pLog(nullptr),
	m_pEntry(nullptr),
	m_hContact(0)
{
	m_bFilterEnabled = db_get_b(NULL, CHAT_MODULE, "FilterEnabled", 0) != 0;
	m_bNicklistEnabled = db_get_b(NULL, CHAT_MODULE, "ShowNicklist", 1) != 0;
	m_iLogFilterFlags = db_get_dw(NULL, CHAT_MODULE, "FilterFlags", 0x03E0);

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
	CHARRANGE sel;

	if (msg == WM_NOTIFY && m_pLog != nullptr) {
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
				m_pLog->SendMsg(EM_EXGETSEL, 0, (LPARAM)&sel);
				if (sel.cpMin != sel.cpMax)
					break;

				CMStringW wszText(' ', pLink->chrg.cpMax - pLink->chrg.cpMin + 1);
				{
					TEXTRANGE tr;
					tr.chrg = pLink->chrg;
					tr.lpstrText = wszText.GetBuffer();
					m_pLog->SendMsg(EM_GETTEXTRANGE, 0, (LPARAM)&tr);
					if (wcschr(tr.lpstrText, '@') != NULL && wcschr(tr.lpstrText, ':') == NULL && wcschr(tr.lpstrText, '/') == NULL)
						wszText.Insert(0, L"mailto:");
				}

				if (pLink->msg == WM_RBUTTONDOWN) {
					HMENU hMenu = LoadMenu(g_hInst, MAKEINTRESOURCE(IDR_CONTEXT));
					HMENU hSubMenu = GetSubMenu(hMenu, 6);
					TranslateMenu(hSubMenu);

					POINT pt = { GET_X_LPARAM(pLink->lParam), GET_Y_LPARAM(pLink->lParam) };
					ClientToScreen(((NMHDR *)lParam)->hwndFrom, &pt);

					switch (TrackPopupMenu(hSubMenu, TPM_RETURNCMD, pt.x, pt.y, 0, m_hwnd, NULL)) {
					case IDM_OPENLINK:
						Utils_OpenUrlW(wszText);
						break;

					case IDM_COPYLINK:
						if (OpenClipboard(m_hwnd)) {
							EmptyClipboard();
							HGLOBAL hData = GlobalAlloc(GMEM_MOVEABLE, (wszText.GetLength()+1) * sizeof(wchar_t));
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

	return CDlgBase::DlgProc(msg, wParam, lParam);
}
