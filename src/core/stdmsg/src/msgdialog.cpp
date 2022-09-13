/*

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
#include "statusicon.h"

#define TIMEOUT_FLASHWND     900
#define TIMEOUT_TYPEOFF      10000      //send type off after 10 seconds of inactivity
#define SB_CHAR_WIDTH        45
#define SB_TIME_WIDTH        60
#define SB_GRIP_WIDTH        20         // pixels - buffer used to prevent sizegrip from overwriting statusbar icons
#define VALID_AVATAR(x)      (x == PA_FORMAT_PNG || x == PA_FORMAT_JPEG || x == PA_FORMAT_ICON || x == PA_FORMAT_BMP || x == PA_FORMAT_GIF)

#define ENTERCLICKTIME   1000   //max time in ms during which a double-tap on enter will cause a send

LIST<CMsgDialog> g_arDialogs(10, PtrKeySortT);

/////////////////////////////////////////////////////////////////////////////////////////

CMsgDialog::CMsgDialog(CTabbedWindow *pOwner, MCONTACT hContact) :
	CSuper(g_plugin, IDD_MSG),
	m_avatar(this, IDC_AVATAR),
	m_splitterX(this, IDC_SPLITTERX),
	m_splitterY(this, IDC_SPLITTERY),
	m_cmdList(20),
	m_pOwner(pOwner)
{
	m_hContact = hContact;
	Init();
}

CMsgDialog::CMsgDialog(CTabbedWindow *pOwner, SESSION_INFO *si) :
	CSuper(g_plugin, IDD_MSG, si),
	m_avatar(this, IDC_AVATAR),
	m_splitterX(this, IDC_SPLITTERX),
	m_splitterY(this, IDC_SPLITTERY),
	m_cmdList(20),
	m_pOwner(pOwner)
{
	m_si->pDlg = this;

	m_iSplitterX = g_Settings.iSplitterX;
	m_iSplitterY = g_Settings.iSplitterY;

	m_btnOk.OnClick = Callback(this, &CMsgDialog::onClick_Ok);

	m_btnFilter.OnClick = Callback(this, &CMsgDialog::onClick_Filter);
	m_btnNickList.OnClick = Callback(this, &CMsgDialog::onClick_NickList);

	m_splitterX.OnChange = Callback(this, &CMsgDialog::onSplitterX);

	Init();
}

void CMsgDialog::Init()
{
	m_szTabSave[0] = 0;
	m_autoClose = 0;
	m_forceResizable = true;
	m_bNoActivate = g_plugin.bDoNotStealFocus;

	g_arDialogs.insert(this);

	m_btnOk.OnClick = Callback(this, &CMsgDialog::onClick_Ok);

	m_message.OnChange = Callback(this, &CMsgDialog::onChange_Text);

	m_splitterY.OnChange = Callback(this, &CMsgDialog::onSplitterY);

	timerFlash.OnEvent = Callback(this, &CMsgDialog::OnFlash);
	timerType.OnEvent = Callback(this, &CMsgDialog::OnType);
}

/////////////////////////////////////////////////////////////////////////////////////////

bool CMsgDialog::OnInitDialog()
{
	CSuper::OnInitDialog();

	m_szProto = Proto_GetBaseAccountName(m_hContact);
	m_bIsMeta = db_mc_isMeta(m_hContact) != 0;
	m_hTimeZone = TimeZone_CreateByContact(m_hContact, nullptr, TZF_KNOWNONLY);
	m_wMinute = 61;

	NotifyEvent(MSG_WINDOW_EVT_OPENING);
	if (m_wszInitialText) {
		m_message.SetText(m_wszInitialText);

		int len = GetWindowTextLength(m_message.GetHwnd());
		PostMessage(m_message.GetHwnd(), EM_SETSEL, len, len);
		mir_free(m_wszInitialText);
	}

	// avatar stuff
	m_avatar.Disable();
	m_limitAvatarH = g_plugin.bLimitAvatarHeight ? g_plugin.iAvatarHeight : 0;

	if (m_hContact && m_szProto != nullptr) {
		m_wStatus = db_get_w(m_hContact, m_szProto, "Status", ID_STATUS_OFFLINE);

		int nMax = CallProtoService(m_szProto, PS_GETCAPS, PFLAG_MAXLENOFMESSAGE, m_hContact);
		if (nMax)
			m_message.SendMsg(EM_EXLIMITTEXT, 0, nMax);
	}

	m_wOldStatus = m_wStatus;
	m_cmdListInd = -1;
	m_nTypeMode = PROTOTYPE_SELFTYPING_OFF;
	timerType.Start(1000);

	GetWindowRect(m_message.GetHwnd(), &m_minEditInit);
	m_iSplitterY = g_plugin.getDword(g_plugin.bSavePerContact ? m_hContact : 0, "splitterPos", m_minEditInit.bottom - m_minEditInit.top);
	UpdateSizeBar();

	m_message.SendMsg(EM_SETEVENTMASK, 0, ENM_MOUSEEVENTS | ENM_CHANGE);

	if (isChat()) {
		OnOptionsApplied(false);
		OnActivate();
		UpdateAvatar();
		UpdateOptions();
		UpdateStatusBar();
		UpdateTitle();

		NotifyEvent(MSG_WINDOW_EVT_OPEN);
	}
	else {
		m_nickList.Hide();
		m_splitterX.Hide();

		// This finds the first message to display, it works like shit
		m_hDbEventFirst = db_event_firstUnread(m_hContact);
		{
			DB::ECPTR pCursor(DB::EventsRev(m_hContact, m_hDbEventFirst));

			switch (g_plugin.iLoadHistory) {
			case LOADHISTORY_COUNT:
				for (int i = g_plugin.nLoadCount; i--;) {
					MEVENT hPrevEvent = pCursor.FetchNext();
					if (hPrevEvent == 0)
						break;

					DBEVENTINFO dbei = {};
					m_hDbEventFirst = hPrevEvent;
					db_event_get(hPrevEvent, &dbei);
					if (!DbEventIsShown(&dbei))
						i++;
				}
				break;

			case LOADHISTORY_TIME:
				DBEVENTINFO dbei = {};
				if (m_hDbEventFirst == 0)
					dbei.timestamp = (uint32_t)time(0);
				else
					db_event_get(m_hDbEventFirst, &dbei);

				uint32_t firstTime = dbei.timestamp - 60 * g_plugin.nLoadTime;
				while (MEVENT hPrevEvent = pCursor.FetchNext()) {
					dbei.cbBlob = 0;
					db_event_get(hPrevEvent, &dbei);
					if (dbei.timestamp < firstTime)
						break;
					m_hDbEventFirst = hPrevEvent;
				}
				break;
			}
		}

		bool bUpdate = false;
		DB::ECPTR pCursor(DB::EventsRev(m_hContact));
		while (MEVENT hdbEvent = pCursor.FetchNext()) {
			DBEVENTINFO dbei = {};
			db_event_get(hdbEvent, &dbei);
			if ((dbei.eventType == EVENTTYPE_MESSAGE) && !(dbei.flags & DBEF_SENT)) {
				m_lastMessage = dbei.timestamp;
				bUpdate = true;
				break;
			}
		}

		if (bUpdate)
			UpdateLastMessage();

		OnOptionsApplied(false);

		// restore saved msg if any...
		if (m_hContact) {
			DBVARIANT dbv;
			if (!db_get_ws(m_hContact, SRMSGMOD, DBSAVEDMSG, &dbv)) {
				if (dbv.pwszVal[0]) {
					m_message.SetText(dbv.pwszVal);
					m_btnOk.Enable(true);
					UpdateReadChars();
					PostMessage(m_message.GetHwnd(), EM_SETSEL, -1, -1);
				}
				db_free(&dbv);
			}
		}

		uint32_t dwFlags = SWP_NOMOVE | SWP_NOSIZE;
		if (!g_Settings.bTabsEnable)
			dwFlags |= SWP_SHOWWINDOW;

		if (m_bNoActivate) {
			SetWindowPos(m_hwnd, HWND_BOTTOM, 0, 0, 0, 0, dwFlags | SWP_NOACTIVATE);
			StartFlash();
		}
		else {
			SetWindowPos(m_hwnd, HWND_TOP, 0, 0, 0, 0, dwFlags);
			SetForegroundWindow(m_hwnd);
			SetFocus(m_message.GetHwnd());
		}

		UpdateAvatar();
	}

	NotifyEvent(MSG_WINDOW_EVT_OPEN);
	return true;
}

void CMsgDialog::OnDestroy()
{
	g_arDialogs.remove(this);

	NotifyEvent(MSG_WINDOW_EVT_CLOSING);

	// save string from the editor
	if (m_hContact) {
		ptrW msg(m_message.GetText());
		if (msg[0])
			db_set_ws(m_hContact, SRMSGMOD, DBSAVEDMSG, msg);
		else
			db_unset(m_hContact, SRMSGMOD, DBSAVEDMSG);
	}

	if (m_nTypeMode == PROTOTYPE_SELFTYPING_ON)
		NotifyTyping(PROTOTYPE_SELFTYPING_OFF);

	for (auto &it : m_cmdList)
		mir_free(it);
	m_cmdList.destroy();

	MCONTACT hContact = (g_plugin.bSavePerContact) ? m_hContact : 0;
	g_plugin.setDword(hContact ? m_hContact : 0, "splitterPos", m_iSplitterY);

	if (m_hFont) {
		DeleteObject(m_hFont);
		m_hFont = nullptr;
	}

	WINDOWPLACEMENT wp = { sizeof(wp) };
	GetWindowPlacement(m_hwnd, &wp);
	if (!m_bWindowCascaded) {
		g_plugin.setDword(hContact, "x", wp.rcNormalPosition.left);
		g_plugin.setDword(hContact, "y", wp.rcNormalPosition.top);
	}
	g_plugin.setDword(hContact, "width", wp.rcNormalPosition.right - wp.rcNormalPosition.left);
	g_plugin.setDword(hContact, "height", wp.rcNormalPosition.bottom - wp.rcNormalPosition.top);

	NotifyEvent(MSG_WINDOW_EVT_CLOSE);

	if (isChat()) {
		m_si->pDlg = nullptr;
		m_si->wState &= ~STATE_TALK;
	}		 

	Window_FreeIcon_IcoLib(m_hwnd);

	CSuper::OnDestroy();

	// a temporary contact should be destroyed after removing window from the window list to prevent recursion
	if (m_hContact && g_plugin.bDeleteTempCont)
		if (!Contact::OnList(m_hContact))
			db_delete_contact(m_hContact);
}

/////////////////////////////////////////////////////////////////////////////////////////

void CMsgDialog::OnActivate()
{
	UpdateTitle();
	StopFlash();

	if (isChat()) {
		g_chatApi.SetActiveSession(m_si);
		UpdateStatusBar();

		if (db_get_w(m_hContact, m_si->pszModule, "ApparentMode", 0) != 0)
			db_set_w(m_hContact, m_si->pszModule, "ApparentMode", 0);
		if (g_clistApi.pfnGetEvent(m_hContact, 0))
			g_clistApi.pfnRemoveEvent(m_hContact, GC_FAKE_EVENT);
	}
	else {
		SetupStatusBar();
		UpdateLastMessage();
		FixTabIcons();
	}
	
	SetFocus(m_message.GetHwnd());
}

void CMsgDialog::onClick_Filter(CCtrlButton *pButton)
{
	if (!pButton->Enabled())
		return;

	m_bFilterEnabled = !m_bFilterEnabled;
	pButton->SendMsg(BM_SETIMAGE, IMAGE_ICON, (LPARAM)g_plugin.getIcon(m_bFilterEnabled ? IDI_FILTER2 : IDI_FILTER, FALSE));
	if (m_bFilterEnabled && !g_chatApi.bRightClickFilter)
		ShowFilterMenu();
	else
		RedrawLog();
}

void CMsgDialog::onClick_NickList(CCtrlButton *pButton)
{
	if (!pButton->Enabled() || m_si->iType == GCW_SERVER)
		return;

	m_bNicklistEnabled = !m_bNicklistEnabled;
	pButton->SendMsg(BM_SETIMAGE, IMAGE_ICON, (LPARAM)g_plugin.getIcon(m_bNicklistEnabled ? IDI_NICKLIST2 : IDI_NICKLIST, FALSE));

	m_pLog->ScrollToBottom();
	Resize();
}

void CMsgDialog::onClick_Ok(CCtrlButton *pButton)
{
	if (!pButton->Enabled())
		return;

	ptrA msgText(m_message.GetRichTextRtf(true));
	if (msgText == nullptr)
		return;

	if (isChat()) {
		CMStringW ptszText(ptrW(mir_utf8decodeW(msgText)));
		g_chatApi.DoRtfToTags(ptszText, 0, nullptr);
		ptszText.Trim();

		m_cmdList.insert(mir_wstrdup(ptszText));
		m_cmdListInd = -1;

		ptszText.Replace(L"%", L"%%");

		if (m_si->pMI->bAckMsg) {
			m_message.Disable();
			m_message.SendMsg(EM_SETREADONLY, TRUE, 0);
		}
		else m_message.SetText(L"");

		Chat_DoEventHook(m_si, GC_USER_MESSAGE, nullptr, ptszText, 0);
	}
	else {
		ptrW temp(mir_utf8decodeW(msgText));
		if (!temp[0])
			return;

		int sendId = SendMessageDirect(rtrimw(temp), m_hContact);
		if (sendId) {
			m_cmdList.insert(temp.detach());
			m_cmdListInd = -1;

			if (m_nTypeMode == PROTOTYPE_SELFTYPING_ON)
				NotifyTyping(PROTOTYPE_SELFTYPING_OFF);

			m_message.SetText(L"");

			if (g_Settings.bTabsEnable) {
				if (g_plugin.bAutoClose) {
					m_pOwner->RemoveTab(this);
					m_autoClose = CLOSE_ON_OK;
				}
				else if (g_plugin.bAutoMin)
					::ShowWindow(GetParent(m_hwndParent), SW_MINIMIZE);
			}
			else {
				if (g_plugin.bAutoClose)
					::PostMessage(m_hwndParent, WM_CLOSE, 0, 0);
				else if (g_plugin.bAutoMin)
					::ShowWindow(m_hwndParent, SW_MINIMIZE);
			}
		}
	}

	m_btnOk.Disable();
	SetFocus(m_message.GetHwnd());
}

void CMsgDialog::onChange_Text(CCtrlEdit*)
{
	int len = GetWindowTextLength(m_message.GetHwnd());
	UpdateReadChars();
	m_btnOk.Enable(len != 0);
	if (!(GetKeyState(VK_CONTROL) & 0x8000) && !(GetKeyState(VK_SHIFT) & 0x8000)) {
		m_nLastTyping = GetTickCount();
		if (len) {
			if (m_nTypeMode == PROTOTYPE_SELFTYPING_OFF)
				NotifyTyping(PROTOTYPE_SELFTYPING_ON);
		}
		else if (m_nTypeMode == PROTOTYPE_SELFTYPING_ON)
			NotifyTyping(PROTOTYPE_SELFTYPING_OFF);
	}
}

void CMsgDialog::OnFlash(CTimer *)
{
	FixTabIcons();
	if (!g_plugin.nFlashMax || m_nFlash < 2 * g_plugin.nFlashMax)
		FlashWindow(m_pOwner->GetHwnd(), TRUE);
	m_nFlash++;
}

void CMsgDialog::OnType(CTimer*)
{
	ShowTime(false);
	if (m_nTypeMode == PROTOTYPE_SELFTYPING_ON && GetTickCount() - m_nLastTyping > TIMEOUT_TYPEOFF)
		NotifyTyping(PROTOTYPE_SELFTYPING_OFF);

	if (m_bShowTyping) {
		if (m_nTypeSecs) {
			m_nTypeSecs--;
			if (GetForegroundWindow() == m_pOwner->GetHwnd())
				FixTabIcons();
		}
		else {
			UpdateLastMessage();
			if (g_plugin.bShowTypingWin)
				FixTabIcons();
			m_bShowTyping = false;
		}
	}
	else {
		if (m_nTypeSecs) {
			HICON hTyping = Skin_LoadIcon(SKINICON_OTHER_TYPING);

			wchar_t szBuf[256];
			mir_snwprintf(szBuf, TranslateT("%s is typing a message..."),
				(m_pUserTyping) ? m_pUserTyping->pszNick : Clist_GetContactDisplayName(m_hContact));
			m_nTypeSecs--;

			SendMessage(m_pOwner->m_hwndStatus, SB_SETTEXT, 0, (LPARAM)szBuf);
			SendMessage(m_pOwner->m_hwndStatus, SB_SETICON, 0, (LPARAM)hTyping);
			if (g_plugin.bShowTypingWin && GetForegroundWindow() != m_pOwner->GetHwnd()) {
				HICON hIcon = (HICON)SendMessage(m_hwnd, WM_GETICON, ICON_SMALL, 0);
				SendMessage(m_hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hTyping);
				IcoLib_ReleaseIcon(hIcon);
			}
			m_bShowTyping = true;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

int CMsgDialog::Resizer(UTILRESIZECONTROL *urc)
{
	bool bToolbar = g_plugin.bShowButtons;
	bool bSend = g_plugin.bSendButton;
	bool bNick = false;
	int underTB = urc->dlgNewSize.cy - m_iSplitterY;
	underTB += bToolbar ? m_iBBarHeight : 2;

	if (isChat()) bNick = m_si->iType != GCW_SERVER && m_bNicklistEnabled;

	switch (urc->wId) {
	case IDOK:
		urc->rcItem.left = bSend ? urc->dlgNewSize.cx - 64 + 2 : urc->dlgNewSize.cx;
		urc->rcItem.right = urc->dlgNewSize.cx;
		urc->rcItem.top = underTB;
		urc->rcItem.bottom = urc->dlgNewSize.cy - 1;
		return RD_ANCHORX_CUSTOM | RD_ANCHORY_CUSTOM;

	case IDC_SRMM_LOG:
		urc->rcItem.top = 2;
		urc->rcItem.left = 0;
		urc->rcItem.right = bNick ? urc->dlgNewSize.cx - m_iSplitterX : urc->dlgNewSize.cx;
		urc->rcItem.bottom = urc->dlgNewSize.cy - m_iSplitterY;
		m_rcLog = urc->rcItem;
		return RD_ANCHORX_CUSTOM | RD_ANCHORY_CUSTOM;

	case IDC_SRMM_NICKLIST:
		urc->rcItem.top = 2;
		urc->rcItem.right = urc->dlgNewSize.cx;
		urc->rcItem.left = urc->dlgNewSize.cx - m_iSplitterX + 2;
LBL_CalcBottom:
		urc->rcItem.bottom = urc->dlgNewSize.cy - m_iSplitterY;
		return RD_ANCHORX_CUSTOM | RD_ANCHORY_CUSTOM;

	case IDC_SPLITTERX:
		urc->rcItem.top = 1;
		urc->rcItem.left = urc->dlgNewSize.cx - m_iSplitterX;
		urc->rcItem.right = urc->rcItem.left + 2;
		goto LBL_CalcBottom;

	case IDC_SPLITTERY:
		urc->rcItem.top = urc->dlgNewSize.cy - m_iSplitterY;
		urc->rcItem.bottom = urc->rcItem.top + 2;
		return RD_ANCHORX_WIDTH | RD_ANCHORY_CUSTOM;

	case IDC_SRMM_MESSAGE:
		urc->rcItem.right = bSend ? urc->dlgNewSize.cx - 64 : urc->dlgNewSize.cx;
		urc->rcItem.top = underTB;
		urc->rcItem.bottom = urc->dlgNewSize.cy - 1;
		if (g_plugin.bShowAvatar && m_avatarPic)
			urc->rcItem.left = m_avatarWidth + 4;
		return RD_ANCHORX_CUSTOM | RD_ANCHORY_CUSTOM;

	case IDC_AVATAR:
		urc->rcItem.top = underTB + (urc->dlgNewSize.cy - underTB - m_avatarHeight)/2;
		urc->rcItem.bottom = urc->rcItem.top + m_avatarHeight + 2;
		urc->rcItem.right = urc->rcItem.left + (m_avatarWidth + 2);
		return RD_ANCHORX_LEFT | RD_ANCHORY_CUSTOM;
	}

	return RD_ANCHORX_LEFT | RD_ANCHORY_TOP;
}

/////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CMsgDialog::DlgProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	RECT rc;

	switch (uMsg) {
	case WM_CONTEXTMENU:
		if (m_pOwner->m_hwndStatus == (HWND)wParam) {
			POINT pt, pt2;
			GetCursorPos(&pt);
			pt2.x = pt.x; pt2.y = pt.y;
			ScreenToClient(m_pOwner->m_hwndStatus, &pt);

			// no popup menu for status icons - this is handled via NM_RCLICK notification and the plugins that added the icons
			SendMessage(m_pOwner->m_hwndStatus, SB_GETRECT, SendMessage(m_pOwner->m_hwndStatus, SB_GETPARTS, 0, 0) - 1, (LPARAM)& rc);
			if (pt.x >= rc.left)
				break;

			HMENU hMenu = Menu_BuildContactMenu(m_hContact);
			TrackPopupMenu(hMenu, 0, pt2.x, pt2.y, 0, m_hwnd, nullptr);
			DestroyMenu(hMenu);
		}
		break;

	case WM_DROPFILES: // Mod from tabsrmm
		ProcessFileDrop((HDROP)wParam, m_hContact);
		return FALSE;

	case HM_AVATARACK:
		ShowAvatar();
		break;

	case DM_OPTIONSAPPLIED:
		OnOptionsApplied(wParam != 0);
		break;

	case DM_NEWTIMEZONE:
		m_hTimeZone = TimeZone_CreateByContact(m_hContact, nullptr, TZF_KNOWNONLY);
		m_wMinute = 61;
		Resize();
		break;

	case WM_CBD_LOADICONS:
		Srmm_UpdateToolbarIcons(m_hwnd);
		break;

	case WM_CBD_UPDATED:
		SetButtonsPos();
		break;

	case WM_CTLCOLORLISTBOX:
		SetBkColor((HDC)wParam, g_Settings.crUserListBGColor);
		return (INT_PTR)g_chatApi.hListBkgBrush;

	case WM_SIZE:
		if (!IsIconic(m_hwnd)) {
			if (isChat()) {
				m_btnOk.Show(g_plugin.bSendButton);
				m_splitterX.Show(m_si->iType != GCW_SERVER && m_bNicklistEnabled);
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
						m_btnChannelMgr.Enable(m_si->pMI->bChanMgr);
				}
			}
			else {
				SetupStatusBar();
			}

			CSuper::DlgProc(uMsg, wParam, lParam); // call built-in resizer
			SetButtonsPos();
			m_pLog->Resize();

			InvalidateRect(m_pOwner->m_hwndStatus, nullptr, true);
			RedrawWindow(m_message.GetHwnd(), nullptr, nullptr, RDW_INVALIDATE);
			RedrawWindow(m_btnOk.GetHwnd(), nullptr, nullptr, RDW_INVALIDATE);
			if (g_plugin.bShowAvatar && m_avatarPic)
				RedrawWindow(m_avatar.GetHwnd(), nullptr, nullptr, RDW_INVALIDATE);
		}
		return TRUE;

	case HM_DBEVENTADDED:
		if (wParam == m_hContact) {
			MEVENT hDbEvent = lParam;
			if (m_hDbEventFirst == 0)
				m_hDbEventFirst = hDbEvent;

			DBEVENTINFO dbei = {};
			db_event_get(hDbEvent, &dbei);
			bool isMessage = (dbei.eventType == EVENTTYPE_MESSAGE), isSent = ((dbei.flags & DBEF_SENT) != 0);
			bool isActive = IsActive();
			if (DbEventIsShown(&dbei)) {
				// Sounds *only* for sent messages, not for custom events
				if (isMessage && !isSent) {
					if (isActive)
						Skin_PlaySound("RecvMsgActive");
					else
						Skin_PlaySound("RecvMsgInactive");
				}
				if (isMessage && !isSent) {
					m_lastMessage = dbei.timestamp;
					UpdateLastMessage();
				}

				if (hDbEvent != m_hDbEventFirst && db_event_next(m_hContact, hDbEvent) == 0)
					m_pLog->LogEvents(hDbEvent, 1, 1);
				else
					RemakeLog();

				// Flash window *only* for messages, not for custom events
				if (isMessage && !isSent) {
					if (isActive) {
						if (m_pLog->AtBottom())
							StartFlash();
					}
					else StartFlash();
				}
			}
		}
		break;

	case WM_TIMECHANGE:
		PostMessage(m_hwnd, DM_NEWTIMEZONE, 0, 0);
		RemakeLog();
		break;

	case WM_MEASUREITEM:
		{
			LPMEASUREITEMSTRUCT mis = (LPMEASUREITEMSTRUCT)lParam;
			if (mis->CtlType == ODT_MENU)
				return Menu_MeasureItem(lParam);

			int ih = Chat_GetTextPixelSize(L"AQGgl'", g_Settings.UserListFont, FALSE);
			int ih2 = Chat_GetTextPixelSize(L"AQGg'", g_Settings.UserListHeadingsFont, FALSE);
			int font = ih > ih2 ? ih : ih2;
			int height = db_get_b(0, CHAT_MODULE, "NicklistRowDist", 12);

			// make sure we have space for icon!
			if (g_Settings.bShowContactStatus)
				font = font > 16 ? font : 16;

			mis->itemHeight = height > font ? height : font;
		}
		return TRUE;

	case WM_DRAWITEM:
		{
			LPDRAWITEMSTRUCT dis = (LPDRAWITEMSTRUCT)lParam;
			if (dis->CtlType == ODT_MENU)
				return Menu_DrawItem(lParam);

			if (dis->CtlID == IDC_AVATAR && m_avatarPic && g_plugin.bShowAvatar) {
				AVATARDRAWREQUEST adr = { sizeof(adr) };
				adr.hContact = m_hContact;
				adr.hTargetDC = dis->hDC;
				adr.rcDraw.right = m_avatarWidth;
				adr.rcDraw.bottom = m_avatarHeight;
				adr.dwFlags = AVDRQ_DRAWBORDER | AVDRQ_HIDEBORDERONTRANSPARENCY;
				CallService(MS_AV_DRAWAVATAR, 0, (LPARAM)&adr);
				return TRUE;
			}

			if (dis->CtlID == IDC_SRMM_NICKLIST) {
				int index = dis->itemID;
				USERINFO *ui = g_chatApi.SM_GetUserFromIndex(m_si->ptszID, m_si->pszModule, index);
				if (ui) {
					int x_offset = 2;

					int height = dis->rcItem.bottom - dis->rcItem.top;
					if (height & 1)
						height++;

					int offset = (height == 10) ? 0 : height / 2 - 4;
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
						x_offset += 18;
					}
					DrawIconEx(dis->hDC, x_offset, dis->rcItem.top + offset, g_chatApi.SM_GetStatusIcon(m_si, ui), 10, 10, 0, nullptr, DI_NORMAL);
					x_offset += 12;
					if (g_Settings.bShowContactStatus && !g_Settings.bContactStatusFirst && ui->ContactStatus) {
						HICON hIcon = Skin_LoadProtoIcon(m_si->pszModule, ui->ContactStatus);
						DrawIconEx(dis->hDC, x_offset, dis->rcItem.top + offset - 3, hIcon, 16, 16, 0, nullptr, DI_NORMAL);
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

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_USERMENU:
			if (GetKeyState(VK_SHIFT) & 0x8000) {    // copy user name
				ptrW id(Contact::GetInfo(CNF_UNIQUEID, m_hContact, m_szProto));
				if (id != nullptr && OpenClipboard(m_hwnd)) {
					HGLOBAL hData = GlobalAlloc(GMEM_MOVEABLE, mir_wstrlen(id) * sizeof(wchar_t) + 1);
					if (hData) {
						EmptyClipboard();
						mir_wstrcpy((wchar_t *)GlobalLock(hData), id);
						GlobalUnlock(hData);
						SetClipboardData(CF_UNICODETEXT, hData);
						CloseClipboard();
					}
				}
			}
			else {
				HMENU hMenu = Menu_BuildContactMenu(m_hContact);
				GetWindowRect(GetDlgItem(m_hwnd, LOWORD(wParam)), &rc);
				TrackPopupMenu(hMenu, 0, rc.left, rc.bottom, 0, m_hwnd, nullptr);
				DestroyMenu(hMenu);
			}
			break;

		case IDC_DETAILS:
			CallService(MS_USERINFO_SHOWDIALOG, m_hContact, 0);
			break;

		case IDC_ADD:
			Contact::Add(m_hContact, m_hwnd);

			if (Contact::OnList(m_hContact))
				ShowWindow(GetDlgItem(m_hwnd, IDC_ADD), FALSE);
			break;
		}
		break;

	case WM_NOTIFY:
		HCURSOR hCur;
		switch (((LPNMHDR)lParam)->idFrom) {
		case IDC_SRMM_LOG:
			switch (((LPNMHDR)lParam)->code) {
			case EN_MSGFILTER:
				switch (((MSGFILTER *)lParam)->msg) {
				case WM_LBUTTONDOWN:
					hCur = GetCursor();
					if (hCur == LoadCursor(nullptr, IDC_SIZENS) || hCur == LoadCursor(nullptr, IDC_SIZEWE) || hCur == LoadCursor(nullptr, IDC_SIZENESW) || hCur == LoadCursor(nullptr, IDC_SIZENWSE)) {
						SetWindowLongPtr(m_hwnd, DWLP_MSGRESULT, TRUE);
						return TRUE;
					}
					break;

				case WM_MOUSEMOVE:
					hCur = GetCursor();
					if (hCur == LoadCursor(nullptr, IDC_SIZENS) || hCur == LoadCursor(nullptr, IDC_SIZEWE) || hCur == LoadCursor(nullptr, IDC_SIZENESW) || hCur == LoadCursor(nullptr, IDC_SIZENWSE))
						SetCursor(LoadCursor(nullptr, IDC_ARROW));
					break;

				case WM_RBUTTONUP:
					SetWindowLongPtr(m_hwnd, DWLP_MSGRESULT, TRUE);
					return TRUE;
				}
				break;

			case EN_VSCROLL:
				if (LOWORD(wParam) == IDC_SRMM_LOG && GetWindowLongPtr((HWND)lParam, GWL_STYLE) & WS_VSCROLL) {
					SCROLLINFO si = {};
					si.cbSize = sizeof(si);
					si.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;
					GetScrollInfo((HWND)lParam, SB_VERT, &si);
					if ((si.nPos + (int)si.nPage + 5) >= si.nMax)
						StopFlash();
				}
			}
			break;

		case IDC_SRMM_MESSAGE:
			if (((LPNMHDR)lParam)->code == EN_MSGFILTER && ((MSGFILTER *)lParam)->msg == WM_RBUTTONUP) {
				SetWindowLongPtr(m_hwnd, DWLP_MSGRESULT, TRUE);
				return TRUE;
			}
			break;
		}
		break;

	case DM_UPDATETITLE:
		if (lParam != 0) {
			if (isChat()) {
				if ((MCONTACT)lParam != m_hContact)
					break;
			}
			else {
				bool bIsMe = ((MCONTACT)lParam == m_hContact) || (m_bIsMeta && db_mc_getMeta(lParam) == m_hContact);
				if (!bIsMe)
					break;
			}
		}
		UpdateIcon(wParam);
		UpdateTitle();
		break;

	case DM_STATUSICONCHANGE:
		SendMessage(m_pOwner->m_hwndStatus, SB_SETTEXT, (SBT_OWNERDRAW | (SendMessage(m_pOwner->m_hwndStatus, SB_GETPARTS, 0, 0) - 1)), 0);
		break;

	case WM_KEYDOWN:
		SetFocus(m_message.GetHwnd());
		break;

	case WM_LBUTTONDBLCLK:
		if (LOWORD(lParam) < 30)
			m_pLog->ScrollToBottom();
		break;

	case WM_ACTIVATE:
		if (LOWORD(wParam) != WA_ACTIVE)
			break;

		__fallthrough;

	case WM_MOUSEACTIVATE:
		OnActivate();
		break;
	}

	return CSuper::DlgProc(uMsg, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////////////////

LRESULT CMsgDialog::WndProc_Nicklist(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_KEYDOWN:
		if (wParam == 0x57 && GetKeyState(VK_CONTROL) & 0x8000) { // ctrl-w (close window)
			CloseTab();
			return TRUE;
		}
		break;
	}

	return CSuper::WndProc_Nicklist(msg, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////////////////

static const CHARRANGE rangeAll = { 0, -1 };

LRESULT CMsgDialog::WndProc_Message(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case EM_REPLACESEL:
		PostMessage(m_message.GetHwnd(), EM_ACTIVATE, 0, 0);
		break;

	case EM_ACTIVATE:
		SetActiveWindow(m_hwnd);
		break;

	case WM_DROPFILES:
		ProcessFileDrop((HDROP)wParam, m_hContact);
		return FALSE;

	case WM_SYSCHAR:
		if ((wParam == 's' || wParam == 'S') && GetKeyState(VK_MENU) & 0x8000) {
			m_btnOk.Click();
			return 0;
		}
		break;

	case WM_CONTEXTMENU:
		{
			MessageWindowPopupData mwpd = {};
			mwpd.uType = MSG_WINDOWPOPUP_SHOWING;
			mwpd.uFlags = MSG_WINDOWPOPUP_INPUT;
			mwpd.hContact = m_hContact;
			mwpd.hwnd = m_message.GetHwnd();

			HMENU hMenu = LoadMenu(g_plugin.getInst(), MAKEINTRESOURCE(IDR_CONTEXT));

			mwpd.hMenu = GetSubMenu(hMenu, 1);
			TranslateMenu(mwpd.hMenu);

			CHARRANGE sel;
			m_message.SendMsg(EM_EXGETSEL, 0, (LPARAM)&sel);
			if (sel.cpMin == sel.cpMax) {
				EnableMenuItem(mwpd.hMenu, IDM_CUT, MF_BYCOMMAND | MF_GRAYED);
				EnableMenuItem(mwpd.hMenu, IDM_COPY, MF_BYCOMMAND | MF_GRAYED);
				EnableMenuItem(mwpd.hMenu, IDM_DELETE, MF_BYCOMMAND | MF_GRAYED);
			}
			if (!m_message.SendMsg(EM_CANUNDO, 0, 0))
				EnableMenuItem(mwpd.hMenu, IDM_UNDO, MF_BYCOMMAND | MF_GRAYED);

			if (!m_message.SendMsg(EM_CANREDO, 0, 0))
				EnableMenuItem(mwpd.hMenu, IDM_REDO, MF_BYCOMMAND | MF_GRAYED);

			if (!m_message.SendMsg(EM_CANPASTE, 0, 0)) {
				if (!IsClipboardFormatAvailable(CF_HDROP))
					EnableMenuItem(mwpd.hMenu, IDM_PASTE, MF_BYCOMMAND | MF_GRAYED);
				EnableMenuItem(mwpd.hMenu, IDM_PASTESEND, MF_BYCOMMAND | MF_GRAYED);
			}

			if (lParam == 0xFFFFFFFF) {
				m_message.SendMsg(EM_POSFROMCHAR, (WPARAM)&mwpd.pt, sel.cpMax);
				ClientToScreen(m_message.GetHwnd(), &mwpd.pt);
			}
			else {
				mwpd.pt.x = GET_X_LPARAM(lParam);
				mwpd.pt.y = GET_Y_LPARAM(lParam);
			}

			// First notification
			NotifyEventHooks(g_chatApi.hevWinPopup, 0, (LPARAM)&mwpd);

			// Someone added items?
			if (GetMenuItemCount(mwpd.hMenu) > 0) {
				m_bInMenu = true;
				mwpd.selection = TrackPopupMenu(mwpd.hMenu, TPM_RETURNCMD, mwpd.pt.x, mwpd.pt.y, 0, m_message.GetHwnd(), nullptr);
				m_bInMenu = false;
			}

			// Second notification
			mwpd.uType = MSG_WINDOWPOPUP_SELECTED;
			NotifyEventHooks(g_chatApi.hevWinPopup, 0, (LPARAM)& mwpd);

			switch (mwpd.selection) {
			case IDM_UNDO:
				m_message.SendMsg(WM_UNDO, 0, 0);
				break;

			case IDM_REDO:
				m_message.SendMsg(EM_REDO, 0, 0);
				break;

			case IDM_CUT:
				m_message.SendMsg(WM_CUT, 0, 0);
				break;

			case IDM_COPY:
				m_message.SendMsg(WM_COPY, 0, 0);
				break;

			case IDM_PASTE:
				m_message.SendMsg(WM_PASTE, 0, 0);
				break;

			case IDM_PASTESEND:
				m_message.SendMsg(EM_PASTESPECIAL, CF_UNICODETEXT, 0);
				m_btnOk.Click();
				break;

			case IDM_DELETE:
				m_message.SendMsg(EM_REPLACESEL, TRUE, 0);
				break;

			case IDM_SELECTALL:
				m_message.SendMsg(EM_EXSETSEL, 0, (LPARAM)&rangeAll);
				break;

			case IDM_CLEAR:
				m_message.SetText(L"");
				break;
			}
			DestroyMenu(hMenu);
			return 0;
		}

	case EM_PASTESPECIAL:
	case WM_PASTE:
		if (IsClipboardFormatAvailable(CF_HDROP)) {
			if (OpenClipboard(m_message.GetHwnd())) {
				HANDLE hDrop = GetClipboardData(CF_HDROP);
				if (hDrop)
					ProcessFileDrop((HDROP)hDrop, m_hContact);
				CloseClipboard();
			}
			return 0;
		}
		break;

	case WM_KEYDOWN:
		bool isShift = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
		bool isCtrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
		bool isAlt = (GetKeyState(VK_MENU) & 0x8000) != 0;

		if (g_Settings.bTabsEnable) {
			if (wParam <= '9' && wParam >= '1' && isCtrl && !isAlt) { // CTRL + 1 -> 9 (switch tab)
				m_pOwner->SwitchTab(wParam - '1');
				return TRUE;
			}

			if (wParam <= VK_NUMPAD9 && wParam >= VK_NUMPAD1 && isCtrl && !isAlt) { // CTRL + 1 -> 9 (switch tab)
				m_pOwner->SwitchTab(wParam - VK_NUMPAD1);
				return TRUE;
			}

			if (wParam == VK_TAB && isCtrl && !isShift) { // CTRL-TAB (switch tab/window)
				m_pOwner->SwitchNextTab();
				return TRUE;
			}

			if (wParam == VK_TAB && isCtrl && isShift) { // CTRL_SHIFT-TAB (switch tab/window)
				m_pOwner->SwitchPrevTab();
				return TRUE;
			}

			if (wParam == 0x57 && isCtrl && !isAlt) { // ctrl-w (close window)
				CloseTab();
				return TRUE;
			}
		}

		if (isChat()) {
			if (wParam == 0x46 && isCtrl && !isAlt) { // ctrl-f (toggle filter)
				m_btnFilter.Click();
				return TRUE;
			}

			if (wParam == 0x4e && isCtrl && !isAlt) { // ctrl-n (nicklist)
				m_btnNickList.Click();
				return TRUE;
			}

			if (wParam == 0x4f && isCtrl && !isAlt) { // ctrl-o (options)
				m_btnChannelMgr.Click();
				return TRUE;
			}

			if (wParam == VK_TAB && isShift && !isCtrl) { // SHIFT-TAB (go to nick list)
				SetFocus(m_nickList.GetHwnd());
				return TRUE;
			}

			if (wParam == VK_TAB && !isCtrl && !isShift) { // tab-autocomplete
				TabAutoComplete();
				return 0;
			}

			if (m_szTabSave[0] != '\0' && wParam != VK_RIGHT && wParam != VK_LEFT && wParam != VK_SPACE && wParam != VK_RETURN && wParam != VK_BACK && wParam != VK_DELETE) {
				if (g_Settings.bAddColonToAutoComplete && m_iTabStart == 0)
					SendMessageA(m_message.GetHwnd(), EM_REPLACESEL, FALSE, (LPARAM) ": ");

				m_szTabSave[0] = '\0';
			}

			if (wParam == VK_NEXT || wParam == VK_PRIOR) {
				((CLogWindow *)m_pLog)->WndProc(msg, wParam, lParam);
				return TRUE;
			}
		}

		if (isCtrl && g_plugin.bCtrlSupport && m_cmdList.getCount()) {
			if (wParam == VK_UP && m_cmdListInd != 0) {
				if (m_cmdListInd < 0)
					m_cmdListInd = m_cmdList.getCount() - 1;
				else
					m_cmdListInd--;

				m_message.SetText(m_cmdList[m_cmdListInd]);
				m_message.SendMsg(EM_SETSEL, -1, -1);

				m_btnOk.Enable(GetWindowTextLength(m_message.GetHwnd()) != 0);
				UpdateReadChars();
				return 0;
			}

			if (wParam == VK_DOWN && m_cmdListInd != -1) {
				const wchar_t *pwszText;
				if (m_cmdListInd == m_cmdList.getCount() - 1) {
					m_cmdListInd = -1;
					pwszText = L"";
				}
				else {
					m_cmdListInd++;
					pwszText = m_cmdList[m_cmdListInd];
				}

				m_message.SetText(pwszText);
				m_message.SendMsg(EM_SETSEL, -1, -1);

				m_btnOk.Enable(GetWindowTextLength(m_message.GetHwnd()) != 0);
				UpdateReadChars();
				return 0;
			}
		}

		if (ProcessHotkeys(wParam, isShift, isCtrl, isAlt))
			return FALSE;
		break;
	}

	return CSuper::WndProc_Message(msg, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////////////////

int CMsgDialog::GetImageId() const
{
	if (m_nFlash & 1)
		return 0;

	return g_clistApi.pfnIconFromStatusMode(m_szProto, m_wStatus, m_hContact);
}

bool CMsgDialog::IsActive() const
{
	bool bRes = m_pOwner->IsActive();
	if (g_Settings.bTabsEnable && bRes)
		bRes &= m_pOwner->m_tab.GetActivePage() == this;

	return bRes;
}

void CMsgDialog::StartFlash()
{
	timerFlash.Start(1000);
}

void CMsgDialog::StopFlash()
{
	if (timerFlash.Stop()) {
		::FlashWindow(m_pOwner->GetHwnd(), FALSE);

		m_nFlash = 0;
		FixTabIcons();
	}
}

void CMsgDialog::TabAutoComplete()
{
	LRESULT lResult = (LRESULT)m_message.SendMsg(EM_GETSEL, 0, 0);

	m_message.SetDraw(false);
	m_iTabStart = LOWORD(lResult);
	int end = HIWORD(lResult);
	m_message.SendMsg(EM_SETSEL, end, end);

	GETTEXTLENGTHEX gtl = {};
	gtl.flags = GTL_PRECISE;
	gtl.codepage = CP_ACP;
	int iLen = m_message.SendMsg(EM_GETTEXTLENGTHEX, (WPARAM)& gtl, 0);
	if (iLen > 0) {
		wchar_t *pszText = (wchar_t *)mir_alloc(sizeof(wchar_t) * (iLen + 100));

		GETTEXTEX gt = {};
		gt.cb = iLen + 99;
		gt.flags = GT_DEFAULT;
		gt.codepage = 1200;
		m_message.SendMsg(EM_GETTEXTEX, (WPARAM)& gt, (LPARAM)pszText);

		while (m_iTabStart > 0 && pszText[m_iTabStart - 1] != ' ' && pszText[m_iTabStart - 1] != 13 && pszText[m_iTabStart - 1] != VK_TAB)
			m_iTabStart--;
		while (end < iLen && pszText[end] != ' ' && pszText[end] != 13 && pszText[end - 1] != VK_TAB)
			end++;

		if (m_szTabSave[0] == '\0')
			mir_wstrncpy(m_szTabSave, pszText + m_iTabStart, end - m_iTabStart + 1);

		wchar_t *pszSelName = (wchar_t *)mir_alloc(sizeof(wchar_t) * (end - m_iTabStart + 1));
		mir_wstrncpy(pszSelName, pszText + m_iTabStart, end - m_iTabStart + 1);

		wchar_t *pszName = g_chatApi.UM_FindUserAutoComplete(m_si, m_szTabSave, pszSelName);
		if (pszName == nullptr) {
			pszName = m_szTabSave;
			m_message.SendMsg(EM_SETSEL, m_iTabStart, end);
			if (end != m_iTabStart)
				m_message.SendMsg(EM_REPLACESEL, FALSE, (LPARAM)pszName);
			m_szTabSave[0] = '\0';
		}
		else {
			m_message.SendMsg(EM_SETSEL, m_iTabStart, end);
			if (end != m_iTabStart)
				m_message.SendMsg(EM_REPLACESEL, FALSE, (LPARAM)pszName);
		}
		mir_free(pszText);
		mir_free(pszSelName);
	}

	m_message.SetDraw(true);
	RedrawWindow(m_message.GetHwnd(), nullptr, nullptr, RDW_INVALIDATE);
}

/////////////////////////////////////////////////////////////////////////////////////////

void CMsgDialog::OnOptionsApplied(bool bUpdateAvatar)
{
	CustomButtonData *cbd;
	for (int i = 0; cbd = Srmm_GetNthButton(i); i++) {
		HWND hwndButton = GetDlgItem(m_hwnd, cbd->m_dwButtonCID);
		if (hwndButton == nullptr)
			continue;

		bool bShow = false;
		if (m_hContact && g_plugin.bShowButtons) {
			if (cbd->m_dwButtonCID == IDC_ADD) {
				bShow = !Contact::OnList(m_hContact);
				cbd->m_bHidden = !bShow;
			}
			else bShow = true;
		}
		ShowWindow(hwndButton, (bShow) ? SW_SHOW : SW_HIDE);
	}

	m_splitterY.Show();
	
	m_btnOk.Show(g_plugin.bSendButton);
	m_btnOk.Enable(GetWindowTextLength(m_message.GetHwnd()) != 0);
	
	if (m_avatarPic == nullptr || !g_plugin.bShowAvatar)
		m_avatar.Hide();
	
	UpdateIcon(0);
	UpdateTitle();
	Resize();

	m_pLog->UpdateOptions();
	m_message.SendMsg(EM_SETBKGNDCOLOR, 0, g_plugin.getDword(SRMSGSET_BKGCOLOUR, SRMSGDEFSET_BKGCOLOUR));

	// avatar stuff
	m_avatarPic = nullptr;
	m_limitAvatarH = 0;
	if (CallProtoService(m_szProto, PS_GETCAPS, PFLAGNUM_4, 0) & PF4_AVATARS)
		m_limitAvatarH = g_plugin.bLimitAvatarHeight ? g_plugin.iAvatarHeight : 0;

	if (bUpdateAvatar)
		UpdateAvatar();

	InvalidateRect(m_message.GetHwnd(), nullptr, FALSE);

	LOGFONT lf;
	CHARFORMAT cf = {};
	if (m_hFont)
		DeleteObject(m_hFont);
	LoadMsgDlgFont(MSGFONTID_MESSAGEAREA, &lf, &cf.crTextColor);
	m_hFont = CreateFontIndirect(&lf);
	m_message.SendMsg(WM_SETFONT, (WPARAM)m_hFont, MAKELPARAM(TRUE, 0));

	cf.cbSize = sizeof(CHARFORMAT);
	cf.dwMask = CFM_COLOR;
	m_message.SendMsg(EM_SETCHARFORMAT, SCF_ALL, (WPARAM)&cf);

	m_pLog->Clear();
	if (isChat())
		RedrawLog();
	else
		RemakeLog();

	FixTabIcons();
}

void CMsgDialog::onSplitterX(CSplitter *pSplitter)
{
	RECT rc;
	GetClientRect(m_hwnd, &rc);

	m_iSplitterX = rc.right - pSplitter->GetPos() + 1;
	if (m_iSplitterX < 35)
		m_iSplitterX = 35;
	if (m_iSplitterX > rc.right - rc.left - 35)
		m_iSplitterX = rc.right - rc.left - 35;
	g_Settings.iSplitterX = m_iSplitterX;
}

void CMsgDialog::onSplitterY(CSplitter *pSplitter)
{
	RECT rc;
	GetClientRect(m_hwnd, &rc);

	m_iSplitterY = rc.bottom - pSplitter->GetPos() + 1;

	int toplimit = 63;
	int min = m_minEditBoxSize.cy;
	if (g_plugin.bShowButtons)
		min += m_iBBarHeight;

	if (m_iSplitterY < min)
		m_iSplitterY = min;
	if (m_iSplitterY > rc.bottom - rc.top - toplimit)
		m_iSplitterY = rc.bottom - rc.top - toplimit;
	g_Settings.iSplitterY = m_iSplitterY;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CMsgDialog::CloseTab()
{
	if (g_Settings.bTabsEnable) {
		m_pOwner->RemoveTab(this);
		Close();
	}
	else SendMessage(m_hwndParent, WM_CLOSE, 0, 0);
}

void CMsgDialog::NotifyTyping(int mode)
{
	if (!m_hContact)
		return;

	// Don't send to protocols who don't support typing
	// Don't send to users who are unchecked in the typing notification options
	// Don't send to protocols that are offline
	// Don't send to users who are not visible and
	// Don't send to users who are not on the visible list when you are in invisible mode.
	if (!g_plugin.getByte(m_hContact, SRMSGSET_TYPING, g_plugin.bTypingNew))
		return;

	if (!m_szProto)
		return;

	uint32_t typeCaps = CallProtoService(m_szProto, PS_GETCAPS, PFLAGNUM_4, 0);
	if (!(typeCaps & PF4_SUPPORTTYPING))
		return;

	int protoStatus = Proto_GetStatus(m_szProto);
	if (protoStatus < ID_STATUS_ONLINE)
		return;

	if (isChat()) {
		m_nTypeMode = mode;
		Chat_DoEventHook(m_si, GC_USER_TYPNOTIFY, 0, 0, m_nTypeMode);
	}
	else {
		uint32_t protoCaps = CallProtoService(m_szProto, PS_GETCAPS, PFLAGNUM_1, 0);
		if (protoCaps & PF1_VISLIST && db_get_w(m_hContact, m_szProto, "ApparentMode", 0) == ID_STATUS_OFFLINE)
			return;

		if (protoCaps & PF1_INVISLIST && protoStatus == ID_STATUS_INVISIBLE && db_get_w(m_hContact, m_szProto, "ApparentMode", 0) != ID_STATUS_ONLINE)
			return;

		if (!g_plugin.bTypingUnknown && !Contact::OnList(m_hContact))
			return;

		// End user check
		m_nTypeMode = mode;
		CallService(MS_PROTO_SELFISTYPING, m_hContact, m_nTypeMode);
	}
}

void CMsgDialog::RemakeLog()
{
	m_pLog->LogEvents(m_hDbEventFirst, -1, false);
}

void CMsgDialog::ShowAvatar()
{
	if (g_plugin.bShowAvatar) {
		AVATARCACHEENTRY *ace = (AVATARCACHEENTRY *)CallService(MS_AV_GETAVATARBITMAP, getActiveContact(), 0);
		if (ace && (INT_PTR)ace != CALLSERVICE_NOTFOUND && (ace->dwFlags & AVS_BITMAP_VALID) && !(ace->dwFlags & AVS_HIDEONCLIST))
			m_avatarPic = ace->hbmPic;
		else
			m_avatarPic = nullptr;
	}
	else m_avatarPic = nullptr;

	UpdateSizeBar();
	Resize();
}

void CMsgDialog::ShowTime(bool bForce)
{
	if (!m_hTimeZone)
		return;

	SYSTEMTIME st;
	GetSystemTime(&st);
	if (m_wMinute != st.wMinute || bForce) {
		if (m_pOwner->m_tab.GetActivePage() == this) {
			wchar_t buf[32];
			unsigned i = g_plugin.bShowReadChar ? 2 : 1;

			TimeZone_PrintDateTime(m_hTimeZone, L"t", buf, _countof(buf), 0);
			SendMessage(m_pOwner->m_hwndStatus, SB_SETTEXT, i, (LPARAM)buf);
		}
		m_wMinute = st.wMinute;
	}
}

void CMsgDialog::SetupStatusBar()
{
	int i = 0, statwidths[4];
	int icons_width = GetStatusIconsCount(m_hContact) * (GetSystemMetrics(SM_CXSMICON) + 2) + SB_GRIP_WIDTH;

	RECT rc;
	GetWindowRect(m_pOwner->m_hwndStatus, &rc);
	int cx = rc.right - rc.left;

	if (m_hTimeZone) {
		if (g_plugin.bShowReadChar)
			statwidths[i++] = cx - SB_TIME_WIDTH - SB_CHAR_WIDTH - icons_width;
		statwidths[i++] = cx - SB_TIME_WIDTH - icons_width;
	}
	else if (g_plugin.bShowReadChar)
		statwidths[i++] = cx - SB_CHAR_WIDTH - icons_width;

	statwidths[i++] = cx - icons_width;
	statwidths[i++] = -1;
	SendMessage(m_pOwner->m_hwndStatus, SB_SETPARTS, i, (LPARAM)statwidths);

	UpdateReadChars();
	ShowTime(true);
	SendMessage(m_hwnd, DM_STATUSICONCHANGE, 0, 0);
}

void CMsgDialog::SetStatusText(const wchar_t *wszText, HICON hIcon)
{
	SendMessage(m_pOwner->m_hwndStatus, SB_SETICON, 0, (LPARAM)hIcon);
	SendMessage(m_pOwner->m_hwndStatus, SB_SETTEXT, 0, (LPARAM)(wszText == nullptr ? L"" : wszText));
}

void CMsgDialog::UpdateAvatar()
{
	PROTO_AVATAR_INFORMATION ai = {};
	ai.hContact = m_hContact;
	CallProtoService(m_szProto, PS_GETAVATARINFO, GAIF_FORCE, (LPARAM)&ai);

	ShowAvatar();
	SetWindowLongPtr(m_hwnd, DWLP_MSGRESULT, 1);
}

void CMsgDialog::UpdateIcon(WPARAM wParam)
{
	if (!m_hContact || !m_szProto)
		return;

	bool bIsStatus = false;
	DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING *)wParam;
	if (cws && !mir_strcmp(cws->szModule, m_szProto) && !mir_strcmp(cws->szSetting, "Status")) {
		bIsStatus = true;
		m_wStatus = cws->value.wVal;
	}

	if (!cws || bIsStatus)
		if (g_plugin.bUseStatusWinIcon)
			FixTabIcons();
}

void CMsgDialog::UpdateLastMessage()
{
	if (m_nTypeSecs)
		return;

	if (m_lastMessage) {
		wchar_t date[64], time[64], fmt[128];
		TimeZone_PrintTimeStamp(nullptr, m_lastMessage, L"d", date, _countof(date), 0);
		TimeZone_PrintTimeStamp(nullptr, m_lastMessage, L"t", time, _countof(time), 0);
		mir_snwprintf(fmt, TranslateT("Last message received on %s at %s."), date, time);
		SetStatusText(fmt, nullptr);
	}
	else SetStatusText(nullptr, nullptr);
}

void CMsgDialog::UpdateReadChars()
{
	if (g_plugin.bShowReadChar) {
		wchar_t buf[32];
		int len = GetWindowTextLength(m_message.GetHwnd());

		mir_snwprintf(buf, L"%d", len);
		SendMessage(m_pOwner->m_hwndStatus, SB_SETTEXT, 1, (LPARAM)buf);
	}
}

void CMsgDialog::UpdateSizeBar()
{
	m_minEditBoxSize.cx = m_minEditInit.right - m_minEditInit.left;
	m_minEditBoxSize.cy = m_minEditInit.bottom - m_minEditInit.top;
	if (g_plugin.bShowAvatar) {
		if (m_avatarPic == nullptr || !g_plugin.bShowAvatar) {
			m_avatarWidth = 50;
			m_avatarHeight = 50;
			m_avatar.Hide();
			return;
		}
		else {
			BITMAP bminfo;
			GetObject(m_avatarPic, sizeof(bminfo), &bminfo);
			m_avatarWidth = bminfo.bmWidth + 2;
			m_avatarHeight = bminfo.bmHeight + 2;
			if (m_limitAvatarH && m_avatarHeight > m_limitAvatarH) {
				m_avatarWidth = bminfo.bmWidth * m_limitAvatarH / bminfo.bmHeight + 2;
				m_avatarHeight = m_limitAvatarH + 2;
			}
			m_avatar.Show();
		}

		if (m_avatarPic && m_minEditBoxSize.cy <= m_avatarHeight) {
			m_minEditBoxSize.cy = m_avatarHeight;
			if (m_iSplitterY < m_minEditBoxSize.cy) {
				m_iSplitterY = m_minEditBoxSize.cy;
				Resize();
			}
		}
	}
}

void CMsgDialog::UpdateTitle()
{
	wchar_t newtitle[256];
	if (isChat()) {
		int nUsers = m_si->getUserList().getCount();

		switch (m_si->iType) {
		case GCW_CHATROOM:
			mir_snwprintf(newtitle,
				(nUsers == 1) ? TranslateT("%s: chat room (%u user)") : TranslateT("%s: chat room (%u users)"),
				m_si->ptszName, nUsers);
			break;
		case GCW_PRIVMESS:
			mir_snwprintf(newtitle,
				(nUsers == 1) ? TranslateT("%s: message session") : TranslateT("%s: message session (%u users)"),
				m_si->ptszName, nUsers);
			break;
		case GCW_SERVER:
			mir_snwprintf(newtitle, L"%s: Server", m_si->ptszName);
			break;
		}
	}
	else {
		if (m_hContact && m_szProto) {
			m_wStatus = db_get_w(m_hContact, m_szProto, "Status", ID_STATUS_OFFLINE);
			wchar_t *contactName = Clist_GetContactDisplayName(m_hContact);

			if (g_plugin.bUseStatusWinIcon)
				mir_snwprintf(newtitle, L"%s - %s", contactName, TranslateT("Message session"));
			else {
				wchar_t *szStatus = Clist_GetStatusModeDescription(m_szProto == nullptr ? ID_STATUS_OFFLINE : db_get_w(m_hContact, m_szProto, "Status", ID_STATUS_OFFLINE), 0);
				mir_snwprintf(newtitle, L"%s (%s): %s", contactName, szStatus, TranslateT("Message session"));
			}

			m_wOldStatus = m_wStatus;
		}
		else wcsncpy_s(newtitle, TranslateT("Message session"), _TRUNCATE);
	}

	if (this == m_pOwner->CurrPage()) {
		wchar_t oldtitle[256];
		GetWindowText(m_pOwner->GetHwnd(), oldtitle, _countof(oldtitle));
		if (mir_wstrcmp(newtitle, oldtitle)) //swt() flickers even if the title hasn't actually changed
			SetWindowText(m_pOwner->GetHwnd(), newtitle);
	}

	if (!isChat()) {
		int idx = m_pOwner->m_tab.GetDlgIndex(this);
		if (idx == -1)
			return;

		auto *pwszName = Clist_GetContactDisplayName(m_hContact);
		wchar_t oldtitle[256];

		TCITEM ti;
		ti.mask = TCIF_TEXT;
		ti.pszText = oldtitle;
		ti.cchTextMax = _countof(oldtitle);
		TabCtrl_GetItem(m_pOwner->m_tab.GetHwnd(), idx, &ti);

		// change text only if it was changed
		if (mir_wstrcmp(pwszName, oldtitle)) {
			ti.pszText = pwszName;
			ti.cchTextMax = 0;
			TabCtrl_SetItem(m_pOwner->m_tab.GetHwnd(), idx, &ti);
		}
	}
}

void CMsgDialog::UserTyping(int nSecs)
{
	setTyping((nSecs > 0) ? nSecs : 0);
}
