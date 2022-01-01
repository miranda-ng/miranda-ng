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

#pragma once

#define INCL_WINSOCK_API_TYPEDEFS 1

#ifdef _MSC_VER
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <windowsx.h>
#include <shlobj.h>
#include <commctrl.h>
#include <ShellAPI.h>
#include <vssym32.h>
#include <Uxtheme.h>
#include <Richedit.h>
#include <Wtsapi32.h>

#include <process.h>
#include <io.h>
#include <direct.h>
#endif // _WINDOWS

#include <malloc.h>
#include <stdio.h>
#include <time.h>
#include <stdarg.h>
#include <stddef.h>
#include <limits.h>
#include <string.h>
#include <locale.h>

#define __NO_CMPLUGIN_NEEDED
#include <m_system.h>
#include <m_database.h>
#include <m_db_int.h>
#include <newpluginapi.h>
#include <m_langpack.h>
#include <m_metacontacts.h>
#include <m_skin.h>
#include <m_icolib.h>
#include <m_netlib.h>
#include <m_timezones.h>
#include <m_protocols.h>
#include <m_button.h>
#include <m_gui.h>
#include <m_chat_int.h>

#include "miranda.h"

#include <m_xml.h>

#include <m_string.inl>

void GetDefaultLang();
