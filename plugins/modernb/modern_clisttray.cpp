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
#include <m_clui.h>
#include "hdr/modern_clist.h"
#include "hdr/modern_commonprototypes.h"
#include "hdr/modern_statusbar.h"

int cliShowHide(WPARAM wParam,LPARAM lParam);
int g_mutex_bOnTrayRightClick=0;

BOOL g_bMultiConnectionMode=FALSE;
static int RefreshTimerId=0;   /////by FYR
static VOID CALLBACK RefreshTimerProc(HWND hwnd,UINT message,UINT idEvent,DWORD dwTime); ///// by FYR
static HMENU hMainMenu, hStatusMenu;
BOOL IS_WM_MOUSE_DOWN_IN_TRAY;

BOOL g_trayTooltipActive = FALSE;
POINT tray_hover_pos = {0};

// don't move to win2k.h, need new and old versions to work on 9x/2000/XP
#define NIF_STATE       0x00000008
#define NIF_INFO        0x00000010

#ifndef _INC_SHLWAPI

typedef struct _DllVersionInfo {
	DWORD cbSize;
	DWORD dwMajorVersion;                   // Major version
	DWORD dwMinorVersion;                   // Minor version
	DWORD dwBuildNumber;                    // Build number
	DWORD dwPlatformID;                     // DLLVER_PLATFORM_*
} DLLVERSIONINFO;

#define DLLVER_PLATFORM_WINDOWS         0x00000001      // Windows 95
#define DLLVER_PLATFORM_NT              0x00000002      // Windows NT
typedef HRESULT (CALLBACK* DLLGETVERSIONPROC)(DLLVERSIONINFO *);

#endif

void mir_strset(TCHAR ** dest, TCHAR *source)
{
	if (*dest) mir_free_and_nill(*dest);
	if (source) *dest=mir_tstrdup(source);
}

static DLLVERSIONINFO dviShell;
BOOL g_MultiConnectionMode=FALSE;
char * g_szConnectingProto=NULL;
int GetStatusVal(int status)
{
	switch ( status ) {
		case ID_STATUS_OFFLINE:               return 50;
		case ID_STATUS_ONLINE:                return 100;
		case ID_STATUS_FREECHAT:              return 110;
		case ID_STATUS_INVISIBLE:             return 120;
		case ID_STATUS_AWAY:                  return 200;
		case ID_STATUS_DND:					  return 210;
		case ID_STATUS_NA:                    return 220;
		case ID_STATUS_OCCUPIED:              return 230;
		case ID_STATUS_ONTHEPHONE:            return 400;
		case ID_STATUS_OUTTOLUNCH:            return 410;
	}
	if (status<ID_STATUS_OFFLINE && status>0) return 600; //connecting is most priority
	return 0;
}

int GetStatusOrder(int currentStatus, int newStatus)
{
	int current=GetStatusVal(currentStatus);
	int newstat=GetStatusVal(newStatus);
	return (current>newstat)?currentStatus:newStatus;
}

INT_PTR CListTray_GetGlobalStatus(WPARAM wparam,LPARAM lparam)
{
	int curstatus=0;
	int i;
	int connectingCount=0;
	for (i=0;i<pcli->hClcProtoCount;i++)
	{
		if(!pcli->pfnGetProtocolVisibility(pcli->clcProto[i].szProto)) continue;
		if (pcli->clcProto[i].dwStatus>=ID_STATUS_CONNECTING &&
			pcli->clcProto[i].dwStatus<ID_STATUS_CONNECTING+MAX_CONNECT_RETRIES)
		{
			connectingCount++;
			if (connectingCount==1) g_szConnectingProto=pcli->clcProto[i].szProto;
		}
		curstatus=GetStatusOrder(curstatus,pcli->clcProto[i].dwStatus);
	}
	if (connectingCount==0)
	{
		//g_szConnectingProto=NULL;
		g_bMultiConnectionMode=FALSE;
	}
	else if (connectingCount>1)
		g_bMultiConnectionMode=TRUE;
	else
		g_bMultiConnectionMode=FALSE;
	return curstatus?curstatus:ID_STATUS_OFFLINE;
}

