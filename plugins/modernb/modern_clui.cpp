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

#include "m_clc.h"
#include "m_clui.h"
#include "m_skin.h"
#include "m_api/m_skinbutton.h"
#include "hdr/modern_skinengine.h"
#include "hdr/modern_statusbar.h"

#include "hdr/modern_static_clui.h"
#include <locale.h>
#include "hdr/modern_clcpaint.h"
#include "hdr/modern_sync.h"



HRESULT (WINAPI *g_proc_DWMEnableBlurBehindWindow)(HWND hWnd, DWM_BLURBEHIND *pBlurBehind);
BOOL CALLBACK ProcessCLUIFrameInternalMsg(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, LRESULT& result );
void DestroyTrayMenu(HMENU hMenu);

// new sources
#ifdef _MSC_VER
#include <crtdbg.h>
#endif

//////////////// CLUI CLASS IMPLEMENTATION /////////////////////////////////
#include "hdr/modern_clui.h"

#define STATIC_METHOD

CLUI* CLUI::m_pCLUI = NULL;
BOOL CLUI::m_fMainMenuInited = FALSE;
HWND CLUI::m_hWnd = NULL;

void CLUI::cliOnCreateClc(void)
{  
	STATIC_METHOD;
	_ASSERT( m_pCLUI );
	m_pCLUI->CreateCluiFrames();
}

int CLUI::OnEvent_ModulesLoaded(WPARAM wParam,LPARAM lParam)
{
	STATIC_METHOD;

	g_CluiData.bMetaAvail = ServiceExists(MS_MC_GETDEFAULTCONTACT) ? TRUE : FALSE;
	setlocale(LC_ALL, "");  //fix for case insensitive comparing

	if (ServiceExists(MS_MC_DISABLEHIDDENGROUP))
		CallService(MS_MC_DISABLEHIDDENGROUP, (WPARAM)TRUE, (LPARAM)0);

	if (ServiceExists(MS_MC_GETPROTOCOLNAME))
		g_szMetaModuleName = (char *)CallService(MS_MC_GETPROTOCOLNAME, 0, 0);

	CLUIServices_ProtocolStatusChanged(0,0);
	SleepEx(0,TRUE);
	g_flag_bOnModulesLoadedCalled=TRUE;	
	///pcli->pfnInvalidateDisplayNameCacheEntry(INVALID_HANDLE_VALUE);   
	SendMessage(pcli->hwndContactList,UM_CREATECLC,0,0); //$$$
	InitSkinHotKeys();
	g_CluiData.bSTATE = STATE_NORMAL;
	ske_RedrawCompleteWindow();

	return 0;
}

int CLUI::OnEvent_FontReload(WPARAM wParam,LPARAM lParam)
{
    STATIC_METHOD;
    
    pcli->pfnClcBroadcast( INTM_RELOADOPTIONS, wParam, lParam );

	g_CluiData.dwKeyColor=ModernGetSettingDword(NULL,"ModernSettings","KeyColor",(DWORD)SETTING_KEYCOLOR_DEFAULT);

	CLUI__cliInvalidateRect( pcli->hwndContactList, 0, 0 );
	
    return 0;
}

int CLUI::OnEvent_ContactMenuPreBuild(WPARAM wParam, LPARAM lParam) 
{
	TCHAR cls[128];
	HANDLE hItem;
	HWND hwndClist = GetFocus();
	CLISTMENUITEM mi;
	if (MirandaExiting()) return 0;
	ZeroMemory(&mi,sizeof(mi));
	mi.cbSize = sizeof(mi);
	mi.flags = CMIM_FLAGS;
	GetClassName(hwndClist,cls,SIZEOF(cls));
	hwndClist = (!lstrcmp(CLISTCONTROL_CLASS,cls))?hwndClist:pcli->hwndContactList;
	hItem = (HANDLE)SendMessage(hwndClist,CLM_GETSELECTION,0,0);
	if (!hItem) {
		mi.flags = CMIM_FLAGS | CMIF_HIDDEN;
	}
	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hRenameMenuItem, (LPARAM)&mi);

	if (!hItem || !IsHContactContact(hItem) || !ModernGetSettingByte(NULL,"CList","AvatarsShow",SETTINGS_SHOWAVATARS_DEFAULT))
	{
		mi.flags = CMIM_FLAGS | CMIF_HIDDEN;
		CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hShowAvatarMenuItem, (LPARAM)&mi);
		CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hHideAvatarMenuItem, (LPARAM)&mi);
	}
	else
	{
		int has_avatar;

		if (ServiceExists(MS_AV_GETAVATARBITMAP))
		{
			has_avatar = CallService(MS_AV_GETAVATARBITMAP, (WPARAM)hItem, 0);
		}
		else
		{
			DBVARIANT dbv={0};
			if (ModernGetSettingString(hItem, "ContactPhoto", "File", &dbv))
			{
				has_avatar = 0;
			}
			else
			{
				has_avatar = 1;
				ModernDBFreeVariant(&dbv);
			}
		}

		if (ModernGetSettingByte(hItem, "CList", "HideContactAvatar", 0))
		{
			mi.flags = CMIM_FLAGS | (has_avatar ? 0 : CMIF_GRAYED);
			CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hShowAvatarMenuItem, (LPARAM)&mi);
			mi.flags = CMIM_FLAGS | CMIF_HIDDEN;
			CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hHideAvatarMenuItem, (LPARAM)&mi);
		}
		else
		{
			mi.flags = CMIM_FLAGS | CMIF_HIDDEN;
			CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hShowAvatarMenuItem, (LPARAM)&mi);
			mi.flags = CMIM_FLAGS | (has_avatar ? 0 : CMIF_GRAYED);
			CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hHideAvatarMenuItem, (LPARAM)&mi);
		}
	}

	return 0;
}
int CLUI::OnEvent_DBSettingChanging(WPARAM wParam,LPARAM lParam)
{
	if (ServiceExists("ExtraIcon/Register"))
		return 0;

	DBCONTACTWRITESETTING *dbcws=(DBCONTACTWRITESETTING *)lParam;
	if (wParam==0) return 0;  
	if (dbcws==NULL) return(0);
	if (MirandaExiting()) return 0;

	if ( ( dbcws->value.type==DBVT_WORD && !mir_strcmp(dbcws->szSetting,"ApparentMode") ) ||
		( dbcws->value.type==DBVT_ASCIIZ && 
		( ( !mir_strcmp(dbcws->szSetting,"e-mail") ||
		!mir_strcmp(dbcws->szSetting,"Mye-mail0") ||
		!mir_strcmp(dbcws->szSetting,"Cellular") )  ||
		( !mir_strcmp(dbcws->szModule,"UserInfo") &&
		( !mir_strcmp(dbcws->szSetting,"MyPhone0") || 
		!mir_strcmp(dbcws->szSetting,"Mye-mail0") ) ) ) ) )
		ExtraImage_SetAllExtraIcons(pcli->hwndContactTree,(HANDLE)wParam);
	return(0);
};
INT_PTR CLUI::Service_ShowMainMenu(WPARAM wParam,LPARAM lParam)
{
	HMENU hMenu;
	POINT pt;
	hMenu=(HMENU)CallService(MS_CLIST_MENUGETMAIN,0,0);
	GetCursorPos(&pt);
	TrackPopupMenu(hMenu,TPM_TOPALIGN|TPM_LEFTALIGN|TPM_LEFTBUTTON,pt.x,pt.y,0,pcli->hwndContactList,NULL);				
	return 0;
}
INT_PTR CLUI::Service_ShowStatusMenu(WPARAM wParam,LPARAM lParam)
{
	HMENU hMenu;
	POINT pt;
	hMenu=(HMENU)CallService(MS_CLIST_MENUGETSTATUS,0,0);
	GetCursorPos(&pt);
	TrackPopupMenu(hMenu,TPM_TOPALIGN|TPM_LEFTALIGN|TPM_LEFTBUTTON,pt.x,pt.y,0,pcli->hwndContactList,NULL);				
	return 0;
}

INT_PTR CLUI::Service_Menu_ShowContactAvatar(WPARAM wParam,LPARAM lParam)
{
	HANDLE hContact = (HANDLE) wParam;

	ModernWriteSettingByte(hContact, "CList", "HideContactAvatar", 0);

	pcli->pfnClcBroadcast( INTM_AVATARCHANGED,wParam,0);
	return 0;
}

INT_PTR CLUI::Service_Menu_HideContactAvatar(WPARAM wParam,LPARAM lParam)
{
	HANDLE hContact = (HANDLE) wParam;

	ModernWriteSettingByte(hContact, "CList", "HideContactAvatar", 1);

	pcli->pfnClcBroadcast( INTM_AVATARCHANGED,wParam,0);
	return 0;
}



HRESULT CLUI::CreateCluiFrames()
{
	g_hMenuMain=GetMenu(pcli->hwndContactList);
	MENUITEMINFO mii;
	ZeroMemory(&mii,sizeof(mii));
	mii.cbSize=MENUITEMINFO_V4_SIZE;
	mii.fMask=MIIM_SUBMENU;
	mii.hSubMenu=(HMENU)CallService(MS_CLIST_MENUGETMAIN,0,0);
	SetMenuItemInfo(g_hMenuMain,0,TRUE,&mii);
	mii.hSubMenu=(HMENU)CallService(MS_CLIST_MENUGETSTATUS,0,0);
	SetMenuItemInfo(g_hMenuMain,1,TRUE,&mii);

	CreateCLCWindow(CluiWnd());

	CLUI_ChangeWindowMode();

	RegisterAvatarMenu(); 

	CLUI_ReloadCLUIOptions();	

	CreateUIFrames();

	ModernHookEvent(ME_SYSTEM_MODULESLOADED,CLUI::OnEvent_ModulesLoaded);
	ModernHookEvent(ME_SKIN2_ICONSCHANGED,CLUI_IconsChanged);
    ModernHookEvent(ME_FONT_RELOAD, CLUI::OnEvent_FontReload);
	return S_OK;
}
CLUI::CLUI() :
m_hUserDll( NULL ),
m_hDwmapiDll( NULL )
{
	m_pCLUI = this;
	g_CluiData.bSTATE = STATE_CLUI_LOADING;
	LoadDllsRuntime();
	hFrameContactTree=NULL;

	CLUIServices_LoadModule();

	// Call InitGroup menus before
	GroupMenus_Init();

	CreateServiceFunction(MS_CLUI_SHOWMAINMENU,Service_ShowMainMenu);
	CreateServiceFunction(MS_CLUI_SHOWSTATUSMENU,Service_ShowStatusMenu);


	//TODO Add Row template loading here.

	RowHeight_InitModernRow();
	nLastRequiredHeight=0;

	LoadCLUIFramesModule();
	ExtraImage_LoadModule();	

	g_CluiData.boldHideOffline=-1;
	bOldHideOffline=ModernGetSettingByte(NULL,"CList","HideOffline",SETTING_HIDEOFFLINE_DEFAULT);

	g_CluiData.bOldUseGroups=-1;
	bOldUseGroups = ModernGetSettingByte( NULL,"CList","UseGroups", SETTING_USEGROUPS_DEFAULT );
}

CLUI::~CLUI()
{
	FreeLibrary(m_hUserDll);
	FreeLibrary(m_hDwmapiDll);
	m_pCLUI = NULL;
}

HRESULT CLUI::LoadDllsRuntime()
{
	m_hUserDll = LoadLibrary(TEXT("user32.dll"));
	if (m_hUserDll)
	{
		g_proc_UpdateLayeredWindow = (BOOL (WINAPI *)(HWND,HDC,POINT*,SIZE*,HDC,POINT*,COLORREF,BLENDFUNCTION*,DWORD))GetProcAddress(m_hUserDll, "UpdateLayeredWindow");
		g_proc_SetLayeredWindowAttributesNew = (BOOL (WINAPI *)(HWND,COLORREF,BYTE,DWORD))GetProcAddress(m_hUserDll, "SetLayeredWindowAttributes");
		g_proc_AnimateWindow=(BOOL (WINAPI*)(HWND,DWORD,DWORD))GetProcAddress(m_hUserDll,"AnimateWindow");

		g_CluiData.fLayered=(g_proc_UpdateLayeredWindow!=NULL) && !ModernGetSettingByte(NULL,"ModernData","DisableEngine", SETTING_DISABLESKIN_DEFAULT);
		g_CluiData.fSmoothAnimation=IsWinVer2000Plus()&&ModernGetSettingByte(NULL, "CLUI", "FadeInOut", SETTING_FADEIN_DEFAULT);
		g_CluiData.fLayered=(g_CluiData.fLayered*ModernGetSettingByte(NULL, "ModernData", "EnableLayering", g_CluiData.fLayered))&&!ModernGetSettingByte(NULL,"ModernData","DisableEngine", SETTING_DISABLESKIN_DEFAULT);
	}

    if (IsWinVerVistaPlus())
    {
	    m_hDwmapiDll = LoadLibrary(TEXT("dwmapi.dll"));
	    if (m_hDwmapiDll)
	    {
		    g_proc_DWMEnableBlurBehindWindow = (HRESULT (WINAPI *)(HWND, DWM_BLURBEHIND *))GetProcAddress(m_hDwmapiDll, "DwmEnableBlurBehindWindow");
	    }
    }
	g_CluiData.fAeroGlass = FALSE;

	return S_OK;
}
HRESULT CLUI::RegisterAvatarMenu()
{
	CLISTMENUITEM mi;

	ZeroMemory(&mi,sizeof(mi));
	mi.cbSize=sizeof(mi);
	mi.flags=0;
	mi.pszContactOwner=NULL;    //on every contact
	CreateServiceFunction("CList/ShowContactAvatar",CLUI::Service_Menu_ShowContactAvatar);
	mi.position=2000150000;
	mi.hIcon=LoadSmallIcon(g_hInst,MAKEINTRESOURCE(IDI_SHOW_AVATAR));
	mi.pszName=LPGEN("Show Contact &Avatar");
	mi.pszService="CList/ShowContactAvatar";
	hShowAvatarMenuItem = (HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM,0,(LPARAM)&mi);
	DestroyIcon_protect(mi.hIcon);

	CreateServiceFunction("CList/HideContactAvatar",CLUI::Service_Menu_HideContactAvatar);
	mi.position=2000150001;
	mi.hIcon=LoadSmallIcon(g_hInst,MAKEINTRESOURCE(IDI_HIDE_AVATAR));
	mi.pszName=LPGEN("Hide Contact &Avatar");
	mi.pszService="CList/HideContactAvatar";
	hHideAvatarMenuItem = (HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM,0,(LPARAM)&mi);
	DestroyIcon_protect(mi.hIcon);

	ModernHookEvent(ME_CLIST_PREBUILDCONTACTMENU, CLUI::OnEvent_ContactMenuPreBuild);

	return S_OK;
}
HRESULT CLUI::CreateCLCWindow(const HWND hwndClui)
{    
	ClcWnd() = CreateWindow(CLISTCONTROL_CLASS,TEXT(""),
		WS_CHILD|WS_CLIPCHILDREN|CLS_CONTACTLIST
		|(ModernGetSettingByte(NULL,"CList","UseGroups",SETTING_USEGROUPS_DEFAULT)?CLS_USEGROUPS:0)
		|(ModernGetSettingByte(NULL,"CList","HideOffline",SETTING_HIDEOFFLINE_DEFAULT)?CLS_HIDEOFFLINE:0)
		|(ModernGetSettingByte(NULL,"CList","HideEmptyGroups",SETTING_HIDEEMPTYGROUPS_DEFAULT)?CLS_HIDEEMPTYGROUPS:0
		|CLS_MULTICOLUMN
		),
		0,0,0,0,hwndClui,NULL,g_hInst,NULL);

	return S_OK;
}

HRESULT CLUI::CreateUIFrames()
{
	EventArea_Create(pcli->hwndContactList);
	CreateViewModeFrame();
	pcli->hwndStatus = StatusBar_Create(pcli->hwndContactList);    

	return S_OK;
}



HRESULT CLUI::FillAlphaChannel( HDC hDC, RECT * prcParent, BYTE bAlpha)
{

	RECT rcWindow;
	GetWindowRect( m_hWnd, &rcWindow );

	HRGN hRgn = CreateRectRgn(0,0,0,0);

	if ( GetWindowRgn(m_hWnd,hRgn)==ERROR )
	{
		DeleteObject(hRgn);
		hRgn=CreateRectRgn(rcWindow.left ,rcWindow.top ,rcWindow.right,rcWindow.bottom);
	}

	OffsetRgn(hRgn,-prcParent->left,-prcParent->top);

	RECT rcBounds;
	GetRgnBox(hRgn,&rcBounds);

	if ( IsRectEmpty(&rcBounds) ) 
	{
		DeleteObject(hRgn);
		return S_FALSE;
	}

	DWORD dwRgnSize = GetRegionData( hRgn, 0, NULL );
	RGNDATA * rgnData=(RGNDATA *)malloc(dwRgnSize);
	GetRegionData(hRgn,dwRgnSize,rgnData);

	RECT * pRect=(RECT *)rgnData->Buffer;

	for (DWORD i=0; i<rgnData->rdh.nCount; i++)
		ske_SetRectOpaque( hDC, &pRect[i] );

	free(rgnData);
	DeleteObject(hRgn);

	return S_OK;
}

