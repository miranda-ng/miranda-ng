/*
Popup Plus plugin for Miranda IM

Copyright	© 2002 Luca Santarelli,
© 2004-2007 Victor Pavlychko
© 2010 MPK
© 2010 Merlin_de

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

static void sttResetListOptions(HWND hwndList)
{
	SendMessage(hwndList, CLM_SETHIDEEMPTYGROUPS, 1, 0);
}

static void sttSetAllContactIcons(HWND hwndList)
{
	for (auto &hContact : Contacts()) {
		HANDLE hItem = (HANDLE)SendMessage(hwndList, CLM_FINDCONTACT, hContact, 0);
		uint32_t dwMode = g_plugin.getByte(hContact, "ShowMode", 0);
		for (int i = 0; i < 4 /*_countof(sttIcons)*/; ++i)
			// hIml element [0]    = SKINICON_OTHER_SMALLDOT
			// hIml element [1..5] = IcoLib_GetIcon(....)   ~ old sttIcons
			SendMessage(hwndList, CLM_SETEXTRAIMAGE, (WPARAM)hItem, MAKELPARAM(i, ((int)dwMode == i) ? i + 1 : 0));
	}
}

INT_PTR CALLBACK DlgProcContactOpts(HWND hwnd, UINT msg, WPARAM, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwnd);

		SendDlgItemMessage(hwnd, IDC_ICO_AUTO, STM_SETICON, (WPARAM)g_plugin.getIcon(IDI_ACT_OK), 0);
		SendDlgItemMessage(hwnd, IDC_ICO_FAVORITE, STM_SETICON, (WPARAM)g_plugin.getIcon(IDI_OPT_FAVORITE), 0);
		SendDlgItemMessage(hwnd, IDC_ICO_FULLSCREEN, STM_SETICON, (WPARAM)g_plugin.getIcon(IDI_OPT_FULLSCREEN), 0);
		SendDlgItemMessage(hwnd, IDC_ICO_BLOCK, STM_SETICON, (WPARAM)g_plugin.getIcon(IDI_OPT_BLOCK), 0);
		{
			HIMAGELIST hIml = ImageList_Create(GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), ILC_COLOR32 | ILC_MASK, 5, 5);
			ImageList_AddSkinIcon(hIml, SKINICON_OTHER_SMALLDOT);
			g_plugin.addImgListIcon(hIml, IDI_ACT_OK);
			g_plugin.addImgListIcon(hIml, IDI_OPT_FAVORITE);
			g_plugin.addImgListIcon(hIml, IDI_OPT_FULLSCREEN);
			g_plugin.addImgListIcon(hIml, IDI_OPT_BLOCK);
			SendDlgItemMessage(hwnd, IDC_LIST, CLM_SETEXTRAIMAGELIST, 0, (LPARAM)hIml);
			SendDlgItemMessage(hwnd, IDC_LIST, CLM_SETEXTRACOLUMNS, 4 /*_countof(sttIcons)*/, 0);
			sttResetListOptions(GetDlgItem(hwnd, IDC_LIST));
			sttSetAllContactIcons(GetDlgItem(hwnd, IDC_LIST));
		}
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case IDC_LIST:
			switch (((LPNMHDR)lParam)->code) {
			case CLN_NEWCONTACT:
			case CLN_LISTREBUILT:
				sttSetAllContactIcons(GetDlgItem(hwnd, IDC_LIST));
				break;
			case CLN_OPTIONSCHANGED:
				sttResetListOptions(GetDlgItem(hwnd, IDC_LIST));
				break;
			case NM_CLICK:
				NMCLISTCONTROL *nm = (NMCLISTCONTROL*)lParam;
				if (nm->iColumn == -1) break;

				uint32_t hitFlags;
				HANDLE hItem = (HANDLE)SendDlgItemMessage(hwnd, IDC_LIST, CLM_HITTEST, (WPARAM)&hitFlags, MAKELPARAM(nm->pt.x, nm->pt.y));
				if (hItem == nullptr) break;
				if (!(hitFlags&CLCHT_ONITEMEXTRA)) break;

				int iImage = SendDlgItemMessage(hwnd, IDC_LIST, CLM_GETEXTRAIMAGE, (WPARAM)hItem, MAKELPARAM(nm->iColumn, 0));
				if (iImage != EMPTY_EXTRA_ICON) {
					for (int i = 0; i < 4 /*_countof(sttIcons)*/; ++i)
						// hIml element [0]    = SKINICON_OTHER_SMALLDOT
						// hIml element [1..5] = IcoLib_GetIcon(....)   ~ old sttIcons
						SendDlgItemMessage(hwnd, IDC_LIST, CLM_SETEXTRAIMAGE, (WPARAM)hItem, MAKELPARAM(i, (i == nm->iColumn) ? i + 1 : 0));
				}
				SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
			}
			break;

		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_APPLY:
				HWND hwndList = GetDlgItem(hwnd, IDC_LIST);
				for (auto &hContact : Contacts()) {
					HANDLE hItem = (HANDLE)SendMessage(hwndList, CLM_FINDCONTACT, hContact, 0);
					for (int i = 0; i < 4 /*_countof(sttIcons)*/; ++i) {
						if (SendMessage(hwndList, CLM_GETEXTRAIMAGE, (WPARAM)hItem, MAKELPARAM(i, 0))) {
							g_plugin.setByte(hContact, "ShowMode", i);
							break;
						}
					}
				}
				return TRUE;
			}
		}
		break;
	}

	return FALSE;
}
