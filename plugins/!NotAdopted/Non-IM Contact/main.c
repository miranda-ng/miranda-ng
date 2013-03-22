

//=====================================================
//	Includes
//=====================================================
#include "commonheaders.h"

#pragma comment(linker,"/MERGE:.rdata=.text")
#pragma comment(linker,"/FILEALIGN:512 /SECTION:.text,EWRX /IGNORE:4078")

HINSTANCE hinstance;
PLUGINLINK *pluginLink;

PLUGININFO pluginInfo={
	sizeof(PLUGININFO),
	"Non-IM Contact",
	PLUGIN_MAKE_VERSION(0,5,0,1),
	"Non-IM Contact allows you to add \"contacts\" that can act as shortcuts to other programs, or links to web pages.\r\nThe contacts name can be read from a text file (includes any ASCII file).\r\nThis plugin is a combination of Favorites and Text Reader plugins both made by me)",
	"Jonathan Gordon",
	"ICQ 98791178, MSN jonnog@hotmail.com",
	"© 2003-2004 Jonathan Gordon, jdgordy@gmail.com",
	"",		
	0,
	0
};

int doubleClick(WPARAM wParam,LPARAM lParam)
{
	char program[MAX_PATH], params[MAX_PATH];
	int shellEXEerror = 0;
	char* proto = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)wParam, 0);
	if (proto && !strcmp(proto, modname))
	{
		if (GetKeyState(VK_CONTROL)&0x8000) // ctrl is pressed
			editContact((WPARAM)(HANDLE)wParam, (LPARAM)NULL);		// for later when i add a second double click setting
		else if (DBGetContactSettingString((HANDLE)wParam, modname, "Program", program) && strcmp(program, ""))
		{
			if (!DBGetContactSettingString((HANDLE)wParam, modname, "ProgramParams", params) )
				strcpy(params, "");
			if (strstr(program, "http://") || strstr(program, "https://"))
				CallService(MS_UTILS_OPENURL,1,(LPARAM)program);
			else shellEXEerror = (int)ShellExecute(NULL,NULL,program,params,NULL,SW_SHOW);  //ignore the warning, its M$'s backwards compatabilty screwup :)
			if (shellEXEerror == ERROR_FILE_NOT_FOUND || shellEXEerror == ERROR_PATH_NOT_FOUND)
				CallService(MS_UTILS_OPENURL,1,(LPARAM)program);
		}
		else editContact((WPARAM)(HANDLE)wParam, (LPARAM)NULL);
		return 1;
	}
	return 0;	
}

//=====================================================
//	Definitions
//=====================================================
int LCStatus = ID_STATUS_OFFLINE;
//=====================================================

//=====================================================
// Name : MainInit
// Parameters: wparam , lparam
// Returns : int
// Description : Called at very beginning of plugin
//=====================================================
int NimcOptInit(WPARAM wParam,LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp;
	
	ZeroMemory(&odp,sizeof(odp));
	odp.cbSize=sizeof(odp);
	odp.position=0;
	odp.hInstance=hInst;
	odp.pszTemplate=MAKEINTRESOURCE(IDD_OPTIONS);
	odp.pszGroup= Translate("Plugins");
	odp.pszTitle=Translate("Non-IM Contacts");
	odp.pfnDlgProc=DlgProcNimcOpts;
	odp.expertOnlyControls=NULL;
	CallService(MS_OPT_ADDPAGE,wParam,(LPARAM)&odp);
	
	return 0;
}

int MainInit(WPARAM wparam,LPARAM lparam)
{
	return 0;
}

//=====================================================
// Name : __declspec(dllexport) PLUGININFO* MirandaPluginInfo
// Parameters: (DWORD mirandaVersion)
// Returns : 
// Description : Sets plugin info
//=====================================================

__declspec(dllexport) PLUGININFO* MirandaPluginInfo(DWORD mirandaVersion)
{
return &pluginInfo;
}

//=====================================================
// Name : Unload
// Parameters: void
// Returns : 
// Description : Unloads plugin
//=====================================================

__declspec(dllexport)int Unload(void)
{
	return 0;
}

//=====================================================
// Name : WINAPI DllMain
// Parameters: HINSTANCE hinst,DWORD fdwReason,LPVOID lpvReserved
// Returns : BOOL
// Description :
//=====================================================

BOOL WINAPI DllMain(HINSTANCE hinst,DWORD fdwReason,LPVOID lpvReserved)
{
	hInst=hinst;
	return TRUE;
}
HANDLE hModulesLoadedHook;
int ModulesLoaded(WPARAM wParam,LPARAM lParam) 
{
	NetlibInit();
	UnhookEvent(hModulesLoadedHook);
	return 0;
}

//=====================================================
// Name : Load
// Parameters: PLUGINLINK *link
// Returns : int
// Description : Called when plugin is loaded into Miranda
//=====================================================

