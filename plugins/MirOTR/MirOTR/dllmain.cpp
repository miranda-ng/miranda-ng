// dllmain.cpp : Definiert den Einstiegspunkt für die DLL-Anwendung.
#include "stdafx.h"
#include "dllmain.h"
#include "otr.h"
#include "dbfilter.h"
#include <commctrl.h>

HANDLE hEventWindow;
HINSTANCE hInst;

int hLangpack;
char* g_metaproto;


PLUGININFOEX pluginInfo={
	sizeof(PLUGININFOEX),
	SHORT_NAME_STRING,
	PLUGIN_MAKE_VERSION(VER_MAJOR,VER_MINOR,VER_RELEASE,VER_BUILD),
	LONGDESC_STRING,
	AUTHOR,
	AUTHOR_MAIL,
	LEGAL_COPYRIGHT_LONG,
	HOMEPAGE,
	UNICODE_AWARE,
	MIID_OTRPLUGIN		// ANSI and Unicode have different IDs
};

BOOL APIENTRY DllMain( HMODULE hModule,
					   DWORD  ul_reason_for_call,
					   LPVOID lpReserved
					 )
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
		hInst=hModule;
		OTRL_INIT;
		INITCOMMONCONTROLSEX icce = {0};
		icce.dwSize = sizeof(icce);
		icce.dwICC = ICC_LISTVIEW_CLASSES|ICC_PROGRESS_CLASS;
		InitCommonControlsEx(&icce);
	}
	return TRUE;
}

DLLFUNC PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}

int ModulesLoaded(WPARAM wParam, LPARAM lParam) {
	if(ServiceExists(MS_MC_GETPROTOCOLNAME))
		g_metaproto = (char *)CallService(MS_MC_GETPROTOCOLNAME, 0, 0);

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
	if (!options.bHaveSRMMIcons) {
		MessageBox(0, _T("OTR Info"), _T("Your SRMM plugin does not support status icons. Not all OTR-functions are available!"), 0x30);
	}

	// HookEvent(ME_OPT_INITIALISE, OptInit);

	// hook setting changed to monitor status
	//hSettingChanged = HookEvent(ME_DB_CONTACT_SETTINGCHANGED, SettingChanged);

	// hook status mode changes to terminate sessions when we go offline
	// (this would be hooked as the ME_CLIST_STATUSMODECHANGE handler except that event is sent *after* the proto goes offline)
	// (instead, it's called from the SettingChanged handler for protocol status db setting changes)
	//HookEvent(ME_CLIST_STATUSMODECHANGE, StatusModeChange);

	return 0;
}

DLLFUNC int Load(void)
{
	DEBUGOUT_T("LOAD MIROTR")

	mir_getLP( &pluginInfo );
	/* for timezones
	 mir_getTMI(&tmi);  */

	CallService(MS_DB_SETSETTINGRESIDENT, TRUE, (LPARAM)(MODULENAME "/TrustLevel"));

	/////////////
	////// init plugin
	PROTOCOLDESCRIPTOR pd = {0};
	pd.cbSize = sizeof(pd);
	pd.szName = MODULENAME;
	pd.type = PROTOTYPE_ENCRYPTION;
	CallService(MS_PROTO_REGISTERMODULE,0,(LPARAM)&pd);

	// remove us as a filter to all contacts - fix filter type problem
	if(DBGetContactSettingByte(0, MODULENAME, "FilterOrderFix", 0) != 2) {
		HANDLE hContact = ( HANDLE )CallService( MS_DB_CONTACT_FINDFIRST, 0, 0 );
		while ( hContact != NULL ) {
			CallService( MS_PROTO_REMOVEFROMCONTACT, ( WPARAM )hContact, ( LPARAM )MODULENAME );
			hContact = ( HANDLE )CallService( MS_DB_CONTACT_FINDNEXT,( WPARAM )hContact, 0 );
		}
		DBWriteContactSettingByte(0, MODULENAME, "FilterOrderFix", 2);
	}

	// create our services
	CreateProtoServiceFunction(MODULENAME, PSS_MESSAGE, SVC_OTRSendMessage);
	CreateProtoServiceFunction(MODULENAME, PSS_MESSAGE"W", SVC_OTRSendMessageW);
	CreateProtoServiceFunction(MODULENAME, PSR_MESSAGE, SVC_OTRRecvMessage);


	// hook modules loaded for updater support
	HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);

	return 0;
}

DLLFUNC int Unload(void)
{
	//UnhookEvent(hSettingChanged);
	UnhookEvent(hEventWindow);
	//UnhookEvent(hEventDbEventAddedFilter);
	//UnhookEvent(hEventDbEventAdded);
	DEBUGOUT_T("UNLOAD MIROTR")
	DeinitSRMM();
	DeinitOptions();
	DeinitMenu();
	DeinitDBFilter();

	lib_cs_lock();
	otrl_userstate_free(otr_user_state);
	lib_cs_unlock();

	DeinitIcons();
	DeinitUtils();

	return 0;
}