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
#include "hdr/modern_commonheaders.h"
#include "m_clc.h"
#include "m_clui.h"
#include "hdr/modern_commonprototypes.h"

INT_PTR CListTray_GetGlobalStatus(WPARAM wparam,LPARAM lparam);

int CLUIUnreadEmailCountChanged(WPARAM wParam,LPARAM lParam)
{
	CallService(MS_SKINENG_INVALIDATEFRAMEIMAGE, 0 ,0);
	return 0;
}

INT_PTR CLUIServices_ProtocolStatusChanged(WPARAM wParam,LPARAM lParam)
{
	CallService(MS_SKINENG_INVALIDATEFRAMEIMAGE,(WPARAM)pcli->hwndStatus,0);
	if (lParam) cliTrayIconUpdateBase((char*)lParam);
	return 0;
}

void cliCluiProtocolStatusChanged(int status,const char * proto)
{
	CLUIServices_ProtocolStatusChanged((WPARAM)status,(LPARAM)proto);
}

INT_PTR SortList(WPARAM wParam,LPARAM lParam)
{
    pcli->pfnClcBroadcast( WM_TIMER,TIMERID_DELAYEDRESORTCLC,0);
    pcli->pfnClcBroadcast( INTM_SCROLLBARCHANGED,0,0);

	return 0;
}

static INT_PTR MetaSupportCheck(WPARAM wParam,LPARAM lParam)
{
	return 1;
}

static INT_PTR GetHwnd(WPARAM wParam, LPARAM lParam)
{
	return (INT_PTR)pcli->hwndContactList;
}

static INT_PTR GetHwndTree(WPARAM wParam,LPARAM lParam)
{
	return (INT_PTR)pcli->hwndContactTree;
}

static INT_PTR GroupAdded(WPARAM wParam, LPARAM lParam)
{
	//CLC does this automatically unless it's a new group
	if (lParam) {
		HANDLE hItem;
		TCHAR szFocusClass[64];
		HWND hwndFocus = GetFocus();

		GetClassName(hwndFocus, szFocusClass, SIZEOF(szFocusClass));
		if (!lstrcmp(szFocusClass, CLISTCONTROL_CLASS)) {
			hItem = (HANDLE) SendMessage(hwndFocus, CLM_FINDGROUP, wParam, 0);
			if (hItem)
				SendMessage(hwndFocus, CLM_EDITLABEL, (WPARAM) hItem, 0);
		}
	}
	return 0;
}

static INT_PTR ContactSetIcon(WPARAM wParam, LPARAM lParam)
{
	//unnecessary: CLC does this automatically
	return 0;
}

static INT_PTR ContactDeleted(WPARAM wParam, LPARAM lParam)
{
	//unnecessary: CLC does this automatically
	return 0;
}

static INT_PTR ContactAdded(WPARAM wParam, LPARAM lParam)
{
	//unnecessary: CLC does this automatically
	return 0;
}

static INT_PTR ListBeginRebuild(WPARAM wParam, LPARAM lParam)
{
	//unnecessary: CLC does this automatically
	return 0;
}

static INT_PTR ContactRenamed(WPARAM wParam, LPARAM lParam)
{
	//unnecessary: CLC does this automatically
	return 0;
}

