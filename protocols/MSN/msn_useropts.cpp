/*
Plugin of Miranda IM for communicating with users of the MSN Messenger protocol.
Copyright (c) 2006-2012 Boris Krasnovskiy.
Copyright (c) 2003-2005 George Hazan.
Copyright (c) 2002-2003 Richard Hughes (original version).

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "msn_global.h"

/////////////////////////////////////////////////////////////////////////////////////////
// MSN contact option page dialog procedure.

/*
INT_PTR CALLBACK MsnDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) 
	{
		case WM_INITDIALOG:
			{
				TranslateDialogDefault(hwndDlg);
				
				const HANDLE hContact = (HANDLE)lParam;

				SetDlgItemInt(hwndDlg, IDC_MOBILE, DBGetContactSettingByte(hContact, m_szModuleName, "MobileAllowed", 0), 0);
				SetDlgItemInt(hwndDlg, IDC_MSN_MOBILE, DBGetContactSettingByte(hContact, m_szModuleName, "MobileEnabled", 0), 0);

				DWORD dwFlagBits = setDword(hContact, "FlagBits", 0);
				SetDlgItemTextA(hwndDlg, IDC_WEBMESSENGER, (dwFlagBits & 0x200) ? "Y" : "N");
			}

			return TRUE;

		case WM_DESTROY:
			break;
	}

	return FALSE;
}
*/
/////////////////////////////////////////////////////////////////////////////////////////
// MsnOnDetailsInit - initializes user info dialog pages.

int MsnOnDetailsInit(WPARAM wParam, LPARAM lParam)
{
/*
	if (!MSN_IsMyContact(hContact))
		return 0;

	if (getDword(hContact, "FlagBits", 0)) 
	{
		OPTIONSDIALOGPAGE odp = {0};

		odp.cbSize      = sizeof(odp);
		odp.pfnDlgProc  = MsnDlgProc;
		odp.position    = -1900000000;
		odp.pszTemplate = MAKEINTRESOURCEA(IDD_USEROPTS);
		odp.ptszTitle   = m_tszUserName;
		odp.flags       = ODPF_TCHAR | ODPF_DONTTRANSLATE;

		MSN_CallService(MS_USERINFO_ADDPAGE, wParam, (LPARAM)&odp);
	}
*/	
	return 0;
}
