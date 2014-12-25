/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-14 Miranda NG project (http://miranda-ng.org),
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

#define WINVER 0x0700
#define _WIN32_WINNT 0x0700
#define _WIN32_IE 0x0601

#include <tchar.h>
#include <winsock2.h>
#include <shlobj.h>
#include <commctrl.h>
#include <vssym32.h>

#include <stdio.h>
#include <time.h>
#include <stddef.h>
#include <process.h>
#include <io.h>
#include <limits.h>
#include <string.h>
#include <locale.h>
#include <direct.h>
#include <malloc.h>

#include <win2k.h>

#include <m_system.h>
#include <m_system_cpp.h>
#include <m_core.h>
#include <newpluginapi.h>
#include <m_database.h>
#include <m_netlib.h>
#include <m_langpack.h>
#include <m_clist.h>
#include <m_clistint.h>
#include <m_button.h>
#include <m_protosvc.h>
#include <m_protomod.h>
#include <m_protocols.h>
#include <m_options.h>
#include <m_skin.h>
#include <m_contacts.h>
#include <m_message.h>
#include <m_userinfo.h>
#include <m_history.h>
#include <m_findadd.h>
#include <m_icolib.h>
#include <m_modernopt.h>
#include <m_timezones.h>

#include "version.h"

#include "../../resource.h"
#include "../stdplug.h"

extern HINSTANCE hInst;

#pragma comment(lib, "version.lib")
