/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2003 Miranda ICQ/IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#define MIRANDA_VER 0x1000

#define _WIN32_WINNT 0x0501

#include "m_stdhdr.h"

#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stddef.h>
#include <process.h>
#include <io.h>
#include <string.h>
#include <direct.h>
#include "resource.h"
#include <newpluginapi.h>
#include <win2k.h>
#include <m_system.h>
#include <m_database.h>
#include <m_langpack.h>
#include "version.h"

extern PLUGINLINK *pluginLink;

extern struct LIST_INTERFACE li;

#ifdef __GNUC__
#define mir_i64(x) (x##LL)
#else
#define mir_i64(x) (x##i64)
#endif

#define NEWSTR_ALLOCA(A) (A==NULL)?NULL:strcpy((char*)alloca(strlen(A)+1),A)
