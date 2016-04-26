/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-16 Miranda NG project (http://miranda-ng.org),
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

EXTERN_C MIR_APP_DLL(void) Clist_GroupAdded(MGROUP hGroup)
{
	// CLC does this automatically unless it's a new group
	HWND hwndFocus = GetFocus();

	TCHAR szFocusClass[64];
	GetClassName(hwndFocus, szFocusClass, _countof(szFocusClass));
	if (!mir_tstrcmp(szFocusClass, _T(CLISTCONTROL_CLASS))) {
		HANDLE hItem = (HANDLE)SendMessage(hwndFocus, CLM_FINDGROUP, hGroup, 0);
		if (hItem)
			SendMessage(hwndFocus, CLM_EDITLABEL, (WPARAM)hItem, 0);
	}
}

EXTERN_C MIR_APP_DLL(void) Clist_EndRebuild(void)
{
	bool bRebuild = false;
	LONG_PTR dwStyle = GetWindowLongPtr(cli.hwndContactTree, GWL_STYLE);

	// CLC does this automatically, but we need to force it if hideoffline or hideempty has changed
	if ((db_get_b(NULL, "CList", "HideOffline", SETTING_HIDEOFFLINE_DEFAULT) == 0) != ((dwStyle & CLS_HIDEOFFLINE) == 0)) {
		if (db_get_b(NULL, "CList", "HideOffline", SETTING_HIDEOFFLINE_DEFAULT))
			dwStyle |= CLS_HIDEOFFLINE;
		else
			dwStyle &= ~CLS_HIDEOFFLINE;
		bRebuild = true;
	}

	if ((db_get_b(NULL, "CList", "HideEmptyGroups", SETTING_HIDEEMPTYGROUPS_DEFAULT) == 0) != ((dwStyle & CLS_HIDEEMPTYGROUPS) == 0)) {
		if (db_get_b(NULL, "CList", "HideEmptyGroups", SETTING_HIDEEMPTYGROUPS_DEFAULT))
			dwStyle |= CLS_HIDEEMPTYGROUPS;
		else
			dwStyle &= ~CLS_HIDEEMPTYGROUPS;
		bRebuild = true;
	}

	if ((db_get_b(NULL, "CList", "UseGroups", SETTING_USEGROUPS_DEFAULT) == 0) != ((dwStyle & CLS_USEGROUPS) == 0)) {
		if (db_get_b(NULL, "CList", "UseGroups", SETTING_USEGROUPS_DEFAULT))
			dwStyle |= CLS_USEGROUPS;
		else
			dwStyle &= ~CLS_USEGROUPS;
		bRebuild = true;
	}

	if (bRebuild) {
		SetWindowLongPtr(cli.hwndContactTree, GWL_STYLE, dwStyle);
		cli.pfnInitAutoRebuild(cli.hwndContactTree);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// default protocol status notification handler

void fnCluiProtocolStatusChanged(int, const char*)
{
	if (cli.menuProtoCount == 0) {
		SendMessage(cli.hwndStatus, SB_SETPARTS, 0, 0);
		SendMessage(cli.hwndStatus, SB_SETTEXT, SBT_OWNERDRAW, 0);
		return;
	}

	int borders[3];
	SendMessage(cli.hwndStatus, SB_GETBORDERS, 0, (LPARAM)&borders);

	int *partWidths = (int*)alloca(cli.menuProtoCount * sizeof(int));
	if (db_get_b(NULL, "CLUI", "EqualSections", 0)) {
		RECT rc;
		GetClientRect(cli.hwndStatus, &rc);
		rc.right -= borders[0] * 2 + (db_get_b(NULL, "CLUI", "ShowGrip", 1) ? GetSystemMetrics(SM_CXVSCROLL) : 0);
		for (int i = 0; i < cli.menuProtoCount; i++)
			partWidths[i] = (i + 1) * rc.right / cli.menuProtoCount - (borders[2] >> 1);
	}
	else {
		SIZE textSize;
		BYTE showOpts = db_get_b(NULL, "CLUI", "SBarShow", 1);

		HDC hdc = GetDC(NULL);
		HFONT hFont = (HFONT)SelectObject(hdc, (HFONT)SendMessage(cli.hwndStatus, WM_GETFONT, 0, 0));
		for (int i = 0; i < cli.menuProtoCount; i++) {  //count down since built in ones tend to go at the end
			int x = 2;
			if (showOpts & 1)
				x += g_IconWidth;
			if (showOpts & 2) {
				TCHAR tszName[64];
				PROTOACCOUNT *pa = Proto_GetAccount(cli.menuProtos[i].szProto);
				if (pa)
					mir_sntprintf(tszName, _T("%s "), pa->tszAccountName);
				else
					tszName[0] = 0;

				if (showOpts & 4 && mir_tstrlen(tszName) < _countof(tszName) - 1)
					mir_tstrcat(tszName, _T(" "));
				GetTextExtentPoint32(hdc, tszName, (int)mir_tstrlen(tszName), &textSize);
				x += textSize.cx;
				x += GetSystemMetrics(SM_CXBORDER) * 4; // The SB panel doesnt allocate enough room
			}
			if (showOpts & 4) {
				TCHAR* modeDescr = cli.pfnGetStatusModeDescription(CallProtoServiceInt(NULL, cli.menuProtos[i].szProto, PS_GETSTATUS, 0, 0), 0);
				GetTextExtentPoint32(hdc, modeDescr, (int)mir_tstrlen(modeDescr), &textSize);
				x += textSize.cx;
				x += GetSystemMetrics(SM_CXBORDER) * 4; // The SB panel doesnt allocate enough room
			}
			partWidths[i] = (i ? partWidths[i - 1] : 0) + x + 2;
		}
		SelectObject(hdc, hFont);
		ReleaseDC(NULL, hdc);
	}

	partWidths[cli.menuProtoCount - 1] = -1;
	SendMessage(cli.hwndStatus, SB_SETMINHEIGHT, g_IconHeight, 0);
	SendMessage(cli.hwndStatus, SB_SETPARTS, cli.menuProtoCount, (LPARAM)partWidths);
	
	int flags = SBT_OWNERDRAW;
	if (db_get_b(NULL, "CLUI", "SBarBevel", 1) == 0)
		flags |= SBT_NOBORDERS;
	
	for (int i = 0; i < cli.menuProtoCount; i++)
		SendMessage(cli.hwndStatus, SB_SETTEXT, i | flags, (LPARAM)cli.menuProtos[i].szProto);
}
