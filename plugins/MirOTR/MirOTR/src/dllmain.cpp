// dllmain.cpp : Definiert den Einstiegspunkt für die DLL-Anwendung.
#include "stdafx.h"

HANDLE hEventWindow;
HINSTANCE hInst;

int hLangpack;

PLUGININFOEX pluginInfo={
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {12D8FAAD-78AB-4E3C-9854-320E9EA5CC9F}
	{0x12d8faad, 0x78ab, 0x4e3c, {0x98, 0x54, 0x32, 0xe, 0x9e, 0xa5, 0xcc, 0x9f}}
};

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	hInst = hModule;
	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}

int ModulesLoaded(WPARAM wParam, LPARAM lParam)
{
	InitUtils();

	lib_cs_lock();
	otr_user_state = otrl_userstate_create();
	lib_cs_unlock();

	// this calls ReadPrivkeyFiles (above) to set filename values (also called on ME_FOLDERS_PATH_CHANGED)
	InitOptions();

	InitDBFilter();
	InitIcons();
	InitMenu();

	InitSRMM();

	hEventWindow = HookEvent(ME_MSG_WINDOWEVENT, WindowEvent);

	if (options.bHaveSecureIM && !db_get_b(0, MODULENAME, "sim_warned", 0)) {
		db_set_b(0, MODULENAME, "sim_warned", 1);
		options.default_policy = OTRL_POLICY_MANUAL_MOD;
		SaveOptions();
		MessageBox(0, TranslateT(LANG_OTR_SECUREIM_TEXT), TranslateT(LANG_OTR_SECUREIM_TITLE), 0x30);
	}
	return 0;
}

extern "C" __declspec(dllexport) int Load(void)
{
	DEBUGOUT_T("LOAD MIROTR")

	mir_getLP( &pluginInfo );
	/* for timezones
	 mir_getTMI(&tmi);  */

	OTRL_INIT;
	INITCOMMONCONTROLSEX icce = {0};
	icce.dwSize = sizeof(icce);
	icce.dwICC = ICC_LISTVIEW_CLASSES|ICC_PROGRESS_CLASS;
	InitCommonControlsEx(&icce);

	db_set_resident(MODULENAME, "TrustLevel");

	/////////////
	////// init plugin
	PROTOCOLDESCRIPTOR pd = { sizeof(pd) };
	pd.szName = MODULENAME;
	pd.type = PROTOTYPE_ENCRYPTION;
	CallService(MS_PROTO_REGISTERMODULE,0,(LPARAM)&pd);

	// remove us as a filter to all contacts - fix filter type problem
	if(db_get_b(0, MODULENAME, "FilterOrderFix", 0) != 2) {
		for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact))
			CallService(MS_PROTO_REMOVEFROMCONTACT, hContact, (LPARAM)MODULENAME);
		db_set_b(0, MODULENAME, "FilterOrderFix", 2);
	}

	// create our services
	CreateProtoServiceFunction(MODULENAME, PSS_MESSAGE, SVC_OTRSendMessage);
	CreateProtoServiceFunction(MODULENAME, PSR_MESSAGE, SVC_OTRRecvMessage);

	// hook modules loaded for updater support
	HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);

	return 0;
}

extern "C" __declspec(dllexport) int Unload(void)
{
	//UnhookEvent(hSettingChanged);
	UnhookEvent(hEventWindow);
	//UnhookEvent(hEventDbEventAddedFilter);
	//UnhookEvent(hEventDbEventAdded);
	DEBUGOUT_T("UNLOAD MIROTR")
	DeinitSRMM();
	DeinitOptions();
	DeinitDBFilter();

	lib_cs_lock();
	otrl_userstate_free(otr_user_state);
	lib_cs_unlock();

	DeinitUtils();

	return 0;
}