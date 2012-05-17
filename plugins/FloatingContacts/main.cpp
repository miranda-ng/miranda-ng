/*
Miranda Floating Contacts plugin, created by Iavor Vajarov ( ivajarov@code.bg )
http://miranda-icq.sourceforge.net/

Miranda fonts and colors settings by Ranger.
Extended bonts and backgrounds settings by Oleksiy Shurubura


This file is placed in the public domain. Anybody is free to use or
modify it as they wish with no restriction.

This plugin has been tested to work under Windows ME, 2000 and XP.

No warranty for any misbehaviour.
*/

#include "stdhdr.h"
#include "shlwapi.h"

#include "version.h"

#pragma comment ( lib, "comctl32.lib" )
#pragma comment ( lib, "shlwapi.lib" )

#pragma warning ( default : 4201 )

//#define DB_POS_GETX(pos) LOWORD(pos)
//#define DB_POS_GETY(pos) HIWORD(pos)
//#define DB_POS_MAKE_XY(x, y) MAKELONG(x, y)

BOOL (WINAPI *pSetLayeredWindowAttributes)(HWND,COLORREF,BYTE,DWORD);
BOOL (WINAPI *pUpdateLayeredWindow)
	(HWND hwnd, HDC hdcDST, POINT *pptDst, SIZE *psize, HDC hdcSrc, POINT *pptSrc,
	 COLORREF crKey, BLENDFUNCTION *pblend, DWORD dwFlags);

// Globals

// TODO: move to some more approriate place, probably part of Thumbs manager
static void	LoadContacts			( void );
static void LoadContact				( HANDLE hContact );

// Internal funcs
static void	RepaintWindow			( HWND hwnd, HDC hdc );	
static void	LoadMenus				();
static void	CreateThumbWnd			( TCHAR *ptszName, HANDLE hContact, int nX, int nY );
static void	RegisterWindowClass		( void );
static void	UnregisterWindowClass	( void );
static void	CleanUp					( void );
static BOOL GetOSPlatform			( void );
static void LoadDBSettings			( void );
static void CreateThumbsFont		( void );
static void CreateBackgroundBrush	( void );
static int	GetContactStatus		( HANDLE hContact );
static void	GetScreenRect			( void );
extern void SetThumbsOpacity		( BYTE btAlpha );
static int	ClcStatusToPf2			( int status );

static LRESULT __stdcall CommWndProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
static LRESULT __stdcall newMirandaWndProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

static int OnContactDeleted			( WPARAM wParam,LPARAM lParam );
static int OnContactIconChanged		( WPARAM wParam, LPARAM lParam );
static int OnContactDrag			( WPARAM wParam, LPARAM lParam );
static int OnContactDrop			( WPARAM wParam, LPARAM lParam );
static int OnContactDragStop		( WPARAM wParam, LPARAM lParam );
static int OnSkinIconsChanged		( WPARAM wParam, LPARAM lParam );
static int OnContactSettingChanged	( WPARAM wParam, LPARAM lParam );
static int OnStatusModeChange		( WPARAM wParam, LPARAM lParam );
static int OnModulesLoded			( WPARAM wParam, LPARAM lParam );
static int OnPrebuildContactMenu	( WPARAM wParam, LPARAM lParam );

static int OnContactMenu_Remove		( WPARAM wParam,LPARAM lParam );
//static int OnContactMenu_HideAll	( WPARAM wParam,LPARAM lParam );
static int OnMainMenu_HideAll		( WPARAM wParam,LPARAM lParam );
static int OnHotKey_HideWhenCListShow( WPARAM wParam,LPARAM lParam );
static VOID CALLBACK ToTopTimerProc ( HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);

WNDPROC oldMirandaWndProc;


HINSTANCE	hInst				= NULL;
HMODULE		hUserDll			= NULL;
PLUGINLINK	*pluginLink			= NULL;
LIST_INTERFACE	li				= {0};
HFONT		hFont[FLT_FONTIDS]	= {NULL};
COLORREF	tColor[FLT_FONTIDS]	= {0};
HIMAGELIST	himl				= NULL;
HANDLE		hevContactIcon		= NULL;
HANDLE		hevContactDrop		= NULL;
HANDLE		hevContactDragStop	= NULL;
HANDLE		hevSkinIcons		= NULL;
HANDLE		hevContactDrag		= NULL;
HANDLE		hevContactSetting	= NULL;
HANDLE		hevContactDeleted	= NULL;
HANDLE		hevOptionsInit		= NULL;
HANDLE		hevStatusMode		= NULL;
HANDLE		hevModules			= NULL;
HANDLE		hevPrebuildMenu		= NULL;
HANDLE		hNewContact			= NULL;

HPEN		hLTEdgesPen			= NULL;
HPEN		hRBEdgesPen			= NULL;
HBRUSH		hBkBrush			= NULL;
DWORD		bkColor				= 0;
HBITMAP		hBmpBackground		= NULL;
WORD		nBackgroundBmpUse	= CLB_STRETCH;

HWND		hwndMiranda			= NULL;
BOOL		bVersionOK			= FALSE;
BOOL		bNT					= FALSE;
BOOL		bDockHorz			= TRUE;
//UINT		nStatus				= 0;
HMENU		hContactMenu		= NULL;
HANDLE		hMenuItemRemove		= NULL;
HANDLE		hMenuItemHideAll	= NULL;
HANDLE		hMainMenuItemHideAll	= NULL;
RECT		rcScreen;
DWORD		dwOfflineModes		= 0;
BOOL		bEnableTip			= FALSE;
UINT_PTR	ToTopTimerID		= 0;
BOOL		bIsCListShow		= TRUE;

HANDLE		hRemoveThumb		= NULL;
HANDLE		hMainHideAllThumbs	= NULL;
HANDLE		hHideWhenCListShow	= NULL;

//Options

FCOptions fcOpt = {0};

static void InitOptions(){
	fcOpt.thumbAlpha			= 255;
	fcOpt.bHideOffline			= FALSE;
	fcOpt.bHideAll				= FALSE;
	fcOpt.bHideWhenFullscreen	= FALSE;
	fcOpt.bMoveTogether			= FALSE;
	fcOpt.bFixedWidth			= FALSE;
	fcOpt.nThumbWidth			= 0;
	fcOpt.bShowTip				= TRUE;
	fcOpt.TimeIn				= 0;
	fcOpt.bToTop				= TRUE;
	fcOpt.ToTopTime				= TOTOPTIME_DEF;
	fcOpt.bHideWhenCListShow	= FALSE;
	fcOpt.bUseSingleClick		= FALSE;
	fcOpt.bShowIdle				= TRUE;
}