HRESULT CLUI::CreateCLC()
{

	INIT<CLISTFrame> Frame;

	Frame.hWnd=ClcWnd();
	Frame.align=alClient;
	Frame.hIcon=LoadSkinnedIcon(SKINICON_OTHER_MIRANDA);
	Frame.Flags=F_VISIBLE|/*F_SHOWTB|*/F_SHOWTBTIP|F_NO_SUBCONTAINER|F_TCHAR;
	Frame.tname=LPGENT("My Contacts");
	Frame.TBtname=TranslateT("My Contacts");
	hFrameContactTree=(HWND)CallService(MS_CLIST_FRAMES_ADDFRAME,(WPARAM)&Frame,(LPARAM)0);

	CallService(MS_SKINENG_REGISTERPAINTSUB,(WPARAM)Frame.hWnd,(LPARAM)CLCPaint::PaintCallbackProc);
	CallService(MS_CLIST_FRAMES_SETFRAMEOPTIONS,MAKEWPARAM(FO_TBTIPNAME,hFrameContactTree),(LPARAM)Translate("My Contacts"));	

	ExtraImage_ReloadExtraIcons();

	nLastRequiredHeight=0;
	if ( g_CluiData.current_viewmode[0] == '\0' )
	{
		if (bOldHideOffline != (BYTE)-1) 
			CallService( MS_CLIST_SETHIDEOFFLINE,(WPARAM)bOldHideOffline, 0);
		else
			CallService( MS_CLIST_SETHIDEOFFLINE,(WPARAM)0, 0);
		if (bOldUseGroups !=(BYTE)-1)  
			CallService( MS_CLIST_SETUSEGROUPS ,(WPARAM)bOldUseGroups, 0);
		else
			CallService( MS_CLIST_SETUSEGROUPS ,(WPARAM)bOldUseGroups, 0);
	}
	nLastRequiredHeight=0;
	mutex_bDisableAutoUpdate=0;

	hSettingChangedHook=ModernHookEvent(ME_DB_CONTACT_SETTINGCHANGED,CLUI::OnEvent_DBSettingChanging);

	return S_OK;

};
HRESULT CLUI::SnappingToEdge( WINDOWPOS * lpWindowPos )
{ 
	//by ZORG
	if ( MyMonitorFromWindow == NULL || MyGetMonitorInfo == NULL )
		return S_FALSE;

	if (ModernGetSettingByte(NULL,"CLUI","SnapToEdges",SETTING_SNAPTOEDGES_DEFAULT))
	{
		RECT* dr;
		MONITORINFO monInfo;
		HMONITOR curMonitor = MyMonitorFromWindow(m_hWnd, MONITOR_DEFAULTTONEAREST);

		monInfo.cbSize = sizeof(monInfo);
		MyGetMonitorInfo(curMonitor, &monInfo);

		dr = &(monInfo.rcWork);

		// Left side
		if ( lpWindowPos->x < dr->left + SNAPTOEDGESENSIVITY && lpWindowPos->x > dr->left - SNAPTOEDGESENSIVITY && g_CluiData.bBehindEdgeSettings!=1)
			lpWindowPos->x = dr->left;

		// Right side
		if ( dr->right - lpWindowPos->x - lpWindowPos->cx <SNAPTOEDGESENSIVITY && dr->right - lpWindowPos->x - lpWindowPos->cx > -SNAPTOEDGESENSIVITY && g_CluiData.bBehindEdgeSettings!=2)
			lpWindowPos->x = dr->right - lpWindowPos->cx;

		// Top side
		if ( lpWindowPos->y < dr->top + SNAPTOEDGESENSIVITY && lpWindowPos->y > dr->top - SNAPTOEDGESENSIVITY)
			lpWindowPos->y = dr->top;

		// Bottom side
		if ( dr->bottom - lpWindowPos->y - lpWindowPos->cy <SNAPTOEDGESENSIVITY && dr->bottom - lpWindowPos->y - lpWindowPos->cy > -SNAPTOEDGESENSIVITY)
			lpWindowPos->y = dr->bottom - lpWindowPos->cy;
	}
	return S_OK;
}


HICON GetMainStatusOverlay(int STATUS)
{
	return ImageList_GetIcon(hAvatarOverlays,g_pStatusOverlayIcons[STATUS-ID_STATUS_OFFLINE].listID,ILD_NORMAL);
}

void UnloadAvatarOverlayIcon()
{
	int i;
	for (i = 0 ; i < MAX_REGS(g_pAvatarOverlayIcons) ; i++)
	{
		g_pAvatarOverlayIcons[i].listID=-1;
		g_pStatusOverlayIcons[i].listID=-1;
	}
	ImageList_Destroy(hAvatarOverlays);
	hAvatarOverlays=NULL;
	DestroyIcon_protect(g_hListeningToIcon);
	g_hListeningToIcon=NULL;
}


BOOL CLUI_CheckOwnedByClui(HWND hWnd)
{
	HWND hWndMid, hWndClui;
	if (!hWnd) return FALSE;
	hWndClui=pcli->hwndContactList;
	hWndMid=fnGetAncestor(hWnd,GA_ROOTOWNER);
	if(hWndMid==hWndClui) return TRUE;
	{
		TCHAR buf[255];
		GetClassName(hWndMid,buf,254);
		if (!mir_tstrcmpi(buf,CLUIFrameSubContainerClassName)) return TRUE;
	}
	return FALSE;
}

int CLUI_ShowWindowMod(HWND hWnd, int nCmd) 
{
	int res=0;

	if (hWnd==pcli->hwndContactList && (nCmd==SW_HIDE || nCmd==SW_MINIMIZE) )
	{	
		AniAva_InvalidateAvatarPositions(NULL);
		AniAva_RemoveInvalidatedAvatars();
	}

	if (hWnd==pcli->hwndContactList 
		&& !g_mutex_bChangingMode
		&& nCmd==SW_HIDE 
		&& !g_CluiData.fLayered 
		&& IsWinVerXPPlus()
		&& ModernGetSettingByte(NULL,"CList","WindowShadow",SETTING_WINDOWSHADOW_DEFAULT))
	{
		ShowWindow(hWnd,SW_MINIMIZE); //removing of shadow
		return ShowWindow(hWnd,nCmd);
	}
	if (hWnd==pcli->hwndContactList
		&& !g_mutex_bChangingMode
		&& nCmd==SW_RESTORE
		&& !g_CluiData.fLayered 
		&& IsWinVerXPPlus()		
		&& g_CluiData.fSmoothAnimation
		&& !g_bTransparentFlag 
		)	
	{	
		if(ModernGetSettingByte(NULL,"CList","WindowShadow",SETTING_WINDOWSHADOW_DEFAULT))
		{
			CLUI_SmoothAlphaTransition(hWnd, 255, 1);
		}
		else
		{

			int ret=ShowWindow(hWnd,nCmd);
			CLUI_SmoothAlphaTransition(hWnd, 255, 1);
			return ret;
		}
	}
	return ShowWindow(hWnd,nCmd);
}

static BOOL CLUI_WaitThreadsCompletion(HWND hwnd)
{
	static BYTE bEntersCount=0;
	static const BYTE bcMAX_AWAITING_RETRY = 10; //repeat awaiting only 10 times
	TRACE("CLUI_WaitThreadsCompletion Enter");
	if (bEntersCount<bcMAX_AWAITING_RETRY
		&&( g_mutex_nCalcRowHeightLock ||
		g_CluiData.mutexPaintLock || 
		g_dwAwayMsgThreadID || 
		g_dwGetTextAsyncThreadID || 
		g_dwSmoothAnimationThreadID || 
		g_dwFillFontListThreadID) 
		&&!Miranda_Terminated())
	{
		TRACE("Waiting threads");
		TRACEVAR("g_mutex_nCalcRowHeightLock: %x",g_mutex_nCalcRowHeightLock);
		TRACEVAR("g_CluiData.mutexPaintLock: %x",g_CluiData.mutexPaintLock);
		TRACEVAR("g_dwAwayMsgThreadID: %x",g_dwAwayMsgThreadID);
		TRACEVAR("g_dwGetTextAsyncThreadID: %x",g_dwGetTextAsyncThreadID);
		TRACEVAR("g_dwSmoothAnimationThreadID: %x",g_dwSmoothAnimationThreadID);
		TRACEVAR("g_dwFillFontListThreadID: %x",g_dwFillFontListThreadID);
		bEntersCount++;
		SleepEx(10,TRUE);
		return TRUE;
	}

	return FALSE;
}

void CLUI_UpdateLayeredMode()
{	
	g_CluiData.fDisableSkinEngine=ModernGetSettingByte(NULL,"ModernData","DisableEngine", SETTING_DISABLESKIN_DEFAULT);
	if (IsWinVer2000Plus())
	{
		BOOL tLayeredFlag=TRUE;
		tLayeredFlag&=(ModernGetSettingByte(NULL, "ModernData", "EnableLayering", tLayeredFlag) && !g_CluiData.fDisableSkinEngine);

		if (g_CluiData.fLayered != tLayeredFlag)
		{
			LONG exStyle;
			BOOL fWasVisible=IsWindowVisible(pcli->hwndContactList);
			if (fWasVisible) ShowWindow(pcli->hwndContactList,SW_HIDE);
			//change layered mode
			exStyle=GetWindowLong(pcli->hwndContactList,GWL_EXSTYLE);
			if (tLayeredFlag) 
				exStyle|=WS_EX_LAYERED;
			else
				exStyle&=~WS_EX_LAYERED;
			SetWindowLong(pcli->hwndContactList,GWL_EXSTYLE,exStyle&~WS_EX_LAYERED);
			SetWindowLong(pcli->hwndContactList,GWL_EXSTYLE,exStyle);
			g_CluiData.fLayered = tLayeredFlag;
			Sync(CLUIFrames_SetLayeredMode, tLayeredFlag,pcli->hwndContactList);			
			CLUI_ChangeWindowMode();
			Sync(CLUIFrames_OnClistResize_mod, (WPARAM)0, (LPARAM)0 );
			if (fWasVisible) ShowWindow(pcli->hwndContactList,SW_SHOW);
		}
	}
}

void CLUI_UpdateAeroGlass()
{
	BOOL tAeroGlass = ModernGetSettingByte(NULL, "ModernData", "AeroGlass", SETTING_AEROGLASS_DEFAULT) && (g_CluiData.fLayered);
	if (g_proc_DWMEnableBlurBehindWindow && (tAeroGlass != g_CluiData.fAeroGlass))
	{			
		if (g_CluiData.hAeroGlassRgn)
		{
			DeleteObject(g_CluiData.hAeroGlassRgn);
			g_CluiData.hAeroGlassRgn = 0;
		}

		DWM_BLURBEHIND bb = {0};
		bb.dwFlags = DWM_BB_ENABLE;
		bb.fEnable = tAeroGlass;

		if (tAeroGlass)
		{
			g_CluiData.hAeroGlassRgn = ske_CreateOpaqueRgn(AEROGLASS_MINALPHA, true);
			bb.hRgnBlur = g_CluiData.hAeroGlassRgn;
			bb.dwFlags |= DWM_BB_BLURREGION;
		}

		g_proc_DWMEnableBlurBehindWindow(pcli->hwndContactList, &bb);
		g_CluiData.fAeroGlass = tAeroGlass;
	}
}

extern int CLUIFrames_UpdateBorders();

void CLUI_ChangeWindowMode()
{
	BOOL storedVisMode=FALSE;
	LONG style,styleEx;
	LONG oldStyle,oldStyleEx;
	LONG styleMask=WS_CLIPCHILDREN|WS_BORDER|WS_CAPTION|WS_MINIMIZEBOX|WS_POPUPWINDOW|WS_CLIPCHILDREN|WS_THICKFRAME|WS_SYSMENU;
	LONG styleMaskEx=WS_EX_TOOLWINDOW|WS_EX_LAYERED;
	LONG curStyle,curStyleEx;
	if (!pcli->hwndContactList) return;

	g_mutex_bChangingMode=TRUE;
	g_bTransparentFlag=IsWinVer2000Plus()&&ModernGetSettingByte( NULL,"CList","Transparent",SETTING_TRANSPARENT_DEFAULT);
	g_CluiData.fSmoothAnimation=IsWinVer2000Plus()&&ModernGetSettingByte(NULL, "CLUI", "FadeInOut", SETTING_FADEIN_DEFAULT);
	if (g_bTransparentFlag==0 && g_CluiData.bCurrentAlpha!=0)
		g_CluiData.bCurrentAlpha=255;
	//2- Calculate STYLES and STYLESEX
	if (!g_CluiData.fLayered)
	{
		style=0;
		styleEx=0;
		if (ModernGetSettingByte(NULL,"CList","ThinBorder",SETTING_THINBORDER_DEFAULT) || (ModernGetSettingByte(NULL,"CList","NoBorder",SETTING_NOBORDER_DEFAULT)))
		{
			style=WS_CLIPCHILDREN| (ModernGetSettingByte(NULL,"CList","ThinBorder",SETTING_THINBORDER_DEFAULT)?WS_BORDER:0);
			styleEx=WS_EX_TOOLWINDOW;
			styleMaskEx |= WS_EX_APPWINDOW;
		} 
		else if (ModernGetSettingByte(NULL,"CLUI","ShowCaption",SETTING_SHOWCAPTION_DEFAULT) && ModernGetSettingByte(NULL,"CList","ToolWindow",SETTING_TOOLWINDOW_DEFAULT))
		{
			styleEx=WS_EX_TOOLWINDOW/*|WS_EX_WINDOWEDGE*/;
			style=WS_CAPTION|WS_POPUPWINDOW|WS_CLIPCHILDREN|WS_THICKFRAME;
			styleMaskEx |= WS_EX_APPWINDOW;
		}
		else if (ModernGetSettingByte(NULL,"CLUI","ShowCaption",SETTING_SHOWCAPTION_DEFAULT))
		{
			style=WS_CAPTION|WS_SYSMENU|WS_POPUPWINDOW|WS_CLIPCHILDREN|WS_THICKFRAME|WS_MINIMIZEBOX;
		}
		else 
		{
			style=WS_POPUPWINDOW|WS_CLIPCHILDREN|WS_THICKFRAME;
			styleEx=WS_EX_TOOLWINDOW/*|WS_EX_WINDOWEDGE*/;
			styleMaskEx |= WS_EX_APPWINDOW;
		}
	}
	else 
	{
		style=WS_CLIPCHILDREN;
		styleEx=WS_EX_TOOLWINDOW;
		styleMaskEx |= WS_EX_APPWINDOW;
	}
	//3- TODO Update Layered mode
	if(g_bTransparentFlag&&g_CluiData.fLayered)
		styleEx|=WS_EX_LAYERED;

	//4- Set Title
	{
		TCHAR titleText[255]={0};
		DBVARIANT dbv={0};
		if(ModernGetSettingTString(NULL,"CList","TitleText",&dbv))
			lstrcpyn(titleText,TEXT(MIRANDANAME),SIZEOF(titleText));
		else 
		{
			lstrcpyn(titleText,dbv.ptszVal,SIZEOF(titleText));
			ModernDBFreeVariant(&dbv);
		}
		SetWindowText(pcli->hwndContactList,titleText);
	}
	//<->
	//1- If visible store it and hide

	if (g_CluiData.fLayered && (ModernGetSettingByte(NULL,"CList","OnDesktop", SETTING_ONDESKTOP_DEFAULT)))// && !flag_bFirstTimeCall))
	{
		SetParent(pcli->hwndContactList,NULL);
		Sync( CLUIFrames_SetParentForContainers, (HWND) NULL );
		UpdateWindow(pcli->hwndContactList);
		g_CluiData.fOnDesktop=0;
	}
	//5- TODO Apply Style
	oldStyleEx = curStyleEx = GetWindowLong(pcli->hwndContactList,GWL_EXSTYLE);
	oldStyle = curStyle = GetWindowLong(pcli->hwndContactList,GWL_STYLE);	

	curStyleEx = (curStyleEx & ~styleMaskEx) | styleEx;
	curStyle = (curStyle & ~styleMask) | style;
	if (oldStyleEx != curStyleEx || oldStyle != curStyle)
	{
		if (IsWindowVisible(pcli->hwndContactList)) 
		{
			storedVisMode=TRUE;
			mutex_bShowHideCalledFromAnimation=TRUE;
			ShowWindow(pcli->hwndContactList,SW_HIDE);
			Sync(CLUIFrames_OnShowHide, pcli->hwndContactList,0);
		}
		SetWindowLong(pcli->hwndContactList,GWL_EXSTYLE,curStyleEx);
		SetWindowLong(pcli->hwndContactList,GWL_STYLE,curStyle);
	}

	CLUI_UpdateAeroGlass();

	if(g_CluiData.fLayered || !ModernGetSettingByte(NULL,"CLUI","ShowMainMenu",SETTING_SHOWMAINMENU_DEFAULT)) 
    {
        HMENU m = GetMenu(pcli->hwndContactList);
		SetMenu(pcli->hwndContactList,NULL);
    }
	else
		SetMenu(pcli->hwndContactList,g_hMenuMain);

	if (g_CluiData.fLayered&&(ModernGetSettingByte(NULL,"CList","OnDesktop", SETTING_ONDESKTOP_DEFAULT)))
		ske_UpdateWindowImage();


	//6- Pin to desktop mode
	if (ModernGetSettingByte(NULL,"CList","OnDesktop", SETTING_ONDESKTOP_DEFAULT))
	{
		HWND hProgMan=FindWindow(TEXT("Progman"),NULL);
		if (IsWindow(hProgMan)) 
		{
			SetParent(pcli->hwndContactList,hProgMan);
			Sync( CLUIFrames_SetParentForContainers, (HWND) hProgMan );
			g_CluiData.fOnDesktop=1;
		}
	} 
	else 
	{
		//	HWND parent=GetParent(pcli->hwndContactList);
		//	HWND progman=FindWindow(TEXT("Progman"),NULL);
		//	if (parent==progman)
		{
			SetParent(pcli->hwndContactList,NULL);
			Sync(CLUIFrames_SetParentForContainers, (HWND) NULL);
		}
		g_CluiData.fOnDesktop=0;
	}

	//7- if it was visible - show
	if (storedVisMode) 
	{
		ShowWindow(pcli->hwndContactList,SW_SHOW);
		Sync(CLUIFrames_OnShowHide, pcli->hwndContactList,1);
	}
	mutex_bShowHideCalledFromAnimation=FALSE;

	CLUIFrames_UpdateBorders();

	if (!g_CluiData.fLayered)
	{
		HRGN hRgn1;
		RECT r;
		int v,h;
		int w=10;
		GetWindowRect(pcli->hwndContactList,&r);
		h=(r.right-r.left)>(w*2)?w:(r.right-r.left);
		v=(r.bottom-r.top)>(w*2)?w:(r.bottom-r.top);
		h=(h<v)?h:v;
		hRgn1=CreateRoundRectRgn(0,0,(r.right-r.left+1),(r.bottom-r.top+1),h,h);
		if ((ModernGetSettingByte(NULL,"CLC","RoundCorners",SETTING_ROUNDCORNERS_DEFAULT)) && (!CallService(MS_CLIST_DOCKINGISDOCKED,0,0)))
			SetWindowRgn(pcli->hwndContactList,hRgn1,1); 
		else 
		{
			DeleteObject(hRgn1);
			SetWindowRgn(pcli->hwndContactList,NULL,1);
		}

		RedrawWindow(pcli->hwndContactList,NULL,NULL,RDW_INVALIDATE|RDW_ERASE|RDW_FRAME|RDW_UPDATENOW|RDW_ALLCHILDREN);   
	} 			
	g_mutex_bChangingMode=FALSE;
	flag_bFirstTimeCall=TRUE;
	AniAva_UpdateParent();
}
struct  _tagTimerAsync
{
	HWND hwnd;
	int ID; 
	int Timeout; 
	TIMERPROC proc;
};
static UINT_PTR SetTimerSync(WPARAM wParam , LPARAM lParam)
{
	struct  _tagTimerAsync * call=(struct  _tagTimerAsync *) wParam;
	return SetTimer(call->hwnd, call->ID ,call->Timeout, call->proc);
}

