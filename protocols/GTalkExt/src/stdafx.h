//***************************************************************************************
//
//   Google Extension plugin for the Miranda IM's Jabber protocol
//   Copyright (c) 2011 bems@jabber.org, George Hazan (ghazan@jabber.ru)
//
//   This program is free software; you can redistribute it and/or
//   modify it under the terms of the GNU General Public License
//   as published by the Free Software Foundation; either version 2
//   of the License, or (at your option) any later version.
//
//   This program is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
//   You should have received a copy of the GNU General Public License
//   along with this program; if not, write to the Free Software
//   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
//***************************************************************************************

#pragma once

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#define _CRT_SECURE_NO_WARNINGS

#include <assert.h>
#include <malloc.h>
#include <time.h>
#include <stdlib.h>
#include <tchar.h>

// Windows Header Files:
#include <windows.h>
#include <windowsx.h>
#include <PrSht.h>
#include <shellapi.h>

#include <psapi.h>
#pragma comment(lib, "psapi.lib")

#include <winsock.h>

// Miranda&Plugins API
#include <newpluginapi.h>
#include <m_database.h>

#pragma warning(push)
#pragma warning(disable:4996)

#include <m_system.h>
#include <m_system_cpp.h>
#include <m_popup.h>
#include <m_jabber.h>
#include <m_langpack.h>
#include <m_protosvc.h>
#include <m_protocols.h>
#include <m_protomod.h>
#include <m_protoint.h>
#include <m_options.h>
#include <m_utils.h>
#include <m_clist.h>
#include <m_avatars.h>
#include <m_netlib.h>
#include <win2k.h>
#pragma warning(pop)

#include "googletalk.h"

extern HINSTANCE g_hInst;
extern LIST<GoogleTalkAcc> g_accs;

void SetAvatar(MCONTACT hContact);
BOOL InitAvaUnit(BOOL init);