PLUGININFO pluginInfo ={
	sizeof(PLUGININFO),
		__PLUGIN_NAME,
		0,
		__DESC,
		__AUTHOR,
		__AUTHOREMAIL,
		__COPYRIGHT,
		__AUTHORWEB,
		UNICODE_AWARE,
		0
};

// {53C715A8-EB01-4136-A73C-441868610074}
#define MIID_FLTCONT  { 0x53c715a8, 0xeb01, 0x4136, { 0xa7, 0x3c, 0x44, 0x18, 0x68, 0x61, 0x0, 0x74 } }

static const MUUID interfaces[] = {MIID_FLTCONT, MIID_LAST};

extern "C" __declspec(dllexport) const MUUID * MirandaPluginInterfaces(void)
{
	return interfaces;
}

PLUGININFOEX pluginInfoEx ={
	sizeof(PLUGININFOEX),
		__PLUGIN_NAME,
		0,
		__DESC,
		__AUTHOR,
		__AUTHOREMAIL,
		__COPYRIGHT,
		__AUTHORWEB,
		UNICODE_AWARE,
		0,
		MIID_FLTCONT
};

_inline unsigned int MakeVer(int a,int b,int c,int d)
{
    return PLUGIN_MAKE_VERSION(a,b,c,d);
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion) {

	if( mirandaVersion < MakeVer(MINIMAL_COREVERSION) ) return NULL;
    pluginInfoEx.version=MakeVer(PRODUCT_VERSION);
	return &pluginInfoEx;
}


///////////////////////////////////////////////////////
// Load / unload
BOOL WINAPI DllMain( HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved )
{
	hInst = hinstDLL;
	
	switch ( fdwReason )
	{
		case DLL_PROCESS_ATTACH:	
			break;

		case DLL_PROCESS_DETACH:
			break;
	}
		
	return( TRUE );
}



extern "C" __declspec(dllexport) PLUGININFO* MirandaPluginInfo( DWORD mirandaVersion )
{
	if( mirandaVersion < MakeVer(MINIMAL_COREVERSION) ) return NULL;
    pluginInfo.version=MakeVer(PRODUCT_VERSION);
	return &pluginInfo;
}



extern "C" int __declspec(dllexport) Load( PLUGINLINK *link )
{
	SetLastError( 0 );
	InitOptions();
	pluginLink	= link;

	mir_getLI(&li);

	hevModules		= HookEvent( ME_SYSTEM_MODULESLOADED,  OnModulesLoded );
	bNT				= GetOSPlatform();

	if (hUserDll = LoadLibrary(_T("user32.dll")))
	{
		pSetLayeredWindowAttributes =
			(BOOL (WINAPI *)(HWND,COLORREF,BYTE,DWORD))
			GetProcAddress(hUserDll, "SetLayeredWindowAttributes");
		pUpdateLayeredWindow =
			(BOOL (WINAPI *)(HWND, HDC, POINT *, SIZE *, HDC, POINT *, COLORREF, BLENDFUNCTION *, DWORD))
			GetProcAddress(hUserDll, "UpdateLayeredWindow");
	} else
	{
		pSetLayeredWindowAttributes = NULL;
		pUpdateLayeredWindow = NULL;
	}

	return 0;
}



extern "C" int __declspec(dllexport) Unload()
{
	CleanUp();
	return 0;
}



static void CleanUp()
{
	int nFontId;

	UnhookEvent( hevContactIcon );
	UnhookEvent( hevContactDrag );
	UnhookEvent( hevContactDrop );
	UnhookEvent( hevContactDragStop );
	UnhookEvent( hevSkinIcons );
	UnhookEvent( hevContactDeleted );
	UnhookEvent( hevContactSetting );
	UnhookEvent( hevOptionsInit );
	UnhookEvent( hevStatusMode );
	UnhookEvent( hevModules );
	UnhookEvent( hevPrebuildMenu );

	if(hRemoveThumb) 
		DestroyServiceFunction(hRemoveThumb);
	if(hMainHideAllThumbs) 
		DestroyServiceFunction(hMainHideAllThumbs);
	if(hHideWhenCListShow) 
		DestroyServiceFunction(hHideWhenCListShow);

//	while( pThumbsList != NULL )
//		RemoveThumb( pThumbsList );

	if(ServiceExists(MS_HOTKEY_UNREGISTER)){
		CallService(MS_HOTKEY_UNREGISTER, 0, (LPARAM)(sModule "/MainHideAllThumbs"));
		CallService(MS_HOTKEY_UNREGISTER, 0, (LPARAM)(sModule "/HideWhenCListShow"));
	}

	if (NULL != hLTEdgesPen)
		DeleteObject(hLTEdgesPen);
	if (NULL != hRBEdgesPen)
		DeleteObject(hRBEdgesPen);
	if (NULL != hBmpBackground)
		DeleteObject(hBmpBackground);
	if (NULL != hBkBrush)
	{
		SetClassLong((HWND)WND_CLASS, GCL_HBRBACKGROUND, (LONG)NULL);
		DeleteObject(hBkBrush);
		hBkBrush=NULL;
	}

	for (nFontId = 0; nFontId < FLT_FONTIDS; nFontId++)
		if (NULL != hFont[nFontId])
			DeleteObject(hFont[nFontId]);

	UnregisterWindowClass();

	FreeFileDropping();

	if ( hUserDll != NULL )
	{
		FreeLibrary( hUserDll );
	}
}



