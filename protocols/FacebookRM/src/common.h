/*

Facebook plugin for Miranda Instant Messenger
_____________________________________________

Copyright © 2009-11 Michal Zelinka, 2011-12 Robert Pösel

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#pragma once

//#pragma warning(push)
//#pragma warning(disable:4312)
#pragma warning(disable:4996)

#define MIRANDA_VER    0x0A00
#define _WIN32_WINNT   0x0500
#define _WIN32_WINDOWS 0x0500

#include <m_stdhdr.h>

#include <string>
#include <sstream>
#include <fstream>
#include <list>
#include <map>
#include <vector>
#include <algorithm>

#include <stdarg.h>
#include <time.h>
#include <assert.h>
#include <io.h>

#include <windows.h>
#include <win2k.h>
#include <commctrl.h>

#include <newpluginapi.h>
#include <m_system.h>
#include <m_system_cpp.h>
#include <m_avatars.h>
#include <m_button.h>
#include <m_chat.h>
#include <m_clc.h>
#include <m_clist.h>
#include <m_clistint.h>
#include <m_clui.h>
#include <m_database.h>
#include <m_history.h>
#include <m_idle.h>
#include <m_ignore.h>
#include <m_langpack.h>
#include <m_message.h>
#include <m_netlib.h>
#include <m_options.h>
#include <m_popup.h>
#include <m_protocols.h>
#include <m_protosvc.h>
#include <m_protoint.h>
#include <m_protomod.h>
#include <m_skin.h>
#include <statusmodes.h>
#include <m_userinfo.h>
#include <m_addcontact.h>
#include <m_icolib.h>
#include <m_utils.h>
#include <m_hotkeys.h>
#include <m_folders.h>
#include <m_smileyadd.h>

#include "version.h"

class FacebookProto;

#include "definitions.h"
#include "entities.h"
#include "avatars.h"
#include "http.h"
#include "list.hpp"
#include "utils.h"
#include "client.h"
#include "proto.h"
#include "json.h"
#include "db.h"
#include "constants.h"
#include "dialogs.h"
#include "theme.h"
#include "resource.h"

extern HINSTANCE g_hInstance;
extern std::string g_strUserAgent;
extern DWORD g_mirandaVersion;