int GetAverageMode()
{
	int count,netProtoCount,i;
	int averageMode=0;
	PROTOACCOUNT **accs;
	ProtoEnumAccounts( &count, &accs );
	for(i=0,netProtoCount=0;i<count;i++) {
		if ( pcli->pfnGetProtocolVisibility(accs[i]->szModuleName) == 0 ) continue;
		pcli->cycleStep=i;
		netProtoCount++;
		if(averageMode==0) averageMode=CallProtoService(accs[i]->szModuleName,PS_GETSTATUS,0,0);
		else if(averageMode!=CallProtoService(accs[i]->szModuleName,PS_GETSTATUS,0,0)) {averageMode=-1; break;}
	}
	return averageMode;
}

////////////////////////////////////////////////////////////
///// Need to refresh trays icon  after timely changing/////
////////////////////////////////////////////////////////////

static VOID CALLBACK RefreshTimerProc(HWND hwnd,UINT message,UINT idEvent,DWORD dwTime)
{
	int count,i;
	PROTOACCOUNT **accs;

	if(RefreshTimerId) {KillTimer(NULL,RefreshTimerId); RefreshTimerId=0;}

	ProtoEnumAccounts( &count, &accs );
	for (i=0; i<count; i++)
		if ( pcli->pfnGetProtocolVisibility(accs[i]->szModuleName))
			pcli->pfnTrayIconUpdateBase(accs[i]->szModuleName);

}
//////// End by FYR /////////

