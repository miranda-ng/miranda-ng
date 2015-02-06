/*

Omegle plugin for Miranda Instant Messenger
_____________________________________________

Copyright © 2011-15 Robert Pösel

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

#include "common.h"

static BOOL LoadDBCheckState(OmegleProto* ppro, HWND hwnd, int idCtrl, const char* szSetting, BYTE bDef = 0)
{
	BOOL state = db_get_b(NULL, ppro->m_szModuleName, szSetting, bDef);
	CheckDlgButton(hwnd, idCtrl, state ? BST_CHECKED : BST_UNCHECKED);
	return state;
}

static BOOL StoreDBCheckState(OmegleProto* ppro, HWND hwnd, int idCtrl, const char* szSetting)
{
	BOOL state = IsDlgButtonChecked(hwnd, idCtrl);
	db_set_b(NULL, ppro->m_szModuleName, szSetting, (BYTE)state);
	return state;
}

static void LoadDBText(OmegleProto* ppro, HWND hwnd, int idCtrl, const char* szSetting)
{
	ptrT tstr( db_get_tsa(NULL, ppro->m_szModuleName, szSetting));
	if (tstr)
		SetDlgItemText(hwnd, idCtrl, tstr);
}

static void StoreDBText(OmegleProto* ppro, HWND hwnd, int idCtrl, const char* szSetting)
{
	TCHAR tstr[250+1];

	GetDlgItemText(hwnd, idCtrl, tstr, SIZEOF(tstr));
	if (tstr[0] != '\0') {
		db_set_ts(NULL, ppro->m_szModuleName, szSetting, tstr);
	} else {
		db_unset(NULL, ppro->m_szModuleName, szSetting);
	}
}


INT_PTR CALLBACK OmegleAccountProc( HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam )
{
	OmegleProto *proto;

	switch ( message )
	{

	case WM_INITDIALOG:
		TranslateDialogDefault(hwnd);

		proto = reinterpret_cast<OmegleProto*>(lparam);
		SetWindowLongPtr(hwnd,GWLP_USERDATA,lparam);

		SendDlgItemMessage(hwnd,IDC_INTERESTS, EM_LIMITTEXT, 512, 0);

		// Server
		SendDlgItemMessageA(hwnd, IDC_SERVER, CB_INSERTSTRING, 0, reinterpret_cast<LPARAM>(Translate(servers[0])));
		for(size_t i=1; i<SIZEOF(servers); i++)
			SendDlgItemMessageA(hwnd, IDC_SERVER, CB_INSERTSTRING, i, reinterpret_cast<LPARAM>(servers[i]));
		SendDlgItemMessage(hwnd, IDC_SERVER, CB_SETCURSEL, db_get_b(NULL, proto->m_szModuleName, OMEGLE_KEY_SERVER, 0), 0);

		// Language
		for(size_t i=0; i<SIZEOF(languages); i++)
			SendDlgItemMessageA(hwnd, IDC_LANGUAGE, CB_INSERTSTRING, i, reinterpret_cast<LPARAM>(Translate(languages[i].lang)));
		SendDlgItemMessage(hwnd, IDC_LANGUAGE, CB_SETCURSEL, db_get_b(NULL, proto->m_szModuleName, OMEGLE_KEY_LANGUAGE, 0), 0);

		LoadDBText(proto, hwnd, IDC_NAME, OMEGLE_KEY_NAME);
		LoadDBText(proto, hwnd, IDC_INTERESTS, OMEGLE_KEY_INTERESTS);
		LoadDBCheckState(proto, hwnd, IDC_MEET_COMMON, OMEGLE_KEY_MEET_COMMON);

		return TRUE;

	case WM_COMMAND:
		switch(LOWORD(wparam))
		{
			case IDC_LANGUAGE:
			case IDC_SERVER:	
				if (HIWORD(wparam) == CBN_SELCHANGE) {
					SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
				} break;
			case IDC_NAME:
			case IDC_INTERESTS:
				if (HIWORD(wparam) != EN_CHANGE || (HWND) lparam != GetFocus()) {
					return TRUE;
				} else {
					SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
				} break;
			case IDC_MEET_COMMON:
				if (HIWORD(wparam) == BN_CLICKED) {
					SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
				} break;
		}
		break;

	case WM_NOTIFY:
		if ( reinterpret_cast<NMHDR*>(lparam)->code == PSN_APPLY )
		{
			proto = reinterpret_cast<OmegleProto*>(GetWindowLongPtr(hwnd,GWLP_USERDATA));

			db_set_b(NULL, proto->m_szModuleName, OMEGLE_KEY_SERVER, SendDlgItemMessage(hwnd, IDC_SERVER, CB_GETCURSEL, 0, 0));
			db_set_b(NULL, proto->m_szModuleName, OMEGLE_KEY_LANGUAGE, SendDlgItemMessage(hwnd, IDC_LANGUAGE, CB_GETCURSEL, 0, 0));

			StoreDBText(proto, hwnd, IDC_NAME, OMEGLE_KEY_NAME);
			StoreDBText(proto, hwnd, IDC_INTERESTS, OMEGLE_KEY_INTERESTS);
			StoreDBCheckState(proto, hwnd, IDC_MEET_COMMON, OMEGLE_KEY_MEET_COMMON);

			return TRUE;
		}
		break;

	}
	return FALSE;
}

INT_PTR CALLBACK OmegleOptionsProc( HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam )
{
	OmegleProto *proto = reinterpret_cast<OmegleProto*>(GetWindowLongPtr(hwnd,GWLP_USERDATA));

	switch ( message )
	{

	case WM_INITDIALOG:
	{
		TranslateDialogDefault(hwnd);

		proto = reinterpret_cast<OmegleProto*>(lparam);
		SetWindowLongPtr(hwnd,GWLP_USERDATA,lparam);

		SendDlgItemMessage(hwnd,IDC_INTERESTS, EM_LIMITTEXT, 250, 0);
		SendDlgItemMessage(hwnd,IDC_HI_MESSAGE, EM_LIMITTEXT, 250, 0);
		SendDlgItemMessage(hwnd,IDC_ASL_MESSAGE, EM_LIMITTEXT, 250, 0);

		// Server
		SendDlgItemMessageA(hwnd, IDC_SERVER, CB_INSERTSTRING, 0, reinterpret_cast<LPARAM>(Translate(servers[0])));
		for(size_t i=1; i<SIZEOF(servers); i++)
			SendDlgItemMessageA(hwnd, IDC_SERVER, CB_INSERTSTRING, i, reinterpret_cast<LPARAM>(servers[i]));
		SendDlgItemMessage(hwnd, IDC_SERVER, CB_SETCURSEL, db_get_b(NULL, proto->m_szModuleName, OMEGLE_KEY_SERVER, 0), 0);

		// Language
		for(size_t i=0; i<SIZEOF(languages); i++)
			SendDlgItemMessageA(hwnd, IDC_LANGUAGE, CB_INSERTSTRING, i, reinterpret_cast<LPARAM>(Translate(languages[i].lang)));
		SendDlgItemMessage(hwnd, IDC_LANGUAGE, CB_SETCURSEL, db_get_b(NULL, proto->m_szModuleName, OMEGLE_KEY_LANGUAGE, 0), 0);

		LoadDBText(proto, hwnd, IDC_NAME, OMEGLE_KEY_NAME);
		LoadDBText(proto, hwnd, IDC_INTERESTS, OMEGLE_KEY_INTERESTS);
		LoadDBText(proto, hwnd, IDC_HI_MESSAGE, OMEGLE_KEY_HI);
		LoadDBText(proto, hwnd, IDC_ASL_MESSAGE, OMEGLE_KEY_ASL);
		LoadDBText(proto, hwnd, IDC_LAST_QUESTION, OMEGLE_KEY_LAST_QUESTION);

		LoadDBCheckState(proto, hwnd, IDC_MEET_COMMON, OMEGLE_KEY_MEET_COMMON);
		LoadDBCheckState(proto, hwnd, IDC_HI_ENABLED, OMEGLE_KEY_HI_ENABLED);
		LoadDBCheckState(proto, hwnd, IDC_NOCLEAR, OMEGLE_KEY_NO_CLEAR);
		LoadDBCheckState(proto, hwnd, IDC_DONTSTOP, OMEGLE_KEY_DONT_STOP);
		LoadDBCheckState(proto, hwnd, IDC_REUSE_QUESTIONS, OMEGLE_KEY_REUSE_QUESTION);
		LoadDBCheckState(proto, hwnd, IDC_SERVER_INFO, OMEGLE_KEY_SERVER_INFO);
		LoadDBCheckState(proto, hwnd, IDC_AUTO_CONNECT, OMEGLE_KEY_AUTO_CONNECT);		

	} return TRUE;

	case WM_COMMAND: {

		switch(LOWORD(wparam))
		{
			case IDC_SERVER:
			case IDC_LANGUAGE:
				if (HIWORD(wparam) == CBN_SELCHANGE) {
					SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
				} break;
			case IDC_NAME:
			case IDC_INTERESTS:			
			case IDC_HI_MESSAGE:
			case IDC_ASL_MESSAGE:
				if (HIWORD(wparam) != EN_CHANGE || (HWND) lparam != GetFocus()) {
					return TRUE;
				} else {
					SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
				} break;
			case IDC_MEET_COMMON:
			case IDC_HI_ENABLED:
			case IDC_NOCLEAR:
			case IDC_DONTSTOP:
			case IDC_REUSE_QUESTIONS:
			case IDC_SERVER_INFO:
			case IDC_AUTO_CONNECT:
				if (HIWORD(wparam) == BN_CLICKED) {
					SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
				} break;
		}

	} break;

	case WM_NOTIFY:
		if ( reinterpret_cast<NMHDR*>(lparam)->code == PSN_APPLY )
		{
			proto = reinterpret_cast<OmegleProto*>(GetWindowLongPtr(hwnd,GWLP_USERDATA));

			db_set_b(NULL, proto->m_szModuleName, OMEGLE_KEY_SERVER, SendDlgItemMessage(hwnd, IDC_SERVER, CB_GETCURSEL, 0, 0));
			db_set_b(NULL, proto->m_szModuleName, OMEGLE_KEY_LANGUAGE, SendDlgItemMessage(hwnd, IDC_LANGUAGE, CB_GETCURSEL, 0, 0));

			StoreDBText(proto, hwnd, IDC_NAME, OMEGLE_KEY_NAME);
			StoreDBText(proto, hwnd, IDC_INTERESTS, OMEGLE_KEY_INTERESTS);
			StoreDBText(proto, hwnd, IDC_HI_MESSAGE, OMEGLE_KEY_HI);
			StoreDBText(proto, hwnd, IDC_ASL_MESSAGE, OMEGLE_KEY_ASL);

			StoreDBCheckState(proto, hwnd, IDC_MEET_COMMON, OMEGLE_KEY_MEET_COMMON);
			StoreDBCheckState(proto, hwnd, IDC_HI_ENABLED, OMEGLE_KEY_HI_ENABLED);
			StoreDBCheckState(proto, hwnd, IDC_NOCLEAR, OMEGLE_KEY_NO_CLEAR);
			StoreDBCheckState(proto, hwnd, IDC_DONTSTOP, OMEGLE_KEY_DONT_STOP);
			StoreDBCheckState(proto, hwnd, IDC_REUSE_QUESTIONS, OMEGLE_KEY_REUSE_QUESTION);
			StoreDBCheckState(proto, hwnd, IDC_SERVER_INFO, OMEGLE_KEY_SERVER_INFO);
			StoreDBCheckState(proto, hwnd, IDC_AUTO_CONNECT, OMEGLE_KEY_AUTO_CONNECT);
			
			return TRUE;
		}
		break;

	}
	
	return FALSE;
}
