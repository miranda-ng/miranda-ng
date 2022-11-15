////////////////////////////////////////////////////////////////////////////////
// Gadu-Gadu Plugin for Miranda IM
//
// Copyright (c) 2003-2006 Adam Strzelecki <ono+miranda@java.pl>
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
////////////////////////////////////////////////////////////////////////////////

#include "gg.h"

////////////////////////////////////////////////////////////////////////////////
// Check if new user data has been filled in for specified account
//

void GaduProto::checknewuser(uin_t uin, const char* passwd)
{
	char oldpasswd[128];
	DBVARIANT dbv;

	oldpasswd[0] = '\0';
	if (!getString(GG_KEY_PASSWORD, &dbv)) {
		if (dbv.pszVal)
			mir_strcpy(oldpasswd, dbv.pszVal);
		db_free(&dbv);
	}

	uin_t olduin = (uin_t)getDword(GG_KEY_UIN, 0);
	if (uin > 0 && mir_strlen(passwd) > 0 && (uin != olduin || mir_strcmp(oldpasswd, passwd)))
		check_first_conn = 1;
}

////////////////////////////////////////////////////////////////////////////////
// Options Page : Proc

void gg_optsdlgcheck(HWND m_hwnd)
{
	wchar_t text[128];
	GetDlgItemText(m_hwnd, IDC_UIN, text, _countof(text));
	if (text[0]) {
		GetDlgItemText(m_hwnd, IDC_EMAIL, text, _countof(text));
		if (text[0])
			ShowWindow(GetDlgItem(m_hwnd, IDC_CHEMAIL), SW_SHOW);
		else
			ShowWindow(GetDlgItem(m_hwnd, IDC_CHEMAIL), SW_HIDE);
		ShowWindow(GetDlgItem(m_hwnd, IDC_CHPASS), SW_SHOW);
		ShowWindow(GetDlgItem(m_hwnd, IDC_LOSTPASS), SW_SHOW);
		ShowWindow(GetDlgItem(m_hwnd, IDC_REMOVEACCOUNT), SW_SHOW);
		ShowWindow(GetDlgItem(m_hwnd, IDC_CREATEACCOUNT), SW_HIDE);
	}
	else {
		ShowWindow(GetDlgItem(m_hwnd, IDC_REMOVEACCOUNT), SW_HIDE);
		ShowWindow(GetDlgItem(m_hwnd, IDC_LOSTPASS), SW_HIDE);
		ShowWindow(GetDlgItem(m_hwnd, IDC_CHPASS), SW_HIDE);
		ShowWindow(GetDlgItem(m_hwnd, IDC_CHEMAIL), SW_HIDE);
		ShowWindow(GetDlgItem(m_hwnd, IDC_CREATEACCOUNT), SW_SHOW);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////
// Proc: Account manager options dialog

INT_PTR CALLBACK gg_acc_mgr_guidlgproc(HWND m_hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	GaduProto *gg = (GaduProto *)GetWindowLongPtr(m_hwnd, GWLP_USERDATA);

	switch (msg) {
	case WM_INITDIALOG:
		{
			DBVARIANT dbv;
			gg = (GaduProto *)lParam;
			SetWindowLongPtr(m_hwnd, GWLP_USERDATA, (LONG_PTR)lParam);

			TranslateDialogDefault(m_hwnd);
			uint32_t num = gg->getDword(GG_KEY_UIN, 0);
			if (num)
				SetDlgItemTextA(m_hwnd, IDC_UIN, ditoa(num));
			if (!gg->getString(GG_KEY_PASSWORD, &dbv)) {
				SetDlgItemTextA(m_hwnd, IDC_PASSWORD, dbv.pszVal);
				db_free(&dbv);
			}
			if (!gg->getString(GG_KEY_EMAIL, &dbv)) {
				SetDlgItemTextA(m_hwnd, IDC_EMAIL, dbv.pszVal);
				db_free(&dbv);
			}
			break;
		}
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_CREATEACCOUNT:
			{
				// Readup data
				GGUSERUTILDLGDATA dat;
				char pass[128], email[128];
				GetDlgItemTextA(m_hwnd, IDC_UIN, pass, _countof(pass));
				dat.uin = atoi(pass);
				GetDlgItemTextA(m_hwnd, IDC_PASSWORD, pass, _countof(pass));
				GetDlgItemTextA(m_hwnd, IDC_EMAIL, email, _countof(email));
				dat.pass = pass;
				dat.email = email;
				dat.gg = gg;
				dat.mode = GG_USERUTIL_CREATE;
				int ret = DialogBoxParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_CREATEACCOUNT), m_hwnd, gg_userutildlgproc, (LPARAM)&dat);

				if (ret == IDOK) {
					DBVARIANT dbv;
					uint32_t num;
					// Show reload required window
					ShowWindow(GetDlgItem(m_hwnd, IDC_RELOADREQD), SW_SHOW);

					// Update uin
					if (num = gg->getDword(GG_KEY_UIN, 0))
						SetDlgItemTextA(m_hwnd, IDC_UIN, ditoa(num));
					else
						SetDlgItemTextA(m_hwnd, IDC_UIN, "");

					// Update password
					if (!gg->getString(GG_KEY_PASSWORD, &dbv)) {
						SetDlgItemTextA(m_hwnd, IDC_PASSWORD, dbv.pszVal);
						db_free(&dbv);
					}
					else SetDlgItemTextA(m_hwnd, IDC_PASSWORD, "");

					// Update e-mail
					if (!gg->getString(GG_KEY_EMAIL, &dbv)) {
						SetDlgItemTextA(m_hwnd, IDC_EMAIL, dbv.pszVal);
						db_free(&dbv);
					}
					else SetDlgItemTextA(m_hwnd, IDC_EMAIL, "");
				}
			}
			break;
		
		case IDC_UIN:
		case IDC_PASSWORD:
		case IDC_EMAIL:
			if (HIWORD(wParam) == EN_CHANGE && (HWND)lParam == GetFocus()) {
				SendMessage(GetParent(m_hwnd), PSM_CHANGED, 0, 0);
				break;
			}
		}
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_APPLY:
				char str[128];

				// Write Gadu-Gadu number & password
				GetDlgItemTextA(m_hwnd, IDC_UIN, str, _countof(str));
				uin_t uin = atoi(str);
				GetDlgItemTextA(m_hwnd, IDC_PASSWORD, str, _countof(str));
				gg->checknewuser(uin, str);
				gg->setDword(GG_KEY_UIN, uin);
				gg->setString(GG_KEY_PASSWORD, str);

				// Write Gadu-Gadu email
				GetDlgItemTextA(m_hwnd, IDC_EMAIL, str, _countof(str));
				gg->setString(GG_KEY_EMAIL, str);
			}
		}
		break;
	}
	return FALSE;
}
