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

#include "stdafx.h"

void CMsgDialog::ClearLog()
{
	CSuper::ClearLog();

	m_hDbEventFirst = 0;
}

void CMsgDialog::CloseTab()
{
	Close();
}

void CMsgDialog::DrawNickList(USERINFO *ui, DRAWITEMSTRUCT *dis)
{
	int x_offset = 2;

	int height = dis->rcItem.bottom - dis->rcItem.top;
	if (height & 1)
		height++;

	int offset = (height == 10) ? 0 : height / 2 - 5;
	HFONT hFont = (ui->iStatusEx == 0) ? g_Settings.UserListFont : g_Settings.UserListHeadingsFont;
	HFONT hOldFont = (HFONT)SelectObject(dis->hDC, hFont);
	SetBkMode(dis->hDC, TRANSPARENT);

	if (dis->itemAction == ODA_FOCUS && dis->itemState & ODS_SELECTED)
		FillRect(dis->hDC, &dis->rcItem, g_chatApi.hListSelectedBkgBrush);
	else //if (dis->itemState & ODS_INACTIVE)
		FillRect(dis->hDC, &dis->rcItem, g_chatApi.hListBkgBrush);

	if (g_Settings.bShowContactStatus && g_Settings.bContactStatusFirst && ui->ContactStatus) {
		HICON hIcon = Skin_LoadProtoIcon(m_si->pszModule, ui->ContactStatus);
		DrawIconEx(dis->hDC, x_offset, dis->rcItem.top + offset - 3, hIcon, 16, 16, 0, nullptr, DI_NORMAL);
		IcoLib_ReleaseIcon(hIcon);
		x_offset += 18;
	}
	DrawIconEx(dis->hDC, x_offset, dis->rcItem.top + offset, g_chatApi.SM_GetStatusIcon(m_si, ui), 10, 10, 0, nullptr, DI_NORMAL);
	x_offset += 12;
	if (g_Settings.bShowContactStatus && !g_Settings.bContactStatusFirst && ui->ContactStatus) {
		HICON hIcon = Skin_LoadProtoIcon(m_si->pszModule, ui->ContactStatus);
		DrawIconEx(dis->hDC, x_offset, dis->rcItem.top + offset - 3, hIcon, 16, 16, 0, nullptr, DI_NORMAL);
		IcoLib_ReleaseIcon(hIcon);
		x_offset += 18;
	}

	SetTextColor(dis->hDC, ui->iStatusEx == 0 ? g_Settings.crUserListColor : g_Settings.crUserListHeadingsColor);
	TextOut(dis->hDC, dis->rcItem.left + x_offset, dis->rcItem.top, ui->pszNick, (int)mir_wstrlen(ui->pszNick));
	SelectObject(dis->hDC, hOldFont);
}

void CMsgDialog::EventAdded(MEVENT hDbEvent, const DB::EventInfo &dbei)
{
	if (m_hDbEventFirst == 0)
		m_hDbEventFirst = hDbEvent;

	if (DbEventIsShown(dbei)) {
		bool bIsActive = IsActive();
		if (dbei.eventType == EVENTTYPE_MESSAGE && !(dbei.flags & (DBEF_SENT))) {
			/* store the event when the container is hidden so that clist notifications can be removed */
			if (!IsWindowVisible(m_hwndParent) && m_hDbUnreadEventFirst == 0)
				m_hDbUnreadEventFirst = hDbEvent;
			m_lastMessage = dbei.getUnixtime();
			UpdateStatusBar();
			if (bIsActive)
				Skin_PlaySound("RecvMsgActive");
			else
				Skin_PlaySound("RecvMsgInactive");
			if (g_dat.flags2.bSwitchToActive && (IsIconic(m_hwndParent) || GetActiveWindow() != m_hwndParent) && IsWindowVisible(m_hwndParent))
				m_pParent->ActivateChild(this);
			if (IsAutoPopup(m_hContact))
				PopupWindow(true);
		}

		if (hDbEvent != m_hDbEventFirst && db_event_next(m_hContact, hDbEvent) == 0)
			m_pLog->LogEvents(hDbEvent, 1, 1);
		else
			RemakeLog();

		if (!dbei.bSent && !dbei.isCustom(DETF_MSGWINDOW)) {
			if (!bIsActive) {
				m_iShowUnread = 1;
				UpdateIcon();
				SetTimer(m_hwnd, TIMERID_UNREAD, TIMEOUT_UNREAD, nullptr);
			}
			StartFlashing();
		}
	}
}

