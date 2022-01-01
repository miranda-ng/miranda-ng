/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org),
Copyright (c) 2000-12 Miranda IM project,
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

#pragma once

struct PROFILEMANAGERDATA
{
	wchar_t *ptszProfile;     // in/out
	wchar_t *ptszProfileDir;  // in/out
	BOOL noProfiles;      // in
	
	BOOL bRun;            // out
	BOOL newProfile;      // out
	DATABASELINK *dblink; // out
};

char* makeFileName(const wchar_t *tszOriginalName);
int touchDatabase(const wchar_t *tszProfile, DATABASELINK **pDblink);
int getProfileManager(PROFILEMANAGERDATA *pd);
int getProfilePath(wchar_t *buf, size_t cch);
int isValidProfileName(const wchar_t *name);
bool shouldAutoCreate(wchar_t *szProfile);

extern wchar_t g_profileDir[MAX_PATH], g_profileName[MAX_PATH], g_shortProfileName[MAX_PATH];
extern bool  g_bDbCreated;
extern UINT uMsgProcessProfile;
