/*

MUCC Group Chat GUI Plugin for Miranda IM
Copyright (C) 2004  Piotr Piastucki

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
#ifndef MUCC_INCLUDED
#define MUCC_INCLUDED

//#define ENABLE_LOGGING

#define _WIN32_WINNT 0x0501
#define _WIN32_IE 0x0500

#include <windows.h>
#include <commctrl.h>
#include <richedit.h>
#include <process.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <limits.h>

#ifdef __MINGW32__
#define EM_SETTEXTEX	(WM_USER + 97)
#define ST_DEFAULT		0
#define ST_KEEPUNDO		1
#define ST_SELECTION	2
#define ST_NEWCHARS		4
#else
#include <win2k.h>
#endif

#define MIRANDA_VER 0x0700

#include <newpluginapi.h>
#include <m_system.h>
#include <m_protomod.h>
#include <m_protosvc.h>
#include <m_clist.h>
#include <m_clui.h>
#include <m_options.h>
#include <m_userinfo.h>
#include <m_database.h>
#include <m_langpack.h>
#include <m_utils.h>
#include <m_message.h>
#include <m_skin.h>
#include <m_popup.h>
#include <m_button.h>

#include "resource.h"

enum {
	MUCC_IDI_CHAT = 0,
	MUCC_IDI_U_GLOBALOWNER,
	MUCC_IDI_U_OWNER,
	MUCC_IDI_U_ADMIN,
	MUCC_IDI_U_REGISTERED,
	MUCC_IDI_R_MODERATED,
	MUCC_IDI_R_MEMBERS,
	MUCC_IDI_R_ANONYMOUS,
	MUCC_IDI_PREV,
	MUCC_IDI_NEXT,
	MUCC_IDI_SEARCH,
	MUCC_IDI_BOLD,
	MUCC_IDI_ITALIC,
	MUCC_IDI_UNDERLINE,
	MUCC_IDI_OPTIONS,
	MUCC_IDI_INVITE,
	MUCC_IDI_ADMINISTRATION,
	MUCC_IDI_SMILEY,
	MUCC_IDI_MESSAGE,
	MUCC_ICON_TOTAL

};

extern PLUGINLINK * pluginLink;
extern char *muccModuleName;
extern HINSTANCE hInst;
extern HANDLE hHookEvent;
extern HIMAGELIST hImageList;
extern HICON muccIcon[MUCC_ICON_TOTAL];

#include "m_mucc.h"

#define WM_MUCC_REFRESH_ROOMS		(WM_USER+101)
#define WM_MUCC_REFRESH_USER_ROOMS  (WM_USER+102)
#define WM_MUCC_REFRESH_USER_NICKS  (WM_USER+103)

#endif

