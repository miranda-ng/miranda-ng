/*

StartPosition plugin for Miranda NG

Copyright (C) 2005-2008 Felipe Brahm - souFrag
ICQ#50566818
http://www.soufrag.cl

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "Common.h"

HINSTANCE hInst;
int hLangpack;

PLUGININFOEX pluginInfo = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {211F6277-6F9B-4b77-A939-84D04B26B38C}
	{0x211f6277, 0x6f9b, 0x4b77, {0xa9, 0x39, 0x84, 0xd0, 0x4b, 0x26, 0xb3, 0x8c}}
};

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	hInst = hinstDLL;
	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}

static INT_PTR CALLBACK OptionsDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_INITDIALOG:
		{
			TranslateDialogDefault(hwndDlg);

			if(DBGetContactSettingByte(NULL, "WindowStartPosition", "CLState", 2))
				CheckRadioButton(hwndDlg, IDC_CLSTATE0, IDC_CLSTATE2, IDC_CLSTATE2);
			else
				CheckRadioButton(hwndDlg, IDC_CLSTATE0, IDC_CLSTATE2, IDC_CLSTATE0);

			if(DBGetContactSettingByte(NULL, "WindowStartPosition", "CLAlign", 1))
				CheckRadioButton(hwndDlg, IDC_CLALIGN1, IDC_CLALIGN2, IDC_CLALIGN2);
			else
				CheckRadioButton(hwndDlg, IDC_CLALIGN1, IDC_CLALIGN2, IDC_CLALIGN1);
			
			CheckDlgButton(hwndDlg, IDC_CLSTATEENABLE, DBGetContactSettingByte(NULL, "WindowStartPosition", "CLEnableState", 0) ? BST_CHECKED : BST_UNCHECKED);
			
			CheckDlgButton(hwndDlg, IDC_CLTOPENABLE, DBGetContactSettingByte(NULL, "WindowStartPosition", "CLEnableTop", 1) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_CLBOTTOMENABLE, DBGetContactSettingByte(NULL, "WindowStartPosition", "CLEnableBottom", 1) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_CLSIDEENABLE, DBGetContactSettingByte(NULL, "WindowStartPosition", "CLEnableSide", 1) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_CLWIDTHENABLE, DBGetContactSettingByte(NULL, "WindowStartPosition", "CLEnableWidth", 0) ? BST_CHECKED : BST_UNCHECKED);

			SetDlgItemInt(hwndDlg, IDC_CLTOP, DBGetContactSettingDword(NULL, "WindowStartPosition", "CLpixelsTop", 3), TRUE);
			SetDlgItemInt(hwndDlg, IDC_CLBOTTOM, DBGetContactSettingDword(NULL, "WindowStartPosition", "CLpixelsBottom", 3), TRUE);
			SetDlgItemInt(hwndDlg, IDC_CLSIDE, DBGetContactSettingDword(NULL, "WindowStartPosition", "CLpixelsSide", 3), TRUE);
			SetDlgItemInt(hwndDlg, IDC_CLWIDTH, DBGetContactSettingDword(NULL, "WindowStartPosition", "CLWidth", 180), FALSE);

			return TRUE;
		}
		case WM_COMMAND:
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;
		case WM_NOTIFY:
			switch (((LPNMHDR)lParam)->code)
			{
				case PSN_APPLY:
				{
					/*begin uninstall old settings*/
					if(DBGetContactSettingByte(NULL, "WindowStartPosition", "CLEnableTop", dbERROR) == dbERROR) {
						DBDeleteContactSetting(NULL, "WindowStartPosition", "CLEnable");
						DBDeleteContactSetting(NULL, "WindowStartPosition", "CLuseLastWidth");
					}
					/*end uninstall old settings*/

					if(IsDlgButtonChecked(hwndDlg, IDC_CLSTATE2))
						DBWriteContactSettingByte(NULL, "WindowStartPosition", "CLState", 2);
					else
						DBWriteContactSettingByte(NULL, "WindowStartPosition", "CLState", 0);

					if(IsDlgButtonChecked(hwndDlg, IDC_CLALIGN1))
						DBWriteContactSettingByte(NULL, "WindowStartPosition", "CLAlign", 0);
					else
						DBWriteContactSettingByte(NULL, "WindowStartPosition", "CLAlign", 1);

					DBWriteContactSettingByte(NULL, "WindowStartPosition", "CLEnableState", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_CLSTATEENABLE));

					DBWriteContactSettingByte(NULL, "WindowStartPosition", "CLEnableTop", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_CLTOPENABLE));
					DBWriteContactSettingByte(NULL, "WindowStartPosition", "CLEnableBottom", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_CLBOTTOMENABLE));
					DBWriteContactSettingByte(NULL, "WindowStartPosition", "CLEnableSide", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_CLSIDEENABLE));
					DBWriteContactSettingByte(NULL, "WindowStartPosition", "CLEnableWidth", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_CLWIDTHENABLE));
					DBWriteContactSettingDword(NULL, "WindowStartPosition", "CLWidth", GetDlgItemInt(hwndDlg, IDC_CLWIDTH, NULL, FALSE));
					DBWriteContactSettingDword(NULL, "WindowStartPosition", "CLpixelsTop", GetDlgItemInt(hwndDlg, IDC_CLTOP, NULL, TRUE));
					DBWriteContactSettingDword(NULL, "WindowStartPosition", "CLpixelsBottom", GetDlgItemInt(hwndDlg, IDC_CLBOTTOM, NULL, TRUE));
					DBWriteContactSettingDword(NULL, "WindowStartPosition", "CLpixelsSide", GetDlgItemInt(hwndDlg, IDC_CLSIDE, NULL, TRUE));

					return TRUE;
				}
			}
			break;
	}
	return FALSE;
}

