/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2003 Miranda ICQ/IM project, 
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
#include "m_clui.h"

#define TIM_CALLBACK   (WM_USER+1857)
#define TIM_CREATE     (WM_USER+1858)

extern INT_PTR ( *saveTrayIconProcessMessage )(WPARAM wParam,LPARAM lParam);
void DestroyTrayMenu(HMENU hMenu);

INT_PTR TrayIconProcessMessage(WPARAM wParam,LPARAM lParam)
{
	MSG *msg=(MSG*)wParam;
	switch(msg->message) {
	case WM_DRAWITEM:
		return CallService(MS_CLIST_MENUDRAWITEM,msg->wParam,msg->lParam);
		break;
	case WM_MEASUREITEM:
		return CallService(MS_CLIST_MENUMEASUREITEM,msg->wParam,msg->lParam);
		break;
	case TIM_CALLBACK:
		if (msg->lParam == WM_RBUTTONUP)
		{
			POINT pt;	
			HMENU hMenu=(HMENU)CallService(MS_CLIST_MENUBUILDTRAY,(WPARAM)0,(LPARAM)0);

			SetForegroundWindow(msg->hwnd);
			SetFocus(msg->hwnd);
			GetCursorPos(&pt);
			TrackPopupMenu(hMenu, TPM_TOPALIGN | TPM_LEFTALIGN, pt.x, pt.y, 0, msg->hwnd, NULL);
            DestroyTrayMenu(hMenu);
		}
		else break;

		*((LRESULT*)lParam)=0;
		return TRUE;
	}
	
	return saveTrayIconProcessMessage( wParam, lParam );
}

//////////////////////////////TRAY MENU/////////////////////////
HANDLE hTrayMenuObject;

HANDLE hTrayMainMenuItemProxy;
HANDLE hHideShowMainMenuItem;
HANDLE hTrayStatusMenuItemProxy;
HANDLE hPreBuildTrayMenuEvent;

//traymenu exec param(ownerdata)
typedef struct{
char *szServiceName;
INT_PTR Param1;
}TrayMenuExecParam,*lpTrayMenuExecParam;

/*
wparam=handle to the menu item returned by MS_CLIST_ADDCONTACTMENUITEM
return 0 on success.
*/
static INT_PTR RemoveTrayMenuItem(WPARAM wParam,LPARAM lParam)
{
	CallService(MO_REMOVEMENUITEM,wParam,0);
	return 0;
}

static INT_PTR BuildTrayMenu(WPARAM wParam,LPARAM lParam)
{
	int tick;
	HMENU hMenu;
	ListParam param = { 0 };
	param.MenuObjectHandle=hTrayMenuObject;

	//hMenu=hMainMenu;
	hMenu=CreatePopupMenu();
	//hMenu=wParam;
	tick=GetTickCount();
	
	NotifyEventHooks(hPreBuildTrayMenuEvent,0,0);

	CallService(MO_BUILDMENU,(WPARAM)hMenu,(LPARAM)&param);
	//DrawMenuBar((HWND)CallService("CLUI/GetHwnd",0,0));
	tick=GetTickCount()-tick;
	return (INT_PTR)hMenu;
}

