/*
	Themes.cpp
	Copyright (c) 2005-2007 Chervov Dmitry

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "Common.h"
#include "Themes.h"
#include <tchar.h>

tOpenThemeData pOpenThemeData = NULL;
tCloseThemeData pCloseThemeData = NULL;
tDrawThemeBackground pDrawThemeBackground = NULL;
tDrawThemeParentBackground pDrawThemeParentBackground = NULL;
tDrawThemeText pDrawThemeText = NULL;
tGetThemeTextExtent pGetThemeTextExtent = NULL;
tEnableThemeDialogTexture pEnableThemeDialogTexture = NULL;
tSetWindowTheme pSetWindowTheme = NULL;

void InitThemes()
{
	if (IsWinVerXPPlus())
	{
		HMODULE hThemeAPI = GetModuleHandle(_T("uxtheme"));
		if (hThemeAPI)
		{
			pOpenThemeData = (tOpenThemeData)GetProcAddress(hThemeAPI, "OpenThemeData");
			pCloseThemeData = (tCloseThemeData)GetProcAddress(hThemeAPI, "CloseThemeData");
			pDrawThemeBackground = (tDrawThemeBackground)GetProcAddress(hThemeAPI, "DrawThemeBackground");
			pDrawThemeParentBackground = (tDrawThemeParentBackground)GetProcAddress(hThemeAPI, "DrawThemeParentBackground");
			pDrawThemeText = (tDrawThemeText)GetProcAddress(hThemeAPI, "DrawThemeText");
			pGetThemeTextExtent = (tGetThemeTextExtent)GetProcAddress(hThemeAPI, "GetThemeTextExtent");
			pEnableThemeDialogTexture = (tEnableThemeDialogTexture)GetProcAddress(hThemeAPI, "EnableThemeDialogTexture");
			pSetWindowTheme = (tSetWindowTheme)GetProcAddress(hThemeAPI, "SetWindowTheme");
		}
	}
}
