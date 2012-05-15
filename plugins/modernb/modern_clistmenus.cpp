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
#include "hdr/modern_clist.h"
#include "m_genmenu.h"
#include "m_clui.h"
#include "hdr/modern_commonprototypes.h"

int LoadFavoriteContactMenu();
int UnloadFavoriteContactMenu();

#pragma hdrstop

INT_PTR CloseAction(WPARAM wParam,LPARAM lParam)
{
	int k;
	g_CluiData.bSTATE = STATE_PREPEARETOEXIT;  // workaround for avatar service and other wich destroys service on OK_TOEXIT
	do
	{	k = CallService(MS_SYSTEM_OKTOEXIT,(WPARAM)0,(LPARAM)0);
	}
	while (!k);

	if ( k ) {
		DestroyWindow(pcli->hwndContactList);
	}

	return 0;
}

int InitCustomMenus(void)
{
	CreateServiceFunction( "CloseAction", CloseAction );
	LoadFavoriteContactMenu();
	return 0;
}

void UninitCustomMenus(void)
{
	UnloadFavoriteContactMenu();
}

//////////////////////////////////////////////////////////////////////////
//
//  FAVORITE CONTACT SYSTEM
//
//////////////////////////////////////////////////////////////////////////

#define CLUI_FAVSETRATE "CLUI/SetContactRate"  //LParam is rate, Wparam is contact handle
#define CLUI_FAVTOGGLESHOWOFFLINE "CLUI/ToggleContactShowOffline" 

static HANDLE hFavoriteContactMenu=NULL;
static HANDLE *hFavoriteContactMenuItems = NULL;
static HANDLE hShowIfOflineItem=NULL;
static HANDLE hOnContactMenuBuild_FAV=NULL;



