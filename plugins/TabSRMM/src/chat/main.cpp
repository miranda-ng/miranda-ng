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
 * This code is based on and still contains large parts of the the
 * original chat module for Miranda NG, written and copyrighted
 * by Joergen Persson in 2005.
 *
 * (C) 2005-2009 by silvercircle _at_ gmail _dot_ com and contributors
 *
 * chat module exports and functions to load/unload the plugin.
 *
 */

#include "..\commonheaders.h"

HANDLE g_hWindowList;
HMENU  g_hMenu = NULL;

CHAT_MANAGER *pci;
TMUCSettings g_Settings;

static void OnAddLog(SESSION_INFO *si, int isOk)
{
	if (isOk && si->hWnd)
		SendMessage(si->hWnd, GC_ADDLOG, 0, 0);
	else if (si->hWnd)
		SendMessage(si->hWnd, GC_REDRAWLOG2, 0, 0);
}

static void OnCreateSession(SESSION_INFO *si, MODULEINFO *mi)
{
	si->Highlight = g_Settings.Highlight;
	if (mi) {
		mi->idleTimeStamp = time(0);
		pci->SM_BroadcastMessage(mi->pszModule, GC_UPDATESTATUSBAR, 0, 1, TRUE);
	}
}

static void OnSessionDblClick(SESSION_INFO *si)
{
	PostMessage(si->hWnd, GC_CLOSEWINDOW, 0, 0);
}

static void OnSessionRemove(SESSION_INFO *si)
{
	if (si->hWnd)
		SendMessage(si->hWnd, GC_EVENT_CONTROL + WM_USER + 500, SESSION_TERMINATE, 0);
}

static void OnSessionRename(SESSION_INFO *si)
{
	if (si->hWnd)
		SendMessage(si->hWnd, GC_UPDATETITLE, 0, 0);
}

static void OnSessionReplace(SESSION_INFO *si)
{
	if (si->hContact)
		Chat_SetFilters(si);
	if (si->hWnd)
		RedrawWindow(GetDlgItem(si->hWnd, IDC_LIST), NULL, NULL, RDW_INVALIDATE);
}

static void OnEventBroadcast(SESSION_INFO *si, GCEVENT *gce)
{
	if (pci->SM_AddEvent(si->ptszID, si->pszModule, gce, FALSE) && si->hWnd && si->bInitDone)
		SendMessage(si->hWnd, GC_ADDLOG, 0, 0);
	else if (si->hWnd && si->bInitDone)
		SendMessage(si->hWnd, GC_REDRAWLOG2, 0, 0);
}

static void OnSetTopic(SESSION_INFO *si)
{
	if (si->hWnd)
		SendMessage(si->hWnd, DM_INVALIDATEPANEL, 0, 0);
}

static void OnSetStatusBar(SESSION_INFO *si)
{
	if (si->hWnd)
		SendMessage(si->hWnd, GC_UPDATESTATUSBAR, 0, 0);
}

static void OnNewUser(SESSION_INFO *si, USERINFO*)
{
	if (si->hWnd) {
		SendMessage(si->hWnd, GC_UPDATENICKLIST, 0, 0);
		if (si->dat)
			GetMyNick(si->dat);
	}
}

static void OnChangeNick(SESSION_INFO *si)
{
	if (si->hWnd) {
		if (si->dat)
			GetMyNick(si->dat);
		SendMessage(si->hWnd, GC_UPDATESTATUSBAR, 0, 0);
	}
}

static void OnSetStatus(SESSION_INFO *si, int wStatus)
{
	PostMessage(si->hWnd, GC_FIXTABICONS, 0, 0);
}

static void OnCreateModule(MODULEINFO *mi)
{
	mi->idleTimeStamp = time(0);
}

