/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-14 Miranda NG project (http://miranda-ng.org),
Copyright (c) 2000-08 Miranda ICQ/IM project,
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
#include "hdr/modern_clist.h"
#include "hdr/modern_commonprototypes.h"
#include "hdr/modern_sync.h"

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

int SkinOptInit(WPARAM wParam, LPARAM lParam)
{
	if (!g_CluiData.fDisableSkinEngine) {
		//Tabbed settings
		OPTIONSDIALOGPAGE odp = { sizeof(odp) };
		odp.position = -200000000;
		odp.hInstance = g_hInst;
		odp.pfnDlgProc = DlgSkinOpts;
		odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_SKIN);
		odp.ptszGroup = LPGENT("Skins");
		odp.ptszTitle = LPGENT("Contact list");
		odp.flags = ODPF_BOLDGROUPS | ODPF_TCHAR;
		Options_AddPage(wParam, &odp);
	}
	return 0;
}

int ModernSkinOptInit(WPARAM wParam, LPARAM lParam)
{
	MODERNOPTOBJECT obj = {0};
	obj.cbSize = sizeof(obj);
	obj.dwFlags = MODEROPT_FLG_TCHAR;
	obj.hIcon = LoadSkinnedIcon(SKINICON_OTHER_MIRANDA);
	obj.hInstance = g_hInst;
	obj.iSection = MODERNOPT_PAGE_SKINS;
	obj.iType = MODERNOPT_TYPE_SELECTORPAGE;
	obj.lptzSubsection = _T("Contact list");
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
			TranslateDialogDefault(hwndDlg);
			SetWindowText(GetDlgItem(hwndDlg, IDC_SKINFOLDERLABEL), SkinsFolder);
			HTREEITEM it = FillAvailableSkinList(hwndDlg);
			HWND wnd = GetDlgItem(hwndDlg, IDC_TREE1);
			TreeView_SelectItem(wnd, it);
		}
		return 0;
	case WM_COMMAND:
		{
			int isLoad = 0;
			switch ( LOWORD(wParam )) {
			case IDC_COLOUR_MENUNORMAL:
			case IDC_COLOUR_MENUSELECTED:
			case IDC_COLOUR_FRAMES:
			case IDC_COLOUR_STATUSBAR:
				SendMessage( GetParent( hwndDlg ), PSM_CHANGED, 0, 0 );
				break;

			case IDC_BUTTON_INFO:
				{
					TCHAR Author[255], URL[MAX_PATH], Contact[255], Description[400], text[2000];
					SkinListData *sd = NULL;
					HTREEITEM hti = TreeView_GetSelection( GetDlgItem( hwndDlg, IDC_TREE1 ));
					if ( hti == 0 ) return 0;
					{
						TVITEM tvi = {0};
						tvi.hItem = hti;
						tvi.mask = TVIF_HANDLE|TVIF_PARAM;
						TreeView_GetItem( GetDlgItem( hwndDlg, IDC_TREE1 ), &tvi );
						sd = ( SkinListData* )( tvi.lParam);
					}
					if (!sd ) return 0;
					if ( sd->File && !_tcschr( sd->File, _T('%')))
					{
						GetPrivateProfileString( _T("Skin_Description_Section"), _T("Author"), 	TranslateT("( unknown )"), 	Author, 		SIZEOF( Author ), 		sd->File );
						GetPrivateProfileString( _T("Skin_Description_Section"), _T("URL"), 		_T(""), 						URL, 		SIZEOF( URL ), 		sd->File );
						GetPrivateProfileString( _T("Skin_Description_Section"), _T("Contact"), 	_T(""), 						Contact, 	SIZEOF( Contact ), 	sd->File );
						GetPrivateProfileString( _T("Skin_Description_Section"), _T("Description"), _T(""), 					Description, SIZEOF( Description ), sd->File );
						mir_sntprintf(text, SIZEOF(text), TranslateT("%s\n\n%s\n\nAuthor(s):\t %s\nContact:\t %s\nWeb:\t %s\n\nFile:\t %s"),
							sd->Name, Description, Author, Contact, URL, sd->File);
					}
					else
					{
						mir_sntprintf(text, SIZEOF(text), TranslateT("%s\n\n%s\n\nAuthor(s): %s\nContact:\t %s\nWeb:\t %s\n\nFile:\t %s"),
							TranslateT("reVista for Modern v0.5"),
							TranslateT("This is second default Modern Contact list skin in Vista Aero style"),
							TranslateT("Angeli-Ka (graphics), FYR (template)"),
							_T("JID: fyr@jabber.ru"),
							_T("fyr.mirandaim.ru"),
							TranslateT("Inside library"));
					}
					MessageBox( hwndDlg, text, TranslateT("Skin Information"), MB_OK|MB_ICONINFORMATION );
				}
				break;
			case IDC_BUTTON_APPLY_SKIN:
				if ( HIWORD(wParam ) == BN_CLICKED )
				{
					SkinListData *sd = NULL;
					HTREEITEM hti = TreeView_GetSelection( GetDlgItem( hwndDlg, IDC_TREE1 ));
					if ( hti == 0 ) return 0;
					{
						TVITEM tvi = {0};
						tvi.hItem = hti;
						tvi.mask = TVIF_HANDLE|TVIF_PARAM;
						TreeView_GetItem( GetDlgItem( hwndDlg, IDC_TREE1 ), &tvi );
						sd = ( SkinListData* )( tvi.lParam);
					}
					if (!sd ) return 0;
					ske_LoadSkinFromIniFile( sd->File, FALSE );
					ske_LoadSkinFromDB();
					pcli->pfnClcBroadcast(INTM_RELOADOPTIONS, 0, 0 );
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
						SendDlgItemMessage( g_hCLUIOptionsWnd, IDC_LEFTMARGINSPIN, UDM_SETPOS, 0, db_get_b( NULL, "CLUI", "LeftClientMargin", SETTING_LEFTCLIENTMARIGN_DEFAULT ));
						SendDlgItemMessage( g_hCLUIOptionsWnd, IDC_RIGHTMARGINSPIN, UDM_SETPOS, 0, db_get_b( NULL, "CLUI", "RightClientMargin", SETTING_RIGHTCLIENTMARIGN_DEFAULT ));
						SendDlgItemMessage( g_hCLUIOptionsWnd, IDC_TOPMARGINSPIN, UDM_SETPOS, 0, db_get_b( NULL, "CLUI", "TopClientMargin", SETTING_TOPCLIENTMARIGN_DEFAULT ));
						SendDlgItemMessage( g_hCLUIOptionsWnd, IDC_BOTTOMMARGINSPIN, UDM_SETPOS, 0, db_get_b( NULL, "CLUI", "BottomClientMargin", SETTING_BOTTOMCLIENTMARIGN_DEFAULT ));
					}
				}
				break;
			case IDC_BUTTON_RESCAN:
				if (HIWORD(wParam ) == BN_CLICKED)
				{
					HTREEITEM it = FillAvailableSkinList(hwndDlg);
					HWND wnd = GetDlgItem(hwndDlg, IDC_TREE1);
					TreeView_SelectItem(wnd, it);
				}
			}
			break;
		}
	case WM_DRAWITEM:
		if ( wParam == IDC_PREVIEW )
		{
			//TODO:Draw hPreviewBitmap here
			int mWidth, mHeight;
			HBRUSH hbr = CreateSolidBrush( GetSysColor( COLOR_3DFACE ));
			DRAWITEMSTRUCT *dis = ( DRAWITEMSTRUCT * )lParam;
			mWidth = dis->rcItem.right-dis->rcItem.left;
			mHeight = dis->rcItem.bottom-dis->rcItem.top;
			HDC memDC = CreateCompatibleDC( dis->hDC );
			HBITMAP hbmp = ske_CreateDIB32( mWidth, mHeight );
			HBITMAP holdbmp = ( HBITMAP )SelectObject( memDC, hbmp );
			RECT workRect = dis->rcItem;
			OffsetRect( &workRect, -workRect.left, -workRect.top );
			FillRect( memDC, &workRect, hbr );
			DeleteObject( hbr );
			if ( hPreviewBitmap )
			{
				//variables
				BITMAP bmp = {0};
				POINT imgPos = {0};
				float xScale = 1, yScale = 1;
				//GetSize
				GetObject( hPreviewBitmap, sizeof( BITMAP ), &bmp );
				int wWidth = workRect.right-workRect.left;
				int wHeight = workRect.bottom-workRect.top;
				if ( wWidth < bmp.bmWidth ) xScale = ( float )wWidth/bmp.bmWidth;
				if ( wHeight < bmp.bmHeight ) yScale = ( float )wHeight/bmp.bmHeight;
				xScale = min( xScale, yScale );
				yScale = xScale;
				int dWidth = ( int )( xScale*bmp.bmWidth );
				int dHeight = ( int )( yScale*bmp.bmHeight );
				//CalcPosition
				imgPos.x = workRect.left+(( wWidth-dWidth )>>1 );
				imgPos.y = workRect.top+(( wHeight-dHeight )>>1 );
				//DrawImage
				DrawAvatarImageWithGDIp( memDC, imgPos.x, imgPos.y, dWidth, dHeight, hPreviewBitmap, 0, 0, bmp.bmWidth, bmp.bmHeight, 8, 255 );
			}
			BitBlt( dis->hDC, dis->rcItem.left, dis->rcItem.top, mWidth, mHeight, memDC, 0, 0, SRCCOPY );
			SelectObject( memDC, holdbmp );
			DeleteObject( hbmp );
			DeleteDC( memDC );
		}
		break;

	case WM_NOTIFY:
		switch (( ( LPNMHDR )lParam)->idFrom ) {
		case IDC_TREE1:
			{
				NMTREEVIEW * nmtv = ( NMTREEVIEW * ) lParam;
				if (nmtv == NULL)
					return 0;

				if (nmtv->hdr.code == TVN_SELCHANGEDA || nmtv->hdr.code == TVN_SELCHANGEDW) {
					SkinListData * sd = NULL;
					if (hPreviewBitmap) {
						ske_UnloadGlyphImage( hPreviewBitmap );
						hPreviewBitmap = NULL;
					}

					if (nmtv->itemNew.lParam) {
						sd = ( SkinListData* )nmtv->itemNew.lParam;

						TCHAR buf[MAX_PATH];
						PathToRelativeT(sd->File, buf);
						SetDlgItemText(hwndDlg,IDC_EDIT_SKIN_FILENAME,buf);

						TCHAR prfn[MAX_PATH] = {0}, imfn[MAX_PATH] = {0}, skinfolder[MAX_PATH] = {0};
						GetPrivateProfileString( _T("Skin_Description_Section"), _T("Preview"), _T(""), imfn, SIZEOF( imfn ), sd->File );
						IniParser::GetSkinFolder( sd->File, skinfolder );
						mir_sntprintf(prfn, SIZEOF(prfn), _T("%s\\%s"), skinfolder, imfn);
						PathToAbsoluteT(prfn, imfn);
						hPreviewBitmap = ske_LoadGlyphImage(imfn);

						EnableWindow( GetDlgItem( hwndDlg, IDC_BUTTON_APPLY_SKIN ), TRUE );
						EnableWindow( GetDlgItem( hwndDlg, IDC_BUTTON_INFO ), TRUE );
						if ( hPreviewBitmap )
							InvalidateRect( GetDlgItem( hwndDlg, IDC_PREVIEW ), NULL, TRUE );
						else { //prepare text
							TCHAR Author[255], URL[MAX_PATH], Contact[255], Description[400], text[2000];
							SkinListData* sd = NULL;
							HTREEITEM hti = TreeView_GetSelection( GetDlgItem( hwndDlg, IDC_TREE1 ));
							if ( hti == 0 ) return 0;
							{
								TVITEM tvi = {0};
								tvi.hItem = hti;
								tvi.mask = TVIF_HANDLE|TVIF_PARAM;
								TreeView_GetItem( GetDlgItem( hwndDlg, IDC_TREE1 ), &tvi );
								sd = ( SkinListData* )( tvi.lParam);
							}
							if (!sd ) return 0;

							if ( sd->File && !_tcschr( sd->File, _T('%')))
							{
								GetPrivateProfileString( _T("Skin_Description_Section"), _T("Author"), 	TranslateT("( unknown )"), 	Author, 		SIZEOF( Author ), 		sd->File );
								GetPrivateProfileString( _T("Skin_Description_Section"), _T("URL"), 		_T(""), 						URL, 		SIZEOF( URL ), 		sd->File );
								GetPrivateProfileString( _T("Skin_Description_Section"), _T("Contact"), 	_T(""), 						Contact, 	SIZEOF( Contact ), 	sd->File );
								GetPrivateProfileString( _T("Skin_Description_Section"), _T("Description"), _T(""), 					Description, SIZEOF( Description ), sd->File );
								mir_sntprintf(text, SIZEOF(text), TranslateT("Preview is not available\n\n%s\n----------------------\n\n%s\n\nAUTHOR(S):\n%s\n\nCONTACT:\n%s\n\nHOMEPAGE:\n%s"),
									sd->Name, Description, Author, Contact, URL);
							}
							else
							{
								mir_sntprintf(text, SIZEOF(text), TranslateT("%s\n\n%s\n\nAUTHORS:\n%s\n\nCONTACT:\n%s\n\nWEB:\n%s\n\n\n"),
									TranslateT("reVista for Modern v0.5"),
									TranslateT("This is second default Modern Contact list skin in Vista Aero style"),
									TranslateT("graphics by Angeli-Ka\ntemplate by FYR"),
									_T("JID: fyr@jabber.ru"),
									_T("fyr.mirandaim.ru"));
							}
							ShowWindow( GetDlgItem( hwndDlg, IDC_PREVIEW ), SW_HIDE );
							ShowWindow( GetDlgItem( hwndDlg, IDC_STATIC_INFO ), SW_SHOW );
							SetDlgItemText(hwndDlg, IDC_STATIC_INFO, text);
						}
					}
					else {
						//no selected
						SetDlgItemText(hwndDlg, IDC_EDIT_SKIN_FILENAME, TranslateT("Select skin from list"));
						EnableWindow( GetDlgItem( hwndDlg, IDC_BUTTON_APPLY_SKIN ), FALSE );
						EnableWindow( GetDlgItem( hwndDlg, IDC_BUTTON_INFO ), FALSE );
						SetDlgItemText(hwndDlg, IDC_STATIC_INFO, TranslateT("Please select skin to apply"));
						ShowWindow( GetDlgItem( hwndDlg, IDC_PREVIEW ), SW_HIDE );
					}
					ShowWindow( GetDlgItem( hwndDlg, IDC_PREVIEW ), hPreviewBitmap?SW_SHOW:SW_HIDE );
					return 0;
				}
				else if (nmtv->hdr.code == TVN_DELETEITEMA || nmtv->hdr.code == TVN_DELETEITEMW) {
					mir_free_and_nil( nmtv->itemOld.lParam);
					return 0;
				}
			}
			break;

		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_APPLY:
				pcli->pfnClcBroadcast(INTM_RELOADOPTIONS, 0, 0 );
				NotifyEventHooks( g_CluiData.hEventBkgrChanged, 0, 0 );
				pcli->pfnClcBroadcast(INTM_INVALIDATE, 0, 0 );
				RedrawWindow( GetParent( pcli->hwndContactTree ), NULL, NULL, RDW_INVALIDATE|RDW_FRAME|RDW_ALLCHILDREN );
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
	mir_sntprintf(mask, SIZEOF(mask), _T("%s\\*.msf"), Folder);
	//fd.attrib = _A_SUBDIR;
	hFile = _tfindfirst( mask, &fd );
	if ( hFile != -1 )
	{
		do {
			AddSkinToList( hwndDlg, Folder, fd.name );
		}
			while (!_tfindnext( hFile, &fd ));
		_findclose( hFile );
	}
	mir_sntprintf(mask, SIZEOF(mask), _T("%s\\*"), Folder);
	hFile = _tfindfirst( mask, &fd );
	{
		do {
			if ( fd.attrib&_A_SUBDIR && !( _tcsicmp( fd.name, _T("."))  == 0  || _tcsicmp( fd.name, _T("..")) == 0 ))
			{//Next level of subfolders
				TCHAR path[MAX_PATH];
				mir_sntprintf(path, SIZEOF(path), _T("%s\\%s"), Folder, fd.name);
				SearchSkinFiles( hwndDlg, path );
			}
		}while (!_tfindnext( hFile, &fd ));
		_findclose( hFile );
	}
	return 0;
}

