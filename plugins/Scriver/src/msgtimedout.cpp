/*
Scriver

Copyright (c) 2000-09 Miranda ICQ/IM project,

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

INT_PTR CALLBACK ErrorDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	ErrorWindowData *ewd = (ErrorWindowData *) GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	//if (ewd==NULL && msg!=WM_INITDIALOG) return FALSE;
	switch (msg) {
		case WM_INITDIALOG:
		{
			RECT rc, rcParent;
			TCHAR szText[2048];
			ewd = (ErrorWindowData *) lParam;
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR) ewd);
			TranslateDialogDefault(hwndDlg);
			if (ewd != NULL) {
				ShowWindow(GetParent(ewd->hwndParent), SW_RESTORE);
				if (ewd->szDescription)
					SetDlgItemText(hwndDlg, IDC_ERRORTEXT, ewd->szDescription);
				else
					SetDlgItemText(hwndDlg, IDC_ERRORTEXT, TranslateT("An unknown error has occurred."));

				if (ewd->szText) {
					SETTEXTEX  st = {0};
					st.flags = ST_DEFAULT;
					st.codepage = 1200;
					
					SendDlgItemMessage(hwndDlg, IDC_MSGTEXT, EM_SETTEXTEX, (WPARAM)&st, (LPARAM)ewd->szText);
				}
				if (ewd->szName)
					mir_sntprintf(szText, SIZEOF(szText), _T("%s - %s"), TranslateT("Send Error"), ewd->szName);
				else
					_tcsncpy_s(szText, TranslateT("Send Error"), _TRUNCATE);

				SetWindowText(hwndDlg, szText);
				GetWindowRect(hwndDlg, &rc);
				GetWindowRect(GetParent(ewd->hwndParent), &rcParent);
				SetWindowPos(hwndDlg, HWND_TOP, rcParent.left + (rcParent.right - rcParent.left - rc.right + rc.left) / 2, rcParent.top + (rcParent.bottom - rcParent.top - rc.bottom + rc.top) / 2, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
			}
		}
		return TRUE;
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDOK:
					SendMessage(ewd->hwndParent, DM_ERRORDECIDED, MSGERROR_RETRY, (LPARAM)ewd->queueItem);
					DestroyWindow(hwndDlg);
					break;
				case IDCANCEL:
					SendMessage(ewd->hwndParent, DM_ERRORDECIDED, MSGERROR_CANCEL, (LPARAM)ewd->queueItem);
					DestroyWindow(hwndDlg);
					break;
			}
			break;
		case WM_DESTROY:
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, 0);
			mir_free(ewd->szName);
			mir_free(ewd->szDescription);
			mir_free(ewd->szText);
			mir_free(ewd);
			break;

	}
	return FALSE;
}
