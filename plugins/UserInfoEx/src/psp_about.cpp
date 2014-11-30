/*
UserinfoEx plugin for Miranda IM

Copyright:
© 2006-2010 DeathAxe, Yasnovidyashii, Merlin, K. Romanov, Kreol

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include "commonheaders.h"

/**
 * Dialog procedure for the about/nodes information propertysheetpage
 *
 * @param	 hDlg	- handle to the dialog window
 * @param	 uMsg	- the message to handle
 * @param	 wParam	- parameter
 * @param	 lParam	- parameter
 *
 * @return	different values
 **/
INT_PTR CALLBACK PSPProcEdit(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam, const CHAR *pszSetting)
{
	switch (uMsg) {
	case WM_INITDIALOG:
		{
			CCtrlList *pCtrlList = CCtrlList::CreateObj(hDlg);
			if (pCtrlList) {
				HFONT hBoldFont;
				PSGetBoldFont(hDlg, hBoldFont);
				SendDlgItemMessage(hDlg, IDC_PAGETITLE, WM_SETFONT, (WPARAM)hBoldFont, 0);

				if (!mir_strcmp(pszSetting, SET_CONTACT_MYNOTES))
					SetDlgItemText(hDlg, IDC_PAGETITLE, LPGENT("My notes:"));
				else
					SetDlgItemText(hDlg, IDC_PAGETITLE, LPGENT("About:"));

				TranslateDialogDefault(hDlg);
				
				pCtrlList->insert(CEditCtrl::CreateObj(hDlg, EDIT_ABOUT, pszSetting, DBVT_TCHAR));

				// remove static edge in aero mode
				if (IsAeroMode())
					SetWindowLongPtr(GetDlgItem(hDlg, EDIT_ABOUT), GWL_EXSTYLE, GetWindowLongPtr(GetDlgItem(hDlg, EDIT_ABOUT), GWL_EXSTYLE) & ~WS_EX_STATICEDGE);

				SendDlgItemMessage(hDlg, EDIT_ABOUT, EM_SETEVENTMASK, 0, /*ENM_KEYEVENTS | */ENM_LINK | ENM_CHANGE);
				SendDlgItemMessage(hDlg, EDIT_ABOUT, EM_AUTOURLDETECT, TRUE, NULL);
				if (!lParam) SendDlgItemMessage(hDlg, EDIT_ABOUT, EM_LIMITTEXT, 1024, NULL);
			}
		}
		break;
		
	case WM_NOTIFY:
		{
			switch (((LPNMHDR)lParam)->idFrom) 
			{
			// notification handler for richedit control
			case EDIT_ABOUT:
				{
					switch (((LPNMHDR)lParam)->code) {

					// notification handler for a link within the richedit control
					case EN_LINK:
						return CEditCtrl::GetObj(((LPNMHDR)lParam)->hwndFrom)->LinkNotificationHandler((ENLINK *)lParam);
					}
				}
				return FALSE;
			}
		}
		break;

	case WM_COMMAND:
		{
			switch (LOWORD(wParam)) {
			case EDIT_ABOUT:
				{
					if (HIWORD(wParam) == EN_CHANGE)
					{
						CBaseCtrl *pResult;

						pResult = CBaseCtrl::GetObj((HWND)lParam);
						if (PtrIsValid(pResult) && (pResult->_cbSize == sizeof(CBaseCtrl)))
							pResult->OnChangedByUser(HIWORD(wParam));
					}
				}
			}
		}
		return FALSE;
	}
	return PSPBaseProc(hDlg, uMsg, wParam, lParam);
}
