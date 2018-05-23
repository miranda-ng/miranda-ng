/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-18 Miranda NG team (https://miranda-ng.org),
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

CMPlugin g_plugin;
HINSTANCE g_hMirApp = nullptr;

CLIST_INTERFACE corecli = { 0 };
CLUIDATA g_CluiData = {};

static HRESULT SubclassClistInterface();
static HRESULT CreateHookableEvents();
int EventArea_UnloadModule();

/////////////////////////////////////////////////////////////////////////////////////////

PLUGININFOEX pluginInfoEx = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {043909B6-AAD8-4D82-8EB5-9F64CFE867CD}
	{ 0x43909b6, 0xaad8, 0x4d82, { 0x8e, 0xb5, 0x9f, 0x64, 0xcf, 0xe8, 0x67, 0xcd } }
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>("CList", pluginInfoEx)
{}

/////////////////////////////////////////////////////////////////////////////////////////

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = { MIID_CLIST, MIID_LAST };

/////////////////////////////////////////////////////////////////////////////////////////

extern "C" __declspec(dllexport) int CListInitialise()
{
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

/////////////////////////////////////////////////////////////////////////////////////////
// never called by a newer plugin loader.

extern "C" __declspec(dllexport) int Load(void)
{
	return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////

extern "C" __declspec(dllexport) int Unload(void)
{
	TRACE("Unloading Clist Modern\r\n");

	if (IsWindow(g_CLI.hwndContactList))
		DestroyWindow(g_CLI.hwndContactList);
	g_CLI.hwndContactList = nullptr;

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

	Clist_GetInterface();
	corecli = g_CLI;

	g_CLI.hInst = g_plugin.getInst();

	g_CLI.pfnCreateCacheItem = cliCreateCacheItem;
	g_CLI.pfnCheckCacheItem = cliCheckCacheItem;
	g_CLI.pfnFreeCacheItem = cliFreeCacheItem;
	g_CLI.pfnInvalidateDisplayNameCacheEntry = cliInvalidateDisplayNameCacheEntry;

	g_CLI.pfnTrayIconPauseAutoHide = cliTrayIconPauseAutoHide;
	g_CLI.pfnTrayIconInit = cliTrayIconInit;
	g_CLI.pfnTrayCalcChanged = cliTrayCalcChanged;

	g_CLI.pfnCluiProtocolStatusChanged = cliCluiProtocolStatusChanged;

	g_CLI.pfnBeginRenameSelection = cliBeginRenameSelection;
	g_CLI.pfnCreateClcContact = cliCreateClcContact;
	g_CLI.pfnGetRowBottomY = cliGetRowBottomY;
	g_CLI.pfnGetRowHeight = cliGetRowHeight;
	g_CLI.pfnGetRowTopY = cliGetRowTopY;
	g_CLI.pfnGetRowTotalHeight = cliGetRowTotalHeight;
	g_CLI.pfnInvalidateRect = cliInvalidateRect;
	g_CLI.pfnOnCreateClc = CLUI::cliOnCreateClc;
	g_CLI.pfnPaintClc = CLCPaint::cliPaintClc;
	g_CLI.pfnRebuildEntireList = cliRebuildEntireList;
	g_CLI.pfnIsVisibleContact = cliIsVisibleContact;
	g_CLI.pfnRecalcScrollBar = cliRecalcScrollBar;
	g_CLI.pfnRowHitTest = cliRowHitTest;
	g_CLI.pfnScrollTo = cliScrollTo;
	g_CLI.pfnShowHide = cliShowHideStub;
	g_CLI.pfnHitTest = cliHitTest;
	g_CLI.pfnCompareContacts = cliCompareContacts;
	g_CLI.pfnGetIconFromStatusMode = cliGetIconFromStatusMode;
	g_CLI.pfnFindItem = cliFindItem;
	g_CLI.pfnGetRowByIndex = cliGetRowByIndex;
	g_CLI.pfnGetRowsPriorTo = cliGetRowsPriorTo;
	g_CLI.pfnGetGroupContentsCount = cliGetGroupContentsCount;
	g_CLI.pfnFindRowByText = cliFindRowByText;
	g_CLI.pfnGetContactHiddenStatus = CLVM_GetContactHiddenStatus;

	// partialy overloaded - call default handlers from inside
	g_CLI.pfnIconFromStatusMode = cli_IconFromStatusMode;
	g_CLI.pfnLoadCluiGlobalOpts = CLUI_cli_LoadCluiGlobalOpts;
	g_CLI.pfnLoadClcOptions = cli_LoadCLCOptions;
	g_CLI.pfnSortCLC = cli_SortCLC;
	g_CLI.pfnAddContactToGroup = cli_AddContactToGroup;
	g_CLI.pfnAddContactToTree = cli_AddContactToTree;
	g_CLI.pfnContactListWndProc = CLUI::cli_ContactListWndProc;
	g_CLI.pfnFreeContact = cli_FreeContact;
	g_CLI.pfnSetContactCheckboxes = cli_SetContactCheckboxes;
	g_CLI.pfnTrayIconProcessMessage = cli_TrayIconProcessMessage;
	g_CLI.pfnContactListControlWndProc = cli_ContactListControlWndProc;
	g_CLI.pfnProcessExternalMessages = cli_ProcessExternalMessages;
	g_CLI.pfnAddEvent = cli_AddEvent;
	g_CLI.pfnRemoveEvent = cli_RemoveEvent;
	g_CLI.pfnDocking_ProcessWindowMessage = Docking_ProcessWindowMessage;
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