bool CMsgDialog::GetFirstEvent()
{
	bool notifyUnread = false;

	if (m_hContact) {
		int historyMode = Srmm::iHistoryMode;
		// This finds the first message to display, it works like shit
		m_hDbEventFirst = db_event_firstUnread(m_hContact);
		if (m_hDbEventFirst != 0) {
			DB::EventInfo dbei(m_hDbEventFirst, false);
			if (dbei.isSrmm() && !dbei.bRead && !dbei.bSent)
				notifyUnread = true;
		}

		DB::ECPTR pCursor(DB::EventsRev(m_hContact, m_hDbEventFirst));

		DB::EventInfo dbei;
		MEVENT hPrevEvent;
		switch (historyMode) {
		case LOADHISTORY_COUNT:
			for (int i = g_plugin.iLoadCount; i > 0; i--) {
				hPrevEvent = pCursor.FetchNext();
				if (hPrevEvent == 0)
					break;

				dbei.cbBlob = 0;
				m_hDbEventFirst = hPrevEvent;
				db_event_get(m_hDbEventFirst, &dbei);
				if (!DbEventIsShown(dbei))
					i++;
			}
			break;

		case LOADHISTORY_TIME:
			if (m_hDbEventFirst == 0)
				dbei.iTimestamp = time(0);
			else
				db_event_get(m_hDbEventFirst, &dbei);

			uint32_t firstTime = dbei.getUnixtime() - 60 * g_plugin.iLoadTime;
			for (;;) {
				hPrevEvent = pCursor.FetchNext();
				if (hPrevEvent == 0)
					break;

				dbei.cbBlob = 0;
				db_event_get(hPrevEvent, &dbei);
				if (dbei.getUnixtime() < firstTime)
					break;
				if (DbEventIsShown(dbei))
					m_hDbEventFirst = hPrevEvent;
			}
			break;
		}
	}
	return notifyUnread;
}

void CMsgDialog::GetInputFont(LOGFONTW &lf, COLORREF &bg, COLORREF &fg) const
{
	bg = m_clrInputBG;
	LoadMsgDlgFont(MSGFONTID_MESSAGEAREA, &lf, &fg);
}

void CMsgDialog::FixTabIcons()
{
	HICON hIcon;
	if (!(m_si->wState & GC_EVENT_HIGHLIGHT)) {
		if (m_si->wState & STATE_TALK)
			hIcon = (m_si->wStatus == ID_STATUS_ONLINE) ? m_si->pMI->hOnlineTalkIcon : m_si->pMI->hOfflineTalkIcon;
		else
			hIcon = (m_si->wStatus == ID_STATUS_ONLINE) ? m_si->pMI->hOnlineIcon : m_si->pMI->hOfflineIcon;
	}
	else hIcon = g_dat.hMsgIcon;

	TabControlData tcd = {};
	tcd.iFlags = TCDF_ICON;
	tcd.hIcon = hIcon;
	m_pParent->UpdateTabControl(tcd, m_hwnd);
}

void CMsgDialog::GetAvatar()
{
	PROTO_AVATAR_INFORMATION ai = {};
	ai.hContact = m_hContact;
	CallProtoService(m_szProto, PS_GETAVATARINFO, GAIF_FORCE, (LPARAM)&ai);
	ShowAvatar();
}

HICON CMsgDialog::GetTabIcon()
{
	if (m_bShowTyping)
		return g_plugin.getIcon(IDI_TYPING);

	if (m_iShowUnread != 0)
		return m_hStatusIconOverlay;

	return m_hStatusIcon;
}

void CMsgDialog::GetTitlebarIcon(TitleBarData *tbd)
{
	if (m_bShowTyping && g_dat.flags2.bShowTypingWin)
		tbd->hIconNot = tbd->hIcon = g_plugin.getIcon(IDI_TYPING);
	else if (m_iShowUnread && (GetActiveWindow() != m_hwndParent || GetForegroundWindow() != m_hwndParent)) {
		tbd->hIcon = m_hStatusIcon;
		tbd->hIconNot = g_dat.hMsgIcon;
	}
	else {
		tbd->hIcon = m_hStatusIcon;
		tbd->hIconNot = nullptr;
	}
	tbd->hIconBig = m_hStatusIconBig;
}