void cliTrayIconUpdateBase(const char *szChangedProto)
{
	int i,count,netProtoCount,changed=-1;
	PROTOACCOUNT **accs;
	int averageMode=0;
	HWND hwnd=pcli->hwndContactList;

	if (!szChangedProto) return;

	if (!pcli->pfnGetProtocolVisibility(szChangedProto)) return;

	pcli->pfnLockTray();	
	if ( pcli->cycleTimerId ) {
		KillTimer( NULL, pcli->cycleTimerId);
		pcli->cycleTimerId=0;
	}
	ProtoEnumAccounts( &count, &accs );
	for(i=0,netProtoCount=0;i<count;i++) {
		if ( pcli->pfnGetProtocolVisibility(accs[i]->szModuleName)==0 ) continue;
		netProtoCount++;
		if(!lstrcmpA(szChangedProto,accs[i]->szModuleName)) pcli->cycleStep=i;
		if(averageMode==0) averageMode=CallProtoService(accs[i]->szModuleName,PS_GETSTATUS,0,0);
		else if(averageMode!=CallProtoService(accs[i]->szModuleName,PS_GETSTATUS,0,0)) {averageMode=-1; break;}
	}

	if(netProtoCount>1) {
		if(averageMode>=ID_STATUS_OFFLINE) {
			if(ModernGetSettingByte(NULL,"CList","TrayIcon",SETTING_TRAYICON_DEFAULT)==SETTING_TRAYICON_MULTI) {
				if(ModernGetSettingByte(NULL,"CList","AlwaysMulti",SETTING_ALWAYSMULTI_DEFAULT))
					changed=pcli->pfnTrayIconSetBaseInfo(cliGetIconFromStatusMode(NULL,szChangedProto,averageMode),szChangedProto);
				else if(pcli->trayIcon && pcli->trayIcon[0].szProto!=NULL) {
					pcli->pfnTrayIconDestroy(hwnd);
					pcli->pfnTrayIconInit(hwnd);
				}
				else
					changed=pcli->pfnTrayIconSetBaseInfo(cliGetIconFromStatusMode(NULL,NULL,averageMode),NULL);
			}
			else
			{
				if(ModernGetSettingByte(NULL,"CList","TrayIcon",SETTING_TRAYICON_DEFAULT)==SETTING_TRAYICON_SINGLE
					&& ModernGetSettingByte(NULL,"CList","AlwaysPrimary",SETTING_ALWAYSPRIMARY_DEFAULT))
				{
					DBVARIANT dbv={DBVT_DELETED};
					char *szProto;
					if(ModernGetSettingString(NULL,"CList","PrimaryStatus",&dbv)) szProto=NULL;
					else szProto=dbv.pszVal;
					changed=pcli->pfnTrayIconSetBaseInfo(cliGetIconFromStatusMode(NULL,szProto,averageMode),NULL);
					if (szProto) mir_free_and_nill(szProto);
				}
				else
					changed=pcli->pfnTrayIconSetBaseInfo(cliGetIconFromStatusMode(NULL,NULL,averageMode),NULL);
			}
		}
		else {
			switch(ModernGetSettingByte(NULL,"CList","TrayIcon",SETTING_TRAYICON_DEFAULT)) {
			case SETTING_TRAYICON_SINGLE:
				{
					DBVARIANT dbv={DBVT_DELETED};
					char *szProto;
					int status;
					if(ModernGetSettingString(NULL,"CList","PrimaryStatus",&dbv)) szProto=NULL;
					else szProto=dbv.pszVal;
					status=CallProtoService(szChangedProto,PS_GETSTATUS,0,0);

					if ((g_StatusBarData.connectingIcon==1)&&status>=ID_STATUS_CONNECTING&&status<=ID_STATUS_CONNECTING+MAX_CONNECT_RETRIES) {
						//
						HICON hIcon;
						// 1 check if multi connecting icon
						CListTray_GetGlobalStatus(0,0);
						if (g_bMultiConnectionMode)
							if (_strcmpi(szChangedProto,g_szConnectingProto))
								{ pcli->pfnUnlockTray(); return; }
							else
								hIcon=(HICON)CLUI_GetConnectingIconService((WPARAM)"",1);
						else
							hIcon=(HICON)CLUI_GetConnectingIconService((WPARAM)szChangedProto,0);
						if (hIcon) {
							changed=pcli->pfnTrayIconSetBaseInfo(hIcon,NULL);
							ModernDBFreeVariant(&dbv);
							break;
						}
					}
					else
						changed=pcli->pfnTrayIconSetBaseInfo(cliGetIconFromStatusMode(NULL,szProto,szProto?CallProtoService(szProto,PS_GETSTATUS,0,0):CallService(MS_CLIST_GETSTATUSMODE,0,0)),NULL);

					ModernDBFreeVariant(&dbv);
				}
				break;

			case SETTING_TRAYICON_CYCLE:
				{
					int status=szChangedProto ? CallProtoService(szChangedProto,PS_GETSTATUS,0,0) : averageMode;
					if ((g_StatusBarData.connectingIcon==1 && CListTray_GetGlobalStatus(0,0)
						&& ((status>=ID_STATUS_CONNECTING && status<=ID_STATUS_CONNECTING+MAX_CONNECT_RETRIES)|| g_bMultiConnectionMode )))
					{
						//connecting
						status=status;
						//stop cycling
						if (pcli->cycleTimerId)
							KillTimer(NULL,pcli->cycleTimerId);
						pcli->cycleTimerId=0;
						{
							HICON hIcon;
							// 1 check if multi connecting icon							
							if (g_bMultiConnectionMode)
								if (_strcmpi(szChangedProto,g_szConnectingProto))
								{ pcli->pfnUnlockTray(); return; }
								else
									hIcon=(HICON)CLUI_GetConnectingIconService((WPARAM)"",1);
							else
								hIcon=(HICON)CLUI_GetConnectingIconService((WPARAM)szChangedProto,0);
							if (hIcon)
								changed=pcli->pfnTrayIconSetBaseInfo(hIcon,NULL);
						}
					}
					else
					{
						pcli->cycleTimerId=CLUI_SafeSetTimer(NULL,0,ModernGetSettingWord(NULL,"CList","CycleTime",SETTING_CYCLETIME_DEFAULT)*1000,pcli->pfnTrayCycleTimerProc);
						changed=pcli->pfnTrayIconSetBaseInfo(cliGetIconFromStatusMode(NULL,szChangedProto,status),NULL);
					}

				}
				break;

			case SETTING_TRAYICON_MULTI:
				if (!pcli->trayIcon)
					pcli->pfnTrayIconRemove(NULL,NULL);
				else if ( ModernGetSettingByte(NULL,"CList","AlwaysMulti",SETTING_ALWAYSMULTI_DEFAULT )) {
					if (pcli->pfnGetProtocolVisibility(szChangedProto))
					{

						int status;
						status=CallProtoService(szChangedProto,PS_GETSTATUS,0,0);
						if ((g_StatusBarData.connectingIcon==1)&&status>=ID_STATUS_CONNECTING&&status<=ID_STATUS_CONNECTING+MAX_CONNECT_RETRIES)
						{
							//
							HICON hIcon;
							hIcon=(HICON)CLUI_GetConnectingIconService((WPARAM)szChangedProto,0);;
							if (hIcon)
								changed=pcli->pfnTrayIconSetBaseInfo(hIcon,szChangedProto);
						}
						else
							changed=pcli->pfnTrayIconSetBaseInfo(cliGetIconFromStatusMode(NULL,szChangedProto,CallProtoService(szChangedProto,PS_GETSTATUS,0,0)),szChangedProto);
					}
				}
				else if (pcli->pfnGetProtocolVisibility(szChangedProto)) {
					int i;
					int avg;
					avg=GetAverageMode();
					i=pcli->pfnTrayIconSetBaseInfo(cliGetIconFromStatusMode(NULL,szChangedProto,CallProtoService(szChangedProto,PS_GETSTATUS,0,0)),szChangedProto);
					if (i<0 /*|| (avg!=-1)*/) {
						pcli->pfnTrayIconDestroy(hwnd);
						pcli->pfnTrayIconInit(hwnd);
					}
					else {
						int status;
						changed=i;
						status=CallProtoService(szChangedProto,PS_GETSTATUS,0,0);
						if ((g_StatusBarData.connectingIcon==1)&&status>=ID_STATUS_CONNECTING&&status<=ID_STATUS_CONNECTING+MAX_CONNECT_RETRIES) {
							//
							HICON hIcon;
							hIcon=(HICON)CLUI_GetConnectingIconService((WPARAM)szChangedProto,0);;
							if (hIcon) {
								changed=pcli->pfnTrayIconSetBaseInfo(hIcon,szChangedProto);
				}	}	}	}
				break;
		}	}
	}
	else if ( pcli->pfnGetProtocolVisibility( szChangedProto )) 
	{
		DBVARIANT dbv={DBVT_DELETED};
		char *szProto;
		int status=CallProtoService(szChangedProto,PS_GETSTATUS,0,0);
		
		if ((g_StatusBarData.connectingIcon==1)&&status>=ID_STATUS_CONNECTING&&status<=ID_STATUS_CONNECTING+MAX_CONNECT_RETRIES) 
		{
			HICON hIcon = ( HICON )CLUI_GetConnectingIconService((WPARAM)szChangedProto,0);;
			if (hIcon) 
				changed=pcli->pfnTrayIconSetBaseInfo(hIcon,NULL);
		}
		else if (status>=ID_STATUS_OFFLINE && status<=ID_STATUS_IDLE) 
		{
			if(ModernGetSettingString(NULL,"CList","PrimaryStatus",&dbv))
				szProto=NULL;
			else 
				szProto=dbv.pszVal;
			changed=pcli->pfnTrayIconSetBaseInfo(cliGetIconFromStatusMode(NULL,szProto,status),NULL);
			if (szProto) 
				mir_free_and_nill(szProto);
		}
	}

	if (changed != -1) // && pcli->trayIcon[changed].isBase)
		pcli->pfnTrayIconUpdate(pcli->trayIcon[changed].hBaseIcon,NULL,szChangedProto,1);  // by FYR (No suitable protocol)
	{ pcli->pfnUnlockTray(); return; }
}

