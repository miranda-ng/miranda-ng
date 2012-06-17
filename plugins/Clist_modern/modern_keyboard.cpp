/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2008 Miranda ICQ/IM project, 
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
#include "hdr/modern_commonheaders.h"
#include <m_hotkeys.h>

int InitSkinHotKeys(void);

INT_PTR hkCloseMiranda(WPARAM wParam,LPARAM lParam)
{
	CallService("CloseAction", 0, 0);
	return 0;
}

INT_PTR hkRestoreStatus(WPARAM wParam,LPARAM lParam)
{
	int nStatus = ModernGetSettingWord(NULL, "CList", "Status", ID_STATUS_OFFLINE);
    CallService(MS_CLIST_SETSTATUSMODE, nStatus, 0);

	return 0;
}

int InitSkinHotKeys(void)
{
	CreateServiceFunction("CLIST/HK/CloseMiranda",hkCloseMiranda);
	CreateServiceFunction("CLIST/HK/RestoreStatus",hkRestoreStatus);

	HOTKEYDESC shk = {0};
	shk.cbSize=sizeof(shk);

	shk.pszDescription=LPGEN("Close Miranda");
	shk.pszName="CloseMiranda";
	shk.pszSection=LPGEN("Main");
	shk.pszService="CLIST/HK/CloseMiranda";
	shk.DefHotKey=0;
	CallService(MS_HOTKEY_REGISTER,0,(LPARAM)&shk);	

	shk.pszDescription=LPGEN("Restore last status");
	shk.pszName="RestoreLastStatus";
	shk.pszSection=LPGEN("Status");
	shk.pszService="CLIST/HK/RestoreStatus";
	shk.DefHotKey=0;
	CallService(MS_HOTKEY_REGISTER,0,(LPARAM)&shk);	

	return 0;
}

void UninitSkinHotKeys(void)
{
}
