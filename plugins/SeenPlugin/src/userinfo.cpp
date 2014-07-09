/*
"Last Seen mod" plugin for Miranda IM
Copyright ( C ) 2002-03  micron-x
Copyright ( C ) 2005-07  Y.B.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or ( at your option ) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "seen.h"

LRESULT CALLBACK EditProc(HWND hdlg,UINT msg,WPARAM wparam,LPARAM lparam)
{
	switch(msg){
	case WM_SETCURSOR:
		SetCursor(LoadCursor(NULL,IDC_ARROW));
		return 1;
	}
	return mir_callNextSubclass(hdlg, EditProc, msg, wparam, lparam);
}

INT_PTR CALLBACK UserinfoDlgProc(HWND hdlg,UINT msg,WPARAM wparam,LPARAM lparam)
{
	DBVARIANT dbv;
	
	switch(msg) {
	case WM_INITDIALOG:
		mir_subclassWindow( GetDlgItem(hdlg, IDC_INFOTEXT), EditProc);
		{
			TCHAR *szout;
			if ( !db_get_ts(NULL, S_MOD, "UserStamp", &dbv)) {
				szout = ParseString(dbv.ptszVal, (MCONTACT)lparam, 0);
				db_free(&dbv);
			}
			else szout = ParseString(DEFAULT_USERSTAMP, (MCONTACT)lparam, 0);

			SetDlgItemText(hdlg, IDC_INFOTEXT, szout);
			if ( !lstrcmp(szout, TranslateT("<unknown>")))
				EnableWindow( GetDlgItem(hdlg, IDC_INFOTEXT), FALSE);
		}
		break;

	case WM_COMMAND:
		if (HIWORD(wparam) == EN_SETFOCUS)
			SetFocus( GetParent(hdlg));
		break;
	}

	return 0;
}

int UserinfoInit(WPARAM wparam,LPARAM lparam)
{
	MCONTACT hContact = (MCONTACT)lparam;
	char *szProto = GetContactProto(hContact);

	if (IsWatchedProtocol(szProto) && !db_get_b(hContact, szProto, "ChatRoom", false)) {
		OPTIONSDIALOGPAGE uip = { sizeof(uip) };
		uip.hInstance = hInstance;
		uip.pszTemplate = MAKEINTRESOURCEA(IDD_USERINFO);
		uip.flags = ODPF_TCHAR;
		uip.ptszTitle = LPGENT("Last seen");
		uip.pfnDlgProc = UserinfoDlgProc;
		UserInfo_AddPage(wparam, &uip);
	}
	return 0;
}