bool CMsgDialog::IsTypingNotificationSupported()
{
	if (!m_hContact || !m_szProto)
		return false;

	uint32_t typeCaps = CallProtoService(m_szProto, PS_GETCAPS, PFLAGNUM_4, 0);
	if (!(typeCaps & PF4_SUPPORTTYPING))
		return false;
	return true;
}

bool CMsgDialog::IsTypingNotificationEnabled()
{
	if (!g_plugin.getByte(m_hContact, SRMSGSET_TYPING, g_plugin.bTypingNew))
		return FALSE;

	uint32_t protoStatus = Proto_GetStatus(m_szProto);
	if (protoStatus < ID_STATUS_ONLINE)
		return FALSE;

	if (!Contact::OnList(m_hContact) && !g_plugin.bTypingUnknown)
		return FALSE;
	return TRUE;
}

void CMsgDialog::LoadSettings()
{
	m_clrInputBG = g_plugin.getDword(SRMSGSET_INPUTBKGCOLOUR, SRMSGDEFSET_INPUTBKGCOLOUR);
	LoadMsgDlgFont(MSGFONTID_MESSAGEAREA, nullptr, &m_clrInputFG);
}

void CMsgDialog::MessageSend(const SendQueue::Item &msi)
{
	StartMessageSending();

	SendQueue::Item *item = SendQueue::CreateItem(this);
	item->hContact = m_hContact;
	item->proto = mir_strdup(m_szProto);
	item->flags = msi.flags;
	item->sendBufferSize = msi.sendBufferSize;
	item->sendBuffer = mir_strndup(msi.sendBuffer, msi.sendBufferSize);
	SendQueue::SendItem(item);
}

// Don't send to protocols who don't support typing
// Don't send to users who are unchecked in the typing notification options
// Don't send to protocols that are offline
// Don't send to users who are not visible and
// Don't send to users who are not on the visible list when you are in invisible mode.

void CMsgDialog::NotifyTyping(int mode)
{
	if (!IsTypingNotificationSupported())
		return;

	if (!IsTypingNotificationEnabled())
		return;

	// End user check
	m_nTypeMode = mode;
	if (isChat())
		Chat_DoEventHook(m_si, GC_USER_TYPNOTIFY, 0, 0, m_nTypeMode);
	else
		CallService(MS_PROTO_SELFISTYPING, m_hContact, m_nTypeMode);
}

void CMsgDialog::OnOptionsApplied()
{
	CSuper::OnOptionsApplied();

	GetAvatar();
	UpdateTitle();
	UpdateStatusBar();

	// messagebox
	LOGFONT lf;
	CHARFORMAT2 cf = {};
	cf.cbSize = sizeof(CHARFORMAT2);
	LoadMsgDlgFont(MSGFONTID_MESSAGEAREA, &lf, &cf.crTextColor);
	cf.crBackColor = g_plugin.getDword(SRMSGSET_INPUTBKGCOLOUR, SRMSGDEFSET_INPUTBKGCOLOUR);
	cf.dwMask = CFM_COLOR | CFM_BACKCOLOR | CFM_FACE | CFM_CHARSET | CFM_SIZE | CFM_WEIGHT | CFM_BOLD | CFM_ITALIC;
	cf.bCharSet = lf.lfCharSet;
	wcsncpy(cf.szFaceName, lf.lfFaceName, LF_FACESIZE);
	cf.dwEffects = ((lf.lfWeight >= FW_BOLD) ? CFE_BOLD : 0) | (lf.lfItalic ? CFE_ITALIC : 0);
	cf.wWeight = (uint16_t)lf.lfWeight;
	cf.bPitchAndFamily = lf.lfPitchAndFamily;
	cf.yHeight = abs(lf.lfHeight) * 1440 / g_dat.logPixelSY;
	m_message.SendMsg(EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf);
	m_message.SendMsg(EM_SETBKGNDCOLOR, 0, cf.crBackColor);
	m_message.SendMsg(EM_SETLANGOPTIONS, 0, (LPARAM)m_message.SendMsg(EM_GETLANGOPTIONS, 0, 0) & ~IMF_AUTOKEYBOARD);
	m_message.SendMsg(WM_SETFONT, (WPARAM)g_Settings.MessageBoxFont, MAKELPARAM(TRUE, 0));
	InvalidateRect(m_message.GetHwnd(), nullptr, FALSE);

	if (isChat()) {
		FixTabIcons();

		// nicklist
		int ih = Chat_GetTextPixelSize(L"AQG_glo'", g_Settings.UserListFont, false);
		int ih2 = Chat_GetTextPixelSize(L"AQG_glo'", g_Settings.UserListHeadingsFont, false);
		int height = db_get_b(0, CHAT_MODULE, "NicklistRowDist", 12);
		int font = ih > ih2 ? ih : ih2;

		// make sure we have space for icon!
		if (Chat::bShowContactStatus)
			font = font > 16 ? font : 16;

		m_nickList.SendMsg(LB_SETITEMHEIGHT, 0, height > font ? height : font);
		InvalidateRect(m_nickList.GetHwnd(), nullptr, TRUE);

		UpdateChatOptions();
	}
	else {
		SetDialogToType();

		UpdateTabControl();
		SetupInfobar();
	}

	ScheduleRedrawLog();
	m_message.SendMsg(EM_REQUESTRESIZE, 0, 0);
}

