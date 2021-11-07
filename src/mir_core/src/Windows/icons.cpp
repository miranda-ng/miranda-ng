/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-21 Miranda NG team,
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

#include "../stdafx.h"

MIR_CORE_DLL(void) Icon_Register(HINSTANCE hInst, const char *szSection, IconItem *pIcons, size_t iCount, const char *prefix, HPLUGIN pPlugin)
{
	wchar_t szFile[MAX_PATH];
	GetModuleFileName(hInst, szFile, MAX_PATH);

	SKINICONDESC sid = {};
	sid.defaultFile.w = szFile;
	sid.section.a = (char*)szSection;
	sid.flags = SIDF_PATH_UNICODE;

	for (unsigned i = 0; i < iCount; i++) {
		char szSetting[100];
		if (prefix) {
			mir_snprintf(szSetting, "%s_%s", prefix, pIcons[i].szName);
			sid.pszName = szSetting;
		}
		else sid.pszName = pIcons[i].szName;

		sid.cx = sid.cy = pIcons[i].size;
		sid.description.a = pIcons[i].szDescr;
		sid.iDefaultIndex = -pIcons[i].defIconID;
		pIcons[i].hIcolib = IcoLib_AddIcon(&sid, pPlugin);
	}
}

MIR_CORE_DLL(void) Icon_RegisterT(HINSTANCE hInst, const wchar_t *szSection, IconItemT *pIcons, size_t iCount, const char *prefix, HPLUGIN pPlugin)
{
	wchar_t szFile[MAX_PATH];
	GetModuleFileName(hInst, szFile, MAX_PATH);

	SKINICONDESC sid = {};
	sid.defaultFile.w = szFile;
	sid.section.w = (wchar_t*)szSection;
	sid.flags = SIDF_ALL_UNICODE;

	for (unsigned i = 0; i < iCount; i++) {
		char szSetting[100];
		if (prefix) {
			mir_snprintf(szSetting, "%s_%s", prefix, pIcons[i].szName);
			sid.pszName = szSetting;
		}
		else sid.pszName = pIcons[i].szName;

		sid.cx = sid.cy = pIcons[i].size;
		sid.description.w = pIcons[i].tszDescr;
		sid.iDefaultIndex = -pIcons[i].defIconID;
		pIcons[i].hIcolib = IcoLib_AddIcon(&sid, pPlugin);
	}
}