///////////////////////////////////////////////////////
// Hooked events
static int OnModulesLoded( WPARAM wParam, LPARAM lParam )
{
	hevContactIcon		= HookEvent( ME_CLIST_CONTACTICONCHANGED,  OnContactIconChanged );
	hevSkinIcons		= HookEvent( ME_SKIN_ICONSCHANGED,  OnSkinIconsChanged );
	hevContactDrag		= HookEvent( ME_CLUI_CONTACTDRAGGING,  OnContactDrag );
	hevContactDrop		= HookEvent( ME_CLUI_CONTACTDROPPED,  OnContactDrop );
	hevContactDragStop	= HookEvent( ME_CLUI_CONTACTDRAGSTOP,  OnContactDragStop );
	hevContactSetting	= HookEvent( ME_DB_CONTACT_SETTINGCHANGED, OnContactSettingChanged );
	hevContactDeleted	= HookEvent( ME_DB_CONTACT_DELETED, OnContactDeleted );
	hevOptionsInit		= HookEvent( ME_OPT_INITIALISE, OnOptionsInitialize );
	hevStatusMode		= HookEvent( ME_CLIST_STATUSMODECHANGE, OnStatusModeChange );
	hevPrebuildMenu		= HookEvent( ME_CLIST_PREBUILDCONTACTMENU, OnPrebuildContactMenu );
	hwndMiranda			= (HWND)CallService( MS_CLUI_GETHWND, 0, 0 );

	oldMirandaWndProc	= (WNDPROC)SetWindowLong( hwndMiranda, GWL_WNDPROC, (LONG)newMirandaWndProc); 

	
	
	// No thumbs yet
//	pThumbsList	= NULL;
	bEnableTip = ServiceExists("mToolTip/ShowTip");
	
	RegisterWindowClass();
	InitFileDropping();
	GetScreenRect();
	LoadDBSettings();
	CreateBackgroundBrush();
	CreateThumbsFont();
	LoadContacts();
	LoadMenus();

	if(fcOpt.bToTop){
		fcOpt.ToTopTime = (fcOpt.ToTopTime<1)?1:fcOpt.ToTopTime;
		fcOpt.ToTopTime = (fcOpt.ToTopTime>TOTOPTIME_MAX)?TOTOPTIME_MAX:fcOpt.ToTopTime;
		ToTopTimerID=SetTimer(NULL, 0, fcOpt.ToTopTime*TOTOPTIME_P, ToTopTimerProc);
	}
	return( 0 );
}


static int OnContactDeleted( WPARAM wParam, LPARAM lParam )
{
	HANDLE		hContact	= ( HANDLE )wParam;
	ThumbInfo	*pThumb		= thumbList.FindThumbByContact( hContact );
	if (!pThumb) return 0;

	pThumb->DeleteContactPos();
	thumbList.RemoveThumb(pThumb);
	
	return 0;
}


static int OnContactIconChanged( WPARAM wParam, LPARAM lParam )
{
	HANDLE		hContact	= ( HANDLE )wParam;
	ThumbInfo	*pThumb		= thumbList.FindThumbByContact( hContact );

	if ( pThumb )
	{
		pThumb->RefreshContactIcon( ( int )lParam );
		//pThumb->ThumbSelect( FALSE );
		//SetTimer( pThumb->hwnd, TIMERID_SELECT_T, 1000, NULL ); 
	}

	return 0;
}


static int OnContactDrag( WPARAM wParam, LPARAM lParam )
{
	ThumbInfo	*pNewThumb	= NULL;
	HANDLE		hContact	= ( HANDLE )wParam;
	ThumbInfo	*pThumb		= thumbList.FindThumbByContact( hContact );
	TCHAR		*ptName		= (TCHAR*)CallService( MS_CLIST_GETCONTACTDISPLAYNAME, wParam, (LPARAM)GCDNF_TCHAR );
	int			idStatus	= ID_STATUS_OFFLINE;
	POINT pt;
	GetCursorPos(&pt);
	
	if ( pThumb == NULL )
	{
		idStatus = GetContactStatus( hContact );
		
		if ( !fcOpt.bHideAll && !HideOnFullScreen() && ( !fcOpt.bHideOffline || IsStatusVisible( idStatus ) )  && (!fcOpt.bHideWhenCListShow || !bIsCListShow) )
		{
//			CreateThumbWnd( pName, hContact, LOWORD( lParam ), HIWORD( lParam ) );
			CreateThumbWnd( ptName, hContact, pt.x, pt.y );
			pNewThumb = thumbList.FindThumbByContact( hContact );
			ShowWindow( pNewThumb->hwnd, SW_SHOWNA );
			hNewContact = hContact;

			OnStatusChanged();
		}
	}
	else if ( hContact == hNewContact )
	{
//		PositionThumb( pThumb, (short)( LOWORD( lParam ) - 5 ), (short)( HIWORD( lParam ) - 5 ) );
		pThumb->PositionThumb( (short)( pt.x - 5 ), (short)( pt.y - 5 ) );
	}
	
	return( hNewContact != NULL ? 1 : 0 );
}



static int OnContactDrop( WPARAM wParam, LPARAM lParam )
{
	RECT	rcMiranda;
	RECT	rcThumb;
//	RECT	rcOverlap;

	HANDLE	hContact	= ( HANDLE )wParam;
	ThumbInfo *pThumb	= thumbList.FindThumbByContact( hContact );

	if ( ( hNewContact == hContact ) && ( pThumb != NULL ) )
	{
		hNewContact = NULL;
		
		GetWindowRect( hwndMiranda, &rcMiranda );
		pThumb->GetThumbRect( &rcThumb );
		
		//if ( IntersectRect( &rcOverlap, &rcThumb, &rcMiranda ) )
		//{
		//	pThumb->OnLButtonDown( (short)(rcThumb.left + 5), (short)(rcThumb.top + 5) );
		//}
	}

	SaveContactsPos();

	return( 1 );
}


static int OnContactDragStop( WPARAM wParam, LPARAM lParam )
{
	HANDLE	hContact = ( HANDLE )wParam;
	ThumbInfo *pThumb = thumbList.FindThumbByContact( hContact );

	if ( ( pThumb != NULL ) && ( hNewContact == hContact ) )
	{
		thumbList.RemoveThumb( pThumb );
		hNewContact = NULL;
	}

	return( 0 );
}


static int OnSkinIconsChanged( WPARAM wParam, LPARAM lParam )
{
	// Get handle to the image list
	himl = ( HIMAGELIST )CallService( MS_CLIST_GETICONSIMAGELIST, 0, 0 );
	
	// Update thumbs
	for (int i = 0; i < thumbList.getCount(); ++i)
		thumbList[i]->UpdateContent();

	return( 0 );
}



