/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-25 Miranda NG team (https://miranda-ng.org),
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

#define HSSL_DEFINED 1
typedef struct SslHandle *HSSL;

#ifdef _WINDOWS
	#include <winsock2.h>
	#include <ws2tcpip.h>
	#include <windows.h>
	#include <windowsx.h>
	#include <ShlObj.h>
	#include <uxtheme.h>
	#include <vssym32.h>
	#include <Shlwapi.h>
	#include <Richedit.h>
	
	#include <io.h>
	#include <fcntl.h>
	#include <direct.h>
	#include <process.h>
#else
	#include <pthread.h>
#endif

#include <assert.h>
#include <malloc.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <stddef.h>
#include <limits.h>
#include <string.h>
#include <locale.h>

#include <map>
#include <memory>

#define __NO_CMPLUGIN_NEEDED
#include <newpluginapi.h>
#include <m_avatars.h>
#include <m_awaymsg.h>
#include <m_button.h>
#include <m_clc.h>
#include <m_clistint.h>
#include <m_cluiframes.h>
#include <m_contacts.h>
#include <m_crypto.h>
#include <m_db_int.h>
#include <m_extraicons.h>
#include <m_file.h>
#include <m_findadd.h>
#include <m_folders.h>
#include <m_fontservice.h>
#include <m_gui.h>
#include <m_history.h>
#include <m_hotkeys.h>
#include <m_hpp.h>
#include <m_icolib.h>
#include <m_idle.h>
#include <m_ieview.h>
#include <m_ignore.h>
#include <m_imgsrvc.h>
#include <m_json.h>
#include <m_langpack.h>
#include <m_metacontacts.h>
#include <m_netlib.h>
#include <m_options.h>
#include <m_message.h>
#include <m_popup_int.h>
#include <m_protosvc.h>
#include <m_protocols.h>
#include <m_protoint.h>
#include <m_skin.h>
#include <m_srmm_int.h>
#include <m_timezones.h>
#include <m_tipper.h>
#include <m_toptoolbar.h>
#include <m_userinfo.h>
#include <m_version.h>
#include <m_xstatus.h>

#include "miranda.h"

typedef struct GlobalLogSettingsBase GlobalLogSettings;
#include <m_chat_int.h>

#include "copyright.h"
#include "resource.h"

struct CMPlugin : public PLUGIN<CMPlugin>
{
	CMPlugin();
};
