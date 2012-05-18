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
#include <commonheaders.h>

//////////////////////////////Group MENU/////////////////////////
HANDLE hGroupMenuObject;

HANDLE hGroupMainMenuItemProxy;
HANDLE hHideShowMainMenuItem;
HANDLE hGroupStatusMenuItemProxy;
HANDLE hPreBuildGroupMenuEvent;
HANDLE hAppearanceMenuItemProxy;
HANDLE hEventAreaMenuItemProxy;

static HMENU hMenuOldContext;

HANDLE hHideOfflineUsersMenuItem;
HANDLE hHideOfflineUsersOutHereMenuItem;
HANDLE hHideEmptyGroupsMenuItem;
HANDLE hDisableGroupsMenuItem;
HANDLE hNewGroupMenuItem;
HANDLE hNewSubGroupMenuItem;

int NewGroupIconidx;

extern HIMAGELIST hCListImages;
extern HICON LoadIconFromExternalFile (char *filename,int i,boolean UseLibrary,boolean registerit,char *IconName,char *SectName,char *Description,int internalidx);

void InitSubGroupMenus(void);

//Groupmenu exec param(ownerdata)
typedef struct {
	char *szServiceName;
	int Param1,Param2;
}GroupMenuExecParam,*lpGroupMenuExecParam;

/*
wparam=handle to the menu item returned by MS_CLIST_ADDCONTACTMENUITEM
return 0 on success.
*/
static INT_PTR RemoveGroupMenuItem(WPARAM wParam,LPARAM lParam)
{
	CallService(MO_REMOVEMENUITEM,wParam,0);
	return 0;
}


static INT_PTR BuildGroupMenu(WPARAM wParam,LPARAM lParam)
{
	int tick;
	HMENU hMenu;
	ListParam param = { 0 };
	param.MenuObjectHandle=hGroupMenuObject;

	//hMenu=hMainMenu;
	hMenu=CreatePopupMenu();
	//hMenu=wParam;
	tick=GetTickCount();

	NotifyEventHooks(hPreBuildGroupMenuEvent,0,0);

	CallService(MO_BUILDMENU,(WPARAM)hMenu,(LPARAM)&param);
	//DrawMenuBar((HWND)CallService("CLUI/GetHwnd",0,0));
	tick=GetTickCount()-tick;
	return(INT_PTR)hMenu;
}

static INT_PTR AddGroupMenuItem(WPARAM wParam,LPARAM lParam)
{
	CLISTMENUITEM *mi=(CLISTMENUITEM*)lParam;
	lpGroupMenuParam gmp=(lpGroupMenuParam)wParam;
	char buf[1024];

	TMO_MenuItem tmi;
	OptParam op;

	if (mi->cbSize!=sizeof(CLISTMENUITEM)) return 0;

	memset(&tmi,0,sizeof(tmi));
	tmi.cbSize=sizeof(tmi);
	tmi.flags=mi->flags;
	tmi.hIcon=mi->hIcon;
	tmi.hotKey=mi->hotKey;
	tmi.pszName=mi->pszName;
	tmi.position=mi->position;

	//pszPopupName for new system mean root level
	//pszPopupName for old system mean that exists popup
	tmi.root = mi->hParentMenu;

	tmi.ownerdata=NULL;

	{
		lpGroupMenuExecParam mmep;
		mmep=(lpGroupMenuExecParam)mir_alloc(sizeof(GroupMenuExecParam));
		if (mmep==NULL) {
			return(0);
		};

		//we need just one parametr.
		mmep->szServiceName=mir_strdup(mi->pszService);
		mmep->Param1=mi->popupPosition;
		if (gmp!=NULL) {
			mmep->Param1=gmp->wParam;
			mmep->Param2=gmp->lParam;

		}


		tmi.ownerdata=mmep;
	}
	op.Handle=(HANDLE)CallService(MO_ADDNEWMENUITEM,(WPARAM)hGroupMenuObject,(LPARAM)&tmi);
	op.Setting=OPT_MENUITEMSETUNIQNAME;
	sprintf(buf,"%s/%s",mi->pszService,mi->pszName);
	op.Value=(INT_PTR)buf;
	CallService(MO_SETOPTIONSMENUITEM,(WPARAM)0,(LPARAM)&op);
	return (INT_PTR)op.Handle;

	//	mainItemCount++;
	//	return MENU_CUSTOMITEMMAIN|(mainMenuItem[mainItemCount-1].id);
}

INT_PTR GroupMenuCheckService(WPARAM wParam,LPARAM lParam) {
	//not used
	return(0);
};

