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
#include "m_clui.h"
#include <m_file.h>
#include <m_addcontact.h>
#include "hdr/modern_clist.h"
#include "hdr/modern_commonprototypes.h"
#include "hdr/modern_sync.h"
#include "hdr/modern_clui.h"
#include <m_modernopt.h>
pfnMyMonitorFromPoint  MyMonitorFromPoint = NULL;
pfnMyMonitorFromWindow MyMonitorFromWindow = NULL;
pfnMyGetMonitorInfo    MyGetMonitorInfo = NULL;

static HANDLE hookSystemShutdown_CListMod=NULL;
HANDLE  hookOptInitialise_CList=NULL,
        hookOptInitialise_Skin=NULL,
        hookContactAdded_CListSettings=NULL;


int CListMod_HideWindow(HWND hwndContactList, int mode);

void GroupMenus_Init(void);
int AddMainMenuItem(WPARAM wParam,LPARAM lParam);
int AddContactMenuItem(WPARAM wParam,LPARAM lParam);
void UninitCListEvents(void);
int ContactSettingChanged(WPARAM wParam,LPARAM lParam);
int ContactAdded(WPARAM wParam,LPARAM lParam);
int GetContactDisplayName(WPARAM wParam,LPARAM lParam);
int CListOptInit(WPARAM wParam,LPARAM lParam);
int SkinOptInit(WPARAM wParam,LPARAM lParam);
int ModernSkinOptInit(WPARAM wParam,LPARAM lParam);
int EventsProcessContactDoubleClick(HANDLE hContact);

INT_PTR TrayIconPauseAutoHide(WPARAM wParam,LPARAM lParam);
INT_PTR ContactChangeGroup(WPARAM wParam,LPARAM lParam);
void InitTrayMenus(void);


HIMAGELIST hCListImages=NULL;

BOOL (WINAPI *MySetProcessWorkingSetSize)(HANDLE,SIZE_T,SIZE_T);


static HANDLE hSettingChanged;

