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

#include "stdafx.h"

/////////////////////////////////////////////////////////////////////////////////////////
// 'User details' dialog

const wchar_t *STR_BASIC = LPGENW("Faster! Searches the network for an exact match of the nickname only. The hostmask is optional and provides further security if used. Wildcards (? and *) are allowed.");
const wchar_t *STR_ADVANCED = LPGENW("Slower! Searches the network for nicknames matching a wildcard string. The hostmask is mandatory and a minimum of 4 characters is necessary in the \"Nick\" field. Wildcards (? and *) are allowed.");
const wchar_t *STR_ERROR = LPGENW("Settings could not be saved!\n\nThe \"Nick\" field must contain at least four characters including wildcards,\n and it must also match the default nickname for this contact.");
const wchar_t *STR_ERROR2 = LPGENW("Settings could not be saved!\n\nA full hostmask must be set for this online detection mode to work.");

class IrcUserInfoDlg : public CUserInfoPageDlg
{
	CIrcProto *ppro;
	CCtrlButton btn1, btn2;
	CCtrlCheck radio1, radio2;

public:
	IrcUserInfoDlg(CIrcProto *_ppro) :
		CUserInfoPageDlg(g_plugin, IDD_USERINFO),
		ppro(_ppro),
		btn1(this, IDC_BUTTON),
		btn2(this, IDC_BUTTON2),
		radio1(this, IDC_RADIO1),
		radio2(this, IDC_RADIO2)
	{
		btn1.OnClick = Callback(this, &IrcUserInfoDlg::onClick_Button1);
		btn2.OnClick = Callback(this, &IrcUserInfoDlg::onClick_Button2);

		radio1.OnChange = Callback(this, &IrcUserInfoDlg::onChange_Radio1);
		radio2.OnChange = Callback(this, &IrcUserInfoDlg::onChange_Radio2);
	}

	bool OnRefresh() override
	{
		DBVARIANT dbv;
		uint8_t bAdvanced = ppro->getByte(m_hContact, "AdvancedMode", 0);

		TranslateDialogDefault(m_hwnd);

		radio1.SetState(!bAdvanced);
		radio2.SetState(bAdvanced);
		EnableWindow(GetDlgItem(m_hwnd, IDC_WILDCARD), bAdvanced);

		if (!bAdvanced) {
			SetDlgItemText(m_hwnd, IDC_DEFAULT, TranslateW(STR_BASIC));
			if (!ppro->getWString(m_hContact, "Default", &dbv)) {
				SetDlgItemText(m_hwnd, IDC_WILDCARD, dbv.pwszVal);
				db_free(&dbv);
			}
		}
		else {
			SetDlgItemText(m_hwnd, IDC_DEFAULT, TranslateW(STR_ADVANCED));
			if (!ppro->getWString(m_hContact, "UWildcard", &dbv)) {
				SetDlgItemText(m_hwnd, IDC_WILDCARD, dbv.pwszVal);
				db_free(&dbv);
			}
		}

		if (!ppro->getWString(m_hContact, "UUser", &dbv)) {
			SetDlgItemText(m_hwnd, IDC_USER, dbv.pwszVal);
			db_free(&dbv);
		}

		if (!ppro->getWString(m_hContact, "UHost", &dbv)) {
			SetDlgItemText(m_hwnd, IDC_HOST, dbv.pwszVal);
			db_free(&dbv);
		}
		ProtoBroadcastAck(ppro->m_szModuleName, m_hContact, ACKTYPE_GETINFO, ACKRESULT_SUCCESS, (HANDLE)1);
		return false;
	}

	void OnChange() override
	{
		btn1.Enable();
		btn2.Enable();
	}