INT_PTR GroupMenuonAddService(WPARAM wParam,LPARAM lParam) {

	MENUITEMINFO *mii=(MENUITEMINFO* )wParam;
	if (mii==NULL) return 0;

	if (hHideShowMainMenuItem==(HANDLE)lParam) {
		mii->fMask|=MIIM_STATE;
		mii->fState|=MFS_DEFAULT;

	}
	if (hGroupMainMenuItemProxy==(HANDLE)lParam) {
		mii->fMask|=MIIM_SUBMENU;
		//mi.fType=MFT_STRING;
		mii->hSubMenu=(HMENU)CallService(MS_CLIST_MENUGETMAIN,0,0);
	}

	if (hGroupStatusMenuItemProxy==(HANDLE)lParam) {
		mii->fMask|=MIIM_SUBMENU;
		//mi.fType=MFT_STRING;
		mii->hSubMenu=(HMENU)CallService(MS_CLIST_MENUGETSTATUS,0,0);
	}
	if (hAppearanceMenuItemProxy==(HANDLE)lParam) {
		hMenuOldContext = GetSubMenu(LoadMenu(g_hInst, MAKEINTRESOURCE(IDR_CONTEXT)), 4);
		CallService(MS_LANGPACK_TRANSLATEMENU, (WPARAM) hMenuOldContext, 0);

		CheckMenuItem(hMenuOldContext, POPUP_VISIBILITY, MF_BYCOMMAND | (cfg::dat.dwFlags & CLUI_SHOWVISI ? MF_CHECKED : MF_UNCHECKED));
		CheckMenuItem(hMenuOldContext, POPUP_FRAME, MF_BYCOMMAND | (cfg::dat.dwFlags & CLUI_FRAME_CLISTSUNKEN ? MF_CHECKED : MF_UNCHECKED));
		CheckMenuItem(hMenuOldContext, POPUP_TOOLBAR, MF_BYCOMMAND | (cfg::dat.dwFlags & CLUI_FRAME_SHOWTOPBUTTONS ? MF_CHECKED : MF_UNCHECKED));
		CheckMenuItem(hMenuOldContext, POPUP_BUTTONS, MF_BYCOMMAND | (cfg::dat.dwFlags & CLUI_FRAME_SHOWBOTTOMBUTTONS ? MF_CHECKED : MF_UNCHECKED));
		CheckMenuItem(hMenuOldContext, POPUP_SHOWMETAICONS, MF_BYCOMMAND | (cfg::dat.dwFlags & CLUI_USEMETAICONS ? MF_CHECKED : MF_UNCHECKED));
		CheckMenuItem(hMenuOldContext, POPUP_SHOWSTATUSICONS, MF_BYCOMMAND | (cfg::dat.dwFlags & CLUI_FRAME_STATUSICONS ? MF_CHECKED : MF_UNCHECKED));

		// floater menu items

		EnableMenuItem(hMenuOldContext, POPUP_FLOATER_AUTOHIDE, MF_BYCOMMAND | (cfg::dat.bUseFloater & CLUI_USE_FLOATER ? MF_ENABLED : MF_GRAYED));
		EnableMenuItem(hMenuOldContext, POPUP_FLOATER_EVENTS, MF_BYCOMMAND | (cfg::dat.bUseFloater & CLUI_USE_FLOATER ? MF_ENABLED : MF_GRAYED));
		CheckMenuItem(hMenuOldContext, POPUP_FLOATER, MF_BYCOMMAND | (cfg::dat.bUseFloater & CLUI_USE_FLOATER ? MF_CHECKED : MF_UNCHECKED));
		CheckMenuItem(hMenuOldContext, POPUP_FLOATER_AUTOHIDE, MF_BYCOMMAND | (cfg::dat.bUseFloater & CLUI_FLOATER_AUTOHIDE ? MF_CHECKED : MF_UNCHECKED));
		CheckMenuItem(hMenuOldContext, POPUP_FLOATER_EVENTS, MF_BYCOMMAND | (cfg::dat.bUseFloater & CLUI_FLOATER_EVENTS ? MF_CHECKED : MF_UNCHECKED));

		mii->fMask|=MIIM_SUBMENU;
		//mi.fType=MFT_STRING;
		mii->hSubMenu=(HMENU)hMenuOldContext;
	}
	return(TRUE);
};


