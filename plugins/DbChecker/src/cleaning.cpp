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

INT_PTR CALLBACK CleaningDlgProc(HWND hdlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	INT_PTR bReturn;
	if (DoMyControlProcessing(hdlg, message, wParam, lParam, &bReturn))
		return bReturn;

	switch (message) {
	case WM_INITDIALOG:
		CheckDlgButton(hdlg, IDC_ERASEHISTORY, opts.bEraseHistory ? BST_CHECKED : BST_UNCHECKED);
		EnableWindow(GetDlgItem(hdlg, IDC_ERASEHISTORY), !opts.bAggressive);
		CheckDlgButton(hdlg, IDC_MARKREAD, opts.bMarkRead ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hdlg, IDC_CONVERTUTF, opts.bConvertUtf ? BST_CHECKED : BST_UNCHECKED);
		TranslateDialogDefault(hdlg);
		return TRUE;

	case WZN_PAGECHANGING:
		opts.bEraseHistory = IsDlgButtonChecked(hdlg, IDC_ERASEHISTORY) && !opts.bAggressive;
		opts.bMarkRead = IsDlgButtonChecked(hdlg, IDC_MARKREAD);
		opts.bConvertUtf = IsDlgButtonChecked(hdlg, IDC_CONVERTUTF);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_BACK:
			if (opts.bCheckOnly)
				PostMessage(GetParent(hdlg), WZM_GOTOPAGE, IDD_SELECTDB, (LPARAM)SelectDbDlgProc);
			else
				PostMessage(GetParent(hdlg), WZM_GOTOPAGE, IDD_FILEACCESS, (LPARAM)FileAccessDlgProc);
			break;

		case IDOK:
			OpenDatabase(hdlg, IDD_PROGRESS);
			break;
		}
		break;
	}
	return FALSE;
}