UINT_PTR CLUI_SafeSetTimer(HWND hwnd, int ID, int Timeout, TIMERPROC proc)
{
	struct  _tagTimerAsync param={  hwnd, ID, Timeout, proc };
	return Sync(SetTimerSync, (WPARAM) &param, (LPARAM) 0);		
}

int CLUI_UpdateTimer(BYTE BringIn)
{  
	if (g_CluiData.nBehindEdgeState==0)
	{
		KillTimer(pcli->hwndContactList,TM_BRINGOUTTIMEOUT);
		CLUI_SafeSetTimer(pcli->hwndContactList,TM_BRINGOUTTIMEOUT,wBehindEdgeHideDelay*100,NULL);
	}
	if (bShowEventStarted==0 && g_CluiData.nBehindEdgeState>0 ) 
	{
		KillTimer(pcli->hwndContactList,TM_BRINGINTIMEOUT);
		bShowEventStarted=(BOOL)CLUI_SafeSetTimer(pcli->hwndContactList,TM_BRINGINTIMEOUT,wBehindEdgeShowDelay*100,NULL);
	}
	return 0;
}

int CLUI_HideBehindEdge()
{
	int method=g_CluiData.bBehindEdgeSettings;
	if (method)
	{
		// if (DBGetContactSettingByte(NULL, "ModernData", "BehindEdge", SETTING_BEHINDEDGE_DEFAULT)==0)
		{
			RECT rcScreen;
			RECT rcWindow;
			int bordersize=0;
			//Need to be moved out of screen
			bShowEventStarted=0;
			//1. get work area rectangle 
			Docking_GetMonitorRectFromWindow(pcli->hwndContactList,&rcScreen);
			//SystemParametersInfo(SPI_GETWORKAREA,0,&rcScreen,FALSE);
			//2. move out
			bordersize=wBehindEdgeBorderSize;
			GetWindowRect(pcli->hwndContactList,&rcWindow);
			switch (method)
			{
			case 1: //left
				rcWindow.left=rcScreen.left-(rcWindow.right-rcWindow.left)+bordersize;
				break;
			case 2: //right
				rcWindow.left=rcScreen.right-bordersize;
				break;
			}
			g_CluiData.mutexPreventDockMoving=0;
			SetWindowPos(pcli->hwndContactList,NULL,rcWindow.left,rcWindow.top,0,0,SWP_NOZORDER|SWP_NOSIZE|SWP_NOACTIVATE);
			Sync(CLUIFrames_OnMoving,pcli->hwndContactList,&rcWindow);
			g_CluiData.mutexPreventDockMoving=1;

			//3. store setting
			ModernWriteSettingByte(NULL, "ModernData", "BehindEdge",method);
			g_CluiData.nBehindEdgeState=method;
			return 1;
		}
		return 2;
	}
	return 0;
}


int CLUI_ShowFromBehindEdge()
{
	int method=g_CluiData.bBehindEdgeSettings;
	bShowEventStarted=0;
	if (g_mutex_bOnTrayRightClick) 
	{
		g_mutex_bOnTrayRightClick=0;
		return 0;
	}
	if (method)// && (DBGetContactSettingByte(NULL, "ModernData", "BehindEdge", SETTING_BEHINDEDGE_DEFAULT)==0))
	{
		RECT rcScreen;
		RECT rcWindow;
		int bordersize=0;
		//Need to be moved out of screen

		//1. get work area rectangle 
		//SystemParametersInfo(SPI_GETWORKAREA,0,&rcScreen,FALSE);
		Docking_GetMonitorRectFromWindow(pcli->hwndContactList,&rcScreen);
		//2. move out
		bordersize=wBehindEdgeBorderSize;
		GetWindowRect(pcli->hwndContactList,&rcWindow);
		switch (method)
		{
		case 1: //left
			rcWindow.left=rcScreen.left;
			break;
		case 2: //right
			rcWindow.left=rcScreen.right-(rcWindow.right-rcWindow.left);
			break;
		}
		g_CluiData.mutexPreventDockMoving=0;
		SetWindowPos(pcli->hwndContactList,NULL,rcWindow.left,rcWindow.top,0,0,SWP_NOZORDER|SWP_NOSIZE);
		Sync(CLUIFrames_OnMoving,pcli->hwndContactList,&rcWindow);
		g_CluiData.mutexPreventDockMoving=1;

		//3. store setting
		ModernWriteSettingByte(NULL, "ModernData", "BehindEdge",0);
		g_CluiData.nBehindEdgeState=0;
	}
	return 0;
}


int CLUI_IsInMainWindow(HWND hwnd)
{
	if (hwnd==pcli->hwndContactList) return 1;
	if (GetParent(hwnd)==pcli->hwndContactList) return 2;
	return 0;
}

int CLUI_OnSkinLoad(WPARAM wParam, LPARAM lParam)
{
	ske_LoadSkinFromDB();

	return 0;
}




static LPPROTOTICKS CLUI_GetProtoTicksByProto(char * szProto)
{
	int i;

	for (i=0;i<64;i++)
	{
		if (CycleStartTick[i].szProto==NULL) break;
		if (mir_strcmp(CycleStartTick[i].szProto,szProto)) continue;
		return(&CycleStartTick[i]);
	}
	for (i=0;i<64;i++)
	{
		if (CycleStartTick[i].szProto==NULL)
		{
			CycleStartTick[i].szProto=mir_strdup(szProto);
			CycleStartTick[i].nCycleStartTick=0;
			CycleStartTick[i].nIndex=i;			
			CycleStartTick[i].bGlobal=( szProto[0]==0 );
			CycleStartTick[i].himlIconList=NULL;
			return(&CycleStartTick[i]);
		}
	}
	return (NULL);
}

static int CLUI_GetConnectingIconForProtoCount(char *szAccoName)
{
	char fileFull[MAX_PATH];
	static char szFolderPath[MAX_PATH] = "";

	int count = 8;

	if ( !szFolderPath[0] )
	{
		char szRelativePath[MAX_PATH];
		GetModuleFileNameA(GetModuleHandle(NULL), szRelativePath, MAX_PATH);
		char *str = strrchr( szRelativePath, '\\' ); 
		if( str != NULL ) *str=0;
		CallService(MS_UTILS_PATHTOABSOLUTE, (WPARAM)szRelativePath, (LPARAM)szFolderPath);
	}

	if ( szAccoName )
	{
		// first of all try to find by account name( or empty - global )
		mir_snprintf( fileFull, SIZEOF(fileFull), "%s\\Icons\\proto_conn_%s.dll", szFolderPath, szAccoName );
		count = ExtractIconExA(fileFull,-1,NULL,NULL,1);
		if ( count ) return count;

		if ( szAccoName[0] )
		{
			// second try to find by protocol name
			PROTOACCOUNT * acc = ProtoGetAccount( szAccoName );
			if ( acc && !acc->bOldProto )
			{
				mir_snprintf( fileFull, SIZEOF(fileFull), "%s\\Icons\\proto_conn_%s.dll", szFolderPath, acc->szProtoName );
				count = ExtractIconExA(fileFull,-1,NULL,NULL,1);
				if ( count ) return count;
			}
		}
	}
	// third try global
	mir_snprintf( fileFull, SIZEOF(fileFull), "%s\\Icons\\proto_conn.dll", szFolderPath );
	count = ExtractIconExA(fileFull,-1,NULL,NULL,1);
	if ( count ) return count;

	return 8;
}

static HICON CLUI_ExtractIconFromPath(const char *path, BOOL * needFree)
{
	char *comma;
	char file[MAX_PATH],fileFull[MAX_PATH];
	int n;
	HICON hIcon;
	lstrcpynA(file,path,sizeof(file));
	comma=strrchr(file,',');
	if(comma==NULL) n=0;
	else {n=atoi(comma+1); *comma=0;}
	CallService(MS_UTILS_PATHTOABSOLUTE, (WPARAM)file, (LPARAM)fileFull);
	hIcon=NULL;
	ExtractIconExA(fileFull,n,NULL,&hIcon,1);
	if (needFree)
	{
		*needFree=(hIcon!=NULL);
	}
	return hIcon;
}

HICON CLUI_LoadIconFromExternalFile(char *filename,int i,boolean UseLibrary,boolean registerit,char *IconName,char *SectName,char *Description,int internalidx, BOOL * needFree)
{
	char szPath[MAX_PATH],szMyPath[MAX_PATH], szFullPath[MAX_PATH],*str;
	HICON hIcon=NULL;
	BOOL has_proto_icon=FALSE;
	SKINICONDESC sid={0};
	if (needFree) *needFree=FALSE;
	GetModuleFileNameA(GetModuleHandle(NULL), szPath, MAX_PATH);
	GetModuleFileNameA(g_hInst, szMyPath, MAX_PATH);
	str=strrchr(szPath,'\\');
	if(str!=NULL) *str=0;
	if (UseLibrary&2) 
		mir_snprintf(szMyPath, SIZEOF(szMyPath), "%s\\Icons\\%s", szPath, filename);
	mir_snprintf(szFullPath, SIZEOF(szFullPath), "%s\\Icons\\%s,%d", szPath, filename, i);
	if(str!=NULL) *str='\\';
	if (UseLibrary&2)
	{
		BOOL nf;
		HICON hi=CLUI_ExtractIconFromPath(szFullPath,&nf);
		if (hi) has_proto_icon=TRUE;
		if (hi && nf) DestroyIcon(hi);
	}
	if (!UseLibrary||!ServiceExists(MS_SKIN2_ADDICON))
	{		
		hIcon=CLUI_ExtractIconFromPath(szFullPath,needFree);
		if (hIcon) return hIcon;
		if (UseLibrary)
		{
			mir_snprintf(szFullPath, SIZEOF(szFullPath), "%s,%d", szMyPath, internalidx);
			hIcon=CLUI_ExtractIconFromPath(szFullPath,needFree);
			if (hIcon) return hIcon;		
		}
	}
	else
	{
		if (registerit&&IconName!=NULL&&SectName!=NULL)	
		{
			sid.cbSize = sizeof(sid);
			sid.cx=16;
			sid.cy=16;
			sid.hDefaultIcon = (has_proto_icon||!(UseLibrary&2))?NULL:(HICON)CallService(MS_SKIN_LOADPROTOICON,(WPARAM)NULL,(LPARAM)(-internalidx));
			sid.pszSection=SectName;
			sid.pszName=IconName;
			sid.pszDescription=Description;
			sid.pszDefaultFile=internalidx<0?szMyPath:szPath;

			sid.iDefaultIndex=(UseLibrary&2)?i:(internalidx<0)?internalidx:-internalidx;
			CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);
		}
		return ((HICON)CallService(MS_SKIN2_GETICON, 0, (LPARAM)IconName));
	}




	return (HICON)0;
}

static HICON CLUI_GetConnectingIconForProto(char *szAccoName, int b)
{
	char szFullPath[MAX_PATH];
	HICON hIcon=NULL;
	BOOL needFree;
	b=b-1;
	
	if ( szAccoName )
	{
		mir_snprintf(szFullPath, SIZEOF(szFullPath), "proto_conn_%s.dll",szAccoName);
		hIcon = CLUI_LoadIconFromExternalFile(szFullPath,b+1,FALSE,FALSE,NULL,NULL,NULL,0,&needFree);
		if (hIcon) return hIcon;

		if ( szAccoName[0] )
		{
			// second try to find by protocol name
			PROTOACCOUNT * acc = ProtoGetAccount( szAccoName );
			if ( acc && !acc->bOldProto )
			{
				mir_snprintf( szFullPath, SIZEOF(szFullPath), "proto_conn_%s.dll", acc->szProtoName );
				hIcon = CLUI_LoadIconFromExternalFile(szFullPath,b+1,FALSE,FALSE,NULL,NULL,NULL,0,&needFree);
				if ( hIcon ) return hIcon;
			}
		}
	}
	// third try global
	mir_snprintf( szFullPath, SIZEOF(szFullPath), "proto_conn.dll" );
	hIcon = CLUI_LoadIconFromExternalFile(szFullPath,b+1,FALSE,FALSE,NULL,NULL,NULL,0,&needFree);
	if ( hIcon ) return hIcon;

	hIcon=LoadSmallIcon(g_hInst,(TCHAR *)(IDI_ICQC1+b+1));
	return(hIcon);
}


INT_PTR CLUI_GetConnectingIconService(WPARAM wParam,LPARAM lParam)
{
	int b;						
	PROTOTICKS *pt=NULL;
	HICON hIcon=NULL;

	char *szProto=(char *)wParam;
	if (!szProto) return 0;

	pt=CLUI_GetProtoTicksByProto(szProto);

	if (pt!=NULL)
	{
		if (pt->nCycleStartTick==0)
		{
			CLUI_CreateTimerForConnectingIcon(ID_STATUS_CONNECTING,wParam);
			pt=CLUI_GetProtoTicksByProto(szProto);
		}
	}
	if (pt!=NULL)
	{
		if (pt->nCycleStartTick!=0&&pt->nIconsCount!=0) 
		{					
			b=((GetTickCount()-pt->nCycleStartTick)/(nAnimatedIconStep))%(pt->nIconsCount);
			//	if (lParam)
			//		hIcon=CLUI_GetConnectingIconForProto("Global",b);
			//	else
			if (pt->himlIconList)
				hIcon=ske_ImageList_GetIcon(pt->himlIconList,b,ILD_NORMAL);
			else
				hIcon=NULL;
			//hIcon=CLUI_GetConnectingIconForProto(szProto,b);
		};
	}


	return (INT_PTR)hIcon;
};


static int CLUI_CreateTimerForConnectingIcon(WPARAM wParam,LPARAM lParam)
{

	int status=(int)wParam;
	char *szProto=(char *)lParam;					
	if (!szProto) return (0);
	if (!status) return (0);

	if ((g_StatusBarData.connectingIcon==1)&&status>=ID_STATUS_CONNECTING&&status<=ID_STATUS_CONNECTING+MAX_CONNECT_RETRIES)
	{

		PROTOTICKS *pt=NULL;
		int cnt;

		pt=CLUI_GetProtoTicksByProto(szProto);
		if (pt!=NULL)
		{
			if (pt->nCycleStartTick==0) 
			{					
				KillTimer(pcli->hwndContactList,TM_STATUSBARUPDATE+pt->nIndex);
				cnt=CLUI_GetConnectingIconForProtoCount(szProto);
				if (cnt!=0)
				{
					int i=0;
					nAnimatedIconStep=100;/*DBGetContactSettingWord(NULL,"CLUI","DefaultStepConnectingIcon",100);*/
					pt->nIconsCount=cnt;
					if (pt->himlIconList) ImageList_Destroy(pt->himlIconList);
					pt->himlIconList=ImageList_Create(16,16,ILC_MASK|ILC_COLOR32,cnt,1);
					for (i=0; i<cnt; i++)
					{
						HICON ic=CLUI_GetConnectingIconForProto(szProto,i);
						if (ic) ImageList_AddIcon(pt->himlIconList,ic);
						DestroyIcon_protect(ic);
					}
					CLUI_SafeSetTimer(pcli->hwndContactList,TM_STATUSBARUPDATE+pt->nIndex,(int)(nAnimatedIconStep)/1,0);
					pt->bTimerCreated=1;
					pt->nCycleStartTick=GetTickCount();
				}

			};
		};
	}
	return 0;
}

