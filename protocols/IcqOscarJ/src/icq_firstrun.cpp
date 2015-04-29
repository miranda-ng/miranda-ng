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

#include "stdafx.h"

static void accountLoadDetails(CIcqProto *ppro, HWND hwndDlg)
{
	char pszUIN[20];
	DWORD dwUIN = ppro->getContactUin(NULL);
	if (dwUIN) {
		mir_snprintf(pszUIN, SIZEOF(pszUIN), "%u", dwUIN);
		SetDlgItemTextA(hwndDlg, IDC_UIN, pszUIN);
	}

	char pszPwd[PASSWORDMAXLEN];
	if (ppro->GetUserStoredPassword(pszPwd, PASSWORDMAXLEN))
		SetDlgItemTextA(hwndDlg, IDC_PW, pszPwd);
}

INT_PTR CALLBACK icq_FirstRunDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CIcqProto* ppro = (CIcqProto*)GetWindowLongPtr( hwndDlg, GWLP_USERDATA );

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);

		ppro = (CIcqProto*)lParam;
		SetWindowLongPtr( hwndDlg, GWLP_USERDATA, lParam );

		SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM)Skin_GetIconByHandle(ppro->m_hProtoIcon, true));
		SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, (LPARAM)Skin_GetIconByHandle(ppro->m_hProtoIcon));

		SendDlgItemMessage(hwndDlg, IDC_PW, EM_LIMITTEXT, PASSWORDMAXLEN - 1, 0);

		accountLoadDetails(ppro, hwndDlg);
		return TRUE;

	case WM_DESTROY:
		Skin_ReleaseIcon((HICON)SendMessage(hwndDlg, WM_GETICON, ICON_BIG, 0));
		Skin_ReleaseIcon((HICON)SendMessage(hwndDlg, WM_GETICON, ICON_SMALL, 0));
		break;

	case WM_CLOSE:
		EndDialog(hwndDlg, 0);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_REGISTER:
			CallService(MS_UTILS_OPENURL, OUF_NEWWINDOW, (LPARAM)URL_REGISTER);
			break;

		case IDC_UIN:
		case IDC_PW:
			if (HIWORD(wParam) == EN_CHANGE && (HWND)lParam == GetFocus()) {
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				break;
			}
		}
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code) {
		case PSN_APPLY:
			char str[128];
			GetDlgItemTextA(hwndDlg, IDC_UIN, str, SIZEOF(str));
			ppro->setDword(UNIQUEIDSETTING, atoi(str));

			GetDlgItemTextA(hwndDlg, IDC_PW, str, SIZEOF(ppro->m_szPassword));
			strcpy(ppro->m_szPassword, str);
			ppro->setString("Password", str);
			break;

		case PSN_RESET:
			accountLoadDetails(ppro, hwndDlg);
			break;
		}
		break;
	}

	return FALSE;
}

INT_PTR CIcqProto::OnCreateAccMgrUI(WPARAM, LPARAM lParam)
{
	return (INT_PTR)CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_ICQACCOUNT), (HWND)lParam, icq_FirstRunDlgProc, LPARAM(this));
}
