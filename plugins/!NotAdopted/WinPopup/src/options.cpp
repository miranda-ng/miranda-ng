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

static netbios_name_list	nns;
static HWND					hTree = NULL;

typedef struct _DlgDataOptions
{
	HMODULE				hContact;
	HIMAGELIST			hTreeImages;
	bool				need_restart;
} DlgDataOptions;

static void FillTreeThread (LPVOID param)
{
	// Ожидание инициализации NetBIOS (20 секунд)
	for ( int i = 0; i < 20 && ! pluginNetBIOS && IsWindow( hTree ); i++ )
		Sleep( 1000 );

	if ( IsWindow( hTree ) )
	{
		// ...вначале своими именами
		pluginNetBIOS.GetRegisteredNames( nns );
		// ...потом остальными именами
		pluginNetBIOS.GetNames( nns, pluginMachineName, false );
		// ...и уведомление о готовности данных
		PostMessage( reinterpret_cast <HWND>(param), WM_FILLTREE, 0, 0 );
	}
}

static void Refresh (HWND hwndDlg, HWND hwndTree)
{
	bool bOnline = ( pluginCurrentStatus != ID_STATUS_OFFLINE );

	nns.RemoveAll ();

	// Заполнение дерева имён...
	TreeView_DeleteAllItems (hwndTree);
	TVINSERTSTRUCT tvis = { TVI_ROOT, TVI_LAST };
	tvis.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
	tvis.item.iImage = tvis.item.iSelectedImage = bOnline ? 0 : 8;
	tvis.item.pszText = bOnline ? TranslateT("Retrieving...") : TranslateT("Offline");
	TreeView_InsertItem (hwndTree, &tvis);

	if ( bOnline )
	{
		// Запуск опроса хоста
		mir_forkthread( FillTreeThread, hwndDlg );
	}
}

