/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2008 Miranda ICQ/IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

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

//include
#include "hdr/modern_commonheaders.h"
#include "hdr/modern_commonprototypes.h"
#include "version.h"

#include "hdr/modern_clui.h"
#include "hdr/modern_clcpaint.h"

//definitions

#define CHECKRES(sub) if (sub != S_OK) return S_FALSE;

HINSTANCE g_hInst = 0;
CLIST_INTERFACE *pcli = NULL;
CLIST_INTERFACE corecli = {0};
CLUIDATA g_CluiData = {0};
int hLangpack;

TIME_API tmi;

pfnTryEnterCriticalSection fnTryEnterCriticalSection;

pfnGetAncestor fnGetAncestor;
pfnGetMenuBarInfo fnGetMenuBarInfo;
pfnGetScrollBarInfo fnGetScrollBarInfo;
pfnMsgWaitForMultipleObjectsEx fnMsgWaitForMultipleObjectsEx;

static HRESULT SubclassClistInterface();
static HRESULT CreateHookableEvents();
int EventArea_UnloadModule();

__forceinline int MakeVer(int a, int b, int c, int d)
{	return PLUGIN_MAKE_VERSION(a,b,c,d);
}

PLUGININFOEX pluginInfo = {
	sizeof(PLUGININFOEX),
	"Modern Contact List",
	MakeVer(PRODUCT_VERSION),
	"Displays contacts, event notifications, protocol status with advantage visual modifications. Supported MW modifications, enchanced metacontact cooperation.",
	"Artem Shpynov, Ricardo Pescuma Domenecci and Anton Senko based on clist_mw by Bethoven",
	"ashpynov@gmail.com" ,
	"Copyright 2000-2010 Miranda-IM project",
	"http://miranda-ng.org/",
	UNICODE_AWARE,
	//{043909B6-AAD8-4d82-8EB5-9F64CFE867CD}
	{0x43909b6, 0xaad8, 0x4d82, { 0x8e, 0xb5, 0x9f, 0x64, 0xcf, 0xe8, 0x67, 0xcd }}
};

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = {MIID_CLIST, MIID_LAST};

BOOL WINAPI DllMain(HINSTANCE hInstDLL, DWORD dwReason, LPVOID reserved)
{
	g_hInst = hInstDLL;
	DisableThreadLibraryCalls(g_hInst);
	return TRUE;
}

PLUGININTERFACE PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}

PLUGININTERFACE int CListInitialise()
{
	HMODULE hKernel = GetModuleHandleA("kernel32.dll");
	fnTryEnterCriticalSection = ( pfnTryEnterCriticalSection )GetProcAddress( hKernel, "TryEnterCriticalSection");

	HMODULE hUser = GetModuleHandleA("user32.dll");
	fnGetMenuBarInfo = ( pfnGetMenuBarInfo )GetProcAddress( hUser, "GetMenuBarInfo");
	fnGetScrollBarInfo = ( pfnGetScrollBarInfo )GetProcAddress( hUser, "GetScrollBarInfo");
	fnMsgWaitForMultipleObjectsEx = ( pfnMsgWaitForMultipleObjectsEx )GetProcAddress( hUser, "MsgWaitForMultipleObjectsEx");

	if (( fnGetAncestor = ( pfnGetAncestor )GetProcAddress( hUser, "GetAncestor")) == NULL )
		fnGetAncestor = MyGetAncestor;

	g_dwMainThreadID = GetCurrentThreadId();
	mir_getTMI(&tmi);
	mir_getLP( &pluginInfo );

	CHECKRES ( PreLoadContactListModule ( )	);
	CHECKRES ( SubclassClistInterface ( )	);
	CHECKRES ( CreateHookableEvents ( )		);
	CHECKRES ( XPThemesLoadModule ( )		);
	CHECKRES ( SkinEngineLoadModule ( )		);
	CHECKRES ( BackgroundsLoadModule ( )	);
	CHECKRES ( CluiLoadModule ( )			);
	CHECKRES ( ClcLoadModule ( )			);
	CHECKRES ( ToolbarButtonLoadModule( )   );
	CHECKRES ( ToolbarLoadModule()			);

	TRACE("CListInitialise Modern Contact List ... Done\r\n");

	return S_OK;
}

// never called by a newer plugin loader.
PLUGININTERFACE int Load(void)
{
	MessageBoxA(0,"You Running Old Miranda, use " MINIMAL_COREVERSION_STR " version!","Modern Clist",0);
	return 1;
}

PLUGININTERFACE int Unload(void)
{
	TRACE("Unloading Clist Modern\r\n");

	if (IsWindow(pcli->hwndContactList)) DestroyWindow(pcli->hwndContactList);
	pcli->hwndContactList = NULL;

	BackgroundsUnloadModule();
	SkinEngineUnloadModule();
	XPThemesUnloadModule();

	UnloadAvatarOverlayIcon();
	UninitSkinHotKeys();
	FreeRowCell();
	EventArea_UnloadModule();

	TRACE("Unloading Clist Modern COMPLETE\r\n");
	return 0;
}


