//=====================================================
//	Includes
//=====================================================

#include "commonheaders.h"

#include "Version.h"

CLIST_INTERFACE *pcli;
HINSTANCE hInst;
int hLangpack;

PLUGININFOEX pluginInfoEx = {
	sizeof(pluginInfoEx),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	//2e0d2ae3-e123-4607-8539-d4448d675ddb
	{ 0x2e0d2ae3, 0xe123, 0x4607, {0x85, 0x39, 0xd4, 0x44, 0x8d, 0x67, 0x5d, 0xdb} }
};

INT_PTR doubleClick(WPARAM wParam, LPARAM lParam)
{
	char program[MAX_PATH], params[MAX_PATH];
	int shellEXEerror = 0;
	char* proto = GetContactProto(wParam);
	if (proto && !strcmp(proto, MODNAME)) {
		if (GetKeyState(VK_CONTROL) & 0x8000) // ctrl is pressed
			editContact(wParam, 0);		// for later when i add a second double click setting
		else if (db_get_static(wParam, MODNAME, "Program", program, SIZEOF(program)) && strcmp(program, "")) {
			if (!db_get_static(wParam, MODNAME, "ProgramParams", params, SIZEOF(params)))
				strcpy(params, "");
			if (strstr(program, "http://") || strstr(program, "https://"))
				CallService(MS_UTILS_OPENURL, OUF_NEWWINDOW, (LPARAM)program);
			else shellEXEerror = (int)ShellExecuteA(NULL, NULL, program, params, NULL, SW_SHOW);  //ignore the warning, its M$'s backwards compatabilty screwup :)
			if (shellEXEerror == ERROR_FILE_NOT_FOUND || shellEXEerror == ERROR_PATH_NOT_FOUND)
				CallService(MS_UTILS_OPENURL, OUF_NEWWINDOW, (LPARAM)program);
		}
		else editContact(wParam, 0);
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
int NimcOptInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = { sizeof(odp) };
	odp.hInstance = hInst;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS);
	odp.pszGroup = LPGEN("Plugins");
	odp.pszTitle = LPGEN("Non-IM Contacts");
	odp.pfnDlgProc = DlgProcNimcOpts;
	Options_AddPage(wParam, &odp);
	return 0;
}
//=====================================================
// Name : __declspec(dllexport) PLUGININFO* MirandaPluginInfo
// Parameters: (DWORD mirandaVersion)
// Returns :
// Description : Sets plugin info
//=====================================================

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfoEx;
}

//=====================================================
// Name : WINAPI DllMain
// Parameters: HINSTANCE hinst,DWORD fdwReason,LPVOID lpvReserved
// Returns : BOOL
// Description :
//=====================================================

BOOL WINAPI DllMain(HINSTANCE hinst, DWORD fdwReason, LPVOID lpvReserved)
{
	hInst = hinst;
	return TRUE;
}

int ModulesLoaded(WPARAM wParam, LPARAM lParam)
{
	NetlibInit();
	return 0;
}

//=====================================================
// Name : Load
// Parameters: PLUGINLINK *link
// Returns : int
// Description : Called when plugin is loaded into Miranda
//=====================================================

IconItem icoList[] =
{
	{ LPGEN("Main Icon"), MODNAME, IDI_MAIN },
};

extern "C" __declspec(dllexport) int Load()
{
	mir_getLP(&pluginInfoEx);
	mir_getCLI();

	Icon_Register(hInst, LPGEN("Non-IM Contact"), icoList, SIZEOF(icoList));

	HookEvent(ME_CLIST_DOUBLECLICKED, (MIRANDAHOOK)doubleClick);
	HookEvent(ME_OPT_INITIALISE, NimcOptInit);
	HookEvent(ME_CLIST_STATUSMODECHANGE, SetLCStatus);

	PROTOCOLDESCRIPTOR pd = { PROTOCOLDESCRIPTOR_V3_SIZE, MODNAME, PROTOTYPE_VIRTUAL };
	CallService(MS_PROTO_REGISTERMODULE, 0, (LPARAM)&pd);

	//load services (the first 5 are the basic ones needed to make a new protocol)
	CreateServiceFunction(MODNAME PS_GETCAPS, GetLCCaps);
	CreateServiceFunction(MODNAME PS_GETNAME, GetLCName);
	CreateServiceFunction(MODNAME PS_LOADICON, LoadLCIcon);
	CreateServiceFunction(MODNAME PS_GETSTATUS, GetLCStatus);

	CreateServiceFunction("AddLCcontact", addContact);
	CreateServiceFunction("EditLCcontact", editContact);
	CreateServiceFunction("LoadFilesDlg", LoadFilesDlg);
	CreateServiceFunction("ExportLCcontacts", exportContacts);
	CreateServiceFunction("ImportLCcontacts", ImportContacts);
	CreateServiceFunction("TestStringReplaceLine", testStringReplacer);
	CreateServiceFunction("NIM_Contact/DoubleClick", doubleClick);

	CLISTMENUITEM mi = { sizeof(mi) };
	mi.position = 600090000;
	mi.pszPopupName = LPGEN("&Non-IM Contact");
	mi.popupPosition = 600090000;
	mi.pszName = LPGEN("&Add Non-IM Contact");
	mi.pszService = "AddLCcontact";
	mi.icolibItem = icoList[0].hIcolib;
	Menu_AddMainMenuItem(&mi);

	mi.position = 600090001;
	mi.pszName = LPGEN("&View/Edit Files");
	mi.pszService = "LoadFilesDlg";
	Menu_AddMainMenuItem(&mi);

	if (db_get_b(NULL, MODNAME, "Beta", 0)) {
		mi.position = 600090002;
		mi.pszName = LPGEN("&Export all Non-IM Contacts");
		mi.pszService = "ExportLCcontacts";
		Menu_AddMainMenuItem(&mi);

		mi.position = 600090003;
		mi.pszName = LPGEN("&Import Non-IM Contacts");
		mi.pszService = "ImportLCcontacts";
		Menu_AddMainMenuItem(&mi);
	}

	mi.position = 600090000;
	mi.pszPopupName = LPGEN("&Non-IM Contact");
	mi.pszName = LPGEN("&String Maker");
	mi.pszService = "TestStringReplaceLine";
	Menu_AddMainMenuItem(&mi);

	mi.position = -2000080000;
	mi.pszContactOwner = MODNAME;
	mi.pszName = LPGEN("E&dit Contact Settings");
	mi.pszService = "EditLCcontact";
	Menu_AddContactMenuItem(&mi);

	HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);

	return 0;
}

//=====================================================
// Name : Unload
// Parameters: void
// Returns :
// Description : Unloads plugin
//=====================================================

extern "C" __declspec(dllexport) int Unload(void)
{
	killTimer();
	return 0;
}