static BOOL CALLBACK BroadcastEnumChildProc(HWND hwndChild, LPARAM lParam) 
{
	MSG * pMsg=(MSG*)lParam;
	SendNotifyMessage( hwndChild, pMsg->message, pMsg->wParam, pMsg->lParam );
	EnumChildWindows( hwndChild, BroadcastEnumChildProc, lParam );
	return TRUE;
}

static LRESULT BroadCastMessageToChild(HWND hwnd, int message, WPARAM wParam, LPARAM lParam )
{
	MSG msg={0};
	msg.hwnd=hwnd;
	msg.lParam=lParam;
	msg.wParam=wParam;
	msg.message=message;
	EnumChildWindows(hwnd, BroadcastEnumChildProc, (LPARAM) &msg);
	return 1;
}

int CLUI_ReloadCLUIOptions()
{
	KillTimer(pcli->hwndContactList,TM_UPDATEBRINGTIMER);
	g_CluiData.bBehindEdgeSettings=ModernGetSettingByte(NULL, "ModernData", "HideBehind", SETTING_HIDEBEHIND_DEFAULT);
	wBehindEdgeShowDelay=ModernGetSettingWord(NULL,"ModernData","ShowDelay",SETTING_SHOWDELAY_DEFAULT);
	wBehindEdgeHideDelay=ModernGetSettingWord(NULL,"ModernData","HideDelay",SETTING_HIDEDELAY_DEFAULT);
	wBehindEdgeBorderSize=ModernGetSettingWord(NULL,"ModernData","HideBehindBorderSize",SETTING_HIDEBEHINDBORDERSIZE_DEFAULT);

	g_CluiData.fAutoSize=ModernGetSettingByte(NULL,"CLUI","AutoSize",SETTING_AUTOSIZE_DEFAULT);
	g_CluiData.bInternalAwayMsgDiscovery = ModernGetSettingByte(NULL,"ModernData","InternalAwayMsgDiscovery",SETTING_INTERNALAWAYMSGREQUEST_DEFAULT);
	g_CluiData.bRemoveAwayMessageForOffline =   ModernGetSettingByte(NULL,"ModernData","RemoveAwayMessageForOffline",SETTING_REMOVEAWAYMSGFOROFFLINE_DEFAULT);
	//window borders
	if (g_CluiData.fDisableSkinEngine) {
		g_CluiData.LeftClientMargin=0;
		g_CluiData.RightClientMargin=0; 
		g_CluiData.TopClientMargin=0;
		g_CluiData.BottomClientMargin=0;
	} else {
		//window borders
		g_CluiData.LeftClientMargin=(int)ModernGetSettingByte(NULL,"CLUI","LeftClientMargin",SETTING_LEFTCLIENTMARIGN_DEFAULT);
		g_CluiData.RightClientMargin=(int)ModernGetSettingByte(NULL,"CLUI","RightClientMargin",SETTING_RIGHTCLIENTMARIGN_DEFAULT); 
		g_CluiData.TopClientMargin=(int)ModernGetSettingByte(NULL,"CLUI","TopClientMargin",SETTING_TOPCLIENTMARIGN_DEFAULT);
		g_CluiData.BottomClientMargin=(int)ModernGetSettingByte(NULL,"CLUI","BottomClientMargin",SETTING_BOTTOMCLIENTMARIGN_DEFAULT);
	}			
	BroadCastMessageToChild(pcli->hwndContactList, WM_THEMECHANGED, 0, 0);

	NotifyEventHooks(g_CluiData.hEventBkgrChanged, 0, 0);
	return 0;
}

void CLUI_DisconnectAll()
{
	PROTOACCOUNT **accs;
	int nProtoCount;
	int nProto;

	ProtoEnumAccounts( &nProtoCount, &accs );
	for (nProto = 0; nProto < nProtoCount; nProto++)
        if ( IsAccountEnabled( accs[nProto] ))
		    CallProtoService( accs[nProto]->szModuleName, PS_SETSTATUS, ID_STATUS_OFFLINE, 0 );
}

static int CLUI_DrawMenuBackGround(HWND hwnd, HDC hdc, int item, int state)
{
	RECT ra,r1;
	//    HBRUSH hbr;
	HRGN treg,treg2;
	struct ClcData * dat;

	if ( !fnGetMenuBarInfo )
		return 1;

	dat=(struct ClcData*)GetWindowLongPtr(pcli->hwndContactTree,0);
	if (!dat) return 1;
	GetWindowRect(hwnd,&ra);
	{
		MENUBARINFO mbi={0};
		mbi.cbSize=sizeof(MENUBARINFO);
		fnGetMenuBarInfo(hwnd,OBJID_MENU, 0, &mbi);
		if (!(mbi.rcBar.right-mbi.rcBar.left>0 && mbi.rcBar.bottom-mbi.rcBar.top>0)) return 1;
		r1=mbi.rcBar;  
		r1.bottom+= !ModernGetSettingByte(NULL,"CLUI","LineUnderMenu",SETTING_LINEUNDERMENU_DEFAULT);
		if (item<1)
		{           
			treg=CreateRectRgn(mbi.rcBar.left,mbi.rcBar.top,mbi.rcBar.right,r1.bottom);
			if (item==0)  //should remove item clips
			{
				int t;
				for (t=1; t<=2; t++)
				{
					fnGetMenuBarInfo(hwnd,OBJID_MENU, t, &mbi);
					treg2=CreateRectRgn(mbi.rcBar.left,mbi.rcBar.top,mbi.rcBar.right,mbi.rcBar.bottom);
					CombineRgn(treg,treg,treg2,RGN_DIFF);
					DeleteObject(treg2);  
				}

			}
		}
		else
		{
			fnGetMenuBarInfo(hwnd,OBJID_MENU, item, &mbi);
			treg=CreateRectRgn(mbi.rcBar.left,mbi.rcBar.top,mbi.rcBar.right,mbi.rcBar.bottom+!ModernGetSettingByte(NULL,"CLUI","LineUnderMenu",SETTING_LINEUNDERMENU_DEFAULT));
		}
		OffsetRgn(treg,-ra.left,-ra.top);
		r1.left-=ra.left;
		r1.top-=ra.top;
		r1.bottom-=ra.top;
		r1.right-=ra.left;
	}   
	//SelectClipRgn(hdc,NULL);
	SelectClipRgn(hdc,treg);
	DeleteObject(treg); 
	{
		RECT rc;
		HWND hwnd=pcli->hwndContactList;
		GetWindowRect(hwnd,&rc);
		OffsetRect(&rc,-rc.left, -rc.top);
		FillRect(hdc,&r1,GetSysColorBrush(COLOR_MENU));
		ske_SetRectOpaque(hdc,&r1);
		//ske_BltBackImage(hwnd,hdc,&r1);
	}
	if (!g_CluiData.fDisableSkinEngine)
		SkinDrawGlyph(hdc,&r1,&r1,"Main,ID=MenuBar");
	else
	{
		HBRUSH hbr=NULL;
		if (dat->hMenuBackground)
		{
			BITMAP bmp;
			HBITMAP oldbm;
			HDC hdcBmp;
			int x,y;
			int maxx,maxy;
			int destw,desth;
			RECT clRect=r1;


			// XXX: Halftone isnt supported on 9x, however the scretch problems dont happen on 98.
			SetStretchBltMode(hdc, HALFTONE);

			GetObject(dat->hMenuBackground,sizeof(bmp),&bmp);
			hdcBmp=CreateCompatibleDC(hdc);
			oldbm=(HBITMAP)SelectObject(hdcBmp,dat->hMenuBackground);
			y=clRect.top;
			x=clRect.left;
			maxx=dat->MenuBmpUse&CLBF_TILEH?maxx=r1.right:x+1;
			maxy=dat->MenuBmpUse&CLBF_TILEV?maxy=r1.bottom:y+1;
			switch(dat->MenuBmpUse&CLBM_TYPE) {
	case CLB_STRETCH:
		if(dat->MenuBmpUse&CLBF_PROPORTIONAL) {
			if(clRect.right-clRect.left*bmp.bmHeight<clRect.bottom-clRect.top*bmp.bmWidth) 
			{
				desth=clRect.bottom-clRect.top;
				destw=desth*bmp.bmWidth/bmp.bmHeight;
			}
			else 
			{
				destw=clRect.right-clRect.left;
				desth=destw*bmp.bmHeight/bmp.bmWidth;
			}
		}
		else {
			destw=clRect.right-clRect.left;
			desth=clRect.bottom-clRect.top;
		}
		break;
	case CLB_STRETCHH:
		if(dat->MenuBmpUse&CLBF_PROPORTIONAL) {
			destw=clRect.right-clRect.left;
			desth=destw*bmp.bmHeight/bmp.bmWidth;
		}
		else {
			destw=clRect.right-clRect.left;
			desth=bmp.bmHeight;
		}
		break;
	case CLB_STRETCHV:
		if(dat->MenuBmpUse&CLBF_PROPORTIONAL) {
			desth=clRect.bottom-clRect.top;
			destw=desth*bmp.bmWidth/bmp.bmHeight;
		}
		else {
			destw=bmp.bmWidth;
			desth=clRect.bottom-clRect.top;
		}
		break;
	default:    //clb_topleft
		destw=bmp.bmWidth;
		desth=bmp.bmHeight;					
		break;
			}
			if (desth && destw)
				for(y=clRect.top;y<maxy;y+=desth) {
					for(x=clRect.left;x<maxx;x+=destw)
						StretchBlt(hdc,x,y,destw,desth,hdcBmp,0,0,bmp.bmWidth,bmp.bmHeight,SRCCOPY);
				}
				SelectObject(hdcBmp,oldbm);
				DeleteDC(hdcBmp);

		}                  

		else
		{
			hbr=CreateSolidBrush(dat->MenuBkColor);
			FillRect(hdc,&r1,hbr);
			DeleteObject(hbr);    
		}
		if (item!=0 && state&(ODS_SELECTED)) 
		{
			hbr=CreateSolidBrush(dat->MenuBkHiColor);
			FillRect(hdc,&r1,hbr);
			DeleteObject(hbr);
		}
	}
	SelectClipRgn(hdc,NULL);
	return 0;
}

int CLUI_SizingGetWindowRect(HWND hwnd,RECT * rc)
{
	if (mutex_bDuringSizing && hwnd==pcli->hwndContactList)
		*rc=rcSizingRect;
	else
		GetWindowRect(hwnd,rc);
	return 1;
}


int CLUI_SyncGetPDNCE(WPARAM wParam, LPARAM lParam)
{
	//log0("CLUI_SyncGetPDNCE");
	return CListSettings_GetCopyFromCache((pdisplayNameCacheEntry)lParam, wParam ? (DWORD) wParam : CCI_ALL );
}

int CLUI_SyncSetPDNCE(WPARAM wParam, LPARAM lParam)
{
	//log0("CLUI_SyncSetPDNCE");
	return CListSettings_SetToCache((pdisplayNameCacheEntry)lParam, wParam ?  (DWORD) wParam : CCI_ALL );
}

int CLUI_SyncGetShortData(WPARAM wParam, LPARAM lParam)
{
	HWND hwnd=(HWND) wParam;
	struct ClcData * dat=(struct ClcData * )GetWindowLongPtr(hwnd,0);
	//log0("CLUI_SyncGetShortData");
	return ClcGetShortData(dat,(struct SHORTDATA *)lParam);
}

int CLUI_SyncSmoothAnimation(WPARAM wParam, LPARAM lParam)
{
	return CLUI_SmoothAlphaThreadTransition((HWND)lParam);
}



int CLUI_IconsChanged(WPARAM wParam,LPARAM lParam)
{
	if (MirandaExiting()) return 0;
	DrawMenuBar(pcli->hwndContactList);
	ExtraImage_ReloadExtraIcons();
	ExtraImage_SetAllExtraIcons(pcli->hwndContactTree,0);
	// need to update tray cause it use combined icons
	pcli->pfnTrayIconIconsChanged();  //TODO: remove as soon as core will include icolib
	ske_RedrawCompleteWindow();
	//	pcli->pfnClcBroadcast( INTM_INVALIDATE,0,0);
	return 0;
}









void CLUI_cli_LoadCluiGlobalOpts()
{
	BOOL tLayeredFlag=FALSE;
	tLayeredFlag=IsWinVer2000Plus();
	tLayeredFlag&=ModernGetSettingByte(NULL, "ModernData", "EnableLayering", tLayeredFlag);

	if(tLayeredFlag)
	{
		if (ModernGetSettingByte(NULL,"CList","WindowShadow",SETTING_WINDOWSHADOW_DEFAULT)==1)
			ModernWriteSettingByte(NULL,"CList","WindowShadow",2);    
	}
	else
	{
		if (ModernGetSettingByte(NULL,"CList","WindowShadow",SETTING_WINDOWSHADOW_DEFAULT)==2)
			ModernWriteSettingByte(NULL,"CList","WindowShadow",1); 
	}
	corecli.pfnLoadCluiGlobalOpts();
}


int CLUI_TestCursorOnBorders()
{
	HWND hwnd=pcli->hwndContactList;
	HCURSOR hCurs1=NULL;
	RECT r;  
	POINT pt;
	int k=0, t=0, fx,fy;
	HWND hAux;
	BOOL mouse_in_window=0;
	HWND gf=GetForegroundWindow();
	GetCursorPos(&pt);
	hAux = WindowFromPoint(pt);	
	if (CLUI_CheckOwnedByClui(hAux))
	{
		if(g_bTransparentFlag) {
			if (!bTransparentFocus && gf!=hwnd) {
				CLUI_SmoothAlphaTransition(hwnd, ModernGetSettingByte(NULL,"CList","Alpha",SETTING_ALPHA_DEFAULT), 1);
				//g_proc_SetLayeredWindowAttributes(hwnd, RGB(0,0,0), (BYTE)DBGetContactSettingByte(NULL,"CList","Alpha",SETTING_ALPHA_DEFAULT), LWA_ALPHA);
				bTransparentFocus=1;
				CLUI_SafeSetTimer(hwnd, TM_AUTOALPHA,250,NULL);
			}
		}
	}

	mutex_bIgnoreActivation=0;
	GetWindowRect(hwnd,&r);
	/*
	*  Size borders offset (contract)
	*/
	r.top+=ModernGetSettingDword(NULL,"ModernSkin","SizeMarginOffset_Top",SKIN_OFFSET_TOP_DEFAULT);
	r.bottom-=ModernGetSettingDword(NULL,"ModernSkin","SizeMarginOffset_Bottom",SKIN_OFFSET_BOTTOM_DEFAULT);
	r.left+=ModernGetSettingDword(NULL,"ModernSkin","SizeMarginOffset_Left",SKIN_OFFSET_LEFT_DEFAULT);
	r.right-=ModernGetSettingDword(NULL,"ModernSkin","SizeMarginOffset_Right",SKIN_OFFSET_RIGHT_DEFAULT);

	if (r.right<r.left) r.right=r.left;
	if (r.bottom<r.top) r.bottom=r.top;

	/*
	*  End of size borders offset (contract)
	*/

	hAux = WindowFromPoint(pt);
	while(hAux != NULL) 
	{
		if (hAux == hwnd) {mouse_in_window=1; break;}
		hAux = GetParent(hAux);
	}
	fx=GetSystemMetrics(SM_CXFULLSCREEN);
	fy=GetSystemMetrics(SM_CYFULLSCREEN);
	if (g_CluiData.fDocked || g_CluiData.nBehindEdgeState==0)
		//if (g_CluiData.fDocked) || ((pt.x<fx-1) && (pt.y<fy-1) && pt.x>1 && pt.y>1)) // workarounds for behind the edge.
	{
		//ScreenToClient(hwnd,&pt);
		//GetClientRect(hwnd,&r);
		if (pt.y<=r.bottom && pt.y>=r.bottom-SIZING_MARGIN && !g_CluiData.fAutoSize) k=6;
		else if (pt.y>=r.top && pt.y<=r.top+SIZING_MARGIN && !g_CluiData.fAutoSize) k=3;
		if (pt.x<=r.right && pt.x>=r.right-SIZING_MARGIN && g_CluiData.bBehindEdgeSettings!=2) k+=2;
		else if (pt.x>=r.left && pt.x<=r.left+SIZING_MARGIN && g_CluiData.bBehindEdgeSettings!=1) k+=1;
		if (!(pt.x>=r.left && pt.x<=r.right && pt.y>=r.top && pt.y<=r.bottom)) k=0;
		k*=mouse_in_window;
		hCurs1 = LoadCursor(NULL, IDC_ARROW);
		if(g_CluiData.nBehindEdgeState<=0 && (!(ModernGetSettingByte(NULL,"CLUI","LockSize",SETTING_LOCKSIZE_DEFAULT))))
			switch(k)
		{
			case 1: 
			case 2:
				if(!g_CluiData.fDocked||(g_CluiData.fDocked==2 && k==1)||(g_CluiData.fDocked==1 && k==2)){hCurs1 = LoadCursor(NULL, IDC_SIZEWE); break;}
			case 3: if(!g_CluiData.fDocked) {hCurs1 = LoadCursor(NULL, IDC_SIZENS); break;}
			case 4: if(!g_CluiData.fDocked) {hCurs1 = LoadCursor(NULL, IDC_SIZENWSE); break;}
			case 5: if(!g_CluiData.fDocked) {hCurs1 = LoadCursor(NULL, IDC_SIZENESW); break;}
			case 6: if(!g_CluiData.fDocked) {hCurs1 = LoadCursor(NULL, IDC_SIZENS); break;}
			case 7: if(!g_CluiData.fDocked) {hCurs1 = LoadCursor(NULL, IDC_SIZENESW); break;}
			case 8: if(!g_CluiData.fDocked) {hCurs1 = LoadCursor(NULL, IDC_SIZENWSE); break;}
		}
		if (hCurs1) SetCursor(hCurs1);
		return k;
	}

	return 0;
}

