/*
	ClientChangeNotify - Plugin for Miranda IM
	Copyright (c) 2006-2008 Chervov Dmitry

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

#pragma once

#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN

#define CHARARRAY_CONVERT

#include <windows.h>
#include <commctrl.h>
#include <stdlib.h>
#include <crtdbg.h>
#include <shellapi.h>
#include <commdlg.h>

#include "newpluginapi.h"
#include "statusmodes.h"
#include "m_popup.h"
#include "m_skin.h"
#include "m_langpack.h"
#include "m_options.h"
#include "m_clist.h"
#include "m_system.h"
#include "m_message.h"
#include "m_userinfo.h"
#include "m_history.h"
#include "m_protocols.h"
#include "m_protosvc.h"
#include "m_icolib.h"
#include "m_genmenu.h"
#include "win2k.h"
#include "m_metacontacts.h"

#include "m_fingerprint.h"

#include "CommonLibs\TMyArray.h"
#include "CommonLibs\Options.h"
#include "CommonLibs\CString.h"
#include "CommonLibs\pcre.h"

#include "resource.h"
#include "Misc.h"
#include "Version.h"

// Actions on popup click
#define PCA_OPENMESSAGEWND	0	// open message window
#define PCA_CLOSEPOPUP			1	// close popup
#define PCA_OPENDETAILS			2	// open contact details window
#define PCA_OPENMENU				3	// open contact menu
#define PCA_OPENHISTORY			4	// open contact history
#define PCA_DONOTHING				6 // do nothing

#define POPUP_DEF_LCLICKACTION PCA_OPENMESSAGEWND
#define POPUP_DEF_RCLICKACTION PCA_CLOSEPOPUP
#define POPUP_DEF_POPUP_BGCOLOUR 0xEAFFFB
#define POPUP_DEF_POPUP_TEXTCOLOUR 0
#define POPUP_DEF_USEDEFBGCOLOUR 0
#define POPUP_DEF_USEDEFTEXTCOLOUR 0
#define POPUP_DEF_POPUPDELAY 0

#define IGNORESTRINGS_MAX_LEN 2048
#define MAX_MSG_LEN 8192

#define NOTIFYTIMER_INTERVAL 3500

#define MOD_NAME "ClientChangeNotify"
#define LOG_ID MOD_NAME
#define LOG_PREFIX MOD_NAME ": " // for netlib.log

#define MS_NETLIB_LOG "Netlib/Log"

#define DB_MIRVER "MirVer"
#define DB_OLDMIRVER "OldMirVer"
#define DB_NO_FINGERPRINT_ERROR "NoFingerprintError"
#define DB_IGNORESUBSTRINGS "IgnoreSubstrings"
#define DB_SETTINGSVER "SettingsVer"

#define DB_CCN_NOTIFY "Notify" // database key name used for per-contact settings; 0 = ignore, 1 = notify always, 2 = use global settings
#define NOTIFY_IGNORE 0
#define NOTIFY_ALMOST_ALWAYS 1 // don't notify only when CCN popups are disabled globally in the plugin
#define NOTIFY_ALWAYS 2
#define NOTIFY_USEGLOBAL 3

#define CLIENTCHANGED_SOUND "ClientChanged"

extern HINSTANCE g_hInstance;
extern BOOL bPopupExists, bFingerprintExists;

extern COptPage g_PopupOptPage;
extern COptPage *g_PreviewOptPage;

#define MS_VARS_FORMATSTRING "Vars/FormatString"
#define MS_CCN_TOGGLEPOPUPS "ClientChangeNotify/TogglePopups"


static __inline CString LogMessage(const char *Format, ...)
{
	va_list va;
	char szText[8096];
	strcpy(szText, LOG_PREFIX);
	va_start(va, Format);
	mir_vsnprintf(szText + (lengthof(LOG_PREFIX) - 1), sizeof(szText) - (lengthof(LOG_PREFIX) - 1), Format, va);
	va_end(va);
	CallService(MS_NETLIB_LOG, NULL, (LPARAM)szText);
	return CString(szText);
}


// ClientChangeNotify.cpp

struct PLUGIN_DATA
{
	HICON hIcon; // needed here to destroy its handle on UM_FREEPLUGINDATA
	int PopupLClickAction;
	int PopupRClickAction;
};

struct SHOWPOPUP_DATA
{
	MCONTACT hContact;
	TCString OldMirVer;
	TCString MirVer;
	COptPage *PopupOptPage;
};


int ContactSettingChanged(WPARAM wParam, LPARAM lParam);

// OptDlg.cpp
int OptionsDlgInit(WPARAM wParam, LPARAM lParam);
void InitOptions();
