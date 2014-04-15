/*
    ICQ Corporate protocol plugin for Miranda IM.
    Copyright (C) 2003-2005 Eugene Tarasenko <zlyden13@inbox.ru>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "corp.h"

///////////////////////////////////////////////////////////////////////////////

static BOOL CALLBACK icqOptionsDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    DBVARIANT dbv;
    char str[128];

    switch (msg)
    {
    case WM_INITDIALOG:
        TranslateDialogDefault(hWnd);
        SetDlgItemInt(hWnd, IDC_OPT_UIN, db_get_dw(NULL, protoName, "UIN", 0), FALSE);
        if (!db_get(NULL, protoName, "Password", &dbv))
        {
            SetDlgItemText(hWnd, IDC_OPT_PASSWORD, dbv.pszVal);
            db_free(&dbv);
        }
        if(!db_get(NULL, protoName, "Server", &dbv))
        {
            SetDlgItemText(hWnd, IDC_OPT_SERVER, dbv.pszVal);
            db_free(&dbv);
        }
        SetDlgItemInt(hWnd, IDC_OPT_PORT, db_get_w(NULL, protoName, "Port", 4000), FALSE);
        ShowWindow(GetDlgItem(hWnd, IDC_OPT_RECONNECT), SW_HIDE);
        return TRUE;

    case WM_NOTIFY:
        switch (((LPNMHDR)lParam)->code)
        {
        case PSN_APPLY:
            db_set_dw(NULL, protoName, "UIN", (DWORD)GetDlgItemInt(hWnd, IDC_OPT_UIN, NULL, FALSE));
            GetDlgItemText(hWnd, IDC_OPT_PASSWORD, str, sizeof(str));
            db_set_s(NULL, protoName, "Password", str);
            GetDlgItemText(hWnd, IDC_OPT_SERVER, str, sizeof(str));
            db_set_s(NULL, protoName, "Server", str);
            db_set_w(NULL, protoName, "Port", (WORD)GetDlgItemInt(hWnd, IDC_OPT_PORT, NULL, FALSE));
            return TRUE;
        }
        break;

    case WM_COMMAND:
        if ((LOWORD(wParam) == IDC_OPT_UIN || LOWORD(wParam) == IDC_OPT_PASSWORD || LOWORD(wParam) == IDC_OPT_SERVER || LOWORD(wParam) == IDC_OPT_PORT) && (HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus())) return 0;
        SendMessage(GetParent(hWnd), PSM_CHANGED, 0, 0);
        break;
    }
    return FALSE;
}

///////////////////////////////////////////////////////////////////////////////

int icqOptionsInitialise(WPARAM wParam, LPARAM lParam)
{
    OPTIONSDIALOGPAGE odp = { 0 };

    odp.cbSize = sizeof(odp);
    odp.position = -800000000;
    odp.pszTitle = Translate(protoName);
    odp.pfnDlgProc = (DLGPROC)icqOptionsDlgProc;
    odp.pszTemplate = MAKEINTRESOURCE(IDD_OPT_ICQCORP);
    odp.hInstance = hInstance;
	odp.pszGroup = Translate("Network");
    odp.flags = ODPF_BOLDGROUPS;
//    odp.nIDBottomSimpleControl = IDC_STICQGROUP;
	Options_AddPage(wParam, &odp);

    return 0;
}

///////////////////////////////////////////////////////////////////////////////
