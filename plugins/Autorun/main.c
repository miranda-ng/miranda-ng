#include <windows.h>
#include "autorun.h"

#include "newpluginapi.h"
#include "m_langpack.h"
#include "m_options.h"
#include "m_database.h"
#pragma hdrstop


HINSTANCE hInst;
PLUGINLINK *pluginLink;
HANDLE hHookOptionInit = NULL;

PLUGININFOEX pluginInfoEx=
{ 	// about plugin
	sizeof(PLUGININFOEX),
	"Autorun",
	PLUGIN_MAKE_VERSION(0,1,0,1),
	"This plugin is a simple way to enable/disable to launch Miranda IM with system startup.",
	"Sergey V. Gershovich a.k.a. Jazzy$ (fixed by Wolfram3D)",
	"",
	"Copyright © 2002-2007 Sergey V. Gershovich a.k.a. Jazzy$",
	"http://miranda-im.org/download/index.php?action=viewfile&id=285",
	0, 0,
	MIID_AUTORUN
};

BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	// plugin entry point
	if (fdwReason == DLL_PROCESS_ATTACH)
	{
 		hInst = hinstDLL;
		DisableThreadLibraryCalls(hinstDLL);
	}
	return TRUE;
}
void GetProfilePath(char *res)
{
  	char dbname[MAX_PATH], dbpath[MAX_PATH], exename[MAX_PATH];
	CallService(MS_DB_GETPROFILENAME,sizeof(dbname),(LPARAM)(char*) dbname);
	CallService(MS_DB_GETPROFILEPATH,sizeof(dbpath),(LPARAM)(char*) dbpath);
	GetModuleFileName(NULL,exename,sizeof(exename));
	lstrcat(dbpath,"\\");		
	lstrcpyn(dbpath+lstrlen(dbpath),dbname,lstrlen(dbname)-3);
	strcat(dbpath, "\\");
	strcat(dbpath, dbname);		// path + profile name
	wsprintf(res,"\"%s\" \"%s\"",exename, dbpath);
}

static void SetAutorun(BOOL autorun)
{
  HKEY hKey;
  DWORD dw;
  switch (autorun)
  {
  case TRUE:
	if (RegCreateKeyEx(ROOT_KEY, SUB_KEY,0,NULL,0,KEY_CREATE_SUB_KEY|KEY_SET_VALUE,NULL,&hKey,&dw) == ERROR_SUCCESS)
	{
	  	char result[MAX_PATH];
		GetProfilePath (result);
		RegSetValueEx(hKey,"MirandaIM",0,REG_SZ,result,lstrlen(result)+1);
		RegCloseKey(hKey);
		break;
	}
  case FALSE:
	if (RegOpenKey(ROOT_KEY,SUB_KEY,&hKey) == ERROR_SUCCESS)
	{
		RegDeleteValue(hKey,"MirandaIM");
		RegCloseKey(hKey);
		break;
	}
  }
}


static BOOL CmpCurrentAndRegistry()
{
	HKEY hKey;
	DWORD dwBufLen = MAX_PATH;
	char result[MAX_PATH], dbpath[MAX_PATH];
	GetProfilePath (result);
	
	if (RegOpenKeyEx(ROOT_KEY, SUB_KEY, 0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
	{
		if (RegQueryValueEx(hKey,"MirandaIM",NULL,NULL,(LPBYTE)dbpath,&dwBufLen) == ERROR_SUCCESS)
		{
			return (lstrcmpi(result,dbpath) == 0 ? TRUE : FALSE);
		} 
		else 
			return FALSE;
	} 
	else 
		return FALSE;
}

static INT_PTR CALLBACK DlgProcAutorunOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
        CheckDlgButton(hwndDlg,IDC_AUTORUN,CmpCurrentAndRegistry()); // Check chekbox if Registry value exists
		return TRUE;
	case WM_COMMAND:
		SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0); // Send message to activate "Apply" button
		return TRUE;
		break;
	case WM_NOTIFY:
		switch(((LPNMHDR)lParam)->idFrom) {
		case 0:
			switch (((LPNMHDR)lParam)->code)
			{
			case PSN_APPLY: // if "Apply" pressed then...
				SetAutorun(IsDlgButtonChecked(hwndDlg,IDC_AUTORUN)); //Save changes to registry;
				return TRUE;
			}
			break;
		}
		break;
	}
	return FALSE;
}


static int AutorunOptInitialise(WPARAM wParam,LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp;
	odp.cbSize=sizeof(odp);
	odp.position=100100000;
	odp.hInstance=hInst;
	odp.pszTemplate=MAKEINTRESOURCE(IDD_OPT_AUTORUN);
	odp.pszTitle=Translate(ModuleName);
	odp.pszGroup=Translate("Plugins");
	odp.pfnDlgProc=DlgProcAutorunOpts;
	odp.flags=ODPF_BOLDGROUPS;	
	CallService(MS_OPT_ADDPAGE,wParam,(LPARAM)&odp);
	return 0;
}
//==========================================================================
__declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfoEx;
}

static const MUUID interfaces[] = {MIID_AUTORUN, MIID_LAST};
__declspec(dllexport) const MUUID* MirandaPluginInterfaces(void)
{
	return interfaces;
}

int __declspec(dllexport) Load(PLUGINLINK *link)
{
	pluginLink = link;
	hHookOptionInit = HookEvent(ME_OPT_INITIALISE,AutorunOptInitialise);
	return 0;
}

int __declspec(dllexport) Unload(void)
{
	if (hHookOptionInit)
		UnhookEvent(hHookOptionInit);    
	return 0;
}