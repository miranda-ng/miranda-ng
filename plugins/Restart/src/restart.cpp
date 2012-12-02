#include <windows.h>
#include <newpluginapi.h>
#include <m_clist.h>
#include <m_skin.h>
#include <m_icolib.h>
#include <m_langpack.h>
#include <m_system.h>
#include <m_genmenu.h>
#include <m_utils.h>
#include <win2k.h>
#include "resource.h"

HINSTANCE hInst;
int hLangpack;
HANDLE hRestartMe;

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

static INT_PTR RestartMe(WPARAM wParam, LPARAM lParam)
{
	TCHAR mirandaPath[MAX_PATH], cmdLine[100];
	PROCESS_INFORMATION pi;
	STARTUPINFO si = {0};
	si.cb = sizeof(si);
	GetModuleFileName(NULL, mirandaPath, SIZEOF(mirandaPath));
	TCHAR *profilename = Utils_ReplaceVarsT(_T("%miranda_profilename%"));
	mir_sntprintf(cmdLine, SIZEOF(cmdLine), _T("\"%s\" /restart:%d /profile=%s"), mirandaPath, GetCurrentProcessId(), profilename);
	CallService("CloseAction", 0, 0);
	CreateProcess(mirandaPath, cmdLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
	mir_free(profilename);
	return 0;
}

static IconItem icon = { LPGEN("Restart"), "rst_restart_icon", IDI_RESTARTICON };

extern "C" __declspec(dllexport) int Load(void)
{
	mir_getLP( &pluginInfo );

	// IcoLib support
	Icon_Register(hInst, "Restart Plugin", &icon, 1);

	hRestartMe = CreateServiceFunction("System/RestartMe", RestartMe);

	CLISTMENUITEM mi = { sizeof(mi) };
	mi.position = -0x7FFFFFFF;
	mi.flags = CMIF_ICONFROMICOLIB | CMIF_TCHAR;
	mi.icolibItem = icon.hIcolib;
	mi.ptszName = _T("Restart");
	mi.pszService = "System/RestartMe";
	Menu_AddMainMenuItem(&mi);
	Menu_AddTrayMenuItem(&mi);
	return 0;
}

extern "C" __declspec(dllexport) int Unload(void)
{
	return 0;
}
