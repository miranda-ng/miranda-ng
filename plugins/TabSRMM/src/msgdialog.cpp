/*
 * Miranda NG: the free IM client for Microsoft* Windows*
 *
 * Copyright (c) 2000-09 Miranda ICQ/IM project,
 * all portions of this codebase are copyrighted to the people
 * listed in contributors.txt.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * you should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * part of tabSRMM messaging plugin for Miranda.
 *
 * (C) 2005-2010 by silvercircle _at_ gmail _dot_ com and contributors
 *
 * This implements the message dialog window.
 *
 */

#include "commonheaders.h"

#define MS_HTTPSERVER_ADDFILENAME "HTTPServer/AddFileName"

bool IsStringValidLink(TCHAR* pszText);

const TCHAR *pszIDCSAVE_close = 0, *pszIDCSAVE_save = 0;

static const UINT sendControls[]   = { IDC_MESSAGE, IDC_LOG };
static const UINT formatControls[] = { IDC_SMILEYBTN, IDC_FONTBOLD, IDC_FONTITALIC, IDC_FONTUNDERLINE, IDC_FONTFACE,IDC_FONTSTRIKEOUT };
static const UINT addControls[]    = { IDC_ADD, IDC_CANCELADD };
static const UINT btnControls[]    = { IDC_RETRY, IDC_CANCELSEND, IDC_MSGSENDLATER, IDC_ADD, IDC_CANCELADD };
static const UINT errorControls[]  = { IDC_STATICERRORICON, IDC_STATICTEXT, IDC_RETRY, IDC_CANCELSEND, IDC_MSGSENDLATER};

static struct {
	int id;
	const TCHAR* text;
}
tooltips[] =
{
	{ IDC_ADD, LPGENT("Add this contact permanently to your contact list") },
	{ IDC_CANCELADD, LPGENT("Do not add this contact permanently") },
	{ IDC_TOGGLESIDEBAR, LPGENT("Expand or collapse the side bar") }
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

BOOL TSAPI IsUtfSendAvailable(MCONTACT hContact)
{
	char *szProto = GetContactProto(hContact);
	if (szProto == NULL)
		return FALSE;

	return (CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_4, 0) & PF4_IMSENDUTF) ? TRUE : FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// show a modified context menu for the richedit control(s)
// @param dat			message window data
// @param idFrom		dlg ctrl id
// @param hwndFrom		src window handle
// @param pt			mouse pointer position

static void ShowPopupMenu(TWindowData *dat, int idFrom, HWND hwndFrom, POINT pt)
{
	CHARRANGE sel, all = { 0, -1};
	int  oldCodepage = dat->codePage;
	int  iPrivateBG = M.GetByte(dat->hContact, "private_bg", 0);
	HWND hwndDlg = dat->hwnd;

	HMENU hSubMenu, hMenu = LoadMenu(g_hInst, MAKEINTRESOURCE(IDR_CONTEXT));
	if (idFrom == IDC_LOG)
		hSubMenu = GetSubMenu(hMenu, 0);
	else {
		hSubMenu = GetSubMenu(hMenu, 2);
		EnableMenuItem(hSubMenu, IDM_PASTEFORMATTED, MF_BYCOMMAND | (dat->SendFormat != 0 ? MF_ENABLED : MF_GRAYED));
		EnableMenuItem(hSubMenu, ID_EDITOR_PASTEANDSENDIMMEDIATELY, MF_BYCOMMAND | (PluginConfig.m_PasteAndSend ? MF_ENABLED : MF_GRAYED));
		CheckMenuItem(hSubMenu, ID_EDITOR_SHOWMESSAGELENGTHINDICATOR, MF_BYCOMMAND | (PluginConfig.m_visualMessageSizeIndicator ? MF_CHECKED : MF_UNCHECKED));
		EnableMenuItem(hSubMenu, ID_EDITOR_SHOWMESSAGELENGTHINDICATOR, MF_BYCOMMAND | (dat->pContainer->hwndStatus ? MF_ENABLED : MF_GRAYED));
	}
	TranslateMenu(hSubMenu);
	SendMessage(hwndFrom, EM_EXGETSEL, 0, (LPARAM)&sel);
	if (sel.cpMin == sel.cpMax) {
		EnableMenuItem(hSubMenu, IDM_COPY, MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(hSubMenu, IDM_QUOTE, MF_BYCOMMAND | MF_GRAYED);
		if (idFrom == IDC_MESSAGE)
			EnableMenuItem(hSubMenu, IDM_CUT, MF_BYCOMMAND | MF_GRAYED);
	}

	if (idFrom == IDC_LOG)  {
		InsertMenuA(hSubMenu, 6/*5*/, MF_BYPOSITION | MF_SEPARATOR, 0, 0);
		InsertMenu(hSubMenu, 7/*6*/, MF_BYPOSITION | MF_POPUP, (UINT_PTR) PluginConfig.g_hMenuEncoding, TranslateT("Character Encoding"));
		for (int i=0; i < GetMenuItemCount(PluginConfig.g_hMenuEncoding); i++)
			CheckMenuItem(PluginConfig.g_hMenuEncoding, i, MF_BYPOSITION | MF_UNCHECKED);

		if (dat->codePage == CP_ACP)
			CheckMenuItem(PluginConfig.g_hMenuEncoding, 0, MF_BYPOSITION | MF_CHECKED);
		else
			CheckMenuItem(PluginConfig.g_hMenuEncoding, dat->codePage, MF_BYCOMMAND | MF_CHECKED);

		CheckMenuItem(hSubMenu, ID_LOG_FREEZELOG, MF_BYCOMMAND | (dat->dwFlagsEx & MWF_SHOW_SCROLLINGDISABLED ? MF_CHECKED : MF_UNCHECKED));
	}

	MessageWindowPopupData mwpd;
	if (idFrom == IDC_LOG || idFrom == IDC_MESSAGE) {
		// First notification
		mwpd.cbSize = sizeof(mwpd);
		mwpd.uType = MSG_WINDOWPOPUP_SHOWING;
		mwpd.uFlags = (idFrom == IDC_LOG ? MSG_WINDOWPOPUP_LOG : MSG_WINDOWPOPUP_INPUT);
		mwpd.hContact = dat->hContact;
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

	if (((iSelection > 800 && iSelection < 1400) || iSelection == 20866) && idFrom == IDC_LOG) {
		dat->codePage = iSelection;
		db_set_dw(dat->hContact, SRMSGMOD_T, "ANSIcodepage", dat->codePage);
	}
	else if (iSelection == 500 && idFrom == IDC_LOG) {
		dat->codePage = CP_ACP;
		db_unset(dat->hContact, SRMSGMOD_T, "ANSIcodepage");
	}
	else {
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
			ClearLog(dat);
			break;
		case ID_LOG_FREEZELOG:
			SendMessage(GetDlgItem(hwndDlg, IDC_LOG), WM_KEYDOWN, VK_F12, 0);
			break;
		case ID_EDITOR_SHOWMESSAGELENGTHINDICATOR:
			PluginConfig.m_visualMessageSizeIndicator = !PluginConfig.m_visualMessageSizeIndicator;
			db_set_b(0, SRMSGMOD_T, "msgsizebar", (BYTE)PluginConfig.m_visualMessageSizeIndicator);
			M.BroadcastMessage(DM_CONFIGURETOOLBAR, 0, 0);
			SendMessage(hwndDlg, WM_SIZE, 0, 0);
			if (dat->pContainer->hwndStatus)
				RedrawWindow(dat->pContainer->hwndStatus, 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);
			break;
		case ID_EDITOR_PASTEANDSENDIMMEDIATELY:
			HandlePasteAndSend(dat);
			break;
		}
	}
	if (idFrom == IDC_LOG)
		RemoveMenu(hSubMenu, 7, MF_BYPOSITION);
	DestroyMenu(hMenu);
	if (dat->codePage != oldCodepage) {
		SendMessage(hwndDlg, DM_REMAKELOG, 0, 0);
		SendMessage(hwndDlg, DM_UPDATETITLE, 0, 1);
	}
}

static void ResizeIeView(const TWindowData *dat, DWORD px, DWORD py, DWORD cx, DWORD cy)
{
	RECT rcRichEdit;
	GetWindowRect(GetDlgItem(dat->hwnd, IDC_LOG), &rcRichEdit);

	POINT pt = { rcRichEdit.left, rcRichEdit.top };
	ScreenToClient(dat->hwnd, &pt);

	IEVIEWWINDOW ieWindow = { sizeof(ieWindow) };
	ieWindow.iType = IEW_SETPOS;
	ieWindow.parent = dat->hwnd;
	ieWindow.hwnd = dat->hwndIEView ? dat->hwndIEView : dat->hwndHPP;
	ieWindow.x = pt.x;
	ieWindow.y = pt.y;
	ieWindow.cx = rcRichEdit.right - rcRichEdit.left;
	ieWindow.cy = rcRichEdit.bottom - rcRichEdit.top;
	if (ieWindow.cx != 0 && ieWindow.cy != 0)
		CallService(dat->hwndIEView ? MS_IEVIEW_WINDOW : MS_HPP_EG_WINDOW, 0, (LPARAM)&ieWindow);
}

LRESULT CALLBACK IEViewSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	TWindowData *mwdat = (TWindowData*)GetWindowLongPtr(GetParent(hwnd), GWLP_USERDATA);

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
	TWindowData *mwdat = (TWindowData*)GetWindowLongPtr(GetParent(hwnd), GWLP_USERDATA);
	if (mwdat) {
		BOOL isCtrl, isShift, isAlt;
		KbdState(mwdat, isShift, isCtrl, isAlt);

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
					 wParam != VK_RIGHT && wParam != VK_TAB  && wParam != VK_SPACE)
				{
					SetFocus(GetDlgItem(mwdat->hwnd, IDC_MESSAGE));
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

static void MsgWindowUpdateState(TWindowData *dat, UINT msg)
{
	if (dat && dat->iTabID >= 0) {
		HWND hwndDlg = dat->hwnd;
		HWND hwndTab = GetParent(hwndDlg);

		if (msg == WM_ACTIVATE) {
			if (dat->pContainer->dwFlags & CNT_TRANSPARENCY) {
				DWORD trans = LOWORD(dat->pContainer->settings->dwTransparency);
				SetLayeredWindowAttributes(dat->pContainer->hwnd, 0, (BYTE)trans, (dat->pContainer->dwFlags & CNT_TRANSPARENCY ? LWA_ALPHA : 0));
			}
		}

		if (dat->bIsAutosizingInput && dat->iInputAreaHeight == -1) {
			dat->iInputAreaHeight = 0;
			SendDlgItemMessage(hwndDlg, IDC_MESSAGE, EM_REQUESTRESIZE, 0, 0);
		}

		if (dat->pWnd)
			dat->pWnd->activateTab();
		dat->Panel->dismissConfig();
		dat->dwUnread = 0;
		if (dat->pContainer->hwndSaved == hwndDlg)
			return;

		dat->pContainer->hwndSaved = hwndDlg;

		dat->dwTickLastEvent = 0;
		dat->dwFlags &= ~MWF_DIVIDERSET;
		if (KillTimer(hwndDlg, TIMERID_FLASHWND)) {
			FlashTab(dat, hwndTab, dat->iTabID, &dat->bTabFlash, FALSE, dat->hTabIcon);
			dat->mayFlashTab = FALSE;
		}
		if (dat->pContainer->dwFlashingStarted != 0) {
			FlashContainer(dat->pContainer, 0, 0);
			dat->pContainer->dwFlashingStarted = 0;
		}
		if (dat->dwFlagsEx & MWF_SHOW_FLASHCLIST) {
			dat->dwFlagsEx &= ~MWF_SHOW_FLASHCLIST;
			if (dat->hFlashingEvent != 0)
				CallService(MS_CLIST_REMOVEEVENT, dat->hContact, (LPARAM)dat->hFlashingEvent);
			dat->hFlashingEvent = 0;
		}
		dat->pContainer->dwFlags &= ~CNT_NEED_UPDATETITLE;

		if (dat->dwFlags & MWF_DEFERREDREMAKELOG) {
			SendMessage(hwndDlg, DM_REMAKELOG, 0, 0);
			dat->dwFlags &= ~MWF_DEFERREDREMAKELOG;
		}

		if (dat->dwFlags & MWF_NEEDCHECKSIZE)
			PostMessage(hwndDlg, DM_SAVESIZE, 0, 0);

		if (PluginConfig.m_AutoLocaleSupport) {
			if (dat->hkl == 0)
				DM_LoadLocale(dat);
			else
				SendMessage(hwndDlg, DM_SETLOCALE, 0, 0);
		}

		dat->pContainer->hIconTaskbarOverlay = 0;
		SendMessage(dat->pContainer->hwnd, DM_UPDATETITLE, dat->hContact, 0);

		UpdateStatusBar(dat);
		dat->dwLastActivity = GetTickCount();
		dat->pContainer->dwLastActivity = dat->dwLastActivity;

		dat->pContainer->MenuBar->configureMenu();
		UpdateTrayMenuState(dat, FALSE);

		if (dat->pContainer->hwndActive == hwndDlg)
			PostMessage(hwndDlg, DM_REMOVEPOPUPS, PU_REMOVE_ON_FOCUS, 0);

		dat->Panel->Invalidate();

		if (dat->dwFlags & MWF_DEFERREDSCROLL && dat->hwndIEView == 0 && dat->hwndHPP == 0) {
			HWND hwnd = GetDlgItem(hwndDlg, IDC_LOG);

			SendMessage(hwnd, WM_SETREDRAW, FALSE, 0);
			dat->dwFlags &= ~MWF_DEFERREDSCROLL;
			SendMessage(hwnd, WM_VSCROLL, MAKEWPARAM(SB_TOP, 0), 0);
			SendMessage(hwnd, WM_SETREDRAW, TRUE, 0);
			DM_ScrollToBottom(dat, 0, 1);
			PostMessage(hwnd, WM_VSCROLL, MAKEWPARAM(SB_PAGEDOWN, 0), 0);    // XXX was post()
		}
		DM_SetDBButtonStates(hwndDlg, dat);

		if (dat->hwndIEView) {
			RECT rcRTF;
			POINT pt;

			GetWindowRect(GetDlgItem(hwndDlg, IDC_LOG), &rcRTF);
			rcRTF.left += 20;
			rcRTF.top += 20;
			pt.x = rcRTF.left;
			pt.y = rcRTF.top;
			if (dat->hwndIEView) {
				if (M.GetByte("subclassIEView", 0)) {
					mir_subclassWindow(dat->hwndIEView, IEViewSubclassProc);
					SetWindowPos(dat->hwndIEView, 0, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_DRAWFRAME);
					RedrawWindow(dat->hwndIEView, 0, 0, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW);
				}
			}
			dat->hwndIWebBrowserControl = WindowFromPoint(pt);
		}

		if (dat->dwFlagsEx & MWF_EX_DELAYEDSPLITTER) {
			dat->dwFlagsEx &= ~MWF_EX_DELAYEDSPLITTER;
			ShowWindow(dat->pContainer->hwnd, SW_RESTORE);
			PostMessage(hwndDlg, DM_SPLITTERGLOBALEVENT, dat->wParam, dat->lParam);
			dat->wParam = dat->lParam = 0;
		}
		if (dat->dwFlagsEx & MWF_EX_AVATARCHANGED) {
			dat->dwFlagsEx &= ~MWF_EX_AVATARCHANGED;
			PostMessage(hwndDlg, DM_UPDATEPICLAYOUT, 0, 0);
		}
		BB_SetButtonsPos(dat);
		if (M.isAero())
			InvalidateRect(hwndTab, NULL, FALSE);
		if (dat->pContainer->dwFlags & CNT_SIDEBAR)
			dat->pContainer->SideBar->setActiveItem(dat);

		if (dat->pWnd)
			dat->pWnd->Invalidate();
	}
}

void TSAPI ShowMultipleControls(HWND hwndDlg, const UINT *controls, int cControls, int state)
{
	for (int i = 0; i < cControls; i++)
		Utils::showDlgControl(hwndDlg, controls[i], state);
}

void TSAPI SetDialogToType(HWND hwndDlg)
{
	TWindowData *dat = (TWindowData*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	int showToolbar = dat->pContainer->dwFlags & CNT_HIDETOOLBAR ? 0 : 1;

	if (dat->hContact) {
		if (db_get_b(dat->hContact, "CList", "NotOnList", 0)) {
			dat->bNotOnList = TRUE;
			ShowMultipleControls(hwndDlg, addControls, 2, SW_SHOW);
			Utils::showDlgControl(hwndDlg, IDC_LOGFROZENTEXT, SW_SHOW);
			SetWindowText(GetDlgItem(hwndDlg, IDC_LOGFROZENTEXT), TranslateT("Contact not on list. You may add it..."));
		}
		else {
			ShowMultipleControls(hwndDlg, addControls, 2, SW_HIDE);
			dat->bNotOnList = FALSE;
			Utils::showDlgControl(hwndDlg, IDC_LOGFROZENTEXT, SW_HIDE);
		}
	}

	Utils::enableDlgControl(hwndDlg, IDC_TIME, TRUE);

	if (dat->hwndIEView || dat->hwndHPP) {
		Utils::showDlgControl(hwndDlg, IDC_LOG, SW_HIDE);
		Utils::enableDlgControl(hwndDlg, IDC_LOG, FALSE);
		Utils::showDlgControl(hwndDlg, IDC_MESSAGE, SW_SHOW);
	}
	else ShowMultipleControls(hwndDlg, sendControls, sizeof(sendControls) / sizeof(sendControls[0]), SW_SHOW);

	ShowMultipleControls(hwndDlg, errorControls, sizeof(errorControls) / sizeof(errorControls[0]), dat->dwFlags & MWF_ERRORSTATE ? SW_SHOW : SW_HIDE);

	if (!dat->SendFormat)
		ShowMultipleControls(hwndDlg, &formatControls[1], 5, SW_HIDE);

	ConfigureSmileyButton(dat);

	if (dat->pContainer->hwndActive == hwndDlg)
		UpdateReadChars(dat);

	SetDlgItemText(hwndDlg, IDC_STATICTEXT, TranslateT("A message failed to send successfully."));

	DM_RecalcPictureSize(dat);
	GetAvatarVisibility(hwndDlg, dat);

	Utils::showDlgControl(hwndDlg, IDC_CONTACTPIC, dat->bShowAvatar ? SW_SHOW : SW_HIDE);
	Utils::showDlgControl(hwndDlg, IDC_SPLITTER, dat->bIsAutosizingInput ? SW_HIDE : SW_SHOW);
	Utils::showDlgControl(hwndDlg, IDC_MULTISPLITTER, (dat->sendMode & SMODE_MULTIPLE) ? SW_SHOW : SW_HIDE);

	EnableSendButton(dat, GetWindowTextLength(GetDlgItem(hwndDlg, IDC_MESSAGE)) != 0);
	SendMessage(hwndDlg, DM_UPDATETITLE, 0, 1);
	SendMessage(hwndDlg, WM_SIZE, 0, 0);

	Utils::enableDlgControl(hwndDlg, IDC_CONTACTPIC, FALSE);

	dat->Panel->Configure();
}

static LRESULT CALLBACK MessageLogSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HWND hwndParent = GetParent(hwnd);
	TWindowData *mwdat = (TWindowData*)GetWindowLongPtr(hwndParent, GWLP_USERDATA);
	BOOL isCtrl, isShift, isAlt;
	KbdState(mwdat, isShift, isCtrl, isAlt);

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
			return DM_WMCopyHandler(hwnd, MessageLogSubclassProc, msg, wParam, lParam);
		if (wParam == 0x11 && isCtrl)
			SendMessage(mwdat->hwnd, WM_COMMAND, IDC_QUOTE, 0);
		break;

	case WM_SYSKEYUP:
		if (wParam == VK_MENU) {
			ProcessHotkeysByMsgFilter(hwnd, msg, wParam, lParam, IDC_LOG);
			return 0;
		}
		break;

	case WM_SYSKEYDOWN:
		mwdat->fkeyProcessed = false;
		if (ProcessHotkeysByMsgFilter(hwnd, msg, wParam, lParam, IDC_LOG)) {
			mwdat->fkeyProcessed = true;
			return 0;
		}
		break;

	case WM_SYSCHAR:
		if (mwdat->fkeyProcessed) {
			mwdat->fkeyProcessed = false;
			return 0;
		}
		break;

	case WM_KEYDOWN:
		if (wParam == VK_INSERT && isCtrl)
			return DM_WMCopyHandler(hwnd, MessageLogSubclassProc, msg, wParam, lParam);
		break;

	case WM_COPY:
		return DM_WMCopyHandler(hwnd, MessageLogSubclassProc, msg, wParam, lParam);

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
			pt.x = (short)LOWORD(lParam);
			pt.y = (short)HIWORD(lParam);
		}

		ShowPopupMenu(mwdat, IDC_LOG, hwnd, pt);
		return TRUE;
	}

	return mir_callNextSubclass(hwnd, MessageLogSubclassProc, msg, wParam, lParam);
}

static LRESULT CALLBACK MessageEditSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HWND hwndParent = GetParent(hwnd);
	TWindowData *mwdat = (TWindowData*)GetWindowLongPtr(hwndParent, GWLP_USERDATA);
	if (mwdat == NULL)
		return 0;

	// prevent the rich edit from switching text direction or keyboard layout when
	// using hotkeys with ctrl-shift or alt-shift modifiers
	if (mwdat->fkeyProcessed && (msg == WM_KEYUP)) {
		GetKeyboardState(mwdat->kstate);
		if (mwdat->kstate[VK_CONTROL] & 0x80 || mwdat->kstate[VK_SHIFT] & 0x80)
			return 0;

		mwdat->fkeyProcessed = false;
		return 0;
	}

	switch (msg) {
	case WM_NCCALCSIZE:
		return CSkin::NcCalcRichEditFrame(hwnd, mwdat, ID_EXTBKINPUTAREA, msg, wParam, lParam, MessageEditSubclassProc);

	case WM_NCPAINT:
		return CSkin::DrawRichEditFrame(hwnd, mwdat, ID_EXTBKINPUTAREA, msg, wParam, lParam, MessageEditSubclassProc);

	case WM_DROPFILES:
		SendMessage(hwndParent, WM_DROPFILES, (WPARAM)wParam, (LPARAM)lParam);
		break;

	case WM_CHAR:
		{
			BOOL isCtrl, isShift, isAlt;
			KbdState(mwdat, isShift, isCtrl, isAlt);

			if (PluginConfig.g_bSoundOnTyping && !isAlt && !isCtrl && !(mwdat->pContainer->dwFlags & CNT_NOSOUND) && wParam != VK_ESCAPE && !(wParam == VK_TAB && PluginConfig.m_AllowTab))
				SkinPlaySound("SoundOnTyping");

			if (isCtrl && !isAlt) {
				switch (wParam) {
				case 0x02:               // bold
					if (mwdat->SendFormat)
						SendMessage(hwndParent, WM_COMMAND, MAKELONG(IDC_FONTBOLD, IDC_MESSAGE), 0);
					return 0;
				case 0x09:
					if (mwdat->SendFormat)
						SendMessage(hwndParent, WM_COMMAND, MAKELONG(IDC_FONTITALIC, IDC_MESSAGE), 0);
					return 0;
				case 21:
					if (mwdat->SendFormat)
						SendMessage(hwndParent, WM_COMMAND, MAKELONG(IDC_FONTUNDERLINE, IDC_MESSAGE), 0);
					return 0;
				case 0x0b:
					SetWindowText(hwnd, _T(""));
					return 0;
				}
				break;
			}
		}
		break;

	case WM_MOUSEWHEEL:
		if (DM_MouseWheelHandler(hwnd, hwndParent, mwdat, wParam, lParam) == 0)
			return 0;
		break;

	case WM_PASTE:
	case EM_PASTESPECIAL:
		if (OpenClipboard(hwnd)) {
			HANDLE hClip;
			if (hClip = GetClipboardData(CF_TEXT)) {
				if (lstrlenA((char *)hClip) > mwdat->nMax) {
					TCHAR szBuffer[512];
					if (M.GetByte("autosplit", 0))
						mir_sntprintf(szBuffer, SIZEOF(szBuffer), TranslateT("WARNING: The message you are trying to paste exceeds the message size limit for the active protocol. It will be sent in chunks of max %d characters"), mwdat->nMax - 10);
					else
						mir_sntprintf(szBuffer, SIZEOF(szBuffer), TranslateT("The message you are trying to paste exceeds the message size limit for the active protocol. Only the first %d characters will be sent."), mwdat->nMax);
					SendMessage(hwndParent, DM_ACTIVATETOOLTIP, IDC_MESSAGE, (LPARAM)szBuffer);
				}
			}
			else if (hClip = GetClipboardData(CF_BITMAP))
				SendHBitmapAsFile(mwdat, (HBITMAP)hClip);

			CloseClipboard();
		}
		break;

	case WM_KEYDOWN:
		{
			BOOL isCtrl, isShift, isAlt;
			KbdState(mwdat, isShift, isCtrl, isAlt);

			if (PluginConfig.g_bSoundOnTyping && !isAlt && !(mwdat->pContainer->dwFlags & CNT_NOSOUND) && wParam == VK_DELETE)
				SkinPlaySound("SoundOnTyping");

			if (wParam == VK_INSERT && !isShift && !isCtrl && !isAlt) {
				mwdat->fInsertMode = !mwdat->fInsertMode;
				SendMessage(hwndParent, WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(hwnd), EN_CHANGE), (LPARAM)hwnd);
			}
			if (wParam == VK_CAPITAL || wParam == VK_NUMLOCK)
				SendMessage(hwndParent, WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(hwnd), EN_CHANGE), (LPARAM)hwnd);

			if (wParam == VK_RETURN) {
				if (mwdat->fEditNotesActive)
					break;

				if (isShift) {
					if (PluginConfig.m_SendOnShiftEnter) {
						PostMessage(hwndParent, WM_COMMAND, IDOK, 0);
						return 0;
					}
					else break;
				}
				if ((isCtrl && !isShift) ^ (0 != PluginConfig.m_SendOnEnter)) {
					PostMessage(hwndParent, WM_COMMAND, IDOK, 0);
					return 0;
				}
				if (PluginConfig.m_SendOnEnter || PluginConfig.m_SendOnDblEnter) {
					if (isCtrl)
						break;

					if (PluginConfig.m_SendOnDblEnter) {
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
						mwdat->cache->inputHistoryEvent(wParam);
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
						DM_ScrollToBottom(mwdat, 0, 0);
						return 0;
					}
					else if (wParam == VK_DOWN)
						wp = MAKEWPARAM(SB_LINEDOWN, 0);

					if (mwdat->hwndIEView == 0 && mwdat->hwndHPP == 0)
						SendMessage(GetDlgItem(hwndParent, IDC_LOG), WM_VSCROLL, wp, 0);
					else
						SendMessage(mwdat->hwndIWebBrowserControl, WM_VSCROLL, wp, 0);
					return 0;
				}
			}
			if (wParam == VK_RETURN)
				break;
		}

	case WM_SYSKEYDOWN:
		mwdat->fkeyProcessed = false;
		if (ProcessHotkeysByMsgFilter(hwnd, msg, wParam, lParam, IDC_MESSAGE)) {
			mwdat->fkeyProcessed = true;
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
		if (mwdat->fkeyProcessed) {
			mwdat->fkeyProcessed = false;
			return 0;
		}
		{
			HWND hwndDlg = hwndParent;
			BOOL isCtrl, isShift, isAlt;

			KbdState(mwdat, isShift, isCtrl, isAlt);
			if ((wParam >= '0' && wParam <= '9') && isAlt) {      // ALT-1 -> ALT-0 direct tab selection
				BYTE bChar = (BYTE)wParam;
				int iIndex;

				if (bChar == '0')
					iIndex = 10;
				else
					iIndex = bChar - (BYTE)'0';
				SendMessage(mwdat->pContainer->hwnd, DM_SELECTTAB, DM_SELECT_BY_INDEX, (LPARAM)iIndex);
				return 0;
			}
		}
		break;

	case WM_INPUTLANGCHANGE:
		if (PluginConfig.m_AutoLocaleSupport && GetFocus() == hwnd && mwdat->pContainer->hwndActive == hwndParent && GetForegroundWindow() == mwdat->pContainer->hwnd && GetActiveWindow() == mwdat->pContainer->hwnd) {
			DM_SaveLocale(mwdat, wParam, lParam);
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
			pt.x = (short)LOWORD(lParam);
			pt.y = (short)HIWORD(lParam);
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
	}
	return mir_callNextSubclass(hwnd, AvatarSubclassProc, msg, wParam, lParam);
}