static UINT_PTR autoHideTimerId;

static VOID CALLBACK TrayIconAutoHideTimer(HWND hwnd,UINT message,UINT_PTR idEvent,DWORD dwTime)
{
	HWND hwndClui, ActiveWindow;
	KillTimer(hwnd,idEvent);
	hwndClui=pcli->hwndContactList;
	ActiveWindow=GetActiveWindow();
	if(ActiveWindow==hwndClui) return;
	if (CLUI_CheckOwnedByClui(ActiveWindow)) return;
	//CLUI_ShowWindowMod(hwndClui,SW_HIDE);
	CListMod_HideWindow(hwndClui, SW_HIDE);
	if(MySetProcessWorkingSetSize!=NULL)
		MySetProcessWorkingSetSize(GetCurrentProcess(),-1,-1);
}

INT_PTR TrayIconPauseAutoHide(WPARAM wParam,LPARAM lParam)
{
	if (ModernGetSettingByte(NULL,"CList","AutoHide",SETTING_AUTOHIDE_DEFAULT)) 
	{
		if (GetActiveWindow() != pcli->hwndContactList
			&& GetWindow(GetParent(GetActiveWindow()),GW_OWNER) != pcli->hwndContactList)
		{
			KillTimer(NULL,autoHideTimerId);
			autoHideTimerId=CLUI_SafeSetTimer(NULL,0,1000*ModernGetSettingWord(NULL,"CList","HideTime",SETTING_HIDETIME_DEFAULT),TrayIconAutoHideTimer);
	}	}

	return 0;
}

