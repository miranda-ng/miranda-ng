/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2010 Miranda ICQ/IM project,
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
#include "genmenu.h"

static bool bIsGenMenuInited;
bool bIconsDisabled;
static CRITICAL_SECTION csMenuHook;

static int NextObjectId = 0x100, NextObjectMenuItemId = CLISTMENUIDMIN;

#if defined( _DEBUG )
static void DumpMenuItem( TMO_IntMenuItem* pParent, int level = 0 )
{
	char temp[ 30 ];
	memset( temp, '\t', level );
	temp[ level ] = 0;

	for ( PMO_IntMenuItem pimi = pParent; pimi != NULL; pimi = pimi->next ) {
		Netlib_Logf( NULL, "%sMenu item %08p [%08p]: %S", temp, pimi, pimi->mi.root, pimi->mi.ptszName );

		PMO_IntMenuItem submenu = pimi->submenu.first;
		if ( submenu )
			DumpMenuItem( submenu, level+1 );
}	}

#endif

static int CompareMenus( const TIntMenuObject* p1, const TIntMenuObject* p2 )
{
	return lstrcmpA( p1->Name, p2->Name );
}

LIST<TIntMenuObject> g_menus( 10, CompareMenus );

void FreeAndNil( void **p )
{
	if ( p == NULL )
		return;

	if ( *p != NULL ) {
		mir_free( *p );
		*p = NULL;
}	}

int GetMenuObjbyId( const int id )
{
	for ( int i=0; i < g_menus.getCount(); i++ )
		if ( g_menus[i]->id == id )
			return i;

	return -1;
}

PMO_IntMenuItem MO_RecursiveWalkMenu( PMO_IntMenuItem parent, pfnWalkFunc func, void* param )
{
	if ( parent == NULL )
		return FALSE;

	PMO_IntMenuItem pnext;
	for ( PMO_IntMenuItem pimi = parent; pimi != NULL; pimi = pnext ) {
		PMO_IntMenuItem submenu = pimi->submenu.first;
		pnext = pimi->next;
		if ( func( pimi, param )) // it can destroy the menu item
			return pimi;

		if ( submenu ) {
			PMO_IntMenuItem res = MO_RecursiveWalkMenu( submenu, func, param );
			if ( res )
				return res;
		}
	}

	return FALSE;
}

//wparam=0
//lparam=LPMEASUREITEMSTRUCT
int MO_MeasureMenuItem( LPMEASUREITEMSTRUCT mis )
{
	// prevent win9x from ugly menus displaying when there is no icon
	mis->itemWidth = 0;
	mis->itemHeight = 0;

	if ( !bIsGenMenuInited )
		return -1;

	if ( mis == NULL )
		return FALSE;

	PMO_IntMenuItem pimi = MO_GetIntMenuItem(( HGENMENU )mis->itemData );
	if ( pimi == NULL )
		return FALSE;

	if ( pimi->iconId == -1 )
		return FALSE;

	mis->itemWidth = max(0,GetSystemMetrics(SM_CXSMICON)-GetSystemMetrics(SM_CXMENUCHECK)+4);
	mis->itemHeight = GetSystemMetrics(SM_CYSMICON)+2;
	return TRUE;
}

//wparam=0
//lparam=LPDRAWITEMSTRUCT
int MO_DrawMenuItem( LPDRAWITEMSTRUCT dis )
{
	if ( !bIsGenMenuInited )
		return -1;

	if ( dis == NULL )
		return FALSE;

	EnterCriticalSection( &csMenuHook );

	PMO_IntMenuItem pimi = MO_GetIntMenuItem(( HGENMENU )dis->itemData );
	if ( pimi == NULL || pimi->iconId == -1 ) {
		LeaveCriticalSection( &csMenuHook );
		return FALSE;
	}

	int y = (dis->rcItem.bottom - dis->rcItem.top - GetSystemMetrics(SM_CYSMICON))/2+1;
	if ( dis->itemState & ODS_SELECTED ) {
		if ( dis->itemState & ODS_CHECKED ) {
			RECT rc;
			rc.left = 2; rc.right = GetSystemMetrics(SM_CXSMICON)+2;
			rc.top = y; rc.bottom = rc.top+GetSystemMetrics(SM_CYSMICON)+2;
			FillRect(dis->hDC, &rc, GetSysColorBrush( COLOR_HIGHLIGHT ));
			ImageList_DrawEx( pimi->parent->m_hMenuIcons, pimi->iconId, dis->hDC, 2, y, 0, 0, CLR_NONE, CLR_DEFAULT, ILD_SELECTED );
		}
		else ImageList_DrawEx( pimi->parent->m_hMenuIcons, pimi->iconId, dis->hDC, 2, y, 0, 0, CLR_NONE, CLR_DEFAULT, ILD_FOCUS );
	}
	else {
		if ( dis->itemState & ODS_CHECKED) {
			RECT rc;
			rc.left = 0; rc.right = GetSystemMetrics(SM_CXSMICON)+4;
			rc.top = y-2; rc.bottom = rc.top + GetSystemMetrics(SM_CYSMICON)+4;
			DrawEdge(dis->hDC,&rc,BDR_SUNKENOUTER,BF_RECT);
			InflateRect(&rc,-1,-1);
			COLORREF menuCol = GetSysColor(COLOR_MENU);
			COLORREF hiliteCol = GetSysColor(COLOR_3DHIGHLIGHT);
			HBRUSH hBrush = CreateSolidBrush(RGB((GetRValue(menuCol)+GetRValue(hiliteCol))/2,(GetGValue(menuCol)+GetGValue(hiliteCol))/2,(GetBValue(menuCol)+GetBValue(hiliteCol))/2));
			FillRect(dis->hDC,&rc,GetSysColorBrush(COLOR_MENU));
			DeleteObject(hBrush);
			ImageList_DrawEx(pimi->parent->m_hMenuIcons,pimi->iconId,dis->hDC,2,y,0,0,CLR_NONE,GetSysColor(COLOR_MENU),ILD_BLEND50);
		}
		else ImageList_DrawEx(pimi->parent->m_hMenuIcons,pimi->iconId,dis->hDC,2,y,0,0,CLR_NONE,CLR_NONE,ILD_NORMAL);
	}
	LeaveCriticalSection( &csMenuHook );
	return TRUE;
}

