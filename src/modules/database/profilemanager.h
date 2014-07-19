/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-14 Miranda NG project (http://miranda-ng.org),
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

struct PROFILEMANAGERDATA
{
	TCHAR *szProfile;		// in/out
	TCHAR *szProfileDir;	// in/out
	BOOL noProfiles;		// in
	BOOL newProfile;		// out
	DATABASELINK *dblink;	// out
};

char* makeFileName(const TCHAR *tszOriginalName);
int getProfileManager(PROFILEMANAGERDATA *pd);
int getProfilePath(TCHAR *buf, size_t cch);
int isValidProfileName(const TCHAR *name);
bool fileExist(const TCHAR *fname);
bool shouldAutoCreate(TCHAR *szProfile);

extern TCHAR g_profileDir[MAX_PATH], g_profileName[MAX_PATH], g_shortProfileName[MAX_PATH];
extern bool  g_bDbCreated;
