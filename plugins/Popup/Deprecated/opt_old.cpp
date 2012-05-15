/*
Popup Plus plugin for Miranda IM

Copyright	© 2002 Luca Santarelli,
			© 2004-2007 Victor Pavlychko
			© 2010 MPK
			© 2010 Merlin_de

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

===============================================================================

File name      : $HeadURL: http://svn.miranda.im/mainrepo/popup/trunk/src/opt_old.cpp $
Revision       : $Revision: 1610 $
Last change on : $Date: 2010-06-23 00:55:13 +0300 (Ð¡Ñ€, 23 Ð¸ÑŽÐ½ 2010) $
Last change by : $Author: Merlin_de $

===============================================================================
*/

#include "headers.h"


void RestartNeeded_adv(HWND hwndDlg){
	ShowWindow(GetDlgItem(hwndDlg, IDC_ICO_RESTART), SW_SHOW);
	ShowWindow(GetDlgItem(hwndDlg, IDC_TXT_RESTART), SW_SHOW);
}

void LoadOption_OldOpts() {
}

INT_PTR CALLBACK DlgProcPopUpOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
		case WM_INITDIALOG:
		{
			TranslateDialogDefault(hwndDlg);

			return TRUE;
		}
		//End WM_INITDIALOG
		case WM_COMMAND: {
			WORD idCtrl = LOWORD(wParam), wNotifyCode = HIWORD(wParam);
			if (wNotifyCode == BN_KILLFOCUS || wNotifyCode == BN_SETFOCUS) return TRUE;
			//These are simple clicks: we don't save, but we tell the Options Page to enable the "Apply" button.
			switch(idCtrl) {
				case IDC_PREVIEW:
					PopUpPreview();
					break;
			} //End switch(WM_COMMAND)
			break;
		//End WM_COMMAND
		}
		case WM_NOTIFY: { //Here we have pressed either the OK or the APPLY button.
			switch(((LPNMHDR)lParam)->idFrom) {
				case 0:
					switch (((LPNMHDR)lParam)->code) {
						case PSN_RESET:
							//Restore the options stored in memory.
							LoadOption_OldOpts();
							return TRUE;
						case PSN_APPLY: {
							return TRUE;
						} //case PSN_APPLY
					} // switch code
					break;
			} //switch idFrom
			break; //End WM_NOTIFY
		}
		case UM_SETDLGITEMINT:
			SetDlgItemInt(hwndDlg, (int)wParam, (int)lParam, FALSE);
			break;
	} //switch message
	return FALSE;
}

