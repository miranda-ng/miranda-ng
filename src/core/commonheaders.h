/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (�) 2012-15 Miranda NG project (http://miranda-ng.org),
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

#define _ALPHA_BASE_ 1	// defined for CVS builds
#define _ALPHA_FUSE_ 1	// defined for fuse powered core

#define WINVER 0x0700
#define _WIN32_WINNT 0x0700
#define _WIN32_IE 0x0601

#define INCL_WINSOCK_API_TYPEDEFS 1

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <windowsx.h>
#include <shlobj.h>
#include <uxtheme.h>
#include <commctrl.h>
#include <vssym32.h>
#include <Shlwapi.h>
#include <Richedit.h>

#ifdef _DEBUG
#include <crtdbg.h>
#endif

#include <malloc.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <stddef.h>
#include <process.h>
#include <io.h>
#include <limits.h>
#include <string.h>
#include <locale.h>
#include <direct.h>

#include <win2k.h>

#include <m_system.h>
#include <m_system_cpp.h>
#include <m_string.h>
#include <m_core.h>
#include <newpluginapi.h>
#include <m_database.h>
#include <m_db_int.h>
#include <m_clc.h>
#include <m_clui.h>
#include <m_crypto.h>
#include <m_langpack.h>
#include <m_clist.h>
#include <m_clistint.h>
#include <m_avatars.h>
#include <m_button.h>
#include <m_protosvc.h>
#include <m_protomod.h>
#include <m_protocols.h>
#include <m_protoint.h>
#include <m_options.h>
#include <m_skin.h>
#include <m_contacts.h>
#include <m_message.h>
#include <m_userinfo.h>
#include <m_addcontact.h>
#include <m_findadd.h>
#include <m_file.h>
#include <m_awaymsg.h>
#include <m_ignore.h>
#include <m_icolib.h>
#include <m_modernopt.h>
#include <m_timezones.h>
#include <m_extraicons.h>
#include <m_xstatus.h>
#include <m_metacontacts.h>

#include "miranda.h"
#include "stdplug.h"

#include <m_ssl.h>
#include <m_netlib.h>
#include <m_xml.h>

typedef struct GlobalLogSettingsBase GlobalLogSettings;
#include <m_chat_int.h>

#include "../resource.h"