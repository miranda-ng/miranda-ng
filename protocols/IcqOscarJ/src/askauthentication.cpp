// ---------------------------------------------------------------------------80
//                ICQ plugin for Miranda Instant Messenger
//                ________________________________________
//
// Copyright © 2000-2001 Richard Hughes, Roland Rabien, Tristan Van de Vreede
// Copyright © 2001-2002 Jon Keating, Richard Hughes
// Copyright © 2002-2004 Martin Öberg, Sam Kothari, Robert Rainwater
// Copyright © 2004-2008 Joe Kucera
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

struct AskAuthParam
{
	CIcqProto* ppro;
	MCONTACT hContact;
};

static INT_PTR CALLBACK AskAuthProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	AskAuthParam* dat = (AskAuthParam*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (msg) {
	case WM_INITDIALOG:
		dat = (AskAuthParam*)lParam;
		if (!dat->hContact || !dat->ppro->icqOnline())
			EndDialog(hwndDlg, 0);

		TranslateDialogDefault(hwndDlg);
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
		SendDlgItemMessage(hwndDlg, IDC_EDITAUTH, EM_LIMITTEXT, (WPARAM)255, 0);
		SetDlgItemText(hwndDlg, IDC_EDITAUTH, TranslateT("Please authorize me to add you to my contact list."));
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			if (dat->ppro->icqOnline())
			{
				DWORD dwUin;
				uid_str szUid;
				if ( dat->ppro->getContactUid(dat->hContact, &dwUin, &szUid))
					return TRUE; // Invalid contact

				char* szReason = GetDlgItemTextUtf(hwndDlg, IDC_EDITAUTH);
				dat->ppro->icq_sendAuthReqServ(dwUin, szUid, szReason);
				SAFE_FREE((void**)&szReason);

				// auth bug fix (thx Bio)
				if (dat->ppro->m_bSsiEnabled && dwUin)
					dat->ppro->resetServContactAuthState(dat->hContact);

				EndDialog(hwndDlg, 0);
			}
			return TRUE;

		case IDCANCEL:
			EndDialog(hwndDlg, 0);
			return TRUE;
		}

		break;

	case WM_CLOSE:
		EndDialog(hwndDlg,0);
		return TRUE;
	}

	return FALSE;
}

INT_PTR CIcqProto::RequestAuthorization(WPARAM wParam, LPARAM)
{
	AskAuthParam param = { this, wParam };
	DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_ASKAUTH), NULL, AskAuthProc, (LPARAM)&param);
	return 0;
}