//returns normal icon or combined with status overlay. Needs to be destroyed.
HICON cliGetIconFromStatusMode(HANDLE hContact, const char *szProto,int status)
{
	HICON hIcon=NULL;
	HICON hXIcon=NULL;
	// check if options is turned on
	BYTE trayOption=ModernGetSettingByte(NULL,"CLUI","XStatusTray",SETTING_TRAYOPTION_DEFAULT);
	if (trayOption&3 && szProto!=NULL)
	{
		// check service exists
		char str[MAXMODULELABELLENGTH];
		strcpy(str,szProto);
		strcat(str,"/GetXStatusIcon");
		if (ServiceExists(str))
		{
			// check status is online
			if (status>ID_STATUS_OFFLINE)
			{
				// get xicon
				hXIcon=(HICON)CallService(str,0,0);
				if (hXIcon)
				{
					// check overlay mode
					if (trayOption&2)
					{
						// get overlay
						HICON MainOverlay=(HICON)GetMainStatusOverlay(status);
						hIcon=ske_CreateJoinedIcon(hXIcon,MainOverlay,(trayOption&4)?192:0);
						DestroyIcon_protect(hXIcon);
                        DestroyIcon_protect(MainOverlay);
					}
					else
					{
						// paint it
						hIcon=hXIcon;
					}
				}
			}
		}
	}
	if (!hIcon)
	{
		hIcon=ske_ImageList_GetIcon(g_himlCListClc,ExtIconFromStatusMode(hContact,szProto,status),ILD_NORMAL);
	}
	// if not ready take normal icon
	return hIcon;
}
////////// By FYR/////////////
int ExtIconFromStatusMode(HANDLE hContact, const char *szProto,int status)
{
	/*pdisplayNameCacheEntry cacheEntry;
	if ((DBGetContactSettingByte(NULL,"CLC","Meta",0)!=1) && szProto!=NULL)
    {
		if (meta_module && mir_strcmp(szProto,meta_module)==0)
        {
			hContact=(HANDLE)CallService(MS_MC_GETMOSTONLINECONTACT,(UINT)hContact,0);
			if (hContact!=0)
            {
				szProto=(char*)CallService(MS_PROTO_GETCONTACTBASEPROTO,(UINT)hContact,0);
				status=DBGetContactSettingWord(hContact,szProto,"Status",ID_STATUS_OFFLINE);
			}
        }
    }
    cacheEntry=(pdisplayNameCacheEntry)pcli->pfnGetCacheEntry(hContact);
    if (cacheEntry->isTransport>0)  return GetTrasportStatusIconIndex(cacheEntry->isTransport-1,status);
    */
    return pcli->pfnIconFromStatusMode(szProto,status,hContact);
}
/////////// End by FYR ////////
int cli_IconFromStatusMode(const char *szProto,int nStatus, HANDLE hContact)
{
   int result=-1;
   if (hContact && szProto)
   {
       char * szActProto=(char*)szProto;
       char AdvancedService[255]={0};
       int  nActStatus=nStatus;
       HANDLE hActContact=hContact;
       if (!ModernGetSettingByte(NULL,"CLC","Meta",SETTING_USEMETAICON_DEFAULT) && g_szMetaModuleName && !mir_strcmp(szActProto,g_szMetaModuleName))
       {
            // substitute params by mostonline contact datas
           HANDLE hMostOnlineContact=(HANDLE)CallService(MS_MC_GETMOSTONLINECONTACT,(WPARAM)hActContact,0);
           if (hMostOnlineContact)
           {
                pdisplayNameCacheEntry cacheEntry;
                cacheEntry=(pdisplayNameCacheEntry)pcli->pfnGetCacheEntry(hMostOnlineContact);
                if (cacheEntry && cacheEntry->m_cache_cszProto)
                {
                    szActProto=cacheEntry->m_cache_cszProto;
                    nActStatus=pdnce___GetStatus( cacheEntry );
                    hActContact=hMostOnlineContact;
                }
           }
       }
       mir_snprintf(AdvancedService,SIZEOF(AdvancedService),"%s%s",szActProto,"/GetAdvancedStatusIcon");

       if (ServiceExists(AdvancedService))
          result=CallService(AdvancedService,(WPARAM)hActContact, (LPARAM)0);

       if (result==-1 || !(LOWORD(result)))
       {
           //Get normal Icon
           int  basicIcon=corecli.pfnIconFromStatusMode(szActProto,nActStatus,NULL);
           if (result!=-1 && basicIcon!=1) result|=basicIcon;
           else result=basicIcon;
       }
   }
   else
   {
       result=corecli.pfnIconFromStatusMode(szProto,nStatus,NULL);
   }
   return result;
}


int GetContactIconC(pdisplayNameCacheEntry cacheEntry)
{
	return ExtIconFromStatusMode(cacheEntry->m_cache_hContact,cacheEntry->m_cache_cszProto,cacheEntry->m_cache_cszProto==NULL ? ID_STATUS_OFFLINE : pdnce___GetStatus( cacheEntry ));
}

//lParam
// 0 - default - return icon id in order: transport status icon, protostatus icon, meta is affected


INT_PTR GetContactIcon(WPARAM wParam,LPARAM lParam)
{
	char *szProto;
	int status;
	int res;
	szProto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, wParam, 0);
	if (szProto == NULL)
		status = ID_STATUS_OFFLINE;
	else
		status = ModernGetSettingWord((HANDLE) wParam, szProto, "Status", ID_STATUS_OFFLINE);
    res=ExtIconFromStatusMode((HANDLE)wParam,szProto,szProto==NULL?ID_STATUS_OFFLINE:status); //by FYR
    if (lParam==0 && res!=-1) res&=0xFFFF;
    return res;
}
void UninitTrayMenu();
void UnLoadContactListModule()  //unhooks noncritical events
{
    UninitTrayMenu();
    UninitCustomMenus();
   // UnloadMainMenu();
   // UnloadStatusMenu();
    ModernUnhookEvent(hookOptInitialise_CList);
    ModernUnhookEvent(hookOptInitialise_Skin);
    ModernUnhookEvent(hSettingChanged);
    ModernUnhookEvent(hookContactAdded_CListSettings);
}
int CListMod_ContactListShutdownProc(WPARAM wParam,LPARAM lParam)
{
    ModernUnhookEvent(hookSystemShutdown_CListMod);
    FreeDisplayNameCache();
    if(g_hMainThread) CloseHandle(g_hMainThread);
    g_hMainThread=NULL;
	return 0;
}

