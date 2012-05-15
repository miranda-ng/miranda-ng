/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2009 Miranda ICQ/IM project,
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

#include "IcoLib.h"

static BOOL bModuleInitialized = FALSE;
static HANDLE hIcons2ChangedEvent, hIconsChangedEvent;
static HICON hIconBlank = NULL;

HANDLE hIcoLib_AddNewIcon, hIcoLib_RemoveIcon, hIcoLib_GetIcon, hIcoLib_GetIcon2,
       hIcoLib_GetIconHandle, hIcoLib_IsManaged, hIcoLib_AddRef, hIcoLib_ReleaseIcon;

static int iconEventActive = 0;

static BOOL bNeedRebuild = FALSE;

struct IcoLibOptsData {
	HWND hwndIndex;
};

CRITICAL_SECTION csIconList;

#define SECTIONPARAM_MAKE(index, level, flags) MAKELONG( (index)&0xFFFF, MAKEWORD( level, flags ) )
#define SECTIONPARAM_INDEX(lparam) LOWORD( lparam )
#define SECTIONPARAM_LEVEL(lparam) LOBYTE( HIWORD(lparam) )
#define SECTIONPARAM_FLAGS(lparam) HIBYTE( HIWORD(lparam) )
#define SECTIONPARAM_HAVEPAGE	0x0001

static int sttCompareSections( const SectionItem* p1, const SectionItem* p2 )
{	return _tcscmp( p1->name, p2->name );
}

static LIST<SectionItem> sectionList( 20, sttCompareSections );

static int sttCompareIconSourceFiles( const IconSourceFile* p1, const IconSourceFile* p2 )
{	return _tcsicmp( p1->file, p2->file );
}

static LIST<IconSourceFile> iconSourceFileList( 10, sttCompareIconSourceFiles );

static int sttCompareIconSourceItems( const IconSourceItem* p1, const IconSourceItem* p2 )
{	if (p1->indx < p2->indx)
		return -1;

	if (p1->indx > p2->indx)
		return 1;

	if (p1->cx < p2->cx)
		return -1;

	if (p1->cx > p2->cx)
		return 1;

	if (p1->cy < p2->cy)
		return -1;

	if (p1->cy > p2->cy)
		return 1;

	if ( p1->file == p2->file )
		return 0;

	return ( p1->file > p2->file ) ? 1 : -1;
}

static LIST<IconSourceItem> iconSourceList( 20, sttCompareIconSourceItems );

static int sttCompareIcons( const IconItem* p1, const IconItem* p2 )
{	return strcmp( p1->name, p2->name );
}

static LIST<IconItem> iconList( 20, sttCompareIcons );

/////////////////////////////////////////////////////////////////////////////////////////
// Utility functions

static void __fastcall MySetCursor(TCHAR* nCursor)
{	SetCursor( LoadCursor( NULL, nCursor ));
}

static void __fastcall SAFE_FREE(void** p)
{
	if ( *p ) {
		mir_free( *p );
		*p = NULL;
}	}

static void __fastcall SafeDestroyIcon( HICON* icon )
{
	if ( *icon ) {
		DestroyIcon( *icon );
		*icon = NULL;
}	}


// Helper functions to manage Icon resources

IconSourceFile* IconSourceFile_Get( const TCHAR* file, bool isPath )
{
	TCHAR fileFull[ MAX_PATH ];

	if ( !file )
		return NULL;

    if (isPath)
		 pathToAbsoluteT( file, fileFull, NULL );
		 /// TODO: convert path to long - eliminate duplicate items
	 else
		 _tcscpy( fileFull, file );

	IconSourceFile key = { fileFull };
	int ix;
	if (( ix = iconSourceFileList.getIndex( &key )) != -1 ) {
		iconSourceFileList[ ix ]->ref_count++;
		return iconSourceFileList[ ix ];
	}

	IconSourceFile* newItem = (IconSourceFile*)mir_calloc( sizeof( IconSourceFile ));
	newItem->file = mir_tstrdup( fileFull );
	newItem->ref_count = 1;
	iconSourceFileList.insert( newItem );

	return newItem;
}

int IconSourceFile_Release( IconSourceFile** pitem )
{
	if ( pitem && *pitem && (*pitem)->ref_count ) {
		IconSourceFile* item = *pitem;
		if ( --item->ref_count <= 0 ) {
			int indx;
			if (( indx = iconSourceFileList.getIndex( item )) != -1 ) {
				SAFE_FREE(( void** )&item->file );
				iconSourceFileList.remove( indx );
				SAFE_FREE(( void** )&item );
			}
		}
		*pitem = NULL;
		return 0;
	}
	return 1;
}

static int BytesPerScanLine(int PixelsPerScanline, int BitsPerPixel, int Alignment)
{	Alignment--;
	int bytes = ((PixelsPerScanline * BitsPerPixel) + Alignment) & ~Alignment;
	return bytes / 8;
}

static int InitializeBitmapInfoHeader( HBITMAP bitmap, BITMAPINFOHEADER* bi )
{
	DIBSECTION DS;
	int bytes;

	DS.dsBmih.biSize = 0;
	bytes = GetObject( bitmap, sizeof(DS), &DS );
	if ( bytes == 0 ) return 1; // Failure
	else if (( bytes >= (sizeof(DS.dsBm) + sizeof(DS.dsBmih))) &&
		(DS.dsBmih.biSize >= DWORD(sizeof(DS.dsBmih))))
		*bi = DS.dsBmih;
	else {
		memset(bi, 0, sizeof(BITMAPINFOHEADER));
		bi->biSize = sizeof(BITMAPINFOHEADER);
		bi->biWidth = DS.dsBm.bmWidth;
		bi->biHeight = DS.dsBm.bmHeight;
	}
	bi->biBitCount = DS.dsBm.bmBitsPixel * DS.dsBm.bmPlanes;
	bi->biPlanes = 1;
	if ( bi->biClrImportant > bi->biClrUsed )
		bi->biClrImportant = bi->biClrUsed;
	if ( !bi->biSizeImage )
		bi->biSizeImage = BytesPerScanLine( bi->biWidth, bi->biBitCount, 32 ) * abs( bi->biHeight );
	return 0; // Success
}

static int InternalGetDIBSizes( HBITMAP bitmap, int* InfoHeaderSize, int* ImageSize ) 
{
	BITMAPINFOHEADER bi;

	if ( InitializeBitmapInfoHeader( bitmap, &bi )) return 1; // Failure
	if ( bi.biBitCount > 8 ) {
		*InfoHeaderSize = sizeof(BITMAPINFOHEADER);
		if ((bi.biCompression & BI_BITFIELDS) != 0 )
			*InfoHeaderSize += 12;
	}
	else {
		if ( bi.biClrUsed == 0 )
			*InfoHeaderSize = sizeof(BITMAPINFOHEADER) +
				sizeof(RGBQUAD) * (int)(1 << bi.biBitCount);
		else
			*InfoHeaderSize = sizeof(BITMAPINFOHEADER) +
				sizeof(RGBQUAD) * bi.biClrUsed;
	}
	*ImageSize = bi.biSizeImage;
	return 0; // Success
}

static int InternalGetDIB( HBITMAP bitmap, HPALETTE palette, void* bitmapInfo, void* Bits )
{
	HPALETTE oldPal = 0;

	if ( InitializeBitmapInfoHeader( bitmap, (BITMAPINFOHEADER*)bitmapInfo )) return 1; // Failure

	HDC DC = CreateCompatibleDC(0);
	if ( palette ) {
		oldPal = SelectPalette( DC, palette, FALSE );
		RealizePalette( DC );
	}
	int result = GetDIBits( DC, bitmap, 0, ((BITMAPINFOHEADER*)bitmapInfo)->biHeight, Bits, (BITMAPINFO*)bitmapInfo, DIB_RGB_COLORS) == 0;

	if ( oldPal ) SelectPalette( DC, oldPal, FALSE );
	DeleteDC( DC );
	return result;
}

static int GetIconData( HICON icon, BYTE** data, int* size )
{
	ICONINFO iconInfo;
	int MonoInfoSize, ColorInfoSize;
	int MonoBitsSize, ColorBitsSize;

	if ( !data || !size ) return 1; // Failure

	if ( !GetIconInfo( icon, &iconInfo )) return 1; // Failure

	if ( InternalGetDIBSizes( iconInfo.hbmMask, &MonoInfoSize, &MonoBitsSize ) ||
		InternalGetDIBSizes( iconInfo.hbmColor, &ColorInfoSize, &ColorBitsSize )) {
		DeleteObject( iconInfo.hbmColor );
		DeleteObject( iconInfo.hbmMask );
		return 1; // Failure
	}
	void* MonoInfo = mir_alloc( MonoInfoSize );
	void* MonoBits = mir_alloc( MonoBitsSize );
	void* ColorInfo = mir_alloc( ColorInfoSize );
	void* ColorBits = mir_alloc( ColorBitsSize );

	if ( InternalGetDIB( iconInfo.hbmMask, 0, MonoInfo, MonoBits ) ||
		InternalGetDIB( iconInfo.hbmColor, 0, ColorInfo, ColorBits )) {
		SAFE_FREE( &MonoInfo );
		SAFE_FREE( &MonoBits );
		SAFE_FREE( &ColorInfo );
		SAFE_FREE( &ColorBits );
		DeleteObject( iconInfo.hbmColor );
		DeleteObject( iconInfo.hbmMask );
		return 1; // Failure
	}

	*size = ColorInfoSize + ColorBitsSize + MonoBitsSize;
	*data = (BYTE*)mir_alloc(*size);
	
	BYTE* buf = *data;
	((BITMAPINFOHEADER*)ColorInfo)->biHeight *= 2; // color height includes mono bits
	memcpy( buf, ColorInfo, ColorInfoSize );
	buf += ColorInfoSize;
	memcpy( buf, ColorBits, ColorBitsSize );
	buf += ColorBitsSize;
	memcpy( buf, MonoBits, MonoBitsSize );

	SAFE_FREE( &MonoInfo );
	SAFE_FREE( &MonoBits );
	SAFE_FREE( &ColorInfo );
	SAFE_FREE( &ColorBits );
	DeleteObject( iconInfo.hbmColor );
	DeleteObject( iconInfo.hbmMask );
	return 0; // Success
}

