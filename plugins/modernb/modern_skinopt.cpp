/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2008 Miranda ICQ/IM project, 
all portions of this codebase are copyrighted to the people 
listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or ( at your option ) any later version.

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
#include "hdr/modern_clist.h"
#include "m_clc.h"
#include "io.h"
#include "hdr/modern_commonprototypes.h"
#include "hdr/modern_sync.h"
#include <m_utils.h>
#include <m_database.h>
#include <m_modernopt.h>

/*******************************/
// Main skin selection routine //
/*******************************/
#define MAX_NAME 100
typedef struct _SkinListData
{
	TCHAR Name[MAX_NAME];
	TCHAR File[MAX_PATH];
} SkinListData;

HBITMAP hPreviewBitmap = NULL;
HTREEITEM AddItemToTree( HWND hTree, TCHAR * folder, TCHAR * itemName, void * data );
HTREEITEM AddSkinToListFullName( HWND hwndDlg, TCHAR * fullName );
HTREEITEM AddSkinToList( HWND hwndDlg, TCHAR * path, TCHAR* file );
HTREEITEM FillAvailableSkinList( HWND hwndDlg );

INT_PTR CALLBACK DlgSkinOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

int SkinOptInit( WPARAM wParam, LPARAM lParam )
{
	OPTIONSDIALOGPAGE odp;
	if ( !g_CluiData.fDisableSkinEngine )
	{
		//Tabbed settings
		ZeroMemory( &odp, sizeof( odp ) );
		odp.cbSize = sizeof( odp );
		odp.position = -200000000;
		odp.hInstance = g_hInst;
		odp.pfnDlgProc = DlgSkinOpts;
		odp.pszTemplate = MAKEINTRESOURCEA( IDD_OPT_SKIN );
		odp.ptszGroup = LPGENT( "Skins" );
		odp.ptszTitle = LPGENT( "Contact List" );
		odp.flags = ODPF_BOLDGROUPS|ODPF_TCHAR;
		odp.ptszTab = LPGENT( "Load/Save" );
		CallService( MS_OPT_ADDPAGE, wParam, ( LPARAM )&odp );

		if ( ModernGetSettingByte( NULL, "ModernData", "EnableSkinEditor", SETTING_ENABLESKINEDITOR_DEFAULT ) )
		{
			odp.flags |= ODPF_EXPERTONLY;
			odp.pfnDlgProc = DlgSkinEditorOpts;
			odp.pszTemplate = MAKEINTRESOURCEA( IDD_OPT_SKINEDITOR );
			odp.ptszTab = LPGENT( "Object Editor" );
			CallService( MS_OPT_ADDPAGE, wParam, ( LPARAM )&odp );
		}
	}
	return 0;
}
int ModernSkinOptInit( WPARAM wParam, LPARAM lParam )
{
	MODERNOPTOBJECT obj = {0};
	obj.cbSize = sizeof(obj);
	obj.dwFlags = MODEROPT_FLG_TCHAR;
	obj.hIcon = LoadSkinnedIcon(SKINICON_OTHER_MIRANDA);
	obj.hInstance = g_hInst;
	obj.iSection = MODERNOPT_PAGE_SKINS;
	obj.iType = MODERNOPT_TYPE_SELECTORPAGE;
	obj.lptzSubsection = _T("Contact List");
	obj.lpzThemeExtension = ".msf";
	obj.lpzThemeModuleName = "ModernSkinSel";
	CallService(MS_MODERNOPT_ADDOBJECT, wParam, (LPARAM)&obj);
	return 0;
}
INT_PTR CALLBACK DlgSkinOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch ( msg )
	{
	case WM_DESTROY: 
		{
			if ( hPreviewBitmap ) ske_UnloadGlyphImage( hPreviewBitmap );
			break;
		}

	case WM_INITDIALOG:
		{ 
			HTREEITEM it;
			TranslateDialogDefault( hwndDlg );
			it = FillAvailableSkinList( hwndDlg );
			HWND wnd = GetDlgItem( hwndDlg, IDC_TREE1 );
			TreeView_SelectItem( wnd, it );						
		}
		return 0;
	case WM_COMMAND:
		{
			int isLoad = 0;
			switch ( LOWORD( wParam ) ) 
			{
			case IDC_COLOUR_MENUNORMAL:
			case IDC_COLOUR_MENUSELECTED:
			case IDC_COLOUR_FRAMES:
			case IDC_COLOUR_STATUSBAR:
				SendMessage( GetParent( hwndDlg ), PSM_CHANGED, 0, 0 );
				break;

			case IDC_BUTTON_INFO:
				{
					TCHAR Author[255];
					TCHAR URL[MAX_PATH];
					TCHAR Contact[255];
					TCHAR Description[400];
					TCHAR text[2000];
					SkinListData *sd = NULL;  
					HTREEITEM hti = TreeView_GetSelection( GetDlgItem( hwndDlg, IDC_TREE1 ) );				
					if ( hti == 0 ) return 0;
					{
						TVITEM tvi = {0};
						tvi.hItem = hti;
						tvi.mask = TVIF_HANDLE|TVIF_PARAM;
						TreeView_GetItem( GetDlgItem( hwndDlg, IDC_TREE1 ), &tvi );
						sd = ( SkinListData* )( tvi.lParam );
					}
					if ( !sd ) return 0;
					if ( sd->File && !_tcschr( sd->File, _T('%') ) )
					{
						GetPrivateProfileString( _T( "Skin_Description_Section" ), _T( "Author" ), 	TranslateT( "( unknown )" ), 	Author, 		SIZEOF( Author ), 		sd->File );
						GetPrivateProfileString( _T( "Skin_Description_Section" ), _T( "URL" ), 		_T( "" ), 						URL, 		SIZEOF( URL ), 		sd->File );
						GetPrivateProfileString( _T( "Skin_Description_Section" ), _T( "Contact" ), 	_T( "" ), 						Contact, 	SIZEOF( Contact ), 	sd->File );
						GetPrivateProfileString( _T( "Skin_Description_Section" ), _T( "Description" ), _T( "" ), 					Description, SIZEOF( Description ), sd->File );
						_sntprintf( text, SIZEOF( text ), TranslateT( "%s\n\n%s\n\nAuthor(s):\t %s\nContact:\t %s\nWeb:\t %s\n\nFile:\t %s" ), 
							sd->Name, Description, Author, Contact, URL, sd->File );
					}
					else
					{
						_sntprintf( text, SIZEOF( text ), TranslateT( "%s\n\n%s\n\nAuthor(s): %s\nContact:\t %s\nWeb:\t %s\n\nFile:\t %s" ), 
							TranslateT( "reVista for Modern v0.5" ), 
							TranslateT( "This is second default Modern Contact list skin in Vista Aero style" ), 
							TranslateT( "Angeli-Ka (graphics), FYR (template)" ), 
							_T( "JID: fyr@jabber.ru" ), 
							_T("fyr.mirandaim.ru"), 
							TranslateT( "Inside library" ) );
					}
					MessageBox( hwndDlg, text, TranslateT( "Skin Information" ), MB_OK|MB_ICONINFORMATION );
				}
				break;
			case IDC_BUTTON_APPLY_SKIN:
				if ( HIWORD( wParam ) == BN_CLICKED )
				{ 		
					SkinListData *sd = NULL;  
					HTREEITEM hti = TreeView_GetSelection( GetDlgItem( hwndDlg, IDC_TREE1 ) );				
					if ( hti == 0 ) return 0;
					{
						TVITEM tvi = {0};
						tvi.hItem = hti;
						tvi.mask = TVIF_HANDLE|TVIF_PARAM;
						TreeView_GetItem( GetDlgItem( hwndDlg, IDC_TREE1 ), &tvi );
						sd = ( SkinListData* )( tvi.lParam );
					}
					if ( !sd ) return 0;
					if ( glSkinWasModified>0 )
					{
						int res = 0;
						if ( glSkinWasModified == 1 )
							res = MessageBox( hwndDlg, TranslateT( "Skin editor contains not stored changes.\n\nAll changes will be lost.\n\n Continue to load new skin?" ), TranslateT( "Warning!" ), MB_OKCANCEL|MB_ICONWARNING|MB_DEFBUTTON2|MB_TOPMOST );
						else
							res = MessageBox( hwndDlg, TranslateT( "Current skin was not saved to file.\n\nAll changes will be lost.\n\n Continue to load new skin?" ), TranslateT( "Warning!" ), MB_OKCANCEL|MB_ICONWARNING|MB_DEFBUTTON2|MB_TOPMOST );
						if ( res!= IDOK ) return 0;
					}
					ske_LoadSkinFromIniFile( sd->File, FALSE );
					ske_LoadSkinFromDB( );	
					glOtherSkinWasLoaded = TRUE;
					pcli->pfnClcBroadcast( INTM_RELOADOPTIONS, 0, 0 );
					Sync( CLUIFrames_OnClistResize_mod, 0, 0 );
					ske_RedrawCompleteWindow( );        
					Sync( CLUIFrames_OnClistResize_mod, 0, 0 );
					{
						HWND hwnd = pcli->hwndContactList;
						RECT rc = {0};
						GetWindowRect( hwnd, &rc );
						Sync( CLUIFrames_OnMoving, hwnd, &rc );
					}
					if ( g_hCLUIOptionsWnd )
					{
						SendDlgItemMessage( g_hCLUIOptionsWnd, IDC_LEFTMARGINSPIN, UDM_SETPOS, 0, ModernGetSettingByte( NULL, "CLUI", "LeftClientMargin", SETTING_LEFTCLIENTMARIGN_DEFAULT ) );
						SendDlgItemMessage( g_hCLUIOptionsWnd, IDC_RIGHTMARGINSPIN, UDM_SETPOS, 0, ModernGetSettingByte( NULL, "CLUI", "RightClientMargin", SETTING_RIGHTCLIENTMARIGN_DEFAULT ) );
						SendDlgItemMessage( g_hCLUIOptionsWnd, IDC_TOPMARGINSPIN, UDM_SETPOS, 0, ModernGetSettingByte( NULL, "CLUI", "TopClientMargin", SETTING_TOPCLIENTMARIGN_DEFAULT ) );
						SendDlgItemMessage( g_hCLUIOptionsWnd, IDC_BOTTOMMARGINSPIN, UDM_SETPOS, 0, ModernGetSettingByte( NULL, "CLUI", "BottomClientMargin", SETTING_BOTTOMCLIENTMARIGN_DEFAULT ) );
					}
				}
				break;
			case IDC_BUTTON_LOAD:
				isLoad = 1;
				if ( HIWORD( wParam ) == BN_CLICKED )
				{
					{   		
						TCHAR str[MAX_PATH] = {0};
						OPENFILENAME ofn = {0};
						TCHAR filter[512] = {0};
						int res = 0;
						ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
						ofn.hwndOwner = hwndDlg;
						ofn.hInstance = NULL;

						mir_sntprintf(filter, SIZEOF(filter), _T("%s (*.msf)%c*.MSF%c%c"), TranslateT("Miranda skin file"), 0, 0, 0);
						ofn.lpstrFilter = filter;
						ofn.lpstrFile = str;
						ofn.Flags = isLoad?( OFN_FILEMUSTEXIST | OFN_HIDEREADONLY ) : ( OFN_OVERWRITEPROMPT|OFN_HIDEREADONLY ) | OFN_DONTADDTORECENT;
						ofn.nMaxFile = sizeof( str );
						ofn.nMaxFileTitle = MAX_PATH;
						ofn.lpstrDefExt = _T( "msf" );

						{
							DWORD tick = GetTickCount( );
							res = GetOpenFileName( &ofn );
							if( !res ) 
								if ( GetTickCount( )-tick<100 )
								{
									res = GetOpenFileName( &ofn );
									if( !res ) break;
								}
								else break;
						}
						if ( res )
						{
							HTREEITEM it = AddSkinToListFullName( hwndDlg, ofn.lpstrFile );
							TreeView_SelectItem( GetDlgItem( hwndDlg, IDC_TREE1 ), it );
							//SendDlgItemMessage( hwndDlg, IDC_SKINS_LIST, LB_SETCURSEL, it, 0 ); 
							//SendMessage( hwndDlg, WM_COMMAND, MAKEWPARAM( IDC_SKINS_LIST, LBN_SELCHANGE ), 0 );
						}
					}
				}
			}
			break;
		}
	case WM_DRAWITEM:
		if ( wParam == IDC_PREVIEW )
		{
			//TODO:Draw hPreviewBitmap here
			HDC memDC, imgDC;
			HBITMAP hbmp, holdbmp, imgOldbmp;
			int mWidth, mHeight;
			RECT workRect = {0};
			HBRUSH hbr = CreateSolidBrush( GetSysColor( COLOR_3DFACE ) );
			DRAWITEMSTRUCT *dis = ( DRAWITEMSTRUCT * )lParam;
			mWidth = dis->rcItem.right-dis->rcItem.left;
			mHeight = dis->rcItem.bottom-dis->rcItem.top;
			memDC = CreateCompatibleDC( dis->hDC );
			hbmp = ske_CreateDIB32( mWidth, mHeight );
			holdbmp = ( HBITMAP )SelectObject( memDC, hbmp );
			workRect = dis->rcItem;
			OffsetRect( &workRect, -workRect.left, -workRect.top );
			FillRect( memDC, &workRect, hbr );     
			DeleteObject( hbr );
			if ( hPreviewBitmap )
			{
				//variables
				BITMAP bmp = {0};
				POINT imgPos = {0};
				int wWidth, wHeight;
				int dWidth, dHeight;
				float xScale = 1, yScale = 1;
				//GetSize
				GetObject( hPreviewBitmap, sizeof( BITMAP ), &bmp );
				wWidth = workRect.right-workRect.left;
				wHeight = workRect.bottom-workRect.top;
				if ( wWidth<bmp.bmWidth ) xScale = ( float )wWidth/bmp.bmWidth;
				if ( wHeight<bmp.bmHeight ) yScale = ( float )wHeight/bmp.bmHeight;
				xScale = min( xScale, yScale );
				yScale = xScale;                    
				dWidth = ( int )( xScale*bmp.bmWidth );
				dHeight = ( int )( yScale*bmp.bmHeight );
				//CalcPosition
				imgPos.x = workRect.left+( ( wWidth-dWidth )>>1 );
				imgPos.y = workRect.top+( ( wHeight-dHeight )>>1 );     
				//DrawImage
				if ( !g_CluiData.fGDIPlusFail ) //Use gdi+ engine
				{
					DrawAvatarImageWithGDIp( memDC, imgPos.x, imgPos.y, dWidth, dHeight, hPreviewBitmap, 0, 0, bmp.bmWidth, bmp.bmHeight, 8, 255 );
				}   
				else
				{
					BLENDFUNCTION bf = {AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };
					imgDC = CreateCompatibleDC( dis->hDC );
					imgOldbmp = ( HBITMAP )SelectObject( imgDC, hPreviewBitmap );                 
					ske_AlphaBlend( memDC, imgPos.x, imgPos.y, dWidth, dHeight, imgDC, 0, 0, bmp.bmWidth, bmp.bmHeight, bf );
					SelectObject( imgDC, imgOldbmp );
					mod_DeleteDC( imgDC );
				}
			}
			BitBlt( dis->hDC, dis->rcItem.left, dis->rcItem.top, mWidth, mHeight, memDC, 0, 0, SRCCOPY );
			SelectObject( memDC, holdbmp );
			DeleteObject( hbmp );
			mod_DeleteDC( memDC );
		}
		break;

	case WM_NOTIFY:
		switch ( ( ( LPNMHDR )lParam )->idFrom ) 
		{
		case IDC_TREE1:
			{		
				NMTREEVIEW * nmtv = ( NMTREEVIEW * ) lParam;
				if ( !nmtv ) return 0;
				if ( nmtv->hdr.code == TVN_SELCHANGEDA
					|| nmtv->hdr.code == TVN_SELCHANGEDW )
				{	
					SkinListData * sd = NULL;
					if ( hPreviewBitmap ) 
					{
						ske_UnloadGlyphImage( hPreviewBitmap );
						hPreviewBitmap = NULL;
					}
					if ( nmtv->itemNew.lParam )
					{
						sd = ( SkinListData* )nmtv->itemNew.lParam;
						{
							TCHAR buf[MAX_PATH];
							CallService( MS_UTILS_PATHTORELATIVET, ( WPARAM )sd->File, ( LPARAM )buf );
							SendDlgItemMessage( hwndDlg, IDC_EDIT_SKIN_FILENAME, WM_SETTEXT, 0, ( LPARAM )buf );
						}
						{
							TCHAR prfn[MAX_PATH] = {0};
							TCHAR imfn[MAX_PATH] = {0};
							TCHAR skinfolder[MAX_PATH] = {0};
							GetPrivateProfileString( _T( "Skin_Description_Section" ), _T( "Preview" ), _T( "" ), imfn, SIZEOF( imfn ), sd->File );
							IniParser::GetSkinFolder( sd->File, skinfolder );
							_sntprintf( prfn, SIZEOF( prfn ), _T("%s\\%s"), skinfolder, imfn );
							CallService( MS_UTILS_PATHTOABSOLUTET, ( WPARAM )prfn, ( LPARAM ) imfn );
							char * imfn_ch = mir_t2a( imfn ); 
							hPreviewBitmap = ske_LoadGlyphImage( imfn_ch );
							mir_free( imfn_ch );
						}
						EnableWindow( GetDlgItem( hwndDlg, IDC_BUTTON_APPLY_SKIN ), TRUE );
						EnableWindow( GetDlgItem( hwndDlg, IDC_BUTTON_INFO ), TRUE );
						if ( hPreviewBitmap ) 
							InvalidateRect( GetDlgItem( hwndDlg, IDC_PREVIEW ), NULL, TRUE );
						else  //prepare text
						{
							TCHAR Author[255];
							TCHAR URL[MAX_PATH];
							TCHAR Contact[255];
							TCHAR Description[400];
							TCHAR text[2000];
							SkinListData* sd = NULL;
							HTREEITEM hti = TreeView_GetSelection( GetDlgItem( hwndDlg, IDC_TREE1 ) );				
							if ( hti == 0 ) return 0;
							{
								TVITEM tvi = {0};
								tvi.hItem = hti;
								tvi.mask = TVIF_HANDLE|TVIF_PARAM;
								TreeView_GetItem( GetDlgItem( hwndDlg, IDC_TREE1 ), &tvi );
								sd = ( SkinListData* )( tvi.lParam );
							}
							if ( !sd ) return 0;

							if( sd->File && !_tcschr( sd->File, _T('%') ) )
							{
								GetPrivateProfileString( _T( "Skin_Description_Section" ), _T( "Author" ), 	TranslateT( "( unknown )" ), 	Author, 		SIZEOF( Author ), 		sd->File );
								GetPrivateProfileString( _T( "Skin_Description_Section" ), _T( "URL" ), 		_T( "" ), 						URL, 		SIZEOF( URL ), 		sd->File );
								GetPrivateProfileString( _T( "Skin_Description_Section" ), _T( "Contact" ), 	_T( "" ), 						Contact, 	SIZEOF( Contact ), 	sd->File );
								GetPrivateProfileString( _T( "Skin_Description_Section" ), _T( "Description" ), _T( "" ), 					Description, SIZEOF( Description ), sd->File );
								_sntprintf( text, SIZEOF( text ), TranslateT( "Preview is not available\n\n%s\n----------------------\n\n%s\n\nAUTHOR(S):\n%s\n\nCONTACT:\n%s\n\nHOMEPAGE:\n%s" ), 
									sd->Name, Description, Author, Contact, URL );
							}
							else
							{
								_sntprintf( text, SIZEOF( text ), TranslateT( "%s\n\n%s\n\nAUTHORS:\n%s\n\nCONTACT:\n%s\n\nWEB:\n%s\n\n\n" ), 
									TranslateT( "reVista for Modern v0.5" ), 
									TranslateT( "This is second default Modern Contact list skin in Vista Aero style" ), 
									TranslateT( "graphics by Angeli-Ka\ntemplate by FYR" ), 
									_T("JID: fyr@jabber.ru"), 
									_T("fyr.mirandaim.ru") );
							}
							ShowWindow( GetDlgItem( hwndDlg, IDC_PREVIEW ), SW_HIDE );
							ShowWindow( GetDlgItem( hwndDlg, IDC_STATIC_INFO ), SW_SHOW );
							SendDlgItemMessage( hwndDlg, IDC_STATIC_INFO, WM_SETTEXT, 0, ( LPARAM )text );
						}					
					}
					else
					{
						//no selected
						SendDlgItemMessage( hwndDlg, IDC_EDIT_SKIN_FILENAME, WM_SETTEXT, 0, ( LPARAM )TranslateT( "Select skin from list" ) );
						EnableWindow( GetDlgItem( hwndDlg, IDC_BUTTON_APPLY_SKIN ), FALSE );
						EnableWindow( GetDlgItem( hwndDlg, IDC_BUTTON_INFO ), FALSE );
						SendDlgItemMessage( hwndDlg, IDC_STATIC_INFO, WM_SETTEXT, 0, ( LPARAM )TranslateT( "Please select skin to apply" ) );
						ShowWindow( GetDlgItem( hwndDlg, IDC_PREVIEW ), SW_HIDE );
					}
					ShowWindow( GetDlgItem( hwndDlg, IDC_PREVIEW ), hPreviewBitmap?SW_SHOW:SW_HIDE );
					return 0;
				}			
				else if ( nmtv->hdr.code == TVN_DELETEITEMA || nmtv->hdr.code == TVN_DELETEITEMW )
				{
					if ( nmtv->itemOld.lParam )
						mir_free_and_nill( nmtv->itemOld.lParam );
					return 0;
				}
				break;
			}
		case 0:
			switch ( ( ( LPNMHDR )lParam )->code )
			{
			case PSN_APPLY:
				{
					{
						DWORD tick = GetTickCount( );
						pcli->pfnClcBroadcast( INTM_RELOADOPTIONS, 0, 0 );
						NotifyEventHooks( g_CluiData.hEventBkgrChanged, 0, 0 );
						pcli->pfnClcBroadcast( INTM_INVALIDATE, 0, 0 );	
						RedrawWindow( GetParent( pcli->hwndContactTree ), NULL, NULL, RDW_INVALIDATE|RDW_FRAME|RDW_ALLCHILDREN );
					}
					return 0;
				}
				break;
			}
			break;
		}
	}
	return 0;
}