static int OnContactSettingChanged( WPARAM wParam, LPARAM lParam )
{
	HANDLE		hContact	= ( HANDLE )wParam;
	ThumbInfo	*pThumb		= thumbList.FindThumbByContact( hContact );
	int			idStatus	= ID_STATUS_OFFLINE;
	BOOL		bRefresh	= TRUE;
	DBCONTACTWRITESETTING* pdbcws = ( DBCONTACTWRITESETTING* )lParam;
	

	if ( hContact == NULL )
	{
		if(( 0 == stricmp( pdbcws->szModule, "CLC" ) ) || ( 0 == stricmp( pdbcws->szModule, sModule ) ) ){
			LoadDBSettings();
			ApplyOptionsChanges();
		}
		
		return( 0 );
	}

	if ( pThumb == NULL ) return( 0 );

	// Only on these 2 events we need to refresh
	if( 0 == stricmp( pdbcws->szSetting, "Status" ) ) 
	{
		idStatus = pdbcws->value.wVal;
	}
	else if ( 0 == stricmp( pdbcws->szSetting, "Nick" ) ) 
	{
		idStatus = GetContactStatus( hContact );
	}
	else if ( 0 == stricmp( pdbcws->szSetting, "MyHandle" ) ) 
	{
		idStatus = GetContactStatus( hContact );
	}
	else if ( fcOpt.bShowIdle && 0 == stricmp( pdbcws->szSetting, "IdleTS" ))
	{
		idStatus = GetContactStatus( hContact );
	}
	else
	{
		bRefresh = FALSE;
	}

	if ( bRefresh )
	{
		// Detach call
		PostMessage( pThumb->hwnd, WM_REFRESH_CONTACT, 0, idStatus );
	}

	return( 0 );
}


static int OnStatusModeChange( WPARAM wParam, LPARAM lParam )
{
	int			idStatus;
	
	//nStatus = (int)wParam;
	
	for (int i = 0; i < thumbList.getCount(); ++i)
	{
		idStatus = GetContactStatus( thumbList[i]->hContact );
		thumbList[i]->RefreshContactStatus( idStatus );
	}
	if ( wParam == ID_STATUS_OFFLINE )
	{
		// Floating status window will use this
	}

	return( 0 );
}



static int OnPrebuildContactMenu( WPARAM wParam, LPARAM lParam )
{
	ThumbInfo *pThumb = thumbList.FindThumbByContact( (HANDLE) wParam );
	CLISTMENUITEM clmi;

	ZeroMemory( &clmi, sizeof( clmi ) );
	clmi.cbSize = sizeof( clmi );

	clmi.flags = ( pThumb == NULL ) ? CMIM_FLAGS | CMIF_HIDDEN : CMIM_FLAGS &~CMIF_HIDDEN;
	CallService( MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenuItemRemove, (LPARAM)&clmi );

	clmi.flags = fcOpt.bHideAll ? CMIM_FLAGS | CMIF_HIDDEN : CMIM_FLAGS &~CMIF_HIDDEN;
	CallService( MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenuItemHideAll, (LPARAM)&clmi );

	return( 0 );
}




///////////////////////////////////////////////////////
// General functions

static void LoadDBSettings()
{
	fcOpt.thumbAlpha			= (BYTE)((double)DBGetContactSettingByte( NULL, sModule, "Opacity", 100 ) * 2.55 );
	fcOpt.bHideOffline			= (BOOL)DBGetContactSettingByte( NULL, sModule, "HideOffline", 0 );
	fcOpt.bHideAll				= (BOOL)DBGetContactSettingByte( NULL, sModule, "HideAll", 0 );
	fcOpt.bHideWhenFullscreen	= (BOOL)DBGetContactSettingByte( NULL, sModule, "HideWhenFullscreen", 0 );
	fcOpt.bMoveTogether			= (BOOL)DBGetContactSettingByte( NULL, sModule, "MoveTogether", 0 );
	fcOpt.bFixedWidth			= (BOOL)DBGetContactSettingByte( NULL, sModule, "FixedWidth", 0 );
	fcOpt.nThumbWidth			= (DWORD)DBGetContactSettingDword( NULL, sModule, "Width", 0 );
	dwOfflineModes				= DBGetContactSettingDword( NULL, "CLC","OfflineModes", CLCDEFAULT_OFFLINEMODES );
	fcOpt.bShowTip				= (BOOL)DBGetContactSettingByte( NULL, sModule, "ShowTip", 1 );
	fcOpt.TimeIn				= (WORD)DBGetContactSettingWord( NULL, sModule, "TimeIn", 0 );
	fcOpt.bToTop				= (BOOL)DBGetContactSettingByte( NULL, sModule, "ToTop", 0 );
	fcOpt.ToTopTime				= (WORD)DBGetContactSettingWord( NULL, sModule, "ToTopTime", TOTOPTIME_DEF );
	fcOpt.bHideWhenCListShow	= (BOOL)DBGetContactSettingByte( NULL, sModule, "HideWhenCListShow", 0 );
	fcOpt.bUseSingleClick		= (BOOL)DBGetContactSettingByte( NULL, sModule, "UseSingleClick", 0 );
	fcOpt.bShowIdle				= (BOOL)DBGetContactSettingByte( NULL, sModule, "ShowIdle", 0 );

	if(DBGetContactSettingByte(NULL, "ModernData", "HideBehind", 0))
		bIsCListShow			= (DBGetContactSettingByte(NULL, "ModernData", "BehindEdge", 0)==0);
	else 
		bIsCListShow			= (DBGetContactSettingByte(NULL,"CList","State",0)==2);
}

void SendMsgDialog( HWND hwnd, char *pText )
{
	ThumbInfo *pThumb = thumbList.FindThumb( hwnd );

	if ( pThumb != NULL )
		CallService( MS_MSG_SENDMESSAGE, (WPARAM)pThumb->hContact, (LPARAM)pText );
}


static void ShowContactMenu( HWND hwnd, POINT pt )
{
	ThumbInfo	*pThumb		= thumbList.FindThumb( hwnd );
	int			idCommand	= 0;

	if ( pThumb != NULL )
	{
		hContactMenu = (HMENU)CallService( MS_CLIST_MENUBUILDCONTACT, (WPARAM)pThumb->hContact, (LPARAM)0 );
		
		if ( hContactMenu == NULL ) return;
		
		idCommand = TrackPopupMenu( hContactMenu, TPM_RIGHTALIGN | TPM_TOPALIGN | TPM_RETURNCMD, pt.x, pt.y, 0 , hwnd, NULL );
		CallService( MS_CLIST_MENUPROCESSCOMMAND, MAKEWPARAM( idCommand ,MPCF_CONTACTMENU), (LPARAM)pThumb->hContact );
	}
}

