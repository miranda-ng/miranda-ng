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

INT_PTR CALLBACK FileAccessDlgProc(HWND hdlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	INT_PTR bReturn;
	if (DoMyControlProcessing(hdlg, message, wParam, lParam, &bReturn))
		return bReturn;

	switch (message) {
	case WM_INITDIALOG:
		if (bShortMode)
			EnableWindow(GetDlgItem(GetParent(hdlg), IDC_BACK), FALSE);
		CheckDlgButton(hdlg, IDC_CHECKONLY, opts.bCheckOnly ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hdlg, IDC_BACKUP, opts.bBackup ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hdlg, IDC_AGGRESSIVE, opts.bAggressive ? BST_CHECKED : BST_UNCHECKED);
		SendMessage(hdlg, WM_COMMAND, MAKEWPARAM(IDC_CHECKONLY, BN_CLICKED), 0);
		TranslateDialogDefault(hdlg);
		return TRUE;

	case WZN_PAGECHANGING:
		opts.bCheckOnly = IsDlgButtonChecked(hdlg, IDC_CHECKONLY);
		opts.bAggressive = IsDlgButtonChecked(hdlg, IDC_AGGRESSIVE);
		if (opts.bCheckOnly) opts.bBackup = 0;
		else opts.bBackup = IsDlgButtonChecked(hdlg, IDC_BACKUP);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_BACK:
			PostMessage(GetParent(hdlg), WZM_GOTOPAGE, IDD_SELECTDB, (LPARAM)SelectDbDlgProc);
			break;

		case IDOK:
			if (opts.bCheckOnly)
				OpenDatabase(hdlg, IDD_PROGRESS);
			else
				PostMessage(GetParent(hdlg), WZM_GOTOPAGE, IDD_CLEANING, (LPARAM)CleaningDlgProc);
			break;

		case IDC_CHECKONLY:
			EnableWindow(GetDlgItem(hdlg, IDC_BACKUP), BST_UNCHECKED == IsDlgButtonChecked(hdlg, IDC_CHECKONLY));
			EnableWindow(GetDlgItem(hdlg, IDC_STBACKUP), BST_UNCHECKED == IsDlgButtonChecked(hdlg, IDC_CHECKONLY));
			break;
		}
		break;
	}
	return FALSE;
}
