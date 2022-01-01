/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org),
Copyright (c) 2000-03 Miranda ICQ/IM project,
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

uint32_t GetCLUIWindowStyle(uint8_t style)
{
	uint32_t dwBasic = WS_CLIPCHILDREN;

	if (style == SETTING_WINDOWSTYLE_THINBORDER)
		return dwBasic | WS_BORDER;
	else if (style == SETTING_WINDOWSTYLE_TOOLWINDOW || style == 0)
		return dwBasic | (WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_POPUPWINDOW | WS_THICKFRAME);
	else if (style == SETTING_WINDOWSTYLE_NOBORDER)
		return dwBasic;

	return dwBasic;
}

void ApplyCLUIBorderStyle()
{
	uint8_t windowStyle = db_get_b(0, "CLUI", "WindowStyle", SETTING_WINDOWSTYLE_TOOLWINDOW);
	WINDOWPLACEMENT p;
	bool minToTray = TRUE;

	p.length = sizeof(p);
	GetWindowPlacement(g_clistApi.hwndContactList, &p);
	ShowWindow(g_clistApi.hwndContactList, SW_HIDE);

	if (windowStyle == SETTING_WINDOWSTYLE_DEFAULT || windowStyle == SETTING_WINDOWSTYLE_TOOLWINDOW) {
		SetWindowLongPtr(g_clistApi.hwndContactList, GWL_STYLE, GetWindowLongPtr(g_clistApi.hwndContactList, GWL_STYLE) | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_POPUPWINDOW | WS_THICKFRAME);
		if (SETTING_WINDOWSTYLE_DEFAULT == windowStyle) {
			SetWindowLongPtr(g_clistApi.hwndContactList, GWL_STYLE, GetWindowLongPtr(g_clistApi.hwndContactList, GWL_STYLE) & ~(WS_MAXIMIZEBOX/* | WS_MINIMIZEBOX*/));
			minToTray = FALSE;
		}
	}
	else if (windowStyle == SETTING_WINDOWSTYLE_THINBORDER) {
		SetWindowLongPtr(g_clistApi.hwndContactList, GWL_STYLE, GetWindowLongPtr(g_clistApi.hwndContactList, GWL_STYLE) & ~(WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_POPUPWINDOW | WS_THICKFRAME));
		SetWindowLongPtr(g_clistApi.hwndContactList, GWL_STYLE, GetWindowLongPtr(g_clistApi.hwndContactList, GWL_STYLE) | WS_BORDER | WS_CLIPCHILDREN);
	}
	else {
		SetWindowLongPtr(g_clistApi.hwndContactList, GWL_STYLE, GetWindowLongPtr(g_clistApi.hwndContactList, GWL_STYLE) & ~(WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_POPUPWINDOW | WS_THICKFRAME));
		SetWindowLongPtr(g_clistApi.hwndContactList, GWL_STYLE, GetWindowLongPtr(g_clistApi.hwndContactList, GWL_STYLE) | WS_CLIPCHILDREN);
	}
	p.showCmd = SW_HIDE;
	SetWindowPlacement(g_clistApi.hwndContactList, &p);

	g_plugin.setByte("Min2Tray", minToTray);
}
