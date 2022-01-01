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

EXTERN_C MIR_APP_DLL(void) Clist_GroupAdded(MGROUP hGroup)
{
	// CLC does this automatically unless it's a new group
	HWND hwndFocus = GetFocus();

	wchar_t szFocusClass[64];
	GetClassName(hwndFocus, szFocusClass, _countof(szFocusClass));
	if (!mir_wstrcmp(szFocusClass, CLISTCONTROL_CLASSW)) {
		HANDLE hItem = (HANDLE)SendMessage(hwndFocus, CLM_FINDGROUP, hGroup, 0);
		if (hItem)
			SendMessage(hwndFocus, CLM_EDITLABEL, (WPARAM)hItem, 0);
	}
}

EXTERN_C MIR_APP_DLL(void) Clist_EndRebuild(void)
{
	if (g_clistApi.hwndContactTree == nullptr)
		return;

	bool bRebuild = false;
	LONG_PTR dwStyle = GetWindowLongPtr(g_clistApi.hwndContactTree, GWL_STYLE);

	// CLC does this automatically, but we need to force it if hideoffline or hideempty has changed
	if ((Clist::HideOffline == 0) != ((dwStyle & CLS_HIDEOFFLINE) == 0)) {
		if (Clist::HideOffline)
			dwStyle |= CLS_HIDEOFFLINE;
		else
			dwStyle &= ~CLS_HIDEOFFLINE;
		bRebuild = true;
	}

	if ((Clist::HideEmptyGroups == 0) != ((dwStyle & CLS_HIDEEMPTYGROUPS) == 0)) {
		if (Clist::HideEmptyGroups)
			dwStyle |= CLS_HIDEEMPTYGROUPS;
		else
			dwStyle &= ~CLS_HIDEEMPTYGROUPS;
		bRebuild = true;
	}

	if ((Clist::UseGroups == 0) != ((dwStyle & CLS_USEGROUPS) == 0)) {
		if (Clist::UseGroups)
			dwStyle |= CLS_USEGROUPS;
		else
			dwStyle &= ~CLS_USEGROUPS;
		bRebuild = true;
	}

	if (bRebuild) {
		SetWindowLongPtr(g_clistApi.hwndContactTree, GWL_STYLE, dwStyle);
		Clist_InitAutoRebuild(g_clistApi.hwndContactTree);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// default protocol status notification handler

void fnCluiProtocolStatusChanged(int, const char*)
{
	if (g_menuProtos.getCount() == 0) {
		SendMessage(g_clistApi.hwndStatus, SB_SETPARTS, 0, 0);
		SendMessage(g_clistApi.hwndStatus, SB_SETTEXT, SBT_OWNERDRAW, 0);
		return;
	}

	int borders[3];
	SendMessage(g_clistApi.hwndStatus, SB_GETBORDERS, 0, (LPARAM)&borders);

	int *partWidths = (int*)alloca(g_menuProtos.getCount() * sizeof(int));
	if (db_get_b(0, "CLUI", "EqualSections", 0)) {
		RECT rc;
		GetClientRect(g_clistApi.hwndStatus, &rc);
		rc.right -= borders[0] * 2 + (db_get_b(0, "CLUI", "ShowGrip", 1) ? GetSystemMetrics(SM_CXVSCROLL) : 0);
		for (int i = 0; i < g_menuProtos.getCount(); i++)
			partWidths[i] = (i + 1) * rc.right / g_menuProtos.getCount() - (borders[2] >> 1);
	}
	else {
		SIZE textSize;
		uint8_t showOpts = db_get_b(0, "CLUI", "SBarShow", 1);

		HDC hdc = GetDC(nullptr);
		HFONT hFont = (HFONT)SelectObject(hdc, (HFONT)SendMessage(g_clistApi.hwndStatus, WM_GETFONT, 0, 0));
		for (int i = 0; i < g_menuProtos.getCount(); i++) {  //count down since built in ones tend to go at the end
			int x = 2;
			if (showOpts & 1)
				x += g_IconWidth;
			if (showOpts & 2) {
				wchar_t tszName[64];
				PROTOACCOUNT *pa = Proto_GetAccount(g_menuProtos[i].szProto);
				if (pa)
					mir_snwprintf(tszName, L"%s ", pa->tszAccountName);
				else
					tszName[0] = 0;

				if (showOpts & 4 && mir_wstrlen(tszName) < _countof(tszName) - 1)
					mir_wstrcat(tszName, L" ");
				GetTextExtentPoint32(hdc, tszName, (int)mir_wstrlen(tszName), &textSize);
				x += textSize.cx;
				x += GetSystemMetrics(SM_CXBORDER) * 4; // The SB panel doesnt allocate enough room
			}
			if (showOpts & 4) {
				wchar_t* modeDescr = Clist_GetStatusModeDescription(Proto_GetStatus(g_menuProtos[i].szProto), 0);
				GetTextExtentPoint32(hdc, modeDescr, (int)mir_wstrlen(modeDescr), &textSize);
				x += textSize.cx;
				x += GetSystemMetrics(SM_CXBORDER) * 4; // The SB panel doesnt allocate enough room
			}
			partWidths[i] = (i ? partWidths[i - 1] : 0) + x + 2;
		}
		SelectObject(hdc, hFont);
		ReleaseDC(nullptr, hdc);
	}

	partWidths[g_menuProtos.getCount()-1] = -1;
	SendMessage(g_clistApi.hwndStatus, SB_SETMINHEIGHT, g_IconHeight, 0);
	SendMessage(g_clistApi.hwndStatus, SB_SETPARTS, g_menuProtos.getCount(), (LPARAM)partWidths);
	
	int flags = SBT_OWNERDRAW;
	if (db_get_b(0, "CLUI", "SBarBevel", 1) == 0)
		flags |= SBT_NOBORDERS;
	
	for (int i = 0; i < g_menuProtos.getCount(); i++)
		SendMessage(g_clistApi.hwndStatus, SB_SETTEXT, i | flags, (LPARAM)g_menuProtos[i].szProto);
}
