/*
    AdvancedAutoAway Plugin for Miranda-IM (www.miranda-im.org)
    KeepStatus Plugin for Miranda-IM (www.miranda-im.org)
    StartupStatus Plugin for Miranda-IM (www.miranda-im.org)
    Copyright 2003-2006 P. Boon

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#ifndef COMMONSTATUSHEADER
#define COMMONSTATUSHEADER

#define _CRT_SECURE_NO_WARNINGS

#define _WIN32_WINNT 0x0501
#include <windows.h>
#include <shlobj.h>
#include <uxtheme.h>
#include <stdio.h>
#include <tchar.h>

#include <newpluginapi.h>
#include <m_system.h>
#include <m_system_cpp.h>
#include <m_awaymsg.h>
#include <m_database.h>
#include <m_protosvc.h>
#include <m_clist.h>
#include <m_clistint.h>
#include <m_string.h>
#include <m_langpack.h>
#include <m_popup.h>
#include <m_variables.h>
#include <m_netlib.h>
#include "m_statusplugins.h"
#include <m_utils.h>
#include <m_NewAwaySys.h>
#include <win2k.h>

#if defined( _WIN64 )
	#define __PLATFORM_NAME  "64"
#else
	#define __PLATFORM_NAME  ""
#endif

#include "../helpers/gen_helpers.h"

#define UM_STSMSGDLGCLOSED				WM_APP+1
#define UM_CLOSECONFIRMDLG				WM_APP+2

#define PREFIX_LAST						"last_"
#define PREFIX_LASTMSG					"lastmsg_"
#define DEFAULT_STATUS					ID_STATUS_LAST
#define ID_STATUS_LAST					40081 // doesn't interfere with ID_STATUS_IDLE, since we don't use it. However this *is* a good lesson not to do this again.
#define ID_STATUS_CURRENT				40082
#define ID_STATUS_DISABLED				41083 // this should not be send to setstatus(ex)
#define MAX_STATUS						ID_STATUS_CURRENT
#define MIN_STATUS						ID_STATUS_OFFLINE
#define DEF_CLOSE_TIME					5 //secs
#define PF2_OFFLINE						0x00000200
static int statusModeList[]={ID_STATUS_OFFLINE,ID_STATUS_ONLINE,ID_STATUS_AWAY,ID_STATUS_NA,ID_STATUS_OCCUPIED,ID_STATUS_DND,ID_STATUS_FREECHAT,ID_STATUS_INVISIBLE,ID_STATUS_ONTHEPHONE,ID_STATUS_OUTTOLUNCH};
static int statusModePf2List[]={PF2_OFFLINE,PF2_ONLINE,PF2_SHORTAWAY,PF2_LONGAWAY,PF2_LIGHTDND,PF2_HEAVYDND,PF2_FREECHAT,PF2_INVISIBLE,PF2_ONTHEPHONE,PF2_OUTTOLUNCH};

TCHAR *GetDefaultStatusMessage(PROTOCOLSETTINGEX *ps, int status);
int GetActualStatus(PROTOCOLSETTINGEX *protoSetting);
int InitCommonStatus();
bool IsSuitableProto(PROTOACCOUNT *pa);

/* SimpleAway */

// wParam = 0
// lParam = 0
// allways returns 1
#define MS_SA_ISSARUNNING  "SimpleAway/IsSARunning"

extern HINSTANCE hInst;
extern OBJLIST<PROTOCOLSETTINGEX>* protoList;

#endif //COMMONSTATUSHEADER