void CMsgDialog::Reattach(HWND hwndContainer)
{
	MCONTACT hContact = m_hContact;

	POINT pt;
	GetCursorPos(&pt);
	HWND hParent = WindowFromPoint(pt);
	while (GetParent(hParent) != nullptr)
		hParent = GetParent(hParent);

	hParent = WindowList_Find(g_dat.hParentWindowList, (UINT_PTR)hParent);
	if ((hParent != nullptr && hParent != hwndContainer) || (hParent == nullptr && m_pParent->m_iChildrenCount > 1 && (GetKeyState(VK_CONTROL) & 0x8000))) {
		if (hParent == nullptr) {
			hParent = GetParentWindow(hContact, false);

			RECT rc;
			GetWindowRect(hParent, &rc);

			rc.right = (rc.right - rc.left);
			rc.bottom = (rc.bottom - rc.top);
			rc.left = pt.x - rc.right / 2;
			rc.top = pt.y - rc.bottom / 2;
			HMONITOR hMonitor = MonitorFromRect(&rc, MONITOR_DEFAULTTONEAREST);

			MONITORINFO mi;
			mi.cbSize = sizeof(mi);
			GetMonitorInfo(hMonitor, &mi);

			RECT rcDesktop = mi.rcWork;
			if (rc.left < rcDesktop.left)
				rc.left = rcDesktop.left;
			if (rc.top < rcDesktop.top)
				rc.top = rcDesktop.top;
			MoveWindow(hParent, rc.left, rc.top, rc.right, rc.bottom, FALSE);
		}
		NotifyEvent(MSG_WINDOW_EVT_CLOSING);
		NotifyEvent(MSG_WINDOW_EVT_CLOSE);
		SetParent(hParent);
		m_pParent->RemoveChild(m_hwnd);
		SendMessage(m_hwnd, DM_SETPARENT, 0, (LPARAM)hParent);
		m_pParent->AddChild(this);
		UpdateTabControl();
		m_pParent->ActivateChild(this);
		NotifyEvent(MSG_WINDOW_EVT_OPENING);
		NotifyEvent(MSG_WINDOW_EVT_OPEN);
		ShowWindow(hParent, SW_SHOWNA);
		EnableWindow(hParent, TRUE);
	}
}

void CMsgDialog::SetDialogToType()
{
	if (!isChat()) {
		ParentWindowData *pdat = m_pParent;
		if (pdat->flags2.bShowInfoBar)
			ShowWindow(m_hwndInfo, SW_SHOW);
		else
			ShowWindow(m_hwndInfo, SW_HIDE);

		SendMessage(m_hwnd, DM_CLISTSETTINGSCHANGED, 0, 0);
		UpdateReadChars();
	}

	m_message.Show();
	m_splitterY.Show();
	m_btnOk.Enable(m_message.GetRichTextLength() != 0);
	Resize();
}

