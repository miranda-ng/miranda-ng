/*
IRC plugin for Miranda IM

Copyright (C) 2003-05 Jurgen Persson
Copyright (C) 2007-09 George Hazan

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

#include "irc.h"

/////////////////////////////////////////////////////////////////////////////////////////
// 'User details' dialog

struct UserDetailsDlgProcParam
{
	UserDetailsDlgProcParam(CIrcProto* _pro, MCONTACT _info) :
		ppro(_pro),
		hContact(_info)
	{}

	CIrcProto *ppro;
	MCONTACT hContact;
};

#define STR_BASIC LPGEN("Faster! Searches the network for an exact match of the nickname only. The hostmask is optional and provides further security if used. Wildcards (? and *) are allowed.")
#define STR_ADVANCED LPGEN("Slower! Searches the network for nicknames matching a wildcard string. The hostmask is mandatory and a minimum of 4 characters is necessary in the \"Nick\" field. Wildcards (? and *) are allowed.")
#define STR_ERROR LPGEN("Settings could not be saved!\n\nThe \"Nick\" field must contain at least four characters including wildcards,\n and it must also match the default nickname for this contact.")
#define STR_ERROR2 LPGEN("Settings could not be saved!\n\nA full hostmask must be set for this online detection mode to work.")

INT_PTR CALLBACK UserDetailsDlgProc(HWND m_hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	UserDetailsDlgProcParam *p = (UserDetailsDlgProcParam*)GetWindowLongPtr(m_hwnd, GWLP_USERDATA);
	switch (msg) {
	case WM_INITDIALOG:
		p = new UserDetailsDlgProcParam(NULL, lParam);
		SetWindowLongPtr(m_hwnd, GWLP_USERDATA, (LPARAM)p);
		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->idFrom == 0 && ((LPNMHDR)lParam)->code == PSN_PARAMCHANGED) {
			p->ppro = (CIrcProto*)((PSHNOTIFY*)lParam)->lParam;

			DBVARIANT dbv;
			BYTE bAdvanced = p->ppro->getByte(p->hContact, "AdvancedMode", 0);

			TranslateDialogDefault(m_hwnd);

			CheckDlgButton(m_hwnd, IDC_RADIO1, bAdvanced ? BST_UNCHECKED : BST_CHECKED);
			CheckDlgButton(m_hwnd, IDC_RADIO2, bAdvanced ? BST_CHECKED : BST_UNCHECKED);
			EnableWindow(GetDlgItem(m_hwnd, IDC_WILDCARD), bAdvanced);

			if (!bAdvanced) {
				SetDlgItemText(m_hwnd, IDC_DEFAULT, TranslateT(STR_BASIC));
				if (!p->ppro->getTString(p->hContact, "Default", &dbv)) {
					SetDlgItemText(m_hwnd, IDC_WILDCARD, dbv.ptszVal);
					db_free(&dbv);
				}
			}
			else {
				SetDlgItemText(m_hwnd, IDC_DEFAULT, TranslateT(STR_ADVANCED));
				if (!p->ppro->getTString(p->hContact, "UWildcard", &dbv)) {
					SetDlgItemText(m_hwnd, IDC_WILDCARD, dbv.ptszVal);
					db_free(&dbv);
				}
			}

			if (!p->ppro->getTString(p->hContact, "UUser", &dbv)) {
				SetDlgItemText(m_hwnd, IDC_USER, dbv.ptszVal);
				db_free(&dbv);
			}

			if (!p->ppro->getTString(p->hContact, "UHost", &dbv)) {
				SetDlgItemText(m_hwnd, IDC_HOST, dbv.ptszVal);
				db_free(&dbv);
			}
			ProtoBroadcastAck(p->ppro->m_szModuleName, p->hContact, ACKTYPE_GETINFO, ACKRESULT_SUCCESS, (HANDLE)1, 0);
		}
		break;

	case WM_COMMAND:
		if ((LOWORD(wParam) == IDC_WILDCARD || LOWORD(wParam) == IDC_USER || LOWORD(wParam) == IDC_HOST) &&
			(HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus()))
			return true;

		EnableWindow(GetDlgItem(m_hwnd, IDC_BUTTON), true);
		EnableWindow(GetDlgItem(m_hwnd, IDC_BUTTON2), true);

		if (HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == IDC_BUTTON) {
			TCHAR temp[500];
			GetDlgItemText(m_hwnd, IDC_WILDCARD, temp, SIZEOF(temp));

			BYTE bAdvanced = IsDlgButtonChecked(m_hwnd, IDC_RADIO1) ? 0 : 1;
			if (bAdvanced) {
				if (GetWindowTextLength(GetDlgItem(m_hwnd, IDC_WILDCARD)) == 0 ||
					GetWindowTextLength(GetDlgItem(m_hwnd, IDC_USER)) == 0 ||
					GetWindowTextLength(GetDlgItem(m_hwnd, IDC_HOST)) == 0) {
					MessageBox(NULL, TranslateT(STR_ERROR2), TranslateT("IRC error"), MB_OK | MB_ICONERROR);
					return FALSE;
				}

				DBVARIANT dbv;
				if (!p->ppro->getTString(p->hContact, "Default", &dbv)) {
					CMString S = _T(STR_ERROR);
					S += _T(" (");
					S += dbv.ptszVal;
					S += _T(")");
					if ((mir_tstrlen(temp) < 4 && mir_tstrlen(temp)) || !WCCmp(CharLower(temp), CharLower(dbv.ptszVal))) {
						MessageBox(NULL, TranslateTS(S.c_str()), TranslateT("IRC error"), MB_OK | MB_ICONERROR);
						db_free(&dbv);
						return FALSE;
					}
					db_free(&dbv);
				}

				GetDlgItemText(m_hwnd, IDC_WILDCARD, temp, SIZEOF(temp));
				if (mir_tstrlen(GetWord(temp, 0).c_str()))
					p->ppro->setTString(p->hContact, "UWildcard", GetWord(temp, 0).c_str());
				else
					db_unset(p->hContact, p->ppro->m_szModuleName, "UWildcard");
			}

			p->ppro->setByte(p->hContact, "AdvancedMode", bAdvanced);

			GetDlgItemText(m_hwnd, IDC_USER, temp, SIZEOF(temp));
			if (mir_tstrlen(GetWord(temp, 0).c_str()))
				p->ppro->setTString(p->hContact, "UUser", GetWord(temp, 0).c_str());
			else
				db_unset(p->hContact, p->ppro->m_szModuleName, "UUser");

			GetDlgItemText(m_hwnd, IDC_HOST, temp, SIZEOF(temp));
			if (mir_tstrlen(GetWord(temp, 0).c_str()))
				p->ppro->setTString(p->hContact, "UHost", GetWord(temp, 0).c_str());
			else
				db_unset(p->hContact, p->ppro->m_szModuleName, "UHost");

			EnableWindow(GetDlgItem(m_hwnd, IDC_BUTTON), FALSE);
		}

		if (HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == IDC_BUTTON2) {
			if (IsDlgButtonChecked(m_hwnd, IDC_RADIO2))
				SetDlgItemTextA(m_hwnd, IDC_WILDCARD, "");
			SetDlgItemTextA(m_hwnd, IDC_HOST, "");
			SetDlgItemTextA(m_hwnd, IDC_USER, "");
			db_unset(p->hContact, p->ppro->m_szModuleName, "UWildcard");
			db_unset(p->hContact, p->ppro->m_szModuleName, "UUser");
			db_unset(p->hContact, p->ppro->m_szModuleName, "UHost");
			EnableWindow(GetDlgItem(m_hwnd, IDC_BUTTON), FALSE);
			EnableWindow(GetDlgItem(m_hwnd, IDC_BUTTON2), FALSE);
		}

		if (HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == IDC_RADIO1) {
			SetDlgItemText(m_hwnd, IDC_DEFAULT, TranslateT(STR_BASIC));

			DBVARIANT dbv;
			if (!p->ppro->getTString(p->hContact, "Default", &dbv)) {
				SetDlgItemText(m_hwnd, IDC_WILDCARD, dbv.ptszVal);
				db_free(&dbv);
			}
			EnableWindow(GetDlgItem(m_hwnd, IDC_WILDCARD), FALSE);
		}

		if (HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == IDC_RADIO2) {
			DBVARIANT dbv;
			SetDlgItemText(m_hwnd, IDC_DEFAULT, TranslateT(STR_ADVANCED));
			if (!p->ppro->getTString(p->hContact, "UWildcard", &dbv)) {
				SetDlgItemText(m_hwnd, IDC_WILDCARD, dbv.ptszVal);
				db_free(&dbv);
			}
			EnableWindow(GetDlgItem(m_hwnd, IDC_WILDCARD), true);
		}
		break;
	}
	return FALSE;
}

int __cdecl CIrcProto::OnInitUserInfo(WPARAM wParam, LPARAM lParam)
{
	MCONTACT hContact = lParam;
	char *szProto = GetContactProto(hContact);
	if (!hContact || !szProto || mir_strcmpi(szProto, m_szModuleName))
		return 0;

	if (isChatRoom(hContact))
		return 0;

	if (getByte(hContact, "DCC", 0) != 0)
		return 0;

	DBVARIANT dbv;
	if (!getTString(hContact, "Default", &dbv)) {
		if (IsChannel(dbv.ptszVal)) {
			db_free(&dbv);
			return 0;
		}
		db_free(&dbv);
	}

	OPTIONSDIALOGPAGE odp = { 0 };
	odp.flags = ODPF_DONTTRANSLATE;
	odp.pszTitle = m_szModuleName;
	odp.hIcon = NULL;
	odp.dwInitParam = (LPARAM)this;
	odp.hInstance = hInst;
	odp.position = -1900000000;
	odp.pfnDlgProc = UserDetailsDlgProc;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_USERINFO);
	odp.pszTitle = m_szModuleName;
	UserInfo_AddPage(wParam, &odp);
	return 0;
}
