// ---------------------------------------------------------------------------80
//                ICQ plugin for Miranda Instant Messenger
//                ________________________________________
//
// Copyright © 2000-2001 Richard Hughes, Roland Rabien, Tristan Van de Vreede
// Copyright © 2001-2002 Jon Keating, Richard Hughes
// Copyright © 2002-2004 Martin Öberg, Sam Kothari, Robert Rainwater
// Copyright © 2004-2009 Joe Kucera
// Copyright © 2012-2014 Miranda NG Team
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
// -----------------------------------------------------------------------------

#include "icqoscar.h"

INT_PTR CALLBACK LoginPasswdDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CIcqProto* ppro = (CIcqProto*)GetWindowLongPtr( hwndDlg, GWLP_USERDATA );

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);

		ppro = (CIcqProto*)lParam;
		SetWindowLongPtr( hwndDlg, GWLP_USERDATA, lParam );

		SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM)Skin_GetIconByHandle(ppro->m_hProtoIcon, true));
		SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, (LPARAM)Skin_GetIconByHandle(ppro->m_hProtoIcon));
		{
			DWORD dwUin = ppro->getContactUin(NULL);

			TCHAR pszUIN[MAX_PATH];
			mir_sntprintf(pszUIN, SIZEOF(pszUIN), TranslateT("Enter a password for UIN %u:"), dwUin);
			SetDlgItemText(hwndDlg, IDC_INSTRUCTION, pszUIN);

			SendDlgItemMessage(hwndDlg, IDC_LOGINPW, EM_LIMITTEXT, PASSWORDMAXLEN - 1, 0);

			CheckDlgButton(hwndDlg, IDC_SAVEPASS, ppro->getByte("RememberPass", 0));
		}
		break;

	case WM_DESTROY:
		Skin_ReleaseIcon((HICON)SendMessage(hwndDlg, WM_GETICON, ICON_BIG, 0));
		Skin_ReleaseIcon((HICON)SendMessage(hwndDlg, WM_GETICON, ICON_SMALL, 0));
		break;

	case WM_CLOSE:
		EndDialog(hwndDlg, 0);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			ppro->m_bRememberPwd = (BYTE)IsDlgButtonChecked(hwndDlg, IDC_SAVEPASS);
			ppro->setByte("RememberPass", ppro->m_bRememberPwd);

			GetDlgItemTextA(hwndDlg, IDC_LOGINPW, ppro->m_szPassword, SIZEOF(ppro->m_szPassword));

			ppro->icq_login(ppro->m_szPassword);

			EndDialog(hwndDlg, IDOK);
			break;

		case IDCANCEL:
			ppro->SetCurrentStatus(ID_STATUS_OFFLINE);
			EndDialog(hwndDlg, IDCANCEL);
			break;
		}
		break;
	}

	return FALSE;
}

void CIcqProto::RequestPassword()
{
	DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_LOGINPW), NULL, LoginPasswdDlgProc, LPARAM(this));
}