static void OnLoadSettings()
{
	LOGFONT lf;
	char szBuf[40];

	g_Settings.iEventLimitThreshold = db_get_w(NULL, "Chat", "LogLimitThreshold", 20);
	g_Settings.dwIconFlags = M.GetDword("Chat", "IconFlags", 0x0000);
	g_Settings.bOpenInDefault = M.GetBool("Chat", "DefaultContainer", true);
	g_Settings.bFlashWindowHightlight = M.GetBool("Chat", "FlashWindowHighlight", false);
	g_Settings.UserListColors[CHAT_STATUS_NORMAL] = M.GetDword(CHAT_FONTMODULE, "Font18Col", RGB(0, 0, 0));
	g_Settings.UserListColors[CHAT_STATUS_AWAY] = M.GetDword(CHAT_FONTMODULE, "Font19Col", RGB(170, 170, 170));
	g_Settings.UserListColors[CHAT_STATUS_OFFLINE] = M.GetDword(CHAT_FONTMODULE, "Font5Col", RGB(160, 90, 90));
	g_Settings.bBBCodeInPopups = M.GetByte("Chat", "BBCodeInPopups", 0) != 0;
	g_Settings.bClassicIndicators = M.GetBool("Chat", "ClassicIndicators", false);
	g_Settings.bLogClassicIndicators = M.GetBool("Chat", "LogClassicIndicators", false);
	g_Settings.bAlternativeSorting = M.GetBool("Chat", "AlternativeSorting", true);
	g_Settings.bAnnoyingHighlight = M.GetBool("Chat", "AnnoyingHighlight", false);
	g_Settings.bCreateWindowOnHighlight = M.GetBool("Chat", "CreateWindowOnHighlight", true);

	g_Settings.bLogSymbols = M.GetBool("Chat", "LogSymbols", true);
	g_Settings.bClickableNicks = M.GetBool("Chat", "ClickableNicks", true);
	g_Settings.bColorizeNicks = M.GetBool("Chat", "ColorizeNicks", true);
	g_Settings.bColorizeNicksInLog = M.GetBool("Chat", "ColorizeNicksInLog", true);
	g_Settings.bScaleIcons = M.GetBool("Chat", "ScaleIcons", true);
	g_Settings.bUseDividers = M.GetBool("Chat", "UseDividers", true);
	g_Settings.bDividersUsePopupConfig = M.GetBool("Chat", "DividersUsePopupConfig", true);

	g_Settings.bDoubleClick4Privat = M.GetBool("Chat", "DoubleClick4Privat", false);
	g_Settings.bShowContactStatus = M.GetBool("Chat", "ShowContactStatus", true);
	g_Settings.bContactStatusFirst = M.GetBool("Chat", "ContactStatusFirst", false);

	// nicklist
	if (g_Settings.UserListFonts[0]) {
		DeleteObject(g_Settings.UserListFonts[CHAT_STATUS_NORMAL]);
		DeleteObject(g_Settings.UserListFonts[CHAT_STATUS_AWAY]);
		DeleteObject(g_Settings.UserListFonts[CHAT_STATUS_OFFLINE]);
	}

	LoadMsgDlgFont(FONTSECTION_CHAT, 18, &lf, NULL, CHAT_FONTMODULE);
	g_Settings.UserListFonts[CHAT_STATUS_NORMAL] = CreateFontIndirect(&lf);

	LoadMsgDlgFont(FONTSECTION_CHAT, 19, &lf, NULL, CHAT_FONTMODULE);
	g_Settings.UserListFonts[CHAT_STATUS_AWAY] = CreateFontIndirect(&lf);

	LoadMsgDlgFont(FONTSECTION_CHAT, 5, &lf, NULL, CHAT_FONTMODULE);
	g_Settings.UserListFonts[CHAT_STATUS_OFFLINE] = CreateFontIndirect(&lf);

	int ih = GetTextPixelSize(_T("AQGglo"), g_Settings.UserListFonts[CHAT_STATUS_NORMAL], false);
	int ih2 = GetTextPixelSize(_T("AQGglo"), g_Settings.UserListFonts[CHAT_STATUS_AWAY], false);
	g_Settings.iNickListFontHeight = max(M.GetByte("Chat", "NicklistRowDist", 12), (ih > ih2 ? ih : ih2));

	for (int i = 0; i < 7; i++) {
		mir_snprintf(szBuf, 20, "NickColor%d", i);
		g_Settings.nickColors[i] = M.GetDword("Chat", szBuf, g_Settings.UserListColors[0]);
	}
	g_Settings.nickColors[5] = M.GetDword("Chat", "NickColor5", GetSysColor(COLOR_HIGHLIGHT));
	g_Settings.nickColors[6] = M.GetDword("Chat", "NickColor6", GetSysColor(COLOR_HIGHLIGHTTEXT));
	
	if (g_Settings.SelectionBGBrush)
		DeleteObject(g_Settings.SelectionBGBrush);
	g_Settings.SelectionBGBrush = CreateSolidBrush(g_Settings.nickColors[5]);

	Log_SetStyles();
}

/////////////////////////////////////////////////////////////////////////////////////////