int CLUI_SizingOnBorder(POINT pt, int PerformSize)
{
	if (!(ModernGetSettingByte(NULL,"CLUI","LockSize",SETTING_LOCKSIZE_DEFAULT)))
	{
		RECT r;
		HWND hwnd=pcli->hwndContactList;
		int sizeOnBorderFlag=0;
		GetWindowRect(hwnd,&r);
		/*
		*  Size borders offset (contract)
		*/
		r.top+=ModernGetSettingDword(NULL,"ModernSkin","SizeMarginOffset_Top",SKIN_OFFSET_TOP_DEFAULT);
		r.bottom-=ModernGetSettingDword(NULL,"ModernSkin","SizeMarginOffset_Bottom",SKIN_OFFSET_BOTTOM_DEFAULT);
		r.left+=ModernGetSettingDword(NULL,"ModernSkin","SizeMarginOffset_Left",SKIN_OFFSET_LEFT_DEFAULT);
		r.right-=ModernGetSettingDword(NULL,"ModernSkin","SizeMarginOffset_Right",SKIN_OFFSET_RIGHT_DEFAULT);

		if (r.right<r.left) r.right=r.left;
		if (r.bottom<r.top) r.bottom=r.top;

		/*
		*  End of size borders offset (contract)
		*/
		if ( !g_CluiData.fAutoSize )
		{
			if      ( pt.y <= r.bottom && pt.y >= r.bottom - SIZING_MARGIN )    sizeOnBorderFlag  = SCF_BOTTOM;
			else if ( pt.y >= r.top    && pt.y <= r.top + SIZING_MARGIN )       sizeOnBorderFlag  = SCF_TOP;
		}

		if ( pt.x <= r.right && pt.x >= r.right - SIZING_MARGIN )               sizeOnBorderFlag += SCF_RIGHT;
		else if ( pt.x >= r.left && pt.x <= r.left + SIZING_MARGIN )            sizeOnBorderFlag += SCF_LEFT;

		if (!(pt.x>=r.left && pt.x<=r.right && pt.y>=r.top && pt.y<=r.bottom))  sizeOnBorderFlag  = SCF_NONE;

		if (sizeOnBorderFlag && PerformSize) 
		{ 
			ReleaseCapture();
			SendMessage(hwnd, WM_SYSCOMMAND, SC_SIZE + sizeOnBorderFlag,MAKELPARAM(pt.x,pt.y));					
			return sizeOnBorderFlag;
		}
		else return sizeOnBorderFlag;
	}
	return SCF_NONE;
}
int CLUI_SyncSmoothAnimation(WPARAM wParam, LPARAM lParam);
static void CLUI_SmoothAnimationThreadProc(HWND hwnd)
{
	//  return;
	if (!mutex_bAnimationInProgress) 
	{
		g_dwSmoothAnimationThreadID=0;
		return;  /// Should be some locked to avoid painting against contact deletion.
	}
	do
	{
		if (!g_mutex_bLockUpdating)
		{
			if (!MirandaExiting())
				Sync(CLUI_SyncSmoothAnimation, (WPARAM)0, (LPARAM)hwnd );
			SleepEx(20,TRUE);
			if (MirandaExiting()) 
			{
				g_dwSmoothAnimationThreadID=0;
				return;
			}
		}
		else SleepEx(0,TRUE);

	} while (mutex_bAnimationInProgress);
	g_dwSmoothAnimationThreadID=0;
	return;
}

static int CLUI_SmoothAlphaThreadTransition(HWND hwnd)
{
	int step;
	int a;

	step=(g_CluiData.bCurrentAlpha>bAlphaEnd)?-1*ANIMATION_STEP:ANIMATION_STEP;
	a=g_CluiData.bCurrentAlpha+step;
	if ((step>=0 && a>=bAlphaEnd) || (step<=0 && a<=bAlphaEnd))
	{
		mutex_bAnimationInProgress=0;
		g_CluiData.bCurrentAlpha=bAlphaEnd;
		if (g_CluiData.bCurrentAlpha==0)
		{			
			g_CluiData.bCurrentAlpha=1;
			ske_JustUpdateWindowImage();
			mutex_bShowHideCalledFromAnimation=1;             
			CLUI_ShowWindowMod(pcli->hwndContactList,0);
			Sync(CLUIFrames_OnShowHide, hwnd,0);
			mutex_bShowHideCalledFromAnimation=0;
			g_CluiData.bCurrentAlpha=0;
			if (!g_CluiData.fLayered) RedrawWindow(pcli->hwndContactList,NULL,NULL,RDW_ERASE|RDW_FRAME);
			return 0;			
		}
	}
	else   g_CluiData.bCurrentAlpha=a;   
	ske_JustUpdateWindowImage();     
	return 1;
}

int CLUI_SmoothAlphaTransition(HWND hwnd, BYTE GoalAlpha, BOOL wParam)
{ 

	if ((!g_CluiData.fLayered
		&& (!g_CluiData.fSmoothAnimation && !g_bTransparentFlag))||!g_proc_SetLayeredWindowAttributesNew)
	{
		if (GoalAlpha>0 && wParam!=2)
		{
			if (!IsWindowVisible(hwnd))
			{
				mutex_bShowHideCalledFromAnimation=1;
				CLUI_ShowWindowMod(pcli->hwndContactList,SW_RESTORE);
				Sync(CLUIFrames_OnShowHide, hwnd,1);
				mutex_bShowHideCalledFromAnimation=0;
				g_CluiData.bCurrentAlpha=GoalAlpha;
				ske_UpdateWindowImage();

			}
		}
		else if (GoalAlpha==0 && wParam!=2)
		{
			if (IsWindowVisible(hwnd))
			{
				mutex_bShowHideCalledFromAnimation=1;
				CLUI_ShowWindowMod(pcli->hwndContactList,0);
				Sync(CLUIFrames_OnShowHide, hwnd,0);
				g_CluiData.bCurrentAlpha=GoalAlpha;
				mutex_bShowHideCalledFromAnimation=0;

			}
		}
		return 0;
	}
	if (g_CluiData.bCurrentAlpha==GoalAlpha &&0)
	{
		if (mutex_bAnimationInProgress)
		{
			KillTimer(hwnd,TM_SMOTHALPHATRANSITION);
			mutex_bAnimationInProgress=0;
		}
		return 0;
	}
	if (mutex_bShowHideCalledFromAnimation) return 0;
	if (wParam!=2)  //not from timer
	{   
		bAlphaEnd=GoalAlpha;
		if (!mutex_bAnimationInProgress)
		{
			if ((!IsWindowVisible(hwnd)||g_CluiData.bCurrentAlpha==0) && bAlphaEnd>0 )
			{
				mutex_bShowHideCalledFromAnimation=1;            
				CLUI_ShowWindowMod(pcli->hwndContactList,SW_SHOWNA);			 
				Sync(CLUIFrames_OnShowHide, hwnd,SW_SHOW);
				mutex_bShowHideCalledFromAnimation=0;
				g_CluiData.bCurrentAlpha=1;
				ske_UpdateWindowImage();
			}
			if (IsWindowVisible(hwnd) && !g_dwSmoothAnimationThreadID)
			{
				mutex_bAnimationInProgress=1;
				if (g_CluiData.fSmoothAnimation)
					g_dwSmoothAnimationThreadID=(DWORD)mir_forkthread((pThreadFunc)CLUI_SmoothAnimationThreadProc,pcli->hwndContactList);	

			}
		}
	}

	{
		int step;
		int a;
		step=(g_CluiData.bCurrentAlpha>bAlphaEnd)?-1*ANIMATION_STEP:ANIMATION_STEP;
		a=g_CluiData.bCurrentAlpha+step;
		if ((step>=0 && a>=bAlphaEnd) || (step<=0 && a<=bAlphaEnd) || g_CluiData.bCurrentAlpha==bAlphaEnd || !g_CluiData.fSmoothAnimation) //stop animation;
		{
			KillTimer(hwnd,TM_SMOTHALPHATRANSITION);
			mutex_bAnimationInProgress=0;
			if (bAlphaEnd==0) 
			{
				g_CluiData.bCurrentAlpha=1;
				ske_UpdateWindowImage();
				mutex_bShowHideCalledFromAnimation=1;             
				CLUI_ShowWindowMod(pcli->hwndContactList,0);
				Sync(CLUIFrames_OnShowHide, pcli->hwndContactList,0);
				mutex_bShowHideCalledFromAnimation=0;
				g_CluiData.bCurrentAlpha=0;
			}
			else
			{
				g_CluiData.bCurrentAlpha=bAlphaEnd;
				ske_UpdateWindowImage();
			}
		}
		else
		{
			g_CluiData.bCurrentAlpha=a;
			ske_UpdateWindowImage();
		}
	}

	return 0;
}

BOOL CLUI__cliInvalidateRect(HWND hWnd, CONST RECT* lpRect,BOOL bErase )
{
	if (g_mutex_bSetAllExtraIconsCycle)
		return FALSE;
	if (CLUI_IsInMainWindow(hWnd) && g_CluiData.fLayered)// && IsWindowVisible(hWnd))
	{
		if (IsWindowVisible(hWnd))
			return SkinInvalidateFrame( hWnd, lpRect );
		else
		{
			g_flag_bFullRepaint=1;
			return 0;
		}
	}
	else 
		return InvalidateRect(hWnd,lpRect,bErase);
	return 1;
}

static BOOL FileExists(TCHAR * tszFilename)
{
	BOOL result=FALSE;
	FILE * f=_tfopen(tszFilename,_T("r"));
	if (f==NULL) return FALSE;
	fclose(f);
	return TRUE;
}

HANDLE RegisterIcolibIconHandle(char * szIcoID, char *szSectionName,  char * szDescription, TCHAR * tszDefaultFile, int iDefaultIndex, HINSTANCE hDefaultModuleInst, int iDefaultResource )
{
	TCHAR fileFull[MAX_PATH]={0};
	SKINICONDESC sid={0};
	HANDLE hIcolibItem=NULL;
	sid.cbSize = sizeof(sid);
	sid.cx = 16;
	sid.cy = 16;
	sid.pszSection = szSectionName;
	sid.pszName = szIcoID;
	sid.flags|=SIDF_PATH_TCHAR;
	sid.pszDescription = szDescription;

	if (tszDefaultFile)
	{
		CallService( MS_UTILS_PATHTOABSOLUTET, ( WPARAM )tszDefaultFile, ( LPARAM )fileFull );
		if (!FileExists(fileFull)) fileFull[0]=_T('\0'); 
	}
	if (fileFull[0]!=_T('\0'))
	{
		sid.ptszDefaultFile = fileFull;
		sid.iDefaultIndex = iDefaultIndex;
		sid.hDefaultIcon = NULL;
	}
	else
	{
		sid.pszDefaultFile = NULL;
		sid.iDefaultIndex = 0;
		sid.hDefaultIcon = LoadSmallIcon( hDefaultModuleInst, MAKEINTRESOURCE(iDefaultResource) );
	}
	hIcolibItem = ( HANDLE )CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);
	if ( sid.hDefaultIcon )	DestroyIcon(sid.hDefaultIcon);
	return hIcolibItem; 
}

// MAIN WINPROC MESSAGE HANDLERS
LRESULT CLUI::PreProcessWndProc( UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	// proxy CLUI Messages
	LRESULT result = 0;
	if ( ProcessCLUIFrameInternalMsg( m_hWnd, msg, wParam, lParam, result ) )
	{
		bHandled = TRUE;
		return result;
	}

	/*
	This registers a window message with RegisterWindowMessage() and then waits for such a message,
	if it gets it, it tries to open a file mapping object and then maps it to this process space,
	it expects 256 bytes of data (incl. NULL) it will then write back the profile it is using the DB to fill in the answer.

	The caller is expected to create this mapping object and tell us the ID we need to open ours.	
	*/
	if (g_CluiData.bSTATE==STATE_EXITING && msg!=WM_DESTROY)
	{
		bHandled = TRUE;
		return 0;
	}
	if (msg == uMsgGetProfile && wParam != 0) /* got IPC message */
	{ 
		HANDLE hMap;
		char szName[MAX_PATH];
		int rc = 0;
		mir_snprintf( szName, SIZEOF(szName), "Miranda::%u", wParam ); // caller will tell us the ID of the map
		hMap = OpenFileMappingA( FILE_MAP_ALL_ACCESS,FALSE, szName );
		if ( hMap != NULL ) 
		{
			void *hView = NULL;
			hView  =MapViewOfFile( hMap, FILE_MAP_ALL_ACCESS, 0, 0, MAX_PATH );
			if (hView) 
			{
				char szFilePath[MAX_PATH], szProfile[MAX_PATH];
				CallService( MS_DB_GETPROFILEPATH,MAX_PATH,(LPARAM)&szFilePath );
				CallService( MS_DB_GETPROFILENAME,MAX_PATH,(LPARAM)&szProfile );
				_snprintf( (char*)hView, MAX_PATH, "%s\\%s", szFilePath, szProfile );
				UnmapViewOfFile( hView );
				rc = 1;
			}
			CloseHandle( hMap );
		}
		bHandled = TRUE;
		return rc;
	}
	return FALSE;
}