int SearchSkinFiles( HWND hwndDlg, TCHAR * Folder )
{
	struct _tfinddata_t fd = {0};
	TCHAR mask[MAX_PATH];
	long hFile; 
	_sntprintf( mask, SIZEOF( mask ), _T("%s\\*.msf"), Folder ); 
	//fd.attrib = _A_SUBDIR;
	hFile = _tfindfirst( mask, &fd );
	if ( hFile!= -1 )
	{
		do {     
			AddSkinToList( hwndDlg, Folder, fd.name );
		}while ( !_tfindnext( hFile, &fd ) );
		_findclose( hFile );
	}
	_sntprintf( mask, SIZEOF( mask ), _T("%s\\*"), Folder );
	hFile = _tfindfirst( mask, &fd );
	{
		do {
			if ( fd.attrib&_A_SUBDIR && !( _tcsicmp( fd.name, _T(".")) ==0 ||_tcsicmp( fd.name, _T("..") )==0 ) )
			{//Next level of subfolders
				TCHAR path[MAX_PATH];
				_sntprintf( path, SIZEOF( path ), _T("%s\\%s"), Folder, fd.name );
				SearchSkinFiles( hwndDlg, path );
			}
		}while ( !_tfindnext( hFile, &fd ) );
		_findclose( hFile );
	}
	return 0;
}
HTREEITEM FillAvailableSkinList( HWND hwndDlg )
{
	struct _finddata_t fd = {0};
	//long hFile; 
	HTREEITEM res = (HTREEITEM)-1;
	TCHAR path[MAX_PATH];//, mask[MAX_PATH];
	int attrib;
	TCHAR *SkinsFolder = ModernGetStringT( NULL, "ModernData", "SkinsFolder" );
	if ( !SkinsFolder ) SkinsFolder = mir_tstrdup( _T("Skins") );

	CallService( MS_UTILS_PATHTOABSOLUTET, ( WPARAM )SkinsFolder, ( LPARAM )path );
	mir_free_and_nill( SkinsFolder );

	AddSkinToList( hwndDlg, TranslateT( "Default Skin" ), _T("%Default Skin%") );
	attrib = GetFileAttributes( path );
	if ( attrib != INVALID_FILE_ATTRIBUTES && ( attrib & FILE_ATTRIBUTE_DIRECTORY ) )
		SearchSkinFiles( hwndDlg, path );
	{
		TCHAR * skinfile;
		TCHAR skinfull[MAX_PATH];
		skinfile = ModernGetStringT( NULL, SKIN, "SkinFile" );
		if ( skinfile )
		{
			CallService( MS_UTILS_PATHTOABSOLUTET, ( WPARAM )skinfile, ( LPARAM )skinfull );
			res = AddSkinToListFullName( hwndDlg, skinfull );

			mir_free_and_nill( skinfile );
		}
	}
	return res;
}
HTREEITEM AddSkinToListFullName( HWND hwndDlg, TCHAR * fullName )
{
	TCHAR path[MAX_PATH] = {0};
	TCHAR file[MAX_PATH] = {0};
	TCHAR *buf;
	_tcsncpy( path, fullName, SIZEOF( path ) );
	buf = path + _tcslen( path );  
	while ( buf > path )
	{
		if ( *buf == _T( '\\' ) )
		{
			*buf = _T( '\0' );
			break;
		}
		buf--;
	}
	buf++;
	_tcsncpy( file, buf, SIZEOF( file ) );
	return AddSkinToList( hwndDlg, path, file );
}


