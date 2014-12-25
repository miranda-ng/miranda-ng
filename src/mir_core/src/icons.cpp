/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-14 Miranda NG project,
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

#include <m_icolib.h>
#include "..\src\core\stdplug.h"

MIR_CORE_DLL(void) Icon_Register(HINSTANCE hInst, const char *szSection, IconItem *pIcons, size_t iCount, char *prefix, int hLangpack)
{
	TCHAR szFile[MAX_PATH];
	GetModuleFileName(hInst, szFile, MAX_PATH);

	SKINICONDESC sid = { sizeof(sid) };
	sid.ptszDefaultFile = szFile;
	sid.pszSection = (char*)szSection;
	sid.flags = SIDF_PATH_TCHAR;

	for (unsigned i = 0; i < iCount; i++) {
		char szSetting[100];
		if (prefix) {
			mir_snprintf(szSetting, SIZEOF(szSetting), "%s_%s", prefix, pIcons[i].szName);
			sid.pszName = szSetting;
		}
		else sid.pszName = pIcons[i].szName;

		sid.cx = sid.cy = pIcons[i].size;
		sid.pszDescription = pIcons[i].szDescr;
		sid.iDefaultIndex = -pIcons[i].defIconID;
		pIcons[i].hIcolib = (HANDLE)CallService("Skin2/Icons/AddIcon", hLangpack, (LPARAM)&sid);
	}
}

MIR_CORE_DLL(void) Icon_RegisterT(HINSTANCE hInst, const TCHAR *szSection, IconItemT *pIcons, size_t iCount, char *prefix, int hLangpack)
{
	TCHAR szFile[MAX_PATH];
	GetModuleFileName(hInst, szFile, MAX_PATH);

	SKINICONDESC sid = { sizeof(sid) };
	sid.ptszDefaultFile = szFile;
	sid.ptszSection = (TCHAR*)szSection;
	sid.flags = SIDF_ALL_TCHAR;

	for (unsigned i = 0; i < iCount; i++) {
		char szSetting[100];
		if (prefix) {
			mir_snprintf(szSetting, SIZEOF(szSetting), "%s_%s", prefix, pIcons[i].szName);
			sid.pszName = szSetting;
		}
		else sid.pszName = pIcons[i].szName;

		sid.cx = sid.cy = pIcons[i].size;
		sid.ptszDescription = pIcons[i].tszDescr;
		sid.iDefaultIndex = -pIcons[i].defIconID;
		pIcons[i].hIcolib = (HANDLE)CallService("Skin2/Icons/AddIcon", hLangpack, (LPARAM)&sid);
	}
}
