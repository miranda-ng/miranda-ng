/*

Miranda IM: the free IM client for Microsoft* Windows*


Copyright 2007 Artem Shpynov
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
#include "hdr/modern_commonprototypes.h"
#include "./m_api/m_skinbutton.h"
#include "./m_api/m_toolbar.h"
#include "hdr/modern_sync.h"

//external definition
COLORREF sttGetColor(char * module, char * color, COLORREF defColor);

#define MS_CLUI_SHOWMAINMENU    "CList/ShowMainMenu"
#define MS_CLUI_SHOWSTATUSMENU  "CList/ShowStatusMenu"
#define MS_TTB_TOGGLEHIDEOFFLINE  "CList/ToggleHideOffline"

#define MIRANDATOOLBARCLASSNAME "MirandaToolBar"

#define tbcheck if (!g_CluiData.hEventToolBarModuleLoaded) return
#define tblock EnterCriticalSection(&tbdat.cs)
#define tbunlock LeaveCriticalSection(&tbdat.cs)

enum {
	MTBM_FIRST = WM_USER,
	MTBM_ADDBUTTON,				// will be broad casted thought bars to add button
	MTBM_REPOSBUTTONS,
	MTBM_LAYEREDPAINT,
	MTBM_SETBUTTONSTATE,
	MTBM_SETBUTTONSTATEBYID,
	MTBM_GETBUTTONSTATE,
	MTBM_GETBUTTONSTATEBYID,
	MTBM_REMOVE_ALL_BUTTONS,
	MTBM_UPDATEFRAMEVISIBILITY,
	MTBM_REMOVEBUTTON,
	MTBM_LAST
};

enum {
	SEPARATOR_NOT=0,
	SEPARATOR_FIXED,
	SEPARATOR_FLEX
};

typedef struct _tag_MTB_BUTTONINFO
{
	HWND    hWindow;
	HWND    hwndToolBar;
	char *  szButtonID;						//
	char *  szButtonName;					// The name of button (untranslated)
	char *  szService;						// Service to be executed
	TCHAR * szTooltip;
	TCHAR * szTooltipPressed;
	int	    nOrderValue;					// Order of button
	BYTE    position;
	BOOL    bPushButton;					// 2 icon state button.
	BYTE	bSeparator;						//0 - No, 2- flex, 1 -fixed
	HANDLE	hPrimaryIconHandle;
	HANDLE  hSecondaryIconHandle;
	BOOL	bVisible;
	BYTE	bPanelID;
	LPARAM  lParam;
	void (*ParamDestructor)(void *);		//will be called on parameters deletion
}MTB_BUTTONINFO;

typedef struct _tagMTBInfo
{
	int		cbSize;
	HANDLE	hFrame;
	HWND	hWnd;
	SortedList * pButtonList;
	int		nButtonWidth;
	int     nButtonHeight;
	int		nButtonSpace;
	BOOL	bFlatButtons;
	XPTHANDLE	mtbXPTheme;
	BOOL	fAutoSize;
	int		nLineCount;
	WORD	wLastHeight;
	BOOL	fSingleLine;
}MTBINFO;

typedef struct _tag_MTB_GLOBALDAT
{
	HANDLE hToolBarWindowList;

	//hooks and services handles
	HANDLE hehModulesLoaded;
	HANDLE hehSystemShutdown;
	HANDLE hehSettingsChanged;
	HANDLE hehOptInit;
	HANDLE hsvcToolBarAddButton;
	HANDLE hsvcToolBarRemoveButton;
	HANDLE hsvcToolBarGetButtonState;
	HANDLE hsvcToolBarSetButtonState;

	HBITMAP mtb_hBmpBackground;
	COLORREF mtb_bkColour;
	WORD mtb_backgroundBmpUse;
	BOOL mtb_useWinColors; 
	HANDLE hHideOfflineButton;

	SortedList * listOfButtons;
	CRITICAL_SECTION cs;
} MTB_GLOBALDAT;


static INT_PTR svcToolBarAddButton(WPARAM wParam, LPARAM lParam);
static INT_PTR svcToolBarRemoveButton(WPARAM wParam, LPARAM lParam);
static INT_PTR svcToolBarGetButtonState(WPARAM wParam, LPARAM lParam);
static INT_PTR svcToolBarGetButtonStateById(WPARAM wParam, LPARAM lParam);
static INT_PTR svcToolBarSetButtonState(WPARAM wParam, LPARAM lParam);
static INT_PTR svcToolBarSetButtonStateById(WPARAM wParam, LPARAM lParam);

static int ehhToolbarModulesLoaded(WPARAM wParam, LPARAM lParam);
static int ehhToolBarSystemShutdown(WPARAM wParam, LPARAM lParam);
static int ehhToolBarSettingsChanged( WPARAM wParam, LPARAM lParam );
static int ehhToolBarBackgroundSettingsChanged(WPARAM wParam, LPARAM lParam);
static int ehhToolbarOptInit(WPARAM wParam, LPARAM lParam);

static MTB_BUTTONINFO * ToolBar_AddButtonToBars(MTB_BUTTONINFO * mtbi);
static LRESULT CALLBACK ToolBar_WndProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);
static LRESULT CALLBACK ToolBar_OptDlgProc(HWND hwndDlg,UINT msg,WPARAM wParam,LPARAM lParam);
static int				ToolBar_LayeredPaintProc(HWND hWnd, HDC hDC, RECT * rcPaint, HRGN rgn, DWORD dFlags, void * CallBackData);
static void				ToolBar_DefaultButtonRegistration();


static void sttSetButtonPressed( char * szButton, BOOL bPressed );
static HWND sttCreateToolBarFrame( HWND hwndParent, char * szCaption, int nHeight );
static void sttGetButtonSettings(char * ID, BYTE * pbVisible, DWORD * pdwOrder, BYTE * pbPanelID);
static void sttReloadButtons();
static void sttTBButton2MTBBUTTONINFO(TBButton * bi, MTB_BUTTONINFO * mtbi);
static int  sttSortButtons(const void * vmtbi1, const void * vmtbi2);


static MTB_GLOBALDAT tbdat ={0};

//destructor for MTB_BUTTONINFO
static void   delete_MTB_BUTTONINFO(void * input)
{
	MTB_BUTTONINFO * mtbi=(MTB_BUTTONINFO *)input;
	if (mtbi->hWindow && IsWindow(mtbi->hWindow)) DestroyWindow(mtbi->hWindow);   
	mir_safe_free(mtbi->szButtonID);
	mir_safe_free(mtbi->szButtonName);
	mir_safe_free(mtbi->szService);
	mir_safe_free(mtbi->szTooltip);
	mir_safe_free(mtbi->szTooltipPressed);
	mir_safe_free(mtbi);
}

HRESULT ToolbarLoadModule()
{

	tbdat.hehModulesLoaded=ModernHookEvent(ME_SYSTEM_MODULESLOADED, ehhToolbarModulesLoaded);
	tbdat.hehSystemShutdown=ModernHookEvent(ME_SYSTEM_SHUTDOWN, ehhToolBarSystemShutdown);
	
	{	//create window class
		WNDCLASS wndclass={0};
		if (GetClassInfo(g_hInst,_T(MIRANDATOOLBARCLASSNAME),&wndclass) ==0)
		{
			wndclass.style         = 0;
			wndclass.lpfnWndProc   = ToolBar_WndProc;
			wndclass.cbClsExtra    = 0;
			wndclass.cbWndExtra    = 0;
			wndclass.hInstance     = g_hInst;
			wndclass.hIcon         = NULL;
			wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW);
			wndclass.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
			wndclass.lpszMenuName  = NULL;
			wndclass.lpszClassName = _T(MIRANDATOOLBARCLASSNAME);
			RegisterClass(&wndclass);
		}	  
	}
	tbdat.listOfButtons=li.List_Create(0,1);
	InitializeCriticalSection(&tbdat.cs);		
	return S_OK;
}

static int    ehhToolbarModulesLoaded(WPARAM wParam, LPARAM lParam)
{
	CallService(MS_BACKGROUNDCONFIG_REGISTER,(WPARAM)"ToolBar Background/ToolBar",0);
	ModernHookEvent(ME_BACKGROUNDCONFIG_CHANGED,ehhToolBarBackgroundSettingsChanged);
	tbdat.hehOptInit=ModernHookEvent(ME_OPT_INITIALISE,ehhToolbarOptInit);
	ehhToolBarBackgroundSettingsChanged(0,0);
	tbdat.hehSettingsChanged=ModernHookEvent(ME_DB_CONTACT_SETTINGCHANGED, ehhToolBarSettingsChanged );
	
	tbdat.hToolBarWindowList=(HANDLE) CallService(MS_UTILS_ALLOCWINDOWLIST,0,0);
	
	CreateServiceFunction(MS_TB_ADDBUTTON,svcToolBarAddButton);
	CreateServiceFunction(MS_TB_REMOVEBUTTON, svcToolBarRemoveButton);	
	
	CreateServiceFunction(MS_TB_SETBUTTONSTATE, svcToolBarSetButtonState);	
	CreateServiceFunction(MS_TB_SETBUTTONSTATEBYID, svcToolBarSetButtonStateById);	

	CreateServiceFunction(MS_TB_GETBUTTONSTATE, svcToolBarGetButtonState);	
	CreateServiceFunction(MS_TB_GETBUTTONSTATEBYID, svcToolBarGetButtonStateById);	

	HWND hwndClist=(HWND) CallService(MS_CLUI_GETHWND,0,0);
	sttCreateToolBarFrame( hwndClist, ("ToolBar"), 24);

	NotifyEventHooks(g_CluiData.hEventToolBarModuleLoaded, 0, 0);

	return 0;
}

static int    ehhToolBarSystemShutdown(WPARAM wParam, LPARAM lParam)
{
	//Remove services;
	ModernUnhookEvent(tbdat.hehSettingsChanged);
	ModernUnhookEvent(tbdat.hehModulesLoaded);
	ModernUnhookEvent(tbdat.hehSystemShutdown);	
	ModernUnhookEvent(tbdat.hehOptInit);
	EnterCriticalSection(&tbdat.cs);
	g_CluiData.hEventToolBarModuleLoaded=NULL;

	li_ListDestruct(tbdat.listOfButtons,delete_MTB_BUTTONINFO);

	LeaveCriticalSection(&tbdat.cs);
	DeleteCriticalSection(&tbdat.cs);

	return 0;
}


static int    ehhToolBarSettingsChanged( WPARAM wParam, LPARAM lParam )
{
	DBCONTACTWRITESETTING *cws=(DBCONTACTWRITESETTING*)lParam;
	if ((HANDLE)wParam!=NULL) return 0;
	if (!mir_strcmp(cws->szModule,"CList"))
	{
		if (!mir_strcmp(cws->szSetting,"HideOffline"))
			sttSetButtonPressed("ShowHideOffline", (BOOL) ModernGetSettingByte(NULL, "CList", "HideOffline", SETTING_HIDEOFFLINE_DEFAULT) );
		else if (!mir_strcmp(cws->szSetting,"UseGroups"))
			sttSetButtonPressed( "UseGroups", (BOOL) ModernGetSettingByte(NULL, "CList", "UseGroups", SETTING_USEGROUPS_DEFAULT) );
	}
	else if (!mir_strcmp(cws->szModule,"Skin"))
	{
		if (!mir_strcmp(cws->szSetting,"UseSound"))
			sttSetButtonPressed( "EnableSounds", (BOOL) ModernGetSettingByte(NULL, "Skin", "UseSound", SETTING_ENABLESOUNDS_DEFAULT ) );
	}
	
	return 0;
}
static int    ehhToolBarBackgroundSettingsChanged(WPARAM wParam, LPARAM lParam)
{
	if(tbdat.mtb_hBmpBackground) 
	{
		DeleteObject(tbdat.mtb_hBmpBackground); 
		tbdat.mtb_hBmpBackground=NULL;
	}
	if (g_CluiData.fDisableSkinEngine)
	{
		DBVARIANT dbv;
		tbdat.mtb_bkColour=sttGetColor("ToolBar","BkColour",CLCDEFAULT_BKCOLOUR);
		if(ModernGetSettingByte(NULL,"ToolBar","UseBitmap",CLCDEFAULT_USEBITMAP)) {
			if(!ModernGetSettingString(NULL,"ToolBar","BkBitmap",&dbv)) {
				tbdat.mtb_hBmpBackground=(HBITMAP)CallService(MS_UTILS_LOADBITMAP,0,(LPARAM)dbv.pszVal);
				ModernDBFreeVariant(&dbv);
			}
		}
		tbdat.mtb_useWinColors = ModernGetSettingByte(NULL, "ToolBar", "UseWinColours", CLCDEFAULT_USEWINDOWSCOLOURS);
		tbdat.mtb_backgroundBmpUse = ModernGetSettingWord(NULL, "ToolBar", "BkBmpUse", CLCDEFAULT_BKBMPUSE);
	}	
	PostMessage(pcli->hwndContactList,WM_SIZE,0,0);
	return 0;
}

static int	  ehhToolbarOptInit(WPARAM wParam, LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp;
	ZeroMemory(&odp,sizeof(odp));
	odp.cbSize=sizeof(odp);
	odp.position=0;
	odp.hInstance=g_hInst;
	odp.ptszGroup=LPGENT("Contact List");
	odp.pszTemplate=MAKEINTRESOURCEA(IDD_OPT_TOOLBAR);
	odp.ptszTitle=LPGENT("ToolBar");
	odp.pfnDlgProc=(DLGPROC)ToolBar_OptDlgProc;
	odp.flags=ODPF_BOLDGROUPS|ODPF_TCHAR;
	CallService(MS_OPT_ADDPAGE,wParam,(LPARAM)&odp); 
	return 0;
}
static INT_PTR    svcToolBarAddButton(WPARAM wParam, LPARAM lParam)
{
	INT_PTR result=0;
	BYTE bVisible;
	BYTE bPanel;
	DWORD dwOrder;
	TBButton * bi=(TBButton *)lParam;
	bVisible=(bi->tbbFlags&TBBF_VISIBLE ? TRUE : FALSE);
	if (!ServiceExists(bi->pszServiceName))
		return 0;
	tbcheck 0;
	tblock;
	{	

		MTB_BUTTONINFO * mtbi=(MTB_BUTTONINFO *)mir_calloc(sizeof(MTB_BUTTONINFO));
		sttTBButton2MTBBUTTONINFO(bi,mtbi);
	
		sttGetButtonSettings(mtbi->szButtonID, &bVisible, &dwOrder, &bPanel);
		mtbi->nOrderValue = dwOrder ? dwOrder : bi->defPos;
		mtbi->bVisible = bVisible;
		mtbi->bPanelID = bPanel;
	
		li.List_InsertPtr(tbdat.listOfButtons,mtbi);
		
		if (mtbi->bVisible)
			result=(INT_PTR)ToolBar_AddButtonToBars(mtbi);
	}	
	tbunlock;
	return result;
}
static INT_PTR    svcToolBarRemoveButton(WPARAM wParam, LPARAM lParam)
{
	HANDLE hButton=(HANDLE)wParam;
	tbcheck 0;
	tblock;
	WindowList_Broadcast(tbdat.hToolBarWindowList, MTBM_REMOVEBUTTON, wParam, lParam);
	tbunlock;
	return 0;
}
static INT_PTR    svcToolBarGetButtonState(WPARAM wParam, LPARAM lParam)
{
	int res=-1;
	WindowList_Broadcast(tbdat.hToolBarWindowList, MTBM_GETBUTTONSTATE, wParam, (LPARAM) &res);
	return res;
}
static INT_PTR    svcToolBarGetButtonStateById(WPARAM wParam, LPARAM lParam)
{
	int res=-1;
	WindowList_Broadcast(tbdat.hToolBarWindowList, MTBM_GETBUTTONSTATEBYID, wParam, (LPARAM) &res);
	return res;
}
static INT_PTR    svcToolBarSetButtonState(WPARAM wParam, LPARAM lParam)
{
	WindowList_Broadcast(tbdat.hToolBarWindowList, MTBM_SETBUTTONSTATE, wParam, lParam);
	return 0;
}

static INT_PTR    svcToolBarSetButtonStateById(WPARAM wParam, LPARAM lParam)
{
	WindowList_Broadcast(tbdat.hToolBarWindowList, MTBM_SETBUTTONSTATEBYID, wParam, lParam);
	return 0;
}

static void	  sttTBButton2MTBBUTTONINFO(TBButton * bi, MTB_BUTTONINFO * mtbi)
{
	// Convert TBButton struct to MTB_BUTTONINFO
	if (!bi || !mtbi) return;
	if (!(bi->tbbFlags&TBBF_ISSEPARATOR))
	{
		mtbi->szButtonName=mir_strdup(bi->pszButtonName);
		mtbi->szService=mir_strdup(bi->pszServiceName);
		mtbi->szButtonID=mir_strdup(bi->pszButtonID);
		mtbi->bPushButton=(bi->tbbFlags&TBBF_PUSHED)?TRUE:FALSE;
		mtbi->szTooltip=mir_a2t(Translate(bi->pszTooltipUp));
		mtbi->szTooltipPressed=mir_a2t(Translate(bi->pszTooltipDn));
		mtbi->bSeparator=SEPARATOR_NOT;
		mtbi->hPrimaryIconHandle=bi->hPrimaryIconHandle;
		mtbi->hSecondaryIconHandle=bi->hSecondaryIconHandle;
		mtbi->lParam=bi->lParam;
	}		
	else
	{
		mtbi->nOrderValue=bi->defPos;
		mtbi->bSeparator= (((bi->tbbFlags & TBBF_FLEXSIZESEPARATOR) == TBBF_FLEXSIZESEPARATOR)? 	SEPARATOR_FLEX :
		((bi->tbbFlags & TBBF_ISSEPARATOR) == TBBF_ISSEPARATOR)? SEPARATOR_FIXED : SEPARATOR_NOT);
	}
	mtbi->bVisible = ((bi->tbbFlags&TBBF_VISIBLE)!=0);
}
static void   sttUpdateButtonState(MTB_BUTTONINFO * mtbi)
{
	HANDLE ilIcon;
	ilIcon=(mtbi->bPushButton)?mtbi->hSecondaryIconHandle:mtbi->hPrimaryIconHandle;
	SendMessage(mtbi->hWindow, MBM_SETICOLIBHANDLE, 0, (LPARAM) ilIcon );	
	SendMessage(mtbi->hWindow, BUTTONADDTOOLTIP, (WPARAM)((mtbi->bPushButton) ? mtbi->szTooltipPressed : mtbi->szTooltip), 0);
	
}
static int    sttSortButtons(const void * vmtbi1, const void * vmtbi2)
{
	MTB_BUTTONINFO * mtbi1=(MTB_BUTTONINFO *)*((MTB_BUTTONINFO ** )vmtbi1);
	MTB_BUTTONINFO * mtbi2=(MTB_BUTTONINFO *)*((MTB_BUTTONINFO ** )vmtbi2);
	if (mtbi1==NULL || mtbi2==NULL) return (mtbi1-mtbi2);
	return mtbi1->nOrderValue-mtbi2->nOrderValue;
}

static int   sttReposButtons(MTBINFO * mti)
{
	RECT rcClient;
	HDWP hdwp;
	int  nBarSize;
	int  nFlexSeparatorsCount=0;
	int  nUsedWidth=0;
	int  i=0;
	int  nextX=0;
	int  nFreeSpace;
	BOOL bWasButttonBefore=FALSE;
	int  iFirstButtonId=0;
	int  iLastButtonId=0;
	int	 y=0;
//  firstly let sort it by order
	qsort(mti->pButtonList->items,mti->pButtonList->realCount,sizeof(MTB_BUTTONINFO *),sttSortButtons);

	GetClientRect(mti->hWnd, &rcClient);
    nBarSize=rcClient.right-rcClient.left;
	if (nBarSize == 0) return 0; 
	mti->nLineCount=0;
	hdwp=BeginDeferWindowPos( mti->pButtonList->realCount );
	do
	{	
		++mti->nLineCount;
		bWasButttonBefore=FALSE;
		nUsedWidth=0;
		//calculating count of visible buttons and separators in line
		for (i=iFirstButtonId; i<mti->pButtonList->realCount; i++)
		{
			
			MTB_BUTTONINFO * mtbi=(MTB_BUTTONINFO *)mti->pButtonList->items[i];
			if ( mtbi->bSeparator==2 )
				nFlexSeparatorsCount++;
			else
			{
				int width=(mtbi->bSeparator==1)? 4 : mti->nButtonWidth+(bWasButttonBefore ? mti->nButtonSpace : 0);
				if (nUsedWidth+width>nBarSize) break;
				nUsedWidth+=width;
			}
			iLastButtonId=i+1;
			bWasButttonBefore= (mtbi->bSeparator == 0) ? TRUE : FALSE;			
		}

		nFreeSpace=nBarSize-nUsedWidth;

		for (i=iFirstButtonId; i<iLastButtonId; i++)
		{
			MTB_BUTTONINFO * mtbi=(MTB_BUTTONINFO *)mti->pButtonList->items[i];
			if (mtbi->hWindow)
			{
				if (hdwp)
				{
					//if (nextX+mti->nButtonWidth +mti->nButtonSpace <= nBarSize)
						hdwp=DeferWindowPos(hdwp, mtbi->hWindow, NULL, nextX, y, 0,	0,	SWP_NOSIZE|SWP_NOZORDER|SWP_SHOWWINDOW/*|SWP_NOREDRAW*/);
					//else break;
