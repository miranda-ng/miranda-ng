#include "common.h"
#include "version.h"
#include "mirandaInterface.h"


//Miranda - handle from DllMain
HINSTANCE hInst;

//Miranda - Langpack
int hLangpack;

//Miranda - HANDLE to hooked protocols ACK
HANDLE hProtoAck;

//popup classes handles
HANDLE hPopupNotify;
HANDLE hPopupError;

//main add-on's object
CMirfoxMiranda mirfoxMiranda;

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
	{ 0xe99a09b2, 0xe05b, 0x4633, { 0xaa, 0x3a, 0x5c, 0x83, 0xef, 0x1c, 0xba, 0xb6 } }
};


extern "C" BOOL APIENTRY DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {

	hInst = hinstDLL;
	return TRUE;
}


extern "C" __declspec (dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion) {
	return &pluginInfo;
}


/*
 * hook on ME_SYSTEM_MODULESLOADED at Load()
 */
static int onModulesLoaded(WPARAM, LPARAM)
{
	//init popup classes
	POPUPCLASS puc = {0};
	puc.cbSize = sizeof(puc);
	puc.flags = PCF_TCHAR;

	puc.pszName = "MirFox_Notify";
	puc.ptszDescription = TranslateT("MirFox/Notification");
	puc.colorBack = RGB(173, 206, 247); //light blue
	puc.colorText = GetSysColor(COLOR_WINDOWTEXT);
	puc.iSeconds = 3;
	puc.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON_PN));
	hPopupNotify = Popup_RegisterClass(&puc);

	puc.pszName = "MirFox_Error";
	puc.ptszDescription = TranslateT("MirFox/Error");
	puc.colorBack = RGB(255, 128, 128); //light red
	puc.colorText = GetSysColor(COLOR_WINDOWTEXT);
	puc.iSeconds = 20;
	puc.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON_PE));
	hPopupError = Popup_RegisterClass(&puc);

	return 0;
}

static int OnShutdown(WPARAM, LPARAM)
{
	Popup_UnregisterClass(hPopupError);
	Popup_UnregisterClass(hPopupNotify);

	return 0;
}


extern "C" int __declspec(dllexport) Load(void){
	mir_getLP(&pluginInfo);
	HookEvent(ME_SYSTEM_MODULESLOADED, onModulesLoaded);
	HookEvent(ME_SYSTEM_SHUTDOWN, OnShutdown);

	hProtoAck = HookEvent(ME_PROTO_ACK, MirandaUtils::onProtoAck);

	//Ensure that the common control DLL is loaded. needed to use ICC_LISTVIEW_CLASSES control in options TODO move to InitOptions();?
	INITCOMMONCONTROLSEX icex = {0};
	icex.dwSize = sizeof(icex);
	icex.dwICC = ICC_LISTVIEW_CLASSES;
	InitCommonControlsEx(&icex);

	mirfoxMiranda.getMirfoxData().Plugin_Terminated = false;
	mirfoxMiranda.getMirfoxData().setPluginState(MFENUM_PLUGIN_STATE_INIT);
	mirfoxMiranda.onMirandaInterfaceLoad();
	InitOptions();
	
	if (mirfoxMiranda.getMirfoxData().getPluginState() != MFENUM_PLUGIN_STATE_ERROR){
		mirfoxMiranda.getMirfoxData().setPluginState(MFENUM_PLUGIN_STATE_WORK);
	}  /*else {
		mirfoxMiranda.onMirandaInterfaceUnload();
		DeinitOptions();
	} */ //TODO
	
	return 0;
}


extern "C" __declspec (dllexport) int Unload(void) {

	MFLogger::getInstance()->log(L"Unload: start");


	//wait for csmThread, msgQueueThread and userActionThread's end
	mirfoxMiranda.getMirfoxData().Plugin_Terminated = true;
	UnhookEvent(hProtoAck);
	int counter = 0;
	const int UNLOAD_WAIT_TIME = 50;		//[ms]
	const int MAX_UNLOAD_WAIT_COUNTER = 10;	//10 * 50ms = 0,5s
	while (mirfoxMiranda.getMirfoxData().workerThreadsCount > 0 ){
		MFLogger::getInstance()->log_p(L"Unload: workerThreadsCount=%d > 0, waiting. counter=%d", mirfoxMiranda.getMirfoxData().workerThreadsCount, counter);
		SleepEx(UNLOAD_WAIT_TIME, TRUE);
		counter++;
	}
	MFLogger::getInstance()->log_p(L"Unload: workerThreadsCount=%d. counter=%d", mirfoxMiranda.getMirfoxData().workerThreadsCount, counter);


	MFLogger::getInstance()->log(L"Unload: last log");

	mirfoxMiranda.onMirandaInterfaceUnload();
	DeinitOptions();

	return 0;
}

