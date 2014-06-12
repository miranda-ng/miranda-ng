// ---------------------------------------------------------------------------80
//                ICQ plugin for Miranda Instant Messenger
//                ________________________________________
//
// Copyright © 2000-2001 Richard Hughes, Roland Rabien, Tristan Van de Vreede
// Copyright © 2001-2002 Jon Keating, Richard Hughes
// Copyright © 2002-2004 Martin Öberg, Sam Kothari, Robert Rainwater
// Copyright © 2004-2010 Joe Kucera
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
//
// -----------------------------------------------------------------------------
//  DESCRIPTION:
//
// Includes all header files that should be precompiled to speed up compilation.
//
// -----------------------------------------------------------------------------
#define _WIN32_WINNT 0x0501
#define _WIN32_IE 0x0501

// Windows includes
#include <windows.h>
#include <commctrl.h>

// Standard includes
#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <io.h>
#include <direct.h>
#include <fcntl.h>
#include <process.h>

//C++
#include <list>

#ifndef _DEBUG
#include <crtdbg.h>
#endif

#ifndef AW_VER_POSITIVE
#define AW_VER_POSITIVE 0x00000004
#endif

#ifndef _ASSERTE
#define _ASSERTE(x)
#endif

// Miranda IM SDK includes
#include <newpluginapi.h> // This must be included first
#include <m_clist.h>
#include <m_clui.h>
#include <m_database.h>
#include <m_langpack.h>
#include <m_message.h>
#include <m_netlib.h>
#include <m_protocols.h>
#include <m_protomod.h>
#include <m_protosvc.h>
#include <m_options.h>
#include <m_system.h>
#include <m_system_cpp.h>
#include <m_userinfo.h>
#include <m_utils.h>
#include <m_idle.h>
#include <m_skin.h>
#include <m_popup.h>
#include <m_clistint.h>
#include <m_cluiframes.h>
#include <m_ignore.h>
#include <m_icolib.h>
#include <m_avatars.h>
#include <m_xstatus.h>
#include <m_timezones.h>
#include <win2k.h>

// Project resources
#include "resource.h"

// ICQ plugin includes
#include "version.h"
#include "globals.h"
#include "i18n.h"
#include "cookies.h"
#include "icq_packet.h"
#include "utilities.h"
#include "oscar_filetransfer.h"
#include "icq_direct.h"
#include "icq_server.h"
#include "icqosc_svcs.h"
#include "icq_servlist.h"
#include "icq_http.h"
#include "icq_fieldnames.h"
#include "icq_constants.h"
#include "capabilities.h"
#include "guids.h"
#include "init.h"
#include "stdpackets.h"
#include "tlv.h"
#include "channels.h"
#include "families.h"
#include "m_icq.h"
#include "icq_advsearch.h"
#include "log.h"

#include "icq_rates.h"

#include "icq_avatar.h"

#include "changeinfo/changeinfo.h"
#include "icq_popups.h"
#include "icq_proto.h"

extern LIST<CIcqProto> g_Instances;