void DestroyTrayMenu(HMENU hMenu)
{
    int cnt = GetMenuItemCount(hMenu);
    for (int i=0; i<cnt; ++i)
    {
        HMENU hSubMenu = GetSubMenu(hMenu, i);
        if (hSubMenu == hStatusMenu || hSubMenu == hMainMenu)
            RemoveMenu(hMenu, i--, MF_BYPOSITION);
    }
    DestroyMenu(hMenu);
}

INT_PTR cli_TrayIconProcessMessage(WPARAM wParam,LPARAM lParam)
{
	MSG *msg=(MSG*)wParam;
	switch(msg->message) {
	case WM_EXITMENULOOP:
		if (pcli->bTrayMenuOnScreen)
			pcli->bTrayMenuOnScreen=FALSE;
		break;

		// case WM_DRAWITEM:
	//	return CallService(MS_CLIST_MENUDRAWITEM,msg->wParam,msg->lParam);

	// case WM_MEASUREITEM:
	//	return CallService(MS_CLIST_MENUMEASUREITEM,msg->wParam,msg->lParam);


	case WM_ACTIVATE:
		{
			HWND h1,h2,h4;
			SetCursor(LoadCursor(NULL, IDC_ARROW));
			h1=(HWND)msg->lParam;
			h2=h1?GetParent(h1):NULL;
			h4=pcli->hwndContactList;
			if(ModernGetSettingByte(NULL,"CList","AutoHide",SETTING_AUTOHIDE_DEFAULT)) {
				if (LOWORD(msg->wParam)==WA_INACTIVE && h2!=h4)
					autoHideTimerId=CLUI_SafeSetTimer(NULL,0,1000*ModernGetSettingWord(NULL,"CList","HideTime",SETTING_HIDETIME_DEFAULT),TrayIconAutoHideTimer);
				else { 
					KillTimer(NULL,autoHideTimerId);
					autoHideTimerId = 0;
				}
			}
			else if (autoHideTimerId) {
				KillTimer(NULL, autoHideTimerId);
				autoHideTimerId = 0;
			}
		}
		return FALSE; //to avoid autohideTimer in core

	case TIM_CALLBACK:
		if ((GetAsyncKeyState(VK_CONTROL)&0x8000) && msg->lParam == WM_LBUTTONDOWN && !ModernGetSettingByte(NULL,"CList","Tray1Click",SETTING_TRAY1CLICK_DEFAULT)) {
			POINT pt;
			HMENU hMenu;
			hMenu=(HMENU)CallService(MS_CLIST_MENUGETSTATUS,(WPARAM)0,(LPARAM)0);
			g_mutex_bOnTrayRightClick=1;
			IS_WM_MOUSE_DOWN_IN_TRAY=1;
			SetForegroundWindow(msg->hwnd);
			SetFocus(msg->hwnd);
			GetCursorPos(&pt);
			pcli->bTrayMenuOnScreen=TRUE;
			TrackPopupMenu(hMenu, TPM_TOPALIGN | TPM_LEFTALIGN|TPM_LEFTBUTTON, pt.x, pt.y, 0, msg->hwnd, NULL);
			PostMessage(msg->hwnd, WM_NULL, 0, 0);
			g_mutex_bOnTrayRightClick=0;
			IS_WM_MOUSE_DOWN_IN_TRAY=0;
		}
		else if (msg->lParam==WM_MBUTTONDOWN ||msg->lParam==WM_LBUTTONDOWN ||msg->lParam==WM_RBUTTONDOWN) {
			IS_WM_MOUSE_DOWN_IN_TRAY=1;
		}
		else if (msg->lParam == WM_RBUTTONUP) {
			POINT pt;
			HMENU hMenu;
			hMenu=(HMENU)CallService(MS_CLIST_MENUBUILDTRAY,(WPARAM)0,(LPARAM)0);
			g_mutex_bOnTrayRightClick=1;

			SetForegroundWindow(msg->hwnd);
			SetFocus(msg->hwnd);

			GetCursorPos(&pt);
			pcli->bTrayMenuOnScreen=TRUE;
			TrackPopupMenu(hMenu, TPM_TOPALIGN | TPM_LEFTALIGN|TPM_LEFTBUTTON, pt.x, pt.y, 0, msg->hwnd, NULL);
            DestroyTrayMenu(hMenu);
			PostMessage(msg->hwnd, WM_NULL, 0, 0);
		}
		else break;
		*((LRESULT*)lParam)=0;
		return TRUE;
	}
	return corecli.pfnTrayIconProcessMessage(wParam, lParam);
}

