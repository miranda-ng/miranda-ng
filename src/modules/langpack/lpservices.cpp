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

#if defined( _UNICODE )
	#define FLAGS LANG_UNICODE
#else
	#define FLAGS 0
#endif

LangPackMuuid* __fastcall LangPackLookupUuid( WPARAM );
int LangPackMarkPluginLoaded( PLUGININFOEX* pInfo );

/////////////////////////////////////////////////////////////////////////////////////////

static INT_PTR TranslateString(WPARAM wParam,LPARAM lParam)
{
	return (INT_PTR)LangPackTranslateString( LangPackLookupUuid(wParam), (const char *)lParam, (wParam & LANG_UNICODE) ? 1 : 0);
}

/////////////////////////////////////////////////////////////////////////////////////////

static INT_PTR TranslateMenu(WPARAM wParam, LPARAM lParam)
{
	HMENU        hMenu = ( HMENU )wParam;
	int          i;
	MENUITEMINFO mii;
	TCHAR        str[256];
	LangPackMuuid* uuid = LangPackLookupUuid( lParam );

	mii.cbSize = MENUITEMINFO_V4_SIZE;
	for ( i = GetMenuItemCount( hMenu )-1; i >= 0; i--) {
		mii.fMask = MIIM_TYPE|MIIM_SUBMENU;
		mii.dwTypeData = ( TCHAR* )str;
		mii.cch = SIZEOF(str);
		GetMenuItemInfo(hMenu, i, TRUE, &mii);

		if ( mii.cch && mii.dwTypeData ) {
			TCHAR* result = ( TCHAR* )LangPackTranslateString( uuid, ( const char* )mii.dwTypeData, FLAGS );
			if ( result != mii.dwTypeData ) {
				mii.dwTypeData = result;
				mii.fMask = MIIM_TYPE;
				SetMenuItemInfo( hMenu, i, TRUE, &mii );
		}	}

		if ( mii.hSubMenu != NULL ) TranslateMenu(( WPARAM )mii.hSubMenu, lParam );
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

static void TranslateWindow( LangPackMuuid* pUuid, HWND hwnd )
{
	TCHAR title[2048];
	GetWindowText(hwnd, title, SIZEOF( title ));
	{
		TCHAR* result = ( TCHAR* )LangPackTranslateString( pUuid, ( const char* )title, FLAGS );
		if ( result != title )
			SetWindowText(hwnd, result );
}	}

static BOOL CALLBACK TranslateDialogEnumProc(HWND hwnd,LPARAM lParam)
{
	LANGPACKTRANSLATEDIALOG *lptd = (LANGPACKTRANSLATEDIALOG*)lParam;
	TCHAR szClass[32];
	int i,id = GetDlgCtrlID( hwnd );

	if ( lptd->ignoreControls != NULL )
		for ( i=0; lptd->ignoreControls[i]; i++ )
			if ( lptd->ignoreControls[i] == id )
				return TRUE;

	LangPackMuuid* uuid = LangPackLookupUuid( lptd->flags );

	GetClassName( hwnd, szClass, SIZEOF(szClass));
	if(!lstrcmpi(szClass,_T("static")) || !lstrcmpi(szClass,_T("hyperlink")) || !lstrcmpi(szClass,_T("button")) || !lstrcmpi(szClass,_T("MButtonClass")) || !lstrcmpi(szClass,_T("MHeaderbarCtrl")))
		TranslateWindow( uuid, hwnd );
	else if ( !lstrcmpi( szClass,_T("edit"))) {
		if( lptd->flags & LPTDF_NOIGNOREEDIT || GetWindowLongPtr(hwnd,GWL_STYLE) & ES_READONLY )
			TranslateWindow( uuid, hwnd );
	}
	return TRUE;
}

static INT_PTR TranslateDialog(WPARAM wParam, LPARAM lParam)
{
	LANGPACKTRANSLATEDIALOG *lptd = (LANGPACKTRANSLATEDIALOG*)lParam;
	if ( lptd == NULL || lptd->cbSize != sizeof(LANGPACKTRANSLATEDIALOG))
		return 1;

	if ( !( lptd->flags & LPTDF_NOTITLE ))
		TranslateWindow( LangPackLookupUuid( lptd->flags ), lptd->hwndDlg );

	EnumChildWindows( lptd->hwndDlg, TranslateDialogEnumProc, lParam );
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

static INT_PTR LPRegister(WPARAM wParam, LPARAM lParam)
{
	*( int* )wParam = LangPackMarkPluginLoaded(( PLUGININFOEX* )lParam );
	return 0;
}

static INT_PTR GetDefaultCodePage(WPARAM,LPARAM)
{
	return LangPackGetDefaultCodePage();
}

static INT_PTR GetDefaultLocale(WPARAM, LPARAM)
{
	return LangPackGetDefaultLocale();
}

static INT_PTR PcharToTchar(WPARAM wParam, LPARAM lParam)
{
	char* pszStr = ( char* )lParam;
	if ( pszStr == NULL )
		return NULL;

	LangPackMuuid* uuid = LangPackLookupUuid( wParam );

	#if defined( _UNICODE )
	{	int len = (int)strlen( pszStr );
		TCHAR* result = ( TCHAR* )alloca(( len+1 )*sizeof( TCHAR ));
		MultiByteToWideChar( LangPackGetDefaultCodePage(), 0, pszStr, -1, result, len );
		result[len] = 0;
		return ( INT_PTR )mir_wstrdup(( wchar_t* )LangPackTranslateString( uuid, ( char* )result, 1 ));
	}
	#else
		return ( INT_PTR )mir_strdup( LangPackTranslateString( uuid, pszStr, 0 ));
	#endif
}

int LoadLangPackServices(void)
{
	CreateServiceFunction(MS_LANGPACK_TRANSLATESTRING,TranslateString);
	CreateServiceFunction(MS_LANGPACK_TRANSLATEMENU,TranslateMenu);
	CreateServiceFunction(MS_LANGPACK_TRANSLATEDIALOG,TranslateDialog);
	CreateServiceFunction(MS_LANGPACK_GETCODEPAGE,GetDefaultCodePage);
	CreateServiceFunction(MS_LANGPACK_GETLOCALE,GetDefaultLocale);
	CreateServiceFunction(MS_LANGPACK_PCHARTOTCHAR,PcharToTchar);
	CreateServiceFunction(MS_LANGPACK_REGISTER,LPRegister);
	return 0;
}

