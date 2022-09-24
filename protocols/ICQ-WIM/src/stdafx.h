// ---------------------------------------------------------------------------80
//                ICQ plugin for Miranda Instant Messenger
//                ________________________________________
//
// Copyright © 2000-2001 Richard Hughes, Roland Rabien, Tristan Van de Vreede
// Copyright © 2001-2002 Jon Keating, Richard Hughes
// Copyright © 2002-2004 Martin Öberg, Sam Kothari, Robert Rainwater
// Copyright © 2004-2010 Joe Kucera
// Copyright © 2012-2022 Miranda NG team
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
// -----------------------------------------------------------------------------
//  DESCRIPTION:
//
// Includes all header files that should be precompiled to speed up compilation.
// -----------------------------------------------------------------------------

#pragma once

// Windows includes
#include <windows.h>

// Standard includes
#include <stdio.h>
#include <time.h>
#include <io.h>
#include <malloc.h>
#include <direct.h>
#include <fcntl.h>
#include <process.h>

// Miranda IM SDK includes
#include <newpluginapi.h> // This must be included first
#include <m_avatars.h>
#include <m_chat_int.h>
#include <m_clistint.h>
#include <m_contacts.h>
#include <m_database.h>
#include <m_gui.h>
#include <m_idle.h>
#include <m_icolib.h>
#include <m_ignore.h>
#include <m_json.h>
#include <m_langpack.h>
#include <m_message.h>
#include <m_messagestate.h>
#include <m_netlib.h>
#include <m_protocols.h>
#include <m_protosvc.h>
#include <m_options.h>
#include <m_popup.h>
#include <m_skin.h>
#include <m_smileyadd.h>
#include <m_system.h>
#include <m_timezones.h>
#include <m_userinfo.h>
#include <m_utils.h>

#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/rand.h>
#include <openssl/sha.h>

// Project resources
#include "resource.h"

// ICQ plugin includes
#include "version.h"

#define MODULENAME "ICQ"

#define DB_KEY_ID "aimId"
#define DB_KEY_IDLE "IdleTS"
#define DB_KEY_ABOUT "About"
#define DB_KEY_PHONE "Phone"
#define DB_KEY_ATOKEN "AToken"
#define DB_KEY_ICQNICK "IcqNick"
#define DB_KEY_PHONEREG "PhoneReg"
#define DB_KEY_LASTSEEN "LastSeen"
#define DB_KEY_ONLINETS "OnlineTS"
#define DB_KEY_PATCHVER "PatchVersion"
#define DB_KEY_RCLIENTID "RClientID"
#define DB_KEY_LASTMSGID "LastMsgId"
#define DB_KEY_REMOTEREAD "RemoteReadId"
#define DB_KEY_SESSIONKEY "SessionKey"
#define DB_KEY_MEMBERSINCE "MemberSince"

#include "http.h"
#include "proto.h"

bool IsChat(const CMStringW &aimid);
bool IsValidType(const JSONNode &aimid);

void RefreshGroups(void);
wchar_t* time2text(time_t time);

extern bool g_bSecureIM, g_bMessageState;