//called with:
//wparam - ownerdata
//lparam - lparam from winproc
INT_PTR GroupMenuExecService(WPARAM wParam,LPARAM lParam) {
	if (wParam!=0) {
		lpGroupMenuExecParam mmep=(lpGroupMenuExecParam)wParam;
		if (!strcmp(mmep->szServiceName,"Help/AboutCommand")) {
			//bug in help.c,it used wparam as parent window handle without reason.
			mmep->Param1=0;
			CallService(mmep->szServiceName,mmep->Param1,lParam);
		}
		else {
			CallService(mmep->szServiceName,mmep->Param1,mmep->Param2);
		}

	};
	return(1);
};
INT_PTR FreeOwnerDataGroupMenu (WPARAM wParam,LPARAM lParam)
{

	lpGroupMenuExecParam mmep;
	mmep=(lpGroupMenuExecParam)lParam;
	if (mmep!=NULL) {
		FreeAndNil((void **)&mmep->szServiceName);
		FreeAndNil((void **)&mmep);
	}

	return(0);
};

INT_PTR HideGroupsHelper(WPARAM wParam,LPARAM lParam)
{
	int newVal=!(GetWindowLong((HWND)CallService(MS_CLUI_GETHWNDTREE,0,0),GWL_STYLE)&CLS_HIDEEMPTYGROUPS);
	cfg::writeByte("CList","HideEmptyGroups",(BYTE)newVal);
	SendMessage((HWND)CallService(MS_CLUI_GETHWNDTREE,0,0),CLM_SETHIDEEMPTYGROUPS,newVal,0);
	return 0;
}

INT_PTR UseGroupsHelper(WPARAM wParam,LPARAM lParam)
{
	int newVal=!(GetWindowLong((HWND)CallService(MS_CLUI_GETHWNDTREE,0,0),GWL_STYLE)&CLS_USEGROUPS);
	cfg::writeByte("CList","UseGroups",(BYTE)newVal);
	SendMessage((HWND)CallService(MS_CLUI_GETHWNDTREE,0,0),CLM_SETUSEGROUPS,newVal,0);
	return 0;
}

INT_PTR HideOfflineRootHelper(WPARAM wParam,LPARAM lParam)
{
	SendMessage(
		(HWND)CallService(MS_CLUI_GETHWNDTREE,0,0),
		CLM_SETHIDEOFFLINEROOT,
		!SendMessage((HWND)CallService(MS_CLUI_GETHWNDTREE,0,0),CLM_GETHIDEOFFLINEROOT,0,0),
		0);
	return 0;
};

static int OnBuildGroupMenu(WPARAM wParam,LPARAM lParam)
{
	CLISTMENUITEM mi;


	ZeroMemory(&mi,sizeof(mi));
	mi.cbSize = sizeof(mi);
	mi.flags = CMIM_FLAGS | (cfg::getByte("CList","HideOffline",SETTING_HIDEOFFLINE_DEFAULT)?CMIF_CHECKED:0);
	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hHideOfflineUsersMenuItem, (LPARAM)&mi);

	ZeroMemory(&mi,sizeof(mi));
	mi.cbSize = sizeof(mi);
	mi.flags = CMIM_FLAGS | (SendMessage(pcli->hwndContactTree,CLM_GETHIDEOFFLINEROOT,0,0)?CMIF_CHECKED:0);
	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hHideOfflineUsersOutHereMenuItem, (LPARAM)&mi);

	ZeroMemory(&mi,sizeof(mi));
	mi.cbSize = sizeof(mi);
	mi.flags = CMIM_FLAGS | (GetWindowLong(pcli->hwndContactTree,GWL_STYLE)&CLS_HIDEEMPTYGROUPS?CMIF_CHECKED:0);
	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hHideEmptyGroupsMenuItem, (LPARAM)&mi);

	ZeroMemory(&mi,sizeof(mi));
	mi.cbSize = sizeof(mi);
	mi.flags = CMIM_FLAGS | (GetWindowLong(pcli->hwndContactTree,GWL_STYLE)&CLS_USEGROUPS?0:CMIF_CHECKED);
	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hDisableGroupsMenuItem, (LPARAM)&mi);

	ZeroMemory(&mi,sizeof(mi));
	mi.cbSize = sizeof(mi);
	mi.flags = CMIM_FLAGS;
	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hGroupMainMenuItemProxy, (LPARAM)&mi);

	ZeroMemory(&mi,sizeof(mi));
	mi.cbSize = sizeof(mi);
	mi.flags = CMIM_FLAGS;
	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hAppearanceMenuItemProxy, (LPARAM)&mi);

	return 0;
};