//////////////////////////////TRAY MENU/////////////////////////
HANDLE hTrayMenuObject;

HANDLE hTrayMainMenuItemProxy;
HANDLE hTrayStatusMenuItemProxy;
HANDLE hTrayHideShowMainMenuItem;

//traymenu exec param(ownerdata)
typedef struct{
	char *szServiceName;
	int Param1;
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
	ListParam param;

	memset(&param,0,sizeof(param));
	param.MenuObjectHandle=hTrayMenuObject;

	//hMenu=hMainMenu;
	hMenu=CreatePopupMenu();
	//hMenu=wParam;
	tick=GetTickCount();

	NotifyEventHooks(g_CluiData.hEventPreBuildTrayMenu,0,0);

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
	tmi.root = mi->hParentMenu;

	tmi.ownerdata=NULL;

	{
		lpTrayMenuExecParam mmep;
		mmep=(lpTrayMenuExecParam)mir_alloc(sizeof(TrayMenuExecParam));
		if(mmep==NULL){return(0);};

		//we need just one parametr.
		mmep->szServiceName=mir_strdup(mi->pszService);
		mmep->Param1=mi->popupPosition;

		tmi.ownerdata=mmep;
	}
	op.Handle=(HANDLE)CallService(MO_ADDNEWMENUITEM,(WPARAM)hTrayMenuObject,(LPARAM)&tmi);
	op.Setting=OPT_MENUITEMSETUNIQNAME;
	op.Value=(INT_PTR)mi->pszService;
	CallService(MO_SETOPTIONSMENUITEM,(WPARAM)0,(LPARAM)&op);
	return (INT_PTR)op.Handle;

	//	mainItemCount++;
	//	return MENU_CUSTOMITEMMAIN|(mainMenuItem[mainItemCount-1].id);
}

INT_PTR TrayMenuonAddService(WPARAM wParam,LPARAM lParam) {

	MENUITEMINFO *mii=(MENUITEMINFO* )wParam;
	if (mii==NULL) return 0;

	if (hTrayHideShowMainMenuItem==(HANDLE)lParam)
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
		if (!mir_strcmp(mmep->szServiceName,"Help/AboutCommand"))
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
		FreeAndNil((void**)&mmep->szServiceName);
		FreeAndNil((void**)&mmep);
	}

	return(0);
}

