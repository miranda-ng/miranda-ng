/*
Chat module plugin for Miranda IM

Copyright (C) 2003 Jörgen Persson
Copyright 2003-2009 Miranda ICQ/IM project,

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

void CChatRoomDlg::TabAutoComplete()
{
	LRESULT lResult = (LRESULT)m_message.SendMsg(EM_GETSEL, 0, 0);
	int start = LOWORD(lResult), end = HIWORD(lResult);
	m_message.SendMsg(EM_SETSEL, end, end);

	int iLen = m_message.GetRichTextLength(1200);
	if (iLen <= 0)
		return;

	bool isTopic = false, isRoom = false;
	wchar_t *pszName = nullptr;
	wchar_t* pszText = (wchar_t*)mir_alloc(iLen + 100 * sizeof(wchar_t));

	GETTEXTEX gt = {};
	gt.codepage = 1200;
	gt.cb = iLen + 99 * sizeof(wchar_t);
	gt.flags = GT_DEFAULT;
	m_message.SendMsg(EM_GETTEXTEX, (WPARAM)&gt, (LPARAM)pszText);

	if (start > 1 && pszText[start - 1] == ' ' && pszText[start - 2] == ':')
		start -= 2;

	if (m_wszSearchResult != nullptr) {
		int cbResult = (int)mir_wstrlen(m_wszSearchResult);
		if (start >= cbResult && !wcsnicmp(m_wszSearchResult, pszText + start - cbResult, cbResult)) {
			start -= cbResult;
			goto LBL_SkipEnd;
		}
	}

	while (start > 0 && pszText[start - 1] != ' ' && pszText[start - 1] != 13 && pszText[start - 1] != VK_TAB)
		start--;

LBL_SkipEnd:
	while (end < iLen && pszText[end] != ' ' && pszText[end] != 13 && pszText[end - 1] != VK_TAB)
		end++;

	if (pszText[start] == '#')
		isRoom = true;
	else {
		int topicStart = start;
		while (topicStart >0 && (pszText[topicStart - 1] == ' ' || pszText[topicStart - 1] == 13 || pszText[topicStart - 1] == VK_TAB))
			topicStart--;
		if (topicStart > 5 && wcsstr(&pszText[topicStart - 6], L"/topic") == &pszText[topicStart - 6])
			isTopic = true;
	}

	if (m_wszSearchQuery == nullptr) {
		m_wszSearchQuery = (wchar_t*)mir_alloc(sizeof(wchar_t)*(end - start + 1));
		mir_wstrncpy(m_wszSearchQuery, pszText + start, end - start + 1);
		m_wszSearchResult = mir_wstrdup(m_wszSearchQuery);
		m_pLastSession = nullptr;
	}

	if (isTopic)
		pszName = m_si->ptszTopic;
	else if (isRoom) {
		m_pLastSession = SM_FindSessionAutoComplete(m_si->pszModule, m_si, m_pLastSession, m_wszSearchQuery, m_wszSearchResult);
		if (m_pLastSession != nullptr)
			pszName = m_pLastSession->ptszName;
	}
	else pszName = pci->UM_FindUserAutoComplete(m_si->pUsers, m_wszSearchQuery, m_wszSearchResult);

	mir_free(pszText);
	replaceStrW(m_wszSearchResult, nullptr);

	if (pszName == nullptr) {
		if (end != start) {
			m_message.SendMsg(EM_SETSEL, start, end);
			m_message.SendMsg(EM_REPLACESEL, FALSE, (LPARAM)m_wszSearchQuery);
		}
		replaceStrW(m_wszSearchQuery, nullptr);
	}
	else {
		m_wszSearchResult = mir_wstrdup(pszName);
		if (end != start) {
			ptrW szReplace;
			if (!isRoom && !isTopic && g_Settings.bAddColonToAutoComplete && start == 0) {
				szReplace = (wchar_t*)mir_alloc((mir_wstrlen(pszName) + 4) * sizeof(wchar_t));
				mir_wstrcpy(szReplace, pszName);
				mir_wstrcat(szReplace, L": ");
				pszName = szReplace;
			}
			m_message.SendMsg(EM_SETSEL, start, end);
			m_message.SendMsg(EM_REPLACESEL, FALSE, (LPARAM)pszName);
		}
	}
}

void CChatRoomDlg::FixTabIcons()
{
	HICON hIcon;
	if (!(m_si->wState & GC_EVENT_HIGHLIGHT)) {
		if (m_si->wState & STATE_TALK)
			hIcon = (m_si->wStatus == ID_STATUS_ONLINE) ? pci->MM_FindModule(m_si->pszModule)->hOnlineTalkIcon : pci->MM_FindModule(m_si->pszModule)->hOfflineTalkIcon;
		else
			hIcon = (m_si->wStatus == ID_STATUS_ONLINE) ? pci->MM_FindModule(m_si->pszModule)->hOnlineIcon : pci->MM_FindModule(m_si->pszModule)->hOfflineIcon;
	}
	else hIcon = g_dat.hMsgIcon;

	TabControlData tcd = {};
	tcd.iFlags = TCDF_ICON;
	tcd.hIcon = hIcon;
	SendMessage(m_hwndParent, CM_UPDATETABCONTROL, (WPARAM)&tcd, (LPARAM)m_hwnd);
}

/////////////////////////////////////////////////////////////////////////////////////////

void CChatRoomDlg::MessageDialogResize(int w, int h)
{
	bool bNick = m_si->iType != GCW_SERVER && m_bNicklistEnabled;
	bool bToolbar = SendMessage(m_hwndParent, CM_GETTOOLBARSTATUS, 0, 0) != 0;
	int  hSplitterMinTop = TOOLBAR_HEIGHT + m_minLogBoxHeight, hSplitterMinBottom = m_minEditBoxHeight;
	int  toolbarHeight = bToolbar ? TOOLBAR_HEIGHT : 0;

	if (h - m_pParent->iSplitterY < hSplitterMinTop)
		m_pParent->iSplitterY = h - hSplitterMinTop;
	if (m_pParent->iSplitterY < hSplitterMinBottom)
		m_pParent->iSplitterY = hSplitterMinBottom;

	m_splitterX.Show(bNick);
	if (m_si->iType != GCW_SERVER)
		m_nickList.Show(m_bNicklistEnabled);
	else
		m_nickList.Hide();

	if (m_si->iType == GCW_SERVER) {
		m_btnNickList.Enable(false);
		m_btnFilter.Enable(false);
		m_btnChannelMgr.Enable(false);
	}
	else {
		m_btnNickList.Enable(true);
		m_btnFilter.Enable(true);
		if (m_si->iType == GCW_CHATROOM)
			m_btnChannelMgr.Enable(pci->MM_FindModule(m_si->pszModule)->bChanMgr);
	}

	int toolbarTopY = bToolbar ? h - m_pParent->iSplitterY - toolbarHeight : h - m_pParent->iSplitterY;
	int logBottom = (m_hwndIeview != nullptr) ? toolbarTopY / 2 : toolbarTopY;

	HDWP hdwp = BeginDeferWindowPos(5);
	hdwp = DeferWindowPos(hdwp, m_log.GetHwnd(), 0, 1, 0, bNick ? w - m_pParent->iSplitterX - 1 : w - 2, logBottom, SWP_NOZORDER);
	hdwp = DeferWindowPos(hdwp, m_nickList.GetHwnd(), 0, w - m_pParent->iSplitterX + 2, 0, m_pParent->iSplitterX - 3, toolbarTopY, SWP_NOZORDER);
	hdwp = DeferWindowPos(hdwp, m_splitterX.GetHwnd(), 0, w - m_pParent->iSplitterX, 1, 2, toolbarTopY - 1, SWP_NOZORDER);
	hdwp = DeferWindowPos(hdwp, m_splitterY.GetHwnd(), 0, 0, h - m_pParent->iSplitterY, w, SPLITTER_HEIGHT, SWP_NOZORDER);
	hdwp = DeferWindowPos(hdwp, m_message.GetHwnd(), 0, 1, h - m_pParent->iSplitterY + SPLITTER_HEIGHT, w - 2, m_pParent->iSplitterY - SPLITTER_HEIGHT - 1, SWP_NOZORDER);
	EndDeferWindowPos(hdwp);

	SetButtonsPos(m_hwnd, m_hContact, bToolbar);

	if (m_hwndIeview != nullptr) {
		IEVIEWWINDOW ieWindow;
		ieWindow.cbSize = sizeof(IEVIEWWINDOW);
		ieWindow.iType = IEW_SETPOS;
		ieWindow.parent = m_hwnd;
		ieWindow.hwnd = m_hwndIeview;
		ieWindow.x = 0;
		ieWindow.y = logBottom + 1;
		ieWindow.cx = bNick ? w - m_pParent->iSplitterX : w;
		ieWindow.cy = logBottom;
		CallService(MS_IEVIEW_WINDOW, 0, (LPARAM)&ieWindow);
	}
	else RedrawWindow(m_log.GetHwnd(), nullptr, nullptr, RDW_INVALIDATE);

	RedrawWindow(m_nickList.GetHwnd(), nullptr, nullptr, RDW_INVALIDATE);
	RedrawWindow(m_message.GetHwnd(), nullptr, nullptr, RDW_INVALIDATE);
}

CChatRoomDlg::CChatRoomDlg(SESSION_INFO *si)
	: CScriverWindow(IDD_CHANNEL, si),
	m_splitterX(this, IDC_SPLITTERX),
	m_splitterY(this, IDC_SPLITTERY),
	m_btnOk(this, IDOK)
{
	m_btnOk.OnClick = Callback(this, &CChatRoomDlg::onClick_Ok);
	m_btnFilter.OnClick = Callback(this, &CChatRoomDlg::onClick_Filter);
	m_btnNickList.OnClick = Callback(this, &CChatRoomDlg::onClick_ShowList);

	m_message.OnChange = Callback(this, &CChatRoomDlg::onChange_Message);
	
	m_splitterX.OnChange = Callback(this, &CChatRoomDlg::OnSplitterX);
	m_splitterY.OnChange = Callback(this, &CChatRoomDlg::OnSplitterY);
}

void CChatRoomDlg::OnInitDialog()
{
	CSuper::OnInitDialog();
	m_si->pDlg = this;

	NotifyEvent(MSG_WINDOW_EVT_OPENING);

	m_pParent = (ParentWindowData *)GetWindowLongPtr(m_hwndParent, GWLP_USERDATA);

	RECT rc;
	GetWindowRect(m_message.GetHwnd(), &rc);
	m_minLogBoxHeight = m_minEditBoxHeight = rc.bottom - rc.top;
	if (m_pParent->iSplitterY == -1)
		m_pParent->iSplitterY = m_minEditBoxHeight;

	if (m_pParent->iSplitterX == -1) {
		GetWindowRect(m_nickList.GetHwnd(), &rc);
		m_pParent->iSplitterX = rc.right - rc.left;
	}

	m_message.SendMsg(EM_SUBCLASSED, 0, 0);
	m_message.SendMsg(EM_SETEVENTMASK, 0, ENM_MOUSEEVENTS | ENM_KEYEVENTS | ENM_CHANGE | ENM_REQUESTRESIZE);

	int mask = (int)m_log.SendMsg(EM_GETEVENTMASK, 0, 0);
	m_log.SendMsg(EM_SETEVENTMASK, 0, mask | ENM_LINK | ENM_MOUSEEVENTS);
	m_log.SendMsg(EM_LIMITTEXT, sizeof(wchar_t) * 0x7FFFFFFF, 0);
	m_log.SendMsg(EM_AUTOURLDETECT, 1, 0);

	if (db_get_b(0, CHAT_MODULE, "UseIEView", 0)) {
		IEVIEWWINDOW ieWindow = { sizeof(ieWindow) };
		ieWindow.iType = IEW_CREATE;
		ieWindow.dwMode = IEWM_CHAT;
		ieWindow.parent = m_hwnd;
		ieWindow.cx = 200;
		ieWindow.cy = 300;
		CallService(MS_IEVIEW_WINDOW, 0, (LPARAM)&ieWindow);

		m_hwndIeview = ieWindow.hwnd;

		IEVIEWEVENT iee = { sizeof(iee) };
		iee.iType = IEE_CLEAR_LOG;
		iee.hwnd = m_hwndIeview;
		iee.hContact = m_hContact;
		iee.pszProto = m_si->pszModule;
		CallService(MS_IEVIEW_EVENT, 0, (LPARAM)&iee);
	}

	m_log.SendMsg(EM_HIDESELECTION, TRUE, 0);

	UpdateOptions();
	UpdateStatusBar();
	UpdateTitle();

	SendMessage(m_hwndParent, CM_ADDCHILD, (WPARAM)this, 0);
	UpdateNickList();
	NotifyEvent(MSG_WINDOW_EVT_OPEN);
}

void CChatRoomDlg::OnDestroy()
{
	NotifyEvent(MSG_WINDOW_EVT_CLOSING);

	m_si->pDlg = nullptr;

	SendMessage(m_hwndParent, CM_REMOVECHILD, 0, (LPARAM)m_hwnd);
	if (m_hwndIeview != nullptr) {
		IEVIEWWINDOW ieWindow;
		ieWindow.cbSize = sizeof(IEVIEWWINDOW);
		ieWindow.iType = IEW_DESTROY;
		ieWindow.hwnd = m_hwndIeview;
		CallService(MS_IEVIEW_WINDOW, 0, (LPARAM)&ieWindow);
	}

	NotifyEvent(MSG_WINDOW_EVT_CLOSE);

	CSuper::OnDestroy();
}

void CChatRoomDlg::OnSplitterX(CSplitter *pSplitter)
{
	RECT rc;
	GetClientRect(m_hwnd, &rc);

	m_pParent->iSplitterX = rc.right - pSplitter->GetPos() + 1;
	if (m_pParent->iSplitterX < 35)
		m_pParent->iSplitterX = 35;
	if (m_pParent->iSplitterX > rc.right - rc.left - 35)
		m_pParent->iSplitterX = rc.right - rc.left - 35;
}

void CChatRoomDlg::OnSplitterY(CSplitter *pSplitter)
{
	RECT rc;
	GetClientRect(m_hwnd, &rc);
	m_pParent->iSplitterY = rc.bottom - pSplitter->GetPos();
}

void CChatRoomDlg::onClick_Ok(CCtrlButton *pButton)
{
	if (!pButton->Enabled())
		return;

	MODULEINFO *mi = pci->MM_FindModule(m_si->pszModule);
	if (mi == nullptr)
		return;

	char *pszRtf = m_message.GetRichTextRtf();
	if (pszRtf == nullptr)
		return;

	TCmdList *cmdListNew = tcmdlist_last(cmdList);
	while (cmdListNew != nullptr && cmdListNew->temporary) {
		cmdList = tcmdlist_remove(cmdList, cmdListNew);
		cmdListNew = tcmdlist_last(cmdList);
	}

	// takes pszRtf to a queue, no leak here
	cmdList = tcmdlist_append(cmdList, pszRtf, 20, FALSE);

	CMStringW ptszText(ptrW(mir_utf8decodeW(pszRtf)));
	pci->DoRtfToTags(ptszText, 0, nullptr);
	ptszText.Trim();
	ptszText.Replace(L"%", L"%%");

	if (mi->bAckMsg) {
		EnableWindow(m_message.GetHwnd(), FALSE);
		m_message.SendMsg(EM_SETREADONLY, TRUE, 0);
	}
	else m_message.SetText(L"");

	EnableWindow(m_btnOk.GetHwnd(), FALSE);

	Chat_DoEventHook(m_si, GC_USER_MESSAGE, nullptr, ptszText, 0);
	SetFocus(m_message.GetHwnd());
}

void CChatRoomDlg::onClick_ShowList(CCtrlButton *pButton)
{
	if (!pButton->Enabled() || m_si->iType == GCW_SERVER)
		return;

	m_bNicklistEnabled = !m_bNicklistEnabled;
	pButton->SendMsg(BM_SETIMAGE, IMAGE_ICON, (LPARAM)GetCachedIcon(m_bNicklistEnabled ? "chat_nicklist" : "chat_nicklist2"));
	ScrollToBottom();
	Resize();
}

void CChatRoomDlg::onClick_Filter(CCtrlButton *pButton)
{
	if (!pButton->Enabled())
		return;

	m_bFilterEnabled = !m_bFilterEnabled;
	pButton->SendMsg(BM_SETIMAGE, IMAGE_ICON, (LPARAM)GetCachedIcon(m_bFilterEnabled ? "chat_filter" : "chat_filter2"));
	if (m_bFilterEnabled && db_get_b(0, CHAT_MODULE, "RightClickFilter", 0) == 0)
		ShowFilterMenu();
	else
		RedrawLog();
}

void CChatRoomDlg::onChange_Message(CCtrlEdit*)
{
	cmdListCurrent = nullptr;
	m_btnOk.Enable(m_message.GetRichTextLength() != 0);
}

/////////////////////////////////////////////////////////////////////////////////////////

static void __cdecl phase2(void *lParam)
{
	Thread_SetName("Scriver: phase2");

	SESSION_INFO *si = (SESSION_INFO*)lParam;
	Sleep(30);
	if (si && si->pDlg)
		si->pDlg->RedrawLog2();
}

void CChatRoomDlg::RedrawLog()
{
	m_si->LastTime = 0;
	if (m_si->pLog) {
		LOGINFO *pLog = m_si->pLog;
		if (m_si->iEventCount > 60) {
			int index = 0;
			while (index < 59) {
				if (pLog->next == nullptr)
					break;

				pLog = pLog->next;
				if ((m_si->iType != GCW_CHATROOM && m_si->iType != GCW_PRIVMESS) || !m_bFilterEnabled || (m_iLogFilterFlags & pLog->iType) != 0)
					index++;
			}
			StreamInEvents(pLog, true);
			mir_forkthread(phase2, m_si);
		}
		else StreamInEvents(m_si->pLogEnd, true);
	}
	else ClearLog();
}

void CChatRoomDlg::ScrollToBottom()
{
	if (GetWindowLongPtr(m_log.GetHwnd(), GWL_STYLE) & WS_VSCROLL) {
		SCROLLINFO si = { sizeof(si) };
		si.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;
		if (GetScrollInfo(m_log.GetHwnd(), SB_VERT, &si)) {
			si.fMask = SIF_POS;
			si.nPos = si.nMax - si.nPage + 1;
			SetScrollInfo(m_log.GetHwnd(), SB_VERT, &si, TRUE);

			PostMessage(m_log.GetHwnd(), WM_VSCROLL, MAKEWPARAM(SB_BOTTOM, 0), 0);
		}
	}
}

void CChatRoomDlg::ShowFilterMenu()
{
	HWND hwnd = CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_FILTER), m_hwnd, FilterWndProc, (LPARAM)this);
	TranslateDialogDefault(hwnd);

	RECT rc;
	GetWindowRect(m_btnFilter.GetHwnd(), &rc);
	SetWindowPos(hwnd, HWND_TOP, rc.left - 85, (IsWindowVisible(m_btnFilter.GetHwnd()) || IsWindowVisible(m_btnBold.GetHwnd())) ? rc.top - 206 : rc.top - 186, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
}

void CChatRoomDlg::UpdateNickList()
{
	m_nickList.SendMsg(WM_SETREDRAW, FALSE, 0);
	m_nickList.SendMsg(LB_RESETCONTENT, 0, 0);
	for (int index = 0; index < m_si->nUsersInNicklist; index++) {
		USERINFO *ui = pci->SM_GetUserFromIndex(m_si->ptszID, m_si->pszModule, index);
		if (ui) {
			char szIndicator = SM_GetStatusIndicator(m_si, ui);
			if (szIndicator > '\0') {
				static wchar_t ptszBuf[128];
				mir_snwprintf(ptszBuf, L"%c%s", szIndicator, ui->pszNick);
				m_nickList.SendMsg(LB_ADDSTRING, 0, (LPARAM)ptszBuf);
			}
			else m_nickList.SendMsg(LB_ADDSTRING, 0, (LPARAM)ui->pszNick);
		}
	}
	m_nickList.SendMsg(WM_SETREDRAW, TRUE, 0);
	InvalidateRect(m_nickList.GetHwnd(), nullptr, FALSE);
	UpdateWindow(m_nickList.GetHwnd());
	UpdateTitle();
}

void CChatRoomDlg::UpdateOptions()
{
	m_btnNickList.SendMsg(BM_SETIMAGE, IMAGE_ICON, (LPARAM)GetCachedIcon(m_bNicklistEnabled ? "chat_nicklist" : "chat_nicklist2"));
	m_btnFilter.SendMsg(BM_SETIMAGE, IMAGE_ICON, (LPARAM)GetCachedIcon(m_bFilterEnabled ? "chat_filter" : "chat_filter2"));
	{
		MODULEINFO *pInfo = pci->MM_FindModule(m_si->pszModule);
		if (pInfo) {
			m_btnBold.Enable(pInfo->bBold);
			m_btnItalic.Enable(pInfo->bItalics);
			m_btnUnderline.Enable(pInfo->bUnderline);
			m_btnColor.Enable(pInfo->bColor);
			m_btnBkColor.Enable(pInfo->bBkgColor);
			if (m_si->iType == GCW_CHATROOM)
				m_btnChannelMgr.Enable(pInfo->bChanMgr);
		}
	}

	UpdateStatusBar();
	UpdateTitle();
	FixTabIcons();

	m_log.SendMsg(EM_SETBKGNDCOLOR, 0, g_Settings.crLogBackground);

	// messagebox
	COLORREF crFore;
	LoadMsgDlgFont(MSGFONTID_MESSAGEAREA, nullptr, &crFore);

	CHARFORMAT2 cf;
	cf.cbSize = sizeof(CHARFORMAT2);
	cf.dwMask = CFM_COLOR | CFM_BOLD | CFM_UNDERLINE | CFM_BACKCOLOR;
	cf.dwEffects = 0;
	cf.crTextColor = crFore;
	cf.crBackColor = db_get_dw(0, SRMM_MODULE, SRMSGSET_INPUTBKGCOLOUR, SRMSGDEFSET_INPUTBKGCOLOUR);
	m_message.SendMsg(EM_SETBKGNDCOLOR, 0, db_get_dw(0, SRMM_MODULE, SRMSGSET_INPUTBKGCOLOUR, SRMSGDEFSET_INPUTBKGCOLOUR));
	m_message.SendMsg(WM_SETFONT, (WPARAM)g_Settings.MessageBoxFont, MAKELPARAM(TRUE, 0));
	m_message.SendMsg(EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf);
	{
		// nicklist
		int ih = Chat_GetTextPixelSize(L"AQG_glo'", g_Settings.UserListFont, false);
		int ih2 = Chat_GetTextPixelSize(L"AQG_glo'", g_Settings.UserListHeadingsFont, false);
		int height = db_get_b(0, CHAT_MODULE, "NicklistRowDist", 12);
		int font = ih > ih2 ? ih : ih2;
		// make sure we have space for icon!
		if (db_get_b(0, CHAT_MODULE, "ShowContactStatus", 0))
			font = font > 16 ? font : 16;

		m_nickList.SendMsg(LB_SETITEMHEIGHT, 0, height > font ? height : font);
		InvalidateRect(m_nickList.GetHwnd(), nullptr, TRUE);
	}
	m_message.SendMsg(EM_REQUESTRESIZE, 0, 0);
	Resize();
	RedrawLog2();
}

void CChatRoomDlg::UpdateStatusBar()
{
	MODULEINFO *mi = pci->MM_FindModule(m_si->pszModule);
	wchar_t szTemp[512];
	mir_snwprintf(szTemp, L"%s : %s", mi->ptszModDispName, m_si->ptszStatusbarText ? m_si->ptszStatusbarText : L"");

	StatusBarData sbd;
	sbd.iItem = 0;
	sbd.iFlags = SBDF_TEXT | SBDF_ICON;
	sbd.hIcon = m_si->wStatus == ID_STATUS_ONLINE ? mi->hOnlineIcon : mi->hOfflineIcon;
	sbd.pszText = szTemp;
	SendMessage(m_hwndParent, CM_UPDATESTATUSBAR, (WPARAM)&sbd, (LPARAM)m_hwnd);

	sbd.iItem = 1;
	sbd.hIcon = nullptr;
	sbd.pszText = L"";
	SendMessage(m_hwndParent, CM_UPDATESTATUSBAR, (WPARAM)&sbd, (LPARAM)m_hwnd);

	StatusIconData sid = {};
	sid.szModule = SRMM_MODULE;
	Srmm_ModifyIcon(m_hContact, &sid);
}

void CChatRoomDlg::UpdateTitle()
{
	TitleBarData tbd = {};
	if (g_dat.flags & SMF_STATUSICON) {
		MODULEINFO *mi = pci->MM_FindModule(m_si->pszModule);
		tbd.hIcon = (m_si->wStatus == ID_STATUS_ONLINE) ? mi->hOnlineIcon : mi->hOfflineIcon;
		tbd.hIconBig = (m_si->wStatus == ID_STATUS_ONLINE) ? mi->hOnlineIconBig : mi->hOfflineIconBig;
	}
	else {
		tbd.hIcon = GetCachedIcon("chat_window");
		tbd.hIconBig = g_dat.hIconChatBig;
	}
	tbd.hIconNot = (m_si->wState & (GC_EVENT_HIGHLIGHT | STATE_TALK)) ? GetCachedIcon("chat_overlay") : nullptr;

	wchar_t szTemp[512];
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
	tbd.iFlags = TBDF_TEXT | TBDF_ICON;
	tbd.pszText = szTemp;
	SendMessage(m_hwndParent, CM_UPDATETITLEBAR, (WPARAM)&tbd, (LPARAM)m_hwnd);
	SendMessage(m_hwnd, DM_UPDATETABCONTROL, 0, 0);
}

/////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CALLBACK CChatRoomDlg::FilterWndProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static CChatRoomDlg *pDlg = nullptr;
	switch (uMsg) {
	case WM_INITDIALOG:
		pDlg = (CChatRoomDlg*)lParam;
		CheckDlgButton(hwndDlg, IDC_CHAT_1, pDlg->m_iLogFilterFlags & GC_EVENT_ACTION ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHAT_2, pDlg->m_iLogFilterFlags & GC_EVENT_MESSAGE ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHAT_3, pDlg->m_iLogFilterFlags & GC_EVENT_NICK ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHAT_4, pDlg->m_iLogFilterFlags & GC_EVENT_JOIN ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHAT_5, pDlg->m_iLogFilterFlags & GC_EVENT_PART ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHAT_6, pDlg->m_iLogFilterFlags & GC_EVENT_TOPIC ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHAT_7, pDlg->m_iLogFilterFlags & GC_EVENT_ADDSTATUS ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHAT_8, pDlg->m_iLogFilterFlags & GC_EVENT_INFORMATION ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHAT_9, pDlg->m_iLogFilterFlags & GC_EVENT_QUIT ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHAT_10, pDlg->m_iLogFilterFlags & GC_EVENT_KICK ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHAT_11, pDlg->m_iLogFilterFlags & GC_EVENT_NOTICE ? BST_CHECKED : BST_UNCHECKED);
		break;

	case WM_CTLCOLOREDIT:
	case WM_CTLCOLORSTATIC:
		SetTextColor((HDC)wParam, RGB(60, 60, 150));
		SetBkColor((HDC)wParam, GetSysColor(COLOR_WINDOW));
		return (INT_PTR)GetSysColorBrush(COLOR_WINDOW);

	case WM_ACTIVATE:
		if (LOWORD(wParam) == WA_INACTIVE) {
			int iFlags = 0;

			if (IsDlgButtonChecked(hwndDlg, IDC_CHAT_1) == BST_CHECKED)
				iFlags |= GC_EVENT_ACTION;
			if (IsDlgButtonChecked(hwndDlg, IDC_CHAT_2) == BST_CHECKED)
				iFlags |= GC_EVENT_MESSAGE;
			if (IsDlgButtonChecked(hwndDlg, IDC_CHAT_3) == BST_CHECKED)
				iFlags |= GC_EVENT_NICK;
			if (IsDlgButtonChecked(hwndDlg, IDC_CHAT_4) == BST_CHECKED)
				iFlags |= GC_EVENT_JOIN;
			if (IsDlgButtonChecked(hwndDlg, IDC_CHAT_5) == BST_CHECKED)
				iFlags |= GC_EVENT_PART;
			if (IsDlgButtonChecked(hwndDlg, IDC_CHAT_6) == BST_CHECKED)
				iFlags |= GC_EVENT_TOPIC;
			if (IsDlgButtonChecked(hwndDlg, IDC_CHAT_7) == BST_CHECKED)
				iFlags |= GC_EVENT_ADDSTATUS;
			if (IsDlgButtonChecked(hwndDlg, IDC_CHAT_8) == BST_CHECKED)
				iFlags |= GC_EVENT_INFORMATION;
			if (IsDlgButtonChecked(hwndDlg, IDC_CHAT_9) == BST_CHECKED)
				iFlags |= GC_EVENT_QUIT;
			if (IsDlgButtonChecked(hwndDlg, IDC_CHAT_10) == BST_CHECKED)
				iFlags |= GC_EVENT_KICK;
			if (IsDlgButtonChecked(hwndDlg, IDC_CHAT_11) == BST_CHECKED)
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

/////////////////////////////////////////////////////////////////////////////////////////

LRESULT CChatRoomDlg::WndProc_Message(UINT msg, WPARAM wParam, LPARAM lParam)
{
	bool isShift = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
	bool isCtrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
	bool isAlt = (GetKeyState(VK_MENU) & 0x8000) != 0;

	int result = InputAreaShortcuts(m_message.GetHwnd(), msg, wParam, lParam);
	if (result != -1)
		return result;

	switch (msg) {
	case WM_MOUSEWHEEL:
		if ((GetWindowLongPtr(m_message.GetHwnd(), GWL_STYLE) & WS_VSCROLL) == 0)
			m_log.SendMsg(WM_MOUSEWHEEL, wParam, lParam);

		m_iLastEnterTime = 0;
		return TRUE;

	case EM_REPLACESEL:
		PostMessage(m_message.GetHwnd(), EM_ACTIVATE, 0, 0);
		break;

	case EM_ACTIVATE:
		SetActiveWindow(m_hwnd);
		break;

	case WM_KEYDOWN:
		if (wParam == VK_RETURN) {
			replaceStrW(m_wszSearchQuery, nullptr);
			replaceStrW(m_wszSearchResult, nullptr);
			if ((isCtrl != 0) ^ (0 != db_get_b(0, SRMM_MODULE, SRMSGSET_SENDONENTER, SRMSGDEFSET_SENDONENTER))) {
				PostMessage(m_hwnd, WM_COMMAND, IDOK, 0);
				return 0;
			}
			if (db_get_b(0, SRMM_MODULE, SRMSGSET_SENDONDBLENTER, SRMSGDEFSET_SENDONDBLENTER)) {
				if (m_iLastEnterTime + 2 < time(nullptr))
					m_iLastEnterTime = time(nullptr);
				else {
					m_message.SendMsg(WM_KEYDOWN, VK_BACK, 0);
					m_message.SendMsg(WM_KEYUP, VK_BACK, 0);
					PostMessage(m_hwnd, WM_COMMAND, IDOK, 0);
					return 0;
				}
			}
		}
		else m_iLastEnterTime = 0;

		if (wParam == VK_TAB && isShift && !isCtrl) { // SHIFT-TAB (go to nick list)
			SetFocus(m_nickList.GetHwnd());
			return TRUE;
		}

		if (wParam == VK_TAB && !isCtrl && !isShift) {    //tab-autocomplete
			m_message.SendMsg(WM_SETREDRAW, FALSE, 0);
			TabAutoComplete();
			m_message.SendMsg(WM_SETREDRAW, TRUE, 0);
			RedrawWindow(m_nickList.GetHwnd(), nullptr, nullptr, RDW_INVALIDATE);
			return 0;
		}
		if (wParam != VK_RIGHT && wParam != VK_LEFT) {
			mir_free(m_wszSearchQuery);
			m_wszSearchQuery = nullptr;
			mir_free(m_wszSearchResult);
			m_wszSearchResult = nullptr;
		}

		if (ProcessHotkeys(wParam, isShift, isCtrl, isAlt))
			return TRUE;

		if (wParam == 0x4e && isCtrl && !isAlt) { // ctrl-n (nicklist)
			if (m_btnNickList.Enabled())
				m_btnNickList.Click();
			return TRUE;
		}

		if (wParam == 0x48 && isCtrl && !isAlt) { // ctrl-h (history)
			m_btnHistory.Click();
			return TRUE;
		}

		if (wParam == 0x4f && isCtrl && !isAlt) { // ctrl-o (options)
			if (m_btnChannelMgr.Enabled())
				m_btnChannelMgr.Click();
			return TRUE;
		}

		if (((wParam == VK_INSERT && isShift) || (wParam == 'V' && isCtrl)) && !isAlt) { // ctrl-v (paste clean text)
			m_message.SendMsg(EM_PASTESPECIAL, CF_UNICODETEXT, 0);
			return TRUE;
		}

		if (wParam == VK_NEXT || wParam == VK_PRIOR) {
			m_log.SendMsg(msg, wParam, lParam);
			return TRUE;
		}
		break;

	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_KILLFOCUS:
		m_iLastEnterTime = 0;
		break;

	case WM_CONTEXTMENU:
		InputAreaContextMenu(m_message.GetHwnd(), wParam, lParam, m_hContact);
		return TRUE;

	case WM_KEYUP:
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
		RefreshButtonStatus();
		break;

	case WM_DESTROY:
		mir_free(m_wszSearchQuery);
		mir_free(m_wszSearchResult);
		return 0;
	}

	return CSuper::WndProc_Message(msg, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////////////////

LRESULT CChatRoomDlg::WndProc_Log(UINT msg, WPARAM wParam, LPARAM lParam)
{
	int result = InputAreaShortcuts(m_log.GetHwnd(), msg, wParam, lParam);
	if (result != -1)
		return result;

	CHARRANGE sel;

	switch (msg) {
	case WM_MEASUREITEM:
		MeasureMenuItem(wParam, lParam);
		return TRUE;

	case WM_DRAWITEM:
		return DrawMenuItem(wParam, lParam);

	case WM_LBUTTONUP:
		SendMessage(m_log.GetHwnd(), EM_EXGETSEL, 0, (LPARAM)&sel);
		if (sel.cpMin != sel.cpMax) {
			SendMessage(m_log.GetHwnd(), WM_COPY, 0, 0);
			sel.cpMin = sel.cpMax;
			SendMessage(m_log.GetHwnd(), EM_EXSETSEL, 0, (LPARAM)&sel);
		}
		SetFocus(m_message.GetHwnd());
		break;
	}

	return CSuper::WndProc_Log(msg, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////////////////

LRESULT CChatRoomDlg::WndProc_Nicklist(UINT msg, WPARAM wParam, LPARAM lParam)
{
	int result = InputAreaShortcuts(m_nickList.GetHwnd(), msg, wParam, lParam);
	if (result != -1)
		return result;

	switch (msg) {
	case WM_GETDLGCODE:
		{
			BOOL isAlt = GetKeyState(VK_MENU) & 0x8000;
			BOOL isCtrl = (GetKeyState(VK_CONTROL) & 0x8000) && !isAlt;

			LPMSG lpmsg;
			if ((lpmsg = (LPMSG)lParam) != nullptr) {
				if (lpmsg->message == WM_KEYDOWN
					&& (lpmsg->wParam == VK_RETURN || lpmsg->wParam == VK_ESCAPE || (lpmsg->wParam == VK_TAB && (isAlt || isCtrl))))
					return DLGC_WANTALLKEYS;
			}
		}
		break;

	case WM_KEYDOWN:
		if (wParam == VK_RETURN) {
			int index = m_nickList.SendMsg(LB_GETCURSEL, 0, 0);
			if (index != LB_ERR) {
				USERINFO *ui = pci->SM_GetUserFromIndex(m_si->ptszID, m_si->pszModule, index);
				Chat_DoEventHook(m_si, GC_USER_PRIVMESS, ui, nullptr, 0);
			}
			break;
		}
		
		if (wParam == VK_ESCAPE || wParam == VK_UP || wParam == VK_DOWN || wParam == VK_NEXT || wParam == VK_PRIOR || wParam == VK_TAB || wParam == VK_HOME || wParam == VK_END)
			m_wszSearch[0] = 0;
		break;

	case WM_CHAR:
	case WM_UNICHAR:
		/*
		* simple incremental search for the user (nick) - list control
		* typing esc or movement keys will clear the current search string
		*/
		if (wParam == 27 && m_wszSearch[0]) {						// escape - reset everything
			m_wszSearch[0] = 0;
			break;
		}
		else if (wParam == '\b' && m_wszSearch[0])					// backspace
			m_wszSearch[mir_wstrlen(m_wszSearch) - 1] = '\0';
		else if (wParam < ' ')
			break;
		else {
			wchar_t szNew[2];
			szNew[0] = (wchar_t)wParam;
			szNew[1] = '\0';
			if (mir_wstrlen(m_wszSearch) >= _countof(m_wszSearch) - 2) {
				MessageBeep(MB_OK);
				break;
			}
			mir_wstrcat(m_wszSearch, szNew);
		}
		if (m_wszSearch[0]) {
			// iterate over the (sorted) list of nicknames and search for the
			// string we have
			int iItems = m_nickList.SendMsg(LB_GETCOUNT, 0, 0);
			for (int i = 0; i < iItems; i++) {
				USERINFO *ui = pci->UM_FindUserFromIndex(m_si->pUsers, i);
				if (ui) {
					if (!wcsnicmp(ui->pszNick, m_wszSearch, mir_wstrlen(m_wszSearch))) {
						m_nickList.SendMsg(LB_SETCURSEL, i, 0);
						InvalidateRect(m_nickList.GetHwnd(), nullptr, FALSE);
						return 0;
					}
				}
			}

			MessageBeep(MB_OK);
			m_wszSearch[mir_wstrlen(m_wszSearch) - 1] = '\0';
			return 0;
		}
		break;
	}

	return CSuper::WndProc_Nicklist(msg, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CChatRoomDlg::DlgProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static HMENU hToolbarMenu;
	RECT rc;
	POINT pt;
	HICON hIcon;
	TabControlData tcd;

	switch (uMsg) {
	case DM_SWITCHINFOBAR:
	case DM_SWITCHTOOLBAR:
		Resize();
		break;

	case WM_SIZE:
		if (wParam == SIZE_MAXIMIZED)
			ScrollToBottom();

		if (IsIconic(m_hwnd)) break;

		if (wParam == SIZE_RESTORED || wParam == SIZE_MAXIMIZED) {
			GetClientRect(m_hwnd, &rc);
			if (rc.right && rc.bottom)
				MessageDialogResize(rc.right, rc.bottom);
		}
		break;

	case DM_UPDATETABCONTROL:
		tcd.iFlags = TCDF_TEXT;
		tcd.pszText = m_si->ptszName;
		SendMessage(m_hwndParent, CM_UPDATETABCONTROL, (WPARAM)&tcd, (LPARAM)m_hwnd);
		// fall through

	case DM_ACTIVATE:
		if (m_si->wState & STATE_TALK) {
			m_si->wState &= ~STATE_TALK;
			db_set_w(m_hContact, m_si->pszModule, "ApparentMode", 0);
		}

		if (m_si->wState & GC_EVENT_HIGHLIGHT) {
			m_si->wState &= ~GC_EVENT_HIGHLIGHT;

			if (pcli->pfnGetEvent(m_hContact, 0))
				pcli->pfnRemoveEvent(m_hContact, GC_FAKE_EVENT);
		}

		FixTabIcons();
		if (!m_si->pDlg) {
			ShowRoom(m_si);
			SendMessage(m_hwnd, WM_MOUSEACTIVATE, 0, 0);
		}
		break;

	case WM_CTLCOLORLISTBOX:
		SetBkColor((HDC)wParam, g_Settings.crUserListBGColor);
		return (INT_PTR)pci->hListBkgBrush;

	case WM_MEASUREITEM:
		if (!MeasureMenuItem(wParam, lParam)) {
			MEASUREITEMSTRUCT *mis = (MEASUREITEMSTRUCT *)lParam;
			if (mis->CtlType == ODT_MENU)
				return Menu_MeasureItem(lParam);

			int ih = Chat_GetTextPixelSize(L"AQGgl'", g_Settings.UserListFont, false);
			int ih2 = Chat_GetTextPixelSize(L"AQGg'", g_Settings.UserListHeadingsFont, false);
			int font = ih > ih2 ? ih : ih2;
			int height = db_get_b(0, CHAT_MODULE, "NicklistRowDist", 12);
			// make sure we have space for icon!
			if (db_get_b(0, CHAT_MODULE, "ShowContactStatus", 0))
				font = font > 16 ? font : 16;
			mis->itemHeight = height > font ? height : font;
		}
		return TRUE;

	case WM_DRAWITEM:
		if (!DrawMenuItem(wParam, lParam)) {
			DRAWITEMSTRUCT *dis = (DRAWITEMSTRUCT *)lParam;
			if (dis->CtlType == ODT_MENU)
				return Menu_DrawItem(lParam);

			if (dis->CtlID == IDC_SRMM_NICKLIST) {
				int index = dis->itemID;
				USERINFO *ui = pci->SM_GetUserFromIndex(m_si->ptszID, m_si->pszModule, index);
				if (ui) {
					int x_offset = 2;

					int height = dis->rcItem.bottom - dis->rcItem.top;
					if (height & 1)
						height++;

					int offset = (height == 10) ? 0 : height / 2 - 5;
					HFONT hFont = (ui->iStatusEx == 0) ? g_Settings.UserListFont : g_Settings.UserListHeadingsFont;
					HFONT hOldFont = (HFONT)SelectObject(dis->hDC, hFont);
					SetBkMode(dis->hDC, TRANSPARENT);

					if (dis->itemAction == ODA_FOCUS && dis->itemState & ODS_SELECTED)
						FillRect(dis->hDC, &dis->rcItem, pci->hListSelectedBkgBrush);
					else //if (dis->itemState & ODS_INACTIVE)
						FillRect(dis->hDC, &dis->rcItem, pci->hListBkgBrush);

					if (g_Settings.bShowContactStatus && g_Settings.bContactStatusFirst && ui->ContactStatus) {
						hIcon = Skin_LoadProtoIcon(m_si->pszModule, ui->ContactStatus);
						DrawIconEx(dis->hDC, x_offset, dis->rcItem.top + offset - 3, hIcon, 16, 16, 0, nullptr, DI_NORMAL);
						IcoLib_ReleaseIcon(hIcon);
						x_offset += 18;
					}
					DrawIconEx(dis->hDC, x_offset, dis->rcItem.top + offset, pci->SM_GetStatusIcon(m_si, ui), 10, 10, 0, nullptr, DI_NORMAL);
					x_offset += 12;
					if (g_Settings.bShowContactStatus && !g_Settings.bContactStatusFirst && ui->ContactStatus) {
						hIcon = Skin_LoadProtoIcon(m_si->pszModule, ui->ContactStatus);
						DrawIconEx(dis->hDC, x_offset, dis->rcItem.top + offset - 3, hIcon, 16, 16, 0, nullptr, DI_NORMAL);
						IcoLib_ReleaseIcon(hIcon);
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

	case WM_ACTIVATE:
		if (LOWORD(wParam) != WA_ACTIVE)
			break;

		// fall through
	case WM_MOUSEACTIVATE:
		if (uMsg != WM_ACTIVATE)
			SetFocus(m_message.GetHwnd());

		pci->SetActiveSession(m_si);

		if (db_get_w(m_hContact, m_si->pszModule, "ApparentMode", 0) != 0)
			db_set_w(m_hContact, m_si->pszModule, "ApparentMode", 0);
		if (pcli->pfnGetEvent(m_hContact, 0))
			pcli->pfnRemoveEvent(m_hContact, GC_FAKE_EVENT);
		break;

	case WM_NOTIFY:
		{
			LPNMHDR pNmhdr = (LPNMHDR)lParam;
			switch (pNmhdr->code) {
			case EN_MSGFILTER:
				if (pNmhdr->idFrom == IDC_SRMM_LOG && ((MSGFILTER *)lParam)->msg == WM_RBUTTONUP) {
					SetWindowLongPtr(m_hwnd, DWLP_MSGRESULT, TRUE);
					return TRUE;
				}
				break;

			case TTN_NEEDTEXT:
				if (pNmhdr->idFrom == (UINT_PTR)m_nickList.GetHwnd()) {
					LPNMTTDISPINFO lpttd = (LPNMTTDISPINFO)lParam;
					SESSION_INFO* parentdat = (SESSION_INFO*)GetWindowLongPtr(m_hwnd, GWLP_USERDATA);

					POINT p;
					GetCursorPos(&p);
					ScreenToClient(m_nickList.GetHwnd(), &p);
					int item = LOWORD(m_nickList.SendMsg(LB_ITEMFROMPOINT, 0, MAKELPARAM(p.x, p.y)));
					USERINFO *ui = pci->SM_GetUserFromIndex(parentdat->ptszID, parentdat->pszModule, item);
					if (ui != nullptr) {
						static wchar_t ptszBuf[1024];
						mir_snwprintf(ptszBuf, L"%s: %s\r\n%s: %s\r\n%s: %s",
							TranslateT("Nickname"), ui->pszNick,
							TranslateT("Unique ID"), ui->pszUID,
							TranslateT("Status"), pci->TM_WordToString(parentdat->pStatuses, ui->Status));
						lpttd->lpszText = ptszBuf;
					}
				}
				break;
			}
		}
		break;

	case WM_KEYDOWN:
		SetFocus(m_message.GetHwnd());
		break;

	case WM_GETMINMAXINFO:
		{
			MINMAXINFO *mmi = (MINMAXINFO*)lParam;
			mmi->ptMinTrackSize.x = m_pParent->iSplitterX + 43;
			if (mmi->ptMinTrackSize.x < 350)
				mmi->ptMinTrackSize.x = 350;

			mmi->ptMinTrackSize.y = m_minLogBoxHeight + TOOLBAR_HEIGHT + m_minEditBoxHeight + 5;
		}
		break;

	case WM_LBUTTONDBLCLK:
		if (LOWORD(lParam) < 30)
			ScrollToBottom();
		else
			SendMessage(m_hwndParent, WM_SYSCOMMAND, SC_MINIMIZE, 0);
		break;

	case WM_LBUTTONDOWN:
		SendMessage(m_hwndParent, WM_LBUTTONDOWN, wParam, lParam);
		return TRUE;

	case DM_GETCONTEXTMENU:
		SetWindowLongPtr(m_hwnd, DWLP_MSGRESULT, (LPARAM)Menu_BuildContactMenu(m_hContact));
		return TRUE;

	case WM_CONTEXTMENU:
		if (m_hwndParent == (HWND)wParam) {
			HMENU hMenu = Menu_BuildContactMenu(m_hContact);
			GetCursorPos(&pt);
			TrackPopupMenu(hMenu, 0, pt.x, pt.y, 0, m_hwnd, nullptr);
			DestroyMenu(hMenu);
		}
		break;
	}
	return CSuper::DlgProc(uMsg, wParam, lParam);
}

void ShowRoom(SESSION_INFO *si)
{
	if (si == nullptr)
		return;

	// Do we need to create a window?
	CChatRoomDlg *pDlg;
	if (si->pDlg == nullptr) {
		HWND hParent = GetParentWindow(si->hContact, TRUE);

		pDlg = new CChatRoomDlg(si);
		pDlg->SetParent(hParent);
		pDlg->Show();
		
		pDlg->m_pParent = (ParentWindowData*)GetWindowLongPtr(hParent, GWLP_USERDATA);
		si->pDlg = pDlg;
	}
	else pDlg = si->pDlg;
	
	SendMessage(pDlg->GetHwnd(), DM_UPDATETABCONTROL, -1, (LPARAM)si);
	SendMessage(GetParent(pDlg->GetHwnd()), CM_ACTIVATECHILD, 0, (LPARAM)pDlg->GetHwnd());
	SendMessage(GetParent(pDlg->GetHwnd()), CM_POPUPWINDOW, 0, (LPARAM)pDlg->GetHwnd());
	SendMessage(pDlg->GetHwnd(), WM_MOUSEACTIVATE, 0, 0);
	SetFocus(GetDlgItem(pDlg->GetHwnd(), IDC_SRMM_MESSAGE));
}
