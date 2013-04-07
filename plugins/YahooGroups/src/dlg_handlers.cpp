/*
YahooGroups plugin for Miranda IM

Copyright © 2007 Cristian Libotean

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

INT_PTR CALLBACK DlgProcOptions(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_INITDIALOG:
		{
			TranslateDialogDefault(hWnd);
			
			int autoMove = db_get_b(NULL, ModuleName, "MoveAutomatically", 0);
			CheckDlgButton(hWnd, IDC_MOVEAUTOMATICALLY, autoMove ? BST_CHECKED : BST_UNCHECKED);
		
			return 0;
			break;
		}
		
		case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
				case IDC_YAHOOGROUPS:
				{
					YahooGroupsMoveService(0,0);
					break;
				}
				
				case IDC_MOVEAUTOMATICALLY:
				{
					SendMessage(GetParent(hWnd), PSM_CHANGED, 0, 0);
					break;
				}
			}
		
			break;
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
							int autoMove = IsDlgButtonChecked(hWnd, IDC_MOVEAUTOMATICALLY);
							db_set_b(NULL, ModuleName, "MoveAutomatically", autoMove);
							break;
						}
					}
				
					break;
				}
			}
		
			break;
		}
		
	}
	
	return 0;
}