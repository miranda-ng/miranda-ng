/*
Copyright (c) 2005 Victor Pavlychko (nullbyte@sotline.net.ua)
Copyright (C) 2012-24 Miranda NG team (https://miranda-ng.org)

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

INT_PTR CALLBACK CalendarToolDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		// This causes ALL miranda dialogs to have drop-shadow enabled. That's bad =(
		// SetClassLong(hwnd, GCL_STYLE, GetClassLong(hwnd, GCL_STYLE)|CS_DROPSHADOW);
		SetWindowPos(hwnd, HWND_TOP, LOWORD(lParam), HIWORD(lParam), 0, 0, SWP_NOSIZE);
		return TRUE;

	case WM_ACTIVATE:
		if (wParam == WA_INACTIVE)
			PostMessage(hwnd, WM_CLOSE, 0, 0);
		break;

	case WM_NOTIFY:
		{
			LPNMHDR hdr = (LPNMHDR)lParam;
			if ((hdr->idFrom == IDC_MONTHCALENDAR) && (hdr->code == MCN_SELECT)) {
				LPNMSELCHANGE lpnmsc = (LPNMSELCHANGE)lParam;
				struct tm tm_sel;
				tm_sel.tm_hour = tm_sel.tm_min = tm_sel.tm_sec = 0;
				tm_sel.tm_isdst = 1;
				tm_sel.tm_mday = lpnmsc->stSelStart.wDay;
				tm_sel.tm_mon = lpnmsc->stSelStart.wMonth - 1;
				tm_sel.tm_year = lpnmsc->stSelStart.wYear - 1900;
				PostMessage(GetParent(hwnd), WM_USER + 0x600, mktime(&tm_sel), 0);
				EndDialog(hwnd, 0);
			}
		}
		return TRUE;

	case WM_CLOSE:
		DestroyWindow(hwnd);
		return TRUE;
	}
	return FALSE;
}

time_t CalendarTool_Show(HWND hwnd, int x, int y)
{
	HWND hwndCalendar = CreateDialogParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_CALENDARTOOL), hwnd, CalendarToolDlgProc, MAKELONG(x, y));
	ShowWindow(hwndCalendar, SW_SHOW);
	return 0;
}