HTREEITEM AddSkinToList( HWND hwndDlg, TCHAR * path, TCHAR* file )
{
	{
		TCHAR buf[MAX_PATH];
		_sntprintf( buf, SIZEOF( buf ), _T("%s\\%s"), path, file );

	}
	{
		TCHAR fullName[MAX_PATH] = {0};     
		TCHAR defskinname[MAX_PATH] = {0};
		SkinListData * sd = NULL;
		sd = ( SkinListData * )mir_alloc( sizeof( SkinListData ) );
		if ( !sd ) return 0;
		_sntprintf( fullName, SIZEOF( fullName ), _T("%s\\%s"), path, file );
		memmove( defskinname, file, (_tcslen( file )-4) * sizeof(TCHAR) );
		defskinname[_tcslen( file )+1] = _T('\0');
		if ( !file || _tcschr( file, _T('%') ) ) 
		{
			//sd->File = "%Default Skin%";
			_sntprintf( sd->File, MAX_PATH, _T("%%Default Skin%%") );
			_sntprintf( sd->Name, 100, TranslateT( "%Default Skin%" ) );
			return AddItemToTree( GetDlgItem( hwndDlg, IDC_TREE1 ), TranslateT( "Default Skin" ), sd->Name, sd );
		}
		else
		{
			GetPrivateProfileString( _T("Skin_Description_Section"), _T("Name"), defskinname, sd->Name, SIZEOF( sd->Name ), fullName );
			_tcscpy( sd->File, fullName );
		}
		return AddItemToTree( GetDlgItem( hwndDlg, IDC_TREE1 ), fullName, sd->Name, sd );
	}
	return (HTREEITEM)-1;
}



