/*

Facebook plugin for Miranda Instant Messenger
_____________________________________________

Copyright © 2009-11 Michal Zelinka, 2011-17 Robert Pösel, 2017-22 Miranda NG team

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "stdafx.h"

INT_PTR CALLBACK FBAccountProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	FacebookProto *proto = reinterpret_cast<FacebookProto*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

	switch (message) {

	case WM_INITDIALOG:
		{
			TranslateDialogDefault(hwnd);

			proto = reinterpret_cast<FacebookProto*>(lparam);
			SetWindowLongPtr(hwnd, GWLP_USERDATA, lparam);

			ptrA login(db_get_sa(0, proto->ModuleName(), DBKEY_LOGIN));
			if (login != nullptr)
				SetDlgItemTextA(hwnd, IDC_UN, login);

			ptrA password(db_get_sa(0, proto->ModuleName(), DBKEY_PASS));
			if (password != nullptr)
				SetDlgItemTextA(hwnd, IDC_PW, password);

			//if (!proto->isOffline()) {
			//	SendDlgItemMessage(hwnd, IDC_UN, EM_SETREADONLY, 1, 0);
			//	SendDlgItemMessage(hwnd, IDC_PW, EM_SETREADONLY, 1, 0);
			//}
			return TRUE;
		}
	case WM_COMMAND:
		if (HIWORD(wparam) == EN_CHANGE && reinterpret_cast<HWND>(lparam) == GetFocus()) {
			switch (LOWORD(wparam)) {
			case IDC_UN:
			case IDC_PW:
				SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
			}
		}
		break;

	case WM_NOTIFY:
		if (reinterpret_cast<NMHDR*>(lparam)->code == PSN_APPLY) {
			char str[128];

			GetDlgItemTextA(hwnd, IDC_UN, str, _countof(str));
			db_set_s(0, proto->ModuleName(), DBKEY_LOGIN, str);

			GetDlgItemTextA(hwnd, IDC_PW, str, _countof(str));
			db_set_s(0, proto->ModuleName(), DBKEY_PASS, str);
			return TRUE;
		}
		break;

	}

	return FALSE;
}