static LRESULT __stdcall CommWndProc(	HWND	hwnd, 
										UINT	uMsg, 
										WPARAM	wParam, 
										LPARAM  lParam	)
{
	LRESULT		lResult		= 0;
	ThumbInfo	*pThumb		= thumbList.FindThumb( hwnd );

	switch( uMsg )
	{
	case WM_RBUTTONUP:
		{
			POINT pt;
			pt.x = LOWORD( lParam ); 
			pt.y = HIWORD( lParam ); 

			if (pThumb) pThumb->ThumbDeselect( TRUE );

			ClientToScreen(hwnd,&pt);
			ShowContactMenu( hwnd, pt );
		}

		break;

//#if 0
	case WM_NCPAINT:
		if(pThumb){
			HDC hdc = GetWindowDC( hwnd );
			BitBlt(hdc, 0, 0, pThumb->bmpContent.getWidth(), pThumb->bmpContent.getHeight(), pThumb->bmpContent.getDC(), 0, 0, SRCCOPY);
			//RepaintWindow( hwnd, hdc );
			ReleaseDC( hwnd, hdc );
			ValidateRect( hwnd, NULL );
			return 0;
		}

	case WM_PAINT:
		if(pThumb){
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);
			BitBlt(hdc, 0, 0, pThumb->bmpContent.getWidth(), pThumb->bmpContent.getHeight(), pThumb->bmpContent.getDC(), 0, 0, SRCCOPY);
			//RepaintWindow( hwnd, hdc );
			EndPaint(hwnd, &ps);
			break;
		}

	case WM_PRINT:
	case WM_PRINTCLIENT:
		if(pThumb){
			BitBlt((HDC)wParam, 0, 0, pThumb->bmpContent.getWidth(), pThumb->bmpContent.getHeight(), pThumb->bmpContent.getDC(), 0, 0, SRCCOPY);
			//RepaintWindow(hwnd, (HDC)wParam);
			break;
		}
//#endif

	case WM_MEASUREITEM:
		lResult = CallService( MS_CLIST_MENUMEASUREITEM,wParam,lParam );
		break;

	case WM_DRAWITEM:
		lResult = CallService( MS_CLIST_MENUDRAWITEM,wParam,lParam );
		break;

	case WM_LBUTTONDOWN:
		if (pThumb) pThumb->OnLButtonDown( (short)LOWORD( lParam ), (short)HIWORD( lParam ) );
		break;

	case WM_MOUSEMOVE:
		if (pThumb) pThumb->OnMouseMove( (short)LOWORD( lParam ), (short)HIWORD( lParam ), wParam);
		break;

	case WM_LBUTTONUP:
		if (pThumb) pThumb->OnLButtonUp();
		//if (bMouseMoved || !DBGetContactSettingByte(NULL, "CList", "Tray1Click", SETTING_TRAY1CLICK_DEFAULT))
		break;
		// FALL THRU

	case WM_LBUTTONDBLCLK:
		// Popup message dialog
		//if (pThumb) pThumb->ThumbDeselect( TRUE );
		if(!fcOpt.bUseSingleClick && pThumb)
			pThumb->PopUpMessageDialog();
		break;

	case WM_RBUTTONDOWN:
		if(bEnableTip && fcOpt.bShowTip && pThumb) pThumb->KillTip();
		break;
	case WM_REFRESH_CONTACT:
		if (pThumb)
		{
			_tcsncpy( pThumb->ptszName, (TCHAR*)CallService( MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)pThumb->hContact, (LPARAM)GCDNF_TCHAR ), USERNAME_LEN - 1 );
			pThumb->RefreshContactStatus( (int)lParam );
			pThumb->ResizeThumb();
		}
		break;

	case WM_TIMER:
		if(pThumb)pThumb->OnTimer(wParam);
		break;

	case WM_HOTKEY:
		{
			ThumbInfo *pThumb = thumbList.FindThumb( ( HWND )wParam );

			if (pThumb)	pThumb->PopUpMessageDialog();
		}

	default:
		break;
	}

	lResult = DefWindowProc( hwnd, uMsg, wParam, lParam );

	return( lResult );
}



extern void SetThumbsOpacity( BYTE btAlpha )
{
	for (int i = 0; i < thumbList.getCount(); ++i)
		thumbList[i]->SetThumbOpacity( btAlpha );
}



static void GetScreenRect()
{
	rcScreen.left	= GetSystemMetrics( SM_XVIRTUALSCREEN );
	rcScreen.top	= GetSystemMetrics( SM_YVIRTUALSCREEN );
	rcScreen.right	= GetSystemMetrics( SM_CXVIRTUALSCREEN ) + rcScreen.left;
	rcScreen.bottom	= GetSystemMetrics( SM_CYVIRTUALSCREEN ) + rcScreen.top;
}



void OnStatusChanged()
{
	int			idStatus	= ID_STATUS_OFFLINE;

	for (int i = 0; i < thumbList.getCount(); ++i)
	{
		idStatus = GetContactStatus( thumbList[i]->hContact );
		thumbList[i]->RefreshContactStatus( idStatus );
	}
}



void ApplyOptionsChanges()
{
	CreateThumbsFont();
	CreateBackgroundBrush();

//	dwOfflineModes	= DBGetContactSettingDword( NULL, "CLC","OfflineModes", CLCDEFAULT_OFFLINEMODES );
	if(!fcOpt.bToTop && ToTopTimerID){
		KillTimer(NULL, ToTopTimerID);
		ToTopTimerID = 0;
	}
	if(fcOpt.bToTop){
		if (ToTopTimerID) KillTimer(NULL, ToTopTimerID);
		fcOpt.ToTopTime = (fcOpt.ToTopTime<1)?1:fcOpt.ToTopTime;
		fcOpt.ToTopTime = (fcOpt.ToTopTime>TOTOPTIME_MAX)?TOTOPTIME_MAX:fcOpt.ToTopTime;
		ToTopTimerID = SetTimer(NULL, 0, fcOpt.ToTopTime*TOTOPTIME_P, ToTopTimerProc);
	}

	OnStatusChanged();

	for (int i = 0; i < thumbList.getCount(); ++i)
		thumbList[i]->ResizeThumb();
}



