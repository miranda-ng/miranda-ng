/*
Chat module plugin for Miranda IM

Copyright 2000-12 Miranda IM, 2012-22 Miranda NG team,
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

/////////////////////////////////////////////////////////////////////////////////////////

void CMsgDialog::LoadSettings()
{
	m_clrInputBG = db_get_dw(0, CHAT_MODULE, "ColorMessageBG", GetSysColor(COLOR_WINDOW));
	LoadMsgDlgFont(MSGFONTID_MESSAGEAREA, nullptr, &m_clrInputFG);
}

/////////////////////////////////////////////////////////////////////////////////////////

void CMsgDialog::ShowFilterMenu()
{
	HWND hwnd = CreateDialogParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_FILTER), m_hwnd, FilterWndProc, (LPARAM)this);
	TranslateDialogDefault(hwnd);

	RECT rc;
	GetWindowRect(m_btnFilter.GetHwnd(), &rc);
	SetWindowPos(hwnd, HWND_TOP, rc.left - 85, (IsWindowVisible(m_btnFilter.GetHwnd()) || IsWindowVisible(m_btnBold.GetHwnd())) ? rc.top - 206 : rc.top - 186, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
}

void CMsgDialog::UpdateNickList()
{
	int i = m_nickList.SendMsg(LB_GETTOPINDEX, 0, 0);
	m_nickList.SendMsg(LB_SETCOUNT, m_si->getUserList().getCount(), 0);
	m_nickList.SendMsg(LB_SETTOPINDEX, i, 0);

	UpdateTitle();
}

void CMsgDialog::UpdateOptions()
{
	m_btnNickList.SendMsg(BM_SETIMAGE, IMAGE_ICON, (LPARAM)g_plugin.getIcon(m_bNicklistEnabled ? IDI_NICKLIST2 : IDI_NICKLIST, FALSE));
	m_btnFilter.SendMsg(BM_SETIMAGE, IMAGE_ICON, (LPARAM)g_plugin.getIcon(m_bFilterEnabled ? IDI_FILTER2 : IDI_FILTER, FALSE));

	HICON hIcon = ImageList_GetIcon(Clist_GetImageList(), GetImageId(), ILD_TRANSPARENT);
	SendMessage(m_pOwner->m_hwndStatus, SB_SETICON, 0, (LPARAM)hIcon);
	DestroyIcon(hIcon);

	Window_SetIcon_IcoLib(m_pOwner->GetHwnd(), g_plugin.getIconHandle(IDI_CHANMGR));

	m_pLog->UpdateOptions();

	// nicklist
	int ih = Chat_GetTextPixelSize(L"AQGglo", g_Settings.UserListFont, FALSE);
	int ih2 = Chat_GetTextPixelSize(L"AQGglo", g_Settings.UserListHeadingsFont, FALSE);
	int height = db_get_b(0, CHAT_MODULE, "NicklistRowDist", 12);
	int font = ih > ih2 ? ih : ih2;

	// make sure we have space for icon!
	if (g_Settings.bShowContactStatus)
		font = font > 16 ? font : 16;

	m_nickList.SendMsg(LB_SETITEMHEIGHT, 0, height > font ? height : font);
	InvalidateRect(m_nickList.GetHwnd(), nullptr, TRUE);

	CSuper::UpdateOptions();
}

void CMsgDialog::UpdateStatusBar()
{
	wchar_t *ptszDispName = m_si->pMI->ptszModDispName;
	int x = 12;
	x += Chat_GetTextPixelSize(ptszDispName, (HFONT)SendMessage(m_pOwner->m_hwndStatus, WM_GETFONT, 0, 0), TRUE);
	x += GetSystemMetrics(SM_CXSMICON);
	int iStatusbarParts[2] = { x, -1 };
	SendMessage(m_pOwner->m_hwndStatus, SB_SETPARTS, 2, (LPARAM)&iStatusbarParts);

	HICON hIcon = ImageList_GetIcon(Clist_GetImageList(), GetImageId(), ILD_TRANSPARENT);
	SendMessage(m_pOwner->m_hwndStatus, SB_SETICON, 0, (LPARAM)hIcon);
	DestroyIcon(hIcon);

	SendMessage(m_pOwner->m_hwndStatus, SB_SETTEXT, 0, (LPARAM)ptszDispName);
	SendMessage(m_pOwner->m_hwndStatus, SB_SETTEXT, 1, (LPARAM)(m_si->ptszStatusbarText ? m_si->ptszStatusbarText : L""));
	SendMessage(m_pOwner->m_hwndStatus, SB_SETTIPTEXT, 1, (LPARAM)(m_si->ptszStatusbarText ? m_si->ptszStatusbarText : L""));
}

/////////////////////////////////////////////////////////////////////////////////////////

void CLogWindow::LogEvents(LOGINFO *lin, bool bRedraw)
{
	auto *si = m_pDlg.m_si;
	if (lin == nullptr || si == nullptr)
		return;

	if (!bRedraw && si->iType == GCW_CHATROOM && m_pDlg.m_bFilterEnabled && (m_pDlg.m_iLogFilterFlags & lin->iType) == 0)
		return;

	LOGSTREAMDATA streamData;
	memset(&streamData, 0, sizeof(streamData));
	streamData.hwnd = m_rtf.GetHwnd();
	streamData.si = si;
	streamData.lin = lin;
	streamData.bStripFormat = FALSE;

	bool bFlag = false;

	EDITSTREAM stream = {};
	stream.pfnCallback = Srmm_LogStreamCallback;
	stream.dwCookie = (DWORD_PTR)& streamData;

	SCROLLINFO scroll;
	scroll.cbSize = sizeof(SCROLLINFO);
	scroll.fMask = SIF_RANGE | SIF_POS | SIF_PAGE;
	GetScrollInfo(m_rtf.GetHwnd(), SB_VERT, &scroll);

	POINT point = {};
	m_rtf.SendMsg(EM_GETSCROLLPOS, 0, (LPARAM)&point);

	// do not scroll to bottom if there is a selection
	CHARRANGE oldsel, sel;
	m_rtf.SendMsg(EM_EXGETSEL, 0, (LPARAM)&oldsel);
	if (oldsel.cpMax != oldsel.cpMin)
		m_rtf.SetDraw(false);

	//set the insertion point at the bottom
	sel.cpMin = sel.cpMax = m_rtf.GetRichTextLength();
	m_rtf.SendMsg(EM_EXSETSEL, 0, (LPARAM)&sel);

	// fix for the indent... must be a M$ bug
	if (sel.cpMax == 0)
		bRedraw = TRUE;

	// should the event(s) be appended to the current log
	WPARAM wp = bRedraw ? SF_RTF : SFF_SELECTION | SF_RTF;

	//get the number of pixels per logical inch
	if (bRedraw) {
		HDC hdc = GetDC(nullptr);
		g_chatApi.logPixelSY = GetDeviceCaps(hdc, LOGPIXELSY);
		g_chatApi.logPixelSX = GetDeviceCaps(hdc, LOGPIXELSX);
		ReleaseDC(nullptr, hdc);
		m_rtf.SetDraw(false);
		bFlag = true;
	}

	// stream in the event(s)
	streamData.lin = lin;
	streamData.bRedraw = bRedraw;
	m_rtf.SendMsg(EM_STREAMIN, wp, (LPARAM)&stream);

	// do smileys
	if (g_plugin.bSmileyInstalled && (bRedraw || (lin->ptszText && lin->iType != GC_EVENT_JOIN && lin->iType != GC_EVENT_NICK && lin->iType != GC_EVENT_ADDSTATUS && lin->iType != GC_EVENT_REMOVESTATUS))) {
		CHARRANGE newsel;
		newsel.cpMax = -1;
		newsel.cpMin = sel.cpMin;
		if (newsel.cpMin < 0)
			newsel.cpMin = 0;

		SMADD_RICHEDIT3 sm = {};
		sm.cbSize = sizeof(sm);
		sm.hwndRichEditControl = m_rtf.GetHwnd();
		sm.Protocolname = si->pszModule;
		sm.rangeToReplace = bRedraw ? nullptr : &newsel;
		sm.disableRedraw = TRUE;
		sm.hContact = si->hContact;
		CallService(MS_SMILEYADD_REPLACESMILEYS, 0, (LPARAM)&sm);
	}

	// scroll log to bottom if the log was previously scrolled to bottom, else restore old position
	if (bRedraw || (UINT)scroll.nPos >= (UINT)scroll.nMax - scroll.nPage - 5 || scroll.nMax - scroll.nMin - scroll.nPage < 50)
		ScrollToBottom();
	else
		m_rtf.SendMsg(EM_SETSCROLLPOS, 0, (LPARAM)&point);

	// do we need to restore the selection
	if (oldsel.cpMax != oldsel.cpMin) {
		m_rtf.SendMsg(EM_EXSETSEL, 0, (LPARAM)&oldsel);
		m_rtf.SetDraw(true);
		InvalidateRect(m_rtf.GetHwnd(), nullptr, TRUE);
	}

	// need to invalidate the window
	if (bFlag) {
		sel.cpMin = sel.cpMax = m_rtf.GetRichTextLength();
		m_rtf.SendMsg(EM_EXSETSEL, 0, (LPARAM)&sel);
		m_rtf.SetDraw(true);
		InvalidateRect(m_rtf.GetHwnd(), nullptr, TRUE);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CALLBACK CMsgDialog::FilterWndProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static CMsgDialog *pDlg = nullptr;
	switch (uMsg) {
	case WM_INITDIALOG:
		pDlg = (CMsgDialog*)lParam;
		CheckDlgButton(hwndDlg, IDC_1, pDlg->m_iLogFilterFlags & GC_EVENT_ACTION ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_2, pDlg->m_iLogFilterFlags & GC_EVENT_MESSAGE ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_3, pDlg->m_iLogFilterFlags & GC_EVENT_NICK ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_4, pDlg->m_iLogFilterFlags & GC_EVENT_JOIN ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_5, pDlg->m_iLogFilterFlags & GC_EVENT_PART ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_6, pDlg->m_iLogFilterFlags & GC_EVENT_TOPIC ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_7, pDlg->m_iLogFilterFlags & GC_EVENT_ADDSTATUS ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_8, pDlg->m_iLogFilterFlags & GC_EVENT_INFORMATION ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_9, pDlg->m_iLogFilterFlags & GC_EVENT_QUIT ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_10, pDlg->m_iLogFilterFlags & GC_EVENT_KICK ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_11, pDlg->m_iLogFilterFlags & GC_EVENT_NOTICE ? BST_CHECKED : BST_UNCHECKED);
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
