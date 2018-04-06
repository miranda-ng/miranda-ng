/*
Miranda IM History Sweeper Light plugin
Copyright (C) 2002-2003  Sergey V. Gershovich
Copyright (C) 2006-2009  Boris Krasnovskiy
Copyright (C) 2010, 2011 tico-tico

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
#ifndef __HISTORYSWEEPERLIGHT_H__
#define __HISTORYSWEEPERLIGHT_H__

#if !defined( _WIN64 )
	#define _USE_32BIT_TIME_T
#endif

#include <windows.h>
#include <time.h>

#include <win2k.h>
#include <newpluginapi.h>
#include <m_database.h>
#include <m_langpack.h>
#include <m_options.h>
#include <m_message.h>
#include <m_icolib.h>
#include <m_clistint.h>
#include <m_skin.h>

#include "resource.h"
#include "version.h"

// Plugin name
#define ModuleName "History Sweeper Light"

// main.c
extern HINSTANCE hInst;
extern LIST<void> g_hWindows;

// options.c
extern char* time_stamp_strings[];
extern char* keep_strings[];
int HSOptInitialise(WPARAM wParam, LPARAM lParam);
void InitIcons(void);
void  ReleaseIconEx(const char* name);
HICON LoadIconEx(const char* name);
HANDLE GetIconHandle(const char* name);

// historysweeper.c
void ShutdownAction(void);
int OnWindowEvent(WPARAM wParam, LPARAM lParam);
void SetSrmmIcon(MCONTACT hContact);

#endif