HTREEITEM FillAvailableSkinList(HWND hwndDlg)
{
	struct _finddata_t fd = {0};
	//long hFile;
	HTREEITEM res = (HTREEITEM)-1;
	int attrib;

	TreeView_DeleteAllItems(GetDlgItem(hwndDlg, IDC_TREE1));
	AddSkinToList(hwndDlg, TranslateT("Default Skin"), _T("%Default Skin%"));
	attrib = GetFileAttributes(SkinsFolder);
	if (attrib != INVALID_FILE_ATTRIBUTES && (attrib & FILE_ATTRIBUTE_DIRECTORY))
		SearchSkinFiles(hwndDlg, SkinsFolder);
	{
		TCHAR skinfull[MAX_PATH];
		ptrT skinfile( db_get_tsa( NULL, SKIN, "SkinFile"));
		if (skinfile) {
			PathToAbsoluteT(skinfile, skinfull);
			res = AddSkinToListFullName(hwndDlg, skinfull);
		}
	}
	return res;
}
HTREEITEM AddSkinToListFullName( HWND hwndDlg, TCHAR * fullName )
{
	TCHAR path[MAX_PATH] = {0};
	TCHAR file[MAX_PATH] = {0};
	TCHAR *buf;
	_tcsncpy( path, fullName, SIZEOF( path ));
	buf = path + _tcslen( path );
	while ( buf > path )
	{
		if ( *buf == _T( '\\' ))
		{
			*buf = _T( '\0' );
			break;
		}
		buf--;
	}
	buf++;
	_tcsncpy( file, buf, SIZEOF( file ));
	return AddSkinToList( hwndDlg, path, file );
}


