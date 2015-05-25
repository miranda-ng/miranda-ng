/*

Minecraft Dynmap plugin for Miranda Instant Messenger
_____________________________________________

Copyright © 2015 Robert Pösel

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

extern HINSTANCE g_hInstance;

static IconItem icons[] = {
	{ "proto", LPGEN("Protocol icon"), IDI_PROTO },
};

static HANDLE hIconLibItem[SIZEOF(icons)];

void InitIcons(void) {
	Icon_Register(g_hInstance, "Protocols/MinecraftDynmap", icons, SIZEOF(icons), "MinecraftDynmap");
}

HANDLE GetIconHandle(const char* name) {
	for (size_t i = 0; i < SIZEOF(icons); i++) {
		if (strcmp(icons[i].szName, name) == 0) {
			return hIconLibItem[i];
		}
	}
	return 0;
}


// Dialogs

static BOOL LoadDBCheckState(MinecraftDynmapProto* ppro, HWND hwnd, int idCtrl, const char* szSetting, BYTE bDef = 0)
{
	BOOL state = db_get_b(NULL, ppro->m_szModuleName, szSetting, bDef);
	CheckDlgButton(hwnd, idCtrl, state ? BST_CHECKED : BST_UNCHECKED);
	return state;
}

static BOOL StoreDBCheckState(MinecraftDynmapProto* ppro, HWND hwnd, int idCtrl, const char* szSetting)
{
	BOOL state = IsDlgButtonChecked(hwnd, idCtrl);
	db_set_b(NULL, ppro->m_szModuleName, szSetting, (BYTE)state);
	return state;
}

static void LoadDBText(MinecraftDynmapProto* ppro, HWND hwnd, int idCtrl, const char* szSetting)
{
	ptrT tstr(db_get_tsa(NULL, ppro->m_szModuleName, szSetting));
	if (tstr)
		SetDlgItemText(hwnd, idCtrl, tstr);
}

static void StoreDBText(MinecraftDynmapProto* ppro, HWND hwnd, int idCtrl, const char* szSetting)
{
	TCHAR tstr[250+1];

	GetDlgItemText(hwnd, idCtrl, tstr, SIZEOF(tstr));
	if (tstr[0] != '\0') {
		db_set_ts(NULL, ppro->m_szModuleName, szSetting, tstr);
	} else {
		db_unset(NULL, ppro->m_szModuleName, szSetting);
	}
}


INT_PTR CALLBACK MinecraftDynmapAccountProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	MinecraftDynmapProto *proto;

	switch (message)
	{

	case WM_INITDIALOG:
		TranslateDialogDefault(hwnd);

		proto = reinterpret_cast<MinecraftDynmapProto*>(lparam);
		SetWindowLongPtr(hwnd,GWLP_USERDATA,lparam);

		LoadDBText(proto, hwnd, IDC_SERVER, MINECRAFTDYNMAP_KEY_SERVER);
		LoadDBText(proto, hwnd, IDC_NAME, MINECRAFTDYNMAP_KEY_NAME);

		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wparam)) {
			case IDC_NAME: {
				if (HIWORD(wparam) != EN_CHANGE || (HWND)lparam != GetFocus()) {
					return TRUE;
				} else {
					SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
				}
				break;
			}
		}
		break;

	case WM_NOTIFY:
		if (reinterpret_cast<NMHDR*>(lparam)->code == PSN_APPLY) {
			proto = reinterpret_cast<MinecraftDynmapProto*>(GetWindowLongPtr(hwnd,GWLP_USERDATA));

			StoreDBText(proto, hwnd, IDC_SERVER, MINECRAFTDYNMAP_KEY_SERVER);
			StoreDBText(proto, hwnd, IDC_NAME, MINECRAFTDYNMAP_KEY_NAME);

			return TRUE;
		}
		break;

	}
	return FALSE;
}
