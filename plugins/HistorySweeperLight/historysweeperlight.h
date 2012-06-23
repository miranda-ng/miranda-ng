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

#define MIRANDA_VER 0x0A00

#if !defined( _WIN64 )
	#define _USE_32BIT_TIME_T
#endif

#include <windows.h>
#include <commctrl.h>
#include <time.h>

#include <win2k.h>
#include <newpluginapi.h>
#include <m_clc.h>
#include <m_database.h>
#include <m_langpack.h>
#include <m_options.h>
#include <m_message.h>
#include <m_icolib.h>
#include <m_clistint.h>
#include <m_skin.h>

#include "resource.h"

// Plugin name
#define ModuleName "History Sweeper Light"

// Plugin UUID for New plugin loader
// req. 0.7.18+ core
// {1D9BF74A-44A8-4b3f-A6E5-73069D3A8979}
#define MIID_HISTORYSWEEPERLIGHT { 0x1d9bf74a, 0x44a8, 0x4b3f, { 0xa6, 0xe5, 0x73, 0x6, 0x9d, 0x3a, 0x89, 0x79 } }

// main.c
extern HINSTANCE hInst;

// options.c
extern const char* time_stamp_strings[];
extern const char* keep_strings[];
int HSOptInitialise(WPARAM wParam, LPARAM lParam);
void InitIcons(void);
void  ReleaseIconEx(const char* name);
HICON LoadIconEx(const char* name);
HANDLE GetIconHandle(const char* name);

// historysweeper.c
void ShutdownAction(void);
int OnWindowEvent(WPARAM wParam, LPARAM lParam);

#endif