int __declspec(dllexport)Load(PLUGINLINK *link)
{
	
	PROTOCOLDESCRIPTOR pd;
	CLISTMENUITEM mi;

	pluginLink=link;
	
	HookEvent(ME_CLIST_DOUBLECLICKED,doubleClick);
	HookEvent(ME_OPT_INITIALISE,NimcOptInit);

	ZeroMemory(&pd,sizeof(pd));
	pd.cbSize=sizeof(pd);
	pd.szName=modname;
	pd.type=PROTOTYPE_PROTOCOL;
	CallService(MS_PROTO_REGISTERMODULE,0,(LPARAM)&pd);

	//load services (the first 5 are the basic ones needed to make a new protocol)
	CreateServiceFunction(modname PS_GETCAPS,GetLCCaps);
	CreateServiceFunction(modname PS_GETNAME,GetLCName);
	CreateServiceFunction(modname PS_LOADICON,LoadLCIcon);
	HookEvent(ME_CLIST_STATUSMODECHANGE,SetLCStatus);
	CreateServiceFunction(modname PS_GETSTATUS,GetLCStatus);

	
	CreateServiceFunction("AddLCcontact",addContact);
	CreateServiceFunction("EditLCcontact",editContact);
	CreateServiceFunction("LoadFilesDlg",LoadFilesDlg);
	CreateServiceFunction("ExportLCcontacts",exportContacts);
	CreateServiceFunction("ImportLCcontacts",ImportContacts);
	CreateServiceFunction("TestStringReplaceLine",testStringReplacer);
	CreateServiceFunction("NIM_Contact/DoubleClick",doubleClick);


	ZeroMemory(&mi,sizeof(mi));
	mi.cbSize=sizeof(mi);
	mi.position=600090000;
	mi.flags=0;
	mi.pszContactOwner=NULL;
	mi.pszPopupName=Translate("&Non-IM Contact");
	mi.popupPosition=600090000;
	mi.pszName=Translate("&Add Non-IM Contact");
	mi.pszService="AddLCcontact";
	mi.hIcon=LoadIcon(hInst,MAKEINTRESOURCE(IDI_MAIN));
	CallService(MS_CLIST_ADDMAINMENUITEM,0,(LPARAM)&mi);

	ZeroMemory(&mi,sizeof(mi));
	mi.cbSize=sizeof(mi);
	mi.position=600090001;
	mi.flags=0;
	mi.pszContactOwner=NULL;
	mi.pszPopupName=Translate("&Non-IM Contact");
	mi.popupPosition=600090000;
	mi.pszName=Translate("&View/Edit Files");
	mi.pszService="LoadFilesDlg";
	mi.hIcon=LoadIcon(hInst,MAKEINTRESOURCE(IDI_MAIN));
	CallService(MS_CLIST_ADDMAINMENUITEM,0,(LPARAM)&mi);

	if (DBGetContactSettingByte(NULL, modname, "Beta",0))
	{
		ZeroMemory(&mi,sizeof(mi));
		mi.cbSize=sizeof(mi);
		mi.position=600090000;
		mi.flags=0;
		mi.pszContactOwner=NULL;
		mi.pszPopupName=Translate("&Non-IM Contact");
		mi.popupPosition=600090000;
		mi.pszName=Translate("&Export all Non-IM Contacts");
		mi.pszService="ExportLCcontacts";
	//	mi.hIcon=LoadIcon(hInst,MAKEINTRESOURCE(IDI_MAIN));
		CallService(MS_CLIST_ADDMAINMENUITEM,0,(LPARAM)&mi);

		ZeroMemory(&mi,sizeof(mi));
		mi.cbSize=sizeof(mi);
		mi.position=600090000;
		mi.flags=0;
		mi.popupPosition=600090000;
		mi.pszContactOwner=NULL;
		mi.pszPopupName=Translate("&Non-IM Contact");
		mi.pszName=Translate("&Import Non-IM Contacts");
		mi.pszService="ImportLCcontacts";
	//	mi.hIcon=LoadIcon(hInst,MAKEINTRESOURCE(IDI_MAIN));
		CallService(MS_CLIST_ADDMAINMENUITEM,0,(LPARAM)&mi);
	}

	ZeroMemory(&mi,sizeof(mi));
	mi.cbSize=sizeof(mi);
	mi.position=600090000;
	mi.flags=0;
	mi.popupPosition=600090000;
	mi.pszContactOwner=NULL;
	mi.pszPopupName=Translate("&Non-IM Contact");
	mi.pszName=Translate("&String Maker");
	mi.pszService="TestStringReplaceLine";
//	mi.hIcon=LoadIcon(hInst,MAKEINTRESOURCE(IDI_MAIN));
	CallService(MS_CLIST_ADDMAINMENUITEM,0,(LPARAM)&mi);

	ZeroMemory(&mi,sizeof(mi));
	mi.cbSize=sizeof(mi);
	mi.position=-2000080000;
	mi.flags=0;
	mi.pszContactOwner=modname;
	mi.pszName=Translate("E&dit Contact Settings");
	mi.pszService="EditLCcontact";
	mi.hIcon=LoadIcon(hInst,MAKEINTRESOURCE(IDI_MAIN));
	CallService(MS_CLIST_ADDCONTACTMENUITEM,0,(LPARAM)&mi);

	hWindowList=(HWND)CallService(MS_UTILS_ALLOCWINDOWLIST,0,0);
	hModulesLoadedHook = HookEvent(ME_SYSTEM_MODULESLOADED,ModulesLoaded);
	

	{   // known modules list
		DBVARIANT dbv;
		if (DBGetContactSetting(NULL, "KnownModules","Non-IM Contact", &dbv))
			DBWriteContactSettingString(NULL, "KnownModules","Non-IM Contact", modname);
		DBFreeVariant(&dbv);
	}
	
	return 0;
}
/*
//uninstall support
int __declspec(dllexport) UninstallEx(PLUGINUNINSTALLPARAMS* ppup) 
{ 
    // Delete Files 
    const char* apszFiles[] = {"nimcontact_readme.txt", 0}; 
    PUIRemoveFilesInDirectory(ppup->pszPluginsPath, apszFiles); 
	
	  if((ppup->bDoDeleteSettings == TRUE) && (ppup->bIsMirandaRunning == TRUE)) 
		{
			char szModule[] = modname; 
			PUICallService(MS_PLUGINUNINSTALLER_REMOVEDBMODULE, (WPARAM)szModule, (LPARAM)NULL); 
		}
	return 0;
}*/