INT_PTR CLUIGetCapsService(WPARAM wParam,LPARAM lParam)
{
	if (lParam)
	{
		switch (lParam)
		{
		case 0:
			return 0;
		case CLUIF2_PLUGININFO:
			return (INT_PTR)&pluginInfo;
		case CLUIF2_CLISTTYPE:
	#ifdef UNICODE
				return 0x0107;
	#else
				return 0x0007;
	#endif
		case CLUIF2_EXTRACOLUMNCOUNT:
			return EXTRA_ICON_COUNT;
		case CLUIF2_USEREXTRASTART:
			return EXTRA_ICON_ADV3;
		}
		return 0;
	}
	else
	{
		switch (wParam)
		{
		case CLUICAPS_FLAGS1:
			return CLUIF_HIDEEMPTYGROUPS|CLUIF_DISABLEGROUPS|CLUIF_HASONTOPOPTION|CLUIF_HASAUTOHIDEOPTION;
		case CLUICAPS_FLAGS2:
			return MAKELONG(EXTRACOLUMNCOUNT,1);
		}
	}
	return 0;
}
HRESULT PreLoadContactListModule()
{
	/* Global data initialization */
	{
		g_CluiData.fOnDesktop=FALSE;
		g_CluiData.dwKeyColor=RGB(255,0,255);
		g_CluiData.bCurrentAlpha=255;
	}

	//initialize firstly hooks
	//clist interface is empty yet so handles should check
	hSettingChanged = ModernHookEvent(ME_DB_CONTACT_SETTINGCHANGED,ContactSettingChanged);
	CreateServiceFunction(MS_CLIST_GETCONTACTICON,GetContactIcon);

	return S_OK;
}

INT_PTR SvcActiveSkin(WPARAM wParam, LPARAM lParam);
INT_PTR SvcPreviewSkin(WPARAM wParam, LPARAM lParam);
INT_PTR SvcApplySkin(WPARAM wParam, LPARAM lParam);

HRESULT  CluiLoadModule()
{
	CreateServiceFunction(MS_CLUI_GETCAPS,CLUIGetCapsService);

	InitDisplayNameCache();
	hookSystemShutdown_CListMod  = ModernHookEvent(ME_SYSTEM_SHUTDOWN,CListMod_ContactListShutdownProc);
	hookOptInitialise_CList      = ModernHookEvent(ME_OPT_INITIALISE,CListOptInit);
	hookOptInitialise_Skin       = ModernHookEvent(ME_OPT_INITIALISE,SkinOptInit);

	CreateServiceFunction("ModernSkinSel/Active", SvcActiveSkin);
	CreateServiceFunction("ModernSkinSel/Preview", SvcPreviewSkin);
	CreateServiceFunction("ModernSkinSel/Apply", SvcApplySkin);
	
	hookContactAdded_CListSettings = ModernHookEvent(ME_DB_CONTACT_ADDED,ContactAdded);	
	CreateServiceFunction(MS_CLIST_TRAYICONPROCESSMESSAGE,cli_TrayIconProcessMessage);
	CreateServiceFunction(MS_CLIST_PAUSEAUTOHIDE,TrayIconPauseAutoHide);
	CreateServiceFunction(MS_CLIST_CONTACTCHANGEGROUP,ContactChangeGroup);
	CreateServiceFunction(MS_CLIST_TOGGLEHIDEOFFLINE,ToggleHideOffline);

	CreateServiceFunction(MS_CLIST_TOGGLEGROUPS,ToggleGroups);
	CreateServiceFunction(MS_CLIST_TOGGLESOUNDS,ToggleSounds);
	CreateServiceFunction(MS_CLIST_SETUSEGROUPS,SetUseGroups);


	CreateServiceFunction(MS_CLIST_GETCONTACTICON,GetContactIcon);

	MySetProcessWorkingSetSize=(BOOL (WINAPI*)(HANDLE,SIZE_T,SIZE_T))GetProcAddress(GetModuleHandle(TEXT("kernel32")),"SetProcessWorkingSetSize");
	hCListImages = ImageList_Create(16, 16, ILC_MASK|ILC_COLOR32, 32, 0);
	InitCustomMenus();
	InitTray();
	{
		HINSTANCE hUser = GetModuleHandleA("USER32");
		MyMonitorFromPoint  = ( pfnMyMonitorFromPoint )GetProcAddress( hUser,"MonitorFromPoint" );
		MyMonitorFromWindow = ( pfnMyMonitorFromWindow )GetProcAddress( hUser, "MonitorFromWindow" );
		#if defined( _UNICODE )
			MyGetMonitorInfo = ( pfnMyGetMonitorInfo )GetProcAddress( hUser, "GetMonitorInfoW");
		#else
			MyGetMonitorInfo = ( pfnMyGetMonitorInfo )GetProcAddress( hUser, "GetMonitorInfoA");
		#endif
	}
	CLUI::InitClui();
	
	return S_OK;
}

