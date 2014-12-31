/*
Copyright (c) 2013-15 Miranda NG project (http://miranda-ng.org)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#ifndef _WIN32_WINNT            // Specifies that the minimum required platform is Windows Vista.
#define _WIN32_WINNT 0x0600     // Change this to the appropriate value to target other versions of Windows.
#endif

#include <Windows.h>
#include <Shlwapi.h>
#include <Wincrypt.h>

#include <stdio.h>
#include <malloc.h>
#include <tchar.h>
#include <time.h>

#include <newpluginapi.h>
#include <m_system.h>
#include <m_system_cpp.h>

#include <m_avatars.h>
#include <m_chat.h>
#include <m_clistint.h>
#include <m_database.h>
#include <m_extraicons.h>
#include <m_file.h>
#include <m_fontservice.h>
#include <m_genmenu.h>
#include <m_hotkeys.h>
#include <m_icolib.h>
#include <m_idle.h>
#include <m_imgsrvc.h>
#include <m_json.h>
#include <m_langpack.h>
#include <m_message.h>
#include <m_netlib.h>
#include <m_options.h>
#include <m_protomod.h>
#include <m_protosvc.h>
#include <m_protoint.h>
#include <m_skin.h>
#include <m_string.h>
#include <m_timezones.h>
#include <m_toptoolbar.h>
#include <m_userinfo.h>
#include <m_utils.h>
#include <m_proto_listeningto.h>

#include <m_popup.h>
#include <m_folders.h>

#include "win2k.h"

#include "resource.h"
#include "vk.h"
#include "vk_proto.h"
