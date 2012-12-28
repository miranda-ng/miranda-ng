/*

PLEASE CONTACT ME FIRST IF YOU WANT TO USE PART OF MY CODE IN YOUR OWN WORK :-)

Felipe Brahm - souFrag
ICQ#50566818
http://www.soufrag.cl

You cand send me an e-mail from my webpage :-)

*/

#include <windows.h>
#include <stdio.h>

#include <newpluginapi.h>
#include <m_langpack.h>
#include <m_database.h>
#include <m_options.h>

#include "resource.h"
#include "version.h"

#define dbERROR -999999
#define RIGHT 1
#define LEFT 0

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

			if(DBGetContactSettingByte(NULL,"WindowStartPosition","CLState",2))
				CheckRadioButton(hwndDlg, IDC_CLSTATE0, IDC_CLSTATE2, IDC_CLSTATE2);
			else
				CheckRadioButton(hwndDlg, IDC_CLSTATE0, IDC_CLSTATE2, IDC_CLSTATE0);

			if(DBGetContactSettingByte(NULL,"WindowStartPosition","CLAlign",1))
				CheckRadioButton(hwndDlg, IDC_CLALIGN1, IDC_CLALIGN2, IDC_CLALIGN2);
			else
				CheckRadioButton(hwndDlg, IDC_CLALIGN1, IDC_CLALIGN2, IDC_CLALIGN1);
			
			CheckDlgButton(hwndDlg, IDC_CLSTATEENABLE, DBGetContactSettingByte(NULL,"WindowStartPosition","CLEnableState",0) ? BST_CHECKED : BST_UNCHECKED);
			
			CheckDlgButton(hwndDlg, IDC_CLTOPENABLE, DBGetContactSettingByte(NULL,"WindowStartPosition","CLEnableTop",1) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_CLBOTTOMENABLE, DBGetContactSettingByte(NULL,"WindowStartPosition","CLEnableBottom",1) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_CLSIDEENABLE, DBGetContactSettingByte(NULL,"WindowStartPosition","CLEnableSide",1) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_CLWIDTHENABLE, DBGetContactSettingByte(NULL,"WindowStartPosition","CLEnableWidth",0) ? BST_CHECKED : BST_UNCHECKED);

			SetDlgItemInt(hwndDlg,IDC_CLTOP,DBGetContactSettingDword(NULL,"WindowStartPosition","CLpixelsTop",3),TRUE);
			SetDlgItemInt(hwndDlg,IDC_CLBOTTOM,DBGetContactSettingDword(NULL,"WindowStartPosition","CLpixelsBottom",3),TRUE);
			SetDlgItemInt(hwndDlg,IDC_CLSIDE,DBGetContactSettingDword(NULL,"WindowStartPosition","CLpixelsSide",3),TRUE);
			SetDlgItemInt(hwndDlg,IDC_CLWIDTH,DBGetContactSettingDword(NULL,"WindowStartPosition","CLWidth",180),FALSE);

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

						DBDeleteContactSetting(NULL,"WindowStartPosition","CLEnable");
						DBDeleteContactSetting(NULL,"WindowStartPosition","CLuseLastWidth");
					}
					/*end uninstall old settings*/

					if(IsDlgButtonChecked(hwndDlg, IDC_CLSTATE2))
						DBWriteContactSettingByte(NULL,"WindowStartPosition","CLState",2);
					else
						DBWriteContactSettingByte(NULL,"WindowStartPosition","CLState",0);

					if(IsDlgButtonChecked(hwndDlg, IDC_CLALIGN1))
						DBWriteContactSettingByte(NULL,"WindowStartPosition","CLAlign",0);
					else
						DBWriteContactSettingByte(NULL,"WindowStartPosition","CLAlign",1);

					DBWriteContactSettingByte(NULL,"WindowStartPosition","CLEnableState",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_CLSTATEENABLE));

					DBWriteContactSettingByte(NULL,"WindowStartPosition","CLEnableTop",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_CLTOPENABLE));
					DBWriteContactSettingByte(NULL,"WindowStartPosition","CLEnableBottom",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_CLBOTTOMENABLE));
					DBWriteContactSettingByte(NULL,"WindowStartPosition","CLEnableSide",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_CLSIDEENABLE));
					DBWriteContactSettingByte(NULL,"WindowStartPosition","CLEnableWidth",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_CLWIDTHENABLE));
					DBWriteContactSettingDword(NULL,"WindowStartPosition","CLWidth",GetDlgItemInt(hwndDlg,IDC_CLWIDTH,NULL,FALSE));
					DBWriteContactSettingDword(NULL,"WindowStartPosition","CLpixelsTop",GetDlgItemInt(hwndDlg,IDC_CLTOP,NULL,TRUE));
					DBWriteContactSettingDword(NULL,"WindowStartPosition","CLpixelsBottom",GetDlgItemInt(hwndDlg,IDC_CLBOTTOM,NULL,TRUE));
					DBWriteContactSettingDword(NULL,"WindowStartPosition","CLpixelsSide",GetDlgItemInt(hwndDlg,IDC_CLSIDE,NULL,TRUE));

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

extern "C" __declspec(dllexport) int Load(void)
{
	mir_getLP(&pluginInfo);

	RECT WorkArea;

	int clWidth=0;
	int clTop, clBottom, clSide; //pixels from top, bottom, side
	int clAlign;
	int clEnableTop, clEnableBottom, clEnableSide, clEnableWidth, clEnableState;
	int clState;

	HookEvent(ME_OPT_INITIALISE,OptInit);

	SystemParametersInfo(SPI_GETWORKAREA, 0, &WorkArea, 0);

	clEnableTop = DBGetContactSettingByte(NULL, "WindowStartPosition", "CLEnableTop", 1);
	clEnableBottom = DBGetContactSettingByte(NULL, "WindowStartPosition", "CLEnableBottom", 1);
	clEnableSide = DBGetContactSettingByte(NULL, "WindowStartPosition", "CLEnableSide", 1);
	clEnableWidth = DBGetContactSettingByte(NULL, "WindowStartPosition", "CLEnableWidth", 0);

	clTop = DBGetContactSettingDword(NULL, "WindowStartPosition", "CLpixelsTop", 3);
	clBottom = DBGetContactSettingDword(NULL, "WindowStartPosition", "CLpixelsBottom", 3);
	clSide = DBGetContactSettingDword(NULL, "WindowStartPosition", "CLpixelsSide", 3);
	clAlign = DBGetContactSettingByte(NULL, "WindowStartPosition", "CLAlign", RIGHT);
	clWidth = DBGetContactSettingDword(NULL, "WindowStartPosition", "CLWidth", 180);

	clEnableState = DBGetContactSettingByte(NULL, "WindowStartPosition", "CLEnableState", 0);
	clState = DBGetContactSettingByte(NULL, "WindowStartPosition", "CLState", 2);

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

extern "C" __declspec(dllexport) int Unload(void)
{
	return 0;
}