HTREEITEM FindChild( HWND hTree, HTREEITEM Parent, TCHAR * Caption, void * data )
{
	HTREEITEM res = NULL, tmp = NULL;
	if ( Parent ) 
		tmp = TreeView_GetChild( hTree, Parent );
	else 
		tmp = TreeView_GetRoot( hTree );
	while ( tmp )
	{
		TVITEM tvi;
		TCHAR buf[255];
		tvi.hItem = tmp;
		tvi.mask = TVIF_TEXT|TVIF_HANDLE;
		tvi.pszText = (LPTSTR)&buf;
		tvi.cchTextMax = 254;
		TreeView_GetItem( hTree, &tvi );
		if ( _tcsicmp( Caption, tvi.pszText )==0 )
		{
			if ( data )
			{
				SkinListData * sd = NULL;
				TVITEM tvi = {0};
				tvi.hItem = tmp;
				tvi.mask = TVIF_HANDLE|TVIF_PARAM;
				TreeView_GetItem( hTree, &tvi );
				sd = ( SkinListData* )( tvi.lParam );
				if ( sd )
					if ( !_tcsicmp( sd->File, ( ( SkinListData* )data )->File ) )
						return tmp;
			}
			else
				return tmp;
		}
		tmp = TreeView_GetNextSibling( hTree, tmp );
	}
	return tmp;
}


