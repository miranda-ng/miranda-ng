/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-17 Miranda NG project (https://miranda-ng.org),
Copyright (c) 2000-08 Miranda ICQ/IM project,
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

#include "stdafx.h"
#include "version.h"

#include "modern_clui.h"
#include "modern_clcpaint.h"

//definitions

#define CHECKRES(sub) if (sub != S_OK) return S_FALSE;

HINSTANCE g_hInst = 0, g_hMirApp = 0;
CLIST_INTERFACE *pcli = nullptr;
CLIST_INTERFACE corecli = { 0 };
CLUIDATA g_CluiData = { 0 };
int hLangpack;

static HRESULT SubclassClistInterface();
static HRESULT CreateHookableEvents();
int EventArea_UnloadModule();

PLUGININFOEX pluginInfo = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {043909B6-AAD8-4D82-8EB5-9F64CFE867CD}
	{ 0x43909b6, 0xaad8, 0x4d82, { 0x8e, 0xb5, 0x9f, 0x64, 0xcf, 0xe8, 0x67, 0xcd } }
};

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = { MIID_CLIST, MIID_LAST };

BOOL WINAPI DllMain(HINSTANCE hInstDLL, DWORD, LPVOID)
{
	g_hInst = hInstDLL;
	DisableThreadLibraryCalls(g_hInst);
	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD)
{
	return &pluginInfo;
}

extern "C" __declspec(dllexport) int CListInitialise()
{
	mir_getLP(&pluginInfo);

	g_dwMainThreadID = GetCurrentThreadId();
	g_hMirApp = GetModuleHandleA("mir_app.mir");

	CHECKRES(PreLoadContactListModule());
	CHECKRES(SubclassClistInterface());
	CHECKRES(CreateHookableEvents());
	CHECKRES(SkinEngineLoadModule());
	CHECKRES(BackgroundsLoadModule());
	CHECKRES(CluiLoadModule());
	CHECKRES(ClcLoadModule());
	CHECKRES(ToolbarButtonLoadModule());
	CHECKRES(ToolbarLoadModule());

	TRACE("CListInitialise Modern Contact List ... Done\r\n");

	return S_OK;
}

// never called by a newer plugin loader.
extern "C" __declspec(dllexport) int Load(void)
{
	return 1;
}