	void onClick_Button1(CCtrlButton *)
	{
		wchar_t temp[500];
		GetDlgItemText(m_hwnd, IDC_WILDCARD, temp, _countof(temp));

		bool bAdvanced = !radio1.GetState();
		if (bAdvanced) {
			if (GetWindowTextLength(GetDlgItem(m_hwnd, IDC_WILDCARD)) == 0 ||
				GetWindowTextLength(GetDlgItem(m_hwnd, IDC_USER)) == 0 ||
				GetWindowTextLength(GetDlgItem(m_hwnd, IDC_HOST)) == 0) {
				MessageBox(nullptr, TranslateW(STR_ERROR2), TranslateT("IRC error"), MB_OK | MB_ICONERROR);
				return;
			}

			DBVARIANT dbv;
			if (!ppro->getWString(m_hContact, "Default", &dbv)) {
				CMStringW S = STR_ERROR;
				S += L" (";
				S += dbv.pwszVal;
				S += L")";
				if ((mir_wstrlen(temp) < 4 && mir_wstrlen(temp)) || !WCCmp(CharLower(temp), CharLower(dbv.pwszVal))) {
					MessageBox(nullptr, TranslateW(S.c_str()), TranslateT("IRC error"), MB_OK | MB_ICONERROR);
					db_free(&dbv);
					return;
				}
				db_free(&dbv);
			}

			GetDlgItemText(m_hwnd, IDC_WILDCARD, temp, _countof(temp));
			if (mir_wstrlen(GetWord(temp, 0).c_str()))
				ppro->setWString(m_hContact, "UWildcard", GetWord(temp, 0).c_str());
			else
				db_unset(m_hContact, ppro->m_szModuleName, "UWildcard");
		}

		ppro->setByte(m_hContact, "AdvancedMode", bAdvanced);

		GetDlgItemText(m_hwnd, IDC_USER, temp, _countof(temp));
		if (mir_wstrlen(GetWord(temp, 0).c_str()))
			ppro->setWString(m_hContact, "UUser", GetWord(temp, 0).c_str());
		else
			db_unset(m_hContact, ppro->m_szModuleName, "UUser");

		GetDlgItemText(m_hwnd, IDC_HOST, temp, _countof(temp));
		if (mir_wstrlen(GetWord(temp, 0).c_str()))
			ppro->setWString(m_hContact, "UHost", GetWord(temp, 0).c_str());
		else
			db_unset(m_hContact, ppro->m_szModuleName, "UHost");

		btn1.Disable();
	}

	void onClick_Button2(CCtrlButton *)
	{
		if (radio2.GetState())
			SetDlgItemTextA(m_hwnd, IDC_WILDCARD, "");
		SetDlgItemTextA(m_hwnd, IDC_HOST, "");
		SetDlgItemTextA(m_hwnd, IDC_USER, "");
		db_unset(m_hContact, ppro->m_szModuleName, "UWildcard");
		db_unset(m_hContact, ppro->m_szModuleName, "UUser");
		db_unset(m_hContact, ppro->m_szModuleName, "UHost");
		btn1.Disable();
		btn2.Disable();
	}

	void onChange_Radio1(CCtrlButton *)
	{
		SetDlgItemText(m_hwnd, IDC_DEFAULT, TranslateW(STR_BASIC));

		DBVARIANT dbv;
		if (!ppro->getWString(m_hContact, "Default", &dbv)) {
			SetDlgItemText(m_hwnd, IDC_WILDCARD, dbv.pwszVal);
			db_free(&dbv);
		}
		EnableWindow(GetDlgItem(m_hwnd, IDC_WILDCARD), FALSE);
	}

	void onChange_Radio2(CCtrlButton *)
	{
		DBVARIANT dbv;
		SetDlgItemText(m_hwnd, IDC_DEFAULT, TranslateW(STR_ADVANCED));
		if (!ppro->getWString(m_hContact, "UWildcard", &dbv)) {
			SetDlgItemText(m_hwnd, IDC_WILDCARD, dbv.pwszVal);
			db_free(&dbv);
		}
		EnableWindow(GetDlgItem(m_hwnd, IDC_WILDCARD), true);
	}
};

int __cdecl CIrcProto::OnInitUserInfo(WPARAM wParam, LPARAM hContact)
{
	char *szProto = Proto_GetBaseAccountName(hContact);
	if (!hContact || !szProto || mir_strcmpi(szProto, m_szModuleName))
		return 0;

	if (isChatRoom(hContact))
		return 0;

	if (getByte(hContact, "DCC", 0) != 0)
		return 0;

	ptrW wszChannel(getWStringA(hContact, "Default"));
	if (!wszChannel || !IsChannel(wszChannel))
		return 0;

	USERINFOPAGE uip = {};
	uip.position = -1900000000;
	uip.flags = ODPF_DONTTRANSLATE | ODPF_UNICODE;
	uip.pDialog = new IrcUserInfoDlg(this);
	uip.szTitle.w = m_tszUserName;
	g_plugin.addUserInfo(wParam, &uip);
	return 0;
}