HTREEITEM AddItemToTree( HWND hTree, TCHAR * folder, TCHAR * itemName, void * data )
{
	HTREEITEM rootItem = NULL;
	HTREEITEM cItem = NULL;
	TCHAR path[MAX_PATH];//, mask[MAX_PATH];
	TCHAR * ptr;
	TCHAR * ptrE;
	BOOL ext = FALSE;
	CallService( MS_UTILS_PATHTORELATIVET, ( WPARAM )folder, ( LPARAM )path );
	ptrE = path;
	while ( *ptrE != _T('\\') && *ptrE != _T('\0') && *ptrE != _T(':') ) ptrE++;
	if ( *ptrE == _T('\\') )
	{
		*ptrE = _T('\0');
		ptrE++;
	}
	else ptrE = path;
	ptr = ptrE;
	do 
	{

		while ( *ptrE!= _T('\\') && *ptrE!= _T('\0') ) ptrE++;
		if ( *ptrE == _T('\\') )
		{
			*ptrE = _T('\0');
			ptrE++;
			// find item if not - create;
			{
				cItem = FindChild( hTree, rootItem, ptr, NULL );
				if ( !cItem ) // not found - create node
				{
					TVINSERTSTRUCT tvis;
					tvis.hParent = rootItem;
					tvis.hInsertAfter = TVI_ROOT;
					tvis.item.mask = TVIF_PARAM|TVIF_TEXT|TVIF_PARAM;
					tvis.item.pszText = ptr;
					{
						tvis.item.lParam = ( LPARAM )NULL;
					}
					cItem = TreeView_InsertItem( hTree, &tvis );

				}	
				rootItem = cItem;
			}
			ptr = ptrE;
		}
		else ext = TRUE;
	}while ( !ext );
	//Insert item node
	cItem = FindChild( hTree, rootItem, itemName, data );
	if ( !cItem )
	{
		TVINSERTSTRUCT tvis;
		tvis.hParent = rootItem;
		tvis.hInsertAfter = TVI_SORT;
		tvis.item.mask = TVIF_PARAM|TVIF_TEXT|TVIF_PARAM;
		tvis.item.pszText = itemName;
		tvis.item.lParam = ( LPARAM )data;
		return TreeView_InsertItem( hTree, &tvis );
	}
	else
	{
		mir_free_and_nill( data ); //need to free otherwise memory leak
		return cItem;
	}
	return 0;
}













