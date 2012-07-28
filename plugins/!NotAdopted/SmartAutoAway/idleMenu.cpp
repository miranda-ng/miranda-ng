/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2006 Miranda ICQ/IM project, 
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
#include "globals.h"
#ifdef SAA_PLUGIN
# include "m_icolib.h"
#endif

HANDLE hIdleMenu[4] = {NULL,NULL,NULL,NULL};
HANDLE hHookIconsChanged = NULL;
static char *iconDescs[4]={"Not Idle","Short Idle","Long Idle","Reconnect"};
static int iconInd[4]={IDI_IDLE_HERE,IDI_IDLE_SHORT,IDI_IDLE_LONG,IDI_RECONNECT};
static char *IdleServices[4]={AA_IDLE_BENOTIDLESERVICE,AA_IDLE_BESHORTIDLESERVICE,AA_IDLE_BELONGIDLESERVICE,AA_IDLE_RECONNECTSERVICE};

void xModifyMenu(int menuInd,long flags)
{	if (hIdleMenu[menuInd]){
	CLISTMENUITEM mi = {0};

	mi.cbSize = sizeof(mi);
	mi.flags = CMIM_FLAGS;
	mi.flags |= flags;
	CallService(MS_CLIST_MODIFYMENUITEM,(WPARAM)hIdleMenu[menuInd],(LPARAM)&mi);
}	}

static int hasIcoLib = 0;

static void CListSetMenuItemIcon(HANDLE hMenuItem, HICON hIcon)
{
  CLISTMENUITEM mi = {0};

  mi.cbSize = sizeof(mi);
  mi.flags = CMIM_FLAGS | CMIM_ICON;

  mi.hIcon = hIcon;
  CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenuItem, (LPARAM)&mi);
}

static int IconLibIconsChanged(WPARAM wParam, LPARAM lParam)
{
	int i;
	for (i=0;i<4;i++)
	{
		HICON hIcon = (HICON)CallService(MS_SKIN2_GETICON, 0, (LPARAM)IdleServices[i]);
		CListSetMenuItemIcon(hIdleMenu[i], hIcon);
		CallService(MS_SKIN2_RELEASEICON, 0, (LPARAM)IdleServices[i]);
	}
	return 0;
}

void AddIdleMenu(){	
	CLISTMENUITEM mi;
	int i;
	if (ServiceExists(MS_SKIN2_GETICON)){
		SKINICONDESC sid = {0};
		hHookIconsChanged = HookEvent(ME_SKIN2_ICONSCHANGED, IconLibIconsChanged);
		hasIcoLib = 1;
		sid.cbSize = SKINICONDESC_SIZE_V2;
		sid.pszSection = SECTIONNAME;
		sid.pszDefaultFile = NULL;
		sid.iDefaultIndex = 0;
		for (i=0;i<4;i++){
			sid.pszDescription = iconDescs[i];
			sid.pszName = IdleServices[i];
			sid.iDefaultIndex = iconInd[i];
			sid.hDefaultIcon = (HICON)LoadImage( 
									#ifdef SAA_PLUGIN
										g_hInst,
									#else
										GetModuleHandle(NULL),
									#endif
										MAKEINTRESOURCE( iconInd[i] ), IMAGE_ICON, 0, 0, 0);
			CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);
			DestroyIcon(sid.hDefaultIcon);
		}
	}

 	ZeroMemory(&mi,sizeof(mi));
	mi.cbSize=sizeof(mi);
	mi.pszPopupName="&Idle";
	mi.popupPosition=1000090000;
	for (i=0;i<4;i++)
	{
		if (hasIcoLib)
			mi.hIcon = (HICON)CallService(MS_SKIN2_GETICON, 0, (LPARAM)IdleServices[i]);
		else
			mi.hIcon = (HICON)LoadImage(
			#ifdef SAA_PLUGIN
				g_hInst,
			#else
				GetModuleHandle(NULL),
			#endif
				MAKEINTRESOURCE(iconInd[i]), IMAGE_ICON, 0, 0, 0);

		mi.position=i+1;
		mi.pszName=iconDescs[i];
		mi.pszService=IdleServices[i];
		if (i==3) mi.position=1000000;
		hIdleMenu[i]=(HANDLE)CallService(MS_CLIST_ADDMAINMENUITEM,0,(LPARAM)&mi);

		if (hasIcoLib)
			CallService(MS_SKIN2_RELEASEICON, 0, (LPARAM)IdleServices[i]);
		else
			DestroyIcon(mi.hIcon);
	}
//	return 0; 
}

MIRANDASERVICE idleServiceNotIdle(WPARAM w, LPARAM l){
	SimulateIdle(0); 
	return 0;
};
MIRANDASERVICE idleServiceShortIdle(WPARAM w, LPARAM l){
	SimulateIdle(1); 
	return 0;
};
MIRANDASERVICE idleServiceLongIdle(WPARAM w, LPARAM l){
	SimulateIdle(2); 
	return 0;
};
MIRANDASERVICE reconnectService(WPARAM w, LPARAM l){
	int i;
	for (i=0;i<protoCount;i++){
		if (isInterestingProto(i)){
			protoStatus[i]|=4;
		}
	}
	return 0;
};
