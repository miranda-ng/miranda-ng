/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org),
Copyright (c) 2000-08 Miranda ICQ/IM project,
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
#define _WIN32_WINNT 0x0501

#include <windows.h>
#include <commctrl.h>
#include <malloc.h>

#include <newpluginapi.h>
#include <win2k.h>
#include <m_netlib.h>
#include <m_database.h>
#include <m_options.h>
#include <m_langpack.h>
#include <m_clist.h>
#include <m_button.h>
#include <m_fontservice.h>
#include <m_hotkeys.h>

#include <m_toptoolbar.h>

#include "resource.h"
#include "version.h"

extern HINSTANCE hInst;

void InitConsole();
void ShutdownConsole();
HANDLE LoadIcon(int iIconID);

#define MS_NETLIB_LOGWIN "Netlib/Log/Win"