static INT_PTR CALLBACK DlgProcOptions (HWND hwndDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	static bool bLastOnline = false;
	DlgDataOptions* data = reinterpret_cast <DlgDataOptions*> (GetWindowLongPtr (hwndDlg, DWLP_USER));

	switch ( Msg )
	{
		case WM_INITDIALOG:
		{
			TranslateDialogDefault (hwndDlg);

			data = (DlgDataOptions*)mir_alloc( sizeof( DlgDataOptions ) );
			if ( ! data )
				return FALSE;

			SetWindowLongPtr (hwndDlg, DWLP_USER, reinterpret_cast <LONG> (data));
			data->need_restart = false;
			data->hTreeImages = ImageList_Create (16, 16, ILC_COLOR8 | ILC_MASK, 8, 0);

			hTree = GetDlgItem (hwndDlg, IDC_TREE);
			
			ImageList_AddIcon (data->hTreeImages, reinterpret_cast <HICON> (LoadImage (	// 0
				pluginModule, MAKEINTRESOURCE (IDI_COMPUTER), IMAGE_ICON, 16, 16, LR_SHARED )));
			
			ImageList_AddIcon (data->hTreeImages, reinterpret_cast <HICON> (LoadImage (	// 1
				pluginModule, MAKEINTRESOURCE (IDI_LANA), IMAGE_ICON, 16, 16, LR_SHARED )));
			
			HICON hIcon;
			hIcon = reinterpret_cast <HICON> (LoadImage (
				pluginModule, MAKEINTRESOURCE (IDI_GOOD_NAME), IMAGE_ICON, 16, 16, LR_SHARED ));
			ImageList_AddIcon (data->hTreeImages, hIcon);									// 2
			SendDlgItemMessage (hwndDlg, IDC_LEGEND_1, STM_SETICON,
				 reinterpret_cast <WPARAM> (hIcon), 0);
			
			hIcon = reinterpret_cast <HICON> (LoadImage (
				pluginModule, MAKEINTRESOURCE (IDI_BAD_NAME), IMAGE_ICON, 16, 16, LR_SHARED ));
			ImageList_AddIcon (data->hTreeImages, hIcon);									// 3
			SendDlgItemMessage (hwndDlg, IDC_LEGEND_2, STM_SETICON,
				 reinterpret_cast <WPARAM> (hIcon), 0);
			
			hIcon = reinterpret_cast <HICON> (LoadImage (
				pluginModule, MAKEINTRESOURCE (IDI_OTHER_NAME), IMAGE_ICON, 16, 16, LR_SHARED ));
			ImageList_AddIcon (data->hTreeImages, hIcon);									// 4
			SendDlgItemMessage (hwndDlg, IDC_LEGEND_3, STM_SETICON,
				 reinterpret_cast <WPARAM> (hIcon), 0);
			
			hIcon = reinterpret_cast <HICON> (LoadImage (
				pluginModule, MAKEINTRESOURCE (IDI_GOOD_NAMES), IMAGE_ICON, 16, 16, LR_SHARED ));
			ImageList_AddIcon (data->hTreeImages, hIcon);									// 5
			SendDlgItemMessage (hwndDlg, IDC_LEGEND_4, STM_SETICON,
				 reinterpret_cast <WPARAM> (hIcon), 0);
			
			hIcon = reinterpret_cast <HICON> (LoadImage (
				pluginModule, MAKEINTRESOURCE (IDI_BAD_NAMES), IMAGE_ICON, 16, 16, LR_SHARED ));
			ImageList_AddIcon (data->hTreeImages, hIcon);									// 6
			SendDlgItemMessage (hwndDlg, IDC_LEGEND_5, STM_SETICON,
				 reinterpret_cast <WPARAM> (hIcon), 0);
			
			hIcon = reinterpret_cast <HICON> (LoadImage (
				pluginModule, MAKEINTRESOURCE (IDI_OTHER_NAMES), IMAGE_ICON, 16, 16, LR_SHARED ));
			ImageList_AddIcon (data->hTreeImages, hIcon);									// 7
			SendDlgItemMessage (hwndDlg, IDC_LEGEND_6, STM_SETICON,
				 reinterpret_cast <WPARAM> (hIcon), 0);

			hIcon = reinterpret_cast <HICON> (LoadImage (
				pluginModule, MAKEINTRESOURCE (IDI_COMPUTER_ERROR), IMAGE_ICON, 16, 16, LR_SHARED ));
			ImageList_AddIcon (data->hTreeImages, hIcon);									// 8
			
			TreeView_SetImageList (hTree, data->hTreeImages, TVSIL_NORMAL);

			BOOL b = db_get_b (NULL, modname, "RegisterNick", TRUE);
			CheckDlgButton (hwndDlg, IDC_CHECK_NICK, (UINT)( b ? BST_CHECKED : BST_UNCHECKED ));
			EnableWindow (GetDlgItem (hwndDlg, IDC_NICK1), b);
			EnableWindow (GetDlgItem (hwndDlg, IDC_NICK2), b);
			
			b = db_get_b (NULL, modname, "RegisterUser", TRUE);
			CheckDlgButton (hwndDlg, IDC_CHECK_USER, (UINT)( b ? BST_CHECKED : BST_UNCHECKED ));
			EnableWindow (GetDlgItem (hwndDlg, IDC_USER), b);

			CheckDlgButton (hwndDlg, IDC_AUTOANSWER,
				(UINT)( db_get_b (NULL, modname, "Auto-answer", FALSE) ?
				BST_CHECKED : BST_UNCHECKED ) );
			CheckDlgButton (hwndDlg, IDC_DUPS,
				(UINT)( db_get_b (NULL, modname, "Filter-dups", TRUE) ?
				BST_CHECKED : BST_UNCHECKED ) );
			CheckDlgButton (hwndDlg, IDC_ALWAYSCHECK00FORONLINE,
				(UINT)( IsLegacyOnline( NULL ) ? BST_CHECKED : BST_UNCHECKED ) );
			
			BYTE method = (BYTE) db_get_b (NULL, modname, "SendMethod", 0);
			CheckRadioButton (hwndDlg, IDC_USE_MAILSLOT, IDC_USE_NETSEND,
				IDC_USE_MAILSLOT + method);
			EnableWindow (GetDlgItem (hwndDlg, IDC_USE_NETSEND), (fnNetMessageBufferSend != NULL));
			
			CString sMyNick = GetNick( NULL );
			if ( ! sMyNick.IsEmpty() )
			{
				netbios_name nname (sMyNick, 3);
				SetDlgItemText (hwndDlg, IDC_NICK1, CA2T( nname.GetANSIFullName() ) );
			}

			if ( ! sMyNick.IsEmpty() )
			{
				netbios_name nname (sMyNick, 1);
				SetDlgItemText (hwndDlg, IDC_NICK2, CA2T( nname.GetANSIFullName() ) );
			}

			DBVARIANT dbv = {};
			if ( ! db_get_ts( NULL, modname, "User", &dbv ) )
			{
				netbios_name nname (dbv.ptszVal, 3);
				SetDlgItemText (hwndDlg, IDC_USER, CA2T( nname.GetANSIFullName() ) );
				db_free (&dbv);
			}

			bLastOnline = ! ( pluginCurrentStatus != ID_STATUS_OFFLINE );

			SetTimer( hwndDlg, 55, 500, NULL );

			return TRUE;
		}

		case WM_TIMER:
			if ( bLastOnline != ( pluginCurrentStatus != ID_STATUS_OFFLINE ) )
			{
				bLastOnline = ( pluginCurrentStatus != ID_STATUS_OFFLINE );
	            Refresh( hwndDlg, hTree );
			}
			return TRUE;

		case WM_DESTROY:
		{
			nns.RemoveAll();
			SetWindowLongPtr( hwndDlg, DWLP_USER, NULL );
			if ( data )
			{
				ImageList_Destroy( data->hTreeImages );
				mir_free( data );
			}
			hTree = NULL;
			break;
		}

		case WM_FILLTREE:
		{
			// Заполнение дерева имён
			TreeView_DeleteAllItems (hTree);
			TVINSERTSTRUCT tvis = { TVI_ROOT, TVI_LAST };
			tvis.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
			tvis.item.pszText = (LPTSTR)(LPCTSTR)pluginMachineName;
			tvis.item.iImage = tvis.item.iSelectedImage = nns.GetCount() ? 0 : 8;
			HTREEITEM hRoot = TreeView_InsertItem (hTree, &tvis);
			if ( nns.GetCount() )
			{
				TVITEM item = { 0 };
				for (POSITION pos = nns.GetHeadPosition (); pos;)
				{
					netbios_name nname (nns.GetNext (pos));
					tvis.item.lParam = nname.GetLana();

					// Поиск ланы в дереве по номеру
					item.hItem = TreeView_GetChild (hTree, hRoot);
					while (item.hItem)
					{					
						item.mask = TVIF_HANDLE | TVIF_PARAM;
						if (TreeView_GetItem (hTree, &item) &&
							item.lParam == nname.GetLana ())
							// найден
							break;
						item.hItem = TreeView_GetNextSibling (hTree, item.hItem);
					}
					if (item.hItem)
						// Лана уже есть
						tvis.hParent = item.hItem;
					else
					{
						// Ланы ещё нет
						tvis.hParent = hRoot;
						tvis.item.iImage = tvis.item.iSelectedImage = 1;
						CString tmp;
						tmp.Format ( _T("%s #%d"), TranslateT ("LAN adapter"), nname.GetLana ());
						tvis.item.pszText = (LPTSTR) (LPCTSTR) tmp;
						tvis.hParent = TreeView_InsertItem (hTree, &tvis);
						
						pluginNetBIOS.GetMAC (nname.GetLana (), tmp);
						tmp.Insert (0, _T("MAC: "));
						tvis.item.pszText = (LPTSTR) (LPCTSTR) tmp;
						TreeView_InsertItem (hTree, &tvis);
					}

					CA2T textT( nname.GetANSIFullName() );
					tvis.item.pszText = (LPTSTR)(LPCTSTR)textT;
					tvis.item.iImage = tvis.item.iSelectedImage =
						(nname.IsOwnName () ? (nname.IsError () ? 1 : 0) : 2) + 
							(nname.IsGroupName () ? 5 : 2);
					
					// Поиск имени в лане по имени
					item.hItem = TreeView_GetChild (hTree, tvis.hParent);
					while (item.hItem)
					{
						item.mask = TVIF_HANDLE | TVIF_TEXT;
						item.cchTextMax = 64;
						CString tmp;
						item.pszText = tmp.GetBuffer (item.cchTextMax);
						BOOL ret = TreeView_GetItem (hTree, &item);
						tmp.ReleaseBuffer ();
						if (ret && tmp == textT )
							// найден
							break;
						item.hItem = TreeView_GetNextSibling (hTree, item.hItem);
					}
					if (!item.hItem)
						// Имени ещё нет
						TreeView_InsertItem (hTree, &tvis);
				}
			}
			TreeView_Expand (hTree, hRoot, TVE_EXPAND);

			nns.RemoveAll ();
			return TRUE;
		}

        case WM_NOTIFY:
		{
			LPPSHNOTIFY lpHdr = reinterpret_cast <LPPSHNOTIFY> (lParam);
			if (lpHdr->hdr.idFrom == 0) {
				data->hContact = reinterpret_cast <HMODULE> (lpHdr->lParam);
				switch (lpHdr->hdr.code) {
					case PSN_KILLACTIVE:
						SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, FALSE);
						return TRUE;

					case PSN_APPLY:
						db_set_b (NULL, modname, "RegisterNick",
							(BYTE)( (IsDlgButtonChecked (hwndDlg, IDC_CHECK_NICK) == BST_CHECKED ? TRUE : FALSE ) ));   
						db_set_b (NULL, modname, "RegisterUser",
							(BYTE)( (IsDlgButtonChecked (hwndDlg, IDC_CHECK_USER) == BST_CHECKED ? TRUE : FALSE ) ));   
						db_set_b (NULL, modname, "Auto-answer",
							(BYTE)( (IsDlgButtonChecked (hwndDlg, IDC_AUTOANSWER) == BST_CHECKED ? TRUE : FALSE ) ));   
						db_set_b (NULL, modname, "Filter-dups",
							(BYTE)( (IsDlgButtonChecked (hwndDlg, IDC_DUPS) == BST_CHECKED ? TRUE : FALSE ) ));
						db_set_b (NULL, modname, "SendMethod",
							(BYTE)( (((IsDlgButtonChecked (hwndDlg, IDC_USE_MAILSLOT) == BST_CHECKED) ? 0 :
							((IsDlgButtonChecked (hwndDlg, IDC_USE_NETBIOS) == BST_CHECKED) ? 1 :
							((IsDlgButtonChecked (hwndDlg, IDC_USE_NETSEND) == BST_CHECKED) ? 2 :
							0 ) )))));
						SetLegacyOnline( NULL, ( IsDlgButtonChecked( hwndDlg,
							IDC_ALWAYSCHECK00FORONLINE ) == BST_CHECKED ) );   
						SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, PSNRET_NOERROR);
						if (data->need_restart) {
							data->need_restart = false;
							GotoOffline ();
							Sleep (2000);
							Refresh (hwndDlg, hTree);
						}
                        return TRUE;
                }
			}
			break;
		}

        case WM_COMMAND:
		{
			switch (LOWORD(wParam)) {
				case IDC_CHECK_NICK:
				case IDC_CHECK_USER:
					EnableWindow (GetDlgItem (hwndDlg, IDC_NICK1),
						IsDlgButtonChecked (hwndDlg, IDC_CHECK_NICK) == BST_CHECKED);
					EnableWindow (GetDlgItem (hwndDlg, IDC_NICK2),
						IsDlgButtonChecked (hwndDlg, IDC_CHECK_NICK) == BST_CHECKED);
					EnableWindow (GetDlgItem (hwndDlg, IDC_USER),
						IsDlgButtonChecked (hwndDlg, IDC_CHECK_USER) == BST_CHECKED);

				case IDC_USE_MAILSLOT:
				case IDC_USE_NETBIOS:
				case IDC_USE_NETSEND:
					data->need_restart = true;

				case IDC_AUTOANSWER:
				case IDC_ALWAYSCHECK00FORONLINE:
				case IDC_DUPS:
					PropSheet_Changed (GetParent (hwndDlg), hwndDlg);
					break;

				case IDC_ADD:
					AddDialog( hwndDlg );
					break;
            }
            break;
		}
	}
	return FALSE;
}

int __cdecl OPT_INITIALISE (WPARAM wParam, LPARAM /* lParam */)
{
	OPTIONSDIALOGPAGE odp = {0};
	odp.cbSize = sizeof (odp);
	odp.ptszGroup = LPGENT("Network");
	odp.position = odp.pszGroup[0];
	odp.hInstance = pluginModule;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS);
	odp.ptszTitle = LPGENT(modtitle);
	odp.pfnDlgProc = DlgProcOptions;
	odp.flags = ODPF_BOLDGROUPS | ODPF_TCHAR;
	Options_AddPage(wParam, &odp);

    return 0;
}
