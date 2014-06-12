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

#include <time.h>   /* for mktime(),time() */
#include <windows.h>
#include <Uxtheme.h>
#include <Shlwapi.h>
#include <PowrProf.h>
#include <Ras.h>

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

#include <newpluginapi.h>
#include <m_database.h>
#include <m_clui.h>
#include <m_langpack.h>
#include <m_skin.h>
#include <m_protosvc.h>
#include <m_options.h>
#include <m_message.h>
#include <m_file.h>
#include <m_idle.h>
#include <win2k.h>
#include <m_cluiframes.h>
#include <m_clistint.h>
#include <m_fontservice.h>
#include <m_icolib.h>
#include <m_hotkeys.h>

#include <m_weather.h>
#include <m_toptoolbar.h>
#include <m_shutdown.h>

#include "cpuusage.h"
#include "frame.h"
#include "options.h"
#include "settingsdlg.h"
#include "shutdownsvc.h"
#include "utils.h"
#include "watcher.h"
#include "resource.h"
#include "version.h"

extern IconItem iconList[];