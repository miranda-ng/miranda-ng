/*
Miranda plugin template, originally by Richard Hughes
http://miranda-icq.sourceforge.net/

This file is placed in the public domain. Anybody is free to use or
modify it as they wish with no restriction.
There is no warranty.
*/

/*

PLEASE CONTACT ME FIRST IF YOU WANT TO USE PART OF MY CODE IN YOUR OWN WORK :-)

Felipe Brahm - souFrag
ICQ#50566818
http://www.soufrag.cl

You cand send me an e-mail from my webpage :-)

*/

#include "../../AggressiveOptimize.h"

#include <windows.h>
#include <stdio.h>
//#include <afxwin.h>
#include "resource.h"
#include "resrc1.h"

/*
#include "../miranda_src/SDK/Headers_c/newpluginapi.h"
#include "../miranda_src/SDK/Headers_c/m_skin.h"
#include "../miranda_src/SDK/Headers_c/m_langpack.h"
#include "../miranda_src/SDK/Headers_c/m_database.h"
#include "../miranda_src/SDK/Headers_c/m_options.h"
#include "../miranda_src/SDK/Headers_c/m_utils.h"
*/


#include "../../headers_c/newpluginapi.h"
#include "../../headers_c/m_langpack.h"
#include "../../headers_c/m_database.h"
#include "../../headers_c/m_options.h"
//#include "../../headers_c/m_utils.h"

//#include "../../headers_c/m_clist.h"
//#include "../../headers_c/m_skin.h"

#define dbERROR -999999
#define RIGHT 1
#define LEFT 0

HINSTANCE hInst;
PLUGINLINK *pluginLink;

PLUGININFOEX pluginInfo = {

	sizeof(PLUGININFOEX),
	"Window Start Position",
	PLUGIN_MAKE_VERSION(0,2,0,2),
	"Makes the Contact List always start on the right or left of the screen, X pixels away from the taskbar, from the top (or bottom dependig where you have the taskbar), and from the right of the screen. It uses the same clWidth the clist had when you last closed Miranda. You may choose whether to start Miranda minimized to System Tray or not.",
	"Felipe Brahm - souFrag",
	"felipebrahm <at> gmail <dot> com",
	"©2005-2008 Felipe Brahm - souFrag",
	"http://www.felipebrahm.com/",
	0,		//not transient
	0,		//doesn't replace anything built-in

	{ 0x211f6277, 0x6f9b, 0x4b77, { 0xa9, 0x39, 0x84, 0xd0, 0x4b, 0x26, 0xb3, 0x8c } } // {211F6277-6F9B-4b77-A939-84D04B26B38C}

};

BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	hInst=hinstDLL;
	return TRUE;
}

/*static int PluginMenuCommand(WPARAM wParam,LPARAM lParam)
{
	MessageBox(NULL,"Just groovy, baby!","Plugin-o-rama",MB_OK);
	return 0;
}*/

__declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}

#define MIID_WINDOWPOSITION { 0x219d0d15, 0xd8e0, 0x4634, { 0xba, 0xa8, 0x94, 0x6a, 0xf1, 0x8f, 0x13, 0x83 } } // {219D0D15-D8E0-4634-BAA8-946AF18F1383}

static const MUUID interfaces[] = {MIID_WINDOWPOSITION, MIID_LAST};
__declspec(dllexport) const MUUID * MirandaPluginInterfaces(void)
{
	return interfaces;
}

static BOOL CALLBACK OptionsDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_INITDIALOG:
		{
			//DBVARIANT dbv;

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

			//SetDlgItemText(hwndDlg,IDC_TEXTO,Translate("algún texto"));

			//EnableWindow(GetDlgItem(hwndDlg,IDC_AWAYTIME),IsDlgButtonChecked(hwndDlg,IDC_TIMED));

			return TRUE;
		}
		case WM_COMMAND:
			/*switch(LOWORD(wParam)) {
				case IDC_TIMED:
					EnableWindow(GetDlgItem(hwndDlg,IDC_AWAYTIME),IsDlgButtonChecked(hwndDlg,IDC_TIMED));					
					EnableWindow(GetDlgItem(hwndDlg,IDC_STAWAYTYPE),IsDlgButtonChecked(hwndDlg,IDC_TIMED));				
					break;
				case IDC_SETNA:
					EnableWindow(GetDlgItem(hwndDlg,IDC_NATIME),IsDlgButtonChecked(hwndDlg,IDC_SETNA));

			}*/
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
						
						/*
						DBDeleteContactSetting(NULL,"CListStartPosition","pixelsTop");
						DBDeleteContactSetting(NULL,"CListStartPosition","pixelsBottom");
						DBDeleteContactSetting(NULL,"CListStartPosition","pixelsSide");
						DBDeleteContactSetting(NULL,"CListStartPosition","align");
						DBDeleteContactSetting(NULL,"CListStartPosition","useLastWidth");
						DBDeleteContactSetting(NULL,"CListStartPosition","width");
						*/

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

