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
#include "m_clui.h"
#include "m_clist.h"

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

			if(db_get_b(NULL, MODULE_NAME, "CLState", 2))
				CheckRadioButton(hwndDlg, IDC_CLSTATE0, IDC_CLSTATE2, IDC_CLSTATE2);
			else
				CheckRadioButton(hwndDlg, IDC_CLSTATE0, IDC_CLSTATE2, IDC_CLSTATE0);

			if(db_get_b(NULL, MODULE_NAME, "CLAlign", 1))
				CheckRadioButton(hwndDlg, IDC_CLALIGN1, IDC_CLALIGN2, IDC_CLALIGN2);
			else
				CheckRadioButton(hwndDlg, IDC_CLALIGN1, IDC_CLALIGN2, IDC_CLALIGN1);
			
			CheckDlgButton(hwndDlg, IDC_CLSTATEENABLE, DBGetContactSettingByte(NULL, MODULE_NAME, "CLEnableState", 0) ? BST_CHECKED : BST_UNCHECKED);
			
			CheckDlgButton(hwndDlg, IDC_CLTOPENABLE, db_get_b(NULL, MODULE_NAME, "CLEnableTop", 1) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_CLBOTTOMENABLE, db_get_b(NULL, MODULE_NAME, "CLEnableBottom", 0) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_CLSIDEENABLE, db_get_b(NULL, MODULE_NAME, "CLEnableSide", 1) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_CLWIDTHENABLE, db_get_b(NULL, MODULE_NAME, "CLEnableWidth", 0) ? BST_CHECKED : BST_UNCHECKED);

			SetDlgItemInt(hwndDlg, IDC_CLTOP, db_get_dw(NULL, MODULE_NAME, "CLpixelsTop", 3), TRUE);
			SetDlgItemInt(hwndDlg, IDC_CLBOTTOM, db_get_dw(NULL, MODULE_NAME, "CLpixelsBottom", 3), TRUE);
			SetDlgItemInt(hwndDlg, IDC_CLSIDE, db_get_dw(NULL, MODULE_NAME, "CLpixelsSide", 3), TRUE);
			SetDlgItemInt(hwndDlg, IDC_CLWIDTH, db_get_dw(NULL, MODULE_NAME, "CLWidth", 180), FALSE);

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
					if(db_get_b(NULL, MODULE_NAME, "CLEnableTop", dbERROR) == dbERROR) {
						db_unset(NULL, MODULE_NAME, "CLEnable");
						db_unset(NULL, MODULE_NAME, "CLuseLastWidth");
					}
					/*end uninstall old settings*/

					if(IsDlgButtonChecked(hwndDlg, IDC_CLSTATE2))
						db_set_b(NULL, MODULE_NAME, "CLState", 2);
					else
						db_set_b(NULL, MODULE_NAME, "CLState", 0);

					if(IsDlgButtonChecked(hwndDlg, IDC_CLALIGN1))
						db_set_b(NULL, MODULE_NAME, "CLAlign", 0);
					else
						db_set_b(NULL, MODULE_NAME, "CLAlign", 1);

					db_set_b(NULL, MODULE_NAME, "CLEnableState", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_CLSTATEENABLE));

					db_set_b(NULL, MODULE_NAME, "CLEnableTop", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_CLTOPENABLE));
					db_set_b(NULL, MODULE_NAME, "CLEnableBottom", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_CLBOTTOMENABLE));
					db_set_b(NULL, MODULE_NAME, "CLEnableSide", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_CLSIDEENABLE));
					db_set_b(NULL, MODULE_NAME, "CLEnableWidth", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_CLWIDTHENABLE));
					db_set_dw(NULL, MODULE_NAME, "CLWidth", GetDlgItemInt(hwndDlg, IDC_CLWIDTH, NULL, FALSE));
					db_set_dw(NULL, MODULE_NAME, "CLpixelsTop", GetDlgItemInt(hwndDlg, IDC_CLTOP, NULL, TRUE));
					db_set_dw(NULL, MODULE_NAME, "CLpixelsBottom", GetDlgItemInt(hwndDlg, IDC_CLBOTTOM, NULL, TRUE));
					db_set_dw(NULL, MODULE_NAME, "CLpixelsSide", GetDlgItemInt(hwndDlg, IDC_CLSIDE, NULL, TRUE));

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
	odp.pszTitle = LPGEN("Start Position");
	odp.pfnDlgProc = OptionsDlgProc;
	odp.flags = ODPF_BOLDGROUPS;
	Options_AddPage(wParam, &odp);
	
	return 0;
}

