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
// these are generic message handlers which are used by the message dialog window procedure.
// calling them directly instead of using SendMessage() is faster.
// also contains various callback functions for custom buttons

#include "stdafx.h"

/////////////////////////////////////////////////////////////////////////////////////////
// Save message log for given session as RTF document

void CTabBaseDlg::DM_SaveLogAsRTF() const
{
	if (m_hwndIEView != 0) {
		IEVIEWEVENT event = { sizeof(event) };
		event.hwnd = m_hwndIEView;
		event.hContact = m_hContact;
		event.iType = IEE_SAVE_DOCUMENT;
		CallService(MS_IEVIEW_EVENT, 0, (LPARAM)&event);
	}
	else {
		wchar_t szFilter[MAX_PATH], szFilename[MAX_PATH];
		mir_snwprintf(szFilter, L"%s%c*.rtf%c%c", TranslateT("Rich Edit file"), 0, 0, 0);
		mir_snwprintf(szFilename, L"%s.rtf", m_cache->getNick());

		Utils::sanitizeFilename(szFilename);

		wchar_t szInitialDir[MAX_PATH + 2];
		mir_snwprintf(szInitialDir, L"%s%s\\", M.getDataPath(), L"\\Saved message logs");
		CreateDirectoryTreeW(szInitialDir);

		OPENFILENAME ofn = { 0 };
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = m_hwnd;
		ofn.lpstrFile = szFilename;
		ofn.lpstrFilter = szFilter;
		ofn.lpstrInitialDir = szInitialDir;
		ofn.nMaxFile = MAX_PATH;
		ofn.Flags = OFN_HIDEREADONLY;
		ofn.lpstrDefExt = L"rtf";
		if (GetSaveFileName(&ofn)) {
			EDITSTREAM stream = { 0 };
			stream.dwCookie = (DWORD_PTR)szFilename;
			stream.dwError = 0;
			stream.pfnCallback = Utils::StreamOut;
			SendDlgItemMessage(m_hwnd, IDC_LOG, EM_STREAMOUT, SF_RTF | SF_USECODEPAGE, (LPARAM)&stream);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// This is broadcasted by the container to all child windows to check if the
// container can be autohidden or -closed.
//
// wParam is the autohide timeout (in seconds)
// lParam points to a BOOL and a session which wants to prevent auto-hiding
// the container must set it to FALSE.
//
// If no session in the container disagrees, the container will be hidden.

void CTabBaseDlg::DM_CheckAutoHide(WPARAM wParam, LPARAM lParam) const
{
	if (!lParam)
		return;

	BOOL *fResult = (BOOL*)lParam;
	if (GetWindowTextLength(GetDlgItem(m_hwnd, IDC_MESSAGE)) > 0) {
		*fResult = FALSE;
		return;				// text entered in the input area -> prevent autohide/cose
	}
	if (m_dwUnread) {
		*fResult = FALSE;
		return;				// unread events, do not hide or close the container
	}
	if (((GetTickCount() - m_dwLastActivity) / 1000) <= wParam)
		*fResult = FALSE;		// time since last activity did not yet reach the threshold.
}

/////////////////////////////////////////////////////////////////////////////////////////
// checks if the balloon tooltip can be dismissed (usually called by WM_MOUSEMOVE events)

void CTabBaseDlg::DM_DismissTip(const POINT& pt)
{
	if (!IsWindowVisible(m_hwndTip))
		return;

	RECT rc;
	GetWindowRect(m_hwndTip, &rc);
	if (PtInRect(&rc, pt))
		return;

	if (abs(pt.x - m_ptTipActivation.x) > 5 || abs(pt.y - m_ptTipActivation.y) > 5) {
		SendMessage(m_hwndTip, TTM_TRACKACTIVATE, FALSE, 0);
		m_ptTipActivation.x = m_ptTipActivation.y = 0;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// initialize the balloon tooltip for message window notifications

void CTabBaseDlg::DM_InitTip()
{
	m_hwndTip = CreateWindowEx(0, TOOLTIPS_CLASS, NULL, WS_POPUP | TTS_NOPREFIX | TTS_BALLOON, CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT, m_hwnd, NULL, g_hInst, (LPVOID)NULL);

	memset(&ti, 0, sizeof(ti));
	ti.cbSize = sizeof(ti);
	ti.lpszText = TranslateT("No status message");
	ti.hinst = g_hInst;
	ti.hwnd = m_hwnd;
	ti.uFlags = TTF_TRACK | TTF_IDISHWND | TTF_TRANSPARENT;
	ti.uId = (UINT_PTR)m_hwnd;
	SendMessage(m_hwndTip, TTM_ADDTOOL, 0, (LPARAM)&ti);

	SetWindowPos(m_hwndTip, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER);
}

/////////////////////////////////////////////////////////////////////////////////////////
// checks generic hotkeys valid for both IM and MUC sessions
//
// returns 1 for handled hotkeys, 0 otherwise.

bool CTabBaseDlg::DM_GenericHotkeysCheck(MSG *message)
{
	LRESULT mim_hotkey_check = CallService(MS_HOTKEY_CHECK, (WPARAM)message, (LPARAM)TABSRMM_HK_SECTION_GENERIC);

	switch (mim_hotkey_check) {
	case TABSRMM_HK_PASTEANDSEND:
		HandlePasteAndSend();
		return true;

	case TABSRMM_HK_HISTORY:
		SendMessage(m_hwnd, WM_COMMAND, IDC_HISTORY, 0);
		return true;

	case TABSRMM_HK_CONTAINEROPTIONS:
		if (m_pContainer->hWndOptions == 0)
			CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_CONTAINEROPTIONS), m_pContainer->hwnd, DlgProcContainerOptions, (LPARAM)m_pContainer);
		return true;

	case TABSRMM_HK_SEND:
		if (!(GetWindowLongPtr(GetDlgItem(m_hwnd, IDC_MESSAGE), GWL_STYLE) & ES_READONLY)) {
			PostMessage(m_hwnd, WM_COMMAND, IDOK, 0);
			return true;
		}
		break;

	case TABSRMM_HK_TOGGLEINFOPANEL:
		m_Panel->setActive(!m_Panel->isActive());
		m_Panel->showHide();
		return true;

	case TABSRMM_HK_TOGGLETOOLBAR:
		SendMessage(m_hwnd, WM_COMMAND, IDC_TOGGLETOOLBAR, 0);
		return true;

	case TABSRMM_HK_CLEARLOG:
		ClearLog();
		return true;

	case TABSRMM_HK_TOGGLESIDEBAR:
		if (m_pContainer->SideBar->isActive())
			SendMessage(m_hwnd, WM_COMMAND, IDC_TOGGLESIDEBAR, 0);
		return true;

	case TABSRMM_HK_CLOSE_OTHER:
		CloseOtherTabs(GetDlgItem(m_pContainer->hwnd, IDC_MSGTABS), *this);
		return true;
	}
	return false;
}

LRESULT CTabBaseDlg::DM_MsgWindowCmdHandler(UINT cmd, WPARAM wParam, LPARAM lParam)
{
	RECT  rc;
	int   iSelection;
	HMENU submenu;

	switch (cmd) {
	case IDC_FONTBOLD:
	case IDC_FONTITALIC:
	case IDC_FONTUNDERLINE:
	case IDC_FONTSTRIKEOUT:
		if (m_SendFormat != 0) { // dont use formatting if disabled
			CHARFORMAT2 cf, cfOld;
			memset(&cf, 0, sizeof(CHARFORMAT2));
			memset(&cfOld, 0, sizeof(CHARFORMAT2));
			cfOld.cbSize = cf.cbSize = sizeof(CHARFORMAT2);
			cfOld.dwMask = CFM_BOLD | CFM_ITALIC | CFM_UNDERLINE | CFM_STRIKEOUT;
			SendDlgItemMessage(m_hwnd, IDC_MESSAGE, EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM)&cfOld);
			BOOL isBold = (cfOld.dwEffects & CFE_BOLD) && (cfOld.dwMask & CFM_BOLD);
			BOOL isItalic = (cfOld.dwEffects & CFE_ITALIC) && (cfOld.dwMask & CFM_ITALIC);
			BOOL isUnderline = (cfOld.dwEffects & CFE_UNDERLINE) && (cfOld.dwMask & CFM_UNDERLINE);
			BOOL isStrikeout = (cfOld.dwEffects & CFM_STRIKEOUT) && (cfOld.dwMask & CFM_STRIKEOUT);

			int ctrlId = LOWORD(wParam);
			if (ctrlId == IDC_FONTBOLD && !IsWindowEnabled(GetDlgItem(m_hwnd, IDC_FONTBOLD)))
				break;
			if (ctrlId == IDC_FONTITALIC && !IsWindowEnabled(GetDlgItem(m_hwnd, IDC_FONTITALIC)))
				break;
			if (ctrlId == IDC_FONTUNDERLINE && !IsWindowEnabled(GetDlgItem(m_hwnd, IDC_FONTUNDERLINE)))
				break;
			if (ctrlId == IDC_FONTSTRIKEOUT && !IsWindowEnabled(GetDlgItem(m_hwnd, IDC_FONTSTRIKEOUT)))
				break;
			if (ctrlId == IDC_FONTBOLD) {
				cf.dwEffects = isBold ? 0 : CFE_BOLD;
				cf.dwMask = CFM_BOLD;
				CheckDlgButton(m_hwnd, IDC_FONTBOLD, !isBold ? BST_CHECKED : BST_UNCHECKED);
			}
			else if (ctrlId == IDC_FONTITALIC) {
				cf.dwEffects = isItalic ? 0 : CFE_ITALIC;
				cf.dwMask = CFM_ITALIC;
				CheckDlgButton(m_hwnd, IDC_FONTITALIC, !isItalic ? BST_CHECKED : BST_UNCHECKED);
			}
			else if (ctrlId == IDC_FONTUNDERLINE) {
				cf.dwEffects = isUnderline ? 0 : CFE_UNDERLINE;
				cf.dwMask = CFM_UNDERLINE;
				CheckDlgButton(m_hwnd, IDC_FONTUNDERLINE, !isUnderline ? BST_CHECKED : BST_UNCHECKED);
			}
			else if (ctrlId == IDC_FONTSTRIKEOUT) {
				cf.dwEffects = isStrikeout ? 0 : CFM_STRIKEOUT;
				cf.dwMask = CFM_STRIKEOUT;
				CheckDlgButton(m_hwnd, IDC_FONTSTRIKEOUT, !isStrikeout ? BST_CHECKED : BST_UNCHECKED);
			}
			SendDlgItemMessage(m_hwnd, IDC_MESSAGE, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
		}
		break;

	case IDCANCEL:
		ShowWindow(m_pContainer->hwnd, SW_MINIMIZE);
		return FALSE;

	case IDC_SAVE:
		SendMessage(m_hwnd, WM_CLOSE, 1, 0);
		break;

	case IDC_NAME:
		if (GetKeyState(VK_SHIFT) & 0x8000)   // copy UIN
			SendMessage(m_hwnd, DM_UINTOCLIPBOARD, 0, 0);
		else
			CallService(MS_USERINFO_SHOWDIALOG, (WPARAM)(m_cache->getActiveContact()), 0);
		break;

	case IDC_HISTORY:
		CallService(MS_HISTORY_SHOWCONTACTHISTORY, m_hContact, 0);
		break;

	case IDC_TIME:
		submenu = GetSubMenu(PluginConfig.g_hMenuContext, 2);
		MsgWindowUpdateMenu(submenu, MENU_LOGMENU);

		GetWindowRect(GetDlgItem(m_hwnd, IDC_TIME), &rc);

		iSelection = TrackPopupMenu(submenu, TPM_RETURNCMD, rc.left, rc.bottom, 0, m_hwnd, NULL);
		return MsgWindowMenuHandler(iSelection, MENU_LOGMENU);

	case IDC_PROTOMENU:
		if (m_hContact) {
			submenu = GetSubMenu(PluginConfig.g_hMenuContext, 4);
			int iOldGlobalSendFormat = PluginConfig.m_SendFormat;
			int iLocalFormat = M.GetDword(m_hContact, "sendformat", 0);
			int iNewLocalFormat = iLocalFormat;

			GetWindowRect(GetDlgItem(m_hwnd, IDC_PROTOCOL), &rc);

			CheckMenuItem(submenu, ID_MODE_GLOBAL, MF_BYCOMMAND | (!(m_dwFlagsEx & MWF_SHOW_SPLITTEROVERRIDE) ? MF_CHECKED : MF_UNCHECKED));
			CheckMenuItem(submenu, ID_MODE_PRIVATE, MF_BYCOMMAND | (m_dwFlagsEx & MWF_SHOW_SPLITTEROVERRIDE ? MF_CHECKED : MF_UNCHECKED));

			// formatting menu..
			CheckMenuItem(submenu, ID_GLOBAL_BBCODE, MF_BYCOMMAND | ((PluginConfig.m_SendFormat) ? MF_CHECKED : MF_UNCHECKED));
			CheckMenuItem(submenu, ID_GLOBAL_OFF, MF_BYCOMMAND | ((PluginConfig.m_SendFormat == SENDFORMAT_NONE) ? MF_CHECKED : MF_UNCHECKED));

			CheckMenuItem(submenu, ID_THISCONTACT_GLOBALSETTING, MF_BYCOMMAND | ((iLocalFormat == SENDFORMAT_NONE) ? MF_CHECKED : MF_UNCHECKED));
			CheckMenuItem(submenu, ID_THISCONTACT_BBCODE, MF_BYCOMMAND | ((iLocalFormat > 0) ? MF_CHECKED : MF_UNCHECKED));
			CheckMenuItem(submenu, ID_THISCONTACT_OFF, MF_BYCOMMAND | ((iLocalFormat == -1) ? MF_CHECKED : MF_UNCHECKED));

			iSelection = TrackPopupMenu(submenu, TPM_RETURNCMD, rc.left, rc.bottom, 0, m_hwnd, NULL);
			switch (iSelection) {
			case ID_MODE_GLOBAL:
				m_dwFlagsEx &= ~(MWF_SHOW_SPLITTEROVERRIDE);
				db_set_b(m_hContact, SRMSGMOD_T, "splitoverride", 0);
				LoadSplitter();
				AdjustBottomAvatarDisplay();
				DM_RecalcPictureSize();
				SendMessage(m_hwnd, WM_SIZE, 0, 0);
				break;
			
			case ID_MODE_PRIVATE:
				m_dwFlagsEx |= MWF_SHOW_SPLITTEROVERRIDE;
				db_set_b(m_hContact, SRMSGMOD_T, "splitoverride", 1);
				LoadSplitter();
				AdjustBottomAvatarDisplay();
				DM_RecalcPictureSize();
				SendMessage(m_hwnd, WM_SIZE, 0, 0);
				break;
			
			case ID_GLOBAL_BBCODE:
				PluginConfig.m_SendFormat = SENDFORMAT_BBCODE;
				break;
			
			case ID_GLOBAL_OFF:
				PluginConfig.m_SendFormat = SENDFORMAT_NONE;
				break;
			
			case ID_THISCONTACT_GLOBALSETTING:
				iNewLocalFormat = 0;
				break;
			
			case ID_THISCONTACT_BBCODE:
				iNewLocalFormat = SENDFORMAT_BBCODE;
				break;
			
			case ID_THISCONTACT_OFF:
				iNewLocalFormat = -1;
				break;
			}
			
			if (iNewLocalFormat == 0)
				db_unset(m_hContact, SRMSGMOD_T, "sendformat");
			else if (iNewLocalFormat != iLocalFormat)
				db_set_dw(m_hContact, SRMSGMOD_T, "sendformat", iNewLocalFormat);

			if (PluginConfig.m_SendFormat != iOldGlobalSendFormat)
				db_set_b(0, SRMSGMOD_T, "sendformat", (BYTE)PluginConfig.m_SendFormat);
			if (iNewLocalFormat != iLocalFormat || PluginConfig.m_SendFormat != iOldGlobalSendFormat) {
				m_SendFormat = M.GetDword(m_hContact, "sendformat", PluginConfig.m_SendFormat);
				if (m_SendFormat == -1)          // per contact override to disable it..
					m_SendFormat = 0;
				M.BroadcastMessage(DM_CONFIGURETOOLBAR, 0, 1);
			}
		}
		break;

	case IDC_TOGGLETOOLBAR:
		if (lParam == 1)
			ApplyContainerSetting(m_pContainer, CNT_NOMENUBAR, m_pContainer->dwFlags & CNT_NOMENUBAR ? 0 : 1, true);
		else
			ApplyContainerSetting(m_pContainer, CNT_HIDETOOLBAR, m_pContainer->dwFlags & CNT_HIDETOOLBAR ? 0 : 1, true);
		break;

	case IDC_INFOPANELMENU:
		submenu = GetSubMenu(PluginConfig.g_hMenuContext, 9);
		GetWindowRect(GetDlgItem(m_hwnd, IDC_NAME), &rc);

		EnableMenuItem(submenu, ID_FAVORITES_ADDCONTACTTOFAVORITES, !m_cache->isFavorite() ? MF_ENABLED : MF_GRAYED);
		EnableMenuItem(submenu, ID_FAVORITES_REMOVECONTACTFROMFAVORITES, !m_cache->isFavorite() ? MF_GRAYED : MF_ENABLED);

		iSelection = TrackPopupMenu(submenu, TPM_RETURNCMD, rc.left, rc.bottom, 0, m_hwnd, NULL);

		switch (iSelection) {
		case ID_FAVORITES_ADDCONTACTTOFAVORITES:
			db_set_b(m_hContact, SRMSGMOD_T, "isFavorite", 1);
			AddContactToFavorites(m_hContact, m_cache->getNick(), m_cache->getProto(), m_wszStatus, m_wStatus, Skin_LoadProtoIcon(m_cache->getProto(), m_cache->getStatus()), 1, PluginConfig.g_hMenuFavorites);
			break;
		
		case ID_FAVORITES_REMOVECONTACTFROMFAVORITES:
			db_set_b(m_hContact, SRMSGMOD_T, "isFavorite", 0);
			DeleteMenu(PluginConfig.g_hMenuFavorites, m_hContact, MF_BYCOMMAND);
			break;
		}
		m_cache->updateFavorite();
		break;

	case IDC_SENDMENU:
		submenu = GetSubMenu(PluginConfig.g_hMenuContext, 3);

		GetWindowRect(GetDlgItem(m_hwnd, IDOK), &rc);
		CheckMenuItem(submenu, ID_SENDMENU_SENDTOMULTIPLEUSERS, MF_BYCOMMAND | (m_sendMode & SMODE_MULTIPLE ? MF_CHECKED : MF_UNCHECKED));
		CheckMenuItem(submenu, ID_SENDMENU_SENDDEFAULT, MF_BYCOMMAND | (m_sendMode == 0 ? MF_CHECKED : MF_UNCHECKED));
		CheckMenuItem(submenu, ID_SENDMENU_SENDTOCONTAINER, MF_BYCOMMAND | (m_sendMode & SMODE_CONTAINER ? MF_CHECKED : MF_UNCHECKED));
		CheckMenuItem(submenu, ID_SENDMENU_SENDLATER, MF_BYCOMMAND | (m_sendMode & SMODE_SENDLATER ? MF_CHECKED : MF_UNCHECKED));
		CheckMenuItem(submenu, ID_SENDMENU_SENDWITHOUTTIMEOUTS, MF_BYCOMMAND | (m_sendMode & SMODE_NOACK ? MF_CHECKED : MF_UNCHECKED));

		EnableMenuItem(submenu, ID_SENDMENU_SENDNUDGE, MF_BYCOMMAND | ((ProtoServiceExists(m_cache->getActiveProto(), PS_SEND_NUDGE) && ServiceExists(MS_NUDGE_SEND)) ? MF_ENABLED : MF_GRAYED));

		if (lParam)
			iSelection = TrackPopupMenu(submenu, TPM_RETURNCMD, rc.left, rc.bottom, 0, m_hwnd, NULL);
		else
			iSelection = HIWORD(wParam);

		switch (iSelection) {
		case ID_SENDMENU_SENDTOMULTIPLEUSERS:
			m_sendMode ^= SMODE_MULTIPLE;
			if (m_sendMode & SMODE_MULTIPLE)
				DM_CreateClist();
			else if (IsWindow(GetDlgItem(m_hwnd, IDC_CLIST)))
				DestroyWindow(GetDlgItem(m_hwnd, IDC_CLIST));
			break;
		case ID_SENDMENU_SENDNUDGE:
			SendNudge();
			break;
		case ID_SENDMENU_SENDDEFAULT:
			m_sendMode = 0;
			break;
		case ID_SENDMENU_SENDTOCONTAINER:
			m_sendMode ^= SMODE_CONTAINER;
			RedrawWindow(m_hwnd, 0, 0, RDW_ERASENOW | RDW_UPDATENOW);
			break;
		case ID_SENDMENU_SENDLATER:
			if (sendLater->isAvail())
				m_sendMode ^= SMODE_SENDLATER;
			else
				CWarning::show(CWarning::WARN_NO_SENDLATER, MB_OK | MB_ICONINFORMATION, TranslateT("Configuration issue|The unattended send feature is disabled. The \\b1 send later\\b0  and \\b1 send to multiple contacts\\b0  features depend on it.\n\nYou must enable it under \\b1Options -> Message sessions -> Advanced tweaks\\b0. Changing this option requires a restart."));
			break;
		case ID_SENDMENU_SENDWITHOUTTIMEOUTS:
			m_sendMode ^= SMODE_NOACK;
			if (m_sendMode & SMODE_NOACK)
				db_set_b(m_hContact, SRMSGMOD_T, "no_ack", 1);
			else
				db_unset(m_hContact, SRMSGMOD_T, "no_ack");
			break;
		}
		db_set_b(m_hContact, SRMSGMOD_T, "no_ack", (BYTE)(m_sendMode & SMODE_NOACK ? 1 : 0));
		SetWindowPos(GetDlgItem(m_hwnd, IDC_MESSAGE), 0, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOSIZE | SWP_NOMOVE);
		if (m_sendMode & SMODE_MULTIPLE || m_sendMode & SMODE_CONTAINER) {
			SetWindowPos(GetDlgItem(m_hwnd, IDC_MESSAGE), 0, 0, 0, 0, 0, SWP_DRAWFRAME | SWP_FRAMECHANGED | SWP_NOZORDER |
				SWP_NOMOVE | SWP_NOSIZE | SWP_NOCOPYBITS);
			RedrawWindow(m_hwnd, 0, 0, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW | RDW_ALLCHILDREN);
		}
		else {
			if (IsWindow(GetDlgItem(m_hwnd, IDC_CLIST)))
				DestroyWindow(GetDlgItem(m_hwnd, IDC_CLIST));
			SetWindowPos(GetDlgItem(m_hwnd, IDC_MESSAGE), 0, 0, 0, 0, 0, SWP_DRAWFRAME | SWP_FRAMECHANGED | SWP_NOZORDER |
				SWP_NOMOVE | SWP_NOSIZE | SWP_NOCOPYBITS);
			RedrawWindow(m_hwnd, 0, 0, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW | RDW_ALLCHILDREN);
		}
		SendMessage(m_pContainer->hwnd, DM_QUERYCLIENTAREA, 0, (LPARAM)&rc);
		SendMessage(m_hwnd, WM_SIZE, 0, 0);
		DM_ScrollToBottom(1, 1);
		Utils::showDlgControl(m_hwnd, IDC_MULTISPLITTER, (m_sendMode & SMODE_MULTIPLE) ? SW_SHOW : SW_HIDE);
		Utils::showDlgControl(m_hwnd, IDC_CLIST, (m_sendMode & SMODE_MULTIPLE) ? SW_SHOW : SW_HIDE);
		break;

	case IDC_TOGGLESIDEBAR:
		SendMessage(m_pContainer->hwnd, WM_COMMAND, IDC_TOGGLESIDEBAR, 0);
		break;

	case IDC_PIC:
		GetClientRect(m_hwnd, &rc);

		m_fEditNotesActive = !m_fEditNotesActive;
		if (m_fEditNotesActive) {
			int iLen = GetWindowTextLength(GetDlgItem(m_hwnd, IDC_MESSAGE));
			if (iLen != 0) {
				SendMessage(m_hwnd, DM_ACTIVATETOOLTIP, IDC_MESSAGE, (LPARAM)TranslateT("You cannot edit user notes when there are unsent messages"));
				m_fEditNotesActive = false;
				break;
			}

			if (!m_bIsAutosizingInput) {
				m_iSplitterSaved = m_splitterY;
				m_splitterY = rc.bottom / 2;
				SendMessage(m_hwnd, WM_SIZE, 1, 1);
			}

			DBVARIANT dbv = { 0 };

			if (0 == db_get_ws(m_hContact, "UserInfo", "MyNotes", &dbv)) {
				SetDlgItemText(m_hwnd, IDC_MESSAGE, dbv.ptszVal);
				mir_free(dbv.ptszVal);
			}
		}
		else {
			int iLen = GetWindowTextLength(GetDlgItem(m_hwnd, IDC_MESSAGE));

			wchar_t *buf = (wchar_t*)mir_alloc((iLen + 2) * sizeof(wchar_t));
			GetDlgItemText(m_hwnd, IDC_MESSAGE, buf, iLen + 1);
			db_set_ws(m_hContact, "UserInfo", "MyNotes", buf);
			SetDlgItemText(m_hwnd, IDC_MESSAGE, L"");

			if (!m_bIsAutosizingInput) {
				m_splitterY = m_iSplitterSaved;
				SendMessage(m_hwnd, WM_SIZE, 0, 0);
				DM_ScrollToBottom(0, 1);
			}
		}
		SetWindowPos(GetDlgItem(m_hwnd, IDC_MESSAGE), 0, 0, 0, 0, 0, SWP_DRAWFRAME | SWP_FRAMECHANGED | SWP_NOZORDER |
			SWP_NOMOVE | SWP_NOSIZE | SWP_NOCOPYBITS);
		RedrawWindow(m_hwnd, 0, 0, RDW_INVALIDATE | RDW_ERASE | RDW_FRAME | RDW_UPDATENOW | RDW_ALLCHILDREN);

		if (m_fEditNotesActive)
			CWarning::show(CWarning::WARN_EDITUSERNOTES, MB_OK | MB_ICONINFORMATION);
		break;

	case IDM_CLEAR:
		ClearLog();
		break;

	case IDC_PROTOCOL:
		submenu = Menu_BuildContactMenu(m_hContact);
		if (lParam == 0)
			GetWindowRect(GetDlgItem(m_hwnd, IDC_PROTOCOL), &rc);
		else
			GetWindowRect((HWND)lParam, &rc);

		iSelection = TrackPopupMenu(submenu, TPM_RETURNCMD, rc.left, rc.bottom, 0, m_hwnd, NULL);
		if (iSelection)
			Clist_MenuProcessCommand(LOWORD(iSelection), MPCF_CONTACTMENU, m_hContact);

		DestroyMenu(submenu);
		break;

	// error control
	case IDC_CANCELSEND:
		SendMessage(m_hwnd, DM_ERRORDECIDED, MSGERROR_CANCEL, 0);
		break;

	case IDC_RETRY:
		SendMessage(m_hwnd, DM_ERRORDECIDED, MSGERROR_RETRY, 0);
		break;

	case IDC_MSGSENDLATER:
		SendMessage(m_hwnd, DM_ERRORDECIDED, MSGERROR_SENDLATER, 0);
		break;

	case IDC_SELFTYPING:
		if (m_hContact) {
			int iCurrentTypingMode = db_get_b(m_hContact, SRMSGMOD, SRMSGSET_TYPING, M.GetByte(SRMSGMOD, SRMSGSET_TYPINGNEW, SRMSGDEFSET_TYPINGNEW));

			if (m_nTypeMode == PROTOTYPE_SELFTYPING_ON && iCurrentTypingMode) {
				DM_NotifyTyping(PROTOTYPE_SELFTYPING_OFF);
				m_nTypeMode = PROTOTYPE_SELFTYPING_OFF;
			}
			db_set_b(m_hContact, SRMSGMOD, SRMSGSET_TYPING, (BYTE)!iCurrentTypingMode);
		}
		break;

	default:
		return 0;
	}
	return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////
// initialize rich edit control (log and edit control) for both MUC and
// standard IM session windows.

void CTabBaseDlg::DM_InitRichEdit()
{
	bool fIsChat = ((m_bType == SESSIONTYPE_CHAT) ? true : false);
	HWND hwndLog = GetDlgItem(m_hwnd, !fIsChat ? IDC_LOG : IDC_LOG);
	HWND hwndEdit = GetDlgItem(m_hwnd, !fIsChat ? IDC_MESSAGE : IDC_MESSAGE);
	HWND hwndDlg = m_hwnd;

	m_inputbg = fIsChat ? M.GetDword(FONTMODULE, "inputbg", SRMSGDEFSET_BKGCOLOUR) : m_pContainer->theme.inputbg;
	COLORREF colour = fIsChat ? g_Settings.crLogBackground : m_pContainer->theme.bg;
	COLORREF inputcharcolor;

	char *szStreamOut = NULL;
	if (!fIsChat && GetWindowTextLength(hwndEdit) > 0)
		szStreamOut = Message_GetFromStream(hwndEdit);
	SetWindowText(hwndEdit, L"");

	SendMessage(hwndLog, EM_SETBKGNDCOLOR, 0, colour);
	SendMessage(hwndEdit, EM_SETBKGNDCOLOR, 0, m_inputbg);

	CHARFORMAT2A cf2;
	memset(&cf2, 0, sizeof(CHARFORMAT2A));
	cf2.cbSize = sizeof(cf2);

	if (fIsChat) {
		LOGFONTA lf;
		LoadLogfont(MSGFONTID_MESSAGEAREA, &lf, &inputcharcolor, FONTMODULE);

		cf2.dwMask = CFM_COLOR | CFM_FACE | CFM_CHARSET | CFM_SIZE | CFM_WEIGHT | CFM_ITALIC | CFM_BACKCOLOR;
		cf2.crTextColor = inputcharcolor;
		cf2.bCharSet = lf.lfCharSet;
		cf2.crBackColor = m_inputbg;
		strncpy(cf2.szFaceName, lf.lfFaceName, LF_FACESIZE);
		cf2.dwEffects = 0;
		cf2.wWeight = (WORD)lf.lfWeight;
		cf2.bPitchAndFamily = lf.lfPitchAndFamily;
		cf2.yHeight = abs(lf.lfHeight) * 15;
	}
	else {
		LOGFONTA lf = m_pContainer->theme.logFonts[MSGFONTID_MESSAGEAREA];
		inputcharcolor = m_pContainer->theme.fontColors[MSGFONTID_MESSAGEAREA];

		for (int i = 0; i < Utils::rtf_ctable_size; i++)
			if (Utils::rtf_ctable[i].clr == inputcharcolor)
				inputcharcolor = RGB(GetRValue(inputcharcolor), GetGValue(inputcharcolor), GetBValue(inputcharcolor) == 0 ? GetBValue(inputcharcolor) + 1 : GetBValue(inputcharcolor) - 1);

		cf2.dwMask = CFM_COLOR | CFM_FACE | CFM_CHARSET | CFM_SIZE | CFM_WEIGHT | CFM_BOLD | CFM_ITALIC;
		cf2.crTextColor = inputcharcolor;
		cf2.bCharSet = lf.lfCharSet;
		strncpy(cf2.szFaceName, lf.lfFaceName, LF_FACESIZE - 1);
		cf2.dwEffects = ((lf.lfWeight >= FW_BOLD) ? CFE_BOLD : 0) | (lf.lfItalic ? CFE_ITALIC : 0) | (lf.lfUnderline ? CFE_UNDERLINE : 0) | (lf.lfStrikeOut ? CFE_STRIKEOUT : 0);
		cf2.wWeight = (WORD)lf.lfWeight;
		cf2.bPitchAndFamily = lf.lfPitchAndFamily;
		cf2.yHeight = abs(lf.lfHeight) * 15;
	}
	SendMessage(hwndEdit, EM_SETCHARFORMAT, SCF_DEFAULT, (LPARAM)&cf2);
	SendMessage(hwndEdit, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf2); /* WINE: fix send colour text. */
	SendMessage(hwndEdit, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf2); /* WINE: fix send colour text. */

	// setup the rich edit control(s)
	// LOG is always set to RTL, because this is needed for proper bidirectional operation later.
	// The real text direction is then enforced by the streaming code which adds appropiate paragraph
	// and textflow formatting commands to the
	PARAFORMAT2 pf2;
	memset(&pf2, 0, sizeof(PARAFORMAT2));
	pf2.cbSize = sizeof(pf2);
	pf2.wEffects = PFE_RTLPARA;
	pf2.dwMask = PFM_RTLPARA;
	if (Utils::FindRTLLocale(this))
		SendMessage(hwndEdit, EM_SETPARAFORMAT, 0, (LPARAM)&pf2);
	if (!(m_dwFlags & MWF_LOG_RTL)) {
		pf2.wEffects = 0;
		SendMessage(hwndEdit, EM_SETPARAFORMAT, 0, (LPARAM)&pf2);
	}
	SendMessage(hwndEdit, EM_SETLANGOPTIONS, 0, (LPARAM)SendMessage(hwndEdit, EM_GETLANGOPTIONS, 0, 0) & ~IMF_AUTOKEYBOARD);
	pf2.wEffects = PFE_RTLPARA;
	pf2.dwMask |= PFM_OFFSET;
	if (m_dwFlags & MWF_INITMODE) {
		pf2.dwMask |= (PFM_RIGHTINDENT | PFM_OFFSETINDENT);
		pf2.dxStartIndent = 30;
		pf2.dxRightIndent = 30;
	}
	pf2.dxOffset = m_pContainer->theme.left_indent + 30;

	if (!fIsChat) {
		SetWindowText(hwndLog, L"");
		SendMessage(hwndLog, EM_SETPARAFORMAT, 0, (LPARAM)&pf2);
		SendMessage(hwndLog, EM_SETLANGOPTIONS, 0, (LPARAM)SendDlgItemMessage(hwndDlg, IDC_LOG, EM_GETLANGOPTIONS, 0, 0) & ~IMF_AUTOKEYBOARD);
		// set the scrollbars etc to RTL/LTR (only for manual RTL mode)
		if (m_dwFlags & MWF_LOG_RTL) {
			SetWindowLongPtr(hwndEdit, GWL_EXSTYLE, GetWindowLongPtr(hwndEdit, GWL_EXSTYLE) | WS_EX_RIGHT | WS_EX_RTLREADING | WS_EX_LEFTSCROLLBAR);
			SetWindowLongPtr(hwndLog, GWL_EXSTYLE, GetWindowLongPtr(hwndLog, GWL_EXSTYLE) | WS_EX_RIGHT | WS_EX_RTLREADING | WS_EX_LEFTSCROLLBAR);
		}
		else {
			SetWindowLongPtr(hwndEdit, GWL_EXSTYLE, GetWindowLongPtr(hwndEdit, GWL_EXSTYLE) &~(WS_EX_RIGHT | WS_EX_RTLREADING | WS_EX_LEFTSCROLLBAR));
			SetWindowLongPtr(hwndLog, GWL_EXSTYLE, GetWindowLongPtr(hwndLog, GWL_EXSTYLE) &~(WS_EX_RIGHT | WS_EX_RTLREADING | WS_EX_LEFTSCROLLBAR));
		}
	}
	if (szStreamOut != NULL) {
		SETTEXTEX stx = { ST_DEFAULT, CP_UTF8 };
		SendMessage(hwndEdit, EM_SETTEXTEX, (WPARAM)&stx, (LPARAM)szStreamOut);
		mir_free(szStreamOut);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// set the states of defined database action buttons(only if button is a toggle)

void CTabBaseDlg::DM_SetDBButtonStates()
{
	ButtonItem *buttonItem = m_pContainer->buttonItems;
	MCONTACT hFinalContact = 0;
	HWND hwndContainer = m_pContainer->hwnd;

	while (buttonItem) {
		HWND hWnd = GetDlgItem(hwndContainer, buttonItem->uId);

		if (buttonItem->pfnCallback)
			buttonItem->pfnCallback(buttonItem, m_hwnd, this, hWnd);

		if (!(buttonItem->dwFlags & BUTTON_ISTOGGLE && buttonItem->dwFlags & BUTTON_ISDBACTION)) {
			buttonItem = buttonItem->nextItem;
			continue;
		}

		BOOL result = FALSE;
		char *szModule = buttonItem->szModule;
		char *szSetting = buttonItem->szSetting;
		if (buttonItem->dwFlags & BUTTON_DBACTIONONCONTACT || buttonItem->dwFlags & BUTTON_ISCONTACTDBACTION) {
			if (m_hContact == 0) {
				SendMessage(hWnd, BM_SETCHECK, BST_UNCHECKED, 0);
				buttonItem = buttonItem->nextItem;
				continue;
			}
			if (buttonItem->dwFlags & BUTTON_ISCONTACTDBACTION)
				szModule = GetContactProto(m_hContact);
			hFinalContact = m_hContact;
		}
		else hFinalContact = 0;

		switch (buttonItem->type) {
		case DBVT_BYTE:
			result = (db_get_b(hFinalContact, szModule, szSetting, 0) == buttonItem->bValuePush[0]);
			break;
		case DBVT_WORD:
			result = (db_get_w(hFinalContact, szModule, szSetting, 0) == *((WORD *)&buttonItem->bValuePush));
			break;
		case DBVT_DWORD:
			result = (db_get_dw(hFinalContact, szModule, szSetting, 0) == *((DWORD *)&buttonItem->bValuePush));
			break;
		case DBVT_ASCIIZ:
			ptrA szValue(db_get_sa(hFinalContact, szModule, szSetting));
			if (szValue)
				result = !mir_strcmp((char*)buttonItem->bValuePush, szValue);
			break;
		}
		SendMessage(hWnd, BM_SETCHECK, result, 0);
		buttonItem = buttonItem->nextItem;
	}
}

void CTabBaseDlg::DM_ScrollToBottom(WPARAM wParam, LPARAM lParam)
{
	if (m_dwFlagsEx & MWF_SHOW_SCROLLINGDISABLED)
		return;

	if (IsIconic(m_pContainer->hwnd))
		m_dwFlags |= MWF_DEFERREDSCROLL;

	if (m_hwndIEView) {
		PostMessage(GetHwnd(), DM_SCROLLIEVIEW, 0, 0);
		return;
	}
	if (m_hwndHPP) {
		SendMessage(GetHwnd(), DM_SCROLLIEVIEW, 0, 0);
		return;
	}

	HWND hwnd = GetDlgItem(GetHwnd(), m_bType == SESSIONTYPE_IM ? IDC_LOG : IDC_LOG);
	if (lParam)
		SendMessage(hwnd, WM_SIZE, 0, 0);

	if (wParam == 1 && lParam == 1) {
		int len = GetWindowTextLength(hwnd);
		SendMessage(hwnd, EM_SETSEL, len - 1, len - 1);
	}

	if (wParam)
		SendMessage(hwnd, WM_VSCROLL, MAKEWPARAM(SB_BOTTOM, 0), 0);
	else
		PostMessage(hwnd, WM_VSCROLL, MAKEWPARAM(SB_BOTTOM, 0), 0);

	if (lParam)
		InvalidateRect(hwnd, NULL, FALSE);
}

static void LoadKLThread(LPVOID _param)
{
	Thread_SetName("TabSRMM: LoadKLThread");

	DBVARIANT dbv;
	if (!db_get_ws((UINT_PTR)_param, SRMSGMOD_T, "locale", &dbv)) {
		HKL hkl = LoadKeyboardLayout(dbv.ptszVal, 0);
		PostMessage(PluginConfig.g_hwndHotkeyHandler, DM_SETLOCALE, (WPARAM)_param, (LPARAM)hkl);
		db_free(&dbv);
	}
}

void CTabBaseDlg::DM_LoadLocale()
{
	if (!PluginConfig.m_bAutoLocaleSupport)
		return;

	if (m_dwFlags & MWF_WASBACKGROUNDCREATE)
		return;

	DBVARIANT dbv;
	if (!db_get_ws(m_hContact, SRMSGMOD_T, "locale", &dbv))
		db_free(&dbv);
	else {
		wchar_t szKLName[KL_NAMELENGTH + 1];
		if (!PluginConfig.m_bDontUseDefaultKbd) {
			wchar_t	szBuf[20];
			GetLocaleInfo(LOCALE_SYSTEM_DEFAULT, LOCALE_ILANGUAGE, szBuf, 20);
			mir_snwprintf(szKLName, L"0000%s", szBuf);
			db_set_ws(m_hContact, SRMSGMOD_T, "locale", szKLName);
		}
		else {
			GetKeyboardLayoutName(szKLName);
			db_set_ws(m_hContact, SRMSGMOD_T, "locale", szKLName);
		}
	}

	mir_forkthread(LoadKLThread, (void*)m_hContact);
}

void CTabBaseDlg::DM_RecalcPictureSize()
{
	HBITMAP hbm = ((m_Panel->isActive()) && m_pContainer->avatarMode != 3) ? m_hOwnPic : (m_ace ? m_ace->hbmPic : PluginConfig.g_hbmUnknown);
	if (hbm) {
		BITMAP bminfo;
		GetObject(hbm, sizeof(bminfo), &bminfo);
		CalcDynamicAvatarSize(&bminfo);
		SendMessage(m_hwnd, WM_SIZE, 0, 0);
	}
	else m_pic.cy = m_pic.cx = 60;
}

void CTabBaseDlg::DM_UpdateLastMessage() const
{
	if (m_pContainer->hwndStatus == 0 || m_pContainer->hwndActive != m_hwnd)
		return;

	wchar_t szBuf[100];
	if (m_bShowTyping) {
		SendMessage(m_pContainer->hwndStatus, SB_SETICON, 0, (LPARAM)PluginConfig.g_buttonBarIcons[ICON_DEFAULT_TYPING]);
		mir_snwprintf(szBuf, TranslateT("%s is typing a message..."), m_cache->getNick());
	}
	else if (m_sbCustom) {
		SendMessage(m_pContainer->hwndStatus, SB_SETICON, 0, (LPARAM)m_sbCustom->hIcon);
		SendMessage(m_pContainer->hwndStatus, SB_SETTEXT, 0, (LPARAM)m_sbCustom->tszText);
		return;
	}
	else {
		SendMessage(m_pContainer->hwndStatus, SB_SETICON, 0, 0);

		if (m_pContainer->dwFlags & CNT_UINSTATUSBAR)
			mir_snwprintf(szBuf, L"UID: %s", m_cache->getUIN());
		else if (m_lastMessage) {
			wchar_t date[64], time[64];
			TimeZone_PrintTimeStamp(NULL, m_lastMessage, L"d", date, _countof(date), 0);
			if (m_pContainer->dwFlags & CNT_UINSTATUSBAR && mir_wstrlen(date) > 6)
				date[mir_wstrlen(date) - 5] = 0;
			TimeZone_PrintTimeStamp(NULL, m_lastMessage, L"t", time, _countof(time), 0);
			mir_snwprintf(szBuf, TranslateT("Last received: %s at %s"), date, time);
		}
		else szBuf[0] = 0;
	}

	SendMessage(m_pContainer->hwndStatus, SB_SETTEXT, 0, (LPARAM)szBuf);
}

/////////////////////////////////////////////////////////////////////////////////////////
// save current keyboard layout for the given contact

void CTabBaseDlg::DM_SaveLocale(WPARAM, LPARAM lParam)
{
	if (PluginConfig.m_bAutoLocaleSupport && m_hContact && m_pContainer->hwndActive == m_hwnd) {
		wchar_t szKLName[KL_NAMELENGTH + 1];
		if ((HKL)lParam != m_hkl) {
			m_hkl = (HKL)lParam;
			ActivateKeyboardLayout(m_hkl, 0);
			GetKeyboardLayoutName(szKLName);
			db_set_ws(m_hContact, SRMSGMOD_T, "locale", szKLName);
			GetLocaleID(szKLName);
			UpdateReadChars();
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// create embedded contact list control

HWND CTabBaseDlg::DM_CreateClist()
{
	if (!sendLater->isAvail()) {
		CWarning::show(CWarning::WARN_NO_SENDLATER, MB_OK | MB_ICONINFORMATION, TranslateT("Configuration issue|The unattended send feature is disabled. The \\b1 send later\\b0  and \\b1 send to multiple contacts\\b0  features depend on it.\n\nYou must enable it under \\b1Options -> Message sessions -> Advanced tweaks\\b0. Changing this option requires a restart."));
		m_sendMode &= ~SMODE_MULTIPLE;
		return 0;
	}

	HWND hwndClist = CreateWindowExA(0, "CListControl", "", WS_TABSTOP | WS_VISIBLE | WS_CHILD | 0x248, 184, 0, 30, 30, m_hwnd, (HMENU)IDC_CLIST, g_hInst, NULL);
	SendMessage(hwndClist, WM_TIMER, 14, 0);
	HANDLE hItem = (HANDLE)SendMessage(hwndClist, CLM_FINDCONTACT, m_hContact, 0);

	SetWindowLongPtr(hwndClist, GWL_EXSTYLE, GetWindowLongPtr(hwndClist, GWL_EXSTYLE) & ~CLS_EX_TRACKSELECT);
	SetWindowLongPtr(hwndClist, GWL_EXSTYLE, GetWindowLongPtr(hwndClist, GWL_EXSTYLE) | (CLS_EX_NOSMOOTHSCROLLING | CLS_EX_NOTRANSLUCENTSEL));

	if (!PluginConfig.m_bAllowOfflineMultisend)
		SetWindowLongPtr(hwndClist, GWL_STYLE, GetWindowLongPtr(hwndClist, GWL_STYLE) | CLS_HIDEOFFLINE);

	if (hItem)
		SendMessage(hwndClist, CLM_SETCHECKMARK, (WPARAM)hItem, 1);

	SendMessage(hwndClist, CLM_SETHIDEEMPTYGROUPS, M.GetByte("CList", "HideEmptyGroups", SETTING_USEGROUPS_DEFAULT), 0);
	SendMessage(hwndClist, CLM_SETUSEGROUPS, M.GetByte("CList", "UseGroups", SETTING_USEGROUPS_DEFAULT), 0);
	SendMessage(hwndClist, CLM_FIRST + 106, 0, 1);
	SendMessage(hwndClist, CLM_AUTOREBUILD, 0, 0);
	if (hwndClist)
		RedrawWindow(hwndClist, 0, 0, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW);
	return hwndClist;
}

LRESULT CTabBaseDlg::DM_MouseWheelHandler(WPARAM wParam, LPARAM lParam)
{
	UINT uID = m_bType == SESSIONTYPE_IM ? IDC_LOG : IDC_LOG;
	UINT uIDMsg = m_bType == SESSIONTYPE_IM ? IDC_MESSAGE : IDC_MESSAGE;

	POINT pt;
	GetCursorPos(&pt);

	RECT rc, rc1;
	GetWindowRect(m_message.GetHwnd(), &rc);
	if (PtInRect(&rc, pt))
		return 1;

	if (m_pContainer->dwFlags & CNT_SIDEBAR) {
		GetWindowRect(GetDlgItem(m_pContainer->hwnd, IDC_SIDEBARUP), &rc);
		GetWindowRect(GetDlgItem(m_pContainer->hwnd, IDC_SIDEBARDOWN), &rc1);
		rc.bottom = rc1.bottom;
		if (PtInRect(&rc, pt)) {
			short amount = (short)(HIWORD(wParam));
			SendMessage(m_pContainer->hwnd, WM_COMMAND, MAKELONG(amount > 0 ? IDC_SIDEBARUP : IDC_SIDEBARDOWN, 0), (LPARAM)uIDMsg);
			return 0;
		}
	}
	if (m_bType == SESSIONTYPE_CHAT) {					// scroll nick list by just hovering it
		RECT	rcNicklist;
		GetWindowRect(GetDlgItem(GetHwnd(), IDC_LIST), &rcNicklist);
		if (PtInRect(&rcNicklist, pt)) {
			SendDlgItemMessage(GetHwnd(), IDC_LIST, WM_MOUSEWHEEL, wParam, lParam);
			return 0;
		}
	}
	if (m_hwndIEView)
		GetWindowRect(m_hwndIEView, &rc);
	else if (m_hwndHPP)
		GetWindowRect(m_hwndHPP, &rc);
	else
		GetWindowRect(GetDlgItem(m_hwnd, uID), &rc);
	if (PtInRect(&rc, pt)) {
		HWND hwndLog = (m_hwndIEView || m_hwndHPP) ? m_hwndIWebBrowserControl : GetDlgItem(m_hwnd, uID);
		short wDirection = (short)HIWORD(wParam);

		if (hwndLog == 0)
			hwndLog = WindowFromPoint(pt);

		if (LOWORD(wParam) & MK_SHIFT || M.GetByte("fastscroll", 0)) {
			if (wDirection < 0)
				SendMessage(hwndLog, WM_VSCROLL, MAKEWPARAM(SB_PAGEDOWN, 0), 0);
			else if (wDirection > 0)
				SendMessage(hwndLog, WM_VSCROLL, MAKEWPARAM(SB_PAGEUP, 0), 0);
		}
		else SendMessage(hwndLog, WM_MOUSEWHEEL, wParam, lParam);
		return 0;
	}

	HWND hwndTab = GetDlgItem(m_pContainer->hwnd, IDC_MSGTABS);
	if (GetTabItemFromMouse(hwndTab, &pt) != -1) {
		SendMessage(hwndTab, WM_MOUSEWHEEL, wParam, -1);
		return 0;
	}
	return 1;
}

void CTabBaseDlg::DM_FreeTheme()
{
	if (m_hTheme) {
		CloseThemeData(m_hTheme);
		m_hTheme = 0;
	}
	if (m_hThemeIP) {
		CloseThemeData(m_hThemeIP);
		m_hThemeIP = 0;
	}
	if (m_hThemeToolbar) {
		CloseThemeData(m_hThemeToolbar);
		m_hThemeToolbar = 0;
	}
}

void CTabBaseDlg::DM_ThemeChanged()
{
	CSkinItem *item_log = &SkinItems[ID_EXTBKHISTORY];
	CSkinItem *item_msg = &SkinItems[ID_EXTBKINPUTAREA];

	m_hTheme = OpenThemeData(m_hwnd, L"EDIT");

	if (m_bType == SESSIONTYPE_IM) {
		if (m_hTheme != 0 || (CSkin::m_skinEnabled && !item_log->IGNORED))
			SetWindowLongPtr(GetDlgItem(m_hwnd, IDC_LOG), GWL_EXSTYLE, GetWindowLongPtr(GetDlgItem(m_hwnd, IDC_LOG), GWL_EXSTYLE) & ~WS_EX_STATICEDGE);
		if (m_hTheme != 0 || (CSkin::m_skinEnabled && !item_msg->IGNORED))
			SetWindowLongPtr(GetDlgItem(m_hwnd, IDC_MESSAGE), GWL_EXSTYLE, GetWindowLongPtr(GetDlgItem(m_hwnd, IDC_MESSAGE), GWL_EXSTYLE) & ~WS_EX_STATICEDGE);
	}
	else {
		if (m_hTheme != 0 || (CSkin::m_skinEnabled && !item_log->IGNORED)) {
			SetWindowLongPtr(GetDlgItem(m_hwnd, IDC_LOG), GWL_EXSTYLE, GetWindowLongPtr(GetDlgItem(m_hwnd, IDC_LOG), GWL_EXSTYLE) & ~WS_EX_STATICEDGE);
			SetWindowLongPtr(GetDlgItem(m_hwnd, IDC_LIST), GWL_EXSTYLE, GetWindowLongPtr(GetDlgItem(m_hwnd, IDC_LIST), GWL_EXSTYLE) & ~(WS_EX_CLIENTEDGE | WS_EX_STATICEDGE));
		}
		if (m_hTheme != 0 || (CSkin::m_skinEnabled && !item_msg->IGNORED))
			SetWindowLongPtr(GetDlgItem(m_hwnd, IDC_MESSAGE), GWL_EXSTYLE, GetWindowLongPtr(GetDlgItem(m_hwnd, IDC_MESSAGE), GWL_EXSTYLE) & ~WS_EX_STATICEDGE);
	}
	m_hThemeIP = M.isAero() ? OpenThemeData(m_hwnd, L"ButtonStyle") : 0;
	m_hThemeToolbar = (M.isAero() || (!CSkin::m_skinEnabled && M.isVSThemed())) ? OpenThemeData(m_hwnd, L"REBAR") : 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// send out message typing notifications (MTN) when the
// user is typing/editing text in the message input area.

void CTabBaseDlg::DM_NotifyTyping(int mode)
{
	if (!m_hContact)
		return;

	DeletePopupsForContact(m_hContact, PU_REMOVE_ON_TYPE);

	const char *szProto = m_cache->getActiveProto();
	MCONTACT hContact = m_cache->getActiveContact();

	// editing user notes or preparing a message for queued delivery -> don't send MTN
	if (m_fEditNotesActive || (m_sendMode & SMODE_SENDLATER))
		return;

	// allow supression of sending out TN for the contact (NOTE: for metacontacts, do NOT use the subcontact handle)
	if (!db_get_b(hContact, SRMSGMOD, SRMSGSET_TYPING, M.GetByte(SRMSGMOD, SRMSGSET_TYPINGNEW, SRMSGDEFSET_TYPINGNEW)))
		return;

	if (szProto == NULL) // should not, but who knows...
		return;

	// check status and capabilities of the protocol
	DWORD typeCaps = CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_4, 0);
	if (!(typeCaps & PF4_SUPPORTTYPING))
		return;

	DWORD protoStatus = CallProtoService(szProto, PS_GETSTATUS, 0, 0);
	if (protoStatus < ID_STATUS_ONLINE)
		return;

	// check visibility/invisibility lists to not "accidentially" send MTN to contacts who
	// should not see them (privacy issue)
	DWORD protoCaps = CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_1, 0);
	if (protoCaps & PF1_VISLIST && db_get_w(hContact, szProto, "ApparentMode", 0) == ID_STATUS_OFFLINE)
		return;

	if (protoCaps & PF1_INVISLIST && protoStatus == ID_STATUS_INVISIBLE && db_get_w(hContact, szProto, "ApparentMode", 0) != ID_STATUS_ONLINE)
		return;

	// don't send to contacts which are not permanently added to the contact list,
	// unless the option to ignore added status is set.
	if (db_get_b(m_hContact, "CList", "NotOnList", 0) && !M.GetByte(SRMSGMOD, SRMSGSET_TYPINGUNKNOWN, SRMSGDEFSET_TYPINGUNKNOWN))
		return;

	// End user check
	m_nTypeMode = mode;
	CallService(MS_PROTO_SELFISTYPING, hContact, m_nTypeMode);
}

void CSrmmWindow::DM_OptionsApplied(WPARAM, LPARAM lParam)
{
	m_szMicroLf[0] = 0;
	if (!(m_pContainer->theme.isPrivate)) {
		LoadThemeDefaults(m_pContainer);
		m_dwFlags = m_pContainer->theme.dwFlags;
	}

	LoadLocalFlags();
	m_hTimeZone = TimeZone_CreateByContact(m_hContact, 0, TZF_KNOWNONLY);

	m_bShowUIElements = (m_pContainer->dwFlags & CNT_HIDETOOLBAR) == 0;

	m_dwFlagsEx = M.GetByte(m_hContact, "splitoverride", 0) ? MWF_SHOW_SPLITTEROVERRIDE : 0;
	m_Panel->getVisibility();

	// small inner margins (padding) for the text areas
	m_log.SendMsg(EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELONG(0, 0));
	m_message.SendMsg(EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELONG(3, 3));

	GetSendFormat();
	SetDialogToType(m_hwnd);
	SendMessage(m_hwnd, DM_CONFIGURETOOLBAR, 0, 0);

	DM_InitRichEdit();
	if (m_hwnd == m_pContainer->hwndActive)
		SendMessage(m_pContainer->hwnd, WM_SIZE, 0, 0);
	InvalidateRect(GetDlgItem(m_hwnd, IDC_MESSAGE), NULL, FALSE);
	if (!lParam) {
		if (IsIconic(m_pContainer->hwnd))
			m_dwFlags |= MWF_DEFERREDREMAKELOG;
		else
			SendMessage(m_hwnd, DM_REMAKELOG, 0, 0);
	}

	ShowWindow(m_hwndPanelPicParent, SW_SHOW);
	EnableWindow(m_hwndPanelPicParent, TRUE);

	SendMessage(m_hwnd, DM_UPDATEWINICON, 0, 0);
}
	
void CTabBaseDlg::DM_Typing(bool fForceOff)
{
	HWND hwndContainer = m_pContainer->hwnd;
	HWND hwndStatus = m_pContainer->hwndStatus;

	if (m_nTypeMode == PROTOTYPE_SELFTYPING_ON && GetTickCount() - m_nLastTyping > TIMEOUT_TYPEOFF)
		DM_NotifyTyping(PROTOTYPE_SELFTYPING_OFF);

	if (m_bShowTyping == 1) {
		if (m_nTypeSecs > 0) {
			m_nTypeSecs--;
			if (GetForegroundWindow() == hwndContainer)
				SendMessage(m_hwnd, DM_UPDATEWINICON, 0, 0);
		}
		else {
			if (!fForceOff) {
				m_bShowTyping = 2;
				m_nTypeSecs = 86400;

				mir_snwprintf(m_wszStatusBar, TranslateT("%s has entered text."), m_cache->getNick());
				if (hwndStatus && m_pContainer->hwndActive == m_hwnd)
					SendMessage(hwndStatus, SB_SETTEXT, 0, (LPARAM)m_wszStatusBar);
			}
			SendMessage(m_hwnd, DM_UPDATEWINICON, 0, 0);
			HandleIconFeedback(this, (HICON)-1);
			CTabBaseDlg *dat_active = (CTabBaseDlg*)GetWindowLongPtr(m_pContainer->hwndActive, GWLP_USERDATA);
			if (dat_active && dat_active->m_bType == SESSIONTYPE_IM)
				SendMessage(hwndContainer, DM_UPDATETITLE, 0, 0);
			else
				SendMessage(hwndContainer, DM_UPDATETITLE, (WPARAM)m_pContainer->hwndActive, 1);
			if (!(m_pContainer->dwFlags & CNT_NOFLASH) && PluginConfig.m_FlashOnMTN)
				ReflashContainer(m_pContainer);
		}
	}
	else if (m_bShowTyping == 2) {
		if (m_nTypeSecs > 0)
			m_nTypeSecs--;
		else {
			m_wszStatusBar[0] = 0;
			m_bShowTyping = 0;
		}
		UpdateStatusBar();
	}
	else if (m_nTypeSecs > 0) {
		mir_snwprintf(m_wszStatusBar, TranslateT("%s is typing a message"), m_cache->getNick());

		m_nTypeSecs--;
		if (hwndStatus && m_pContainer->hwndActive == m_hwnd) {
			SendMessage(hwndStatus, SB_SETTEXT, 0, (LPARAM)m_wszStatusBar);
			SendMessage(hwndStatus, SB_SETICON, 0, (LPARAM)PluginConfig.g_buttonBarIcons[ICON_DEFAULT_TYPING]);
		}
		if (IsIconic(hwndContainer) || GetForegroundWindow() != hwndContainer || GetActiveWindow() != hwndContainer) {
			SetWindowText(hwndContainer, m_wszStatusBar);
			m_pContainer->dwFlags |= CNT_NEED_UPDATETITLE;
			if (!(m_pContainer->dwFlags & CNT_NOFLASH) && PluginConfig.m_FlashOnMTN)
				ReflashContainer(m_pContainer);
		}

		if (m_pContainer->hwndActive != m_hwnd) {
			if (m_bCanFlashTab)
				m_iFlashIcon = PluginConfig.g_IconTypingEvent;
			HandleIconFeedback(this, PluginConfig.g_IconTypingEvent);
		}
		else { // active tab may show icon if status bar is disabled
			if (!hwndStatus) {
				if (TabCtrl_GetItemCount(m_hwndParent) > 1 || !(m_pContainer->dwFlags & CNT_HIDETABS))
					HandleIconFeedback(this, PluginConfig.g_IconTypingEvent);
			}
		}
		if ((GetForegroundWindow() != hwndContainer) || (m_pContainer->hwndStatus == 0) || (m_pContainer->hwndActive != m_hwnd))
			SendMessage(hwndContainer, DM_SETICON, (WPARAM)this, (LPARAM)PluginConfig.g_buttonBarIcons[ICON_DEFAULT_TYPING]);

		m_bShowTyping = 1;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// sync splitter position for all open sessions.
// This cares about private / per container / MUC <> IM splitter syncing and everything.
// called from IM and MUC windows via DM_SPLITTERGLOBALEVENT

int CTabBaseDlg::DM_SplitterGlobalEvent(WPARAM wParam, LPARAM lParam)
{
	CTabBaseDlg *srcDat = PluginConfig.lastSPlitterPos.pSrcDat;
	TContainerData *srcCnt = PluginConfig.lastSPlitterPos.pSrcContainer;
	bool fCntGlobal = (!m_pContainer->settings->fPrivate ? true : false);

	if (m_bIsAutosizingInput)
		return 0;

	RECT rcWin;
	GetWindowRect(m_hwnd, &rcWin);

	LONG newPos;
	if (wParam == 0 && lParam == 0) {
		if ((m_dwFlagsEx & MWF_SHOW_SPLITTEROVERRIDE) && this != srcDat)
			return 0;

		if (srcDat->m_bType == m_bType)
			newPos = PluginConfig.lastSPlitterPos.pos;
		else if (srcDat->m_bType == SESSIONTYPE_IM && m_bType == SESSIONTYPE_CHAT)
			newPos = PluginConfig.lastSPlitterPos.pos + PluginConfig.lastSPlitterPos.off_im;
		else if (srcDat->m_bType == SESSIONTYPE_CHAT && m_bType == SESSIONTYPE_IM)
			newPos = PluginConfig.lastSPlitterPos.pos + PluginConfig.lastSPlitterPos.off_im;
		else
			newPos = 0;

		if (this == srcDat) {
			if (m_bType == SESSIONTYPE_IM) {
				m_pContainer->settings->splitterPos = m_splitterY;
				if (fCntGlobal) {
					SaveSplitter();
					if (PluginConfig.lastSPlitterPos.bSync)
						g_Settings.iSplitterY = m_splitterY - DPISCALEY_S(23);
				}
			}
			if (m_bType == SESSIONTYPE_CHAT) {
				if (si) {
					m_pContainer->settings->splitterPos = si->iSplitterY + DPISCALEY_S(23);
					if (fCntGlobal) {
						g_Settings.iSplitterY = si->iSplitterY;
						if (PluginConfig.lastSPlitterPos.bSync)
							db_set_dw(0, SRMSGMOD_T, "splitsplity", (DWORD)si->iSplitterY + DPISCALEY_S(23));
					}
				}
			}
			return 0;
		}

		if (!fCntGlobal && m_pContainer != srcCnt)
			return 0;
		if (srcCnt->settings->fPrivate && m_pContainer != srcCnt)
			return 0;

		if (!PluginConfig.lastSPlitterPos.bSync && m_bType != srcDat->m_bType)
			return 0;

		// for inactive sessions, delay the splitter repositioning until they become
		// active (faster, avoid redraw/resize problems for minimized windows)
		if (IsIconic(m_pContainer->hwnd) || m_pContainer->hwndActive != m_hwnd) {
			m_dwFlagsEx |= MWF_EX_DELAYEDSPLITTER;
			m_wParam = newPos;
			m_lParam = PluginConfig.lastSPlitterPos.lParam;
			return 0;
		}
	}
	else newPos = wParam;

	if (m_bType == SESSIONTYPE_IM) {
		LoadSplitter();
		AdjustBottomAvatarDisplay();
		DM_RecalcPictureSize();
		SendMessage(m_hwnd, WM_SIZE, 0, 0);
		DM_ScrollToBottom(1, 1);
		if (this != srcDat)
			UpdateToolbarBG();
	}
	else {
		if (si) {
			si->iSplitterY = g_Settings.iSplitterY;
			SendMessage(m_hwnd, WM_SIZE, 0, 0);
		}
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// incoming event handler

void CTabBaseDlg::DM_EventAdded(WPARAM hContact, LPARAM lParam)
{
	MEVENT hDbEvent = (MEVENT)lParam;

	DBEVENTINFO dbei = {};
	db_event_get(hDbEvent, &dbei);
	if (m_hDbEventFirst == NULL)
		m_hDbEventFirst = hDbEvent;

	bool bIsStatusChangeEvent = IsStatusEvent(dbei.eventType);
	bool bDisableNotify = (dbei.eventType == EVENTTYPE_MESSAGE && (dbei.flags & DBEF_READ));

	if (!DbEventIsShown(&dbei))
		return;

	if (dbei.eventType == EVENTTYPE_MESSAGE && !(dbei.flags & (DBEF_SENT))) {
		m_lastMessage = dbei.timestamp;
		m_wszStatusBar[0] = 0;
		if (m_bShowTyping) {
			m_nTypeSecs = 0;
			DM_Typing(true);
			m_bShowTyping = 0;
		}
		HandleIconFeedback(this, (HICON)-1);
		if (m_pContainer->hwndStatus)
			PostMessage(m_hwnd, DM_UPDATELASTMESSAGE, 0, 0);
	}

	// set the message log divider to mark new (maybe unseen) messages, if the container has
	// been minimized or in the background.
	if (!(dbei.flags & DBEF_SENT) && !bIsStatusChangeEvent) {
		if (PluginConfig.m_bDividersUsePopupConfig && PluginConfig.m_bUseDividers) {
			if (!MessageWindowOpened(m_hContact, 0))
				SendMessage(m_hwnd, DM_ADDDIVIDER, 0, 0);
		}
		else if (PluginConfig.m_bUseDividers) {
			if ((GetForegroundWindow() != m_pContainer->hwnd || GetActiveWindow() != m_pContainer->hwnd))
				SendMessage(m_hwnd, DM_ADDDIVIDER, 0, 0);
			else if (m_pContainer->hwndActive != m_hwnd)
				SendMessage(m_hwnd, DM_ADDDIVIDER, 0, 0);
		}
		if (!bDisableNotify)
			tabSRMM_ShowPopup(hContact, hDbEvent, dbei.eventType, m_pContainer->fHidden ? 0 : 1, m_pContainer, m_hwnd, m_cache->getActiveProto());
		if (IsWindowVisible(m_pContainer->hwnd))
			m_pContainer->fHidden = false;
	}
	m_cache->updateStats(TSessionStats::UPDATE_WITH_LAST_RCV, 0);

	if (hDbEvent != m_hDbEventFirst) {
		if (!(m_dwFlagsEx & MWF_SHOW_SCROLLINGDISABLED))
			SendMessage(m_hwnd, DM_APPENDTOLOG, hDbEvent, 0);
		else {
			if (m_iNextQueuedEvent >= m_iEventQueueSize) {
				m_hQueuedEvents = (MEVENT*)mir_realloc(m_hQueuedEvents, (m_iEventQueueSize + 10) * sizeof(MEVENT));
				m_iEventQueueSize += 10;
			}
			m_hQueuedEvents[m_iNextQueuedEvent++] = hDbEvent;

			wchar_t szBuf[100];
			mir_snwprintf(szBuf, TranslateT("Auto scrolling is disabled, %d message(s) queued (press F12 to enable it)"), m_iNextQueuedEvent);
			SetDlgItemText(m_hwnd, IDC_LOGFROZENTEXT, szBuf);
			RedrawWindow(GetDlgItem(m_hwnd, IDC_LOGFROZENTEXT), NULL, NULL, RDW_INVALIDATE);
		}
	}
	else SendMessage(m_hwnd, DM_REMAKELOG, 0, 0);

	// handle tab flashing
	if (!bDisableNotify && !bIsStatusChangeEvent)
		if ((TabCtrl_GetCurSel(m_hwndParent) != m_iTabID) && !(dbei.flags & DBEF_SENT)) {
			switch (dbei.eventType) {
			case EVENTTYPE_MESSAGE:
				m_iFlashIcon = PluginConfig.g_IconMsgEvent;
				break;
			case EVENTTYPE_FILE:
				m_iFlashIcon = PluginConfig.g_IconFileEvent;
				break;
			default:
				m_iFlashIcon = PluginConfig.g_IconMsgEvent;
				break;
			}
			SetTimer(m_hwnd, TIMERID_FLASHWND, TIMEOUT_FLASHWND, NULL);
			m_bCanFlashTab = true;
		}

	// try to flash the contact list...
	if (!bDisableNotify)
		FlashOnClist(hDbEvent, &dbei);

	// autoswitch tab if option is set AND container is minimized (otherwise, we never autoswitch)
	// never switch for status changes...
	if (!(dbei.flags & DBEF_SENT) && !bIsStatusChangeEvent) {
		if (PluginConfig.haveAutoSwitch() && m_pContainer->hwndActive != m_hwnd) {
			if ((IsIconic(m_pContainer->hwnd) && !IsZoomed(m_pContainer->hwnd)) || (PluginConfig.m_bHideOnClose && !IsWindowVisible(m_pContainer->hwnd))) {
				int iItem = GetTabIndexFromHWND(GetParent(m_hwnd), m_hwnd);
				if (iItem >= 0) {
					TabCtrl_SetCurSel(m_hwndParent, iItem);
					ShowWindow(m_pContainer->hwndActive, SW_HIDE);
					m_pContainer->hwndActive = m_hwnd;
					SendMessage(m_pContainer->hwnd, DM_UPDATETITLE, m_hContact, 0);
					m_pContainer->dwFlags |= CNT_DEFERREDTABSELECT;
				}
			}
		}
	}

	// flash window if it is not focused
	if (!bDisableNotify && !bIsStatusChangeEvent)
		if ((GetActiveWindow() != m_pContainer->hwnd || GetForegroundWindow() != m_pContainer->hwnd || m_pContainer->hwndActive != m_hwnd) && !(dbei.flags & DBEF_SENT)) {
			if (!(m_pContainer->dwFlags & CNT_NOFLASH) && (GetActiveWindow() != m_pContainer->hwnd || GetForegroundWindow() != m_pContainer->hwnd))
				FlashContainer(m_pContainer, 1, 0);
			SendMessage(m_pContainer->hwnd, DM_SETICON, (WPARAM)this, (LPARAM)Skin_LoadIcon(SKINICON_EVENT_MESSAGE));
			m_pContainer->dwFlags |= CNT_NEED_UPDATETITLE;
		}

	// play a sound
	if (!bDisableNotify && dbei.eventType == EVENTTYPE_MESSAGE && !(dbei.flags & (DBEF_SENT)))
		PostMessage(m_hwnd, DM_PLAYINCOMINGSOUND, 0, 0);

	if (m_pWnd)
		m_pWnd->Invalidate();
}

void CTabBaseDlg::DM_HandleAutoSizeRequest(REQRESIZE* rr)
{
	if (rr == NULL || GetForegroundWindow() != m_pContainer->hwnd)
		return;

	if (!m_bIsAutosizingInput || m_iInputAreaHeight == -1)
		return;

	LONG heightLimit = M.GetDword("autoSplitMinLimit", 0);
	LONG iNewHeight = rr->rc.bottom - rr->rc.top;

	if (CSkin::m_skinEnabled && !SkinItems[ID_EXTBKINPUTAREA].IGNORED)
		iNewHeight += (SkinItems[ID_EXTBKINPUTAREA].MARGIN_TOP + SkinItems[ID_EXTBKINPUTAREA].MARGIN_BOTTOM - 2);

	if (heightLimit && iNewHeight < heightLimit)
		iNewHeight = heightLimit;

	if (iNewHeight == m_iInputAreaHeight)
		return;

	RECT rc;
	GetClientRect(m_hwnd, &rc);
	LONG cy = rc.bottom - rc.top;
	LONG panelHeight = (m_Panel->isActive() ? m_Panel->getHeight() : 0);

	if (iNewHeight > (cy - panelHeight) / 2)
		iNewHeight = (cy - panelHeight) / 2;

	if (m_bType == SESSIONTYPE_IM) {
		m_dynaSplitter = rc.bottom - (rc.bottom - iNewHeight + DPISCALEY_S(2));
		if (m_pContainer->dwFlags & CNT_BOTTOMTOOLBAR)
			m_dynaSplitter += DPISCALEY_S(22);
		m_splitterY = m_dynaSplitter + DPISCALEY_S(34);
		DM_RecalcPictureSize();
	}
	else if (si) {
		si->iSplitterY = (rc.bottom - (rc.bottom - iNewHeight + DPISCALEY_S(3))) + DPISCALEY_S(34);
		if (!(m_pContainer->dwFlags & CNT_BOTTOMTOOLBAR))
			si->iSplitterY -= DPISCALEY_S(22);
		SendMessage(m_hwnd, WM_SIZE, 0, 0);
	}
	m_iInputAreaHeight = iNewHeight;
	UpdateToolbarBG();
	DM_ScrollToBottom(1, 0);
}

void CTabBaseDlg::DM_UpdateTitle(WPARAM, LPARAM lParam)
{
	DWORD dwOldIdle = m_idle;
	const char *szActProto = 0;

	m_wszStatus[0] = 0;

	if (m_iTabID == -1)
		return;

	TCITEM item = { 0 };

	wchar_t newtitle[128];
	if (m_hContact) {
		const wchar_t *szNick = m_cache->getNick();

		if (m_szProto) {
			szActProto = m_cache->getProto();

			bool bHasName = (m_cache->getUIN()[0] != 0);
			m_idle = m_cache->getIdleTS();
			m_dwFlagsEx = m_idle ? m_dwFlagsEx | MWF_SHOW_ISIDLE : m_dwFlagsEx & ~MWF_SHOW_ISIDLE;

			m_wStatus = m_cache->getStatus();
			wcsncpy_s(m_wszStatus, pcli->pfnGetStatusModeDescription(m_szProto == NULL ? ID_STATUS_OFFLINE : m_wStatus, 0), _TRUNCATE);

			if (lParam != 0) {
				wchar_t newcontactname[128]; newcontactname[0] = 0;
				if (PluginConfig.m_bCutContactNameOnTabs)
					CutContactName(szNick, newcontactname, _countof(newcontactname));
				else
					wcsncpy_s(newcontactname, szNick, _TRUNCATE);

				Utils::DoubleAmpersands(newcontactname, _countof(newcontactname));

				if (newcontactname[0] != 0) {
					if (PluginConfig.m_bStatusOnTabs)
						mir_snwprintf(newtitle, L"%s (%s)", newcontactname, m_wszStatus);
					else
						wcsncpy_s(newtitle, newcontactname, _TRUNCATE);
				}
				else wcsncpy_s(newtitle, L"Forward", _TRUNCATE);

				item.mask |= TCIF_TEXT;
			}
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

	if (m_idle != dwOldIdle || lParam != 0) {
		if (item.mask & TCIF_TEXT) {
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
		if (m_pContainer->hwndActive == m_hwnd && lParam)
			SendMessage(m_pContainer->hwnd, DM_UPDATETITLE, m_hContact, 0);

		UpdateTrayMenuState(this, TRUE);
		if (m_cache->isFavorite())
			AddContactToFavorites(m_hContact, m_cache->getNick(), szActProto, m_wszStatus, m_wStatus,
				Skin_LoadProtoIcon(m_cache->getProto(), m_cache->getStatus()), 0, PluginConfig.g_hMenuFavorites);

		if (m_cache->isRecent())
			AddContactToFavorites(m_hContact, m_cache->getNick(), szActProto, m_wszStatus, m_wStatus,
				Skin_LoadProtoIcon(m_cache->getProto(), m_cache->getStatus()), 0, PluginConfig.g_hMenuRecent);

		m_Panel->Invalidate();
		if (m_pWnd)
			m_pWnd->Invalidate();
	}

	// care about MetaContacts and update the statusbar icon with the currently "most online" contact...
	if (m_bIsMeta) {
		PostMessage(m_hwnd, DM_UPDATEMETACONTACTINFO, 0, 0);
		PostMessage(m_hwnd, DM_OWNNICKCHANGED, 0, 0);
		if (m_pContainer->dwFlags & CNT_UINSTATUSBAR)
			DM_UpdateLastMessage();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// status icon stuff (by sje, used for indicating encryption status in the status bar
// this is now part of the message window api

static HANDLE hHookIconPressedEvt;

static int OnSrmmIconChanged(WPARAM hContact, LPARAM)
{
	if (hContact == NULL)
		M.BroadcastMessage(DM_STATUSICONCHANGE, 0, 0);
	else {
		HWND hwnd = M.FindWindow(hContact);
		if (hwnd)
			PostMessage(hwnd, DM_STATUSICONCHANGE, 0, 0);
	}
	return 0;
}

void CTabBaseDlg::DrawStatusIcons(HDC hDC, const RECT &rc, int gap)
{
	int x = rc.left;
	int y = (rc.top + rc.bottom - PluginConfig.m_smcxicon) >> 1;

	SetBkMode(hDC, TRANSPARENT);

	int nIcon = 0;
	while (StatusIconData *sid = Srmm_GetNthIcon(m_hContact, nIcon++)) {
		if (!mir_strcmp(sid->szModule, MSG_ICON_MODULE)) {
			if (sid->dwId == MSG_ICON_SOUND) {
				DrawIconEx(hDC, x, y, PluginConfig.g_buttonBarIcons[ICON_DEFAULT_SOUNDS],
					PluginConfig.m_smcxicon, PluginConfig.m_smcyicon, 0, NULL, DI_NORMAL);

				DrawIconEx(hDC, x, y, m_pContainer->dwFlags & CNT_NOSOUND ?
					PluginConfig.g_iconOverlayDisabled : PluginConfig.g_iconOverlayEnabled,
					PluginConfig.m_smcxicon, PluginConfig.m_smcyicon, 0, NULL, DI_NORMAL);
			}
			else if (sid->dwId == MSG_ICON_UTN) {
				if (m_bType == SESSIONTYPE_IM || si->iType == GCW_PRIVMESS) {
					DrawIconEx(hDC, x, y, PluginConfig.g_buttonBarIcons[ICON_DEFAULT_TYPING], PluginConfig.m_smcxicon, PluginConfig.m_smcyicon, 0, NULL, DI_NORMAL);

					DrawIconEx(hDC, x, y, db_get_b(m_hContact, SRMSGMOD, SRMSGSET_TYPING, M.GetByte(SRMSGMOD, SRMSGSET_TYPINGNEW, SRMSGDEFSET_TYPINGNEW)) ?
						PluginConfig.g_iconOverlayEnabled : PluginConfig.g_iconOverlayDisabled, PluginConfig.m_smcxicon, PluginConfig.m_smcyicon, 0, NULL, DI_NORMAL);
				}
				else CSkin::DrawDimmedIcon(hDC, x, y, PluginConfig.m_smcxicon, PluginConfig.m_smcyicon, PluginConfig.g_buttonBarIcons[ICON_DEFAULT_TYPING], 50);
			}
			else if (sid->dwId == MSG_ICON_SESSION) {
				DrawIconEx(hDC, x, y, PluginConfig.g_sideBarIcons[0], PluginConfig.m_smcxicon, PluginConfig.m_smcyicon, 0, NULL, DI_NORMAL);
			}
		}
		else {
			HICON hIcon;
			if ((sid->flags & MBF_DISABLED) && sid->hIconDisabled)
				hIcon = sid->hIconDisabled;
			else
				hIcon = sid->hIcon;

			if ((sid->flags & MBF_DISABLED) && sid->hIconDisabled == NULL)
				CSkin::DrawDimmedIcon(hDC, x, y, PluginConfig.m_smcxicon, PluginConfig.m_smcyicon, hIcon, 50);
			else
				DrawIconEx(hDC, x, y, hIcon, 16, 16, 0, NULL, DI_NORMAL);
		}

		x += PluginConfig.m_smcxicon + gap;
	}
}

void CTabBaseDlg::CheckStatusIconClick(POINT pt, const RECT &rc, int gap, int code)
{
	if (code == NM_CLICK || code == NM_RCLICK) {
		POINT	ptScreen;
		GetCursorPos(&ptScreen);
		if (!PtInRect(&rcLastStatusBarClick, ptScreen))
			return;
	}

	UINT iconNum = (pt.x - (rc.left + 0)) / (PluginConfig.m_smcxicon + gap);
	
	StatusIconData *sid = Srmm_GetNthIcon(m_hContact, iconNum);
	if (sid == NULL)
		return;

	if (!mir_strcmp(sid->szModule, MSG_ICON_MODULE)) {
		if (sid->dwId == MSG_ICON_SOUND && code != NM_RCLICK) {
			if (GetKeyState(VK_SHIFT) & 0x8000) {
				for (TContainerData *p = pFirstContainer; p; p = p->pNext) {
					p->dwFlags = ((m_pContainer->dwFlags & CNT_NOSOUND) ? p->dwFlags | CNT_NOSOUND : p->dwFlags & ~CNT_NOSOUND);
					InvalidateRect(m_pContainer->hwndStatus, NULL, TRUE);
				}
			}
			else {
				m_pContainer->dwFlags ^= CNT_NOSOUND;
				InvalidateRect(m_pContainer->hwndStatus, NULL, TRUE);
			}
		}
		else if (sid->dwId == MSG_ICON_UTN && code != NM_RCLICK && (m_bType == SESSIONTYPE_IM || si->iType == GCW_PRIVMESS)) {
			SendMessage(m_pContainer->hwndActive, WM_COMMAND, IDC_SELFTYPING, 0);
			InvalidateRect(m_pContainer->hwndStatus, NULL, TRUE);
		}
		else if (sid->dwId == MSG_ICON_SESSION) {
			if (code == NM_CLICK)
				PostMessage(PluginConfig.g_hwndHotkeyHandler, DM_TRAYICONNOTIFY, 101, WM_LBUTTONUP);
			else if (code == NM_RCLICK)
				PostMessage(PluginConfig.g_hwndHotkeyHandler, DM_TRAYICONNOTIFY, 101, WM_RBUTTONUP);
		}
	}
	else {
		StatusIconClickData sicd = { sizeof(sicd) };
		GetCursorPos(&sicd.clickLocation);
		sicd.dwId = sid->dwId;
		sicd.szModule = sid->szModule;
		sicd.flags = (code == NM_RCLICK ? MBCF_RIGHTBUTTON : 0);
		NotifyEventHooks(hHookIconPressedEvt, m_hContact, (LPARAM)&sicd);
		InvalidateRect(m_pContainer->hwndStatus, NULL, TRUE);
	}
}

int SI_InitStatusIcons()
{
	StatusIconData sid = { sizeof(sid) };
	sid.szModule = MSG_ICON_MODULE;
	sid.dwId = MSG_ICON_SOUND; // Sounds
	Srmm_AddIcon(&sid);

	sid.dwId = MSG_ICON_UTN;
	Srmm_AddIcon(&sid);

	sid.dwId = MSG_ICON_SESSION;
	Srmm_AddIcon(&sid);

	HookEvent(ME_MSG_ICONSCHANGED, OnSrmmIconChanged);

	hHookIconPressedEvt = CreateHookableEvent(ME_MSG_ICONPRESSED);
	return 0;
}

int SI_DeinitStatusIcons()
{
	DestroyHookableEvent(hHookIconPressedEvt);
	return 0;
}
