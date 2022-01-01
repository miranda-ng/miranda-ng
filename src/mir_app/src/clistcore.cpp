/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org),
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

MIR_APP_EXPORT CLIST_INTERFACE g_clistApi;

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
	g_clistApi.menuProtos = &g_menuProtos;
	g_clistApi.bOwnerDrawMenu = true;

	g_clistApi.pfnContactListControlWndProc = fnContactListControlWndProc;

	g_clistApi.pfnGetRowsPriorTo = fnGetRowsPriorTo;
	g_clistApi.pfnFindItem = fnFindItem;
	g_clistApi.pfnGetRowByIndex = fnGetRowByIndex;
	g_clistApi.pfnGetContactHiddenStatus = fnGetContactHiddenStatus;

	g_clistApi.pfnAddGroup = fnAddGroup;
	g_clistApi.pfnAddItemToGroup = fnAddItemToGroup;
	g_clistApi.pfnCreateClcContact = fnCreateClcContact;
	
	g_clistApi.pfnFreeContact = fnFreeContact;	
	g_clistApi.pfnAddInfoItemToGroup = fnAddInfoItemToGroup;	
	g_clistApi.pfnAddContactToGroup = fnAddContactToGroup;	
	g_clistApi.pfnAddContactToTree = fnAddContactToTree;	
	g_clistApi.pfnRebuildEntireList = fnRebuildEntireList;	
	g_clistApi.pfnGetGroupContentsCount = fnGetGroupContentsCount;
	g_clistApi.pfnSortCLC = fnSortCLC;

	g_clistApi.pfnProcessExternalMessages = fnProcessExternalMessages;

	g_clistApi.pfnPaintClc = fnPaintClc;

	g_clistApi.pfnHitTest = fnHitTest;	
	g_clistApi.pfnScrollTo = fnScrollTo;	
	g_clistApi.pfnRecalcScrollBar = fnRecalcScrollBar;	
	g_clistApi.pfnSetGroupExpand = fnSetGroupExpand;
	g_clistApi.pfnFindRowByText = fnFindRowByText;	
	g_clistApi.pfnBeginRenameSelection = fnBeginRenameSelection;
	g_clistApi.pfnIsVisibleContact = fnIsVisibleContact;
	g_clistApi.pfnGetDefaultFontSetting = fnGetDefaultFontSetting;
	g_clistApi.pfnLoadClcOptions = fnLoadClcOptions;
	g_clistApi.pfnGetRowBottomY = fnGetRowBottomY;
	g_clistApi.pfnGetRowHeight = fnGetRowHeight;
	g_clistApi.pfnGetRowTopY = fnGetRowTopY;
	g_clistApi.pfnGetRowTotalHeight = fnGetRowTotalHeight;
	g_clistApi.pfnRowHitTest = fnRowHitTest;

	g_clistApi.pfnAddEvent = fnAddEvent;
	g_clistApi.pfnGetEvent = fnGetEvent;
	g_clistApi.pfnGetImlIconIndex = fnGetImlIconIndex;
	g_clistApi.pfnRemoveEvent = fnRemoveEvent;

	g_clistApi.pfnInvalidateDisplayNameCacheEntry = fnInvalidateDisplayNameCacheEntry;
	g_clistApi.pfnCreateCacheItem = fnCreateCacheItem;
	g_clistApi.pfnCheckCacheItem = fnCheckCacheItem;
	g_clistApi.pfnFreeCacheItem = fnFreeCacheItem;

	g_clistApi.szTip = szTip;

	g_clistApi.pfnTrayIconInit = fnTrayIconInit;
	g_clistApi.pfnTrayIconPauseAutoHide = fnTrayIconPauseAutoHide;
	g_clistApi.pfnTrayIconProcessMessage = fnTrayIconProcessMessage;

	g_clistApi.pfnContactListWndProc = fnContactListWndProc;
	g_clistApi.pfnLoadCluiGlobalOpts = fnLoadCluiGlobalOpts;
	g_clistApi.pfnCluiProtocolStatusChanged = fnCluiProtocolStatusChanged;
	g_clistApi.pfnInvalidateRect = fnInvalidateRect;
	g_clistApi.pfnOnCreateClc = fnOnCreateClc;

	g_clistApi.pfnSetHideOffline = fnSetHideOffline;

	g_clistApi.pfnDocking_ProcessWindowMessage = fnDocking_ProcessWindowMessage;

	g_clistApi.pfnGetIconFromStatusMode = fnGetIconFromStatusMode;
	g_clistApi.pfnGetWindowVisibleState = fnGetWindowVisibleState;
	g_clistApi.pfnIconFromStatusMode = fnIconFromStatusMode;
	g_clistApi.pfnShowHide = fnShowHide;

	g_clistApi.pfnTrayCalcChanged = fnTrayCalcChanged;
	g_clistApi.pfnSetContactCheckboxes = fnSetContactCheckboxes;
}

MIR_APP_DLL(CLIST_INTERFACE*) Clist_GetInterface(void)
{
	if (g_bReadyToInitClist) {
		LoadContactListModule2();
		LoadCLCModule();
		g_bReadyToInitClist = false;
	}

	return &g_clistApi;
}