void CMsgDialog::SetStatusIcon()
{
	if (m_szProto == nullptr)
		return;

	MCONTACT hContact = db_mc_getSrmmSub(m_hContact);
	if (hContact == 0)
		hContact = m_hContact;

	char *szProto = Proto_GetBaseAccountName(hContact);
	m_hStatusIcon = Skin_LoadProtoIcon(szProto, m_wStatus, false);
	m_hStatusIconBig = Skin_LoadProtoIcon(szProto, m_wStatus, true);

	if (m_hStatusIconOverlay != nullptr)
		DestroyIcon(m_hStatusIconOverlay);

	int index = ImageList_ReplaceIcon(g_dat.hHelperIconList, 0, m_hStatusIcon);
	m_hStatusIconOverlay = ImageList_GetIcon(g_dat.hHelperIconList, index, ILD_TRANSPARENT | INDEXTOOVERLAYMASK(1));
}

void CMsgDialog::ShowAvatar()
{
	INT_PTR res = CallService(MS_AV_GETAVATARBITMAP, m_hContact, 0);
	m_ace = res != CALLSERVICE_NOTFOUND ? (AVATARCACHEENTRY*)res : nullptr;
	m_hbmpAvatarPic = (m_ace != nullptr && (m_ace->dwFlags & AVS_HIDEONCLIST) == 0) ? m_ace->hbmPic : nullptr;
	Resize();

	RefreshInfobar();

	RedrawWindow(GetDlgItem(m_hwnd, IDC_AVATAR), nullptr, nullptr, RDW_INVALIDATE);
}

/////////////////////////////////////////////////////////////////////////////////////////

static INT_PTR CALLBACK FilterWndProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int iFlags;
	auto *pDlg = (CMsgDialog*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (uMsg) {
	case WM_INITDIALOG:
		pDlg = (CMsgDialog *)lParam;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);

		iFlags = db_get_dw(pDlg->m_hContact, CHAT_MODULE, "FilterFlags");
		CheckDlgButton(hwndDlg, IDC_CHAT_1, iFlags & GC_EVENT_ACTION ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHAT_2, iFlags & GC_EVENT_MESSAGE ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHAT_3, iFlags & GC_EVENT_NICK ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHAT_4, iFlags & GC_EVENT_JOIN ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHAT_5, iFlags & GC_EVENT_PART ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHAT_6, iFlags & GC_EVENT_TOPIC ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHAT_7, iFlags & GC_EVENT_ADDSTATUS ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHAT_8, iFlags & GC_EVENT_INFORMATION ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHAT_9, iFlags & GC_EVENT_QUIT ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHAT_10, iFlags & GC_EVENT_KICK ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHAT_11, iFlags & GC_EVENT_NOTICE ? BST_CHECKED : BST_UNCHECKED);
		break;

	case WM_CTLCOLOREDIT:
	case WM_CTLCOLORSTATIC:
		SetTextColor((HDC)wParam, RGB(60, 60, 150));
		SetBkColor((HDC)wParam, GetSysColor(COLOR_WINDOW));
		return (INT_PTR)GetSysColorBrush(COLOR_WINDOW);

	case WM_ACTIVATE:
		if (LOWORD(wParam) == WA_INACTIVE) {
			iFlags = 0;

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

			db_set_dw(pDlg->m_hContact, CHAT_MODULE, "FilterFlags", iFlags);
			db_set_dw(pDlg->m_hContact, CHAT_MODULE, "FilterMask", 0xFFFF);

			Chat_SetFilters(pDlg->getChat());
			pDlg->RedrawLog();
			PostMessage(hwndDlg, WM_CLOSE, 0, 0);
		}
		break;

	case WM_CLOSE:
		pDlg->m_hwndFilter = nullptr;
		DestroyWindow(hwndDlg);
		break;
	}

	return FALSE;
}

void CMsgDialog::ShowFilterMenu()
{
	m_hwndFilter = CreateDialogParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_FILTER), m_hwnd, FilterWndProc, (LPARAM)this);
	TranslateDialogDefault(m_hwndFilter);

	RECT rc;
	GetWindowRect(m_btnFilter.GetHwnd(), &rc);
	SetWindowPos(m_hwndFilter, HWND_TOP, rc.left - 85, (IsWindowVisible(m_btnFilter.GetHwnd()) || IsWindowVisible(m_btnBold.GetHwnd())) ? rc.top - 206 : rc.top - 186, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
}

void CMsgDialog::ShowMessageSending()
{
	SetTimer(m_hwnd, TIMERID_MSGSEND, 1000, nullptr);
	if (g_dat.flags.bShowProgress)
		UpdateStatusBar();
}