HTREEITEM AddSkinToList( HWND hwndDlg, TCHAR * path, TCHAR* file )
{
	TCHAR fullName[MAX_PATH], defskinname[MAX_PATH];
	SkinListData *sd = ( SkinListData * )mir_alloc( sizeof( SkinListData ));
	if (!sd )
		return 0;

	if (!file || _tcschr( file, _T('%'))) {
		mir_sntprintf(sd->File, SIZEOF(sd->File), _T("%%Default Skin%%"));
		mir_sntprintf(sd->Name, SIZEOF(sd->Name), TranslateT("%Default Skin%"));
		_tcsncpy(fullName, TranslateT("Default Skin"), SIZEOF(fullName));
	}
	else {
		mir_sntprintf(fullName, SIZEOF(fullName), _T("%s\\%s"), path, file);
		memcpy(defskinname, file, (_tcslen( file )-4) * sizeof(TCHAR));
		defskinname[_tcslen( file )+1] = _T('\0');
		GetPrivateProfileString( _T("Skin_Description_Section"), _T("Name"), defskinname, sd->Name, SIZEOF( sd->Name ), fullName );
		_tcscpy(sd->File, fullName);
	}
	return AddItemToTree( GetDlgItem( hwndDlg, IDC_TREE1 ), fullName, sd->Name, sd );
}

