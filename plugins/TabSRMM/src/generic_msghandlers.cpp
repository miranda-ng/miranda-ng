/////////////////////////////////////////////////////////////////////////////////////////
// Miranda NG: the free IM client for Microsoft* Windows*
//
// Copyright (ñ) 2012-15 Miranda NG project,
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

#include "commonheaders.h"

/////////////////////////////////////////////////////////////////////////////////////////
// Save message log for given session as RTF document

void TSAPI DM_SaveLogAsRTF(const TWindowData *dat)
{
	if (dat && dat->hwndIEView != 0) {
		IEVIEWEVENT event = { sizeof(event) };
		event.hwnd = dat->hwndIEView;
		event.hContact = dat->hContact;
		event.iType = IEE_SAVE_DOCUMENT;
		CallService(MS_IEVIEW_EVENT, 0, (LPARAM)&event);
	}
	else if (dat) {
		TCHAR szFilter[MAX_PATH], szFilename[MAX_PATH];
		mir_sntprintf(szFilter, SIZEOF(szFilter), _T("%s%c*.rtf%c%c"), TranslateT("Rich Edit file"), 0, 0, 0);
		mir_sntprintf(szFilename, SIZEOF(szFilename), _T("%s.rtf"), dat->cache->getNick());

		Utils::sanitizeFilename(szFilename);

		TCHAR szInitialDir[MAX_PATH + 2];
		mir_sntprintf(szInitialDir, SIZEOF(szInitialDir), _T("%s%s\\"), M.getDataPath(), _T("\\Saved message logs"));
		CreateDirectoryTreeT(szInitialDir);

		OPENFILENAME ofn = { 0 };
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = dat->hwnd;
		ofn.lpstrFile = szFilename;
		ofn.lpstrFilter = szFilter;
		ofn.lpstrInitialDir = szInitialDir;
		ofn.nMaxFile = MAX_PATH;
		ofn.Flags = OFN_HIDEREADONLY;
		ofn.lpstrDefExt = _T("rtf");
		if (GetSaveFileName(&ofn)) {
			EDITSTREAM stream = { 0 };
			stream.dwCookie = (DWORD_PTR)szFilename;
			stream.dwError = 0;
			stream.pfnCallback = Utils::StreamOut;
			SendDlgItemMessage(dat->hwnd, dat->bType == SESSIONTYPE_IM ? IDC_LOG : IDC_CHAT_LOG, EM_STREAMOUT, SF_RTF | SF_USECODEPAGE, (LPARAM)&stream);
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

void TSAPI DM_CheckAutoHide(const TWindowData *dat, WPARAM wParam, LPARAM lParam)
{
	if (dat && lParam) {
		BOOL	*fResult = (BOOL *)lParam;

		if (GetWindowTextLength(GetDlgItem(dat->hwnd, dat->bType == SESSIONTYPE_IM ? IDC_MESSAGE : IDC_CHAT_MESSAGE)) > 0) {
			*fResult = FALSE;
			return;				// text entered in the input area -> prevent autohide/cose
		}
		if (dat->dwUnread) {
			*fResult = FALSE;
			return;				// unread events, do not hide or close the container
		}
		if (((GetTickCount() - dat->dwLastActivity) / 1000) <= wParam)
			*fResult = FALSE;		// time since last activity did not yet reach the threshold.
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// checks if the balloon tooltip can be dismissed (usually called by WM_MOUSEMOVE events)

void TSAPI DM_DismissTip(TWindowData *dat, const POINT& pt)
{
	if (!IsWindowVisible(dat->hwndTip))
		return;

	RECT rc;
	GetWindowRect(dat->hwndTip, &rc);
	if (PtInRect(&rc, pt))
		return;

	if (abs(pt.x - dat->ptTipActivation.x) > 5 || abs(pt.y - dat->ptTipActivation.y) > 5) {
		SendMessage(dat->hwndTip, TTM_TRACKACTIVATE, FALSE, 0);
		dat->ptTipActivation.x = dat->ptTipActivation.y = 0;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// initialize the balloon tooltip for message window notifications

void TSAPI DM_InitTip(TWindowData *dat)
{
	dat->hwndTip = CreateWindowEx(0, TOOLTIPS_CLASS, NULL, WS_POPUP | TTS_NOPREFIX | TTS_BALLOON, CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT, dat->hwnd, NULL, g_hInst, (LPVOID)NULL);

	memset(&dat->ti, 0, sizeof(dat->ti));
	dat->ti.cbSize = sizeof(dat->ti);
	dat->ti.lpszText = TranslateT("No status message");
	dat->ti.hinst = g_hInst;
	dat->ti.hwnd = dat->hwnd;
	dat->ti.uFlags = TTF_TRACK | TTF_IDISHWND | TTF_TRANSPARENT;
	dat->ti.uId = (UINT_PTR)dat->hwnd;
	SendMessage(dat->hwndTip, TTM_ADDTOOL, 0, (LPARAM)&dat->ti);

	SetWindowPos(dat->hwndTip, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER);
}

/////////////////////////////////////////////////////////////////////////////////////////
// checks generic hotkeys valid for both IM and MUC sessions
//
// returns 1 for handled hotkeys, 0 otherwise.

LRESULT TSAPI DM_GenericHotkeysCheck(MSG *message, TWindowData *dat)
{
	LRESULT mim_hotkey_check = CallService(MS_HOTKEY_CHECK, (WPARAM)message, (LPARAM)(TABSRMM_HK_SECTION_GENERIC));
	HWND	hwndDlg = dat->hwnd;

	switch (mim_hotkey_check) {
	case TABSRMM_HK_PASTEANDSEND:
		HandlePasteAndSend(dat);
		return 1;
	case TABSRMM_HK_HISTORY:
		SendMessage(hwndDlg, WM_COMMAND, IDC_HISTORY, 0);
		return 1;
	case TABSRMM_HK_CONTAINEROPTIONS:
		if (dat->pContainer->hWndOptions == 0)
			CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_CONTAINEROPTIONS), dat->pContainer->hwnd,
			DlgProcContainerOptions, (LPARAM)dat->pContainer);
		return 1;
	case TABSRMM_HK_SEND:
		if (!(GetWindowLongPtr(GetDlgItem(hwndDlg, IDC_MESSAGE), GWL_STYLE) & ES_READONLY)) {
			PostMessage(hwndDlg, WM_COMMAND, IDOK, 0);
			return 1;
		}
		break;
	case TABSRMM_HK_TOGGLEINFOPANEL:
		dat->Panel->setActive(!dat->Panel->isActive());
		dat->Panel->showHide();
		return 1;
	case TABSRMM_HK_EMOTICONS:
		SendMessage(hwndDlg, WM_COMMAND, IDC_SMILEYBTN, 0);
		return 1;
	case TABSRMM_HK_TOGGLETOOLBAR:
		SendMessage(hwndDlg, WM_COMMAND, IDC_TOGGLETOOLBAR, 0);
		return 1;
	case TABSRMM_HK_CLEARLOG:
		ClearLog(dat);
		return 1;
	case TABSRMM_HK_TOGGLESIDEBAR:
		if (dat->pContainer->SideBar->isActive())
			SendMessage(hwndDlg, WM_COMMAND, IDC_TOGGLESIDEBAR, 0);
		return 1;
	case TABSRMM_HK_CLOSE_OTHER:
		CloseOtherTabs(GetDlgItem(dat->pContainer->hwnd, IDC_MSGTABS), *dat);
		return 1;
	}
	return 0;
}

LRESULT TSAPI DM_MsgWindowCmdHandler(HWND hwndDlg, TContainerData *m_pContainer, TWindowData *dat, UINT cmd, WPARAM wParam, LPARAM lParam)
{
	RECT  rc;
	HWND  hwndContainer = m_pContainer->hwnd;
	int   iSelection;
	HMENU submenu;

	switch (cmd) {
	case IDC_FONTBOLD:
	case IDC_FONTITALIC:
	case IDC_FONTUNDERLINE:
	case IDC_FONTSTRIKEOUT:
		if (dat->SendFormat != 0) { // dont use formatting if disabled
			CHARFORMAT2 cf, cfOld;
			memset(&cf, 0, sizeof(CHARFORMAT2));
			memset(&cfOld, 0, sizeof(CHARFORMAT2));
			cfOld.cbSize = cf.cbSize = sizeof(CHARFORMAT2);
			cfOld.dwMask = CFM_BOLD | CFM_ITALIC | CFM_UNDERLINE | CFM_STRIKEOUT;
			SendDlgItemMessage(hwndDlg, IDC_MESSAGE, EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM)&cfOld);
			BOOL isBold = (cfOld.dwEffects & CFE_BOLD) && (cfOld.dwMask & CFM_BOLD);
			BOOL isItalic = (cfOld.dwEffects & CFE_ITALIC) && (cfOld.dwMask & CFM_ITALIC);
			BOOL isUnderline = (cfOld.dwEffects & CFE_UNDERLINE) && (cfOld.dwMask & CFM_UNDERLINE);
			BOOL isStrikeout = (cfOld.dwEffects & CFM_STRIKEOUT) && (cfOld.dwMask & CFM_STRIKEOUT);

			int cmd = LOWORD(wParam);
			if (cmd == IDC_FONTBOLD && !IsWindowEnabled(GetDlgItem(hwndDlg, IDC_FONTBOLD)))
				break;
			if (cmd == IDC_FONTITALIC && !IsWindowEnabled(GetDlgItem(hwndDlg, IDC_FONTITALIC)))
				break;
			if (cmd == IDC_FONTUNDERLINE && !IsWindowEnabled(GetDlgItem(hwndDlg, IDC_FONTUNDERLINE)))
				break;
			if (cmd == IDC_FONTSTRIKEOUT && !IsWindowEnabled(GetDlgItem(hwndDlg, IDC_FONTSTRIKEOUT)))
				break;
			if (cmd == IDC_FONTBOLD) {
				cf.dwEffects = isBold ? 0 : CFE_BOLD;
				cf.dwMask = CFM_BOLD;
				CheckDlgButton(hwndDlg, IDC_FONTBOLD, !isBold ? BST_CHECKED : BST_UNCHECKED);
			}
			else if (cmd == IDC_FONTITALIC) {
				cf.dwEffects = isItalic ? 0 : CFE_ITALIC;
				cf.dwMask = CFM_ITALIC;
				CheckDlgButton(hwndDlg, IDC_FONTITALIC, !isItalic ? BST_CHECKED : BST_UNCHECKED);
			}
			else if (cmd == IDC_FONTUNDERLINE) {
				cf.dwEffects = isUnderline ? 0 : CFE_UNDERLINE;
				cf.dwMask = CFM_UNDERLINE;
				CheckDlgButton(hwndDlg, IDC_FONTUNDERLINE, !isUnderline ? BST_CHECKED : BST_UNCHECKED);
			}
			else if (cmd == IDC_FONTSTRIKEOUT) {
				cf.dwEffects = isStrikeout ? 0 : CFM_STRIKEOUT;
				cf.dwMask = CFM_STRIKEOUT;
				CheckDlgButton(hwndDlg, IDC_FONTSTRIKEOUT, !isStrikeout ? BST_CHECKED : BST_UNCHECKED);
			}
			SendDlgItemMessage(hwndDlg, IDC_MESSAGE, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
		}
		break;

	case IDC_FONTFACE:
		submenu = GetSubMenu(m_pContainer->hMenuContext, 7);
		{
			CHARFORMAT2 cf;
			memset(&cf, 0, sizeof(CHARFORMAT2));
			cf.cbSize = sizeof(CHARFORMAT2);
			cf.dwMask = CFM_COLOR;

			GetWindowRect(GetDlgItem(hwndDlg, IDC_FONTFACE), &rc);
			iSelection = TrackPopupMenu(submenu, TPM_RETURNCMD, rc.left, rc.bottom, 0, hwndDlg, NULL);
			if (iSelection == ID_FONT_CLEARALLFORMATTING) {
				cf.dwMask = CFM_BOLD | CFM_COLOR | CFM_ITALIC | CFM_UNDERLINE | CFM_STRIKEOUT;
				cf.crTextColor = M.GetDword(FONTMODULE, "Font16Col", 0);
				SendDlgItemMessage(hwndDlg, IDC_MESSAGE, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
				break;
			}
			if (iSelection == ID_FONT_DEFAULTCOLOR) {
				cf.crTextColor = M.GetDword(FONTMODULE, "Font16Col", 0);
				for (int i = 0; i < Utils::rtf_ctable_size; i++)
					if (Utils::rtf_ctable[i].clr == cf.crTextColor)
						cf.crTextColor = RGB(GetRValue(cf.crTextColor), GetGValue(cf.crTextColor), GetBValue(cf.crTextColor) == 0 ? GetBValue(cf.crTextColor) + 1 : GetBValue(cf.crTextColor) - 1);

				SendDlgItemMessage(hwndDlg, IDC_MESSAGE, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
				break;
			}
			for (int i = 0; i < RTF_CTABLE_DEFSIZE; i++)
				if (Utils::rtf_ctable[i].menuid == iSelection) {
					cf.crTextColor = Utils::rtf_ctable[i].clr;
					SendDlgItemMessage(hwndDlg, IDC_MESSAGE, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
				}
		}
		break;

	case IDCANCEL:
		ShowWindow(hwndContainer, SW_MINIMIZE);
		return FALSE;

	case IDC_SAVE:
		SendMessage(hwndDlg, WM_CLOSE, 1, 0);
		break;

	case IDC_NAME:
		if (GetKeyState(VK_SHIFT) & 0x8000)   // copy UIN
			SendMessage(hwndDlg, DM_UINTOCLIPBOARD, 0, 0);
		else
			CallService(MS_USERINFO_SHOWDIALOG, (WPARAM)(dat->cache->getActiveContact()), 0);
		break;

	case IDC_HISTORY:
		CallService(MS_HISTORY_SHOWCONTACTHISTORY, dat->hContact, 0);
		break;

	case IDC_SMILEYBTN:
		if (dat->bShowSmileys && PluginConfig.g_SmileyAddAvail) {
			MCONTACT hContact = dat->cache->getActiveContact();
			if (CheckValidSmileyPack(dat->cache->getProto(), hContact) != 0) {
				SMADD_SHOWSEL3 smaddInfo = { 0 };

				if (lParam == 0)
					GetWindowRect(GetDlgItem(hwndDlg, IDC_SMILEYBTN), &rc);
				else
					GetWindowRect((HWND)lParam, &rc);
				smaddInfo.cbSize = sizeof(SMADD_SHOWSEL3);
				smaddInfo.hwndTarget = GetDlgItem(hwndDlg, IDC_MESSAGE);
				smaddInfo.targetMessage = EM_REPLACESEL;
				smaddInfo.targetWParam = TRUE;
				smaddInfo.Protocolname = const_cast<char *>(dat->cache->getProto());
				smaddInfo.Direction = 0;
				smaddInfo.xPosition = rc.left;
				smaddInfo.yPosition = rc.top + 24;
				smaddInfo.hwndParent = hwndContainer;
				smaddInfo.hContact = hContact;
				CallService(MS_SMILEYADD_SHOWSELECTION, (WPARAM)hwndContainer, (LPARAM)&smaddInfo);
			}
		}
		break;

	case IDC_TIME:
		submenu = GetSubMenu(m_pContainer->hMenuContext, 2);
		MsgWindowUpdateMenu(dat, submenu, MENU_LOGMENU);

		GetWindowRect(GetDlgItem(hwndDlg, IDC_TIME), &rc);

		iSelection = TrackPopupMenu(submenu, TPM_RETURNCMD, rc.left, rc.bottom, 0, hwndDlg, NULL);
		return MsgWindowMenuHandler(dat, iSelection, MENU_LOGMENU);

	case IDC_PROTOMENU:
		if (dat->hContact) {
			submenu = GetSubMenu(m_pContainer->hMenuContext, 4);
			int iOldGlobalSendFormat = PluginConfig.m_SendFormat;
			int iLocalFormat = M.GetDword(dat->hContact, "sendformat", 0);
			int iNewLocalFormat = iLocalFormat;

			GetWindowRect(GetDlgItem(hwndDlg, IDC_PROTOCOL), &rc);

			CheckMenuItem(submenu, ID_MODE_GLOBAL, MF_BYCOMMAND | (!(dat->dwFlagsEx & MWF_SHOW_SPLITTEROVERRIDE) ? MF_CHECKED : MF_UNCHECKED));
			CheckMenuItem(submenu, ID_MODE_PRIVATE, MF_BYCOMMAND | (dat->dwFlagsEx & MWF_SHOW_SPLITTEROVERRIDE ? MF_CHECKED : MF_UNCHECKED));

			// formatting menu..
			CheckMenuItem(submenu, ID_GLOBAL_BBCODE, MF_BYCOMMAND | ((PluginConfig.m_SendFormat) ? MF_CHECKED : MF_UNCHECKED));
			CheckMenuItem(submenu, ID_GLOBAL_OFF, MF_BYCOMMAND | ((PluginConfig.m_SendFormat == SENDFORMAT_NONE) ? MF_CHECKED : MF_UNCHECKED));

			CheckMenuItem(submenu, ID_THISCONTACT_GLOBALSETTING, MF_BYCOMMAND | ((iLocalFormat == SENDFORMAT_NONE) ? MF_CHECKED : MF_UNCHECKED));
			CheckMenuItem(submenu, ID_THISCONTACT_BBCODE, MF_BYCOMMAND | ((iLocalFormat > 0) ? MF_CHECKED : MF_UNCHECKED));
			CheckMenuItem(submenu, ID_THISCONTACT_OFF, MF_BYCOMMAND | ((iLocalFormat == -1) ? MF_CHECKED : MF_UNCHECKED));

			iSelection = TrackPopupMenu(submenu, TPM_RETURNCMD, rc.left, rc.bottom, 0, hwndDlg, NULL);
			switch (iSelection) {
			case ID_MODE_GLOBAL:
				dat->dwFlagsEx &= ~(MWF_SHOW_SPLITTEROVERRIDE);
				db_set_b(dat->hContact, SRMSGMOD_T, "splitoverride", 0);
				LoadSplitter(dat);
				AdjustBottomAvatarDisplay(dat);
				DM_RecalcPictureSize(dat);
				SendMessage(hwndDlg, WM_SIZE, 0, 0);
				break;
			case ID_MODE_PRIVATE:
				dat->dwFlagsEx |= MWF_SHOW_SPLITTEROVERRIDE;
				db_set_b(dat->hContact, SRMSGMOD_T, "splitoverride", 1);
				LoadSplitter(dat);
				AdjustBottomAvatarDisplay(dat);
				DM_RecalcPictureSize(dat);
				SendMessage(hwndDlg, WM_SIZE, 0, 0);
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
				db_unset(dat->hContact, SRMSGMOD_T, "sendformat");
			else if (iNewLocalFormat != iLocalFormat)
				db_set_dw(dat->hContact, SRMSGMOD_T, "sendformat", iNewLocalFormat);

			if (PluginConfig.m_SendFormat != iOldGlobalSendFormat)
				db_set_b(0, SRMSGMOD_T, "sendformat", (BYTE)PluginConfig.m_SendFormat);
			if (iNewLocalFormat != iLocalFormat || PluginConfig.m_SendFormat != iOldGlobalSendFormat) {
				dat->SendFormat = M.GetDword(dat->hContact, "sendformat", PluginConfig.m_SendFormat);
				if (dat->SendFormat == -1)          // per contact override to disable it..
					dat->SendFormat = 0;
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
		submenu = GetSubMenu(m_pContainer->hMenuContext, 9);
		GetWindowRect(GetDlgItem(hwndDlg, IDC_NAME), &rc);

		EnableMenuItem(submenu, ID_FAVORITES_ADDCONTACTTOFAVORITES, !dat->cache->isFavorite() ? MF_ENABLED : MF_GRAYED);
		EnableMenuItem(submenu, ID_FAVORITES_REMOVECONTACTFROMFAVORITES, !dat->cache->isFavorite() ? MF_GRAYED : MF_ENABLED);

		iSelection = TrackPopupMenu(submenu, TPM_RETURNCMD, rc.left, rc.bottom, 0, hwndDlg, NULL);

		switch (iSelection) {
		case ID_FAVORITES_ADDCONTACTTOFAVORITES:
			db_set_b(dat->hContact, SRMSGMOD_T, "isFavorite", 1);
			AddContactToFavorites(dat->hContact, dat->cache->getNick(), dat->cache->getProto(), dat->szStatus, dat->wStatus, LoadSkinnedProtoIcon(dat->cache->getProto(), dat->cache->getStatus()), 1, PluginConfig.g_hMenuFavorites);
			break;
		case ID_FAVORITES_REMOVECONTACTFROMFAVORITES:
			db_set_b(dat->hContact, SRMSGMOD_T, "isFavorite", 0);
			DeleteMenu(PluginConfig.g_hMenuFavorites, (UINT_PTR)dat->hContact, MF_BYCOMMAND);
			break;
		}
		dat->cache->updateFavorite();
		break;

	case IDC_SENDMENU:
		submenu = GetSubMenu(m_pContainer->hMenuContext, 3);

		GetWindowRect(GetDlgItem(hwndDlg, IDOK), &rc);
		CheckMenuItem(submenu, ID_SENDMENU_SENDTOMULTIPLEUSERS, MF_BYCOMMAND | (dat->sendMode & SMODE_MULTIPLE ? MF_CHECKED : MF_UNCHECKED));
		CheckMenuItem(submenu, ID_SENDMENU_SENDDEFAULT, MF_BYCOMMAND | (dat->sendMode == 0 ? MF_CHECKED : MF_UNCHECKED));
		CheckMenuItem(submenu, ID_SENDMENU_SENDTOCONTAINER, MF_BYCOMMAND | (dat->sendMode & SMODE_CONTAINER ? MF_CHECKED : MF_UNCHECKED));
		CheckMenuItem(submenu, ID_SENDMENU_FORCEANSISEND, MF_BYCOMMAND | (dat->sendMode & SMODE_FORCEANSI ? MF_CHECKED : MF_UNCHECKED));
		CheckMenuItem(submenu, ID_SENDMENU_SENDLATER, MF_BYCOMMAND | (dat->sendMode & SMODE_SENDLATER ? MF_CHECKED : MF_UNCHECKED));
		CheckMenuItem(submenu, ID_SENDMENU_SENDWITHOUTTIMEOUTS, MF_BYCOMMAND | (dat->sendMode & SMODE_NOACK ? MF_CHECKED : MF_UNCHECKED));

		EnableMenuItem(submenu, ID_SENDMENU_SENDNUDGE, MF_BYCOMMAND | ((ProtoServiceExists(dat->cache->getActiveProto(), PS_SEND_NUDGE) && ServiceExists(MS_NUDGE_SEND)) ? MF_ENABLED : MF_GRAYED));

		if (lParam)
			iSelection = TrackPopupMenu(submenu, TPM_RETURNCMD, rc.left, rc.bottom, 0, hwndDlg, NULL);
		else
			iSelection = HIWORD(wParam);

		switch (iSelection) {
		case ID_SENDMENU_SENDTOMULTIPLEUSERS:
			dat->sendMode ^= SMODE_MULTIPLE;
			if (dat->sendMode & SMODE_MULTIPLE)
				DM_CreateClist(dat);
			else if (IsWindow(GetDlgItem(hwndDlg, IDC_CLIST)))
				DestroyWindow(GetDlgItem(hwndDlg, IDC_CLIST));
			break;
		case ID_SENDMENU_SENDNUDGE:
			SendNudge(dat);
			break;
		case ID_SENDMENU_SENDDEFAULT:
			dat->sendMode = 0;
			break;
		case ID_SENDMENU_SENDTOCONTAINER:
			dat->sendMode ^= SMODE_CONTAINER;
			RedrawWindow(hwndDlg, 0, 0, RDW_ERASENOW | RDW_UPDATENOW);
			break;
		case ID_SENDMENU_FORCEANSISEND:
			dat->sendMode ^= SMODE_FORCEANSI;
			break;
		case ID_SENDMENU_SENDLATER:
			if (sendLater->isAvail())
				dat->sendMode ^= SMODE_SENDLATER;
			else
				CWarning::show(CWarning::WARN_NO_SENDLATER, MB_OK | MB_ICONINFORMATION, TranslateT("Configuration issue|The unattended send feature is disabled. The \\b1 send later\\b0  and \\b1 send to multiple contacts\\b0  features depend on it.\n\nYou must enable it under \\b1Options->Message sessions->Advanced tweaks\\b0. Changing this option requires a restart."));
			break;
		case ID_SENDMENU_SENDWITHOUTTIMEOUTS:
			dat->sendMode ^= SMODE_NOACK;
			if (dat->sendMode & SMODE_NOACK)
				db_set_b(dat->hContact, SRMSGMOD_T, "no_ack", 1);
			else
				db_unset(dat->hContact, SRMSGMOD_T, "no_ack");
			break;
		}
		db_set_b(dat->hContact, SRMSGMOD_T, "no_ack", (BYTE)(dat->sendMode & SMODE_NOACK ? 1 : 0));
		db_set_b(dat->hContact, SRMSGMOD_T, "forceansi", (BYTE)(dat->sendMode & SMODE_FORCEANSI ? 1 : 0));
		SetWindowPos(GetDlgItem(hwndDlg, IDC_MESSAGE), 0, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOSIZE | SWP_NOMOVE);
		if (dat->sendMode & SMODE_MULTIPLE || dat->sendMode & SMODE_CONTAINER) {
			SetWindowPos(GetDlgItem(hwndDlg, IDC_MESSAGE), 0, 0, 0, 0, 0, SWP_DRAWFRAME | SWP_FRAMECHANGED | SWP_NOZORDER |
				SWP_NOMOVE | SWP_NOSIZE | SWP_NOCOPYBITS);
			RedrawWindow(hwndDlg, 0, 0, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW | RDW_ALLCHILDREN);
		}
		else {
			if (IsWindow(GetDlgItem(hwndDlg, IDC_CLIST)))
				DestroyWindow(GetDlgItem(hwndDlg, IDC_CLIST));
			SetWindowPos(GetDlgItem(hwndDlg, IDC_MESSAGE), 0, 0, 0, 0, 0, SWP_DRAWFRAME | SWP_FRAMECHANGED | SWP_NOZORDER |
				SWP_NOMOVE | SWP_NOSIZE | SWP_NOCOPYBITS);
			RedrawWindow(hwndDlg, 0, 0, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW | RDW_ALLCHILDREN);
		}
		SendMessage(hwndContainer, DM_QUERYCLIENTAREA, 0, (LPARAM)&rc);
		SendMessage(hwndDlg, WM_SIZE, 0, 0);
		DM_ScrollToBottom(dat, 1, 1);
		Utils::showDlgControl(hwndDlg, IDC_MULTISPLITTER, (dat->sendMode & SMODE_MULTIPLE) ? SW_SHOW : SW_HIDE);
		Utils::showDlgControl(hwndDlg, IDC_CLIST, (dat->sendMode & SMODE_MULTIPLE) ? SW_SHOW : SW_HIDE);
		break;

	case IDC_TOGGLESIDEBAR:
		SendMessage(m_pContainer->hwnd, WM_COMMAND, IDC_TOGGLESIDEBAR, 0);
		break;

	case IDC_PIC:
		GetClientRect(hwndDlg, &rc);

		dat->fEditNotesActive = !dat->fEditNotesActive;
		if (dat->fEditNotesActive) {
			int iLen = GetWindowTextLength(GetDlgItem(hwndDlg, IDC_MESSAGE));
			if (iLen != 0) {
				SendMessage(hwndDlg, DM_ACTIVATETOOLTIP, IDC_MESSAGE, (LPARAM)TranslateT("You cannot edit user notes when there are unsent messages"));
				dat->fEditNotesActive = false;
				break;
			}

			if (!dat->bIsAutosizingInput) {
				dat->iSplitterSaved = dat->splitterY;
				dat->splitterY = rc.bottom / 2;
				SendMessage(hwndDlg, WM_SIZE, 1, 1);
			}

			DBVARIANT dbv = { 0 };

			if (0 == db_get_ts(dat->hContact, "UserInfo", "MyNotes", &dbv)) {
				SetDlgItemText(hwndDlg, IDC_MESSAGE, dbv.ptszVal);
				mir_free(dbv.ptszVal);
			}
		}
		else {
			int iLen = GetWindowTextLength(GetDlgItem(hwndDlg, IDC_MESSAGE));

			TCHAR *buf = (TCHAR*)mir_alloc((iLen + 2) * sizeof(TCHAR));
			GetDlgItemText(hwndDlg, IDC_MESSAGE, buf, iLen + 1);
			db_set_ts(dat->hContact, "UserInfo", "MyNotes", buf);
			SetDlgItemText(hwndDlg, IDC_MESSAGE, _T(""));

			if (!dat->bIsAutosizingInput) {
				dat->splitterY = dat->iSplitterSaved;
				SendMessage(hwndDlg, WM_SIZE, 0, 0);
				DM_ScrollToBottom(dat, 0, 1);
			}
		}
		SetWindowPos(GetDlgItem(hwndDlg, IDC_MESSAGE), 0, 0, 0, 0, 0, SWP_DRAWFRAME | SWP_FRAMECHANGED | SWP_NOZORDER |
			SWP_NOMOVE | SWP_NOSIZE | SWP_NOCOPYBITS);
		RedrawWindow(hwndDlg, 0, 0, RDW_INVALIDATE | RDW_ERASE | RDW_FRAME | RDW_UPDATENOW | RDW_ALLCHILDREN);

		if (dat->fEditNotesActive)
			CWarning::show(CWarning::WARN_EDITUSERNOTES, MB_OK | MB_ICONINFORMATION);
		break;

	case IDM_CLEAR:
		ClearLog(dat);
		break;

	case IDC_PROTOCOL:
		submenu = (HMENU)CallService(MS_CLIST_MENUBUILDCONTACT, dat->hContact, 0);
		if (lParam == 0)
			GetWindowRect(GetDlgItem(hwndDlg, IDC_PROTOCOL), &rc);
		else
			GetWindowRect((HWND)lParam, &rc);

		iSelection = TrackPopupMenu(submenu, TPM_RETURNCMD, rc.left, rc.bottom, 0, hwndDlg, NULL);
		if (iSelection)
			CallService(MS_CLIST_MENUPROCESSCOMMAND, MAKEWPARAM(LOWORD(iSelection), MPCF_CONTACTMENU), (LPARAM)dat->hContact);

		DestroyMenu(submenu);
		break;

		// error control
	case IDC_CANCELSEND:
		SendMessage(hwndDlg, DM_ERRORDECIDED, MSGERROR_CANCEL, 0);
		break;

	case IDC_RETRY:
		SendMessage(hwndDlg, DM_ERRORDECIDED, MSGERROR_RETRY, 0);
		break;

	case IDC_MSGSENDLATER:
		SendMessage(hwndDlg, DM_ERRORDECIDED, MSGERROR_SENDLATER, 0);
		break;

	case IDC_SELFTYPING:
		if (dat->hContact) {
			int iCurrentTypingMode = db_get_b(dat->hContact, SRMSGMOD, SRMSGSET_TYPING, M.GetByte(SRMSGMOD, SRMSGSET_TYPINGNEW, SRMSGDEFSET_TYPINGNEW));

			if (dat->nTypeMode == PROTOTYPE_SELFTYPING_ON && iCurrentTypingMode) {
				DM_NotifyTyping(dat, PROTOTYPE_SELFTYPING_OFF);
				dat->nTypeMode = PROTOTYPE_SELFTYPING_OFF;
			}
			db_set_b(dat->hContact, SRMSGMOD, SRMSGSET_TYPING, (BYTE)!iCurrentTypingMode);
		}
		break;

	default:
		return 0;
	}
	return 1;
}

static INT_PTR CALLBACK DlgProcAbout(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{
			WORD v[4];
			CallService(MS_SYSTEM_GETFILEVERSION, 0, (LPARAM)&v);

			TCHAR tStr[80];
			mir_sntprintf(tStr, SIZEOF(tStr), _T("%s %d.%d.%d.%d [build %d]"),
				TranslateT("Version"), __MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM, v[3]);
			SetDlgItemText(hwndDlg, IDC_HEADERBAR, tStr);
		}
		SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM)LoadSkinnedIconBig(SKINICON_EVENT_MESSAGE));
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
		case IDCANCEL:
			DestroyWindow(hwndDlg);
			return TRUE;
		case IDC_SUPPORT:
			CallService(MS_UTILS_OPENURL, OUF_NEWWINDOW, (LPARAM)"http://miranda.or.at/");
			break;
		}
		break;

	case WM_CTLCOLOREDIT:
	case WM_CTLCOLORSTATIC:
		SetTextColor((HDC)wParam, RGB(60, 60, 150));
		SetBkColor((HDC)wParam, GetSysColor(COLOR_WINDOW));
		return (INT_PTR)GetSysColorBrush(COLOR_WINDOW);
	}
	return FALSE;
}

LRESULT TSAPI DM_ContainerCmdHandler(TContainerData *pContainer, UINT cmd, WPARAM wParam, LPARAM lParam)
{
	if (!pContainer)
		return 0;

	HWND hwndDlg = pContainer->hwnd;
	TWindowData *dat = (TWindowData*)GetWindowLongPtr(pContainer->hwndActive, GWLP_USERDATA);

	switch (cmd) {
	case IDC_CLOSE:
		SendMessage(hwndDlg, WM_SYSCOMMAND, SC_CLOSE, 0);
		break;
	case IDC_MINIMIZE:
		PostMessage(hwndDlg, WM_SYSCOMMAND, SC_MINIMIZE, 0);
		break;
	case IDC_MAXIMIZE:
		SendMessage(hwndDlg, WM_SYSCOMMAND, IsZoomed(hwndDlg) ? SC_RESTORE : SC_MAXIMIZE, 0);
		break;
	case IDOK:
		SendMessage(pContainer->hwndActive, WM_COMMAND, wParam, lParam);      // pass the IDOK command to the active child - fixes the "enter not working
		break;
	case ID_FILE_SAVEMESSAGELOGAS:
		SendMessage(pContainer->hwndActive, DM_SAVEMESSAGELOG, 0, 0);
		break;
	case ID_FILE_CLOSEMESSAGESESSION:
		PostMessage(pContainer->hwndActive, WM_CLOSE, 0, 1);
		break;
	case ID_FILE_CLOSE:
		PostMessage(hwndDlg, WM_CLOSE, 0, 1);
		break;
	case ID_VIEW_SHOWSTATUSBAR:
		ApplyContainerSetting(pContainer, CNT_NOSTATUSBAR, pContainer->dwFlags & CNT_NOSTATUSBAR ? 0 : 1, true);
		break;
	case ID_VIEW_VERTICALMAXIMIZE:
		ApplyContainerSetting(pContainer, CNT_VERTICALMAX, pContainer->dwFlags & CNT_VERTICALMAX ? 0 : 1, false);
		break;
	case ID_VIEW_BOTTOMTOOLBAR:
		ApplyContainerSetting(pContainer, CNT_BOTTOMTOOLBAR, pContainer->dwFlags & CNT_BOTTOMTOOLBAR ? 0 : 1, false);
		M.BroadcastMessage(DM_CONFIGURETOOLBAR, 0, 1);
		return 0;
	case ID_VIEW_SHOWTOOLBAR:
		ApplyContainerSetting(pContainer, CNT_HIDETOOLBAR, pContainer->dwFlags & CNT_HIDETOOLBAR ? 0 : 1, false);
		M.BroadcastMessage(DM_CONFIGURETOOLBAR, 0, 1);
		return 0;
	case ID_VIEW_SHOWMENUBAR:
		ApplyContainerSetting(pContainer, CNT_NOMENUBAR, pContainer->dwFlags & CNT_NOMENUBAR ? 0 : 1, true);
		break;
	case ID_VIEW_SHOWTITLEBAR:
		ApplyContainerSetting(pContainer, CNT_NOTITLE, pContainer->dwFlags & CNT_NOTITLE ? 0 : 1, true);
		break;
	case ID_VIEW_TABSATBOTTOM:
		ApplyContainerSetting(pContainer, CNT_TABSBOTTOM, pContainer->dwFlags & CNT_TABSBOTTOM ? 0 : 1, false);
		break;
	case ID_VIEW_SHOWMULTISENDCONTACTLIST:
		SendMessage(pContainer->hwndActive, WM_COMMAND, MAKEWPARAM(IDC_SENDMENU, ID_SENDMENU_SENDTOMULTIPLEUSERS), 0);
		break;
	case ID_VIEW_STAYONTOP:
		SendMessage(hwndDlg, WM_SYSCOMMAND, IDM_STAYONTOP, 0);
		break;
	case ID_CONTAINER_CONTAINEROPTIONS:
		SendMessage(hwndDlg, WM_SYSCOMMAND, IDM_MOREOPTIONS, 0);
		break;
	case ID_EVENTPOPUPS_DISABLEALLEVENTPOPUPS:
		ApplyContainerSetting(pContainer, (CNT_DONTREPORT | CNT_DONTREPORTUNFOCUSED | CNT_DONTREPORTFOCUSED | CNT_ALWAYSREPORTINACTIVE), 0, false);
		return 0;
	case ID_EVENTPOPUPS_SHOWPOPUPSIFWINDOWISMINIMIZED:
		ApplyContainerSetting(pContainer, CNT_DONTREPORT, pContainer->dwFlags & CNT_DONTREPORT ? 0 : 1, false);
		return 0;
	case ID_EVENTPOPUPS_SHOWPOPUPSIFWINDOWISUNFOCUSED:
		ApplyContainerSetting(pContainer, CNT_DONTREPORTUNFOCUSED, pContainer->dwFlags & CNT_DONTREPORTUNFOCUSED ? 0 : 1, false);
		return 0;
	case ID_EVENTPOPUPS_SHOWPOPUPSIFWINDOWISFOCUSED:
		ApplyContainerSetting(pContainer, CNT_DONTREPORTFOCUSED, pContainer->dwFlags & CNT_DONTREPORTFOCUSED ? 0 : 1, false);
		return 0;
	case ID_EVENTPOPUPS_SHOWPOPUPSFORALLINACTIVESESSIONS:
		ApplyContainerSetting(pContainer, CNT_ALWAYSREPORTINACTIVE, pContainer->dwFlags & CNT_ALWAYSREPORTINACTIVE ? 0 : 1, false);
		return 0;
	case ID_WINDOWFLASHING_DISABLEFLASHING:
		ApplyContainerSetting(pContainer, CNT_NOFLASH, 1, false);
		ApplyContainerSetting(pContainer, CNT_FLASHALWAYS, 0, false);
		return 0;
	case ID_WINDOWFLASHING_FLASHUNTILFOCUSED:
		ApplyContainerSetting(pContainer, CNT_NOFLASH, 0, false);
		ApplyContainerSetting(pContainer, CNT_FLASHALWAYS, 1, false);
		return 0;
	case ID_WINDOWFLASHING_USEDEFAULTVALUES:
		ApplyContainerSetting(pContainer, (CNT_NOFLASH | CNT_FLASHALWAYS), 0, false);
		return 0;
	case ID_OPTIONS_SAVECURRENTWINDOWPOSITIONASDEFAULT:
	{
		WINDOWPLACEMENT wp = { 0 };
		wp.length = sizeof(wp);
		if (GetWindowPlacement(hwndDlg, &wp)) {
			db_set_dw(0, SRMSGMOD_T, "splitx", wp.rcNormalPosition.left);
			db_set_dw(0, SRMSGMOD_T, "splity", wp.rcNormalPosition.top);
			db_set_dw(0, SRMSGMOD_T, "splitwidth", wp.rcNormalPosition.right - wp.rcNormalPosition.left);
			db_set_dw(0, SRMSGMOD_T, "splitheight", wp.rcNormalPosition.bottom - wp.rcNormalPosition.top);
		}
	}
	return 0;

	case ID_VIEW_INFOPANEL:
		if (dat) {
			RECT	rc;
			POINT	pt;
			GetWindowRect(pContainer->hwndActive, &rc);
			pt.x = rc.left + 10;
			pt.y = rc.top + dat->Panel->getHeight() - 10;
			dat->Panel->invokeConfigDialog(pt);
		}
		return 0;

		// commands from the message log popup will be routed to the
		// message log menu handler
	case ID_MESSAGELOGSETTINGS_FORTHISCONTACT:
	case ID_MESSAGELOGSETTINGS_GLOBAL:
		if (dat) {
			MsgWindowMenuHandler(dat, (int)LOWORD(wParam), MENU_LOGMENU);
			return 1;
		}
		break;

	case ID_HELP_ABOUTTABSRMM:
		CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_ABOUT), 0, DlgProcAbout, 0);
		break;

	default:
		return 0; 		// not handled
	}
	return 1;				// handled
}

/////////////////////////////////////////////////////////////////////////////////////////
// initialize rich edit control (log and edit control) for both MUC and
// standard IM session windows.

void TSAPI DM_InitRichEdit(TWindowData *dat)
{
	char *szStreamOut = NULL;
	bool  fIsChat = ((dat->bType == SESSIONTYPE_CHAT) ? true : false);
	HWND  hwndLog = GetDlgItem(dat->hwnd, !fIsChat ? IDC_LOG : IDC_CHAT_LOG);
	HWND  hwndEdit = GetDlgItem(dat->hwnd, !fIsChat ? IDC_MESSAGE : IDC_CHAT_MESSAGE);
	HWND  hwndDlg = dat->hwnd;

	dat->inputbg = fIsChat ? M.GetDword(FONTMODULE, "inputbg", SRMSGDEFSET_BKGCOLOUR) : dat->pContainer->theme.inputbg;
	COLORREF colour = fIsChat ? g_Settings.crLogBackground : dat->pContainer->theme.bg;
	COLORREF inputcharcolor;

	if (!fIsChat && GetWindowTextLength(hwndEdit) > 0)
		szStreamOut = Message_GetFromStream(hwndEdit);
	SetWindowText(hwndEdit, _T(""));

	SendMessage(hwndLog, EM_SETBKGNDCOLOR, 0, colour);
	SendMessage(hwndEdit, EM_SETBKGNDCOLOR, 0, dat->inputbg);

	CHARFORMAT2A cf2;
	memset(&cf2, 0, sizeof(CHARFORMAT2A));
	cf2.cbSize = sizeof(cf2);

	if (fIsChat) {
		LOGFONTA lf;
		LoadLogfont(MSGFONTID_MESSAGEAREA, &lf, &inputcharcolor, FONTMODULE);

		cf2.dwMask = CFM_COLOR | CFM_FACE | CFM_CHARSET | CFM_SIZE | CFM_WEIGHT | CFM_ITALIC | CFM_BACKCOLOR;
		cf2.crTextColor = inputcharcolor;
		cf2.bCharSet = lf.lfCharSet;
		cf2.crBackColor = dat->inputbg;
		strncpy(cf2.szFaceName, lf.lfFaceName, LF_FACESIZE);
		cf2.dwEffects = 0;
		cf2.wWeight = (WORD)lf.lfWeight;
		cf2.bPitchAndFamily = lf.lfPitchAndFamily;
		cf2.yHeight = abs(lf.lfHeight) * 15;
	}
	else {
		LOGFONTA lf = dat->pContainer->theme.logFonts[MSGFONTID_MESSAGEAREA];
		inputcharcolor = dat->pContainer->theme.fontColors[MSGFONTID_MESSAGEAREA];

		for (int i = 0; i < Utils::rtf_ctable_size; i++)
			if (Utils::rtf_ctable[i].clr == inputcharcolor)
				inputcharcolor = RGB(GetRValue(inputcharcolor), GetGValue(inputcharcolor), GetBValue(inputcharcolor) == 0 ? GetBValue(inputcharcolor) + 1 : GetBValue(inputcharcolor) - 1);

		cf2.dwMask = CFM_COLOR | CFM_FACE | CFM_CHARSET | CFM_SIZE | CFM_WEIGHT | CFM_BOLD | CFM_ITALIC;
		cf2.crTextColor = inputcharcolor;
		cf2.bCharSet = lf.lfCharSet;
		strncpy(cf2.szFaceName, lf.lfFaceName, LF_FACESIZE-1);
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
	if (Utils::FindRTLLocale(dat))
		SendMessage(hwndEdit, EM_SETPARAFORMAT, 0, (LPARAM)&pf2);
	if (!(dat->dwFlags & MWF_LOG_RTL)) {
		pf2.wEffects = 0;
		SendMessage(hwndEdit, EM_SETPARAFORMAT, 0, (LPARAM)&pf2);
	}
	SendMessage(hwndEdit, EM_SETLANGOPTIONS, 0, (LPARAM)SendMessage(hwndEdit, EM_GETLANGOPTIONS, 0, 0) & ~IMF_AUTOKEYBOARD);
	pf2.wEffects = PFE_RTLPARA;
	pf2.dwMask |= PFM_OFFSET;
	if (dat->dwFlags & MWF_INITMODE) {
		pf2.dwMask |= (PFM_RIGHTINDENT | PFM_OFFSETINDENT);
		pf2.dxStartIndent = 30;
		pf2.dxRightIndent = 30;
	}
	pf2.dxOffset = dat->pContainer->theme.left_indent + 30;

	if (!fIsChat) {
		SetWindowText(hwndLog, _T(""));
		SendMessage(hwndLog, EM_SETPARAFORMAT, 0, (LPARAM)&pf2);
		SendMessage(hwndLog, EM_SETLANGOPTIONS, 0, (LPARAM)SendDlgItemMessage(hwndDlg, IDC_LOG, EM_GETLANGOPTIONS, 0, 0) & ~IMF_AUTOKEYBOARD);
		// set the scrollbars etc to RTL/LTR (only for manual RTL mode)
		if (dat->dwFlags & MWF_LOG_RTL) {
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

void TSAPI DM_SetDBButtonStates(HWND hwndChild, TWindowData *dat)
{
	ButtonItem *buttonItem = dat->pContainer->buttonItems;
	MCONTACT hContact = dat->hContact, hFinalContact = 0;
	HWND hwndContainer = dat->pContainer->hwnd;

	while (buttonItem) {
		HWND hWnd = GetDlgItem(hwndContainer, buttonItem->uId);

		if (buttonItem->pfnCallback)
			buttonItem->pfnCallback(buttonItem, hwndChild, dat, hWnd);

		if (!(buttonItem->dwFlags & BUTTON_ISTOGGLE && buttonItem->dwFlags & BUTTON_ISDBACTION)) {
			buttonItem = buttonItem->nextItem;
			continue;
		}

		BOOL result = FALSE;
		char *szModule = buttonItem->szModule;
		char *szSetting = buttonItem->szSetting;
		if (buttonItem->dwFlags & BUTTON_DBACTIONONCONTACT || buttonItem->dwFlags & BUTTON_ISCONTACTDBACTION) {
			if (hContact == 0) {
				SendMessage(hWnd, BM_SETCHECK, BST_UNCHECKED, 0);
				buttonItem = buttonItem->nextItem;
				continue;
			}
			if (buttonItem->dwFlags & BUTTON_ISCONTACTDBACTION)
				szModule = GetContactProto(hContact);
			hFinalContact = hContact;
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

void TSAPI DM_ScrollToBottom(TWindowData *dat, WPARAM wParam, LPARAM lParam)
{
	if (dat == NULL)
		return;

	if (dat->dwFlagsEx & MWF_SHOW_SCROLLINGDISABLED)
		return;

	if (IsIconic(dat->pContainer->hwnd))
		dat->dwFlags |= MWF_DEFERREDSCROLL;

	if (dat->hwndIEView) {
		PostMessage(dat->hwnd, DM_SCROLLIEVIEW, 0, 0);
		return;
	}
	if (dat->hwndHPP) {
		SendMessage(dat->hwnd, DM_SCROLLIEVIEW, 0, 0);
		return;
	}

	HWND hwnd = GetDlgItem(dat->hwnd, dat->bType == SESSIONTYPE_IM ? IDC_LOG : IDC_CHAT_LOG);
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
	DBVARIANT dbv;
	if (!db_get_ts((MCONTACT)_param, SRMSGMOD_T, "locale", &dbv)) {
		HKL hkl = LoadKeyboardLayout(dbv.ptszVal, 0);
		PostMessage(PluginConfig.g_hwndHotkeyHandler, DM_SETLOCALE, (WPARAM)_param, (LPARAM)hkl);
		db_free(&dbv);
	}
}

void TSAPI DM_LoadLocale(TWindowData *dat)
{
	if (dat == NULL || !PluginConfig.m_bAutoLocaleSupport)
		return;

	if (dat->dwFlags & MWF_WASBACKGROUNDCREATE)
		return;

	DBVARIANT dbv;
	if (!db_get_ts(dat->hContact, SRMSGMOD_T, "locale", &dbv))
		db_free(&dbv);
	else {
		TCHAR szKLName[KL_NAMELENGTH + 1];
		if (!PluginConfig.m_bDontUseDefaultKbd) {
			TCHAR	szBuf[20];
			GetLocaleInfo(LOCALE_SYSTEM_DEFAULT, LOCALE_ILANGUAGE, szBuf, 20);
			mir_sntprintf(szKLName, SIZEOF(szKLName), _T("0000%s"), szBuf);
			db_set_ts(dat->hContact, SRMSGMOD_T, "locale", szKLName);
		}
		else {
			GetKeyboardLayoutName(szKLName);
			db_set_ts(dat->hContact, SRMSGMOD_T, "locale", szKLName);
		}
	}

	mir_forkthread(LoadKLThread, (void*)dat->hContact);
}

LRESULT TSAPI DM_RecalcPictureSize(TWindowData *dat)
{
	if (dat) {
		HBITMAP hbm = ((dat->Panel->isActive()) && dat->pContainer->avatarMode != 3) ? dat->hOwnPic : (dat->ace ? dat->ace->hbmPic : PluginConfig.g_hbmUnknown);
		if (hbm) {
			BITMAP bminfo;
			GetObject(hbm, sizeof(bminfo), &bminfo);
			CalcDynamicAvatarSize(dat, &bminfo);
			SendMessage(dat->hwnd, WM_SIZE, 0, 0);
		}
		else dat->pic.cy = dat->pic.cx = 60;
	}
	return 0;
}

void TSAPI DM_UpdateLastMessage(const TWindowData *dat)
{
	if (dat == NULL)
		return;

	if (dat->pContainer->hwndStatus == 0 || dat->pContainer->hwndActive != dat->hwnd)
		return;

	TCHAR szBuf[100];
	if (dat->bShowTyping) {
		SendMessage(dat->pContainer->hwndStatus, SB_SETICON, 0, (LPARAM)PluginConfig.g_buttonBarIcons[ICON_DEFAULT_TYPING]);
		mir_sntprintf(szBuf, SIZEOF(szBuf), TranslateT("%s is typing a message..."), dat->cache->getNick());
	}
	else if (dat->sbCustom) {
		SendMessage(dat->pContainer->hwndStatus, SB_SETICON, 0, (LPARAM)dat->sbCustom->hIcon);
		SendMessage(dat->pContainer->hwndStatus, SB_SETTEXT, 0, (LPARAM)dat->sbCustom->tszText);
		return;
	}
	else {
		SendMessage(dat->pContainer->hwndStatus, SB_SETICON, 0, 0);

		if (dat->pContainer->dwFlags & CNT_UINSTATUSBAR)
			mir_sntprintf(szBuf, SIZEOF(szBuf), _T("UID: %s"), dat->cache->getUIN());
		else if (dat->lastMessage) {
			TCHAR date[64], time[64];
			tmi.printTimeStamp(NULL, dat->lastMessage, _T("d"), date, SIZEOF(date), 0);
			if (dat->pContainer->dwFlags & CNT_UINSTATUSBAR && mir_tstrlen(date) > 6)
				date[mir_tstrlen(date) - 5] = 0;
			tmi.printTimeStamp(NULL, dat->lastMessage, _T("t"), time, SIZEOF(time), 0);
			mir_sntprintf(szBuf, SIZEOF(szBuf), TranslateT("Last received: %s at %s"), date, time);
		}
		else szBuf[0] = 0;
	}

	SendMessage(dat->pContainer->hwndStatus, SB_SETTEXT, 0, (LPARAM)szBuf);
}

/////////////////////////////////////////////////////////////////////////////////////////
// save current keyboard layout for the given contact

void TSAPI DM_SaveLocale(TWindowData *dat, WPARAM, LPARAM lParam)
{
	if (!dat)
		return;

	if (PluginConfig.m_bAutoLocaleSupport && dat->hContact && dat->pContainer->hwndActive == dat->hwnd) {
		TCHAR szKLName[KL_NAMELENGTH + 1];
		if ((HKL)lParam != dat->hkl) {
			dat->hkl = (HKL)lParam;
			ActivateKeyboardLayout(dat->hkl, 0);
			GetKeyboardLayoutName(szKLName);
			db_set_ts(dat->hContact, SRMSGMOD_T, "locale", szKLName);
			GetLocaleID(dat, szKLName);
			UpdateReadChars(dat);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// generic handler for the WM_COPY message in message log/chat history richedit control(s).
// it filters out the invisible event boundary markers from the text copied to the clipboard.
// WINE Fix: overwrite clippboad data from original control data
LRESULT TSAPI DM_WMCopyHandler(HWND hwnd, WNDPROC oldWndProc, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LRESULT result = mir_callNextSubclass(hwnd, oldWndProc, msg, wParam, lParam);

	ptrA szFromStream(Message_GetFromStream(hwnd, SF_TEXT | SFF_SELECTION));
	if (szFromStream != NULL) {
		ptrW converted(mir_utf8decodeW(szFromStream));
		if (converted != NULL) {
			Utils::FilterEventMarkers(converted);
			Utils::CopyToClipBoard(converted, hwnd);
		}
	}

	return result;
}

/////////////////////////////////////////////////////////////////////////////////////////
// create embedded contact list control

HWND TSAPI DM_CreateClist(TWindowData *dat)
{
	if (!sendLater->isAvail()) {
		CWarning::show(CWarning::WARN_NO_SENDLATER, MB_OK | MB_ICONINFORMATION, TranslateT("Configuration issue|The unattended send feature is disabled. The \\b1 send later\\b0  and \\b1 send to multiple contacts\\b0  features depend on it.\n\nYou must enable it under \\b1Options->Message sessions->Advanced tweaks\\b0. Changing this option requires a restart."));
		dat->sendMode &= ~SMODE_MULTIPLE;
		return 0;
	}

	HWND hwndClist = CreateWindowExA(0, "CListControl", "", WS_TABSTOP | WS_VISIBLE | WS_CHILD | 0x248,
		184, 0, 30, 30, dat->hwnd, (HMENU)IDC_CLIST, g_hInst, NULL);
	SendMessage(hwndClist, WM_TIMER, 14, 0);
	HANDLE hItem = (HANDLE)SendMessage(hwndClist, CLM_FINDCONTACT, dat->hContact, 0);

	SetWindowLongPtr(hwndClist, GWL_EXSTYLE, GetWindowLongPtr(hwndClist, GWL_EXSTYLE) & ~CLS_EX_TRACKSELECT);
	SetWindowLongPtr(hwndClist, GWL_EXSTYLE, GetWindowLongPtr(hwndClist, GWL_EXSTYLE) | (CLS_EX_NOSMOOTHSCROLLING | CLS_EX_NOTRANSLUCENTSEL));

	if (!PluginConfig.m_bAllowOfflineMultisend)
		SetWindowLongPtr(hwndClist, GWL_STYLE, GetWindowLongPtr(hwndClist, GWL_STYLE) | CLS_HIDEOFFLINE);

	if (hItem)
		SendMessage(hwndClist, CLM_SETCHECKMARK, (WPARAM)hItem, 1);

	if (CallService(MS_CLUI_GETCAPS, 0, 0) & CLUIF_DISABLEGROUPS && !M.GetByte("CList", "UseGroups", SETTING_USEGROUPS_DEFAULT))
		SendMessage(hwndClist, CLM_SETUSEGROUPS, FALSE, 0);
	else
		SendMessage(hwndClist, CLM_SETUSEGROUPS, TRUE, 0);
	if (CallService(MS_CLUI_GETCAPS, 0, 0) & CLUIF_HIDEEMPTYGROUPS && M.GetByte("CList", "HideEmptyGroups", SETTING_USEGROUPS_DEFAULT))
		SendMessage(hwndClist, CLM_SETHIDEEMPTYGROUPS, TRUE, 0);
	else
		SendMessage(hwndClist, CLM_SETHIDEEMPTYGROUPS, FALSE, 0);
	SendMessage(hwndClist, CLM_FIRST + 106, 0, 1);
	SendMessage(hwndClist, CLM_AUTOREBUILD, 0, 0);
	if (hwndClist)
		RedrawWindow(hwndClist, 0, 0, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW);
	return hwndClist;
}

LRESULT TSAPI DM_MouseWheelHandler(HWND hwnd, HWND hwndParent, TWindowData *mwdat, WPARAM wParam, LPARAM lParam)
{
	POINT pt;
	RECT rc, rc1;
	UINT uID = mwdat->bType == SESSIONTYPE_IM ? IDC_LOG : IDC_CHAT_LOG;
	UINT uIDMsg = mwdat->bType == SESSIONTYPE_IM ? IDC_MESSAGE : IDC_CHAT_MESSAGE;

	GetCursorPos(&pt);
	GetWindowRect(hwnd, &rc);
	if (PtInRect(&rc, pt))
		return 1;

	if (mwdat->pContainer->dwFlags & CNT_SIDEBAR) {
		GetWindowRect(GetDlgItem(mwdat->pContainer->hwnd, IDC_SIDEBARUP), &rc);
		GetWindowRect(GetDlgItem(mwdat->pContainer->hwnd, IDC_SIDEBARDOWN), &rc1);
		rc.bottom = rc1.bottom;
		if (PtInRect(&rc, pt)) {
			short amount = (short)(HIWORD(wParam));
			SendMessage(mwdat->pContainer->hwnd, WM_COMMAND, MAKELONG(amount > 0 ? IDC_SIDEBARUP : IDC_SIDEBARDOWN, 0), (LPARAM)uIDMsg);
			return 0;
		}
	}
	if (mwdat->bType == SESSIONTYPE_CHAT) {					// scroll nick list by just hovering it
		RECT	rcNicklist;
		GetWindowRect(GetDlgItem(mwdat->hwnd, IDC_LIST), &rcNicklist);
		if (PtInRect(&rcNicklist, pt)) {
			SendDlgItemMessage(mwdat->hwnd, IDC_LIST, WM_MOUSEWHEEL, wParam, lParam);
			return 0;
		}
	}
	if (mwdat->hwndIEView)
		GetWindowRect(mwdat->hwndIEView, &rc);
	else if (mwdat->hwndHPP)
		GetWindowRect(mwdat->hwndHPP, &rc);
	else
		GetWindowRect(GetDlgItem(hwndParent, uID), &rc);
	if (PtInRect(&rc, pt)) {
		HWND hwnd = (mwdat->hwndIEView || mwdat->hwndHPP) ? mwdat->hwndIWebBrowserControl : GetDlgItem(hwndParent, uID);
		short wDirection = (short)HIWORD(wParam);

		if (hwnd == 0)
			hwnd = WindowFromPoint(pt);

		if (LOWORD(wParam) & MK_SHIFT || M.GetByte("fastscroll", 0)) {
			if (wDirection < 0)
				SendMessage(hwnd, WM_VSCROLL, MAKEWPARAM(SB_PAGEDOWN, 0), 0);
			else if (wDirection > 0)
				SendMessage(hwnd, WM_VSCROLL, MAKEWPARAM(SB_PAGEUP, 0), 0);
		}
		else SendMessage(hwnd, WM_MOUSEWHEEL, wParam, lParam);
		return 0;
	}

	HWND hwndTab = GetDlgItem(mwdat->pContainer->hwnd, IDC_MSGTABS);
	if (GetTabItemFromMouse(hwndTab, &pt) != -1) {
		SendMessage(hwndTab, WM_MOUSEWHEEL, wParam, -1);
		return 0;
	}
	return 1;
}

void TSAPI DM_FreeTheme(TWindowData *dat)
{
	if (dat) {
		if (dat->hTheme) {
			CloseThemeData(dat->hTheme);
			dat->hTheme = 0;
		}
		if (dat->hThemeIP) {
			CloseThemeData(dat->hThemeIP);
			dat->hThemeIP = 0;
		}
		if (dat->hThemeToolbar) {
			CloseThemeData(dat->hThemeToolbar);
			dat->hThemeToolbar = 0;
		}
	}
}

LRESULT TSAPI DM_ThemeChanged(TWindowData *dat)
{
	CSkinItem *item_log = &SkinItems[ID_EXTBKHISTORY];
	CSkinItem *item_msg = &SkinItems[ID_EXTBKINPUTAREA];

	HWND	hwnd = dat->hwnd;

	dat->hTheme = OpenThemeData(hwnd, L"EDIT");

	if (dat->bType == SESSIONTYPE_IM) {
		if (dat->hTheme != 0 || (CSkin::m_skinEnabled && !item_log->IGNORED))
			SetWindowLongPtr(GetDlgItem(hwnd, IDC_LOG), GWL_EXSTYLE, GetWindowLongPtr(GetDlgItem(hwnd, IDC_LOG), GWL_EXSTYLE) & ~WS_EX_STATICEDGE);
		if (dat->hTheme != 0 || (CSkin::m_skinEnabled && !item_msg->IGNORED))
			SetWindowLongPtr(GetDlgItem(hwnd, IDC_MESSAGE), GWL_EXSTYLE, GetWindowLongPtr(GetDlgItem(hwnd, IDC_MESSAGE), GWL_EXSTYLE) & ~WS_EX_STATICEDGE);
	}
	else {
		if (dat->hTheme != 0 || (CSkin::m_skinEnabled && !item_log->IGNORED)) {
			SetWindowLongPtr(GetDlgItem(hwnd, IDC_CHAT_LOG), GWL_EXSTYLE, GetWindowLongPtr(GetDlgItem(hwnd, IDC_CHAT_LOG), GWL_EXSTYLE) & ~WS_EX_STATICEDGE);
			SetWindowLongPtr(GetDlgItem(hwnd, IDC_LIST), GWL_EXSTYLE, GetWindowLongPtr(GetDlgItem(hwnd, IDC_LIST), GWL_EXSTYLE) & ~(WS_EX_CLIENTEDGE | WS_EX_STATICEDGE));
		}
		if (dat->hTheme != 0 || (CSkin::m_skinEnabled && !item_msg->IGNORED))
			SetWindowLongPtr(GetDlgItem(hwnd, IDC_CHAT_MESSAGE), GWL_EXSTYLE, GetWindowLongPtr(GetDlgItem(hwnd, IDC_CHAT_MESSAGE), GWL_EXSTYLE) & ~WS_EX_STATICEDGE);
	}
	dat->hThemeIP = M.isAero() ? OpenThemeData(hwnd, L"ButtonStyle") : 0;
	dat->hThemeToolbar = (M.isAero() || (!CSkin::m_skinEnabled && M.isVSThemed())) ? OpenThemeData(hwnd, L"REBAR") : 0;

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// send out message typing notifications (MTN) when the
// user is typing/editing text in the message input area.

void TSAPI DM_NotifyTyping(TWindowData *dat, int mode)
{
	if (!dat || !dat->hContact)
		return;

	DeletePopupsForContact(dat->hContact, PU_REMOVE_ON_TYPE);

	const char *szProto = dat->cache->getActiveProto();
	MCONTACT hContact = dat->cache->getActiveContact();

	// editing user notes or preparing a message for queued delivery -> don't send MTN
	if (dat->fEditNotesActive || dat->sendMode & SMODE_SENDLATER)
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
	if (db_get_b(dat->hContact, "CList", "NotOnList", 0) && !M.GetByte(SRMSGMOD, SRMSGSET_TYPINGUNKNOWN, SRMSGDEFSET_TYPINGUNKNOWN))
		return;

	// End user check
	dat->nTypeMode = mode;
	CallService(MS_PROTO_SELFISTYPING, hContact, dat->nTypeMode);
}

void TSAPI DM_OptionsApplied(TWindowData *dat, WPARAM, LPARAM lParam)
{
	if (dat == 0)
		return;

	HWND hwndDlg = dat->hwnd;
	TContainerData *m_pContainer = dat->pContainer;

	dat->szMicroLf[0] = 0;
	if (!(dat->pContainer->theme.isPrivate)) {
		LoadThemeDefaults(dat->pContainer);
		dat->dwFlags = dat->pContainer->theme.dwFlags;
	}
	LoadLocalFlags(dat);

	LoadTimeZone(dat);

	dat->bShowUIElements = (m_pContainer->dwFlags & CNT_HIDETOOLBAR) == 0;

	dat->dwFlagsEx = M.GetByte(dat->hContact, "splitoverride", 0) ? MWF_SHOW_SPLITTEROVERRIDE : 0;
	dat->Panel->getVisibility();

	// small inner margins (padding) for the text areas
	SendDlgItemMessage(hwndDlg, IDC_LOG, EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELONG(0, 0));
	SendDlgItemMessage(hwndDlg, IDC_MESSAGE, EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELONG(3, 3));

	GetSendFormat(dat);
	SetDialogToType(hwndDlg);
	SendMessage(hwndDlg, DM_CONFIGURETOOLBAR, 0, 0);

	DM_InitRichEdit(dat);
	if (hwndDlg == m_pContainer->hwndActive)
		SendMessage(m_pContainer->hwnd, WM_SIZE, 0, 0);
	InvalidateRect(GetDlgItem(hwndDlg, IDC_MESSAGE), NULL, FALSE);
	if (!lParam) {
		if (IsIconic(m_pContainer->hwnd))
			dat->dwFlags |= MWF_DEFERREDREMAKELOG;
		else
			SendMessage(hwndDlg, DM_REMAKELOG, 0, 0);
	}

	ShowWindow(dat->hwndPanelPicParent, SW_SHOW);
	EnableWindow(dat->hwndPanelPicParent, TRUE);

	SendMessage(hwndDlg, DM_UPDATEWINICON, 0, 0);
}

void TSAPI DM_Typing(TWindowData *dat, bool fForceOff)
{
	if (dat == 0)
		return;

	HWND hwndDlg = dat->hwnd;
	HWND hwndContainer = dat->pContainer->hwnd;
	HWND hwndStatus = dat->pContainer->hwndStatus;

	if (dat->nTypeMode == PROTOTYPE_SELFTYPING_ON && GetTickCount() - dat->nLastTyping > TIMEOUT_TYPEOFF)
		DM_NotifyTyping(dat, PROTOTYPE_SELFTYPING_OFF);

	if (dat->bShowTyping == 1) {
		if (dat->nTypeSecs > 0) {
			dat->nTypeSecs--;
			if (GetForegroundWindow() == hwndContainer)
				SendMessage(hwndDlg, DM_UPDATEWINICON, 0, 0);
		}
		else {
			if (!fForceOff) {
				dat->bShowTyping = 2;
				dat->nTypeSecs = 86400;

				mir_sntprintf(dat->szStatusBar, SIZEOF(dat->szStatusBar), TranslateT("%s has entered text."), dat->cache->getNick());
				if (hwndStatus && dat->pContainer->hwndActive == hwndDlg)
					SendMessage(hwndStatus, SB_SETTEXT, 0, (LPARAM)dat->szStatusBar);
			}
			SendMessage(hwndDlg, DM_UPDATEWINICON, 0, 0);
			HandleIconFeedback(dat, (HICON)-1);
			TWindowData *dat_active = (TWindowData*)GetWindowLongPtr(dat->pContainer->hwndActive, GWLP_USERDATA);
			if (dat_active && dat_active->bType == SESSIONTYPE_IM)
				SendMessage(hwndContainer, DM_UPDATETITLE, 0, 0);
			else
				SendMessage(hwndContainer, DM_UPDATETITLE, (WPARAM)dat->pContainer->hwndActive, 1);
			if (!(dat->pContainer->dwFlags & CNT_NOFLASH) && PluginConfig.m_FlashOnMTN)
				ReflashContainer(dat->pContainer);
		}
	}
	else if (dat->bShowTyping == 2) {
		if (dat->nTypeSecs > 0)
			dat->nTypeSecs--;
		else {
			dat->szStatusBar[0] = 0;
			dat->bShowTyping = 0;
		}
		UpdateStatusBar(dat);
	}
	else if (dat->nTypeSecs > 0) {
		mir_sntprintf(dat->szStatusBar, SIZEOF(dat->szStatusBar), TranslateT("%s is typing a message"), dat->cache->getNick());

		dat->nTypeSecs--;
		if (hwndStatus && dat->pContainer->hwndActive == hwndDlg) {
			SendMessage(hwndStatus, SB_SETTEXT, 0, (LPARAM)dat->szStatusBar);
			SendMessage(hwndStatus, SB_SETICON, 0, (LPARAM)PluginConfig.g_buttonBarIcons[ICON_DEFAULT_TYPING]);
		}
		if (IsIconic(hwndContainer) || GetForegroundWindow() != hwndContainer || GetActiveWindow() != hwndContainer) {
			SetWindowText(hwndContainer, dat->szStatusBar);
			dat->pContainer->dwFlags |= CNT_NEED_UPDATETITLE;
			if (!(dat->pContainer->dwFlags & CNT_NOFLASH) && PluginConfig.m_FlashOnMTN)
				ReflashContainer(dat->pContainer);
		}

		if (dat->pContainer->hwndActive != hwndDlg) {
			if (dat->mayFlashTab)
				dat->iFlashIcon = PluginConfig.g_IconTypingEvent;
			HandleIconFeedback(dat, PluginConfig.g_IconTypingEvent);
		}
		else { // active tab may show icon if status bar is disabled
			if (!hwndStatus) {
				if (TabCtrl_GetItemCount(GetParent(hwndDlg)) > 1 || !(dat->pContainer->dwFlags & CNT_HIDETABS))
					HandleIconFeedback(dat, PluginConfig.g_IconTypingEvent);
			}
		}
		if ((GetForegroundWindow() != hwndContainer) || (dat->pContainer->hwndStatus == 0) || (dat->pContainer->hwndActive != hwndDlg))
			SendMessage(hwndContainer, DM_SETICON, (WPARAM)dat, (LPARAM)PluginConfig.g_buttonBarIcons[ICON_DEFAULT_TYPING]);

		dat->bShowTyping = 1;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// sync splitter position for all open sessions.
// This cares about private / per container / MUC <> IM splitter syncing and everything.
// called from IM and MUC windows via DM_SPLITTERGLOBALEVENT

int TSAPI DM_SplitterGlobalEvent(TWindowData *dat, WPARAM wParam, LPARAM lParam)
{
	RECT rcWin;
	short newMessagePos;
	TWindowData *srcDat = PluginConfig.lastSPlitterPos.pSrcDat;
	TContainerData *srcCnt = PluginConfig.lastSPlitterPos.pSrcContainer;
	bool fCntGlobal = (!dat->pContainer->settings->fPrivate ? true : false);

	if (dat->bIsAutosizingInput)
		return 0;

	GetWindowRect(dat->hwnd, &rcWin);

	LONG newPos;
	if (wParam == 0 && lParam == 0) {
		if ((dat->dwFlagsEx & MWF_SHOW_SPLITTEROVERRIDE) && dat != srcDat)
			return 0;

		if (srcDat->bType == dat->bType)
			newPos = PluginConfig.lastSPlitterPos.pos;
		else if (srcDat->bType == SESSIONTYPE_IM && dat->bType == SESSIONTYPE_CHAT)
			newPos = PluginConfig.lastSPlitterPos.pos + PluginConfig.lastSPlitterPos.off_im;
		else if (srcDat->bType == SESSIONTYPE_CHAT && dat->bType == SESSIONTYPE_IM)
			newPos = PluginConfig.lastSPlitterPos.pos + PluginConfig.lastSPlitterPos.off_im;
		else
			newPos = 0;

		if (dat == srcDat) {
			if (dat->bType == SESSIONTYPE_IM) {
				dat->pContainer->settings->splitterPos = dat->splitterY;
				if (fCntGlobal) {
					SaveSplitter(dat);
					if (PluginConfig.lastSPlitterPos.bSync)
						g_Settings.iSplitterY = dat->splitterY - DPISCALEY_S(23);
				}
			}
			if (dat->bType == SESSIONTYPE_CHAT) {
				SESSION_INFO *si = dat->si;
				if (si) {
					dat->pContainer->settings->splitterPos = si->iSplitterY + DPISCALEY_S(23);
					if (fCntGlobal) {
						g_Settings.iSplitterY = si->iSplitterY;
						if (PluginConfig.lastSPlitterPos.bSync)
							db_set_dw(0, SRMSGMOD_T, "splitsplity", (DWORD)si->iSplitterY + DPISCALEY_S(23));
					}
				}
			}
			return 0;
		}

		if (!fCntGlobal && dat->pContainer != srcCnt)
			return 0;
		if (srcCnt->settings->fPrivate && dat->pContainer != srcCnt)
			return 0;

		if (!PluginConfig.lastSPlitterPos.bSync && dat->bType != srcDat->bType)
			return 0;

		// for inactive sessions, delay the splitter repositioning until they become
		// active (faster, avoid redraw/resize problems for minimized windows)
		if (IsIconic(dat->pContainer->hwnd) || dat->pContainer->hwndActive != dat->hwnd) {
			dat->dwFlagsEx |= MWF_EX_DELAYEDSPLITTER;
			dat->wParam = newPos;
			dat->lParam = PluginConfig.lastSPlitterPos.lParam;
			return 0;
		}
	}
	else newPos = wParam;

	newMessagePos = (short)rcWin.bottom - (short)newPos;

	if (dat->bType == SESSIONTYPE_IM) {
		LoadSplitter(dat);
		AdjustBottomAvatarDisplay(dat);
		DM_RecalcPictureSize(dat);
		SendMessage(dat->hwnd, WM_SIZE, 0, 0);
		DM_ScrollToBottom(dat, 1, 1);
		if (dat != srcDat)
			CSkin::UpdateToolbarBG(dat);
	}
	else {
		SESSION_INFO *si = dat->si;
		if (si) {
			si->iSplitterY = g_Settings.iSplitterY;
			SendMessage(dat->hwnd, WM_SIZE, 0, 0);
		}
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// incoming event handler

void TSAPI DM_EventAdded(TWindowData *dat, WPARAM hContact, LPARAM lParam)
{
	TContainerData *m_pContainer = dat->pContainer;
	HWND hwndDlg = dat->hwnd, hwndContainer = m_pContainer->hwnd, hwndTab = GetParent(dat->hwnd);
	MEVENT hDbEvent = (MEVENT)lParam;

	DBEVENTINFO dbei = { sizeof(dbei) };
	db_event_get(hDbEvent, &dbei);
	if (dat->hDbEventFirst == NULL)
		dat->hDbEventFirst = hDbEvent;

	bool bIsStatusChangeEvent = IsStatusEvent(dbei.eventType);
	bool bDisableNotify = (dbei.eventType == EVENTTYPE_MESSAGE && (dbei.flags & DBEF_READ));

	if (!DbEventIsShown(&dbei))
		return;

	if (dbei.eventType == EVENTTYPE_MESSAGE && !(dbei.flags & (DBEF_SENT))) {
		dat->lastMessage = dbei.timestamp;
		dat->szStatusBar[0] = 0;
		if (dat->bShowTyping) {
			dat->nTypeSecs = 0;
			DM_Typing(dat, true);
			dat->bShowTyping = 0;
		}
		HandleIconFeedback(dat, (HICON)-1);
		if (m_pContainer->hwndStatus)
			PostMessage(hwndDlg, DM_UPDATELASTMESSAGE, 0, 0);
	}

	// set the message log divider to mark new (maybe unseen) messages, if the container has
	// been minimized or in the background.
	if (!(dbei.flags & DBEF_SENT) && !bIsStatusChangeEvent) {
		if (PluginConfig.m_bDividersUsePopupConfig && PluginConfig.m_bUseDividers) {
			if (!MessageWindowOpened(dat->hContact, 0))
				SendMessage(hwndDlg, DM_ADDDIVIDER, 0, 0);
		}
		else if (PluginConfig.m_bUseDividers) {
			if ((GetForegroundWindow() != hwndContainer || GetActiveWindow() != hwndContainer))
				SendMessage(hwndDlg, DM_ADDDIVIDER, 0, 0);
			else {
				if (m_pContainer->hwndActive != hwndDlg)
					SendMessage(hwndDlg, DM_ADDDIVIDER, 0, 0);
			}
		}
		if (!bDisableNotify)
			tabSRMM_ShowPopup(hContact, hDbEvent, dbei.eventType, m_pContainer->fHidden ? 0 : 1, m_pContainer, hwndDlg, dat->cache->getActiveProto());
		if (IsWindowVisible(m_pContainer->hwnd))
			m_pContainer->fHidden = false;
	}
	dat->cache->updateStats(TSessionStats::UPDATE_WITH_LAST_RCV, 0);

	if (hDbEvent != dat->hDbEventFirst) {
		if (!(dat->dwFlagsEx & MWF_SHOW_SCROLLINGDISABLED))
			SendMessage(hwndDlg, DM_APPENDTOLOG, hDbEvent, 0);
		else {
			if (dat->iNextQueuedEvent >= dat->iEventQueueSize) {
				dat->hQueuedEvents = (MEVENT*)mir_realloc(dat->hQueuedEvents, (dat->iEventQueueSize + 10) * sizeof(MEVENT));
				dat->iEventQueueSize += 10;
			}
			dat->hQueuedEvents[dat->iNextQueuedEvent++] = hDbEvent;

			TCHAR szBuf[100];
			mir_sntprintf(szBuf, SIZEOF(szBuf), TranslateT("Auto scrolling is disabled, %d message(s) queued (press F12 to enable it)"),
				dat->iNextQueuedEvent);
			SetDlgItemText(hwndDlg, IDC_LOGFROZENTEXT, szBuf);
			RedrawWindow(GetDlgItem(hwndDlg, IDC_LOGFROZENTEXT), NULL, NULL, RDW_INVALIDATE);
		}
	}
	else SendMessage(hwndDlg, DM_REMAKELOG, 0, 0);

	// handle tab flashing
	if (!bDisableNotify && !bIsStatusChangeEvent)
		if ((TabCtrl_GetCurSel(hwndTab) != dat->iTabID) && !(dbei.flags & DBEF_SENT)) {
			switch (dbei.eventType) {
			case EVENTTYPE_MESSAGE:
				dat->iFlashIcon = PluginConfig.g_IconMsgEvent;
				break;
			case EVENTTYPE_FILE:
				dat->iFlashIcon = PluginConfig.g_IconFileEvent;
				break;
			default:
				dat->iFlashIcon = PluginConfig.g_IconMsgEvent;
				break;
			}
			SetTimer(hwndDlg, TIMERID_FLASHWND, TIMEOUT_FLASHWND, NULL);
			dat->mayFlashTab = TRUE;
		}

	// try to flash the contact list...
	if (!bDisableNotify)
		FlashOnClist(hwndDlg, dat, hDbEvent, &dbei);

	// autoswitch tab if option is set AND container is minimized (otherwise, we never autoswitch)
	// never switch for status changes...
	if (!(dbei.flags & DBEF_SENT) && !bIsStatusChangeEvent) {
		if (PluginConfig.haveAutoSwitch() && m_pContainer->hwndActive != hwndDlg) {
			if ((IsIconic(hwndContainer) && !IsZoomed(hwndContainer)) || (PluginConfig.m_bHideOnClose && !IsWindowVisible(m_pContainer->hwnd))) {
				int iItem = GetTabIndexFromHWND(GetParent(hwndDlg), hwndDlg);
				if (iItem >= 0) {
					TabCtrl_SetCurSel(GetParent(hwndDlg), iItem);
					ShowWindow(m_pContainer->hwndActive, SW_HIDE);
					m_pContainer->hwndActive = hwndDlg;
					SendMessage(hwndContainer, DM_UPDATETITLE, dat->hContact, 0);
					m_pContainer->dwFlags |= CNT_DEFERREDTABSELECT;
				}
			}
		}
	}

	// flash window if it is not focused
	if (!bDisableNotify && !bIsStatusChangeEvent)
		if ((GetActiveWindow() != hwndContainer || GetForegroundWindow() != hwndContainer || dat->pContainer->hwndActive != hwndDlg) && !(dbei.flags & DBEF_SENT)) {
			if (!(m_pContainer->dwFlags & CNT_NOFLASH) && (GetActiveWindow() != hwndContainer || GetForegroundWindow() != hwndContainer))
				FlashContainer(m_pContainer, 1, 0);
			SendMessage(hwndContainer, DM_SETICON, (WPARAM)dat, (LPARAM)LoadSkinnedIcon(SKINICON_EVENT_MESSAGE));
			m_pContainer->dwFlags |= CNT_NEED_UPDATETITLE;
		}

	// play a sound
	if (dbei.eventType == EVENTTYPE_MESSAGE && !(dbei.flags & (DBEF_SENT)))
		PostMessage(hwndDlg, DM_PLAYINCOMINGSOUND, 0, 0);

	if (dat->pWnd)
		dat->pWnd->Invalidate();
}

void TSAPI DM_HandleAutoSizeRequest(TWindowData *dat, REQRESIZE* rr)
{
	if (dat == NULL || rr == NULL || GetForegroundWindow() != dat->pContainer->hwnd)
		return;

	if (!dat->bIsAutosizingInput || dat->iInputAreaHeight == -1)
		return;

	LONG heightLimit = M.GetDword("autoSplitMinLimit", 0);
	LONG iNewHeight = rr->rc.bottom - rr->rc.top;

	if (CSkin::m_skinEnabled && !SkinItems[ID_EXTBKINPUTAREA].IGNORED)
		iNewHeight += (SkinItems[ID_EXTBKINPUTAREA].MARGIN_TOP + SkinItems[ID_EXTBKINPUTAREA].MARGIN_BOTTOM - 2);

	if (heightLimit && iNewHeight < heightLimit)
		iNewHeight = heightLimit;

	if (iNewHeight == dat->iInputAreaHeight)
		return;

	RECT rc;
	GetClientRect(dat->hwnd, &rc);
	LONG cy = rc.bottom - rc.top;
	LONG panelHeight = (dat->Panel->isActive() ? dat->Panel->getHeight() : 0);

	if (iNewHeight > (cy - panelHeight) / 2)
		iNewHeight = (cy - panelHeight) / 2;

	if (dat->bType == SESSIONTYPE_IM) {
		dat->dynaSplitter = rc.bottom - (rc.bottom - iNewHeight + DPISCALEY_S(2));
		if (dat->pContainer->dwFlags & CNT_BOTTOMTOOLBAR)
			dat->dynaSplitter += DPISCALEY_S(22);
		dat->splitterY = dat->dynaSplitter + DPISCALEY_S(34);
		DM_RecalcPictureSize(dat);
	}
	else if (dat->si) {
		dat->si->iSplitterY = (rc.bottom - (rc.bottom - iNewHeight + DPISCALEY_S(3))) + DPISCALEY_S(34);
		if (!(dat->pContainer->dwFlags & CNT_BOTTOMTOOLBAR))
			dat->si->iSplitterY -= DPISCALEY_S(22);
		SendMessage(dat->hwnd, WM_SIZE, 0, 0);
	}
	dat->iInputAreaHeight = iNewHeight;
	CSkin::UpdateToolbarBG(dat);
	DM_ScrollToBottom(dat, 1, 0);
}

void TSAPI DM_UpdateTitle(TWindowData *dat, WPARAM, LPARAM lParam)
{
	TCHAR newtitle[128], newcontactname[128];
	DWORD dwOldIdle = dat->idle;
	const char *szActProto = 0;

	HWND hwndDlg = dat->hwnd;
	HWND hwndTab = GetParent(hwndDlg);
	HWND hwndContainer = dat->pContainer->hwnd;
	TContainerData*	m_pContainer = dat->pContainer;

	newcontactname[0] = 0;
	dat->szStatus[0] = 0;

	if (dat->iTabID == -1)
		return;

	TCITEM item = { 0 };

	if (dat->hContact) {
		const TCHAR *szNick = dat->cache->getNick();

		if (dat->szProto) {
			szActProto = dat->cache->getProto();

			bool bHasName = (dat->cache->getUIN()[0] != 0);
			dat->idle = dat->cache->getIdleTS();
			dat->dwFlagsEx = dat->idle ? dat->dwFlagsEx | MWF_SHOW_ISIDLE : dat->dwFlagsEx & ~MWF_SHOW_ISIDLE;

			dat->wStatus = dat->cache->getStatus();
			_tcsncpy_s(dat->szStatus, pcli->pfnGetStatusModeDescription(dat->szProto == NULL ? ID_STATUS_OFFLINE : dat->wStatus, 0), _TRUNCATE);

			if (lParam != 0) {
				if (PluginConfig.m_bCutContactNameOnTabs)
					CutContactName(szNick, newcontactname, SIZEOF(newcontactname));
				else
					_tcsncpy_s(newcontactname, szNick, _TRUNCATE);

				Utils::DoubleAmpersands(newcontactname);

				if (mir_tstrlen(newcontactname) != 0) {
					if (PluginConfig.m_bStatusOnTabs)
						mir_sntprintf(newtitle, SIZEOF(newtitle), _T("%s (%s)"), newcontactname, dat->szStatus);
					else
						_tcsncpy_s(newtitle, newcontactname, _TRUNCATE);
				}
				else _tcsncpy_s(newtitle, _T("Forward"), _TRUNCATE);

				item.mask |= TCIF_TEXT;
			}
			SendMessage(hwndDlg, DM_UPDATEWINICON, 0, 0);

			TCHAR fulluin[256];
			if (dat->bIsMeta)
				mir_sntprintf(fulluin, SIZEOF(fulluin),
				TranslateT("UID: %s (SHIFT click -> copy to clipboard)\nClick for user's details\nRight click for metacontact control\nClick dropdown to add or remove user from your favorites."),
				bHasName ? dat->cache->getUIN() : TranslateT("No UID"));
			else
				mir_sntprintf(fulluin, SIZEOF(fulluin),
				TranslateT("UID: %s (SHIFT click -> copy to clipboard)\nClick for user's details\nClick dropdown to change this contact's favorite status."),
				bHasName ? dat->cache->getUIN() : TranslateT("No UID"));

			SendDlgItemMessage(hwndDlg, IDC_NAME, BUTTONADDTOOLTIP, (WPARAM)fulluin, BATF_TCHAR);
		}
	}
	else _tcsncpy_s(newtitle, _T("Message Session"), _TRUNCATE);

	if (dat->idle != dwOldIdle || lParam != 0) {
		if (item.mask & TCIF_TEXT) {
			item.pszText = newtitle;
			_tcsncpy(dat->newtitle, newtitle, SIZEOF(dat->newtitle));
			dat->newtitle[127] = 0;
			if (dat->pWnd)
				dat->pWnd->updateTitle(dat->cache->getNick());
		}
		if (dat->iTabID >= 0) {
			TabCtrl_SetItem(hwndTab, dat->iTabID, &item);
			if (m_pContainer->dwFlags & CNT_SIDEBAR)
				m_pContainer->SideBar->updateSession(dat);
		}
		if (m_pContainer->hwndActive == hwndDlg && lParam)
			SendMessage(hwndContainer, DM_UPDATETITLE, dat->hContact, 0);

		UpdateTrayMenuState(dat, TRUE);
		if (dat->cache->isFavorite())
			AddContactToFavorites(dat->hContact, dat->cache->getNick(), szActProto, dat->szStatus, dat->wStatus,
			LoadSkinnedProtoIcon(dat->cache->getProto(), dat->cache->getStatus()), 0, PluginConfig.g_hMenuFavorites);

		if (dat->cache->isRecent())
			AddContactToFavorites(dat->hContact, dat->cache->getNick(), szActProto, dat->szStatus, dat->wStatus,
			LoadSkinnedProtoIcon(dat->cache->getProto(), dat->cache->getStatus()), 0, PluginConfig.g_hMenuRecent);

		dat->Panel->Invalidate();
		if (dat->pWnd)
			dat->pWnd->Invalidate();
	}

	// care about MetaContacts and update the statusbar icon with the currently "most online" contact...
	if (dat->bIsMeta) {
		PostMessage(hwndDlg, DM_UPDATEMETACONTACTINFO, 0, 0);
		PostMessage(hwndDlg, DM_OWNNICKCHANGED, 0, 0);
		if (m_pContainer->dwFlags & CNT_UINSTATUSBAR)
			DM_UpdateLastMessage(dat);
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

void DrawStatusIcons(TWindowData *dat, HDC hDC, const RECT &rc, int gap)
{
	HICON hIcon = NULL;
	LONG 	cx_icon = PluginConfig.m_smcxicon;
	LONG	cy_icon = PluginConfig.m_smcyicon;
	int 	x = rc.left;
	LONG	y = (rc.top + rc.bottom - cx_icon) >> 1;

	SetBkMode(hDC, TRANSPARENT);

	int nIcon = 0;
	while (StatusIconData *si = Srmm_GetNthIcon(dat->hContact, nIcon++)) {
		if (!mir_strcmp(si->szModule, MSG_ICON_MODULE)) {
			if (si->dwId == MSG_ICON_SOUND) {
				DrawIconEx(hDC, x, y, PluginConfig.g_buttonBarIcons[ICON_DEFAULT_SOUNDS],
					cx_icon, cy_icon, 0, NULL, DI_NORMAL);

				DrawIconEx(hDC, x, y, dat->pContainer->dwFlags & CNT_NOSOUND ?
					PluginConfig.g_iconOverlayDisabled : PluginConfig.g_iconOverlayEnabled,
					cx_icon, cy_icon, 0, NULL, DI_NORMAL);
			}
			else if (si->dwId == MSG_ICON_UTN) {
				if (dat->bType == SESSIONTYPE_IM || dat->si->iType == GCW_PRIVMESS) {
					DrawIconEx(hDC, x, y, PluginConfig.g_buttonBarIcons[ICON_DEFAULT_TYPING], cx_icon, cy_icon, 0, NULL, DI_NORMAL);

					DrawIconEx(hDC, x, y, db_get_b(dat->hContact, SRMSGMOD, SRMSGSET_TYPING, M.GetByte(SRMSGMOD, SRMSGSET_TYPINGNEW, SRMSGDEFSET_TYPINGNEW)) ?
						PluginConfig.g_iconOverlayEnabled : PluginConfig.g_iconOverlayDisabled, cx_icon, cy_icon, 0, NULL, DI_NORMAL);
				}
				else CSkin::DrawDimmedIcon(hDC, x, y, cx_icon, cy_icon, PluginConfig.g_buttonBarIcons[ICON_DEFAULT_TYPING], 50);
			}
			else if (si->dwId == MSG_ICON_SESSION) {
				DrawIconEx(hDC, x, y, PluginConfig.g_sideBarIcons[0], cx_icon, cy_icon, 0, NULL, DI_NORMAL);
			}
		}
		else {
			if ((si->flags & MBF_DISABLED) && si->hIconDisabled)
				hIcon = si->hIconDisabled;
			else
				hIcon = si->hIcon;

			if ((si->flags & MBF_DISABLED) && si->hIconDisabled == NULL)
				CSkin::DrawDimmedIcon(hDC, x, y, cx_icon, cy_icon, hIcon, 50);
			else
				DrawIconEx(hDC, x, y, hIcon, 16, 16, 0, NULL, DI_NORMAL);
		}

		x += cx_icon + gap;
	}
}

void CheckStatusIconClick(TWindowData *dat, POINT pt, const RECT &rc, int gap, int code)
{
	if (dat && (code == NM_CLICK || code == NM_RCLICK)) {
		POINT	ptScreen;
		GetCursorPos(&ptScreen);
		if (!PtInRect(&rcLastStatusBarClick, ptScreen))
			return;
	}

	UINT iconNum = (pt.x - (rc.left + 0)) / (PluginConfig.m_smcxicon + gap);
	if (dat == NULL)
		return;
	StatusIconData *si = Srmm_GetNthIcon(dat->hContact, iconNum);
	if (si == NULL)
		return;

	if (!mir_strcmp(si->szModule, MSG_ICON_MODULE)) {
		if (si->dwId == MSG_ICON_SOUND && code != NM_RCLICK) {
			if (GetKeyState(VK_SHIFT) & 0x8000) {
				for (TContainerData *p = pFirstContainer; p; p = p->pNext) {
					p->dwFlags = ((dat->pContainer->dwFlags & CNT_NOSOUND) ? p->dwFlags | CNT_NOSOUND : p->dwFlags & ~CNT_NOSOUND);
					InvalidateRect(dat->pContainer->hwndStatus, NULL, TRUE);
				}
			}
			else {
				dat->pContainer->dwFlags ^= CNT_NOSOUND;
				InvalidateRect(dat->pContainer->hwndStatus, NULL, TRUE);
			}
		}
		else if (si->dwId == MSG_ICON_UTN && code != NM_RCLICK && (dat->bType == SESSIONTYPE_IM || dat->si->iType == GCW_PRIVMESS)) {
			SendMessage(dat->pContainer->hwndActive, WM_COMMAND, IDC_SELFTYPING, 0);
			InvalidateRect(dat->pContainer->hwndStatus, NULL, TRUE);
		}
		else if (si->dwId == MSG_ICON_SESSION) {
			if (code == NM_CLICK)
				PostMessage(PluginConfig.g_hwndHotkeyHandler, DM_TRAYICONNOTIFY, 101, WM_LBUTTONUP);
			else if (code == NM_RCLICK)
				PostMessage(PluginConfig.g_hwndHotkeyHandler, DM_TRAYICONNOTIFY, 101, WM_RBUTTONUP);
		}
	}
	else {
		StatusIconClickData sicd = { sizeof(sicd) };
		GetCursorPos(&sicd.clickLocation);
		sicd.dwId = si->dwId;
		sicd.szModule = si->szModule;
		sicd.flags = (code == NM_RCLICK ? MBCF_RIGHTBUTTON : 0);
		NotifyEventHooks(hHookIconPressedEvt, dat->hContact, (LPARAM)&sicd);
		InvalidateRect(dat->pContainer->hwndStatus, NULL, TRUE);
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