void CMsgDialog::StartMessageSending()
{
	m_iMessagesInProgress++;
	ShowMessageSending();
}

void CMsgDialog::StopMessageSending()
{
	if (m_iMessagesInProgress > 0) {
		m_iMessagesInProgress--;
		if (g_dat.flags.bShowProgress)
			UpdateStatusBar();
	}
	if (m_iMessagesInProgress == 0)
		KillTimer(m_hwnd, TIMERID_MSGSEND);
}

void CMsgDialog::SwitchTyping()
{
	if (IsTypingNotificationSupported()) {
		uint8_t typingNotify = (g_plugin.getByte(m_hContact, SRMSGSET_TYPING, g_plugin.bTypingNew));
		g_plugin.setByte(m_hContact, SRMSGSET_TYPING, (uint8_t)!typingNotify);
		Srmm_SetIconFlags(m_hContact, SRMM_MODULE, 1, typingNotify ? MBF_DISABLED : 0);
	}
}

void CMsgDialog::TabAutoComplete()
{
	LRESULT lResult = (LRESULT)m_message.SendMsg(EM_GETSEL, 0, 0);
	int start = LOWORD(lResult), end = HIWORD(lResult);
	m_message.SendMsg(EM_SETSEL, end, end);

	int iLen = m_message.GetRichTextLength(1200);
	if (iLen <= 0)
		return;

	bool isTopic = false, isRoom = false;
	wchar_t *pszName = nullptr;
	wchar_t *pszText = (wchar_t *)mir_alloc(iLen + 100 * sizeof(wchar_t));

	GETTEXTEX gt = {};
	gt.codepage = 1200;
	gt.cb = iLen + 99 * sizeof(wchar_t);
	gt.flags = GT_DEFAULT;
	m_message.SendMsg(EM_GETTEXTEX, (WPARAM)& gt, (LPARAM)pszText);

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
		while (topicStart > 0 && (pszText[topicStart - 1] == ' ' || pszText[topicStart - 1] == 13 || pszText[topicStart - 1] == VK_TAB))
			topicStart--;
		if (topicStart > 5 && wcsstr(&pszText[topicStart - 6], L"/topic") == &pszText[topicStart - 6])
			isTopic = true;
	}

	if (m_wszSearchQuery == nullptr) {
		m_wszSearchQuery = (wchar_t *)mir_alloc(sizeof(wchar_t) * (end - start + 1));
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
	else pszName = g_chatApi.UM_FindUserAutoComplete(m_si, m_wszSearchQuery, m_wszSearchResult);

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
				szReplace = (wchar_t *)mir_alloc((mir_wstrlen(pszName) + 4) * sizeof(wchar_t));
				mir_wstrcpy(szReplace, pszName);
				mir_wstrcat(szReplace, L": ");
				pszName = szReplace;
			}
			m_message.SendMsg(EM_SETSEL, start, end);
			m_message.SendMsg(EM_REPLACESEL, FALSE, (LPARAM)pszName);
		}
	}
}

void CMsgDialog::ToggleRtl()
{
	PARAFORMAT2 pf2;
	memset(&pf2, 0, sizeof(pf2));
	pf2.cbSize = sizeof(pf2);
	pf2.dwMask = PFM_RTLPARA;
	m_bUseRtl = !m_bUseRtl;
	if (m_bUseRtl) {
		pf2.wEffects = PFE_RTLPARA;
		SetWindowLongPtr(m_message.GetHwnd(), GWL_EXSTYLE, GetWindowLongPtr(m_message.GetHwnd(), GWL_EXSTYLE) | WS_EX_RIGHT | WS_EX_RTLREADING | WS_EX_LEFTSCROLLBAR);
		SetWindowLongPtr(m_pLog->GetHwnd(), GWL_EXSTYLE, GetWindowLongPtr(m_pLog->GetHwnd(), GWL_EXSTYLE) | WS_EX_LEFTSCROLLBAR);
	}
	else {
		pf2.wEffects = 0;
		SetWindowLongPtr(m_message.GetHwnd(), GWL_EXSTYLE, GetWindowLongPtr(m_message.GetHwnd(), GWL_EXSTYLE) & ~(WS_EX_RIGHT | WS_EX_RTLREADING | WS_EX_LEFTSCROLLBAR));
		SetWindowLongPtr(m_pLog->GetHwnd(), GWL_EXSTYLE, GetWindowLongPtr(m_pLog->GetHwnd(), GWL_EXSTYLE) & ~(WS_EX_LEFTSCROLLBAR));
	}
	m_message.SendMsg(EM_SETPARAFORMAT, 0, (LPARAM)&pf2);
	RemakeLog();
}

