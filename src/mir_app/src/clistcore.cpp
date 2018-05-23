/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-18 Miranda NG team (https://miranda-ng.org),
Copyright (c) 2000-12 Miranda IM project,
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
#include "clc.h"
#include "genmenu.h"

int LoadContactListModule2(void);
int LoadCLCModule(void);

MIR_APP_DLL(CLIST_INTERFACE) g_CLI;

static wchar_t szTip[MAX_TIP_SIZE+1];

static void fnPaintClc(HWND, ClcData*, HDC, RECT*)
{
}

static ClcContact* fnCreateClcContact(void)
{
	return (ClcContact*)mir_calloc(sizeof(ClcContact));
}

static BOOL fnInvalidateRect(HWND hwnd, CONST RECT* lpRect, BOOL bErase)
{
	return InvalidateRect(hwnd, lpRect, bErase);
}

static void fnOnCreateClc(void)
{
}

static int fnIsVisibleContact(ClcCacheEntry*, ClcGroup*)
{
	return false;
}

void InitClistCore()
{
	g_CLI.menuProtos = &g_menuProtos;

	g_CLI.pfnContactListControlWndProc = fnContactListControlWndProc;

	g_CLI.pfnGetRowsPriorTo = fnGetRowsPriorTo;
	g_CLI.pfnFindItem = fnFindItem;
	g_CLI.pfnGetRowByIndex = fnGetRowByIndex;
	g_CLI.pfnGetContactHiddenStatus = fnGetContactHiddenStatus;

	g_CLI.pfnAddGroup = fnAddGroup;
	g_CLI.pfnAddItemToGroup = fnAddItemToGroup;
	g_CLI.pfnCreateClcContact = fnCreateClcContact;
	
	g_CLI.pfnFreeContact = fnFreeContact;	
	g_CLI.pfnAddInfoItemToGroup = fnAddInfoItemToGroup;	
	g_CLI.pfnAddContactToGroup = fnAddContactToGroup;	
	g_CLI.pfnAddContactToTree = fnAddContactToTree;	
	g_CLI.pfnRebuildEntireList = fnRebuildEntireList;	
	g_CLI.pfnGetGroupContentsCount = fnGetGroupContentsCount;
	g_CLI.pfnSortCLC = fnSortCLC;

	g_CLI.pfnProcessExternalMessages = fnProcessExternalMessages;

	g_CLI.pfnPaintClc = fnPaintClc;

	g_CLI.pfnHitTest = fnHitTest;	
	g_CLI.pfnScrollTo = fnScrollTo;	
	g_CLI.pfnRecalcScrollBar = fnRecalcScrollBar;	
	g_CLI.pfnSetGroupExpand = fnSetGroupExpand;
	g_CLI.pfnFindRowByText = fnFindRowByText;	
	g_CLI.pfnBeginRenameSelection = fnBeginRenameSelection;
	g_CLI.pfnIsVisibleContact = fnIsVisibleContact;
	g_CLI.pfnGetDefaultFontSetting = fnGetDefaultFontSetting;
	g_CLI.pfnLoadClcOptions = fnLoadClcOptions;
	g_CLI.pfnGetRowBottomY = fnGetRowBottomY;
	g_CLI.pfnGetRowHeight = fnGetRowHeight;
	g_CLI.pfnGetRowTopY = fnGetRowTopY;
	g_CLI.pfnGetRowTotalHeight = fnGetRowTotalHeight;
	g_CLI.pfnRowHitTest = fnRowHitTest;

	g_CLI.pfnAddEvent = fnAddEvent;
	g_CLI.pfnGetEvent = fnGetEvent;
	g_CLI.pfnGetImlIconIndex = fnGetImlIconIndex;
	g_CLI.pfnRemoveEvent = fnRemoveEvent;

	g_CLI.pfnInvalidateDisplayNameCacheEntry = fnInvalidateDisplayNameCacheEntry;
	g_CLI.pfnCreateCacheItem = fnCreateCacheItem;
	g_CLI.pfnCheckCacheItem = fnCheckCacheItem;
	g_CLI.pfnFreeCacheItem = fnFreeCacheItem;

	g_CLI.szTip = szTip;

	g_CLI.pfnTrayIconInit = fnTrayIconInit;
	g_CLI.pfnTrayIconPauseAutoHide = fnTrayIconPauseAutoHide;
	g_CLI.pfnTrayIconProcessMessage = fnTrayIconProcessMessage;

	g_CLI.pfnContactListWndProc = fnContactListWndProc;
	g_CLI.pfnLoadCluiGlobalOpts = fnLoadCluiGlobalOpts;
	g_CLI.pfnCluiProtocolStatusChanged = fnCluiProtocolStatusChanged;
	g_CLI.pfnInvalidateRect = fnInvalidateRect;
	g_CLI.pfnOnCreateClc = fnOnCreateClc;

	g_CLI.pfnSetHideOffline = fnSetHideOffline;

	g_CLI.pfnDocking_ProcessWindowMessage = fnDocking_ProcessWindowMessage;

	g_CLI.pfnGetIconFromStatusMode = fnGetIconFromStatusMode;
	g_CLI.pfnGetWindowVisibleState = fnGetWindowVisibleState;
	g_CLI.pfnIconFromStatusMode = fnIconFromStatusMode;
	g_CLI.pfnShowHide = fnShowHide;

	g_CLI.pfnTrayCalcChanged = fnTrayCalcChanged;
	g_CLI.pfnSetContactCheckboxes = fnSetContactCheckboxes;
}

MIR_APP_DLL(CLIST_INTERFACE*) Clist_GetInterface(void)
{
	if (g_bReadyToInitClist) {
		LoadContactListModule2();
		LoadCLCModule();
		g_bReadyToInitClist = false;
	}

	return &g_CLI;
}
