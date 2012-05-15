#include "commonheaders.h"


// dllmain
BOOL WINAPI DllMain(HINSTANCE hInst, DWORD dwReason, LPVOID) {
	g_hInst = hInst;
	if( dwReason == DLL_PROCESS_ATTACH ) {
		{
		char temp[MAX_PATH];
		GetTempPath(sizeof(temp),temp);
		GetLongPathName(temp,TEMP,sizeof(TEMP));
		TEMP_SIZE = strlen(TEMP);
		if(TEMP[TEMP_SIZE-1]=='\\') {
			TEMP_SIZE--;
			TEMP[TEMP_SIZE]='\0';
		}
		}
		InitializeCriticalSection(&localQueueMutex);
		InitializeCriticalSection(&localContextMutex);
#ifdef _DEBUG
		isVista = 1;
#else
		isVista = ( (DWORD)(LOBYTE(LOWORD(GetVersion()))) == 6 );
#endif
	}
	//DLL_THREAD_ATTACH
	return TRUE;
}


PLUGININFO *MirandaPluginInfo(DWORD mirandaVersion) {
	return &pluginInfo;
}


PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfoEx;
}


MUUID* MirandaPluginInterfaces(void)
{
	return interfaces;
}


int onModulesLoaded(WPARAM wParam,LPARAM lParam) {
    // updater plugin support
#if defined(_DEBUG) || defined(NETLIB_LOG)
	InitNetlib();
#endif
	if(ServiceExists(MS_UPDATE_REGISTERFL)) {
		CallService(MS_UPDATE_REGISTERFL, (WPARAM)2669, (LPARAM)&pluginInfo);
	}
	return 0;
}


int Load(PLUGINLINK *link) {

	pluginLink = link;
	DisableThreadLibraryCalls(g_hInst);

	// get memoryManagerInterface address
	mir_getMMI( &mmi );

	// register plugin module
	PROTOCOLDESCRIPTOR pd;
	memset(&pd,0,sizeof(pd));
	pd.cbSize = sizeof(pd);
	pd.szName = (char*)szModuleName;
	pd.type = PROTOTYPE_ENCRYPTION;
	CallService(MS_PROTO_REGISTERMODULE, 0, (LPARAM)&pd);

	// hook events
	HookEvent(ME_SYSTEM_MODULESLOADED,onModulesLoaded);
	return 0;
}


int Unload() {
	return 0;
}


// EOF
