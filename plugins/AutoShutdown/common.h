/*

'AutoShutdown'-Plugin for Miranda IM

Copyright 2004-2007 H. Herkenrath

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program (Shutdown-License.txt); if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include <stdio.h>  /* for mir_sntprintf() */
#include <time.h>   /* for mktime(),time() */
#include <tchar.h>
#include <windows.h>
#include <commctrl.h>

/* WinXP+: shutdown reason codes */
#if defined(EWX_RESTARTAPPS)   /* new MS Platform SDK */
	#include <reason.h>
#else
	#define SHTDN_REASON_MAJOR_OTHER   0x00000000
	#define SHTDN_REASON_MINOR_OTHER   0x00000000
	#define SHTDN_REASON_FLAG_PLANNED  0x80000000
#endif

/* RAS */
#undef WINVER
#define WINVER  0x400  /* prevent INVALID_BUFFER error */ 
#include <ras.h>       /* for RasEnumConnections(), RasHangUp() */
#include <raserror.h>  /* error codes for RAS */

#define MIRANDA_VER  0x0A00
#include <newpluginapi.h>
#include <m_utils.h>
#include <m_database.h>
#include <m_clist.h>
#include <m_clui.h>
#include <m_langpack.h>
#include <m_system.h>
#include <m_skin.h>
#include <m_protosvc.h>
#include <m_protocols.h>
#include <m_plugins.h>
#include <m_options.h>
#include <m_message.h>
#include <m_file.h>
#include <m_idle.h>
#include <win2k.h>
#include <m_cluiframes.h>
#include <m_clistint.h>
#include <m_fontservice.h>
#include <m_icolib.h>
#include <m_clc.h>
#include <m_genmenu.h>
#include <m_button.h>

#include <m_weather.h>
#include <m_hddinfo.h>
#include <m_toptoolbar.h>
#include <m_hotkey.h>
#include <m_hotkeysplus.h>
#include <m_hotkeysservice.h>
#include <m_trigger.h>
#include <m_mwclc.h>
#include <m_autoreplacer.h>
#include <m_magneticwindows.h>
#include <m_snappingwindows.h>
#include "m_shutdown.h"

#include "cpuusage.h"
#include "frame.h"
#include "options.h"
#include "settingsdlg.h"
#include "shutdownsvc.h"
#include "utils.h"
#include "watcher.h"
#include "resource.h"