int MO_RemoveAllObjects()
{
	int i;
	for ( i=0; i < g_menus.getCount(); i++ )
		delete g_menus[i];

	g_menus.destroy();
	return 0;
}

//wparam=MenuObjectHandle
INT_PTR MO_RemoveMenuObject(WPARAM wParam, LPARAM)
{
	int objidx;

	if ( !bIsGenMenuInited) return -1;
	EnterCriticalSection( &csMenuHook );

	objidx = GetMenuObjbyId(( int )wParam );
	if ( objidx == -1 ) {
		LeaveCriticalSection( &csMenuHook );
		return -1;
	}

	delete g_menus[ objidx ];
	g_menus.remove( objidx );
	LeaveCriticalSection( &csMenuHook );
	return 0;
}

//wparam=MenuObjectHandle
//lparam=vKey
INT_PTR MO_ProcessHotKeys( HANDLE menuHandle, INT_PTR vKey )
{
	if ( !bIsGenMenuInited)
		return -1;

	EnterCriticalSection( &csMenuHook );

	int objidx = GetMenuObjbyId( (int)menuHandle );
	if ( objidx == -1 ) {
		LeaveCriticalSection( &csMenuHook );
		return FALSE;
	}

	for ( PMO_IntMenuItem pimi = g_menus[objidx]->m_items.first; pimi != NULL; pimi = pimi->next ) {
		if ( pimi->mi.hotKey == 0 ) continue;
		if ( HIWORD(pimi->mi.hotKey) != vKey) continue;
		if ( !(LOWORD(pimi->mi.hotKey) & MOD_ALT     ) != !( GetKeyState( VK_MENU    ) & 0x8000)) continue;
		if ( !(LOWORD(pimi->mi.hotKey) & MOD_CONTROL ) != !( GetKeyState( VK_CONTROL ) & 0x8000)) continue;
		if ( !(LOWORD(pimi->mi.hotKey) & MOD_SHIFT   ) != !( GetKeyState( VK_SHIFT   ) & 0x8000)) continue;

		MO_ProcessCommand( pimi, 0 );
		LeaveCriticalSection( &csMenuHook );
		return TRUE;
	}

	LeaveCriticalSection( &csMenuHook );
	return FALSE;
}

INT_PTR MO_GetProtoRootMenu(WPARAM wParam,LPARAM lParam)
{
	char* szProto = ( char* )wParam;
	if ( szProto == NULL )
		return 0;

	if ( DBGetContactSettingByte( NULL, "CList", "MoveProtoMenus", FALSE ))
		return ( INT_PTR )cli.pfnGetProtocolMenu( szProto );

	int objidx = GetMenuObjbyId(( int )hMainMenuObject );
	if ( objidx == -1 )
		return NULL;

	EnterCriticalSection( &csMenuHook );

	TIntMenuObject* pmo = g_menus[objidx];
	PMO_IntMenuItem p;
	for ( p = pmo->m_items.first; p != NULL; p = p->next )
		if ( !lstrcmpA( p->UniqName, szProto ))
			break;

	LeaveCriticalSection( &csMenuHook );
	return ( INT_PTR )p;
}

//wparam=MenuItemHandle
//lparam=PMO_MenuItem
INT_PTR MO_GetMenuItem(WPARAM wParam,LPARAM lParam)
{
	PMO_MenuItem mi = (PMO_MenuItem)lParam;
	if ( !bIsGenMenuInited || mi == NULL )
		return -1;

	PMO_IntMenuItem pimi = MO_GetIntMenuItem(( HGENMENU )wParam);
	EnterCriticalSection( &csMenuHook );
	if ( !pimi ) {
		LeaveCriticalSection( &csMenuHook );
		return -1;
	}

	*mi = pimi->mi;
	LeaveCriticalSection( &csMenuHook );
	return 0;
}

static int FindDefaultItem( PMO_IntMenuItem pimi, void* )
{
	if ( pimi->mi.flags & ( CMIF_GRAYED | CMIF_HIDDEN ))
		return FALSE;

	return ( pimi->mi.flags & CMIF_DEFAULT ) ? TRUE : FALSE;
}

INT_PTR MO_GetDefaultMenuItem(WPARAM wParam, LPARAM)
{
	if ( !bIsGenMenuInited )
		return -1;

	PMO_IntMenuItem pimi = MO_GetIntMenuItem(( HGENMENU )wParam);
	EnterCriticalSection( &csMenuHook );
	if ( pimi )
		pimi = MO_RecursiveWalkMenu( pimi, FindDefaultItem, NULL );

	LeaveCriticalSection( &csMenuHook );
	return ( INT_PTR )pimi;
}

