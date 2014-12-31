/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org),
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

#include "..\..\core\commonheaders.h"
#include "clc.h"
#include "genmenu.h"
#include "..\extraicons\extraicons.h"

CLIST_INTERFACE cli = { 0 };

static TCHAR szTip[MAX_TIP_SIZE+1];

int LoadContactListModule2(void);
int LoadCLCModule(void);
void BuildProtoMenus(void);

static int interfaceInited = 0;

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

static void fnReloadProtoMenus(void)
{
	RebuildMenuOrder();
	if (db_get_b(NULL, "CList", "MoveProtoMenus", TRUE))
		BuildProtoMenus();
	cli.pfnCluiProtocolStatusChanged(0, 0);
}

static INT_PTR srvRetrieveInterface(WPARAM, LPARAM)
{
	int rc;

	if (interfaceInited == 0) {
		cli.version = 6;
		cli.bDisplayLocked = TRUE;

		cli.pfnClcOptionsChanged               = fnClcOptionsChanged;
		cli.pfnClcBroadcast                    = fnClcBroadcast;
		cli.pfnContactListControlWndProc       = fnContactListControlWndProc;
		cli.pfnBuildGroupPopupMenu             = fnBuildGroupPopupMenu;

		cli.pfnRegisterFileDropping            = fnRegisterFileDropping;
		cli.pfnUnregisterFileDropping          = fnUnregisterFileDropping;

		cli.pfnGetRowsPriorTo                  = fnGetRowsPriorTo;
		cli.pfnFindItem                        = fnFindItem;
		cli.pfnGetRowByIndex                   = fnGetRowByIndex;
		cli.pfnContactToHItem                  = fnContactToHItem;
		cli.pfnContactToItemHandle             = fnContactToItemHandle;

		cli.pfnAddGroup                        = fnAddGroup;
		cli.pfnAddItemToGroup                  = fnAddItemToGroup;
		cli.pfnCreateClcContact                = fnCreateClcContact;
		cli.pfnRemoveItemFromGroup             = fnRemoveItemFromGroup;
		cli.pfnFreeContact                     = fnFreeContact;
		cli.pfnFreeGroup                       = fnFreeGroup;
		cli.pfnAddInfoItemToGroup              = fnAddInfoItemToGroup;
		cli.pfnAddContactToGroup               = fnAddContactToGroup;
		cli.pfnAddContactToTree                = fnAddContactToTree;
		cli.pfnDeleteItemFromTree              = fnDeleteItemFromTree;
		cli.pfnRebuildEntireList               = fnRebuildEntireList;
		cli.pfnGetGroupContentsCount           = fnGetGroupContentsCount;
		cli.pfnSortCLC                         = fnSortCLC;
		cli.pfnSaveStateAndRebuildList         = fnSaveStateAndRebuildList;

		cli.pfnProcessExternalMessages         = fnProcessExternalMessages;

		cli.pfnPaintClc                        = fnPaintClc;

		cli.pfnGetGroupCountsText              = fnGetGroupCountsText;
		cli.pfnHitTest                         = fnHitTest;
		cli.pfnScrollTo                        = fnScrollTo;
		cli.pfnEnsureVisible                   = fnEnsureVisible;
		cli.pfnRecalcScrollBar                 = fnRecalcScrollBar;
		cli.pfnSetGroupExpand                  = fnSetGroupExpand;
		cli.pfnDoSelectionDefaultAction        = fnDoSelectionDefaultAction;
		cli.pfnFindRowByText                   = fnFindRowByText;
		cli.pfnEndRename                       = fnEndRename;
		cli.pfnDeleteFromContactList           = fnDeleteFromContactList;
		cli.pfnBeginRenameSelection            = fnBeginRenameSelection;
		cli.pfnCalcEipPosition                 = fnCalcEipPosition;
		cli.pfnGetDropTargetInformation        = fnGetDropTargetInformation;
		cli.pfnClcStatusToPf2                  = fnClcStatusToPf2;
		cli.pfnIsHiddenMode                    = fnIsHiddenMode;
		cli.pfnHideInfoTip                     = fnHideInfoTip;
		cli.pfnNotifyNewContact                = fnNotifyNewContact;
		cli.pfnGetDefaultExStyle               = fnGetDefaultExStyle;
		cli.pfnGetDefaultFontSetting           = fnGetDefaultFontSetting;
		cli.pfnGetFontSetting                  = fnGetFontSetting;
		cli.pfnLoadClcOptions                  = fnLoadClcOptions;
		cli.pfnRecalculateGroupCheckboxes	   = fnRecalculateGroupCheckboxes;
		cli.pfnSetGroupChildCheckboxes		   = fnSetGroupChildCheckboxes;
		cli.pfnSetContactCheckboxes            = fnSetContactCheckboxes;
		cli.pfnInvalidateItem                  = fnInvalidateItem;
		cli.pfnGetRowBottomY                   = fnGetRowBottomY;
		cli.pfnGetRowHeight                    = fnGetRowHeight;
		cli.pfnGetRowTopY                      = fnGetRowTopY;
		cli.pfnGetRowTotalHeight               = fnGetRowTotalHeight;
		cli.pfnRowHitTest                      = fnRowHitTest;

		cli.pfnAddEvent                        = fnAddEvent;
		cli.pfnCreateEvent                     = fnCreateEvent;
		cli.pfnEventsProcessContactDoubleClick = fnEventsProcessContactDoubleClick;
		cli.pfnEventsProcessTrayDoubleClick	   = fnEventsProcessTrayDoubleClick;
		cli.pfnFreeEvent                       = fnFreeEvent;
		cli.pfnGetEvent                        = fnGetEvent;
		cli.pfnGetImlIconIndex                 = fnGetImlIconIndex;
		cli.pfnRemoveEvent                     = fnRemoveEvent;

		cli.pfnGetContactDisplayName           = fnGetContactDisplayName;
		cli.pfnInvalidateDisplayNameCacheEntry = fnInvalidateDisplayNameCacheEntry;
		cli.pfnCreateCacheItem                 = fnCreateCacheItem;
		cli.pfnCheckCacheItem                  = fnCheckCacheItem;
		cli.pfnFreeCacheItem                   = fnFreeCacheItem;
		cli.pfnGetCacheEntry                   = fnGetCacheEntry;

		cli.szTip                              = szTip;
		cli.pfnInitTray                        = fnInitTray;
		cli.pfnUninitTray                      = fnUninitTray;

		cli.pfnTrayCycleTimerProc              = fnTrayCycleTimerProc;
		cli.pfnTrayIconAdd                     = fnTrayIconAdd;
		cli.pfnTrayIconDestroy                 = fnTrayIconDestroy;
		cli.pfnTrayIconIconsChanged            = fnTrayIconIconsChanged;
		cli.pfnTrayIconInit                    = fnTrayIconInit;
		cli.pfnTrayIconMakeTooltip             = fnTrayIconMakeTooltip;
		cli.pfnTrayIconPauseAutoHide           = fnTrayIconPauseAutoHide;
		cli.pfnTrayIconProcessMessage          = fnTrayIconProcessMessage;
		cli.pfnTrayIconRemove                  = fnTrayIconRemove;
		cli.pfnTrayIconSetBaseInfo             = fnTrayIconSetBaseInfo;
		cli.pfnTrayIconSetToBase               = fnTrayIconSetToBase;
		cli.pfnTrayIconTaskbarCreated          = fnTrayIconTaskbarCreated;
		cli.pfnTrayIconUpdate                  = fnTrayIconUpdate;
		cli.pfnTrayIconUpdateBase              = fnTrayIconUpdateBase;
		cli.pfnTrayCalcChanged                 = fnTrayCalcChanged;
		cli.pfnTrayIconUpdateWithImageList	   = fnTrayIconUpdateWithImageList;
		cli.pfnCListTrayNotify                 = fnCListTrayNotify;

		cli.pfnContactListWndProc              = fnContactListWndProc;
		cli.pfnLoadCluiGlobalOpts              = fnLoadCluiGlobalOpts;
		cli.pfnCluiProtocolStatusChanged       = fnCluiProtocolStatusChanged;
		cli.pfnDrawMenuItem                    = fnDrawMenuItem;
		cli.pfnInvalidateRect                  = fnInvalidateRect;
		cli.pfnOnCreateClc                     = fnOnCreateClc;

		cli.pfnChangeContactIcon               = fnChangeContactIcon;
		cli.pfnLoadContactTree                 = fnLoadContactTree;
		cli.pfnCompareContacts                 = fnCompareContacts;
		cli.pfnSortContacts                    = fnSortContacts;
		cli.pfnSetHideOffline                  = fnSetHideOffline;

		cli.pfnDocking_ProcessWindowMessage	   = fnDocking_ProcessWindowMessage;

		cli.pfnGetIconFromStatusMode           = fnGetIconFromStatusMode;
		cli.pfnGetWindowVisibleState           = fnGetWindowVisibleState;
		cli.pfnIconFromStatusMode              = fnIconFromStatusMode;
		cli.pfnShowHide                        = fnShowHide;
		cli.pfnGetStatusModeDescription        = fnGetStatusModeDescription;

		cli.pfnGetGroupName                    = fnGetGroupName;
		cli.pfnRenameGroup                     = fnRenameGroup;

		cli.pfnHotKeysRegister                 = fnHotKeysRegister;
		cli.pfnHotKeysUnregister               = fnHotKeysUnregister;
		cli.pfnHotKeysProcess                  = fnHotKeysProcess;
		cli.pfnHotkeysProcessMessage           = fnHotkeysProcessMessage;

		cli.pfnGetProtocolVisibility           = fnGetProtocolVisibility;
		cli.pfnGetProtoIndexByPos              = fnGetProtoIndexByPos;
		cli.pfnReloadProtoMenus                = fnReloadProtoMenus;
		cli.pfnGetAccountIndexByPos            = fnGetAccountIndexByPos;
		cli.pfnGetProtocolMenu                 = fnGetProtocolMenu;
		cli.pfnConvertMenu                     = fnConvertMenu;

		cli.pfnReloadExtraIcons                = fnReloadExtraIcons;
		cli.pfnSetAllExtraIcons                = fnSetAllExtraIcons;

		cli.pfnGetContactIcon                  = fnGetContactIcon;
		cli.pfnGetAverageMode                  = fnGetAverageMode;
		cli.pfnInitAutoRebuild                 = fnInitAutoRebuild;

		rc = LoadContactListModule2();
		if (rc == 0)
			rc = LoadCLCModule();
		interfaceInited = 1;
	}

	return (LPARAM)&cli;
}

int LoadContactListModule()
{
	CreateServiceFunction(MS_CLIST_RETRIEVE_INTERFACE, srvRetrieveInterface);
	return 0;
}
