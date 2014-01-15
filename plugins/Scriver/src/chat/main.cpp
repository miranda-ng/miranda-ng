/*
Chat module plugin for Miranda IM

Copyright (C) 2003 Jörgen Persson
Copyright 2003-2009 Miranda ICQ/IM project,

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

#include "../commonheaders.h"

void RegisterChatFonts( void );

//globals
HMENU       g_hMenu = NULL;
HGENMENU    hJoinMenuItem, hLeaveMenuItem;

FONTINFO    aFonts[OPTIONS_FONTCOUNT];
HBRUSH      hListBkgBrush = NULL;
HBRUSH      hListSelectedBkgBrush = NULL;

TCHAR*      pszActiveWndID = 0;
char*       pszActiveWndModule = 0;

struct GlobalLogSettings_t g_Settings;

int Chat_Load()
{
	InitializeCriticalSection(&cs);
	g_hMenu = LoadMenu(g_hInst, MAKEINTRESOURCE(IDR_MENU));
	TranslateMenu(g_hMenu);
	HookEvents();
	CreateServiceFunctions();
	CreateHookableEvents();
	return 0;
}

int Chat_Unload(void)
{
	db_set_w(NULL, "Chat", "SplitterX", (WORD)g_Settings.iSplitterX);

	CList_SetAllOffline(TRUE, NULL);

	mir_free( pszActiveWndID );
	mir_free( pszActiveWndModule );

	DestroyHookableEvents();

	DestroyMenu(g_hMenu);
	FreeIcons();
	OptionsUnInit();
	DeleteCriticalSection(&cs);
	return 0;
}

int Chat_ModulesLoaded(WPARAM wParam,LPARAM lParam)
{
	char* mods[3] = { "Chat", "ChatFonts" };
	CallService("DBEditorpp/RegisterModule", (WPARAM)mods, 2);
	RegisterChatFonts();
	OptionsInit();
	LoadIcons();
	{
		CLISTMENUITEM mi = { sizeof(mi) };
		mi.cbSize = sizeof(mi);
		mi.position = -2000090001;
		mi.flags = CMIF_DEFAULT;
		mi.icolibItem = LoadSkinnedIconHandle( SKINICON_CHAT_JOIN );
		mi.pszName = LPGEN("&Join");
		mi.pszService = "GChat/JoinChat";
		hJoinMenuItem = Menu_AddContactMenuItem(&mi);

		mi.position = -2000090000;
		mi.flags = CMIF_NOTOFFLINE;
		mi.icolibItem = LoadSkinnedIconHandle( SKINICON_CHAT_LEAVE );
		mi.pszName = LPGEN("&Leave");
		mi.pszService = "GChat/LeaveChat";
		hLeaveMenuItem = Menu_AddContactMenuItem(&mi);
	}
	CList_SetAllOffline(TRUE, NULL);
 	return 0;
}

void LoadIcons(void)
{
	LoadMsgLogBitmaps();
}

void FreeIcons(void)
{
	FreeMsgLogBitmaps();
}
