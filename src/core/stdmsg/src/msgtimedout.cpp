/*

Copyright 2000-12 Miranda IM, 2012-17 Miranda NG project,
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

#include "stdafx.h"
#include "msgs.h"

struct ErrorDlgParam
{
	const char *szMsg;
	TMsgQueue *item;
};

INT_PTR CALLBACK ErrorDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	TMsgQueue *item = (TMsgQueue*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{
			ErrorDlgParam *param = (ErrorDlgParam *)lParam;
			item = param->item;

			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)item);

			if (!param->szMsg || !param->szMsg[0])
				SetDlgItemText(hwndDlg, IDC_ERRORTEXT, TranslateT("An unknown error has occurred."));
			else
				SetDlgItemText(hwndDlg, IDC_ERRORTEXT, ptrW(Langpack_PcharToTchar(param->szMsg)));

			SetDlgItemText(hwndDlg, IDC_MSGTEXT, ptrW(mir_utf8decodeW(item->szMsg)));

			HWND hwndParent = GetParent(hwndDlg);
			if (hwndParent != nullptr) {
				RECT rc, rcParent;
				if (GetWindowRect(hwndDlg, &rc))
					if (GetWindowRect(hwndParent, &rcParent))
						SetWindowPos(hwndDlg, 0, (rcParent.left + rcParent.right - (rc.right - rc.left)) / 2,
							(rcParent.top + rcParent.bottom - (rc.bottom - rc.top)) / 2,
							0, 0, SWP_NOZORDER | SWP_NOSIZE);
			}
		}
		return TRUE;

	case WM_DESTROY:
		mir_free(item->szMsg);
		mir_free(item);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			SendMessageDirect(ptrW(mir_utf8decodeW(item->szMsg)), item->hContact);
			DestroyWindow(hwndDlg);
			break;

		case IDCANCEL:
			DestroyWindow(hwndDlg);
			break;
		}
		break;
	}
	return FALSE;
}

void MessageFailureProcess(TMsgQueue *item, const char* err)
{
	MCONTACT hContact = db_mc_tryMeta(item->hContact);

	HWND hwnd = Srmm_FindWindow(hContact);
	if (hwnd == nullptr) {
		// If message window doesn't already exist, open a new one
		SendMessageCmd(item->hContact, nullptr);
		hwnd = Srmm_FindWindow(hContact);
	}
	else SendMessage(hwnd, DM_REMAKELOG, 0, 0);

	Skin_PlaySound("SendError");

	ErrorDlgParam param = { err, item };
	CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_MSGSENDERROR), hwnd, ErrorDlgProc, (LPARAM)&param);
}
