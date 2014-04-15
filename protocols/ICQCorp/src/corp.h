/*
    ICQ Corporate protocol plugin for Miranda IM.
    Copyright (C) 2003-2005 Eugene Tarasenko <zlyden13@inbox.ru>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef corp_h
#define corp_h

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <winsock2.h>
#include <windows.h>
#include <string.h>
#include <process.h>
#include <vector>
#include <time.h>

#include <newpluginapi.h>
#include <m_system.h>
#include <m_protomod.h>
#include <m_protosvc.h>
#include <m_langpack.h>
#include <m_database.h>
#include <m_options.h>
#include <statusmodes.h>
#include <m_clist.h>
#include <m_userinfo.h>

#include "user.h"
#include "transfer.h"
#include "packet.h"
#include "socket.h"
#include "event.h"
#include "protocol.h"
#include "options.h"
#include "resource.h"
#include "version.h"


///////////////////////////////////////////////////////////////////////////////

extern HINSTANCE hInstance;
extern char protoName[64];

extern int LoadServices();
extern int UnloadServices();

#ifdef _DEBUG
extern void T(char *format, ...);
#else
inline void T(char *format, ...) {};
#endif

///////////////////////////////////////////////////////////////////////////////

#endif