///////////////////////////////////////////////////////
// Window creation
static void RegisterWindowClass()
{
	WNDCLASSEX	wcx;
	ZeroMemory( &wcx, sizeof( wcx ) );
		
	wcx.cbSize			=	sizeof( WNDCLASSEX );
	wcx.style			=	CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS;
	wcx.lpfnWndProc		=	CommWndProc;
	wcx.cbClsExtra		=	0;
	wcx.cbWndExtra		=	0;
	wcx.hInstance		=	hInst;
	wcx.hIcon			=	NULL;
	wcx.hCursor			=	NULL;
	wcx.hbrBackground	=	GetSysColorBrush(COLOR_3DFACE);
	wcx.lpszMenuName	=	NULL;
	wcx.lpszClassName	=	WND_CLASS;
	wcx.hIconSm			=	NULL;
	
	SetLastError( 0 );
	
	RegisterClassEx( &wcx );
}



static void UnregisterWindowClass()
{
	UnregisterClass( WND_CLASS, hInst );
}



static void CreateThumbWnd( TCHAR *ptszName, HANDLE hContact, int nX, int nY )
{
	HWND		hwnd		= NULL;
	ThumbInfo	*pThumb		= thumbList.FindThumbByContact( hContact );

	// Prepare for window creation
	if ( pThumb == NULL )
	{
		hwnd = CreateWindowEx(	WS_EX_TOOLWINDOW | WS_EX_TOPMOST,
								WND_CLASS,
								ptszName,
								WS_POPUP,
								nX,
								nY,
								50,
								20,
								NULL /*hwndMiranda*/,
								NULL,
								hInst,
								NULL
								 );

		if ( hwnd != NULL ) 
		{
			pThumb = thumbList.AddThumb( hwnd, ptszName, hContact );
			SetWindowLong( hwnd, GWL_USERDATA, (long)pThumb );
			pThumb->ResizeThumb();
			
			pThumb->SetThumbOpacity( fcOpt.thumbAlpha );
			ShowWindow( hwnd, ( fcOpt.bHideAll || HideOnFullScreen() || fcOpt.bHideOffline  || (fcOpt.bHideWhenCListShow && bIsCListShow) )? SW_HIDE : SW_SHOWNA );
			pThumb->PositionThumb( (short)nX, (short)nY );

			// force repaint
			pThumb->UpdateContent();
		}
	}
}



static BOOL GetOSPlatform()
{
	OSVERSIONINFO	VersionInfo;
	
	// Get OS platform
	ZeroMemory( &VersionInfo, sizeof( VersionInfo ) );
	VersionInfo.dwOSVersionInfoSize = sizeof( VersionInfo );
	
	GetVersionEx( &VersionInfo );
	return( VersionInfo.dwPlatformId == VER_PLATFORM_WIN32_NT );
}



static void CreateThumbsFont()
{
	int nFontId;
	LOGFONTA lf;

	for (nFontId = 0; nFontId < FLT_FONTIDS; nFontId++)
	{
		if (NULL != hFont[nFontId])
		{
			DeleteObject(hFont[nFontId]);
			hFont[nFontId] = NULL;
		}
		GetFontSetting(TRUE, nFontId, &lf, &tColor[nFontId]);
		{
			LONG height;
			HDC hdc=GetDC(NULL);
			height=lf.lfHeight;
			lf.lfHeight=-MulDiv(lf.lfHeight, GetDeviceCaps(hdc, LOGPIXELSY), 72);
			ReleaseDC(NULL,hdc);				

			hFont[nFontId] = CreateFontIndirectA(&lf);

			lf.lfHeight=height;
		}
	}
}



static void CreateBackgroundBrush()
{
//	LOGBRUSH	lb;
	bkColor = DBGetContactSettingDword(NULL, sModule, "BkColor", FLT_DEFAULT_BKGNDCOLOR);


	if (NULL != hLTEdgesPen)
	{
		DeleteObject(hLTEdgesPen);
		hLTEdgesPen = NULL;
	}
	if (NULL != hRBEdgesPen)
	{
		DeleteObject(hRBEdgesPen);
		hRBEdgesPen = NULL;
	}
	if (NULL != hBmpBackground)
	{
		DeleteObject(hBmpBackground);
		hBmpBackground = NULL;
	}
	if (NULL != hBkBrush)
	{
		SetClassLong((HWND)WND_CLASS, GCL_HBRBACKGROUND, (LONG)NULL);
		DeleteObject( hBkBrush );
		hBkBrush = NULL;
	}

	if (DBGetContactSettingByte(NULL, sModule, "DrawBorder", FLT_DEFAULT_DRAWBORDER))
	{
		COLORREF cr;

		cr = (COLORREF)DBGetContactSettingDword(NULL, sModule, "LTEdgesColor", FLT_DEFAULT_LTEDGESCOLOR);
		hLTEdgesPen = CreatePen(PS_SOLID, 1, cr);
		cr = (COLORREF)DBGetContactSettingDword(NULL, sModule, "RBEdgesColor", FLT_DEFAULT_RBEDGESCOLOR);
		hRBEdgesPen = CreatePen(PS_SOLID, 1, cr);
	}
	if (DBGetContactSettingByte(NULL, sModule, "BkUseBitmap", FLT_DEFAULT_BKGNDUSEBITMAP))
	{
		DBVARIANT dbv;

		if (!DBGetContactSetting(NULL, sModule, "BkBitmap", &dbv))
		{
			hBmpBackground = (HBITMAP)CallService(MS_UTILS_LOADBITMAP, 0, (LPARAM)dbv.pszVal);
			DBFreeVariant(&dbv);
		}
	}
	nBackgroundBmpUse = (WORD)DBGetContactSettingWord(NULL, sModule, "BkBitmapOpt", FLT_DEFAULT_BKGNDBITMAPOPT);

	// Create brush
	hBkBrush	= CreateSolidBrush(bkColor);

	// Attach brush to the window
	SetClassLong((HWND)WND_CLASS, GCL_HBRBACKGROUND, (LONG)hBkBrush);
}


static int GetContactStatus( HANDLE hContact )
{
	char	*szProto	= NULL;
	int		idStatus	= ID_STATUS_OFFLINE;

	if ( hContact == NULL )
	{
		assert( !"Contact is NULL" );
		return( 0 );
	}

	szProto	= ( char* )CallService( "Proto/GetContactBaseProto", (WPARAM)hContact, 0 );
	
	if ( NULL != szProto )
	{
		idStatus	= DBGetContactSettingWord( hContact, szProto, "Status", ID_STATUS_OFFLINE );
	}
	

	return( idStatus );
}