//						hdwp=DeferWindowPos(hdwp, mtbi->hWindow, NULL, nextX, y, 0,	0,	SWP_NOSIZE|SWP_NOZORDER|SWP_HIDEWINDOW|SWP_NOREDRAW);
				}
				nextX+=mti->nButtonWidth+mti->nButtonSpace;
			}
			else if ( mtbi->bSeparator==1 )
			{
				nextX+=4;
			}
			else if (nFlexSeparatorsCount>0)
			{
				nextX+=nFreeSpace/nFlexSeparatorsCount;
				nFreeSpace-=nFreeSpace/nFlexSeparatorsCount;
				nFlexSeparatorsCount--;
			}
		}
		if ( iFirstButtonId == iLastButtonId ) 
			break;
		iFirstButtonId = iLastButtonId;
		
		y+=mti->nButtonHeight+mti->nButtonSpace;
		nextX=0;
		if (mti->fSingleLine) break;
	} while (iFirstButtonId<mti->pButtonList->realCount && y >= 0 &&(mti->fAutoSize || (y+mti->nButtonHeight <= rcClient.bottom-rcClient.top)));
	for (i=iFirstButtonId; i< mti->pButtonList->realCount; i++)
	{
		MTB_BUTTONINFO * mtbi=(MTB_BUTTONINFO *)mti->pButtonList->items[i];
		if (mtbi->hWindow && hdwp)
		{
				hdwp=DeferWindowPos(hdwp, mtbi->hWindow, NULL, nextX, y, 0,	0,	SWP_NOSIZE|SWP_NOZORDER|SWP_HIDEWINDOW/*|SWP_NOREDRAW*/);
		}
	}
	if (hdwp) EndDeferWindowPos(hdwp);
	return (mti->nButtonHeight+mti->nButtonSpace)*mti->nLineCount-mti->nButtonSpace;	
}


