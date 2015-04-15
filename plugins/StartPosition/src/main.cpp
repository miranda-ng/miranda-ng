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
	// {211F6277-6F9B-4B77-A939-84D04B26B38C}
	{0x211f6277, 0x6f9b, 0x4b77, {0xa9, 0x39, 0x84, 0xd0, 0x4b, 0x26, 0xb3, 0x8c}}
};

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	hInst = hinstDLL;
	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD)
{
	return &pluginInfo;
}

INT_PTR CALLBACK OptionsDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_INITDIALOG:
		{
			TranslateDialogDefault(hwndDlg);

			if(db_get_b(NULL, MODULE_NAME, "CLState", 2))
				CheckRadioButton(hwndDlg, IDC_CLSTATETRAY, IDC_CLSTATEOPENED, IDC_CLSTATEOPENED);
			else
				CheckRadioButton(hwndDlg, IDC_CLSTATETRAY, IDC_CLSTATEOPENED, IDC_CLSTATETRAY);

			CheckDlgButton(hwndDlg, IDC_CLSTATEENABLE, db_get_b(NULL, MODULE_NAME, "CLEnableState", 0) ? BST_CHECKED : BST_UNCHECKED);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CLSTATETRAY), IsDlgButtonChecked(hwndDlg, IDC_CLSTATEENABLE));
			EnableWindow(GetDlgItem(hwndDlg, IDC_CLSTATEOPENED), IsDlgButtonChecked(hwndDlg, IDC_CLSTATEENABLE));

			if(db_get_b(NULL, MODULE_NAME, "CLAlign", RIGHT))
				CheckRadioButton(hwndDlg, IDC_CLALIGNLEFT, IDC_CLALIGNRIGHT, IDC_CLALIGNRIGHT);
			else
				CheckRadioButton(hwndDlg, IDC_CLALIGNLEFT, IDC_CLALIGNRIGHT, IDC_CLALIGNLEFT);
			CheckDlgButton(hwndDlg, IDC_CLSIDEENABLE, db_get_b(NULL, MODULE_NAME, "CLEnableSide", 1) ? BST_CHECKED : BST_UNCHECKED);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CLALIGNLEFT), IsDlgButtonChecked(hwndDlg, IDC_CLSIDEENABLE));
			EnableWindow(GetDlgItem(hwndDlg, IDC_CLALIGNRIGHT), IsDlgButtonChecked(hwndDlg, IDC_CLSIDEENABLE));

			CheckDlgButton(hwndDlg, IDC_CLTOPENABLE, db_get_b(NULL, MODULE_NAME, "CLEnableTop", 1) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_CLBOTTOMENABLE, db_get_b(NULL, MODULE_NAME, "CLEnableBottom", 0) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_CLWIDTHENABLE, db_get_b(NULL, MODULE_NAME, "CLEnableWidth", 0) ? BST_CHECKED : BST_UNCHECKED);

			SetDlgItemInt(hwndDlg, IDC_CLTOP, db_get_dw(NULL, MODULE_NAME, "CLpixelsTop", 3), TRUE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CLTOP), IsDlgButtonChecked(hwndDlg, IDC_CLTOPENABLE));
			SetDlgItemInt(hwndDlg, IDC_CLBOTTOM, db_get_dw(NULL, MODULE_NAME, "CLpixelsBottom", 3), TRUE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CLBOTTOM), IsDlgButtonChecked(hwndDlg, IDC_CLBOTTOMENABLE));
			SetDlgItemInt(hwndDlg, IDC_CLSIDE, db_get_dw(NULL, MODULE_NAME, "CLpixelsSide", 3), TRUE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CLSIDE), IsDlgButtonChecked(hwndDlg, IDC_CLSIDEENABLE));
			SetDlgItemInt(hwndDlg, IDC_CLWIDTH, db_get_dw(NULL, MODULE_NAME, "CLWidth", 180), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CLWIDTH), IsDlgButtonChecked(hwndDlg, IDC_CLWIDTHENABLE));

			return TRUE;
		}
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_CLALIGNLEFT:
				case IDC_CLALIGNRIGHT:
				case IDC_CLSTATETRAY:
				case IDC_CLSTATEOPENED:
					SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
					break;

				case IDC_CLWIDTHENABLE:
				{
					EnableWindow(GetDlgItem(hwndDlg, IDC_CLWIDTH), IsDlgButtonChecked(hwndDlg, IDC_CLWIDTHENABLE));
					SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				}
				break;

				case IDC_CLBOTTOMENABLE:
				{
					EnableWindow(GetDlgItem(hwndDlg, IDC_CLBOTTOM), IsDlgButtonChecked(hwndDlg, IDC_CLBOTTOMENABLE));
					SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				}
				break;

				case IDC_CLTOPENABLE:
				{
					EnableWindow(GetDlgItem(hwndDlg, IDC_CLTOP), IsDlgButtonChecked(hwndDlg, IDC_CLTOPENABLE));
					SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				}
				break;

				case IDC_CLSIDEENABLE:
				{
					BOOL value = IsDlgButtonChecked(hwndDlg, IDC_CLSIDEENABLE);
					EnableWindow(GetDlgItem(hwndDlg, IDC_CLSIDE), value);
					EnableWindow(GetDlgItem(hwndDlg, IDC_CLALIGNLEFT), value);
					EnableWindow(GetDlgItem(hwndDlg, IDC_CLALIGNRIGHT), value);
					SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				}
				break;

				case IDC_CLSTATEENABLE:
				{
					BOOL value = IsDlgButtonChecked(hwndDlg, IDC_CLSTATEENABLE);
					EnableWindow(GetDlgItem(hwndDlg, IDC_CLSTATETRAY), value);
					EnableWindow(GetDlgItem(hwndDlg, IDC_CLSTATEOPENED), value);
					SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				}
				break;

				case IDC_CLTOP:
				case IDC_CLBOTTOM:
				case IDC_CLSIDE:
				case IDC_CLWIDTH:
					if (HIWORD(wParam) != EN_CHANGE || (HWND) lParam != GetFocus())
						return FALSE;
					SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
					break;
			}
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

					if(IsDlgButtonChecked(hwndDlg, IDC_CLSTATEOPENED))
						db_set_b(NULL, MODULE_NAME, "CLState", 2);
					else
						db_set_b(NULL, MODULE_NAME, "CLState", 0);

					if(IsDlgButtonChecked(hwndDlg, IDC_CLALIGNLEFT))
						db_set_b(NULL, MODULE_NAME, "CLAlign", LEFT);
					else
						db_set_b(NULL, MODULE_NAME, "CLAlign", RIGHT);

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

int OptInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = { 0 };
	odp.hInstance = hInst;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS);
	odp.pszGroup = LPGEN("Contact list");
	odp.pszTitle = LPGEN("Start position");
	odp.pfnDlgProc = OptionsDlgProc;
	odp.flags = ODPF_BOLDGROUPS;
	Options_AddPage(wParam, &odp);
	return 0;
}

extern "C" __declspec(dllexport) int Load(void)
{
	mir_getLP(&pluginInfo);

	RECT WorkArea;

	SystemParametersInfo(SPI_GETWORKAREA, 0, &WorkArea, 0);

	BYTE clEnableTop = db_get_b(NULL, MODULE_NAME, "CLEnableTop", 1);
	BYTE clEnableBottom = db_get_b(NULL, MODULE_NAME, "CLEnableBottom", 0);
	BYTE clEnableSide = db_get_b(NULL, MODULE_NAME, "CLEnableSide", 1);
	BYTE clEnableWidth = db_get_b(NULL, MODULE_NAME, "CLEnableWidth", 0);

	DWORD clTop = db_get_dw(NULL, MODULE_NAME, "CLpixelsTop", 3);
	DWORD clBottom = db_get_dw(NULL, MODULE_NAME, "CLpixelsBottom", 3);
	DWORD clSide = db_get_dw(NULL, MODULE_NAME, "CLpixelsSide", 3);
	BYTE clAlign = db_get_b(NULL, MODULE_NAME, "CLAlign", RIGHT);
	DWORD clWidth = db_get_dw(NULL, MODULE_NAME, "CLWidth", 180);

	BYTE clEnableState = db_get_b(NULL, MODULE_NAME, "CLEnableState", 0);
	BYTE clState = db_get_b(NULL, MODULE_NAME, "CLState", 2);

	if(clEnableState)
		db_set_b(NULL,"CList", "State", (BYTE)clState);

	if(clEnableWidth) {
		if(clWidth > 0)
			db_set_dw(NULL, "CList", "Width", clWidth);
	} else {
		clWidth = db_get_dw(NULL, "CList", "Width", 180);
	}

	if(clEnableTop || clEnableBottom || clEnableSide)
		db_set_b(NULL,"CList", "Docked", 0);

	if(clEnableTop)
		db_set_dw(NULL, "CList", "y", clTop);

	//thx ValeraVi
	if(clEnableBottom) {
		if(clEnableTop)
			db_set_dw(NULL, "CList", "Height", (WorkArea.bottom - WorkArea.top - clTop - clBottom));
		else
			db_set_dw(NULL, "CList", "y", (WorkArea.bottom - clBottom - (int)db_get_dw(NULL, "CList", "Height", 0)));
	}

	if(clEnableSide) {
		if(clAlign == LEFT)
			db_set_dw(NULL, "CList", "x", (WorkArea.left + clSide));
		else
			db_set_dw(NULL, "CList", "x", (WorkArea.right - clWidth - clSide));
	}

	HookEvent(ME_OPT_INITIALISE, OptInit);

	return 0;
}

extern "C" __declspec(dllexport) int Unload(void)
{
	return 0;
}