static INT_PTR ListEndRebuild(WPARAM wParam, LPARAM lParam)
{
	int rebuild = 0;
	//CLC does this automatically, but we need to force it if hideoffline or hideempty has changed
	if ((ModernGetSettingByte(NULL, "CList", "HideOffline", SETTING_HIDEOFFLINE_DEFAULT) == 0) != ((GetWindowLong(pcli->hwndContactTree, GWL_STYLE) & CLS_HIDEOFFLINE) == 0)) {
		if (ModernGetSettingByte(NULL, "CList", "HideOffline", SETTING_HIDEOFFLINE_DEFAULT))
			SetWindowLong(pcli->hwndContactTree, GWL_STYLE, GetWindowLong(pcli->hwndContactTree, GWL_STYLE) | CLS_HIDEOFFLINE);
		else
			SetWindowLong(pcli->hwndContactTree, GWL_STYLE, GetWindowLong(pcli->hwndContactTree, GWL_STYLE) & ~CLS_HIDEOFFLINE);
		rebuild = 1;
	}
	if ((ModernGetSettingByte(NULL, "CList", "HideEmptyGroups", SETTING_HIDEEMPTYGROUPS_DEFAULT) == 0) != ((GetWindowLong(pcli->hwndContactTree, GWL_STYLE) & CLS_HIDEEMPTYGROUPS) == 0)) {
		if (ModernGetSettingByte(NULL, "CList", "HideEmptyGroups", SETTING_HIDEEMPTYGROUPS_DEFAULT))
			SetWindowLong(pcli->hwndContactTree, GWL_STYLE, GetWindowLong(pcli->hwndContactTree, GWL_STYLE) | CLS_HIDEEMPTYGROUPS);
		else
			SetWindowLong(pcli->hwndContactTree, GWL_STYLE, GetWindowLong(pcli->hwndContactTree, GWL_STYLE) & ~CLS_HIDEEMPTYGROUPS);
		rebuild = 1;
	}
	if ((ModernGetSettingByte(NULL, "CList", "UseGroups", SETTING_USEGROUPS_DEFAULT) == 0) != ((GetWindowLong(pcli->hwndContactTree, GWL_STYLE) & CLS_USEGROUPS) == 0)) {
		if (ModernGetSettingByte(NULL, "CList", "UseGroups", SETTING_USEGROUPS_DEFAULT))
			SetWindowLong(pcli->hwndContactTree, GWL_STYLE, GetWindowLong(pcli->hwndContactTree, GWL_STYLE) | CLS_USEGROUPS);
		else
			SetWindowLong(pcli->hwndContactTree, GWL_STYLE, GetWindowLong(pcli->hwndContactTree, GWL_STYLE) & ~CLS_USEGROUPS);
		rebuild = 1;
	}
	if (rebuild)
		SendMessage(pcli->hwndContactTree, CLM_AUTOREBUILD, 0, 0);
	return 0;
}

static int GetCaps(WPARAM wParam, LPARAM lParam)
{
	switch (wParam) {
	case CLUICAPS_FLAGS1:
		return CLUIF_HIDEEMPTYGROUPS | CLUIF_DISABLEGROUPS | CLUIF_HASONTOPOPTION | CLUIF_HASAUTOHIDEOPTION;
	}
	return 0;
}



int CLUIServices_LoadModule(void)
{
	CreateServiceFunction(MS_CLUI_METASUPPORT,MetaSupportCheck);
	CreateServiceFunction(MS_CLUI_PROTOCOLSTATUSCHANGED,CLUIServices_ProtocolStatusChanged);
	CreateServiceFunction(MS_CLUI_SORTLIST,SortList);
	CreateServiceFunction(MS_CLIST_GETSTATUSMODE,CListTray_GetGlobalStatus);

	CreateServiceFunction(MS_CLUI_GETHWND, GetHwnd);
	CreateServiceFunction(MS_CLUI_GETHWNDTREE,GetHwndTree);
	CreateServiceFunction(MS_CLUI_GROUPADDED, GroupAdded);
	CreateServiceFunction(MS_CLUI_CONTACTSETICON, ContactSetIcon);
	CreateServiceFunction(MS_CLUI_CONTACTADDED, ContactAdded);
	CreateServiceFunction(MS_CLUI_CONTACTDELETED, ContactDeleted);
	CreateServiceFunction(MS_CLUI_CONTACTRENAMED, ContactRenamed);
	CreateServiceFunction(MS_CLUI_LISTBEGINREBUILD, ListBeginRebuild);
	CreateServiceFunction(MS_CLUI_LISTENDREBUILD, ListEndRebuild);
	return 0;
}