static INT_PTR AddTrayMenuItem(WPARAM wParam,LPARAM lParam)
{
	CLISTMENUITEM *mi=(CLISTMENUITEM*)lParam;
	TMO_MenuItem tmi;
	OptParam op;

	if(mi->cbSize!=sizeof(CLISTMENUITEM)) return 0;

	memset(&tmi,0,sizeof(tmi));
	tmi.cbSize=sizeof(tmi);
	tmi.flags=mi->flags;
	tmi.hIcon=mi->hIcon;
	tmi.hotKey=mi->hotKey;
	tmi.pszName=mi->pszName;
	tmi.position=mi->position;

	//pszPopupName for new system mean root level
        //pszPopupName for old system mean that exists popup
	tmi.root=(HGENMENU)mi->pszPopupName;

	tmi.ownerdata=NULL;
	
	{
		lpTrayMenuExecParam mmep;
		mmep=(lpTrayMenuExecParam)mir_alloc(sizeof(TrayMenuExecParam));
		if(mmep==NULL){return(0);}
		
		//we need just one parametr.
		mmep->szServiceName = mir_strdup(mi->pszService);
		mmep->Param1 = mi->popupPosition;
		
		tmi.ownerdata=mmep;
	}
	op.Handle=(HANDLE)CallService(MO_ADDNEWMENUITEM,(WPARAM)hTrayMenuObject,(LPARAM)&tmi);
	op.Setting=OPT_MENUITEMSETUNIQNAME;
	op.Value=(INT_PTR)mi->pszService;
	CallService(MO_SETOPTIONSMENUITEM,(WPARAM)0,(LPARAM)&op);
	return (INT_PTR)op.Handle;
}

INT_PTR TrayMenuCheckService(WPARAM wParam,LPARAM lParam) 
{
	return(0);
}

INT_PTR TrayMenuonAddService(WPARAM wParam,LPARAM lParam) 
{
	MENUITEMINFO *mii=(MENUITEMINFO* )wParam;
	if (mii==NULL) return 0;

	if (hHideShowMainMenuItem==(HANDLE)lParam)
	{
		mii->fMask|=MIIM_STATE;
		mii->fState|=MFS_DEFAULT;

	}
	if (hTrayMainMenuItemProxy==(HANDLE)lParam)
	{
				mii->fMask|=MIIM_SUBMENU;
				//mi.fType=MFT_STRING;
				mii->hSubMenu=(HMENU)CallService(MS_CLIST_MENUGETMAIN,0,0);		
	}

	if (hTrayStatusMenuItemProxy==(HANDLE)lParam)
	{
				mii->fMask|=MIIM_SUBMENU;
				//mi.fType=MFT_STRING;
				mii->hSubMenu=(HMENU)CallService(MS_CLIST_MENUGETSTATUS,0,0);		
	}
	
	return(TRUE);
}


//called with:
//wparam - ownerdata
//lparam - lparam from winproc
INT_PTR TrayMenuExecService(WPARAM wParam,LPARAM lParam) {
	if (wParam!=0)
	{
		lpTrayMenuExecParam mmep=(lpTrayMenuExecParam)wParam;	
		if (!strcmp(mmep->szServiceName,"Help/AboutCommand"))
		{
			//bug in help.c,it used wparam as parent window handle without reason.
			mmep->Param1=0;
		}
		CallService(mmep->szServiceName,mmep->Param1,lParam);	
	}
	return(1);
}

INT_PTR FreeOwnerDataTrayMenu (WPARAM wParam,LPARAM lParam)
{

	lpTrayMenuExecParam mmep;
	mmep=(lpTrayMenuExecParam)lParam;
	if (mmep!=NULL){
		FreeAndNil(&mmep->szServiceName);
		FreeAndNil(&mmep);
	}

	return(0);
}

HANDLE hPreBuildTrayMenuEvent;

