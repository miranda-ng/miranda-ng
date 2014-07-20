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

INT_PTR CALLBACK FinishedDlgProc(HWND hdlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	INT_PTR bReturn;
	if (DoMyControlProcessing(hdlg, message, wParam, lParam, &bReturn))
		return bReturn;

	switch (message) {
	case WM_INITDIALOG:
		EnableWindow(GetDlgItem(GetParent(hdlg), IDC_BACK), FALSE);
		EnableWindow(GetDlgItem(GetParent(hdlg), IDOK), FALSE);
		SetDlgItemText(GetParent(hdlg), IDCANCEL, TranslateT("&Finish"));
		SetWindowLongPtr(GetDlgItem(hdlg, IDC_DBFILE), GWL_STYLE, GetWindowLongPtr(GetDlgItem(hdlg, IDC_DBFILE), GWL_STYLE) | SS_PATHELLIPSIS);
		SetDlgItemText(hdlg, IDC_DBFILE, opts.filename);
		if (opts.bBackup) {
			ShowWindow(GetDlgItem(hdlg, IDC_STBACKUP), TRUE);
			SetWindowLongPtr(GetDlgItem(hdlg, IDC_BACKUPFILE), GWL_STYLE, GetWindowLongPtr(GetDlgItem(hdlg, IDC_BACKUPFILE), GWL_STYLE) | SS_PATHELLIPSIS);
			SetDlgItemText(hdlg, IDC_BACKUPFILE, opts.backupFilename);
		}
		else ShowWindow(GetDlgItem(hdlg, IDC_STBACKUP), FALSE);
		TranslateDialogDefault(hdlg);
		return TRUE;

	case WZN_CANCELCLICKED:
		bLaunchMiranda = IsDlgButtonChecked(hdlg, IDC_LAUNCHMIRANDA) == BST_CHECKED;
		CallService(MS_DB_SETDEFAULTPROFILE, (WPARAM)opts.filename, 0);
		wizardResult = 1;
		return TRUE;
	}
	return FALSE;
}
