/////////////////////////////////////////////////////////////////////////////////////////
// Miranda NG: the free IM client for Microsoft* Windows*
//
// Copyright (C) 2012-22 Miranda NG team,
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

/*
void CMsgDialog::DM_SaveLogAsRTF() const
{
	if (m_hwndIEView != nullptr) {
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
			m_rtf.SendMsg(EM_STREAMOUT, SF_RTF | SF_USECODEPAGE, (LPARAM)&stream);
		}
	}
}
*/

/////////////////////////////////////////////////////////////////////////////////////////
// checks if the balloon tooltip can be dismissed (usually called by WM_MOUSEMOVE events)

void CMsgDialog::DM_DismissTip(const POINT& pt)
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

void CMsgDialog::DM_InitTip()
{
	m_hwndTip = CreateWindowEx(0, TOOLTIPS_CLASS, nullptr, WS_POPUP | TTS_NOPREFIX | TTS_BALLOON, CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT, m_hwnd, nullptr, g_plugin.getInst(), (LPVOID)nullptr);

	memset(&ti, 0, sizeof(ti));
	ti.cbSize = sizeof(ti);
	ti.lpszText = TranslateT("No status message");
	ti.hinst = g_plugin.getInst();
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

bool CMsgDialog::DM_GenericHotkeysCheck(MSG *message)
{
	LRESULT mim_hotkey_check = Hotkey_Check(message, TABSRMM_HK_SECTION_GENERIC);

	switch (mim_hotkey_check) {
	case TABSRMM_HK_PASTEANDSEND:
		HandlePasteAndSend();
		return true;

	case TABSRMM_HK_HISTORY:
		m_btnHistory.Click();
		return true;

	case TABSRMM_HK_CONTAINEROPTIONS:
		m_pContainer->OptionsDialog();
		return true;

	case TABSRMM_HK_TOGGLEINFOPANEL:
		m_pPanel.setActive(!m_pPanel.isActive());
		m_pPanel.showHide();
		return true;

	case TABSRMM_HK_TOGGLETOOLBAR:
		SendMessage(m_hwnd, WM_COMMAND, IDC_TOGGLETOOLBAR, 0);
		return true;

	case TABSRMM_HK_CLEARLOG:
		tabClearLog();
		return true;

	case TABSRMM_HK_TOGGLESIDEBAR:
		if (m_pContainer->m_pSideBar->isActive())
			SendMessage(m_hwnd, WM_COMMAND, IDC_TOGGLESIDEBAR, 0);
		return true;

	case TABSRMM_HK_CLOSE_OTHER:
		CloseOtherTabs(m_pContainer->m_hwndTabs, *this);
		return true;
	}
	return false;
}

LRESULT CMsgDialog::DM_MsgWindowCmdHandler(UINT cmd, WPARAM wParam, LPARAM lParam)
{
	RECT  rc;
	int   iSelection;
	HMENU submenu;

	switch (cmd) {
	case IDC_SRMM_BOLD:
	case IDC_SRMM_ITALICS:
	case IDC_SRMM_UNDERLINE:
	case IDC_FONTSTRIKEOUT:
		if (m_SendFormat != 0) { // dont use formatting if disabled
			CHARFORMAT2 cf, cfOld;
			memset(&cf, 0, sizeof(CHARFORMAT2));
			memset(&cfOld, 0, sizeof(CHARFORMAT2));
			cfOld.cbSize = cf.cbSize = sizeof(CHARFORMAT2);
			cfOld.dwMask = CFM_BOLD | CFM_ITALIC | CFM_UNDERLINE | CFM_STRIKEOUT;
			m_message.SendMsg(EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM)&cfOld);
			BOOL isBold = (cfOld.dwEffects & CFE_BOLD) && (cfOld.dwMask & CFM_BOLD);
			BOOL isItalic = (cfOld.dwEffects & CFE_ITALIC) && (cfOld.dwMask & CFM_ITALIC);
			BOOL isUnderline = (cfOld.dwEffects & CFE_UNDERLINE) && (cfOld.dwMask & CFM_UNDERLINE);
			BOOL isStrikeout = (cfOld.dwEffects & CFM_STRIKEOUT) && (cfOld.dwMask & CFM_STRIKEOUT);

			int ctrlId = LOWORD(wParam);
			if (ctrlId == IDC_SRMM_BOLD && !IsWindowEnabled(GetDlgItem(m_hwnd, IDC_SRMM_BOLD)))
				break;
			if (ctrlId == IDC_SRMM_ITALICS && !IsWindowEnabled(GetDlgItem(m_hwnd, IDC_SRMM_ITALICS)))
				break;
			if (ctrlId == IDC_SRMM_UNDERLINE && !IsWindowEnabled(GetDlgItem(m_hwnd, IDC_SRMM_UNDERLINE)))
				break;
			if (ctrlId == IDC_FONTSTRIKEOUT && !IsWindowEnabled(GetDlgItem(m_hwnd, IDC_FONTSTRIKEOUT)))
				break;
			if (ctrlId == IDC_SRMM_BOLD) {
				cf.dwEffects = isBold ? 0 : CFE_BOLD;
				cf.dwMask = CFM_BOLD;
				CheckDlgButton(m_hwnd, IDC_SRMM_BOLD, !isBold ? BST_CHECKED : BST_UNCHECKED);
			}
			else if (ctrlId == IDC_SRMM_ITALICS) {
				cf.dwEffects = isItalic ? 0 : CFE_ITALIC;
				cf.dwMask = CFM_ITALIC;
				CheckDlgButton(m_hwnd, IDC_SRMM_ITALICS, !isItalic ? BST_CHECKED : BST_UNCHECKED);
			}
			else if (ctrlId == IDC_SRMM_UNDERLINE) {
				cf.dwEffects = isUnderline ? 0 : CFE_UNDERLINE;
				cf.dwMask = CFM_UNDERLINE;
				CheckDlgButton(m_hwnd, IDC_SRMM_UNDERLINE, !isUnderline ? BST_CHECKED : BST_UNCHECKED);
			}
			else if (ctrlId == IDC_FONTSTRIKEOUT) {
				cf.dwEffects = isStrikeout ? 0 : CFM_STRIKEOUT;
				cf.dwMask = CFM_STRIKEOUT;
				CheckDlgButton(m_hwnd, IDC_FONTSTRIKEOUT, !isStrikeout ? BST_CHECKED : BST_UNCHECKED);
			}
			m_message.SendMsg(EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
		}
		break;

	case IDCANCEL:
		ShowWindow(m_pContainer->m_hwnd, SW_MINIMIZE);
		return FALSE;

	case IDC_CLOSE:
		PostMessage(m_hwnd, WM_CLOSE, 1, 0);
		break;

	case IDC_NAME:
		if (GetKeyState(VK_SHIFT) & 0x8000)   // copy UIN
			Utils_ClipboardCopy(m_cache->getUIN());
		else
			CallService(MS_USERINFO_SHOWDIALOG, (WPARAM)(m_cache->getActiveContact()), 0);
		break;

	case IDC_TIME:
		submenu = GetSubMenu(PluginConfig.g_hMenuContext, 2);
		MsgWindowUpdateMenu(submenu, MENU_LOGMENU);

		GetWindowRect(GetDlgItem(m_hwnd, IDC_TIME), &rc);

		iSelection = TrackPopupMenu(submenu, TPM_RETURNCMD, rc.left, rc.bottom, 0, m_hwnd, nullptr);
		return MsgWindowMenuHandler(iSelection, MENU_LOGMENU);

	case IDC_PROTOMENU:
		submenu = GetSubMenu(PluginConfig.g_hMenuContext, 4);
		{
			bool iOldGlobalSendFormat = g_plugin.bSendFormat;
			int iLocalFormat = M.GetDword(m_hContact, "sendformat", 0);
			int iNewLocalFormat = iLocalFormat;

			GetWindowRect(GetDlgItem(m_hwnd, IDC_PROTOCOL), &rc);

			CheckMenuItem(submenu, ID_MODE_GLOBAL, !m_bSplitterOverride ? MF_CHECKED : MF_UNCHECKED);
			CheckMenuItem(submenu, ID_MODE_PRIVATE, m_bSplitterOverride ? MF_CHECKED : MF_UNCHECKED);

			// formatting menu..
			CheckMenuItem(submenu, ID_GLOBAL_BBCODE, (g_plugin.bSendFormat) ? MF_CHECKED : MF_UNCHECKED);
			CheckMenuItem(submenu, ID_GLOBAL_OFF, (g_plugin.bSendFormat == SENDFORMAT_NONE) ? MF_CHECKED : MF_UNCHECKED);

			CheckMenuItem(submenu, ID_THISCONTACT_GLOBALSETTING, (iLocalFormat == SENDFORMAT_NONE) ? MF_CHECKED : MF_UNCHECKED);
			CheckMenuItem(submenu, ID_THISCONTACT_BBCODE, (iLocalFormat > 0) ? MF_CHECKED : MF_UNCHECKED);
			CheckMenuItem(submenu, ID_THISCONTACT_OFF, (iLocalFormat == -1) ? MF_CHECKED : MF_UNCHECKED);

			iSelection = TrackPopupMenu(submenu, TPM_RETURNCMD, rc.left, rc.bottom, 0, m_hwnd, nullptr);
			switch (iSelection) {
			case ID_MODE_GLOBAL:
				m_bSplitterOverride = false;
				db_set_b(m_hContact, SRMSGMOD_T, "splitoverride", 0);
				LoadSplitter();
				AdjustBottomAvatarDisplay();
				DM_RecalcPictureSize();
				Resize();
				break;

			case ID_MODE_PRIVATE:
				m_bSplitterOverride = true;
				db_set_b(m_hContact, SRMSGMOD_T, "splitoverride", 1);
				LoadSplitter();
				AdjustBottomAvatarDisplay();
				DM_RecalcPictureSize();
				Resize();
				break;

			case ID_GLOBAL_BBCODE:
				g_plugin.bSendFormat = SENDFORMAT_BBCODE;
				break;

			case ID_GLOBAL_OFF:
				g_plugin.bSendFormat = SENDFORMAT_NONE;
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

			if (iNewLocalFormat != iLocalFormat || g_plugin.bSendFormat != iOldGlobalSendFormat) {
				m_SendFormat = M.GetDword(m_hContact, "sendformat", g_plugin.bSendFormat);
				if (m_SendFormat == -1)          // per contact override to disable it..
					m_SendFormat = 0;
				Srmm_Broadcast(DM_CONFIGURETOOLBAR, 0, 1);
			}
		}
		break;

	case IDC_TOGGLETOOLBAR:
		if (lParam == 1)
			m_pContainer->cfg.flags.m_bNoMenuBar = !m_pContainer->cfg.flags.m_bNoMenuBar;
		else
			m_pContainer->cfg.flags.m_bHideToolbar = !m_pContainer->cfg.flags.m_bHideToolbar;
		m_pContainer->ApplySetting(true);
		break;

	case IDC_SENDMENU:
		submenu = GetSubMenu(PluginConfig.g_hMenuContext, 3);

		GetWindowRect(GetDlgItem(m_hwnd, IDOK), &rc);
		CheckMenuItem(submenu, ID_SENDMENU_SENDTOMULTIPLEUSERS, (m_sendMode & SMODE_MULTIPLE) ? MF_CHECKED : MF_UNCHECKED);
		CheckMenuItem(submenu, ID_SENDMENU_SENDDEFAULT, m_sendMode == 0 ? MF_CHECKED : MF_UNCHECKED);
		CheckMenuItem(submenu, ID_SENDMENU_SENDTOCONTAINER, (m_sendMode & SMODE_CONTAINER) ? MF_CHECKED : MF_UNCHECKED);
		CheckMenuItem(submenu, ID_SENDMENU_SENDLATER, (m_sendMode & SMODE_SENDLATER) ? MF_CHECKED : MF_UNCHECKED);
		CheckMenuItem(submenu, ID_SENDMENU_SENDWITHOUTTIMEOUTS, (m_sendMode & SMODE_NOACK) ? MF_CHECKED : MF_UNCHECKED);

		if (lParam)
			iSelection = TrackPopupMenu(submenu, TPM_RETURNCMD, rc.left, rc.bottom, 0, m_hwnd, nullptr);
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
		case ID_SENDMENU_SENDDEFAULT:
			m_sendMode = 0;
			break;
		case ID_SENDMENU_SENDTOCONTAINER:
			m_sendMode ^= SMODE_CONTAINER;
			RedrawWindow(m_hwnd, nullptr, nullptr, RDW_ERASENOW | RDW_UPDATENOW);
			break;
		case ID_SENDMENU_SENDLATER:
			if (SendLater::Avail)
				m_sendMode ^= SMODE_SENDLATER;
			else
				CWarning::show(CWarning::WARN_NO_SENDLATER, MB_OK | MB_ICONINFORMATION);
			break;
		case ID_SENDMENU_SENDWITHOUTTIMEOUTS:
			m_sendMode ^= SMODE_NOACK;
			if (m_sendMode & SMODE_NOACK)
				db_set_b(m_hContact, SRMSGMOD_T, "no_ack", 1);
			else
				db_unset(m_hContact, SRMSGMOD_T, "no_ack");
			break;
		}
		db_set_b(m_hContact, SRMSGMOD_T, "no_ack", (uint8_t)(m_sendMode & SMODE_NOACK ? 1 : 0));
		SetWindowPos(m_message.GetHwnd(), nullptr, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOSIZE | SWP_NOMOVE);
		if (m_sendMode & SMODE_MULTIPLE || m_sendMode & SMODE_CONTAINER) {
			SetWindowPos(m_message.GetHwnd(), nullptr, 0, 0, 0, 0, SWP_DRAWFRAME | SWP_FRAMECHANGED | SWP_NOZORDER |
				SWP_NOMOVE | SWP_NOSIZE | SWP_NOCOPYBITS);
			RedrawWindow(m_hwnd, nullptr, nullptr, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW | RDW_ALLCHILDREN);
		}
		else {
			if (IsWindow(GetDlgItem(m_hwnd, IDC_CLIST)))
				DestroyWindow(GetDlgItem(m_hwnd, IDC_CLIST));
			SetWindowPos(m_message.GetHwnd(), nullptr, 0, 0, 0, 0, SWP_DRAWFRAME | SWP_FRAMECHANGED | SWP_NOZORDER |
				SWP_NOMOVE | SWP_NOSIZE | SWP_NOCOPYBITS);
			RedrawWindow(m_hwnd, nullptr, nullptr, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW | RDW_ALLCHILDREN);
		}
		m_pContainer->QueryClientArea(rc);
		Resize();
		DM_ScrollToBottom(1, 1);
		Utils::showDlgControl(m_hwnd, IDC_MULTISPLITTER, (m_sendMode & SMODE_MULTIPLE) ? SW_SHOW : SW_HIDE);
		Utils::showDlgControl(m_hwnd, IDC_CLIST, (m_sendMode & SMODE_MULTIPLE) ? SW_SHOW : SW_HIDE);
		break;

	case IDC_TOGGLESIDEBAR:
		SendMessage(m_pContainer->m_hwnd, WM_COMMAND, IDC_TOGGLESIDEBAR, 0);
		break;

	case IDC_PIC:
		GetClientRect(m_hwnd, &rc);

		m_bEditNotesActive = !m_bEditNotesActive;
		if (m_bEditNotesActive) {
			int iLen = GetWindowTextLength(m_message.GetHwnd());
			if (iLen != 0) {
				ActivateTooltip(IDC_SRMM_MESSAGE, TranslateT("You cannot edit user notes when there are unsent messages"));
				m_bEditNotesActive = false;
				break;
			}

			if (!m_bIsAutosizingInput) {
				m_iSplitterSaved = m_iSplitterY;
				m_iSplitterY = rc.bottom / 2;
				SendMessage(m_hwnd, WM_SIZE, 1, 1);
			}

			ptrW wszText(db_get_wsa(m_hContact, "UserInfo", "MyNotes"));
			if (wszText != nullptr)
				m_message.SetText(wszText);
		}
		else {
			ptrW buf(m_message.GetText());
			db_set_ws(m_hContact, "UserInfo", "MyNotes", buf);
			m_message.SetText(L"");

			if (!m_bIsAutosizingInput) {
				m_iSplitterY = m_iSplitterSaved;
				Resize();
				DM_ScrollToBottom(0, 1);
			}
		}
		SetWindowPos(m_message.GetHwnd(), nullptr, 0, 0, 0, 0, SWP_DRAWFRAME | SWP_FRAMECHANGED | SWP_NOZORDER |
			SWP_NOMOVE | SWP_NOSIZE | SWP_NOCOPYBITS);
		RedrawWindow(m_hwnd, nullptr, nullptr, RDW_INVALIDATE | RDW_ERASE | RDW_FRAME | RDW_UPDATENOW | RDW_ALLCHILDREN);

		if (m_bEditNotesActive)
			CWarning::show(CWarning::WARN_EDITUSERNOTES, MB_OK | MB_ICONINFORMATION);
		break;

	case IDM_CLEAR:
		tabClearLog();
		break;

	case IDC_PROTOCOL:
		submenu = Menu_BuildContactMenu(m_hContact);
		if (lParam == 0)
			GetWindowRect(GetDlgItem(m_hwnd, IDC_PROTOCOL), &rc);
		else
			GetWindowRect((HWND)lParam, &rc);

		iSelection = TrackPopupMenu(submenu, TPM_RETURNCMD, rc.left, rc.bottom, 0, m_hwnd, nullptr);
		if (iSelection)
			Clist_MenuProcessCommand(LOWORD(iSelection), MPCF_CONTACTMENU, m_hContact);

		DestroyMenu(submenu);
		break;

	// error control
	case IDC_CANCELSEND:
		DM_ErrorDetected(MSGERROR_CANCEL, 0);
		break;

	case IDC_RETRY:
		DM_ErrorDetected(MSGERROR_RETRY, 0);
		break;

	case IDC_MSGSENDLATER:
		DM_ErrorDetected(MSGERROR_SENDLATER, 0);
		break;

	case IDC_SELFTYPING:
		if (AllowTyping()) {
			int iCurrentTypingMode = g_plugin.getByte(m_hContact, SRMSGSET_TYPING, g_plugin.bTypingNew);
			if (m_nTypeMode == PROTOTYPE_SELFTYPING_ON && iCurrentTypingMode) {
				DM_NotifyTyping(PROTOTYPE_SELFTYPING_OFF);
				m_nTypeMode = PROTOTYPE_SELFTYPING_OFF;
			}
			g_plugin.setByte(m_hContact, SRMSGSET_TYPING, (uint8_t)!iCurrentTypingMode);
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

void CMsgDialog::DM_InitRichEdit()
{
	char *szStreamOut = nullptr;
	if (!isChat() && GetWindowTextLength(m_message.GetHwnd()) > 0)
		szStreamOut = m_message.GetRichTextRtf();
	SetWindowText(m_message.GetHwnd(), L"");

	m_pLog->UpdateOptions();

	m_message.SendMsg(EM_SETBKGNDCOLOR, 0, m_pContainer->m_theme.inputbg);

	CHARFORMAT2 cf2 = {};
	cf2.cbSize = sizeof(cf2);

	if (isChat()) {
		LOGFONTW lf;
		COLORREF inputcharcolor;
		LoadMsgDlgFont(FONTSECTION_IM, MSGFONTID_MESSAGEAREA, &lf, &inputcharcolor);

		cf2.dwMask = CFM_COLOR | CFM_FACE | CFM_CHARSET | CFM_SIZE | CFM_WEIGHT | CFM_ITALIC | CFM_BACKCOLOR;
		cf2.crTextColor = inputcharcolor;
		cf2.bCharSet = lf.lfCharSet;
		cf2.crBackColor = m_pContainer->m_theme.inputbg;
		wcsncpy_s(cf2.szFaceName, lf.lfFaceName, _TRUNCATE);
		cf2.dwEffects = 0;
		cf2.wWeight = (uint16_t)lf.lfWeight;
		cf2.bPitchAndFamily = lf.lfPitchAndFamily;
		cf2.yHeight = abs(lf.lfHeight) * 15;
	}
	else {
		LOGFONTW lf = m_pContainer->m_theme.logFonts[MSGFONTID_MESSAGEAREA];
		COLORREF inputcharcolor = m_pContainer->m_theme.fontColors[MSGFONTID_MESSAGEAREA];

		for (auto &it : Utils::rtf_clrs)
			if (it->clr == inputcharcolor)
				inputcharcolor = RGB(GetRValue(inputcharcolor), GetGValue(inputcharcolor), GetBValue(inputcharcolor) == 0 ? GetBValue(inputcharcolor) + 1 : GetBValue(inputcharcolor) - 1);

		cf2.dwMask = CFM_COLOR | CFM_FACE | CFM_CHARSET | CFM_SIZE | CFM_WEIGHT | CFM_BOLD | CFM_ITALIC;
		cf2.crTextColor = inputcharcolor;
		cf2.bCharSet = lf.lfCharSet;
		wcsncpy_s(cf2.szFaceName, lf.lfFaceName, _TRUNCATE);
		cf2.dwEffects = ((lf.lfWeight >= FW_BOLD) ? CFE_BOLD : 0) | (lf.lfItalic ? CFE_ITALIC : 0) | (lf.lfUnderline ? CFE_UNDERLINE : 0) | (lf.lfStrikeOut ? CFE_STRIKEOUT : 0);
		cf2.wWeight = (uint16_t)lf.lfWeight;
		cf2.bPitchAndFamily = lf.lfPitchAndFamily;
		cf2.yHeight = abs(lf.lfHeight) * 15;
	}
	m_message.SendMsg(EM_SETCHARFORMAT, SCF_DEFAULT, (LPARAM)&cf2);
	m_message.SendMsg(EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf2); /* WINE: fix send colour text. */
	m_message.SendMsg(EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf2); /* WINE: fix send colour text. */

	// setup the rich edit control(s)
	// LOG is always set to RTL, because this is needed for proper bidirectional operation later.
	// The real text direction is then enforced by the streaming code which adds appropiate paragraph
	// and textflow formatting commands to the
	PARAFORMAT2 pf2;
	memset(&pf2, 0, sizeof(PARAFORMAT2));
	pf2.cbSize = sizeof(pf2);
	pf2.wEffects = PFE_RTLPARA;
	pf2.dwMask = PFM_RTLPARA;
	if (FindRTLLocale())
		m_message.SendMsg(EM_SETPARAFORMAT, 0, (LPARAM)&pf2);
	if (!(m_dwFlags & MWF_LOG_RTL)) {
		pf2.wEffects = 0;
		m_message.SendMsg(EM_SETPARAFORMAT, 0, (LPARAM)&pf2);
	}
	m_message.SendMsg(EM_SETLANGOPTIONS, 0, (LPARAM)m_message.SendMsg(EM_GETLANGOPTIONS, 0, 0) & ~IMF_AUTOKEYBOARD);

	if (m_dwFlags & MWF_LOG_RTL)
		SetWindowLongPtr(m_message.GetHwnd(), GWL_EXSTYLE, GetWindowLongPtr(m_message.GetHwnd(), GWL_EXSTYLE) | WS_EX_RIGHT | WS_EX_RTLREADING | WS_EX_LEFTSCROLLBAR);
	else
		SetWindowLongPtr(m_message.GetHwnd(), GWL_EXSTYLE, GetWindowLongPtr(m_message.GetHwnd(), GWL_EXSTYLE) & ~(WS_EX_RIGHT | WS_EX_RTLREADING | WS_EX_LEFTSCROLLBAR));

	if (szStreamOut != nullptr) {
		SETTEXTEX stx = { ST_DEFAULT, CP_UTF8 };
		m_message.SendMsg(EM_SETTEXTEX, (WPARAM)&stx, (LPARAM)szStreamOut);
		mir_free(szStreamOut);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// set the states of defined database action buttons(only if button is a toggle)

void CMsgDialog::DM_SetDBButtonStates()
{
	ButtonItem *buttonItem = m_pContainer->m_buttonItems;
	MCONTACT hFinalContact = 0;
	HWND hwndContainer = m_pContainer->m_hwnd;

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
			if (buttonItem->dwFlags & BUTTON_ISCONTACTDBACTION)
				szModule = Proto_GetBaseAccountName(m_hContact);
			hFinalContact = m_hContact;
		}
		else hFinalContact = 0;

		switch (buttonItem->type) {
		case DBVT_BYTE:
			result = (db_get_b(hFinalContact, szModule, szSetting, 0) == buttonItem->bValuePush[0]);
			break;
		case DBVT_WORD:
			result = (db_get_w(hFinalContact, szModule, szSetting, 0) == *((uint16_t *)&buttonItem->bValuePush));
			break;
		case DBVT_DWORD:
			result = (db_get_dw(hFinalContact, szModule, szSetting, 0) == *((uint32_t *)&buttonItem->bValuePush));
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

void CMsgDialog::DM_ScrollToBottom(WPARAM wParam, LPARAM lParam)
{
	if (m_bScrollingDisabled)
		return;

	if (IsIconic(m_pContainer->m_hwnd))
		m_bDeferredScroll = true;

	if (m_iLogMode == WANT_BUILTIN_LOG)
		((CLogWindow *)m_pLog)->ScrollToBottom(wParam != 0, lParam != 0);
	else
		m_pLog->ScrollToBottom();
}

void CMsgDialog::DM_RecalcPictureSize()
{
	HBITMAP hbm = ((m_pPanel.isActive()) && m_pContainer->cfg.avatarMode != 3) ? m_hOwnPic : (m_ace ? m_ace->hbmPic : PluginConfig.g_hbmUnknown);
	if (hbm) {
		BITMAP bminfo;
		GetObject(hbm, sizeof(bminfo), &bminfo);
		CalcDynamicAvatarSize(&bminfo);
		Resize();
	}
	else m_pic.cy = m_pic.cx = 60;
}

void CMsgDialog::DM_UpdateLastMessage() const
{
	if (m_pContainer->m_hwndStatus == nullptr || m_pContainer->m_hwndActive != m_hwnd)
		return;

	wchar_t szBuf[100];
	if (m_bShowTyping) {
		SendMessage(m_pContainer->m_hwndStatus, SB_SETICON, 0, (LPARAM)PluginConfig.g_buttonBarIcons[ICON_DEFAULT_TYPING]);
		mir_snwprintf(szBuf, TranslateT("%s is typing a message..."), m_cache->getNick());
	}
	else if (m_bStatusSet) {
		SendMessage(m_pContainer->m_hwndStatus, SB_SETICON, 0, (LPARAM)m_szStatusIcon);
		SendMessage(m_pContainer->m_hwndStatus, SB_SETTEXT, 0, (LPARAM)m_szStatusText.c_str());
		return;
	}
	else {
		SendMessage(m_pContainer->m_hwndStatus, SB_SETICON, 0, 0);

		if (m_pContainer->cfg.flags.m_bUinStatusBar)
			mir_snwprintf(szBuf, L"UID: %s", m_cache->getUIN());
		else if (m_lastMessage) {
			wchar_t date[64], time[64];
			TimeZone_PrintTimeStamp(nullptr, m_lastMessage, L"d", date, _countof(date), 0);
			if (m_pContainer->cfg.flags.m_bUinStatusBar && mir_wstrlen(date) > 6)
				date[mir_wstrlen(date) - 5] = 0;
			TimeZone_PrintTimeStamp(nullptr, m_lastMessage, L"t", time, _countof(time), 0);
			mir_snwprintf(szBuf, TranslateT("Last received: %s at %s"), date, time);
		}
		else szBuf[0] = 0;
	}

	SendMessage(m_pContainer->m_hwndStatus, SB_SETTEXT, 0, (LPARAM)szBuf);
}

/////////////////////////////////////////////////////////////////////////////////////////
// create embedded contact list control

HWND CMsgDialog::DM_CreateClist()
{
	if (!SendLater::Avail) {
		CWarning::show(CWarning::WARN_NO_SENDLATER, MB_OK | MB_ICONINFORMATION);
		m_sendMode &= ~SMODE_MULTIPLE;
		return nullptr;
	}

	HWND hwndClist = CreateWindowExA(0, "CListControl", "", WS_TABSTOP | WS_VISIBLE | WS_CHILD | 0x248, 184, 0, 30, 30, m_hwnd, (HMENU)IDC_CLIST, g_plugin.getInst(), nullptr);
	SendMessage(hwndClist, WM_TIMER, 14, 0);
	HANDLE hItem = (HANDLE)SendMessage(hwndClist, CLM_FINDCONTACT, m_hContact, 0);

	SetWindowLongPtr(hwndClist, GWL_EXSTYLE, GetWindowLongPtr(hwndClist, GWL_EXSTYLE) & ~CLS_EX_TRACKSELECT);
	SetWindowLongPtr(hwndClist, GWL_EXSTYLE, GetWindowLongPtr(hwndClist, GWL_EXSTYLE) | (CLS_EX_NOSMOOTHSCROLLING | CLS_EX_NOTRANSLUCENTSEL));

	if (!g_plugin.bAllowOfflineMultisend)
		SetWindowLongPtr(hwndClist, GWL_STYLE, GetWindowLongPtr(hwndClist, GWL_STYLE) | CLS_HIDEOFFLINE);

	if (hItem)
		SendMessage(hwndClist, CLM_SETCHECKMARK, (WPARAM)hItem, 1);

	SendMessage(hwndClist, CLM_SETHIDEEMPTYGROUPS, Clist::HideEmptyGroups, 0);
	SendMessage(hwndClist, CLM_SETUSEGROUPS, Clist::UseGroups, 0);
	SendMessage(hwndClist, CLM_FIRST + 106, 0, 1);
	SendMessage(hwndClist, CLM_AUTOREBUILD, 0, 0);
	if (hwndClist)
		RedrawWindow(hwndClist, nullptr, nullptr, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW);
	return hwndClist;
}

LRESULT CMsgDialog::DM_MouseWheelHandler(WPARAM wParam, LPARAM lParam)
{
	POINT pt;
	GetCursorPos(&pt);

	RECT rc;
	GetWindowRect(m_message.GetHwnd(), &rc);
	if (PtInRect(&rc, pt))
		return 1;

	if (isChat()) {					// scroll nick list by just hovering it
		RECT rcNicklist;
		GetWindowRect(m_nickList.GetHwnd(), &rcNicklist);
		if (PtInRect(&rcNicklist, pt)) {
			m_nickList.SendMsg(WM_MOUSEWHEEL, wParam, lParam);
			return 0;
		}
	}

	GetWindowRect(m_pLog->GetHwnd(), &rc);
	if (PtInRect(&rc, pt)) {
		short wDirection = (short)HIWORD(wParam);

		if (LOWORD(wParam) & MK_SHIFT || M.GetByte("fastscroll", 0)) {
			if (wDirection < 0)
				SendMessage(m_pLog->GetHwnd(), WM_VSCROLL, MAKEWPARAM(SB_PAGEDOWN, 0), 0);
			else if (wDirection > 0)
				SendMessage(m_pLog->GetHwnd(), WM_VSCROLL, MAKEWPARAM(SB_PAGEUP, 0), 0);
		}
		else SendMessage(m_pLog->GetHwnd(), WM_MOUSEWHEEL, wParam, lParam);
		return 0;
	}

	if (GetTabItemFromMouse(m_pContainer->m_hwndTabs, &pt) != -1) {
		SendMessage(m_pContainer->m_hwndTabs, WM_MOUSEWHEEL, wParam, -1);
		return 0;
	}
	return 1;
}

void CMsgDialog::DM_FreeTheme()
{
	if (m_hTheme) {
		CloseThemeData(m_hTheme);
		m_hTheme = nullptr;
	}
	if (m_hThemeIP) {
		CloseThemeData(m_hThemeIP);
		m_hThemeIP = nullptr;
	}
	if (m_hThemeToolbar) {
		CloseThemeData(m_hThemeToolbar);
		m_hThemeToolbar = nullptr;
	}
}

void CMsgDialog::DM_ThemeChanged()
{
	CSkinItem *item_log = &SkinItems[ID_EXTBKHISTORY];
	CSkinItem *item_msg = &SkinItems[ID_EXTBKINPUTAREA];

	m_hTheme = OpenThemeData(m_hwnd, L"EDIT");

	if (m_hTheme != nullptr || (CSkin::m_skinEnabled && !item_log->IGNORED)) {
		if (m_iLogMode == WANT_BUILTIN_LOG)
			LOG()->DisableStaticEdge();

		if (isChat())
			SetWindowLongPtr(m_nickList.GetHwnd(), GWL_EXSTYLE, GetWindowLongPtr(m_nickList.GetHwnd(), GWL_EXSTYLE) & ~(WS_EX_CLIENTEDGE | WS_EX_STATICEDGE));
	}
	
	if (m_hTheme != nullptr || (CSkin::m_skinEnabled && !item_msg->IGNORED))
		SetWindowLongPtr(m_message.GetHwnd(), GWL_EXSTYLE, GetWindowLongPtr(m_message.GetHwnd(), GWL_EXSTYLE) & ~WS_EX_STATICEDGE);

	m_hThemeIP = M.isAero() ? OpenThemeData(m_hwnd, L"ButtonStyle") : nullptr;
	m_hThemeToolbar = (M.isAero() || (!CSkin::m_skinEnabled && M.isVSThemed())) ? OpenThemeData(m_hwnd, L"REBAR") : nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////
// send out message typing notifications (MTN) when the
// user is typing/editing text in the message input area.

void CMsgDialog::DM_NotifyTyping(int mode)
{
	const char *szProto = m_cache->getActiveProto();
	MCONTACT hContact = m_cache->getActiveContact();

	// editing user notes or preparing a message for queued delivery -> don't send MTN
	if (m_bEditNotesActive || (m_sendMode & SMODE_SENDLATER))
		return;

	// allow supression of sending out TN for the contact (NOTE: for metacontacts, do NOT use the subcontact handle)
	if (!g_plugin.getByte(hContact, SRMSGSET_TYPING, g_plugin.bTypingNew))
		return;

	if (szProto == nullptr) // should not, but who knows...
		return;

	// check status and capabilities of the protocol
	uint32_t typeCaps = CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_4, 0);
	if (!(typeCaps & PF4_SUPPORTTYPING))
		return;

	if (isChat()) {
		m_nTypeMode = mode;
		Chat_DoEventHook(m_si, GC_USER_TYPNOTIFY, 0, 0, m_nTypeMode);
	}
	else {
		uint32_t protoStatus = Proto_GetStatus(szProto);
		if (protoStatus < ID_STATUS_ONLINE)
			return;

		// check visibility/invisibility lists to not "accidentially" send MTN to contacts who
		// should not see them (privacy issue)
		uint32_t protoCaps = CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_1, 0);
		if (protoCaps & PF1_VISLIST && db_get_w(hContact, szProto, "ApparentMode", 0) == ID_STATUS_OFFLINE)
			return;

		if (protoCaps & PF1_INVISLIST && protoStatus == ID_STATUS_INVISIBLE && db_get_w(hContact, szProto, "ApparentMode", 0) != ID_STATUS_ONLINE)
			return;

		// don't send to contacts which are not permanently added to the contact list,
		// unless the option to ignore added status is set.
		if (!Contact::OnList(m_hContact) && !g_plugin.bTypingUnknown)
			return;

		// End user check
		m_nTypeMode = mode;
		CallService(MS_PROTO_SELFISTYPING, hContact, m_nTypeMode);
	}
}

void CMsgDialog::DM_OptionsApplied(bool bRemakeLog)
{
	m_szMicroLf[0] = 0;
	if (!m_pContainer->m_theme.isPrivate) {
		m_pContainer->LoadThemeDefaults();
		m_dwFlags = m_pContainer->m_theme.dwFlags;
	}

	LoadLocalFlags();
	m_hTimeZone = TimeZone_CreateByContact(m_hContact, nullptr, TZF_KNOWNONLY);

	m_bShowUIElements = (m_pContainer->cfg.flags.m_bHideToolbar) == 0;
	m_bSplitterOverride = M.GetByte(m_hContact, "splitoverride", 0) != 0;
	m_pPanel.getVisibility();

	// small inner margins (padding) for the text areas
	m_message.SendMsg(EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELONG(3, 3));

	GetSendFormat();
	SetDialogToType();
	SendMessage(m_hwnd, DM_CONFIGURETOOLBAR, 0, 0);

	DM_InitRichEdit();
	if (m_hwnd == m_pContainer->m_hwndActive)
		SendMessage(m_pContainer->m_hwnd, WM_SIZE, 0, 0);
	InvalidateRect(m_message.GetHwnd(), nullptr, FALSE);
	if (bRemakeLog) {
		if (IsIconic(m_pContainer->m_hwnd))
			m_bDeferredRemakeLog = true;
		else if (isChat())
			RedrawLog();
		else
			RemakeLog();
	}

	ShowWindow(m_hwndPanelPicParent, SW_SHOW);
	EnableWindow(m_hwndPanelPicParent, TRUE);

	UpdateWindowIcon();
}
	
void CMsgDialog::DM_Typing(bool fForceOff)
{
	HWND hwndContainer = m_pContainer->m_hwnd;
	HWND hwndStatus = m_pContainer->m_hwndStatus;

	if (m_nTypeMode == PROTOTYPE_SELFTYPING_ON && GetTickCount() - m_nLastTyping > TIMEOUT_TYPEOFF)
		DM_NotifyTyping(PROTOTYPE_SELFTYPING_OFF);

	if (m_bShowTyping == 1) {
		if (m_nTypeSecs > 0) {
			m_nTypeSecs--;
			if (GetForegroundWindow() == hwndContainer)
				UpdateWindowIcon();
		}
		else {
			if (!fForceOff) {
				m_bShowTyping = 2;
				m_nTypeSecs = 86400;

				if (!isChat())
					mir_snwprintf(m_wszStatusBar, TranslateT("%s has entered text."), m_cache->getNick());

				if (hwndStatus && m_pContainer->m_hwndActive == m_hwnd)
					SendMessage(hwndStatus, SB_SETTEXT, 0, (LPARAM)m_wszStatusBar);
			}
			UpdateWindowIcon();
			HandleIconFeedback(this, (HICON)-1);
			CMsgDialog *dat_active = (CMsgDialog*)GetWindowLongPtr(m_pContainer->m_hwndActive, GWLP_USERDATA);
			if (dat_active && !dat_active->isChat())
				m_pContainer->UpdateTitle(0);
			else
				m_pContainer->UpdateTitle(0, dat_active);
			if (!m_pContainer->cfg.flags.m_bNoFlash && PluginConfig.m_FlashOnMTN)
				m_pContainer->ReflashContainer();
		}
	}
	else if (m_bShowTyping == 2) {
		if (m_nTypeSecs > 0)
			m_nTypeSecs--;
		else {
			m_wszStatusBar[0] = 0;
			m_bShowTyping = 0;
		}
		tabUpdateStatusBar();
	}
	else if (m_nTypeSecs > 0) {
		mir_snwprintf(m_wszStatusBar, TranslateT("%s is typing a message"), 
			(m_pUserTyping) ? m_pUserTyping->pszNick : m_cache->getNick());

		m_nTypeSecs--;
		if (hwndStatus && m_pContainer->m_hwndActive == m_hwnd) {
			SendMessage(hwndStatus, SB_SETTEXT, 0, (LPARAM)m_wszStatusBar);
			SendMessage(hwndStatus, SB_SETICON, 0, (LPARAM)PluginConfig.g_buttonBarIcons[ICON_DEFAULT_TYPING]);
		}
		if (IsIconic(hwndContainer) || !IsActive()) {
			SetWindowText(hwndContainer, m_wszStatusBar);
			m_pContainer->cfg.flags.m_bNeedsUpdateTitle = true;
			if (!m_pContainer->cfg.flags.m_bNoFlash && PluginConfig.m_FlashOnMTN)
				m_pContainer->ReflashContainer();
		}

		if (m_pContainer->m_hwndActive != m_hwnd) {
			if (m_bCanFlashTab)
				m_iFlashIcon = PluginConfig.g_IconTypingEvent;
			HandleIconFeedback(this, PluginConfig.g_IconTypingEvent);
		}
		else { // active tab may show icon if status bar is disabled
			if (!hwndStatus) {
				if (TabCtrl_GetItemCount(m_hwndParent) > 1 || !m_pContainer->cfg.flags.m_bHideTabs)
					HandleIconFeedback(this, PluginConfig.g_IconTypingEvent);
			}
		}
		if ((GetForegroundWindow() != hwndContainer) || (m_pContainer->m_hwndStatus == nullptr) || (m_pContainer->m_hwndActive != m_hwnd))
			m_pContainer->SetIcon(this, PluginConfig.g_buttonBarIcons[ICON_DEFAULT_TYPING]);

		m_bShowTyping = 1;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// sync splitter position for all open sessions.
// This cares about private / per container / MUC <> IM splitter syncing and everything.
// called from IM and MUC windows via DM_SPLITTERGLOBALEVENT

int CMsgDialog::DM_SplitterGlobalEvent(WPARAM wParam, LPARAM lParam)
{
	CMsgDialog *srcDat = PluginConfig.lastSPlitterPos.pSrcDat;
	TContainerData *srcCnt = PluginConfig.lastSPlitterPos.pSrcContainer;
	bool fCntGlobal = (!m_pContainer->cfg.fPrivate ? true : false);

	if (m_bIsAutosizingInput)
		return 0;

	RECT rcWin;
	GetWindowRect(m_hwnd, &rcWin);

	LONG newPos;
	if (wParam == 0 && lParam == 0) {
		if (m_bSplitterOverride && this != srcDat)
			return 0;

		if (srcDat->isChat() == isChat())
			newPos = PluginConfig.lastSPlitterPos.pos;
		else if (!srcDat->isChat() && isChat())
			newPos = PluginConfig.lastSPlitterPos.pos + PluginConfig.lastSPlitterPos.off_im;
		else if (srcDat->isChat() && !isChat())
			newPos = PluginConfig.lastSPlitterPos.pos + PluginConfig.lastSPlitterPos.off_im;
		else
			newPos = 0;

		if (this == srcDat) {
			m_pContainer->cfg.iSplitterY = m_iSplitterY;
			if (fCntGlobal)
				SaveSplitter();
			return 0;
		}

		if (!fCntGlobal && m_pContainer != srcCnt)
			return 0;
		if (srcCnt->cfg.fPrivate && m_pContainer != srcCnt)
			return 0;

		// for inactive sessions, delay the splitter repositioning until they become
		// active (faster, avoid redraw/resize problems for minimized windows)
		if (IsIconic(m_pContainer->m_hwnd) || m_pContainer->m_hwndActive != m_hwnd) {
			m_bDelayedSplitter = true;
			m_wParam = newPos;
			m_lParam = PluginConfig.lastSPlitterPos.lParam;
			return 0;
		}
	}
	else newPos = wParam;

	LoadSplitter();
	AdjustBottomAvatarDisplay();
	DM_RecalcPictureSize();
	Resize();
	DM_ScrollToBottom(1, 1);
	if (this != srcDat)
		UpdateToolbarBG();
	return 0;
}

void CMsgDialog::DM_AddDivider()
{
	if (!m_bDividerSet && g_plugin.bUseDividers)
		if (GetWindowTextLength(m_pLog->GetHwnd()) > 0)
			m_bDividerSet = m_bDividerWanted = true;
}

/////////////////////////////////////////////////////////////////////////////////////////
// incoming event handler

void CMsgDialog::DM_EventAdded(WPARAM, LPARAM lParam)
{
	MEVENT hDbEvent = (MEVENT)lParam;

	DBEVENTINFO dbei = {};
	db_event_get(hDbEvent, &dbei);
	if (m_hDbEventFirst == 0)
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
		if (m_pContainer->m_hwndStatus)
			PostMessage(m_hwnd, DM_UPDATELASTMESSAGE, 0, 0);
	}

	// set the message log divider to mark new (maybe unseen) messages, if the container has
	// been minimized or in the background.
	if (!(dbei.flags & DBEF_SENT) && !bIsStatusChangeEvent) {
		if (g_plugin.bDividersUsePopupConfig && g_plugin.bUseDividers) {
			if (!MessageWindowOpened(m_hContact, nullptr))
				DM_AddDivider();
		}
		else if (g_plugin.bUseDividers) {
			if (!m_pContainer->IsActive())
				DM_AddDivider();
			else if (m_pContainer->m_hwndActive != m_hwnd)
				DM_AddDivider();
		}

		if (IsWindowVisible(m_pContainer->m_hwnd))
			m_pContainer->m_bHidden = false;
	}
	m_cache->updateStats(TSessionStats::UPDATE_WITH_LAST_RCV, 0);

	if (hDbEvent != m_hDbEventFirst || isChat())
		StreamEvents(hDbEvent, 1, 1);
	else
		RemakeLog();

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
			timerFlash.Start(TIMEOUT_FLASHWND);
			m_bCanFlashTab = true;
		}

	// try to flash the contact list...
	if (!bDisableNotify)
		FlashOnClist(hDbEvent, &dbei);

	// autoswitch tab if option is set AND container is minimized (otherwise, we never autoswitch)
	// never switch for status changes...
	if (!(dbei.flags & DBEF_SENT) && !bIsStatusChangeEvent) {
		if (g_plugin.bAutoSwitchTabs && m_pContainer->m_hwndActive != m_hwnd) {
			if ((IsIconic(m_pContainer->m_hwnd) && !IsZoomed(m_pContainer->m_hwnd)) || (g_plugin.bHideOnClose && !IsWindowVisible(m_pContainer->m_hwnd))) {
				int iItem = GetTabIndexFromHWND(GetParent(m_hwnd), m_hwnd);
				if (iItem >= 0) {
					TabCtrl_SetCurSel(m_hwndParent, iItem);
					ShowWindow(m_pContainer->m_hwndActive, SW_HIDE);
					m_pContainer->m_hwndActive = m_hwnd;
					m_pContainer->UpdateTitle(m_hContact);
					m_pContainer->cfg.flags.m_bDeferredTabSelect = true;
				}
			}
		}
	}

	// flash window if it is not focused
	if (!bDisableNotify && !bIsStatusChangeEvent)
		if (!IsActive() && !(dbei.flags & DBEF_SENT)) {
			if (!m_pContainer->cfg.flags.m_bNoFlash && !m_pContainer->IsActive())
				m_pContainer->FlashContainer(1, 0);
			m_pContainer->SetIcon(this, Skin_LoadIcon(SKINICON_EVENT_MESSAGE));
			m_pContainer->cfg.flags.m_bNeedsUpdateTitle = true;
		}

	// play a sound
	if (!bDisableNotify && dbei.eventType == EVENTTYPE_MESSAGE && !(dbei.flags & (DBEF_SENT)))
		PlayIncomingSound();
	
	if (m_pWnd)
		m_pWnd->Invalidate();
}

void CMsgDialog::DM_HandleAutoSizeRequest(REQRESIZE* rr)
{
	if (rr == nullptr || GetForegroundWindow() != m_pContainer->m_hwnd)
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
	LONG panelHeight = (m_pPanel.isActive() ? m_pPanel.getHeight() : 0);

	if (iNewHeight > (cy - panelHeight) / 2)
		iNewHeight = (cy - panelHeight) / 2;

	m_dynaSplitter = iNewHeight - DPISCALEY_S(2);
	if (m_pContainer->cfg.flags.m_bBottomToolbar)
		m_dynaSplitter += DPISCALEY_S(22);
	m_iSplitterY = m_dynaSplitter + DPISCALEY_S(34);
	DM_RecalcPictureSize();

	m_iInputAreaHeight = iNewHeight;
	UpdateToolbarBG();
	DM_ScrollToBottom(1, 0);
}

/////////////////////////////////////////////////////////////////////////////////////////
// status icon stuff (by sje, used for indicating encryption status in the status bar
// this is now part of the message window api


static int OnSrmmIconChanged(WPARAM hContact, LPARAM)
{
	if (hContact == 0)
		Srmm_Broadcast(DM_STATUSICONCHANGE, 0, 0);
	else {
		HWND hwnd = Srmm_FindWindow(hContact);
		if (hwnd)
			PostMessage(hwnd, DM_STATUSICONCHANGE, 0, 0);
	}
	return 0;
}

void CMsgDialog::DrawStatusIcons(HDC hDC, const RECT &rc, int gap)
{
	int x = rc.left;
	int y = (rc.top + rc.bottom - PluginConfig.m_smcxicon) >> 1;

	SetBkMode(hDC, TRANSPARENT);

	int nIcon = 0;
	while (StatusIconData *sid = Srmm_GetNthIcon(m_hContact, nIcon++)) {
		if (!mir_strcmp(sid->szModule, MSG_ICON_MODULE)) {
			if (sid->dwId == MSG_ICON_SOUND) {
				DrawIconEx(hDC, x, y, PluginConfig.g_buttonBarIcons[ICON_DEFAULT_SOUNDS],
					PluginConfig.m_smcxicon, PluginConfig.m_smcyicon, 0, nullptr, DI_NORMAL);

				DrawIconEx(hDC, x, y, m_pContainer->cfg.flags.m_bNoSound ?
					PluginConfig.g_iconOverlayDisabled : PluginConfig.g_iconOverlayEnabled,
					PluginConfig.m_smcxicon, PluginConfig.m_smcyicon, 0, nullptr, DI_NORMAL);
			}
			else if (sid->dwId == MSG_ICON_UTN) {
				if (AllowTyping()) {
					DrawIconEx(hDC, x, y, PluginConfig.g_buttonBarIcons[ICON_DEFAULT_TYPING], PluginConfig.m_smcxicon, PluginConfig.m_smcyicon, 0, nullptr, DI_NORMAL);

					DrawIconEx(hDC, x, y, g_plugin.getByte(m_hContact, SRMSGSET_TYPING, g_plugin.bTypingNew) ?
						PluginConfig.g_iconOverlayEnabled : PluginConfig.g_iconOverlayDisabled, PluginConfig.m_smcxicon, PluginConfig.m_smcyicon, 0, nullptr, DI_NORMAL);
				}
				else CSkin::DrawDimmedIcon(hDC, x, y, PluginConfig.m_smcxicon, PluginConfig.m_smcyicon, PluginConfig.g_buttonBarIcons[ICON_DEFAULT_TYPING], 50);
			}
		}
		else {
			HICON hIcon;
			if ((sid->flags & MBF_DISABLED) && sid->hIconDisabled)
				hIcon = sid->hIconDisabled;
			else
				hIcon = sid->hIcon;

			if ((sid->flags & MBF_DISABLED) && sid->hIconDisabled == nullptr)
				CSkin::DrawDimmedIcon(hDC, x, y, PluginConfig.m_smcxicon, PluginConfig.m_smcyicon, hIcon, 50);
			else
				DrawIconEx(hDC, x, y, hIcon, 16, 16, 0, nullptr, DI_NORMAL);
		}

		x += PluginConfig.m_smcxicon + gap;
	}
}

void CMsgDialog::CheckStatusIconClick(POINT pt, const RECT &rc, int gap, int code)
{
	if (code == NM_CLICK || code == NM_RCLICK) {
		POINT	ptScreen;
		GetCursorPos(&ptScreen);
		if (!PtInRect(&rcLastStatusBarClick, ptScreen))
			return;
	}

	UINT iconNum = (pt.x - (rc.left + 0)) / (PluginConfig.m_smcxicon + gap);
	
	StatusIconData *sid = Srmm_GetNthIcon(m_hContact, iconNum);
	if (sid == nullptr)
		return;

	if (!mir_strcmp(sid->szModule, MSG_ICON_MODULE)) {
		if (sid->dwId == MSG_ICON_SOUND && code != NM_RCLICK) {
			if (GetKeyState(VK_SHIFT) & 0x8000) {
				for (TContainerData *p = pFirstContainer; p; p = p->pNext) {
					p->cfg.flags.m_bNoSound = m_pContainer->cfg.flags.m_bNoSound;
					InvalidateRect(m_pContainer->m_hwndStatus, nullptr, TRUE);
				}
			}
			else {
				m_pContainer->cfg.flags.m_bNoSound = !m_pContainer->cfg.flags.m_bNoSound;
				InvalidateRect(m_pContainer->m_hwndStatus, nullptr, TRUE);
			}
		}
		else if (sid->dwId == MSG_ICON_UTN && code != NM_RCLICK && AllowTyping()) {
			SendMessage(m_pContainer->m_hwndActive, WM_COMMAND, IDC_SELFTYPING, 0);
			InvalidateRect(m_pContainer->m_hwndStatus, nullptr, TRUE);
		}
	}
	else {
		StatusIconClickData sicd = { sizeof(sicd) };
		GetCursorPos(&sicd.clickLocation);
		sicd.dwId = sid->dwId;
		sicd.szModule = sid->szModule;
		sicd.flags = (code == NM_RCLICK ? MBCF_RIGHTBUTTON : 0);
		Srmm_ClickStatusIcon(m_hContact, &sicd);
		InvalidateRect(m_pContainer->m_hwndStatus, nullptr, TRUE);
	}
}

void CMsgDialog::DM_ErrorDetected(int type, int flag)
{
	switch (type) {
	case MSGERROR_CANCEL:
	case MSGERROR_SENDLATER:
		if (m_bErrorState) {
			m_cache->saveHistory();
			if (type == MSGERROR_SENDLATER)
				sendQueue->doSendLater(m_iCurrentQueueError, this); // to be implemented at a later time
			m_iOpenJobs--;
			sendQueue->dec();
			if (m_iCurrentQueueError >= 0 && m_iCurrentQueueError < SendQueue::NR_SENDJOBS)
				sendQueue->clearJob(m_iCurrentQueueError);
			m_iCurrentQueueError = -1;
			sendQueue->showErrorControls(this, FALSE);
			if (type != MSGERROR_CANCEL || flag == 0)
				m_message.SetText(L"");
			sendQueue->checkQueue(this);
			int iNextFailed = sendQueue->findNextFailed(this);
			if (iNextFailed >= 0)
				sendQueue->handleError(this, iNextFailed);
		}
		break;

	case MSGERROR_RETRY:
		if (m_bErrorState) {
			int resent = 0;

			m_cache->saveHistory();
			if (m_iCurrentQueueError >= 0 && m_iCurrentQueueError < SendQueue::NR_SENDJOBS) {
				SendJob *job = sendQueue->getJobByIndex(m_iCurrentQueueError);
				if (job->iSendId == 0 && job->hContact == 0)
					break;

				job->iSendId = ProtoChainSend(job->hContact, PSS_MESSAGE, job->dwFlags, (LPARAM)job->szSendBuffer);
				resent++;
			}

			if (resent) {
				SendJob *job = sendQueue->getJobByIndex(m_iCurrentQueueError);

				SetTimer(m_hwnd, TIMERID_MSGSEND + m_iCurrentQueueError, PluginConfig.m_MsgTimeout, nullptr);
				job->iStatus = SendQueue::SQ_INPROGRESS;
				m_iCurrentQueueError = -1;
				sendQueue->showErrorControls(this, FALSE);
				m_message.SetText(L"");
				sendQueue->checkQueue(this);

				int iNextFailed = sendQueue->findNextFailed(this);
				if (iNextFailed >= 0)
					sendQueue->handleError(this, iNextFailed);
			}
		}
	}
}

int SI_InitStatusIcons()
{
	StatusIconData sid = {};
	sid.szModule = MSG_ICON_MODULE;
	sid.dwId = MSG_ICON_SOUND; // Sounds
	Srmm_AddIcon(&sid, &g_plugin);

	sid.dwId = MSG_ICON_UTN;
	Srmm_AddIcon(&sid, &g_plugin);

	HookEvent(ME_MSG_ICONSCHANGED, OnSrmmIconChanged);
	return 0;
}