HTREEITEM FindChild( HWND hTree, HTREEITEM Parent, TCHAR * Caption, void * data )
{
	HTREEITEM res = NULL, tmp = NULL;
	if ( Parent )
		tmp = TreeView_GetChild( hTree, Parent );
	else
		tmp = TreeView_GetRoot( hTree );

	while (tmp) {
		TVITEM tvi;
		TCHAR buf[255];
		tvi.hItem = tmp;
		tvi.mask = TVIF_TEXT|TVIF_HANDLE;
		tvi.pszText = buf;
		tvi.cchTextMax = SIZEOF(buf);
		TreeView_GetItem( hTree, &tvi );
		if ( _tcsicmp(Caption, tvi.pszText) == 0) {
			if (!data)
				return tmp;

			TVITEM tvi = {0};
			tvi.hItem = tmp;
			tvi.mask = TVIF_HANDLE|TVIF_PARAM;
			TreeView_GetItem(hTree, &tvi);
			SkinListData *sd = (SkinListData*)tvi.lParam;
			if (sd)
				if (!_tcsicmp(sd->File, (( SkinListData* )data )->File))
					return tmp;
		}
		tmp = TreeView_GetNextSibling(hTree, tmp);
	}
	return tmp;
}

HTREEITEM AddItemToTree(HWND hTree, TCHAR *folder, TCHAR *itemName, void *data)
{
	HTREEITEM cItem = NULL;
	//Insert item node
	cItem = FindChild( hTree, 0, itemName, data );
	if (!cItem) {
		TVINSERTSTRUCT tvis = {0};
		tvis.hInsertAfter = TVI_SORT;
		tvis.item.mask = TVIF_PARAM | TVIF_TEXT | TVIF_PARAM;
		tvis.item.pszText = itemName;
		tvis.item.lParam = (LPARAM)data;
		return TreeView_InsertItem(hTree, &tvis);
	}

	mir_free(data); //need to free otherwise memory leak
	return cItem;
}