static HWND   sttCreateToolBarFrame( HWND hwndParent, char * szCaption, int nHeight )
{
	TCHAR * Caption=mir_a2t(szCaption);
	HWND hwnd=CreateWindow(_T(MIRANDATOOLBARCLASSNAME), TranslateTS(Caption), WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN,0,0,0,nHeight,hwndParent,NULL,g_hInst, (void*) szCaption);
	mir_free(Caption);
	return hwnd;
}
static int    sttButtonPressed(MTBINFO * pMTBInfo, HWND hwndbutton)
{
	MTB_BUTTONINFO * mtbi=(MTB_BUTTONINFO *)GetWindowLongPtr(hwndbutton, GWLP_USERDATA);
	if (mtbi && mtbi->hWindow==hwndbutton && mtbi->hwndToolBar==pMTBInfo->hWnd)
	{
		if (mtbi->szService && ServiceExists(mtbi->szService))
			return CallService(mtbi->szService, 0, mtbi->lParam);
	}
	return 0;
}
static BOOL   sttDrawToolBarBackground(HWND hwnd, HDC hdc, RECT * rect, MTBINFO * pMTBInfo)
{
	BOOL bFloat = (GetParent(hwnd)!=pcli->hwndContactList);
	if (g_CluiData.fDisableSkinEngine || !g_CluiData.fLayered || bFloat)
	{	
		RECT rc;
		HBRUSH hbr;

		if (rect) 
			rc=*rect;
		else
			GetClientRect(hwnd,&rc);

		if (!(tbdat.mtb_backgroundBmpUse && tbdat.mtb_hBmpBackground) && tbdat.mtb_useWinColors)
		{
			if (xpt_IsThemed(pMTBInfo->mtbXPTheme))
			{
				xpt_DrawTheme(pMTBInfo->mtbXPTheme,pMTBInfo->hWnd, hdc, 0, 0, &rc, &rc);
			}
			else
			{
				hbr=GetSysColorBrush(COLOR_3DFACE);
				FillRect(hdc, &rc, hbr);
			}
		}
		else if (!tbdat.mtb_hBmpBackground && !tbdat.mtb_useWinColors)
		{			
			hbr=CreateSolidBrush(tbdat.mtb_bkColour);
			FillRect(hdc, &rc, hbr);
			DeleteObject(hbr);
		}
		else 
		{
			DrawBackGround(hwnd,hdc,tbdat.mtb_hBmpBackground,tbdat.mtb_bkColour,tbdat.mtb_backgroundBmpUse);
		}
	}
	return TRUE;
}
static void   sttRegisterToolBarButton(char * pszButtonID, char * pszButtonName, char * pszServiceName,
									   char * pszTooltipUp, char * pszTooltipDn, int icoDefIdx, int defResource, int defResource2, BOOL bVisByDefault)
{
	TBButton tbb;
	static int defPos=0;
	defPos+=100;
	memset(&tbb,0, sizeof(TBButton));
	tbb.cbSize=sizeof(TBButton);
	if (pszButtonID) 	
	{
		tbb.defPos=defPos;
		tbb.pszButtonID=pszButtonID;
		tbb.pszButtonName=pszButtonName;
		tbb.pszServiceName=pszServiceName;
		tbb.pszTooltipUp=pszTooltipUp;
		tbb.pszTooltipDn=pszTooltipDn;
		{
			char buf[255];
			mir_snprintf(buf,SIZEOF(buf),"%s%s%s",TOOLBARBUTTON_ICONIDPREFIX, pszButtonID, TOOLBARBUTTON_ICONIDPRIMARYSUFFIX);
			tbb.hPrimaryIconHandle=RegisterIcolibIconHandle( buf, "ToolBar", pszButtonName , _T("icons\\toolbar_icons.dll"),-icoDefIdx, g_hInst, defResource );
		}
		if (pszTooltipDn)
		{
			char buf[255];
			mir_snprintf(buf,SIZEOF(buf),"%s%s%s",TOOLBARBUTTON_ICONIDPREFIX, pszButtonID, TOOLBARBUTTON_ICONIDSECONDARYSUFFIX);
			tbb.hSecondaryIconHandle=RegisterIcolibIconHandle( buf, "ToolBar", pszTooltipDn , _T("icons\\toolbar_icons.dll"),-(icoDefIdx+1), g_hInst, defResource2 );
		}
	}
	else 
	{		
		if ( pszButtonName[0] == 'D' ) tbb.tbbFlags=TBBF_FLEXSIZESEPARATOR;
		else tbb.tbbFlags=TBBF_ISSEPARATOR;
	}
	tbb.tbbFlags|=(bVisByDefault ? TBBF_VISIBLE :0 );
	CallService(MS_TB_ADDBUTTON,0, (LPARAM)&tbb);
}

