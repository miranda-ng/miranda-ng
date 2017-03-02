/////////////////////////////////////////////////////////////////////////////////////////
// Miranda NG: the free IM client for Microsoft* Windows*
//
// Copyright (ñ) 2012-17 Miranda NG project,
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

bool IsStringValidLink(wchar_t* pszText);

const wchar_t *pszIDCSAVE_close = 0, *pszIDCSAVE_save = 0;

static const UINT sendControls[] = { IDC_MESSAGE, IDC_LOG };
static const UINT formatControls[] = { IDC_FONTBOLD, IDC_FONTITALIC, IDC_FONTUNDERLINE, IDC_FONTSTRIKEOUT };
static const UINT addControls[] = { IDC_ADD, IDC_CANCELADD };
static const UINT btnControls[] = { IDC_RETRY, IDC_CANCELSEND, IDC_MSGSENDLATER, IDC_ADD, IDC_CANCELADD };
static const UINT errorControls[] = { IDC_STATICERRORICON, IDC_STATICTEXT, IDC_RETRY, IDC_CANCELSEND, IDC_MSGSENDLATER };

static COLORREF rtfDefColors[] = { RGB(255, 0, 0), RGB(0, 0, 255), RGB(0, 255, 0), RGB(255, 0, 255), RGB(255, 255, 0), RGB(0, 255, 255), 0, RGB(255, 255, 255) };

static struct
{
	int id;
	const wchar_t* text;
}
tooltips[] =
{
	{ IDC_ADD, LPGENW("Add this contact permanently to your contact list") },
	{ IDC_CANCELADD, LPGENW("Do not add this contact permanently") },
	{ IDC_TOGGLESIDEBAR, LPGENW("Expand or collapse the side bar") }
};

static struct
{
	int id;
	HICON *pIcon;
}
buttonicons[] =
{
	{ IDC_ADD, &PluginConfig.g_buttonBarIcons[ICON_BUTTON_ADD] },
	{ IDC_CANCELADD, &PluginConfig.g_buttonBarIcons[ICON_BUTTON_CANCEL] }
};

static void _clrMsgFilter(LPARAM lParam)
{
	MSGFILTER *m = reinterpret_cast<MSGFILTER *>(lParam);
	m->msg = 0;
	m->lParam = 0;
	m->wParam = 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// show a modified context menu for the richedit control(s)
// @param dat			message window data
// @param idFrom		dlg ctrl id
// @param hwndFrom		src window handle
// @param pt			mouse pointer position

static void ShowPopupMenu(CSrmmWindow *dat, int idFrom, HWND hwndFrom, POINT pt)
{
	CHARRANGE sel, all = { 0, -1 };
	HWND hwndDlg = dat->GetHwnd();

	HMENU hSubMenu, hMenu = LoadMenu(g_hInst, MAKEINTRESOURCE(IDR_CONTEXT));
	if (idFrom == IDC_LOG)
		hSubMenu = GetSubMenu(hMenu, 0);
	else {
		hSubMenu = GetSubMenu(hMenu, 2);
		EnableMenuItem(hSubMenu, IDM_PASTEFORMATTED, MF_BYCOMMAND | (dat->m_SendFormat != 0 ? MF_ENABLED : MF_GRAYED));
		EnableMenuItem(hSubMenu, ID_EDITOR_PASTEANDSENDIMMEDIATELY, MF_BYCOMMAND | (PluginConfig.m_PasteAndSend ? MF_ENABLED : MF_GRAYED));
		CheckMenuItem(hSubMenu, ID_EDITOR_SHOWMESSAGELENGTHINDICATOR, MF_BYCOMMAND | (PluginConfig.m_visualMessageSizeIndicator ? MF_CHECKED : MF_UNCHECKED));
		EnableMenuItem(hSubMenu, ID_EDITOR_SHOWMESSAGELENGTHINDICATOR, MF_BYCOMMAND | (dat->m_pContainer->hwndStatus ? MF_ENABLED : MF_GRAYED));
	}
	TranslateMenu(hSubMenu);
	SendMessage(hwndFrom, EM_EXGETSEL, 0, (LPARAM)&sel);
	if (sel.cpMin == sel.cpMax) {
		EnableMenuItem(hSubMenu, IDM_COPY, MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(hSubMenu, IDM_QUOTE, MF_BYCOMMAND | MF_GRAYED);
		if (idFrom == IDC_MESSAGE)
			EnableMenuItem(hSubMenu, IDM_CUT, MF_BYCOMMAND | MF_GRAYED);
	}

	if (idFrom == IDC_LOG) {
		InsertMenuA(hSubMenu, 6, MF_BYPOSITION | MF_SEPARATOR, 0, 0);
		CheckMenuItem(hSubMenu, ID_LOG_FREEZELOG, MF_BYCOMMAND | (dat->m_dwFlagsEx & MWF_SHOW_SCROLLINGDISABLED ? MF_CHECKED : MF_UNCHECKED));
	}

	MessageWindowPopupData mwpd;
	if (idFrom == IDC_LOG || idFrom == IDC_MESSAGE) {
		// First notification
		mwpd.cbSize = sizeof(mwpd);
		mwpd.uType = MSG_WINDOWPOPUP_SHOWING;
		mwpd.uFlags = (idFrom == IDC_LOG ? MSG_WINDOWPOPUP_LOG : MSG_WINDOWPOPUP_INPUT);
		mwpd.hContact = dat->m_hContact;
		mwpd.hwnd = hwndFrom;
		mwpd.hMenu = hSubMenu;
		mwpd.selection = 0;
		mwpd.pt = pt;
		NotifyEventHooks(PluginConfig.m_event_MsgPopup, 0, (LPARAM)&mwpd);
	}

	int iSelection = TrackPopupMenu(hSubMenu, TPM_RETURNCMD, pt.x, pt.y, 0, hwndDlg, NULL);

	if (idFrom == IDC_LOG || idFrom == IDC_MESSAGE) {
		// Second notification
		mwpd.selection = iSelection;
		mwpd.uType = MSG_WINDOWPOPUP_SELECTED;
		NotifyEventHooks(PluginConfig.m_event_MsgPopup, 0, (LPARAM)&mwpd);
	}

	switch (iSelection) {
	case IDM_COPY:
		SendMessage(hwndFrom, WM_COPY, 0, 0);
		break;
	case IDM_CUT:
		SendMessage(hwndFrom, WM_CUT, 0, 0);
		break;
	case IDM_PASTE:
	case IDM_PASTEFORMATTED:
		if (idFrom == IDC_MESSAGE)
			SendMessage(hwndFrom, EM_PASTESPECIAL, (iSelection == IDM_PASTE) ? CF_UNICODETEXT : 0, 0);
		break;
	case IDM_COPYALL:
		SendMessage(hwndFrom, EM_EXSETSEL, 0, (LPARAM)&all);
		SendMessage(hwndFrom, WM_COPY, 0, 0);
		SendMessage(hwndFrom, EM_EXSETSEL, 0, (LPARAM)&sel);
		break;
	case IDM_QUOTE:
		SendMessage(hwndDlg, WM_COMMAND, IDC_QUOTE, 0);
		break;
	case IDM_SELECTALL:
		SendMessage(hwndFrom, EM_EXSETSEL, 0, (LPARAM)&all);
		break;
	case IDM_CLEAR:
		dat->ClearLog();
		break;
	case ID_LOG_FREEZELOG:
		SendDlgItemMessage(hwndDlg, IDC_LOG, WM_KEYDOWN, VK_F12, 0);
		break;
	case ID_EDITOR_SHOWMESSAGELENGTHINDICATOR:
		PluginConfig.m_visualMessageSizeIndicator = !PluginConfig.m_visualMessageSizeIndicator;
		db_set_b(0, SRMSGMOD_T, "msgsizebar", (BYTE)PluginConfig.m_visualMessageSizeIndicator);
		M.BroadcastMessage(DM_CONFIGURETOOLBAR, 0, 0);
		SendMessage(hwndDlg, WM_SIZE, 0, 0);
		if (dat->m_pContainer->hwndStatus)
			RedrawWindow(dat->m_pContainer->hwndStatus, 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);
		break;
	case ID_EDITOR_PASTEANDSENDIMMEDIATELY:
		dat->HandlePasteAndSend();
		break;
	}

	if (idFrom == IDC_LOG)
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
					SetFocus(GetDlgItem(mwdat->GetHwnd(), IDC_MESSAGE));
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
			SetLayeredWindowAttributes(m_pContainer->hwnd, 0, (BYTE)trans, (m_pContainer->dwFlags & CNT_TRANSPARENCY ? LWA_ALPHA : 0));
		}
	}

	if (m_bIsAutosizingInput && m_iInputAreaHeight == -1) {
		m_iInputAreaHeight = 0;
		SendDlgItemMessage(m_hwnd, IDC_MESSAGE, EM_REQUESTRESIZE, 0, 0);
	}

	if (m_pWnd)
		m_pWnd->activateTab();
	m_Panel->dismissConfig();
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
			pcli->pfnRemoveEvent(m_hContact, m_hFlashingEvent);
		m_hFlashingEvent = 0;
	}
	m_pContainer->dwFlags &= ~CNT_NEED_UPDATETITLE;

	if ((m_dwFlags & MWF_DEFERREDREMAKELOG) && !IsIconic(m_pContainer->hwnd)) {
		SendMessage(m_hwnd, DM_REMAKELOG, 0, 0);
		m_dwFlags &= ~MWF_DEFERREDREMAKELOG;
	}

	if (m_dwFlags & MWF_NEEDCHECKSIZE)
		PostMessage(m_hwnd, DM_SAVESIZE, 0, 0);

	if (PluginConfig.m_bAutoLocaleSupport) {
		if (m_hkl == 0)
			DM_LoadLocale();
		else
			SendMessage(m_hwnd, DM_SETLOCALE, 0, 0);
	}

	m_pContainer->hIconTaskbarOverlay = 0;
	SendMessage(m_pContainer->hwnd, DM_UPDATETITLE, m_hContact, 0);

	UpdateStatusBar();
	m_dwLastActivity = GetTickCount();
	m_pContainer->dwLastActivity = m_dwLastActivity;

	m_pContainer->MenuBar->configureMenu();
	UpdateTrayMenuState(this, FALSE);

	if (m_pContainer->hwndActive == m_hwnd)
		PostMessage(m_hwnd, DM_REMOVEPOPUPS, PU_REMOVE_ON_FOCUS, 0);

	m_Panel->Invalidate();

	if (m_dwFlags & MWF_DEFERREDSCROLL && m_hwndIEView == 0 && m_hwndHPP == 0) {
		m_dwFlags &= ~MWF_DEFERREDSCROLL;
		DM_ScrollToBottom(0, 1);
	}

	DM_SetDBButtonStates();

	if (m_hwndIEView) {
		RECT rcRTF;
		POINT pt;

		GetWindowRect(GetDlgItem(m_hwnd, IDC_LOG), &rcRTF);
		rcRTF.left += 20;
		rcRTF.top += 20;
		pt.x = rcRTF.left;
		pt.y = rcRTF.top;
		if (m_hwndIEView) {
			if (M.GetByte("subclassIEView", 0)) {
				mir_subclassWindow(m_hwndIEView, IEViewSubclassProc);
				SetWindowPos(m_hwndIEView, 0, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_DRAWFRAME);
				RedrawWindow(m_hwndIEView, 0, 0, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW);
			}
		}
		m_hwndIWebBrowserControl = WindowFromPoint(pt);
	}

	if (m_dwFlagsEx & MWF_EX_DELAYEDSPLITTER) {
		m_dwFlagsEx &= ~MWF_EX_DELAYEDSPLITTER;
		ShowWindow(m_pContainer->hwnd, SW_RESTORE);
		PostMessage(m_hwnd, DM_SPLITTERGLOBALEVENT, m_wParam, m_lParam);
		m_wParam = m_lParam = 0;
	}
	if (m_dwFlagsEx & MWF_EX_AVATARCHANGED) {
		m_dwFlagsEx &= ~MWF_EX_AVATARCHANGED;
		PostMessage(m_hwnd, DM_UPDATEPICLAYOUT, 0, 0);
	}
	BB_SetButtonsPos();
	if (M.isAero())
		InvalidateRect(m_hwndParent, NULL, FALSE);
	if (m_pContainer->dwFlags & CNT_SIDEBAR)
		m_pContainer->SideBar->setActiveItem(this);

	if (m_pWnd)
		m_pWnd->Invalidate();
}

void TSAPI ShowMultipleControls(HWND hwndDlg, const UINT *controls, int cControls, int state)
{
	for (int i = 0; i < cControls; i++)
		Utils::showDlgControl(hwndDlg, controls[i], state);
}

