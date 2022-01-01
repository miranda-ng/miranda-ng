// -----------------------------------------------------------------------------
// ICQ plugin for Miranda NG
// -----------------------------------------------------------------------------
// Copyright © 2018-22 Miranda NG team
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
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
// -----------------------------------------------------------------------------

#include "stdafx.h"

static INT_PTR CALLBACK IcqDlgProc(HWND hwndDlg, UINT msg, WPARAM, LPARAM lParam)
{
	switch(msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->idFrom == 0 && ((LPNMHDR)lParam)->code == PSN_PARAMCHANGED) {
			MCONTACT hContact = (MCONTACT)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
			CIcqProto *ppro = (CIcqProto*)((PSHNOTIFY*)lParam)->lParam;

			SetDlgItemTextW(hwndDlg, IDC_UIN, ppro->GetUserId(hContact));
			SetDlgItemTextW(hwndDlg, IDC_NICK, ppro->getMStringW(hContact, DB_KEY_ICQNICK));

			SetDlgItemTextA(hwndDlg, IDC_IDLETIME, time2text(ppro->getDword(hContact, DB_KEY_IDLE)));
			SetDlgItemTextA(hwndDlg, IDC_LASTSEEN, time2text(ppro->getDword(hContact, DB_KEY_LASTSEEN)));
			SetDlgItemTextA(hwndDlg, IDC_MEMBERSINCE, time2text(ppro->getDword(hContact, DB_KEY_MEMBERSINCE)));
			SetDlgItemTextA(hwndDlg, IDC_ONLINESINCE, time2text(time(0) - ppro->getDword(hContact, DB_KEY_ONLINETS)));
		}
		break;
	}

	return 0;
}

int CIcqProto::OnUserInfoInit(WPARAM wParam, LPARAM hContact)
{
	if (hContact && mir_strcmp(Proto_GetBaseAccountName(hContact), m_szModuleName))
		return 0;

	if (isChatRoom(hContact))
		return 0;

	OPTIONSDIALOGPAGE odp = {};
	odp.flags = ODPF_UNICODE;
	odp.dwInitParam = LPARAM(this);
	if (hContact == 0) {
		odp.flags |= ODPF_DONTTRANSLATE;
		odp.szTitle.w = m_tszUserName;
	}
	else odp.szTitle.w = L"ICQ";

	odp.pfnDlgProc = IcqDlgProc;
	odp.position = -1900000000;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_INFO_ICQ);
	g_plugin.addUserInfo(wParam, &odp);
	return 0;
}