LRESULT CLUI::OnSizingMoving( UINT msg, WPARAM wParam, LPARAM lParam )
{
	if (g_CluiData.fLayered)
	{
		if (msg==WM_SIZING)
		{
			static int a=0;
			RECT* wp=(RECT*)lParam;
			if (bNeedFixSizingRect && (rcCorrectSizeRect.bottom!=0||rcCorrectSizeRect.top!=0))
			{
				if(wParam!=WMSZ_BOTTOM) wp->bottom=rcCorrectSizeRect.bottom;
				if(wParam!=WMSZ_TOP) wp->top=rcCorrectSizeRect.top;       
			}
			bNeedFixSizingRect=0;
			rcSizingRect=*wp;
			mutex_bDuringSizing=1;
			return 1;
		}

		if (msg==WM_WINDOWPOSCHANGING)
		{


			WINDOWPOS * wp;
			HDWP PosBatch;
			RECT work_rect={0};
			RECT temp_rect={0};
			wp=(WINDOWPOS *)lParam;
			GetWindowRect(m_hWnd,&rcOldWindowRect);

			// Прилипание к краям by ZorG	
			CLUI::SnappingToEdge(wp);

			if ((rcOldWindowRect.bottom-rcOldWindowRect.top!=wp->cy || rcOldWindowRect.right-rcOldWindowRect.left !=wp->cx)&&!(wp->flags&SWP_NOSIZE))
			{
				{         
					if (!(wp->flags&SWP_NOMOVE)) 
					{
						rcNewWindowRect.left=wp->x;
						rcNewWindowRect.top=wp->y;
					}
					else
					{
						rcNewWindowRect.left=rcOldWindowRect.left;
						rcNewWindowRect.top=rcOldWindowRect.top;
					}
					rcNewWindowRect.right=rcNewWindowRect.left+wp->cx;  
					rcNewWindowRect.bottom=rcNewWindowRect.top+wp->cy;
					work_rect=rcNewWindowRect;
				}
				//resize frames (batch)
				{
					PosBatch=BeginDeferWindowPos(1);
					SizeFramesByWindowRect(&work_rect,&PosBatch,0);
				}
				//Check rect after frames resize
				{
					GetWindowRect(m_hWnd,&temp_rect);
				}
				//Here work_rect should be changed to fit possible changes in cln_listsizechange
				if(bNeedFixSizingRect)
				{
					work_rect=rcSizingRect;
					wp->x=work_rect.left;
					wp->y=work_rect.top;
					wp->cx=work_rect.right-work_rect.left;
					wp->cy=work_rect.bottom-work_rect.top;
					wp->flags&=~(SWP_NOMOVE);
				}
				//reposition buttons and new size applying
				{
					ModernSkinButton_ReposButtons( m_hWnd, SBRF_DO_NOT_DRAW, &work_rect );
					ske_PrepeareImageButDontUpdateIt(&work_rect);
					g_CluiData.mutexPreventDockMoving=0;			
					ske_UpdateWindowImageRect(&work_rect);        
					EndDeferWindowPos(PosBatch);
					g_CluiData.mutexPreventDockMoving=1;
				}       
				Sleep(0);               
				mutex_bDuringSizing=0; 
				DefWindowProc(m_hWnd,msg,wParam,lParam);
				return SendMessage(m_hWnd,WM_WINDOWPOSCHANGED,wParam,lParam);
			}
			else
			{
				SetRect(&rcCorrectSizeRect,0,0,0,0);
				// bNeedFixSizingRect=0;
			}
			return DefWindowProc(m_hWnd,msg,wParam,lParam); 
		}	
	}

	else if (msg==WM_WINDOWPOSCHANGING)
	{
		// Snaping if it is not in LayeredMode	
		WINDOWPOS * wp;
		wp=(WINDOWPOS *)lParam;
		CLUI::SnappingToEdge( wp );
		return DefWindowProc(m_hWnd,msg,wParam,lParam); 
	}
	switch (msg)
	{
	case WM_DISPLAYCHANGE:
		SendMessage(pcli->hwndContactTree, WM_SIZE, 0, 0);
		return TRUE;

	case WM_EXITSIZEMOVE:
		{
			int res=DefWindowProc(m_hWnd, msg, wParam, lParam);
			ReleaseCapture();
			TRACE("WM_EXITSIZEMOVE\n");
			SendMessage(m_hWnd, WM_ACTIVATE, (WPARAM)WA_ACTIVE, (LPARAM)m_hWnd);
			return res;
		}

	case WM_SIZING:
		return DefWindowProc(m_hWnd, msg, wParam, lParam);;
	case WM_MOVE:
		{
			RECT rc;
			CallWindowProc(DefWindowProc, m_hWnd, msg, wParam, lParam);
			mutex_bDuringSizing=0;      
			GetWindowRect(m_hWnd, &rc);
			CheckFramesPos(&rc);
			Sync(CLUIFrames_OnMoving,m_hWnd,&rc);
			if(!IsIconic(m_hWnd)) {
				if(!CallService(MS_CLIST_DOCKINGISDOCKED,0,0))
				{ //if g_CluiData.fDocked, dont remember pos (except for width)
					ModernWriteSettingDword(NULL,"CList","Height",(DWORD)(rc.bottom - rc.top));
					ModernWriteSettingDword(NULL,"CList","x",(DWORD)rc.left);
					ModernWriteSettingDword(NULL,"CList","y",(DWORD)rc.top);
				}
				ModernWriteSettingDword(NULL,"CList","Width",(DWORD)(rc.right - rc.left));
			}
			return TRUE;
		}
	case WM_SIZE:    
		{   
			RECT rc;
			if (g_mutex_bSizing) return 0;
			if(wParam!=SIZE_MINIMIZED /*&& IsWindowVisible(m_hWnd)*/) 
			{
				if ( pcli->hwndContactList == NULL )
					return 0;

				if (!g_CluiData.fLayered && !g_CluiData.fDisableSkinEngine)
					ske_ReCreateBackImage(TRUE,NULL);

				GetWindowRect(m_hWnd, &rc);
				CheckFramesPos(&rc);
				ModernSkinButton_ReposButtons( m_hWnd, SBRF_DO_NOT_DRAW, &rc);
				ModernSkinButton_ReposButtons( m_hWnd, SBRF_REDRAW, NULL);
				if (g_CluiData.fLayered)
					CallService(MS_SKINENG_UPTATEFRAMEIMAGE,(WPARAM)m_hWnd,0);

				if (!g_CluiData.fLayered)
				{
					g_mutex_bSizing=1;
					Sync(CLUIFrames_OnClistResize_mod,(WPARAM)m_hWnd,(LPARAM)1);
					CLUIFrames_ApplyNewSizes(2);
					CLUIFrames_ApplyNewSizes(1);
					SendMessage(m_hWnd,CLN_LISTSIZECHANGE,0,0);				  
					g_mutex_bSizing=0;
				}

				//       ske_RedrawCompleteWindow();
				if(!CallService(MS_CLIST_DOCKINGISDOCKED,0,0))
				{ //if g_CluiData.fDocked, dont remember pos (except for width)
					ModernWriteSettingDword(NULL,"CList","Height",(DWORD)(rc.bottom - rc.top));
					ModernWriteSettingDword(NULL,"CList","x",(DWORD)rc.left);
					ModernWriteSettingDword(NULL,"CList","y",(DWORD)rc.top);
				}
				else SetWindowRgn(m_hWnd,NULL,0);
				ModernWriteSettingDword(NULL,"CList","Width",(DWORD)(rc.right - rc.left));	                               

				if (!g_CluiData.fLayered)
				{
					HRGN hRgn1;
					RECT r;
					int v,h;
					int w=10;
					GetWindowRect(m_hWnd,&r);
					h=(r.right-r.left)>(w*2)?w:(r.right-r.left);
					v=(r.bottom-r.top)>(w*2)?w:(r.bottom-r.top);
					h=(h<v)?h:v;
					hRgn1=CreateRoundRectRgn(0,0,(r.right-r.left+1),(r.bottom-r.top+1),h,h);
					if ((ModernGetSettingByte(NULL,"CLC","RoundCorners",SETTING_ROUNDCORNERS_DEFAULT)) && (!CallService(MS_CLIST_DOCKINGISDOCKED,0,0)))
						SetWindowRgn(m_hWnd,hRgn1,FALSE); 
					else
					{
						DeleteObject(hRgn1);
						SetWindowRgn(m_hWnd,NULL,FALSE);
					}
					RedrawWindow(m_hWnd,NULL,NULL,RDW_INVALIDATE|RDW_ERASE|RDW_FRAME|RDW_UPDATENOW|RDW_ALLCHILDREN);   
				} 			
			}
			else {
				if(ModernGetSettingByte(NULL,"CList","Min2Tray",SETTING_MIN2TRAY_DEFAULT)) {
					CLUI_ShowWindowMod(m_hWnd, SW_HIDE);
					ModernWriteSettingByte(NULL,"CList","State",SETTING_STATE_HIDDEN);
				}
				else ModernWriteSettingByte(NULL,"CList","State",SETTING_STATE_MINIMIZED);
				if (MySetProcessWorkingSetSize != NULL) 
					MySetProcessWorkingSetSize(GetCurrentProcess(),-1,-1);
			}

			return TRUE;
		}
	case WM_WINDOWPOSCHANGING:
		{
			WINDOWPOS * wp;
			wp=(WINDOWPOS *)lParam; 
			if (wp->flags&SWP_HIDEWINDOW && mutex_bAnimationInProgress) 
				return 0;               
			if (g_CluiData.fOnDesktop)
				wp->flags|=SWP_NOACTIVATE|SWP_NOZORDER;
			return DefWindowProc(m_hWnd, msg, wParam, lParam);
		}
	}
	return 0;
}

LRESULT CLUI::OnThemeChanged( UINT /*msg*/, WPARAM /*wParam*/, LPARAM /*lParam*/ )
{
	xpt_OnWM_THEMECHANGED();
	return FALSE;
}

LRESULT CLUI::OnDwmCompositionChanged( UINT /*msg*/, WPARAM /*wParam*/, LPARAM /*lParam*/ )
{
	g_CluiData.fAeroGlass = false;
	CLUI_UpdateAeroGlass();
	return FALSE;
}

LRESULT CLUI::OnSyncCall( UINT /*msg*/, WPARAM wParam, LPARAM /*lParam*/ )
{
	return SyncOnWndProcCall( wParam );
}

LRESULT CLUI::OnUpdate( UINT /*msg*/, WPARAM /*wParam*/, LPARAM /*lParam*/ )
{
	if ( g_flag_bPostWasCanceled ) return FALSE;
	return ske_ValidateFrameImageProc( NULL );
}
LRESULT CLUI::OnInitMenu( UINT /*msg*/, WPARAM /*wParam*/, LPARAM /*lParam*/ )
{
	if ( !CLUI::IsMainMenuInited() )
	{
		if ( ServiceExists( MS_CLIST_MENUBUILDMAIN ) ) CallService( MS_CLIST_MENUBUILDMAIN, 0, 0 );
		CLUI::m_fMainMenuInited = TRUE;
	}
	return FALSE;
}
LRESULT CLUI::OnNcPaint( UINT msg, WPARAM wParam, LPARAM lParam )
{
	int lRes = DefWindowProc( m_hWnd, msg, wParam, lParam );
	if ( !g_CluiData.fLayered && ModernGetSettingByte( NULL,"CLUI","ShowMainMenu",SETTING_SHOWMAINMENU_DEFAULT ) )
	{
		HDC hdc = NULL;
		if ( msg == WM_PRINT ) hdc=(HDC)wParam;
		if ( !hdc ) hdc = GetWindowDC( m_hWnd );
		CLUI_DrawMenuBackGround( m_hWnd, hdc, 0, 0 );
		if ( msg != WM_PRINT ) ReleaseDC( m_hWnd, hdc );
	}
	return lRes;
}
LRESULT CLUI::OnEraseBkgnd( UINT /*msg*/, WPARAM /*wParam*/, LPARAM /*lParam*/ )
{
	return TRUE;
}
LRESULT CLUI::OnNcCreate( UINT msg, WPARAM wParam, LPARAM lParam )
{
	( (LPCREATESTRUCT)lParam )->style &= ~(CS_HREDRAW | CS_VREDRAW);
	return DefCluiWndProc( msg, wParam, lParam );
}

LRESULT CLUI::OnPaint( UINT msg, WPARAM wParam, LPARAM lParam )
{
	if (!g_CluiData.fLayered && IsWindowVisible(m_hWnd))
	{
		RECT w={0};
		RECT w2={0};
		PAINTSTRUCT ps={0};
		HDC hdc;
		HBITMAP hbmp,oldbmp;
		HDC paintDC;

		GetClientRect(m_hWnd,&w);
		if (!(w.right>0 && w.bottom>0)) return DefWindowProc(m_hWnd, msg, wParam, lParam); 

		if (!g_CluiData.fDisableSkinEngine)
		{ 
			paintDC = GetDC(m_hWnd);
			w2=w;
			hdc=CreateCompatibleDC(paintDC);
			hbmp=ske_CreateDIB32(w.right,w.bottom);
			oldbmp=(HBITMAP)SelectObject(hdc,hbmp);
			ske_ReCreateBackImage(FALSE,NULL);
			BitBlt(paintDC,w2.left,w2.top,w2.right-w2.left,w2.bottom-w2.top,g_pCachedWindow->hBackDC,w2.left,w2.top,SRCCOPY);
			SelectObject(hdc,oldbmp);
			DeleteObject(hbmp);
			mod_DeleteDC(hdc);
			ReleaseDC(m_hWnd,paintDC);
		}
		else
		{
			HDC hdc=BeginPaint(m_hWnd,&ps);
			ske_BltBackImage(m_hWnd,hdc,&ps.rcPaint);
			ps.fErase=FALSE;
			EndPaint(m_hWnd,&ps); 
		}

		ValidateRect(m_hWnd,NULL);

	}
	if (0&&(ModernGetSettingDword(NULL,"CLUIFrames","GapBetweenFrames",SETTING_GAPFRAMES_DEFAULT) || ModernGetSettingDword(NULL,"CLUIFrames","GapBetweenTitleBar",SETTING_GAPTITLEBAR_DEFAULT)))
	{
		if (IsWindowVisible(m_hWnd))
			if (g_CluiData.fLayered)
				SkinInvalidateFrame(m_hWnd,NULL);
			else 
			{
				RECT w={0};
				RECT w2={0};
				PAINTSTRUCT ps={0};
				GetWindowRect(m_hWnd,&w);
				OffsetRect(&w,-w.left,-w.top);
				BeginPaint(m_hWnd,&ps);
				if ((ps.rcPaint.bottom-ps.rcPaint.top)*(ps.rcPaint.right-ps.rcPaint.left)==0)
					w2=w;
				else
					w2=ps.rcPaint;
				SkinDrawGlyph(ps.hdc,&w,&w2,"Main,ID=Background,Opt=Non-Layered");
				ps.fErase=FALSE;
				EndPaint(m_hWnd,&ps); 
			}
	}
	return DefWindowProc(m_hWnd, msg, wParam, lParam);
}

LRESULT CLUI::OnCreate( UINT msg, WPARAM wParam, LPARAM lParam )
{
	CallService(MS_LANGPACK_TRANSLATEMENU,(WPARAM)GetMenu(m_hWnd),0);
	DrawMenuBar(m_hWnd);       
	CLUIServices_ProtocolStatusChanged(0,0);

	{	MENUITEMINFO mii;
	ZeroMemory(&mii,sizeof(mii));
	mii.cbSize=MENUITEMINFO_V4_SIZE;
	mii.fMask=MIIM_TYPE|MIIM_DATA;
	mii.dwItemData=MENU_MIRANDAMENU;
	mii.fType=MFT_OWNERDRAW;
	mii.dwTypeData=NULL;
	SetMenuItemInfo(GetMenu(m_hWnd),0,TRUE,&mii);

	// mii.fMask=MIIM_TYPE;
	mii.fType=MFT_OWNERDRAW;
	mii.dwItemData=MENU_STATUSMENU;
	SetMenuItemInfo(GetMenu(m_hWnd),1,TRUE,&mii);

	// mii.fMask=MIIM_TYPE;
	mii.fType=MFT_OWNERDRAW;
	mii.dwItemData=MENU_MINIMIZE;
	SetMenuItemInfo(GetMenu(m_hWnd),2,TRUE,&mii);		
	}
	//PostMessage(m_hWnd, M_CREATECLC, 0, 0);
	//pcli->hwndContactList=m_hWnd;
	uMsgGetProfile=RegisterWindowMessage(TEXT("Miranda::GetProfile")); // don't localise
	bTransparentFocus=1;
	return FALSE;
}
LRESULT CLUI::OnSetAllExtraIcons( UINT /*msg*/, WPARAM /*wParam*/, LPARAM /*lParam*/ )
{
	return FALSE;
}
LRESULT CLUI::OnCreateClc( UINT /*msg*/, WPARAM /*wParam*/, LPARAM /*lParam*/ )
{
	CreateCLC();
	if ( ModernGetSettingByte( NULL, "CList", "ShowOnStart", SETTING_SHOWONSTART_DEFAULT ) ) 
		cliShowHide( (WPARAM) m_hWnd, (LPARAM)TRUE );
	PostMessage( pcli->hwndContactTree, CLM_AUTOREBUILD, 0, 0 );

	return FALSE;
}
LRESULT CLUI::OnLButtonDown( UINT msg, WPARAM wParam, LPARAM lParam )
{
	POINT pt;
	pt.x = (short)LOWORD(lParam);
	pt.y = (short)HIWORD(lParam);
	ClientToScreen( m_hWnd, &pt );

	if ( CLUI_SizingOnBorder( pt, 1 ) )
	{
		mutex_bIgnoreActivation = TRUE;
		return FALSE;
	}
	return DefCluiWndProc( msg, wParam, lParam );
}
LRESULT CLUI::OnParentNotify( UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch ( wParam )
	{
	case WM_LBUTTONDOWN:
		{
			POINT pt;

			pt.x = (short)LOWORD(lParam); 
			pt.y = (short)HIWORD(lParam); 
			ClientToScreen( m_hWnd, &pt );
			wParam = 0;
			lParam = 0;

			if ( CLUI_SizingOnBorder( pt,1 ) ) 
			{
				mutex_bIgnoreActivation = TRUE;
				return 0;
			}
		}
	}
	return DefWindowProc(m_hWnd, msg, wParam, lParam);;
}

LRESULT CLUI::OnSetFocus( UINT msg, WPARAM wParam, LPARAM lParam )
{
	if ( hFrameContactTree && ( !CallService( MS_CLIST_FRAMES_GETFRAMEOPTIONS, MAKEWPARAM( FO_FLOATING, hFrameContactTree ), 0 ) ) )
	{
		SetFocus(pcli->hwndContactTree);
	}

	return FALSE;
}

LRESULT CLUI::OnStatusBarUpdateTimer( UINT msg, WPARAM wParam, LPARAM lParam )
{
	int status,i;
	PROTOTICKS *pt=NULL;

	for (i=0;i<64;i++)
	{

		pt=&CycleStartTick[i];

		if (pt->szProto!=NULL&&pt->bTimerCreated==1)
		{
			if (pt->bGlobal)
				status=g_bMultiConnectionMode?ID_STATUS_CONNECTING:0;
			else
				status=CallProtoService(pt->szProto,PS_GETSTATUS,0,0);

			if (!(status>=ID_STATUS_CONNECTING&&status<=ID_STATUS_CONNECTING+MAX_CONNECT_RETRIES))
			{													
				pt->nCycleStartTick=0;
				ImageList_Destroy(pt->himlIconList);
				pt->himlIconList=NULL;
				KillTimer(m_hWnd,TM_STATUSBARUPDATE+pt->nIndex);
				pt->bTimerCreated=0;
			}
		}

	};

	pt=&CycleStartTick[wParam-TM_STATUSBARUPDATE];
	{
		if(IsWindowVisible(pcli->hwndStatus)) pcli->pfnInvalidateRect(pcli->hwndStatus,NULL,0);//InvalidateRectZ(pcli->hwndStatus,NULL,TRUE);
		//if (DBGetContactSettingByte(NULL,"CList","TrayIcon",SETTING_TRAYICON_DEFAULT)!=SETTING_TRAYICON_CYCLE) 
		if (pt->bGlobal) 
			cliTrayIconUpdateBase(g_szConnectingProto);
		else
			cliTrayIconUpdateBase(pt->szProto);

	}
	pcli->pfnInvalidateRect(pcli->hwndStatus,NULL,TRUE);
	return DefCluiWndProc( msg, wParam, lParam );
}

