
//This file is part of Msg_Export a Miranda IM plugin
//Copyright (C)2002 Kennet Nielsen ( http://sourceforge.net/projects/msg-export/ )
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#ifndef MSG_EXP_GLOB_H
#define MSG_EXP_GLOB_H

#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>
#include <windowsx.h>
#include <Richedit.h>
using namespace std;
#include <Shlobj.h>
#include <list>
#include <string>
#include <map>

#include <newpluginapi.h>
#include <m_database.h>
#include <m_clist.h>
#include <m_langpack.h>
#include <m_options.h>
#include <m_history.h>
#include <m_userinfo.h>
#include <m_protosvc.h>
#include <m_icq.h>
#include <win2k.h>

#include "utils.h"
#include "options.h"
#include "FileViewer.h"
#include "resource.h"
#include "version.h"

#define MODULE "Msg_Export"
#define MSG_BOX_TITEL TranslateT("Miranda NG (Message Export Plugin)")
#define MS_SHOW_EXPORT_HISTORY "History/ShowExportHistory"
#define szFileViewDB "FileV_"
#define WM_RELOAD_FILE (WM_USER+10)

extern HINSTANCE hInstance;

extern HANDLE hInternalWindowList;

#endif