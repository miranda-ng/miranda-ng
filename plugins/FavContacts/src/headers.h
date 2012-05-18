/*
Favourite Contacts for Miranda IM

Copyright 2007 Victor Pavlychko

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

#define _CRT_SECURE_NO_DEPRECATE

#if defined(UNICODE) && !defined(_UNICODE)
   #define _UNICODE
#endif

#include <tchar.h>
#define _WIN32_WINNT 0x0501
#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <malloc.h>

#define MIRANDA_VER	0x0A00
#include <win2k.h>
#include <newpluginapi.h>
#include <m_system.h>
#include <m_system_cpp.h>
#include <m_database.h>
#include <m_langpack.h>
#include <m_button.h>
#include <m_clist.h>
#include <m_clc.h>
#include <m_clui.h>
#include <m_options.h>
#include <m_protosvc.h>
#include <m_utils.h>
#include <m_skin.h>
#include <m_contacts.h>
#include <m_userinfo.h>
#include <m_history.h>
#include <m_addcontact.h>
#include <m_message.h>
#include <m_file.h>
#include <m_icolib.h>
#include <m_idle.h>
#include <m_awaymsg.h>
#include <m_avatars.h>
#include <m_toolbar.h>
#include <m_fontservice.h>
#include <m_hotkeys.h>

#include "../resource.h"

#define NEWTSTR_ALLOCA(A) (A==NULL)?NULL:_tcscpy((TCHAR*)alloca(sizeof(TCHAR)*(_tcslen(A)+1)),A)

struct Options
{
	BYTE bSecondLine;
	BYTE bAvatars;
	BYTE bAvatarBorder;
	WORD wAvatarRadius;
	BYTE bNoTransparentBorder;
	BYTE bSysColors;
	BYTE bCenterHotkey;
	BYTE bUseGroups;
	BYTE bUseColumns;
	BYTE bRightAvatars;
	BYTE bDimIdle;
	WORD wMaxRecent;

	COLORREF clLine1, clLine2, clBack;
	COLORREF clLine1Sel, clLine2Sel, clBackSel;
	HFONT hfntName, hfntSecond;
};


#include "contact_cache.h"

extern Options g_Options;
extern CContactCache *g_contactCache;

#include "favlist.h"
#include "http_api.h"
