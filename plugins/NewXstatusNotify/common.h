/*
	NewXstatusNotify YM - Plugin for Miranda IM
	Copyright (c) 2001-2004 Luca Santarelli
	Copyright (c) 2005-2007 Vasilich
	Copyright (c) 2007-2011 yaho

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

#ifndef COMMON_H
#define COMMON_H

#pragma once

#define STRICT
#define WIN32_LEAN_AND_MEAN

#define _WIN32_WINNT 0x0500
#define WINVER 0x0600
#define MIRANDA_VER 0x0A00

//Standard Windows "libraries"
#include <windows.h>
#include <mmsystem.h>
#include <commctrl.h>
#include <commdlg.h>
#include <shellapi.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>

#include "newpluginapi.h"
#include "statusmodes.h"
#include "win2k.h"

#include "m_awaymsg.h"
#include "m_button.h"
#include "m_clc.h"
#include "m_database.h"
#include "m_genmenu.h"
#include "m_history.h"
#include "m_icolib.h"
#include "m_ignore.h"
#include "m_langpack.h"
#include "m_message.h"
#include "m_options.h"
#include "m_plugins.h"
#include "m_popup.h"
#include "m_protosvc.h"
#include "m_skin.h"
#include "m_system.h"
#include "m_userinfo.h"
#include "m_utils.h"
#include "m_icq.h"

#include "m_metacontacts.h"
#include "m_updater.h"
#include "m_toolbar.h"
#include "resource.h"

#if defined _WIN64
#define	MIID_NXSN { 0x16f0674f, 0xc4e7, 0x494e, { 0xb7, 0x5e, 0x41, 0xbb, 0xf4, 0xaf, 0xe6, 0x11 } }
#elif defined _UNICODE
#define MIID_NXSN { 0xebf19652, 0xe434, 0x4d79, { 0x98, 0x97, 0x91, 0xa0, 0xff, 0x22, 0x6f, 0x51 } }
#else
#define	MIID_NXSN { 0x90887a8f, 0x9087, 0x44d6, { 0xb4, 0xcc, 0xa6, 0x9d, 0xe0, 0x2e, 0x15, 0x17 } }
#endif

#define MODULE "NewStatusNotify"

#define WM_AWAYMSG				WM_USER + 0x0192

#define MAX_STATUSTEXT			36
#define MAX_STANDARDTEXT		36
#define MAX_SKINSOUNDNAME		36
#define MAX_SKINSOUNDDESC		36

#define GENDER_UNSPECIFIED		0
#define GENDER_MALE				1
#define GENDER_FEMALE			2

#define NOTIFY_INTERVAL_JABBER  2000
#define NOTIFY_INTERVAL_ICQ		5000

#define ID_STATUS_FROMOFFLINE	1
#define ID_STATUS_EXTRASTATUS   40081
#define ID_STATUS_MIN			ID_STATUS_OFFLINE
#define ID_STATUS_MAX			ID_STATUS_OUTTOLUNCH
#define ID_STATUS_MAX2			ID_STATUS_EXTRASTATUS
#define STATUS_COUNT			ID_STATUS_MAX2 - ID_STATUS_MIN + 1
#define Index(ID_STATUS)		ID_STATUS - ID_STATUS_OFFLINE

#define COLOR_BG_AVAILDEFAULT	RGB(173,206,247)
#define COLOR_BG_NAVAILDEFAULT	RGB(255,189,189)
#define COLOR_TX_DEFAULT		RGB(0,0,0)

#define ICO_NOTIFICATION_OFF	"notification_off"
#define ICO_NOTIFICATION_ON		"notification_on"

#define JS_PARSE_XMPP_URI       "/ParseXmppURI"

extern LIST_INTERFACE li;

typedef struct tagSTATUS
{
	int ID;
	int icon;
	TCHAR lpzMStatusText[MAX_STATUSTEXT];
	TCHAR lpzFStatusText[MAX_STATUSTEXT];
	TCHAR lpzUStatusText[MAX_STATUSTEXT];
	TCHAR lpzStandardText[MAX_STANDARDTEXT];
	char lpzSkinSoundName[MAX_SKINSOUNDNAME];
	char lpzSkinSoundDesc[MAX_SKINSOUNDDESC];
	char lpzSkinSoundFile[MAX_PATH];
	COLORREF colorBack;
	COLORREF colorText;
} STATUS;

typedef struct tagPLUGINDATA 
{
	WORD newStatus;
	WORD oldStatus;
	HANDLE hAwayMsgProcess;
	HANDLE hAwayMsgHook;
} PLUGINDATA;

/*
HANDLE hContact = (HANDLE)wParam;
WORD oldStatus = LOWORD(lParam);
WORD newStatus = HIWORD(lParam);
oldStatus is the status the contact was before the change.
newStatus is the status the contact is now.
Cast them to (int) if you need them that way.
*/
#define ME_STATUSCHANGE_CONTACTSTATUSCHANGED "Miranda/StatusChange/ContactStatusChanged"

#define MS_STATUSCHANGE_MENUCOMMAND "NewStatusNotify/EnableDisableMenuCommand"

#endif //COMMON_H