int static OnIconLibIconChanged(WPARAM wParam,LPARAM lParam)
{
	HICON hicon;
	CLISTMENUITEM clmi={0};

	//hicon=LoadIconFromExternalFile("clisticons.dll",2,TRUE,FALSE,"NewGroup","Contact List","New Group",-IDI_NEWGROUP);
	NewGroupIconidx=ImageList_ReplaceIcon(hCListImages,NewGroupIconidx,hicon);

	clmi.cbSize=sizeof(clmi);
	clmi.flags=CMIM_ICON;
	clmi.hIcon=ImageList_GetIcon(hCListImages,NewGroupIconidx,0);
	CallService(MS_CLIST_MODIFYMENUITEM,(WPARAM)hNewSubGroupMenuItem,(LPARAM)&clmi);
	CallService(MS_CLIST_MODIFYMENUITEM,(WPARAM)hNewGroupMenuItem,(LPARAM)&clmi);
	//


	return 0;
};

void InitGroupMenus(void)
{
	TMenuParam tmp;
	OptParam op;
	//hicon=LoadIconFromExternalFile("clisticons.dll",2,TRUE,TRUE,"NewGroup","Contact List","New Group",-IDI_NEWGROUP2);
	//NewGroupIconidx=ImageList_AddIcon(hCListImages,hicon );

	CreateServiceFunction("CLISTMENUSGroup/ExecService",GroupMenuExecService);
	CreateServiceFunction("CLISTMENUSGroup/FreeOwnerDataGroupMenu",FreeOwnerDataGroupMenu);
	CreateServiceFunction("CLISTMENUSGroup/GroupMenuonAddService",GroupMenuonAddService);
	CreateServiceFunction("CLISTMENUSGroup/HideGroupsHelper",HideGroupsHelper);
	CreateServiceFunction("CLISTMENUSGroup/UseGroupsHelper",UseGroupsHelper);
	CreateServiceFunction("CLISTMENUSGroup/HideOfflineRootHelper",HideOfflineRootHelper);

	CreateServiceFunction(MS_CLIST_ADDGROUPMENUITEM,AddGroupMenuItem);
	CreateServiceFunction(MS_CLIST_REMOVEGROUPMENUITEM,RemoveGroupMenuItem);
	CreateServiceFunction(MS_CLIST_MENUBUILDGROUP,BuildGroupMenu);
	hPreBuildGroupMenuEvent=CreateHookableEvent(ME_CLIST_PREBUILDGROUPMENU);
	HookEvent(ME_CLIST_PREBUILDGROUPMENU,OnBuildGroupMenu);

	InitSubGroupMenus();

	//Group menu
	memset(&tmp,0,sizeof(tmp));
	tmp.cbSize=sizeof(tmp);
	tmp.CheckService=NULL;
	tmp.ExecService="CLISTMENUSGroup/ExecService";
	tmp.name="GroupMenu";
	hGroupMenuObject=(HANDLE)CallService(MO_CREATENEWMENUOBJECT,(WPARAM)0,(LPARAM)&tmp);


	op.Handle=hGroupMenuObject;
	op.Setting=OPT_USERDEFINEDITEMS;
	op.Value=TRUE;
	CallService(MO_SETOPTIONSMENUOBJECT,(WPARAM)0,(LPARAM)&op);

	op.Handle=hGroupMenuObject;
	op.Setting=OPT_MENUOBJECT_SET_FREE_SERVICE;
	op.Value=(INT_PTR)"CLISTMENUSGroup/FreeOwnerDataGroupMenu";
	CallService(MO_SETOPTIONSMENUOBJECT,(WPARAM)0,(LPARAM)&op);

	op.Handle=hGroupMenuObject;
	op.Setting=OPT_MENUOBJECT_SET_ONADD_SERVICE;
	op.Value=(INT_PTR)"CLISTMENUSGroup/GroupMenuonAddService";
	CallService(MO_SETOPTIONSMENUOBJECT,(WPARAM)0,(LPARAM)&op);

	{
		//add  exit command to menu
		CLISTMENUITEM mi;
		GroupMenuParam gmp;

		memset(&mi,0,sizeof(mi));
		mi.cbSize=sizeof(mi);
		mi.position=1900000;
		mi.pszService="CloseAction";
		mi.pszName=LPGEN("E&xit");
		AddGroupMenuItem((WPARAM)0,(LPARAM)&mi);

		memset(&mi,0,sizeof(mi));
		mi.cbSize=sizeof(mi);
		mi.position=500;
		mi.pszService=MS_CLIST_SHOWHIDE;
		mi.pszName=LPGEN("&Hide/Show");
		hHideShowMainMenuItem=(HANDLE)AddGroupMenuItem((WPARAM)0,(LPARAM)&mi);

		memset(&mi,0,sizeof(mi));
		mi.cbSize=sizeof(mi);
		mi.position=200000;
		mi.flags = CMIF_ICONFROMICOLIB;
		mi.icolibItem=LoadSkinnedIconHandle(SKINICON_OTHER_FINDUSER); //LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_FINDUSER));
		mi.pszService="FindAdd/FindAddCommand";
		mi.pszName=LPGEN("&Find/Add Contacts...");
		AddGroupMenuItem((WPARAM)0,(LPARAM)&mi);

		memset(&mi,0,sizeof(mi));
		mi.cbSize=sizeof(mi);
		mi.position=300000;
		mi.pszService="";
		mi.pszName=LPGEN("&Main Menu");
		hGroupMainMenuItemProxy=(HANDLE)AddGroupMenuItem((WPARAM)0,(LPARAM)&mi);

		memset(&mi,0,sizeof(mi));
		mi.cbSize=sizeof(mi);
		mi.position=300100;
		mi.pszService="";
		mi.pszName=LPGEN("&Status");
		hGroupStatusMenuItemProxy=(HANDLE)AddGroupMenuItem((WPARAM)0,(LPARAM)&mi);

		memset(&mi,0,sizeof(mi));
		mi.cbSize=sizeof(mi);
		mi.position=390100;
		mi.pszService="";
		mi.pszName=LPGEN("Appearance");
		hAppearanceMenuItemProxy=(HANDLE)AddGroupMenuItem((WPARAM)0,(LPARAM)&mi);

		memset(&mi,0,sizeof(mi));
		mi.cbSize=sizeof(mi);
		mi.position=400000;
		mi.flags = CMIF_ICONFROMICOLIB;
		mi.icolibItem=LoadSkinnedIconHandle(SKINICON_OTHER_OPTIONS); //LoadIcon(GetModuleHandle(NULL),MAKEINTRESOURCE(IDI_OPTIONS));
		mi.pszService="Options/OptionsCommand";
		mi.pszName=LPGEN("&Options...");

		AddGroupMenuItem((WPARAM)0,(LPARAM)&mi);


		memset(&mi,0,sizeof(mi));
		mi.cbSize=sizeof(mi);
		mi.position=500000;
		mi.flags = CMIF_ICONFROMICOLIB;
		mi.icolibItem=LoadSkinnedIconHandle(SKINICON_OTHER_MIRANDA); //LoadIcon(GetModuleHandle(NULL),MAKEINTRESOURCE(IDI_MIRANDA));
		mi.pszService="CLN/About";
		mi.pszName=LPGEN("&About the contact list...");

		AddGroupMenuItem((WPARAM)0,(LPARAM)&mi);

		memset(&mi,0,sizeof(mi));
		mi.cbSize=sizeof(mi);
		mi.position=100000;
		mi.hIcon=ImageList_GetIcon(hCListImages,NewGroupIconidx,0);
		//mi.pszService=MS_CLIST_GROUPCREATE;
		mi.pszService="CLISTMENUSSubGroup/GroupMenuExecProxy";
		//mi.pszService="CLISTMENUSGroup/ExecService";
		mi.pszName=LPGEN("&New Group");
		gmp.lParam=0;gmp.wParam=POPUP_NEWGROUP;
		hNewGroupMenuItem=(HANDLE)AddGroupMenuItem((WPARAM)&gmp,(LPARAM)&mi);

		memset(&mi,0,sizeof(mi));
		mi.cbSize=sizeof(mi);
		mi.position=100001;
		mi.hIcon=NULL;
		mi.pszService=MS_CLIST_SETHIDEOFFLINE;
		mi.pszName=LPGEN("&Hide Offline Users");
		gmp.lParam=0;gmp.wParam=-1;
		hHideOfflineUsersMenuItem=(HANDLE)AddGroupMenuItem((WPARAM)&gmp,(LPARAM)&mi);

		memset(&mi,0,sizeof(mi));
		mi.cbSize=sizeof(mi);
		mi.position=100002;
		mi.hIcon=NULL;
		mi.pszService="CLISTMENUSGroup/HideOfflineRootHelper";
		mi.pszName=LPGEN("Hide &Offline Users out here");
		hHideOfflineUsersOutHereMenuItem=(HANDLE)AddGroupMenuItem((WPARAM)0,(LPARAM)&mi);


		memset(&mi,0,sizeof(mi));
		mi.cbSize=sizeof(mi);
		mi.position=100003;
		mi.hIcon=NULL;
		mi.pszService="CLISTMENUSGroup/HideGroupsHelper";
		mi.pszName=LPGEN("Hide &Empty Groups");
		hHideEmptyGroupsMenuItem=(HANDLE)AddGroupMenuItem((WPARAM)0,(LPARAM)&mi);



		memset(&mi,0,sizeof(mi));
		mi.cbSize=sizeof(mi);
		mi.position=100004;
		mi.hIcon=NULL;
		mi.pszService="CLISTMENUSGroup/UseGroupsHelper";
		mi.pszName=LPGEN("Disable &Groups");
		hDisableGroupsMenuItem=(HANDLE)AddGroupMenuItem((WPARAM)0,(LPARAM)&mi);


		//MS_CLIST_GROUPCREATE

	};
}