void InitTrayMenus(void)
{
	TMenuParam tmp;
	OptParam op;

	CreateServiceFunction("CLISTMENUSTRAY/ExecService",TrayMenuExecService);
	CreateServiceFunction("CLISTMENUSTRAY/FreeOwnerDataTrayMenu",FreeOwnerDataTrayMenu);
	CreateServiceFunction("CLISTMENUSTRAY/TrayMenuonAddService",TrayMenuonAddService);

	CreateServiceFunction(MS_CLIST_ADDTRAYMENUITEM,AddTrayMenuItem);
	CreateServiceFunction(MS_CLIST_REMOVETRAYMENUITEM,RemoveTrayMenuItem);
	CreateServiceFunction(MS_CLIST_MENUBUILDTRAY,BuildTrayMenu);
	

	//Tray menu
	memset(&tmp,0,sizeof(tmp));
	tmp.cbSize=sizeof(tmp);
	tmp.CheckService=NULL;
	tmp.ExecService="CLISTMENUSTRAY/ExecService";
	tmp.name="TrayMenu";
	hTrayMenuObject=(HANDLE)CallService(MO_CREATENEWMENUOBJECT,(WPARAM)0,(LPARAM)&tmp);


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
		mi.flags=CMIF_ICONFROMICOLIB;
		mi.icolibItem=LoadSkinnedIconHandle(SKINICON_OTHER_EXIT);
		AddTrayMenuItem((WPARAM)0,(LPARAM)&mi);
		DestroyIcon_protect(mi.hIcon);

		memset(&mi,0,sizeof(mi));
		mi.cbSize=sizeof(mi);
		mi.position=100000;
		mi.pszService=MS_CLIST_SHOWHIDE;
		mi.pszName=LPGEN("&Hide/Show");
		mi.flags=CMIF_ICONFROMICOLIB;
		mi.icolibItem=LoadSkinnedIconHandle(SKINICON_OTHER_SHOWHIDE);
		hTrayHideShowMainMenuItem=(HANDLE)AddTrayMenuItem((WPARAM)0,(LPARAM)&mi);
		DestroyIcon_protect(mi.hIcon);

		memset(&mi,0,sizeof(mi));
		mi.cbSize=sizeof(mi);
		mi.position=200000;
		mi.flags=CMIF_ICONFROMICOLIB;
		mi.icolibItem=LoadSkinnedIconHandle(SKINICON_OTHER_FINDUSER);
		mi.pszService="FindAdd/FindAddCommand";
		mi.pszName=LPGEN("&Find/Add Contacts...");
		AddTrayMenuItem((WPARAM)0,(LPARAM)&mi);
		DestroyIcon_protect(mi.hIcon);


		memset(&mi,0,sizeof(mi));
		mi.cbSize=sizeof(mi);
		mi.position=300000;
		mi.flags=CMIF_ICONFROMICOLIB; // eternity #004
		mi.icolibItem=LoadSkinnedIconHandle(SKINICON_OTHER_MAINMENU); // eternity #004
		mi.pszService="FakeService_1";
		mi.pszName=LPGEN("&Main Menu");
		hTrayMainMenuItemProxy=(HANDLE)AddTrayMenuItem((WPARAM)0,(LPARAM)&mi);
		DestroyIcon_protect(mi.hIcon); // eternity #004

		memset(&mi,0,sizeof(mi));
		mi.cbSize=sizeof(mi);
		mi.position=300100;
		mi.pszService="FakeService_2";
		mi.flags=CMIF_ICONFROMICOLIB; // eternity #004
		mi.icolibItem=LoadSkinnedIconHandle(SKINICON_OTHER_STATUS); // eternity #004
		mi.pszName=LPGEN("&Status");
		hTrayStatusMenuItemProxy=(HANDLE)AddTrayMenuItem((WPARAM)0,(LPARAM)&mi);
		DestroyIcon_protect(mi.hIcon); // eternity #004

		memset(&mi,0,sizeof(mi));
		mi.cbSize=sizeof(mi);
		mi.position=400000;
		mi.flags=CMIF_ICONFROMICOLIB;
		mi.icolibItem=LoadSkinnedIconHandle(SKINICON_OTHER_OPTIONS);
		mi.pszService="Options/OptionsCommand";
		mi.pszName=LPGEN("&Options...");
		AddTrayMenuItem((WPARAM)0,(LPARAM)&mi);
		DestroyIcon_protect(mi.hIcon);

		memset(&mi,0,sizeof(mi));
		mi.cbSize=sizeof(mi);
		mi.position=500000;
		mi.flags=CMIF_ICONFROMICOLIB;
		mi.icolibItem=LoadSkinnedIconHandle(SKINICON_OTHER_MIRANDA);
		mi.pszService="Help/AboutCommand";
		mi.pszName=LPGEN("&About");
		AddTrayMenuItem((WPARAM)0,(LPARAM)&mi);
		DestroyIcon_protect(mi.hIcon);
	}

    hMainMenu = (HMENU)CallService(MS_CLIST_MENUGETMAIN,0,0);
    hStatusMenu = (HMENU)CallService(MS_CLIST_MENUGETSTATUS,0,0);
}

void UninitTrayMenu()
{
    if (hTrayMenuObject && ServiceExists(MO_REMOVEMENUOBJECT))
        CallService(MO_REMOVEMENUOBJECT,(WPARAM)hTrayMenuObject,0);
    hTrayMenuObject=NULL;
}

void InitTray(void)
{
	InitTrayMenus();
	return;
}

//////////////////////////////END TRAY MENU/////////////////////////
