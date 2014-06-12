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

void UpDate_CountryIcon(HWND hCtrl, int countryID)
{
	HICON hIcon = LoadFlagIcon(countryID);
	HICON hOld  = Static_SetIcon(hCtrl, hIcon);
	ShowWindow(hCtrl, hIcon ? SW_SHOW : SW_HIDE);
	Skin_ReleaseIcon(hOld);
}

// Default dialog procedure, which handles common functions
INT_PTR CALLBACK PSPBaseProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CCtrlList *pCtrlList;

	pCtrlList = CCtrlList::GetObj(hDlg);
	if (PtrIsValid(pCtrlList)) {
		switch (uMsg) {
		case WM_INITDIALOG:
			return TRUE;

		// set propertysheet page's background white in aero mode
		case WM_CTLCOLORSTATIC:
		case WM_CTLCOLORDLG:
			{
				if (IsAeroMode())
					return (INT_PTR)GetStockBrush(WHITE_BRUSH);
			} break;

		// Set text color of edit boxes according to the source of information they display.
		case WM_CTLCOLOREDIT:
			return pCtrlList->OnSetTextColour((HWND)lParam, (HDC)wParam);

		case WM_NOTIFY:
			switch (((LPNMHDR)lParam)->idFrom) {
			case 0:
				MCONTACT hContact = (MCONTACT)((LPPSHNOTIFY)lParam)->lParam;
				LPSTR pszProto;

				switch (((LPNMHDR)lParam)->code) {
				case PSN_RESET:
					pCtrlList->OnReset();
					break;

				case PSN_INFOCHANGED:
					if (PSGetBaseProto(hDlg, pszProto) && *pszProto) {
						BOOL bChanged = (GetWindowLongPtr(hDlg, DWLP_MSGRESULT)&PSP_CHANGED) | pCtrlList->OnInfoChanged(hContact, pszProto);
						SetWindowLongPtr(hDlg, DWLP_MSGRESULT, bChanged ? PSP_CHANGED : 0);
					}
					break;

				case PSN_APPLY:
					if (PSGetBaseProto(hDlg, pszProto) && *pszProto)
						pCtrlList->OnApply(hContact, pszProto);
					break;
				}
				break;
			}
			break;

		case WM_COMMAND:
			if (!PspIsLocked(hDlg))
				pCtrlList->OnChangedByUser(LOWORD(wParam), HIWORD(wParam));
			break;

		case WM_DESTROY:
			// destroy all control objects and the list
			pCtrlList->Release();
		}
	}
	return 0;
}