LRESULT CLUI::OnAutoAlphaTimer( UINT msg, WPARAM wParam, LPARAM lParam )
{
	int inwnd;

	if (GetForegroundWindow()==m_hWnd)
	{
		KillTimer(m_hWnd,TM_AUTOALPHA);
		inwnd=1;
	}
	else 
	{
		POINT pt;
		HWND hwndPt;
		pt.x=(short)LOWORD(GetMessagePos());
		pt.y=(short)HIWORD(GetMessagePos());
		hwndPt=WindowFromPoint(pt);

		inwnd=FALSE;
		inwnd = CLUI_CheckOwnedByClui(hwndPt);
		if (! inwnd )
			inwnd = ( GetCapture()==pcli->hwndContactList );

	}
	if (inwnd!=bTransparentFocus)
	{ //change
		HWND hwn=GetCapture();
		hwn=hwn;
		bTransparentFocus=inwnd;
		if(bTransparentFocus) CLUI_SmoothAlphaTransition(m_hWnd, (BYTE)ModernGetSettingByte(NULL,"CList","Alpha",SETTING_ALPHA_DEFAULT), 1);
		else  
		{
			CLUI_SmoothAlphaTransition(m_hWnd, (BYTE)(g_bTransparentFlag?ModernGetSettingByte(NULL,"CList","AutoAlpha",SETTING_AUTOALPHA_DEFAULT):255), 1);
		}
	}
	if(!bTransparentFocus) KillTimer(m_hWnd,TM_AUTOALPHA);
	return TRUE;
}
LRESULT CLUI::OnSmoothAlphaTransitionTimer( UINT msg, WPARAM wParam, LPARAM lParam )
{
	CLUI_SmoothAlphaTransition(m_hWnd, 0, 2);
	return TRUE;
}
LRESULT CLUI::OnDelayedSizingTimer( UINT msg, WPARAM wParam, LPARAM lParam )
{
	if ( mutex_bDelayedSizing && !mutex_bDuringSizing )
	{
		mutex_bDelayedSizing = 0;
		KillTimer( m_hWnd,TM_DELAYEDSIZING );
		pcli->pfnClcBroadcast( INTM_SCROLLBARCHANGED, 0, 0 );
	}
	return TRUE;
}
LRESULT CLUI::OnBringOutTimer( UINT msg, WPARAM wParam, LPARAM lParam )
{
	//hide
	KillTimer(m_hWnd,TM_BRINGINTIMEOUT);
	KillTimer(m_hWnd,TM_BRINGOUTTIMEOUT);
	bShowEventStarted=0;
	POINT pt; GetCursorPos(&pt);
	HWND hAux = WindowFromPoint(pt);
	BOOL mouse_in_window = CLUI_CheckOwnedByClui(hAux);
	if ( !mouse_in_window && GetForegroundWindow() != m_hWnd ) 
		CLUI_HideBehindEdge(); 
	return TRUE;
}
LRESULT CLUI::OnBringInTimer( UINT msg, WPARAM wParam, LPARAM lParam )
{
	//show	
	KillTimer(m_hWnd,TM_BRINGINTIMEOUT);
	bShowEventStarted=0;
	KillTimer(m_hWnd,TM_BRINGOUTTIMEOUT);
	POINT pt; GetCursorPos(&pt);
	HWND hAux = WindowFromPoint(pt);
	BOOL mouse_in_window = FALSE;
	while(hAux != NULL) 
	{
		if (hAux == m_hWnd) { mouse_in_window = TRUE; break;}
		hAux = GetParent(hAux);
	}
	if ( mouse_in_window )
		CLUI_ShowFromBehindEdge();
	return TRUE;
}
LRESULT CLUI::OnUpdateBringTimer( UINT msg, WPARAM wParam, LPARAM lParam )
{
	CLUI_UpdateTimer( 0 );
	return TRUE;
}

LRESULT CLUI::OnTimer( UINT msg, WPARAM wParam, LPARAM lParam )
{
	if ( MirandaExiting() ) return FALSE;

	if ( (int)wParam >= TM_STATUSBARUPDATE && (int)wParam<=TM_STATUSBARUPDATE+64 )
	{
		if ( !pcli->hwndStatus ) return FALSE;
		else return OnStatusBarUpdateTimer( msg, wParam, lParam );
	}

	switch ( wParam )
	{
	case TM_AUTOALPHA:				return OnAutoAlphaTimer( msg, wParam, lParam );
	case TM_SMOTHALPHATRANSITION:	return OnSmoothAlphaTransitionTimer( msg, wParam, lParam );
	case TM_DELAYEDSIZING:			return OnDelayedSizingTimer( msg, wParam, lParam );
	case TM_BRINGOUTTIMEOUT:		return OnBringOutTimer( msg, wParam, lParam );
	case TM_BRINGINTIMEOUT:			return OnBringInTimer( msg, wParam, lParam );
	case TM_UPDATEBRINGTIMER:		return OnUpdateBringTimer( msg, wParam, lParam );
	default:						return DefCluiWndProc( msg, wParam, lParam );
	}
	return TRUE;
}


LRESULT CLUI::OnActivate( UINT msg, WPARAM wParam, LPARAM lParam )
{
	BOOL IsOption=FALSE;
	SetCursor(LoadCursor(NULL, IDC_ARROW));
	SendMessage(pcli->hwndContactTree, WM_ACTIVATE, wParam, lParam);
	if (ModernGetSettingByte(NULL, "ModernData", "HideBehind", SETTING_HIDEBEHIND_DEFAULT))
	{
		if(wParam==WA_INACTIVE && ((HWND)lParam!=m_hWnd) && GetParent((HWND)lParam)!=m_hWnd && !IsOption) 
		{
			if (!g_bCalledFromShowHide) CLUI_UpdateTimer(0);
		}
		else if (!g_bCalledFromShowHide )
		{
			CLUI_ShowFromBehindEdge();
		}
	}

	if (!IsWindowVisible(m_hWnd) || mutex_bShowHideCalledFromAnimation) 
	{
		KillTimer(m_hWnd,TM_AUTOALPHA);
		return 0;
	}
	if(wParam==WA_INACTIVE && ((HWND)lParam!=m_hWnd) && !CLUI_CheckOwnedByClui((HWND)lParam) && !IsOption) 
	{
		if(g_bTransparentFlag)
			if(bTransparentFocus)
				CLUI_SafeSetTimer(m_hWnd, TM_AUTOALPHA,250,NULL);

	}
	else {
		if (!ModernGetSettingByte(NULL,"CList","OnTop",SETTING_ONTOP_DEFAULT))
			Sync(CLUIFrames_ActivateSubContainers,TRUE);
		if(g_bTransparentFlag) {
			KillTimer(m_hWnd,TM_AUTOALPHA);
			CLUI_SmoothAlphaTransition(m_hWnd, ModernGetSettingByte(NULL,"CList","Alpha",SETTING_ALPHA_DEFAULT), 1);
			bTransparentFocus=1;
		}
	}
	RedrawWindow(m_hWnd,NULL,NULL,RDW_INVALIDATE|RDW_ALLCHILDREN);
	if(g_bTransparentFlag)
	{
		BYTE alpha;
		if (wParam!=WA_INACTIVE || CLUI_CheckOwnedByClui((HWND)lParam)|| IsOption || ((HWND)lParam==m_hWnd) || GetParent((HWND)lParam)==m_hWnd) alpha=ModernGetSettingByte(NULL,"CList","Alpha",SETTING_ALPHA_DEFAULT);
		else 
			alpha=g_bTransparentFlag?ModernGetSettingByte(NULL,"CList","AutoAlpha",SETTING_AUTOALPHA_DEFAULT):255;
		CLUI_SmoothAlphaTransition(m_hWnd, alpha, 1);
		if(IsOption) DefWindowProc(m_hWnd,msg,wParam,lParam);
		else   return 1; 	
	}
	return  DefWindowProc(m_hWnd,msg,wParam,lParam);
}

LRESULT CLUI::OnSetCursor( UINT msg, WPARAM wParam, LPARAM lParam )
{
	HWND gf=GetForegroundWindow();
	if (g_CluiData.nBehindEdgeState>=0)  CLUI_UpdateTimer(1);
	if(g_bTransparentFlag) {
		if (!bTransparentFocus && gf!=m_hWnd)
		{
			CLUI_SmoothAlphaTransition(m_hWnd, ModernGetSettingByte(NULL,"CList","Alpha",SETTING_ALPHA_DEFAULT), 1);
			bTransparentFocus=1;
			CLUI_SafeSetTimer(m_hWnd, TM_AUTOALPHA,250,NULL);
		}
	} 
	int k = CLUI_TestCursorOnBorders();               
	return k ? k : 1;
}

LRESULT CLUI::OnMouseActivate( UINT msg, WPARAM wParam, LPARAM lParam )
{

	if ( mutex_bIgnoreActivation ) 
	{
		mutex_bIgnoreActivation = 0;
		return( MA_NOACTIVATEANDEAT );
	}
	int lRes=DefWindowProc(m_hWnd,msg,wParam,lParam);
	CLUIFrames_RepaintSubContainers();
	return lRes;
}

LRESULT CLUI::OnNcLButtonDown( UINT msg, WPARAM wParam, LPARAM lParam )
{
	POINT pt;
	pt.x = (short)LOWORD(lParam); 
	pt.y = (short)HIWORD(lParam); 
	int k = CLUI_SizingOnBorder(pt,1);
	return k ? k : DefWindowProc( m_hWnd, msg, wParam, lParam );
}

LRESULT CLUI::OnNcLButtonDblClk( UINT msg, WPARAM wParam, LPARAM lParam )
{
	if (wParam == HTMENU || wParam == HTCAPTION)
	{
		RECT rc;
		GetWindowRect(m_hWnd, &rc);
		POINT pt;
		pt.x = (short)LOWORD(lParam); 
		pt.y = (short)HIWORD(lParam); 
		if ( pt.x > rc.right - 16 && pt.x < rc.right )
			return CallService(MS_CLIST_SHOWHIDE, 0, 0);
	}
	return DefCluiWndProc( msg, wParam, lParam );
}

LRESULT CLUI::OnNcHitTest( UINT msg, WPARAM wParam, LPARAM lParam )
{
	LRESULT result;
	result=DefWindowProc(m_hWnd,WM_NCHITTEST,wParam,lParam);

	if( (g_CluiData.fAutoSize) && ( result == HTSIZE || result == HTTOP || 
		result == HTTOPLEFT || result == HTTOPRIGHT ||
		result==HTBOTTOM || result==HTBOTTOMRIGHT || 
		result==HTBOTTOMLEFT) )
		return HTCLIENT;

	if (result==HTMENU) 
	{
		int t;
		POINT pt;
		pt.x=(short)LOWORD(lParam);
		pt.y=(short)HIWORD(lParam);
		t=MenuItemFromPoint(m_hWnd,g_hMenuMain,pt);

		if (t==-1 && (ModernGetSettingByte(NULL,"CLUI","ClientAreaDrag",SETTING_CLIENTDRAG_DEFAULT)))
			return HTCAPTION;
	}

	if (result==HTCLIENT)
	{
		POINT pt;
		int k;
		pt.x=(short)LOWORD(lParam);
		pt.y=(short)HIWORD(lParam);			
		k=CLUI_SizingOnBorder(pt,0);
		if (!k && (ModernGetSettingByte(NULL,"CLUI","ClientAreaDrag",SETTING_CLIENTDRAG_DEFAULT)))
			return HTCAPTION;
		else return k+9;
	}
	return result;
}

LRESULT CLUI::OnShowWindow( UINT msg, WPARAM wParam, LPARAM lParam )
{
	BYTE gAlpha;

	if (lParam) return 0;
	if (mutex_bShowHideCalledFromAnimation) return 1; 
	{

		if (!wParam) gAlpha=0;
		else 
			gAlpha=(ModernGetSettingByte(NULL,"CList","Transparent",SETTING_TRANSPARENT_DEFAULT)?ModernGetSettingByte(NULL,"CList","Alpha",SETTING_ALPHA_DEFAULT):255);
		if (wParam) 
		{
			g_CluiData.bCurrentAlpha=0;
			Sync(CLUIFrames_OnShowHide, pcli->hwndContactList,1);
			ske_RedrawCompleteWindow();
		}
		CLUI_SmoothAlphaTransition(m_hWnd, gAlpha, 1);
	}
	return FALSE;
}

LRESULT CLUI::OnSysCommand( UINT msg, WPARAM wParam, LPARAM lParam )
{
		switch (wParam)
		{
		case SC_MAXIMIZE:
			return 0;

		case SC_CLOSE:
			PostMessage(m_hWnd, msg, SC_MINIMIZE, lParam);
			return 0;
		}

	DefWindowProc(m_hWnd, msg, wParam, lParam);
	if (ModernGetSettingByte(NULL,"CList","OnDesktop",SETTING_ONDESKTOP_DEFAULT))
		Sync( CLUIFrames_ActivateSubContainers, TRUE );
	return FALSE;
}

LRESULT CLUI::OnKeyDown( UINT msg, WPARAM wParam, LPARAM lParam )
{
	if (wParam==VK_F5)
		SendMessage(pcli->hwndContactTree,CLM_AUTOREBUILD,0,0);
	return DefCluiWndProc( msg, wParam, lParam );
}

LRESULT CLUI::OnGetMinMaxInfo( UINT msg, WPARAM wParam, LPARAM lParam )
{
	DefWindowProc(m_hWnd,msg,wParam,lParam);
	((LPMINMAXINFO)lParam)->ptMinTrackSize.x=max(ModernGetSettingWord(NULL,"CLUI","MinWidth",SETTING_MINWIDTH_DEFAULT),max(18,ModernGetSettingByte(NULL,"CLUI","LeftClientMargin",SETTING_LEFTCLIENTMARIGN_DEFAULT)+ModernGetSettingByte(NULL,"CLUI","RightClientMargin",SETTING_RIGHTCLIENTMARIGN_DEFAULT)+18));
	if (nRequiredHeight==0)
	{
		((LPMINMAXINFO)lParam)->ptMinTrackSize.y=CLUIFramesGetMinHeight();
	}
	return FALSE;
}

LRESULT CLUI::OnMoving( UINT msg, WPARAM wParam, LPARAM lParam )
{
	CallWindowProc( DefWindowProc, m_hWnd, msg, wParam, lParam );
	if ( FALSE )  //showcontents is turned on
	{
		Sync(CLUIFrames_OnMoving,m_hWnd,(RECT*)lParam);
	}
	return TRUE;
}

LRESULT CLUI::OnNewContactNotify( NMCLISTCONTROL * pnmc )
{
	ExtraImage_SetAllExtraIcons( pcli->hwndContactTree, pnmc->hItem );
	return FALSE;
}

LRESULT CLUI::OnListRebuildNotify( NMCLISTCONTROL * pnmc )
{
	ExtraImage_SetAllExtraIcons( pcli->hwndContactTree, 0 );
	return FALSE;
}
LRESULT CLUI::OnListSizeChangeNotify( NMCLISTCONTROL * pnmc )
{

	// TODO: Check and refactor possible problem of clist resized to full screen problem
	static RECT rcWindow,rcTree,rcTree2,rcWorkArea,rcOld;
	int maxHeight, minHeight,newHeight;
	int winstyle;
	if (mutex_bDisableAutoUpdate==1)
		return FALSE;
	if (mutex_bDuringSizing)
		rcWindow=rcSizingRect;
	else					
		GetWindowRect(m_hWnd,&rcWindow);
	if ( !g_CluiData.fAutoSize || pcli->hwndContactTree==0 || CallService(MS_CLIST_DOCKINGISDOCKED,0,0) )
		return FALSE;

	maxHeight=ModernGetSettingByte(NULL,"CLUI","MaxSizeHeight",SETTING_MAXSIZEHEIGHT_DEFAULT);
	minHeight=ModernGetSettingByte(NULL,"CLUI","MinSizeHeight",SETTING_MINSIZEHEIGHT_DEFAULT);
	rcOld=rcWindow;
	GetWindowRect(pcli->hwndContactTree,&rcTree);

	FRAMEWND* frm=FindFrameByItsHWND(pcli->hwndContactTree);
	if (frm) 
		rcTree2=frm->wndSize;
	else
		SetRect(&rcTree2,0,0,0,0);

	winstyle=GetWindowLong(pcli->hwndContactTree,GWL_STYLE);

	SystemParametersInfo(SPI_GETWORKAREA,0,&rcWorkArea,FALSE);
	if (MyMonitorFromWindow)
	{
 		HMONITOR hMon = MyMonitorFromWindow(pcli->hwndContactTree, MONITOR_DEFAULTTONEAREST);
		MONITORINFO mi;
		mi.cbSize = sizeof(mi);
		if (MyGetMonitorInfo(hMon, &mi))
 			rcWorkArea = mi.rcWork;
	}


	if (pnmc->pt.y>(rcWorkArea.bottom-rcWorkArea.top)) 
	{
		pnmc->pt.y=(rcWorkArea.bottom-rcWorkArea.top);

	};
	if ((pnmc->pt.y)==nLastRequiredHeight)
	{
	}
	nLastRequiredHeight=pnmc->pt.y;
	newHeight=max(CLUIFramesGetMinHeight(),max(pnmc->pt.y,3)+1+((winstyle&WS_BORDER)?2:0)+(rcWindow.bottom-rcWindow.top)-(rcTree.bottom-rcTree.top));
	if(newHeight<(rcWorkArea.bottom-rcWorkArea.top)*minHeight/100)
		newHeight=(rcWorkArea.bottom-rcWorkArea.top)*minHeight/100;

	if(newHeight>(rcWorkArea.bottom-rcWorkArea.top)*maxHeight/100)
		newHeight=(rcWorkArea.bottom-rcWorkArea.top)*maxHeight/100;

	if (newHeight==(rcWindow.bottom-rcWindow.top)) return 0;

	if(ModernGetSettingByte(NULL,"CLUI","AutoSizeUpward",SETTING_AUTOSIZEUPWARD_DEFAULT)) {
		rcWindow.top=rcWindow.bottom-newHeight;
		if(rcWindow.top<rcWorkArea.top) rcWindow.top=rcWorkArea.top;
	}
	else {
		rcWindow.bottom=rcWindow.top+newHeight;
		if(rcWindow.bottom>rcWorkArea.bottom) rcWindow.bottom=rcWorkArea.bottom;
	}
	if (nRequiredHeight==1)
		return FALSE;
	nRequiredHeight=1;
	if (mutex_bDuringSizing)
	{
		bNeedFixSizingRect=1;           
		rcSizingRect.top=rcWindow.top;
		rcSizingRect.bottom=rcWindow.bottom;
		rcCorrectSizeRect=rcSizingRect;						
	}
	else
	{
		bNeedFixSizingRect=0;
	}
	if (!mutex_bDuringSizing)
		SetWindowPos(m_hWnd,0,rcWindow.left,rcWindow.top,rcWindow.right-rcWindow.left,rcWindow.bottom-rcWindow.top,SWP_NOZORDER|SWP_NOACTIVATE);
	else
	{
		SetWindowPos(m_hWnd,0,rcWindow.left,rcWindow.top,rcWindow.right-rcWindow.left,rcWindow.bottom-rcWindow.top,SWP_NOZORDER|SWP_NOACTIVATE);
	}
	nRequiredHeight=0;

	return FALSE;
}