HANDLE hSubGroupMenuObject;

HANDLE hSubGroupMainMenuItemProxy;
//HANDLE hHideShowMainMenuItem;
HANDLE hSubGroupStatusMenuItemProxy;
HANDLE hPreBuildSubGroupMenuEvent;
HANDLE hHideOfflineUsersHereMenuItem;

//SubGroupmenu exec param(ownerdata)
typedef struct {
	char *szServiceName;
	int Param1,Param2;
}SubGroupMenuExecParam,*lpSubGroupMenuExecParam;

/*
wparam=handle to the menu item returned by MS_CLIST_ADDCONTACTMENUITEM
return 0 on success.
*/
static INT_PTR RemoveSubGroupMenuItem(WPARAM wParam,LPARAM lParam)
{
	CallService(MO_REMOVEMENUITEM,wParam,0);
	return 0;
}

static int OnBuildSubGroupMenu(WPARAM wParam,LPARAM lParam)
{
	CLISTMENUITEM mi;
	struct ClcGroup *group=(struct ClcGroup *)wParam;
	if (wParam==0) return 0;


	//contact->group
	ZeroMemory(&mi,sizeof(mi));
	mi.cbSize = sizeof(mi);
	mi.flags = CMIM_FLAGS | (group->hideOffline?CMIF_CHECKED:0);
	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hHideOfflineUsersHereMenuItem, (LPARAM)&mi);

	return 0;
};
static INT_PTR BuildSubGroupMenu(WPARAM wParam,LPARAM lParam)
{
	int tick;
	HMENU hMenu;
	ListParam param = { 0 };
	param.MenuObjectHandle=hSubGroupMenuObject;
	param.wParam=wParam;

	//hMenu=hMainMenu;
	hMenu=CreatePopupMenu();
	//hMenu=wParam;
	tick=GetTickCount();
	NotifyEventHooks(hPreBuildSubGroupMenuEvent,wParam,0);

	CallService(MO_BUILDMENU,(WPARAM)hMenu,(LPARAM)&param);
	//DrawMenuBar((HWND)CallService("CLUI/GetHwnd",0,0));
	tick=GetTickCount()-tick;
	return(INT_PTR)hMenu;
}

