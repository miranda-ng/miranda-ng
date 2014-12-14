/*
Miranda Database Tool
Copyright (C) 2001-2005  Richard Hughes

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

#include "dbchecker.h"

INT_PTR CALLBACK OpenErrorDlgProc(HWND hdlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	INT_PTR bReturn;
	if (DoMyControlProcessing(hdlg, message, wParam, lParam, &bReturn))
		return bReturn;

	switch (message) {
	case WM_INITDIALOG:
	{
		TCHAR szError[256];
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, opts.error, 0, szError, SIZEOF(szError), NULL);
		SetDlgItemText(hdlg, IDC_ERRORTEXT, szError);
	}
	if (opts.error == ERROR_SHARING_VIOLATION) ShowWindow(GetDlgItem(hdlg, IDC_INUSE), SW_SHOW);
	SetWindowLongPtr(GetDlgItem(hdlg, IDC_FILE), GWL_STYLE, GetWindowLongPtr(GetDlgItem(hdlg, IDC_FILE), GWL_STYLE) | SS_PATHELLIPSIS);
	TranslateDialogDefault(hdlg);
	SetDlgItemText(hdlg, IDC_FILE, opts.filename);
	return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_BACK:
			PostMessage(GetParent(hdlg), WZM_GOTOPAGE, IDD_SELECTDB, (LPARAM)SelectDbDlgProc);
			break;

		case IDOK:
			OpenDatabase(hdlg, IDD_FILEACCESS);
			break;
		}
		break;
	}
	return FALSE;
}