static int ClcStatusToPf2( int status )
{
	switch(status) {
		case ID_STATUS_ONLINE: return PF2_ONLINE;
		case ID_STATUS_AWAY: return PF2_SHORTAWAY;
		case ID_STATUS_DND: return PF2_HEAVYDND;
		case ID_STATUS_NA: return PF2_LONGAWAY;
		case ID_STATUS_OCCUPIED: return PF2_LIGHTDND;
		case ID_STATUS_FREECHAT: return PF2_FREECHAT;
		case ID_STATUS_INVISIBLE: return PF2_INVISIBLE;
		case ID_STATUS_ONTHEPHONE: return PF2_ONTHEPHONE;
		case ID_STATUS_OUTTOLUNCH: return PF2_OUTTOLUNCH;
		case ID_STATUS_OFFLINE: return MODEF_OFFLINE;
	}
	return 0;
}

BOOL IsStatusVisible( int status )
{
	return ( 0 == ( dwOfflineModes & ClcStatusToPf2( status ) ) );
}



void RegHotkey( HANDLE hContact, HWND hwnd )
{
	char szBuf[ MAX_PATH ]= {0};
/*	char szPath[ MAX_PATH ]= {0};

	GetModuleFileName( NULL, szPath, MAX_PATH );
	PathRemoveFileSpec( szPath );
	strcat( szPath, "\\Thumbs.ini" );
	GetPrivateProfileString( "Hotkeys", szName, "", szBuf, MAX_PATH - 1, szPath );*/
	DBVARIANT dbv;
	if(DBGetContactSettingString ( hContact, sModule, "Hotkey", &dbv)) return;
	strncpy(szBuf, dbv.pszVal, MAX_PATH - 1);
	DBFreeVariant( &dbv );

	if ( 0 != strlen( szBuf ) )
	{
		UINT nModifiers			= 0;
		char chKey				= 0;
		char szMod[ 2 ][ 20 ]	= {0};
		char szKey[ 20 ]		= {0};
		int i = 0;

		sscanf( szBuf, "%[^'+']+%[^'+']+%[^'+']", szMod[ 0 ], szMod[ 1 ], szKey );

		for (  i = 0; i < 2; ++i )
		{
			if( 0 == strncmp( szMod[ i ], "ALT", 19 ) )
			{
				nModifiers = nModifiers | MOD_ALT;
			}
			else if( 0 == strncmp( szMod[ i ], "CTRL", 19 ) )
			{
				nModifiers = nModifiers | MOD_CONTROL;
			}
			else if( 0 == strncmp( szMod[ i ], "SHIFT", 19 ) )
			{
				nModifiers = nModifiers | MOD_SHIFT;
			}
		}

		chKey = szKey[ 0 ];

		RegisterHotKey( hwnd, (int)hwnd, nModifiers, VkKeyScan( chKey ) );
	}
}



///////////////////////////////////////////////////////
// Contact sttings

void SaveContactsPos()
{	
	RECT					rc;
	
	for (int i = 0; i < thumbList.getCount(); ++i)
	{
		SetLastError( 0 );

		thumbList[i]->GetThumbRect( &rc );
		
		if( 0 == GetLastError() )
			DBWriteContactSettingDword( thumbList[i]->hContact, sModule, "ThumbsPos", DB_POS_MAKE_XY(rc.left,rc.top) );
	}
}


static void LoadContacts()
{
	HANDLE	hContact = (HANDLE)CallService( MS_DB_CONTACT_FINDFIRST, 0, 0) ;

	while( hContact != NULL )
	{
		LoadContact( hContact );
		
		hContact = (HANDLE)CallService( MS_DB_CONTACT_FINDNEXT, ( WPARAM ) hContact, 0 );
	}
}




static void LoadMenus()
{
	CLISTMENUITEM mi;

		// Remove thumb menu item
		hRemoveThumb = CreateServiceFunction( sModule "/RemoveThumb",OnContactMenu_Remove );
		ZeroMemory( &mi,sizeof( mi ) );
		
		mi.cbSize		= sizeof( mi );
		mi.position		= 0xFFFFF;
		mi.flags		= CMIF_TCHAR;
		mi.hIcon		= LoadIcon( hInst, MAKEINTRESOURCE( IDI_HIDE ) );
		mi.ptszName		= _T("Remove thumb");
		mi.pszService	= sModule "/RemoveThumb";
		hMenuItemRemove	= (HANDLE)CallService( MS_CLIST_ADDCONTACTMENUITEM,0, ( LPARAM )&mi );

		// Hide all thumbs main menu item
		hMainHideAllThumbs = CreateServiceFunction( sModule "/MainHideAllThumbs",OnMainMenu_HideAll );
		ZeroMemory( &mi,sizeof( mi ) );
		
		mi.cbSize		= sizeof( mi );
		mi.position		= 0xFFFFF;
		mi.flags		= CMIF_TCHAR;
		mi.hIcon		= LoadIcon( hInst, MAKEINTRESOURCE( fcOpt.bHideAll ? IDI_SHOW : IDI_HIDE ) );
		mi.ptszName		= fcOpt.bHideAll ? _T("Show all thumbs") : _T("Hide all thumbs");
		mi.pszService	= sModule "/MainHideAllThumbs";
		hMainMenuItemHideAll	= (HANDLE)CallService( MS_CLIST_ADDMAINMENUITEM,0, ( LPARAM )&mi );

		if(ServiceExists(MS_HOTKEY_REGISTER)){
			HOTKEYDESC hkd={0};

			hkd.cbSize = sizeof(hkd);
			hkd.pszSection = "Floating Contacts";
			
			hkd.pszName = sModule "/MainHideAllThumbs";
			hkd.pszDescription = "Show/Hide all thumbs";
			hkd.pszService = sModule "/MainHideAllThumbs";
			CallService(MS_HOTKEY_REGISTER, 0, (LPARAM)&hkd);

			hHideWhenCListShow = CreateServiceFunction( sModule "/HideWhenCListShow",OnHotKey_HideWhenCListShow );
			hkd.pszName = sModule "/HideWhenCListShow";
			hkd.pszDescription = "Hide when contact list is shown";
			hkd.pszService = sModule "/HideWhenCListShow";
			CallService(MS_HOTKEY_REGISTER, 0, (LPARAM)&hkd);
		}
}