//wparam MenuItemHandle
//lparam PMO_MenuItem
int MO_ModifyMenuItem( PMO_IntMenuItem menuHandle, PMO_MenuItem pmi )
{
	int oldflags;

	if ( !bIsGenMenuInited || pmi == NULL || pmi->cbSize != sizeof( TMO_MenuItem ))
		return -1;

	EnterCriticalSection( &csMenuHook );

	PMO_IntMenuItem pimi = MO_GetIntMenuItem(( HGENMENU )menuHandle );
	if ( !pimi ) {
		LeaveCriticalSection( &csMenuHook );
		return -1;
	}

	if ( pmi->flags & CMIM_NAME ) {
		FreeAndNil(( void** )&pimi->mi.pszName );
#if defined( _UNICODE )
		if ( pmi->flags & CMIF_UNICODE )
			pimi->mi.ptszName = mir_tstrdup(( pmi->flags & CMIF_KEEPUNTRANSLATED ) ? pmi->ptszName : TranslateTS( pmi->ptszName ));
		else {
			if ( pmi->flags & CMIF_KEEPUNTRANSLATED ) {
				int len = lstrlenA( pmi->pszName );
				pimi->mi.ptszName = ( TCHAR* )mir_alloc( sizeof( TCHAR )*( len+1 ));
				MultiByteToWideChar( CP_ACP, 0, pmi->pszName, -1, pimi->mi.ptszName, len+1 );
				pimi->mi.ptszName[ len ] = 0;
			}
			else pimi->mi.ptszName = LangPackPcharToTchar( pmi->pszName );
		}
#else
		pimi->mi.ptszName = mir_strdup(( pmi->flags & CMIF_KEEPUNTRANSLATED ) ? pmi->ptszName :  Translate( pmi->ptszName ));
#endif
	}
	if ( pmi->flags & CMIM_FLAGS ) {
		oldflags = pimi->mi.flags & ( CMIF_ROOTHANDLE | CMIF_ICONFROMICOLIB );
		pimi->mi.flags = (pmi->flags & ~CMIM_ALL) | oldflags;
	}
	if ( (pmi->flags & CMIM_ICON) && !bIconsDisabled ) {
		if ( pimi->mi.flags & CMIF_ICONFROMICOLIB ) {
			HICON hIcon = IcoLib_GetIconByHandle( pmi->hIcolibItem, false );
			if ( hIcon != NULL ) {
				pimi->hIcolibItem = pmi->hIcolibItem;
				pimi->iconId = ImageList_ReplaceIcon( pimi->parent->m_hMenuIcons, pimi->iconId, hIcon );
				IconLib_ReleaseIcon( hIcon, 0 );
			}
			else pimi->iconId = -1, pimi->hIcolibItem = NULL;
		}
		else {
			pimi->mi.hIcon = pmi->hIcon;
			if ( pmi->hIcon != NULL )
				pimi->iconId = ImageList_ReplaceIcon( pimi->parent->m_hMenuIcons, pimi->iconId, pmi->hIcon );
			else
				pimi->iconId = -1;	  //fixme, should remove old icon & shuffle all iconIds
		}
		if (pimi->hBmp) DeleteObject(pimi->hBmp); pimi->hBmp = NULL;
	}

	if ( pmi->flags & CMIM_HOTKEY )
		pimi->mi.hotKey = pmi->hotKey;

	LeaveCriticalSection( &csMenuHook );
	return 0;
}

//wparam MenuItemHandle
//return ownerdata useful to free ownerdata before delete menu item,
//NULL on error.
INT_PTR MO_MenuItemGetOwnerData(WPARAM wParam, LPARAM)
{
	if ( !bIsGenMenuInited )
		return -1;

	EnterCriticalSection( &csMenuHook );
	PMO_IntMenuItem pimi = MO_GetIntMenuItem(( HGENMENU )wParam );
	if ( !pimi ) {
		LeaveCriticalSection( &csMenuHook );
		return -1;
	}

	INT_PTR res = ( INT_PTR )pimi->mi.ownerdata;
	LeaveCriticalSection( &csMenuHook );
	return res;
}

PMO_IntMenuItem MO_GetIntMenuItem(HGENMENU wParam)
{
	PMO_IntMenuItem result = ( PMO_IntMenuItem )wParam;
	if ( result == NULL || wParam == (HGENMENU)0xffff1234 || wParam == HGENMENU_ROOT)
		return NULL;

	__try
	{
		if ( result->signature != MENUITEM_SIGNATURE )
			result = NULL;
	}
	__except( EXCEPTION_EXECUTE_HANDLER )
	{
		result = NULL;
	}

	return result;
}

//LOWORD(wparam) menuident

static int FindMenuByCommand( PMO_IntMenuItem pimi, void* pCommand )
{
	return ( pimi->iCommand == (int)pCommand );
}

int MO_ProcessCommandBySubMenuIdent(int menuID, int command, LPARAM lParam)
{
	if ( !bIsGenMenuInited )
		return -1;

	EnterCriticalSection( &csMenuHook );

	int objidx = GetMenuObjbyId( menuID );
	if ( objidx == -1 ) {
		LeaveCriticalSection( &csMenuHook );
		return -1;
	}

	PMO_IntMenuItem pimi = MO_RecursiveWalkMenu( g_menus[objidx]->m_items.first, FindMenuByCommand, ( void* )command );
	if ( pimi ) {
		LeaveCriticalSection( &csMenuHook );
		return MO_ProcessCommand( pimi, lParam );
	}

	LeaveCriticalSection( &csMenuHook );
	return -1;
}

INT_PTR MO_ProcessCommandByMenuIdent(WPARAM wParam,LPARAM lParam)
{
	if ( !bIsGenMenuInited )
		return -1;

	EnterCriticalSection( &csMenuHook );

	for ( int i=0; i < g_menus.getCount(); i++ ) {
		PMO_IntMenuItem pimi = MO_RecursiveWalkMenu( g_menus[i]->m_items.first, FindMenuByCommand, ( void* )wParam );
		if ( pimi ) {
			LeaveCriticalSection( &csMenuHook );
			return MO_ProcessCommand( pimi, lParam );
	}	}

	LeaveCriticalSection( &csMenuHook );
	return FALSE;
}

int MO_ProcessCommand( PMO_IntMenuItem aHandle, LPARAM lParam )
{
	if ( !bIsGenMenuInited )
		return -1;

	EnterCriticalSection( &csMenuHook );
	PMO_IntMenuItem pimi = MO_GetIntMenuItem( aHandle );
	if ( !pimi ) {
		LeaveCriticalSection( &csMenuHook );
		return -1;
	}

	char *srvname = pimi->parent->ExecService;
	void *ownerdata = pimi->mi.ownerdata;
	LeaveCriticalSection( &csMenuHook );
	CallService( srvname, ( WPARAM )ownerdata, lParam );
	return 1;
}

int MO_SetOptionsMenuItem( PMO_IntMenuItem aHandle, int setting, INT_PTR value )
{
	if ( !bIsGenMenuInited )
		return -1;

	EnterCriticalSection( &csMenuHook );
	PMO_IntMenuItem pimi = MO_GetIntMenuItem( aHandle );
	if ( !pimi ) {
		LeaveCriticalSection( &csMenuHook );
		return -1;
	}

	int res = -1;
	__try 
	{
		res = 1;
		if ( setting == OPT_MENUITEMSETUNIQNAME ) {
			mir_free( pimi->UniqName );
			pimi->UniqName = mir_strdup(( char* )value );
		}
	}
	__except( EXCEPTION_EXECUTE_HANDLER ) {}

	LeaveCriticalSection( &csMenuHook );
	return res;
}