/*
Begin of Hrk's code for bug
*/
#define GWVS_HIDDEN 1
#define GWVS_VISIBLE 2
#define GWVS_COVERED 3
#define GWVS_PARTIALLY_COVERED 4

int GetWindowVisibleState(HWND, int, int);
__inline DWORD GetDIBPixelColor(int X, int Y, int Width, int Height, int ByteWidth, BYTE * ptr)
{
	DWORD res=0;
	if (X>=0 && X<Width && Y>=0 && Y<Height && ptr)
		res=*((DWORD*)(ptr+ByteWidth*(Height-Y-1)+X*4));
	return res;
}

int GetWindowVisibleState(HWND hWnd, int iStepX, int iStepY) {
	RECT rc = { 0 };
	POINT pt = { 0 };
	register int    i = 0,
                    j = 0,
                    width = 0,
                    height = 0,
                    iCountedDots = 0,
                    iNotCoveredDots = 0;
	HWND hAux = 0;

	if (hWnd == NULL) {
		SetLastError(0x00000006); //Wrong handle
		return -1;
	}
	//Some defaults now. The routine is designed for thin and tall windows.
	if (iStepX <= 0) iStepX = 8;
	if (iStepY <= 0) iStepY = 16;

	HWND hwndFocused = GetFocus();
	
	if (IsIconic(hWnd) || !IsWindowVisible(hWnd))
		return GWVS_HIDDEN;
	else if ( ModernGetSettingByte(NULL,"CList","OnDesktop",SETTING_ONDESKTOP_DEFAULT) || !ModernGetSettingByte(NULL, "CList", "BringToFront", SETTING_BRINGTOFRONT_DEFAULT) )
		return GWVS_VISIBLE;
	else if ( hwndFocused == pcli->hwndContactList || GetParent(hwndFocused) == pcli->hwndContactList )
		return GWVS_VISIBLE;
	else
	{
		int hstep,vstep;
		BITMAP bmp;
		HBITMAP WindowImage;
		int maxx=0;
		int maxy=0;
		int wx=0;
		int dx,dy;
		BYTE *ptr=NULL;
		HRGN rgn=NULL;
		WindowImage=g_CluiData.fLayered?ske_GetCurrentWindowImage():0;
		if (WindowImage&&g_CluiData.fLayered)
		{
			GetObject(WindowImage,sizeof(BITMAP),&bmp);
			ptr=(BYTE*)bmp.bmBits;
			maxx=bmp.bmWidth;
			maxy=bmp.bmHeight;
			wx=bmp.bmWidthBytes;
		}
		else
		{
			RECT rc;
			int i=0;
			rgn=CreateRectRgn(0,0,1,1);
			GetWindowRect(hWnd,&rc);
			GetWindowRgn(hWnd,rgn);
			OffsetRgn(rgn,rc.left,rc.top);
			GetRgnBox(rgn,&rc);
			i=i;
			//maxx=rc.right;
			//maxy=rc.bottom;
		}
		GetWindowRect(hWnd, &rc);
		{
			RECT rcMonitor={0};
			Docking_GetMonitorRectFromWindow(hWnd,&rcMonitor);
			rc.top=rc.top<rcMonitor.top?rcMonitor.top:rc.top;
			rc.left=rc.left<rcMonitor.left?rcMonitor.left:rc.left;
			rc.bottom=rc.bottom>rcMonitor.bottom?rcMonitor.bottom:rc.bottom;
			rc.right=rc.right>rcMonitor.right?rcMonitor.right:rc.right;
		}
		width = rc.right - rc.left;
		height = rc.bottom- rc.top;
		dx=-rc.left;
		dy=-rc.top;
		hstep=width/iStepX;
		vstep=height/iStepY;
		hstep=hstep>0?hstep:1;
		vstep=vstep>0?vstep:1;

		for (i = rc.top; i < rc.bottom; i+=vstep) {
			pt.y = i;
			for (j = rc.left; j < rc.right; j+=hstep) {
				BOOL po=FALSE;
				pt.x = j;
				if (rgn)
					po=PtInRegion(rgn,j,i);
				else
				{
					DWORD a=(GetDIBPixelColor(j+dx,i+dy,maxx,maxy,wx,ptr)&0xFF000000)>>24;
					a=((a*g_CluiData.bCurrentAlpha)>>8);
					po=(a>16);
				}
				if (po||(!rgn&&ptr==0))
				{
					BOOL hWndFound=FALSE;
					HWND hAuxOld=NULL;
					hAux = WindowFromPoint(pt);
					do
					{
						if (hAux==hWnd)
						{
							hWndFound=TRUE;
							break;
						}
						//hAux = GetParent(hAux);
						hAuxOld=hAux;
						hAux = fnGetAncestor(hAux,GA_ROOTOWNER);
						if (hAuxOld==hAux)
						{
							TCHAR buf[255];
							GetClassName(hAux,buf,SIZEOF(buf));
							if (!lstrcmp(buf,CLUIFrameSubContainerClassName))
							{
								hWndFound=TRUE;
								break;
							}
						}
					}while(hAux!= NULL &&hAuxOld!=hAux);

					if (hWndFound) //There's  window!
						iNotCoveredDots++; //Let's count the not covered dots.
					iCountedDots++; //Let's keep track of how many dots we checked.
				}
			}
		}
		if (rgn) DeleteObject(rgn);
		if ( iCountedDots - iNotCoveredDots<2) //Every dot was not covered: the window is visible.
			return GWVS_VISIBLE;
		else if (iNotCoveredDots == 0) //They're all covered!
			return GWVS_COVERED;
		else //There are dots which are visible, but they are not as many as the ones we counted: it's partially covered.
			return GWVS_PARTIALLY_COVERED;
	}
}
BYTE g_bCalledFromShowHide=0;
int cliShowHide(WPARAM wParam,LPARAM lParam)
{
	BOOL bShow = FALSE;

	int iVisibleState = GetWindowVisibleState(pcli->hwndContactList,0,0);
	int method;
	method=ModernGetSettingByte(NULL, "ModernData", "HideBehind", SETTING_HIDEBEHIND_DEFAULT);; //(0-none, 1-leftedge, 2-rightedge);
	if (method)
	{
		if (ModernGetSettingByte(NULL, "ModernData", "BehindEdge", SETTING_BEHINDEDGE_DEFAULT)==0 && lParam!=1)
		{
			//hide
			CLUI_HideBehindEdge();
		}
		else
		{
			CLUI_ShowFromBehindEdge();
		}
		bShow=TRUE;
		iVisibleState=GWVS_HIDDEN;
	}

	if (!method && ModernGetSettingByte(NULL, "ModernData", "BehindEdge", SETTING_BEHINDEDGE_DEFAULT)>0)
	{
		g_CluiData.bBehindEdgeSettings=ModernGetSettingByte(NULL, "ModernData", "BehindEdge", SETTING_BEHINDEDGE_DEFAULT);
		CLUI_ShowFromBehindEdge();
		g_CluiData.bBehindEdgeSettings=0;
		g_CluiData.nBehindEdgeState=0;
		ModernDeleteSetting(NULL, "ModernData", "BehindEdge");
	}

	//bShow is FALSE when we enter the switch if no hide behind edge.
	switch (iVisibleState) {
		case GWVS_PARTIALLY_COVERED:
			bShow = TRUE; break;
		case GWVS_COVERED: //Fall through (and we're already falling)
			bShow = TRUE; break;
		case GWVS_HIDDEN:
			bShow = TRUE; break;
		case GWVS_VISIBLE: //This is not needed, but goes for readability.
			bShow = FALSE; break;
		case -1: //We can't get here, both pcli->hwndContactList and iStepX and iStepY are right.
			return 0;
	}

	if( (bShow == TRUE || lParam == 1) ) 
	{
		Sync( CLUIFrames_ActivateSubContainers, TRUE );
		CLUI_ShowWindowMod(pcli->hwndContactList, SW_RESTORE);

		if (!ModernGetSettingByte(NULL,"CList","OnDesktop",SETTING_ONDESKTOP_DEFAULT))
		{
			Sync(CLUIFrames_OnShowHide, pcli->hwndContactList,1);	//TO BE PROXIED
			SetWindowPos(pcli->hwndContactList, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE |SWP_NOACTIVATE);
			g_bCalledFromShowHide=1;
			if (!ModernGetSettingByte(NULL,"CList","OnTop",SETTING_ONTOP_DEFAULT))
				SetWindowPos(pcli->hwndContactList, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
			g_bCalledFromShowHide=0;
		}
		else
		{
			SetWindowPos(pcli->hwndContactList, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
			Sync(CLUIFrames_OnShowHide, pcli->hwndContactList,1);
			SetForegroundWindow(pcli->hwndContactList);
		}
		ModernWriteSettingByte(NULL,"CList","State",SETTING_STATE_NORMAL);

		RECT rcWindow;
		GetWindowRect(pcli->hwndContactList,&rcWindow);
		if (Utils_AssertInsideScreen(&rcWindow) == 1)
		{
			MoveWindow(pcli->hwndContactList, rcWindow.left, rcWindow.top, 
				rcWindow.right - rcWindow.left, rcWindow.bottom - rcWindow.top, TRUE);
		}

		//if (DBGetContactSettingByte(NULL,"CList","OnDesktop",SETTING_ONDESKTOP_DEFAULT))
		//    SetWindowPos(pcli->hwndContactList, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

	}
	else { //It needs to be hidden
		if (GetWindowLong(pcli->hwndContactList, GWL_EXSTYLE) & WS_EX_TOOLWINDOW)
		{
			CListMod_HideWindow(pcli->hwndContactList, SW_HIDE);
			ModernWriteSettingByte(NULL,"CList","State",SETTING_STATE_HIDDEN);
		}
		else
		{
			if (ModernGetSettingByte(NULL,"CList","Min2Tray",SETTING_MIN2TRAY_DEFAULT)) {
				CLUI_ShowWindowMod(pcli->hwndContactList, SW_HIDE);
				ModernWriteSettingByte(NULL,"CList","State",SETTING_STATE_HIDDEN);
			}
			else
			{
				CLUI_ShowWindowMod(pcli->hwndContactList, SW_MINIMIZE);
				ModernWriteSettingByte(NULL,"CList","State",SETTING_STATE_MINIMIZED);
			}
		}

		if (MySetProcessWorkingSetSize != NULL) 
			MySetProcessWorkingSetSize(GetCurrentProcess(),-1,-1);
	}
	return 0;
}

int CListMod_HideWindow(HWND hwndContactList, int mode)
{
	KillTimer(pcli->hwndContactList,1/*TM_AUTOALPHA*/);
	if (!CLUI_HideBehindEdge())  return CLUI_SmoothAlphaTransition(pcli->hwndContactList, 0, 1);
	return 0;
}