INT_PTR SvcActiveSkin(WPARAM wParam, LPARAM lParam)
{
	ptrT skinfile( db_get_tsa(NULL, SKIN, "SkinFile"));
	if (skinfile) {
		TCHAR skinfull[MAX_PATH];
		PathToAbsoluteT(skinfile, skinfull);
		return (INT_PTR)mir_tstrdup(skinfull);
	}

	return NULL;
}

INT_PTR SvcApplySkin(WPARAM wParam, LPARAM lParam)
{
	ske_LoadSkinFromIniFile((TCHAR *)lParam, FALSE);
	ske_LoadSkinFromDB( );
	pcli->pfnClcBroadcast(INTM_RELOADOPTIONS, 0, 0 );
	Sync( CLUIFrames_OnClistResize_mod, 0, 0 );
	ske_RedrawCompleteWindow( );
	Sync( CLUIFrames_OnClistResize_mod, 0, 0 );

	HWND hwnd = pcli->hwndContactList;
	RECT rc = {0};
	GetWindowRect( hwnd, &rc );
	Sync( CLUIFrames_OnMoving, hwnd, &rc );

	if (g_hCLUIOptionsWnd) {
		SendDlgItemMessage( g_hCLUIOptionsWnd, IDC_LEFTMARGINSPIN, UDM_SETPOS, 0, db_get_b( NULL, "CLUI", "LeftClientMargin", SETTING_LEFTCLIENTMARIGN_DEFAULT ));
		SendDlgItemMessage( g_hCLUIOptionsWnd, IDC_RIGHTMARGINSPIN, UDM_SETPOS, 0, db_get_b( NULL, "CLUI", "RightClientMargin", SETTING_RIGHTCLIENTMARIGN_DEFAULT ));
		SendDlgItemMessage( g_hCLUIOptionsWnd, IDC_TOPMARGINSPIN, UDM_SETPOS, 0, db_get_b( NULL, "CLUI", "TopClientMargin", SETTING_TOPCLIENTMARIGN_DEFAULT ));
		SendDlgItemMessage( g_hCLUIOptionsWnd, IDC_BOTTOMMARGINSPIN, UDM_SETPOS, 0, db_get_b( NULL, "CLUI", "BottomClientMargin", SETTING_BOTTOMCLIENTMARIGN_DEFAULT ));
	}
	return 0;
}