static void   sttSetButtonPressed( char * hButton, BOOL bPressed )
{
	CallService(MS_TB_SETBUTTONSTATEBYID, (WPARAM) hButton, (LPARAM) (bPressed ? TBST_PUSHED : TBST_RELEASED) );
}
static void   sttAddStaticSeparator( BOOL bVisibleByDefault )
{
	sttRegisterToolBarButton( NULL, "Separator", NULL, NULL, NULL, 0, 0, 0, bVisibleByDefault );
}
static void   sttAddDynamicSeparator( BOOL bVisibleByDefault )
{
	sttRegisterToolBarButton( NULL, "Dynamic separator", NULL, NULL, NULL, 0, 0, 0, bVisibleByDefault );
}
static void   sttGetButtonSettings(char * ID, BYTE * pbVisible, DWORD * pdwOrder, BYTE * pbPanelID)
{
	char key[255]={0};
	BYTE vis=1;

	DWORD ord;
	BYTE panel;

	if (pbVisible) vis=*pbVisible;
	mir_snprintf(key, SIZEOF(key), "visible_%s", ID);
	vis=ModernGetSettingByte(NULL,"ModernToolBar", key, vis);

	mir_snprintf(key, SIZEOF(key), "order_%s", ID);
	ord=ModernGetSettingDword(NULL,"ModernToolBar", key, 0);

	mir_snprintf(key, SIZEOF(key), "panel_%s", ID);
	panel=ModernGetSettingByte(NULL,"ModernToolBar", key, 0);

	if (pbVisible)	*pbVisible=vis;
	if (pdwOrder)	*pdwOrder=ord;
	if (pbPanelID)	*pbPanelID=panel;
}
static void   sttReloadButtons()
{
	int i=0;
	tbcheck ;
	tblock;
	{
		int vis=ModernGetSettingByte(NULL,"CLUI","ShowButtonBar",SETTINGS_SHOWBUTTONBAR_DEFAULT);
		WindowList_Broadcast(tbdat.hToolBarWindowList,MTBM_UPDATEFRAMEVISIBILITY,(WPARAM)ModernGetSettingByte(NULL,"CLUI","ShowButtonBar",SETTINGS_SHOWBUTTONBAR_DEFAULT),0);
	}

	WindowList_Broadcast(tbdat.hToolBarWindowList, MTBM_REMOVE_ALL_BUTTONS, 0,0);
	for (i=0; i<tbdat.listOfButtons->realCount; i++)
	{	
		BYTE bVisible;
		BYTE bPanel;
		DWORD dwOrder;

		MTB_BUTTONINFO * mtbi = (MTB_BUTTONINFO *)tbdat.listOfButtons->items[i];
		
		bVisible=mtbi->bVisible;
		sttGetButtonSettings(mtbi->szButtonID, &bVisible, &dwOrder, &bPanel);
		
		mtbi->nOrderValue = dwOrder ? dwOrder : mtbi->nOrderValue;
		mtbi->bVisible = bVisible;
		mtbi->bPanelID = bPanel;
		
		if (mtbi->bVisible) 
			ToolBar_AddButtonToBars(mtbi);
	}
	tbunlock;
	sttSetButtonPressed( "ShowHideOffline", (BOOL) ModernGetSettingByte(NULL, "CList", "HideOffline", SETTING_HIDEOFFLINE_DEFAULT) );
	sttSetButtonPressed( "UseGroups", (BOOL) ModernGetSettingByte(NULL, "CList", "UseGroups", SETTING_USEGROUPS_DEFAULT) );
	sttSetButtonPressed( "EnableSounds", (BOOL) ModernGetSettingByte(NULL, "Skin", "UseSound", SETTING_ENABLESOUNDS_DEFAULT ) );

}
static int	  sttDBEnumProc (const char *szSetting,LPARAM lParam)
{

	if (szSetting==NULL) return 0;
	if (!strncmp(szSetting,"order_",6))
		ModernDeleteSetting(NULL, "ModernToolBar", szSetting);
	return 0;
};
static void   sttDeleteOrderSettings()
{
	DBCONTACTENUMSETTINGS dbces;
	dbces.pfnEnumProc=sttDBEnumProc;
	dbces.szModule="ToolBar";
	dbces.ofsSettings=0;
	CallService(MS_DB_CONTACT_ENUMSETTINGS,0,(LPARAM)&dbces);
}
static MTB_BUTTONINFO * ToolBar_AddButtonToBars(MTB_BUTTONINFO * mtbi)
{	
	int result=0;
	if (!mtbi->bVisible) return 0;
	WindowList_Broadcast(tbdat.hToolBarWindowList, MTBM_ADDBUTTON, (WPARAM)mtbi, 0);
	if (mtbi->hWindow) 
		sttUpdateButtonState( mtbi );
	if (mtbi->hwndToolBar) 
		SendMessage( mtbi->hwndToolBar, MTBM_REPOSBUTTONS, 0, 0);
	return mtbi;
}

static int				ToolBar_LayeredPaintProc(HWND hWnd, HDC hDC, RECT * rcPaint, HRGN rgn, DWORD dFlags, void * CallBackData)
{
	return SendMessage(hWnd, MTBM_LAYEREDPAINT,(WPARAM)hDC,0);
}