static INT_PTR AddSubGroupMenuItem(WPARAM wParam,LPARAM lParam)
{
	CLISTMENUITEM *mi=(CLISTMENUITEM*)lParam;
	lpGroupMenuParam gmp=(lpGroupMenuParam)wParam;
	char buf[1024];
	TMO_MenuItem tmi;
	OptParam op;

	if (mi->cbSize!=sizeof(CLISTMENUITEM)) return 0;
	memset(&tmi,0,sizeof(tmi));
	tmi.cbSize=sizeof(tmi);
	tmi.flags=mi->flags;
	tmi.hIcon=mi->hIcon;
	tmi.hotKey=mi->hotKey;
	tmi.pszName=mi->pszName;
	tmi.position=mi->position;

	//pszPopupName for new system mean root level
	//pszPopupName for old system mean that exists popup
	tmi.root = mi->hParentMenu;

	tmi.ownerdata=NULL;

	{
		lpSubGroupMenuExecParam mmep;
		mmep=(lpSubGroupMenuExecParam)mir_alloc(sizeof(SubGroupMenuExecParam));
		if (mmep==NULL)
			return(0);

		//we need just one parametr.
		mmep->szServiceName=mir_strdup(mi->pszService);
		mmep->Param1=mi->popupPosition;
		if (gmp!=NULL) {
			mmep->Param1=gmp->wParam;
			mmep->Param2=gmp->lParam;
		}

		tmi.ownerdata=mmep;
	}

	op.Handle=(HANDLE)CallService(MO_ADDNEWMENUITEM,(WPARAM)hSubGroupMenuObject,(LPARAM)&tmi);
	op.Setting=OPT_MENUITEMSETUNIQNAME;
	sprintf(buf,"%s/%s",mi->pszService,mi->pszName);
	op.Value=(INT_PTR)buf;
	CallService(MO_SETOPTIONSMENUITEM,(WPARAM)0,(LPARAM)&op);
	return (INT_PTR)op.Handle;

	//	mainItemCount++;
	//	return MENU_CUSTOMITEMMAIN|(mainMenuItem[mainItemCount-1].id);
}