void CMsgDialog::UpdateIcon()
{
	TitleBarData tbd = {};
	tbd.iFlags = TBDF_ICON;
	GetTitlebarIcon(&tbd);
	m_pParent->UpdateTitleBar(tbd, m_hwnd);

	TabControlData tcd;
	tcd.iFlags = TCDF_ICON;
	tcd.hIcon = GetTabIcon();
	m_pParent->UpdateTabControl(tcd, m_hwnd);

	SendDlgItemMessage(m_hwnd, IDC_USERMENU, BM_SETIMAGE, IMAGE_ICON, (LPARAM)m_hStatusIcon);
}

void CMsgDialog::UpdateFilterButton()
{
	CSuper::UpdateFilterButton();

	m_btnFilter.SendMsg(BM_SETIMAGE, IMAGE_ICON, (LPARAM)g_plugin.getIcon(m_bFilterEnabled ? IDI_FILTER : IDI_FILTER2));
	m_btnNickList.SendMsg(BM_SETIMAGE, IMAGE_ICON, (LPARAM)g_plugin.getIcon(m_bNicklistEnabled ? IDI_NICKLIST2 : IDI_NICKLIST));
}

void CMsgDialog::UpdateStatusBar()
{
	if (m_pParent->m_hwndActive != m_hwnd)
		return;

	if (isChat()) {
		wchar_t szTemp[512];
		mir_snwprintf(szTemp, L"%s : %s", m_si->pMI->ptszModDispName, m_si->ptszStatusbarText ? m_si->ptszStatusbarText : L"");

		StatusBarData sbd;
		sbd.iItem = 0;
		sbd.iFlags = SBDF_TEXT | SBDF_ICON;
		sbd.hIcon = m_si->wStatus == ID_STATUS_ONLINE ? m_si->pMI->hOnlineIcon : m_si->pMI->hOfflineIcon;
		sbd.pszText = szTemp;
		m_pParent->UpdateStatusBar(sbd, m_hwnd);

		sbd.iItem = 1;
		sbd.hIcon = nullptr;
		sbd.pszText = L"";
		m_pParent->UpdateStatusBar(sbd, m_hwnd);

		Srmm_SetIconFlags(m_hContact, SRMM_MODULE, 0, 0);
	}
	else {
		wchar_t szText[256];
		StatusBarData sbd = { 0 };
		sbd.iFlags = SBDF_TEXT | SBDF_ICON;
		if (m_iMessagesInProgress && g_dat.flags.bShowProgress) {
			sbd.hIcon = g_plugin.getIcon(IDI_TIMESTAMP);
			sbd.pszText = szText;
			mir_snwprintf(szText, TranslateT("Sending in progress: %d message(s) left..."), m_iMessagesInProgress);
		}
		else if (m_nTypeSecs) {
			sbd.hIcon = g_plugin.getIcon(IDI_TYPING);
			sbd.pszText = szText;
			mir_snwprintf(szText, TranslateT("%s is typing a message..."), 
				(m_pUserTyping) ? m_pUserTyping->pszNick : Clist_GetContactDisplayName(m_hContact));
			m_nTypeSecs--;
		}
		else if (m_lastMessage) {
			wchar_t date[64], time[64];
			TimeZone_PrintTimeStamp(nullptr, m_lastMessage, L"d", date, _countof(date), 0);
			TimeZone_PrintTimeStamp(nullptr, m_lastMessage, L"t", time, _countof(time), 0);
			mir_snwprintf(szText, TranslateT("Last message received on %s at %s."), date, time);
			sbd.pszText = szText;
		}
		else sbd.pszText = L"";
		m_pParent->UpdateStatusBar(sbd, m_hwnd);

		UpdateReadChars();

		Srmm_SetIconFlags(m_hContact, SRMM_MODULE, 0, MBF_DISABLED);

		if (IsTypingNotificationSupported() && g_dat.flags2.bShowTypingSwitch) {
			int mode = g_plugin.getByte(m_hContact, SRMSGSET_TYPING, g_plugin.bTypingNew);
			Srmm_SetIconFlags(m_hContact, SRMM_MODULE, 1, mode ? 0 : MBF_DISABLED);
		}
		else Srmm_SetIconFlags(m_hContact, SRMM_MODULE, 1, MBF_HIDDEN);
	}
}

