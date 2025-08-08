/*
Copyright © 2025 Miranda NG team

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
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <direct.h>
#include <time.h>

#include "resource.h"

#include <m_system.h>
#include <newpluginapi.h>
#include <m_clist.h>
#include <m_contacts.h>
#include <m_langpack.h>
#include <m_netlib.h>
#include <m_options.h>
#include <m_protocols.h>
#include <m_protosvc.h>
#include <m_protoint.h>
#include <m_skin.h>

#include <m_NewStory.h>

#include "../ffi/deltachat.h"

#define MODULENAME "DeltaChat"

#define DB_KEY_DCID     "DcID"
#define DB_KEY_EMAIL    "email"
#define DB_KEY_CHATID   "ChatID"
#define DB_KEY_PASSWORD "Password"

#include "version.h"
#include "proto.h"
#include "utils.h"
