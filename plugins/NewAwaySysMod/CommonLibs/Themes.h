/*
	Themes.h
	Copyright (c) 2007 Chervov Dmitry

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

#pragma once

#include <windows.h>
#include <tmschema.h>

typedef HANDLE HTHEME;

// EnableThemeDialogTexture() flags
#define ETDT_DISABLE 0x00000001
#define ETDT_ENABLE 0x00000002
#define ETDT_USETABTEXTURE 0x00000004
#define ETDT_ENABLETAB (ETDT_ENABLE | ETDT_USETABTEXTURE)

typedef HANDLE (WINAPI *tOpenThemeData)(HWND, LPCWSTR);
typedef HRESULT (WINAPI *tCloseThemeData)(HANDLE);
typedef HRESULT (WINAPI *tDrawThemeBackground)(HANDLE, HDC, int, int, const RECT*, const RECT*);
typedef HRESULT (WINAPI *tDrawThemeParentBackground)(HWND, HDC, RECT*);
typedef HRESULT (WINAPI *tDrawThemeText)(HANDLE, HDC, int, int, LPCWSTR, int, DWORD, DWORD, const RECT*);
typedef HRESULT (WINAPI *tGetThemeTextExtent)(HANDLE, HDC, int, int, LPCWSTR, int, DWORD, const RECT*, RECT*);
typedef HRESULT (WINAPI *tEnableThemeDialogTexture)(HWND, DWORD);
typedef HRESULT (WINAPI *tSetWindowTheme)(HWND, LPCWSTR, LPCWSTR);

extern tOpenThemeData pOpenThemeData;
extern tCloseThemeData pCloseThemeData;
extern tDrawThemeBackground pDrawThemeBackground;
extern tDrawThemeParentBackground pDrawThemeParentBackground;
extern tDrawThemeText pDrawThemeText;
extern tGetThemeTextExtent pGetThemeTextExtent;
extern tEnableThemeDialogTexture pEnableThemeDialogTexture;
extern tSetWindowTheme pSetWindowTheme;

void InitThemes();