int MO_SetOptionsMenuObject( HANDLE handle, int setting, INT_PTR value )
{
	int  pimoidx;
	int  res = 0;

	if ( !bIsGenMenuInited )
		return -1;

	EnterCriticalSection( &csMenuHook );
	__try 
	{
		pimoidx = GetMenuObjbyId( (int)handle );
		res = pimoidx != -1;
		if ( res ) {
			TIntMenuObject* pmo = g_menus[pimoidx];

			switch ( setting ) {
			case OPT_MENUOBJECT_SET_ONADD_SERVICE:
				FreeAndNil(( void** )&pmo->onAddService );
				pmo->onAddService = mir_strdup(( char* )value );
				break;

			case OPT_MENUOBJECT_SET_FREE_SERVICE:
				FreeAndNil(( void** )&pmo->FreeService );
				pmo->FreeService = mir_strdup(( char* )value );
				break;

			case OPT_MENUOBJECT_SET_CHECK_SERVICE:
				FreeAndNil(( void** )&pmo->CheckService );
				pmo->CheckService = mir_strdup(( char* )value);
				break;

			case OPT_USERDEFINEDITEMS:
				pmo->m_bUseUserDefinedItems = ( BOOL )value;
				break;
			}
		}
	}
	__except( EXCEPTION_EXECUTE_HANDLER ) {}

	LeaveCriticalSection( &csMenuHook );
	return res;
}

//wparam=0;
//lparam=PMenuParam;
//result=MenuObjectHandle
INT_PTR MO_CreateNewMenuObject(WPARAM, LPARAM lParam)
{
	PMenuParam pmp = ( PMenuParam )lParam;
	if ( !bIsGenMenuInited || pmp == NULL )
		return -1;

	EnterCriticalSection( &csMenuHook );
	TIntMenuObject* p = new TIntMenuObject();
	p->id = NextObjectId++;
	p->Name = mir_strdup( pmp->name );
	p->CheckService = mir_strdup( pmp->CheckService );
	p->ExecService = mir_strdup( pmp->ExecService );
	p->m_hMenuIcons = ImageList_Create( GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 
		(IsWinVerXPPlus() ? ILC_COLOR32 : ILC_COLOR16) | ILC_MASK, 15, 100 );
	g_menus.insert(p);

	LeaveCriticalSection( &csMenuHook );
	return p->id;
}

//wparam=MenuItemHandle
//lparam=0

static int FreeMenuItem( TMO_IntMenuItem* pimi, void* )
{
	pimi->parent->freeItem( pimi );
	return FALSE;
}

static int FindParent( TMO_IntMenuItem* pimi, void* p )
{
	return pimi->next == p;
}