#define VER30           0x00030000

static HICON IconSourceItem_GetIcon( IconSourceItem* item )
{
	if ( item->icon ) {
		item->icon_ref_count++;
		return item->icon;
	}

	if ( item->icon_size ) {
		item->icon = CreateIconFromResourceEx( item->icon_data, item->icon_size, TRUE, VER30, item->cx, item->cy, LR_COLOR );
		if ( item->icon ) {
			item->icon_ref_count++;
			return item->icon;
		}
	}
	//SHOULD BE REPLACED WITH GOOD ENOUGH FUNCTION
	_ExtractIconEx( item->file->file, item->indx, item->cx, item->cy, &item->icon, LR_COLOR );

	if ( item->icon )
		item->icon_ref_count++;

	return item->icon;
}

static int IconSourceItem_ReleaseIcon( IconSourceItem* item )
{
	if ( item && item->icon_ref_count )
	{
		item->icon_ref_count--;
		if ( !item->icon_ref_count ) {
			if ( !item->icon_size )
				if ( GetIconData( item->icon, &item->icon_data, &item->icon_size ))
					item->icon_size = 0; // Failure
			SafeDestroyIcon( &item->icon );
		}
		return 0; // Success
	}
	return 1; // Failure
}

IconSourceItem* GetIconSourceItem( const TCHAR* file, int indx, int cxIcon, int cyIcon )
{
	if ( !file )
		return NULL;

	IconSourceFile* r_file = IconSourceFile_Get( file, true );
	IconSourceItem key = { r_file, indx, cxIcon, cyIcon };
	int ix;
	if (( ix = iconSourceList.getIndex( &key )) != -1 ) {
		IconSourceFile_Release( &r_file );
		iconSourceList[ ix ]->ref_count++;
		return iconSourceList[ ix ];
	}

	IconSourceItem* newItem = (IconSourceItem*)mir_calloc( sizeof( IconSourceItem ));
	newItem->file = r_file;
	newItem->indx = indx;
	newItem->ref_count = 1;
	newItem->cx = cxIcon;
	newItem->cy = cyIcon;
	iconSourceList.insert( newItem );

	return newItem;
}

IconSourceItem* GetIconSourceItemFromPath( const TCHAR* path, int cxIcon, int cyIcon )
{
	TCHAR *comma;
	TCHAR file[ MAX_PATH ];
	int n;

	if ( !path )
		return NULL;

	lstrcpyn( file, path, SIZEOF( file ));
	comma = _tcsrchr( file, ',' );
	if ( !comma )
		n = 0;
	else {
		n = _ttoi( comma+1 );
		*comma = 0;
	}
	return GetIconSourceItem( file, n, cxIcon, cyIcon );
}

IconSourceItem* CreateStaticIconSourceItem( int cxIcon, int cyIcon )
{
	TCHAR sourceName[ MAX_PATH ];
	IconSourceFile key = { sourceName };

	int i = 0;
	do { // find new unique name
		mir_sntprintf( sourceName, SIZEOF( sourceName ), _T("*StaticIcon_%d"), ++i);
	} while ( iconSourceFileList.getIndex( &key ) != -1 );

	IconSourceItem* newItem = (IconSourceItem*)mir_calloc( sizeof( IconSourceItem ));
	newItem->file = IconSourceFile_Get( sourceName, false );
	newItem->indx = 0;
	newItem->ref_count = 1;
	newItem->cx = cxIcon;
	newItem->cy = cyIcon;
	iconSourceList.insert( newItem );

	return newItem;
}

