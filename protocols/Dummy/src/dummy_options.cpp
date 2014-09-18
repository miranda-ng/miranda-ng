/*
Copyright (c) 2014 Robert Pösel

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

//////////////////////////////////////////////////////////////////////////////
// Account manager dialog

INT_PTR CALLBACK DummyAccountProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CDummyProto *ppro = (CDummyProto*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (uMsg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);

		ppro = (CDummyProto*)lParam;
		SetWindowLongPtr( hwndDlg, GWLP_USERDATA, lParam );

		SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM)Skin_GetIconByHandle(ppro->m_hProtoIcon, true));
		SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, (LPARAM)Skin_GetIconByHandle(ppro->m_hProtoIcon));
		{
			ptrA tszIdText(ppro->getStringA(DUMMY_ID_TEXT));
			if (tszIdText != NULL)
				SetDlgItemTextA(hwndDlg, IDC_ID_TEXT, tszIdText);

			ptrA tszIdSetting(ppro->getStringA(DUMMY_ID_SETTING));
			if (tszIdSetting != NULL)
				SetDlgItemTextA(hwndDlg, IDC_ID_SETTING, tszIdSetting);
		}
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_ID_TEXT:
		case IDC_ID_SETTING:
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
			GetDlgItemTextA(hwndDlg, IDC_ID_TEXT, str, SIZEOF(str));
			ppro->setString(DUMMY_ID_TEXT, str);

			GetDlgItemTextA(hwndDlg, IDC_ID_SETTING, str, SIZEOF(str));
			ppro->setString(DUMMY_ID_SETTING, str);
		}
		break;

	case WM_CLOSE:
		EndDialog(hwndDlg, 0);
		break;

	case WM_DESTROY:
		Skin_ReleaseIcon((HICON)SendMessage(hwndDlg, WM_GETICON, ICON_BIG, 0));
		Skin_ReleaseIcon((HICON)SendMessage(hwndDlg, WM_GETICON, ICON_SMALL, 0));
		break;
	}

	return FALSE;
}

INT_PTR CDummyProto::SvcCreateAccMgrUI(WPARAM wParam, LPARAM lParam)
{
	return (INT_PTR)CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_ACCMGRUI), (HWND)lParam, DummyAccountProc, (LPARAM)this);
}