int onModulesLoaded(WPARAM wParam, LPARAM lParam)
{
	RECT WorkArea;

	SystemParametersInfo(SPI_GETWORKAREA, 0, &WorkArea, 0);

	BYTE clEnableTop = db_get_b(NULL, MODULE_NAME, "CLEnableTop", 1);
	BYTE clEnableBottom = db_get_b(NULL, MODULE_NAME, "CLEnableBottom", 1);
	BYTE clEnableSide = db_get_b(NULL, MODULE_NAME, "CLEnableSide", 1);
	BYTE clEnableWidth = db_get_b(NULL, MODULE_NAME, "CLEnableWidth", 0);

	DWORD clTop = db_get_dw(NULL, MODULE_NAME, "CLpixelsTop", 3);
	DWORD clBottom = db_get_dw(NULL, MODULE_NAME, "CLpixelsBottom", 3);
	DWORD clSide = db_get_dw(NULL, MODULE_NAME, "CLpixelsSide", 3);
	BYTE clAlign = db_get_b(NULL, MODULE_NAME, "CLAlign", RIGHT);
	DWORD clWidth = db_get_dw(NULL, MODULE_NAME, "CLWidth", 180);

	BYTE clEnableState = db_get_b(NULL, MODULE_NAME, "CLEnableState", 0);
	BYTE clState = db_get_b(NULL, MODULE_NAME, "CLState", 2);

	if(clEnableState == 1)
		db_set_b(NULL,"CList", "State", clState);

	if(clEnableWidth == 1 && clWidth > 0)
		db_set_dw(NULL, "CList", "Width", clWidth);
	else
		clWidth = db_get_dw(NULL, "CList", "Width", 180);

	if(clEnableTop == 1 || clEnableBottom == 1 || clEnableSide == 1)
		db_set_b(NULL,"CList", "Docked", 0);
		db_set_b(NULL,"CLUI", "AutoSize", 0);
		db_set_b(NULL,"CLUI", "DockToSides", 0);

	if(clEnableTop == 1)
		db_set_dw(NULL, "CList", "y", clTop);

	//thx ValeraVi
	if(clEnableBottom == 1) {
		if(clEnableTop == 1)
			db_set_dw(NULL, "CList", "Height", (WorkArea.bottom - WorkArea.top - (LONG)clTop - (LONG)clBottom));
		else
			db_set_dw(NULL, "CList", "y", (WorkArea.bottom - (LONG)clBottom - (LONG)db_get_dw(NULL, "CList", "Height", 0)));
	}

	if(clEnableSide == 1 ) {
		if(clAlign == LEFT)
			db_set_dw(NULL, "CList", "x", (WorkArea.left + clSide));
		else
			db_set_dw(NULL, "CList", "x", (WorkArea.right - clWidth - clSide));
	}
	HWND hClist = (HWND)CallService(MS_CLUI_GETHWND, 0, 0);
	MoveWindow(hClist, (int)db_get_dw(NULL, "CList", "x", 100), (int)db_get_dw(NULL, "CList", "y", 100), (int)clWidth, (int)db_get_dw(NULL, "CList", "Height", 0), 0);
	if(clEnableState == 1 && clState == 0)
		CallService(MS_CLIST_SHOWHIDE, 0, 0);
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