INT_PTR SvcPreviewSkin(WPARAM wParam, LPARAM lParam)
{
	DRAWITEMSTRUCT *dis = ( DRAWITEMSTRUCT * )wParam;
	int mWidth = dis->rcItem.right-dis->rcItem.left;
	int mHeight = dis->rcItem.bottom-dis->rcItem.top;
	RECT workRect = dis->rcItem;
	OffsetRect( &workRect, -workRect.left, -workRect.top );

	if (lParam) {
		TCHAR prfn[MAX_PATH] = {0};
		TCHAR imfn[MAX_PATH] = {0};
		TCHAR skinfolder[MAX_PATH] = {0};
		GetPrivateProfileString( _T("Skin_Description_Section"), _T("Preview"), _T(""), imfn, SIZEOF( imfn ), (LPCTSTR)lParam);
		IniParser::GetSkinFolder((LPCTSTR)lParam, skinfolder );
		mir_sntprintf(prfn, SIZEOF(prfn), _T("%s\\%s"), skinfolder, imfn);
		PathToAbsoluteT(prfn, imfn);

		hPreviewBitmap = ske_LoadGlyphImage(imfn);
		if (hPreviewBitmap) {
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
			if ( wWidth < bmp.bmWidth ) xScale = ( float )wWidth/bmp.bmWidth;
			if ( wHeight < bmp.bmHeight ) yScale = ( float )wHeight/bmp.bmHeight;
			xScale = min( xScale, yScale );
			yScale = xScale;
			dWidth = ( int )( xScale*bmp.bmWidth );
			dHeight = ( int )( yScale*bmp.bmHeight );
			//CalcPosition
			imgPos.x = workRect.left+(( wWidth-dWidth )>>1 );
			imgPos.y = workRect.top+(( wHeight-dHeight )>>1 );
			//DrawImage
			DrawAvatarImageWithGDIp( dis->hDC, imgPos.x, imgPos.y, dWidth, dHeight, hPreviewBitmap, 0, 0, bmp.bmWidth, bmp.bmHeight, 8, 255 );
			ske_UnloadGlyphImage(hPreviewBitmap);
		}
	}

	return 0;
}