void InitTrayMenus(void)
{
	TMenuParam tmp;
	OptParam op;

	//Tray menu
	memset(&tmp,0,sizeof(tmp));
	tmp.cbSize=sizeof(tmp);
	tmp.CheckService=NULL;
	tmp.ExecService="CLISTMENUSTRAY/ExecService";
	tmp.name="Tray Menu";
	hTrayMenuObject=(HANDLE)CallService(MO_CREATENEWMENUOBJECT,(WPARAM)0,(LPARAM)&tmp);
	
	CreateServiceFunction("CLISTMENUSTRAY/ExecService",TrayMenuExecService);
	CreateServiceFunction("CLISTMENUSTRAY/FreeOwnerDataTrayMenu",FreeOwnerDataTrayMenu);
	CreateServiceFunction("CLISTMENUSTRAY/TrayMenuonAddService",TrayMenuonAddService);

	CreateServiceFunction(MS_CLIST_ADDTRAYMENUITEM,AddTrayMenuItem);
	CreateServiceFunction(MS_CLIST_REMOVETRAYMENUITEM,RemoveTrayMenuItem);
	CreateServiceFunction(MS_CLIST_MENUBUILDTRAY,BuildTrayMenu);
	hPreBuildTrayMenuEvent=CreateHookableEvent(ME_CLIST_PREBUILDTRAYMENU);
		
	op.Handle=hTrayMenuObject;
	op.Setting=OPT_USERDEFINEDITEMS;
	op.Value=TRUE;
	CallService(MO_SETOPTIONSMENUOBJECT,(WPARAM)0,(LPARAM)&op);
	
	op.Handle=hTrayMenuObject;
	op.Setting=OPT_MENUOBJECT_SET_FREE_SERVICE;
	op.Value=(INT_PTR)"CLISTMENUSTRAY/FreeOwnerDataTrayMenu";
	CallService(MO_SETOPTIONSMENUOBJECT,(WPARAM)0,(LPARAM)&op);

	op.Handle=hTrayMenuObject;
	op.Setting=OPT_MENUOBJECT_SET_ONADD_SERVICE;
	op.Value=(INT_PTR)"CLISTMENUSTRAY/TrayMenuonAddService";
	CallService(MO_SETOPTIONSMENUOBJECT,(WPARAM)0,(LPARAM)&op);

	{	
		//add  exit command to menu
		CLISTMENUITEM mi;

		memset(&mi,0,sizeof(mi));
		mi.cbSize=sizeof(mi);
		mi.position=900000;
		mi.pszService="CloseAction";
		mi.pszName=LPGEN("E&xit");
		AddTrayMenuItem((WPARAM)0,(LPARAM)&mi);

		memset(&mi,0,sizeof(mi));
		mi.cbSize=sizeof(mi);
		mi.position=100000;
		mi.pszService=MS_CLIST_SHOWHIDE;
		mi.pszName=LPGEN("&Hide/Show");
		hHideShowMainMenuItem=(HANDLE)AddTrayMenuItem((WPARAM)0,(LPARAM)&mi);

		memset(&mi,0,sizeof(mi));
		mi.cbSize=sizeof(mi);
		mi.position=200000;
		mi.hIcon=LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_FINDUSER));
		mi.pszService="FindAdd/FindAddCommand";
		mi.pszName=LPGEN("&Find/Add Contacts...");
		AddTrayMenuItem((WPARAM)0,(LPARAM)&mi);
	
		memset(&mi,0,sizeof(mi));
		mi.cbSize=sizeof(mi);
		mi.position=300000;
		mi.pszService="";
		mi.pszName=LPGEN("&Main Menu");
		hTrayMainMenuItemProxy=(HANDLE)AddTrayMenuItem((WPARAM)0,(LPARAM)&mi);

		memset(&mi,0,sizeof(mi));
		mi.cbSize=sizeof(mi);
		mi.position=300100;
		mi.pszService="";
		mi.pszName=LPGEN("&Status");
		hTrayStatusMenuItemProxy=(HANDLE)AddTrayMenuItem((WPARAM)0,(LPARAM)&mi);

		memset(&mi,0,sizeof(mi));
		mi.cbSize=sizeof(mi);
		mi.position=400000;
		mi.hIcon=LoadIcon(GetModuleHandle(NULL),MAKEINTRESOURCE(IDI_OPTIONS));
		mi.pszService="Options/OptionsCommand";
		mi.pszName=LPGEN("&Options...");
		AddTrayMenuItem((WPARAM)0,(LPARAM)&mi);

		memset(&mi,0,sizeof(mi));
		mi.cbSize=sizeof(mi);
		mi.position=500000;
		mi.hIcon=LoadIcon(GetModuleHandle(NULL),MAKEINTRESOURCE(IDI_CLIENTMIRANDA));
		mi.pszService="Help/AboutCommand";
		mi.pszName=LPGEN("&About");
		AddTrayMenuItem((WPARAM)0,(LPARAM)&mi);
}	}