static void	ToolBar_DefaultButtonRegistration()
{
	
	sttRegisterToolBarButton( "MainMenu", "Main Menu", MS_CLUI_SHOWMAINMENU,
		"Main menu", NULL,  100 , IDI_RESETVIEW, IDI_RESETVIEW, TRUE  );

	
	sttRegisterToolBarButton( "StatusMenu", "Status Menu", MS_CLUI_SHOWSTATUSMENU,
		"Status menu", NULL,  105 , IDI_RESETVIEW, IDI_RESETVIEW, TRUE  );

	sttRegisterToolBarButton( "AccoMgr", "Accounts", MS_PROTO_SHOWACCMGR,
		"Accounts...", NULL,  282 , IDI_ACCMGR, IDI_ACCMGR, TRUE  );
	
	sttRegisterToolBarButton( "ShowHideOffline","Show/Hide offline contacts", MS_CLIST_TOGGLEHIDEOFFLINE,
					    "Hide offline contacts", "Show offline contacts", 110 /*and 111 */ , IDI_RESETVIEW, IDI_RESETVIEW, TRUE  );
	
	sttSetButtonPressed( "ShowHideOffline", (BOOL) ModernGetSettingByte(NULL, "CList", "HideOffline", SETTING_HIDEOFFLINE_DEFAULT) );

	sttRegisterToolBarButton( "DatabaseEditor","DBEditor++", "DBEditorpp/MenuCommand",
		"Database Editor", NULL,  130 , IDI_RESETVIEW, IDI_RESETVIEW, TRUE  );

	sttRegisterToolBarButton( "FindUser","Find User", "FindAdd/FindAddCommand",
		"Find User", NULL,  140 , IDI_RESETVIEW, IDI_RESETVIEW, TRUE  );
	
	sttRegisterToolBarButton( "Options","Options", "Options/OptionsCommand",
		"Options", NULL,  150 , IDI_RESETVIEW, IDI_RESETVIEW, TRUE  );

	sttRegisterToolBarButton( "UseGroups","Use/Disable groups", MS_CLIST_TOGGLEGROUPS,
		"Use groups", "Disable Groups", 160 /*and 161 */ , IDI_RESETVIEW, IDI_RESETVIEW, FALSE  );

	sttSetButtonPressed( "UseGroups", (BOOL) ModernGetSettingByte(NULL, "CList", "UseGroups", SETTING_USEGROUPS_DEFAULT) );

	sttRegisterToolBarButton( "EnableSounds","Enable/Disable sounds", MS_CLIST_TOGGLESOUNDS,
		"Enable sounds", "Disable Sounds", 170 /*and 171 */ , IDI_RESETVIEW, IDI_RESETVIEW, FALSE  );
	
	sttSetButtonPressed( "EnableSounds", (BOOL) ModernGetSettingByte(NULL, "Skin", "UseSound", SETTING_ENABLESOUNDS_DEFAULT ) );
	
	sttAddDynamicSeparator(FALSE);

	sttRegisterToolBarButton( "Minimize","Minimize", "CList/ShowHide",
		"Minimize", NULL,  180 , IDI_RESETVIEW, IDI_RESETVIEW, FALSE  );

	sttReloadButtons();
}
static void sttDrawNonLayeredSkinedBar(HWND hwnd, HDC hdc)
{
		HDC hdc2;
		HBITMAP hbmp,hbmpo;
		RECT rc={0};
		GetClientRect(hwnd,&rc);
		rc.right++;
		rc.bottom++;
		hdc2=CreateCompatibleDC(hdc);
		hbmp=ske_CreateDIB32(rc.right,rc.bottom);
		hbmpo=(HBITMAP)SelectObject(hdc2,hbmp);		
		if (GetParent(hwnd)!=pcli->hwndContactList)
		{
			HBRUSH br=GetSysColorBrush(COLOR_3DFACE);
			FillRect(hdc2,&rc,br);
		}
		else
			ske_BltBackImage(hwnd,hdc2,&rc);
		SendMessage(hwnd,MTBM_LAYEREDPAINT, (WPARAM)hdc2, 0);
		BitBlt(hdc,rc.left,rc.top,rc.right-rc.left,rc.bottom-rc.top,
			hdc2,rc.left,rc.top,SRCCOPY);
		SelectObject(hdc2,hbmpo);
		DeleteObject(hbmp);
		mod_DeleteDC(hdc2);

		SelectObject(hdc,(HFONT)GetStockObject(DEFAULT_GUI_FONT));

		ValidateRect(hwnd,NULL);		        							
}
static LRESULT CALLBACK ToolBar_WndProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	static BOOL supressRepos=FALSE;
	MTBINFO * pMTBInfo=(MTBINFO *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	switch (msg) 
	{	
	case WM_CREATE:
		{
			HANDLE hFrame=NULL;
			CLISTFrame Frame={0};
			CREATESTRUCT * lpcs = (CREATESTRUCT *) lParam;
			//create internal info
			MTBINFO * pMTBInfo = (MTBINFO *) mir_calloc( sizeof(MTBINFO) );
			pMTBInfo->cbSize = sizeof(MTBINFO);
			SetWindowLongPtr( hwnd, GWLP_USERDATA, (LONG_PTR) pMTBInfo );

			pMTBInfo->nButtonWidth = ModernGetSettingByte(NULL, "ModernToolBar", "option_Bar0_BtnWidth",  SETTINGS_BARBTNWIDTH_DEFAULT);
			pMTBInfo->nButtonHeight= ModernGetSettingByte(NULL, "ModernToolBar", "option_Bar0_BtnHeight", SETTINGS_BARBTNHEIGHT_DEFAULT);
			pMTBInfo->nButtonSpace = ModernGetSettingByte(NULL, "ModernToolBar", "option_Bar0_BtnSpace",  SETTINGS_BARBTNSPACE_DEFAULT);
			pMTBInfo->fAutoSize    = ModernGetSettingByte(NULL, "ModernToolBar", "option_Bar0_Autosize",  SETTINGS_BARAUTOSIZE_DEFAULT);
			pMTBInfo->fSingleLine  = ModernGetSettingByte(NULL, "ModernToolBar", "option_Bar0_Multiline", SETTINGS_BARMULTILINE_DEFAULT)==0;

			//register self frame
			Frame.cbSize=sizeof(CLISTFrame);
			Frame.hWnd=hwnd;
			Frame.align=alTop;
			Frame.hIcon=LoadSkinnedIcon (SKINICON_OTHER_MIRANDA);
			Frame.Flags=(ModernGetSettingByte(NULL,"CLUI","ShowButtonBar",SETTINGS_SHOWBUTTONBAR_DEFAULT)?F_VISIBLE:0)|F_LOCKED|F_NOBORDER|F_NO_SUBCONTAINER;
	
			Frame.height=ModernGetSettingDword(NULL, "ModernToolBar", "option_Bar0_OldHeight", pMTBInfo->nButtonHeight);
			pMTBInfo->wLastHeight=Frame.height;
          
            pMTBInfo->nLineCount   = 1;
            pMTBInfo->pButtonList=li.List_Create(0,1);

			Frame.name=(char*) lpcs->lpCreateParams;
			hFrame=(HANDLE)CallService(MS_CLIST_FRAMES_ADDFRAME,(WPARAM)&Frame,(LPARAM)0);
			CallService(MS_SKINENG_REGISTERPAINTSUB,(WPARAM)Frame.hWnd,(LPARAM)ToolBar_LayeredPaintProc); //$$$$$ register sub for frame		
			pMTBInfo->hFrame = hFrame;
			pMTBInfo->hWnd = hwnd;



            //add self to window list
			WindowList_Add(tbdat.hToolBarWindowList, hwnd, NULL);
			pMTBInfo->mtbXPTheme=xpt_AddThemeHandle(hwnd,L"TOOLBAR");
			ToolBar_DefaultButtonRegistration();
			//SetTimer(hwnd,123,1000,NULL);
			return 0;
		}
	case WM_TIMER:
		{
			
			//KillTimer(hwnd,123);
			//ToolBar_DefaultButtonRegistration();
			return 0;
		}
	case WM_SHOWWINDOW:
		{
			{
				int res;
				int ID;
				ID= Sync( FindFrameID, hwnd );
				if (ID)
				{
					res=CallService(MS_CLIST_FRAMES_GETFRAMEOPTIONS, MAKEWPARAM(FO_FLAGS,ID),0);
					if (res>=0) ModernWriteSettingByte(0,"CLUI","ShowButtonBar",(BYTE)(wParam/*(res&F_VISIBLE)*/?1:0));
					if (wParam) SendMessage(hwnd, MTBM_REPOSBUTTONS, 0, 0);
				}
			}
			break;
		}
	case MTBM_UPDATEFRAMEVISIBILITY:
		{
			BOOL vis=(BOOL)wParam;
			INT_PTR frameopt; 
			BOOL curvis=IsWindowVisible(hwnd);
			BOOL bResize=FALSE;
			int frameID=Sync( FindFrameID, hwnd );
			int Height;
		
			pMTBInfo->nButtonWidth = ModernGetSettingByte(NULL, "ModernToolBar", "option_Bar0_BtnWidth",  SETTINGS_BARBTNWIDTH_DEFAULT);
			pMTBInfo->nButtonHeight= ModernGetSettingByte(NULL, "ModernToolBar", "option_Bar0_BtnHeight", SETTINGS_BARBTNHEIGHT_DEFAULT);
			pMTBInfo->nButtonSpace = ModernGetSettingByte(NULL, "ModernToolBar", "option_Bar0_BtnSpace",  SETTINGS_BARBTNSPACE_DEFAULT);
			pMTBInfo->fAutoSize    = ModernGetSettingByte(NULL, "ModernToolBar", "option_Bar0_Autosize",  SETTINGS_BARAUTOSIZE_DEFAULT);
			pMTBInfo->fSingleLine  = ModernGetSettingByte(NULL, "ModernToolBar", "option_Bar0_Multiline", SETTINGS_BARMULTILINE_DEFAULT)==0;

			Height=sttReposButtons( pMTBInfo );

			if (pMTBInfo->fAutoSize)
			{
				frameopt=CallService(MS_CLIST_FRAMES_GETFRAMEOPTIONS, MAKEWPARAM(FO_HEIGHT,frameID),0);			
				if (Height!=frameopt)
				{
					frameopt=Height;
					CallService(MS_CLIST_FRAMES_SETFRAMEOPTIONS,MAKEWPARAM(FO_HEIGHT,frameID),frameopt);
					bResize=TRUE;
				}
			}		

			if ((curvis!=vis || bResize) && vis !=-1)
			{				
				frameopt=CallService(MS_CLIST_FRAMES_GETFRAMEOPTIONS, MAKEWPARAM(FO_FLAGS,frameID),0);
				frameopt&=~F_VISIBLE;
				frameopt|=vis ? F_VISIBLE : 0;
				CallService(MS_CLIST_FRAMES_SETFRAMEOPTIONS,MAKEWPARAM(FO_FLAGS,frameID),frameopt);
			}
			break;
		}
	case WM_DESTROY:
		{
			xpt_FreeThemeForWindow(hwnd);
			WindowList_Remove( tbdat.hToolBarWindowList, hwnd );
			SendMessage(hwnd, MTBM_REMOVE_ALL_BUTTONS, 0, 0 );
			li.List_Destroy( pMTBInfo->pButtonList );
			mir_free( pMTBInfo->pButtonList );
			SetWindowLongPtr( hwnd, GWLP_USERDATA, 0 );
			mir_free( pMTBInfo );
			return 0;
		}
	case WM_COMMAND:
		{
			if (HIWORD(wParam)==BN_CLICKED && lParam!=0 )
				  sttButtonPressed(pMTBInfo, (HWND) lParam );
			return TRUE;
		}
	case MTBM_ADDBUTTON:
		{
			//Adding button
			MTB_BUTTONINFO * mtbi=(MTB_BUTTONINFO * )wParam;
			HWND hwndButton = NULL;
			if (!(mtbi->bSeparator))
				hwndButton= CreateWindow(SKINBUTTONCLASS /*MIRANDABUTTONCLASS*/, _T(""), BS_PUSHBUTTON | WS_VISIBLE | WS_CHILD | WS_TABSTOP , 0, 0, pMTBInfo->nButtonWidth, pMTBInfo->nButtonHeight, 
											hwnd, (HMENU) NULL, g_hInst, NULL);
			mtbi->hWindow=hwndButton;
			mtbi->hwndToolBar=hwnd;

			li.List_Insert(pMTBInfo->pButtonList, mtbi, pMTBInfo->pButtonList->realCount);  //just insert pointer. such object are managed in global tbbutton list
			if (hwndButton) 
			{	
				char * buttonId=(char *)_alloca(sizeof("ToolBar.")+strlen(mtbi->szButtonID)+2);
				strcpy(buttonId,"ToolBar.");
				strcat(buttonId,mtbi->szButtonID);					
				SendMessage(hwndButton, BUTTONSETID, 0 ,(LPARAM) buttonId );
				if (pMTBInfo->bFlatButtons)			
					SendMessage(hwndButton, BUTTONSETASFLATBTN, 0, 1 );
				SetWindowLongPtr(hwndButton,GWLP_USERDATA,(LONG_PTR)mtbi);
				SendMessage(hwndButton, MBM_UPDATETRANSPARENTFLAG, 0, 2);
			}
			return (LRESULT)hwndButton;			
		}

	case MTBM_REMOVE_ALL_BUTTONS:
		{
			int i;
			for (i=0; i<pMTBInfo->pButtonList->realCount; i++ )
			{
				MTB_BUTTONINFO * mtbi=(MTB_BUTTONINFO *)pMTBInfo->pButtonList->items[i];
				if (mtbi->hWindow && mtbi->hwndToolBar==hwnd)
				{
					DestroyWindow(mtbi->hWindow);
					mtbi->hWindow = NULL;
					mtbi->hwndToolBar = NULL;
				}
			}
			li.List_Destroy( pMTBInfo->pButtonList );
			mir_free( pMTBInfo->pButtonList );
			pMTBInfo->pButtonList=li.List_Create(0,1);
			break;
		}
	case WM_SIZE: 
		if (pMTBInfo->wLastHeight!=HIWORD(lParam))
		{
			ModernWriteSettingDword(NULL, "ModernToolBar", "option_Bar0_OldHeight", HIWORD(lParam));
			pMTBInfo->wLastHeight=HIWORD(lParam);
		}
		if (supressRepos)
		{
			supressRepos=FALSE;
			break;
		}
		//fall through
	case MTBM_REPOSBUTTONS:
		{
			if (pMTBInfo && pMTBInfo->hWnd == hwnd)
			{
				int Height=sttReposButtons(pMTBInfo);			
				if ( pMTBInfo->fAutoSize) 
				{
					RECT rcClient;
					GetClientRect(pMTBInfo->hWnd, &rcClient);
					if (rcClient.bottom-rcClient.top != Height && Height)
					{
						supressRepos=TRUE;
						PostMessage(pMTBInfo->hWnd,MTBM_UPDATEFRAMEVISIBILITY, -1, 0);
					}
				}

				return 0;
			}			
			break;
		}
	case WM_ERASEBKGND:
		if (g_CluiData.fDisableSkinEngine)
			return sttDrawToolBarBackground(hwnd, (HDC)wParam, NULL, pMTBInfo);
		else
			return 0;
	
	case WM_NCPAINT:				
	case WM_PAINT:
		{
			BOOL ret=FALSE;
			PAINTSTRUCT ps;
			BOOL bFloat = (GetParent(hwnd)!=pcli->hwndContactList);
			if (g_CluiData.fDisableSkinEngine|| !g_CluiData.fLayered || bFloat )
			{
				BeginPaint(hwnd,&ps);
				if ((!g_CluiData.fLayered || bFloat) && !g_CluiData.fDisableSkinEngine)
				{
					sttDrawNonLayeredSkinedBar(hwnd, ps.hdc);
				}
				else
					ret=sttDrawToolBarBackground(hwnd, ps.hdc, &ps.rcPaint, pMTBInfo);	
				EndPaint(hwnd,&ps);
			}
			
			return DefWindowProc(hwnd, msg, wParam, lParam);
		}
	case WM_NOTIFY:
		{
			if (((LPNMHDR) lParam)->code==BUTTONNEEDREDRAW) 
				pcli->pfnInvalidateRect(hwnd, NULL, FALSE);
			return 0;
		}
	case MTBM_LAYEREDPAINT:
		{
			int i;
			RECT MyRect={0};
			HDC hDC=(HDC)wParam;
			GetWindowRect(hwnd,&MyRect);
			{
				RECT rcClient;
				GetClientRect(hwnd, &rcClient);
				SkinDrawGlyph(hDC,&rcClient,&rcClient,"Bar,ID=ToolBar,Part=Background");
			}
			for (i=0; i<pMTBInfo->pButtonList->realCount; i++)
			{
				RECT childRect;
				POINT Offset;
				MTB_BUTTONINFO * mtbi=(MTB_BUTTONINFO *)pMTBInfo->pButtonList->items[i];
				GetWindowRect(mtbi->hWindow,&childRect);
				Offset.x=childRect.left-MyRect.left;;
				Offset.y=childRect.top-MyRect.top;
				SendMessage(mtbi->hWindow, BUTTONDRAWINPARENT, (WPARAM)hDC,(LPARAM)&Offset);
			}	
			return 0;
		}
	case MTBM_SETBUTTONSTATEBYID:
	case MTBM_SETBUTTONSTATE:
		{	
			char * hButtonId=(msg==MTBM_SETBUTTONSTATEBYID) ? (char *) wParam : NULL;
			void * hButton=(msg==MTBM_SETBUTTONSTATE) ? (void *)wParam : NULL;
			MTB_BUTTONINFO *mtbi=NULL;
			int i;
			for (i=0; i<pMTBInfo->pButtonList->realCount; i++)
			{
				mtbi=(MTB_BUTTONINFO*)pMTBInfo->pButtonList->items[i];
				if ((hButtonId && !strcmp(mtbi->szButtonID, hButtonId)) || (hButton == mtbi))
				{
					mtbi->bPushButton=(BOOL)lParam;
				    sttUpdateButtonState(mtbi);
					pcli->pfnInvalidateRect(hwnd, NULL, FALSE);
				    break;
				}
			}
			break;
		}
	case MTBM_GETBUTTONSTATEBYID:
	case MTBM_GETBUTTONSTATE:
		{		
			int * res= (int*)lParam;
			if (res==NULL) break;
			char * hButtonId=(msg==MTBM_GETBUTTONSTATEBYID) ? (char *) wParam : NULL;
			void * hButton=(msg==MTBM_GETBUTTONSTATE) ? (void *)wParam : NULL;
			MTB_BUTTONINFO *mtbi=NULL;
			int i;
			for (i=0; i<pMTBInfo->pButtonList->realCount; i++)
			{
				mtbi=(MTB_BUTTONINFO*)pMTBInfo->pButtonList->items[i];
				if ((hButtonId && !strcmp(mtbi->szButtonID, hButtonId)) || (hButton == mtbi))
				{
					*res=0;
					*res |= mtbi->bPushButton ? TBST_PUSHED : TBST_RELEASED;
					break;
				}
			}	
			break;
		}

	case MTBM_REMOVEBUTTON:
		{
			MTB_BUTTONINFO *mtbi=NULL;
			for (int i=0; i<pMTBInfo->pButtonList->realCount; i++)
			{
				mtbi=(MTB_BUTTONINFO*)pMTBInfo->pButtonList->items[i];
				if (mtbi==(MTB_BUTTONINFO*)wParam)
				{
					li.List_Remove(pMTBInfo->pButtonList,i);
					for (int j=0; j<tbdat.listOfButtons->realCount; j++)
						if (mtbi==(MTB_BUTTONINFO*)tbdat.listOfButtons->items[j])
						{
							li.List_Remove(tbdat.listOfButtons,j);
							break;
						}
					li.List_RemovePtr(tbdat.listOfButtons,mtbi);
					delete_MTB_BUTTONINFO((void*)mtbi);
					mtbi=NULL;
					pcli->pfnInvalidateRect(hwnd, NULL, FALSE);
					break;	
				}
			}
			break;
		}
	default :
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}		
	return TRUE;
}
static int ControlIDS[]={IDC_TEXT_W, IDC_SPIN_W, IDC_STATIC_W, IDC_TEXT_H, 
						 IDC_SPIN_H, IDC_STATIC_H,IDC_TEXT_S, IDC_SPIN_S, 
						 IDC_STATIC_S, IDC_BTNORDER, IDC_CHECK_MULTILINE, IDC_CHECK_AUTOSIZE };