INT_PTR MO_RemoveMenuItem(WPARAM wParam, LPARAM)
{
	EnterCriticalSection( &csMenuHook );
	PMO_IntMenuItem pimi = MO_GetIntMenuItem(( HGENMENU )wParam );
	if ( !pimi ) {
		LeaveCriticalSection( &csMenuHook );
		return -1;
	}

	if ( pimi->submenu.first ) {
		MO_RecursiveWalkMenu( pimi->submenu.first, FreeMenuItem, NULL );
		pimi->submenu.first = NULL;
	}

	PMO_IntMenuItem prev = MO_RecursiveWalkMenu( pimi->owner->first, FindParent, pimi );
	if ( prev )
		prev->next = pimi->next;
	if ( pimi->owner->first == pimi )
		pimi->owner->first = pimi->next;
	if ( pimi->owner->last == pimi )
		pimi->owner->last = prev;

	pimi->signature = 0; // invalidate all future calls to that object
	pimi->parent->freeItem( pimi );

	LeaveCriticalSection( &csMenuHook );
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// we presume that this function is being called inside csMenuHook only

static int PackMenuItems( PMO_IntMenuItem pimi, void* )
{
	pimi->iCommand = NextObjectMenuItemId++;
	return FALSE;
}

static int GetNextObjectMenuItemId()
{
	// if menu commands are exausted, pack the menu array
	if ( NextObjectMenuItemId >= CLISTMENUIDMAX ) {
		NextObjectMenuItemId = CLISTMENUIDMIN;
		for ( int i=0; i < g_menus.getCount(); i++ )
			MO_RecursiveWalkMenu( g_menus[i]->m_items.first, PackMenuItems, NULL );
	}

	return NextObjectMenuItemId++;
}

//wparam=MenuObjectHandle
//lparam=PMO_MenuItem
//return MenuItemHandle
PMO_IntMenuItem MO_AddNewMenuItem( HANDLE menuobjecthandle, PMO_MenuItem pmi )
{
	if ( !bIsGenMenuInited || pmi == NULL || pmi->cbSize != sizeof( TMO_MenuItem ))
		return NULL;

	//old mode
	if ( !( pmi->flags & CMIF_ROOTHANDLE ))
		return MO_AddOldNewMenuItem( menuobjecthandle, pmi );

	EnterCriticalSection( &csMenuHook );
	int objidx = GetMenuObjbyId( (int)menuobjecthandle );
	if ( objidx == -1 ) {
		LeaveCriticalSection( &csMenuHook );
		return NULL;
	}

	TIntMenuObject* pmo = g_menus[objidx];

	TMO_IntMenuItem* p = ( TMO_IntMenuItem* )mir_calloc( sizeof( TMO_IntMenuItem ));
	p->parent = pmo;
	p->signature = MENUITEM_SIGNATURE;
	p->iCommand = GetNextObjectMenuItemId();
	p->mi = *pmi;
	p->iconId = -1;
	p->OverrideShow = TRUE;
	p->originalPosition = pmi->position;
	#if defined( _UNICODE )
		if ( pmi->flags & CMIF_UNICODE ) 
			p->mi.ptszName = mir_tstrdup(( pmi->flags & CMIF_KEEPUNTRANSLATED ) ? pmi->ptszName : TranslateTS( pmi->ptszName ));
		else {
			if ( pmi->flags & CMIF_KEEPUNTRANSLATED )
				p->mi.ptszName = mir_a2u(pmi->pszName);
			else
				p->mi.ptszName = LangPackPcharToTchar( pmi->pszName );
		}
	#else
		p->mi.ptszName = mir_strdup(( pmi->flags & CMIF_KEEPUNTRANSLATED ) ? pmi->ptszName : Translate( pmi->ptszName ));
	#endif

	if ( pmi->hIcon != NULL && !bIconsDisabled ) {
		if ( pmi->flags & CMIF_ICONFROMICOLIB ) {
			HICON hIcon = IcoLib_GetIconByHandle( pmi->hIcolibItem, false );
			p->iconId = ImageList_AddIcon( pmo->m_hMenuIcons, hIcon );
			p->hIcolibItem = pmi->hIcolibItem;
			IconLib_ReleaseIcon( hIcon, 0 );
		}
		else {
			HANDLE hIcolibItem = IcoLib_IsManaged( pmi->hIcon );
			if ( hIcolibItem ) {
				p->iconId = ImageList_AddIcon( pmo->m_hMenuIcons, pmi->hIcon );
				p->hIcolibItem = hIcolibItem;
			}
			else p->iconId = ImageList_AddIcon( pmo->m_hMenuIcons, pmi->hIcon );
	}	}

	if ( p->mi.root == HGENMENU_ROOT )
		p->mi.root = NULL;

	PMO_IntMenuItem pRoot = ( p->mi.root != NULL ) ? MO_GetIntMenuItem( p->mi.root ) : NULL;
	if ( pRoot )
		p->owner = &pRoot->submenu;
	else
		p->owner = &pmo->m_items;

	if ( !p->owner->first )
		p->owner->first = p;
	if ( p->owner->last )
		p->owner->last->next = p;
	p->owner->last = p;

	LeaveCriticalSection( &csMenuHook );
	return p;
}

//wparam=MenuObjectHandle
//lparam=PMO_MenuItem

int FindRoot( PMO_IntMenuItem pimi, void* param )
{
	if ( pimi->mi.pszName != NULL )
		if ( pimi->submenu.first && !_tcscmp( pimi->mi.ptszName, ( TCHAR* )param ))
			return TRUE;

	return FALSE;
}

PMO_IntMenuItem MO_AddOldNewMenuItem( HANDLE menuobjecthandle, PMO_MenuItem pmi )
{
	if ( !bIsGenMenuInited || pmi == NULL )
		return NULL;

	int objidx = GetMenuObjbyId( (int)menuobjecthandle );
	if ( objidx == -1 )
		return NULL;

	if ( pmi->cbSize != sizeof( TMO_MenuItem ))
		return NULL;

	if ( pmi->flags & CMIF_ROOTHANDLE )
		return NULL;

	//is item with popup or not
	if ( pmi->root == 0 ) {
		//yes,this without popup
		pmi->root = NULL; //first level
	}
	else { // no,search for needed root and create it if need
		TCHAR* tszRoot;
#if defined( _UNICODE )
		if ( pmi->flags & CMIF_UNICODE )
			tszRoot = mir_tstrdup(TranslateTS(( TCHAR* )pmi->root ));
		else
			tszRoot = LangPackPcharToTchar(( char* )pmi->root );
#else
		tszRoot = mir_tstrdup(TranslateTS(( TCHAR* )pmi->root ));
#endif

		PMO_IntMenuItem oldroot = MO_RecursiveWalkMenu( g_menus[objidx]->m_items.first, FindRoot, tszRoot );
		mir_free( tszRoot );

		if ( oldroot == NULL ) {
			//not found,creating root
			TMO_MenuItem tmi = { 0 };
			tmi = *pmi;
			tmi.flags |= CMIF_ROOTHANDLE;
			tmi.ownerdata = 0;
			tmi.root = NULL;
			//copy pszPopupName
			tmi.ptszName = ( TCHAR* )pmi->root;
			if (( oldroot = MO_AddNewMenuItem( menuobjecthandle, &tmi )) != NULL )
				MO_SetOptionsMenuItem( oldroot, OPT_MENUITEMSETUNIQNAME, (INT_PTR)pmi->root );
		}
		pmi->root = oldroot;

		//popup will be created in next commands
	}
	pmi->flags |= CMIF_ROOTHANDLE;
	//add popup(root allready exists)
	return MO_AddNewMenuItem( menuobjecthandle, pmi );
}

static int WhereToPlace( HMENU hMenu, PMO_MenuItem mi )
{
	MENUITEMINFO mii = { 0 };
	mii.cbSize = MENUITEMINFO_V4_SIZE;
	mii.fMask = MIIM_SUBMENU | MIIM_DATA;
	for ( int i=GetMenuItemCount( hMenu )-1; i >= 0; i-- ) {
		GetMenuItemInfo( hMenu, i, TRUE, &mii );
		if ( mii.fType != MFT_SEPARATOR ) {
			PMO_IntMenuItem pimi = MO_GetIntMenuItem(( HGENMENU )mii.dwItemData);
			if ( pimi != NULL )
				if ( pimi->mi.position <= mi->position )
					return i+1;
	}	}

	return 0;
}

static void InsertMenuItemWithSeparators(HMENU hMenu, int uItem, MENUITEMINFO *lpmii)
{
	int needSeparator = 0;
	MENUITEMINFO mii;

	PMO_IntMenuItem pimi = MO_GetIntMenuItem(( HGENMENU )lpmii->dwItemData );
	if ( pimi == NULL )
		return;

	int thisItemPosition = pimi->mi.position;

	ZeroMemory( &mii, sizeof( mii ));
	mii.cbSize = MENUITEMINFO_V4_SIZE;
	//check for separator before
	if ( uItem ) {
		mii.fMask = MIIM_SUBMENU | MIIM_DATA | MIIM_TYPE;
		GetMenuItemInfo( hMenu, uItem-1, TRUE, &mii );
		pimi = MO_GetIntMenuItem(( HGENMENU )mii.dwItemData );
		if ( pimi != NULL ) {
			if ( mii.fType == MFT_SEPARATOR )
				needSeparator = 0;
			else
				needSeparator = ( pimi->mi.position / SEPARATORPOSITIONINTERVAL ) != thisItemPosition / SEPARATORPOSITIONINTERVAL;
		}
		if ( needSeparator) {
			//but might be supposed to be after the next one instead
			mii.fType = 0;
			if ( uItem < GetMenuItemCount( hMenu )) {
				mii.fMask = MIIM_SUBMENU | MIIM_DATA | MIIM_TYPE;
				GetMenuItemInfo( hMenu, uItem, TRUE, &mii );
			}
			if ( mii.fType != MFT_SEPARATOR) {
				mii.fMask = MIIM_TYPE;
				mii.fType = MFT_SEPARATOR;
				InsertMenuItem( hMenu, uItem, TRUE, &mii );
			}
			uItem++;
	}	}

	//check for separator after
	if ( uItem < GetMenuItemCount( hMenu )) {
		mii.fMask = MIIM_SUBMENU | MIIM_DATA | MIIM_TYPE;
		mii.cch = 0;
		GetMenuItemInfo( hMenu, uItem, TRUE, &mii );
		pimi = MO_GetIntMenuItem(( HGENMENU )mii.dwItemData );
		if ( pimi != NULL ) {
			if ( mii.fType == MFT_SEPARATOR )
				needSeparator=0;
			else
				needSeparator = pimi->mi.position / SEPARATORPOSITIONINTERVAL != thisItemPosition / SEPARATORPOSITIONINTERVAL;
		}
		if ( needSeparator) {
			mii.fMask = MIIM_TYPE;
			mii.fType = MFT_SEPARATOR;
			InsertMenuItem( hMenu, uItem, TRUE, &mii );
	}	}

	if ( uItem == GetMenuItemCount( hMenu )-1 ) {
		TCHAR str[32];
		mii.fMask = MIIM_SUBMENU | MIIM_DATA | MIIM_TYPE;
		mii.dwTypeData = str;
		mii.cch = SIZEOF( str );
		GetMenuItemInfo( hMenu, uItem, TRUE, &mii );
	}

	// create local copy *lpmii so we can change some flags
	MENUITEMINFO mii_copy = *lpmii;
	lpmii = &mii_copy;

	if (( GetMenuItemCount( hMenu ) % 35 ) == 33 /* will be 34 after addition :) */ && pimi != NULL )
		if ( pimi->mi.root != NULL ) {
			if ( !( lpmii->fMask & MIIM_FTYPE ))
				lpmii->fType = 0;
			lpmii->fMask |= MIIM_FTYPE;
			lpmii->fType |= MFT_MENUBARBREAK;
		}

	InsertMenuItem( hMenu, uItem, TRUE, lpmii );
}

//wparam started hMenu
//lparam ListParam*
//result hMenu
INT_PTR MO_BuildMenu(WPARAM wParam,LPARAM lParam)
{
	if ( !bIsGenMenuInited )
		return -1;

	EnterCriticalSection( &csMenuHook );

	ListParam *lp = ( ListParam* )lParam;
	int pimoidx = GetMenuObjbyId( (int)lp->MenuObjectHandle );
	if ( pimoidx == -1 ) {
		LeaveCriticalSection( &csMenuHook );
		return 0;
	}

	#if defined( _DEBUG )
		// DumpMenuItem( g_menus[pimoidx]->m_items.first );
	#endif

	INT_PTR res = (INT_PTR)BuildRecursiveMenu(( HMENU )wParam, g_menus[pimoidx]->m_items.first, ( ListParam* )lParam );
	LeaveCriticalSection( &csMenuHook );
	return res;
}

#ifdef _DEBUG
#define PUTPOSITIONSONMENU
#endif

void GetMenuItemName( PMO_IntMenuItem pMenuItem, char* pszDest, size_t cbDestSize )
{
	if ( pMenuItem->UniqName )
		mir_snprintf( pszDest, cbDestSize, "{%s}", pMenuItem->UniqName );
	else if (pMenuItem->mi.flags & CMIF_UNICODE) {
		char* name = mir_t2a( pMenuItem->mi.ptszName );
		mir_snprintf( pszDest, cbDestSize, "{%s}", name );
		mir_free(name);
	}
	else
		mir_snprintf( pszDest, cbDestSize, "{%s}", pMenuItem->mi.pszName );
}

HMENU BuildRecursiveMenu(HMENU hMenu, PMO_IntMenuItem pRootMenu, ListParam *param)
{
	if ( param == NULL || pRootMenu == NULL )
		return NULL;

	TIntMenuObject* pmo = pRootMenu->parent;

	int rootlevel = ( param->rootlevel == -1 ) ? 0 : param->rootlevel;

	ListParam localparam = *param;

	while ( rootlevel == 0 && GetMenuItemCount( hMenu ) > 0 )
		DeleteMenu( hMenu, 0, MF_BYPOSITION );

	for ( PMO_IntMenuItem pmi = pRootMenu; pmi != NULL; pmi = pmi->next ) {
		PMO_MenuItem mi = &pmi->mi;
		if ( mi->cbSize != sizeof( TMO_MenuItem ))
			continue;

		if ( mi->flags & CMIF_HIDDEN )
			continue;

		if ( pmo->CheckService != NULL )	{
			TCheckProcParam CheckParam;
			CheckParam.lParam = param->lParam;
			CheckParam.wParam = param->wParam;
			CheckParam.MenuItemOwnerData = mi->ownerdata;
			CheckParam.MenuItemHandle = pmi;
			if ( CallService( pmo->CheckService, ( WPARAM )&CheckParam, 0 ) == FALSE )
				continue;
		}

		/**************************************/
		if ( rootlevel == 0 && mi->root == NULL && pmo->m_bUseUserDefinedItems ) {
			char DBString[256];
			DBVARIANT dbv = { 0 };
			int pos;
			char MenuNameItems[256];
			mir_snprintf(MenuNameItems, SIZEOF(MenuNameItems), "%s_Items", pmo->Name);

			char menuItemName[256];
			GetMenuItemName( pmi, menuItemName, sizeof( menuItemName ));

			// check if it visible
			mir_snprintf( DBString, SIZEOF(DBString), "%s_visible", menuItemName );
			if ( DBGetContactSettingByte( NULL, MenuNameItems, DBString, -1 ) == -1 )
				DBWriteContactSettingByte( NULL, MenuNameItems, DBString, 1 );

			pmi->OverrideShow = TRUE;
			if ( !DBGetContactSettingByte( NULL, MenuNameItems, DBString, 1 )) {
				pmi->OverrideShow = FALSE;
				continue;  // find out what value to return if not getting added
			}

			// mi.pszName
			mir_snprintf( DBString, SIZEOF(DBString), "%s_name", menuItemName );
			if ( !DBGetContactSettingTString( NULL, MenuNameItems, DBString, &dbv )) {
				if ( _tcslen( dbv.ptszVal ) > 0 ) {
					if ( pmi->CustomName ) mir_free( pmi->CustomName );
					pmi->CustomName = mir_tstrdup( dbv.ptszVal );
				}
				DBFreeVariant( &dbv );
			}

			mir_snprintf( DBString, SIZEOF(DBString), "%s_pos", menuItemName );
			if (( pos = DBGetContactSettingDword( NULL, MenuNameItems, DBString, -1 )) == -1 ) {
				DBWriteContactSettingDword( NULL, MenuNameItems, DBString, mi->position );
				if ( pmi->submenu.first )
					mi->position = 0;
			}
			else mi->position = pos;
		}

		/**************************************/

		if ( rootlevel != (int)pmi->mi.root )
			continue;

		MENUITEMINFO mii = { 0 };
		mii.dwItemData = ( LPARAM )pmi;

		int i = WhereToPlace( hMenu, mi );

		if ( !IsWinVer98Plus()) {
			mii.cbSize = MENUITEMINFO_V4_SIZE;
			mii.fMask = MIIM_DATA | MIIM_TYPE | MIIM_ID;
			mii.fType = MFT_STRING;
		}
		else {
			mii.cbSize = sizeof( mii );
			mii.fMask = MIIM_DATA | MIIM_ID | MIIM_STRING;
			if ( pmi->iconId != -1 ) {
				mii.fMask |= MIIM_BITMAP;
				if (IsWinVerVistaPlus() && isThemeActive()) {
					if (pmi->hBmp == NULL)
						pmi->hBmp = ConvertIconToBitmap(NULL, pmi->parent->m_hMenuIcons, pmi->iconId);
					mii.hbmpItem = pmi->hBmp;
				}
				else
					mii.hbmpItem = HBMMENU_CALLBACK;
			}
		}

		mii.fMask |= MIIM_STATE;
		mii.fState = (( pmi->mi.flags & CMIF_GRAYED ) ? MFS_GRAYED : MFS_ENABLED );
		mii.fState |= (( pmi->mi.flags & CMIF_CHECKED) ? MFS_CHECKED : MFS_UNCHECKED );
		if ( pmi->mi.flags & CMIF_DEFAULT ) mii.fState |= MFS_DEFAULT;

		mii.dwTypeData = ( pmi->CustomName ) ? pmi->CustomName : mi->ptszName;

		// it's a submenu
		if ( pmi->submenu.first ) {
			mii.fMask |= MIIM_SUBMENU;
			mii.hSubMenu = CreatePopupMenu();

			#ifdef PUTPOSITIONSONMENU
				if ( GetKeyState(VK_CONTROL) & 0x8000) {
					TCHAR str[256];
					mir_sntprintf( str, SIZEOF(str), _T( "%s (%d,id %x)" ), mi->pszName, mi->position, mii.dwItemData );
					mii.dwTypeData = str;
				}
			#endif

			InsertMenuItemWithSeparators( hMenu, i, &mii);
			localparam.rootlevel = LPARAM( pmi );
			BuildRecursiveMenu( mii.hSubMenu, pmi->submenu.first, &localparam );
		}
		else {
			mii.wID = pmi->iCommand;

			#ifdef PUTPOSITIONSONMENU
				if ( GetKeyState(VK_CONTROL) & 0x8000) {
					TCHAR str[256];
					mir_sntprintf( str, SIZEOF(str), _T("%s (%d,id %x)"), mi->pszName, mi->position, mii.dwItemData );
					mii.dwTypeData = str;
				}
			#endif

			if ( pmo->onAddService != NULL )
				if ( CallService( pmo->onAddService, ( WPARAM )&mii, ( LPARAM )pmi ) == FALSE )
					continue;

			InsertMenuItemWithSeparators( hMenu, i, &mii );
	}	}

	return hMenu;
}

/////////////////////////////////////////////////////////////////////////////////////////
// iconlib in menu

static int MO_ReloadIcon( PMO_IntMenuItem pmi, void* )
{
	if ( pmi->hIcolibItem ) {
		HICON newIcon = IcoLib_GetIconByHandle( pmi->hIcolibItem, false );
		if ( newIcon )
			ImageList_ReplaceIcon( pmi->parent->m_hMenuIcons, pmi->iconId, newIcon );

		IconLib_ReleaseIcon(newIcon,0);
	}

	return FALSE;
}

int OnIconLibChanges(WPARAM, LPARAM)
{
	EnterCriticalSection( &csMenuHook );
	for ( int mo=0; mo < g_menus.getCount(); mo++ )
		if ( (int)hStatusMenuObject != g_menus[mo]->id ) //skip status menu
			MO_RecursiveWalkMenu( g_menus[mo]->m_items.first, MO_ReloadIcon, 0 );

	LeaveCriticalSection( &csMenuHook );

	cli.pfnReloadProtoMenus();
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
//

static int MO_RegisterIcon( PMO_IntMenuItem pmi, void* )
{
	char *uname, *descr;
	uname = pmi->UniqName;
	if ( uname == NULL )
		#ifdef UNICODE
			uname = mir_u2a(pmi->CustomName);
			descr = mir_u2a(pmi->mi.ptszName);
		#else
			uname = pmi->CustomName;
			descr = pmi->mi.pszName;
		#endif

	if ( !uname && !descr )
		return FALSE;

	if ( !pmi->hIcolibItem ) {
		HICON hIcon = ImageList_GetIcon( pmi->parent->m_hMenuIcons, pmi->iconId, 0 );
		char* buf = NEWSTR_ALLOCA( descr );

		char sectionName[256], iconame[256];
		mir_snprintf( sectionName, sizeof(sectionName), "Menu Icons/%s", pmi->parent->Name );

		// remove '&'
		char* start = buf;
		while ( start ) {
			if (( start = strchr( start, '&' )) == NULL )
				break;

			memmove(start,start+1,strlen(start+1)+1);
			if (*start!='\0') start++;
			else break;
		}

		mir_snprintf(iconame, sizeof(iconame), "genmenu_%s_%s", pmi->parent->Name, uname && *uname ? uname : descr);

		SKINICONDESC sid={0};
		sid.cbSize = sizeof(sid);
		sid.cx = 16;
		sid.cy = 16;
		sid.pszSection = sectionName;
		sid.pszName = iconame;
		sid.pszDefaultFile = NULL;
		sid.pszDescription = buf;
		sid.hDefaultIcon = hIcon;
		pmi->hIcolibItem = ( HANDLE )CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);

		Safe_DestroyIcon( hIcon );
		if ( hIcon = ( HICON )CallService( MS_SKIN2_GETICON, 0, (LPARAM)iconame )) {
			ImageList_ReplaceIcon( pmi->parent->m_hMenuIcons, pmi->iconId, hIcon );
			IconLib_ReleaseIcon( hIcon, 0 );
	}	}

	#ifdef UNICODE
		if ( !pmi->UniqName )
			mir_free( uname );
		mir_free( descr );
	#endif

	return FALSE;
}

int RegisterAllIconsInIconLib()
{
	//register all icons
	for ( int mo=0; mo < g_menus.getCount(); mo++ ) {
		if ( (int)hStatusMenuObject == g_menus[mo]->id ) //skip status menu
			continue;

		MO_RecursiveWalkMenu( g_menus[mo]->m_items.first, MO_RegisterIcon, 0 );
	}

	return 0;
}

int TryProcessDoubleClick( HANDLE hContact )
{
	int iMenuID = GetMenuObjbyId( (int)hContactMenuObject );
	if ( iMenuID != -1 ) {
		NotifyEventHooks(hPreBuildContactMenuEvent,(WPARAM)hContact,0);

		PMO_IntMenuItem pimi = ( PMO_IntMenuItem )MO_GetDefaultMenuItem(( WPARAM )g_menus[ iMenuID ]->m_items.first, 0 );
		if ( pimi != NULL ) {
			MO_ProcessCommand( pimi, ( LPARAM )hContact );
			return 0;
	}	}

	return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Static services

int posttimerid;

static VOID CALLBACK PostRegisterIcons( HWND, UINT, UINT_PTR, DWORD )
{
	KillTimer( 0, posttimerid );
	RegisterAllIconsInIconLib();
}

static int OnModulesLoaded(WPARAM, LPARAM)
{
	posttimerid = SetTimer(( HWND )NULL, 0, 5, ( TIMERPROC )PostRegisterIcons );
	HookEvent(ME_SKIN2_ICONSCHANGED,OnIconLibChanges);
	return 0;
}

static INT_PTR SRVMO_SetOptionsMenuObject( WPARAM, LPARAM lParam)
{
	lpOptParam lpop = ( lpOptParam )lParam;
	if ( lpop == NULL )
		return 0;

	return MO_SetOptionsMenuObject( lpop->Handle, lpop->Setting, lpop->Value );
}

static INT_PTR SRVMO_SetOptionsMenuItem( WPARAM, LPARAM lParam)
{
	lpOptParam lpop = ( lpOptParam )lParam;
	if ( lpop == NULL )
		return 0;

	return MO_SetOptionsMenuItem(( PMO_IntMenuItem )lpop->Handle, lpop->Setting, lpop->Value );
}

int InitGenMenu()
{
	InitializeCriticalSection( &csMenuHook );
	CreateServiceFunction( MO_BUILDMENU, MO_BuildMenu );

	CreateServiceFunction( MO_PROCESSCOMMAND, ( MIRANDASERVICE )MO_ProcessCommand );
	CreateServiceFunction( MO_CREATENEWMENUOBJECT, MO_CreateNewMenuObject );
	CreateServiceFunction( MO_REMOVEMENUITEM, MO_RemoveMenuItem );
	CreateServiceFunction( MO_ADDNEWMENUITEM, ( MIRANDASERVICE )MO_AddNewMenuItem );
	CreateServiceFunction( MO_MENUITEMGETOWNERDATA, MO_MenuItemGetOwnerData );
	CreateServiceFunction( MO_MODIFYMENUITEM, ( MIRANDASERVICE )MO_ModifyMenuItem );
	CreateServiceFunction( MO_GETMENUITEM, MO_GetMenuItem );
	CreateServiceFunction( MO_GETDEFAULTMENUITEM, MO_GetDefaultMenuItem );
	CreateServiceFunction( MO_PROCESSCOMMANDBYMENUIDENT, MO_ProcessCommandByMenuIdent );
	CreateServiceFunction( MO_PROCESSHOTKEYS, ( MIRANDASERVICE )MO_ProcessHotKeys );
	CreateServiceFunction( MO_REMOVEMENUOBJECT, MO_RemoveMenuObject );
	CreateServiceFunction( MO_GETPROTOROOTMENU, MO_GetProtoRootMenu );

	CreateServiceFunction( MO_SETOPTIONSMENUOBJECT, SRVMO_SetOptionsMenuObject );
	CreateServiceFunction( MO_SETOPTIONSMENUITEM, SRVMO_SetOptionsMenuItem );

	bIconsDisabled = DBGetContactSettingByte(NULL, "CList", "DisableMenuIcons", 0) != 0;

	EnterCriticalSection( &csMenuHook );
	bIsGenMenuInited = true;
	LeaveCriticalSection( &csMenuHook );

	HookEvent( ME_SYSTEM_MODULESLOADED, OnModulesLoaded );
	HookEvent( ME_OPT_INITIALISE,       GenMenuOptInit  );
	return 0;
}

int UnitGenMenu()
{
	if ( bIsGenMenuInited ) {
		EnterCriticalSection( &csMenuHook );
		MO_RemoveAllObjects();
		bIsGenMenuInited=false;

		LeaveCriticalSection( &csMenuHook );
		DeleteCriticalSection(&csMenuHook);
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

TIntMenuObject::TIntMenuObject()
{
}

TIntMenuObject::~TIntMenuObject()
{
	MO_RecursiveWalkMenu( m_items.first, FreeMenuItem, NULL );

	FreeAndNil(( void** )&FreeService );
	FreeAndNil(( void** )&onAddService );
	FreeAndNil(( void** )&CheckService );
	FreeAndNil(( void** )&ExecService );
	FreeAndNil(( void** )&Name );

	ImageList_Destroy(m_hMenuIcons);
}

void TIntMenuObject::freeItem( TMO_IntMenuItem* p )
{
	if ( FreeService )
		CallService( FreeService, ( WPARAM )p, ( LPARAM )p->mi.ownerdata );

	FreeAndNil(( void** )&p->mi.pszName );
	FreeAndNil(( void** )&p->UniqName   );
	FreeAndNil(( void** )&p->CustomName );
	if ( p->hBmp ) DeleteObject( p->hBmp );
	mir_free( p );
}