static int FAV_OnContactMenuBuild(WPARAM wParam,LPARAM lParam)
{
	CLISTMENUITEM mi;
	BOOL NeedFree=FALSE;
	BYTE bContactRate=ModernGetSettingByte((HANDLE)wParam, "CList", "Rate",0);
	//if (hFavoriteContactMenu)

	static TCHAR * FAVMENUROOTNAME =_T("&Contact rate");

	TCHAR *rates[]={
		_T( "None" ),
		_T( "Low" ),
		_T( "Medium" ),
		_T( "High" ) 
	};

	char* iconsName[]={
		LPGEN( "Contact rate None" ),
		LPGEN( "Contact rate Low" ),
		LPGEN( "Contact rate Medium" ),
		LPGEN( "Contact rate High" )
	};

	if ( bContactRate >SIZEOF( rates ) - 1 )
		 bContactRate = SIZEOF( rates ) - 1;
	//if (hFavoriteContactMenu)
	//    CallService(MO_REMOVEMENUITEM,(WPARAM)hFavoriteContactMenu,0);
	//hFavoriteContactMenu=NULL;
	//if (!hFavoriteContactMenu)
	{
		BOOL bModifyMenu=FALSE;
		int i;
		TCHAR * name=NULL;


		memset(&mi,0,sizeof(mi));
		mi.cbSize=sizeof(mi);
		mi.hIcon=CLUI_LoadIconFromExternalFile("clisticons.dll",8,TRUE,TRUE,iconsName[bContactRate],"Contact List",Translate(iconsName[bContactRate]),-IDI_FAVORITE_0 - bContactRate, &NeedFree);
		// LoadSmallIcon(g_hInst,MAKEINTRESOURCE(IDI_FAVORITE_0 + bContactRate));
		mi.pszPopupName=(char *)-1;
		mi.position=0;
		if (!bContactRate)
			mi.ptszName=FAVMENUROOTNAME;
		else
		{
			int bufsize=(lstrlen(FAVMENUROOTNAME)+lstrlen(rates[bContactRate])+15)*sizeof(TCHAR);
			name=(TCHAR*)_alloca(bufsize);
			mir_sntprintf(name,bufsize/sizeof(TCHAR),_T("%s (%s)"),FAVMENUROOTNAME,rates[bContactRate]);
			mi.ptszName=name;            
		}
		//mi.pszService="ContactRate MenuItem NoService Fake";
		mi.flags=CMIF_ROOTPOPUP|CMIF_TCHAR;
		if (!hFavoriteContactMenu) 
			hFavoriteContactMenu=(HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM)&mi);				
		else
		{
			mi.flags|=CMIM_FLAGS|CMIM_ICON|CMIM_NAME;
			CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM) hFavoriteContactMenu, (LPARAM)&mi);				
			bModifyMenu=TRUE;
		}
		{
			OptParam op;
			op.Handle	= hFavoriteContactMenu;
			op.Setting	= OPT_MENUITEMSETUNIQNAME;
			op.Value	= (INT_PTR)"ModernClistMenu_ContactRate";
			CallService(MO_SETOPTIONSMENUITEM,(WPARAM)0,(LPARAM)&op);
		}

		CallService(MS_SKIN2_RELEASEICON,(WPARAM)mi.hIcon,0);
		if (mi.hIcon && NeedFree) DestroyIcon(mi.hIcon);

		mi.pszPopupName=(char*)hFavoriteContactMenu;
		if (!hFavoriteContactMenuItems)
		{
			hFavoriteContactMenuItems=(HANDLE*)malloc(sizeof(HANDLE)*SIZEOF(rates));
			memset(hFavoriteContactMenuItems,0,sizeof(HANDLE)*SIZEOF(rates));
		}
		for (i=0; i<SIZEOF(rates); i++)
		{
			mi.hIcon=mi.hIcon=CLUI_LoadIconFromExternalFile("clisticons.dll",8+i,TRUE,TRUE,iconsName[i],"Contact List",Translate(iconsName[i]),-IDI_FAVORITE_0 - i, &NeedFree);
			mi.ptszName=rates[i];
			mi.flags=CMIF_CHILDPOPUP|CMIF_TCHAR|((bContactRate==i)?CMIF_CHECKED:0);
			mi.pszService=CLUI_FAVSETRATE;
			mi.popupPosition=i;
			if (bModifyMenu && hFavoriteContactMenuItems[i])
			{
				mi.flags|=CMIM_FLAGS|CMIM_ICON;
				CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM) hFavoriteContactMenuItems[i], (LPARAM)&mi);
			}
			else
				hFavoriteContactMenuItems[i]=(HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM)&mi);
			CallService(MS_SKIN2_RELEASEICON,(WPARAM)mi.hIcon,0);
			if (mi.hIcon && NeedFree) DestroyIcon(mi.hIcon);
		}
		{
			mi.hIcon=NULL;
			mi.ptszName=_T("Show even if offline");
			mi.flags=CMIF_CHILDPOPUP|CMIF_TCHAR|(ModernGetSettingByte((HANDLE)wParam,"CList","noOffline",0)?CMIF_CHECKED:0);
			mi.pszService=CLUI_FAVTOGGLESHOWOFFLINE;
			mi.popupPosition=i+100000000;
			mi.position=-100000000;
			if (bModifyMenu && hShowIfOflineItem)
			{
				mi.flags|=CMIM_FLAGS|CMIM_ICON;
				CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM) hShowIfOflineItem, (LPARAM)&mi);            
			}
			else
				hShowIfOflineItem=(HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM)&mi);            
		}
	}
	return 0;
}

INT_PTR FAV_SetRate(WPARAM hContact, LPARAM nRate)
{
	if (hContact)
	{
		ModernWriteSettingByte((HANDLE)hContact, "CList", "Rate",(BYTE)nRate);
	}
	return 0;
}

INT_PTR FAV_ToggleShowOffline(WPARAM hContact,LPARAM lParam)
{
	if (hContact)
	{
		ModernWriteSettingByte((HANDLE)hContact,"CList","noOffline",
			ModernGetSettingByte((HANDLE)hContact,"CList","noOffline",0)?0:1);
	}
	return 0;
}

int LoadFavoriteContactMenu()
{
	CreateServiceFunction(CLUI_FAVSETRATE,FAV_SetRate);
	CreateServiceFunction(CLUI_FAVTOGGLESHOWOFFLINE,FAV_ToggleShowOffline);
	hOnContactMenuBuild_FAV=ModernHookEvent(ME_CLIST_PREBUILDCONTACTMENU,FAV_OnContactMenuBuild);
	return 0;
}

int UnloadFavoriteContactMenu()
{
	ModernUnhookEvent(hOnContactMenuBuild_FAV);

	if (hFavoriteContactMenuItems)
		free (hFavoriteContactMenuItems);
	hFavoriteContactMenuItems=NULL;

	if (hFavoriteContactMenu)
		CallService(MO_REMOVEMENUITEM,(WPARAM)hFavoriteContactMenu,0);
	hFavoriteContactMenu=NULL;   

	return 0;
}