void Chat_ModulesLoaded()
{
	ZeroMemory(pci->hIcons, sizeof(HICON)* (ICON_STATUS5 - ICON_ACTION));
	pci->hIcons[ICON_ACTION] = LoadIconEx(IDI_ACTION, "log_action", 16, 16);
	pci->hIcons[ICON_ADDSTATUS] = LoadIconEx(IDI_ADDSTATUS, "log_addstatus", 16, 16);
	pci->hIcons[ICON_HIGHLIGHT] = LoadIconEx(IDI_HIGHLIGHT, "log_highlight", 16, 16);
	pci->hIcons[ICON_INFO] = LoadIconEx(IDI_INFO, "log_info", 16, 16);
	pci->hIcons[ICON_JOIN] = LoadIconEx(IDI_JOIN, "log_join", 16, 16);
	pci->hIcons[ICON_KICK] = LoadIconEx(IDI_KICK, "log_kick", 16, 16);
	pci->hIcons[ICON_MESSAGE] = LoadIconEx(IDI_MESSAGE, "log_message_in", 16, 16);
	pci->hIcons[ICON_MESSAGEOUT] = LoadIconEx(IDI_MESSAGEOUT, "log_message_out", 16, 16);
	pci->hIcons[ICON_NICK] = LoadIconEx(IDI_NICK, "log_nick", 16, 16);
	pci->hIcons[ICON_NOTICE] = LoadIconEx(IDI_NOTICE, "log_notice", 16, 16);
	pci->hIcons[ICON_PART] = LoadIconEx(IDI_PART, "log_part", 16, 16);
	pci->hIcons[ICON_QUIT] = LoadIconEx(IDI_QUIT, "log_quit", 16, 16);
	pci->hIcons[ICON_REMSTATUS] = LoadIconEx(IDI_REMSTATUS, "log_removestatus", 16, 16);
	pci->hIcons[ICON_TOPIC] = LoadIconEx(IDI_TOPIC, "log_topic", 16, 16);
	pci->hIcons[ICON_STATUS1] = LoadIconEx(IDI_STATUS1, "status1", 16, 16);
	pci->hIcons[ICON_STATUS2] = LoadIconEx(IDI_STATUS2, "status2", 16, 16);
	pci->hIcons[ICON_STATUS3] = LoadIconEx(IDI_STATUS3, "status3", 16, 16);
	pci->hIcons[ICON_STATUS4] = LoadIconEx(IDI_STATUS4, "status4", 16, 16);
	pci->hIcons[ICON_STATUS0] = LoadIconEx(IDI_STATUS0, "status0", 16, 16);
	pci->hIcons[ICON_STATUS5] = LoadIconEx(IDI_STATUS5, "status5", 16, 16);

	g_Settings.hIconOverlay = LoadIconEx(IDI_OVERLAY, "overlay", 16, 16);
}

/////////////////////////////////////////////////////////////////////////////////////////
// load the group chat module

static CHAT_MANAGER saveCI;

int Chat_Load()
{
	mir_getCI(&g_Settings);
	saveCI = *pci;
	pci->cbModuleInfo = sizeof(MODULEINFO);
	pci->cbSession = sizeof(SESSION_INFO);
	pci->OnCreateModule = OnCreateModule;
	pci->OnNewUser = OnNewUser;

	pci->OnSetStatus = OnSetStatus;
	pci->OnSetTopic = OnSetTopic;

	pci->OnAddLog = OnAddLog;

	pci->OnCreateSession = OnCreateSession;
	pci->OnSessionRemove = OnSessionRemove;
	pci->OnSessionRename = OnSessionRename;
	pci->OnSessionReplace = OnSessionReplace;
	pci->OnSessionDblClick = OnSessionDblClick;

	pci->OnEventBroadcast = OnEventBroadcast;
	pci->OnSetStatusBar = OnSetStatusBar;
	pci->OnChangeNick = OnChangeNick;
	pci->ShowRoom = ShowRoom;
	pci->OnLoadSettings = OnLoadSettings;

	// this operation is unsafe, that's why we restore the old pci state on exit
	pci->DoSoundsFlashPopupTrayStuff = DoSoundsFlashPopupTrayStuff;

	g_hMenu = LoadMenu(g_hInst, MAKEINTRESOURCE(IDR_MENU));

	OnLoadSettings();
	OptionsInit();
	return 0;
}

/*
 * unload the module. final cleanup
 */

int Chat_Unload(void)
{
	*pci = saveCI;
	DestroyMenu(g_hMenu);
	OptionsUnInit();
	return 0;
}
