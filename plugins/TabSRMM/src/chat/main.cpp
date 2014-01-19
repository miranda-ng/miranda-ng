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
	Chat_SetFilters(si);
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

	replaceStrT(g_Settings.pszLogDir, M.getChatLogPath());

	g_Settings.iSplitterY = db_get_w(NULL, "Chat", "splitY", 50);
	if (g_Settings.iSplitterY <= 20)
		g_Settings.iSplitterY = 50;

	// nicklist
	if (g_Settings.UserListFonts[0]) {
		DeleteObject(g_Settings.UserListFonts[CHAT_STATUS_NORMAL]);
		DeleteObject(g_Settings.UserListFonts[CHAT_STATUS_AWAY]);
		DeleteObject(g_Settings.UserListFonts[CHAT_STATUS_OFFLINE]);
	}

	pci->LoadMsgDlgFont(18, &lf, NULL);
	g_Settings.UserListFonts[CHAT_STATUS_NORMAL] = CreateFontIndirect(&lf);

	pci->LoadMsgDlgFont(19, &lf, NULL);
	g_Settings.UserListFonts[CHAT_STATUS_AWAY] = CreateFontIndirect(&lf);

	pci->LoadMsgDlgFont(5, &lf, NULL);
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
	g_Settings.hIconOverlay = LoadIconEx(IDI_OVERLAY, "overlay", 16, 16);
}

/////////////////////////////////////////////////////////////////////////////////////////
// load the group chat module

CHAT_MANAGER saveCI;

static int CopyChatSetting(const char *szSetting, LPARAM param)
{
	LIST<char> *szSettings = (LIST<char>*)param;
	szSettings->insert(mir_strdup(szSetting));
	return 0;
}

static void CheckUpdate()
{
	// already converted?
	if (db_get_b(NULL, "Compatibility", "TabChatFonts", false))
		return;

	LIST<char> szSettings(120);

	DBCONTACTENUMSETTINGS dbces = { 0 };
	dbces.szModule = CHAT_OLDFONTMODULE;
	dbces.pfnEnumProc = CopyChatSetting;
	dbces.lParam = (LPARAM)&szSettings;
	CallService(MS_DB_CONTACT_ENUMSETTINGS, 0, (LPARAM)&dbces);

	DBVARIANT dbv;
	for (int i = szSettings.getCount() - 1; i >= 0; i--) {
		char *p = szSettings[i];
		db_get(NULL, CHAT_OLDFONTMODULE, p, &dbv);
		db_set(NULL, CHAT_FONTMODULE, p, &dbv);
		db_free(&dbv);
		mir_free(p);
	}

	CallService(MS_DB_MODULE_DELETE, 0, (LPARAM)CHAT_OLDFONTMODULE);
	db_set_b(NULL, "Compatibility", "TabChatFonts", true);
}

int Chat_Load()
{
	CheckUpdate();

	CHAT_MANAGER_INITDATA data = { &g_Settings, sizeof(MODULEINFO), sizeof(SESSION_INFO), LPGENT("Message Sessions")_T("/")LPGENT("Group chats"), FONTMODE_ALTER };
	mir_getCI(&data);
	saveCI = *pci;
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
	pci->IsHighlighted = IsHighlighted;
	pci->LogToFile = LogToFile;
	pci->DoPopup = DoPopup;
	pci->ShowPopup = ShowPopup;
	pci->Log_CreateRtfHeader = Log_CreateRtfHeader;
	pci->UM_CompareItem = UM_CompareItem;
	pci->ReloadSettings();

	g_hMenu = LoadMenu(g_hInst, MAKEINTRESOURCE(IDR_MENU));

	g_Settings.Highlight = new CMUCHighlight();
	return 0;
}

/*
 * unload the module. final cleanup
 */

int Chat_Unload(void)
{
	DestroyMenu(g_hMenu);
	return 0;
}
