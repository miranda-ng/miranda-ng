/*

WinPopup Protocol plugin for Miranda IM.

Copyright (C) 2004-2011 Nikolay Raspopov

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

#include "stdafx.h"

#define WM_FILLTREE (WM_USER+75)

typedef struct _FillTreeThreadData
{
	HMODULE				hContact;
	HWND				hwndDlg;
} FillTreeThreadData;

typedef struct _FillTreeData
{
	netbios_name_list	nns;
	CString		host;
	CString		about;
} FillTreeData;

typedef struct _DlgDataUserInfo
{
	HMODULE				hContact;
	HIMAGELIST			hTreeImages;
	bool				bWorking;
} DlgDataUserInfo;

static void FillTreeThread (LPVOID param)
{
	FillTreeThreadData* fttd = (FillTreeThreadData*)param;

	// Получение имени контакта
	CString sNick = GetNick( fttd->hContact );
	if ( ! sNick.IsEmpty() )
	{
		bool bGroup = IsGroup( fttd->hContact );

		// Опрос хоста
		if ( FillTreeData* ftd = new FillTreeData )
		{
			ftd->host = sNick;

			if ( ! bGroup )
				// Запрос NetBIOS-имён
				pluginNetBIOS.GetNames( ftd->nns, ftd->host, false );

			// Запрос комментария
			DWORD buf_size = 4096;
			if ( NETRESOURCE* buf = (NETRESOURCE*)mir_alloc( buf_size ) )
			{
				CString remote( _T("\\\\") );
				if ( bGroup )
					remote =  (LPCTSTR)ftd->host;
				else
					remote += (LPCTSTR)ftd->host;

				NETRESOURCE nr = {};
				nr.dwScope = RESOURCE_GLOBALNET;
				nr.lpRemoteName = const_cast <LPTSTR>(static_cast <LPCTSTR>(remote));
				LPTSTR sys = NULL;
				if ( WNetGetResourceInformation( &nr, buf, &buf_size, &sys ) == NO_ERROR )
				{
					ftd->about = buf->lpComment;
					db_set_ts( fttd->hContact, modname,
						"About", ftd->about );
				}

				mir_free( buf );
			}

			// ...и уведомление о готовности данных
			if ( ! IsWindow( fttd->hwndDlg ) ||
				 ! PostMessage( fttd->hwndDlg, WM_FILLTREE, 0, reinterpret_cast< LPARAM >( ftd ) ) )
				delete ftd;
		}
	}

	mir_free( fttd );
}

static INT_PTR CALLBACK DlgProcUserInfo (HWND hwndDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	DlgDataUserInfo* data = reinterpret_cast <DlgDataUserInfo*> (GetWindowLongPtr(hwndDlg, DWLP_USER));

	switch (Msg)
	{
		case WM_INITDIALOG:
		{
			TranslateDialogDefault (hwndDlg);

			data = (DlgDataUserInfo*)mir_alloc( sizeof( DlgDataUserInfo ) );
			if ( ! data )
				return FALSE;

			SetWindowLongPtr (hwndDlg, DWLP_USER, reinterpret_cast <LONG> (data));
			data->hTreeImages = ImageList_Create (16, 16, ILC_COLOR8 | ILC_MASK, 5, 0);
			data->bWorking = false;
			
			ImageList_AddIcon (data->hTreeImages, reinterpret_cast <HICON> (LoadImage (	// 0
				pluginModule, MAKEINTRESOURCE (IDI_COMPUTER), IMAGE_ICON, 16, 16, LR_SHARED )));

			ImageList_AddIcon (data->hTreeImages, reinterpret_cast <HICON> (LoadImage (	// 1
				pluginModule, MAKEINTRESOURCE (IDI_GOOD_NAME), IMAGE_ICON, 16, 16, LR_SHARED )));
			
			ImageList_AddIcon (data->hTreeImages, reinterpret_cast <HICON> (LoadImage (	// 2
				pluginModule, MAKEINTRESOURCE (IDI_GOOD_NAMES), IMAGE_ICON, 16, 16, LR_SHARED )));
			
			ImageList_AddIcon (data->hTreeImages, reinterpret_cast <HICON> (LoadImage (	// 3
				pluginModule, MAKEINTRESOURCE (IDI_LANA), IMAGE_ICON, 16, 16, LR_SHARED )));

			ImageList_AddIcon (data->hTreeImages, reinterpret_cast <HICON> (LoadImage (	// 4
				pluginModule, MAKEINTRESOURCE (IDI_COMPUTER_ERROR), IMAGE_ICON, 16, 16, LR_SHARED )));
		
			TreeView_SetImageList (GetDlgItem (hwndDlg, IDC_TREE), data->hTreeImages, TVSIL_NORMAL);

			return TRUE;
		}

		case WM_DESTROY:
		{
			SetWindowLongPtr (hwndDlg, DWLP_USER, NULL);
			if (data) {
				ImageList_Destroy (data->hTreeImages);
				mir_free( data );
			}
			break;
		}

		case WM_FILLTREE:
			// Заполнение дерева имён...
			if ( FillTreeData* ftd = reinterpret_cast <FillTreeData*> (lParam) )
			{
				if ( data )
				{
					// Заполнение дерева имён
					HWND hTree = GetDlgItem (hwndDlg, IDC_TREE);
					TreeView_DeleteAllItems (hTree);
					TVINSERTSTRUCT tvis = { 0 };
					tvis.hParent = TVI_ROOT;
					tvis.hInsertAfter = TVI_LAST;
					tvis.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
					tvis.item.pszText = const_cast <LPTSTR>(static_cast<LPCTSTR>(ftd->host));
					tvis.item.iImage = tvis.item.iSelectedImage =
						IsGroup( data->hContact ) ? 2 :
						( ftd->nns.GetCount() ? 0 : 4 );
					tvis.hParent = TreeView_InsertItem (hTree, &tvis);
					if ( ftd->nns.GetCount() )
					{
						for (POSITION pos = ftd->nns.GetHeadPosition (); pos;)
						{
							netbios_name& nname = ftd->nns.GetNext (pos);
							CA2T textT( nname.GetANSIFullName() );
							tvis.item.pszText = (LPTSTR) (LPCTSTR) textT;
							tvis.item.iImage = tvis.item.iSelectedImage = (nname.IsGroupName () ? 2 : 1);
							TreeView_InsertItem (hTree, &tvis);
						}
					}
					TreeView_Expand (hTree, tvis.hParent, TVE_EXPAND);

					SetDlgItemText (hwndDlg, IDC_ABOUT, ftd->about);

					data->bWorking = false;
				}
				delete ftd;
			}
			return TRUE;

        case WM_NOTIFY:
		{
			LPPSHNOTIFY lpHdr = reinterpret_cast <LPPSHNOTIFY> (lParam);
			if (lpHdr->hdr.idFrom == 0)
			{
				// Сохранение контакта на будущее
				data->hContact = reinterpret_cast <HMODULE> (lpHdr->lParam);
				switch (lpHdr->hdr.code)
				{
					case PSN_INFOCHANGED:
					{
						// Флаг "Always Online"
						BOOL b = db_get_b ( data->hContact, modname,
							"AlwaysOnline", FALSE );
						CheckDlgButton (hwndDlg, IDC_ONLINE_CHECK,
							(UINT)( b ? BST_CHECKED : BST_UNCHECKED ) );
						EnableWindow (GetDlgItem (hwndDlg, IDC_ONLINE_CHECK),
							data->hContact != NULL );

						// Флаг "Legacy online status detection" 
						CheckDlgButton( hwndDlg, IDC_CHECK00FORONLINE,(UINT)(
							IsLegacyOnline( data->hContact ) ? BST_CHECKED : BST_UNCHECKED ) );
						EnableWindow( GetDlgItem( hwndDlg, IDC_CHECK00FORONLINE ),
							data->hContact != NULL );

						// Флаг "Group Contact"
						CheckDlgButton (hwndDlg, IDC_GROUP, (UINT)(
							IsGroup( data->hContact ) ? BST_CHECKED : BST_UNCHECKED ) );
						EnableWindow( GetDlgItem( hwndDlg, IDC_GROUP ),
							data->hContact != NULL );

						// Запуск опроса NetBIOS-имён хоста
						if ( data && ! data->bWorking )
						{
							HWND hTree = GetDlgItem (hwndDlg, IDC_TREE);
							data->bWorking = true;
							TreeView_DeleteAllItems (hTree);
							TVINSERTSTRUCT tvis = { 0 };
							tvis.hParent = TVI_ROOT;
							tvis.hInsertAfter = TVI_LAST;
							tvis.item.mask = TVIF_TEXT;
							tvis.item.pszText = TranslateT ("Retrieving...");
							TreeView_InsertItem (hTree, &tvis);

							SetDlgItemText (hwndDlg, IDC_ABOUT, _T(""));

							if ( FillTreeThreadData* fttd = (FillTreeThreadData*)mir_alloc( sizeof( FillTreeThreadData ) ) )
							{
								fttd->hContact = data->hContact;
								fttd->hwndDlg = hwndDlg;
								mir_forkthread( FillTreeThread, fttd );
							}
						}
						break;
					}

					case PSN_KILLACTIVE:
						SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, FALSE);
						return TRUE;

					case PSN_APPLY:
					{
						BOOL f_now = (IsDlgButtonChecked (hwndDlg, IDC_ONLINE_CHECK) ==
							BST_CHECKED) ? TRUE : FALSE;
						BOOL f_old = db_get_b ( data->hContact, modname,
							"AlwaysOnline", FALSE );
						db_set_b (data->hContact, modname,
							"AlwaysOnline", (BYTE)( f_now ? TRUE : FALSE ) );
						if ( ! f_old && f_now )
							SetContactStatus( data->hContact, ID_STATUS_ONLINE, true );
						else if ( f_old && ! f_now )
							SetContactStatus( data->hContact, ID_STATUS_OFFLINE, true );

						SetLegacyOnline( data->hContact, ( IsDlgButtonChecked( hwndDlg,
							IDC_CHECK00FORONLINE ) == BST_CHECKED ) );

						SetGroup( data->hContact,
							IsDlgButtonChecked( hwndDlg, IDC_GROUP ) == BST_CHECKED );

						SetWindowLongPtr( hwndDlg, DWLP_MSGRESULT, PSNRET_NOERROR );
                        return TRUE;
					}
                }
			}
			break;
		}

		case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
				case IDC_ONLINE_CHECK:
				{
					BOOL f_now = (IsDlgButtonChecked (hwndDlg, IDC_ONLINE_CHECK) ==
						BST_CHECKED) ? TRUE : FALSE;
					BOOL f_old = db_get_b ( data->hContact, modname,
						"AlwaysOnline", FALSE );

					if ( f_old != f_now )
						PropSheet_Changed (GetParent (hwndDlg), hwndDlg);
					else
						PropSheet_UnChanged (GetParent (hwndDlg), hwndDlg);
						
					return TRUE;
				}

				case IDC_CHECK00FORONLINE:
				{
					PropSheet_Changed (GetParent (hwndDlg), hwndDlg);
					return TRUE;
				}

				case IDC_GROUP:
				{
					PropSheet_Changed (GetParent (hwndDlg), hwndDlg);
					return TRUE;
				}
			}
			break;
		}
	}
	return FALSE;
}

int __cdecl USERINFO_INITIALISE (WPARAM wParam, LPARAM lParam)
{
	HANDLE hContact = (HANDLE)lParam;
	if ( ! hContact || ( IsMyContact( hContact ) /*&& ! IsChatRoom( hContact )*/ ) )
	{
		OPTIONSDIALOGPAGE odp = {0};
		odp.cbSize = sizeof(odp);
		odp.hInstance = pluginModule;
		odp.pszTemplate = MAKEINTRESOURCEA( IDD_USERINFO );
		odp.ptszTitle = LPGENT(modtitle);
		odp.flags = ODPF_TCHAR;
		odp.pfnDlgProc = DlgProcUserInfo;
		Options_AddPage(wParam, &odp);
	}
    return 0;
}