static int IconSourceItem_Release( IconSourceItem** pitem )
{
	if ( pitem && *pitem && (*pitem)->ref_count ) {
		IconSourceItem* item = *pitem;
		item->ref_count--;
		if ( !item->ref_count ) {
			int indx;
			if (( indx = iconSourceList.getIndex( item )) != -1 ) {
				IconSourceFile_Release( &item->file );
				SafeDestroyIcon( &item->icon );
				SAFE_FREE(( void** )&item->icon_data );
				iconSourceList.remove( indx );
				SAFE_FREE(( void** )&item );
			}
		}
		*pitem = NULL;
		return 0;
	}
	return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Service functions

static HICON ExtractIconFromPath( const TCHAR *path, int cxIcon, int cyIcon )
{
	TCHAR *comma;
	TCHAR file[ MAX_PATH ], fileFull[ MAX_PATH ];
	int n;
	HICON hIcon;

	if ( !path )
		return (HICON)NULL;

	lstrcpyn( file, path, SIZEOF( file ));
	comma = _tcsrchr( file, ',' );
	if ( !comma )
		n = 0;
	else {
		n = _ttoi( comma+1 );
		*comma = 0;
	}
	pathToAbsoluteT( file, fileFull, NULL );
	hIcon = NULL;

	//SHOULD BE REPLACED WITH GOOD ENOUGH FUNCTION
	_ExtractIconEx( fileFull, n, cxIcon, cyIcon, &hIcon, LR_COLOR );
	return hIcon;
}

static SectionItem* IcoLib_AddSection(TCHAR *sectionName, BOOL create_new)
{
	if ( !sectionName )
		return NULL;

	int indx;
	SectionItem key = { sectionName, 0 };
	if (( indx = sectionList.getIndex( &key )) != -1 )
		return sectionList[ indx ];

	if ( create_new ) {
		SectionItem* newItem = ( SectionItem* )mir_calloc( sizeof( SectionItem ));
		newItem->name = mir_tstrdup( sectionName );
		newItem->flags = 0;
		sectionList.insert( newItem );
		bNeedRebuild = TRUE;
		return newItem;
	}

	return NULL;
}

static void IcoLib_RemoveSection(SectionItem* section)
{
	if ( !section )
		return;

	int indx;

	if (( indx = sectionList.getIndex( section )) != -1 ) {
		sectionList.remove( indx );
		SAFE_FREE(( void** )&section->name);
		SAFE_FREE(( void** )&section );
		bNeedRebuild = TRUE;
	}
}

static IconItem* IcoLib_FindIcon(const char* pszIconName)
{
	int indx;
	IconItem key = { (char*)pszIconName };
	if (( indx = iconList.getIndex( &key )) != -1 )
		return iconList[ indx ];

	return NULL;
}

static IconItem* IcoLib_FindHIcon(HICON hIcon, bool &big)
{
	IconItem* item = NULL;
	int indx;

	for ( indx = 0; indx < iconList.getCount(); indx++ ) {
		if ( iconList[ indx ]->source_small  && iconList[ indx ]->source_small->icon == hIcon) {
			item = iconList[ indx ];
			big = false;
			break;
		}	
		else if ( iconList[ indx ]->source_big && iconList[ indx ]->source_big->icon == hIcon) {
			item = iconList[ indx ];
			big = true;
			break;
		}	
	}

	return item;
}

static void IcoLib_FreeIcon(IconItem* icon)
{
	if ( !icon) return;

	SAFE_FREE(( void** )&icon->name );
	SAFE_FREE(( void** )&icon->description );
	SAFE_FREE(( void** )&icon->default_file );
	SAFE_FREE(( void** )&icon->temp_file );
	if ( icon->section) {
		if ( !--icon->section->ref_count)
			IcoLib_RemoveSection( icon->section );
		icon->section = NULL;
	}
	IconSourceItem_Release( &icon->source_small );
	IconSourceItem_Release( &icon->source_big );
	IconSourceItem_Release( &icon->default_icon );
	SafeDestroyIcon( &icon->temp_icon );
}

/////////////////////////////////////////////////////////////////////////////////////////
// IcoLib_AddNewIcon

HANDLE IcoLib_AddNewIcon( SKINICONDESC* sid )
{
	int utf = 0, utf_path = 0;
	IconItem* item;

	if ( !sid->cbSize )
		return NULL;

	if ( sid->cbSize < SKINICONDESC_SIZE_V1 )
		return NULL;

	if ( sid->cbSize >= SKINICONDESC_SIZE ) {
		utf = sid->flags & SIDF_UNICODE ? 1 : 0;
		utf_path = sid->flags & SIDF_PATH_UNICODE ? 1 : 0;
	}

	EnterCriticalSection( &csIconList );

	item = IcoLib_FindIcon( sid->pszName );
	if ( !item ) {
		item = ( IconItem* )mir_alloc( sizeof( IconItem ));
		item->name = sid->pszName;
		iconList.insert( item );
	}
	else IcoLib_FreeIcon( item );

	ZeroMemory( item, sizeof( *item ));
	item->name = mir_strdup( sid->pszName );
	if ( utf ) {
		item->description = mir_u2t( sid->pwszDescription );
		#ifdef _UNICODE
			item->section = IcoLib_AddSection( sid->pwszSection, TRUE );
		#else
			char* pszSection = sid->pwszSection ? mir_u2a( sid->pwszSection ) : NULL;
			item->section = IcoLib_AddSection( pszSection, TRUE );
			SAFE_FREE(( void** )&pszSection );
		#endif
	}
	else {
		item->description = mir_a2t( sid->pszDescription );
		#ifdef _UNICODE
			WCHAR* pwszSection = sid->pszSection ? mir_a2u( sid->pszSection ) : NULL;

			item->section = IcoLib_AddSection( pwszSection, TRUE );
			SAFE_FREE(( void** )&pwszSection );
		#else
			item->section = IcoLib_AddSection( sid->pszSection, TRUE );
		#endif
	}
	if ( item->section ) {
		item->section->ref_count++;
		item->orderID = ++item->section->maxOrder;
	}
	else
		item->orderID = 0;

	if ( sid->pszDefaultFile ) {
		if ( utf_path ) {
			#ifdef _UNICODE
				WCHAR fileFull[ MAX_PATH ];

				pathToAbsoluteW( sid->pwszDefaultFile, fileFull, NULL );
				item->default_file = mir_wstrdup( fileFull );
			#else
				char* file = mir_u2a( sid->pwszDefaultFile );
				char fileFull[ MAX_PATH ];

				pathToAbsolute( file, fileFull, NULL );
				SAFE_FREE(( void** )&file );
				item->default_file = mir_strdup( fileFull );
			#endif
		}
		else {
			#ifdef _UNICODE
				WCHAR *file = mir_a2u( sid->pszDefaultFile );
				WCHAR fileFull[ MAX_PATH ];

				pathToAbsoluteW( file, fileFull, NULL );
				SAFE_FREE(( void** )&file );
				item->default_file = mir_wstrdup( fileFull );
			#else
				char fileFull[ MAX_PATH ];

				pathToAbsolute( sid->pszDefaultFile, fileFull, NULL );
				item->default_file = mir_strdup( fileFull );
			#endif
	}	}
	item->default_indx = sid->iDefaultIndex;

	if ( sid->cbSize >= SKINICONDESC_SIZE_V3 ) {
		item->cx = sid->cx;
		item->cy = sid->cy;
	}

	if ( sid->cbSize >= SKINICONDESC_SIZE_V2 && sid->hDefaultIcon ) {
		bool big;
		IconItem* def_item = IcoLib_FindHIcon( sid->hDefaultIcon, big );
		if ( def_item ) {
			item->default_icon = big ? def_item->source_big : def_item->source_small;
			item->default_icon->ref_count++;
		}
		else {
			int cx = item->cx ? item->cx : GetSystemMetrics(SM_CXSMICON);
			int cy = item->cy ? item->cy : GetSystemMetrics(SM_CYSMICON);
			item->default_icon = CreateStaticIconSourceItem( cx, cy );
			if ( GetIconData( sid->hDefaultIcon, &item->default_icon->icon_data, &item->default_icon->icon_size )) {
				IconSourceItem_Release( &item->default_icon );
			}
		}
	}

	if ( sid->cbSize >= SKINICONDESC_SIZE && item->section )
		item->section->flags = sid->flags & SIDF_SORTED;

	LeaveCriticalSection( &csIconList );
	return item;
}

/////////////////////////////////////////////////////////////////////////////////////////
// IcoLib_RemoveIcon

static INT_PTR IcoLib_RemoveIcon( WPARAM, LPARAM lParam )
{
	if ( lParam ) {
		int indx;
		EnterCriticalSection( &csIconList );

		if (( indx = iconList.getIndex(( IconItem* )&lParam )) != -1 ) {
			IconItem *item = iconList[ indx ];
			IcoLib_FreeIcon( item );
			iconList.remove( indx );
			SAFE_FREE(( void** )&item );
		}

		LeaveCriticalSection( &csIconList );
		return ( indx == -1 ) ? 1 : 0;
	}
	return 1; // Failed
}

HICON IconItem_GetDefaultIcon( IconItem* item, bool big )
{
	HICON hIcon = NULL;

	if ( item->default_icon && !big ) {
		IconSourceItem_Release( &item->source_small );
		item->source_small = item->default_icon;
		item->source_small->ref_count++;
		hIcon = IconSourceItem_GetIcon( item->source_small );
	}

	if ( !hIcon && item->default_file ) {
		int cx = item->cx ? item->cx : GetSystemMetrics(big ? SM_CXICON : SM_CXSMICON);
		int cy = item->cy ? item->cy : GetSystemMetrics(big ? SM_CYICON : SM_CYSMICON);
		IconSourceItem* def_icon = GetIconSourceItem( item->default_file, item->default_indx, cx, cy );
		if ( big ) {
			if ( def_icon != item->source_big ) {
				IconSourceItem_Release( &item->source_big );
				item->source_big = def_icon;
				if ( def_icon ) {
					def_icon->ref_count++;
					hIcon = IconSourceItem_GetIcon( def_icon );
				}
			}
			else
				IconSourceItem_Release( &def_icon );
		}
		else {
			if ( def_icon != item->default_icon ) {
				IconSourceItem_Release( &item->default_icon );
				item->default_icon = def_icon;
				if ( def_icon ) {
					IconSourceItem_Release( &item->source_small );
					item->source_small = def_icon;
					def_icon->ref_count++;
					hIcon = IconSourceItem_GetIcon( def_icon );
				}
			}
			else
				IconSourceItem_Release( &def_icon );
		}
	}
	return hIcon;
}


/////////////////////////////////////////////////////////////////////////////////////////
// IconItem_GetIcon

HICON IconItem_GetIcon( IconItem* item, bool big )
{
	DBVARIANT dbv = {0};
	HICON hIcon = NULL;

	big = big && !item->cx;
	IconSourceItem* &source = big ? item->source_big : item->source_small;

	if ( !source && !DBGetContactSettingTString( NULL, "SkinIcons", item->name, &dbv )) {
		int cx = item->cx ? item->cx : GetSystemMetrics(big ? SM_CXICON : SM_CXSMICON);
		int cy = item->cy ? item->cy : GetSystemMetrics(big ? SM_CYICON : SM_CYSMICON);
		source = GetIconSourceItemFromPath( dbv.ptszVal, cx, cy );
		DBFreeVariant( &dbv );
	}

	if ( source )
		hIcon = IconSourceItem_GetIcon( source );

	if ( !hIcon )
		hIcon = IconItem_GetDefaultIcon( item, big );

	if ( !hIcon )
		return hIconBlank;

	return hIcon;
}

/////////////////////////////////////////////////////////////////////////////////////////
// IconItem_GetIcon_Preview

HICON IconItem_GetIcon_Preview( IconItem* item )
{
	HICON hIcon = NULL;

	if ( !item->temp_reset ) {
		HICON hRefIcon = IconItem_GetIcon( item, false );
		hIcon = CopyIcon( hRefIcon );
		if ( item->source_small && item->source_small->icon == hRefIcon )
			IconSourceItem_ReleaseIcon( item->source_small );
	}
	else {
		if ( item->default_icon ) {
			HICON hRefIcon = IconSourceItem_GetIcon( item->default_icon );
			if ( hRefIcon ) {
				hIcon = CopyIcon( hRefIcon );
				if ( item->default_icon->icon == hRefIcon )
					IconSourceItem_ReleaseIcon( item->default_icon );
		}	}

		if ( !hIcon && item->default_file ) {
			IconSourceItem_Release( &item->default_icon );
			item->default_icon = GetIconSourceItem( item->default_file, item->default_indx, item->cx, item->cy ); 
			if ( item->default_icon ) {
				HICON hRefIcon = IconSourceItem_GetIcon( item->default_icon );
				if ( hRefIcon ) {
					hIcon = CopyIcon( hRefIcon );
					if ( item->default_icon->icon == hRefIcon )
						IconSourceItem_ReleaseIcon( item->default_icon );
		}	}	}

		if ( !hIcon )
			return CopyIcon(hIconBlank);
	}
	return hIcon;
}


/////////////////////////////////////////////////////////////////////////////////////////
// IcoLib_GetIcon
// lParam: pszIconName
// wParam: PLOADIMAGEPARAM or NULL.
// if wParam == NULL, default is used:
//     uType = IMAGE_ICON
//     cx/cyDesired = GetSystemMetrics(SM_CX/CYSMICON)
//     fuLoad = 0

HICON IcoLib_GetIcon( const char* pszIconName, bool big )
{
	IconItem* item;
	HICON result = NULL;

	if ( !pszIconName )
		return hIconBlank;

	EnterCriticalSection( &csIconList );

	item = IcoLib_FindIcon( pszIconName );
	if ( item ) {
		result = IconItem_GetIcon( item, big );
	}
	LeaveCriticalSection( &csIconList );
	return result;
}

/////////////////////////////////////////////////////////////////////////////////////////
// IcoLib_GetIconHandle
// lParam: pszIconName

HANDLE IcoLib_GetIconHandle( const char* pszIconName )
{
	IconItem* item;

	if ( !pszIconName )
		return NULL;

	EnterCriticalSection( &csIconList );
	item = IcoLib_FindIcon( pszIconName );
	LeaveCriticalSection( &csIconList );

	return (HANDLE)item;
}

/////////////////////////////////////////////////////////////////////////////////////////
// IcoLib_GetIconByHandle
// lParam: icolib item handle
// wParam: 0

HICON IcoLib_GetIconByHandle( HANDLE hItem, bool big )
{
	if ( hItem == NULL )
		return NULL;

	HICON result = hIconBlank;
	IconItem* pi = ( IconItem* )hItem;

	EnterCriticalSection( &csIconList );

	// we can get any junk here... but getIndex() is MUCH faster than indexOf().
	__try
	{
		if ( iconList.getIndex( pi ) != -1 )
			result = IconItem_GetIcon( pi, big );
	}
	__except( EXCEPTION_EXECUTE_HANDLER )
	{
	}

	LeaveCriticalSection( &csIconList );
	return result;
}

/////////////////////////////////////////////////////////////////////////////////////////
// IcoLib_IsManaged
// lParam: NULL
// wParam: HICON

HANDLE IcoLib_IsManaged( HICON hIcon )
{
	IconItem* item;

	EnterCriticalSection( &csIconList );

	bool big;
	item = IcoLib_FindHIcon( hIcon, big );
	if ( item ) {
		IconSourceItem* source = big && !item->cx ? item->source_big : item->source_small;
		if ( source->icon_ref_count == 0 )
			item = NULL;
	}

	LeaveCriticalSection( &csIconList );
	return NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////
// IcoLib_AddRef
// lParam: NULL
// wParam: HICON

static INT_PTR IcoLib_AddRef( WPARAM wParam, LPARAM )
{
	EnterCriticalSection( &csIconList );

	bool big;
	IconItem *item = IcoLib_FindHIcon(( HICON )wParam, big);

	INT_PTR res = 1;
	if ( item ) {
		IconSourceItem* source = big && !item->cx ? item->source_big : item->source_small;
		if ( source->icon_ref_count ) {
			source->icon_ref_count++;
			res = 0;
		}
	}

	LeaveCriticalSection( &csIconList );
	return res;
}

/////////////////////////////////////////////////////////////////////////////////////////
// IcoLib_ReleaseIcon
// lParam: pszIconName or NULL
// wParam: HICON or NULL

int IcoLib_ReleaseIcon( HICON hIcon, char* szIconName, bool big )
{
	IconItem *item = NULL;

	EnterCriticalSection(&csIconList);

	if ( szIconName )
		item = IcoLib_FindIcon( szIconName );

	if ( !item && hIcon ) // find by HICON
		item = IcoLib_FindHIcon( hIcon, big );

	int res = 1;
	if ( item ) {
		IconSourceItem* source = big && !item->cx ? item->source_big : item->source_small;
		if ( source && source->icon_ref_count ) {
			if ( iconEventActive )
				source->icon_ref_count--;
			else
				IconSourceItem_ReleaseIcon( source );
			res = 0;
		}
	}

	LeaveCriticalSection( &csIconList );
	return res;
}

/////////////////////////////////////////////////////////////////////////////////////////
// IcoLib GUI service routines

static void LoadSectionIcons(TCHAR *filename, SectionItem* sectionActive)
{
	TCHAR path[ MAX_PATH ];
	int suffIndx;
	HICON hIcon;
	int indx;

	mir_sntprintf( path, SIZEOF(path), _T("%s,"), filename );
	suffIndx = lstrlen( path );

	EnterCriticalSection( &csIconList );

	for ( indx = 0; indx < iconList.getCount(); indx++ ) {
		IconItem *item = iconList[ indx ];

		if ( item->default_file && item->section == sectionActive ) {
			_itot( item->default_indx, path + suffIndx, 10 );
			hIcon = ExtractIconFromPath( path, item->cx, item->cy );
			if ( hIcon ) {
				SAFE_FREE(( void** )&item->temp_file );
				SafeDestroyIcon( &item->temp_icon );

				item->temp_file = mir_tstrdup( path );
				item->temp_icon = hIcon;
				item->temp_reset = FALSE;
	}	}	}

	LeaveCriticalSection( &csIconList );
}

void LoadSubIcons(HWND htv, TCHAR *filename, HTREEITEM hItem)
{
	TVITEM tvi;
	TreeItem *treeItem;
	SectionItem* sectionActive;

	tvi.mask = TVIF_HANDLE | TVIF_PARAM;
	tvi.hItem = hItem;

	TreeView_GetItem( htv, &tvi );
	treeItem = (TreeItem *)tvi.lParam;
	sectionActive = sectionList[ SECTIONPARAM_INDEX(treeItem->value) ];

	tvi.hItem = TreeView_GetChild( htv, tvi.hItem );
	while ( tvi.hItem ) {
		LoadSubIcons( htv, filename, tvi.hItem );
		tvi.hItem = TreeView_GetNextSibling( htv, tvi.hItem );
	}

	if ( SECTIONPARAM_FLAGS(treeItem->value) & SECTIONPARAM_HAVEPAGE )
		LoadSectionIcons( filename, sectionActive );
}

static void UndoChanges( int iconIndx, int cmd )
{
	IconItem *item = iconList[ iconIndx ];

	if ( !item->temp_file && !item->temp_icon && item->temp_reset && cmd == ID_CANCELCHANGE )
		item->temp_reset = FALSE;
	else
	{
		SAFE_FREE(( void** )&item->temp_file );
		SafeDestroyIcon( &item->temp_icon );
	}

	if ( cmd == ID_RESET )
		item->temp_reset = TRUE;
}

void UndoSubItemChanges( HWND htv, HTREEITEM hItem, int cmd )
{
	TVITEM tvi = {0};
	TreeItem *treeItem;
	int indx;

	tvi.mask = TVIF_HANDLE | TVIF_PARAM;
	tvi.hItem = hItem;
	TreeView_GetItem( htv, &tvi );
	treeItem = (TreeItem *)tvi.lParam;

	if ( SECTIONPARAM_FLAGS( treeItem->value ) & SECTIONPARAM_HAVEPAGE ) {
		EnterCriticalSection( &csIconList );

		for ( indx = 0; indx < iconList.getCount(); indx++ )
			if ( iconList[ indx ]->section == sectionList[ SECTIONPARAM_INDEX(treeItem->value) ])
				UndoChanges( indx, cmd );

		LeaveCriticalSection( &csIconList );
	}

	tvi.hItem = TreeView_GetChild( htv, tvi.hItem );
	while ( tvi.hItem ) {
		UndoSubItemChanges( htv, tvi.hItem, cmd );
		tvi.hItem = TreeView_GetNextSibling( htv, tvi.hItem );
}	}

static void OpenIconsPage()
{
	CallService( MS_UTILS_OPENURL, 1, (LPARAM)"http://addons.miranda-im.org/index.php?action=display&id=35" );
}

static int OpenPopupMenu(HWND hwndDlg)
{
	HMENU hMenu, hPopup;
	POINT pt;
	int cmd;

	GetCursorPos(&pt);
	hMenu = LoadMenu( hMirandaInst, MAKEINTRESOURCE( IDR_ICOLIB_CONTEXT ));
	hPopup = GetSubMenu( hMenu, 0 );
	CallService( MS_LANGPACK_TRANSLATEMENU, ( WPARAM )hPopup, 0 );
	cmd = TrackPopupMenu( hPopup, TPM_RIGHTBUTTON|TPM_RETURNCMD, pt.x,  pt.y, 0, hwndDlg, NULL );
	DestroyMenu( hMenu );
	return cmd;
}

static TCHAR* OpenFileDlg( HWND hParent, const TCHAR* szFile, BOOL bAll )
{
	OPENFILENAME ofn = {0};
	TCHAR filter[512],*pfilter,file[MAX_PATH*2];

	ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
	ofn.hwndOwner = hParent;

	lstrcpy(filter,TranslateT("Icon Sets"));
	if (bAll)
		lstrcat(filter,_T(" (*.dll;*.icl;*.exe;*.ico)"));
	else
		lstrcat(filter,_T(" (*.dll)"));

	pfilter=filter+lstrlen(filter)+1;
	if (bAll)
		lstrcpy(pfilter,_T("*.DLL;*.ICL;*.EXE;*.ICO"));
	else
		lstrcpy(pfilter,_T("*.DLL"));

	pfilter += lstrlen(pfilter) + 1;
	lstrcpy(pfilter, TranslateT("All Files"));
	lstrcat(pfilter,_T(" (*)"));
	pfilter += lstrlen(pfilter) + 1;
	lstrcpy(pfilter,_T("*"));
	pfilter += lstrlen(pfilter) + 1;
	*pfilter='\0';

	ofn.lpstrFilter = filter;
	ofn.lpstrDefExt = _T("dll");
	lstrcpyn(file, szFile, SIZEOF(file));
	ofn.lpstrFile = file;
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_DONTADDTORECENT;
	ofn.nMaxFile = MAX_PATH*2;

	if (!GetOpenFileName(&ofn))
		return NULL;

	return mir_tstrdup(file);
}

//
//  User interface
//

#define DM_REBUILDICONSPREVIEW  (WM_USER+10)
#define DM_CHANGEICON           (WM_USER+11)
#define DM_CHANGESPECIFICICON   (WM_USER+12)
#define DM_UPDATEICONSPREVIEW   (WM_USER+13)
#define DM_REBUILD_CTREE        (WM_USER+14)

INT_PTR CALLBACK DlgProcIconImport(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

void DoOptionsChanged(HWND hwndDlg)
{
	SendMessage(hwndDlg, DM_UPDATEICONSPREVIEW, 0, 0);
	SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
}

void DoIconsChanged(HWND hwndDlg)
{
	SendMessage(hwndDlg, DM_UPDATEICONSPREVIEW, 0, 0);

	iconEventActive = 1; // Disable icon destroying - performance boost
	NotifyEventHooks(hIconsChangedEvent, 0, 0);
	NotifyEventHooks(hIcons2ChangedEvent, 0, 0);
	iconEventActive = 0;

	EnterCriticalSection(&csIconList); // Destroy unused icons
	for (int indx = 0; indx < iconList.getCount(); indx++) {
		IconItem *item = iconList[indx];
		if ( item->source_small && !item->source_small->icon_ref_count) {
			item->source_small->icon_ref_count++;
			IconSourceItem_ReleaseIcon( item->source_small );
		}
		if ( item->source_big && !item->source_big->icon_ref_count) {
			item->source_big->icon_ref_count++;
			IconSourceItem_ReleaseIcon( item->source_big );
		}
	}
	LeaveCriticalSection(&csIconList);
}

static HTREEITEM FindNamedTreeItemAt(HWND hwndTree, HTREEITEM hItem, const TCHAR *name)
{
	TVITEM tvi = {0};
	TCHAR str[MAX_PATH];

	if (hItem)
		tvi.hItem = TreeView_GetChild(hwndTree, hItem);
	else
		tvi.hItem = TreeView_GetRoot(hwndTree);

	if (!name)
		return tvi.hItem;

	tvi.mask = TVIF_TEXT;
	tvi.pszText = str;
	tvi.cchTextMax = MAX_PATH;

	while (tvi.hItem)
	{
		TreeView_GetItem(hwndTree, &tvi);

		if (!lstrcmp(tvi.pszText, name))
			return tvi.hItem;

		tvi.hItem = TreeView_GetNextSibling(hwndTree, tvi.hItem);
	}
	return NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////
// icon import dialog's window procedure

static int IconDlg_Resize(HWND, LPARAM, UTILRESIZECONTROL *urc)
{
	switch (urc->wId) {
	case IDC_ICONSET:
		return RD_ANCHORX_WIDTH | RD_ANCHORY_TOP;

	case IDC_BROWSE:
		return RD_ANCHORX_RIGHT | RD_ANCHORY_TOP;

	case IDC_PREVIEW:
		return RD_ANCHORX_WIDTH | RD_ANCHORY_HEIGHT;

	case IDC_GETMORE:
		return RD_ANCHORX_CENTRE | RD_ANCHORY_BOTTOM;
	}
	return RD_ANCHORX_LEFT | RD_ANCHORY_TOP; // default
}

INT_PTR CALLBACK DlgProcIconImport(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HWND hwndParent,hwndDragOver;
	static int dragging;
	static int dragItem,dropHiLite;
	static HWND hPreview = NULL;

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		hwndParent = (HWND)lParam;
		hPreview = GetDlgItem(hwndDlg, IDC_PREVIEW);
		dragging = dragItem = 0;
		ListView_SetImageList(hPreview, ImageList_Create(GetSystemMetrics(SM_CXSMICON),GetSystemMetrics(SM_CYSMICON),ILC_COLOR32|ILC_MASK,0,100), LVSIL_NORMAL);
		ListView_SetIconSpacing(hPreview, 56, 67);
		{
			RECT rcThis, rcParent;
			int cxScreen = GetSystemMetrics(SM_CXSCREEN);

			GetWindowRect(hwndDlg,&rcThis);
			GetWindowRect(hwndParent,&rcParent);
			OffsetRect(&rcThis,rcParent.right-rcThis.left,0);
			OffsetRect(&rcThis,0,rcParent.top-rcThis.top);
			GetWindowRect(GetParent(hwndParent),&rcParent);
			if (rcThis.right > cxScreen) {
				OffsetRect(&rcParent,cxScreen-rcThis.right,0);
				OffsetRect(&rcThis,cxScreen-rcThis.right,0);
				MoveWindow(GetParent(hwndParent),rcParent.left,rcParent.top,rcParent.right-rcParent.left,rcParent.bottom-rcParent.top,TRUE);
			}
			MoveWindow(hwndDlg,rcThis.left,rcThis.top,rcThis.right-rcThis.left,rcThis.bottom-rcThis.top,FALSE);
			GetClientRect(hwndDlg, &rcThis);
			SendMessage(hwndDlg, WM_SIZE, 0, MAKELPARAM(rcThis.right-rcThis.left, rcThis.bottom-rcThis.top));
		}

        if (shAutoComplete) shAutoComplete(GetDlgItem(hwndDlg,IDC_ICONSET), 1);
		SetDlgItemText(hwndDlg,IDC_ICONSET,_T("icons.dll"));
		return TRUE;

	case DM_REBUILDICONSPREVIEW:
		{
			LVITEM lvi;
			TCHAR filename[MAX_PATH],caption[64];
			HIMAGELIST hIml;
			int count,i;
			HICON hIcon;

			MySetCursor(IDC_WAIT);
			ListView_DeleteAllItems(hPreview);
			hIml = ListView_GetImageList(hPreview, LVSIL_NORMAL);
			ImageList_RemoveAll(hIml);
			GetDlgItemText(hwndDlg, IDC_ICONSET, filename, SIZEOF(filename));
			{
				RECT rcPreview,rcGroup;

				GetWindowRect(hPreview, &rcPreview);
				GetWindowRect(GetDlgItem(hwndDlg,IDC_IMPORTMULTI),&rcGroup);
				//SetWindowPos(hPreview,0,0,0,rcPreview.right-rcPreview.left,rcGroup.bottom-rcPreview.top,SWP_NOZORDER|SWP_NOMOVE);
			}

			if (_taccess(filename,0) != 0) {
				MySetCursor(IDC_ARROW);
				break;
			}

			lvi.mask = LVIF_TEXT|LVIF_IMAGE|LVIF_PARAM;
			lvi.iSubItem = 0;
			lvi.iItem = 0;
			count = (int)_ExtractIconEx( filename, -1, 16,16, NULL, LR_DEFAULTCOLOR );
			for (i = 0; i < count; lvi.iItem++, i++) {
				mir_sntprintf(caption, SIZEOF(caption), _T("%d"), i+1);
				lvi.pszText = caption;
				//hIcon = ExtractIcon(hMirandaInst, filename, i);
				_ExtractIconEx( filename, i, 16,16, &hIcon, LR_DEFAULTCOLOR );
				lvi.iImage = ImageList_AddIcon(hIml, hIcon);
				DestroyIcon(hIcon);
				lvi.lParam = i;
				ListView_InsertItem(hPreview, &lvi);
			}
			MySetCursor(IDC_ARROW);
		}
		break;

	case WM_COMMAND:
		switch( LOWORD( wParam )) {
		case IDC_BROWSE:
			{
				TCHAR str[MAX_PATH];
				TCHAR *file;

				GetDlgItemText(hwndDlg,IDC_ICONSET,str,SIZEOF(str));
				if (!(file = OpenFileDlg(GetParent(hwndDlg), str, TRUE))) break;
				SetDlgItemText(hwndDlg,IDC_ICONSET,file);
				SAFE_FREE(( void** )&file );
			}
			break;

		case IDC_GETMORE:
			OpenIconsPage();
			break;

		case IDC_ICONSET:
			if (HIWORD(wParam) == EN_CHANGE)
				SendMessage(hwndDlg, DM_REBUILDICONSPREVIEW, 0, 0);
			break;
		}
		break;

	case WM_MOUSEMOVE:
		if (dragging) {
			LVHITTESTINFO lvhti;
			int onItem=0;
			HWND hwndOver;
			RECT rc;
			POINT ptDrag;
			HWND hPPreview = GetDlgItem(hwndParent, IDC_PREVIEW);

			lvhti.pt.x = (short)LOWORD(lParam); lvhti.pt.y = (short)HIWORD(lParam);
			ClientToScreen(hwndDlg, &lvhti.pt);
			hwndOver = WindowFromPoint(lvhti.pt);
			GetWindowRect(hwndOver, &rc);
			ptDrag.x = lvhti.pt.x - rc.left; ptDrag.y = lvhti.pt.y - rc.top;
			if (hwndOver != hwndDragOver) {
				ImageList_DragLeave(hwndDragOver);
				hwndDragOver = hwndOver;
				ImageList_DragEnter(hwndDragOver, ptDrag.x, ptDrag.y);
			}

			ImageList_DragMove(ptDrag.x, ptDrag.y);
			if (hwndOver == hPPreview) {
				ScreenToClient(hPPreview, &lvhti.pt);

				if (ListView_HitTest(hPPreview, &lvhti) != -1) {
					if (lvhti.iItem != dropHiLite) {
						ImageList_DragLeave(hwndDragOver);
						if (dropHiLite != -1)
							ListView_SetItemState(hPPreview, dropHiLite, 0, LVIS_DROPHILITED);
						dropHiLite = lvhti.iItem;
						ListView_SetItemState(hPPreview, dropHiLite, LVIS_DROPHILITED, LVIS_DROPHILITED);
						UpdateWindow(hPPreview);
						ImageList_DragEnter(hwndDragOver, ptDrag.x, ptDrag.y);
					}
					onItem = 1;
			}	}

			if (!onItem && dropHiLite != -1) {
				ImageList_DragLeave(hwndDragOver);
				ListView_SetItemState(hPPreview, dropHiLite, 0, LVIS_DROPHILITED);
				UpdateWindow(hPPreview);
				ImageList_DragEnter(hwndDragOver, ptDrag.x, ptDrag.y);
				dropHiLite = -1;
			}
			MySetCursor(onItem ? IDC_ARROW : IDC_NO);
		}
		break;

	case WM_LBUTTONUP:
		if (dragging) {
			ReleaseCapture();
			ImageList_EndDrag();
			dragging = 0;
			if (dropHiLite != -1) {
				TCHAR path[MAX_PATH],fullPath[MAX_PATH],filename[MAX_PATH];
				LVITEM lvi;

				GetDlgItemText(hwndDlg, IDC_ICONSET, fullPath, SIZEOF(fullPath));
				CallService(MS_UTILS_PATHTORELATIVET, (WPARAM)fullPath, (LPARAM)filename);
				lvi.mask=LVIF_PARAM;
				lvi.iItem = dragItem; lvi.iSubItem = 0;
				ListView_GetItem(hPreview, &lvi);
				mir_sntprintf(path, MAX_PATH, _T("%s,%d"), filename, (int)lvi.lParam);
				SendMessage(hwndParent, DM_CHANGEICON, dropHiLite, (LPARAM)path);
				ListView_SetItemState(GetDlgItem(hwndParent, IDC_PREVIEW), dropHiLite, 0, LVIS_DROPHILITED);
		}	}
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case IDC_PREVIEW:
			switch (((LPNMHDR)lParam)->code) {
			case LVN_BEGINDRAG:
				SetCapture(hwndDlg);
				dragging = 1;
				dragItem = ((LPNMLISTVIEW)lParam)->iItem;
				dropHiLite = -1;
				ImageList_BeginDrag(ListView_GetImageList(hPreview, LVSIL_NORMAL), dragItem, GetSystemMetrics(SM_CXICON)/2, GetSystemMetrics(SM_CYICON)/2);
				{
					POINT pt;
					RECT rc;

					GetCursorPos(&pt);
					GetWindowRect(hwndDlg, &rc);
					ImageList_DragEnter(hwndDlg, pt.x - rc.left, pt.y - rc.top);
					hwndDragOver = hwndDlg;
				}
				break;
			}
			break;
		}
		break;

	case WM_CLOSE:
		DestroyWindow(hwndDlg);
		EnableWindow(GetDlgItem(hwndParent,IDC_IMPORT),TRUE);
		break;

	case WM_SIZE:
		{ // make the dlg resizeable
			UTILRESIZEDIALOG urd = {0};

			if (IsIconic(hwndDlg)) break;
			urd.cbSize = sizeof(urd);
			urd.hInstance = hMirandaInst;
			urd.hwndDlg = hwndDlg;
			urd.lParam = 0; // user-defined
			urd.lpTemplate = MAKEINTRESOURCEA(IDD_ICOLIB_IMPORT);
			urd.pfnResizer = IconDlg_Resize;
			CallService(MS_UTILS_RESIZEDIALOG, 0, (LPARAM)&urd);
			break;
	}	}

	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// IcoLib options window procedure

static int CALLBACK DoSortIconsFunc(LPARAM lParam1, LPARAM lParam2, LPARAM )
{	return lstrcmpi(TranslateTS(iconList[lParam1]->description), TranslateTS(iconList[lParam2]->description));
}

static int CALLBACK DoSortIconsFuncByOrder(LPARAM lParam1, LPARAM lParam2, LPARAM )
{	return iconList[lParam1]->orderID - iconList[lParam2]->orderID;
}

static void SaveCollapseState( HWND hwndTree )
{
	HTREEITEM hti;
	TVITEM tvi;

	hti = TreeView_GetRoot( hwndTree );
	while( hti != NULL ) {
		HTREEITEM ht;
		TreeItem *treeItem;

		tvi.mask = TVIF_STATE | TVIF_HANDLE | TVIF_CHILDREN | TVIF_PARAM;
		tvi.hItem = hti;
		tvi.stateMask = (DWORD)-1;
		TreeView_GetItem( hwndTree, &tvi );

		if( tvi.cChildren > 0 ) {
			treeItem = (TreeItem *)tvi.lParam;
			if ( tvi.state & TVIS_EXPANDED )
				DBWriteContactSettingByte(NULL, "SkinIconsUI", treeItem->paramName, TVIS_EXPANDED );
			else
				DBWriteContactSettingByte(NULL, "SkinIconsUI", treeItem->paramName, 0 );
		}

		ht = TreeView_GetChild( hwndTree, hti );
		if( ht == NULL ) {
			ht = TreeView_GetNextSibling( hwndTree, hti );
			while( ht == NULL ) {
				hti = TreeView_GetParent( hwndTree, hti );
				if( hti == NULL ) break;
				ht = TreeView_GetNextSibling( hwndTree, hti );
		}	}

		hti = ht;
}	}

INT_PTR CALLBACK DlgProcIcoLibOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	struct IcoLibOptsData *dat;
	static HTREEITEM prevItem = 0;
	static HWND hPreview = NULL;

	dat = (struct IcoLibOptsData*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		hPreview = GetDlgItem(hwndDlg, IDC_PREVIEW);
		dat = (struct IcoLibOptsData*)mir_alloc(sizeof(struct IcoLibOptsData));
		dat->hwndIndex = NULL;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)dat);
		//
		//  Reset temporary data & upload sections list
		//
		EnterCriticalSection(&csIconList);
		{
			int indx;
			for (indx = 0; indx < iconList.getCount(); indx++) {
				iconList[indx]->temp_file = NULL;
				iconList[indx]->temp_icon = NULL;
				iconList[indx]->temp_reset = FALSE;
			}
			bNeedRebuild = FALSE;
		}
		LeaveCriticalSection(&csIconList);
		//
		//  Setup preview listview
		//
		ListView_SetUnicodeFormat(hPreview, TRUE);
		ListView_SetExtendedListViewStyleEx(hPreview, LVS_EX_INFOTIP, LVS_EX_INFOTIP);
		ListView_SetImageList(hPreview, ImageList_Create(GetSystemMetrics(SM_CXSMICON),GetSystemMetrics(SM_CYSMICON),ILC_COLOR32|ILC_MASK,0,30), LVSIL_NORMAL);
		ListView_SetIconSpacing(hPreview, 56, 67);

		SendMessage(hwndDlg, DM_REBUILD_CTREE, 0, 0);
		return TRUE;

	case DM_REBUILD_CTREE:
		{
			HWND hwndTree = GetDlgItem(hwndDlg, IDC_CATEGORYLIST);
			int indx;
			TCHAR itemName[1024];
			HTREEITEM hSection;

			if (!hwndTree) break;

			TreeView_SelectItem(hwndTree, NULL);
			TreeView_DeleteAllItems(hwndTree);

			for (indx = 0; indx < sectionList.getCount(); indx++) {
				TCHAR* sectionName;
				int sectionLevel = 0;

				hSection = NULL;
				lstrcpy(itemName, sectionList[indx]->name);
				sectionName = itemName;

				while (sectionName) {
					// allow multi-level tree
					TCHAR* pItemName = sectionName;
					HTREEITEM hItem;

					if (sectionName = _tcschr(sectionName, '/')) {
						// one level deeper
						*sectionName = 0;
					}

					pItemName = TranslateTS( pItemName );

					hItem = FindNamedTreeItemAt(hwndTree, hSection, pItemName);
					if (!sectionName || !hItem) {
						if (!hItem) {
							TVINSERTSTRUCT tvis = {0};
							TreeItem *treeItem = (TreeItem *)mir_alloc(sizeof(TreeItem));
							treeItem->value = SECTIONPARAM_MAKE( indx, sectionLevel, sectionName?0:SECTIONPARAM_HAVEPAGE );
							treeItem->paramName = mir_t2a(itemName);

							tvis.hParent = hSection;
							tvis.hInsertAfter = TVI_SORT; //TVI_LAST;
							tvis.item.mask = TVIF_TEXT|TVIF_PARAM|TVIF_STATE;
							tvis.item.pszText = pItemName;
							tvis.item.lParam = (LPARAM) treeItem;

							tvis.item.state = tvis.item.stateMask = DBGetContactSettingByte(NULL, "SkinIconsUI", treeItem->paramName, TVIS_EXPANDED );
							hItem = TreeView_InsertItem(hwndTree, &tvis);
						}
						else {
							TVITEM tvi = {0};
							TreeItem *treeItem;
							tvi.hItem = hItem;
							tvi.mask = TVIF_HANDLE | TVIF_PARAM;
							TreeView_GetItem( hwndTree, &tvi );
							treeItem = (TreeItem *)tvi.lParam;
							treeItem->value = SECTIONPARAM_MAKE( indx, sectionLevel, SECTIONPARAM_HAVEPAGE );
					}	}

					if (sectionName) {
						*sectionName = '/';
						sectionName++;
					}
					sectionLevel++;

					hSection = hItem;
			}	}

			ShowWindow(hwndTree, SW_SHOW);

			TreeView_SelectItem(hwndTree, FindNamedTreeItemAt(hwndTree, 0, NULL));
		}
		break;

	//  Rebuild preview to new section
	case DM_REBUILDICONSPREVIEW:
		{
			LVITEM lvi = {0};
			HIMAGELIST hIml;
			HICON hIcon;
			SectionItem* sectionActive = ( SectionItem* )lParam;
			int indx;

			EnableWindow(hPreview, sectionActive != NULL );

			ListView_DeleteAllItems(hPreview);
			hIml = ListView_GetImageList(hPreview, LVSIL_NORMAL);
			ImageList_RemoveAll(hIml);

			if (sectionActive == NULL)
				break;

			lvi.mask = LVIF_TEXT|LVIF_IMAGE|LVIF_PARAM;

			EnterCriticalSection(&csIconList);

			for (indx = 0; indx < iconList.getCount(); indx++) {
				IconItem *item = iconList[indx];

				if (item->section == sectionActive) {
					lvi.pszText = TranslateTS(item->description);
					hIcon = item->temp_icon;
					if ( !hIcon )
						hIcon = IconItem_GetIcon_Preview( item );
					lvi.iImage = ImageList_AddIcon(hIml, hIcon);
					lvi.lParam = indx;
					ListView_InsertItem(hPreview, &lvi);
					if (hIcon != item->temp_icon) SafeDestroyIcon( &hIcon );
			}	}

			LeaveCriticalSection(&csIconList);

			if ( sectionActive->flags & SIDF_SORTED )
				ListView_SortItems(hPreview, DoSortIconsFunc, 0);
			else
				ListView_SortItems(hPreview, DoSortIconsFuncByOrder, 0);
		}
		break;

	// Refresh preview to new section
	case DM_UPDATEICONSPREVIEW:
		{
			LVITEM lvi = {0};
			HICON hIcon;
			int indx, count;
			HIMAGELIST hIml = ListView_GetImageList(hPreview, LVSIL_NORMAL);

			lvi.mask = LVIF_IMAGE|LVIF_PARAM;
			count = ListView_GetItemCount(hPreview);

			for (indx = 0; indx < count; indx++) {
				lvi.iItem = indx;
				ListView_GetItem(hPreview, &lvi);
				EnterCriticalSection(&csIconList);
				hIcon = iconList[lvi.lParam]->temp_icon;
				if (!hIcon)
					hIcon = IconItem_GetIcon_Preview( iconList[lvi.lParam] );
				LeaveCriticalSection(&csIconList);

				if (hIcon)
					ImageList_ReplaceIcon(hIml, lvi.iImage, hIcon);
				if (hIcon != iconList[lvi.lParam]->temp_icon) SafeDestroyIcon( &hIcon );
			}
			ListView_RedrawItems(hPreview, 0, count);
		}
		break;

	// Temporary change icon - only inside options dialog
	case DM_CHANGEICON:
		{
			TCHAR *path=(TCHAR*)lParam;
			LVITEM lvi = {0};
			IconItem *item;

			lvi.mask = LVIF_PARAM;
			lvi.iItem = wParam;
			ListView_GetItem( hPreview, &lvi );

			EnterCriticalSection( &csIconList );
			item = iconList[ lvi.lParam ];

			SAFE_FREE(( void** )&item->temp_file );
			SafeDestroyIcon( &item->temp_icon );
			item->temp_file = mir_tstrdup( path );
			item->temp_icon = ( HICON )ExtractIconFromPath( path, item->cx, item->cy );
			item->temp_reset = FALSE;

			LeaveCriticalSection( &csIconList );
			DoOptionsChanged( hwndDlg );
		}
		break;

	case WM_COMMAND:
		if ( LOWORD(wParam) == IDC_IMPORT ) {
			dat->hwndIndex = CreateDialogParam(hMirandaInst, MAKEINTRESOURCE(IDD_ICOLIB_IMPORT), GetParent(hwndDlg), DlgProcIconImport, (LPARAM)hwndDlg);
			EnableWindow((HWND)lParam, FALSE);
		}
		else if ( LOWORD(wParam) == IDC_GETMORE ) {
			OpenIconsPage();
			break;
		}
		else if (LOWORD(wParam) == IDC_LOADICONS ) {
			TCHAR filetmp[1] = {0};
			TCHAR *file;

			if ( file = OpenFileDlg( hwndDlg, filetmp, FALSE )) {
				HWND htv = GetDlgItem( hwndDlg, IDC_CATEGORYLIST );
				TCHAR filename[ MAX_PATH ];

				CallService( MS_UTILS_PATHTORELATIVET, ( WPARAM )file, ( LPARAM )filename );
				SAFE_FREE(( void** )&file );

				MySetCursor( IDC_WAIT );
				LoadSubIcons( htv, filename, TreeView_GetSelection( htv ));
				MySetCursor( IDC_ARROW );

				DoOptionsChanged( hwndDlg );
		}	}
		break;

	case WM_CONTEXTMENU:
		if (( HWND )wParam == hPreview ) {
			UINT count = ListView_GetSelectedCount( hPreview );

			if ( count > 0 ) {
				int cmd = OpenPopupMenu( hwndDlg );
				switch( cmd ) {
				case ID_CANCELCHANGE:
				case ID_RESET:
					{
						LVITEM lvi = {0};
						int itemIndx = -1;

						while (( itemIndx = ListView_GetNextItem( hPreview, itemIndx, LVNI_SELECTED )) != -1 ) {
							lvi.mask = LVIF_PARAM;
							lvi.iItem = itemIndx; //lvhti.iItem;
							ListView_GetItem( hPreview, &lvi );

							UndoChanges( lvi.lParam, cmd );
						}

						DoOptionsChanged( hwndDlg );
						break;
			}	}	}
		}
		else {
			HWND htv = GetDlgItem( hwndDlg, IDC_CATEGORYLIST );
			if (( HWND )wParam == htv ) {
				int cmd = OpenPopupMenu( hwndDlg );

				switch( cmd ) {
				case ID_CANCELCHANGE:
				case ID_RESET:
					UndoSubItemChanges( htv, TreeView_GetSelection( htv ), cmd );
					DoOptionsChanged( hwndDlg );
					break;
		}	}	}
		break;

	case WM_NOTIFY:
		switch(((LPNMHDR)lParam)->idFrom) {
		case 0:
			switch(((LPNMHDR)lParam)->code) {
			case PSN_APPLY:
				{
					int indx;

					EnterCriticalSection(&csIconList);

					for (indx = 0; indx < iconList.getCount(); indx++) {
						IconItem *item = iconList[indx];
						if (item->temp_reset) {
							DBDeleteContactSetting(NULL, "SkinIcons", item->name);
							if (item->source_small != item->default_icon) {
								IconSourceItem_Release( &item->source_small );
							}
						}
						else if (item->temp_file) {
							DBWriteContactSettingTString(NULL, "SkinIcons", item->name, item->temp_file);
							IconSourceItem_Release( &item->source_small );
							SafeDestroyIcon( &item->temp_icon );
						}
					}
					LeaveCriticalSection(&csIconList);

					DoIconsChanged(hwndDlg);
					return TRUE;
			}	}
			break;

		case IDC_PREVIEW:
			if(((LPNMHDR)lParam)->code == LVN_GETINFOTIP)
			{
				IconItem *item;
				NMLVGETINFOTIP *pInfoTip = (NMLVGETINFOTIP *)lParam;
				LVITEM lvi;
				lvi.mask = LVIF_PARAM;
				lvi.iItem = pInfoTip->iItem;
				ListView_GetItem(pInfoTip->hdr.hwndFrom, &lvi);

				if( lvi.lParam < iconList.getCount() ) {
					item = iconList[lvi.lParam];
					if( item->temp_file )
						_tcsncpy( pInfoTip->pszText, item->temp_file, pInfoTip->cchTextMax );
					else if( item->default_file )
						mir_sntprintf( pInfoTip->pszText, pInfoTip->cchTextMax, _T("%s,%d"), item->default_file, item->default_indx );
				}
			}
			if ( bNeedRebuild )	{
				EnterCriticalSection(&csIconList);
				bNeedRebuild = FALSE;
				LeaveCriticalSection(&csIconList);
				SendMessage(hwndDlg, DM_REBUILD_CTREE, 0, 0);
			}
			break;

		case IDC_CATEGORYLIST:
			switch(((NMHDR*)lParam)->code) {
			case TVN_SELCHANGEDA: // !!!! This needs to be here - both !!
			case TVN_SELCHANGEDW:
				{
					NMTREEVIEW *pnmtv = (NMTREEVIEW*)lParam;
					TVITEM tvi = pnmtv->itemNew;
					TreeItem *treeItem = (TreeItem *)tvi.lParam;
					if ( treeItem )
						SendMessage(hwndDlg, DM_REBUILDICONSPREVIEW, 0, ( LPARAM )(
							(SECTIONPARAM_FLAGS(treeItem->value)&SECTIONPARAM_HAVEPAGE)?
							sectionList[ SECTIONPARAM_INDEX(treeItem->value) ] : NULL ) );
					break;
				}
			case TVN_DELETEITEMA: // no idea why both TVN_SELCHANGEDA/W should be there but let's keep this both too...
			case TVN_DELETEITEMW:
				{
					TreeItem *treeItem = (TreeItem *)(((LPNMTREEVIEW)lParam)->itemOld.lParam);
					if (treeItem) {
						mir_free(treeItem->paramName);
						mir_free(treeItem);
					}
					break;
			}	}
			if ( bNeedRebuild )	{
				EnterCriticalSection(&csIconList);
				bNeedRebuild = FALSE;
				LeaveCriticalSection(&csIconList);
				SendMessage(hwndDlg, DM_REBUILD_CTREE, 0, 0);
		}	}
		break;
	case WM_DESTROY:
		{
			int indx;

			SaveCollapseState( GetDlgItem(hwndDlg, IDC_CATEGORYLIST) );
			DestroyWindow(dat->hwndIndex);

			EnterCriticalSection(&csIconList);
			for (indx = 0; indx < iconList.getCount(); indx++) {
				IconItem *item = iconList[indx];

				SAFE_FREE(( void** )&item->temp_file);
				SafeDestroyIcon(&item->temp_icon);
			}
			LeaveCriticalSection(&csIconList);

			SAFE_FREE(( void** )&dat);
			break;
	}	}

	return FALSE;
}