void CMsgDialog::UpdateReadChars()
{
	if (m_pParent->m_hwndActive == m_hwnd) {
		wchar_t szText[256];
		int len = m_message.GetRichTextLength(1200);

		StatusBarData sbd;
		sbd.iItem = 1;
		sbd.iFlags = SBDF_TEXT | SBDF_ICON;
		sbd.hIcon = nullptr;
		sbd.pszText = szText;
		mir_snwprintf(szText, L"%d", len);
		m_pParent->UpdateStatusBar(sbd, m_hwnd);
	}
}

void CMsgDialog::UpdateTabControl()
{
	TabControlData tcd;
	tcd.iFlags = TCDF_TEXT | TCDF_ICON;
	tcd.hIcon = GetTabIcon();
	tcd.pszText = Clist_GetContactDisplayName(m_hContact);
	m_pParent->UpdateTabControl(tcd, m_hwnd);
}

void CMsgDialog::UserIsTyping(int iState)
{
	setTyping((iState > 0) ? iState : 0);
}

/////////////////////////////////////////////////////////////////////////////////////////

static const wchar_t *titleTokenNames[] = { L"%name%", L"%status%", L"%statusmsg%", L"%account%" };

void CMsgDialog::UpdateTitle()
{
	TitleBarData tbd = {};
	tbd.iFlags = TBDF_TEXT | TBDF_ICON;

	CMStringW wszTitle;
	if (isChat()) {
		tbd.hIcon = (m_si->wStatus == ID_STATUS_ONLINE) ? m_si->pMI->hOnlineIcon : m_si->pMI->hOfflineIcon;
		tbd.hIconBig = (m_si->wStatus == ID_STATUS_ONLINE) ? m_si->pMI->hOnlineIconBig : m_si->pMI->hOfflineIconBig;
		tbd.hIconNot = (m_si->wState & (GC_EVENT_HIGHLIGHT | STATE_TALK)) ? g_plugin.getIcon(IDI_OVERLAY) : nullptr;

		int nUsers = m_nickList.GetCount();
		switch (m_si->iType) {
		case GCW_CHATROOM:
			wszTitle.Format((nUsers == 1) ? TranslateT("%s: chat room (%u user)") : TranslateT("%s: chat room (%u users)"), m_si->ptszName, nUsers);
			break;
		case GCW_PRIVMESS:
			wszTitle.Format((nUsers == 1) ? TranslateT("%s: message session") : TranslateT("%s: message session (%u users)"), m_si->ptszName, nUsers);
			break;
		case GCW_SERVER:
			wszTitle.Format(L"%s: Server", m_si->ptszName);
			break;
		}
	}
	else {
		if (g_dat.wszTitleFormat[0])
			wszTitle = g_dat.wszTitleFormat;
		else
			wszTitle = L"%name% - ";

		if (m_hContact && m_szProto) {
			wszTitle.Replace(L"%name%", Clist_GetContactDisplayName(m_hContact));
			wszTitle.Replace(L"%status%", Clist_GetStatusModeDescription(db_get_w(m_hContact, m_szProto, "Status", ID_STATUS_OFFLINE), 0));

			CMStringW tszStatus = ptrW(db_get_wsa(m_hContact, "CList", "StatusMsg"));
			tszStatus.Replace(L"\r\n", L" ");
			wszTitle.Replace(L"%statusmsg%", tszStatus);

			char *accModule = Proto_GetBaseAccountName(m_hContact);
			if (accModule != nullptr) {
				PROTOACCOUNT *proto = Proto_GetAccount(accModule);
				if (proto != nullptr)
					wszTitle.Replace(L"%account%", proto->tszAccountName);
			}
		}

		if (g_dat.wszTitleFormat[0] == 0)
			wszTitle.Append(TranslateT("Message session"));

		GetTitlebarIcon(&tbd);
	}

	tbd.pszText = wszTitle.GetBuffer();
	m_pParent->UpdateTitleBar(tbd, m_hwnd);

	if (isChat())
		UpdateTabControl();
}
