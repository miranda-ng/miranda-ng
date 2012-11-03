/*
   IgnoreState plugin for Miranda-IM (www.miranda-im.org)
   (c) 2010 by Kildor

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

#ifndef COMMHEADERS_H
#define COMMHEADERS_H

#define MIRANDA_VER    0x0A00
//#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#define _CRT_SECURE_NO_WARNINGS
#define _WIN32_WINNT 0x0501

#ifndef _WIN32_IE
#define _WIN32_IE 0x0501
#endif


//needed by m_utils.h
#include <stdio.h>

// needed by m_icolib.h > r9256
#include <stddef.h>


//Miranda API headers
#include <windows.h>
#include <win2k.h>
#include <commctrl.h>
#include <newpluginapi.h>
#include <m_database.h>
#include <m_system.h>
//#include <m_protocols.h>
#include <m_utils.h>
#include <m_ignore.h>
#include <m_clist.h>
#include <m_skin.h>
#include <m_options.h>
#include <m_langpack.h>
#include <m_icolib.h>
#include "resource.h"
#include "Version.h"

//external SDK headers
#include <m_extraicons.h>

#define MODULENAME "IgnoreState"

/* 3f6acec5-df0e-4d33-a272-e3630addb6c8 */
#define MIID_IGNORESRATE {0x3f6acec5, 0xdf0e, 0x4d33, {0xa2, 0x72, 0xe3, 0x63, 0x0a, 0xdd, 0xb6, 0xc8}}

struct IGNOREITEMS
{
	TCHAR* name;
	int   type;
	int   icon;
	bool  filtered;
};

extern IGNOREITEMS ii[];
extern int nII;

static byte bUseMirandaSettings;

extern HINSTANCE g_hInst;

void applyExtraImage(HANDLE hContact);

int onOptInitialise(WPARAM wParam, LPARAM lParam);
BOOL checkState(int type);
VOID fill_filter();

extern HANDLE hExtraIcon;

#endif //COMMHEADERS_H
