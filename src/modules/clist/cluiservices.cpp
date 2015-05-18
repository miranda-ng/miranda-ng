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

static INT_PTR GetHwnd(WPARAM, LPARAM)
{
	return (INT_PTR)cli.hwndContactList;
}

static INT_PTR GetHwndTree(WPARAM, LPARAM)
{
	return (INT_PTR)cli.hwndContactTree;
}

static INT_PTR GroupAdded(WPARAM wParam, LPARAM lParam)
{
	//CLC does this automatically unless it's a new group
	if (lParam) {
		HANDLE hItem;
		TCHAR szFocusClass[64];
		HWND hwndFocus = GetFocus();

		GetClassName(hwndFocus, szFocusClass, SIZEOF(szFocusClass));
		if (!mir_tstrcmp(szFocusClass, _T(CLISTCONTROL_CLASS))) {
			hItem = (HANDLE) SendMessage(hwndFocus, CLM_FINDGROUP, wParam, 0);
			if (hItem)
				SendMessage(hwndFocus, CLM_EDITLABEL, (WPARAM) hItem, 0);
		}
	}
	return 0;
}

static INT_PTR ContactSetIcon(WPARAM, LPARAM)
{
	//unnecessary: CLC does this automatically
	return 0;
}

static INT_PTR ContactDeleted(WPARAM, LPARAM)
{
	//unnecessary: CLC does this automatically
	return 0;
}

static INT_PTR ContactAdded(WPARAM, LPARAM)
{
	//unnecessary: CLC does this automatically
	return 0;
}

static INT_PTR ListBeginRebuild(WPARAM, LPARAM)
{
	//unnecessary: CLC does this automatically
	return 0;
}

static INT_PTR ListEndRebuild(WPARAM, LPARAM)
{
	int rebuild = 0;
	//CLC does this automatically, but we need to force it if hideoffline or hideempty has changed
	if ((db_get_b(NULL, "CList", "HideOffline", SETTING_HIDEOFFLINE_DEFAULT) == 0) != ((GetWindowLongPtr(cli.hwndContactTree, GWL_STYLE) & CLS_HIDEOFFLINE) == 0)) {
		if (db_get_b(NULL, "CList", "HideOffline", SETTING_HIDEOFFLINE_DEFAULT))
			SetWindowLongPtr(cli.hwndContactTree, GWL_STYLE, GetWindowLongPtr(cli.hwndContactTree, GWL_STYLE) | CLS_HIDEOFFLINE);
		else
			SetWindowLongPtr(cli.hwndContactTree, GWL_STYLE, GetWindowLongPtr(cli.hwndContactTree, GWL_STYLE) & ~CLS_HIDEOFFLINE);
		rebuild = 1;
	}
	if ((db_get_b(NULL, "CList", "HideEmptyGroups", SETTING_HIDEEMPTYGROUPS_DEFAULT) == 0) != ((GetWindowLongPtr(cli.hwndContactTree, GWL_STYLE) & CLS_HIDEEMPTYGROUPS) == 0)) {
		if (db_get_b(NULL, "CList", "HideEmptyGroups", SETTING_HIDEEMPTYGROUPS_DEFAULT))
			SetWindowLongPtr(cli.hwndContactTree, GWL_STYLE, GetWindowLongPtr(cli.hwndContactTree, GWL_STYLE) | CLS_HIDEEMPTYGROUPS);
		else
			SetWindowLongPtr(cli.hwndContactTree, GWL_STYLE, GetWindowLongPtr(cli.hwndContactTree, GWL_STYLE) & ~CLS_HIDEEMPTYGROUPS);
		rebuild = 1;
	}
	if ((db_get_b(NULL, "CList", "UseGroups", SETTING_USEGROUPS_DEFAULT) == 0) != ((GetWindowLongPtr(cli.hwndContactTree, GWL_STYLE) & CLS_USEGROUPS) == 0)) {
		if (db_get_b(NULL, "CList", "UseGroups", SETTING_USEGROUPS_DEFAULT))
			SetWindowLongPtr(cli.hwndContactTree, GWL_STYLE, GetWindowLongPtr(cli.hwndContactTree, GWL_STYLE) | CLS_USEGROUPS);
		else
			SetWindowLongPtr(cli.hwndContactTree, GWL_STYLE, GetWindowLongPtr(cli.hwndContactTree, GWL_STYLE) & ~CLS_USEGROUPS);
		rebuild = 1;
	}
	if (rebuild)
		cli.pfnInitAutoRebuild(cli.hwndContactTree);
	return 0;
}

static INT_PTR ContactRenamed(WPARAM, LPARAM)
{
	//unnecessary: CLC does this automatically
	return 0;
}

static INT_PTR GetCaps(WPARAM wParam, LPARAM)
{
	switch (wParam) {
	case CLUICAPS_FLAGS1:
		return CLUIF_HIDEEMPTYGROUPS | CLUIF_DISABLEGROUPS | CLUIF_HASONTOPOPTION | CLUIF_HASAUTOHIDEOPTION;
	case CLUICAPS_FLAGS2:
		return MAKELONG(EXTRA_ICON_COUNT,1);
	}
	return 0;
}