INT_PTR SvcActiveSkin(WPARAM wParam, LPARAM lParam)
{
	TCHAR *skinfile;
	TCHAR skinfull[MAX_PATH];
	skinfile = ModernGetStringT( NULL, SKIN, "SkinFile" );
	if ( skinfile )
	{
		CallService( MS_UTILS_PATHTOABSOLUTET, ( WPARAM )skinfile, ( LPARAM )skinfull );
		mir_free(skinfile);
		return (INT_PTR)mir_tstrdup(skinfull);
	}

	return NULL;
}

INT_PTR SvcApplySkin(WPARAM wParam, LPARAM lParam)
{
	ske_LoadSkinFromIniFile( (TCHAR *)lParam, FALSE );
	ske_LoadSkinFromDB( );	
	glOtherSkinWasLoaded = TRUE;
	pcli->pfnClcBroadcast( INTM_RELOADOPTIONS, 0, 0 );
	Sync( CLUIFrames_OnClistResize_mod, 0, 0 );
	ske_RedrawCompleteWindow( );        
	Sync( CLUIFrames_OnClistResize_mod, 0, 0 );
	{
		HWND hwnd = pcli->hwndContactList;
		RECT rc = {0};
		GetWindowRect( hwnd, &rc );
		Sync( CLUIFrames_OnMoving, hwnd, &rc );
	}
	if ( g_hCLUIOptionsWnd )
	{
		SendDlgItemMessage( g_hCLUIOptionsWnd, IDC_LEFTMARGINSPIN, UDM_SETPOS, 0, ModernGetSettingByte( NULL, "CLUI", "LeftClientMargin", SETTING_LEFTCLIENTMARIGN_DEFAULT ) );
		SendDlgItemMessage( g_hCLUIOptionsWnd, IDC_RIGHTMARGINSPIN, UDM_SETPOS, 0, ModernGetSettingByte( NULL, "CLUI", "RightClientMargin", SETTING_RIGHTCLIENTMARIGN_DEFAULT ) );
		SendDlgItemMessage( g_hCLUIOptionsWnd, IDC_TOPMARGINSPIN, UDM_SETPOS, 0, ModernGetSettingByte( NULL, "CLUI", "TopClientMargin", SETTING_TOPCLIENTMARIGN_DEFAULT ) );
		SendDlgItemMessage( g_hCLUIOptionsWnd, IDC_BOTTOMMARGINSPIN, UDM_SETPOS, 0, ModernGetSettingByte( NULL, "CLUI", "BottomClientMargin", SETTING_BOTTOMCLIENTMARIGN_DEFAULT ) );
	}
	return 0;
}

