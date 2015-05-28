/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org),
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

#define _CRT_SECURE_NO_WARNINGS

#undef FASTCALL

#define TSAPI __stdcall
#define FASTCALL __fastcall

#include <windows.h>
#include <windowsx.h>
#include <shlwapi.h>
#include <Richedit.h>

#include <uxtheme.h>
#include <vssym32.h>
#include <time.h>

#include <newpluginapi.h>
#include <win2k.h>
#include <m_clistint.h>
#include <m_database.h>
#include <m_system_cpp.h>
#include <m_langpack.h>
#include <m_avatars.h>
#include <m_button.h>
#include <m_options.h>
#include <m_protosvc.h>
#include <m_skin.h>
#include <m_addcontact.h>
#include <m_timezones.h>
#include <m_cluiframes.h>
#include <m_clui.h>
#include <m_icolib.h>
#include <m_fontservice.h>
#include <m_xstatus.h>
#include <m_extraicons.h>
#include <m_string.h>

#include <m_metacontacts.h>
#include <m_cln_skinedit.h>

#include "resource.h"
#include "Version.h"
#include "extbackg.h"
#include "clc.h"
#include "config.h"
#include "clist.h"
#include "alphablend.h"
#include "rowheight_funcs.h"

// shared vars
extern HINSTANCE g_hInst;
extern LONG g_cxsmIcon, g_cysmIcon;
extern LIST<StatusItems_t> arStatusItems;
extern ImageItem *g_glyphItem;

extern CLIST_INTERFACE coreCli;

typedef  int  (__cdecl *pfnDrawAvatar)(HDC hdcOrig, HDC hdcMem, RECT *rc, ClcContact *contact, int y, struct ClcData *dat, int selected, WORD cstatus, int rowHeight);

BOOL __forceinline GetItemByStatus(int status, StatusItems_t *retitem);

void DrawAlpha(HDC hdcwnd, PRECT rc, DWORD basecolor, int alpha, DWORD basecolor2, BOOL transparent, BYTE FLG_GRADIENT, BYTE FLG_CORNER, DWORD BORDERSTYLE, ImageItem *item);

void CustomizeButton(HWND hWnd, bool bIsSkinned, bool bIsThemed, bool bIsFlat = false, bool bIsTTButton = false);