void TSAPI SetDialogToType(HWND hwndDlg)
{
	CSrmmWindow *dat = (CSrmmWindow*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	if (dat->m_hContact) {
		if (db_get_b(dat->m_hContact, "CList", "NotOnList", 0)) {
			dat->m_bNotOnList = true;
			ShowMultipleControls(hwndDlg, addControls, _countof(addControls), SW_SHOW);
			Utils::showDlgControl(hwndDlg, IDC_LOGFROZENTEXT, SW_SHOW);
			SetDlgItemText(hwndDlg, IDC_LOGFROZENTEXT, TranslateT("Contact not on list. You may add it..."));
		}
		else {
			ShowMultipleControls(hwndDlg, addControls, _countof(addControls), SW_HIDE);
			dat->m_bNotOnList = false;
			Utils::showDlgControl(hwndDlg, IDC_LOGFROZENTEXT, SW_HIDE);
		}
	}

	Utils::enableDlgControl(hwndDlg, IDC_TIME, true);

	if (dat->m_hwndIEView || dat->m_hwndHPP) {
		Utils::showDlgControl(hwndDlg, IDC_LOG, SW_HIDE);
		Utils::enableDlgControl(hwndDlg, IDC_LOG, false);
		Utils::showDlgControl(hwndDlg, IDC_MESSAGE, SW_SHOW);
	}
	else ShowMultipleControls(hwndDlg, sendControls, _countof(sendControls), SW_SHOW);

	ShowMultipleControls(hwndDlg, errorControls, _countof(errorControls), dat->m_dwFlags & MWF_ERRORSTATE ? SW_SHOW : SW_HIDE);

	if (!dat->m_SendFormat)
		ShowMultipleControls(hwndDlg, formatControls, _countof(formatControls), SW_HIDE);

	if (dat->m_pContainer->hwndActive == hwndDlg)
		dat->UpdateReadChars();

	SetDlgItemText(hwndDlg, IDC_STATICTEXT, TranslateT("A message failed to send successfully."));

	dat->DM_RecalcPictureSize();
	dat->GetAvatarVisibility();

	Utils::showDlgControl(hwndDlg, IDC_CONTACTPIC, dat->m_bShowAvatar ? SW_SHOW : SW_HIDE);
	Utils::showDlgControl(hwndDlg, IDC_SPLITTER, dat->m_bIsAutosizingInput ? SW_HIDE : SW_SHOW);
	Utils::showDlgControl(hwndDlg, IDC_MULTISPLITTER, (dat->m_sendMode & SMODE_MULTIPLE) ? SW_SHOW : SW_HIDE);

	dat->EnableSendButton(GetWindowTextLength(GetDlgItem(hwndDlg, IDC_MESSAGE)) != 0);
	SendMessage(hwndDlg, DM_UPDATETITLE, 0, 1);
	SendMessage(hwndDlg, WM_SIZE, 0, 0);

	Utils::enableDlgControl(hwndDlg, IDC_CONTACTPIC, false);

	dat->m_Panel->Configure();
}

static LRESULT CALLBACK MessageLogSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HWND hwndParent = GetParent(hwnd);
	CSrmmWindow *mwdat = (CSrmmWindow*)GetWindowLongPtr(hwndParent, GWLP_USERDATA);
	bool isCtrl, isShift, isAlt;
	mwdat->KbdState(isShift, isCtrl, isAlt);

	switch (msg) {
	case WM_KILLFOCUS:
		if (wParam != (WPARAM)hwnd && 0 != wParam) {
			CHARRANGE cr;
			SendMessage(hwnd, EM_EXGETSEL, 0, (LPARAM)&cr);
			if (cr.cpMax != cr.cpMin) {
				cr.cpMin = cr.cpMax;
				SendMessage(hwnd, EM_EXSETSEL, 0, (LPARAM)&cr);
			}
		}
		break;

	case WM_CHAR:
		if (wParam == 0x03 && isCtrl) // Ctrl+C
			return Utils::WMCopyHandler(hwnd, MessageLogSubclassProc, msg, wParam, lParam);
		if (wParam == 0x11 && isCtrl)
			SendMessage(mwdat->GetHwnd(), WM_COMMAND, IDC_QUOTE, 0);
		break;

	case WM_SYSKEYUP:
		if (wParam == VK_MENU) {
			ProcessHotkeysByMsgFilter(hwnd, msg, wParam, lParam, IDC_LOG);
			return 0;
		}
		break;

	case WM_SYSKEYDOWN:
		mwdat->m_fkeyProcessed = false;
		if (ProcessHotkeysByMsgFilter(hwnd, msg, wParam, lParam, IDC_LOG)) {
			mwdat->m_fkeyProcessed = true;
			return 0;
		}
		break;

	case WM_SYSCHAR:
		if (mwdat->m_fkeyProcessed) {
			mwdat->m_fkeyProcessed = false;
			return 0;
		}
		break;

	case WM_KEYDOWN:
		if (wParam == VK_INSERT && isCtrl)
			return Utils::WMCopyHandler(hwnd, MessageLogSubclassProc, msg, wParam, lParam);
		break;

	case WM_COPY:
		return Utils::WMCopyHandler(hwnd, MessageLogSubclassProc, msg, wParam, lParam);

	case WM_NCCALCSIZE:
		return CSkin::NcCalcRichEditFrame(hwnd, mwdat, ID_EXTBKHISTORY, msg, wParam, lParam, MessageLogSubclassProc);

	case WM_NCPAINT:
		return CSkin::DrawRichEditFrame(hwnd, mwdat, ID_EXTBKHISTORY, msg, wParam, lParam, MessageLogSubclassProc);

	case WM_CONTEXTMENU:
		POINT pt;

		if (lParam == 0xFFFFFFFF) {
			CHARRANGE sel;
			SendMessage(hwnd, EM_EXGETSEL, 0, (LPARAM)&sel);
			SendMessage(hwnd, EM_POSFROMCHAR, (WPARAM)&pt, (LPARAM)sel.cpMax);
			ClientToScreen(hwnd, &pt);
		}
		else {
			pt.x = GET_X_LPARAM(lParam);
			pt.y = GET_Y_LPARAM(lParam);
		}

		ShowPopupMenu(mwdat, IDC_LOG, hwnd, pt);
		return TRUE;
	}

	return mir_callNextSubclass(hwnd, MessageLogSubclassProc, msg, wParam, lParam);
}

