/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2008 Miranda ICQ/IM project,
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


#define _WIN32_WINNT 0x0501

#pragma comment( lib, "comctl32.lib")

#include <windows.h>
#include <stdio.h>
#include <time.h>
#include <stddef.h>
#include <process.h>
#include <commctrl.h>
#include <Richedit.h>
#include <malloc.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>

#define MIRANDA_VER 0x0800

#include <newpluginapi.h>
#include <win2k.h>
#include <m_system.h>
#include <m_utils.h>
#include <m_netlib.h>
#include <m_database.h>
#include <m_options.h>
#include <m_langpack.h>
#include <m_clui.h>
#include <m_clist.h>
#include <m_langpack.h>
#include <m_button.h>
#include <m_fontservice.h>
#include <m_hotkeys.h>
#include <m_icolib.h>
#include "m_toolbar.h"
#include "m_toptoolbar.h"
#include "resource.h"

#include "version.h"

HINSTANCE hInst;

void InitConsole();
void ShutdownConsole();

#define MS_NETLIB_LOGWIN "Netlib/Log/Win"
