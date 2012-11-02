/*
Copyright (C) 2007-2009 Ricardo Pescuma Domenecci

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this file; see the file license.txt.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  
*/

#include <Windows.h>

#include "mir_icons.h"

#include <newpluginapi.h>
#include <m_system.h>
#include <m_icolib.h>

extern HINSTANCE hInst;

HICON IcoLib_LoadIcon(const char *iconName, BOOL copy)
{
	if (iconName == NULL || iconName[0] == 0)
		return NULL;
	
	HICON hIcon = Skin_GetIcon(iconName);
	if (copy && hIcon != NULL) {
		hIcon = CopyIcon(hIcon);
		Skin_ReleaseIcon(hIcon);
	}
	return hIcon;
}

void IcoLib_ReleaseIcon(const char *iconName)
{
	Skin_ReleaseIcon(iconName);
}

void IcoLib_ReleaseIcon(HICON hIcon)
{
	Skin_ReleaseIcon(hIcon);
}

HANDLE IcoLib_Register(char *name, char *section, char *description, int id)
{
	HANDLE hIcon = Skin_GetIconHandle(name);
	if (hIcon != NULL)
		return hIcon;

	TCHAR tszFile[MAX_PATH];
	GetModuleFileName(hInst, tszFile, MAX_PATH);

	SKINICONDESC sid = {0};
	sid.cbSize = sizeof(SKINICONDESC);
	sid.flags = SIDF_PATH_TCHAR;
	sid.pszName = name;
	sid.pszSection = section;
	sid.pszDescription = description;
	sid.ptszDefaultFile = tszFile;
	sid.iDefaultIndex = -id;
	return Skin_AddIcon(&sid);
}
