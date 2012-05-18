/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2007 Artem Shpynov
Copyright 2000-2007 Miranda ICQ/IM project,

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
#include "modernopt.h"

static HWND g_hwndModernOptIgnore = NULL;

void ModernOptIgnore_AddItem(MODERNOPTOBJECT *obj)
{
	if ( g_hwndModernOptIgnore )
		SendMessage(g_hwndModernOptIgnore, WM_APP, 0, (LPARAM)obj);
}

static void ResetListOptions(HWND hwndList)
{
	SendMessage(hwndList,CLM_SETBKBITMAP,0,(LPARAM)(HBITMAP)NULL);
	SendMessage(hwndList,CLM_SETBKCOLOR,GetSysColor(COLOR_WINDOW),0);
	SendMessage(hwndList,CLM_SETGREYOUTFLAGS,0,0);
	SendMessage(hwndList,CLM_SETLEFTMARGIN,4,0);
	SendMessage(hwndList,CLM_SETINDENT,10,0);
	SendMessage(hwndList,CLM_SETHIDEEMPTYGROUPS,1,0);

	for(int i=0;i<=FONTID_MAX;i++)
		SendMessage(hwndList,CLM_SETTEXTCOLOR,i,GetSysColor(COLOR_WINDOWTEXT));
}

static void SetAllContactIcons(HWND hwndList, int count)
{
	HANDLE hContact=(HANDLE)CallService(MS_DB_CONTACT_FINDFIRST,0,0);
	do {
		DWORD hItem = SendMessage(hwndList,CLM_FINDCONTACT,(WPARAM)hContact,0);
		for (int i = 0; i < count; ++i)
			SendMessage(hwndList,CLM_SETEXTRAIMAGE,hItem,MAKELPARAM(i, i+1));
		if(!DBGetContactSettingByte(hContact,"CList","Hidden",0))
			SendMessage(hwndList,CLM_SETCHECKMARK,hItem,1);
	}
		while(hContact=(HANDLE)CallService(MS_DB_CONTACT_FINDNEXT,(WPARAM)hContact,0));
}
