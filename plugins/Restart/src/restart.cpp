#include <windows.h>
#include <newpluginapi.h>
#include <m_clist.h>
#include <m_skin.h>
#include <m_icolib.h>
#include <m_langpack.h>
#include <m_system.h>
#include <m_genmenu.h>
#include "resource.h"

HINSTANCE hInst;
HANDLE hIconHandle;
int hLangpack;

PLUGININFOEX pluginInfo={
	sizeof(PLUGININFOEX),
	"Restart plugin",
	PLUGIN_MAKE_VERSION(0,0,6,0),
	"Adds Restart menu item.",
	"(..нужное вписать..)",
	"ep@eugn.me",
	"© 2008 - 2012 -=J-Scar=-",
	"http://miranda-ng.org",
	UNICODE_AWARE,	 
	0x61bedf3a, 0xcc2, 0x41a3, { 0xb9, 0x80, 0xbb, 0x93, 0x93, 0x36, 0x89, 0x35 } // {61BEDF3A-0CC2-41a3-B980-BB9393368935}
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

extern "C" __declspec(dllexport) int Load(void)
{	
	SKINICONDESC sid = {0};
	CLISTMENUITEM mi = {0};
	TCHAR szFile[MAX_PATH];
	
	// IcoLib support
	GetModuleFileName(hInst, szFile, MAX_PATH);
	sid.ptszDefaultFile = szFile;
	sid.cbSize = sizeof(sid);
	sid.flags = SIDF_ALL_TCHAR;

	sid.ptszSection = _T("Restart Plugin");
	sid.ptszDescription = _T("Restart");
	sid.pszName = "rst_restart_icon";
	sid.iDefaultIndex = -IDI_RESTARTICON;
	hIconHandle = Skin_AddIcon(&sid);
	
	mi.cbSize = sizeof(mi);
	mi.position = -0x7FFFFFFF;
	mi.flags = CMIF_ICONFROMICOLIB | CMIF_TCHAR;
	mi.icolibItem = hIconHandle;
	mi.ptszName = _T("Restart");
	mi.pszService = "Miranda/System/Restart";
	Menu_AddMainMenuItem(&mi);
	Menu_AddTrayMenuItem(&mi);
	return 0;
}

extern "C" __declspec(dllexport) int Unload(void)
{
	return 0;
}