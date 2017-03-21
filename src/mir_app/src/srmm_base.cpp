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
	m_btnFilter(this, IDC_SRMM_FILTER),
	m_btnColor(this, IDC_SRMM_COLOR),
	m_btnBkColor(this, IDC_SRMM_BKGCOLOR),
	m_btnBold(this, IDC_SRMM_BOLD),
	m_btnItalic(this, IDC_SRMM_ITALICS),
	m_btnUnderline(this, IDC_SRMM_UNDERLINE),

	m_si(si),
	m_pLog(nullptr),
	m_pEntry(nullptr),
	m_hContact(0),
	m_clrInputBG(GetSysColor(COLOR_WINDOW))
{
	m_bFilterEnabled = db_get_b(0, CHAT_MODULE, "FilterEnabled", 0) != 0;
	m_bNicklistEnabled = db_get_b(0, CHAT_MODULE, "ShowNicklist", 1) != 0;
	m_iLogFilterFlags = db_get_dw(0, CHAT_MODULE, "FilterFlags", 0x03E0);

	m_btnColor.OnClick = Callback(this, &CSrmmBaseDialog::onClick_Color);
	m_btnBkColor.OnClick = Callback(this, &CSrmmBaseDialog::onClick_BkColor);
	m_btnBold.OnClick = m_btnItalic.OnClick = m_btnUnderline.OnClick = Callback(this, &CSrmmBaseDialog::onClick_BIU);

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

CSrmmBaseDialog::CSrmmBaseDialog(const CSrmmBaseDialog&) :
	CDlgBase(0, 0),
	m_btnColor(0, 0), m_btnBkColor(0, 0), m_btnFilter(0, 0),
	m_btnBold(0, 0), m_btnItalic(0, 0), m_btnUnderline(0, 0)
{
}

CSrmmBaseDialog& CSrmmBaseDialog::operator=(const CSrmmBaseDialog&)
{
	return *this;
}

/////////////////////////////////////////////////////////////////////////////////////////

static LRESULT CALLBACK Srmm_ButtonSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_RBUTTONUP:
		if (db_get_b(0, CHAT_MODULE, "RightClickFilter", 0) != 0) {
			CSrmmBaseDialog *pDlg = (CSrmmBaseDialog*)GetWindowLongPtr(GetParent(hwnd), GWLP_USERDATA);
			if (pDlg == nullptr)
				break;

			switch (GetDlgCtrlID(hwnd)) {
			case IDC_SRMM_FILTER:
				pDlg->ShowFilterMenu();
				break;

			case IDC_SRMM_COLOR:
				pDlg->ShowColorChooser(IDC_SRMM_COLOR);
				break;

			case IDC_SRMM_BKGCOLOR:
				pDlg->ShowColorChooser(IDC_SRMM_BKGCOLOR);
				break;
			}
		}
		break;
	}

	return mir_callNextSubclass(hwnd, Srmm_ButtonSubclassProc, msg, wParam, lParam);
}