static UINT iconsExpertOnlyControls[]={IDC_IMPORT};

static int SkinOptionsInit( WPARAM wParam,LPARAM )
{
	OPTIONSDIALOGPAGE odp = {0};

	odp.cbSize = sizeof(odp);
	odp.hInstance = hMirandaInst;
	odp.flags = ODPF_BOLDGROUPS;
	odp.position = -180000000;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_ICOLIB);
	odp.pszTitle = LPGEN("Icons");
	odp.pszGroup = LPGEN("Customize");
	odp.pfnDlgProc = DlgProcIcoLibOpts;
	odp.expertOnlyControls = iconsExpertOnlyControls;
	odp.nExpertOnlyControls = SIZEOF(iconsExpertOnlyControls);
	CallService(MS_OPT_ADDPAGE, wParam, (LPARAM)&odp);
	return 0;
}

static int SkinSystemModulesLoaded( WPARAM, LPARAM )
{
	HookEvent(ME_OPT_INITIALISE, SkinOptionsInit);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Module initialization and finalization procedure

static INT_PTR sttIcoLib_AddNewIcon( WPARAM, LPARAM lParam )
{	return (INT_PTR)IcoLib_AddNewIcon(( SKINICONDESC* )lParam );
}

static INT_PTR sttIcoLib_GetIcon( WPARAM wParam, LPARAM lParam )
{	return (INT_PTR)IcoLib_GetIcon(( const char* )lParam, wParam != 0 );
}

static INT_PTR sttIcoLib_GetIconHandle( WPARAM, LPARAM lParam )
{	return (INT_PTR)IcoLib_GetIconHandle(( const char* )lParam );
}

static INT_PTR sttIcoLib_GetIconByHandle( WPARAM wParam, LPARAM lParam )
{	return (INT_PTR)IcoLib_GetIconByHandle(( HANDLE )lParam, wParam != 0 );
}

static INT_PTR sttIcoLib_ReleaseIcon( WPARAM wParam, LPARAM lParam )
{	return (INT_PTR)IcoLib_ReleaseIcon(( HICON )wParam, ( char* )lParam, false );
}

static INT_PTR sttIcoLib_ReleaseIconBig( WPARAM wParam, LPARAM lParam )
{	return (INT_PTR)IcoLib_ReleaseIcon(( HICON )wParam, ( char* )lParam, true );
}

static INT_PTR sttIcoLib_IsManaged( WPARAM wParam, LPARAM )
{	return (INT_PTR)IcoLib_IsManaged(( HICON )wParam );
}

int LoadIcoLibModule(void)
{
	bModuleInitialized = TRUE;

	hIconBlank = LoadIconEx(NULL, MAKEINTRESOURCE(IDI_BLANK),0);

	InitializeCriticalSection(&csIconList);
	hIcoLib_AddNewIcon    = CreateServiceFunction(MS_SKIN2_ADDICON,         sttIcoLib_AddNewIcon);
	hIcoLib_RemoveIcon    = CreateServiceFunction(MS_SKIN2_REMOVEICON,      IcoLib_RemoveIcon);
	hIcoLib_GetIcon       = CreateServiceFunction(MS_SKIN2_GETICON,         sttIcoLib_GetIcon);
	hIcoLib_GetIconHandle = CreateServiceFunction(MS_SKIN2_GETICONHANDLE,   sttIcoLib_GetIconHandle);
	hIcoLib_GetIcon2      = CreateServiceFunction(MS_SKIN2_GETICONBYHANDLE, sttIcoLib_GetIconByHandle);
	hIcoLib_IsManaged     = CreateServiceFunction(MS_SKIN2_ISMANAGEDICON,   sttIcoLib_IsManaged);
	hIcoLib_AddRef        = CreateServiceFunction(MS_SKIN2_ADDREFICON,      IcoLib_AddRef);
	hIcoLib_ReleaseIcon   = CreateServiceFunction(MS_SKIN2_RELEASEICON,     sttIcoLib_ReleaseIcon);
	hIcoLib_ReleaseIcon   = CreateServiceFunction(MS_SKIN2_RELEASEICONBIG,  sttIcoLib_ReleaseIconBig);

	hIcons2ChangedEvent = CreateHookableEvent(ME_SKIN2_ICONSCHANGED);
	hIconsChangedEvent = CreateHookableEvent(ME_SKIN_ICONSCHANGED);

	HookEvent(ME_SYSTEM_MODULESLOADED, SkinSystemModulesLoaded);

	return 0;
}

void UnloadIcoLibModule(void)
{
	int indx;

	if ( !bModuleInitialized ) return;

	DestroyHookableEvent(hIconsChangedEvent);
	DestroyHookableEvent(hIcons2ChangedEvent);

	DestroyServiceFunction(hIcoLib_AddNewIcon);
	DestroyServiceFunction(hIcoLib_RemoveIcon);
	DestroyServiceFunction(hIcoLib_GetIcon);
	DestroyServiceFunction(hIcoLib_GetIconHandle);
	DestroyServiceFunction(hIcoLib_GetIcon2);
	DestroyServiceFunction(hIcoLib_IsManaged);
	DestroyServiceFunction(hIcoLib_AddRef);
	DestroyServiceFunction(hIcoLib_ReleaseIcon);
	DeleteCriticalSection(&csIconList);

	for (indx = iconList.getCount()-1; indx >= 0; indx-- ) {
		IconItem* I = iconList[indx];
		iconList.remove( indx );
		IcoLib_FreeIcon( I );
		mir_free( I );
	}
	iconList.destroy();

	for (indx = iconSourceList.getCount()-1; indx >= 0; indx-- ) {
		IconSourceItem* I = iconSourceList[indx];
		iconSourceList.remove( indx );
		IconSourceFile_Release( &I->file );
		SafeDestroyIcon( &I->icon );
		SAFE_FREE(( void** )&I->icon_data );
		SAFE_FREE(( void** )&I );
	}
	iconSourceList.destroy();

	for (indx = iconSourceFileList.getCount()-1; indx >= 0; indx-- ) {
		IconSourceFile* I = iconSourceFileList[indx];
		iconSourceFileList.remove( indx );
		SAFE_FREE(( void** )&I->file );
		SAFE_FREE(( void** )&I );
	}
	iconSourceFileList.destroy();

	for (indx = 0; indx < sectionList.getCount(); indx++) {
		SAFE_FREE(( void** )&sectionList[indx]->name );
		mir_free( sectionList[indx] );
	}
	sectionList.destroy();

	SafeDestroyIcon(&hIconBlank);
}