static LRESULT CALLBACK ToolBar_OptDlgProc(HWND hwndDlg,UINT msg,WPARAM wParam,LPARAM lParam)
{
	static HIMAGELIST himlButtonIcons=NULL;
	static BOOL dragging=FALSE;
	static HANDLE hDragItem=NULL;
	switch (msg)
	{
	case WM_DESTROY:
		ImageList_Destroy(himlButtonIcons);
		break;
	case WM_INITDIALOG:
		{
			HWND hTree=GetDlgItem(hwndDlg,IDC_BTNORDER);
			TranslateDialogDefault(hwndDlg);
			SetWindowLong(hTree,GWL_STYLE,GetWindowLong(hTree,GWL_STYLE)|TVS_NOHSCROLL);
			{					
				himlButtonIcons=ImageList_Create(GetSystemMetrics(SM_CXSMICON),GetSystemMetrics(SM_CYSMICON),ILC_COLOR32|ILC_MASK,2,2);
				TreeView_SetImageList(hTree,himlButtonIcons,TVSIL_NORMAL);
			}
			TreeView_DeleteAllItems(hTree);
			tblock;
			
			qsort(tbdat.listOfButtons->items,tbdat.listOfButtons->realCount,sizeof(MTB_BUTTONINFO *),sttSortButtons);
			{				
				int i=0;
				for (i=0; i<tbdat.listOfButtons->realCount; i++)
				{
					TVINSERTSTRUCT tvis={0};
					HTREEITEM hti;
					MTB_BUTTONINFO * mtbi = (MTB_BUTTONINFO*) tbdat.listOfButtons->items[i];
					TCHAR * szTempName=mir_a2t(mtbi->szButtonName);
					HICON hIcon = (HICON)CallService(MS_SKIN2_GETICONBYHANDLE, 0, (LPARAM)mtbi->hPrimaryIconHandle);
					int index = ImageList_AddIcon(himlButtonIcons, hIcon);
					CallService(MS_SKIN2_RELEASEICON, (WPARAM)hIcon, 0);
					tvis.hParent=NULL;
					tvis.hInsertAfter=TVI_LAST;
					tvis.item.mask=TVIF_PARAM|TVIF_TEXT|TVIF_IMAGE|TVIF_SELECTEDIMAGE|TVIF_STATE;	
					tvis.item.lParam=(LPARAM)(mtbi);
					tvis.item.pszText=TranslateTS(szTempName);
					tvis.item.iImage=tvis.item.iSelectedImage=index;
					hti=TreeView_InsertItem(hTree,&tvis);
					TreeView_SetCheckState(hTree, hti, mtbi->bVisible );
					mir_free(szTempName);
				}				
			}
			tbunlock;

			SendDlgItemMessage(hwndDlg,IDC_SPIN_W,UDM_SETRANGE,0,MAKELONG(50,10));
			SendDlgItemMessage(hwndDlg,IDC_SPIN_W,UDM_SETPOS,0,MAKELONG(ModernGetSettingByte(NULL, "ModernToolBar", "option_Bar0_BtnWidth",  SETTINGS_BARBTNWIDTH_DEFAULT),0));

			SendDlgItemMessage(hwndDlg,IDC_SPIN_H,UDM_SETRANGE,0,MAKELONG(50,10));
			SendDlgItemMessage(hwndDlg,IDC_SPIN_H,UDM_SETPOS,0,MAKELONG(ModernGetSettingByte(NULL, "ModernToolBar", "option_Bar0_BtnHeight", SETTINGS_BARBTNHEIGHT_DEFAULT),0));

			SendDlgItemMessage(hwndDlg,IDC_SPIN_S,UDM_SETRANGE,0,MAKELONG(20,0));
			SendDlgItemMessage(hwndDlg,IDC_SPIN_S,UDM_SETPOS,0,MAKELONG(ModernGetSettingByte(NULL, "ModernToolBar", "option_Bar0_BtnSpace",  SETTINGS_BARBTNSPACE_DEFAULT),0));

			CheckDlgButton(hwndDlg, IDC_CHECK_AUTOSIZE, ModernGetSettingByte(NULL, "ModernToolBar", "option_Bar0_Autosize", SETTINGS_BARAUTOSIZE_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_CHECK_MULTILINE, ModernGetSettingByte(NULL,"ModernToolBar", "option_Bar0_Multiline", SETTINGS_BARMULTILINE_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);

			CheckDlgButton(hwndDlg, IDC_TBSHOW, ModernGetSettingByte(NULL,"CLUI","ShowButtonBar",SETTINGS_SHOWBUTTONBAR_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
			{
				int i;
				BOOL en=IsDlgButtonChecked(hwndDlg,IDC_TBSHOW);
				for (i=0; i<SIZEOF(ControlIDS); i++)
					EnableWindow(GetDlgItem(hwndDlg,ControlIDS[i]), en);
			}
			return TRUE;
		}

	case WM_NOTIFY:
		{
			switch(((LPNMHDR)lParam)->idFrom) 
			{		
			case 0: 
				{
					switch (((LPNMHDR)lParam)->code)
					{
					case PSN_APPLY:
						{
							int order=100;
							HWND hTree=GetDlgItem(hwndDlg,IDC_BTNORDER);
							int count=TreeView_GetCount(hTree);
							HTREEITEM hItem;
							sttDeleteOrderSettings();
							hItem=TreeView_GetRoot(hTree);
							do
							{
								TVITEM tvi={0};
								MTB_BUTTONINFO *mtbi;
								tvi.mask=TBIF_LPARAM|TVIF_HANDLE;
								tvi.hItem=hItem;
								TreeView_GetItem(hTree, &tvi);
								mtbi=(MTB_BUTTONINFO *)tvi.lParam;
								if (mtbi)
								{
									char szTempSetting[200];
									mir_snprintf(szTempSetting, SIZEOF(szTempSetting), "order_%s", mtbi->szButtonID);
									ModernWriteSettingDword(NULL, "ModernToolBar", szTempSetting, order);
									order+=10;
									mir_snprintf(szTempSetting, SIZEOF(szTempSetting), "visible_%s", mtbi->szButtonID);
									ModernWriteSettingByte(NULL, "ModernToolBar", szTempSetting, TreeView_GetCheckState(hTree,hItem));
								}
								hItem=TreeView_GetNextSibling(hTree,hItem);
							} while (hItem!=NULL);
							ModernWriteSettingByte(NULL,"CLUI","ShowButtonBar",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_TBSHOW));
							ModernWriteSettingByte(NULL,"ModernToolBar","option_Bar0_BtnWidth", (BYTE)SendDlgItemMessage(hwndDlg,IDC_SPIN_W,UDM_GETPOS,0,0));
							ModernWriteSettingByte(NULL,"ModernToolBar","option_Bar0_BtnHeight",(BYTE)SendDlgItemMessage(hwndDlg,IDC_SPIN_H,UDM_GETPOS,0,0));
							ModernWriteSettingByte(NULL,"ModernToolBar","option_Bar0_BtnSpace", (BYTE)SendDlgItemMessage(hwndDlg,IDC_SPIN_S,UDM_GETPOS,0,0));
							ModernWriteSettingByte(NULL, "ModernToolBar", "option_Bar0_Autosize", (BYTE)IsDlgButtonChecked(hwndDlg,IDC_CHECK_AUTOSIZE));
							ModernWriteSettingByte(NULL,"ModernToolBar", "option_Bar0_Multiline", (BYTE)IsDlgButtonChecked(hwndDlg,IDC_CHECK_MULTILINE));
							
							sttReloadButtons();
							return TRUE;
						}
					}
					break;
				}
			case IDC_BTNORDER:
				{
					switch (((LPNMHDR)lParam)->code) 
					{
					case TVN_BEGINDRAGA:
					case TVN_BEGINDRAGW:
						SetCapture(hwndDlg);
						dragging=TRUE;
						hDragItem=((LPNMTREEVIEWA)lParam)->itemNew.hItem;
						TreeView_SelectItem(GetDlgItem(hwndDlg,IDC_BTNORDER),hDragItem);
						break;
					case NM_CLICK:
						{						
							TVHITTESTINFO hti;
							hti.pt.x=(short)LOWORD(GetMessagePos());
							hti.pt.y=(short)HIWORD(GetMessagePos());
							ScreenToClient(((LPNMHDR)lParam)->hwndFrom,&hti.pt);
							if(TreeView_HitTest(((LPNMHDR)lParam)->hwndFrom,&hti))
								if(hti.flags&TVHT_ONITEMSTATEICON) 
									SendMessage(GetParent(hwndDlg), PSM_CHANGED, (WPARAM)hwndDlg, 0);
						};
					}
					break;
				}
			} 
			break;
		}
	case WM_MOUSEMOVE:
		{
			if(!dragging) break;
			{	
				TVHITTESTINFO hti;
				hti.pt.x=(short)LOWORD(lParam);
				hti.pt.y=(short)HIWORD(lParam);
				ClientToScreen(hwndDlg,&hti.pt);
				ScreenToClient(GetDlgItem(hwndDlg,IDC_BTNORDER),&hti.pt);
				TreeView_HitTest(GetDlgItem(hwndDlg,IDC_BTNORDER),&hti);
				if(hti.flags&(TVHT_ONITEM|TVHT_ONITEMRIGHT))
				{
					HTREEITEM it=hti.hItem;
					hti.pt.y-=TreeView_GetItemHeight(GetDlgItem(hwndDlg,IDC_BTNORDER))/2;
					TreeView_HitTest(GetDlgItem(hwndDlg,IDC_BTNORDER),&hti);
					if (!(hti.flags&TVHT_ABOVE))
						TreeView_SetInsertMark(GetDlgItem(hwndDlg,IDC_BTNORDER),hti.hItem,1);
					else 
						TreeView_SetInsertMark(GetDlgItem(hwndDlg,IDC_BTNORDER),it,0);
				}
				else 
				{
					if(hti.flags&TVHT_ABOVE) SendDlgItemMessage(hwndDlg,IDC_BTNORDER,WM_VSCROLL,MAKEWPARAM(SB_LINEUP,0),0);
					if(hti.flags&TVHT_BELOW) SendDlgItemMessage(hwndDlg,IDC_BTNORDER,WM_VSCROLL,MAKEWPARAM(SB_LINEDOWN,0),0);
					TreeView_SetInsertMark(GetDlgItem(hwndDlg,IDC_BTNORDER),NULL,0);
				}
			}	
		}
		break;
	case WM_LBUTTONUP:
		{
			if(!dragging) break;
			TreeView_SetInsertMark(GetDlgItem(hwndDlg,IDC_BTNORDER),NULL,0);
			dragging=0;
			ReleaseCapture();
			{	
				TVHITTESTINFO hti;
				TVITEM tvi;
				hti.pt.x=(short)LOWORD(lParam);
				hti.pt.y=(short)HIWORD(lParam);
				ClientToScreen(hwndDlg,&hti.pt);
				ScreenToClient(GetDlgItem(hwndDlg,IDC_BTNORDER),&hti.pt);
				hti.pt.y-=TreeView_GetItemHeight(GetDlgItem(hwndDlg,IDC_BTNORDER))/2;
				TreeView_HitTest(GetDlgItem(hwndDlg,IDC_BTNORDER),&hti);
				if(hDragItem==hti.hItem) break;
				if (hti.flags&TVHT_ABOVE) hti.hItem=TVI_FIRST;
				tvi.mask=TVIF_HANDLE|TVIF_PARAM;
				tvi.hItem=(HTREEITEM)hDragItem;
				TreeView_GetItem(GetDlgItem(hwndDlg,IDC_BTNORDER),&tvi);
				if(hti.flags&(TVHT_ONITEM|TVHT_ONITEMRIGHT)||(hti.hItem==TVI_FIRST)) 
				{
					TVINSERTSTRUCT tvis;
					TCHAR name[128];
					tvis.item.mask=TVIF_HANDLE|TVIF_PARAM|TVIF_TEXT|TVIF_IMAGE|TVIF_SELECTEDIMAGE|TVIF_STATE;
					tvis.item.stateMask=0xFFFFFFFF;
					tvis.item.pszText=name;
					tvis.item.cchTextMax=sizeof(name);
					tvis.item.hItem=(HTREEITEM)hDragItem;
					//tvis.item.iImage=tvis.item.iSelectedImage=((MTB_BUTTONINFO *)tvi.lParam)->bVisible;				
					TreeView_GetItem(GetDlgItem(hwndDlg,IDC_BTNORDER),&tvis.item);				
					TreeView_DeleteItem(GetDlgItem(hwndDlg,IDC_BTNORDER),hDragItem);
					tvis.hParent=NULL;
					tvis.hInsertAfter=hti.hItem;
					TreeView_SelectItem(GetDlgItem(hwndDlg,IDC_BTNORDER),TreeView_InsertItem(GetDlgItem(hwndDlg,IDC_BTNORDER),&tvis));
					SendMessage((GetParent(hwndDlg)), PSM_CHANGED, (WPARAM)hwndDlg, 0);
				}
			}
		}
		break; 
	case WM_COMMAND:
		if (LOWORD(wParam)==IDC_TBSHOW) {
			{
				int i;
				BOOL en=IsDlgButtonChecked(hwndDlg,IDC_TBSHOW);
				for (i=0; i<SIZEOF(ControlIDS); i++)
					EnableWindow(GetDlgItem(hwndDlg,ControlIDS[i]), en);
			}
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, (WPARAM)hwndDlg, 0);
		} else if ( (LOWORD(wParam)==IDC_TEXT_W || 
			         LOWORD(wParam)==IDC_TEXT_H ||
					 LOWORD(wParam)==IDC_TEXT_S ) 
					&& HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus()) return 0; // dont make apply enabled during buddy set crap 
		SendMessage(GetParent(hwndDlg), PSM_CHANGED, (WPARAM)hwndDlg, 0);
		break;
	}
	return FALSE;
}