static LRESULT CALLBACK MessageEditSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	bool isCtrl, isShift, isAlt;
	HWND hwndParent = GetParent(hwnd);
	CSrmmWindow *mwdat = (CSrmmWindow*)GetWindowLongPtr(hwndParent, GWLP_USERDATA);
	if (mwdat == NULL)
		return 0;

	// prevent the rich edit from switching text direction or keyboard layout when
	// using hotkeys with ctrl-shift or alt-shift modifiers
	if (mwdat->m_fkeyProcessed && (msg == WM_KEYUP)) {
		GetKeyboardState(mwdat->kstate);
		if (mwdat->kstate[VK_CONTROL] & 0x80 || mwdat->kstate[VK_SHIFT] & 0x80)
			return 0;

		mwdat->m_fkeyProcessed = false;
		return 0;
	}

	switch (msg) {
	case WM_NCCALCSIZE:
		return CSkin::NcCalcRichEditFrame(hwnd, mwdat, ID_EXTBKINPUTAREA, msg, wParam, lParam, MessageEditSubclassProc);

	case WM_NCPAINT:
		return CSkin::DrawRichEditFrame(hwnd, mwdat, ID_EXTBKINPUTAREA, msg, wParam, lParam, MessageEditSubclassProc);

	case WM_DROPFILES:
		SendMessage(hwndParent, WM_DROPFILES, (WPARAM)wParam, (LPARAM)lParam);
		return 0;

	case WM_CHAR:
		mwdat->KbdState(isShift, isCtrl, isAlt);

		if (PluginConfig.m_bSoundOnTyping && !isAlt && !isCtrl && !(mwdat->m_pContainer->dwFlags & CNT_NOSOUND) && wParam != VK_ESCAPE && !(wParam == VK_TAB && PluginConfig.m_bAllowTab))
			SkinPlaySound("SoundOnTyping");

		if (isCtrl && !isAlt) {
			switch (wParam) {
			case 0x02:               // bold
				if (mwdat->m_SendFormat)
					SendMessage(hwndParent, WM_COMMAND, MAKELONG(IDC_FONTBOLD, IDC_MESSAGE), 0);
				return 0;
			case 0x09:
				if (mwdat->m_SendFormat)
					SendMessage(hwndParent, WM_COMMAND, MAKELONG(IDC_FONTITALIC, IDC_MESSAGE), 0);
				return 0;
			case 21:
				if (mwdat->m_SendFormat)
					SendMessage(hwndParent, WM_COMMAND, MAKELONG(IDC_FONTUNDERLINE, IDC_MESSAGE), 0);
				return 0;
			case 0x0b:
				SetWindowText(hwnd, L"");
				return 0;
			}
		}
		break;

	case WM_MOUSEWHEEL:
		if (mwdat->DM_MouseWheelHandler(wParam, lParam) == 0)
			return 0;
		break;

	case EM_PASTESPECIAL:
	case WM_PASTE:
		if (OpenClipboard(hwnd)) {
			HANDLE hClip = GetClipboardData(CF_TEXT);
			if (hClip) {
				if ((int)mir_strlen((char*)hClip) > mwdat->m_nMax) {
					wchar_t szBuffer[512];
					if (M.GetByte("autosplit", 0))
						mir_snwprintf(szBuffer, TranslateT("WARNING: The message you are trying to paste exceeds the message size limit for the active protocol. It will be sent in chunks of max %d characters"), mwdat->m_nMax - 10);
					else
						mir_snwprintf(szBuffer, TranslateT("The message you are trying to paste exceeds the message size limit for the active protocol. Only the first %d characters will be sent."), mwdat->m_nMax);
					SendMessage(hwndParent, DM_ACTIVATETOOLTIP, IDC_MESSAGE, (LPARAM)szBuffer);
				}
			}
			else if (hClip = GetClipboardData(CF_BITMAP))
				mwdat->SendHBitmapAsFile((HBITMAP)hClip);

			CloseClipboard();
		}
		break;

	case WM_KEYDOWN:
		mwdat->KbdState(isShift, isCtrl, isAlt);

		if (PluginConfig.m_bSoundOnTyping && !isAlt && !(mwdat->m_pContainer->dwFlags & CNT_NOSOUND) && wParam == VK_DELETE)
			SkinPlaySound("SoundOnTyping");

		if (wParam == VK_INSERT && !isShift && !isCtrl && !isAlt) {
			mwdat->m_fInsertMode = !mwdat->m_fInsertMode;
			SendMessage(hwndParent, WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(hwnd), EN_CHANGE), (LPARAM)hwnd);
		}
		if (wParam == VK_CAPITAL || wParam == VK_NUMLOCK)
			SendMessage(hwndParent, WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(hwnd), EN_CHANGE), (LPARAM)hwnd);

		if (wParam == VK_RETURN) {
			if (mwdat->m_fEditNotesActive)
				break;

			if (isShift) {
				if (PluginConfig.m_bSendOnShiftEnter) {
					PostMessage(hwndParent, WM_COMMAND, IDOK, 0);
					return 0;
				}
				else break;
			}
			if ((isCtrl && !isShift) ^ (0 != PluginConfig.m_bSendOnEnter)) {
				PostMessage(hwndParent, WM_COMMAND, IDOK, 0);
				return 0;
			}
			if (PluginConfig.m_bSendOnEnter || PluginConfig.m_bSendOnDblEnter) {
				if (isCtrl)
					break;

				if (PluginConfig.m_bSendOnDblEnter) {
					LONG_PTR lastEnterTime = GetWindowLongPtr(hwnd, GWLP_USERDATA);
					if (lastEnterTime + 2 < time(NULL)) {
						lastEnterTime = time(NULL);
						SetWindowLongPtr(hwnd, GWLP_USERDATA, lastEnterTime);
						break;
					}
					else {
						SendMessage(hwnd, WM_KEYDOWN, VK_BACK, 0);
						SendMessage(hwnd, WM_KEYUP, VK_BACK, 0);
						PostMessage(hwndParent, WM_COMMAND, IDOK, 0);
						return 0;
					}
				}
				PostMessage(hwndParent, WM_COMMAND, IDOK, 0);
				return 0;
			}
			else break;
		}
		else SetWindowLongPtr(hwnd, GWLP_USERDATA, 0);

		if (isCtrl && !isAlt && !isShift) {
			if (!isShift && (wParam == VK_UP || wParam == VK_DOWN)) {          // input history scrolling (ctrl-up / down)
				SetWindowLongPtr(hwnd, GWLP_USERDATA, 0);
				if (mwdat)
					mwdat->m_cache->inputHistoryEvent(wParam);
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

				SetWindowLongPtr(hwnd, GWLP_USERDATA, 0);
				if (wParam == VK_UP)
					wp = MAKEWPARAM(SB_LINEUP, 0);
				else if (wParam == VK_PRIOR)
					wp = MAKEWPARAM(SB_PAGEUP, 0);
				else if (wParam == VK_NEXT)
					wp = MAKEWPARAM(SB_PAGEDOWN, 0);
				else if (wParam == VK_HOME)
					wp = MAKEWPARAM(SB_TOP, 0);
				else if (wParam == VK_END) {
					mwdat->DM_ScrollToBottom(0, 0);
					return 0;
				}
				else if (wParam == VK_DOWN)
					wp = MAKEWPARAM(SB_LINEDOWN, 0);

				if (mwdat->m_hwndIEView == 0 && mwdat->m_hwndHPP == 0)
					SendDlgItemMessage(hwndParent, IDC_LOG, WM_VSCROLL, wp, 0);
				else
					SendMessage(mwdat->m_hwndIWebBrowserControl, WM_VSCROLL, wp, 0);
				return 0;
			}
		}

	case WM_SYSKEYDOWN:
		mwdat->m_fkeyProcessed = false;
		if (ProcessHotkeysByMsgFilter(hwnd, msg, wParam, lParam, IDC_MESSAGE)) {
			mwdat->m_fkeyProcessed = true;
			return 0;
		}
		break;

	case WM_SYSKEYUP:
		if (wParam == VK_MENU) {
			ProcessHotkeysByMsgFilter(hwnd, msg, wParam, lParam, IDC_MESSAGE);
			return 0;
		}
		break;

	case WM_SYSCHAR:
		if (mwdat->m_fkeyProcessed) {
			mwdat->m_fkeyProcessed = false;
			return 0;
		}

		mwdat->KbdState(isShift, isCtrl, isAlt);
		if ((wParam >= '0' && wParam <= '9') && isAlt) {      // ALT-1 -> ALT-0 direct tab selection
			BYTE bChar = (BYTE)wParam;
			int iIndex;

			if (bChar == '0')
				iIndex = 10;
			else
				iIndex = bChar - (BYTE)'0';
			SendMessage(mwdat->m_pContainer->hwnd, DM_SELECTTAB, DM_SELECT_BY_INDEX, (LPARAM)iIndex);
			return 0;
		}
		break;

	case WM_INPUTLANGCHANGE:
		if (PluginConfig.m_bAutoLocaleSupport && GetFocus() == hwnd && mwdat->m_pContainer->hwndActive == hwndParent && GetForegroundWindow() == mwdat->m_pContainer->hwnd && GetActiveWindow() == mwdat->m_pContainer->hwnd) {
			mwdat->DM_SaveLocale(wParam, lParam);
			SendMessage(hwnd, EM_SETLANGOPTIONS, 0, (LPARAM)SendMessage(hwnd, EM_GETLANGOPTIONS, 0, 0) & ~IMF_AUTOKEYBOARD);
			return 1;
		}
		break;

	case WM_ERASEBKGND:
		return(CSkin::m_skinEnabled ? 0 : 1);

		// sent by smileyadd when the smiley selection window dies
		// just grab the focus :)
	case WM_USER + 100:
		SetFocus(hwnd);
		break;

	case WM_CONTEXTMENU:
		POINT pt;
		if (lParam == 0xFFFFFFFF) {
			CHARRANGE sel;
			SendMessage(hwnd, EM_EXGETSEL, 0, (LPARAM)&sel);
			SendMessage(hwnd, EM_POSFROMCHAR, (WPARAM)&pt, (LPARAM)sel.cpMax);
			ClientToScreen(hwnd, &pt);
		}
		else {
			pt.x = GET_X_LPARAM(lParam);
			pt.y = GET_Y_LPARAM(lParam);
		}

		ShowPopupMenu(mwdat, IDC_MESSAGE, hwnd, pt);
		return TRUE;
	}
	return mir_callNextSubclass(hwnd, MessageEditSubclassProc, msg, wParam, lParam);
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
	CSrmmWindow *dat = (CSrmmWindow*)GetWindowLongPtr(hwndParent, GWLP_USERDATA);

	switch (msg) {
	case WM_NCHITTEST:
		return HTCLIENT;

	case WM_SETCURSOR:
		GetClientRect(hwnd, &rc);
		SetCursor(rc.right > rc.bottom ? PluginConfig.hCurSplitNS : PluginConfig.hCurSplitWE);
		return TRUE;

	case WM_LBUTTONDOWN:
		if (hwnd == GetDlgItem(hwndParent, IDC_SPLITTER) || hwnd == GetDlgItem(hwndParent, IDC_SPLITTERY)) {
			if (dat) {
				GetClientRect(hwnd, &rc);
				dat->m_savedSplitter = rc.right > rc.bottom ? (short)HIWORD(GetMessagePos()) + rc.bottom / 2 : (short)LOWORD(GetMessagePos()) + rc.right / 2;
				if (dat->m_bType == SESSIONTYPE_IM)
					dat->m_savedSplitY = dat->m_splitterY;
				else
					dat->m_savedSplitY = dat->si->iSplitterY;

				dat->m_savedDynaSplit = dat->m_dynaSplitter;
			}
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
				CSkin::SkinDrawBG(hwnd, dat->m_pContainer->hwnd, dat->m_pContainer, &rc, dc);
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
		if (dat && dat->m_bType == SESSIONTYPE_IM && hwnd == GetDlgItem(hwndParent, IDC_PANELSPLITTER)) {
			SendMessage(hwndParent, WM_SIZE, 0, 0);
			RedrawWindow(hwndParent, NULL, NULL, RDW_ALLCHILDREN | RDW_INVALIDATE | RDW_UPDATENOW);
		}
		else if ((dat && dat->m_bType == SESSIONTYPE_IM && hwnd == GetDlgItem(hwndParent, IDC_SPLITTER)) ||
			(dat && dat->m_bType == SESSIONTYPE_CHAT && hwnd == GetDlgItem(hwndParent, IDC_SPLITTERY))) {
			POINT pt;
			int selection;
			HMENU hMenu = GetSubMenu(PluginConfig.g_hMenuContext, 12);
			LONG messagePos = GetMessagePos();

			GetClientRect(hwnd, &rc);
			if (hwndCapture != hwnd || dat->m_savedSplitter == (rc.right > rc.bottom ? (short)HIWORD(messagePos) + rc.bottom / 2 : (short)LOWORD(messagePos) + rc.right / 2))
				break;
			GetCursorPos(&pt);

			if (dat->m_bIsAutosizingInput)
				selection = ID_SPLITTERCONTEXT_SETPOSITIONFORTHISSESSION;
			else
				selection = TrackPopupMenu(hMenu, TPM_RETURNCMD, pt.x, pt.y, 0, hwndParent, NULL);

			switch (selection) {
			case ID_SPLITTERCONTEXT_SAVEFORTHISCONTACTONLY:
				dat->m_dwFlagsEx |= MWF_SHOW_SPLITTEROVERRIDE;
				db_set_b(dat->m_hContact, SRMSGMOD_T, "splitoverride", 1);
				if (dat->m_bType == SESSIONTYPE_IM)
					dat->SaveSplitter();
				break;

			case ID_SPLITTERCONTEXT_SETPOSITIONFORTHISSESSION:
				if (dat->m_bIsAutosizingInput) {
					GetWindowRect(GetDlgItem(dat->GetHwnd(), IDC_MESSAGE), &rc);
					dat->m_iInputAreaHeight = 0;
				}
				break;

			case ID_SPLITTERCONTEXT_SAVEGLOBALFORALLSESSIONS:
				{
					BYTE bSync = M.GetByte(CHAT_MODULE, "SyncSplitter", 0);
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
					PluginConfig.lastSPlitterPos.bSync = bSync;
					SendMessage(dat->GetHwnd(), DM_SPLITTERGLOBALEVENT, 0, 0);
					M.BroadcastMessage(DM_SPLITTERGLOBALEVENT, 0, 0);
				}
				break;

			default:
				dat->m_splitterY = dat->m_savedSplitY;
				dat->m_dynaSplitter = dat->m_savedDynaSplit;
				dat->DM_RecalcPictureSize();
				if (dat->m_bType == SESSIONTYPE_CHAT) {
					SESSION_INFO *si = dat->si;
					si->iSplitterY = dat->m_savedSplitY;
					dat->m_splitterY = si->iSplitterY + DPISCALEY_S(22);
				}
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

CSrmmWindow::CSrmmWindow(TNewWindowData *pNewData)
	: CTabBaseDlg(pNewData, IDD_MSGSPLITNEW)
{
	m_pLog = &m_log;
	m_pEntry = &m_message;
	m_autoClose = 0;

	m_dwFlags = MWF_INITMODE;
	m_bType = SESSIONTYPE_IM;
	m_Panel = new CInfoPanel(this);
}

void CSrmmWindow::OnInitDialog()
{
	m_pContainer = newData->pContainer;
	SetWindowLongPtr(m_hwnd, GWLP_USERDATA, (LONG_PTR)this);

	if (Utils::rtf_ctable == 0)
		Utils::RTF_CTableInit();

	newData->item.lParam = (LPARAM)m_hwnd;
	TabCtrl_SetItem(m_hwndParent, newData->iTabID, &newData->item);
	m_iTabID = newData->iTabID;

	DM_ThemeChanged();

	pszIDCSAVE_close = TranslateT("Close session");
	pszIDCSAVE_save = TranslateT("Save and close session");

	m_hContact = newData->hContact;
	m_cache = CContactCache::getContactCache(m_hContact);
	m_cache->updateNick();
	m_cache->setWindowData(m_hwnd, this);
	M.AddWindow(m_hwnd, m_hContact);
	BroadCastContainer(m_pContainer, DM_REFRESHTABINDEX, 0, 0);
	CProxyWindow::add(this);
	m_szProto = const_cast<char *>(m_cache->getProto());
	m_bIsMeta = m_cache->isMeta();
	if (m_bIsMeta)
		m_cache->updateMeta();

	m_cache->updateUIN();

	if (m_hContact && m_szProto != NULL) {
		m_wStatus = db_get_w(m_hContact, m_szProto, "Status", ID_STATUS_OFFLINE);
		wcsncpy_s(m_wszStatus, pcli->pfnGetStatusModeDescription(m_szProto == NULL ? ID_STATUS_OFFLINE : m_wStatus, 0), _TRUNCATE);
	}
	else m_wStatus = ID_STATUS_OFFLINE;

	for (int i = 0; i < _countof(btnControls); i++)
		CustomizeButton(GetDlgItem(m_hwnd, btnControls[i]));

	GetMYUIN();
	GetClientIcon();

	CustomizeButton(CreateWindowEx(0, L"MButtonClass", L"", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 0, 0, 6, DPISCALEY_S(20),
		m_hwnd, (HMENU)IDC_TOGGLESIDEBAR, g_hInst, NULL));
	m_hwndPanelPicParent = CreateWindowEx(WS_EX_TOPMOST, L"Static", L"", SS_OWNERDRAW | WS_VISIBLE | WS_CHILD, 1, 1, 1, 1, m_hwnd, (HMENU)6000, NULL, NULL);
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
		m_hHistoryEvents = NULL;

	if (m_bIsMeta)
		SendMessage(m_hwnd, DM_UPDATEMETACONTACTINFO, 0, 0);
	else
		SendMessage(m_hwnd, DM_UPDATEWINICON, 0, 0);

	GetMyNick();

	m_multiSplitterX = (int)M.GetDword(SRMSGMOD, "multisplit", 150);
	m_nTypeMode = PROTOTYPE_SELFTYPING_OFF;
	SetTimer(m_hwnd, TIMERID_TYPE, 1000, NULL);
	m_iLastEventType = 0xffffffff;

	// load log option flags...
	m_dwFlags = m_pContainer->theme.dwFlags;

	// consider per-contact message setting overrides
	if (m_hContact && M.GetDword(m_hContact, "mwmask", 0))
		LoadLocalFlags();

	DM_InitTip();
	m_Panel->getVisibility();

	m_dwFlagsEx |= M.GetByte(m_hContact, "splitoverride", 0) ? MWF_SHOW_SPLITTEROVERRIDE : 0;
	m_bIsAutosizingInput = IsAutoSplitEnabled();
	m_iInputAreaHeight = -1;
	SetMessageLog();
	if (m_hContact)
		m_Panel->loadHeight();

	m_bShowAvatar = GetAvatarVisibility();

	Utils::showDlgControl(m_hwnd, IDC_MULTISPLITTER, SW_HIDE);

	RECT rc;
	GetWindowRect(GetDlgItem(m_hwnd, IDC_SPLITTER), &rc);
	
	POINT pt;
	pt.y = (rc.top + rc.bottom) / 2;
	pt.x = 0;
	ScreenToClient(m_hwnd, &pt);
	m_originalSplitterY = pt.y;
	if (m_splitterY == -1)
		m_splitterY = m_originalSplitterY + 60;

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

	TABSRMM_FireEvent(m_hContact, m_hwnd, MSG_WINDOW_EVT_OPENING, 0);

	for (int i = 0; i < _countof(tooltips); i++)
		SendDlgItemMessage(m_hwnd, tooltips[i].id, BUTTONADDTOOLTIP, (WPARAM)TranslateW(tooltips[i].text), BATF_UNICODE);

	SetDlgItemText(m_hwnd, IDC_LOGFROZENTEXT, m_bNotOnList ? TranslateT("Contact not on list. You may add it...") :
		TranslateT("Auto scrolling is disabled (press F12 to enable it)"));

	SendDlgItemMessage(m_hwnd, IDC_SAVE, BUTTONADDTOOLTIP, (WPARAM)pszIDCSAVE_close, BATF_UNICODE);
	SendDlgItemMessage(m_hwnd, IDC_PROTOCOL, BUTTONADDTOOLTIP, (WPARAM)TranslateT("Click for contact menu\nClick dropdown for window settings"), BATF_UNICODE);

	SetDlgItemText(m_hwnd, IDC_RETRY, TranslateT("Retry"));
	{
		UINT _ctrls[] = { IDC_RETRY, IDC_CANCELSEND, IDC_MSGSENDLATER };
		for (int i = 0; i < _countof(_ctrls); i++) {
			SendDlgItemMessage(m_hwnd, _ctrls[i], BUTTONSETASPUSHBTN, TRUE, 0);
			SendDlgItemMessage(m_hwnd, _ctrls[i], BUTTONSETASFLATBTN, FALSE, 0);
			SendDlgItemMessage(m_hwnd, _ctrls[i], BUTTONSETASTHEMEDBTN, TRUE, 0);
		}
	}

	SetDlgItemText(m_hwnd, IDC_CANCELSEND, TranslateT("Cancel"));
	SetDlgItemText(m_hwnd, IDC_MSGSENDLATER, TranslateT("Send later"));

	m_log.SendMsg(EM_SETUNDOLIMIT, 0, 0);
	m_log.SendMsg(EM_SETEVENTMASK, 0, ENM_MOUSEEVENTS | ENM_KEYEVENTS | ENM_LINK);
	m_log.SendMsg(EM_SETOLECALLBACK, 0, (LPARAM)&reOleCallback);

	m_message.SendMsg(EM_SETEVENTMASK, 0, ENM_REQUESTRESIZE | ENM_MOUSEEVENTS | ENM_SCROLL | ENM_KEYEVENTS | ENM_CHANGE);
	m_message.SendMsg(EM_SETOLECALLBACK, 0, (LPARAM)&reOleCallback2);

	m_bActualHistory = M.GetByte(m_hContact, "ActualHistory", 0) != 0;

	/* OnO: higligh lines to their end */
	m_log.SendMsg(EM_SETEDITSTYLE, SES_EXTENDBACKCOLOR, SES_EXTENDBACKCOLOR);

	m_log.SendMsg(EM_SETLANGOPTIONS, 0, m_log.SendMsg(EM_GETLANGOPTIONS, 0, 0) & ~IMF_AUTOFONTSIZEADJUST);

	// add us to the tray list (if it exists)
	if (PluginConfig.g_hMenuTrayUnread != 0 && m_hContact != 0 && m_szProto != NULL)
		UpdateTrayMenu(0, m_wStatus, m_szProto, m_wszStatus, m_hContact, FALSE);

	m_log.SendMsg(EM_AUTOURLDETECT, TRUE, 0);
	m_log.SendMsg(EM_EXLIMITTEXT, 0, 0x80000000);

	// subclassing stuff
	mir_subclassWindow(m_message.GetHwnd(), MessageEditSubclassProc);
	mir_subclassWindow(GetDlgItem(m_hwnd, IDC_CONTACTPIC), AvatarSubclassProc);
	mir_subclassWindow(GetDlgItem(m_hwnd, IDC_SPLITTER), SplitterSubclassProc);
	mir_subclassWindow(GetDlgItem(m_hwnd, IDC_MULTISPLITTER), SplitterSubclassProc);
	mir_subclassWindow(GetDlgItem(m_hwnd, IDC_PANELSPLITTER), SplitterSubclassProc);

	// load old messages from history (if wanted...)
	m_cache->updateStats(TSessionStats::INIT_TIMER);
	if (m_hContact) {
		FindFirstEvent();
		m_nMax = (int)m_cache->getMaxMessageLength();
	}
	LoadContactAvatar();
	SendMessage(m_hwnd, DM_OPTIONSAPPLIED, 0, 0);
	LoadOwnAvatar();

	// restore saved msg if any...
	if (m_hContact) {
		ptrW tszSavedMsg(db_get_wsa(m_hContact, SRMSGMOD, "SavedMsg"));
		if (tszSavedMsg != 0) {
			SETTEXTEX stx = { ST_DEFAULT, 1200 };
			m_message.SendMsg(EM_SETTEXTEX, (WPARAM)&stx, tszSavedMsg);
			SendQueue::UpdateSaveAndSendButton(this);
			if (m_pContainer->hwndActive == m_hwnd)
				UpdateReadChars();
		}
	}
	if (newData->szInitialText) {
		if (newData->isWchar)
			SetDlgItemTextW(m_hwnd, IDC_MESSAGE, (wchar_t*)newData->szInitialText);
		else
			SetDlgItemTextA(m_hwnd, IDC_MESSAGE, newData->szInitialText);
		int len = GetWindowTextLength(m_message.GetHwnd());
		PostMessage(m_message.GetHwnd(), EM_SETSEL, len, len);
		if (len)
			EnableSendButton(true);
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

	SendMessage(m_pContainer->hwnd, DM_QUERYCLIENTAREA, 0, (LPARAM)&rc);
	{
		WNDCLASS wndClass = { 0 };
		GetClassInfo(g_hInst, L"RICHEDIT50W", &wndClass);
		mir_subclassWindowFull(m_log.GetHwnd(), MessageLogSubclassProc, wndClass.lpfnWndProc);
	}

	SetWindowPos(m_hwnd, 0, rc.left, rc.top, (rc.right - rc.left), (rc.bottom - rc.top), newData->iActivate ? 0 : SWP_NOZORDER | SWP_NOACTIVATE);
	LoadSplitter();
	ShowPicture(true);

	if (m_pContainer->dwFlags & CNT_CREATE_MINIMIZED || !newData->iActivate || m_pContainer->dwFlags & CNT_DEFERREDTABSELECT) {
		m_iFlashIcon = PluginConfig.g_IconMsgEvent;
		SetTimer(m_hwnd, TIMERID_FLASHWND, TIMEOUT_FLASHWND, NULL);
		m_bCanFlashTab = true;

		DBEVENTINFO dbei = { 0 };
		dbei.eventType = EVENTTYPE_MESSAGE;
		FlashOnClist(m_hDbEventFirst, &dbei);

		SendMessage(m_pContainer->hwnd, DM_SETICON, (WPARAM)this, (LPARAM)Skin_LoadIcon(SKINICON_EVENT_MESSAGE));
		m_pContainer->dwFlags |= CNT_NEED_UPDATETITLE;
		m_dwFlags |= MWF_NEEDCHECKSIZE | MWF_WASBACKGROUNDCREATE | MWF_DEFERREDSCROLL;
	}

	if (newData->iActivate) {
		m_pContainer->hwndActive = m_hwnd;
		ShowWindow(m_hwnd, SW_SHOW);
		SetActiveWindow(m_hwnd);
		SetForegroundWindow(m_hwnd);
	}
	else if (m_pContainer->dwFlags & CNT_CREATE_MINIMIZED) {
		m_dwFlags |= MWF_DEFERREDSCROLL;
		ShowWindow(m_hwnd, SW_SHOWNOACTIVATE);
		m_pContainer->hwndActive = m_hwnd;
		m_pContainer->dwFlags |= CNT_DEFERREDCONFIGURE;
	}
	PostMessage(m_pContainer->hwnd, DM_UPDATETITLE, m_hContact, 0);

	DM_RecalcPictureSize();
	m_dwLastActivity = GetTickCount() - 1000;
	m_pContainer->dwLastActivity = m_dwLastActivity;

	if (m_hwndHPP)
		mir_subclassWindow(m_hwndHPP, HPPKFSubclassProc);

	m_dwFlags &= ~MWF_INITMODE;
	TABSRMM_FireEvent(m_hContact, m_hwnd, MSG_WINDOW_EVT_OPEN, 0);

	// show a popup if wanted...
	if (newData->bWantPopup) {
		DBEVENTINFO dbei = {};
		newData->bWantPopup = FALSE;
		db_event_get(newData->hdbEvent, &dbei);
		tabSRMM_ShowPopup(m_hContact, newData->hdbEvent, dbei.eventType, 0, 0, m_hwnd, m_cache->getActiveProto());
	}

	if (m_pContainer->dwFlags & CNT_CREATE_MINIMIZED) {
		m_pContainer->dwFlags &= ~CNT_CREATE_MINIMIZED;
		m_pContainer->hwndActive = m_hwnd;
	}
}

void CSrmmWindow::OnDestroy()
{
	memset((void*)&m_pContainer->mOld, -1000, sizeof(MARGINS));
	PostMessage(m_pContainer->hwnd, WM_SIZE, 0, 1);
	if (m_pContainer->dwFlags & CNT_SIDEBAR)
		m_pContainer->SideBar->removeSession(this);
	m_cache->setWindowData();
	if (m_cache->isValid() && !m_fIsReattach && m_hContact && M.GetByte("deletetemp", 0))
		if (db_get_b(m_hContact, "CList", "NotOnList", 0))
			db_delete_contact(m_hContact);

	delete m_Panel;

	if (m_hwndContactPic)
		DestroyWindow(m_hwndContactPic);

	if (m_hwndPanelPic)
		DestroyWindow(m_hwndPanelPic);

	if (m_hClientIcon)
		DestroyIcon(m_hClientIcon);

	if (m_hwndPanelPicParent)
		DestroyWindow(m_hwndPanelPicParent);

	if (m_cache->isValid()) { // not valid means the contact was deleted
		TABSRMM_FireEvent(m_hContact, m_hwnd, MSG_WINDOW_EVT_CLOSING, 0);
		AddContactToFavorites(m_hContact, m_cache->getNick(), m_cache->getActiveProto(), m_wszStatus, m_wStatus,
			Skin_LoadProtoIcon(m_cache->getActiveProto(), m_cache->getActiveStatus()), 1, PluginConfig.g_hMenuRecent);
		if (m_hContact) {
			if (!m_fEditNotesActive) {
				char *msg = Message_GetFromStream(m_message.GetHwnd(), SF_TEXT);
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

	mir_free(m_sendBuffer);
	mir_free(m_hHistoryEvents);

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
					jobs[i].hOwnerWnd = 0;
			}
		}
	}

	mir_free(m_hQueuedEvents);

	if (m_hSmileyIcon)
		DestroyIcon(m_hSmileyIcon);

	if (m_hXStatusIcon)
		DestroyIcon(m_hXStatusIcon);

	if (m_hwndTip)
		DestroyWindow(m_hwndTip);

	if (m_hTaskbarIcon)
		DestroyIcon(m_hTaskbarIcon);

	UpdateTrayMenuState(this, FALSE);               // remove me from the tray menu (if still there)
	if (PluginConfig.g_hMenuTrayUnread)
		DeleteMenu(PluginConfig.g_hMenuTrayUnread, m_hContact, MF_BYCOMMAND);
	M.RemoveWindow(m_hwnd);

	if (m_cache->isValid())
		db_set_dw(0, SRMSGMOD, "multisplit", m_multiSplitterX);

	{
		int i = GetTabIndexFromHWND(m_hwndParent, m_hwnd);
		if (i >= 0) {
			SendMessage(m_hwndParent, WM_USER + 100, 0, 0);                      // remove tooltip
			TabCtrl_DeleteItem(m_hwndParent, i);
			BroadCastContainer(m_pContainer, DM_REFRESHTABINDEX, 0, 0);
			m_iTabID = -1;
		}
	}

	TABSRMM_FireEvent(m_hContact, m_hwnd, MSG_WINDOW_EVT_CLOSE, 0);

	// clean up IEView and H++ log windows
	if (m_hwndIEView != 0) {
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
	if (m_pWnd) {
		delete m_pWnd;
		m_pWnd = 0;
	}
	if (m_sbCustom) {
		delete m_sbCustom;
		m_sbCustom = 0;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// resizer proc for the "new" layout.

int CSrmmWindow::Resizer(UTILRESIZECONTROL *urc)
{
	int panelHeight = m_Panel->getHeight() + 1;
	
	bool bInfoPanel = m_Panel->isActive();
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

	case IDC_LOG:
		if (m_dwFlags & MWF_ERRORSTATE)
			urc->rcItem.bottom -= ERRORPANEL_HEIGHT;
		if (m_dwFlagsEx & MWF_SHOW_SCROLLINGDISABLED || m_bNotOnList)
			urc->rcItem.bottom -= 20;
		if (m_sendMode & SMODE_MULTIPLE)
			urc->rcItem.right -= (m_multiSplitterX + 3);
		urc->rcItem.bottom -= m_splitterY - m_originalSplitterY;
		if (!bShowToolbar || bBottomToolbar)
			urc->rcItem.bottom += 21;
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
		GetClientRect(GetDlgItem(m_hwnd, IDC_MESSAGE), &rc);
		urc->rcItem.top -= m_splitterY - m_originalSplitterY;
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

		if (m_hwndContactPic) //if m_Panel control was created?
			SetWindowPos(m_hwndContactPic, HWND_TOP, 1, ((urc->rcItem.bottom - urc->rcItem.top) - (m_pic.cy)) / 2 + 1,  //resizes it
				m_pic.cx - 2, m_pic.cy - 2, SWP_SHOWWINDOW);

		return RD_ANCHORX_RIGHT | RD_ANCHORY_BOTTOM;

	case IDC_SPLITTER:
		urc->rcItem.right = urc->dlgNewSize.cx;
		urc->rcItem.top -= m_splitterY - m_originalSplitterY;
		urc->rcItem.bottom = urc->rcItem.top + 2;
		OffsetRect(&urc->rcItem, 0, 1);
		urc->rcItem.left = 0;

		if (m_bUseOffset)
			urc->rcItem.right -= (m_pic.cx); // + DPISCALEX(2));
		return RD_ANCHORX_CUSTOM | RD_ANCHORY_BOTTOM;

	case IDC_MESSAGE:
		urc->rcItem.right = urc->dlgNewSize.cx;
		if (m_bShowAvatar)
			urc->rcItem.right -= m_pic.cx + 2;
		urc->rcItem.top -= m_splitterY - m_originalSplitterY;
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
		urc->rcItem.left -= m_multiSplitterX;
		urc->rcItem.right -= m_multiSplitterX;
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
			HDC hdcMem = 0;
			HBITMAP hbm, hbmOld;
			HANDLE hpb = 0;

			GetClientRect(m_hwnd, &rcClient);
			DWORD cx = rcClient.right - rcClient.left;
			DWORD cy = rcClient.bottom - rcClient.top;

			if (CMimAPI::m_haveBufferedPaint) {
				hpb = CMimAPI::m_pfnBeginBufferedPaint(hdc, &rcClient, BPBF_TOPDOWNDIB, 0, &hdcMem);
				hbm = hbmOld = 0;
			}
			else {
				hdcMem = CreateCompatibleDC(hdc);
				hbm = CSkin::CreateAeroCompatibleBitmap(rcClient, hdc);
				hbmOld = (HBITMAP)SelectObject(hdcMem, hbm);
			}

			bool	bAero = M.isAero();

			if (CSkin::m_skinEnabled) {
				UINT item_ids[2] = { ID_EXTBKHISTORY, ID_EXTBKINPUTAREA };
				UINT ctl_ids[2] = { IDC_LOG, IDC_MESSAGE };
				BOOL isEditNotesReason = m_fEditNotesActive;
				BOOL isSendLaterReason = (m_sendMode & SMODE_SENDLATER);
				BOOL isMultipleReason = (m_sendMode & SMODE_MULTIPLE || m_sendMode & SMODE_CONTAINER);

				CSkin::SkinDrawBG(m_hwnd, m_pContainer->hwnd, m_pContainer, &rcClient, hdcMem);

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
					rcClient.bottom = m_Panel->isActive() ? m_Panel->getHeight() + 5 : 5;
					FillRect(hdcMem, &rcClient, (HBRUSH)GetStockObject(BLACK_BRUSH));
					rcClient.bottom = temp;
				}
			}

			// draw the (new) infopanel background. Use the gradient from the statusitem.
			GetClientRect(m_hwnd, &rc);
			m_Panel->renderBG(hdcMem, rc, &SkinItems[ID_EXTBKINFOPANELBG], bAero);

			// draw aero related stuff
			if (!CSkin::m_skinEnabled)
				RenderToolbarBG(hdcMem, rcClient);

			// render info panel fields
			m_Panel->renderContent(hdcMem);

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

	case WM_SIZE:
		if (!IsIconic(m_hwnd)) {
			if (m_ipFieldHeight == 0)
				m_ipFieldHeight = CInfoPanel::m_ipConfig.height2;

			if (m_pContainer->uChildMinHeight > 0 && HIWORD(lParam) >= m_pContainer->uChildMinHeight) {
				if (m_splitterY > HIWORD(lParam) - DPISCALEY_S(MINLOGHEIGHT)) {
					m_splitterY = HIWORD(lParam) - DPISCALEY_S(MINLOGHEIGHT);
					m_dynaSplitter = m_splitterY - DPISCALEY_S(34);
					DM_RecalcPictureSize();
				}
				if (m_splitterY < DPISCALEY_S(MINSPLITTERY))
					LoadSplitter();
			}

			HBITMAP hbm = ((m_Panel->isActive()) && m_pContainer->avatarMode != 3) ? m_hOwnPic : (m_ace ? m_ace->hbmPic : PluginConfig.g_hbmUnknown);
			if (hbm != 0) {
				BITMAP bminfo;
				GetObject(hbm, sizeof(bminfo), &bminfo);
				CalcDynamicAvatarSize(&bminfo);
			}

			GetClientRect(m_hwnd, &rc);

			CTabBaseDlg::DlgProc(uMsg, 0, 0); // call basic window resizer

			BB_SetButtonsPos();

			// size info panel fields
			if (m_Panel->isActive()) {
				LONG cx = rc.right;
				LONG panelHeight = m_Panel->getHeight();

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
					ShowWindow(m_hwndPanelPicParent, (m_iPanelAvatarX == 0) || !m_Panel->isActive() ? SW_HIDE : SW_SHOW);
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

				m_Panel->Invalidate();
			}

			if (GetDlgItem(m_hwnd, IDC_CLIST) != 0) {
				RECT rcLog;
				GetClientRect(m_hwnd, &rcClient);
				GetClientRect(m_log.GetHwnd(), &rcLog);
				rc.top = 0;
				rc.right = rcClient.right;
				rc.left = rcClient.right - m_multiSplitterX;
				rc.bottom = rcLog.bottom;
				if (m_Panel->isActive())
					rc.top += (m_Panel->getHeight() + 1);
				MoveWindow(GetDlgItem(m_hwnd, IDC_CLIST), rc.left, rc.top, rc.right - rc.left, rcLog.bottom - rcLog.top, FALSE);
			}

			if (m_hwndIEView || m_hwndHPP)
				ResizeIeView();

			DetermineMinHeight();
		}
		return 0;

	case WM_TIMECHANGE:
		PostMessage(m_hwnd, DM_OPTIONSAPPLIED, 0, 0);
		break;

	case WM_NOTIFY:
		if (this != 0 && ((NMHDR*)lParam)->hwndFrom == m_hwndTip) {
			if (((NMHDR*)lParam)->code == NM_CLICK)
				SendMessage(m_hwndTip, TTM_TRACKACTIVATE, FALSE, 0);
			break;
		}

		switch (((NMHDR*)lParam)->idFrom) {
		case IDC_LOG:
		case IDC_MESSAGE:
			switch (((NMHDR*)lParam)->code) {
			case EN_MSGFILTER:
				{
					DWORD msg = ((MSGFILTER *)lParam)->msg;
					WPARAM wp = ((MSGFILTER *)lParam)->wParam;
					LPARAM lp = ((MSGFILTER *)lParam)->lParam;
					CHARFORMAT2 cf2;
					bool isCtrl, isShift, isAlt;
					KbdState(isShift, isCtrl, isAlt);

					MSG message;
					message.hwnd = m_hwnd;
					message.message = msg;
					message.lParam = lp;
					message.wParam = wp;

					if (msg == WM_SYSKEYUP) {
						if (wp == VK_MENU)
							if (!m_fkeyProcessed && !(GetKeyState(VK_CONTROL) & 0x8000) && !(GetKeyState(VK_SHIFT) & 0x8000) && !(lp & (1 << 24)))
								m_pContainer->MenuBar->autoShow();

						return _dlgReturn(m_hwnd, 0);
					}

					if ((msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN) && !(GetKeyState(VK_RMENU) & 0x8000)) {
						LRESULT mim_hotkey_check = CallService(MS_HOTKEY_CHECK, (WPARAM)&message, (LPARAM)TABSRMM_HK_SECTION_IM);
						if (mim_hotkey_check)
							m_fkeyProcessed = true;

						switch (mim_hotkey_check) {
						case TABSRMM_HK_SETUSERPREFS:
							CallService(MS_TABMSG_SETUSERPREFS, m_hContact, 0);
							return _dlgReturn(m_hwnd, 1);
						case TABSRMM_HK_NUDGE:
							SendNudge();
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
								SetWindowPos(m_message.GetHwnd(), 0, 0, 0, 0, 0, SWP_DRAWFRAME | SWP_FRAMECHANGED | SWP_NOZORDER |
									SWP_NOMOVE | SWP_NOSIZE | SWP_NOCOPYBITS);
								RedrawWindow(m_hwnd, 0, 0, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW | RDW_ALLCHILDREN);
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
								SendMessage(m_hwnd, DM_OPTIONSAPPLIED, 0, 0);
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
							SetWindowPos(hwndEdit, 0, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOSIZE | SWP_NOMOVE);
							SendMessage(m_hwnd, WM_SIZE, 0, 0);
							RedrawWindow(hwndEdit, NULL, NULL, RDW_INVALIDATE | RDW_FRAME | RDW_UPDATENOW | RDW_ERASE);
							DM_ScrollToBottom(0, 0);
							Utils::showDlgControl(m_hwnd, IDC_MULTISPLITTER, (m_sendMode & SMODE_MULTIPLE) ? SW_SHOW : SW_HIDE);
							Utils::showDlgControl(m_hwnd, IDC_CLIST, (m_sendMode & SMODE_MULTIPLE) ? SW_SHOW : SW_HIDE);
							if (m_sendMode & SMODE_MULTIPLE)
								SetFocus(GetDlgItem(m_hwnd, IDC_CLIST));
							else
								SetFocus(m_message.GetHwnd());
							RedrawWindow(m_hwnd, 0, 0, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW | RDW_ALLCHILDREN);
							return _dlgReturn(m_hwnd, 1);
						}
						if (DM_GenericHotkeysCheck(&message)) {
							m_fkeyProcessed = true;
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
							_clrMsgFilter(lParam);
							return _dlgReturn(m_hwnd, 1);
						}
						if (isCtrl && isShift) {
							if (wp == 0x9) {            // ctrl-shift tab
								SendMessage(m_hwnd, DM_SELECTTAB, DM_SELECT_PREV, 0);
								_clrMsgFilter(lParam);
								return _dlgReturn(m_hwnd, 1);
							}
						}
						if (isCtrl && !isShift && !isAlt) {
							if (wp == VK_TAB) {
								SendMessage(m_hwnd, DM_SELECTTAB, DM_SELECT_NEXT, 0);
								_clrMsgFilter(lParam);
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
							SendMessage(m_pContainer->hwnd, DM_SELECTTAB, DM_SELECT_NEXT, 0);
							return _dlgReturn(m_hwnd, 1);
						}
						if (wp == VK_SUBTRACT) {
							SendMessage(m_pContainer->hwnd, DM_SELECTTAB, DM_SELECT_PREV, 0);
							return _dlgReturn(m_hwnd, 1);
						}
					}

					if (msg == WM_KEYDOWN && wp == VK_F12) {
						if (isShift || isCtrl || isAlt)
							return _dlgReturn(m_hwnd, 1);
						if (m_dwFlagsEx & MWF_SHOW_SCROLLINGDISABLED)
							SendMessage(m_hwnd, DM_REPLAYQUEUE, 0, 0);
						m_dwFlagsEx ^= MWF_SHOW_SCROLLINGDISABLED;
						Utils::showDlgControl(m_hwnd, IDC_LOGFROZENTEXT, (m_bNotOnList || m_dwFlagsEx & MWF_SHOW_SCROLLINGDISABLED) ? SW_SHOW : SW_HIDE);
						if (!(m_dwFlagsEx & MWF_SHOW_SCROLLINGDISABLED))
							SetDlgItemText(m_hwnd, IDC_LOGFROZENTEXT, TranslateT("Contact not on list. You may add it..."));
						else
							SetDlgItemText(m_hwnd, IDC_LOGFROZENTEXT, TranslateT("Auto scrolling is disabled (press F12 to enable it)"));
						SendMessage(m_hwnd, WM_SIZE, 0, 0);
						DM_ScrollToBottom(1, 1);
						return _dlgReturn(m_hwnd, 1);
					}

					// tabulation mod
					if (msg == WM_KEYDOWN && wp == VK_TAB) {
						if (PluginConfig.m_bAllowTab) {
							if (((NMHDR*)lParam)->idFrom == IDC_MESSAGE)
								m_message.SendMsg(EM_REPLACESEL, FALSE, (LPARAM)"\t");
							_clrMsgFilter(lParam);
							if (((NMHDR*)lParam)->idFrom != IDC_MESSAGE)
								SetFocus(m_message.GetHwnd());
							return _dlgReturn(m_hwnd, 1);
						}
						else {
							if (((NMHDR*)lParam)->idFrom == IDC_MESSAGE) {
								if (GetSendButtonState(m_hwnd) != PBS_DISABLED && !(m_pContainer->dwFlags & CNT_HIDETOOLBAR))
									SetFocus(GetDlgItem(m_hwnd, IDOK));
								else
									SetFocus(m_log.GetHwnd());
								return _dlgReturn(m_hwnd, 1);
							}
							if (((NMHDR*)lParam)->idFrom == IDC_LOG) {
								SetFocus(m_message.GetHwnd());
								return _dlgReturn(m_hwnd, 1);
							}
						}
						return _dlgReturn(m_hwnd, 0);
					}

					if (msg == WM_MOUSEWHEEL && (((NMHDR*)lParam)->idFrom == IDC_LOG || ((NMHDR*)lParam)->idFrom == IDC_MESSAGE)) {
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
							SendDlgItemMessage(m_hwnd, ((NMHDR*)lParam)->code, WM_COPY, 0, 0);
							break;
						}
					}
					if ((msg == WM_LBUTTONDOWN || msg == WM_KEYUP || msg == WM_LBUTTONUP) && ((NMHDR*)lParam)->idFrom == IDC_MESSAGE) {
						int bBold = IsDlgButtonChecked(m_hwnd, IDC_FONTBOLD);
						int bItalic = IsDlgButtonChecked(m_hwnd, IDC_FONTITALIC);
						int bUnder = IsDlgButtonChecked(m_hwnd, IDC_FONTUNDERLINE);
						int bStrikeout = IsDlgButtonChecked(m_hwnd, IDC_FONTSTRIKEOUT);

						cf2.cbSize = sizeof(CHARFORMAT2);
						cf2.dwMask = CFM_BOLD | CFM_ITALIC | CFM_UNDERLINE | CFM_UNDERLINETYPE | CFM_STRIKEOUT;
						cf2.dwEffects = 0;
						m_message.SendMsg(EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf2);
						if (cf2.dwEffects & CFE_BOLD) {
							if (bBold == BST_UNCHECKED)
								CheckDlgButton(m_hwnd, IDC_FONTBOLD, BST_CHECKED);
						}
						else if (bBold == BST_CHECKED)
							CheckDlgButton(m_hwnd, IDC_FONTBOLD, BST_UNCHECKED);

						if (cf2.dwEffects & CFE_ITALIC) {
							if (bItalic == BST_UNCHECKED)
								CheckDlgButton(m_hwnd, IDC_FONTITALIC, BST_CHECKED);
						}
						else if (bItalic == BST_CHECKED)
							CheckDlgButton(m_hwnd, IDC_FONTITALIC, BST_UNCHECKED);

						if (cf2.dwEffects & CFE_UNDERLINE && (cf2.bUnderlineType & CFU_UNDERLINE || cf2.bUnderlineType & CFU_UNDERLINEWORD)) {
							if (bUnder == BST_UNCHECKED)
								CheckDlgButton(m_hwnd, IDC_FONTUNDERLINE, BST_CHECKED);
						}
						else if (bUnder == BST_CHECKED)
							CheckDlgButton(m_hwnd, IDC_FONTUNDERLINE, BST_UNCHECKED);

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
							if (hCur == LoadCursor(NULL, IDC_SIZENS) || hCur == LoadCursor(NULL, IDC_SIZEWE)
								|| hCur == LoadCursor(NULL, IDC_SIZENESW) || hCur == LoadCursor(NULL, IDC_SIZENWSE)) {
								SetWindowLongPtr(m_hwnd, DWLP_MSGRESULT, TRUE);
								return TRUE;
							}
						}
						break;

						// auto-select-and-copy handling...
						// if enabled, releasing the lmb with an active selection automatically copies the selection
						// to the clipboard.
						// holding ctrl while releasing the button pastes the selection to the input area, using plain text
						// holding ctrl-alt does the same, but pastes formatted text
					case WM_LBUTTONUP:
						if (((NMHDR*)lParam)->idFrom == IDC_LOG && M.GetByte("autocopy", 1)) {
							CHARRANGE cr;
							m_log.SendMsg(EM_EXGETSEL, 0, (LPARAM)&cr);
							if (cr.cpMax == cr.cpMin)
								break;
							cr.cpMin = cr.cpMax;
							if (isCtrl) {
								SETTEXTEX stx = { ST_KEEPUNDO | ST_SELECTION, CP_UTF8 };
								char *streamOut = NULL;
								if (isAlt)
									streamOut = Message_GetFromStream(m_log.GetHwnd(), SF_RTFNOOBJS | SFF_PLAINRTF | SFF_SELECTION);
								else
									streamOut = Message_GetFromStream(m_log.GetHwnd(), SF_TEXT | SFF_SELECTION);
								if (streamOut) {
									Utils::FilterEventMarkers(streamOut);
									m_message.SendMsg(EM_SETTEXTEX, (WPARAM)&stx, (LPARAM)streamOut);
									mir_free(streamOut);
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
						m_Panel->trackMouse(pt);

						HCURSOR hCur = GetCursor();
						if (hCur == LoadCursor(NULL, IDC_SIZENS) || hCur == LoadCursor(NULL, IDC_SIZEWE) || hCur == LoadCursor(NULL, IDC_SIZENESW) || hCur == LoadCursor(NULL, IDC_SIZENWSE))
							SetCursor(LoadCursor(NULL, IDC_ARROW));
						break;
					}
				}
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
			m_hXStatusIcon = 0;
		}

		if (LPCSTR szProto = m_cache->getProto()) {
			m_hTabIcon = m_hTabStatusIcon = GetMyContactIcon("MetaiconTab");
			if (M.GetByte("use_xicons", 1))
				m_hXStatusIcon = GetXStatusIcon();

			SendDlgItemMessage(m_hwnd, IDC_PROTOCOL, BUTTONSETASDIMMED, (m_dwFlagsEx & MWF_SHOW_ISIDLE) != 0, 0);
			SendDlgItemMessage(m_hwnd, IDC_PROTOCOL, BM_SETIMAGE, IMAGE_ICON, (LPARAM)(m_hXStatusIcon ? m_hXStatusIcon : GetMyContactIcon("MetaiconBar")));

			if (m_pContainer->hwndActive == m_hwnd)
				SendMessage(m_pContainer->hwnd, DM_SETICON, (WPARAM)this, (LPARAM)(m_hXStatusIcon ? m_hXStatusIcon : m_hTabIcon));

			if (m_pWnd)
				m_pWnd->updateIcon(m_hXStatusIcon ? m_hXStatusIcon : m_hTabIcon);
		}
		return 0;

		// configures the toolbar only... if lParam != 0, then it also calls
		// SetDialogToType() to reconfigure the message window
	case DM_CONFIGURETOOLBAR:
		m_bShowUIElements = m_pContainer->dwFlags & CNT_HIDETOOLBAR ? 0 : 1;

		SetWindowLongPtr(GetDlgItem(m_hwnd, IDC_SPLITTER), GWL_EXSTYLE, GetWindowLongPtr(GetDlgItem(m_hwnd, IDC_SPLITTER), GWL_EXSTYLE) & ~WS_EX_STATICEDGE);

		if (lParam == 1) {
			GetSendFormat();
			SetDialogToType(m_hwnd);
		}

		if (lParam == 1) {
			DM_RecalcPictureSize();
			SendMessage(m_hwnd, WM_SIZE, 0, 0);
			DM_ScrollToBottom(0, 1);
		}
		return 0;

	case WM_CBD_LOADICONS:
		for (int i = 0; i < _countof(buttonicons); i++) {
			SendDlgItemMessage(m_hwnd, buttonicons[i].id, BM_SETIMAGE, IMAGE_ICON, (LPARAM)*buttonicons[i].pIcon);
			SendDlgItemMessage(m_hwnd, buttonicons[i].id, BUTTONSETCONTAINER, (LPARAM)m_pContainer, 0);
		}

		Srmm_UpdateToolbarIcons(m_hwnd);
		SendMessage(m_hwnd, DM_UPDATEWINICON, 0, 0);
		return 0;

	case DM_OPTIONSAPPLIED:
		DM_OptionsApplied(wParam, lParam);
		return 0;

	case DM_UPDATETITLE:
		DM_UpdateTitle(wParam, lParam);
		return 0;

	case DM_UPDATESTATUSMSG:
		m_Panel->Invalidate();
		return 0;

	case DM_OWNNICKCHANGED:
		GetMyNick();
		return 0;

	case DM_ADDDIVIDER:
		if (!(m_dwFlags & MWF_DIVIDERSET) && PluginConfig.m_bUseDividers) {
			if (GetWindowTextLength(m_log.GetHwnd()) > 0) {
				m_dwFlags |= MWF_DIVIDERWANTED;
				m_dwFlags |= MWF_DIVIDERSET;
			}
		}
		return 0;

	case WM_SETFOCUS:
		MsgWindowUpdateState(WM_SETFOCUS);
		SetFocus(m_message.GetHwnd());
		return 1;

	case WM_ACTIVATE:
		if (LOWORD(wParam) != WA_ACTIVE)
			break;

		//fall through
	case WM_MOUSEACTIVATE:
		MsgWindowUpdateState(WM_ACTIVATE);
		return 1;

	case DM_UPDATEPICLAYOUT:
		LoadContactAvatar();
		SendMessage(m_hwnd, WM_SIZE, 0, 0);
		return 0;

	case DM_SPLITTERGLOBALEVENT:
		DM_SplitterGlobalEvent(wParam, lParam);
		return 0;

	case DM_SPLITTERMOVED:
		if ((HWND)lParam == GetDlgItem(m_hwnd, IDC_MULTISPLITTER)) {
			GetClientRect(m_hwnd, &rc);
			pt.x = wParam;
			pt.y = 0;
			ScreenToClient(m_hwnd, &pt);
			int oldSplitterX = m_multiSplitterX;
			m_multiSplitterX = rc.right - pt.x;
			if (m_multiSplitterX < 25)
				m_multiSplitterX = 25;

			if (m_multiSplitterX > ((rc.right - rc.left) - 80))
				m_multiSplitterX = oldSplitterX;
			SendMessage(m_hwnd, WM_SIZE, 0, 0);
		}
		else if ((HWND)lParam == GetDlgItem(m_hwnd, IDC_SPLITTER)) {
			GetClientRect(m_hwnd, &rc);
			rc.top += (m_Panel->isActive() ? m_Panel->getHeight() + 40 : 30);
			pt.x = 0;
			pt.y = wParam;
			ScreenToClient(m_hwnd, &pt);

			int oldSplitterY = m_splitterY;
			int oldDynaSplitter = m_dynaSplitter;

			m_splitterY = rc.bottom - pt.y + DPISCALEY_S(23);

			// attempt to fix splitter troubles..
			// hardcoded limits... better solution is possible, but this works for now
			int bottomtoolbarH = 0;
			if (m_pContainer->dwFlags & CNT_BOTTOMTOOLBAR)
				bottomtoolbarH = 22;

			if (m_splitterY < (DPISCALEY_S(MINSPLITTERY) + 5 + bottomtoolbarH)) {	// min splitter size
				m_splitterY = (DPISCALEY_S(MINSPLITTERY) + 5 + bottomtoolbarH);
				m_dynaSplitter = m_splitterY - DPISCALEY_S(34);
				DM_RecalcPictureSize();
			}
			else if (m_splitterY >(rc.bottom - rc.top)) {
				m_splitterY = oldSplitterY;
				m_dynaSplitter = oldDynaSplitter;
				DM_RecalcPictureSize();
			}
			else {
				m_dynaSplitter = (rc.bottom - pt.y) - DPISCALEY_S(11);
				DM_RecalcPictureSize();
			}
			UpdateToolbarBG();
			SendMessage(m_hwnd, WM_SIZE, 0, 0);
		}
		else if ((HWND)lParam == GetDlgItem(m_hwnd, IDC_PANELSPLITTER)) {
			GetClientRect(m_log.GetHwnd(), &rc);

			POINT	pnt = { 0, (int)wParam };
			ScreenToClient(m_hwnd, &pnt);
			if ((pnt.y + 2 >= MIN_PANELHEIGHT + 2) && (pnt.y + 2 < 100) && (pnt.y + 2 < rc.bottom - 30))
				m_Panel->setHeight(pnt.y + 2, true);

			RedrawWindow(m_hwnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
			if (M.isAero())
				InvalidateRect(GetParent(m_hwnd), NULL, FALSE);
		}
		break;

		// queue a dm_remakelog
		// wParam = hwnd of the sender, so we can directly do a DM_REMAKELOG if the msg came
		// from ourself. otherwise, the dm_remakelog will be deferred until next window
		// activation (focus)
	case DM_DEFERREDREMAKELOG:
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
		StreamInEvents(m_hDbEventFirst, -1, 0, NULL);
		return 0;

	case DM_APPENDMCEVENT:
		if (m_hContact == db_mc_getMeta(wParam) && m_hDbEventFirst == NULL) {
			m_hDbEventFirst = lParam;
			SendMessage(m_hwnd, DM_REMAKELOG, 0, 0);
		}
		else if (m_hContact == wParam && db_mc_isSub(wParam) && db_event_getContact(lParam) != wParam)
			StreamInEvents(lParam, 1, 1, NULL);
		return 0;

	case DM_APPENDTOLOG:
		StreamInEvents(wParam, 1, 1, NULL);
		return 0;

	case DM_REPLAYQUEUE: // replays queued events after the message log has been frozen for a while
		for (int i = 0; i < m_iNextQueuedEvent; i++)
			if (m_hQueuedEvents[i] != 0)
				StreamInEvents(m_hQueuedEvents[i], 1, 1, NULL);

		m_iNextQueuedEvent = 0;
		SetDlgItemText(m_hwnd, IDC_LOGFROZENTEXT, m_bNotOnList ? TranslateT("Contact not on list. You may add it...") :
			TranslateT("Auto scrolling is disabled (press F12 to enable it)"));
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

			if (wParam == TIMERID_AWAYMSG && m_Panel->hitTest(pt) != CInfoPanel::HTNIRVANA)
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
					SkinPlaySound("SendError");
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

	case DM_ERRORDECIDED:
		switch (wParam) {
		case MSGERROR_CANCEL:
		case MSGERROR_SENDLATER:
			if (m_dwFlags & MWF_ERRORSTATE) {
				m_cache->saveHistory(0, 0);
				if (wParam == MSGERROR_SENDLATER)
					sendQueue->doSendLater(m_iCurrentQueueError, this); // to be implemented at a later time
				m_iOpenJobs--;
				sendQueue->dec();
				if (m_iCurrentQueueError >= 0 && m_iCurrentQueueError < SendQueue::NR_SENDJOBS)
					sendQueue->clearJob(m_iCurrentQueueError);
				m_iCurrentQueueError = -1;
				sendQueue->showErrorControls(this, FALSE);
				if (wParam != MSGERROR_CANCEL || (wParam == MSGERROR_CANCEL && lParam == 0))
					SetDlgItemText(m_hwnd, IDC_MESSAGE, L"");
				sendQueue->checkQueue(this);
				int iNextFailed = sendQueue->findNextFailed(this);
				if (iNextFailed >= 0)
					sendQueue->handleError(this, iNextFailed);
			}
			break;

		case MSGERROR_RETRY:
			if (m_dwFlags & MWF_ERRORSTATE) {
				int resent = 0;

				m_cache->saveHistory(0, 0);
				if (m_iCurrentQueueError >= 0 && m_iCurrentQueueError < SendQueue::NR_SENDJOBS) {
					SendJob *job = sendQueue->getJobByIndex(m_iCurrentQueueError);
					if (job->hSendId == 0 && job->hContact == 0)
						break;

					job->hSendId = (HANDLE)ProtoChainSend(job->hContact, PSS_MESSAGE, job->dwFlags, (LPARAM)job->szSendBuffer);
					resent++;
				}

				if (resent) {
					SendJob *job = sendQueue->getJobByIndex(m_iCurrentQueueError);

					SetTimer(m_hwnd, TIMERID_MSGSEND + m_iCurrentQueueError, PluginConfig.m_MsgTimeout, NULL);
					job->iStatus = SendQueue::SQ_INPROGRESS;
					m_iCurrentQueueError = -1;
					sendQueue->showErrorControls(this, FALSE);
					SetDlgItemText(m_hwnd, IDC_MESSAGE, L"");
					sendQueue->checkQueue(this);

					int iNextFailed = sendQueue->findNextFailed(this);
					if (iNextFailed >= 0)
						sendQueue->handleError(this, iNextFailed);
				}
			}
		}
		break;

	case DM_SELECTTAB:
		SendMessage(m_pContainer->hwnd, DM_SELECTTAB, wParam, lParam);       // pass the msg to our container
		return 0;

	case DM_SETLOCALE:
		if (m_dwFlags & MWF_WASBACKGROUNDCREATE)
			break;
		if (m_pContainer->hwndActive == m_hwnd && PluginConfig.m_bAutoLocaleSupport && m_pContainer->hwnd == GetForegroundWindow() && m_pContainer->hwnd == GetActiveWindow()) {
			if (lParam)
				m_hkl = (HKL)lParam;

			if (m_hkl)
				ActivateKeyboardLayout(m_hkl, 0);
		}
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

	case DM_QUERYCONTAINER:
		{
			TContainerData **pc = (TContainerData **)lParam;
			if (pc)
				*pc = m_pContainer;
		}
		return 0;

	case DM_QUERYHCONTACT:
		{
			MCONTACT *phContact = (MCONTACT*)lParam;
			if (phContact)
				*phContact = m_hContact;
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

		SendMessage(m_pContainer->hwnd, DM_QUERYCLIENTAREA, 0, (LPARAM)&rcClient);
		MoveWindow(m_hwnd, rcClient.left, rcClient.top, (rcClient.right - rcClient.left), (rcClient.bottom - rcClient.top), TRUE);
		if (m_dwFlags & MWF_WASBACKGROUNDCREATE) {
			m_dwFlags &= ~MWF_WASBACKGROUNDCREATE;
			SendMessage(m_hwnd, WM_SIZE, 0, 0);
			PostMessage(m_hwnd, DM_UPDATEPICLAYOUT, 0, 0);
			if (PluginConfig.m_bAutoLocaleSupport) {
				if (m_hkl == 0)
					DM_LoadLocale();
				else
					PostMessage(m_hwnd, DM_SETLOCALE, 0, 0);
			}
			if (m_hwndIEView != 0)
				SetFocus(m_message.GetHwnd());
			if (m_pContainer->dwFlags & CNT_SIDEBAR)
				m_pContainer->SideBar->Layout();
		}
		else {
			SendMessage(m_hwnd, WM_SIZE, 0, 0);
			if (lParam == 0)
				DM_ScrollToBottom(0, 1);
		}
		return 0;

	case DM_CHECKSIZE:
		m_dwFlags |= MWF_NEEDCHECKSIZE;
		return 0;

		// sent by the message input area hotkeys. just pass it to our container
	case DM_QUERYPENDING:
		SendMessage(m_pContainer->hwnd, DM_QUERYPENDING, wParam, lParam);
		return 0;

	case WM_LBUTTONDOWN:
		GetCursorPos(&tmp);
		cur.x = (SHORT)tmp.x;
		cur.y = (SHORT)tmp.y;
		if (!m_Panel->isHovered())
			SendMessage(m_pContainer->hwnd, WM_NCLBUTTONDOWN, HTCAPTION, *((LPARAM*)(&cur)));
		break;

	case WM_LBUTTONUP:
		GetCursorPos(&tmp);
		if (m_Panel->isHovered())
			m_Panel->handleClick(tmp);
		else {
			cur.x = (SHORT)tmp.x;
			cur.y = (SHORT)tmp.y;
			SendMessage(m_pContainer->hwnd, WM_NCLBUTTONUP, HTCAPTION, *((LPARAM*)(&cur)));
		}
		break;

	case WM_RBUTTONUP:
		{
			RECT rcPicture, rcPanelNick = { 0 };
			int menuID = 0;

			GetWindowRect(GetDlgItem(m_hwnd, IDC_CONTACTPIC), &rcPicture);
			rcPanelNick.left = rcPanelNick.right - 30;
			GetCursorPos(&pt);

			if (m_Panel->invokeConfigDialog(pt))
				break;

			if (PtInRect(&rcPicture, pt))
				menuID = MENU_PICMENU;

			if ((menuID == MENU_PICMENU && ((m_ace ? m_ace->hbmPic : PluginConfig.g_hbmUnknown) || m_hOwnPic) && m_bShowAvatar != 0)) {
				HMENU submenu = GetSubMenu(PluginConfig.g_hMenuContext, 1);
				GetCursorPos(&pt);
				MsgWindowUpdateMenu(submenu, menuID);
				
				int iSelection = TrackPopupMenu(submenu, TPM_RETURNCMD, pt.x, pt.y, 0, m_hwnd, NULL);
				MsgWindowMenuHandler(iSelection, menuID);
				break;
			}

			HMENU subMenu = GetSubMenu(PluginConfig.g_hMenuContext, 0);
			MsgWindowUpdateMenu(subMenu, MENU_TABCONTEXT);

			int iSelection = TrackPopupMenu(subMenu, TPM_RETURNCMD, pt.x, pt.y, 0, m_hwnd, NULL);
			if (iSelection >= IDM_CONTAINERMENU) {
				char szIndex[10];
				char *szKey = "TAB_ContainersW";

				mir_snprintf(szIndex, "%d", iSelection - IDM_CONTAINERMENU);
				if (iSelection - IDM_CONTAINERMENU >= 0) {
					ptrW val(db_get_wsa(NULL, szKey, szIndex));
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
		m_Panel->trackMouse(pt);
		break;

	case WM_MEASUREITEM:
		{
			LPMEASUREITEMSTRUCT lpmi = (LPMEASUREITEMSTRUCT)lParam;
			if (m_Panel->isHovered()) {
				lpmi->itemHeight = 0;
				lpmi->itemWidth = 6;
				return TRUE;
			}
		}
		return Menu_MeasureItem(lParam);

	case WM_NCHITTEST:
		SendMessage(m_pContainer->hwnd, WM_NCHITTEST, wParam, lParam);
		break;

	case WM_DRAWITEM:
		return MsgWindowDrawHandler(wParam, lParam);

	case WM_APPCOMMAND:
		{
			DWORD cmd = GET_APPCOMMAND_LPARAM(lParam);
			if (cmd == APPCOMMAND_BROWSER_BACKWARD || cmd == APPCOMMAND_BROWSER_FORWARD) {
				SendMessage(m_pContainer->hwnd, DM_SELECTTAB, cmd == APPCOMMAND_BROWSER_BACKWARD ? DM_SELECT_PREV : DM_SELECT_NEXT, 0);
				return 1;
			}
		}
		break;

	case WM_COMMAND:
		if (!this)
			break;

		// custom button handling
		if (LOWORD(wParam) >= MIN_CBUTTONID && LOWORD(wParam) <= MAX_CBUTTONID) {
			Srmm_ClickToolbarIcon(m_hContact, LOWORD(wParam), GetDlgItem(m_hwnd, LOWORD(wParam)), 0);
			break;
		}

		switch (LOWORD(wParam)) {
		case IDOK:
			if (m_fEditNotesActive) {
				SendMessage(m_hwnd, DM_ACTIVATETOOLTIP, IDC_PIC, (LPARAM)TranslateT("You are editing the user notes. Click the button again or use the hotkey (default: Alt-N) to save the notes and return to normal messaging mode"));
				return 0;
			}
			else {
				// don't parse text formatting when the message contains curly braces - these are used by the rtf syntax
				// and the parser currently cannot handle them properly in the text - XXX needs to be fixed later.
				FINDTEXTEXA fi = { 0 };
				fi.chrg.cpMin = 0;
				fi.chrg.cpMax = -1;
				fi.lpstrText = "{";
				int final_sendformat = SendDlgItemMessageA(m_hwnd, IDC_MESSAGE, EM_FINDTEXTEX, FR_DOWN, (LPARAM)&fi) == -1 ? m_SendFormat : 0;
				fi.lpstrText = "}";
				final_sendformat = SendDlgItemMessageA(m_hwnd, IDC_MESSAGE, EM_FINDTEXTEX, FR_DOWN, (LPARAM)&fi) == -1 ? final_sendformat : 0;

				if (GetSendButtonState(m_hwnd) == PBS_DISABLED)
					break;

				ptrA streamOut(Message_GetFromStream(m_message.GetHwnd(), final_sendformat ? 0 : SF_TEXT));
				if (streamOut == NULL)
					break;

				CMStringW decoded(ptrW(mir_utf8decodeW(streamOut)));
				if (decoded.IsEmpty())
					break;

				if (final_sendformat)
					DoRtfToTags(decoded, _countof(rtfDefColors), rtfDefColors);
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
					if (SendQueue::RTL_Detect(decoded))
						flags |= PREF_RTL;

				SendMessage(hwndEdit, WM_SETREDRAW, TRUE, 0);
				SendMessage(hwndEdit, EM_SETSEL, -1, -1);
				InvalidateRect(hwndEdit, NULL, FALSE);

				if (memRequired > m_iSendBufferSize) {
					m_sendBuffer = (char *)mir_realloc(m_sendBuffer, memRequired);
					m_iSendBufferSize = memRequired;
				}

				memcpy(m_sendBuffer, (char*)utfResult, memRequired);

				if (memRequired == 0 || m_sendBuffer[0] == 0)
					break;

				if (m_sendMode & SMODE_CONTAINER && m_pContainer->hwndActive == m_hwnd && GetForegroundWindow() == m_pContainer->hwnd) {
					int tabCount = TabCtrl_GetItemCount(m_hwndParent);
					ptrA szFromStream(Message_GetFromStream(m_message.GetHwnd(), m_SendFormat ? 0 : SF_TEXT));

					TCITEM tci = { 0 };
					tci.mask = TCIF_PARAM;
					for (int i = 0; i < tabCount; i++) {
						TabCtrl_GetItem(m_hwndParent, i, &tci);
						// get the contact from the tabs lparam which hopefully is the tabs hwnd so we can get its userdata.... hopefully
						HWND contacthwnd = (HWND)tci.lParam;
						if (IsWindow(contacthwnd)) {
							// if the contact hwnd is the current contact then ignore it and let the normal code deal with the msg
							if (contacthwnd != m_hwnd) {
								SETTEXTEX stx = { ST_DEFAULT, CP_UTF8 };
								// send the buffer to the contacts msg typing area
								SendDlgItemMessage(contacthwnd, IDC_MESSAGE, EM_SETTEXTEX, (WPARAM)&stx, (LPARAM)szFromStream);
								SendMessage(contacthwnd, WM_COMMAND, IDOK, 0);
							}
						}
					}
				}
				// END /all /MOD
				if (m_nTypeMode == PROTOTYPE_SELFTYPING_ON)
					DM_NotifyTyping(PROTOTYPE_SELFTYPING_OFF);

				DeletePopupsForContact(m_hContact, PU_REMOVE_ON_SEND);
				if (M.GetByte("allow_sendhook", 0)) {
					int result = TABSRMM_FireEvent(m_hContact, m_hwnd, MSG_WINDOW_EVT_CUSTOM, MAKELONG(flags, tabMSG_WINDOW_EVT_CUSTOM_BEFORESEND));
					if (result)
						return TRUE;
				}
				sendQueue->addTo(this, memRequired, flags);
			}
			return TRUE;

		case IDC_QUOTE:
			{
				CHARRANGE sel;
				SETTEXTEX stx = { ST_SELECTION, 1200 };

				MEVENT hDBEvent = 0;
				if (m_hwndIEView || m_hwndHPP) {                // IEView quoting support..
					wchar_t *selected = 0;

					IEVIEWEVENT event = { sizeof(event) };
					event.hContact = m_hContact;
					event.dwFlags = 0;
					event.iType = IEE_GET_SELECTION;

					if (m_hwndIEView) {
						event.hwnd = m_hwndIEView;
						selected = (wchar_t*)CallService(MS_IEVIEW_EVENT, 0, (LPARAM)&event);
					}
					else {
						event.hwnd = m_hwndHPP;
						selected = (wchar_t*)CallService(MS_HPP_EG_EVENT, 0, (LPARAM)&event);
					}

					if (selected != NULL) {
						ptrW szQuoted(QuoteText(selected));
						m_message.SendMsg(EM_SETTEXTEX, (WPARAM)&stx, (LPARAM)szQuoted);
						break;
					}
					else {
						hDBEvent = db_event_last(m_hContact);
						goto quote_from_last;
					}
				}
				hDBEvent = m_hDbEventLast;

quote_from_last:
				if (hDBEvent == NULL)
					break;

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
						memmove(szText + iDescr + 2, szText + sizeof(DWORD) + iDescr, dbei.cbBlob - iDescr - sizeof(DWORD)-1);
						szText[iDescr] = '\r';
						szText[iDescr + 1] = '\n';
						szConverted = (wchar_t*)mir_alloc(sizeof(wchar_t)* (1 + mir_strlen((char *)szText)));
						MultiByteToWideChar(CP_ACP, 0, (char *)szText, -1, szConverted, 1 + (int)mir_strlen((char *)szText));
						bNeedsFree = true;
					}
					if (szConverted != NULL) {
						ptrW szQuoted(QuoteText(szConverted));
						m_message.SendMsg(EM_SETTEXTEX, (WPARAM)&stx, (LPARAM)szQuoted);
					}
					mir_free(szText);
					if (bNeedsFree)
						mir_free(szConverted);
				}
				else {
					ptrA szFromStream(Message_GetFromStream(m_log.GetHwnd(), SF_TEXT | SFF_SELECTION));
					ptrW converted(mir_utf8decodeW(szFromStream));
					Utils::FilterEventMarkers(converted);
					ptrW szQuoted(QuoteText(converted));
					m_message.SendMsg(EM_SETTEXTEX, (WPARAM)&stx, (LPARAM)szQuoted);
				}
				SetFocus(m_message.GetHwnd());
			}
			break;

		case IDC_ADD:
			{
				ADDCONTACTSTRUCT acs = { 0 };
				acs.hContact = m_hContact;
				acs.handleType = HANDLE_CONTACT;
				acs.szProto = 0;
				CallService(MS_ADDCONTACT_SHOW, (WPARAM)m_hwnd, (LPARAM)&acs);
				if (!db_get_b(m_hContact, "CList", "NotOnList", 0)) {
					m_bNotOnList = FALSE;
					ShowMultipleControls(m_hwnd, addControls, _countof(addControls), SW_HIDE);
					if (!(m_dwFlagsEx & MWF_SHOW_SCROLLINGDISABLED))
						Utils::showDlgControl(m_hwnd, IDC_LOGFROZENTEXT, SW_HIDE);
					SendMessage(m_hwnd, WM_SIZE, 0, 0);
				}
			}
			break;

		case IDC_CANCELADD:
			m_bNotOnList = FALSE;
			ShowMultipleControls(m_hwnd, addControls, _countof(addControls), SW_HIDE);
			if (!(m_dwFlagsEx & MWF_SHOW_SCROLLINGDISABLED))
				Utils::showDlgControl(m_hwnd, IDC_LOGFROZENTEXT, SW_HIDE);
			SendMessage(m_hwnd, WM_SIZE, 0, 0);
			break;

		case IDC_MESSAGE:
			if (HIWORD(wParam) == EN_CHANGE) {
				if (m_pContainer->hwndActive == m_hwnd)
					UpdateReadChars();
				m_dwFlags |= MWF_NEEDHISTORYSAVE;
				m_dwLastActivity = GetTickCount();
				m_pContainer->dwLastActivity = m_dwLastActivity;
				SendQueue::UpdateSaveAndSendButton(this);
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
			break;

		default:
			DM_MsgWindowCmdHandler(LOWORD(wParam), wParam, lParam);
			break;
		}
		break;

	case WM_CONTEXTMENU:
		{
			DWORD idFrom = GetDlgCtrlID((HWND)wParam);
			if (idFrom >= MIN_CBUTTONID && idFrom <= MAX_CBUTTONID) {
				Srmm_ClickToolbarIcon(m_hContact, idFrom, (HWND)wParam, 1);
				break;
			}
		}
		break;

		// this is now *only* called from the global ME_PROTO_ACK handler (static int ProtoAck() in msgs.c)
		// it receives:
		// wParam = index of the sendjob in the queue in the low word, index of the found sendID in the high word
		// (normally 0, but if its a multisend job, then the sendjob may contain more than one hContact/hSendId pairs.)
		// lParam = the original ackdata
		//
		// the "per message window" ACK hook is gone, the global ack handler cares about all types of ack's (currently
		// *_MESSAGE and *_AVATAR and dispatches them to the owner windows).
	case HM_EVENTSENT:
		sendQueue->ackMessage(this, wParam, lParam);
		return 0;

	case DM_ACTIVATEME:
		ActivateExistingTab(m_pContainer, m_hwnd);
		return 0;

		// sent by the select container dialog box when a container was selected...
		// lParam = (wchar_t*)selected name...
	case DM_CONTAINERSELECTED:
		{
			wchar_t *szNewName = (wchar_t*)lParam;
			if (!mir_wstrcmp(szNewName, TranslateT("Default container")))
				szNewName = CGlobals::m_default_container_name;

			int iOldItems = TabCtrl_GetItemCount(m_hwndParent);
			if (!wcsncmp(m_pContainer->szName, szNewName, CONTAINER_NAMELEN))
				break;

			TContainerData *pNewContainer = FindContainerByName(szNewName);
			if (pNewContainer == NULL)
				if ((pNewContainer = CreateContainer(szNewName, FALSE, m_hContact)) == NULL)
					break;

			db_set_ws(m_hContact, SRMSGMOD_T, "containerW", szNewName);
			m_fIsReattach = TRUE;
			PostMessage(PluginConfig.g_hwndHotkeyHandler, DM_DOCREATETAB, (WPARAM)pNewContainer, m_hContact);
			if (iOldItems > 1)                // there were more than 1 tab, container is still valid
				SendMessage(m_pContainer->hwndActive, WM_SIZE, 0, 0);
			SetForegroundWindow(pNewContainer->hwnd);
			SetActiveWindow(pNewContainer->hwnd);
		}
		break;

	case DM_STATUSBARCHANGED:
		UpdateStatusBar();
		return 0;

	case DM_UINTOCLIPBOARD:
		Utils::CopyToClipBoard(m_cache->getUIN(), m_hwnd);
		return 0;

		// broadcasted when GLOBAL info panel setting changes
	case DM_SETINFOPANEL:
		CInfoPanel::setPanelHandler(this, wParam, lParam);
		return 0;

		// show the balloon tooltip control.
		// wParam == id of the "anchor" element, defaults to the panel status field (for away msg retrieval)
		// lParam == new text to show
	case DM_ACTIVATETOOLTIP:
		if (IsIconic(m_pContainer->hwnd) || m_pContainer->hwndActive != m_hwnd)
			break;

		m_Panel->showTip(wParam, lParam);
		break;

	case WM_NEXTDLGCTL:
		if (m_dwFlags & MWF_WASBACKGROUNDCREATE)
			return 1;
		break;

		// save the contents of the log as rtf file
	case DM_SAVEMESSAGELOG:
		DM_SaveLogAsRTF();
		return 0;

	case DM_CHECKAUTOHIDE:
		DM_CheckAutoHide(wParam, lParam);
		return 0;

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

	case DM_GETWINDOWSTATE:
		{
			UINT state = MSG_WINDOW_STATE_EXISTS;
			if (IsWindowVisible(m_hwnd))
				state |= MSG_WINDOW_STATE_VISIBLE;
			if (GetForegroundWindow() == m_pContainer->hwnd)
				state |= MSG_WINDOW_STATE_FOCUS;
			if (IsIconic(m_pContainer->hwnd))
				state |= MSG_WINDOW_STATE_ICONIC;
			SetWindowLongPtr(m_hwnd, DWLP_MSGRESULT, state);
		}
		return TRUE;

	case DM_CLIENTCHANGED:
		GetClientIcon();
		if (m_hClientIcon && m_Panel->isActive())
			InvalidateRect(m_hwnd, NULL, TRUE);
		return 0;

	case DM_UPDATEUIN:
		if (m_Panel->isActive())
			m_Panel->Invalidate();
		if (m_pContainer->dwFlags & CNT_UINSTATUSBAR)
			UpdateStatusBar();
		return 0;

	case DM_REMOVEPOPUPS:
		DeletePopupsForContact(m_hContact, (DWORD)wParam);
		return 0;

	case EM_THEMECHANGED:
		DM_FreeTheme();
		DM_ThemeChanged();
		return 0;

	case DM_PLAYINCOMINGSOUND:
		PlayIncomingSound();
		return 0;

	case DM_REFRESHTABINDEX:
		m_iTabID = GetTabIndexFromHWND(GetParent(m_hwnd), m_hwnd);
		return 0;

	case DM_STATUSICONCHANGE:
		if (m_pContainer->hwndStatus) {
			SendMessage(m_pContainer->hwnd, WM_SIZE, 0, 0);
			SendMessage(m_pContainer->hwndStatus, SB_SETTEXT, (WPARAM)(SBT_OWNERDRAW) | 2, 0);
			InvalidateRect(m_pContainer->hwndStatus, NULL, TRUE);
		}
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
				if (szProto == NULL)
					break;

				int pcaps = CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_1, 0);
				if (!(pcaps & PF1_FILESEND))
					break;
				if (m_wStatus == ID_STATUS_OFFLINE) {
					pcaps = CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_4, 0);
					if (!(pcaps & PF4_OFFLINEFILES)) {
						SendMessage(m_hwnd, DM_ACTIVATETOOLTIP, IDC_MESSAGE, (LPARAM)TranslateT("Contact is offline and this protocol does not support sending files to offline users."));
						break;
					}
				}
			}

			if (m_hContact != NULL) {
				wchar_t szFilename[MAX_PATH];
				HDROP hDrop = (HDROP)wParam;
				int fileCount = DragQueryFile(hDrop, -1, NULL, 0), totalCount = 0, i;
				wchar_t** ppFiles = NULL;
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
						SendDlgItemMessageA(m_hwnd, IDC_MESSAGE, EM_REPLACESEL, TRUE, (LPARAM)szHTTPText);
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
		// esc handles error controls if we are in error state (error controls visible)
		if (wParam == 0 && lParam == 0 && m_dwFlags & MWF_ERRORSTATE) {
			SendMessage(m_hwnd, DM_ERRORDECIDED, MSGERROR_CANCEL, 0);
			return TRUE;
		}

		if (wParam == 0 && lParam == 0) {
			if (PluginConfig.m_EscapeCloses == 1) {
				SendMessage(m_pContainer->hwnd, WM_SYSCOMMAND, SC_MINIMIZE, 0);
				return TRUE;
			}
			else if (PluginConfig.m_bHideOnClose && PluginConfig.m_EscapeCloses == 2) {
				ShowWindow(m_pContainer->hwnd, SW_HIDE);
				return TRUE;
			}
			_dlgReturn(m_hwnd, TRUE);
		}

		if (m_iOpenJobs > 0 && lParam != 2) {
			if (m_dwFlags & MWF_ERRORSTATE) {
				SendMessage(m_hwnd, DM_ERRORDECIDED, MSGERROR_CANCEL, 1);
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
				PostMessage(m_pContainer->hwnd, WM_CLOSE, 0, 1);
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

				TCITEM item = { 0 };
				item.mask = TCIF_PARAM;
				TabCtrl_GetItem(m_hwndParent, i, &item);         // retrieve dialog hwnd for the now active tab...

				m_pContainer->hwndActive = (HWND)item.lParam;

				SendMessage(m_pContainer->hwnd, DM_QUERYCLIENTAREA, 0, (LPARAM)&rc);
				SetWindowPos(m_pContainer->hwndActive, HWND_TOP, rc.left, rc.top, (rc.right - rc.left), (rc.bottom - rc.top), SWP_SHOWWINDOW);
				ShowWindow((HWND)item.lParam, SW_SHOW);
				SetForegroundWindow(m_pContainer->hwndActive);
				SetFocus(m_pContainer->hwndActive);
			}
		}

		SendMessage(m_pContainer->hwnd, WM_SIZE, 0, 0);
		break;

	case WM_DWMCOMPOSITIONCHANGED:
		BB_RefreshTheme();
		memset((void*)&m_pContainer->mOld, -1000, sizeof(MARGINS));
		CProxyWindow::verify(this);
		break;

	case DM_FORCEREDRAW:
		RedrawWindow(m_hwnd, NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW);
		return 0;

	case DM_CHECKINFOTIP:
		m_Panel->hideTip(reinterpret_cast<HWND>(lParam));
		return 0;
	}
	
	return CTabBaseDlg::DlgProc(uMsg, wParam, lParam);
}