static void LoadContact( HANDLE hContact )
{
	DWORD	dwPos		= (DWORD)-1;
	TCHAR	*ptName		= NULL;
	ThumbInfo	*pThumb	= thumbList.FindThumbByContact( hContact );
	int		nX, nY;
	

	if ( hContact == NULL ) return;

	dwPos = DBGetContactSettingDword( hContact, sModule, "ThumbsPos", (DWORD)-1 );

	if ( dwPos != -1 )
	{
		ptName = (TCHAR*)CallService( MS_CLIST_GETCONTACTDISPLAYNAME, ( WPARAM )hContact, (LPARAM)GCDNF_TCHAR );

		if ( ptName != NULL )
		{
			nX = DB_POS_GETX( dwPos );
			nY = DB_POS_GETY( dwPos );
			
			CreateThumbWnd( ptName, hContact, nX, nY );
			pThumb->PositionThumb( (short)nX, (short)nY );
		}
	}
}


BOOL HideOnFullScreen()
{
	BOOL bFullscreen = FALSE;
	HWND hWnd = 0;
	
	if ( fcOpt.bHideWhenFullscreen )
	{
		int w = GetSystemMetrics(SM_CXSCREEN);
		int h = GetSystemMetrics(SM_CYSCREEN);
	
		hWnd = GetForegroundWindow();

		while (GetWindowLong(hWnd, GWL_EXSTYLE) & WS_EX_TOPMOST)
		{
			RECT WindowRect;
			GetWindowRect(hWnd, &WindowRect);
			
			if ( (w == (WindowRect.right - WindowRect.left) ) &&
				(h == (WindowRect.bottom - WindowRect.top)))
			{	
				bFullscreen = TRUE;
				break;
			}
		
			hWnd = GetNextWindow( hWnd, GW_HWNDNEXT );
		}
	}
	
	return bFullscreen && fcOpt.bHideWhenFullscreen;
}


static int OnContactMenu_Remove( WPARAM wParam,LPARAM lParam )
{
	HANDLE		hContact	= ( HANDLE )wParam;
	ThumbInfo	*pThumb		= thumbList.FindThumbByContact( hContact );

	if (pThumb)
	{
		pThumb->DeleteContactPos();
		thumbList.RemoveThumb(pThumb);
	}

	DestroyMenu( hContactMenu );

	return 0;
}

static int OnHotKey_HideWhenCListShow( WPARAM wParam,LPARAM lParam )
{
	fcOpt.bHideWhenCListShow=!fcOpt.bHideWhenCListShow;
	DBWriteContactSettingByte(NULL, sModule, "HideWhenCListShow", (BYTE)fcOpt.bHideWhenCListShow);
	OnStatusChanged();
	return 0;
}


static int OnMainMenu_HideAll( WPARAM wParam,LPARAM lParam )
{
	CLISTMENUITEM clmi = {0};
	int b;

	fcOpt.bHideAll = !fcOpt.bHideAll;
	DBWriteContactSettingByte(NULL, sModule, "HideAll", (BYTE)fcOpt.bHideAll);
	OnStatusChanged();

	clmi.cbSize		= sizeof( clmi );
	clmi.flags		= CMIM_NAME | CMIM_ICON|CMIF_TCHAR;
	clmi.hIcon		= LoadIcon( hInst, MAKEINTRESOURCE( fcOpt.bHideAll ? IDI_SHOW : IDI_HIDE ) );
	clmi.ptszName	= fcOpt.bHideAll ? _T("Show all thumbs") : _T("Hide all thumbs");
	b = CallService( MS_CLIST_MODIFYMENUITEM, ( WPARAM )hMainMenuItemHideAll, ( LPARAM )&clmi );
	return 0;
}

static VOID CALLBACK ToTopTimerProc ( HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	for (int i = 0; i < thumbList.getCount(); ++i)
	{
		SetWindowPos(thumbList[i]->hwnd, 
					HWND_TOPMOST, 
					0, 
					0, 
					0, 
					0,
					SWP_NOSIZE | SWP_NOMOVE | /*SWP_NOZORDER |*/ SWP_NOACTIVATE);
	}
}

void ShowThumbsOnHideCList()
{
	if(!fcOpt.bHideWhenCListShow || fcOpt.bHideAll || HideOnFullScreen())return;
	for (int i = 0; i < thumbList.getCount(); ++i)
	{
		if ( !fcOpt.bHideOffline || IsStatusVisible( GetContactStatus(thumbList[i]->hContact) )  )
		{
			ShowWindow( thumbList[i]->hwnd, SW_SHOWNA );
		}
	}
}


void HideThumbsOnShowCList()
{
	if(!fcOpt.bHideWhenCListShow || fcOpt.bHideAll || HideOnFullScreen())return;
	for (int i = 0; i < thumbList.getCount(); ++i)
		ShowWindow( thumbList[i]->hwnd, SW_HIDE );
}

static LRESULT __stdcall newMirandaWndProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( uMsg == WM_WINDOWPOSCHANGED)
	{			
		WINDOWPOS *wp = (WINDOWPOS *)lParam;
		if(wp->flags&SWP_HIDEWINDOW) {
			bIsCListShow = FALSE;
			ShowThumbsOnHideCList();
		}
		else if(wp->flags&SWP_SHOWWINDOW){
			bIsCListShow = TRUE;
			HideThumbsOnShowCList();
		}
		else if(!(wp->flags&SWP_NOMOVE)){
			BYTE method=DBGetContactSettingByte(NULL, "ModernData", "HideBehind", 0);
			if(method) {
				WORD wBehindEdgeBorderSize=DBGetContactSettingWord(NULL, "ModernData", "HideBehindBorderSize", 0);
				RECT rc = {wp->x, wp->y, wp->x+wp->cx, wp->y+wp->cy};
				RECT rcScreen = {wBehindEdgeBorderSize*(2-method), 0, GetSystemMetrics(SM_CXSCREEN)-wBehindEdgeBorderSize*(method-1), GetSystemMetrics(SM_CYSCREEN)};
				RECT rcOverlap;
				BOOL isIntersect;

				isIntersect = IntersectRect( &rcOverlap, &rc, &rcScreen );
				if ( !isIntersect && bIsCListShow ){
					bIsCListShow = FALSE;
					ShowThumbsOnHideCList();
				}
				else if ( isIntersect && !bIsCListShow ){
					bIsCListShow = TRUE;
					HideThumbsOnShowCList();
				}
			}
		}
	}
	return( CallWindowProc(oldMirandaWndProc, hwnd, uMsg, wParam, lParam) );
} 