void LoadCluiServices(void)
{
	CreateServiceFunction(MS_CLUI_GETHWND, GetHwnd);
	CreateServiceFunction(MS_CLUI_GETHWNDTREE, GetHwndTree);
	CreateServiceFunction(MS_CLUI_GROUPADDED, GroupAdded);
	CreateServiceFunction(MS_CLUI_CONTACTSETICON, ContactSetIcon);
	CreateServiceFunction(MS_CLUI_CONTACTADDED, ContactAdded);
	CreateServiceFunction(MS_CLUI_CONTACTDELETED, ContactDeleted);
	CreateServiceFunction(MS_CLUI_CONTACTRENAMED, ContactRenamed);
	CreateServiceFunction(MS_CLUI_LISTBEGINREBUILD, ListBeginRebuild);
	CreateServiceFunction(MS_CLUI_LISTENDREBUILD, ListEndRebuild);
	CreateServiceFunction(MS_CLUI_GETCAPS, GetCaps);
}

/////////////////////////////////////////////////////////////////////////////////////////
// default protocol status notification handler

void fnCluiProtocolStatusChanged(int, const char*)
{
	int i, *partWidths;
	int borders[3];
	int flags = 0;

	if (cli.menuProtoCount == 0) {
		SendMessage(cli.hwndStatus, SB_SETPARTS, 0, 0);
		SendMessage(cli.hwndStatus, SB_SETTEXT, SBT_OWNERDRAW, 0);
		return;
	}

	SendMessage(cli.hwndStatus, SB_GETBORDERS, 0, (LPARAM)&borders);

	partWidths = (int*)alloca(cli.menuProtoCount * sizeof(int));
	if (db_get_b(NULL, "CLUI", "EqualSections", 0)) {
		RECT rc;
		GetClientRect(cli.hwndStatus, &rc);
		rc.right -= borders[0] * 2 + (db_get_b(NULL, "CLUI", "ShowGrip", 1) ? GetSystemMetrics(SM_CXVSCROLL) : 0);
		for (i=0; i < cli.menuProtoCount; i++)
			partWidths[ i ] = (i+1) * rc.right / cli.menuProtoCount - (borders[2] >> 1);
	}
	else {
		HDC hdc;
		HFONT hFont;
		SIZE textSize;
		BYTE showOpts = db_get_b(NULL, "CLUI", "SBarShow", 1);

		hdc = GetDC(NULL);
		hFont = (HFONT)SelectObject(hdc, (HFONT) SendMessage(cli.hwndStatus, WM_GETFONT, 0, 0));
		for (i=0; i < cli.menuProtoCount; i++) {  //count down since built in ones tend to go at the end
			int x = 2;
			if (showOpts & 1)
				x += g_IconWidth;
			if (showOpts & 2) {
				TCHAR tszName[64];
				PROTOACCOUNT *pa = Proto_GetAccount(cli.menuProtos[i].szProto);
				if (pa)
					mir_sntprintf(tszName, SIZEOF(tszName), _T("%s "), pa->tszAccountName);
				else
					tszName[0] = 0;

				if (showOpts & 4 && mir_tstrlen(tszName) < SIZEOF(tszName)-1)
					mir_tstrcat(tszName, _T(" "));
				GetTextExtentPoint32(hdc, tszName, (int)mir_tstrlen(tszName), &textSize);
				x += textSize.cx;
				x += GetSystemMetrics(SM_CXBORDER) * 4; // The SB panel doesnt allocate enough room
			}
			if (showOpts & 4) {
				TCHAR* modeDescr = cli.pfnGetStatusModeDescription(CallProtoServiceInt(NULL,cli.menuProtos[i].szProto, PS_GETSTATUS, 0, 0), 0);
				GetTextExtentPoint32(hdc, modeDescr, (int)mir_tstrlen(modeDescr), &textSize);
				x += textSize.cx;
				x += GetSystemMetrics(SM_CXBORDER) * 4; // The SB panel doesnt allocate enough room
			}
			partWidths[ i ] = (i ? partWidths[ i-1] : 0) + x + 2;
		}
		SelectObject(hdc, hFont);
		ReleaseDC(NULL, hdc);
	}

	partWidths[ cli.menuProtoCount-1 ] = -1;
	SendMessage(cli.hwndStatus, SB_SETMINHEIGHT, g_IconHeight, 0);
	SendMessage(cli.hwndStatus, SB_SETPARTS, cli.menuProtoCount, (LPARAM)partWidths);
	flags = SBT_OWNERDRAW;
	if (db_get_b(NULL, "CLUI", "SBarBevel", 1) == 0)
		flags |= SBT_NOBORDERS;
	for (i=0; i < cli.menuProtoCount; i++) {
		SendMessage(cli.hwndStatus, SB_SETTEXT, i | flags, (LPARAM)cli.menuProtos[i].szProto);
	}
}