void CSrmmBaseDialog::OnInitDialog()
{
	mir_subclassWindow(m_btnFilter.GetHwnd(), Srmm_ButtonSubclassProc);
	mir_subclassWindow(m_btnColor.GetHwnd(), Srmm_ButtonSubclassProc);
	mir_subclassWindow(m_btnBkColor.GetHwnd(), Srmm_ButtonSubclassProc);

	LoadSettings();
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

/////////////////////////////////////////////////////////////////////////////////////////

void CSrmmBaseDialog::onClick_Color(CCtrlButton *pButton)
{
	if (!pButton->Enabled())
		return;

	CHARFORMAT2 cf;
	cf.cbSize = sizeof(CHARFORMAT2);
	cf.dwEffects = 0;
	cf.dwMask = CFM_COLOR;

	if (IsDlgButtonChecked(m_hwnd, pButton->GetCtrlId())) {
		if (db_get_b(0, CHAT_MODULE, "RightClickFilter", 0) == 0) {
			ShowColorChooser(pButton->GetCtrlId());
			return;
		}
		if (m_bFGSet)
			cf.crTextColor = chatApi.MM_FindModule(m_si->pszModule)->crColors[m_iFG];
	}
	else cf.crTextColor = m_clrInputFG;

	m_pEntry->SendMsg(EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
}

void CSrmmBaseDialog::onClick_BkColor(CCtrlButton *pButton)
{
	if (!pButton->Enabled())
		return;

	CHARFORMAT2 cf;
	cf.cbSize = sizeof(CHARFORMAT2);
	cf.dwEffects = 0;
	cf.dwMask = CFM_BACKCOLOR;

	if (IsDlgButtonChecked(m_hwnd, pButton->GetCtrlId())) {
		if (!db_get_b(0, CHAT_MODULE, "RightClickFilter", 0)) {
			ShowColorChooser(pButton->GetCtrlId());
			return;
		}
		if (m_bBGSet)
			cf.crBackColor = chatApi.MM_FindModule(m_si->pszModule)->crColors[m_iBG];
	}
	else cf.crBackColor = m_clrInputBG;

	m_pEntry->SendMsg(EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
}

void CSrmmBaseDialog::onClick_BIU(CCtrlButton *pButton)
{
	if (!pButton->Enabled())
		return;

	CHARFORMAT2 cf;
	cf.cbSize = sizeof(CHARFORMAT2);
	cf.dwMask = CFM_BOLD | CFM_ITALIC | CFM_UNDERLINE;
	cf.dwEffects = 0;

	if (IsDlgButtonChecked(m_hwnd, IDC_SRMM_BOLD))
		cf.dwEffects |= CFE_BOLD;
	if (IsDlgButtonChecked(m_hwnd, IDC_SRMM_ITALICS))
		cf.dwEffects |= CFE_ITALIC;
	if (IsDlgButtonChecked(m_hwnd, IDC_SRMM_UNDERLINE))
		cf.dwEffects |= CFE_UNDERLINE;
	m_pEntry->SendMsg(EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
}

/////////////////////////////////////////////////////////////////////////////////////////

bool CSrmmBaseDialog::ProcessHotkeys(int key)
{
	BOOL isCtrl = GetKeyState(VK_CONTROL) & 0x8000;
	BOOL isAlt = GetKeyState(VK_MENU) & 0x8000;
	
	if (key == VK_F4 && isCtrl && !isAlt) { // ctrl-F4 (close tab)
		CloseTab();
		return true;
	}

	if (key == VK_ESCAPE && !isCtrl && !isAlt) { // Esc (close tab)
		CloseTab();
		return true;
	}

	if (key == 0x42 && isCtrl && !isAlt) { // ctrl-b (bold)
		m_btnBold.Push(!m_btnBold.IsPushed());
		onClick_BIU(&m_btnBold);
		return true;
	}

	if (key == 0x49 && isCtrl && !isAlt) { // ctrl-i (italics)
		m_btnItalic.Push(!m_btnItalic.IsPushed());
		onClick_BIU(&m_btnItalic);
		return true;
	}

	if (key == 0x55 && isCtrl && !isAlt) { // ctrl-u (paste clean text)
		m_btnUnderline.Push(!m_btnUnderline.IsPushed());
		onClick_BIU(&m_btnUnderline);
		return true;
	}

	if (key == 0x4b && isCtrl && !isAlt) { // ctrl-k (paste clean text)
		m_btnColor.Push(!m_btnColor.IsPushed());
		onClick_Color(&m_btnColor);
		return true;
	}

	if (key == 0x4c && isCtrl && !isAlt) { // ctrl-l (paste clean text)
		m_btnBkColor.Push(!m_btnBkColor.IsPushed());
		onClick_BkColor(&m_btnBkColor);
		return true;
	}

	if (key == VK_SPACE && isCtrl && !isAlt) { // ctrl-space (paste clean text)
		m_btnBold.Push(false); onClick_BIU(&m_btnBold);
		m_btnItalic.Push(false); onClick_BIU(&m_btnItalic);
		m_btnUnderline.Push(false); onClick_BIU(&m_btnUnderline);

		m_btnColor.Push(false); onClick_Color(&m_btnColor);
		m_btnBkColor.Push(false); onClick_BkColor(&m_btnBkColor);
		return true;
	}

	return false;
}

void CSrmmBaseDialog::RefreshButtonStatus(void)
{
	CHARFORMAT2 cf;
	cf.cbSize = sizeof(CHARFORMAT2);
	cf.dwMask = CFM_BOLD | CFM_ITALIC | CFM_UNDERLINE | CFM_BACKCOLOR | CFM_COLOR;
	SendMessage(m_pEntry->GetHwnd(), EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);

	MODULEINFO *mi = chatApi.MM_FindModule(m_si->pszModule);
	if (mi == nullptr)
		return;

	if (mi->bColor) {
		int index = GetColorIndex(m_si->pszModule, cf.crTextColor);
		bool bState = m_btnColor.IsPushed();

		if (index >= 0) {
			m_bFGSet = true;
			m_iFG = index;
		}

		if (!bState && cf.crTextColor != m_clrInputFG)
			m_btnColor.Push(true);
		else if (bState && cf.crTextColor == m_clrInputFG)
			m_btnColor.Push(false);
	}

	if (mi->bBkgColor) {
		int index = GetColorIndex(m_si->pszModule, cf.crBackColor);
		bool bState = m_btnBkColor.IsPushed();

		if (index >= 0) {
			m_bBGSet = true;
			m_iBG = index;
		}

		if (!bState && cf.crBackColor != m_clrInputBG)
			m_btnBkColor.Push(true);
		else if (bState && cf.crBackColor == m_clrInputBG)
			m_btnBkColor.Push(false);
	}

	if (mi->bBold) {
		bool bState = m_btnBold.IsPushed();
		UINT u2 = cf.dwEffects & CFE_BOLD;
		if (!bState && u2 != 0)
			m_btnBold.Push(true);
		else if (bState && u2 == 0)
			m_btnBold.Push(false);
	}

	if (mi->bItalics) {
		bool bState = m_btnItalic.IsPushed();
		UINT u2 = cf.dwEffects & CFE_ITALIC;
		if (!bState && u2 != 0)
			m_btnItalic.Push(true);
		else if (bState && u2 == 0)
			m_btnItalic.Push(false);
	}

	if (mi->bUnderline) {
		bool bState = m_btnUnderline.IsPushed();
		UINT u2 = cf.dwEffects & CFE_UNDERLINE;
		if (!bState && u2 != 0)
			m_btnUnderline.Push(true);
		else if (bState && u2 == 0)
			m_btnUnderline.Push(false);
	}
}