LRESULT CALLBACK SplitterSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	RECT rc;
	HWND hwndParent = GetParent(hwnd);
	TWindowData *dat = (TWindowData*)GetWindowLongPtr(hwndParent, GWLP_USERDATA);

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
				dat->savedSplitter = rc.right > rc.bottom ? (short)HIWORD(GetMessagePos()) + rc.bottom / 2 : (short)LOWORD(GetMessagePos()) + rc.right / 2;
				if (dat->bType == SESSIONTYPE_IM)
					dat->savedSplitY = dat->splitterY;
				else
					dat->savedSplitY = dat->si->iSplitterY;

				dat->savedDynaSplit = dat->dynaSplitter;
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
				CSkin::SkinDrawBG(hwnd, dat->pContainer->hwnd, dat->pContainer, &rc, dc);
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
		DM_ScrollToBottom(dat, 0, 1);
		if (dat && dat->bType == SESSIONTYPE_IM && hwnd == GetDlgItem(hwndParent, IDC_PANELSPLITTER)) {
			SendMessage(hwndParent, WM_SIZE, 0, 0);
			dat->panelWidth = -1;
			RedrawWindow(hwndParent, NULL, NULL, RDW_ALLCHILDREN | RDW_INVALIDATE | RDW_UPDATENOW);
		}
		else if ((dat && dat->bType == SESSIONTYPE_IM && hwnd == GetDlgItem(hwndParent, IDC_SPLITTER)) || 
			      (dat && dat->bType == SESSIONTYPE_CHAT && hwnd == GetDlgItem(hwndParent, IDC_SPLITTERY))) {
			POINT pt;
			int selection;
			HMENU hMenu = GetSubMenu(dat->pContainer->hMenuContext, 12);
			LONG messagePos = GetMessagePos();

			GetClientRect(hwnd, &rc);
			if (hwndCapture != hwnd || dat->savedSplitter == (rc.right > rc.bottom ? (short) HIWORD(messagePos) + rc.bottom / 2 : (short) LOWORD(messagePos) + rc.right / 2))
				break;
			GetCursorPos(&pt);

			if (dat->bIsAutosizingInput)
				selection = ID_SPLITTERCONTEXT_SETPOSITIONFORTHISSESSION;
			else
				selection = TrackPopupMenu(hMenu, TPM_RETURNCMD, pt.x, pt.y, 0, hwndParent, NULL);

			switch (selection) {
			case ID_SPLITTERCONTEXT_SAVEFORTHISCONTACTONLY:
				dat->dwFlagsEx |= MWF_SHOW_SPLITTEROVERRIDE;
				db_set_b(dat->hContact, SRMSGMOD_T, "splitoverride", 1);
				if (dat->bType == SESSIONTYPE_IM)
					SaveSplitter(dat);
				break;

			case ID_SPLITTERCONTEXT_SETPOSITIONFORTHISSESSION:
				if (dat->bIsAutosizingInput) {
					GetWindowRect(GetDlgItem(dat->hwnd, IDC_MESSAGE), &rc);
					dat->iInputAreaHeight = 0;
				}
				break;

			case ID_SPLITTERCONTEXT_SAVEGLOBALFORALLSESSIONS:
				{
					RECT rcWin;
					BYTE bSync = M.GetByte(CHAT_MODULE, "SyncSplitter", 0);
					DWORD dwOff_IM = 0, dwOff_CHAT = 0;

					dwOff_CHAT = -(2 + (PluginConfig.g_DPIscaleY > 1.0 ? 1 : 0));
					dwOff_IM = 2 + (PluginConfig.g_DPIscaleY > 1.0 ? 1 : 0);

					GetWindowRect(hwndParent, &rcWin);
					PluginConfig.lastSPlitterPos.pSrcDat = dat;
					PluginConfig.lastSPlitterPos.pSrcContainer = dat->pContainer;
					PluginConfig.lastSPlitterPos.lParam = rc.bottom;
					PluginConfig.lastSPlitterPos.pos = rcWin.bottom - HIWORD(messagePos);
					PluginConfig.lastSPlitterPos.pos_chat = rcWin.bottom - (short)HIWORD(messagePos) + rc.bottom / 2;
					PluginConfig.lastSPlitterPos.off_chat = dwOff_CHAT;
					PluginConfig.lastSPlitterPos.off_im = dwOff_IM;
					PluginConfig.lastSPlitterPos.bSync = bSync;
					SendMessage(dat->hwnd, DM_SPLITTERGLOBALEVENT, 0, 0);
					M.BroadcastMessage(DM_SPLITTERGLOBALEVENT, 0, 0);
				}
				break;

			default:
				dat->splitterY = dat->savedSplitY;
				dat->dynaSplitter = dat->savedDynaSplit;
				DM_RecalcPictureSize(dat);
				if (dat->bType == SESSIONTYPE_CHAT) {
					SESSION_INFO *si = dat->si;
					si->iSplitterY = dat->savedSplitY;
					dat->splitterY = si->iSplitterY + DPISCALEY_S(22);
				}
				CSkin::UpdateToolbarBG(dat);
				SendMessage(hwndParent, WM_SIZE, 0, 0);
				DM_ScrollToBottom(dat, 0, 1);
				break;
			}
		}
		return 0;
	}
	return mir_callNextSubclass(hwnd, SplitterSubclassProc, msg, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////////////////
// resizer proc for the "new" layout.

static int MessageDialogResize(HWND hwndDlg, LPARAM lParam, UTILRESIZECONTROL * urc)
{
	TWindowData *dat = (TWindowData*)lParam;
	RECT rc, rcButton;
	static int uinWidth, msgTop = 0, msgBottom = 0;

	int showToolbar = dat->pContainer->dwFlags & CNT_HIDETOOLBAR ? 0 : 1;
	BOOL bBottomToolbar = dat->pContainer->dwFlags & CNT_BOTTOMTOOLBAR ? 1 : 0;
	static LONG rcLogBottom;

	int 	panelHeight = dat->Panel->getHeight() + 1;
	int 	s_offset = 0;
	bool 	bInfoPanel = dat->Panel->isActive();
	bool	fErrorState = (dat->dwFlags & MWF_ERRORSTATE) ? true : false;

	GetClientRect(GetDlgItem(hwndDlg, IDC_LOG), &rc);
	GetClientRect(GetDlgItem(hwndDlg, IDC_PROTOCOL), &rcButton);

	if (dat->panelStatusCX == 0)
		dat->panelStatusCX = 80;

	s_offset = 1;

	switch (urc->wId) {
	case IDC_PANELSPLITTER:
		urc->rcItem.bottom = panelHeight;
		urc->rcItem.top = panelHeight - 2;
		return RD_ANCHORX_WIDTH | RD_ANCHORY_TOP;
	case IDC_LOG:
		if (dat->dwFlags & MWF_ERRORSTATE)
			urc->rcItem.bottom -= ERRORPANEL_HEIGHT;
		if (dat->dwFlagsEx & MWF_SHOW_SCROLLINGDISABLED || dat->bNotOnList)
			urc->rcItem.bottom -= 20;
		if (dat->sendMode & SMODE_MULTIPLE)
			urc->rcItem.right -= (dat->multiSplitterX + 3);
		urc->rcItem.bottom -= dat->splitterY - dat->originalSplitterY;
		if (!showToolbar||bBottomToolbar)
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
		GetClientRect(GetDlgItem(hwndDlg, IDC_MESSAGE), &rc);
		urc->rcItem.top -= dat->splitterY - dat->originalSplitterY;
		urc->rcItem.left = urc->rcItem.right - (dat->pic.cx + 2);
		if ((urc->rcItem.bottom - urc->rcItem.top) < (dat->pic.cy/* + 2*/) && dat->bShowAvatar) {
			urc->rcItem.top = urc->rcItem.bottom - dat->pic.cy;
			dat->fMustOffset = true;
		}
		else dat->fMustOffset = false;

		if (showToolbar && bBottomToolbar && (PluginConfig.m_AlwaysFullToolbarWidth || ((dat->pic.cy - DPISCALEY_S(6)) < rc.bottom))) {
			urc->rcItem.bottom -= DPISCALEY_S(22);
			if (dat->bIsAutosizingInput) {
				urc->rcItem.left--;
				urc->rcItem.top--;
			}
		}

		if (dat->hwndContactPic) //if Panel control was created?
			SetWindowPos(dat->hwndContactPic, HWND_TOP, 1, ((urc->rcItem.bottom - urc->rcItem.top) - (dat->pic.cy)) / 2 + 1,  //resizes it
				dat->pic.cx - 2, dat->pic.cy - 2, SWP_SHOWWINDOW);

		return RD_ANCHORX_RIGHT | RD_ANCHORY_BOTTOM;

	case IDC_SPLITTER:
		urc->rcItem.right = urc->dlgNewSize.cx;
		urc->rcItem.top -= dat->splitterY - dat->originalSplitterY;
		urc->rcItem.bottom = urc->rcItem.top + 2;
		OffsetRect(&urc->rcItem, 0, 1);
		urc->rcItem.left = 0;

		if (dat->fMustOffset)
			urc->rcItem.right -= (dat->pic.cx); // + DPISCALEX(2));
		return RD_ANCHORX_CUSTOM | RD_ANCHORY_BOTTOM;

	case IDC_MESSAGE:
		urc->rcItem.right = urc->dlgNewSize.cx;
		if (dat->bShowAvatar)
			urc->rcItem.right -= dat->pic.cx + 2;
		urc->rcItem.top -= dat->splitterY - dat->originalSplitterY;
		if (bBottomToolbar&&showToolbar)
			urc->rcItem.bottom -= DPISCALEY_S(22);

		if (dat->bIsAutosizingInput)
			urc->rcItem.top -= DPISCALEY_S(1);

		msgTop = urc->rcItem.top;
		msgBottom = urc->rcItem.bottom;
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
		urc->rcItem.left -= dat->multiSplitterX;
		urc->rcItem.right -= dat->multiSplitterX;
		urc->rcItem.bottom = rcLogBottom;
		return RD_ANCHORX_RIGHT | RD_ANCHORY_HEIGHT;

	case IDC_LOGFROZENTEXT:
		urc->rcItem.right = urc->dlgNewSize.cx - 50;
		urc->rcItem.bottom = msgTop - (bBottomToolbar ? 0 : 28);
		urc->rcItem.top = msgTop - 16 - (bBottomToolbar ? 0 : 28);
		if (!showToolbar && !bBottomToolbar) {
			urc->rcItem.bottom += 21;
			urc->rcItem.top += 21;
		}
		return RD_ANCHORX_CUSTOM | RD_ANCHORY_BOTTOM;

	case IDC_ADD:
		urc->rcItem.bottom = msgTop - (bBottomToolbar ? 0 : 28);
		urc->rcItem.top = msgTop - 18 - (bBottomToolbar ? 0 : 28);
		urc->rcItem.right = urc->dlgNewSize.cx - 28;
		urc->rcItem.left = urc->rcItem.right - 20;
		if (!showToolbar && !bBottomToolbar) {
			urc->rcItem.bottom += 21;
			urc->rcItem.top += 21;
		}
		return RD_ANCHORX_CUSTOM | RD_ANCHORY_BOTTOM;

	case IDC_CANCELADD:
		urc->rcItem.bottom = msgTop - (bBottomToolbar ? 0 : 28);
		urc->rcItem.top = msgTop - 18 - (bBottomToolbar ? 0 : 28);
		urc->rcItem.right = urc->dlgNewSize.cx - 4;
		urc->rcItem.left = urc->rcItem.right - 20;
		if (!showToolbar && !bBottomToolbar) {
			urc->rcItem.bottom += 21;
			urc->rcItem.top += 21;
		}
		return RD_ANCHORX_CUSTOM | RD_ANCHORY_BOTTOM;

	case IDC_TOGGLESIDEBAR:
		return RD_ANCHORX_CUSTOM | RD_ANCHORY_CUSTOM;

	case IDC_RETRY:
	case IDC_CANCELSEND:
	case IDC_MSGSENDLATER:
		if (fErrorState) {
			urc->rcItem.bottom = msgTop - 5 - (bBottomToolbar ? 0 : 28) - ((dat->bNotOnList || dat->dwFlagsEx & MWF_SHOW_SCROLLINGDISABLED) ? 20 : 0);
			urc->rcItem.top = msgTop - 25 - (bBottomToolbar ? 0 : 28) - ((dat->bNotOnList || dat->dwFlagsEx & MWF_SHOW_SCROLLINGDISABLED) ? 20 : 0);
		}
		if (!showToolbar && !bBottomToolbar) {
			urc->rcItem.bottom += 21;
			urc->rcItem.top += 21;
		}
		return RD_ANCHORX_LEFT | RD_ANCHORY_BOTTOM;

	case IDC_STATICTEXT:
	case IDC_STATICERRORICON:
		if (fErrorState) {
			urc->rcItem.bottom = msgTop - 28 - (bBottomToolbar ? 0 : 28) - ((dat->bNotOnList || dat->dwFlagsEx & MWF_SHOW_SCROLLINGDISABLED) ? 20 : 0);
			urc->rcItem.top = msgTop - 45 - (bBottomToolbar ? 0 : 28) - ((dat->bNotOnList || dat->dwFlagsEx & MWF_SHOW_SCROLLINGDISABLED) ? 20 : 0);
		}
		if (!showToolbar && !bBottomToolbar) {
			urc->rcItem.bottom += 21;
			urc->rcItem.top += 21;
		}
		return RD_ANCHORX_LEFT | RD_ANCHORY_BOTTOM;
	}
	return RD_ANCHORX_LEFT | RD_ANCHORY_BOTTOM;
}

INT_PTR CALLBACK DlgProcMessage(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	TContainerData *m_pContainer = 0;
	TWindowData *dat = (TWindowData*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	HWND hwndTab = GetParent(hwndDlg), hwndContainer;
	POINT pt, tmp, cur;

	if (dat == 0) {
		if (msg == WM_ACTIVATE || msg == WM_SETFOCUS)
			return 0;
	}
	else {
		m_pContainer = dat->pContainer;
		hwndContainer = m_pContainer->hwnd;
	}

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{
			TNewWindowData *newData = (TNewWindowData*)lParam;

			dat = (TWindowData*)mir_calloc(sizeof(TWindowData));
			if (newData->iTabID >= 0) {
				dat->pContainer = newData->pContainer;
				m_pContainer = dat->pContainer;
				hwndContainer = m_pContainer->hwnd;
			}
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)dat);

			if (Utils::rtf_ctable == 0)
				Utils::RTF_CTableInit();

			dat->dwFlags |= MWF_INITMODE;
			dat->bType = SESSIONTYPE_IM;
			dat->fInsertMode = FALSE;
			dat->fLimitedUpdate = false;
			dat->Panel = new CInfoPanel(dat);

			newData->item.lParam = (LPARAM)hwndDlg;
			TabCtrl_SetItem(hwndTab, newData->iTabID, &newData->item);
			dat->iTabID = newData->iTabID;
			dat->hwnd = hwndDlg;

			DM_ThemeChanged(dat);

			pszIDCSAVE_close = TranslateT("Close Session");
			pszIDCSAVE_save = TranslateT("Save and close session");

			dat->hContact = newData->hContact;
			dat->cache = CContactCache::getContactCache(dat->hContact);
			dat->cache->updateState();
			dat->cache->setWindowData(hwndDlg, dat);
			M.AddWindow(hwndDlg, dat->hContact);
			BroadCastContainer(m_pContainer, DM_REFRESHTABINDEX, 0, 0);
			dat->pWnd = 0;
			dat->sbCustom = 0;
			CProxyWindow::add(dat);
			dat->szProto = const_cast<char *>(dat->cache->getProto());
			dat->bIsMeta = dat->cache->isMeta();
			if (dat->bIsMeta)
				dat->cache->updateMeta(true);
			dat->cache->updateUIN();

			if (dat->hContact && dat->szProto != NULL) {
				dat->wStatus = db_get_w(dat->hContact, dat->szProto, "Status", ID_STATUS_OFFLINE);
				_tcsncpy_s(dat->szStatus, pcli->pfnGetStatusModeDescription(dat->szProto == NULL ? ID_STATUS_OFFLINE : dat->wStatus, 0), _TRUNCATE);
			}
			else dat->wStatus = ID_STATUS_OFFLINE;

			for (int i = 0; i < SIZEOF(btnControls); i++)
				CustomizeButton(GetDlgItem(hwndDlg, btnControls[i]));

			GetMYUIN(dat);
			GetClientIcon(dat);

			CustomizeButton(CreateWindowEx(0, _T("MButtonClass"), _T(""), WS_CHILD | WS_VISIBLE | WS_TABSTOP, 0, 0, 6, DPISCALEY_S(20),
				hwndDlg, (HMENU)IDC_TOGGLESIDEBAR, g_hInst, NULL));
			dat->hwndPanelPicParent = CreateWindowEx(WS_EX_TOPMOST, _T("Static"), _T(""), SS_OWNERDRAW | WS_VISIBLE | WS_CHILD, 1, 1, 1, 1, hwndDlg, (HMENU)6000, NULL, NULL);
			mir_subclassWindow(dat->hwndPanelPicParent, CInfoPanel::avatarParentSubclass);

			dat->showUIElements = m_pContainer->dwFlags & CNT_HIDETOOLBAR ? 0 : 1;
			dat->sendMode |= M.GetByte(dat->hContact, "forceansi", 0) ? SMODE_FORCEANSI : 0;
			dat->sendMode |= dat->hContact == 0 ? SMODE_MULTIPLE : 0;
			dat->sendMode |= M.GetByte(dat->hContact, "no_ack", 0) ? SMODE_NOACK : 0;

			dat->hQueuedEvents = (HANDLE*)mir_calloc(sizeof(HANDLE)* EVENT_QUEUE_SIZE);
			dat->iEventQueueSize = EVENT_QUEUE_SIZE;
			dat->iCurrentQueueError = -1;

			// message history limit
			// hHistoryEvents holds up to n event handles
			dat->maxHistory = M.GetDword(dat->hContact, "maxhist", M.GetDword("maxhist", 0));
			dat->curHistory = 0;
			if (dat->maxHistory)
				dat->hHistoryEvents = (HANDLE*)mir_alloc(dat->maxHistory * sizeof(HANDLE));
			else
				dat->hHistoryEvents = NULL;

			if (dat->bIsMeta)
				SendMessage(hwndDlg, DM_UPDATEMETACONTACTINFO, 0, 0);
			else
				SendMessage(hwndDlg, DM_UPDATEWINICON, 0, 0);
			dat->bTabFlash = FALSE;
			dat->mayFlashTab = FALSE;
			GetMyNick(dat);

			dat->multiSplitterX = (int)M.GetDword(SRMSGMOD, "multisplit", 150);
			dat->nTypeMode = PROTOTYPE_SELFTYPING_OFF;
			SetTimer(hwndDlg, TIMERID_TYPE, 1000, NULL);
			dat->iLastEventType = 0xffffffff;

			// load log option flags...
			dat->dwFlags = dat->pContainer->theme.dwFlags;

			// consider per-contact message setting overrides
			if (dat->hContact && M.GetDword(dat->hContact, "mwmask", 0))
				LoadLocalFlags(hwndDlg, dat);

			// allow disabling emoticons per contact (note: currently unused feature)
			{
				int dwLocalSmAdd = M.GetByte(dat->hContact, "doSmileys", 0xff);
				if (dwLocalSmAdd != 0xffffffff)
					dat->doSmileys = dwLocalSmAdd;
			}
			DM_InitTip(dat);
			dat->Panel->getVisibility();

			dat->dwFlagsEx |= M.GetByte(dat->hContact, "splitoverride", 0) ? MWF_SHOW_SPLITTEROVERRIDE : 0;
			dat->bIsAutosizingInput = IsAutoSplitEnabled(dat);
			dat->iInputAreaHeight = -1;
			SetMessageLog(dat);
			dat->panelWidth = -1;
			if (dat->hContact) {
				dat->codePage = M.GetDword(dat->hContact, "ANSIcodepage", CP_ACP);
				dat->Panel->loadHeight();
			}

			dat->bShowAvatar = GetAvatarVisibility(hwndDlg, dat);

			RECT rc;
			GetWindowRect(GetDlgItem(hwndDlg, IDC_SMILEYBTN), &rc);

			Utils::showDlgControl(hwndDlg, IDC_MULTISPLITTER, SW_HIDE);

			GetWindowRect(GetDlgItem(hwndDlg, IDC_SPLITTER), &rc);
			pt.y = (rc.top + rc.bottom) / 2;
			pt.x = 0;
			ScreenToClient(hwndDlg, &pt);
			dat->originalSplitterY = pt.y;
			if (dat->splitterY == -1)
				dat->splitterY = dat->originalSplitterY + 60;

			GetWindowRect(GetDlgItem(hwndDlg, IDC_MESSAGE), &rc);
			dat->minEditBoxSize.cx = rc.right - rc.left;
			dat->minEditBoxSize.cy = rc.bottom - rc.top;

			BB_InitDlgButtons(dat);
			SendMessage(hwndDlg, DM_LOADBUTTONBARICONS, 0, 0);

			BOOL isThemed = TRUE;
			if (CSkin::m_skinEnabled && !SkinItems[ID_EXTBKBUTTONSNPRESSED].IGNORED &&
				!SkinItems[ID_EXTBKBUTTONSPRESSED].IGNORED && !SkinItems[ID_EXTBKBUTTONSMOUSEOVER].IGNORED)
				isThemed = FALSE;

			SendMessage(GetDlgItem(hwndDlg, IDC_ADD), BUTTONSETASFLATBTN, TRUE, 0);
			SendMessage(GetDlgItem(hwndDlg, IDC_CANCELADD), BUTTONSETASFLATBTN, TRUE, 0);

			SendDlgItemMessage(hwndDlg, IDC_TOGGLESIDEBAR, BUTTONSETASFLATBTN, TRUE, 0);
			SendDlgItemMessage(hwndDlg, IDC_TOGGLESIDEBAR, BUTTONSETASTHEMEDBTN, isThemed, 0);
			SendDlgItemMessage(hwndDlg, IDC_TOGGLESIDEBAR, BUTTONSETCONTAINER, (LPARAM)m_pContainer, 0);
			SendDlgItemMessage(hwndDlg, IDC_TOGGLESIDEBAR, BUTTONSETASTOOLBARBUTTON, TRUE, 0);

			TABSRMM_FireEvent(dat->hContact, hwndDlg, MSG_WINDOW_EVT_OPENING, 0);

			for (int i = 0; i < SIZEOF(tooltips); i++)
				SendDlgItemMessage(hwndDlg, tooltips[i].id, BUTTONADDTOOLTIP, (WPARAM)TranslateTS(tooltips[i].text), BATF_TCHAR);

			SetDlgItemText(hwndDlg, IDC_LOGFROZENTEXT, dat->bNotOnList ? TranslateT("Contact not on list. You may add it...") :
				TranslateT("Auto scrolling is disabled (press F12 to enable it)"));

			SendMessage(GetDlgItem(hwndDlg, IDC_SAVE), BUTTONADDTOOLTIP, (WPARAM)pszIDCSAVE_close, BATF_TCHAR);
			SendMessage(GetDlgItem(hwndDlg, IDC_PROTOCOL), BUTTONADDTOOLTIP, (WPARAM)TranslateT("Click for contact menu\nClick dropdown for window settings"), BATF_TCHAR);

			SetWindowText(GetDlgItem(hwndDlg, IDC_RETRY), TranslateT("Retry"));

			UINT _ctrls[] = { IDC_RETRY, IDC_CANCELSEND, IDC_MSGSENDLATER };
			for (int i = 0; i < SIZEOF(_ctrls); i++) {
				SendDlgItemMessage(hwndDlg, _ctrls[i], BUTTONSETASPUSHBTN, TRUE, 0);
				SendDlgItemMessage(hwndDlg, _ctrls[i], BUTTONSETASFLATBTN, FALSE, 0);
				SendDlgItemMessage(hwndDlg, _ctrls[i], BUTTONSETASTHEMEDBTN, TRUE, 0);
			}

			SetWindowText(GetDlgItem(hwndDlg, IDC_CANCELSEND), TranslateT("Cancel"));
			SetWindowText(GetDlgItem(hwndDlg, IDC_MSGSENDLATER), TranslateT("Send later"));

			SendDlgItemMessage(hwndDlg, IDC_LOG, EM_SETUNDOLIMIT, 0, 0);
			SendDlgItemMessage(hwndDlg, IDC_LOG, EM_SETEVENTMASK, 0, ENM_MOUSEEVENTS | ENM_KEYEVENTS | ENM_LINK);
			SendDlgItemMessage(hwndDlg, IDC_MESSAGE, EM_SETEVENTMASK, 0, ENM_REQUESTRESIZE | ENM_MOUSEEVENTS | ENM_SCROLL | ENM_KEYEVENTS | ENM_CHANGE);

			dat->bActualHistory = M.GetByte(dat->hContact, "ActualHistory", 0);

			/* OnO: higligh lines to their end */
			SendDlgItemMessage(hwndDlg, IDC_LOG, EM_SETEDITSTYLE, SES_EXTENDBACKCOLOR, SES_EXTENDBACKCOLOR);

			SendDlgItemMessage(hwndDlg, IDC_LOG, EM_SETLANGOPTIONS, 0, SendDlgItemMessage(hwndDlg, IDC_LOG, EM_GETLANGOPTIONS, 0, 0) & ~IMF_AUTOFONTSIZEADJUST);

			// add us to the tray list (if it exists)
			if (PluginConfig.g_hMenuTrayUnread != 0 && dat->hContact != 0 && dat->szProto != NULL)
				UpdateTrayMenu(0, dat->wStatus, dat->szProto, dat->szStatus, dat->hContact, FALSE);

			SendDlgItemMessage(hwndDlg, IDC_LOG, EM_AUTOURLDETECT, TRUE, 0);
			SendDlgItemMessage(hwndDlg, IDC_LOG, EM_EXLIMITTEXT, 0, 0x80000000);

			// subclassing stuff
			mir_subclassWindow(GetDlgItem(hwndDlg, IDC_MESSAGE), MessageEditSubclassProc);
			mir_subclassWindow(GetDlgItem(hwndDlg, IDC_CONTACTPIC), AvatarSubclassProc);
			mir_subclassWindow(GetDlgItem(hwndDlg, IDC_SPLITTER), SplitterSubclassProc);
			mir_subclassWindow(GetDlgItem(hwndDlg, IDC_MULTISPLITTER), SplitterSubclassProc);
			mir_subclassWindow(GetDlgItem(hwndDlg, IDC_PANELSPLITTER), SplitterSubclassProc);

			// load old messages from history (if wanted...)
			dat->cache->updateStats(TSessionStats::INIT_TIMER);
			if (dat->hContact) {
				FindFirstEvent(dat);
				dat->nMax = dat->cache->getMaxMessageLength();
			}
			LoadContactAvatar(dat);
			SendMessage(hwndDlg, DM_OPTIONSAPPLIED, 0, 0);
			LoadOwnAvatar(dat);

			// restore saved msg if any...
			if (dat->hContact) {
				ptrT tszSavedMsg(db_get_tsa(dat->hContact, SRMSGMOD, "SavedMsg"));
				if (tszSavedMsg != 0) {
					SETTEXTEX stx = { ST_DEFAULT, 1200 };
					SendDlgItemMessage(hwndDlg, IDC_MESSAGE, EM_SETTEXTEX, (WPARAM)&stx, tszSavedMsg);
					SendQueue::UpdateSaveAndSendButton(dat);
					if (m_pContainer->hwndActive == hwndDlg)
						UpdateReadChars(dat);
				}
			}
			if (newData->szInitialText) {
				if (newData->isWchar)
					SetDlgItemTextW(hwndDlg, IDC_MESSAGE, (TCHAR*)newData->szInitialText);
				else
					SetDlgItemTextA(hwndDlg, IDC_MESSAGE, newData->szInitialText);
				int len = GetWindowTextLength(GetDlgItem(hwndDlg, IDC_MESSAGE));
				PostMessage(GetDlgItem(hwndDlg, IDC_MESSAGE), EM_SETSEL, len, len);
				if (len)
					EnableSendButton(dat, TRUE);
			}

			for (HANDLE hdbEvent = db_event_last(dat->hContact); hdbEvent; hdbEvent = db_event_prev(dat->hContact, hdbEvent)) {
				DBEVENTINFO dbei = { sizeof(dbei) };
				db_event_get(hdbEvent, &dbei);
				if (dbei.eventType == EVENTTYPE_MESSAGE && !(dbei.flags & DBEF_SENT)) {
					dat->lastMessage = dbei.timestamp;
					DM_UpdateLastMessage(dat);
					break;
				}
			}

			SendMessage(hwndContainer, DM_QUERYCLIENTAREA, 0, (LPARAM)&rc);

			WNDCLASS wndClass = { 0 };
			GetClassInfo(g_hInst, _T("RichEdit20W"), &wndClass);
			mir_subclassWindowFull(GetDlgItem(hwndDlg, IDC_LOG), MessageLogSubclassProc, wndClass.lpfnWndProc);

			SetWindowPos(hwndDlg, 0, rc.left, rc.top, (rc.right - rc.left), (rc.bottom - rc.top), newData->iActivate ? 0 : SWP_NOZORDER | SWP_NOACTIVATE);
			LoadSplitter(dat);
			ShowPicture(dat, TRUE);

			if (m_pContainer->dwFlags & CNT_CREATE_MINIMIZED || !newData->iActivate || m_pContainer->dwFlags & CNT_DEFERREDTABSELECT) {
				dat->iFlashIcon = PluginConfig.g_IconMsgEvent;
				SetTimer(hwndDlg, TIMERID_FLASHWND, TIMEOUT_FLASHWND, NULL);
				dat->mayFlashTab = true;

				DBEVENTINFO dbei = { 0 };
				dbei.eventType = EVENTTYPE_MESSAGE;
				FlashOnClist(hwndDlg, dat, dat->hDbEventFirst, &dbei);

				SendMessage(hwndContainer, DM_SETICON, (WPARAM)dat, (LPARAM)LoadSkinnedIcon(SKINICON_EVENT_MESSAGE));
				m_pContainer->dwFlags |= CNT_NEED_UPDATETITLE;
				dat->dwFlags |= MWF_NEEDCHECKSIZE | MWF_WASBACKGROUNDCREATE | MWF_DEFERREDSCROLL;
			}

			if (newData->iActivate) {
				m_pContainer->hwndActive = hwndDlg;
				ShowWindow(hwndDlg, SW_SHOW);
				SetActiveWindow(hwndDlg);
				SetForegroundWindow(hwndDlg);
			}
			else if (m_pContainer->dwFlags & CNT_CREATE_MINIMIZED) {
				dat->dwFlags |= MWF_DEFERREDSCROLL;
				ShowWindow(hwndDlg, SW_SHOWNOACTIVATE);
				m_pContainer->hwndActive = hwndDlg;
				m_pContainer->dwFlags |= CNT_DEFERREDCONFIGURE;
			}
			PostMessage(hwndContainer, DM_UPDATETITLE, dat->hContact, 0);

			DM_RecalcPictureSize(dat);
			dat->dwLastActivity = GetTickCount() - 1000;
			m_pContainer->dwLastActivity = dat->dwLastActivity;

			if (dat->hwndHPP)
				mir_subclassWindow(dat->hwndHPP, HPPKFSubclassProc);

			dat->dwFlags &= ~MWF_INITMODE;
			TABSRMM_FireEvent(dat->hContact, hwndDlg, MSG_WINDOW_EVT_OPEN, 0);

			// show a popup if wanted...
			if (newData->bWantPopup) {
				DBEVENTINFO dbei = { sizeof(dbei) };
				newData->bWantPopup = FALSE;
				db_event_get(newData->hdbEvent, &dbei);
				tabSRMM_ShowPopup(dat->hContact, newData->hdbEvent, dbei.eventType, 0, 0, hwndDlg, dat->cache->getActiveProto(), dat);
			}
			if (m_pContainer->dwFlags & CNT_CREATE_MINIMIZED) {
				m_pContainer->dwFlags &= ~CNT_CREATE_MINIMIZED;
				m_pContainer->hwndActive = hwndDlg;
				return FALSE;
			}
			return newData->iActivate != 0;
		}

	case WM_ERASEBKGND:
		{
			HDC hdc = (HDC)wParam;

			RECT rcClient, rcWindow, rc;
			HDC hdcMem = 0;
			HBITMAP hbm, hbmOld;
			HANDLE hpb = 0;

			GetClientRect(hwndDlg, &rcClient);
			DWORD cx = rcClient.right - rcClient.left;
			DWORD cy = rcClient.bottom - rcClient.top;

			if (CMimAPI::m_haveBufferedPaint)
				hpb = CMimAPI::m_pfnBeginBufferedPaint(hdc, &rcClient, BPBF_TOPDOWNDIB, 0, &hdcMem);
			else {
				hdcMem = CreateCompatibleDC(hdc);
				hbm =  CSkin::CreateAeroCompatibleBitmap(rcClient, hdc);
				hbmOld = (HBITMAP)SelectObject(hdcMem, hbm);
			}

			bool	bInfoPanel = dat->Panel->isActive();
			bool	bAero = M.isAero();

			if (CSkin::m_skinEnabled) {
				UINT item_ids[2] = {ID_EXTBKHISTORY, ID_EXTBKINPUTAREA};
				UINT ctl_ids[2] = {IDC_LOG, IDC_MESSAGE};
				BOOL isEditNotesReason = dat->fEditNotesActive;
				BOOL isSendLaterReason = (dat->sendMode & SMODE_SENDLATER);
				BOOL isMultipleReason = (dat->sendMode & SMODE_MULTIPLE || dat->sendMode & SMODE_CONTAINER);

				CSkin::SkinDrawBG(hwndDlg, hwndContainer, m_pContainer, &rcClient, hdcMem);

				for (int i=0; i < 2; i++) {
					CSkinItem *item = &SkinItems[item_ids[i]];
					if (!item->IGNORED) {
						GetWindowRect(GetDlgItem(hwndDlg, ctl_ids[i]), &rcWindow);
						pt.x = rcWindow.left;
						pt.y = rcWindow.top;
						ScreenToClient(hwndDlg, &pt);
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
					rcClient.bottom = dat->Panel->isActive() ? dat->Panel->getHeight() + 5 : 5;
					FillRect(hdcMem, &rcClient, (HBRUSH)GetStockObject(BLACK_BRUSH));
					rcClient.bottom = temp;
				}
			}

			// draw the (new) infopanel background. Use the gradient from the statusitem.
			GetClientRect(hwndDlg, &rc);
			dat->Panel->renderBG(hdcMem, rc, &SkinItems[ID_EXTBKINFOPANELBG], bAero);

			// draw aero related stuff
			if (!CSkin::m_skinEnabled)
				CSkin::RenderToolbarBG(dat, hdcMem, rcClient);
			
			// render info panel fields
			dat->Panel->renderContent(hdcMem);

			if (hpb)
				CSkin::FinalizeBufferedPaint(hpb, &rcClient);
			else {
				BitBlt(hdc, 0, 0, cx, cy, hdcMem, 0, 0, SRCCOPY);
				SelectObject(hdcMem, hbmOld);
				DeleteObject(hbm);
				DeleteDC(hdcMem);
			}
			if (!dat->fLimitedUpdate)
				SetAeroMargins(dat->pContainer);
		}
		return 1;

	case WM_NCPAINT:
		return 0;

	case WM_PAINT:
		// in skinned mode only, draw the background elements for the 2 richedit controls
		// this allows border-less textboxes to appear "skinned" and blended with the background
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwndDlg, &ps);
			EndPaint(hwndDlg, &ps);
		}
		return 0;

	case WM_SIZE:
		if (!IsIconic(hwndDlg)) {
			if (dat->ipFieldHeight == 0)
				dat->ipFieldHeight = CInfoPanel::m_ipConfig.height2;

			if (dat->pContainer->uChildMinHeight > 0 && HIWORD(lParam) >= dat->pContainer->uChildMinHeight) {
				if (dat->splitterY > HIWORD(lParam) - DPISCALEY_S(MINLOGHEIGHT)) {
					dat->splitterY = HIWORD(lParam) - DPISCALEY_S(MINLOGHEIGHT);
					dat->dynaSplitter = dat->splitterY - DPISCALEY_S(34);
					DM_RecalcPictureSize(dat);
				}
				if (dat->splitterY < DPISCALEY_S(MINSPLITTERY))
					LoadSplitter(dat);
			}

			HBITMAP hbm = ((dat->Panel->isActive()) && m_pContainer->avatarMode != 3) ? dat->hOwnPic : (dat->ace ? dat->ace->hbmPic : PluginConfig.g_hbmUnknown);
			if (hbm != 0) {
				BITMAP bminfo;
				GetObject(hbm, sizeof(bminfo), &bminfo);
				CalcDynamicAvatarSize(dat, &bminfo);
			}

			RECT rc;
			GetClientRect(hwndDlg, &rc);

			UTILRESIZEDIALOG urd = { sizeof(urd) };
			urd.hInstance = g_hInst;
			urd.hwndDlg = hwndDlg;
			urd.lParam = (LPARAM)dat;
			urd.lpTemplate = MAKEINTRESOURCEA(IDD_MSGSPLITNEW);
			urd.pfnResizer = MessageDialogResize;
			CallService(MS_UTILS_RESIZEDIALOG, 0, (LPARAM)&urd);

			BB_SetButtonsPos(dat);

			// size info panel fields
			LONG cx = rc.right;
			LONG panelHeight = dat->Panel->getHeight();
			LONG panelWidth = (dat->panelWidth != -1 ? dat->panelWidth : 0);

			rc.top = 1;
			rc.left = cx - (panelWidth > 0 ? panelWidth : panelHeight);
			rc.bottom = rc.top + (panelHeight - 3);
			rc.right = cx;
			rc.bottom--;

			if (dat->bShowInfoAvatar) {
				SetWindowPos(dat->hwndPanelPicParent, HWND_TOP, rc.left - 2, rc.top, rc.right - rc.left, (rc.bottom - rc.top) + 1, 0);
				ShowWindow(dat->hwndPanelPicParent, (dat->panelWidth == -1) || !dat->Panel->isActive() ? SW_HIDE : SW_SHOW);
			}

			dat->rcPic = rc;

			rc.right = cx - panelWidth;
			rc.left = cx - panelWidth - dat->panelStatusCX;
			rc.bottom = panelHeight - 3;
			rc.top = rc.bottom - dat->ipFieldHeight;
			dat->rcStatus = rc;

			rc.left = CInfoPanel::LEFT_OFFSET_LOGO;
			rc.right = cx - dat->panelWidth - (panelHeight < CInfoPanel::DEGRADE_THRESHOLD ? (dat->rcStatus.right - dat->rcStatus.left) + 3 : 0);
			rc.bottom = panelHeight - (panelHeight >= CInfoPanel::DEGRADE_THRESHOLD ? dat->ipFieldHeight : 0) - 1;
			rc.top = 1;
			dat->rcNick = rc;

			rc.left = CInfoPanel::LEFT_OFFSET_LOGO;
			rc.right = cx - (dat->panelWidth + 2) - dat->panelStatusCX;
			rc.bottom = panelHeight - 3;
			rc.top = rc.bottom - dat->ipFieldHeight;
			dat->rcUIN = rc;

			if (GetDlgItem(hwndDlg, IDC_CLIST) != 0) {
				RECT rc, rcClient, rcLog;
				GetClientRect(hwndDlg, &rcClient);
				GetClientRect(GetDlgItem(hwndDlg, IDC_LOG), &rcLog);
				rc.top = 0;
				rc.right = rcClient.right;
				rc.left = rcClient.right - dat->multiSplitterX;
				rc.bottom = rcLog.bottom;
				if (dat->Panel->isActive())
					rc.top += (dat->Panel->getHeight() + 1);
				MoveWindow(GetDlgItem(hwndDlg, IDC_CLIST), rc.left, rc.top, rc.right - rc.left, rcLog.bottom - rcLog.top, FALSE);
			}

			if (dat->hwndIEView || dat->hwndHPP)
				ResizeIeView(dat, 0, 0, 0, 0);

			dat->Panel->Invalidate();
			DetermineMinHeight(dat);
		}
		break;

	case WM_TIMECHANGE:
		PostMessage(hwndDlg, DM_OPTIONSAPPLIED, 0, 0);
		break;

	case WM_NOTIFY:
		if (dat != 0 && ((NMHDR*)lParam)->hwndFrom == dat->hwndTip) {
			if (((NMHDR*)lParam)->code == NM_CLICK)
				SendMessage(dat->hwndTip, TTM_TRACKACTIVATE, FALSE, 0);
			break;
		}

		switch (((NMHDR*)lParam)->idFrom) {
		case IDC_LOG:
		case IDC_MESSAGE:
			switch (((NMHDR*)lParam)->code) {
			case EN_MSGFILTER:
				{
					DWORD msg = ((MSGFILTER *) lParam)->msg;
					WPARAM wp = ((MSGFILTER *) lParam)->wParam;
					LPARAM lp = ((MSGFILTER *) lParam)->lParam;
					CHARFORMAT2 cf2;
					BOOL 	isCtrl, isShift, isAlt;
					KbdState(dat, isShift, isCtrl, isAlt);

					MSG message;
					message.hwnd = hwndDlg;
					message.message = msg;
					message.lParam = lp;
					message.wParam = wp;

					if (msg == WM_SYSKEYUP) {
						UINT ctrlId = 0;

						if (wp == VK_MENU)
							if (!dat->fkeyProcessed && !(GetKeyState(VK_CONTROL) & 0x8000) && !(GetKeyState(VK_SHIFT) & 0x8000) && !(lp & (1 << 24)))
								m_pContainer->MenuBar->autoShow();

						return(_dlgReturn(hwndDlg, 0));
					}

					if ((msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN) && !(GetKeyState(VK_RMENU) & 0x8000)) {
						LRESULT mim_hotkey_check = CallService(MS_HOTKEY_CHECK, (WPARAM)&message, (LPARAM)(TABSRMM_HK_SECTION_IM));
						if (mim_hotkey_check)
							dat->fkeyProcessed = true;

						switch (mim_hotkey_check) {
						case TABSRMM_HK_SETUSERPREFS:
							CallService(MS_TABMSG_SETUSERPREFS, dat->hContact, 0);
							return(_dlgReturn(hwndDlg, 1));
						case TABSRMM_HK_NUDGE:
							SendNudge(dat);
							return(_dlgReturn(hwndDlg, 1));
						case TABSRMM_HK_SENDFILE:
							CallService(MS_FILE_SENDFILE, dat->hContact, 0);
							return(_dlgReturn(hwndDlg, 1));
						case TABSRMM_HK_QUOTEMSG:
							SendMessage(hwndDlg, WM_COMMAND, IDC_QUOTE, 0);
							return(_dlgReturn(hwndDlg, 1));
						case TABSRMM_HK_USERMENU:
							SendMessage(hwndDlg, WM_COMMAND, IDC_PROTOCOL, 0);
							return(_dlgReturn(hwndDlg, 1));
						case TABSRMM_HK_USERDETAILS:
							SendMessage(hwndDlg, WM_COMMAND, MAKELONG(IDC_NAME, BN_CLICKED), 0);
							return(_dlgReturn(hwndDlg, 1));
						case TABSRMM_HK_EDITNOTES:
							PostMessage(hwndDlg, WM_COMMAND, MAKELONG(IDC_PIC, BN_CLICKED), 0);
							return(_dlgReturn(hwndDlg, 1));
						case TABSRMM_HK_TOGGLESENDLATER:
							if (sendLater->isAvail()) {
								dat->sendMode ^= SMODE_SENDLATER;
								SetWindowPos(GetDlgItem(hwndDlg, IDC_MESSAGE), 0, 0, 0, 0, 0, SWP_DRAWFRAME|SWP_FRAMECHANGED|SWP_NOZORDER|
									SWP_NOMOVE|SWP_NOSIZE|SWP_NOCOPYBITS);
								RedrawWindow(hwndDlg, 0, 0, RDW_INVALIDATE|RDW_ERASE|RDW_UPDATENOW|RDW_ALLCHILDREN);
							}
							else
								CWarning::show(CWarning::WARN_NO_SENDLATER, MB_OK|MB_ICONINFORMATION, TranslateT("Configuration issue|The unattended send feature is disabled. The \\b1 send later\\b0  and \\b1 send to multiple contacts\\b0  features depend on it.\n\nYou must enable it under \\b1Options->Message Sessions->Advanced tweaks\\b0. Changing this option requires a restart."));
							return(_dlgReturn(hwndDlg, 1));
						case TABSRMM_HK_TOGGLERTL:
							{
								DWORD	dwGlobal = M.GetDword("mwflags", MWF_LOG_DEFAULT);
								DWORD	dwOldFlags = dat->dwFlags;
								DWORD	dwMask = M.GetDword(dat->hContact, "mwmask", 0);
								DWORD	dwFlags = M.GetDword(dat->hContact, "mwflags", 0);

								dat->dwFlags ^= MWF_LOG_RTL;
								if ((dwGlobal & MWF_LOG_RTL) != (dat->dwFlags & MWF_LOG_RTL)) {
									dwMask |= MWF_LOG_RTL;
									dwFlags |= (dat->dwFlags & MWF_LOG_RTL);
								}
								else {
									dwMask &= ~MWF_LOG_RTL;
									dwFlags &= ~MWF_LOG_RTL;
								}
								if (dwMask) {
									db_set_dw(dat->hContact, SRMSGMOD_T, "mwmask", dwMask);
									db_set_dw(dat->hContact, SRMSGMOD_T, "mwflags", dwFlags);
								}
								else {
									db_unset(dat->hContact, SRMSGMOD_T, "mwmask");
									db_unset(dat->hContact, SRMSGMOD_T, "mwflags");
								}
								SendMessage(hwndDlg, DM_OPTIONSAPPLIED, 0, 0);
								SendMessage(hwndDlg, DM_DEFERREDREMAKELOG, (WPARAM)hwndDlg, 0);
							}
							return(_dlgReturn(hwndDlg, 1));

						case TABSRMM_HK_TOGGLEMULTISEND:
							dat->sendMode ^= SMODE_MULTIPLE;
							if (dat->sendMode & SMODE_MULTIPLE)
								DM_CreateClist(dat);
							else if (IsWindow(GetDlgItem(hwndDlg, IDC_CLIST)))
								DestroyWindow(GetDlgItem(hwndDlg, IDC_CLIST));

							HWND hwndEdit = GetDlgItem(hwndDlg, IDC_MESSAGE);
							SetWindowPos(hwndEdit, 0, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOSIZE | SWP_NOMOVE);
							SendMessage(hwndDlg, WM_SIZE, 0, 0);
							RedrawWindow(hwndEdit, NULL, NULL, RDW_INVALIDATE | RDW_FRAME | RDW_UPDATENOW | RDW_ERASE);
							DM_ScrollToBottom(dat, 0, 0);
							Utils::showDlgControl(hwndDlg, IDC_MULTISPLITTER, (dat->sendMode & SMODE_MULTIPLE) ? SW_SHOW : SW_HIDE);
							Utils::showDlgControl(hwndDlg, IDC_CLIST, (dat->sendMode & SMODE_MULTIPLE) ? SW_SHOW : SW_HIDE);
							if (dat->sendMode & SMODE_MULTIPLE)
								SetFocus(GetDlgItem(hwndDlg, IDC_CLIST));
							else
								SetFocus(GetDlgItem(hwndDlg, IDC_MESSAGE));
							RedrawWindow(hwndDlg, 0, 0, RDW_INVALIDATE|RDW_ERASE|RDW_UPDATENOW|RDW_ALLCHILDREN);
							return(_dlgReturn(hwndDlg, 1));
						}
						if (DM_GenericHotkeysCheck(&message, dat)) {
							dat->fkeyProcessed = true;
							return(_dlgReturn(hwndDlg, 1));
						}
					}
					if (wp == VK_BROWSER_BACK || wp == VK_BROWSER_FORWARD)
						return 1;

					if (msg == WM_CHAR) {
						if (isCtrl && !isShift && !isAlt) {
							switch (wp) {
							case 23:                // ctrl - w
								PostMessage(hwndDlg, WM_CLOSE, 1, 0);
								break;
							case 19:
								PostMessage(hwndDlg, WM_COMMAND, IDC_SENDMENU, IDC_SENDMENU);
								break;
							case 16:
								PostMessage(hwndDlg, WM_COMMAND, IDC_PROTOMENU, IDC_PROTOMENU);
								break;
							case 20:
								PostMessage(hwndDlg, WM_COMMAND, IDC_TOGGLETOOLBAR, 1);
								break;
							}
							return 1;
						}
					}
					if (msg == WM_KEYDOWN) {
						if ((wp == VK_INSERT && isShift && !isCtrl) || (wp == 'V' && isCtrl && !isShift && !isAlt)) {
							SendMessage(GetDlgItem(hwndDlg, IDC_MESSAGE), EM_PASTESPECIAL, CF_UNICODETEXT, 0);
							_clrMsgFilter(lParam);
							return(_dlgReturn(hwndDlg, 1));
						}
						if (isCtrl && isShift) {
							if (wp == 0x9) {            // ctrl-shift tab
								SendMessage(hwndDlg, DM_SELECTTAB, DM_SELECT_PREV, 0);
								_clrMsgFilter(lParam);
								return(_dlgReturn(hwndDlg, 1));
							}
						}
						if (isCtrl && !isShift && !isAlt) {
							if (wp == VK_TAB) {
								SendMessage(hwndDlg, DM_SELECTTAB, DM_SELECT_NEXT, 0);
								_clrMsgFilter(lParam);
								return(_dlgReturn(hwndDlg, 1));
							}
							if (wp == VK_F4) {
								PostMessage(hwndDlg, WM_CLOSE, 1, 0);
								return(_dlgReturn(hwndDlg, 1));
							}
							if (wp == VK_PRIOR) {
								SendMessage(hwndDlg, DM_SELECTTAB, DM_SELECT_PREV, 0);
								return(_dlgReturn(hwndDlg, 1));
							}
							if (wp == VK_NEXT) {
								SendMessage(hwndDlg, DM_SELECTTAB, DM_SELECT_NEXT, 0);
								return(_dlgReturn(hwndDlg, 1));
							}
						}
					}
					if (msg == WM_SYSKEYDOWN && isAlt) {
						if (wp == 0x52) {
							SendMessage(hwndDlg, DM_QUERYPENDING, DM_QUERY_MOSTRECENT, 0);
							return(_dlgReturn(hwndDlg, 1));
						}
						if (wp == VK_MULTIPLY) {
							SetFocus(GetDlgItem(hwndDlg, IDC_MESSAGE));
							return(_dlgReturn(hwndDlg, 1));
						}
						if (wp == VK_DIVIDE) {
							SetFocus(GetDlgItem(hwndDlg, IDC_LOG));
							return(_dlgReturn(hwndDlg, 1));
						}
						if (wp == VK_ADD) {
							SendMessage(hwndContainer, DM_SELECTTAB, DM_SELECT_NEXT, 0);
							return(_dlgReturn(hwndDlg, 1));
						}
						if (wp == VK_SUBTRACT) {
							SendMessage(hwndContainer, DM_SELECTTAB, DM_SELECT_PREV, 0);
							return(_dlgReturn(hwndDlg, 1));
						}
					}

					if (msg == WM_KEYDOWN && wp == VK_F12) {
						if (isShift || isCtrl || isAlt)
							return(_dlgReturn(hwndDlg, 1));
						if (dat->dwFlagsEx & MWF_SHOW_SCROLLINGDISABLED)
							SendMessage(hwndDlg, DM_REPLAYQUEUE, 0, 0);
						dat->dwFlagsEx ^= MWF_SHOW_SCROLLINGDISABLED;
						Utils::showDlgControl(hwndDlg, IDC_LOGFROZENTEXT, (dat->bNotOnList || dat->dwFlagsEx & MWF_SHOW_SCROLLINGDISABLED) ? SW_SHOW : SW_HIDE);
						if (!(dat->dwFlagsEx & MWF_SHOW_SCROLLINGDISABLED))
							SetDlgItemText(hwndDlg, IDC_LOGFROZENTEXT, TranslateT("Contact not on list. You may add it..."));
						else
							SetDlgItemText(hwndDlg, IDC_LOGFROZENTEXT, TranslateT("Auto scrolling is disabled (press F12 to enable it)"));
						SendMessage(hwndDlg, WM_SIZE, 0, 0);
						DM_ScrollToBottom(dat, 1, 1);
						return(_dlgReturn(hwndDlg, 1));
					}
					//MAD: tabulation mod
					if (msg == WM_KEYDOWN && wp == VK_TAB) {
						if (PluginConfig.m_AllowTab) {
							if (((NMHDR*)lParam)->idFrom == IDC_MESSAGE)
								SendMessage(GetDlgItem(hwndDlg, IDC_MESSAGE), EM_REPLACESEL, FALSE, (LPARAM)"\t");
							_clrMsgFilter(lParam);
							if (((NMHDR*)lParam)->idFrom != IDC_MESSAGE)
								SetFocus(GetDlgItem(hwndDlg, IDC_MESSAGE));
							return(_dlgReturn(hwndDlg, 1));
						}
						else {
							if (((NMHDR*)lParam)->idFrom == IDC_MESSAGE) {
								if (GetSendButtonState(hwndDlg) != PBS_DISABLED && !(dat->pContainer->dwFlags & CNT_HIDETOOLBAR))
									SetFocus(GetDlgItem(hwndDlg, IDOK));
								else
									SetFocus(GetDlgItem(hwndDlg, IDC_LOG));
								return(_dlgReturn(hwndDlg, 1));
							}
							if (((NMHDR*)lParam)->idFrom == IDC_LOG) {
								SetFocus(GetDlgItem(hwndDlg, IDC_MESSAGE));
								return(_dlgReturn(hwndDlg, 1));
							}
						}
						return(_dlgReturn(hwndDlg, 0));
					}

					if (msg == WM_MOUSEWHEEL && (((NMHDR*)lParam)->idFrom == IDC_LOG || ((NMHDR*)lParam)->idFrom == IDC_MESSAGE)) {
						RECT rc;
						GetCursorPos(&pt);
						GetWindowRect(GetDlgItem(hwndDlg, IDC_LOG), &rc);
						if (PtInRect(&rc, pt)) {
							short wDirection = (short)HIWORD(wp);
							if (LOWORD(wp) & MK_SHIFT) {
								if (wDirection < 0)
									SendMessage(GetDlgItem(hwndDlg, IDC_LOG), WM_VSCROLL, MAKEWPARAM(SB_PAGEDOWN, 0), 0);
								else if (wDirection > 0)
									SendMessage(GetDlgItem(hwndDlg, IDC_LOG), WM_VSCROLL, MAKEWPARAM(SB_PAGEUP, 0), 0);
								return 0;
							}
							return 0;
						}
						return 1;
					}

					if (msg == WM_CHAR && wp == 'c') {
						if (isCtrl) {
							SendDlgItemMessage(hwndDlg, ((NMHDR*)lParam)->code, WM_COPY, 0, 0);
							break;
						}
					}
					if ((msg == WM_LBUTTONDOWN || msg == WM_KEYUP || msg == WM_LBUTTONUP) && ((NMHDR*)lParam)->idFrom == IDC_MESSAGE) {
						int bBold = IsDlgButtonChecked(hwndDlg, IDC_FONTBOLD);
						int bItalic = IsDlgButtonChecked(hwndDlg, IDC_FONTITALIC);
						int bUnder = IsDlgButtonChecked(hwndDlg, IDC_FONTUNDERLINE);
						int bStrikeout = IsDlgButtonChecked(hwndDlg, IDC_FONTSTRIKEOUT);

						cf2.cbSize = sizeof(CHARFORMAT2);
						cf2.dwMask = CFM_BOLD | CFM_ITALIC | CFM_UNDERLINE | CFM_UNDERLINETYPE | CFM_STRIKEOUT;
						cf2.dwEffects = 0;
						SendDlgItemMessage(hwndDlg, IDC_MESSAGE, EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf2);
						if (cf2.dwEffects & CFE_BOLD) {
							if (bBold == BST_UNCHECKED)
								CheckDlgButton(hwndDlg, IDC_FONTBOLD, BST_CHECKED);
						}
						else if (bBold == BST_CHECKED)
							CheckDlgButton(hwndDlg, IDC_FONTBOLD, BST_UNCHECKED);

						if (cf2.dwEffects & CFE_ITALIC) {
							if (bItalic == BST_UNCHECKED)
								CheckDlgButton(hwndDlg, IDC_FONTITALIC, BST_CHECKED);
						}
						else if (bItalic == BST_CHECKED)
							CheckDlgButton(hwndDlg, IDC_FONTITALIC, BST_UNCHECKED);

						if (cf2.dwEffects & CFE_UNDERLINE && (cf2.bUnderlineType & CFU_UNDERLINE || cf2.bUnderlineType & CFU_UNDERLINEWORD)) {
							if (bUnder == BST_UNCHECKED)
								CheckDlgButton(hwndDlg, IDC_FONTUNDERLINE, BST_CHECKED);
						}
						else if (bUnder == BST_CHECKED)
							CheckDlgButton(hwndDlg, IDC_FONTUNDERLINE, BST_UNCHECKED);

						if (cf2.dwEffects & CFE_STRIKEOUT) {
							if (bStrikeout == BST_UNCHECKED)
								CheckDlgButton(hwndDlg, IDC_FONTSTRIKEOUT, BST_CHECKED);
						}
						else if (bStrikeout == BST_CHECKED)
							CheckDlgButton(hwndDlg, IDC_FONTSTRIKEOUT, BST_UNCHECKED);
					}
					switch (msg) {
					case WM_LBUTTONDOWN:
						{
							HCURSOR hCur = GetCursor();
							m_pContainer->MenuBar->Cancel();
							if (hCur == LoadCursor(NULL, IDC_SIZENS) || hCur == LoadCursor(NULL, IDC_SIZEWE)
								|| hCur == LoadCursor(NULL, IDC_SIZENESW) || hCur == LoadCursor(NULL, IDC_SIZENWSE)) {
									SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, TRUE);
									return TRUE;
							}
							break;
						}
						
					// auto-select-and-copy handling...
					// if enabled, releasing the lmb with an active selection automatically copies the selection
					// to the clipboard.
					// holding ctrl while releasing the button pastes the selection to the input area, using plain text
					// holding ctrl-alt does the same, but pastes formatted text
					case WM_LBUTTONUP:
						if (((NMHDR*) lParam)->idFrom == IDC_LOG) {
							CHARRANGE cr;
							SendMessage(GetDlgItem(hwndDlg, IDC_LOG), EM_EXGETSEL, 0, (LPARAM)&cr);
							if (cr.cpMax != cr.cpMin) {
								cr.cpMin = cr.cpMax;
								if (isCtrl && M.GetByte("autocopy", 1)) {
									SETTEXTEX stx = {ST_KEEPUNDO | ST_SELECTION, CP_UTF8};
									char *streamOut = NULL;
									if (isAlt)
										streamOut = Message_GetFromStream(GetDlgItem(hwndDlg, IDC_LOG), dat, (CP_UTF8 << 16) | (SF_RTFNOOBJS | SFF_PLAINRTF | SFF_SELECTION | SF_USECODEPAGE));
									else
										streamOut = Message_GetFromStream(GetDlgItem(hwndDlg, IDC_LOG), dat, (CP_UTF8 << 16) | (SF_TEXT | SFF_SELECTION | SF_USECODEPAGE));
									if (streamOut) {
										Utils::FilterEventMarkers(streamOut);
										SendMessage(GetDlgItem(hwndDlg, IDC_MESSAGE), EM_SETTEXTEX, (WPARAM)&stx, (LPARAM)streamOut);
										mir_free(streamOut);
									}
									SetFocus(GetDlgItem(hwndDlg, IDC_MESSAGE));
								}
								else if (M.GetByte("autocopy", 1) && !isShift) {
									SendMessage(GetDlgItem(hwndDlg, IDC_LOG), WM_COPY, 0, 0);
									SetFocus(GetDlgItem(hwndDlg, IDC_MESSAGE));
									if (m_pContainer->hwndStatus)
										SendMessage(m_pContainer->hwndStatus, SB_SETTEXT, 0, (LPARAM)TranslateT("Selection copied to clipboard"));
								}
							}
						}
						break;

					case WM_MOUSEMOVE:
						POINT	pt;
						HCURSOR hCur = GetCursor();
						GetCursorPos(&pt);
						DM_DismissTip(dat, pt);
						dat->Panel->trackMouse(pt);
						if (hCur == LoadCursor(NULL, IDC_SIZENS) || hCur == LoadCursor(NULL, IDC_SIZEWE) || hCur == LoadCursor(NULL, IDC_SIZENESW) || hCur == LoadCursor(NULL, IDC_SIZENWSE))
							SetCursor(LoadCursor(NULL, IDC_ARROW));
						break;
					}
				}
				break;

			case EN_REQUESTRESIZE:
				DM_HandleAutoSizeRequest(dat, (REQRESIZE *)lParam);
				break;

			case EN_LINK:
				switch (((ENLINK *)lParam)->msg) {
				case WM_SETCURSOR:
					SetCursor(PluginConfig.hCurHyperlinkHand);
					SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, TRUE);
					return TRUE;

				case WM_RBUTTONDOWN:
				case WM_LBUTTONUP:
					{
						CHARRANGE sel;
						SendDlgItemMessage(hwndDlg, IDC_LOG, EM_EXGETSEL, 0, (LPARAM)&sel);
						if (sel.cpMin != sel.cpMax)
							break;

						TEXTRANGEW tr;
						tr.chrg = ((ENLINK*)lParam)->chrg;
						tr.lpstrText = (TCHAR*)_alloca(sizeof(TCHAR)*(tr.chrg.cpMax - tr.chrg.cpMin + 8));
						SendDlgItemMessage(hwndDlg, IDC_LOG, EM_GETTEXTRANGE, 0, (LPARAM)&tr);
						if (_tcschr(tr.lpstrText, '@') != NULL && _tcschr(tr.lpstrText, ':') == NULL && _tcschr(tr.lpstrText, '/') == NULL) {
							MoveMemory(tr.lpstrText + 7, tr.lpstrText, tr.chrg.cpMax - tr.chrg.cpMin + 1);
							CopyMemory(tr.lpstrText, _T("mailto:"), 7);
						}
						if (!IsStringValidLink(tr.lpstrText))
							break;

						if (((ENLINK*)lParam)->msg != WM_RBUTTONDOWN) {
							CallService(MS_UTILS_OPENURL, OUF_TCHAR, (LPARAM)tr.lpstrText);
							SetFocus(GetDlgItem(hwndDlg, IDC_MESSAGE));
							break;
						}
						HMENU hMenu = LoadMenu(g_hInst, MAKEINTRESOURCE(IDR_CONTEXT));
						HMENU hSubMenu = GetSubMenu(hMenu, 1);
						TranslateMenu(hSubMenu);
						pt.x = (short)LOWORD(((ENLINK*)lParam)->lParam);
						pt.y = (short)HIWORD(((ENLINK*)lParam)->lParam);
						ClientToScreen(((NMHDR*)lParam)->hwndFrom, &pt);
						switch (TrackPopupMenu(hSubMenu, TPM_RETURNCMD, pt.x, pt.y, 0, hwndDlg, NULL)) {
						case IDM_OPENNEW:
							CallService(MS_UTILS_OPENURL, 1 + OUF_TCHAR, (LPARAM)tr.lpstrText);
							break;

						case IDM_OPENEXISTING:
							CallService(MS_UTILS_OPENURL, OUF_TCHAR, (LPARAM)tr.lpstrText);
							break;

						case IDM_COPYLINK:
							if (!OpenClipboard(hwndDlg))
								break;

							EmptyClipboard();
							HGLOBAL hData = GlobalAlloc(GMEM_MOVEABLE, sizeof(TCHAR)*(lstrlen(tr.lpstrText) + 1));
							TCHAR *buf = (TCHAR*)GlobalLock(hData);
							lstrcpy(buf, tr.lpstrText);
							GlobalUnlock(hData);
							SetClipboardData(CF_UNICODETEXT, hData);
							CloseClipboard();
							break;
						}
						DestroyMenu(hMenu);
						SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, TRUE);
						return TRUE;
					}
				}
			}
		}
		break;

	case DM_TYPING:
		{
			int preTyping = dat->nTypeSecs != 0;
			dat->nTypeSecs = (int)lParam > 0 ? (int)lParam : 0;

			if (dat->nTypeSecs)
				dat->showTyping = 0;

			SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, preTyping);
		}
		return TRUE;

	case DM_UPDATEWINICON:
		if (dat->hXStatusIcon) {
			DestroyIcon(dat->hXStatusIcon);
			dat->hXStatusIcon = 0;
		}

		if (LPCSTR szProto = dat->cache->getProto()) {
			dat->hTabIcon = dat->hTabStatusIcon = MY_GetContactIcon(dat, "MetaiconTab");
			if (M.GetByte("use_xicons", 1))
				dat->hXStatusIcon = GetXStatusIcon(dat);

			SendDlgItemMessage(hwndDlg, IDC_PROTOCOL, BUTTONSETASDIMMED, (dat->dwFlagsEx & MWF_SHOW_ISIDLE) != 0, 0);
			SendDlgItemMessage(hwndDlg, IDC_PROTOCOL, BM_SETIMAGE, IMAGE_ICON, (LPARAM)(dat->hXStatusIcon ? dat->hXStatusIcon : MY_GetContactIcon(dat, "MetaiconBar")));

			if (m_pContainer->hwndActive == hwndDlg)
				SendMessage(m_pContainer->hwnd, DM_SETICON, (WPARAM)dat, (LPARAM)(dat->hXStatusIcon ? dat->hXStatusIcon : dat->hTabIcon));

			if (dat->pWnd)
				dat->pWnd->updateIcon(dat->hXStatusIcon ? dat->hXStatusIcon : dat->hTabIcon);
		}
		return 0;

	// configures the toolbar only... if lParam != 0, then it also calls
	// SetDialogToType() to reconfigure the message window
	case DM_CONFIGURETOOLBAR:
		dat->showUIElements = m_pContainer->dwFlags & CNT_HIDETOOLBAR ? 0 : 1;

		SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_SPLITTER), GWL_EXSTYLE, GetWindowLongPtr(GetDlgItem(hwndDlg, IDC_SPLITTER), GWL_EXSTYLE) & ~WS_EX_STATICEDGE);

		if (lParam == 1) {
			GetSendFormat(dat, 1);
			SetDialogToType(hwndDlg);
		}

		if (lParam == 1) {
			DM_RecalcPictureSize(dat);
			SendMessage(hwndDlg, WM_SIZE, 0, 0);
			DM_ScrollToBottom(dat, 0, 1);
		}
		return 0;

	case DM_LOADBUTTONBARICONS:
		for (int i=0; i < SIZEOF(buttonicons); i++) {
			SendDlgItemMessage(hwndDlg, buttonicons[i].id, BM_SETIMAGE, IMAGE_ICON, (LPARAM)*buttonicons[i].pIcon);
			SendDlgItemMessage(hwndDlg, buttonicons[i].id, BUTTONSETCONTAINER, (LPARAM)m_pContainer, 0);
		}

		BB_UpdateIcons(hwndDlg, dat);
		SendMessage(hwndDlg, DM_UPDATEWINICON, 0, 0);
		return 0;

	case DM_OPTIONSAPPLIED:
		DM_OptionsApplied(dat, wParam, lParam);
		return 0;

	case DM_UPDATETITLE:
		DM_UpdateTitle(dat, wParam, lParam);
		return 0;

	case DM_UPDATESTATUSMSG:
		dat->Panel->Invalidate();
		return 0;

	case DM_OWNNICKCHANGED:
		GetMyNick(dat);
		return 0;

	case DM_ADDDIVIDER:
		if (!(dat->dwFlags & MWF_DIVIDERSET) && PluginConfig.m_UseDividers) {
			if (GetWindowTextLengthA(GetDlgItem(hwndDlg, IDC_LOG)) > 0) {
				dat->dwFlags |= MWF_DIVIDERWANTED;
				dat->dwFlags |= MWF_DIVIDERSET;
			}
		}
		return 0;

	case WM_SETFOCUS:
		MsgWindowUpdateState(dat, WM_SETFOCUS);
		SetFocus(GetDlgItem(hwndDlg, IDC_MESSAGE));
		return 1;

	case WM_ACTIVATE:
		if (LOWORD(wParam) != WA_ACTIVE)
			break;

		//fall through
	case WM_MOUSEACTIVATE:
		MsgWindowUpdateState(dat, WM_ACTIVATE);
		return 1;

	case DM_UPDATEPICLAYOUT:
		LoadContactAvatar(dat);
		SendMessage(hwndDlg, WM_SIZE, 0, 0);
		return 0;

	case DM_SPLITTERGLOBALEVENT:
		DM_SplitterGlobalEvent(dat, wParam, lParam);
		return 0;

	case DM_SPLITTERMOVED:
		if ((HWND) lParam == GetDlgItem(hwndDlg, IDC_MULTISPLITTER)) {
			RECT rc;
			GetClientRect(hwndDlg, &rc);
			pt.x = wParam;
			pt.y = 0;
			ScreenToClient(hwndDlg, &pt);
			int oldSplitterX = dat->multiSplitterX;
			dat->multiSplitterX = rc.right - pt.x;
			if (dat->multiSplitterX < 25)
				dat->multiSplitterX = 25;

			if (dat->multiSplitterX > ((rc.right - rc.left) - 80))
				dat->multiSplitterX = oldSplitterX;
			SendMessage(dat->hwnd, WM_SIZE, 0, 0);
		}
		else if ((HWND) lParam == GetDlgItem(hwndDlg, IDC_SPLITTER)) {
			int oldSplitterY, oldDynaSplitter;
			int bottomtoolbarH=0;
			RECT rc;
			GetClientRect(hwndDlg, &rc);
			rc.top += (dat->Panel->isActive() ? dat->Panel->getHeight() + 40 : 30);
			pt.x = 0;
			pt.y = wParam;
			ScreenToClient(hwndDlg, &pt);

			oldSplitterY = dat->splitterY;
			oldDynaSplitter = dat->dynaSplitter;

			dat->splitterY = rc.bottom - pt.y +DPISCALEY_S(23);
	
			// attempt to fix splitter troubles..
			// hardcoded limits... better solution is possible, but this works for now
			if (dat->pContainer->dwFlags & CNT_BOTTOMTOOLBAR)
				bottomtoolbarH = 22;
	
			if (dat->splitterY < (DPISCALEY_S(MINSPLITTERY) + 5 + bottomtoolbarH)) {	// min splitter size
				dat->splitterY = (DPISCALEY_S(MINSPLITTERY) + 5 + bottomtoolbarH);
				dat->dynaSplitter = dat->splitterY - DPISCALEY_S(34);
				DM_RecalcPictureSize(dat);
			}
			else if (dat->splitterY > (rc.bottom - rc.top)) {
				dat->splitterY = oldSplitterY;
				dat->dynaSplitter = oldDynaSplitter;
				DM_RecalcPictureSize(dat);
			}
			else {
				dat->dynaSplitter = (rc.bottom - pt.y) - DPISCALEY_S(11);
				DM_RecalcPictureSize(dat);
			}
			CSkin::UpdateToolbarBG(dat);
			SendMessage(dat->hwnd, WM_SIZE, 0, 0);
		}
		else if ((HWND) lParam == GetDlgItem(hwndDlg, IDC_PANELSPLITTER)) {
			RECT	rc;
			GetClientRect(GetDlgItem(hwndDlg, IDC_LOG), &rc);

			POINT	pt = { 0, wParam };
			ScreenToClient(hwndDlg, &pt);
			if ((pt.y + 2 >= MIN_PANELHEIGHT + 2) && (pt.y + 2 < 100) && (pt.y + 2 < rc.bottom - 30))
				dat->Panel->setHeight(pt.y + 2, true);
			dat->panelWidth = -1;
			RedrawWindow(hwndDlg, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
			if (M.isAero())
				InvalidateRect(GetParent(hwndDlg), NULL, FALSE);
		}
		break;

	// queue a dm_remakelog
	// wParam = hwnd of the sender, so we can directly do a DM_REMAKELOG if the msg came
	// from ourself. otherwise, the dm_remakelog will be deferred until next window
	// activation (focus)
	case DM_DEFERREDREMAKELOG:
		if ((HWND) wParam == hwndDlg)
			SendMessage(hwndDlg, DM_REMAKELOG, 0, 0);
		else {
			if (M.GetByte(dat->hContact, "mwoverride", 0) == 0) {
				dat->dwFlags &= ~(MWF_LOG_ALL);
				dat->dwFlags |= (lParam & MWF_LOG_ALL);
				dat->dwFlags |= MWF_DEFERREDREMAKELOG;
			}
		}
		return 0;

	case DM_FORCEDREMAKELOG:
		if ((HWND) wParam == hwndDlg)
			SendMessage(hwndDlg, DM_REMAKELOG, 0, 0);
		else {
			dat->dwFlags &= ~(MWF_LOG_ALL);
			dat->dwFlags |= (lParam & MWF_LOG_ALL);
			dat->dwFlags |= MWF_DEFERREDREMAKELOG;
		}
		return 0;

	case DM_REMAKELOG:
		dat->szMicroLf[0] = 0;
		dat->lastEventTime = 0;
		dat->iLastEventType = -1;
		StreamInEvents(hwndDlg, dat->hDbEventFirst, -1, 0, NULL);
		return 0;

	case DM_APPENDMCEVENT:
		if (dat->hContact == db_mc_getMeta(wParam) && dat->hDbEventFirst == NULL) {
			dat->hDbEventFirst = (HANDLE)lParam;
			SendMessage(dat->hwnd, DM_REMAKELOG, 0, 0);
		}
		else if (dat->hContact == wParam && db_mc_isSub(wParam) && db_event_getContact(HANDLE(lParam)) != wParam)
			StreamInEvents(hwndDlg, (HANDLE)lParam, 1, 1, NULL);
		return 0;

	case DM_APPENDTOLOG:
		StreamInEvents(hwndDlg, (HANDLE)wParam, 1, 1, NULL);
		return 0;

	// replays queued events after the message log has been frozen for a while
	case DM_REPLAYQUEUE:
		for (int i=0; i < dat->iNextQueuedEvent; i++)
			if (dat->hQueuedEvents[i] != 0)
				StreamInEvents(hwndDlg, dat->hQueuedEvents[i], 1, 1, NULL);

		dat->iNextQueuedEvent = 0;
		SetDlgItemText(hwndDlg, IDC_LOGFROZENTEXT, dat->bNotOnList ? TranslateT("Contact not on list. You may add it...") :
			TranslateT("Auto scrolling is disabled (press F12 to enable it)"));
		return 0;

	case DM_SCROLLIEVIEW:
		{
			IEVIEWWINDOW iew = { sizeof(iew) };
			iew.iType = IEW_SCROLLBOTTOM;
			if (dat->hwndIEView) {
				iew.hwnd = dat->hwndIEView;
				CallService(MS_IEVIEW_WINDOW, 0, (LPARAM)&iew);
			}
			else if (dat->hwndHPP) {
				iew.hwnd = dat->hwndHPP;
				CallService(MS_HPP_EG_WINDOW, 0, (LPARAM)&iew);
			}
		}
		return 0;

	case DM_FORCESCROLL:
		{
			SCROLLINFO *psi = (SCROLLINFO *)lParam;
			POINT *ppt = (POINT *)wParam;

			HWND hwnd = GetDlgItem(hwndDlg, IDC_LOG);
			int len;

			if (wParam == 0 && lParam == 0) {
				DM_ScrollToBottom(dat, 0, 1);
				return 0;
			}

			if (dat->hwndIEView == 0 && dat->hwndHPP == 0) {
				len = GetWindowTextLengthA(GetDlgItem(hwndDlg, IDC_LOG));
				SendDlgItemMessage(hwndDlg, IDC_LOG, EM_SETSEL, len - 1, len - 1);
			}

			if (psi == NULL) {
				DM_ScrollToBottom(dat, 0, 0);
				return 0;
			}

			if ((UINT)psi->nPos >= (UINT)psi->nMax - psi->nPage - 5 || psi->nMax - psi->nMin - psi->nPage < 50)
				DM_ScrollToBottom(dat, 0, 0);
			else
				SendMessage((dat->hwndIEView || dat->hwndHPP) ? (dat->hwndIEView ? dat->hwndIEView : dat->hwndHPP) : hwnd, EM_SETSCROLLPOS, 0, (LPARAM)ppt);
		}
		return 0;
	
	case HM_DBEVENTADDED:
		// this is called whenever a new event has been added to the database.
		// this CAN be posted (some sanity checks required).
		if (!dat)
			return 0;
		if (dat->hContact)
			DM_EventAdded(dat, dat->hContact, lParam);
		return 0;

	case WM_TIMER:
		// timer to control info panel hovering
		if (wParam == TIMERID_AWAYMSG) {
			KillTimer(hwndDlg, wParam);
			GetCursorPos(&pt);

			if (wParam == TIMERID_AWAYMSG && dat->Panel->hitTest(pt) != CInfoPanel::HTNIRVANA)
				SendMessage(hwndDlg, DM_ACTIVATETOOLTIP, 0, 0);
			else
				dat->dwFlagsEx &= ~MWF_SHOW_AWAYMSGTIMER;
			break;
		}

		// timer id for message timeouts is composed like:
		// for single message sends: basevalue (TIMERID_MSGSEND) + send queue index
		if (wParam >= TIMERID_MSGSEND) {
			int iIndex = wParam - TIMERID_MSGSEND;
			if (iIndex < SendQueue::NR_SENDJOBS) {     // single sendjob timer
				SendJob *job = sendQueue->getJobByIndex(iIndex);
				KillTimer(hwndDlg, wParam);
				mir_sntprintf(job->szErrorMsg, SIZEOF(job->szErrorMsg), TranslateT("Delivery failure: %s"),
					TranslateT("The message send timed out"));
				job->iStatus = SendQueue::SQ_ERROR;
				if (!nen_options.iNoSounds && !(m_pContainer->dwFlags & CNT_NOSOUND))
					SkinPlaySound("SendError");
				if (!(dat->dwFlags & MWF_ERRORSTATE))
					sendQueue->handleError(dat, iIndex);
				break;
			}
		}
		else if (wParam == TIMERID_FLASHWND) {
			if (dat->mayFlashTab)
				FlashTab(dat, hwndTab, dat->iTabID, &dat->bTabFlash, TRUE, dat->hTabIcon);
			break;
		}
		else if (wParam == TIMERID_TYPE) {
			DM_Typing(dat);
			break;
		}
		break;

	case DM_ERRORDECIDED:
		switch (wParam) {
		case MSGERROR_CANCEL:
		case MSGERROR_SENDLATER:
			if (dat->dwFlags & MWF_ERRORSTATE) {
				dat->cache->saveHistory(0, 0);
				if (wParam == MSGERROR_SENDLATER)
					sendQueue->doSendLater(dat->iCurrentQueueError, dat);							// to be implemented at a later time
				dat->iOpenJobs--;
				sendQueue->dec();
				if (dat->iCurrentQueueError >= 0 && dat->iCurrentQueueError < SendQueue::NR_SENDJOBS)
					sendQueue->clearJob(dat->iCurrentQueueError);
				dat->iCurrentQueueError = -1;
				sendQueue->showErrorControls(dat, FALSE);
				if (wParam != MSGERROR_CANCEL || (wParam == MSGERROR_CANCEL && lParam == 0))
					SetDlgItemText(hwndDlg, IDC_MESSAGE, _T(""));
				sendQueue->checkQueue(dat);
				int iNextFailed = sendQueue->findNextFailed(dat);
				if (iNextFailed >= 0)
					sendQueue->handleError(dat, iNextFailed);
			}
			break;

		case MSGERROR_RETRY:
			if (dat->dwFlags & MWF_ERRORSTATE) {
				int resent = 0;

				dat->cache->saveHistory(0, 0);
				if (dat->iCurrentQueueError >= 0 && dat->iCurrentQueueError < SendQueue::NR_SENDJOBS) {
					SendJob *job = sendQueue->getJobByIndex(dat->iCurrentQueueError);
					if (job->hSendId == 0 && job->hContact == 0)
						break;

					job->hSendId = (HANDLE)CallContactService(job->hContact, PSS_MESSAGE,
						(dat->sendMode & SMODE_FORCEANSI) ? (job->dwFlags & ~PREF_UNICODE) : job->dwFlags, (LPARAM)job->szSendBuffer);
					resent++;
				}

				if (resent) {
					int iNextFailed;
					SendJob *job = sendQueue->getJobByIndex(dat->iCurrentQueueError);

					SetTimer(hwndDlg, TIMERID_MSGSEND + dat->iCurrentQueueError, PluginConfig.m_MsgTimeout, NULL);
					job->iStatus = SendQueue::SQ_INPROGRESS;
					dat->iCurrentQueueError = -1;
					sendQueue->showErrorControls(dat, FALSE);
					SetDlgItemText(hwndDlg, IDC_MESSAGE, _T(""));
					sendQueue->checkQueue(dat);
					if ((iNextFailed = sendQueue->findNextFailed(dat)) >= 0)
						sendQueue->handleError(dat, iNextFailed);
				}
			}
		}
		break;

	case DM_SELECTTAB:
		SendMessage(hwndContainer, DM_SELECTTAB, wParam, lParam);       // pass the msg to our container
		return 0;

	case DM_SETLOCALE:
		if (dat->dwFlags & MWF_WASBACKGROUNDCREATE)
			break;
		if (m_pContainer->hwndActive == hwndDlg && PluginConfig.m_AutoLocaleSupport && hwndContainer == GetForegroundWindow() && hwndContainer == GetActiveWindow()) {
			if (lParam)
				dat->hkl = (HKL)lParam;

			if (dat->hkl)
				ActivateKeyboardLayout(dat->hkl, 0);
		}
		return 0;

	// return timestamp (in ticks) of last recent message which has not been read yet.
	// 0 if there is none
	// lParam = pointer to a dword receiving the value.
	case DM_QUERYLASTUNREAD:
		{
			DWORD *pdw = (DWORD *)lParam;
			if (pdw)
				*pdw = dat->dwTickLastEvent;
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
				*phContact = dat->hContact;
		}
		return 0;

	case DM_UPDATELASTMESSAGE:
		DM_UpdateLastMessage(dat);
		return 0;

	case DM_SAVESIZE:
		if (dat->dwFlags & MWF_NEEDCHECKSIZE)
			lParam = 0;

		dat->dwFlags &= ~MWF_NEEDCHECKSIZE;
		if (dat->dwFlags & MWF_WASBACKGROUNDCREATE) {
			dat->dwFlags &= ~MWF_INITMODE;
			if (dat->lastMessage)
				DM_UpdateLastMessage(dat);
		}

		RECT rcClient;
		SendMessage(hwndContainer, DM_QUERYCLIENTAREA, 0, (LPARAM)&rcClient);
		MoveWindow(hwndDlg, rcClient.left, rcClient.top, (rcClient.right - rcClient.left), (rcClient.bottom - rcClient.top), TRUE);
		if (dat->dwFlags & MWF_WASBACKGROUNDCREATE) {
			dat->dwFlags &= ~MWF_WASBACKGROUNDCREATE;
			SendMessage(hwndDlg, WM_SIZE, 0, 0);
			PostMessage(hwndDlg, DM_UPDATEPICLAYOUT, 0, 0);
			if (PluginConfig.m_AutoLocaleSupport) {
				if (dat->hkl == 0)
					DM_LoadLocale(dat);
				else
					PostMessage(hwndDlg, DM_SETLOCALE, 0, 0);
			}
			if (dat->hwndIEView != 0)
				SetFocus(GetDlgItem(hwndDlg, IDC_MESSAGE));
			if (dat->pContainer->dwFlags & CNT_SIDEBAR)
				dat->pContainer->SideBar->Layout();
		}
		else {
			SendMessage(hwndDlg, WM_SIZE, 0, 0);
			if (lParam == 0)
				PostMessage(hwndDlg, DM_FORCESCROLL, 0, 0);
		}
		return 0;

	case DM_CHECKSIZE:
		dat->dwFlags |= MWF_NEEDCHECKSIZE;
		return 0;

	// sent by the message input area hotkeys. just pass it to our container
	case DM_QUERYPENDING:
		SendMessage(hwndContainer, DM_QUERYPENDING, wParam, lParam);
		return 0;

	case WM_LBUTTONDOWN:
		GetCursorPos(&tmp);
		cur.x = (SHORT)tmp.x;
		cur.y = (SHORT)tmp.y;
		if (!dat->Panel->isHovered())
			SendMessage(hwndContainer, WM_NCLBUTTONDOWN, HTCAPTION, *((LPARAM*)(&cur)));
		break;

	case WM_LBUTTONUP:
		GetCursorPos(&tmp);
		if (dat->Panel->isHovered())
			dat->Panel->handleClick(tmp);
		else {
			cur.x = (SHORT)tmp.x;
			cur.y = (SHORT)tmp.y;
			SendMessage(hwndContainer, WM_NCLBUTTONUP, HTCAPTION, *((LPARAM*)(&cur)));
		}
		break;

	case WM_RBUTTONUP:
		{
			RECT rcPicture, rcPanelNick = {0};
			int menuID = 0;

			GetWindowRect(GetDlgItem(hwndDlg, IDC_CONTACTPIC), &rcPicture);
			rcPanelNick.left = rcPanelNick.right - 30;
			GetCursorPos(&pt);

			if (dat->Panel->invokeConfigDialog(pt))
				break;

			if (PtInRect(&rcPicture, pt))
				menuID = MENU_PICMENU;

			if ((menuID == MENU_PICMENU && ((dat->ace ? dat->ace->hbmPic : PluginConfig.g_hbmUnknown) || dat->hOwnPic) && dat->bShowAvatar != 0)) {
				HMENU submenu = GetSubMenu(m_pContainer->hMenuContext, menuID == MENU_PICMENU ? 1 : 11);
				GetCursorPos(&pt);
				MsgWindowUpdateMenu(dat, submenu, menuID);
				int iSelection = TrackPopupMenu(submenu, TPM_RETURNCMD, pt.x, pt.y, 0, hwndDlg, NULL);
				MsgWindowMenuHandler(dat, iSelection, menuID);
				break;
			}

			HMENU subMenu = GetSubMenu(m_pContainer->hMenuContext, 0);
			MsgWindowUpdateMenu(dat, subMenu, MENU_TABCONTEXT);

			int iSelection = TrackPopupMenu(subMenu, TPM_RETURNCMD, pt.x, pt.y, 0, hwndDlg, NULL);
			if (iSelection >= IDM_CONTAINERMENU) {
				char szIndex[10];
				char *szKey = "TAB_ContainersW";

				mir_snprintf(szIndex, SIZEOF(szIndex), "%d", iSelection - IDM_CONTAINERMENU);
				if (iSelection - IDM_CONTAINERMENU >= 0) {
					ptrT val(db_get_tsa(NULL, szKey, szIndex));
					if (val)
						SendMessage(hwndDlg, DM_CONTAINERSELECTED, 0, (LPARAM)val);
				}
				break;
			}
			MsgWindowMenuHandler(dat, iSelection, MENU_TABCONTEXT);
		}
		break;

	case WM_MOUSEMOVE:
		GetCursorPos(&pt);
		DM_DismissTip(dat, pt);
		dat->Panel->trackMouse(pt);
		break;

	case WM_MEASUREITEM:
		{
			LPMEASUREITEMSTRUCT lpmi = (LPMEASUREITEMSTRUCT) lParam;
			if (dat->Panel->isHovered()) {
				lpmi->itemHeight = 0;
				lpmi->itemWidth  = 6;
				return TRUE;
			}
		}
		return CallService(MS_CLIST_MENUMEASUREITEM, wParam, lParam);

	case WM_NCHITTEST:
		SendMessage(hwndContainer, WM_NCHITTEST, wParam, lParam);
		break;

	case WM_DRAWITEM:
		return MsgWindowDrawHandler(wParam, lParam, dat);

	case WM_APPCOMMAND:
		{
			DWORD cmd = GET_APPCOMMAND_LPARAM(lParam);
			if (cmd == APPCOMMAND_BROWSER_BACKWARD || cmd == APPCOMMAND_BROWSER_FORWARD) {
				SendMessage(hwndContainer, DM_SELECTTAB, cmd == APPCOMMAND_BROWSER_BACKWARD ? DM_SELECT_PREV : DM_SELECT_NEXT, 0);
				return 1;
			}
		}
		break;

	case WM_COMMAND:
		if (!dat)
			break;
		
		// custom button handling
		if (LOWORD(wParam) >= MIN_CBUTTONID && LOWORD(wParam) <= MAX_CBUTTONID) {
			BB_CustomButtonClick(dat, LOWORD(wParam), GetDlgItem(hwndDlg, LOWORD(wParam)), 0);
			break;
		}

		switch (LOWORD(wParam)) {
		case IDOK:
			if (dat->fEditNotesActive) {
				SendMessage(hwndDlg, DM_ACTIVATETOOLTIP, IDC_PIC, (LPARAM)TranslateT("You are editing the user notes. Click the button again or use the hotkey (default: Alt-N) to save the notes and return to normal messaging mode"));
				return 0;
			}
			else {
				// don't parse text formatting when the message contains curly braces - these are used by the rtf syntax
				// and the parser currently cannot handle them properly in the text - XXX needs to be fixed later.
				FINDTEXTEXA fi = { 0 };
				fi.chrg.cpMin = 0;
				fi.chrg.cpMax = -1;
				fi.lpstrText = "{";
				int final_sendformat = SendDlgItemMessageA(hwndDlg, IDC_MESSAGE, EM_FINDTEXTEX, FR_DOWN, (LPARAM)&fi) == -1 ? dat->SendFormat : 0;
				fi.lpstrText = "}";
				final_sendformat = SendDlgItemMessageA(hwndDlg, IDC_MESSAGE, EM_FINDTEXTEX, FR_DOWN, (LPARAM)&fi) == -1 ? final_sendformat : 0;

				if (GetSendButtonState(hwndDlg) == PBS_DISABLED)
					break;

				ptrA streamOut(Message_GetFromStream(GetDlgItem(hwndDlg, IDC_MESSAGE), dat, final_sendformat ? 0 : (CP_UTF8 << 16) | (SF_TEXT | SF_USECODEPAGE)));
				if (streamOut == NULL)
					break;

				ptrT decoded(mir_utf8decodeT(streamOut));
				if (decoded == NULL)
					break;

				char *utfResult = NULL;
				if (final_sendformat)
					DoRtfToTags(decoded, dat);
				rtrimt(decoded);
				int bufSize = WideCharToMultiByte(dat->codePage, 0, decoded, -1, dat->sendBuffer, 0, 0, 0);

				int memRequired = 0, flags = 0;
				if (!IsUtfSendAvailable(dat->hContact)) {
					flags |= PREF_UNICODE;
					memRequired = bufSize + ((lstrlenW(decoded) + 1) * sizeof(WCHAR));
				}
				else {
					flags |= PREF_UTF;
					utfResult = mir_utf8encodeT(decoded);
					memRequired = (int)(strlen(utfResult)) + 1;
				}

				// try to detect RTL
				HWND hwndEdit = GetDlgItem(hwndDlg, IDC_MESSAGE);
				SendMessage(hwndEdit, WM_SETREDRAW, FALSE, 0);

				PARAFORMAT2 pf2;
				ZeroMemory(&pf2, sizeof(PARAFORMAT2));
				pf2.cbSize = sizeof(pf2);
				pf2.dwMask = PFM_RTLPARA;
				SendMessage(hwndEdit, EM_SETSEL, 0, -1);
				SendMessage(hwndEdit, EM_GETPARAFORMAT, 0, (LPARAM)&pf2);
				if (pf2.wEffects & PFE_RTLPARA)
					if (SendQueue::RTL_Detect(decoded))
						flags |= PREF_RTL;

				SendMessage(hwndEdit, WM_SETREDRAW, TRUE, 0);
				SendMessage(hwndEdit, EM_SETSEL, -1, -1);
				InvalidateRect(hwndEdit, NULL, FALSE);

				if (memRequired > dat->iSendBufferSize) {
					dat->sendBuffer = (char *)mir_realloc(dat->sendBuffer, memRequired);
					dat->iSendBufferSize = memRequired;
				}
				if (utfResult) {
					CopyMemory(dat->sendBuffer, utfResult, memRequired);
					mir_free(utfResult);
				}
				else {
					WideCharToMultiByte(dat->codePage, 0, decoded, -1, dat->sendBuffer, bufSize, 0, 0);
					if (flags & PREF_UNICODE)
						CopyMemory(&dat->sendBuffer[bufSize], decoded, (lstrlenW(decoded) + 1) * sizeof(WCHAR));
				}

				if (memRequired == 0 || dat->sendBuffer[0] == 0)
					break;

				if (dat->sendMode & SMODE_CONTAINER && m_pContainer->hwndActive == hwndDlg && GetForegroundWindow() == hwndContainer) {
					int tabCount = TabCtrl_GetItemCount(hwndTab);
					ptrA szFromStream(Message_GetFromStream(GetDlgItem(hwndDlg, IDC_MESSAGE), dat, dat->SendFormat ? 0 : (CP_UTF8 << 16) | (SF_TEXT | SF_USECODEPAGE)));

					TCITEM tci = { 0 };
					tci.mask = TCIF_PARAM;
					for (int i=0; i < tabCount; i++) {
						TabCtrl_GetItem(hwndTab, i, &tci);
						// get the contact from the tabs lparam which hopefully is the tabs hwnd so we can get its userdata.... hopefully
						HWND contacthwnd = (HWND)tci.lParam;
						if (IsWindow(contacthwnd)) {
							// if the contact hwnd is the current contact then ignore it and let the normal code deal with the msg
							if (contacthwnd != hwndDlg) {
								SETTEXTEX stx = {ST_DEFAULT, CP_UTF8};
								// send the buffer to the contacts msg typing area
								SendDlgItemMessage(contacthwnd, IDC_MESSAGE, EM_SETTEXTEX, (WPARAM)&stx, (LPARAM)szFromStream);
								SendMessage(contacthwnd, WM_COMMAND, IDOK, 0);
							}
						}
					}
				}
				// END /all /MOD
				if (dat->nTypeMode == PROTOTYPE_SELFTYPING_ON)
					DM_NotifyTyping(dat, PROTOTYPE_SELFTYPING_OFF);

				DeletePopupsForContact(dat->hContact, PU_REMOVE_ON_SEND);
				if (M.GetByte("allow_sendhook", 0)) {
					int result = TABSRMM_FireEvent(dat->hContact, hwndDlg, MSG_WINDOW_EVT_CUSTOM, MAKELONG(flags, tabMSG_WINDOW_EVT_CUSTOM_BEFORESEND));
					if (result)
						return TRUE;
				}
				sendQueue->addTo(dat, memRequired, flags);
			}
			return TRUE;

		case IDC_QUOTE:
			{
				int iCharsPerLine = M.GetDword("quoteLineLength", 64);
				CHARRANGE sel;
				SETTEXTEX stx = {ST_SELECTION, 1200};

				HANDLE hDBEvent = 0;
				if (dat->hwndIEView || dat->hwndHPP) {                // IEView quoting support..
					TCHAR *selected = 0, *szQuoted = 0;

					IEVIEWEVENT event = { sizeof(event) };
					event.hContact = dat->hContact;
					event.dwFlags = 0;
					event.iType = IEE_GET_SELECTION;

					if (dat->hwndIEView) {
						event.hwnd = dat->hwndIEView;
						selected = (TCHAR*)CallService(MS_IEVIEW_EVENT, 0, (LPARAM)&event);
					}
					else {
						event.hwnd = dat->hwndHPP;
						selected = (TCHAR*)CallService(MS_HPP_EG_EVENT, 0, (LPARAM)&event);
					}

					if (selected != NULL) {
						szQuoted = QuoteText(selected, iCharsPerLine, 0);
						SendDlgItemMessage(hwndDlg, IDC_MESSAGE, EM_SETTEXTEX, (WPARAM)&stx, (LPARAM)szQuoted);
						if (szQuoted)
							mir_free(szQuoted);
						break;
					}
					else {
						hDBEvent = db_event_last(dat->hContact);
						goto quote_from_last;
					}
				}
				hDBEvent = dat->hDbEventLast;

quote_from_last:
				if (hDBEvent == NULL)
					break;

				SendDlgItemMessage(hwndDlg, IDC_LOG, EM_EXGETSEL, 0, (LPARAM)&sel);
				if (sel.cpMin == sel.cpMax) {
					DBEVENTINFO dbei = { sizeof(dbei) };
					dbei.cbBlob = db_event_getBlobSize(hDBEvent);
					TCHAR *szText = (TCHAR*)mir_alloc((dbei.cbBlob + 1) * sizeof(TCHAR));   //URLs are made one char bigger for crlf
					dbei.pBlob = (BYTE*)szText;
					db_event_get(hDBEvent, &dbei);
					int iSize = int(strlen((char*)dbei.pBlob)) + 1;

					bool iAlloced = false;
					TCHAR *szConverted;
					if (dbei.flags & DBEF_UTF) {
						szConverted = mir_utf8decodeW((char*)szText);
						iAlloced = true;
					}
					else {
						if (iSize != dbei.cbBlob)
							szConverted = (TCHAR*)&dbei.pBlob[iSize];
						else {
							szConverted = (TCHAR*)mir_alloc(sizeof(TCHAR) * iSize);
							iAlloced = true;
							MultiByteToWideChar(CP_ACP, 0, (char*)dbei.pBlob, -1, szConverted, iSize);
						}
					}
					if (dbei.eventType == EVENTTYPE_FILE) {
						int iDescr = lstrlenA((char *)(szText + sizeof(DWORD)));
						MoveMemory(szText, szText + sizeof(DWORD), iDescr);
						MoveMemory(szText + iDescr + 2, szText + sizeof(DWORD)+iDescr, dbei.cbBlob - iDescr - sizeof(DWORD)-1);
						szText[iDescr] = '\r';
						szText[iDescr + 1] = '\n';
						szConverted = (TCHAR*)mir_alloc(sizeof(TCHAR)* (1 + lstrlenA((char *)szText)));
						MultiByteToWideChar(CP_ACP, 0, (char *)szText, -1, szConverted, 1 + lstrlenA((char *)szText));
						iAlloced = true;
					}
					ptrT szQuoted(QuoteText(szConverted, iCharsPerLine, 0));
					SendDlgItemMessage(hwndDlg, IDC_MESSAGE, EM_SETTEXTEX, (WPARAM)&stx, (LPARAM)szQuoted);
					mir_free(szText);
					if (iAlloced)
						mir_free(szConverted);
				}
				else {
					ptrA szFromStream(Message_GetFromStream(GetDlgItem(hwndDlg, IDC_LOG), dat, SF_TEXT | SF_USECODEPAGE | SFF_SELECTION));
					ptrW converted(mir_utf8decodeW(szFromStream));
					Utils::FilterEventMarkers(converted);
					ptrT szQuoted(QuoteText(converted, iCharsPerLine, 0));
					SendDlgItemMessage(hwndDlg, IDC_MESSAGE, EM_SETTEXTEX, (WPARAM)&stx, (LPARAM)szQuoted);
				}
				SetFocus(GetDlgItem(hwndDlg, IDC_MESSAGE));
			}
			break;

		case IDC_ADD:
			{
				ADDCONTACTSTRUCT acs = { 0 };
				acs.hContact = dat->hContact;
				acs.handleType = HANDLE_CONTACT;
				acs.szProto = 0;
				CallService(MS_ADDCONTACT_SHOW, (WPARAM)hwndDlg, (LPARAM)&acs);
				if (!db_get_b(dat->hContact, "CList", "NotOnList", 0)) {
					dat->bNotOnList = FALSE;
					ShowMultipleControls(hwndDlg, addControls, 2, SW_HIDE);
					if (!(dat->dwFlagsEx & MWF_SHOW_SCROLLINGDISABLED))
						Utils::showDlgControl(hwndDlg, IDC_LOGFROZENTEXT, SW_HIDE);
					SendMessage(hwndDlg, WM_SIZE, 0, 0);
				}
			}
			break;

		case IDC_CANCELADD:
			dat->bNotOnList = FALSE;
			ShowMultipleControls(hwndDlg, addControls, 2, SW_HIDE);
			if (!(dat->dwFlagsEx & MWF_SHOW_SCROLLINGDISABLED))
				Utils::showDlgControl(hwndDlg, IDC_LOGFROZENTEXT, SW_HIDE);
			SendMessage(hwndDlg, WM_SIZE, 0, 0);
			break;

		case IDC_MESSAGE:
			if (HIWORD(wParam) == EN_CHANGE) {
				if (m_pContainer->hwndActive == hwndDlg)
					UpdateReadChars(dat);
				dat->dwFlags |= MWF_NEEDHISTORYSAVE;
				dat->dwLastActivity = GetTickCount();
				m_pContainer->dwLastActivity = dat->dwLastActivity;
				SendQueue::UpdateSaveAndSendButton(dat);
				if (!(GetKeyState(VK_CONTROL) & 0x8000)) {
					dat->nLastTyping = GetTickCount();
					if (GetWindowTextLength(GetDlgItem(hwndDlg, IDC_MESSAGE))) {
						if (dat->nTypeMode == PROTOTYPE_SELFTYPING_OFF) {
							if (!(dat->dwFlags & MWF_INITMODE))
								DM_NotifyTyping(dat, PROTOTYPE_SELFTYPING_ON);
						}
					}
					else if (dat->nTypeMode == PROTOTYPE_SELFTYPING_ON)
						DM_NotifyTyping(dat, PROTOTYPE_SELFTYPING_OFF);
				}
			}
			break;

		default:
			Utils::CmdDispatcher(Utils::CMD_MSGDIALOG, hwndDlg, LOWORD(wParam), wParam, lParam, dat, m_pContainer);
			break;
		}
		break;

	case WM_CONTEXTMENU:
		{
			DWORD idFrom = GetDlgCtrlID((HWND)wParam);
			if (idFrom >= MIN_CBUTTONID && idFrom <= MAX_CBUTTONID) {
				BB_CustomButtonClick(dat, idFrom, (HWND)wParam, 1);
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
		sendQueue->ackMessage(dat, wParam, lParam);
		return 0;

	case DM_ACTIVATEME:
		ActivateExistingTab(m_pContainer, hwndDlg);
		return 0;

	// sent by the select container dialog box when a container was selected...
	// lParam = (TCHAR*)selected name...
	case DM_CONTAINERSELECTED:
		{
			TCHAR *szNewName = (TCHAR*)lParam;
			if (!_tcscmp(szNewName, TranslateT("Default container")))
				szNewName = CGlobals::m_default_container_name;

			int iOldItems = TabCtrl_GetItemCount(hwndTab);
			if (!_tcsncmp(m_pContainer->szName, szNewName, CONTAINER_NAMELEN))
				break;

			TContainerData *pNewContainer = FindContainerByName(szNewName);
			if (pNewContainer == NULL)
				if ((pNewContainer = CreateContainer(szNewName, FALSE, dat->hContact)) == NULL)
					break;

			db_set_ts(dat->hContact, SRMSGMOD_T, "containerW", szNewName);
			dat->fIsReattach = TRUE;
			PostMessage(PluginConfig.g_hwndHotkeyHandler, DM_DOCREATETAB, (WPARAM)pNewContainer, dat->hContact);
			if (iOldItems > 1)                // there were more than 1 tab, container is still valid
				SendMessage(m_pContainer->hwndActive, WM_SIZE, 0, 0);
			SetForegroundWindow(pNewContainer->hwnd);
			SetActiveWindow(pNewContainer->hwnd);
		}
		break;

	case DM_STATUSBARCHANGED:
		UpdateStatusBar(dat);
		return 0;

	case DM_UINTOCLIPBOARD: 
		Utils::CopyToClipBoard(const_cast<TCHAR *>(dat->cache->getUIN()), hwndDlg);
		return 0;
	
	// broadcasted when GLOBAL info panel setting changes
	case DM_SETINFOPANEL:
		CInfoPanel::setPanelHandler(dat, wParam, lParam);
		return 0;

	// show the balloon tooltip control.
	// wParam == id of the "anchor" element, defaults to the panel status field (for away msg retrieval)
	// lParam == new text to show
	case DM_ACTIVATETOOLTIP:
		if (IsIconic(hwndContainer) || m_pContainer->hwndActive != hwndDlg)
			break;

		dat->Panel->showTip(wParam, lParam);
		break;

	case WM_NEXTDLGCTL:
		if (dat->dwFlags & MWF_WASBACKGROUNDCREATE)
			return 1;
		break;
		
	// save the contents of the log as rtf file
	case DM_SAVEMESSAGELOG:
		DM_SaveLogAsRTF(dat);
		return 0;

	case DM_CHECKAUTOHIDE:
		DM_CheckAutoHide(dat, wParam, lParam);
		return 0;

	case DM_IEVIEWOPTIONSCHANGED:
		if (dat->hwndIEView)
			SendMessage(hwndDlg, DM_REMAKELOG, 0, 0);
		break;

	case DM_SMILEYOPTIONSCHANGED:
		ConfigureSmileyButton(dat);
		SendMessage(hwndDlg, DM_REMAKELOG, 0, 0);
		break;

	case DM_PROTOAVATARCHANGED:
		dat->ace = Utils::loadAvatarFromAVS(dat->hContact);
		dat->panelWidth = -1;				// force new size calculations
		ShowPicture(dat, TRUE);
		if (dat->Panel->isActive())
			SendMessage(hwndDlg, WM_SIZE, 0, 0);
		return 0;

	case DM_MYAVATARCHANGED:
		{
			const char *szProto = dat->cache->getActiveProto();
			if (!strcmp((char *)wParam, szProto) && lstrlenA(szProto) == lstrlenA((char *)wParam))
				LoadOwnAvatar(dat);
		}
		break;

	case DM_GETWINDOWSTATE:
		{
			UINT state = MSG_WINDOW_STATE_EXISTS;
			if (IsWindowVisible(hwndDlg))
				state |= MSG_WINDOW_STATE_VISIBLE;
			if (GetForegroundWindow() == hwndContainer)
				state |= MSG_WINDOW_STATE_FOCUS;
			if (IsIconic(hwndContainer))
				state |= MSG_WINDOW_STATE_ICONIC;
			SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, state);
		}
		return TRUE;

	case DM_CLIENTCHANGED:
		GetClientIcon(dat);
		if (dat->hClientIcon && dat->Panel->isActive())
			InvalidateRect(hwndDlg, NULL, TRUE);
		return 0;

	case DM_UPDATEUIN:
		if (dat->Panel->isActive())
			dat->Panel->Invalidate();
		if (dat->pContainer->dwFlags & CNT_UINSTATUSBAR)
			UpdateStatusBar(dat);
		return 0;

	case DM_REMOVEPOPUPS:
		DeletePopupsForContact(dat->hContact, (DWORD)wParam);
		return 0;

	case EM_THEMECHANGED:
		DM_FreeTheme(dat);
		return DM_ThemeChanged(dat);

	case DM_PLAYINCOMINGSOUND:
		if (!dat)
			return 0;
		PlayIncomingSound(dat);
		return 0;

	case DM_REFRESHTABINDEX:
		dat->iTabID = GetTabIndexFromHWND(GetParent(hwndDlg), hwndDlg);
		return 0;

	case DM_STATUSICONCHANGE:
		if (m_pContainer->hwndStatus) {
			SendMessage(dat->pContainer->hwnd, WM_SIZE, 0, 0);
			SendMessage(m_pContainer->hwndStatus, SB_SETTEXT, (WPARAM)(SBT_OWNERDRAW) | 2, 0);
			InvalidateRect(m_pContainer->hwndStatus, NULL, TRUE);
		}
		return 0;

	case DM_BBNEEDUPDATE:
		if (lParam)
			CB_ChangeButton(hwndDlg, dat, (CustomButtonData*)lParam);
		else
			BB_InitDlgButtons(dat);

		BB_SetButtonsPos(dat);
		return 0;

	case DM_CBDESTROY:
		if (lParam)
			CB_DestroyButton(hwndDlg, dat, (DWORD)wParam, (DWORD)lParam);
		else
			CB_DestroyAllButtons(hwndDlg, dat);
		return 0;

	case WM_DROPFILES:
		{
			BOOL not_sending = GetKeyState(VK_CONTROL) & 0x8000;
			if (!not_sending) {
				const char *szProto = dat->cache->getActiveProto();
				if (szProto == NULL)
					break;

				int pcaps = CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_1, 0);
				if (!(pcaps & PF1_FILESEND))
					break;
				if (dat->wStatus == ID_STATUS_OFFLINE) {
					pcaps = CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_4, 0);
					if (!(pcaps & PF4_OFFLINEFILES)) {
						TCHAR szBuffer[256];

						mir_sntprintf(szBuffer, SIZEOF(szBuffer), TranslateT("Contact is offline and this protocol does not support sending files to offline users."));
						SendMessage(hwndDlg, DM_ACTIVATETOOLTIP, IDC_MESSAGE, (LPARAM)szBuffer);
						break;
					}
				}
			}

			if (dat->hContact != NULL) {
				TCHAR szFilename[MAX_PATH];
				HDROP hDrop = (HDROP)wParam;
				int fileCount = DragQueryFile(hDrop, -1, NULL, 0), totalCount = 0, i;
				TCHAR** ppFiles = NULL;
				for (i = 0; i < fileCount; i++) {
					DragQueryFile(hDrop, i, szFilename, SIZEOF(szFilename));
					Utils::AddToFileList(&ppFiles, &totalCount, szFilename);
				}

				if (!not_sending)
					CallService(MS_FILE_SENDSPECIFICFILEST, dat->hContact, (LPARAM)ppFiles);
				else {
					if (ServiceExists(MS_HTTPSERVER_ADDFILENAME)) {
						for (int i = 0; i < totalCount; i++) {
							char* szFileName = mir_t2a(ppFiles[i]);
							char *szTemp = (char*)CallService(MS_HTTPSERVER_ADDFILENAME, (WPARAM)szFileName, 0);
							mir_free(szFileName);
						}
						char *szHTTPText = "DEBUG";
						SendDlgItemMessageA(hwndDlg, IDC_MESSAGE, EM_REPLACESEL, TRUE, (LPARAM)szHTTPText);
						SetFocus(GetDlgItem(hwndDlg, IDC_MESSAGE));
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
				*uOpen += dat->iOpenJobs;
		}
		return 0;

	case WM_CLOSE:
		// esc handles error controls if we are in error state (error controls visible)
		if (wParam == 0 && lParam == 0 && dat->dwFlags & MWF_ERRORSTATE) {
			SendMessage(hwndDlg, DM_ERRORDECIDED, MSGERROR_CANCEL, 0);
			return TRUE;
		}

		if (wParam == 0 && lParam == 0) {
			if (PluginConfig.m_EscapeCloses == 1) {
				SendMessage(hwndContainer, WM_SYSCOMMAND, SC_MINIMIZE, 0);
				return TRUE;
			}
			else if (PluginConfig.m_HideOnClose && PluginConfig.m_EscapeCloses == 2) {
				ShowWindow(hwndContainer, SW_HIDE);
				return TRUE;
			}
			_dlgReturn(hwndDlg, TRUE);
		}
		{
			TContainerData *pContainer = dat->pContainer;
			if (dat->iOpenJobs > 0 && lParam != 2) {
				if (dat->dwFlags & MWF_ERRORSTATE)
					SendMessage(hwndDlg, DM_ERRORDECIDED, MSGERROR_CANCEL, 1);
				else if (dat) {
					if (dat->dwFlagsEx & MWF_EX_WARNCLOSE)
						return TRUE;

					dat->dwFlagsEx |= MWF_EX_WARNCLOSE;
					LRESULT result = SendQueue::WarnPendingJobs(0);
					dat->dwFlagsEx &= ~MWF_EX_WARNCLOSE;
					if (result == IDNO)
						return TRUE;
				}
			}
			int iTabs = TabCtrl_GetItemCount(hwndTab);
			if (iTabs == 1) {
				PostMessage(hwndContainer, WM_CLOSE, 0, 1);
				return 1;
			}

			m_pContainer->iChilds--;
			int i = GetTabIndexFromHWND(hwndTab, hwndDlg);

			// after closing a tab, we need to activate the tab to the left side of
			// the previously open tab.
			// normally, this tab has the same index after the deletion of the formerly active tab
			// unless, of course, we closed the last (rightmost) tab.
			if (!m_pContainer->bDontSmartClose && iTabs > 1 && lParam != 3) {
				if (i == iTabs - 1)
					i--;
				else
					i++;
				TabCtrl_SetCurSel(hwndTab, i);

				TCITEM item = { 0 };
				item.mask = TCIF_PARAM;
				TabCtrl_GetItem(hwndTab, i, &item);         // retrieve dialog hwnd for the now active tab...

				m_pContainer->hwndActive = (HWND)item.lParam;

				RECT rc;
				SendMessage(hwndContainer, DM_QUERYCLIENTAREA, 0, (LPARAM)&rc);
				SetWindowPos(m_pContainer->hwndActive, HWND_TOP, rc.left, rc.top, (rc.right - rc.left), (rc.bottom - rc.top), SWP_SHOWWINDOW);
				ShowWindow((HWND)item.lParam, SW_SHOW);
				SetForegroundWindow(m_pContainer->hwndActive);
				SetFocus(m_pContainer->hwndActive);
				SendMessage(hwndContainer, WM_SIZE, 0, 0);
			}

			DestroyWindow(hwndDlg);
			if (iTabs == 1)
				PostMessage(GetParent(GetParent(hwndDlg)), WM_CLOSE, 0, 1);
			else
				SendMessage(pContainer->hwnd, WM_SIZE, 0, 0);
		}
		break;

	case WM_DESTROY:
		if (!dat)
			break;

		if (dat->hwndContactPic)
			DestroyWindow(dat->hwndContactPic);

		if (dat->hwndPanelPic)
			DestroyWindow(dat->hwndPanelPic);

		if (dat->hClientIcon)
			DestroyIcon(dat->hClientIcon);

		if (dat->hwndPanelPicParent)
			DestroyWindow(dat->hwndPanelPicParent);

		if (dat->cache->isValid()) { // not valid means the contact was deleted
			TABSRMM_FireEvent(dat->hContact, hwndDlg, MSG_WINDOW_EVT_CLOSING, 0);
			AddContactToFavorites(dat->hContact, dat->cache->getNick(), dat->cache->getActiveProto(), dat->szStatus, dat->wStatus,
				LoadSkinnedProtoIcon(dat->cache->getActiveProto(), dat->cache->getActiveStatus()), 1, PluginConfig.g_hMenuRecent);
			if (dat->hContact) {
				if (!dat->fEditNotesActive) {
					char *msg = Message_GetFromStream(GetDlgItem(hwndDlg, IDC_MESSAGE), dat, (CP_UTF8 << 16) | (SF_TEXT | SF_USECODEPAGE));
					if (msg) {
						db_set_utf(dat->hContact, SRMSGMOD, "SavedMsg", msg);
						mir_free(msg);
					}
					else db_unset(dat->hContact, SRMSGMOD, "SavedMsg");
				}
				else SendMessage(hwndDlg, WM_COMMAND, IDC_PIC, 0);
			}
		}

		if (dat->nTypeMode == PROTOTYPE_SELFTYPING_ON)
			DM_NotifyTyping(dat, PROTOTYPE_SELFTYPING_OFF);

		DM_FreeTheme(dat);

		mir_free(dat->sendBuffer);
		mir_free(dat->hHistoryEvents);

		// search the sendqueue for unfinished send jobs and mir_free them. Leave unsent
		// messages in the queue as they can be acked later
		{
			SendJob *jobs = sendQueue->getJobByIndex(0);

			for (int i = 0; i < SendQueue::NR_SENDJOBS; i++) {
				if (jobs[i].hContact == dat->hContact) {
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

		mir_free(dat->hQueuedEvents);

		if (dat->hSmileyIcon)
			DestroyIcon(dat->hSmileyIcon);

		if (dat->hXStatusIcon)
			DestroyIcon(dat->hXStatusIcon);

		if (dat->hwndTip)
			DestroyWindow(dat->hwndTip);

		if (dat->hTaskbarIcon)
			DestroyIcon(dat->hTaskbarIcon);

		UpdateTrayMenuState(dat, FALSE);               // remove me from the tray menu (if still there)
		if (PluginConfig.g_hMenuTrayUnread)
			DeleteMenu(PluginConfig.g_hMenuTrayUnread, (UINT_PTR)dat->hContact, MF_BYCOMMAND);
		M.RemoveWindow(hwndDlg);

		if (dat->cache->isValid())
			db_set_dw(0, SRMSGMOD, "multisplit", dat->multiSplitterX);

		{
			HFONT hFont = (HFONT)SendDlgItemMessage(hwndDlg, IDC_MESSAGE, WM_GETFONT, 0, 0);
			if (hFont != NULL && hFont != (HFONT)SendDlgItemMessage(hwndDlg, IDOK, WM_GETFONT, 0, 0))
				DeleteObject(hFont);

			TCITEM item = { 0 };
			item.mask = TCIF_PARAM;

			int i = GetTabIndexFromHWND(hwndTab, hwndDlg);
			if (i >= 0) {
				SendMessage(hwndTab, WM_USER + 100, 0, 0);                      // remove tooltip
				TabCtrl_DeleteItem(hwndTab, i);
				BroadCastContainer(m_pContainer, DM_REFRESHTABINDEX, 0, 0);
				dat->iTabID = -1;
			}
		}

		TABSRMM_FireEvent(dat->hContact, hwndDlg, MSG_WINDOW_EVT_CLOSE, 0);

		// clean up IEView and H++ log windows
		if (dat->hwndIEView != 0) {
			IEVIEWWINDOW ieWindow;
			ieWindow.cbSize = sizeof(IEVIEWWINDOW);
			ieWindow.iType = IEW_DESTROY;
			ieWindow.hwnd = dat->hwndIEView;
			CallService(MS_IEVIEW_WINDOW, 0, (LPARAM)&ieWindow);
		}
		if (dat->hwndHPP) {
			IEVIEWWINDOW ieWindow;
			ieWindow.cbSize = sizeof(IEVIEWWINDOW);
			ieWindow.iType = IEW_DESTROY;
			ieWindow.hwnd = dat->hwndHPP;
			CallService(MS_HPP_EG_WINDOW, 0, (LPARAM)&ieWindow);
		}
		if (dat->pWnd) {
			delete dat->pWnd;
			dat->pWnd = 0;
		}
		if (dat->sbCustom) {
			delete dat->sbCustom;
			dat->sbCustom = 0;
		}
		break;

	case WM_DWMCOMPOSITIONCHANGED:
		BB_RefreshTheme(dat);
		memset((void*)&dat->pContainer->mOld, -1000, sizeof(MARGINS));
		CProxyWindow::verify(dat);
		break;

	case DM_FORCEREDRAW:
		RedrawWindow(hwndDlg, NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW);
		return 0;

	case DM_CHECKINFOTIP:
		dat->Panel->hideTip(reinterpret_cast<HWND>(lParam));
		return 0;

	case WM_NCDESTROY:
		if (dat) {
			memset((void*)&dat->pContainer->mOld, -1000, sizeof(MARGINS));
			PostMessage(dat->pContainer->hwnd, WM_SIZE, 0, 1);
			if (m_pContainer->dwFlags & CNT_SIDEBAR)
				m_pContainer->SideBar->removeSession(dat);
			dat->cache->setWindowData();
			if (dat->cache->isValid() && !dat->fIsReattach && dat->hContact && M.GetByte("deletetemp", 0))
				if (db_get_b(dat->hContact, "CList", "NotOnList", 0))
					CallService(MS_DB_CONTACT_DELETE, dat->hContact, 0);

			delete dat->Panel;
			mir_free(dat);
		}
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, 0);
		break;
	}
	return FALSE;
}