LRESULT CLUI::OnClickNotify( NMCLISTCONTROL * pnmc )
{
	DWORD hitFlags;
	HANDLE hItem = (HANDLE)SendMessage(pcli->hwndContactTree,CLM_HITTEST,(WPARAM)&hitFlags,MAKELPARAM(pnmc->pt.x,pnmc->pt.y));

	if (hitFlags&CLCHT_ONITEMEXTRA)
	{					
		if (!IsHContactGroup(hItem)&&!IsHContactInfo(hItem))
		{
			pdisplayNameCacheEntry pdnce=(pdisplayNameCacheEntry)pcli->pfnGetCacheEntry(pnmc->hItem);
			if (pdnce==NULL) return 0;

			int extra = ExtraImage_ColumnNumToExtraID(pnmc->iColumn);
			NotifyEventHooks(g_CluiData.hEventExtraClick, (WPARAM)pnmc->hItem, extra);

			if (!ServiceExists("ExtraIcon/Register"))
			{
				int v,e,w;
				v=ExtraImage_ExtraIDToColumnNum(EXTRA_ICON_PROTO);
				e=ExtraImage_ExtraIDToColumnNum(EXTRA_ICON_EMAIL);
				w=ExtraImage_ExtraIDToColumnNum(EXTRA_ICON_WEB);

				if(pnmc->iColumn==v) {
					CallService(MS_USERINFO_SHOWDIALOG,(WPARAM)pnmc->hItem,0);
				};
				if(pnmc->iColumn==e) 
				{
					char *email,buf[4096];
					email= ModernGetStringA(pnmc->hItem,"UserInfo", "Mye-mail0");
					if (!email)
						email= ModernGetStringA(pnmc->hItem, pdnce->m_cache_cszProto, "e-mail");																						
					if (email)
					{
						sprintf(buf,"mailto:%s",email);
						mir_free_and_nill(email);
						ShellExecuteA(m_hWnd,"open",buf,NULL,NULL,SW_SHOW);
					}											
				};	
				if(pnmc->iColumn==w) {
					char *homepage;
					homepage= ModernGetStringA(pdnce->m_cache_hContact,"UserInfo", "Homepage");
					if (!homepage)
						homepage= ModernGetStringA(pdnce->m_cache_hContact,pdnce->m_cache_cszProto, "Homepage");
					if (homepage!=NULL)
					{
						CallService(MS_UTILS_OPENURL, 1, (LPARAM)homepage);
						mir_free_and_nill(homepage);
					}
				}
			}
		}
	};	
	if(hItem && !(hitFlags&CLCHT_NOWHERE))
		return DefCluiWndProc( WM_NOTIFY, 0, (LPARAM)pnmc );
	if((hitFlags&(CLCHT_NOWHERE|CLCHT_INLEFTMARGIN|CLCHT_BELOWITEMS))==0) 
		return DefCluiWndProc( WM_NOTIFY, 0, (LPARAM)pnmc );
	if (ModernGetSettingByte(NULL,"CLUI","ClientAreaDrag",SETTING_CLIENTDRAG_DEFAULT)) {
		POINT pt;
		int res;
		pt=pnmc->pt;
		ClientToScreen(pcli->hwndContactTree,&pt);
		res=PostMessage(m_hWnd, WM_SYSCOMMAND, SC_MOVE|HTCAPTION,MAKELPARAM(pt.x,pt.y));
		return res;
	}
	/*===================*/
	if (ModernGetSettingByte(NULL,"CLUI","DragToScroll",SETTING_DRAGTOSCROLL_DEFAULT) && !ModernGetSettingByte(NULL,"CLUI","ClientAreaDrag",SETTING_CLIENTDRAG_DEFAULT))
		return ClcEnterDragToScroll(pcli->hwndContactTree,pnmc->pt.y);
	/*===================*/
	return 0;
}	

LRESULT CLUI::OnNotify( UINT msg, WPARAM wParam, LPARAM lParam )
{
	LPNMHDR pnmhdr = (LPNMHDR)lParam;
	if ( pnmhdr->hwndFrom != pcli->hwndContactTree ) 
		return DefCluiWndProc( msg, wParam, lParam );

	switch ( pnmhdr->code) 
	{
	case CLN_NEWCONTACT:     return OnNewContactNotify( (NMCLISTCONTROL *)pnmhdr );
	case CLN_LISTREBUILT:    return OnListRebuildNotify( (NMCLISTCONTROL *)pnmhdr );
	case CLN_LISTSIZECHANGE: return OnListSizeChangeNotify( (NMCLISTCONTROL *)pnmhdr );
	case NM_CLICK:           return OnClickNotify( (NMCLISTCONTROL *)pnmhdr );

	} 
	return DefCluiWndProc( msg, wParam, lParam );
}

LRESULT CLUI::OnContextMenu( UINT msg, WPARAM wParam, LPARAM lParam )
{
	RECT rc;
	POINT pt;

	pt.x=(short)LOWORD(lParam);
	pt.y=(short)HIWORD(lParam);
	// x/y might be -1 if it was generated by a kb click			
	GetWindowRect(pcli->hwndContactTree,&rc);
	if ( pt.x == -1 && pt.y == -1) {
		// all this is done in screen-coords!
		GetCursorPos(&pt);
		// the mouse isnt near the window, so put it in the middle of the window
		if (!PtInRect(&rc,pt)) {
			pt.x = rc.left + (rc.right - rc.left) / 2;
			pt.y = rc.top + (rc.bottom - rc.top) / 2; 
		}
	}
	if(PtInRect( &rc ,pt ) )
	{
		HMENU hMenu;
		hMenu=(HMENU)CallService(MS_CLIST_MENUBUILDGROUP,0,0);
		TrackPopupMenu(hMenu,TPM_TOPALIGN|TPM_LEFTALIGN|TPM_LEFTBUTTON,pt.x,pt.y,0,m_hWnd,NULL);
        DestroyTrayMenu(hMenu);
	}
	return FALSE;

}
LRESULT CLUI::OnMeasureItem( UINT msg, WPARAM wParam, LPARAM lParam )
{
	LPMEASUREITEMSTRUCT pmis = (LPMEASUREITEMSTRUCT)lParam;
	switch ( pmis->itemData )
	{
	case MENU_MIRANDAMENU:
		{
			pmis->itemWidth = GetSystemMetrics( SM_CXSMICON ) * 4 / 3;
			pmis->itemHeight = 0;
		}
		return TRUE;
	case MENU_STATUSMENU:
		{
			HDC hdc;
			SIZE textSize;
			hdc=GetDC( m_hWnd );
			GetTextExtentPoint32A( hdc, Translate("Status"), lstrlenA( Translate( "Status" ) ), &textSize );
			pmis->itemWidth = textSize.cx;
			pmis->itemHeight = 0;
			ReleaseDC( m_hWnd, hdc );
		}
		return TRUE;
	}
	return CallService( MS_CLIST_MENUMEASUREITEM, wParam, lParam );
}

LRESULT CLUI::OnDrawItem( UINT msg, WPARAM wParam, LPARAM lParam )
{
	struct ClcData * dat=(struct ClcData*)GetWindowLongPtr(pcli->hwndContactTree,0);
	LPDRAWITEMSTRUCT dis=(LPDRAWITEMSTRUCT)lParam;
	if (!dat) return 0;

	if (dis->CtlType==ODT_MENU) {
		if (dis->itemData==MENU_MIRANDAMENU) {
			if (!g_CluiData.fLayered)
			{	
				char buf[255];	
				short offset = 1 + ( dis->itemState&ODS_SELECTED ? 1 : 0 ) 
								 - ( dis->itemState&ODS_HOTLIGHT ? 1 : 0 );
				
				HICON hIcon=LoadSkinnedIcon(SKINICON_OTHER_MAINMENU);
				
				CLUI_DrawMenuBackGround(m_hWnd, dis->hDC, 1, dis->itemState);
				mir_snprintf(buf,SIZEOF(buf),"Main,ID=MainMenu,Selected=%s,Hot=%s",(dis->itemState&ODS_SELECTED)?"True":"False",(dis->itemState&ODS_HOTLIGHT)?"True":"False");
				SkinDrawGlyph(dis->hDC,&dis->rcItem,&dis->rcItem,buf);

				int x = (dis->rcItem.right  + dis->rcItem.left - GetSystemMetrics(SM_CXSMICON)) / 2 + offset;
				int y = (dis->rcItem.bottom + dis->rcItem.top  - GetSystemMetrics(SM_CYSMICON)) / 2 + offset;

				DrawState( dis->hDC, NULL, NULL, (LPARAM)hIcon, 0, x, y , 0 , 0 , 
					DST_ICON | ( dis->itemState & ODS_INACTIVE  && (((FALSE))) ? DSS_DISABLED : DSS_NORMAL ) );

				CallService(MS_SKIN2_RELEASEICON, (WPARAM)hIcon, 0);
				nMirMenuState=dis->itemState;
			} else {
				nMirMenuState=dis->itemState;
				pcli->pfnInvalidateRect(m_hWnd,NULL,0);
			}
			return TRUE;
		}
		else if(dis->itemData==MENU_STATUSMENU) {
			if (!g_CluiData.fLayered)
			{
				char buf[255]={0};
				RECT rc=dis->rcItem;
				short dx=1+(dis->itemState&ODS_SELECTED?1:0)-(dis->itemState&ODS_HOTLIGHT?1:0);
				if (dx>1){
					rc.left+=dx;
					rc.top+=dx;
				}else if (dx==0){
					rc.right-=1;
					rc.bottom-=1;
				}
				CLUI_DrawMenuBackGround(m_hWnd, dis->hDC, 2, dis->itemState);
				SetBkMode(dis->hDC,TRANSPARENT);
				mir_snprintf(buf,SIZEOF(buf),"Main,ID=StatusMenu,Selected=%s,Hot=%s",(dis->itemState&ODS_SELECTED)?"True":"False",(dis->itemState&ODS_HOTLIGHT)?"True":"False");
				SkinDrawGlyph(dis->hDC,&dis->rcItem,&dis->rcItem,buf);
				SetTextColor(dis->hDC, (dis->itemState&ODS_SELECTED/*|dis->itemState&ODS_HOTLIGHT*/)?dat->MenuTextHiColor:dat->MenuTextColor);
				DrawText(dis->hDC,TranslateT("Status"), lstrlen(TranslateT("Status")),&rc, DT_CENTER|DT_VCENTER|DT_SINGLELINE);
				nStatusMenuState=dis->itemState;
			} else {
				nStatusMenuState=dis->itemState;
				pcli->pfnInvalidateRect(m_hWnd,NULL,0);
			}
			return TRUE;
		} else if(dis->itemData==MENU_MINIMIZE && !g_CluiData.fLayered)
		{
			//TODO check if caption is visible
			char buf[255]={0};	
			short dx=1+(dis->itemState&ODS_SELECTED?1:0)-(dis->itemState&ODS_HOTLIGHT?1:0);
			HICON hIcon = LoadSkinnedIcon(SKINICON_OTHER_MIRANDA);
			CLUI_DrawMenuBackGround(m_hWnd, dis->hDC, 3, dis->itemState);
			mir_snprintf(buf,SIZEOF(buf),"Main,ID=MainMenu,Selected=%s,Hot=%s",(dis->itemState&ODS_SELECTED)?"True":"False",(dis->itemState&ODS_HOTLIGHT)?"True":"False");
			SkinDrawGlyph(dis->hDC,&dis->rcItem,&dis->rcItem,buf);
			DrawState(dis->hDC,NULL,NULL,(LPARAM)hIcon,0,(dis->rcItem.right+dis->rcItem.left-GetSystemMetrics(SM_CXSMICON))/2+dx,(dis->rcItem.bottom+dis->rcItem.top-GetSystemMetrics(SM_CYSMICON))/2+dx,0,0,DST_ICON|(dis->itemState&ODS_INACTIVE&&FALSE?DSS_DISABLED:DSS_NORMAL));
			CallService(MS_SKIN2_RELEASEICON, (WPARAM)hIcon, 0);
			nMirMenuState=dis->itemState;
		}

		return CallService(MS_CLIST_MENUDRAWITEM,wParam,lParam);
	}
	return 0;
}

LRESULT CLUI::OnDestroy( UINT msg, WPARAM wParam, LPARAM lParam )
{
	int state=ModernGetSettingByte(NULL,"CList","State",SETTING_STATE_NORMAL);
	BOOL wait = FALSE;

	AniAva_UnloadModule();
	TRACE("CLUI.c: WM_DESTROY\n");
	g_CluiData.bSTATE=STATE_EXITING;
	CLUI_DisconnectAll();
	//fire the "Away Message" Event to wake the thread so it can die.
	//fire the "Get Text Async" Event to wake the thread so it can die.
	if (amWakeThread())
		wait = TRUE;

	if (gtaWakeThread())
		wait = TRUE;

	if (wait)
	{
		//need to give them a little time to exit.
		Sleep(50);
	}

	TRACE("CLUI.c: WM_DESTROY - WaitThreadsCompletion \n");
	while (CLUI_WaitThreadsCompletion(m_hWnd)); //stop all my threads                
	TRACE("CLUI.c: WM_DESTROY - WaitThreadsCompletion DONE\n");
	{
		int i=0;
		for(i=0; i<64; i++)
			if(CycleStartTick[i].szProto) mir_free_and_nill(CycleStartTick[i].szProto);
	}

    if (state==SETTING_STATE_NORMAL){CLUI_ShowWindowMod(m_hWnd,SW_HIDE);};
	UnLoadContactListModule();
	if(hSettingChangedHook!=0)	ModernUnhookEvent(hSettingChangedHook);
	ClcUnloadModule();

    RemoveMenu(g_hMenuMain, 0, MF_BYPOSITION);
    RemoveMenu(g_hMenuMain, 0, MF_BYPOSITION);
    DestroyMenu(g_hMenuMain);

	pcli->pfnTrayIconDestroy(m_hWnd);	
	mutex_bAnimationInProgress=0;  		
	CallService(MS_CLIST_FRAMES_REMOVEFRAME,(WPARAM)hFrameContactTree,(LPARAM)0);		
	TRACE("CLUI.c: WM_DESTROY - hFrameContactTree removed\n");
	pcli->hwndContactTree=NULL;
	pcli->hwndStatus=NULL;
	{
		if(g_CluiData.fAutoSize && !g_CluiData.fDocked)
		{
			RECT r;
			GetWindowRect(pcli->hwndContactList,&r);
			if(ModernGetSettingByte(NULL,"CLUI","AutoSizeUpward",SETTING_AUTOSIZEUPWARD_DEFAULT))
				r.top=r.bottom-CLUIFrames_GetTotalHeight();
			else 
				r.bottom=r.top+CLUIFrames_GetTotalHeight();
			ModernWriteSettingDword(NULL,"CList","y",r.top);
			ModernWriteSettingDword(NULL,"CList","Height",r.bottom-r.top);
		}
	}
	UnLoadCLUIFramesModule();
	//ExtFrames_Uninit();
	TRACE("CLUI.c: WM_DESTROY - UnLoadCLUIFramesModule DONE\n");
	ModernWriteSettingByte(NULL,"CList","State",(BYTE)state);
	ske_UnloadSkin(&g_SkinObjectList);

	delete m_pCLUI;

	pcli->hwndContactList=NULL;
	pcli->hwndStatus=NULL;
	PostQuitMessage(0);
	return 0;
}