INT_PTR SvcPreviewSkin(WPARAM wParam, LPARAM lParam)
{
	DRAWITEMSTRUCT *dis = ( DRAWITEMSTRUCT * )wParam;

	HDC imgDC;
	HBITMAP imgOldbmp;
	int mWidth, mHeight;
	RECT workRect = {0};
	mWidth = dis->rcItem.right-dis->rcItem.left;
	mHeight = dis->rcItem.bottom-dis->rcItem.top;
	workRect = dis->rcItem;
	OffsetRect( &workRect, -workRect.left, -workRect.top );

	if (lParam)
	{
		TCHAR prfn[MAX_PATH] = {0};
		TCHAR imfn[MAX_PATH] = {0};
		TCHAR skinfolder[MAX_PATH] = {0};
		GetPrivateProfileString( _T( "Skin_Description_Section" ), _T( "Preview" ), _T( "" ), imfn, SIZEOF( imfn ), (LPCTSTR)lParam );
		IniParser::GetSkinFolder( (LPCTSTR)lParam, skinfolder );
		_sntprintf( prfn, SIZEOF( prfn ), _T("%s\\%s"), skinfolder, imfn );
		CallService( MS_UTILS_PATHTOABSOLUTET, ( WPARAM )prfn, ( LPARAM ) imfn );
		char * imfn_ch = mir_t2a( imfn ); 
		hPreviewBitmap = ske_LoadGlyphImage( imfn_ch );
		mir_free( imfn_ch );

		if ( hPreviewBitmap )
		{
			//variables
			BITMAP bmp = {0};
			POINT imgPos = {0};
			int wWidth, wHeight;
			int dWidth, dHeight;
			float xScale = 1, yScale = 1;
			//GetSize
			GetObject( hPreviewBitmap, sizeof( BITMAP ), &bmp );
			wWidth = workRect.right-workRect.left;
			wHeight = workRect.bottom-workRect.top;
			if ( wWidth<bmp.bmWidth ) xScale = ( float )wWidth/bmp.bmWidth;
			if ( wHeight<bmp.bmHeight ) yScale = ( float )wHeight/bmp.bmHeight;
			xScale = min( xScale, yScale );
			yScale = xScale;                    
			dWidth = ( int )( xScale*bmp.bmWidth );
			dHeight = ( int )( yScale*bmp.bmHeight );
			//CalcPosition
			imgPos.x = workRect.left+( ( wWidth-dWidth )>>1 );
			imgPos.y = workRect.top+( ( wHeight-dHeight )>>1 );     
			//DrawImage
			if ( !g_CluiData.fGDIPlusFail ) //Use gdi+ engine
			{
				DrawAvatarImageWithGDIp( dis->hDC, imgPos.x, imgPos.y, dWidth, dHeight, hPreviewBitmap, 0, 0, bmp.bmWidth, bmp.bmHeight, 8, 255 );
			}   
			else
			{
				BLENDFUNCTION bf = {AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };
				imgDC = CreateCompatibleDC( dis->hDC );
				imgOldbmp = ( HBITMAP )SelectObject( imgDC, hPreviewBitmap );                 
				ske_AlphaBlend( dis->hDC, imgPos.x, imgPos.y, dWidth, dHeight, imgDC, 0, 0, bmp.bmWidth, bmp.bmHeight, bf );
				SelectObject( imgDC, imgOldbmp );
				mod_DeleteDC( imgDC );
			}
			ske_UnloadGlyphImage(hPreviewBitmap);
		}
	}

	return 0;
}