INT_PTR SubGroupMenuCheckService(WPARAM wParam,LPARAM lParam) {
	//not used
	return(0);
};

INT_PTR SubGroupMenuonAddService(WPARAM wParam,LPARAM lParam) {

	MENUITEMINFO *mii=(MENUITEMINFO* )wParam;

	if (mii==NULL) return 0;

	/*
	if (hHideShowMainMenuItem==(HANDLE)lParam)
	{
	mii->fMask|=MIIM_STATE;
	mii->fState|=MFS_DEFAULT;

	}
	if (hSubGroupMainMenuItemProxy==(HANDLE)lParam)
	{
	mii->fMask|=MIIM_SUBMENU;
	//mi.fType=MFT_STRING;
	mii->hSubMenu=(HMENU)CallService(MS_CLIST_MENUGETMAIN,0,0);
	}

	if (hSubGroupStatusMenuItemProxy==(HANDLE)lParam)
	{
	mii->fMask|=MIIM_SUBMENU;
	//mi.fType=MFT_STRING;
	mii->hSubMenu=(HMENU)CallService(MS_CLIST_MENUGETSTATUS,0,0);
	}
	*/
	return(TRUE);
};


//called with:
//wparam - ownerdata
//lparam - lparam from winproc
INT_PTR SubGroupMenuExecService(WPARAM wParam,LPARAM lParam) {
	if (wParam!=0) {
		lpSubGroupMenuExecParam mmep=(lpSubGroupMenuExecParam)wParam;
		if (!strcmp(mmep->szServiceName,"Help/AboutCommand")) {
			//bug in help.c,it used wparam as parent window handle without reason.
			mmep->Param1=0;
			CallService(mmep->szServiceName,mmep->Param1,lParam);
		}
		else {
			CallService(mmep->szServiceName,mmep->Param1,mmep->Param2);
		}
	}
	return(1);
}

INT_PTR FreeOwnerDataSubGroupMenu (WPARAM wParam,LPARAM lParam)
{
	lpSubGroupMenuExecParam mmep;
	mmep=(lpSubGroupMenuExecParam)lParam;
	if (mmep!=NULL) {
		FreeAndNil((void **)&mmep->szServiceName);
		FreeAndNil((void **)&mmep);
	}

	return(0);
}

//wparam menu handle to pass to clc.c
INT_PTR GroupMenuExecProxy(WPARAM wParam,LPARAM lParam)
{
	SendMessage((HWND)CallService(MS_CLUI_GETHWNDTREE,0,0),WM_COMMAND,wParam,0);
	return 0;
}

