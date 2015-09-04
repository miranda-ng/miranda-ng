//=====================================================
//	Includes
//=====================================================

#include "stdafx.h"

#include "Version.h"

CLIST_INTERFACE *pcli;
HINSTANCE g_hInst;
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

INT_PTR doubleClick(WPARAM wParam, LPARAM)
{
	char program[MAX_PATH], params[MAX_PATH];
	INT_PTR shellEXEerror = 0;
	char* proto = GetContactProto(wParam);
	if (proto && !mir_strcmp(proto, MODNAME)) {
		if (GetKeyState(VK_CONTROL) & 0x8000) // ctrl is pressed
			editContact(wParam, 0);		// for later when i add a second double click setting
		else if (db_get_static(wParam, MODNAME, "Program", program, _countof(program)) && mir_strcmp(program, "")) {
			if (!db_get_static(wParam, MODNAME, "ProgramParams", params, _countof(params)))
				mir_strcpy(params, "");
			if (strstr(program, "http://") || strstr(program, "https://"))
				Utils_OpenUrl(program);
			else
				shellEXEerror = (INT_PTR)ShellExecuteA(NULL, NULL, program, params, NULL, SW_SHOW);  //ignore the warning, its M$'s backwards compatabilty screwup :)
			if (shellEXEerror == ERROR_FILE_NOT_FOUND || shellEXEerror == ERROR_PATH_NOT_FOUND)
				Utils_OpenUrl(program);
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
	OPTIONSDIALOGPAGE odp = { 0 };
	odp.hInstance = g_hInst;
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

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD)
{
	return &pluginInfoEx;
}

//=====================================================
// Name : WINAPI DllMain
// Parameters: HINSTANCE hinst,DWORD fdwReason,LPVOID lpvReserved
// Returns : BOOL
// Description :
//=====================================================

BOOL WINAPI DllMain(HINSTANCE hinst, DWORD, LPVOID)
{
	g_hInst = hinst;
	return TRUE;
}

int ModulesLoaded(WPARAM, LPARAM)
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

	Icon_Register(g_hInst, LPGEN("Non-IM Contact"), icoList, _countof(icoList));

	HookEvent(ME_CLIST_DOUBLECLICKED, (MIRANDAHOOK)doubleClick);
	HookEvent(ME_OPT_INITIALISE, NimcOptInit);
	HookEvent(ME_CLIST_STATUSMODECHANGE, SetLCStatus);

	PROTOCOLDESCRIPTOR pd = { PROTOCOLDESCRIPTOR_V3_SIZE, MODNAME, PROTOTYPE_VIRTUAL };
	Proto_RegisterModule(&pd);

	//load services (the first 5 are the basic ones needed to make a new protocol)
	CreateProtoServiceFunction(MODNAME, PS_GETCAPS, GetLCCaps);
	CreateProtoServiceFunction(MODNAME, PS_GETNAME, GetLCName);
	CreateProtoServiceFunction(MODNAME, PS_LOADICON, LoadLCIcon);
	CreateProtoServiceFunction(MODNAME, PS_GETSTATUS, GetLCStatus);

	CreateServiceFunction("AddLCcontact", addContact);
	CreateServiceFunction("EditLCcontact", editContact);
	CreateServiceFunction("LoadFilesDlg", LoadFilesDlg);
	CreateServiceFunction("ExportLCcontacts", exportContacts);
	CreateServiceFunction("ImportLCcontacts", ImportContacts);
	CreateServiceFunction("TestStringReplaceLine", testStringReplacer);
	CreateServiceFunction("NIM_Contact/DoubleClick", doubleClick);

	CMenuItem mi;

	SET_UID(mi, 0x73c11266, 0x153c, 0x4da4, 0x9b, 0x82, 0x5c, 0xce, 0xca, 0x86, 0xd, 0x41);
	mi.position = 600090000;
	mi.root = Menu_CreateRoot(MO_MAIN, LPGENT("&Non-IM Contact"), 600090000);
	mi.name.a = LPGEN("&Add Non-IM Contact");
	mi.pszService = "AddLCcontact";
	mi.hIcolibItem = icoList[0].hIcolib;
	Menu_AddMainMenuItem(&mi);

	SET_UID(mi, 0xa511c5e, 0x26d2, 0x41b1, 0xbd, 0xb7, 0x3e, 0x62, 0xc8, 0x44, 0x37, 0xc9);
	mi.position = 600090001;
	mi.name.a = LPGEN("&View/Edit Files");
	mi.pszService = "LoadFilesDlg";
	Menu_AddMainMenuItem(&mi);

	if (db_get_b(NULL, MODNAME, "Beta", 0)) {
		SET_UID(mi, 0x23051356, 0xad45, 0x4101, 0x8e, 0x11, 0xf6, 0x3a, 0xe8, 0xa3, 0xa5, 0x25);
		mi.position = 600090002;
		mi.name.a = LPGEN("&Export all Non-IM Contacts");
		mi.pszService = "ExportLCcontacts";
		Menu_AddMainMenuItem(&mi);

		SET_UID(mi, 0xf3c4ebed, 0x789c, 0x4293, 0xaa, 0xcb, 0x22, 0xdc, 0xd4, 0xe0, 0x3c, 0x41);
		mi.position = 600090003;
		mi.name.a = LPGEN("&Import Non-IM Contacts");
		mi.pszService = "ImportLCcontacts";
		Menu_AddMainMenuItem(&mi);
	}

	SET_UID(mi, 0xb653d5e0, 0xb1e6, 0x46fc, 0xa7, 0x82, 0x35, 0x95, 0x74, 0xb1, 0xc, 0xdd);
	mi.position = 600090000;
	mi.name.a = LPGEN("&String Maker");
	mi.pszService = "TestStringReplaceLine";
	Menu_AddMainMenuItem(&mi);

	SET_UID(mi, 0x1033e16d, 0x8a7c, 0x43db, 0xa9, 0x83, 0x56, 0x2f, 0x8a, 0x16, 0x7c, 0xe9);
	mi.root = NULL;
	mi.position = -2000080000;
	mi.name.a = LPGEN("E&dit Contact Settings");
	mi.pszService = "EditLCcontact";
	Menu_AddContactMenuItem(&mi, MODNAME);

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
