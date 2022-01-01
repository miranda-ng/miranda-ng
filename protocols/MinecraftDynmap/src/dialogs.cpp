/*

Minecraft Dynmap plugin for Miranda Instant Messenger
_____________________________________________

Copyright © 2015-17 Robert Pösel, 2017-22 Miranda NG team

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

#include "stdafx.h"

// Icons

static IconItem iconList[] =
{
	{ "proto", LPGEN("Protocol icon"), IDI_PROTO },
};

static HANDLE hIconLibItem[_countof(iconList)];

void InitIcons(void)
{
	g_plugin.registerIcon("Protocols/MinecraftDynmap", iconList, "MinecraftDynmap");
}

// Dialogs

static void LoadDBText(MinecraftDynmapProto* ppro, HWND hwnd, int idCtrl, const char* szSetting)
{
	ptrW tstr(db_get_wsa(0, ppro->m_szModuleName, szSetting));
	if (tstr)
		SetDlgItemText(hwnd, idCtrl, tstr);
}

static void StoreDBText(MinecraftDynmapProto* ppro, HWND hwnd, int idCtrl, const char* szSetting)
{
	wchar_t tstr[250 + 1];

	GetDlgItemText(hwnd, idCtrl, tstr, _countof(tstr));
	if (tstr[0] != '\0')
		db_set_ws(0, ppro->m_szModuleName, szSetting, tstr);
	else
		db_unset(0, ppro->m_szModuleName, szSetting);
}


INT_PTR CALLBACK MinecraftDynmapAccountProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	MinecraftDynmapProto *proto;

	switch (message) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwnd);

		proto = reinterpret_cast<MinecraftDynmapProto*>(lparam);
		SetWindowLongPtr(hwnd, GWLP_USERDATA, lparam);

		LoadDBText(proto, hwnd, IDC_SERVER, MINECRAFTDYNMAP_KEY_SERVER);
		LoadDBText(proto, hwnd, IDC_NAME, MINECRAFTDYNMAP_KEY_NAME);
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wparam)) {
		case IDC_NAME:
			if (HIWORD(wparam) != EN_CHANGE || (HWND)lparam != GetFocus())
				return TRUE;

			SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
			break;
		}
		break;

	case WM_NOTIFY:
		if (reinterpret_cast<NMHDR*>(lparam)->code == PSN_APPLY) {
			proto = reinterpret_cast<MinecraftDynmapProto*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

			StoreDBText(proto, hwnd, IDC_SERVER, MINECRAFTDYNMAP_KEY_SERVER);
			StoreDBText(proto, hwnd, IDC_NAME, MINECRAFTDYNMAP_KEY_NAME);

			return TRUE;
		}
		break;
	}
	return FALSE;
}