void InitSubGroupMenus(void)
{
	TMenuParam tmp;
	OptParam op;

	CreateServiceFunction("CLISTMENUSSubGroup/ExecService",SubGroupMenuExecService);
	CreateServiceFunction("CLISTMENUSSubGroup/FreeOwnerDataSubGroupMenu",FreeOwnerDataSubGroupMenu);
	CreateServiceFunction("CLISTMENUSSubGroup/SubGroupMenuonAddService",SubGroupMenuonAddService);
	CreateServiceFunction("CLISTMENUSSubGroup/GroupMenuExecProxy",GroupMenuExecProxy);

	//CreateServiceFunction("CLISTMENUSSubGroup/HideSubGroupsHelper",HideSubGroupsHelper);
	//CreateServiceFunction("CLISTMENUSSubGroup/UseSubGroupsHelper",UseSubGroupsHelper);
	//CreateServiceFunction("CLISTMENUSSubGroup/HideOfflineRootHelper",HideOfflineRootHelper);

	CreateServiceFunction(MS_CLIST_ADDSUBGROUPMENUITEM,AddSubGroupMenuItem);
	CreateServiceFunction(MS_CLIST_REMOVESUBGROUPMENUITEM,RemoveSubGroupMenuItem);
	CreateServiceFunction(MS_CLIST_MENUBUILDSUBGROUP,BuildSubGroupMenu);
	hPreBuildSubGroupMenuEvent=CreateHookableEvent(ME_CLIST_PREBUILDSUBGROUPMENU);
	HookEvent(ME_CLIST_PREBUILDSUBGROUPMENU,OnBuildSubGroupMenu);


	//SubGroup menu
	memset(&tmp,0,sizeof(tmp));
	tmp.cbSize=sizeof(tmp);
	tmp.CheckService=NULL;
	tmp.ExecService="CLISTMENUSSubGroup/ExecService";
	tmp.name="SubGroupMenu";
	hSubGroupMenuObject=(HANDLE)CallService(MO_CREATENEWMENUOBJECT,(WPARAM)0,(LPARAM)&tmp);


	op.Handle=hSubGroupMenuObject;
	op.Setting=OPT_USERDEFINEDITEMS;
	op.Value=TRUE;
	CallService(MO_SETOPTIONSMENUOBJECT,(WPARAM)0,(LPARAM)&op);

	op.Handle=hSubGroupMenuObject;
	op.Setting=OPT_MENUOBJECT_SET_FREE_SERVICE;
	op.Value=(INT_PTR)"CLISTMENUSSubGroup/FreeOwnerDataSubGroupMenu";
	CallService(MO_SETOPTIONSMENUOBJECT,(WPARAM)0,(LPARAM)&op);

	op.Handle=hSubGroupMenuObject;
	op.Setting=OPT_MENUOBJECT_SET_ONADD_SERVICE;
	op.Value=(INT_PTR)"CLISTMENUSSubGroup/SubGroupMenuonAddService";
	CallService(MO_SETOPTIONSMENUOBJECT,(WPARAM)0,(LPARAM)&op);

	{
		//add  exit command to menu
		CLISTMENUITEM mi;
		GroupMenuParam gmp;

		memset(&mi,0,sizeof(mi));
		mi.cbSize=sizeof(mi);
		mi.position=1000;
		mi.hIcon=ImageList_GetIcon(hCListImages,NewGroupIconidx,0);
		mi.pszService="CLISTMENUSSubGroup/GroupMenuExecProxy";
		mi.pszName=LPGEN("&New SubGroup");
		gmp.lParam=0;gmp.wParam=POPUP_NEWSUBGROUP;
		hNewSubGroupMenuItem=(HANDLE)AddSubGroupMenuItem((WPARAM)&gmp,(LPARAM)&mi);

		memset(&mi,0,sizeof(mi));
		mi.cbSize=sizeof(mi);
		mi.position=1001;
		mi.hIcon=NULL;
		mi.pszService="CLISTMENUSSubGroup/GroupMenuExecProxy";
		mi.pszName=LPGEN("&Hide Offline Users in here");
		gmp.lParam=0;gmp.wParam=POPUP_GROUPHIDEOFFLINE;
		hHideOfflineUsersHereMenuItem=(HANDLE)AddSubGroupMenuItem((WPARAM)&gmp,(LPARAM)&mi);

		memset(&mi,0,sizeof(mi));
		mi.cbSize=sizeof(mi);
		mi.position=900001;
		mi.flags = CMIF_ICONFROMICOLIB;
		mi.icolibItem=LoadSkinnedIconHandle(SKINICON_OTHER_RENAME); //LoadIcon(GetModuleHandle(NULL),MAKEINTRESOURCE(IDI_RENAME));
		mi.pszService="CLISTMENUSSubGroup/GroupMenuExecProxy";
		mi.pszName=LPGEN("&Rename Group");
		gmp.lParam=0;gmp.wParam=POPUP_RENAMEGROUP;
		AddSubGroupMenuItem((WPARAM)&gmp,(LPARAM)&mi);

		memset(&mi,0,sizeof(mi));
		mi.cbSize=sizeof(mi);
		mi.position=900002;
		mi.flags = CMIF_ICONFROMICOLIB;
		mi.icolibItem=LoadSkinnedIconHandle(SKINICON_OTHER_DELETE); //LoadIcon(GetModuleHandle(NULL),MAKEINTRESOURCE(IDI_DELETE));
		mi.pszService="CLISTMENUSSubGroup/GroupMenuExecProxy";
		mi.pszName=LPGEN("&Delete Group");
		gmp.lParam=0;gmp.wParam=POPUP_DELETEGROUP;
		AddSubGroupMenuItem((WPARAM)&gmp,(LPARAM)&mi);
	}
}

//////////////////////////////END SubGroup MENU/////////////////////////