extern "C" __declspec(dllexport) int Unload(void)
{
	TRACE("Unloading Clist Modern\r\n");

	if (IsWindow(pcli->hwndContactList))
		DestroyWindow(pcli->hwndContactList);
	pcli->hwndContactList = nullptr;

	ToolbarButtonUnloadModule();
	BackgroundsUnloadModule();
	SkinEngineUnloadModule();
	XPThemesUnloadModule();

	UnloadAvatarOverlayIcon();
	FreeRowCell();
	EventArea_UnloadModule();

	TRACE("Unloading Clist Modern COMPLETE\r\n");
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

static int cliShowHideStub() { return cliShowHide(false); }

static HRESULT SubclassClistInterface()
{
	// OVERLOAD CLIST INTERFACE FUNCTIONS
	//
	//	Naming convention is:
	//  'cli*'  - new handler without default core service calling
	//  'save*' - pointer to stored default parent handle
	//	'cli_*'	- new handler with default core service calling

	pcli = Clist_GetInterface();
	corecli = *pcli;

	pcli->hInst = g_hInst;
	pcli->bDisplayLocked = TRUE;

	pcli->pfnCreateCacheItem = cliCreateCacheItem;
	pcli->pfnCheckCacheItem = cliCheckCacheItem;
	pcli->pfnFreeCacheItem = cliFreeCacheItem;
	pcli->pfnInvalidateDisplayNameCacheEntry = cliInvalidateDisplayNameCacheEntry;

	pcli->pfnTrayIconPauseAutoHide = cliTrayIconPauseAutoHide;
	pcli->pfnTrayIconInit = cliTrayIconInit;
	pcli->pfnTrayCalcChanged = cliTrayCalcChanged;

	pcli->pfnCluiProtocolStatusChanged = cliCluiProtocolStatusChanged;

	pcli->pfnBeginRenameSelection = cliBeginRenameSelection;
	pcli->pfnCreateClcContact = cliCreateClcContact;
	pcli->pfnGetRowBottomY = cliGetRowBottomY;
	pcli->pfnGetRowHeight = cliGetRowHeight;
	pcli->pfnGetRowTopY = cliGetRowTopY;
	pcli->pfnGetRowTotalHeight = cliGetRowTotalHeight;
	pcli->pfnInvalidateRect = cliInvalidateRect;
	pcli->pfnOnCreateClc = CLUI::cliOnCreateClc;
	pcli->pfnPaintClc = CLCPaint::cliPaintClc;
	pcli->pfnRebuildEntireList = cliRebuildEntireList;
	pcli->pfnIsVisibleContact = cliIsVisibleContact;
	pcli->pfnRecalcScrollBar = cliRecalcScrollBar;
	pcli->pfnRowHitTest = cliRowHitTest;
	pcli->pfnScrollTo = cliScrollTo;
	pcli->pfnShowHide = cliShowHideStub;
	pcli->pfnHitTest = cliHitTest;
	pcli->pfnCompareContacts = cliCompareContacts;
	pcli->pfnGetIconFromStatusMode = cliGetIconFromStatusMode;
	pcli->pfnFindItem = cliFindItem;
	pcli->pfnGetRowByIndex = cliGetRowByIndex;
	pcli->pfnGetRowsPriorTo = cliGetRowsPriorTo;
	pcli->pfnGetGroupContentsCount = cliGetGroupContentsCount;
	pcli->pfnFindRowByText = cliFindRowByText;
	pcli->pfnGetContactHiddenStatus = CLVM_GetContactHiddenStatus;

	// partialy overloaded - call default handlers from inside
	pcli->pfnGetContactIcon = cli_GetContactIcon;
	pcli->pfnIconFromStatusMode = cli_IconFromStatusMode;
	pcli->pfnLoadCluiGlobalOpts = CLUI_cli_LoadCluiGlobalOpts;
	pcli->pfnLoadClcOptions = cli_LoadCLCOptions;
	pcli->pfnSortCLC = cli_SortCLC;
	pcli->pfnAddContactToGroup = cli_AddContactToGroup;
	pcli->pfnAddContactToTree = cli_AddContactToTree;
	pcli->pfnContactListWndProc = CLUI::cli_ContactListWndProc;
	pcli->pfnFreeContact = cli_FreeContact;
	pcli->pfnSetContactCheckboxes = cli_SetContactCheckboxes;
	pcli->pfnTrayIconProcessMessage = cli_TrayIconProcessMessage;
	pcli->pfnContactListControlWndProc = cli_ContactListControlWndProc;
	pcli->pfnProcessExternalMessages = cli_ProcessExternalMessages;
	pcli->pfnAddEvent = cli_AddEvent;
	pcli->pfnRemoveEvent = cli_RemoveEvent;
	pcli->pfnDocking_ProcessWindowMessage = Docking_ProcessWindowMessage;
	return S_OK;
}

static HRESULT CreateHookableEvents()
{
	g_CluiData.hEventBkgrChanged = CreateHookableEvent(ME_BACKGROUNDCONFIG_CHANGED);
	g_CluiData.hEventStatusBarShowToolTip = CreateHookableEvent(ME_CLIST_FRAMES_SB_SHOW_TOOLTIP);
	g_CluiData.hEventStatusBarHideToolTip = CreateHookableEvent(ME_CLIST_FRAMES_SB_HIDE_TOOLTIP);
	g_CluiData.hEventSkinServicesCreated = CreateHookableEvent(ME_SKIN_SERVICESCREATED);
	return S_OK;
}
