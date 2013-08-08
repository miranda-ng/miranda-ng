/*

WinPopup Protocol plugin for Miranda IM.

Copyright (C) 2004-2010 Nikolay Raspopov

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

static INT_PTR CALLBACK DlgProcAddContact(HWND hwndDlg, UINT Msg,
	WPARAM wParam, LPARAM /*lParam*/)
{
	switch ( Msg )
	{
	case WM_INITDIALOG:
		TranslateDialogDefault( hwndDlg );
		return TRUE;

    case WM_COMMAND:
		switch ( LOWORD( wParam ) )
		{
		case IDOK:
			{
				bool bGroup = IsDlgButtonChecked( hwndDlg, IDC_GROUP ) == BST_CHECKED;
				CString sName;
				GetDlgItemText( hwndDlg, IDC_NAME, sName.GetBuffer( 128 ), 127 );
				sName.ReleaseBuffer();
				sName.Trim();
				if ( ! sName.IsEmpty() )
				{
					HCURSOR hCurrent = SetCursor( LoadCursor( NULL, IDC_WAIT ) );
					HANDLE hContact = AddToListByName( sName, 0, NULL, true, bGroup );
					SetCursor( hCurrent );
					if ( hContact )
						EndDialog( hwndDlg, IDOK );						 
				}
			}
			return TRUE;

		case IDCANCEL:
			EndDialog( hwndDlg, IDCANCEL );
			return TRUE;
		}
		break;
	}
	return FALSE;
}

void AddDialog(HWND hParentWnd)
{
	DialogBox( pluginModule, MAKEINTRESOURCE( IDD_ADD ), hParentWnd, DlgProcAddContact );
}