int OptInit(WPARAM wParam,LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp;

	ZeroMemory(&odp,sizeof(odp));
	odp.cbSize=sizeof(odp);
	odp.position=0;
	odp.hInstance=hInst;
	odp.pszTemplate=MAKEINTRESOURCE(IDD_OPTIONS);
	odp.pszGroup= Translate("Contact List");
	odp.pszTitle= Translate("Window Start Position");
	odp.pfnDlgProc=OptionsDlgProc;
	odp.expertOnlyControls=NULL;
	CallService(MS_OPT_ADDPAGE,wParam,(LPARAM)&odp);
	
	return 0;
}

int __declspec(dllexport) Load(PLUGINLINK *link)
{
	//CLISTMENUITEM mi;
	RECT WorkArea;
	/*
	RECT is as this: (i think it's in windows.h)
	typedef struct _RECT { 
	LONG left; 
	LONG top; 
	LONG right; 
	LONG bottom; 
	} RECT, *PRECT;
	*/
	int clWidth=0;
	int clTop, clBottom, clSide; //pixels from top, bottom, side
	int clAlign;
	int clEnableTop, clEnableBottom, clEnableSide, clEnableWidth, clEnableState;
	int clState; //minimizado == 0.
	//int mX,mY,mH,mW;
	//char s[10];

	pluginLink=link;
	/*CreateServiceFunction("TestPlug/MenuCommand",PluginMenuCommand);
	ZeroMemory(&mi,sizeof(mi));
	mi.cbSize=sizeof(mi);
	mi.position=-0x7FFFFFFF;
	mi.flags=0;
	mi.hIcon=LoadSkinnedIcon(SKINICON_OTHER_MIRANDA);
	mi.pszName="&Test Plugin...";
	mi.pszService="TestPlug/MenuCommand";
	CallService(MS_CLIST_ADDMAINMENUITEM,0,(LPARAM)&mi);*/

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

	/*//Begin Mensaje
	mX = DBGetContactSettingDword(NULL, "WindowStartPosition", "MpercenX", dbERROR);//porcentaje mensaje respecto al centro
	mY = DBGetContactSettingDword(NULL, "WindowStartPosition", "MpercenY", dbERROR);//porcentaje mensaje respecto al centro
	mH = DBGetContactSettingDword(NULL, "WindowStartPosition", "MpercenH", dbERROR);//porcentaje alto respecto a workarea
	mW = DBGetContactSettingByte(NULL, "WindowStartPosition", "MpercenW", dbERROR);//porcentaje ancho respecto a workarea
	//End Mensaje*/

	//for debug
	//sprintf(s, "clTop:%i clBottom:%i clSide:%i clAlign:%i clUseLastWidth:%i clWidth:%i", clTop,clBottom,clSide,clAlign,clUseLastWidth,clWidth);
	//MessageBox(NULL, s,"Info",MB_OK);

		/*mX = 50;
		DBWriteContactSettingDword(NULL, "WindowStartPosition", "MpercenX", dbERROR);//porcentaje mensaje respecto al centro
		mY = 50;
		DBWriteContactSettingDword(NULL, "WindowStartPosition", "MpercenY", dbERROR);//porcentaje mensaje respecto al centro
		mH = 30;
		DBWriteContactSettingDword(NULL, "WindowStartPosition", "MpercenH", dbERROR);//porcentaje alto respecto a workarea
		mW = 30;
		DBWriteContactSettingDword(NULL, "WindowStartPosition", "MpercenW", dbERROR);//porcentaje ancho respecto a workarea
		*/

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

	//for debug
	//sprintf(s, "x:%i y:%i top:%i bottom:%i", x, y, WorkArea.top, WorkArea.bottom);
	//sprintf(s, "x:%i y:%i top:%i bottom:%i", x, y, WorkArea.top, WorkArea.bottom);
	//MessageBox(NULL, s,"Info",MB_OK);

	return 0;
}

int __declspec(dllexport) Unload(void)
{
	return 0;
} 