/////////////////////////////////////////////////////////////////////////////////////////
// Miranda NG: the free IM client for Microsoft* Windows*
//
// Copyright (c) 2012-18 Miranda NG team,
// Copyright (c) 2000-09 Miranda ICQ/IM project,
// all portions of this codebase are copyrighted to the people
// listed in contributors.txt.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// you should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// part of tabSRMM messaging plugin for Miranda.
//
// (C) 2005-2010 by silvercircle _at_ gmail _dot_ com and contributors
//
// implements the message dialog window.

#include "stdafx.h"

#define MS_HTTPSERVER_ADDFILENAME "HTTPServer/AddFileName"

bool IsStringValidLink(wchar_t *pszText);

static const UINT sendControls[] = { IDC_SRMM_MESSAGE, IDC_SRMM_LOG };
static const UINT formatControls[] = { IDC_SRMM_BOLD, IDC_SRMM_ITALICS, IDC_SRMM_UNDERLINE, IDC_FONTSTRIKEOUT };
static const UINT addControls[] = { IDC_ADD, IDC_CANCELADD };
static const UINT btnControls[] = { IDC_RETRY, IDC_CANCELSEND, IDC_MSGSENDLATER, IDC_ADD, IDC_CANCELADD };
static const UINT errorControls[] = { IDC_STATICERRORICON, IDC_STATICTEXT, IDC_RETRY, IDC_CANCELSEND, IDC_MSGSENDLATER };

struct
{
	int id;
	const wchar_t* text;
}
static tooltips[] =
{
	{ IDC_ADD, LPGENW("Add this contact permanently to your contact list") },
	{ IDC_CANCELADD, LPGENW("Do not add this contact permanently") },
	{ IDC_TOGGLESIDEBAR, LPGENW("Expand or collapse the side bar") }
};

struct
{
	int id;
	HICON *pIcon;
}
static buttonicons[] =
{
	{ IDC_ADD, &PluginConfig.g_buttonBarIcons[ICON_BUTTON_ADD] },
	{ IDC_CANCELADD, &PluginConfig.g_buttonBarIcons[ICON_BUTTON_CANCEL] }
};

