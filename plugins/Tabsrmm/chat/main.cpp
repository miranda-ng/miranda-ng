/*
 * astyle --force-indent=tab=4 --brackets=linux --indent-switches
 *		  --pad=oper --one-line=keep-blocks  --unpad=paren
 *
 * Miranda IM: the free IM client for Microsoft* Windows*
 *
 * Copyright 2000-2009 Miranda ICQ/IM project,
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
 * original chat module for Miranda IM, written and copyrighted
 * by Joergen Persson in 2005.
 *
 * (C) 2005-2009 by silvercircle _at_ gmail _dot_ com and contributors
 *
 * $Id: main.cpp 13132 2010-11-17 03:47:44Z silvercircle $
 *
 * chat module exports and functions to load/unload the plugin.
 *
 */

#include "../src/commonheaders.h"

HANDLE		g_hWindowList;
HMENU		g_hMenu = NULL;

FONTINFO	aFonts[OPTIONS_FONTCOUNT];
HICON		hIcons[30];
HBRUSH		hListBkgBrush = NULL;

TMUCSettings g_Settings;

TCHAR		*pszActiveWndID = 0;
char		*pszActiveWndModule = 0;

/*
 * load the group chat module
 */

int Chat_Load(PLUGINLINK *link)
{
	if(M->GetByte("forceDisableMUC", 0)) {
		PluginConfig.m_chat_enabled = false;
		return(0);
	}
	g_hMenu = LoadMenu(g_hInst, MAKEINTRESOURCE(IDR_MENU));
	if(CreateServiceFunctions()) {
		HookEvents();
		CreateHookableEvents();
		OptionsInit();
	}
	return 0;
}

/*
 * unload the module. final cleanup
 */

int Chat_Unload(void)
{
	if (!PluginConfig.m_chat_enabled)
		return 0;

	DBWriteContactSettingWord(NULL, "Chat", "SplitterX", (WORD)g_Settings.iSplitterX);
	DBWriteContactSettingWord(NULL, "Chat", "splitY", (WORD)g_Settings.iSplitterY);

	CList_SetAllOffline(TRUE, NULL);

	mir_free(pszActiveWndID);
	mir_free(pszActiveWndModule);

	DestroyMenu(g_hMenu);
	DestroyServiceFunctions();
	DestroyHookableEvents();
	FreeIcons();
	OptionsUnInit();
	UnhookEvents();
	return 0;
}

void LoadLogIcons(void)
{
	ZeroMemory(hIcons, sizeof(HICON) * (ICON_STATUS5 - ICON_ACTION));
	hIcons[ICON_ACTION] = LoadIconEx(IDI_ACTION, "log_action", 16, 16);
	hIcons[ICON_ADDSTATUS] = LoadIconEx(IDI_ADDSTATUS, "log_addstatus", 16, 16);
	hIcons[ICON_HIGHLIGHT] = LoadIconEx(IDI_HIGHLIGHT, "log_highlight", 16, 16);
	hIcons[ICON_INFO] = LoadIconEx(IDI_INFO, "log_info", 16, 16);
	hIcons[ICON_JOIN] = LoadIconEx(IDI_JOIN, "log_join", 16, 16);
	hIcons[ICON_KICK] = LoadIconEx(IDI_KICK, "log_kick", 16, 16);
	hIcons[ICON_MESSAGE] = LoadIconEx(IDI_MESSAGE, "log_message_in", 16, 16);
	hIcons[ICON_MESSAGEOUT] = LoadIconEx(IDI_MESSAGEOUT, "log_message_out", 16, 16);
	hIcons[ICON_NICK] = LoadIconEx(IDI_NICK, "log_nick", 16, 16);
	hIcons[ICON_NOTICE] = LoadIconEx(IDI_NOTICE, "log_notice", 16, 16);
	hIcons[ICON_PART] = LoadIconEx(IDI_PART, "log_part", 16, 16);
	hIcons[ICON_QUIT] = LoadIconEx(IDI_QUIT, "log_quit", 16, 16);
	hIcons[ICON_REMSTATUS] = LoadIconEx(IDI_REMSTATUS, "log_removestatus", 16, 16);
	hIcons[ICON_TOPIC] = LoadIconEx(IDI_TOPIC, "log_topic", 16, 16);
	hIcons[ICON_STATUS1] = LoadIconEx(IDI_STATUS1, "status1", 16, 16);
	hIcons[ICON_STATUS2] = LoadIconEx(IDI_STATUS2, "status2", 16, 16);
	hIcons[ICON_STATUS3] = LoadIconEx(IDI_STATUS3, "status3", 16, 16);
	hIcons[ICON_STATUS4] = LoadIconEx(IDI_STATUS4, "status4", 16, 16);
	hIcons[ICON_STATUS0] = LoadIconEx(IDI_STATUS0, "status0", 16, 16);
	hIcons[ICON_STATUS5] = LoadIconEx(IDI_STATUS5, "status5", 16, 16);

	return;
}

void LoadIcons(void)
{
	int i;

	for (i = 0; i < 20; i++)
		hIcons[i] = NULL;

	LoadLogIcons();
	g_Settings.hIconOverlay = LoadIconEx(IDI_OVERLAY, "overlay", 16, 16);
	LoadMsgLogBitmaps();
	return ;
}

void FreeIcons(void)
{
	FreeMsgLogBitmaps();
	return;
}
