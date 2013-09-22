/*
Basic History plugin
Copyright (C) 2011-2012 Krzysztof Kral

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

#include "StdAfx.h"
#include "HotkeyHelper.h"
#include "resource.h"
#include "Options.h"

static bool isPresed = false;

static LRESULT CALLBACK HotkeySubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_NCPAINT:
		if (Options::instance->noFindBorder && GetDlgCtrlID(hwnd) == IDC_FIND_TEXT)
			return 0;
		break;
	case WM_CHAR:
	case WM_SYSCHAR:
	case WM_UNICHAR:
	case WM_DEADCHAR:
	case WM_SYSDEADCHAR:
		if (isPresed)
			return 0;
		break;
	case WM_KEYUP:
	case WM_SYSKEYUP:
		if (isPresed)
		{
			isPresed = false;
			return 0;
		}
		break;
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		{
			isPresed = false;
			HWND hwndParent = GetParent(hwnd);
			MSGFILTER filter;
			filter.msg = msg;
			filter.lParam = lParam;
			filter.wParam = wParam;
			filter.nmhdr.hwndFrom = hwnd;
			filter.nmhdr.code = EN_MSGFILTER;
			filter.nmhdr.idFrom = GetDlgCtrlID(hwnd);
			if (SendMessage(hwndParent, WM_NOTIFY, NULL, (LPARAM)&filter))
			{
				isPresed = true;
				return 0;
			}

			LRESULT res = mir_callNextSubclass(hwnd, HotkeySubclassProc, msg, wParam, lParam);
			filter.nmhdr.code = CLN_MYSELCHANGED;
			SendMessage(hwndParent, WM_NOTIFY, NULL, (LPARAM)&filter);
			return res;
		}
		break;
	case WM_LBUTTONDOWN:
		{
			HWND hwndParent = GetParent(hwnd);
			MSGFILTER filter;
			filter.msg = msg;
			filter.lParam = lParam;
			filter.wParam = wParam;
			filter.nmhdr.hwndFrom = hwnd;
			filter.nmhdr.code = CLN_MYSELCHANGED;
			filter.nmhdr.idFrom = GetDlgCtrlID(hwnd);

			LRESULT res = mir_callNextSubclass(hwnd, HotkeySubclassProc, msg, wParam, lParam);
			SendMessage(hwndParent, WM_NOTIFY, NULL, (LPARAM)&filter);
			return res;
		}
		break;
	}
	return mir_callNextSubclass(hwnd, HotkeySubclassProc, msg, wParam, lParam);
}

void RegisterHotkeyControl(HWND control)
{
	mir_subclassWindow(control, HotkeySubclassProc);
}
