/*
Copyright (C) 2006 Ricardo Pescuma Domenecci

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this file; see the file license.txt.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.
*/


#ifndef __COMMONS_H__
# define __COMMONS_H__

#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NONSTDC_NO_DEPRECATE

#include <windows.h>
#include <time.h>
#include <vector>
#include <algorithm>

#include <newpluginapi.h>
#include <win2k.h>
#include <m_protosvc.h>
#include <m_langpack.h>
#include <m_database.h>
#include <m_options.h>
#include <m_xstatus.h>
#include <m_clui.h>
#include <m_genmenu.h>
#include <m_hotkeys.h>
#include <m_extraicons.h>

#include <m_metacontacts.h>
#include <m_proto_listeningto.h>
#include <m_music.h>
#include <m_radio.h>
#include <m_toptoolbar.h>
#include <m_listeningto.h>

#include "..\utils\mir_options.h"
#include "..\utils\mir_buffer.h"
#include "..\utils\utf8_helpers.h"

#include "music.h"
#include "resource.h"
#include "options.h"
#include "Version.h"

// Prototypes ///////////////////////////////////////////////////////////////////////////

// Service called by the main menu
#define MS_LISTENINGTO_MAINMENU				"ListeningTo/MainMenu"

// Service called by toptoolbar
#define MS_LISTENINGTO_TTB					"ListeningTo/TopToolBar"

// Services called by hotkeys
#define MS_LISTENINGTO_HOTKEYS_ENABLE		"ListeningTo/HotkeysEnable"
#define MS_LISTENINGTO_HOTKEYS_DISABLE		"ListeningTo/HotkeysDisable"
#define MS_LISTENINGTO_HOTKEYS_TOGGLE		"ListeningTo/HotkeysToggle"

#define MODULE_NAME		"ListeningTo"


// Global Variables
extern HINSTANCE hInst;
extern BOOL loaded;

#define MIR_FREE(_X_) { mir_free(_X_); _X_ = NULL; }

#define MIN_TIME_BEETWEEN_SETS 10000 // ms


void RebuildMenu();
void StartTimer();

struct ProtocolInfo
{
	char proto[128];
	TCHAR account[128];
	HGENMENU hMenu;
	int old_xstatus;
	TCHAR old_xstatus_name[1024];
	TCHAR old_xstatus_message[1024];
};

ProtocolInfo *GetProtoInfo(char *proto);
int m_log(const TCHAR *function, const TCHAR *fmt, ...);


static bool IsEmpty(const char *str)
{
	return str == NULL || str[0] == 0;
}
static bool IsEmpty(const WCHAR *str)
{
	return str == NULL || str[0] == 0;
}

#define DUP(_X_) ( IsEmpty(_X_) ? NULL : mir_tstrdup(_X_))
#define DUPD(_X_, _DEF_) ( IsEmpty(_X_) ? mir_tstrdup(_DEF_) : mir_tstrdup(_X_))
#define U2T(_X_) ( IsEmpty(_X_) ? NULL : mir_u2t(_X_))
#define U2TD(_X_, _DEF_) ( IsEmpty(_X_) ? mir_u2t(_DEF_) : mir_u2t(_X_))


#endif // __COMMONS_H__