int OptInit(WPARAM wParam, LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = {0};
	odp.cbSize = sizeof(odp);
	odp.position = 0;
	odp.hInstance = hInst;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS);
	odp.pszGroup = LPGEN("Contact List");
	odp.pszTitle = LPGEN("Window Start Position");
	odp.pfnDlgProc = OptionsDlgProc;
	odp.flags = ODPF_BOLDGROUPS;
	Options_AddPage(wParam, &odp);
	
	return 0;
}

int onModulesLoaded(WPARAM wParam, LPARAM lParam)
{
	RECT WorkArea;

	SystemParametersInfo(SPI_GETWORKAREA, 0, &WorkArea, 0);

	int clEnableTop = DBGetContactSettingByte(NULL, "WindowStartPosition", "CLEnableTop", 1);
	int clEnableBottom = DBGetContactSettingByte(NULL, "WindowStartPosition", "CLEnableBottom", 1);
	int clEnableSide = DBGetContactSettingByte(NULL, "WindowStartPosition", "CLEnableSide", 1);
	int clEnableWidth = DBGetContactSettingByte(NULL, "WindowStartPosition", "CLEnableWidth", 0);

	int clTop = DBGetContactSettingDword(NULL, "WindowStartPosition", "CLpixelsTop", 3);
	int clBottom = DBGetContactSettingDword(NULL, "WindowStartPosition", "CLpixelsBottom", 3);
	int clSide = DBGetContactSettingDword(NULL, "WindowStartPosition", "CLpixelsSide", 3);
	int clAlign = DBGetContactSettingByte(NULL, "WindowStartPosition", "CLAlign", RIGHT);
	int clWidth = DBGetContactSettingDword(NULL, "WindowStartPosition", "CLWidth", 180);

	int clEnableState = DBGetContactSettingByte(NULL, "WindowStartPosition", "CLEnableState", 0);
	int clState = DBGetContactSettingByte(NULL, "WindowStartPosition", "CLState", 2);

	if(clEnableState)
		DBWriteContactSettingByte(NULL,"CList", "State", (BYTE)clState);

	if(clEnableWidth) {
		if(clWidth > 0)
			DBWriteContactSettingDword(NULL, "CList", "Width", clWidth);
	} else {
		clWidth = DBGetContactSettingDword(NULL, "CList", "Width", 180);
	}

	if(clEnableTop || clEnableBottom || clEnableSide)
		DBWriteContactSettingByte(NULL,"CList", "Docked", 0);
	
	if(clEnableTop)
		DBWriteContactSettingDword(NULL, "CList", "y", clTop);

	//thx ValeraVi
	if(clEnableBottom) {
		if(clEnableTop)
			DBWriteContactSettingDword(NULL, "CList", "Height", (WorkArea.bottom - WorkArea.top - clTop - clBottom));
		else
			DBWriteContactSettingDword(NULL, "CList", "y", (WorkArea.bottom - clBottom - (int)DBGetContactSettingDword(NULL, "CList", "Height", 0)));
	}

	if(clEnableSide) {
		if(clAlign == LEFT)
			DBWriteContactSettingDword(NULL, "CList", "x", (WorkArea.left + clSide));
		else
			DBWriteContactSettingDword(NULL, "CList", "x", (WorkArea.right - clWidth - clSide));
	}

	return 0;
}

extern "C" __declspec(dllexport) int Load(void)
{
	mir_getLP(&pluginInfo);

	HookEvent(ME_OPT_INITIALISE, OptInit);
	HookEvent(ME_SYSTEM_MODULESLOADED, onModulesLoaded);

	return 0;
}

extern "C" __declspec(dllexport) int Unload(void)
{
	return 0;
}