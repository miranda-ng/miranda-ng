/*
Copyright (C) 2026 Miranda NG team

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

#include <windows.h>
#include <io.h>
#include <sys/stat.h>
#include <time.h>

#include <algorithm>
#include <map>
#include <set>
#include <string>
#include <vector>

#include <newpluginapi.h>
#include <m_system.h>
#include <m_avatars.h>
#include <m_chat_int.h>
#include <m_clist.h>
#include <m_contacts.h>
#include <m_database.h>
#include <m_gui.h>
#include <m_json.h>
#include <m_langpack.h>
#include <m_netlib.h>
#include <m_options.h>
#include <m_protocols.h>
#include <m_protosvc.h>
#include <m_protoint.h>
#include <m_skin.h>
#include <m_utils.h>

#include "resource.h"
#include "version.h"
#include "ok_credentials.h"
#include "proto.h"

// the message fields getMessages must ask for: without them the page comes back empty
#define OK_MESSAGE_FIELDS "message.attachments,message.ID,message.TYPE,message.AUTHOR_ID,message.TIME,message.TIME_MS,message.TEXT,message.DATE,message.DATE_MS,message.REPLY_TO_MSG_ID"

typedef CProtoDlgBase<COdnoklassnikiProto> COkDlgBase;
