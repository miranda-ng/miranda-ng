/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org),
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

int CMPlugin::Load(void)
{
	g_dwMainThreadID = GetCurrentThreadId();
	g_hMirApp = GetModuleHandleA("mir_app.mir");

	ReadSettings();

	CHECKRES(PreLoadContactListModule());
	CHECKRES(SubclassClistInterface());
	CHECKRES(CreateHookableEvents());
	CHECKRES(SkinEngineLoadModule());
	CHECKRES(BackgroundsLoadModule());
	CHECKRES(CluiLoadModule());
	CHECKRES(ClcLoadModule());
	CHECKRES(ToolbarButtonLoadModule());
	CHECKRES(ToolbarLoadModule());
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int CMPlugin::Unload(void)
{
	TRACE("Unloading Clist Modern\r\n");

	if (IsWindow(g_clistApi.hwndContactList))
		DestroyWindow(g_clistApi.hwndContactList);
	g_clistApi.hwndContactList = nullptr;

	ToolbarButtonUnloadModule();
	BackgroundsUnloadModule();
	SkinEngineUnloadModule();
	XPThemesUnloadModule();

	UnloadAvatarOverlayIcon();
	EventArea_UnloadModule();

	TRACE("Unloading Clist Modern COMPLETE\r\n");
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CMPlugin::ReadSettings()
{
	wcsncpy_s(secondLine.text, getMStringW("SecondLineText"), _TRUNCATE);
	secondLine.iType = getWord("SecondLineType", SETTING_SECONDLINE_TYPE_DEFAULT);
	secondLine.bActive = getBool("SecondLineShow", SETTING_SECONDLINE_SHOW_DEFAULT);
	secondLine.iTopSpace = getWord("SecondLineTopSpace", SETTING_SECONDLINE_TOPSPACE_DEFAULT);
	secondLine.bDrawSmilies = getBool("SecondLineDrawSmileys", SETTING_SECONDLINE_SMILEYS_DEFAULT);
	secondLine.bXstatusHasPriority = getBool("SecondLineXStatusHasPriority", SETTING_SECONDLINE_XSTATUS_DEFAULT);
	secondLine.bShowStatusIfNoAway = getBool("SecondLineShowStatusIfNoAway", SETTING_SECONDLINE_STATUSIFNOAWAY_DEFAULT);
	secondLine.bShowListeningIfNoAway = getBool("SecondLineShowListeningIfNoAway", SETTING_SECONDLINE_LISTENINGIFNOAWAY_DEFAULT);
	secondLine.bUseNameAndMessageForXstatus = getBool("SecondLineUseNameAndMessageForXStatus", SETTING_SECONDLINE_XSTATUSNAMETEXT_DEFAULT);

	wcsncpy_s(thirdLine.text, getMStringW("ThirdLineText"), _TRUNCATE);
	thirdLine.iType = getWord("ThirdLineType", SETTING_THIRDLINE_TYPE_DEFAULT);
	thirdLine.bActive = getBool("ThirdLineShow", SETTING_THIRDLINE_SHOW_DEFAULT);
	thirdLine.iTopSpace = getWord("ThirdLineTopSpace", SETTING_THIRDLINE_TOPSPACE_DEFAULT);
	thirdLine.bDrawSmilies = getBool("ThirdLineDrawSmileys", SETTING_THIRDLINE_SMILEYS_DEFAULT);
	thirdLine.bXstatusHasPriority = getBool("ThirdLineXStatusHasPriority", SETTING_THIRDLINE_XSTATUS_DEFAULT);
	thirdLine.bShowStatusIfNoAway = getBool("ThirdLineShowStatusIfNoAway", SETTING_THIRDLINE_STATUSIFNOAWAY_DEFAULT);
	thirdLine.bShowListeningIfNoAway = getBool("ThirdLineShowListeningIfNoAway", SETTING_THIRDLINE_LISTENINGIFNOAWAY_DEFAULT);
	thirdLine.bUseNameAndMessageForXstatus = getBool("ThirdLineUseNameAndMessageForXStatus", SETTING_THIRDLINE_XSTATUSNAMETEXT_DEFAULT);
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
	corecli = g_clistApi;

	g_clistApi.hInst = g_plugin.getInst();

	g_clistApi.pfnCreateCacheItem = cliCreateCacheItem;
	g_clistApi.pfnCheckCacheItem = cliCheckCacheItem;
	g_clistApi.pfnFreeCacheItem = cliFreeCacheItem;
	g_clistApi.pfnInvalidateDisplayNameCacheEntry = cliInvalidateDisplayNameCacheEntry;

	g_clistApi.pfnTrayIconPauseAutoHide = cliTrayIconPauseAutoHide;
	g_clistApi.pfnTrayIconInit = cliTrayIconInit;
	g_clistApi.pfnTrayCalcChanged = cliTrayCalcChanged;

	g_clistApi.pfnCluiProtocolStatusChanged = cliCluiProtocolStatusChanged;

	g_clistApi.pfnBeginRenameSelection = cliBeginRenameSelection;
	g_clistApi.pfnCreateClcContact = cliCreateClcContact;
	g_clistApi.pfnGetRowBottomY = cliGetRowBottomY;
	g_clistApi.pfnGetRowHeight = cliGetRowHeight;
	g_clistApi.pfnGetRowTopY = cliGetRowTopY;
	g_clistApi.pfnGetRowTotalHeight = cliGetRowTotalHeight;
	g_clistApi.pfnInvalidateRect = cliInvalidateRect;
	g_clistApi.pfnOnCreateClc = CLUI::cliOnCreateClc;
	g_clistApi.pfnPaintClc = CLCPaint::cliPaintClc;
	g_clistApi.pfnRebuildEntireList = cliRebuildEntireList;
	g_clistApi.pfnIsVisibleContact = cliIsVisibleContact;
	g_clistApi.pfnRecalcScrollBar = cliRecalcScrollBar;
	g_clistApi.pfnRowHitTest = cliRowHitTest;
	g_clistApi.pfnScrollTo = cliScrollTo;
	g_clistApi.pfnShowHide = cliShowHideStub;
	g_clistApi.pfnHitTest = cliHitTest;
	g_clistApi.pfnCompareContacts = cliCompareContacts;
	g_clistApi.pfnGetIconFromStatusMode = cliGetIconFromStatusMode;
	g_clistApi.pfnFindItem = cliFindItem;
	g_clistApi.pfnGetRowByIndex = cliGetRowByIndex;
	g_clistApi.pfnGetRowsPriorTo = cliGetRowsPriorTo;
	g_clistApi.pfnGetGroupContentsCount = cliGetGroupContentsCount;
	g_clistApi.pfnFindRowByText = cliFindRowByText;
	g_clistApi.pfnGetContactHiddenStatus = CLVM_GetContactHiddenStatus;

	// partialy overloaded - call default handlers from inside
	g_clistApi.pfnIconFromStatusMode = cli_IconFromStatusMode;
	g_clistApi.pfnLoadCluiGlobalOpts = CLUI_cli_LoadCluiGlobalOpts;
	g_clistApi.pfnLoadClcOptions = cli_LoadCLCOptions;
	g_clistApi.pfnSortCLC = cli_SortCLC;
	g_clistApi.pfnAddContactToGroup = cli_AddContactToGroup;
	g_clistApi.pfnAddContactToTree = cli_AddContactToTree;
	g_clistApi.pfnContactListWndProc = CLUI::cli_ContactListWndProc;
	g_clistApi.pfnFreeContact = cli_FreeContact;
	g_clistApi.pfnSetContactCheckboxes = cli_SetContactCheckboxes;
	g_clistApi.pfnTrayIconProcessMessage = cli_TrayIconProcessMessage;
	g_clistApi.pfnContactListControlWndProc = cli_ContactListControlWndProc;
	g_clistApi.pfnProcessExternalMessages = cli_ProcessExternalMessages;
	g_clistApi.pfnAddEvent = cli_AddEvent;
	g_clistApi.pfnRemoveEvent = cli_RemoveEvent;
	g_clistApi.pfnDocking_ProcessWindowMessage = Docking_ProcessWindowMessage;
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