static void _clrMsgFilter(MSGFILTER *m)
{
	m->msg = 0;
	m->lParam = 0;
	m->wParam = 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// show a modified context menu for the richedit control(s)

void CTabBaseDlg::ShowPopupMenu(const CCtrlBase &pCtrl, POINT pt)
{
	CHARRANGE sel, all = { 0, -1 };

	HMENU hSubMenu, hMenu = LoadMenu(g_plugin.getInst(), MAKEINTRESOURCE(IDR_CONTEXT));
	if (pCtrl.GetCtrlId() == IDC_SRMM_LOG)
		hSubMenu = GetSubMenu(hMenu, 0);
	else {
		hSubMenu = GetSubMenu(hMenu, 2);
		EnableMenuItem(hSubMenu, IDM_PASTEFORMATTED, MF_BYCOMMAND | (m_SendFormat != 0 ? MF_ENABLED : MF_GRAYED));
		EnableMenuItem(hSubMenu, ID_EDITOR_PASTEANDSENDIMMEDIATELY, MF_BYCOMMAND | (PluginConfig.m_PasteAndSend ? MF_ENABLED : MF_GRAYED));
		CheckMenuItem(hSubMenu, ID_EDITOR_SHOWMESSAGELENGTHINDICATOR, MF_BYCOMMAND | (PluginConfig.m_visualMessageSizeIndicator ? MF_CHECKED : MF_UNCHECKED));
		EnableMenuItem(hSubMenu, ID_EDITOR_SHOWMESSAGELENGTHINDICATOR, MF_BYCOMMAND | (m_pContainer->hwndStatus ? MF_ENABLED : MF_GRAYED));
	}
	TranslateMenu(hSubMenu);
	pCtrl.SendMsg(EM_EXGETSEL, 0, (LPARAM)&sel);
	if (sel.cpMin == sel.cpMax) {
		EnableMenuItem(hSubMenu, IDM_COPY, MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(hSubMenu, IDM_QUOTE, MF_BYCOMMAND | MF_GRAYED);
		if (pCtrl.GetCtrlId() == IDC_SRMM_MESSAGE)
			EnableMenuItem(hSubMenu, IDM_CUT, MF_BYCOMMAND | MF_GRAYED);
	}

	if (pCtrl.GetCtrlId() == IDC_SRMM_LOG) {
		InsertMenuA(hSubMenu, 6, MF_BYPOSITION | MF_SEPARATOR, 0, nullptr);
		CheckMenuItem(hSubMenu, ID_LOG_FREEZELOG, MF_BYCOMMAND | (m_dwFlagsEx & MWF_SHOW_SCROLLINGDISABLED ? MF_CHECKED : MF_UNCHECKED));
	}

	MessageWindowPopupData mwpd;
	// First notification
	mwpd.uType = MSG_WINDOWPOPUP_SHOWING;
	mwpd.uFlags = (pCtrl.GetCtrlId() == IDC_SRMM_LOG ? MSG_WINDOWPOPUP_LOG : MSG_WINDOWPOPUP_INPUT);
	mwpd.hContact = m_hContact;
	mwpd.hwnd = pCtrl.GetHwnd();
	mwpd.hMenu = hSubMenu;
	mwpd.selection = 0;
	mwpd.pt = pt;
	NotifyEventHooks(g_chatApi.hevWinPopup, 0, (LPARAM)&mwpd);

	int iSelection = TrackPopupMenu(hSubMenu, TPM_RETURNCMD, pt.x, pt.y, 0, m_hwnd, nullptr);

	// Second notification
	mwpd.selection = iSelection;
	mwpd.uType = MSG_WINDOWPOPUP_SELECTED;
	NotifyEventHooks(g_chatApi.hevWinPopup, 0, (LPARAM)&mwpd);

	switch (iSelection) {
	case IDM_COPY:
		pCtrl.SendMsg(WM_COPY, 0, 0);
		break;
	case IDM_CUT:
		pCtrl.SendMsg(WM_CUT, 0, 0);
		break;
	case IDM_PASTE:
	case IDM_PASTEFORMATTED:
		if (pCtrl.GetCtrlId() == IDC_SRMM_MESSAGE)
			pCtrl.SendMsg(EM_PASTESPECIAL, (iSelection == IDM_PASTE) ? CF_UNICODETEXT : 0, 0);
		break;
	case IDM_COPYALL:
		pCtrl.SendMsg(EM_EXSETSEL, 0, (LPARAM)&all);
		pCtrl.SendMsg(WM_COPY, 0, 0);
		pCtrl.SendMsg(EM_EXSETSEL, 0, (LPARAM)&sel);
		break;
	case IDM_QUOTE:
		SendMessage(m_hwnd, WM_COMMAND, IDC_QUOTE, 0);
		break;
	case IDM_SELECTALL:
		pCtrl.SendMsg(EM_EXSETSEL, 0, (LPARAM)&all);
		break;
	case IDM_CLEAR:
		tabClearLog();
		break;
	case ID_LOG_FREEZELOG:
		SendDlgItemMessage(m_hwnd, IDC_SRMM_LOG, WM_KEYDOWN, VK_F12, 0);
		break;
	case ID_EDITOR_SHOWMESSAGELENGTHINDICATOR:
		PluginConfig.m_visualMessageSizeIndicator = !PluginConfig.m_visualMessageSizeIndicator;
		db_set_b(0, SRMSGMOD_T, "msgsizebar", (BYTE)PluginConfig.m_visualMessageSizeIndicator);
		Srmm_Broadcast(DM_CONFIGURETOOLBAR, 0, 0);
		Resize();
		if (m_pContainer->hwndStatus)
			RedrawWindow(m_pContainer->hwndStatus, nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW);
		break;
	case ID_EDITOR_PASTEANDSENDIMMEDIATELY:
		HandlePasteAndSend();
		break;
	}

	if (pCtrl.GetCtrlId() == IDC_SRMM_LOG)
		RemoveMenu(hSubMenu, 7, MF_BYPOSITION);
	DestroyMenu(hMenu);
}

void CTabBaseDlg::ResizeIeView()
{
	RECT rcRichEdit;
	GetWindowRect(m_log.GetHwnd(), &rcRichEdit);

	POINT pt = { rcRichEdit.left, rcRichEdit.top };
	ScreenToClient(m_hwnd, &pt);

	IEVIEWWINDOW ieWindow = { sizeof(ieWindow) };
	ieWindow.iType = IEW_SETPOS;
	ieWindow.parent = m_hwnd;
	ieWindow.hwnd = m_hwndIEView ? m_hwndIEView : m_hwndHPP;
	ieWindow.x = pt.x;
	ieWindow.y = pt.y;
	ieWindow.cx = rcRichEdit.right - rcRichEdit.left;
	ieWindow.cy = rcRichEdit.bottom - rcRichEdit.top;
	if (ieWindow.cx != 0 && ieWindow.cy != 0)
		CallService(m_hwndIEView ? MS_IEVIEW_WINDOW : MS_HPP_EG_WINDOW, 0, (LPARAM)&ieWindow);
}

LRESULT CALLBACK IEViewSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CSrmmWindow *mwdat = (CSrmmWindow*)GetWindowLongPtr(GetParent(hwnd), GWLP_USERDATA);

	switch (msg) {
	case WM_NCCALCSIZE:
		return CSkin::NcCalcRichEditFrame(hwnd, mwdat, ID_EXTBKHISTORY, msg, wParam, lParam, IEViewSubclassProc);
	case WM_NCPAINT:
		return CSkin::DrawRichEditFrame(hwnd, mwdat, ID_EXTBKHISTORY, msg, wParam, lParam, IEViewSubclassProc);
	}
	return mir_callNextSubclass(hwnd, IEViewSubclassProc, msg, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////////////////
// sublassing procedure for the h++ based message log viewer

LRESULT CALLBACK HPPKFSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CSrmmWindow *mwdat = (CSrmmWindow*)GetWindowLongPtr(GetParent(hwnd), GWLP_USERDATA);
	if (mwdat) {
		bool isCtrl, isShift, isAlt;
		mwdat->KbdState(isShift, isCtrl, isAlt);

		switch (msg) {
		case WM_NCCALCSIZE:
			return CSkin::NcCalcRichEditFrame(hwnd, mwdat, ID_EXTBKHISTORY, msg, wParam, lParam, HPPKFSubclassProc);
		case WM_NCPAINT:
			return CSkin::DrawRichEditFrame(hwnd, mwdat, ID_EXTBKHISTORY, msg, wParam, lParam, HPPKFSubclassProc);

		case WM_KEYDOWN:
			if (!isCtrl && !isAlt && !isShift) {
				if (wParam != VK_PRIOR && wParam != VK_NEXT && wParam != VK_DELETE &&
					wParam != VK_MENU  && wParam != VK_END  && wParam != VK_HOME &&
					wParam != VK_UP    && wParam != VK_DOWN && wParam != VK_LEFT &&
					wParam != VK_RIGHT && wParam != VK_TAB  && wParam != VK_SPACE) {
					SetFocus(GetDlgItem(mwdat->GetHwnd(), IDC_SRMM_MESSAGE));
					keybd_event((BYTE)wParam, (BYTE)MapVirtualKey(wParam, 0), KEYEVENTF_EXTENDEDKEY | 0, 0);
					return 0;
				}
				break;
			}
		}
	}
	return mir_callNextSubclass(hwnd, HPPKFSubclassProc, msg, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////////////////
// update state of the container - this is called whenever a tab becomes active, no matter how and
// deals with various things like updating the title bar, removing flashing icons, updating the
// session list, switching the keyboard layout (autolocale active)  and the general container status.
//
// it protects itself from being called more than once per session activation and is valid for
// normal IM sessions *only*. Group chat sessions have their own activation handler (see chat/window.c)

void CSrmmWindow::MsgWindowUpdateState(UINT msg)
{
	if (m_iTabID < 0)
		return;

	if (msg == WM_ACTIVATE) {
		if (m_pContainer->dwFlags & CNT_TRANSPARENCY) {
			DWORD trans = LOWORD(m_pContainer->settings->dwTransparency);
			SetLayeredWindowAttributes(m_pContainer->m_hwnd, 0, (BYTE)trans, (m_pContainer->dwFlags & CNT_TRANSPARENCY ? LWA_ALPHA : 0));
		}
	}

	if (m_bIsAutosizingInput && m_iInputAreaHeight == -1) {
		m_iInputAreaHeight = 0;
		m_message.SendMsg(EM_REQUESTRESIZE, 0, 0);
	}

	if (m_pWnd)
		m_pWnd->activateTab();
	m_pPanel.dismissConfig();
	m_dwUnread = 0;
	if (m_pContainer->hwndSaved == m_hwnd)
		return;

	m_pContainer->hwndSaved = m_hwnd;

	m_dwTickLastEvent = 0;
	m_dwFlags &= ~MWF_DIVIDERSET;
	if (KillTimer(m_hwnd, TIMERID_FLASHWND)) {
		FlashTab(false);
		m_bCanFlashTab = false;
	}
	if (m_pContainer->dwFlashingStarted != 0) {
		FlashContainer(m_pContainer, 0, 0);
		m_pContainer->dwFlashingStarted = 0;
	}
	if (m_dwFlagsEx & MWF_SHOW_FLASHCLIST) {
		m_dwFlagsEx &= ~MWF_SHOW_FLASHCLIST;
		if (m_hFlashingEvent != 0)
			g_clistApi.pfnRemoveEvent(m_hContact, m_hFlashingEvent);
		m_hFlashingEvent = 0;
	}
	m_pContainer->dwFlags &= ~CNT_NEED_UPDATETITLE;

	if ((m_dwFlags & MWF_DEFERREDREMAKELOG) && !IsIconic(m_pContainer->m_hwnd)) {
		SendMessage(m_hwnd, DM_REMAKELOG, 0, 0);
		m_dwFlags &= ~MWF_DEFERREDREMAKELOG;
	}

	if (m_dwFlags & MWF_NEEDCHECKSIZE)
		PostMessage(m_hwnd, DM_SAVESIZE, 0, 0);

	m_pContainer->hIconTaskbarOverlay = nullptr;
	m_pContainer->UpdateTitle(m_hContact);

	tabUpdateStatusBar();
	m_dwLastActivity = GetTickCount();
	m_pContainer->dwLastActivity = m_dwLastActivity;

	m_pContainer->MenuBar->configureMenu();
	UpdateTrayMenuState(this, FALSE);

	if (m_pContainer->m_hwndActive == m_hwnd)
		DeletePopupsForContact(m_hContact, PU_REMOVE_ON_FOCUS);

	m_pPanel.Invalidate();

	if (m_dwFlags & MWF_DEFERREDSCROLL && m_hwndIEView == nullptr && m_hwndHPP == nullptr) {
		m_dwFlags &= ~MWF_DEFERREDSCROLL;
		DM_ScrollToBottom(0, 1);
	}

	DM_SetDBButtonStates();

	if (m_hwndIEView) {
		RECT rcRTF;
		POINT pt;

		GetWindowRect(m_log.GetHwnd(), &rcRTF);
		rcRTF.left += 20;
		rcRTF.top += 20;
		pt.x = rcRTF.left;
		pt.y = rcRTF.top;
		if (m_hwndIEView) {
			if (M.GetByte("subclassIEView", 0)) {
				mir_subclassWindow(m_hwndIEView, IEViewSubclassProc);
				SetWindowPos(m_hwndIEView, nullptr, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_DRAWFRAME);
				RedrawWindow(m_hwndIEView, nullptr, nullptr, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW);
			}
		}
		m_hwndIWebBrowserControl = WindowFromPoint(pt);
	}

	if (m_dwFlagsEx & MWF_EX_DELAYEDSPLITTER) {
		m_dwFlagsEx &= ~MWF_EX_DELAYEDSPLITTER;
		ShowWindow(m_pContainer->m_hwnd, SW_RESTORE);
		PostMessage(m_hwnd, DM_SPLITTERGLOBALEVENT, m_wParam, m_lParam);
		m_wParam = m_lParam = 0;
	}
	if (m_dwFlagsEx & MWF_EX_AVATARCHANGED) {
		m_dwFlagsEx &= ~MWF_EX_AVATARCHANGED;
		PostMessage(m_hwnd, DM_UPDATEPICLAYOUT, 0, 0);
	}
	BB_SetButtonsPos();
	if (M.isAero())
		InvalidateRect(m_hwndParent, nullptr, FALSE);
	if (m_pContainer->dwFlags & CNT_SIDEBAR)
		m_pContainer->SideBar->setActiveItem(this);

	if (m_pWnd)
		m_pWnd->Invalidate();
}

static void ShowMultipleControls(HWND hwndDlg, const UINT *controls, int cControls, int state)
{
	for (int i = 0; i < cControls; i++)
		Utils::showDlgControl(hwndDlg, controls[i], state);
}

void CTabBaseDlg::SetDialogToType()
{
	if (m_hContact) {
		if (db_get_b(m_hContact, "CList", "NotOnList", 0)) {
			m_bNotOnList = true;
			ShowMultipleControls(m_hwnd, addControls, _countof(addControls), SW_SHOW);
			Utils::showDlgControl(m_hwnd, IDC_LOGFROZENTEXT, SW_SHOW);
			SetDlgItemText(m_hwnd, IDC_LOGFROZENTEXT, TranslateT("Contact not on list. You may add it..."));
		}
		else {
			ShowMultipleControls(m_hwnd, addControls, _countof(addControls), SW_HIDE);
			m_bNotOnList = false;
			Utils::showDlgControl(m_hwnd, IDC_LOGFROZENTEXT, SW_HIDE);
		}
	}

	Utils::enableDlgControl(m_hwnd, IDC_TIME, true);

	if (m_hwndIEView || m_hwndHPP) {
		m_log.Hide();
		m_log.Enable(false);
		m_message.Show();
	}
	else ShowMultipleControls(m_hwnd, sendControls, _countof(sendControls), SW_SHOW);

	ShowMultipleControls(m_hwnd, errorControls, _countof(errorControls), m_dwFlags & MWF_ERRORSTATE ? SW_SHOW : SW_HIDE);

	if (!m_SendFormat)
		ShowMultipleControls(m_hwnd, formatControls, _countof(formatControls), SW_HIDE);

	if (m_pContainer->m_hwndActive == m_hwnd)
		UpdateReadChars();

	SetDlgItemText(m_hwnd, IDC_STATICTEXT, TranslateT("A message failed to send successfully."));

	DM_RecalcPictureSize();
	GetAvatarVisibility();

	Utils::showDlgControl(m_hwnd, IDC_CONTACTPIC, m_bShowAvatar ? SW_SHOW : SW_HIDE);
	Utils::showDlgControl(m_hwnd, IDC_SPLITTERY, m_bIsAutosizingInput ? SW_HIDE : SW_SHOW);
	Utils::showDlgControl(m_hwnd, IDC_MULTISPLITTER, (m_sendMode & SMODE_MULTIPLE) ? SW_SHOW : SW_HIDE);

	EnableSendButton(GetWindowTextLength(m_message.GetHwnd()) != 0);
	UpdateTitle();
	Resize();

	Utils::enableDlgControl(m_hwnd, IDC_CONTACTPIC, false);

	m_pPanel.Configure();
}

/////////////////////////////////////////////////////////////////////////////////////////
// subclasses the avatar display controls, needed for skinning and to prevent
// it from flickering during resize/move operations.

static LRESULT CALLBACK AvatarSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_ERASEBKGND:
		return TRUE;

	case WM_UPDATEUISTATE:
		return TRUE;

	case WM_NOTIFY:
		ProcessAvatarChange(hwnd, lParam);
		break;
	}
	return mir_callNextSubclass(hwnd, AvatarSubclassProc, msg, wParam, lParam);
}

LRESULT CALLBACK SplitterSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	RECT rc;
	HWND hwndParent = GetParent(hwnd);
	CTabBaseDlg *dat = (CTabBaseDlg*)GetWindowLongPtr(hwndParent, GWLP_USERDATA);

	switch (msg) {
	case WM_NCHITTEST:
		return HTCLIENT;

	case WM_SETCURSOR:
		GetClientRect(hwnd, &rc);
		SetCursor(rc.right > rc.bottom ? PluginConfig.hCurSplitNS : PluginConfig.hCurSplitWE);
		return TRUE;

	case WM_LBUTTONDOWN:
		if (hwnd == GetDlgItem(hwndParent, IDC_SPLITTERY)) {
			GetClientRect(hwnd, &rc);
			dat->m_savedSplitterY = dat->m_iSplitterY;
			dat->m_savedDynaSplit = dat->m_dynaSplitter;
		}
		SetCapture(hwnd);
		return 0;

	case WM_MOUSEMOVE:
		if (GetCapture() == hwnd) {
			GetClientRect(hwnd, &rc);
			SendMessage(hwndParent, DM_SPLITTERMOVED, rc.right > rc.bottom ? (short)HIWORD(GetMessagePos()) + rc.bottom / 2 : (short)LOWORD(GetMessagePos()) + rc.right / 2, (LPARAM)hwnd);
		}
		return 0;

	case WM_ERASEBKGND:
		return 1;

	case WM_PAINT:
		GetClientRect(hwnd, &rc);
		{
			PAINTSTRUCT ps;
			HDC dc = BeginPaint(hwnd, &ps);

			if (dat && CSkin::m_skinEnabled)
				CSkin::SkinDrawBG(hwnd, dat->m_pContainer->m_hwnd, dat->m_pContainer, &rc, dc);
			else if (M.isAero() || M.isVSThemed()) {
				if (GetDlgCtrlID(hwnd) == IDC_PANELSPLITTER) {
					EndPaint(hwnd, &ps);
					return 0;
				}
				CSkin::FillBack(dc, &rc);
			}
			else CSkin::FillBack(dc, &rc);

			EndPaint(hwnd, &ps);
		}
		return 0;

	case WM_LBUTTONUP:
		HWND hwndCapture = GetCapture();

		ReleaseCapture();
		dat->DM_ScrollToBottom(0, 1);
		if (!dat->isChat() && hwnd == GetDlgItem(hwndParent, IDC_PANELSPLITTER)) {
			SendMessage(hwndParent, WM_SIZE, 0, 0);
			RedrawWindow(hwndParent, nullptr, nullptr, RDW_ALLCHILDREN | RDW_INVALIDATE | RDW_UPDATENOW);
		}
		else if (hwnd == GetDlgItem(hwndParent, IDC_SPLITTERY)) {
			if (hwndCapture != hwnd)
				break;

			POINT pt;
			GetCursorPos(&pt);

			LONG messagePos = GetMessagePos();
			GetClientRect(hwnd, &rc);

			int selection;
			if (dat->m_bIsAutosizingInput)
				selection = ID_SPLITTERCONTEXT_SETPOSITIONFORTHISSESSION;
			else
				selection = TrackPopupMenu(GetSubMenu(PluginConfig.g_hMenuContext, 8), TPM_RETURNCMD, pt.x, pt.y, 0, hwndParent, nullptr);

			switch (selection) {
			case ID_SPLITTERCONTEXT_SAVEFORTHISCONTACTONLY:
				dat->m_dwFlagsEx |= MWF_SHOW_SPLITTEROVERRIDE;
				db_set_b(dat->m_hContact, SRMSGMOD_T, "splitoverride", 1);
				if (!dat->isChat())
					dat->SaveSplitter();
				break;

			case ID_SPLITTERCONTEXT_SETPOSITIONFORTHISSESSION:
				if (dat->m_bIsAutosizingInput) {
					GetWindowRect(GetDlgItem(dat->GetHwnd(), IDC_SRMM_MESSAGE), &rc);
					dat->m_iInputAreaHeight = 0;
				}
				break;

			case ID_SPLITTERCONTEXT_SAVEGLOBALFORALLSESSIONS:
				{
					DWORD dwOff_IM = 0, dwOff_CHAT = 0;

					dwOff_CHAT = -(2 + (PluginConfig.m_DPIscaleY > 1.0 ? 1 : 0));
					dwOff_IM = 2 + (PluginConfig.m_DPIscaleY > 1.0 ? 1 : 0);

					RECT rcWin;
					GetWindowRect(hwndParent, &rcWin);

					PluginConfig.lastSPlitterPos.pSrcDat = dat;
					PluginConfig.lastSPlitterPos.pSrcContainer = dat->m_pContainer;
					PluginConfig.lastSPlitterPos.lParam = rc.bottom;
					PluginConfig.lastSPlitterPos.pos = rcWin.bottom - HIWORD(messagePos);
					PluginConfig.lastSPlitterPos.pos_chat = rcWin.bottom - (short)HIWORD(messagePos) + rc.bottom / 2;
					PluginConfig.lastSPlitterPos.off_chat = dwOff_CHAT;
					PluginConfig.lastSPlitterPos.off_im = dwOff_IM;
					SendMessage(dat->GetHwnd(), DM_SPLITTERGLOBALEVENT, 0, 0);
					Srmm_Broadcast(DM_SPLITTERGLOBALEVENT, 0, 0);
				}
				break;

			default:
				dat->m_iSplitterY = dat->m_savedSplitterY;
				dat->m_dynaSplitter = dat->m_savedDynaSplit;
				dat->DM_RecalcPictureSize();
				dat->UpdateToolbarBG();
				SendMessage(hwndParent, WM_SIZE, 0, 0);
				dat->DM_ScrollToBottom(0, 1);
				break;
			}
		}
		return 0;
	}
	return mir_callNextSubclass(hwnd, SplitterSubclassProc, msg, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////////////////

CSrmmWindow::CSrmmWindow()
	: CTabBaseDlg(IDD_MSGSPLITNEW),
	m_btnOk(this, IDOK),
	m_btnAdd(this, IDC_ADD),
	m_btnQuote(this, IDC_QUOTE),
	m_btnCancelAdd(this, IDC_CANCELADD)
{
	m_btnOk.OnClick = Callback(this, &CSrmmWindow::onClick_Ok);
	m_btnAdd.OnClick = Callback(this, &CSrmmWindow::onClick_Add);
	m_btnQuote.OnClick = Callback(this, &CSrmmWindow::onClick_Quote);
	m_btnCancelAdd.OnClick = Callback(this, &CSrmmWindow::onClick_CancelAdd);

	m_message.OnChange = Callback(this, &CSrmmWindow::onChange_Message);
}

void CSrmmWindow::tabClearLog()
{
	if (m_hwndIEView || m_hwndHPP) {
		IEVIEWEVENT event;
		event.cbSize = sizeof(IEVIEWEVENT);
		event.iType = IEE_CLEAR_LOG;
		event.dwFlags = (m_dwFlags & MWF_LOG_RTL) ? IEEF_RTL : 0;
		event.hContact = m_hContact;
		if (m_hwndIEView) {
			event.hwnd = m_hwndIEView;
			CallService(MS_IEVIEW_EVENT, 0, (LPARAM)&event);
		}
		else {
			event.hwnd = m_hwndHPP;
			CallService(MS_HPP_EG_EVENT, 0, (LPARAM)&event);
		}
	}
	m_log.SetText(L"");
	m_hDbEventFirst = 0;
}

CThumbBase* CSrmmWindow::tabCreateThumb(CProxyWindow *pProxy) const
{
	return new CThumbIM(pProxy);
}

void CSrmmWindow::OnInitDialog()
{
	CTabBaseDlg::OnInitDialog();

	m_cache->setWindowData(this);

	m_szProto = const_cast<char *>(m_cache->getProto());
	m_bIsMeta = m_cache->isMeta();
	if (m_bIsMeta)
		m_cache->updateMeta();

	// show a popup if wanted...
	if (m_bWantPopup) {
		DBEVENTINFO dbei = {};
		m_bWantPopup = false;
		db_event_get(m_hDbEventFirst, &dbei);
		tabSRMM_ShowPopup(m_hContact, m_hDbEventFirst, dbei.eventType, 0, nullptr, m_hwnd, m_cache->getActiveProto());
	}
	m_hDbEventFirst = 0;

	if (m_hContact && m_szProto != nullptr) {
		m_wStatus = db_get_w(m_hContact, m_szProto, "Status", ID_STATUS_OFFLINE);
		wcsncpy_s(m_wszStatus, Clist_GetStatusModeDescription(m_szProto == nullptr ? ID_STATUS_OFFLINE : m_wStatus, 0), _TRUNCATE);
	}
	else m_wStatus = ID_STATUS_OFFLINE;

	for (auto &it : btnControls)
		CustomizeButton(GetDlgItem(m_hwnd, it));

	GetMYUIN();
	GetClientIcon();

	CustomizeButton(CreateWindowEx(0, L"MButtonClass", L"", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 0, 0, 6, DPISCALEY_S(20),
		m_hwnd, (HMENU)IDC_TOGGLESIDEBAR, g_plugin.getInst(), nullptr));
	m_hwndPanelPicParent = CreateWindowEx(WS_EX_TOPMOST, L"Static", L"", SS_OWNERDRAW | WS_VISIBLE | WS_CHILD, 1, 1, 1, 1, m_hwnd, (HMENU)6000, nullptr, nullptr);
	mir_subclassWindow(m_hwndPanelPicParent, CInfoPanel::avatarParentSubclass);

	m_bShowUIElements = (m_pContainer->dwFlags & CNT_HIDETOOLBAR) == 0;
	m_sendMode |= m_hContact == 0 ? SMODE_MULTIPLE : 0;
	m_sendMode |= M.GetByte(m_hContact, "no_ack", 0) ? SMODE_NOACK : 0;

	m_hQueuedEvents = (MEVENT*)mir_calloc(sizeof(MEVENT)* EVENT_QUEUE_SIZE);
	m_iEventQueueSize = EVENT_QUEUE_SIZE;
	m_iCurrentQueueError = -1;

	// message history limit
	// hHistoryEvents holds up to n event handles
	m_maxHistory = M.GetDword(m_hContact, "maxhist", M.GetDword("maxhist", 0));
	m_curHistory = 0;
	if (m_maxHistory)
		m_hHistoryEvents = (MEVENT*)mir_alloc(m_maxHistory * sizeof(MEVENT));
	else
		m_hHistoryEvents = nullptr;

	if (!m_bIsMeta)
		SendMessage(m_hwnd, DM_UPDATEWINICON, 0, 0);

	GetMyNick();

	m_iMultiSplit = (int)M.GetDword(SRMSGMOD, "multisplit", 150);
	m_nTypeMode = PROTOTYPE_SELFTYPING_OFF;
	SetTimer(m_hwnd, TIMERID_TYPE, 1000, nullptr);
	m_iLastEventType = 0xffffffff;

	// load log option flags...
	m_dwFlags = m_pContainer->theme.dwFlags;

	// consider per-contact message setting overrides
	if (m_hContact && M.GetDword(m_hContact, "mwmask", 0))
		LoadLocalFlags();

	DM_InitTip();
	m_pPanel.getVisibility();

	m_dwFlagsEx |= M.GetByte(m_hContact, "splitoverride", 0) ? MWF_SHOW_SPLITTEROVERRIDE : 0;
	SetMessageLog();
	if (m_hContact)
		m_pPanel.loadHeight();

	m_bShowAvatar = GetAvatarVisibility();

	Utils::showDlgControl(m_hwnd, IDC_MULTISPLITTER, SW_HIDE);

	RECT rc;
	GetWindowRect(GetDlgItem(m_hwnd, IDC_SPLITTERY), &rc);
	
	POINT pt;
	pt.y = (rc.top + rc.bottom) / 2;
	pt.x = 0;
	ScreenToClient(m_hwnd, &pt);
	m_originalSplitterY = pt.y;
	if (m_iSplitterY == -1)
		m_iSplitterY = m_originalSplitterY + 60;

	GetWindowRect(m_message.GetHwnd(), &rc);
	m_minEditBoxSize.cx = rc.right - rc.left;
	m_minEditBoxSize.cy = rc.bottom - rc.top;

	BB_InitDlgButtons();
	SendMessage(m_hwnd, WM_CBD_LOADICONS, 0, 0);

	SendDlgItemMessage(m_hwnd, IDC_ADD, BUTTONSETASFLATBTN, TRUE, 0);
	SendDlgItemMessage(m_hwnd, IDC_CANCELADD, BUTTONSETASFLATBTN, TRUE, 0);

	SendDlgItemMessage(m_hwnd, IDC_TOGGLESIDEBAR, BUTTONSETASFLATBTN, TRUE, 0);
	SendDlgItemMessage(m_hwnd, IDC_TOGGLESIDEBAR, BUTTONSETASTHEMEDBTN, CSkin::IsThemed(), 0);
	SendDlgItemMessage(m_hwnd, IDC_TOGGLESIDEBAR, BUTTONSETCONTAINER, (LPARAM)m_pContainer, 0);
	SendDlgItemMessage(m_hwnd, IDC_TOGGLESIDEBAR, BUTTONSETASTOOLBARBUTTON, TRUE, 0);

	NotifyEvent(MSG_WINDOW_EVT_OPENING);

	for (auto &it : tooltips)
		SendDlgItemMessage(m_hwnd, it.id, BUTTONADDTOOLTIP, (WPARAM)TranslateW(it.text), BATF_UNICODE);

	SetDlgItemText(m_hwnd, IDC_LOGFROZENTEXT, m_bNotOnList ? TranslateT("Contact not on list. You may add it...") :
		TranslateT("Auto scrolling is disabled (press F12 to enable it)"));

	SendDlgItemMessage(m_hwnd, IDC_CLOSE, BUTTONADDTOOLTIP, (WPARAM)TranslateT("Close session"), BATF_UNICODE);
	SendDlgItemMessage(m_hwnd, IDC_PROTOCOL, BUTTONADDTOOLTIP, (WPARAM)TranslateT("Click for contact menu\nClick dropdown for window settings"), BATF_UNICODE);

	SetDlgItemText(m_hwnd, IDC_RETRY, TranslateT("Retry"));
	{
		UINT _ctrls[] = { IDC_RETRY, IDC_CANCELSEND, IDC_MSGSENDLATER };
		for (auto &it : _ctrls) {
			SendDlgItemMessage(m_hwnd, it, BUTTONSETASPUSHBTN, TRUE, 0);
			SendDlgItemMessage(m_hwnd, it, BUTTONSETASFLATBTN, FALSE, 0);
			SendDlgItemMessage(m_hwnd, it, BUTTONSETASTHEMEDBTN, TRUE, 0);
		}
	}

	SetDlgItemText(m_hwnd, IDC_CANCELSEND, TranslateT("Cancel"));
	SetDlgItemText(m_hwnd, IDC_MSGSENDLATER, TranslateT("Send later"));

	m_log.SendMsg(EM_SETUNDOLIMIT, 0, 0);
	m_log.SendMsg(EM_SETEVENTMASK, 0, ENM_MOUSEEVENTS | ENM_KEYEVENTS | ENM_LINK);

	m_message.SendMsg(EM_SETEVENTMASK, 0, ENM_REQUESTRESIZE | ENM_MOUSEEVENTS | ENM_SCROLL | ENM_KEYEVENTS | ENM_CHANGE);
	m_message.SetReadOnly(false);

	m_bActualHistory = M.GetByte(m_hContact, "ActualHistory", 0) != 0;

	/* OnO: higligh lines to their end */
	m_log.SendMsg(EM_SETEDITSTYLE, SES_EXTENDBACKCOLOR, SES_EXTENDBACKCOLOR);

	m_log.SendMsg(EM_SETLANGOPTIONS, 0, m_log.SendMsg(EM_GETLANGOPTIONS, 0, 0) & ~IMF_AUTOFONTSIZEADJUST);

	// add us to the tray list (if it exists)
	if (PluginConfig.g_hMenuTrayUnread != nullptr && m_hContact != 0 && m_szProto != nullptr)
		UpdateTrayMenu(nullptr, m_wStatus, m_szProto, m_wszStatus, m_hContact, FALSE);

	m_log.SendMsg(EM_AUTOURLDETECT, TRUE, 0);
	m_log.SendMsg(EM_EXLIMITTEXT, 0, 0x80000000);

	// subclassing stuff
	mir_subclassWindow(GetDlgItem(m_hwnd, IDC_CONTACTPIC), AvatarSubclassProc);
	mir_subclassWindow(GetDlgItem(m_hwnd, IDC_SPLITTERY), SplitterSubclassProc);
	mir_subclassWindow(GetDlgItem(m_hwnd, IDC_MULTISPLITTER), SplitterSubclassProc);
	mir_subclassWindow(GetDlgItem(m_hwnd, IDC_PANELSPLITTER), SplitterSubclassProc);

	// load old messages from history (if wanted...)
	m_cache->updateStats(TSessionStats::INIT_TIMER);
	if (m_hContact) {
		FindFirstEvent();
		m_nMax = (int)m_cache->getMaxMessageLength();
	}
	LoadContactAvatar();
	DM_OptionsApplied(0, 0);
	LoadOwnAvatar();

	// restore saved msg if any...
	if (m_hContact) {
		ptrW tszSavedMsg(db_get_wsa(m_hContact, SRMSGMOD, "SavedMsg"));
		if (tszSavedMsg != 0) {
			SETTEXTEX stx = { ST_DEFAULT, 1200 };
			m_message.SendMsg(EM_SETTEXTEX, (WPARAM)&stx, tszSavedMsg);
			UpdateSaveAndSendButton();
			if (m_pContainer->m_hwndActive == m_hwnd)
				UpdateReadChars();
		}
	}
	if (wszInitialText) {
		m_message.SetText(wszInitialText);
		int len = GetWindowTextLength(m_message.GetHwnd());
		PostMessage(m_message.GetHwnd(), EM_SETSEL, len, len);
		if (len)
			EnableSendButton(true);
		mir_free(wszInitialText);
	}

	for (MEVENT hdbEvent = db_event_last(m_hContact); hdbEvent; hdbEvent = db_event_prev(m_hContact, hdbEvent)) {
		DBEVENTINFO dbei = {};
		db_event_get(hdbEvent, &dbei);
		if (dbei.eventType == EVENTTYPE_MESSAGE && !(dbei.flags & DBEF_SENT)) {
			m_lastMessage = dbei.timestamp;
			DM_UpdateLastMessage();
			break;
		}
	}

	SendMessage(m_pContainer->m_hwnd, DM_QUERYCLIENTAREA, 0, (LPARAM)&rc);

	SetWindowPos(m_hwnd, nullptr, rc.left, rc.top, (rc.right - rc.left), (rc.bottom - rc.top), m_bActivate ? 0 : SWP_NOZORDER | SWP_NOACTIVATE);
	LoadSplitter();
	ShowPicture(true);

	if (m_pContainer->dwFlags & CNT_CREATE_MINIMIZED || !m_bActivate || m_pContainer->dwFlags & CNT_DEFERREDTABSELECT) {
		m_iFlashIcon = PluginConfig.g_IconMsgEvent;
		SetTimer(m_hwnd, TIMERID_FLASHWND, TIMEOUT_FLASHWND, nullptr);
		m_bCanFlashTab = true;

		DBEVENTINFO dbei = {};
		dbei.eventType = EVENTTYPE_MESSAGE;
		FlashOnClist(m_hDbEventFirst, &dbei);

		SendMessage(m_pContainer->m_hwnd, DM_SETICON, (WPARAM)this, (LPARAM)Skin_LoadIcon(SKINICON_EVENT_MESSAGE));
		m_pContainer->dwFlags |= CNT_NEED_UPDATETITLE;
		m_dwFlags |= MWF_NEEDCHECKSIZE | MWF_WASBACKGROUNDCREATE | MWF_DEFERREDSCROLL;
	}

	if (m_bActivate) {
		m_pContainer->m_hwndActive = m_hwnd;
		ShowWindow(m_hwnd, SW_SHOW);
		SetActiveWindow(m_hwnd);
		SetForegroundWindow(m_hwnd);
	}
	else if (m_pContainer->dwFlags & CNT_CREATE_MINIMIZED) {
		m_dwFlags |= MWF_DEFERREDSCROLL;
		ShowWindow(m_hwnd, SW_SHOWNOACTIVATE);
		m_pContainer->m_hwndActive = m_hwnd;
		m_pContainer->dwFlags |= CNT_DEFERREDCONFIGURE;
	}
	m_pContainer->UpdateTitle(m_hContact);

	DM_RecalcPictureSize();
	m_dwLastActivity = GetTickCount() - 1000;
	m_pContainer->dwLastActivity = m_dwLastActivity;

	if (m_hwndHPP)
		mir_subclassWindow(m_hwndHPP, HPPKFSubclassProc);

	m_dwFlags &= ~MWF_INITMODE;
	NotifyEvent(MSG_WINDOW_EVT_OPEN);

	if (m_pContainer->dwFlags & CNT_CREATE_MINIMIZED) {
		m_pContainer->dwFlags &= ~CNT_CREATE_MINIMIZED;
		m_pContainer->m_hwndActive = m_hwnd;
	}
}

void CSrmmWindow::OnDestroy()
{
	m_pContainer->ClearMargins();
	PostMessage(m_pContainer->m_hwnd, WM_SIZE, 0, 1);
	if (m_pContainer->dwFlags & CNT_SIDEBAR)
		m_pContainer->SideBar->removeSession(this);
	m_cache->setWindowData();
	if (m_hContact && M.GetByte("deletetemp", 0))
		if (db_get_b(m_hContact, "CList", "NotOnList", 0))
			db_delete_contact(m_hContact);

	if (m_hwndContactPic)
		DestroyWindow(m_hwndContactPic);

	if (m_hwndPanelPic)
		DestroyWindow(m_hwndPanelPic);

	if (m_hwndPanelPicParent)
		DestroyWindow(m_hwndPanelPicParent);

	if (m_cache->isValid()) { // not valid means the contact was deleted
		NotifyEvent(MSG_WINDOW_EVT_CLOSING);
		AddContactToFavorites(m_hContact, m_cache->getNick(), m_cache->getActiveProto(), m_wszStatus, m_wStatus,
			Skin_LoadProtoIcon(m_cache->getActiveProto(), m_cache->getActiveStatus()), 1, PluginConfig.g_hMenuRecent);
		if (m_hContact) {
			if (!m_bEditNotesActive) {
				char *msg = m_message.GetRichTextRtf(true);
				if (msg) {
					db_set_utf(m_hContact, SRMSGMOD, "SavedMsg", msg);
					mir_free(msg);
				}
				else db_unset(m_hContact, SRMSGMOD, "SavedMsg");
			}
			else SendMessage(m_hwnd, WM_COMMAND, IDC_PIC, 0);
		}
	}

	if (m_nTypeMode == PROTOTYPE_SELFTYPING_ON)
		DM_NotifyTyping(PROTOTYPE_SELFTYPING_OFF);

	DM_FreeTheme();

	// search the sendqueue for unfinished send jobs and mir_free them. Leave unsent
	// messages in the queue as they can be acked later
	{
		SendJob *jobs = sendQueue->getJobByIndex(0);

		for (int i = 0; i < SendQueue::NR_SENDJOBS; i++) {
			if (jobs[i].hContact == m_hContact) {
				if (jobs[i].iStatus >(unsigned)SendQueue::SQ_INPROGRESS)
					sendQueue->clearJob(i);

				// unfinished jobs which did not yet return anything are kept in the queue.
				// the hwndOwner is set to 0 because the window handle is now no longer valid.
				// Response for such a job is still silently handled by AckMessage() (sendqueue.c)
				if (jobs[i].iStatus == (unsigned)SendQueue::SQ_INPROGRESS)
					jobs[i].hOwnerWnd = nullptr;
			}
		}
	}

	if (m_hwndTip)
		DestroyWindow(m_hwndTip);

	UpdateTrayMenuState(this, FALSE);               // remove me from the tray menu (if still there)
	if (PluginConfig.g_hMenuTrayUnread)
		DeleteMenu(PluginConfig.g_hMenuTrayUnread, m_hContact, MF_BYCOMMAND);

	if (m_cache->isValid())
		db_set_dw(0, SRMSGMOD, "multisplit", m_iMultiSplit);

	int i = GetTabIndexFromHWND(m_hwndParent, m_hwnd);
	if (i >= 0) {
		SendMessage(m_hwndParent, WM_USER + 100, 0, 0);                      // remove tooltip
		TabCtrl_DeleteItem(m_hwndParent, i);
		m_pContainer->UpdateTabs();
		m_iTabID = -1;
	}

	NotifyEvent(MSG_WINDOW_EVT_CLOSE);

	// clean up IEView and H++ log windows
	if (m_hwndIEView != nullptr) {
		IEVIEWWINDOW ieWindow;
		ieWindow.cbSize = sizeof(IEVIEWWINDOW);
		ieWindow.iType = IEW_DESTROY;
		ieWindow.hwnd = m_hwndIEView;
		CallService(MS_IEVIEW_WINDOW, 0, (LPARAM)&ieWindow);
	}
	if (m_hwndHPP) {
		IEVIEWWINDOW ieWindow;
		ieWindow.cbSize = sizeof(IEVIEWWINDOW);
		ieWindow.iType = IEW_DESTROY;
		ieWindow.hwnd = m_hwndHPP;
		CallService(MS_HPP_EG_WINDOW, 0, (LPARAM)&ieWindow);
	}

	CSuper::OnDestroy();
}

void CSrmmWindow::ReplayQueue()
{
	for (int i = 0; i < m_iNextQueuedEvent; i++)
		if (m_hQueuedEvents[i] != 0)
			StreamInEvents(m_hQueuedEvents[i], 1, 1, nullptr);

	m_iNextQueuedEvent = 0;
	SetDlgItemText(m_hwnd, IDC_LOGFROZENTEXT, m_bNotOnList ? TranslateT("Contact not on list. You may add it...") :
		TranslateT("Auto scrolling is disabled (press F12 to enable it)"));
}

void CSrmmWindow::UpdateTitle()
{
	DWORD dwOldIdle = m_idle;
	const char *szActProto = nullptr;

	m_wszStatus[0] = 0;

	if (m_iTabID == -1)
		return;

	TCITEM item = {};

	bool bChanged = false;
	wchar_t newtitle[128];
	if (m_hContact) {
		if (m_szProto) {
			szActProto = m_cache->getProto();

			bool bHasName = (m_cache->getUIN()[0] != 0);
			m_idle = m_cache->getIdleTS();
			m_dwFlagsEx = m_idle ? m_dwFlagsEx | MWF_SHOW_ISIDLE : m_dwFlagsEx & ~MWF_SHOW_ISIDLE;

			m_wStatus = m_cache->getStatus();
			wcsncpy_s(m_wszStatus, Clist_GetStatusModeDescription(m_szProto == nullptr ? ID_STATUS_OFFLINE : m_wStatus, 0), _TRUNCATE);

			wchar_t newcontactname[128]; newcontactname[0] = 0;
			if (PluginConfig.m_bCutContactNameOnTabs)
				CutContactName(m_cache->getNick(), newcontactname, _countof(newcontactname));
			else
				wcsncpy_s(newcontactname, m_cache->getNick(), _TRUNCATE);

			Utils::DoubleAmpersands(newcontactname, _countof(newcontactname));

			if (newcontactname[0] != 0) {
				if (PluginConfig.m_bStatusOnTabs)
					mir_snwprintf(newtitle, L"%s (%s)", newcontactname, m_wszStatus);
				else
					wcsncpy_s(newtitle, newcontactname, _TRUNCATE);
			}
			else wcsncpy_s(newtitle, L"Forward", _TRUNCATE);

			if (mir_wstrcmp(newtitle, m_wszTitle))
				bChanged = true;
			else if (m_wStatus != m_wOldStatus)
				bChanged = true;

			SendMessage(m_hwnd, DM_UPDATEWINICON, 0, 0);

			wchar_t fulluin[256];
			if (m_bIsMeta)
				mir_snwprintf(fulluin,
					TranslateT("UID: %s (SHIFT click -> copy to clipboard)\nClick for user's details\nRight click for metacontact control\nClick dropdown to add or remove user from your favorites."),
					bHasName ? m_cache->getUIN() : TranslateT("No UID"));
			else
				mir_snwprintf(fulluin,
					TranslateT("UID: %s (SHIFT click -> copy to clipboard)\nClick for user's details\nClick dropdown to change this contact's favorite status."),
					bHasName ? m_cache->getUIN() : TranslateT("No UID"));

			SendDlgItemMessage(m_hwnd, IDC_NAME, BUTTONADDTOOLTIP, (WPARAM)fulluin, BATF_UNICODE);
		}
	}
	else wcsncpy_s(newtitle, L"Message Session", _TRUNCATE);

	m_wOldStatus = m_wStatus;
	if (m_idle != dwOldIdle || bChanged) {
		if (bChanged) {
			item.mask |= TCIF_TEXT;
			item.pszText = m_wszTitle;
			wcsncpy_s(m_wszTitle, newtitle, _TRUNCATE);
			if (m_pWnd)
				m_pWnd->updateTitle(m_cache->getNick());
		}
		if (m_iTabID >= 0) {
			TabCtrl_SetItem(m_hwndParent, m_iTabID, &item);
			if (m_pContainer->dwFlags & CNT_SIDEBAR)
				m_pContainer->SideBar->updateSession(this);
		}
		if (m_pContainer->m_hwndActive == m_hwnd && bChanged)
			m_pContainer->UpdateTitle(m_hContact);

		UpdateTrayMenuState(this, TRUE);
		if (m_cache->isFavorite())
			AddContactToFavorites(m_hContact, m_cache->getNick(), szActProto, m_wszStatus, m_wStatus,
			Skin_LoadProtoIcon(m_cache->getProto(), m_cache->getStatus()), 0, PluginConfig.g_hMenuFavorites);

		if (m_cache->isRecent())
			AddContactToFavorites(m_hContact, m_cache->getNick(), szActProto, m_wszStatus, m_wStatus,
			Skin_LoadProtoIcon(m_cache->getProto(), m_cache->getStatus()), 0, PluginConfig.g_hMenuRecent);

		m_pPanel.Invalidate();
		if (m_pWnd)
			m_pWnd->Invalidate();
	}

	// care about MetaContacts and update the statusbar icon with the currently "most online" contact...
	if (m_bIsMeta) {
		PostMessage(m_hwnd, DM_OWNNICKCHANGED, 0, 0);
		if (m_pContainer->dwFlags & CNT_UINSTATUSBAR)
			DM_UpdateLastMessage();
	}
}

void CSrmmWindow::onClick_Ok(CCtrlButton*)
{
	if (m_bEditNotesActive) {
		SendMessage(m_hwnd, DM_ACTIVATETOOLTIP, IDC_PIC, (LPARAM)TranslateT("You are editing the user notes. Click the button again or use the hotkey (default: Alt-N) to save the notes and return to normal messaging mode"));
		return;
	}

	// don't parse text formatting when the message contains curly braces - these are used by the rtf syntax
	// and the parser currently cannot handle them properly in the text - XXX needs to be fixed later.
	FINDTEXTEX fi = { 0 };
	fi.chrg.cpMin = 0;
	fi.chrg.cpMax = -1;
	fi.lpstrText = L"{";
	int final_sendformat = m_message.SendMsg(EM_FINDTEXTEX, FR_DOWN, (LPARAM)&fi) == -1 ? m_SendFormat : 0;
	fi.lpstrText = L"}";
	final_sendformat = m_message.SendMsg(EM_FINDTEXTEX, FR_DOWN, (LPARAM)&fi) == -1 ? final_sendformat : 0;

	if (GetSendButtonState(m_hwnd) == PBS_DISABLED)
		return;

	ptrA streamOut(m_message.GetRichTextRtf(!final_sendformat));
	if (streamOut == nullptr)
		return;

	CMStringW decoded(ptrW(mir_utf8decodeW(streamOut)));
	if (decoded.IsEmpty())
		return;

	if (final_sendformat)
		DoRtfToTags(decoded);
	decoded.TrimRight();

	T2Utf utfResult(decoded);
	size_t memRequired = mir_strlen(utfResult) + 1;

	// try to detect RTL
	HWND hwndEdit = m_message.GetHwnd();
	SendMessage(hwndEdit, WM_SETREDRAW, FALSE, 0);

	PARAFORMAT2 pf2;
	memset(&pf2, 0, sizeof(PARAFORMAT2));
	pf2.cbSize = sizeof(pf2);
	pf2.dwMask = PFM_RTLPARA;
	SendMessage(hwndEdit, EM_SETSEL, 0, -1);
	SendMessage(hwndEdit, EM_GETPARAFORMAT, 0, (LPARAM)&pf2);

	int flags = 0;
	if (pf2.wEffects & PFE_RTLPARA)
		if (Utils_IsRtl(decoded))
			flags |= PREF_RTL;

	SendMessage(hwndEdit, WM_SETREDRAW, TRUE, 0);
	SendMessage(hwndEdit, EM_SETSEL, -1, -1);
	InvalidateRect(hwndEdit, nullptr, FALSE);

	if (memRequired > m_iSendBufferSize) {
		m_sendBuffer = (char *)mir_realloc(m_sendBuffer, memRequired);
		m_iSendBufferSize = memRequired;
	}

	memcpy(m_sendBuffer, (char*)utfResult, memRequired);

	if (memRequired == 0 || m_sendBuffer[0] == 0)
		return;

	if (m_sendMode & SMODE_CONTAINER && m_pContainer->m_hwndActive == m_hwnd && GetForegroundWindow() == m_pContainer->m_hwnd) {
		int tabCount = TabCtrl_GetItemCount(m_hwndParent);
		ptrA szFromStream(m_message.GetRichTextRtf(!m_SendFormat));

		for (int i = 0; i < tabCount; i++) {
			// get the contact from the tabs lparam which hopefully is the tabs hwnd so we can get its userdata.... hopefully
			HWND contacthwnd = GetTabWindow(m_hwndParent, i);
			if (IsWindow(contacthwnd)) {
				// if the contact hwnd is the current contact then ignore it and let the normal code deal with the msg
				if (contacthwnd != m_hwnd) {
					SETTEXTEX stx = { ST_DEFAULT, CP_UTF8 };
					// send the buffer to the contacts msg typing area
					SendDlgItemMessage(contacthwnd, IDC_SRMM_MESSAGE, EM_SETTEXTEX, (WPARAM)&stx, (LPARAM)szFromStream);
					SendMessage(contacthwnd, WM_COMMAND, IDOK, 0);
				}
			}
		}
	}
	// END /all /MOD
	if (m_nTypeMode == PROTOTYPE_SELFTYPING_ON)
		DM_NotifyTyping(PROTOTYPE_SELFTYPING_OFF);

	DeletePopupsForContact(m_hContact, PU_REMOVE_ON_SEND);
	sendQueue->addTo(this, memRequired, flags);
}

void CSrmmWindow::onClick_Add(CCtrlButton*)
{
	Contact_Add(m_hContact, m_hwnd);

	if (!db_get_b(m_hContact, "CList", "NotOnList", 0)) {
		m_bNotOnList = FALSE;
		ShowMultipleControls(m_hwnd, addControls, _countof(addControls), SW_HIDE);
		if (!(m_dwFlagsEx & MWF_SHOW_SCROLLINGDISABLED))
			Utils::showDlgControl(m_hwnd, IDC_LOGFROZENTEXT, SW_HIDE);
		Resize();
	}
}

void CSrmmWindow::onClick_Quote(CCtrlButton*)
{
	CHARRANGE sel;
	SETTEXTEX stx = { ST_SELECTION, 1200 };

	MEVENT hDBEvent = m_hDbEventLast;
	if (m_hwndIEView || m_hwndHPP) { // IEView quoting support..
		IEVIEWEVENT event = { sizeof(event) };
		event.hContact = m_hContact;
		event.dwFlags = 0;
		event.iType = IEE_GET_SELECTION;

		wchar_t *selected;
		if (m_hwndIEView) {
			event.hwnd = m_hwndIEView;
			selected = (wchar_t*)CallService(MS_IEVIEW_EVENT, 0, (LPARAM)&event);
		}
		else {
			event.hwnd = m_hwndHPP;
			selected = (wchar_t*)CallService(MS_HPP_EG_EVENT, 0, (LPARAM)&event);
		}

		if (selected != nullptr) {
			ptrW szQuoted(QuoteText(selected));
			m_message.SendMsg(EM_SETTEXTEX, (WPARAM)&stx, (LPARAM)szQuoted);
			return;
		}

		hDBEvent = db_event_last(m_hContact);
	}

	if (hDBEvent == 0)
		return;

	m_log.SendMsg(EM_EXGETSEL, 0, (LPARAM)&sel);
	if (sel.cpMin == sel.cpMax) {
		DBEVENTINFO dbei = {};
		dbei.cbBlob = db_event_getBlobSize(hDBEvent);
		wchar_t *szText = (wchar_t*)mir_alloc((dbei.cbBlob + 1) * sizeof(wchar_t));   //URLs are made one char bigger for crlf
		dbei.pBlob = (BYTE*)szText;
		db_event_get(hDBEvent, &dbei);
		int iSize = int(mir_strlen((char*)dbei.pBlob)) + 1;

		bool bNeedsFree = false;
		wchar_t *szConverted;
		if (dbei.flags & DBEF_UTF) {
			szConverted = mir_utf8decodeW((char*)szText);
			bNeedsFree = true;
		}
		else {
			if (iSize != (int)dbei.cbBlob)
				szConverted = (wchar_t*)&dbei.pBlob[iSize];
			else {
				szConverted = (wchar_t*)mir_alloc(sizeof(wchar_t) * iSize);
				bNeedsFree = true;
				MultiByteToWideChar(CP_ACP, 0, (char*)dbei.pBlob, -1, szConverted, iSize);
			}
		}
		
		if (dbei.eventType == EVENTTYPE_FILE) {
			size_t iDescr = mir_strlen((char *)(szText + sizeof(DWORD)));
			memmove(szText, szText + sizeof(DWORD), iDescr);
			memmove(szText + iDescr + 2, szText + sizeof(DWORD) + iDescr, dbei.cbBlob - iDescr - sizeof(DWORD) - 1);
			szText[iDescr] = '\r';
			szText[iDescr + 1] = '\n';
			szConverted = (wchar_t*)mir_alloc(sizeof(wchar_t)* (1 + mir_strlen((char *)szText)));
			MultiByteToWideChar(CP_ACP, 0, (char *)szText, -1, szConverted, 1 + (int)mir_strlen((char *)szText));
			bNeedsFree = true;
		}
		
		if (szConverted != nullptr) {
			ptrW szQuoted(QuoteText(szConverted));
			m_message.SendMsg(EM_SETTEXTEX, (WPARAM)&stx, (LPARAM)szQuoted);
		}
		mir_free(szText);
		if (bNeedsFree)
			mir_free(szConverted);
	}
	else {
		ptrA szFromStream(m_log.GetRichTextRtf(true, true));
		ptrW converted(mir_utf8decodeW(szFromStream));
		Utils::FilterEventMarkers(converted);
		ptrW szQuoted(QuoteText(converted));
		m_message.SendMsg(EM_SETTEXTEX, (WPARAM)&stx, (LPARAM)szQuoted);
	}
	SetFocus(m_message.GetHwnd());
}

void CSrmmWindow::onClick_CancelAdd(CCtrlButton*)
{
	m_bNotOnList = FALSE;
	ShowMultipleControls(m_hwnd, addControls, _countof(addControls), SW_HIDE);
	if (!(m_dwFlagsEx & MWF_SHOW_SCROLLINGDISABLED))
		Utils::showDlgControl(m_hwnd, IDC_LOGFROZENTEXT, SW_HIDE);
	Resize();
}

void CSrmmWindow::onChange_Message(CCtrlEdit*)
{
	if (m_pContainer->m_hwndActive == m_hwnd)
		UpdateReadChars();
	m_dwFlags |= MWF_NEEDHISTORYSAVE;
	m_dwLastActivity = GetTickCount();
	m_pContainer->dwLastActivity = m_dwLastActivity;
	UpdateSaveAndSendButton();
	if (!(GetKeyState(VK_CONTROL) & 0x8000)) {
		m_nLastTyping = GetTickCount();
		if (GetWindowTextLength(m_message.GetHwnd())) {
			if (m_nTypeMode == PROTOTYPE_SELFTYPING_OFF) {
				if (!(m_dwFlags & MWF_INITMODE))
					DM_NotifyTyping(PROTOTYPE_SELFTYPING_ON);
			}
		}
		else if (m_nTypeMode == PROTOTYPE_SELFTYPING_ON)
			DM_NotifyTyping(PROTOTYPE_SELFTYPING_OFF);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// resizer proc for the "new" layout.

int CSrmmWindow::Resizer(UTILRESIZECONTROL *urc)
{
	int panelHeight = m_pPanel.getHeight() + 1;
	
	bool bInfoPanel = m_pPanel.isActive();
	bool bErrorState = (m_dwFlags & MWF_ERRORSTATE) != 0;
	bool bShowToolbar = (m_pContainer->dwFlags & CNT_HIDETOOLBAR) == 0;
	bool bBottomToolbar = (m_pContainer->dwFlags & CNT_BOTTOMTOOLBAR) != 0;

	RECT rc, rcButton;
	GetClientRect(m_log.GetHwnd(), &rc);
	GetClientRect(GetDlgItem(m_hwnd, IDC_PROTOCOL), &rcButton);

	if (m_panelStatusCX == 0)
		m_panelStatusCX = 80;

	switch (urc->wId) {
	case IDC_PANELSPLITTER:
		urc->rcItem.bottom = panelHeight;
		urc->rcItem.top = panelHeight - 2;
		return RD_ANCHORX_WIDTH | RD_ANCHORY_TOP;

	case IDC_SRMM_LOG:
		if (m_dwFlags & MWF_ERRORSTATE)
			urc->rcItem.bottom -= ERRORPANEL_HEIGHT;
		if (m_dwFlagsEx & MWF_SHOW_SCROLLINGDISABLED || m_bNotOnList)
			urc->rcItem.bottom -= 20;
		if (m_sendMode & SMODE_MULTIPLE)
			urc->rcItem.right -= (m_iMultiSplit + 3);
		urc->rcItem.bottom -= m_iSplitterY - m_originalSplitterY;
		if (!bShowToolbar || bBottomToolbar)
			urc->rcItem.bottom += DPISCALEY_S(21);
		if (bInfoPanel)
			urc->rcItem.top += panelHeight;
		urc->rcItem.bottom += 3;
		if (CSkin::m_skinEnabled) {
			CSkinItem *item = &SkinItems[ID_EXTBKHISTORY];
			if (!item->IGNORED) {
				urc->rcItem.left += item->MARGIN_LEFT;
				urc->rcItem.right -= item->MARGIN_RIGHT;
				urc->rcItem.top += item->MARGIN_TOP;
				urc->rcItem.bottom -= item->MARGIN_BOTTOM;
			}
		}
		rcLogBottom = urc->rcItem.bottom;
		return RD_ANCHORX_WIDTH | RD_ANCHORY_HEIGHT;

	case IDC_CONTACTPIC:
		GetClientRect(m_message.GetHwnd(), &rc);
		urc->rcItem.top -= m_iSplitterY - m_originalSplitterY;
		urc->rcItem.left = urc->rcItem.right - (m_pic.cx + 2);
		if ((urc->rcItem.bottom - urc->rcItem.top) < (m_pic.cy/* + 2*/) && m_bShowAvatar) {
			urc->rcItem.top = urc->rcItem.bottom - m_pic.cy;
			m_bUseOffset = true;
		}
		else m_bUseOffset = false;

		if (bShowToolbar && bBottomToolbar && (PluginConfig.m_bAlwaysFullToolbarWidth || ((m_pic.cy - DPISCALEY_S(6)) < rc.bottom))) {
			urc->rcItem.bottom -= DPISCALEY_S(22);
			if (m_bIsAutosizingInput) {
				urc->rcItem.left--;
				urc->rcItem.top--;
			}
		}

		if (m_hwndContactPic) //if m_pPanel control was created?
			SetWindowPos(m_hwndContactPic, HWND_TOP, 1, ((urc->rcItem.bottom - urc->rcItem.top) - (m_pic.cy)) / 2 + 1,  //resizes it
				m_pic.cx - 2, m_pic.cy - 2, SWP_SHOWWINDOW);

		return RD_ANCHORX_RIGHT | RD_ANCHORY_BOTTOM;

	case IDC_SPLITTERY:
		urc->rcItem.right = urc->dlgNewSize.cx;
		urc->rcItem.top -= m_iSplitterY - m_originalSplitterY;
		urc->rcItem.bottom = urc->rcItem.top + 2;
		OffsetRect(&urc->rcItem, 0, 1);
		urc->rcItem.left = 0;

		if (m_bUseOffset)
			urc->rcItem.right -= (m_pic.cx); // + DPISCALEX(2));
		return RD_ANCHORX_CUSTOM | RD_ANCHORY_BOTTOM;

	case IDC_SRMM_MESSAGE:
		urc->rcItem.right = urc->dlgNewSize.cx;
		if (m_bShowAvatar)
			urc->rcItem.right -= m_pic.cx + 2;
		urc->rcItem.top -= m_iSplitterY - m_originalSplitterY;
		if (bBottomToolbar && bShowToolbar)
			urc->rcItem.bottom -= DPISCALEY_S(22);

		if (m_bIsAutosizingInput)
			urc->rcItem.top -= DPISCALEY_S(1);

		msgTop = urc->rcItem.top;
		if (CSkin::m_skinEnabled) {
			CSkinItem *item = &SkinItems[ID_EXTBKINPUTAREA];
			if (!item->IGNORED) {
				urc->rcItem.left += item->MARGIN_LEFT;
				urc->rcItem.right -= item->MARGIN_RIGHT;
				urc->rcItem.top += item->MARGIN_TOP;
				urc->rcItem.bottom -= item->MARGIN_BOTTOM;
			}
		}
		return RD_ANCHORX_CUSTOM | RD_ANCHORY_BOTTOM;

	case IDC_MULTISPLITTER:
		if (bInfoPanel)
			urc->rcItem.top += panelHeight;
		urc->rcItem.left -= m_iMultiSplit;
		urc->rcItem.right -= m_iMultiSplit;
		urc->rcItem.bottom = rcLogBottom;
		return RD_ANCHORX_RIGHT | RD_ANCHORY_HEIGHT;

	case IDC_LOGFROZENTEXT:
		urc->rcItem.right = urc->dlgNewSize.cx - 50;
		urc->rcItem.bottom = msgTop - (bBottomToolbar ? 0 : 28);
		urc->rcItem.top = msgTop - 16 - (bBottomToolbar ? 0 : 28);
		if (!bShowToolbar && !bBottomToolbar) {
			urc->rcItem.bottom += 21;
			urc->rcItem.top += 21;
		}
		return RD_ANCHORX_CUSTOM | RD_ANCHORY_BOTTOM;

	case IDC_ADD:
		urc->rcItem.bottom = msgTop - (bBottomToolbar ? 0 : 28);
		urc->rcItem.top = msgTop - 18 - (bBottomToolbar ? 0 : 28);
		urc->rcItem.right = urc->dlgNewSize.cx - 28;
		urc->rcItem.left = urc->rcItem.right - 20;
		if (!bShowToolbar && !bBottomToolbar) {
			urc->rcItem.bottom += 21;
			urc->rcItem.top += 21;
		}
		return RD_ANCHORX_CUSTOM | RD_ANCHORY_BOTTOM;

	case IDC_CANCELADD:
		urc->rcItem.bottom = msgTop - (bBottomToolbar ? 0 : 28);
		urc->rcItem.top = msgTop - 18 - (bBottomToolbar ? 0 : 28);
		urc->rcItem.right = urc->dlgNewSize.cx - 4;
		urc->rcItem.left = urc->rcItem.right - 20;
		if (!bShowToolbar && !bBottomToolbar) {
			urc->rcItem.bottom += 21;
			urc->rcItem.top += 21;
		}
		return RD_ANCHORX_CUSTOM | RD_ANCHORY_BOTTOM;

	case IDC_TOGGLESIDEBAR:
		return RD_ANCHORX_CUSTOM | RD_ANCHORY_CUSTOM;

	case IDC_RETRY:
	case IDC_CANCELSEND:
	case IDC_MSGSENDLATER:
		if (bErrorState) {
			urc->rcItem.bottom = msgTop - 5 - (bBottomToolbar ? 0 : 28) - ((m_bNotOnList || m_dwFlagsEx & MWF_SHOW_SCROLLINGDISABLED) ? 20 : 0);
			urc->rcItem.top = msgTop - 25 - (bBottomToolbar ? 0 : 28) - ((m_bNotOnList || m_dwFlagsEx & MWF_SHOW_SCROLLINGDISABLED) ? 20 : 0);
		}
		if (!bShowToolbar && !bBottomToolbar) {
			urc->rcItem.bottom += 21;
			urc->rcItem.top += 21;
		}
		return RD_ANCHORX_LEFT | RD_ANCHORY_BOTTOM;

	case IDC_STATICTEXT:
	case IDC_STATICERRORICON:
		if (bErrorState) {
			urc->rcItem.bottom = msgTop - 28 - (bBottomToolbar ? 0 : 28) - ((m_bNotOnList || m_dwFlagsEx & MWF_SHOW_SCROLLINGDISABLED) ? 20 : 0);
			urc->rcItem.top = msgTop - 45 - (bBottomToolbar ? 0 : 28) - ((m_bNotOnList || m_dwFlagsEx & MWF_SHOW_SCROLLINGDISABLED) ? 20 : 0);
		}
		if (!bShowToolbar && !bBottomToolbar) {
			urc->rcItem.bottom += 21;
			urc->rcItem.top += 21;
		}
		return RD_ANCHORX_LEFT | RD_ANCHORY_BOTTOM;
	}
	return RD_ANCHORX_LEFT | RD_ANCHORY_BOTTOM;
}

/////////////////////////////////////////////////////////////////////////////////////////
// message filter

int CSrmmWindow::OnFilter(MSGFILTER *pFilter)
{
	RECT rc;
	POINT pt;
	DWORD msg = pFilter->msg;
	WPARAM wp = pFilter->wParam;
	LPARAM lp = pFilter->lParam;

	bool isCtrl, isShift, isAlt;
	KbdState(isShift, isCtrl, isAlt);

	if (msg == WM_SYSKEYUP) {
		if (wp == VK_MENU)
			if (!m_bkeyProcessed && !(GetKeyState(VK_CONTROL) & 0x8000) && !(GetKeyState(VK_SHIFT) & 0x8000) && !(lp & (1 << 24)))
				m_pContainer->MenuBar->autoShow();

		return _dlgReturn(m_hwnd, 0);
	}

	if ((msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN) && !(GetKeyState(VK_RMENU) & 0x8000)) {
		MSG message = { m_hwnd, msg, wp, lp };
		LRESULT mim_hotkey_check = Hotkey_Check(&message, TABSRMM_HK_SECTION_IM);
		if (mim_hotkey_check)
			m_bkeyProcessed = true;

		switch (mim_hotkey_check) {
		case TABSRMM_HK_SETUSERPREFS:
			CallService(MS_TABMSG_SETUSERPREFS, m_hContact, 0);
			return _dlgReturn(m_hwnd, 1);
		case TABSRMM_HK_SENDFILE:
			CallService(MS_FILE_SENDFILE, m_hContact, 0);
			return _dlgReturn(m_hwnd, 1);
		case TABSRMM_HK_QUOTEMSG:
			SendMessage(m_hwnd, WM_COMMAND, IDC_QUOTE, 0);
			return _dlgReturn(m_hwnd, 1);
		case TABSRMM_HK_USERMENU:
			SendMessage(m_hwnd, WM_COMMAND, IDC_PROTOCOL, 0);
			return _dlgReturn(m_hwnd, 1);
		case TABSRMM_HK_SENDMENU:
			SendMessage(m_hwnd, WM_COMMAND, IDC_SENDMENU, IDC_SENDMENU);
			return _dlgReturn(m_hwnd, 1);
		case TABSRMM_HK_PROTOMENU:
			SendMessage(m_hwnd, WM_COMMAND, IDC_PROTOMENU, IDC_PROTOMENU);
			return _dlgReturn(m_hwnd, 1);
		case TABSRMM_HK_USERDETAILS:
			SendMessage(m_hwnd, WM_COMMAND, MAKELONG(IDC_NAME, BN_CLICKED), 0);
			return _dlgReturn(m_hwnd, 1);
		case TABSRMM_HK_EDITNOTES:
			PostMessage(m_hwnd, WM_COMMAND, MAKELONG(IDC_PIC, BN_CLICKED), 0);
			return _dlgReturn(m_hwnd, 1);
		case TABSRMM_HK_TOGGLESENDLATER:
			if (sendLater->isAvail()) {
				m_sendMode ^= SMODE_SENDLATER;
				SetWindowPos(m_message.GetHwnd(), nullptr, 0, 0, 0, 0, SWP_DRAWFRAME | SWP_FRAMECHANGED | SWP_NOZORDER |
					SWP_NOMOVE | SWP_NOSIZE | SWP_NOCOPYBITS);
				RedrawWindow(m_hwnd, nullptr, nullptr, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW | RDW_ALLCHILDREN);
			}
			else
				CWarning::show(CWarning::WARN_NO_SENDLATER, MB_OK | MB_ICONINFORMATION, TranslateT("Configuration issue|The unattended send feature is disabled. The \\b1 send later\\b0  and \\b1 send to multiple contacts\\b0  features depend on it.\n\nYou must enable it under \\b1Options -> Message sessions -> Advanced tweaks\\b0. Changing this option requires a restart."));
			return _dlgReturn(m_hwnd, 1);
		case TABSRMM_HK_TOGGLERTL:
			m_dwFlags ^= MWF_LOG_RTL;
			{
				DWORD	dwGlobal = M.GetDword("mwflags", MWF_LOG_DEFAULT);
				DWORD	dwMask = M.GetDword(m_hContact, "mwmask", 0);
				DWORD	dwFlags = M.GetDword(m_hContact, "mwflags", 0);

				if ((dwGlobal & MWF_LOG_RTL) != (dwFlags & MWF_LOG_RTL)) {
					dwMask |= MWF_LOG_RTL;
					dwFlags |= (dwFlags & MWF_LOG_RTL);
				}
				else {
					dwMask &= ~MWF_LOG_RTL;
					dwFlags &= ~MWF_LOG_RTL;
				}
				if (dwMask) {
					db_set_dw(m_hContact, SRMSGMOD_T, "mwmask", dwMask);
					db_set_dw(m_hContact, SRMSGMOD_T, "mwflags", dwFlags);
				}
				else {
					db_unset(m_hContact, SRMSGMOD_T, "mwmask");
					db_unset(m_hContact, SRMSGMOD_T, "mwflags");
				}
				DM_OptionsApplied(0, 0);
				SendMessage(m_hwnd, DM_DEFERREDREMAKELOG, (WPARAM)m_hwnd, 0);
			}
			return _dlgReturn(m_hwnd, 1);

		case TABSRMM_HK_TOGGLEMULTISEND:
			m_sendMode ^= SMODE_MULTIPLE;
			if (m_sendMode & SMODE_MULTIPLE)
				DM_CreateClist();
			else if (IsWindow(GetDlgItem(m_hwnd, IDC_CLIST)))
				DestroyWindow(GetDlgItem(m_hwnd, IDC_CLIST));

			HWND hwndEdit = m_message.GetHwnd();
			SetWindowPos(hwndEdit, nullptr, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOSIZE | SWP_NOMOVE);
			Resize();
			RedrawWindow(hwndEdit, nullptr, nullptr, RDW_INVALIDATE | RDW_FRAME | RDW_UPDATENOW | RDW_ERASE);
			DM_ScrollToBottom(0, 0);
			Utils::showDlgControl(m_hwnd, IDC_MULTISPLITTER, (m_sendMode & SMODE_MULTIPLE) ? SW_SHOW : SW_HIDE);
			Utils::showDlgControl(m_hwnd, IDC_CLIST, (m_sendMode & SMODE_MULTIPLE) ? SW_SHOW : SW_HIDE);
			if (m_sendMode & SMODE_MULTIPLE)
				SetFocus(GetDlgItem(m_hwnd, IDC_CLIST));
			else
				SetFocus(m_message.GetHwnd());
			RedrawWindow(m_hwnd, nullptr, nullptr, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW | RDW_ALLCHILDREN);
			return _dlgReturn(m_hwnd, 1);
		}
		if (DM_GenericHotkeysCheck(&message)) {
			m_bkeyProcessed = true;
			return _dlgReturn(m_hwnd, 1);
		}
	}
	if (wp == VK_BROWSER_BACK || wp == VK_BROWSER_FORWARD)
		return 1;

	if (msg == WM_CHAR) {
		if (isCtrl && !isShift && !isAlt) {
			switch (wp) {
			case 23:                // ctrl - w
				PostMessage(m_hwnd, WM_CLOSE, 1, 0);
				break;
			case 20:                // ctrl - t
				PostMessage(m_hwnd, WM_COMMAND, IDC_TOGGLETOOLBAR, 1);
				break;
			}
			return 1;
		}
	}
	if (msg == WM_KEYDOWN) {
		if ((wp == VK_INSERT && isShift && !isCtrl) || (wp == 'V' && isCtrl && !isShift && !isAlt)) {
			m_message.SendMsg(EM_PASTESPECIAL, CF_UNICODETEXT, 0);
			_clrMsgFilter(pFilter);
			return _dlgReturn(m_hwnd, 1);
		}
		if (isCtrl && isShift) {
			if (wp == 0x9) {            // ctrl-shift tab
				SendMessage(m_hwnd, DM_SELECTTAB, DM_SELECT_PREV, 0);
				_clrMsgFilter(pFilter);
				return _dlgReturn(m_hwnd, 1);
			}
		}
		if (isCtrl && !isShift && !isAlt) {
			if (wp == VK_TAB) {
				SendMessage(m_hwnd, DM_SELECTTAB, DM_SELECT_NEXT, 0);
				_clrMsgFilter(pFilter);
				return _dlgReturn(m_hwnd, 1);
			}
			if (wp == VK_F4) {
				PostMessage(m_hwnd, WM_CLOSE, 1, 0);
				return _dlgReturn(m_hwnd, 1);
			}
			if (wp == VK_PRIOR) {
				SendMessage(m_hwnd, DM_SELECTTAB, DM_SELECT_PREV, 0);
				return _dlgReturn(m_hwnd, 1);
			}
			if (wp == VK_NEXT) {
				SendMessage(m_hwnd, DM_SELECTTAB, DM_SELECT_NEXT, 0);
				return _dlgReturn(m_hwnd, 1);
			}
		}
	}
	if (msg == WM_SYSKEYDOWN && isAlt) {
		if (wp == 0x52) {
			SendMessage(m_hwnd, DM_QUERYPENDING, DM_QUERY_MOSTRECENT, 0);
			return _dlgReturn(m_hwnd, 1);
		}
		if (wp == VK_MULTIPLY) {
			SetFocus(m_message.GetHwnd());
			return _dlgReturn(m_hwnd, 1);
		}
		if (wp == VK_DIVIDE) {
			SetFocus(m_log.GetHwnd());
			return _dlgReturn(m_hwnd, 1);
		}
		if (wp == VK_ADD) {
			SendMessage(m_pContainer->m_hwnd, DM_SELECTTAB, DM_SELECT_NEXT, 0);
			return _dlgReturn(m_hwnd, 1);
		}
		if (wp == VK_SUBTRACT) {
			SendMessage(m_pContainer->m_hwnd, DM_SELECTTAB, DM_SELECT_PREV, 0);
			return _dlgReturn(m_hwnd, 1);
		}
	}

	if (msg == WM_KEYDOWN && wp == VK_F12) {
		if (isShift || isCtrl || isAlt)
			return _dlgReturn(m_hwnd, 1);
		if (m_dwFlagsEx & MWF_SHOW_SCROLLINGDISABLED)
			ReplayQueue();
		m_dwFlagsEx ^= MWF_SHOW_SCROLLINGDISABLED;
		Utils::showDlgControl(m_hwnd, IDC_LOGFROZENTEXT, (m_bNotOnList || m_dwFlagsEx & MWF_SHOW_SCROLLINGDISABLED) ? SW_SHOW : SW_HIDE);
		if (!(m_dwFlagsEx & MWF_SHOW_SCROLLINGDISABLED))
			SetDlgItemText(m_hwnd, IDC_LOGFROZENTEXT, TranslateT("Contact not on list. You may add it..."));
		else
			SetDlgItemText(m_hwnd, IDC_LOGFROZENTEXT, TranslateT("Auto scrolling is disabled (press F12 to enable it)"));
		Resize();
		DM_ScrollToBottom(1, 1);
		return _dlgReturn(m_hwnd, 1);
	}

	// tabulation mod
	if (msg == WM_KEYDOWN && wp == VK_TAB) {
		if (PluginConfig.m_bAllowTab) {
			if (pFilter->nmhdr.idFrom == IDC_SRMM_MESSAGE)
				m_message.SendMsg(EM_REPLACESEL, FALSE, (LPARAM)"\t");
			_clrMsgFilter(pFilter);
			if (pFilter->nmhdr.idFrom != IDC_SRMM_MESSAGE)
				SetFocus(m_message.GetHwnd());
			return _dlgReturn(m_hwnd, 1);
		}

		if (pFilter->nmhdr.idFrom == IDC_SRMM_MESSAGE) {
			if (GetSendButtonState(m_hwnd) != PBS_DISABLED && !(m_pContainer->dwFlags & CNT_HIDETOOLBAR))
				SetFocus(GetDlgItem(m_hwnd, IDOK));
			else
				SetFocus(m_log.GetHwnd());
			return _dlgReturn(m_hwnd, 1);
		}

		if (pFilter->nmhdr.idFrom == IDC_SRMM_LOG) {
			SetFocus(m_message.GetHwnd());
			return _dlgReturn(m_hwnd, 1);
		}

		return _dlgReturn(m_hwnd, 0);
	}

	if (msg == WM_MOUSEWHEEL && (pFilter->nmhdr.idFrom == IDC_SRMM_LOG || pFilter->nmhdr.idFrom == IDC_SRMM_MESSAGE)) {
		GetCursorPos(&pt);
		GetWindowRect(m_log.GetHwnd(), &rc);
		if (PtInRect(&rc, pt)) {
			short wDirection = (short)HIWORD(wp);
			if (LOWORD(wp) & MK_SHIFT) {
				if (wDirection < 0)
					m_log.SendMsg(WM_VSCROLL, MAKEWPARAM(SB_PAGEDOWN, 0), 0);
				else if (wDirection > 0)
					m_log.SendMsg(WM_VSCROLL, MAKEWPARAM(SB_PAGEUP, 0), 0);
				return 0;
			}
			return 0;
		}
		return 1;
	}

	if (msg == WM_CHAR && wp == 'c') {
		if (isCtrl) {
			SendDlgItemMessage(m_hwnd, pFilter->nmhdr.code, WM_COPY, 0, 0);
			return 0;
		}
	}

	if ((msg == WM_LBUTTONDOWN || msg == WM_KEYUP || msg == WM_LBUTTONUP) && pFilter->nmhdr.idFrom == IDC_SRMM_MESSAGE) {
		int bBold = IsDlgButtonChecked(m_hwnd, IDC_SRMM_BOLD);
		int bItalic = IsDlgButtonChecked(m_hwnd, IDC_SRMM_ITALICS);
		int bUnder = IsDlgButtonChecked(m_hwnd, IDC_SRMM_UNDERLINE);
		int bStrikeout = IsDlgButtonChecked(m_hwnd, IDC_FONTSTRIKEOUT);

		CHARFORMAT2 cf2;
		cf2.cbSize = sizeof(CHARFORMAT2);
		cf2.dwMask = CFM_BOLD | CFM_ITALIC | CFM_UNDERLINE | CFM_UNDERLINETYPE | CFM_STRIKEOUT;
		cf2.dwEffects = 0;
		m_message.SendMsg(EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf2);
		if (cf2.dwEffects & CFE_BOLD) {
			if (bBold == BST_UNCHECKED)
				CheckDlgButton(m_hwnd, IDC_SRMM_BOLD, BST_CHECKED);
		}
		else if (bBold == BST_CHECKED)
			CheckDlgButton(m_hwnd, IDC_SRMM_BOLD, BST_UNCHECKED);

		if (cf2.dwEffects & CFE_ITALIC) {
			if (bItalic == BST_UNCHECKED)
				CheckDlgButton(m_hwnd, IDC_SRMM_ITALICS, BST_CHECKED);
		}
		else if (bItalic == BST_CHECKED)
			CheckDlgButton(m_hwnd, IDC_SRMM_ITALICS, BST_UNCHECKED);

		if (cf2.dwEffects & CFE_UNDERLINE && (cf2.bUnderlineType & CFU_UNDERLINE || cf2.bUnderlineType & CFU_UNDERLINEWORD)) {
			if (bUnder == BST_UNCHECKED)
				CheckDlgButton(m_hwnd, IDC_SRMM_UNDERLINE, BST_CHECKED);
		}
		else if (bUnder == BST_CHECKED)
			CheckDlgButton(m_hwnd, IDC_SRMM_UNDERLINE, BST_UNCHECKED);

		if (cf2.dwEffects & CFE_STRIKEOUT) {
			if (bStrikeout == BST_UNCHECKED)
				CheckDlgButton(m_hwnd, IDC_FONTSTRIKEOUT, BST_CHECKED);
		}
		else if (bStrikeout == BST_CHECKED)
			CheckDlgButton(m_hwnd, IDC_FONTSTRIKEOUT, BST_UNCHECKED);
	}
	switch (msg) {
	case WM_LBUTTONDOWN:
		{
			HCURSOR hCur = GetCursor();
			m_pContainer->MenuBar->Cancel();
			if (hCur == LoadCursor(nullptr, IDC_SIZENS) || hCur == LoadCursor(nullptr, IDC_SIZEWE)
				|| hCur == LoadCursor(nullptr, IDC_SIZENESW) || hCur == LoadCursor(nullptr, IDC_SIZENWSE)) {
				SetWindowLongPtr(m_hwnd, DWLP_MSGRESULT, TRUE);
				return TRUE;
			}
		}
		break;

	case WM_LBUTTONUP:
		// auto-select-and-copy handling...
		// if enabled, releasing the lmb with an active selection automatically copies the selection
		// to the clipboard.
		// holding ctrl while releasing the button pastes the selection to the input area, using plain text
		// holding ctrl-alt does the same, but pastes formatted text
		if (pFilter->nmhdr.idFrom == IDC_SRMM_LOG && M.GetByte("autocopy", 1)) {
			CHARRANGE cr;
			m_log.SendMsg(EM_EXGETSEL, 0, (LPARAM)&cr);
			if (cr.cpMax == cr.cpMin)
				break;
			cr.cpMin = cr.cpMax;
			if (isCtrl) {
				SETTEXTEX stx = { ST_KEEPUNDO | ST_SELECTION, CP_UTF8 };
				ptrA streamOut(m_log.GetRichTextRtf(!isAlt, true));
				if (streamOut) {
					Utils::FilterEventMarkers(streamOut);
					m_message.SendMsg(EM_SETTEXTEX, (WPARAM)&stx, (LPARAM)streamOut);
				}
				SetFocus(m_message.GetHwnd());
			}
			else if (!isShift) {
				m_log.SendMsg(WM_COPY, 0, 0);
				SetFocus(m_message.GetHwnd());
				if (m_pContainer->hwndStatus)
					SendMessage(m_pContainer->hwndStatus, SB_SETTEXT, 0, (LPARAM)TranslateT("Selection copied to clipboard"));
			}
		}
		break;

	case WM_RBUTTONUP:
		return _dlgReturn(m_hwnd, 1);

	case WM_MOUSEMOVE:
		GetCursorPos(&pt);
		DM_DismissTip(pt);
		m_pPanel.trackMouse(pt);

		HCURSOR hCur = GetCursor();
		if (hCur == LoadCursor(nullptr, IDC_SIZENS) || hCur == LoadCursor(nullptr, IDC_SIZEWE) || hCur == LoadCursor(nullptr, IDC_SIZENESW) || hCur == LoadCursor(nullptr, IDC_SIZENWSE))
			SetCursor(LoadCursor(nullptr, IDC_ARROW));
		break;
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

LRESULT CSrmmWindow::WndProc_Log(UINT msg, WPARAM wParam, LPARAM lParam)
{
	bool isCtrl, isShift, isAlt;

	switch (msg) {
	case WM_KILLFOCUS:
		if (wParam != (WPARAM)m_log.GetHwnd() && 0 != wParam) {
			CHARRANGE cr;
			m_log.SendMsg(EM_EXGETSEL, 0, (LPARAM)&cr);
			if (cr.cpMax != cr.cpMin) {
				cr.cpMin = cr.cpMax;
				m_log.SendMsg(EM_EXSETSEL, 0, (LPARAM)&cr);
			}
		}
		break;

	case WM_CHAR:
		KbdState(isShift, isCtrl, isAlt);
		if (wParam == 0x03 && isCtrl) // Ctrl+C
			return WMCopyHandler(msg, wParam, lParam);
		if (wParam == 0x11 && isCtrl) // Ctrl+Q
			m_btnQuote.Click();
		break;

	case WM_SYSKEYUP:
		if (wParam == VK_MENU) {
			ProcessHotkeysByMsgFilter(m_log, msg, wParam, lParam);
			return 0;
		}
		break;

	case WM_SYSKEYDOWN:
		m_bkeyProcessed = false;
		if (ProcessHotkeysByMsgFilter(m_log, msg, wParam, lParam)) {
			m_bkeyProcessed = true;
			return 0;
		}
		break;

	case WM_SYSCHAR:
		if (m_bkeyProcessed) {
			m_bkeyProcessed = false;
			return 0;
		}
		break;

	case WM_KEYDOWN:
		KbdState(isShift, isCtrl, isAlt);
		if (wParam == VK_INSERT && isCtrl)
			return WMCopyHandler(msg, wParam, lParam);
		break;

	case WM_COPY:
		return WMCopyHandler(msg, wParam, lParam);

	case WM_NCCALCSIZE:
		return CSkin::NcCalcRichEditFrame(m_log.GetHwnd(), this, ID_EXTBKHISTORY, msg, wParam, lParam, stubLogProc);

	case WM_NCPAINT:
		return CSkin::DrawRichEditFrame(m_log.GetHwnd(), this, ID_EXTBKHISTORY, msg, wParam, lParam, stubLogProc);

	case WM_CONTEXTMENU:
		POINT pt;

		if (lParam == 0xFFFFFFFF) {
			CHARRANGE sel;
			m_log.SendMsg(EM_EXGETSEL, 0, (LPARAM)&sel);
			m_log.SendMsg(EM_POSFROMCHAR, (WPARAM)&pt, (LPARAM)sel.cpMax);
			ClientToScreen(m_log.GetHwnd(), &pt);
		}
		else {
			pt.x = GET_X_LPARAM(lParam);
			pt.y = GET_Y_LPARAM(lParam);
		}

		ShowPopupMenu(m_log, pt);
		return TRUE;
	}

	return CSuper::WndProc_Log(msg, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////////////////

LRESULT CSrmmWindow::WndProc_Message(UINT msg, WPARAM wParam, LPARAM lParam)
{
	bool isCtrl, isShift, isAlt;

	// prevent the rich edit from switching text direction or keyboard layout when
	// using hotkeys with ctrl-shift or alt-shift modifiers
	if (m_bkeyProcessed && (msg == WM_KEYUP)) {
		GetKeyboardState(kstate);
		if (kstate[VK_CONTROL] & 0x80 || kstate[VK_SHIFT] & 0x80)
			return 0;

		m_bkeyProcessed = false;
		return 0;
	}

	switch (msg) {
	case WM_NCCALCSIZE:
		return CSkin::NcCalcRichEditFrame(m_message.GetHwnd(), this, ID_EXTBKINPUTAREA, msg, wParam, lParam, stubMessageProc);

	case WM_NCPAINT:
		return CSkin::DrawRichEditFrame(m_message.GetHwnd(), this, ID_EXTBKINPUTAREA, msg, wParam, lParam, stubMessageProc);

	case WM_DROPFILES:
		SendMessage(m_hwnd, WM_DROPFILES, (WPARAM)wParam, (LPARAM)lParam);
		return 0;

	case WM_CHAR:
		KbdState(isShift, isCtrl, isAlt);

		if (PluginConfig.m_bSoundOnTyping && !isAlt && !isCtrl && !(m_pContainer->dwFlags & CNT_NOSOUND) && wParam != VK_ESCAPE && !(wParam == VK_TAB && PluginConfig.m_bAllowTab))
			Skin_PlaySound("SoundOnTyping");

		if (isCtrl && !isAlt) {
			switch (wParam) {
			case 0x02:               // bold
				if (m_SendFormat)
					m_btnBold.Click();
				return 0;
			case 0x09:
				if (m_SendFormat)
					m_btnItalic.Click();
				return 0;
			case 21:
				if (m_SendFormat)
					m_btnUnderline.Click();
				return 0;
			case 0x0b:
				m_message.SetText(L"");
				return 0;
			}
		}
		break;

	case WM_MOUSEWHEEL:
		if (DM_MouseWheelHandler(wParam, lParam) == 0)
			return 0;
		break;

	case EM_PASTESPECIAL:
	case WM_PASTE:
		if (OpenClipboard(m_message.GetHwnd())) {
			HANDLE hClip = GetClipboardData(CF_TEXT);
			if (hClip) {
				if ((int)mir_strlen((char*)hClip) > m_nMax) {
					wchar_t szBuffer[512];
					if (M.GetByte("autosplit", 0))
						mir_snwprintf(szBuffer, TranslateT("WARNING: The message you are trying to paste exceeds the message size limit for the active protocol. It will be sent in chunks of max %d characters"), m_nMax - 10);
					else
						mir_snwprintf(szBuffer, TranslateT("The message you are trying to paste exceeds the message size limit for the active protocol. Only the first %d characters will be sent."), m_nMax);
					SendMessage(m_hwnd, DM_ACTIVATETOOLTIP, IDC_SRMM_MESSAGE, (LPARAM)szBuffer);
				}
			}
			else if (hClip = GetClipboardData(CF_BITMAP))
				SendHBitmapAsFile((HBITMAP)hClip);

			CloseClipboard();
		}
		break;

	case WM_KEYDOWN:
		KbdState(isShift, isCtrl, isAlt);

		if (PluginConfig.m_bSoundOnTyping && !isAlt && !(m_pContainer->dwFlags & CNT_NOSOUND) && wParam == VK_DELETE)
			Skin_PlaySound("SoundOnTyping");

		if (wParam == VK_INSERT && !isShift && !isCtrl && !isAlt) {
			m_bInsertMode = !m_bInsertMode;
			m_message.OnChange(&m_message);
		}
		if (wParam == VK_CAPITAL || wParam == VK_NUMLOCK)
			m_message.OnChange(&m_message);

		if (wParam == VK_RETURN) {
			if (m_bEditNotesActive)
				break;

			if (isShift) {
				if (PluginConfig.m_bSendOnShiftEnter) {
					PostMessage(m_hwnd, WM_COMMAND, IDOK, 0);
					return 0;
				}
				else break;
			}
			if ((isCtrl && !isShift) ^ (0 != PluginConfig.m_bSendOnEnter)) {
				PostMessage(m_hwnd, WM_COMMAND, IDOK, 0);
				return 0;
			}
			if (PluginConfig.m_bSendOnEnter || PluginConfig.m_bSendOnDblEnter) {
				if (isCtrl)
					break;

				if (PluginConfig.m_bSendOnDblEnter) {
					if (m_iLastEnterTime + 2 < time(0)) {
						m_iLastEnterTime = time(0);
						break;
					}
					else {
						m_message.SendMsg(WM_KEYDOWN, VK_BACK, 0);
						m_message.SendMsg(WM_KEYUP, VK_BACK, 0);
						PostMessage(m_hwnd, WM_COMMAND, IDOK, 0);
						return 0;
					}
				}
				PostMessage(m_hwnd, WM_COMMAND, IDOK, 0);
				return 0;
			}
			else break;
		}
		else m_iLastEnterTime = 0;

		if (isCtrl && !isAlt && !isShift) {
			if (!isShift && (wParam == VK_UP || wParam == VK_DOWN)) {          // input history scrolling (ctrl-up / down)
				m_iLastEnterTime = 0;
				m_cache->inputHistoryEvent(wParam);
				return 0;
			}
		}
		if (isCtrl && isAlt && !isShift) {
			switch (wParam) {
			case VK_UP:
			case VK_DOWN:
			case VK_PRIOR:
			case VK_NEXT:
			case VK_HOME:
			case VK_END:
				WPARAM wp = 0;

				m_iLastEnterTime = 0;
				if (wParam == VK_UP)
					wp = MAKEWPARAM(SB_LINEUP, 0);
				else if (wParam == VK_PRIOR)
					wp = MAKEWPARAM(SB_PAGEUP, 0);
				else if (wParam == VK_NEXT)
					wp = MAKEWPARAM(SB_PAGEDOWN, 0);
				else if (wParam == VK_HOME)
					wp = MAKEWPARAM(SB_TOP, 0);
				else if (wParam == VK_END) {
					DM_ScrollToBottom(0, 0);
					return 0;
				}
				else if (wParam == VK_DOWN)
					wp = MAKEWPARAM(SB_LINEDOWN, 0);

				if (m_hwndIEView == nullptr && m_hwndHPP == nullptr)
					m_log.SendMsg(WM_VSCROLL, wp, 0);
				else
					SendMessage(m_hwndIWebBrowserControl, WM_VSCROLL, wp, 0);
				return 0;
			}
		}

	case WM_SYSKEYDOWN:
		m_bkeyProcessed = false;
		if (ProcessHotkeysByMsgFilter(m_message, msg, wParam, lParam)) {
			m_bkeyProcessed = true;
			return 0;
		}
		break;

	case WM_SYSKEYUP:
		if (wParam == VK_MENU) {
			ProcessHotkeysByMsgFilter(m_message, msg, wParam, lParam);
			return 0;
		}
		break;

	case WM_SYSCHAR:
		if (m_bkeyProcessed) {
			m_bkeyProcessed = false;
			return 0;
		}

		KbdState(isShift, isCtrl, isAlt);
		if ((wParam >= '0' && wParam <= '9') && isAlt) {      // ALT-1 -> ALT-0 direct tab selection
			BYTE bChar = (BYTE)wParam;
			int iIndex;

			if (bChar == '0')
				iIndex = 10;
			else
				iIndex = bChar - (BYTE)'0';
			SendMessage(m_pContainer->m_hwnd, DM_SELECTTAB, DM_SELECT_BY_INDEX, (LPARAM)iIndex);
			return 0;
		}
		break;

	case WM_ERASEBKGND:
		return(CSkin::m_skinEnabled ? 0 : 1);

		// sent by smileyadd when the smiley selection window dies
		// just grab the focus :)
	case WM_USER + 100:
		SetFocus(m_message.GetHwnd());
		break;

	case WM_CONTEXTMENU:
		POINT pt;
		if (lParam == 0xFFFFFFFF) {
			CHARRANGE sel;
			m_message.SendMsg(EM_EXGETSEL, 0, (LPARAM)&sel);
			m_message.SendMsg(EM_POSFROMCHAR, (WPARAM)&pt, (LPARAM)sel.cpMax);
			ClientToScreen(m_message.GetHwnd(), &pt);
		}
		else {
			pt.x = GET_X_LPARAM(lParam);
			pt.y = GET_Y_LPARAM(lParam);
		}

		ShowPopupMenu(m_message, pt);
		return TRUE;
	}

	return CSuper::WndProc_Message(msg, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////////////////
// dialog procedure

INT_PTR CSrmmWindow::DlgProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	POINT pt, tmp, cur;
	RECT rc;

	switch (uMsg) {
	case WM_ERASEBKGND:
		RECT rcClient, rcWindow;
		{
			HDC hdc = (HDC)wParam;
			HDC hdcMem = nullptr;
			HBITMAP hbm, hbmOld;
			HANDLE hpb = nullptr;

			GetClientRect(m_hwnd, &rcClient);
			DWORD cx = rcClient.right - rcClient.left;
			DWORD cy = rcClient.bottom - rcClient.top;

			if (CMimAPI::m_haveBufferedPaint) {
				hpb = CMimAPI::m_pfnBeginBufferedPaint(hdc, &rcClient, BPBF_TOPDOWNDIB, nullptr, &hdcMem);
				hbm = hbmOld = nullptr;
			}
			else {
				hdcMem = CreateCompatibleDC(hdc);
				hbm = CSkin::CreateAeroCompatibleBitmap(rcClient, hdc);
				hbmOld = (HBITMAP)SelectObject(hdcMem, hbm);
			}

			bool	bAero = M.isAero();

			if (CSkin::m_skinEnabled) {
				UINT item_ids[2] = { ID_EXTBKHISTORY, ID_EXTBKINPUTAREA };
				UINT ctl_ids[2] = { IDC_SRMM_LOG, IDC_SRMM_MESSAGE };
				BOOL isEditNotesReason = m_bEditNotesActive;
				BOOL isSendLaterReason = (m_sendMode & SMODE_SENDLATER);
				BOOL isMultipleReason = (m_sendMode & SMODE_MULTIPLE || m_sendMode & SMODE_CONTAINER);

				CSkin::SkinDrawBG(m_hwnd, m_pContainer->m_hwnd, m_pContainer, &rcClient, hdcMem);

				for (int i = 0; i < 2; i++) {
					CSkinItem *item = &SkinItems[item_ids[i]];
					if (!item->IGNORED) {
						GetWindowRect(GetDlgItem(m_hwnd, ctl_ids[i]), &rcWindow);
						pt.x = rcWindow.left;
						pt.y = rcWindow.top;
						ScreenToClient(m_hwnd, &pt);
						rc.left = pt.x - item->MARGIN_LEFT;
						rc.top = pt.y - item->MARGIN_TOP;
						rc.right = rc.left + item->MARGIN_RIGHT + (rcWindow.right - rcWindow.left) + item->MARGIN_LEFT;
						rc.bottom = rc.top + item->MARGIN_BOTTOM + (rcWindow.bottom - rcWindow.top) + item->MARGIN_TOP;
						if (item_ids[i] == ID_EXTBKINPUTAREA && (isMultipleReason || isEditNotesReason || isSendLaterReason)) {
							HBRUSH br = CreateSolidBrush(isMultipleReason ? RGB(255, 130, 130) : (isEditNotesReason ? RGB(80, 255, 80) : RGB(80, 80, 255)));
							FillRect(hdcMem, &rc, br);
							DeleteObject(br);
						}
						else CSkin::DrawItem(hdcMem, &rc, item);
					}
				}
			}
			else {
				CSkin::FillBack(hdcMem, &rcClient);

				if (M.isAero()) {
					LONG temp = rcClient.bottom;
					rcClient.bottom = m_pPanel.isActive() ? m_pPanel.getHeight() + 5 : 5;
					FillRect(hdcMem, &rcClient, (HBRUSH)GetStockObject(BLACK_BRUSH));
					rcClient.bottom = temp;
				}
			}

			// draw the (new) infopanel background. Use the gradient from the statusitem.
			GetClientRect(m_hwnd, &rc);
			m_pPanel.renderBG(hdcMem, rc, &SkinItems[ID_EXTBKINFOPANELBG], bAero);

			// draw aero related stuff
			if (!CSkin::m_skinEnabled)
				RenderToolbarBG(hdcMem, rcClient);

			// render info panel fields
			m_pPanel.renderContent(hdcMem);

			if (hpb)
				CSkin::FinalizeBufferedPaint(hpb, &rcClient);
			else {
				BitBlt(hdc, 0, 0, cx, cy, hdcMem, 0, 0, SRCCOPY);
				SelectObject(hdcMem, hbmOld);
				DeleteObject(hbm);
				DeleteDC(hdcMem);
			}
			if (!m_fLimitedUpdate)
				SetAeroMargins(m_pContainer);
		}
		return 1;

	case WM_NCPAINT:
		if (CSkin::m_skinEnabled)
			return 0;
		break;

	case WM_PAINT:
		PAINTSTRUCT ps;
		BeginPaint(m_hwnd, &ps);
		EndPaint(m_hwnd, &ps);
		return 0;

	case WM_SIZE:
		if (!IsIconic(m_hwnd)) {
			if (m_ipFieldHeight == 0)
				m_ipFieldHeight = CInfoPanel::m_ipConfig.height2;

			if (m_pContainer->uChildMinHeight > 0 && HIWORD(lParam) >= m_pContainer->uChildMinHeight) {
				if (m_iSplitterY > HIWORD(lParam) - DPISCALEY_S(MINLOGHEIGHT)) {
					m_iSplitterY = HIWORD(lParam) - DPISCALEY_S(MINLOGHEIGHT);
					m_dynaSplitter = m_iSplitterY - DPISCALEY_S(34);
					DM_RecalcPictureSize();
				}
				if (m_iSplitterY < DPISCALEY_S(MINSPLITTERY))
					LoadSplitter();
			}

			HBITMAP hbm = ((m_pPanel.isActive()) && m_pContainer->avatarMode != 3) ? m_hOwnPic : (m_ace ? m_ace->hbmPic : PluginConfig.g_hbmUnknown);
			if (hbm != nullptr) {
				BITMAP bminfo;
				GetObject(hbm, sizeof(bminfo), &bminfo);
				CalcDynamicAvatarSize(&bminfo);
			}

			GetClientRect(m_hwnd, &rc);

			CTabBaseDlg::DlgProc(uMsg, 0, 0); // call basic window resizer

			BB_SetButtonsPos();

			// size info panel fields
			if (m_pPanel.isActive()) {
				LONG cx = rc.right;
				LONG panelHeight = m_pPanel.getHeight();

				hbm = (m_pContainer->avatarMode == 3) ? m_hOwnPic : (m_ace ? m_ace->hbmPic : PluginConfig.g_hbmUnknown);
				double dHeight = 0, dWidth = 0;
				Utils::scaleAvatarHeightLimited(hbm, dWidth, dHeight, panelHeight - 2);
				m_iPanelAvatarX = (int)dWidth;
				m_iPanelAvatarY = (int)dHeight;

				rc.top = 1;
				rc.left = cx - m_iPanelAvatarX;
				rc.bottom = panelHeight - (CSkin::m_bAvatarBorderType ? 2 : 0);
				rc.right = cx;
				m_rcPic = rc;

				if (m_bShowInfoAvatar) {
					SetWindowPos(m_hwndPanelPicParent, HWND_TOP, rc.left - 2, rc.top, rc.right - rc.left, rc.bottom - rc.top + 1, 0);
					ShowWindow(m_hwndPanelPicParent, (m_iPanelAvatarX == 0) || !m_pPanel.isActive() ? SW_HIDE : SW_SHOW);
				}
				else {
					ShowWindow(m_hwndPanelPicParent, SW_HIDE);
					m_iPanelAvatarX = m_iPanelAvatarY = 0;
				}

				rc.right = cx - m_iPanelAvatarX;
				rc.left = rc.right - m_panelStatusCX;
				rc.bottom = panelHeight - 3;
				rc.top = rc.bottom - m_ipFieldHeight;
				m_rcStatus = rc;

				rc.left = CInfoPanel::LEFT_OFFSET_LOGO;
				rc.right = cx - m_iPanelAvatarX - (panelHeight < CInfoPanel::DEGRADE_THRESHOLD ? (m_rcStatus.right - m_rcStatus.left) + 3 : 0);
				rc.bottom = panelHeight - (panelHeight >= CInfoPanel::DEGRADE_THRESHOLD ? m_ipFieldHeight : 0) - 1;
				rc.top = 1;
				m_rcNick = rc;

				rc.left = CInfoPanel::LEFT_OFFSET_LOGO;
				rc.right = cx - (m_iPanelAvatarX + 2) - m_panelStatusCX;
				rc.bottom = panelHeight - 3;
				rc.top = rc.bottom - m_ipFieldHeight;
				m_rcUIN = rc;

				m_pPanel.Invalidate();
			}

			if (GetDlgItem(m_hwnd, IDC_CLIST) != nullptr) {
				RECT rcLog;
				GetClientRect(m_hwnd, &rcClient);
				GetClientRect(m_log.GetHwnd(), &rcLog);
				rc.top = 0;
				rc.right = rcClient.right;
				rc.left = rcClient.right - m_iMultiSplit;
				rc.bottom = rcLog.bottom;
				if (m_pPanel.isActive())
					rc.top += (m_pPanel.getHeight() + 1);
				MoveWindow(GetDlgItem(m_hwnd, IDC_CLIST), rc.left, rc.top, rc.right - rc.left, rcLog.bottom - rcLog.top, FALSE);
			}

			if (m_hwndIEView || m_hwndHPP)
				ResizeIeView();

			DetermineMinHeight();
		}
		return 0;

	case WM_TIMECHANGE:
		DM_OptionsApplied(0, 0);
		break;

	case WM_NOTIFY:
		if (this != nullptr && ((NMHDR*)lParam)->hwndFrom == m_hwndTip) {
			if (((NMHDR*)lParam)->code == NM_CLICK)
				SendMessage(m_hwndTip, TTM_TRACKACTIVATE, FALSE, 0);
			break;
		}

		switch (((NMHDR*)lParam)->idFrom) {
		case IDC_SRMM_LOG:
		case IDC_SRMM_MESSAGE:
			switch (((NMHDR*)lParam)->code) {
			case EN_MSGFILTER:
				if (OnFilter((MSGFILTER*)lParam))
					return TRUE;
				break;

			case EN_REQUESTRESIZE:
				DM_HandleAutoSizeRequest((REQRESIZE *)lParam);
				break;
			}
		}
		break;

	case DM_TYPING:
		{
			int preTyping = m_nTypeSecs != 0;
			m_nTypeSecs = (int)lParam > 0 ? (int)lParam : 0;

			if (m_nTypeSecs)
				m_bShowTyping = 0;

			SetWindowLongPtr(m_hwnd, DWLP_MSGRESULT, preTyping);
		}
		return TRUE;

	case DM_UPDATEWINICON:
		if (m_hXStatusIcon) {
			DestroyIcon(m_hXStatusIcon);
			m_hXStatusIcon = nullptr;
		}

		if (LPCSTR szProto = m_cache->getProto()) {
			m_hTabIcon = m_hTabStatusIcon = GetMyContactIcon("MetaiconTab");
			if (M.GetByte("use_xicons", 1))
				m_hXStatusIcon = GetXStatusIcon();

			SendDlgItemMessage(m_hwnd, IDC_PROTOCOL, BUTTONSETASDIMMED, (m_dwFlagsEx & MWF_SHOW_ISIDLE) != 0, 0);
			SendDlgItemMessage(m_hwnd, IDC_PROTOCOL, BM_SETIMAGE, IMAGE_ICON, (LPARAM)(m_hXStatusIcon ? m_hXStatusIcon : GetMyContactIcon("MetaiconBar")));

			if (m_pContainer->m_hwndActive == m_hwnd)
				SendMessage(m_pContainer->m_hwnd, DM_SETICON, (WPARAM)this, (LPARAM)(m_hXStatusIcon ? m_hXStatusIcon : m_hTabIcon));

			if (m_pWnd)
				m_pWnd->updateIcon(m_hXStatusIcon ? m_hXStatusIcon : m_hTabIcon);
		}
		return 0;

	case DM_CONFIGURETOOLBAR:
		// configures the toolbar only... if lParam != 0, then it also calls
		// SetDialogToType() to reconfigure the message window
		m_bShowUIElements = m_pContainer->dwFlags & CNT_HIDETOOLBAR ? 0 : 1;

		SetWindowLongPtr(GetDlgItem(m_hwnd, IDC_SPLITTERY), GWL_EXSTYLE, GetWindowLongPtr(GetDlgItem(m_hwnd, IDC_SPLITTERY), GWL_EXSTYLE) & ~WS_EX_STATICEDGE);

		if (lParam == 1) {
			GetSendFormat();
			SetDialogToType();
		}

		if (lParam == 1) {
			DM_RecalcPictureSize();
			Resize();
			DM_ScrollToBottom(0, 1);
		}
		return 0;

	case WM_CBD_LOADICONS:
		for (auto &it : buttonicons) {
			SendDlgItemMessage(m_hwnd, it.id, BM_SETIMAGE, IMAGE_ICON, (LPARAM)*it.pIcon);
			SendDlgItemMessage(m_hwnd, it.id, BUTTONSETCONTAINER, (LPARAM)m_pContainer, 0);
		}

		Srmm_UpdateToolbarIcons(m_hwnd);
		SendMessage(m_hwnd, DM_UPDATEWINICON, 0, 0);
		return 0;

	case DM_OPTIONSAPPLIED:
		DM_OptionsApplied(wParam, lParam);
		return 0;

	case DM_UPDATESTATUSMSG:
		m_pPanel.Invalidate();
		return 0;

	case DM_OWNNICKCHANGED:
		GetMyNick();
		return 0;

	case WM_SETFOCUS:
		MsgWindowUpdateState(WM_SETFOCUS);
		SetFocus(m_message.GetHwnd());
		return 1;

	case WM_ACTIVATE:
		if (LOWORD(wParam) != WA_ACTIVE)
			break;
		__fallthrough;

	case WM_MOUSEACTIVATE:
		MsgWindowUpdateState(WM_ACTIVATE);
		return 1;

	case DM_UPDATEPICLAYOUT:
		if (wParam == 0 || wParam == m_hContact) {
			LoadContactAvatar();
			Resize();
		}
		return 0;

	case DM_SPLITTERMOVED:
		if ((HWND)lParam == GetDlgItem(m_hwnd, IDC_MULTISPLITTER)) {
			GetClientRect(m_hwnd, &rc);
			pt.x = wParam;
			pt.y = 0;
			ScreenToClient(m_hwnd, &pt);
			int oldSplitterX = m_iMultiSplit;
			m_iMultiSplit = rc.right - pt.x;
			if (m_iMultiSplit < 25)
				m_iMultiSplit = 25;

			if (m_iMultiSplit > ((rc.right - rc.left) - 80))
				m_iMultiSplit = oldSplitterX;
			Resize();
		}
		else if ((HWND)lParam == GetDlgItem(m_hwnd, IDC_SPLITTERY)) {
			GetClientRect(m_hwnd, &rc);
			rc.top += (m_pPanel.isActive() ? m_pPanel.getHeight() + 40 : 30);
			pt.x = 0;
			pt.y = wParam;
			ScreenToClient(m_hwnd, &pt);

			int oldSplitterY = m_iSplitterY;
			int oldDynaSplitter = m_dynaSplitter;

			m_iSplitterY = rc.bottom - pt.y + DPISCALEY_S(23);

			// attempt to fix splitter troubles..
			// hardcoded limits... better solution is possible, but this works for now
			int bottomtoolbarH = 0;
			if (m_pContainer->dwFlags & CNT_BOTTOMTOOLBAR)
				bottomtoolbarH = 22;

			if (m_iSplitterY < (DPISCALEY_S(MINSPLITTERY) + 5 + bottomtoolbarH)) {	// min splitter size
				m_iSplitterY = (DPISCALEY_S(MINSPLITTERY) + 5 + bottomtoolbarH);
				m_dynaSplitter = m_iSplitterY - DPISCALEY_S(34);
				DM_RecalcPictureSize();
			}
			else if (m_iSplitterY >(rc.bottom - rc.top)) {
				m_iSplitterY = oldSplitterY;
				m_dynaSplitter = oldDynaSplitter;
				DM_RecalcPictureSize();
			}
			else {
				m_dynaSplitter = (rc.bottom - pt.y) - DPISCALEY_S(11);
				DM_RecalcPictureSize();
			}
			UpdateToolbarBG();
			Resize();
		}
		else if ((HWND)lParam == GetDlgItem(m_hwnd, IDC_PANELSPLITTER)) {
			GetClientRect(m_log.GetHwnd(), &rc);

			POINT	pnt = { 0, (int)wParam };
			ScreenToClient(m_hwnd, &pnt);
			if ((pnt.y + 2 >= MIN_PANELHEIGHT + 2) && (pnt.y + 2 < 100) && (pnt.y + 2 < rc.bottom - 30))
				m_pPanel.setHeight(pnt.y + 2, true);

			RedrawWindow(m_hwnd, nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
			if (M.isAero())
				InvalidateRect(GetParent(m_hwnd), nullptr, FALSE);
		}
		break;

	case DM_DEFERREDREMAKELOG:
		// queue a dm_remakelog
		// wParam = hwnd of the sender, so we can directly do a DM_REMAKELOG if the msg came
		// from ourself. otherwise, the dm_remakelog will be deferred until next window
		// activation (focus)
		if ((HWND)wParam == m_hwnd)
			SendMessage(m_hwnd, DM_REMAKELOG, 0, 0);
		else {
			if (M.GetByte(m_hContact, "mwoverride", 0) == 0) {
				m_dwFlags &= ~(MWF_LOG_ALL);
				m_dwFlags |= (lParam & MWF_LOG_ALL);
				m_dwFlags |= MWF_DEFERREDREMAKELOG;
			}
		}
		return 0;

	case DM_FORCEDREMAKELOG:
		if ((HWND)wParam == m_hwnd)
			SendMessage(m_hwnd, DM_REMAKELOG, 0, 0);
		else {
			m_dwFlags &= ~(MWF_LOG_ALL);
			m_dwFlags |= (lParam & MWF_LOG_ALL);
			m_dwFlags |= MWF_DEFERREDREMAKELOG;
		}
		return 0;

	case DM_REMAKELOG:
		m_szMicroLf[0] = 0;
		m_lastEventTime = 0;
		m_iLastEventType = -1;
		StreamInEvents(m_hDbEventFirst, -1, 0, nullptr);
		return 0;

	case DM_APPENDMCEVENT:
		if (m_hContact == db_mc_getMeta(wParam) && m_hDbEventFirst == 0) {
			m_hDbEventFirst = lParam;
			SendMessage(m_hwnd, DM_REMAKELOG, 0, 0);
		}
		else if (m_hContact == wParam && db_mc_isSub(wParam) && db_event_getContact(lParam) != wParam)
			StreamInEvents(lParam, 1, 1, nullptr);
		return 0;

	case DM_SCROLLIEVIEW:
		{
			IEVIEWWINDOW iew = { sizeof(iew) };
			iew.iType = IEW_SCROLLBOTTOM;
			if (m_hwndIEView) {
				iew.hwnd = m_hwndIEView;
				CallService(MS_IEVIEW_WINDOW, 0, (LPARAM)&iew);
			}
			else if (m_hwndHPP) {
				iew.hwnd = m_hwndHPP;
				CallService(MS_HPP_EG_WINDOW, 0, (LPARAM)&iew);
			}
		}
		return 0;

	case HM_DBEVENTADDED:
		// this is called whenever a new event has been added to the database.
		// this CAN be posted (some sanity checks required).
		if (this && m_hContact)
			DM_EventAdded(m_hContact, lParam);
		return 0;

	case WM_TIMER:
		// timer to control info panel hovering
		if (wParam == TIMERID_AWAYMSG) {
			KillTimer(m_hwnd, wParam);
			GetCursorPos(&pt);

			if (wParam == TIMERID_AWAYMSG && m_pPanel.hitTest(pt) != CInfoPanel::HTNIRVANA)
				SendMessage(m_hwnd, DM_ACTIVATETOOLTIP, 0, 0);
			else
				m_dwFlagsEx &= ~MWF_SHOW_AWAYMSGTIMER;
			break;
		}

		// timer id for message timeouts is composed like:
		// for single message sends: basevalue (TIMERID_MSGSEND) + send queue index
		if (wParam >= TIMERID_MSGSEND) {
			int iIndex = wParam - TIMERID_MSGSEND;
			if (iIndex < SendQueue::NR_SENDJOBS) { // single sendjob timer
				SendJob *job = sendQueue->getJobByIndex(iIndex);
				KillTimer(m_hwnd, wParam);
				mir_snwprintf(job->szErrorMsg, TranslateT("Delivery failure: %s"), TranslateT("The message send timed out"));
				job->iStatus = SendQueue::SQ_ERROR;
				if (!nen_options.iNoSounds && !(m_pContainer->dwFlags & CNT_NOSOUND))
					Skin_PlaySound("SendError");
				if (!(m_dwFlags & MWF_ERRORSTATE))
					sendQueue->handleError(this, iIndex);
				break;
			}
		}
		else if (wParam == TIMERID_FLASHWND) {
			if (m_bCanFlashTab)
				FlashTab(true);
			break;
		}
		else if (wParam == TIMERID_TYPE) {
			DM_Typing(false);
			break;
		}
		break;

	case DM_SELECTTAB:
		SendMessage(m_pContainer->m_hwnd, DM_SELECTTAB, wParam, lParam);       // pass the msg to our container
		return 0;

		// return timestamp (in ticks) of last recent message which has not been read yet.
		// 0 if there is none
		// lParam = pointer to a dword receiving the value.
	case DM_QUERYLASTUNREAD:
		{
			DWORD *pdw = (DWORD *)lParam;
			if (pdw)
				*pdw = m_dwTickLastEvent;
		}
		return 0;

	case DM_UPDATELASTMESSAGE:
		DM_UpdateLastMessage();
		return 0;

	case DM_SAVESIZE:
		if (m_dwFlags & MWF_NEEDCHECKSIZE)
			lParam = 0;

		m_dwFlags &= ~MWF_NEEDCHECKSIZE;
		if (m_dwFlags & MWF_WASBACKGROUNDCREATE) {
			m_dwFlags &= ~MWF_INITMODE;
			if (m_lastMessage)
				DM_UpdateLastMessage();
		}

		SendMessage(m_pContainer->m_hwnd, DM_QUERYCLIENTAREA, 0, (LPARAM)&rcClient);
		MoveWindow(m_hwnd, rcClient.left, rcClient.top, (rcClient.right - rcClient.left), (rcClient.bottom - rcClient.top), TRUE);
		if (m_dwFlags & MWF_WASBACKGROUNDCREATE) {
			m_dwFlags &= ~MWF_WASBACKGROUNDCREATE;
			Resize();
			PostMessage(m_hwnd, DM_UPDATEPICLAYOUT, 0, 0);

			if (m_hwndIEView != nullptr)
				SetFocus(m_message.GetHwnd());
			if (m_pContainer->dwFlags & CNT_SIDEBAR)
				m_pContainer->SideBar->Layout();
		}
		else {
			Resize();
			if (lParam == 0)
				DM_ScrollToBottom(0, 1);
		}
		return 0;

	case DM_QUERYPENDING: // sent by the message input area hotkeys. just pass it to our container
		SendMessage(m_pContainer->m_hwnd, DM_QUERYPENDING, wParam, lParam);
		return 0;

	case WM_LBUTTONDOWN:
		GetCursorPos(&tmp);
		cur.x = (SHORT)tmp.x;
		cur.y = (SHORT)tmp.y;
		if (!m_pPanel.isHovered())
			SendMessage(m_pContainer->m_hwnd, WM_NCLBUTTONDOWN, HTCAPTION, *((LPARAM*)(&cur)));
		break;

	case WM_LBUTTONUP:
		GetCursorPos(&tmp);
		if (m_pPanel.isHovered())
			m_pPanel.handleClick(tmp);
		else {
			cur.x = (SHORT)tmp.x;
			cur.y = (SHORT)tmp.y;
			SendMessage(m_pContainer->m_hwnd, WM_NCLBUTTONUP, HTCAPTION, *((LPARAM*)(&cur)));
		}
		break;

	case WM_RBUTTONUP:
		{
			RECT rcPicture, rcPanelNick = { 0 };
			int menuID = 0;

			GetWindowRect(GetDlgItem(m_hwnd, IDC_CONTACTPIC), &rcPicture);
			rcPanelNick.left = rcPanelNick.right - 30;
			GetCursorPos(&pt);

			if (m_pPanel.invokeConfigDialog(pt))
				break;

			if (PtInRect(&rcPicture, pt))
				menuID = MENU_PICMENU;

			if ((menuID == MENU_PICMENU && ((m_ace ? m_ace->hbmPic : PluginConfig.g_hbmUnknown) || m_hOwnPic) && m_bShowAvatar != 0)) {
				HMENU submenu = GetSubMenu(PluginConfig.g_hMenuContext, 1);
				GetCursorPos(&pt);
				MsgWindowUpdateMenu(submenu, menuID);
				
				int iSelection = TrackPopupMenu(submenu, TPM_RETURNCMD, pt.x, pt.y, 0, m_hwnd, nullptr);
				MsgWindowMenuHandler(iSelection, menuID);
				break;
			}

			HMENU subMenu = GetSubMenu(PluginConfig.g_hMenuContext, 0);
			MsgWindowUpdateMenu(subMenu, MENU_TABCONTEXT);

			int iSelection = TrackPopupMenu(subMenu, TPM_RETURNCMD, pt.x, pt.y, 0, m_hwnd, nullptr);
			if (iSelection >= IDM_CONTAINERMENU) {
				char szIndex[10];
				char *szKey = "TAB_ContainersW";

				mir_snprintf(szIndex, "%d", iSelection - IDM_CONTAINERMENU);
				if (iSelection - IDM_CONTAINERMENU >= 0) {
					ptrW val(db_get_wsa(0, szKey, szIndex));
					if (val)
						SendMessage(m_hwnd, DM_CONTAINERSELECTED, 0, (LPARAM)val);
				}
				break;
			}
			MsgWindowMenuHandler(iSelection, MENU_TABCONTEXT);
		}
		break;

	case WM_MOUSEMOVE:
		GetCursorPos(&pt);
		DM_DismissTip(pt);
		m_pPanel.trackMouse(pt);
		break;

	case WM_MEASUREITEM:
		{
			LPMEASUREITEMSTRUCT lpmi = (LPMEASUREITEMSTRUCT)lParam;
			if (m_pPanel.isHovered()) {
				lpmi->itemHeight = 0;
				lpmi->itemWidth = 6;
				return TRUE;
			}
		}
		return Menu_MeasureItem(lParam);

	case WM_NCHITTEST:
		SendMessage(m_pContainer->m_hwnd, WM_NCHITTEST, wParam, lParam);
		break;

	case WM_DRAWITEM:
		return MsgWindowDrawHandler((DRAWITEMSTRUCT*)lParam);

	case WM_APPCOMMAND:
		{
			DWORD cmd = GET_APPCOMMAND_LPARAM(lParam);
			if (cmd == APPCOMMAND_BROWSER_BACKWARD || cmd == APPCOMMAND_BROWSER_FORWARD) {
				SendMessage(m_pContainer->m_hwnd, DM_SELECTTAB, cmd == APPCOMMAND_BROWSER_BACKWARD ? DM_SELECT_PREV : DM_SELECT_NEXT, 0);
				return 1;
			}
		}
		break;

	case WM_COMMAND:
		DM_MsgWindowCmdHandler(LOWORD(wParam), wParam, lParam);
		break;

	case WM_CONTEXTMENU:
		{
			DWORD idFrom = GetDlgCtrlID((HWND)wParam);
			if (idFrom >= MIN_CBUTTONID && idFrom <= MAX_CBUTTONID) {
				Srmm_ClickToolbarIcon(m_hContact, idFrom, m_hwnd, 1);
				break;
			}
		}
		break;

	case HM_EVENTSENT:
		// this is now *only* called from the global ME_PROTO_ACK handler (static int ProtoAck() in msgs.c)
		// it receives:
		// wParam = index of the sendjob in the queue in the low word, index of the found sendID in the high word
		// (normally 0, but if its a multisend job, then the sendjob may contain more than one hContact/iSendId pairs.)
		// lParam = the original ackdata
		//
		// the "per message window" ACK hook is gone, the global ack handler cares about all types of ack's (currently
		// *_MESSAGE and *_AVATAR and dispatches them to the owner windows).
		sendQueue->ackMessage(this, wParam, lParam);
		return 0;

	case DM_UINTOCLIPBOARD:
		Utils::CopyToClipBoard(m_cache->getUIN(), m_hwnd);
		return 0;

	case WM_NEXTDLGCTL:
		if (m_dwFlags & MWF_WASBACKGROUNDCREATE)
			return 1;
		break;

	case DM_IEVIEWOPTIONSCHANGED:
		if (m_hwndIEView)
			SendMessage(m_hwnd, DM_REMAKELOG, 0, 0);
		break;

	case DM_SMILEYOPTIONSCHANGED:
		SendMessage(m_hwnd, DM_REMAKELOG, 0, 0);
		break;

	case DM_MYAVATARCHANGED:
		{
			const char *szProto = m_cache->getActiveProto();
			if (!mir_strcmp((char *)wParam, szProto) && mir_strlen(szProto) == mir_strlen((char *)wParam))
				LoadOwnAvatar();
		}
		break;

	case DM_CLIENTCHANGED:
		GetClientIcon();
		if (m_hClientIcon && m_pPanel.isActive())
			InvalidateRect(m_hwnd, nullptr, TRUE);
		return 0;

	case DM_UPDATEUIN:
		if (m_pPanel.isActive())
			m_pPanel.Invalidate();
		if (m_pContainer->dwFlags & CNT_UINSTATUSBAR)
			tabUpdateStatusBar();
		return 0;

	case EM_THEMECHANGED:
		DM_FreeTheme();
		DM_ThemeChanged();
		return 0;

	case WM_CBD_UPDATED:
		if (lParam)
			CB_ChangeButton((CustomButtonData*)lParam);
		else
			BB_InitDlgButtons();

		BB_SetButtonsPos();
		return 0;

	case WM_CBD_REMOVED:
		if (lParam)
			CB_DestroyButton((DWORD)wParam, (DWORD)lParam);
		else
			CB_DestroyAllButtons();
		return 0;

	case WM_DROPFILES:
		{
			BOOL not_sending = GetKeyState(VK_CONTROL) & 0x8000;
			if (!not_sending) {
				const char *szProto = m_cache->getActiveProto();
				if (szProto == nullptr)
					break;

				int pcaps = CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_1, 0);
				if (!(pcaps & PF1_FILESEND))
					break;
				if (m_wStatus == ID_STATUS_OFFLINE) {
					pcaps = CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_4, 0);
					if (!(pcaps & PF4_OFFLINEFILES)) {
						SendMessage(m_hwnd, DM_ACTIVATETOOLTIP, IDC_SRMM_MESSAGE, (LPARAM)TranslateT("Contact is offline and this protocol does not support sending files to offline users."));
						break;
					}
				}
			}

			if (m_hContact != 0) {
				wchar_t szFilename[MAX_PATH];
				HDROP hDrop = (HDROP)wParam;
				int fileCount = DragQueryFile(hDrop, -1, nullptr, 0), totalCount = 0, i;
				wchar_t** ppFiles = nullptr;
				for (i = 0; i < fileCount; i++) {
					DragQueryFile(hDrop, i, szFilename, _countof(szFilename));
					Utils::AddToFileList(&ppFiles, &totalCount, szFilename);
				}

				if (!not_sending)
					CallService(MS_FILE_SENDSPECIFICFILEST, m_hContact, (LPARAM)ppFiles);
				else {
					if (ServiceExists(MS_HTTPSERVER_ADDFILENAME)) {
						for (i = 0; i < totalCount; i++) {
							char* szFileName = mir_u2a(ppFiles[i]);
							CallService(MS_HTTPSERVER_ADDFILENAME, (WPARAM)szFileName, 0);
							mir_free(szFileName);
						}
						char *szHTTPText = "DEBUG";
						SendDlgItemMessageA(m_hwnd, IDC_SRMM_MESSAGE, EM_REPLACESEL, TRUE, (LPARAM)szHTTPText);
						SetFocus(m_message.GetHwnd());
					}
				}
				for (i = 0; ppFiles[i]; i++)
					mir_free(ppFiles[i]);
				mir_free(ppFiles);
			}
		}
		return 0;

	case DM_CHECKQUEUEFORCLOSE:
		{
			int *uOpen = (int*)lParam;
			if (uOpen)
				*uOpen += m_iOpenJobs;
		}
		return 0;

	case WM_CLOSE:
		// usual close, not forced
		if (wParam == 0 && lParam == 0) {
			// esc handles error controls if we are in error state (error controls visible)
			if (m_dwFlags & MWF_ERRORSTATE) {
				DM_ErrorDetected(MSGERROR_CANCEL, 0);
				return TRUE;
			}
			
			if (GetCapture() != nullptr)
				return TRUE;

			if (PluginConfig.m_EscapeCloses == 1) {
				SendMessage(m_pContainer->m_hwnd, WM_SYSCOMMAND, SC_MINIMIZE, 0);
				return TRUE;
			}
			else if (PluginConfig.m_bHideOnClose && PluginConfig.m_EscapeCloses == 2) {
				ShowWindow(m_pContainer->m_hwnd, SW_HIDE);
				return TRUE;
			}
			_dlgReturn(m_hwnd, TRUE);
		}

		if (m_iOpenJobs > 0 && lParam != 2) {
			if (m_dwFlags & MWF_ERRORSTATE) {
				DM_ErrorDetected(MSGERROR_CANCEL, 1);
			}
			else {
				if (m_dwFlagsEx & MWF_EX_WARNCLOSE)
					return TRUE;

				m_dwFlagsEx |= MWF_EX_WARNCLOSE;
				LRESULT result = SendQueue::WarnPendingJobs(0);
				m_dwFlagsEx &= ~MWF_EX_WARNCLOSE;
				if (result == IDNO)
					return TRUE;
			}
		}
		{
			int iTabs = TabCtrl_GetItemCount(m_hwndParent);
			if (iTabs == 1) {
				PostMessage(m_pContainer->m_hwnd, WM_CLOSE, 0, 1);
				return 1;
			}

			m_pContainer->iChilds--;

			// after closing a tab, we need to activate the tab to the left side of
			// the previously open tab.
			// normally, this tab has the same index after the deletion of the formerly active tab
			// unless, of course, we closed the last (rightmost) tab.
			if (!m_pContainer->bDontSmartClose && iTabs > 1 && lParam != 3) {
				int i = GetTabIndexFromHWND(m_hwndParent, m_hwnd);
				if (i == iTabs - 1)
					i--;
				else
					i++;
				TabCtrl_SetCurSel(m_hwndParent, i);

				// retrieve dialog hwnd for the now active tab...
				m_pContainer->m_hwndActive = GetTabWindow(m_hwndParent, i);

				SendMessage(m_pContainer->m_hwnd, DM_QUERYCLIENTAREA, 0, (LPARAM)&rc);
				SetWindowPos(m_pContainer->m_hwndActive, HWND_TOP, rc.left, rc.top, (rc.right - rc.left), (rc.bottom - rc.top), SWP_SHOWWINDOW);
				ShowWindow(m_pContainer->m_hwndActive, SW_SHOW);
				SetForegroundWindow(m_pContainer->m_hwndActive);
				SetFocus(m_pContainer->m_hwndActive);
			}
		}

		SendMessage(m_pContainer->m_hwnd, WM_SIZE, 0, 0);
		break;

	case WM_DWMCOMPOSITIONCHANGED:
		BB_RefreshTheme();
		m_pContainer->ClearMargins();
		VerifyProxy();
		break;

	case DM_FORCEREDRAW:
		RedrawWindow(m_hwnd, nullptr, nullptr, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW);
		return 0;

	case DM_CHECKINFOTIP:
		m_pPanel.hideTip(reinterpret_cast<HWND>(lParam));
		return 0;
	}
	
	return CTabBaseDlg::DlgProc(uMsg, wParam, lParam);
}
