/*

Facebook plugin for Miranda NG
Copyright © 2019-25 Miranda NG team

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

#include <windows.h>
#include <malloc.h>
#include <time.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>

#include <newpluginapi.h>
#include <m_avatars.h>
#include <m_chat_int.h>
#include <m_clistint.h>
#include <m_contacts.h>
#include <m_database.h>
#include <m_idle.h>
#include <m_ignore.h>
#include <m_langpack.h>
#include <m_message.h>
#include <m_netlib.h>
#include <m_options.h>
#include <m_popup.h>
#include <m_protosvc.h>
#include <m_protoint.h>
#include <m_skin.h>
#include <m_icolib.h>
#include <m_hotkeys.h>
#include <m_folders.h>
#include <m_smileyadd.h>
#include <m_toptoolbar.h>
#include <m_json.h>
#include <m_imgsrvc.h>
#include <m_http.h>
#include <m_messagestate.h>
#include <m_gui.h>

#include "../../libs/zlib/src/zlib.h"

#include "db.h"
#include "dialogs.h"
#include "mqtt.h"
#include "proto.h"
#include "resource.h"
#include "version.h"

extern bool g_bMessageState;
