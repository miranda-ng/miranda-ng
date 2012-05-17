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


#include "mir_icons.h"

#include <newpluginapi.h>
#include <m_system.h>
#include <m_icolib.h>

extern HINSTANCE hInst;



HICON IcoLib_LoadIcon(const char *iconName, BOOL copy)
{
	if (!ServiceExists(MS_SKIN2_GETICON))
		return NULL;

	if (iconName == NULL || iconName[0] == 0)
		return NULL;
	
	HICON hIcon = (HICON) CallService(MS_SKIN2_GETICON, 0, (LPARAM) iconName);
	if (copy && hIcon != NULL)
	{
		hIcon = CopyIcon(hIcon);
		CallService(MS_SKIN2_RELEASEICON, (WPARAM) hIcon, 0);
	}
	return hIcon;
}

void IcoLib_ReleaseIcon(const char *iconName)
{
	if (ServiceExists(MS_SKIN2_RELEASEICON))
		CallService(MS_SKIN2_RELEASEICON, 0, (LPARAM) iconName);
}


void IcoLib_ReleaseIcon(HICON hIcon)
{
	if (ServiceExists(MS_SKIN2_RELEASEICON))
		CallService(MS_SKIN2_RELEASEICON, (WPARAM) hIcon, 0);
}


void IcoLib_Register(char *name, TCHAR *section, TCHAR *description, int id)
{
	HICON hIcon = (HICON) CallService(MS_SKIN2_GETICON, 0, (LPARAM) name);
	if (hIcon != NULL)
	{
		CallService(MS_SKIN2_RELEASEICON, (WPARAM) hIcon, 0);
		return;
	}

	SKINICONDESC sid = {0};
	sid.cbSize = sizeof(SKINICONDESC);
	sid.flags = SIDF_TCHAR;
	sid.pszName = name;
	sid.ptszSection = section;
	sid.ptszDescription = description;

	int cx = GetSystemMetrics(SM_CXSMICON);
	sid.hDefaultIcon = (HICON) LoadImage(hInst, MAKEINTRESOURCE(id), IMAGE_ICON, cx, cx, LR_DEFAULTCOLOR | LR_SHARED);

	CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);

	if (sid.hDefaultIcon)
		DestroyIcon(sid.hDefaultIcon);
}

