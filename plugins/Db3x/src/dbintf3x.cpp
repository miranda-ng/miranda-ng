/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright 2012-13 Miranda NG project,
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

#include "commonheaders.h"

extern BOOL safetyMode;

CDb3x::CDb3x(const TCHAR* tszFileName) :
	CDb3Base(tszFileName)
{
}

CDb3x::~CDb3x()
{
	if (m_pDbCache) {
		mir_free(m_pDbCache);
		KillTimer(NULL,m_flushBuffersTimerId);
	}
}

///////////////////////////////////////////////////////////////////////////////

//this function caches results
DWORD CDb3x::GetSettingsGroupOfsByModuleNameOfs(DBContact *dbc,DWORD ofsContact,DWORD ofsModuleName)
{
	DBContactSettings *dbcs;
	DWORD ofsThis;

	for ( int i=0; i < SETTINGSGROUPOFSCOUNT; i++)
		if (settingsGroupOfsCache[i].ofsContact == ofsContact && settingsGroupOfsCache[i].ofsModuleName == ofsModuleName)
			return settingsGroupOfsCache[i].ofsSettingsGroup;

	ofsThis = dbc->ofsFirstSettings;
	while(ofsThis) {
		dbcs = (struct DBContactSettings*)DBRead(ofsThis,sizeof(struct DBContactSettings),NULL);
		if (dbcs->signature != DBCONTACTSETTINGS_SIGNATURE) DatabaseCorruption( _T("Invalid database module"));
		if (dbcs->ofsModuleName == ofsModuleName) {
			settingsGroupOfsCache[nextSGOCacheEntry].ofsContact = ofsContact;
			settingsGroupOfsCache[nextSGOCacheEntry].ofsModuleName = ofsModuleName;
			settingsGroupOfsCache[nextSGOCacheEntry].ofsSettingsGroup = ofsThis;
			if (++nextSGOCacheEntry==SETTINGSGROUPOFSCOUNT) nextSGOCacheEntry = 0;
			return ofsThis;
		}
		ofsThis = dbcs->ofsNext;
	}
	return 0;
}

void CDb3x::InvalidateSettingsGroupOfsCacheEntry(DWORD ofsSettingsGroup)
{
	for (int i=0; i < SETTINGSGROUPOFSCOUNT; i++) {
		if (settingsGroupOfsCache[i].ofsSettingsGroup == ofsSettingsGroup) {
			settingsGroupOfsCache[i].ofsContact = 0;
			settingsGroupOfsCache[i].ofsModuleName = 0;
			settingsGroupOfsCache[i].ofsSettingsGroup = 0;
			break;
}	}	}