static HRESULT SubclassClistInterface()
{
	// get the contact list interface
	pcli = ( CLIST_INTERFACE* )CallService(MS_CLIST_RETRIEVE_INTERFACE, 0, (LPARAM)g_hInst);

	// OVERLOAD CLIST INTERFACE FUNCTIONS
	//
	//	Naming convention is:
	//  'cli*'  - new handler without default core service calling
	//  'save*' - pointer to stored default parent handle
	//	'cli_*'	- new handler with default core service calling

	corecli = *pcli;

	pcli->bDisplayLocked = TRUE;

	pcli->pfnCheckCacheItem	= cliCheckCacheItem;
	pcli->pfnFreeCacheItem = cliFreeCacheItem;
	pcli->pfnInvalidateDisplayNameCacheEntry	 = cliInvalidateDisplayNameCacheEntry;

	pcli->pfnTrayIconUpdateBase = cliTrayIconUpdateBase;
	pcli->pfnCluiProtocolStatusChanged = cliCluiProtocolStatusChanged;

	pcli->pfnBeginRenameSelection  = cliBeginRenameSelection;
	pcli->pfnCreateClcContact      = cliCreateClcContact;
	pcli->pfnCreateCacheItem       = cliCreateCacheItem;
	pcli->pfnGetRowBottomY         = cliGetRowBottomY;
	pcli->pfnGetRowHeight          = cliGetRowHeight;
	pcli->pfnGetRowTopY            = cliGetRowTopY;
	pcli->pfnGetRowTotalHeight     = cliGetRowTotalHeight;
	pcli->pfnInvalidateRect        = CLUI__cliInvalidateRect;
	pcli->pfnGetCacheEntry         = cliGetCacheEntry;
	pcli->pfnOnCreateClc           = CLUI::cliOnCreateClc;
	pcli->pfnPaintClc              = CLCPaint::cliPaintClc;
	pcli->pfnRebuildEntireList     = cliRebuildEntireList;
	pcli->pfnRecalcScrollBar       = cliRecalcScrollBar;
	pcli->pfnRowHitTest            = cliRowHitTest;
	pcli->pfnScrollTo              = cliScrollTo;
	pcli->pfnShowHide              = cliShowHide;
	pcli->pfnHitTest               = cliHitTest;
	pcli->pfnCompareContacts       = cliCompareContacts;
	pcli->pfnBuildGroupPopupMenu   = cliBuildGroupPopupMenu;
	pcli->pfnGetIconFromStatusMode = cliGetIconFromStatusMode;
	pcli->pfnFindItem              = cliFindItem;
	pcli->pfnGetRowByIndex         = cliGetRowByIndex;
	pcli->pfnGetRowsPriorTo        = cliGetRowsPriorTo;
	pcli->pfnGetGroupContentsCount = cliGetGroupContentsCount;
	pcli->pfnCreateEvent           = cliCreateEvent;
	pcli->pfnFindRowByText         = cliFindRowByText;

	//partialy overloaded - call default handlers from inside
	pcli->pfnIconFromStatusMode    = cli_IconFromStatusMode;
	pcli->pfnLoadCluiGlobalOpts    = CLUI_cli_LoadCluiGlobalOpts;
	pcli->pfnSortCLC               = cli_SortCLC;
	pcli->pfnAddGroup              = cli_AddGroup;
	pcli->pfnGetGroupCountsText    = cli_GetGroupCountsText;
	pcli->pfnAddContactToTree      = cli_AddContactToTree;
	pcli->pfnAddInfoItemToGroup    = cli_AddInfoItemToGroup;
	pcli->pfnAddItemToGroup        = cli_AddItemToGroup;
	pcli->pfnContactListWndProc    = CLUI::cli_ContactListWndProc;
	pcli->pfnDeleteItemFromTree    = cli_DeleteItemFromTree;
	pcli->pfnFreeContact           = cli_FreeContact;
	pcli->pfnFreeGroup             = cli_FreeGroup;
	pcli->pfnChangeContactIcon     = cli_ChangeContactIcon;
	pcli->pfnTrayIconProcessMessage = cli_TrayIconProcessMessage;
	pcli->pfnSaveStateAndRebuildList = cli_SaveStateAndRebuildList;
	pcli->pfnContactListControlWndProc = cli_ContactListControlWndProc;
	pcli->pfnProcessExternalMessages = cli_ProcessExternalMessages;
	pcli->pfnAddEvent              = cli_AddEvent;
	pcli->pfnRemoveEvent           = cli_RemoveEvent;
	pcli->pfnDocking_ProcessWindowMessage = Docking_ProcessWindowMessage;
	return S_OK;
}

static HRESULT CreateHookableEvents()
{
//	g_CluiData.hEventExtraImageListRebuilding  = CreateHookableEvent(ME_CLIST_EXTRA_LIST_REBUILD);
//	g_CluiData.hEventExtraImageApplying        = CreateHookableEvent(ME_CLIST_EXTRA_IMAGE_APPLY);
// g_CluiData.hEventExtraClick                = CreateHookableEvent(ME_CLIST_EXTRA_CLICK);
	g_CluiData.hEventBkgrChanged               = CreateHookableEvent(ME_BACKGROUNDCONFIG_CHANGED);
	g_CluiData.hEventPreBuildTrayMenu          = CreateHookableEvent(ME_CLIST_PREBUILDTRAYMENU);
	g_CluiData.hEventPreBuildFrameMenu         = CreateHookableEvent(ME_CLIST_PREBUILDFRAMEMENU);
	g_CluiData.hEventPreBuildGroupMenu         = CreateHookableEvent(ME_CLIST_PREBUILDGROUPMENU);
	g_CluiData.hEventPreBuildSubGroupMenu      = CreateHookableEvent(ME_CLIST_PREBUILDSUBGROUPMENU);
	g_CluiData.hEventStatusBarShowToolTip      = CreateHookableEvent(ME_CLIST_FRAMES_SB_SHOW_TOOLTIP);
	g_CluiData.hEventStatusBarHideToolTip      = CreateHookableEvent(ME_CLIST_FRAMES_SB_HIDE_TOOLTIP);
	g_CluiData.hEventSkinServicesCreated       = CreateHookableEvent(ME_SKIN_SERVICESCREATED);
	return S_OK;
}
